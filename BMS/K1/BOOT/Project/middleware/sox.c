
#include "sox.h"
#include "rtc.h"

#define LOG_TAG     "sox"
#define PACK_RATED_CAPACITY				100000 /*电池单体容量为100000mAh*/
#define PACK_CURRENT_MIN_THRESHOLD		500 /*电池最小电流阈值为ADC采样的误差 单位mA*/
#define PACK_ACT_MAX_SOC_THRESHOLD		100 //SOC的最大阈值
#define PACK_ACT_MIN_SOC_THRESHOLD		5  //SOC的最小阈值
#define SOX_FLASH_INFO_ADDR		        0x00101000
#define SOX_FLASH_BACKUP_INFO_ADDR	    0x00103000
#define CHARGE_MAX_VOLTAGE              3650 /*充电的最大电压 	3650mV*/
#define DISCHARGE_MIN_VOLTAGE           2700/*放电的最小电压为2000 mV  此阈值根据温度变化而调整  小于0℃可以放电至2000mV*/

#define SPEED_UP_4          (10) 
#define SPEED_UP_3          (5)  
#define SPEED_UP_2          (2)
#define SPEED_UP_1          (1.5f)
#define SPEED_UP_0          (1.1f)

#define SPEED_CUT_0         (0.9f)
#define SPEED_CUT_1         (0.7f)   
#define SPEED_CUT_2         (0.5f)
#define SPEED_CUT_3         (0.2f)
#define SPEED_CUT_4         (0.1f)

static uint32_t gFullChargeTimes = 0;
uint8_t _SoxInitFlag = 0;
uint8_t gFlagClear = 0;
static uint8_t _gTrickleChargeFlag = 0;
SoxDebugParam_t gSoxDebugParam = 
{
    .cell_current_debug = 100,
};

/*OCV,单位mV   亿纬电芯参数待更新*/
#define OCV_SOC_TEMPERATURE_LEVEL		9/*ocv-soc的温度取值点*/
#define OCV_SOC_LEVEL					21/*ocv-soc的SOC的取值点*/

const uint16_t OCVSOCDefault[OCV_SOC_TEMPERATURE_LEVEL][OCV_SOC_LEVEL] = 
{
	{3225, 3226, 3226, 3227, 3227, 3228, 3229, 3229, 3230, 3231, 3232, 3233, 3235, 3236, 3238, 3240, 3242, 3245, 3250, 3258, 3342},
    {3230, 3231, 3232, 3233, 3234, 3236, 3237, 3238, 3240, 3242, 3243, 3245, 3247, 3250, 3252, 3256, 3261, 3269, 3286, 3295, 3389},
    {3225, 3230, 3235, 3238, 3241, 3244, 3247, 3249, 3252, 3255, 3259, 3263, 3267, 3270, 3274, 3277, 3280, 3286, 3304, 3325, 3373},
    {3220, 3230, 3238, 3243, 3247, 3251, 3256, 3260, 3264, 3269, 3275, 3281, 3287, 3291, 3295, 3298, 3300, 3302, 3322, 3356, 3358},
    {3215, 3229, 3240, 3248, 3254, 3259, 3265, 3271, 3276, 3283, 3291, 3299, 3306, 3312, 3316, 3319, 3320, 3319, 3339, 3386, 3343},
    {3158, 3210, 3225, 3237, 3249, 3259, 3266, 3272, 3277, 3283, 3289, 3295, 3301, 3307, 3312, 3316, 3318, 3319, 3329, 3352, 3358},
    {3103, 3192, 3210, 3227, 3246, 3259, 3268, 3274, 3279, 3284, 3288, 3292, 3298, 3304, 3309, 3314, 3318, 3319, 3319, 3318, 3374},
    {2688, 2702, 2818, 3101, 3206, 3226, 3249, 3270, 3280, 3290, 3294, 3296, 3299, 3308, 3320, 3325, 3327, 3327, 3328, 3326, 3371},
    {2514, 2564, 2647, 3080, 3204, 3220, 3247, 3267, 3282, 3291, 3295, 3298, 3302, 3311, 3323, 3327, 3329, 3329, 3329, 3329, 3370},
};
const int8_t  OCVTemperatureIndex[OCV_SOC_TEMPERATURE_LEVEL] = {-20, -10, -5, 0, 5, 10, 25, 40, 50};/*温度的插值点*/
const uint8_t OCVSOCIndex[OCV_SOC_LEVEL] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};/*SOC的插值点*/






/*放电容量，单位mAh——————————待更新亿纬电芯数据*/
#define DSG_CAPA_TEMPERATURE_LEVEL		10 /*放电容量的温度插值点*/
#define DSG_CAPA_MULTI_LEVEL			4  /*放电容量的最大倍率插值点*/

//uint32_t DsgCapacity[DSG_CAPA_TEMPERATURE_LEVEL][DSG_CAPA_MULTI_LEVEL];
const uint32_t DsgCapacityDefault[DSG_CAPA_TEMPERATURE_LEVEL][DSG_CAPA_MULTI_LEVEL] = /*放电容量的保持率*/
{
	{56557, 64949, 73110, 74748},
	{62371, 73044, 74269, 75500},
	{67231, 71291, 69999, 76675},
	{71475, 73050, 72783, 76933},
	{75015, 74213, 76017, 77459},
	{75157, 72049, 71626, 74498},
	{79197, 77764, 76776, 77123},
	{81492, 80438, 79423, 79454},
	{81512, 81099, 80480, 79984},
    {80934, 80568, 80146, 79732},
};
const int8_t DsgCapaTemperatureIndex[DSG_CAPA_TEMPERATURE_LEVEL] = {-20, -15, -10, -5, 0, 5, 15, 25, 35, 45};/*放电容量的温度插值点*/
const float  DsgCapaMultiIndex[DSG_CAPA_MULTI_LEVEL] = {1/4, 1/2, 1, 2};/*放电容量的倍率插值点*/



/*放电电流*/
#define DSG_CURR_TEMPERATURE_LEVEL		20/*放电电流的温度数量*/
#define DSG_CURR_SOC_LEVEL			13/*放电电流的SOC*/

const int8_t DsgCurrTemperatureIndex[DSG_CURR_TEMPERATURE_LEVEL] = {-30, -25, -20, -15, -10, -5, 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65};
const uint8_t DsgCurrSocIndex[DSG_CURR_SOC_LEVEL] = {0, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 95, 100};
/*长时间60最大放电电流 >=55       60S脉冲放电数据暂未提供 待更新*/
//uint32_t DsgCurrent60[DSG_CURR_TEMPERATURE_LEVEL][DSG_CURR_SOC_LEVEL];
const uint32_t DsgCurrent60Default[DSG_CURR_TEMPERATURE_LEVEL][DSG_CURR_SOC_LEVEL] =
{
/*SOC:0 	5		10		20      30      40      50      60      70      80      90      95      100*/
	{0,     0,      0,		0, 		20000, 	50000, 	50000, 	50000, 	50000, 	100000, 100000, 100000, 100000},	//-30    
    {0,     0,      0, 		20000, 	30000, 	50000, 	50000, 	50000, 	50000, 	100000, 100000, 100000, 100000},	//-25    
    {0,     0,      20000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-20    
    {0,     20000,  30000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-15    
    {0,     30000,  50000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-10    
    {0,     30000,  50000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-5    
    {0,     50000,  100000,	100000, 100000, 100000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //0    
    {0,     50000,  100000, 100000, 100000, 100000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //5    
    {0, 	100000, 100000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //10    
    {0, 	100000, 100000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //15 
    {0, 	100000, 100000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //20 
    {0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //25 
    {0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //30
    {0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //35 	
	{0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //40 
	{0, 	50000,  150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //45 
	{0, 	50000,  150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //50 
	{0, 	50000,  100000,	100000,	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //55 
	{0, 	50000,  100000,	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //60
	{0, 	0,      0,		0, 		0, 		0, 		0, 		0, 		0, 		0, 		0, 		0, 		0	  },    //65 	
};


/*瞬态30最大放电电流 >= 25*/
/*放电电流*/
//#define DSG_CURR_TEMPERATURE_LEVEL		20/*放电电流的温度数量*/
//#define DSG_CURR_SOC_LEVEL			13/*放电电流的SOC*/
//const int8_t DsgCurrTemperatureIndex[DSG_CURR_TEMPERATURE_LEVEL] = {-30, -25, -20, -15, -10, -5, 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65};
//const uint8_t DsgCurrSocIndex[DSG_CURR_SOC_LEVEL] = {0, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 95, 100};
//uint32_t DsgCurrent30[DSG_CURR_TEMPERATURE_LEVEL][DSG_CURR_SOC_LEVEL];
const uint32_t DsgCurrent30Default[DSG_CURR_TEMPERATURE_LEVEL][DSG_CURR_SOC_LEVEL] =
{
/*SOC:0 	5		10		20      30      40      50      60      70      80      90      95      100*/
	{0,     0,      0,		0, 		20000, 	50000, 	50000, 	50000, 	50000, 	100000, 100000, 100000, 100000},	//-30    
    {0,     0,      0, 		20000, 	30000, 	50000, 	50000, 	50000, 	50000, 	100000, 100000, 100000, 100000},	//-25    
    {0,     0,      20000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-20    
    {0,     20000,  30000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-15    
    {0,     30000,  50000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-10    
    {0,     30000,  50000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-5    
    {0,     50000,  100000,	100000, 100000, 100000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //0    
    {0,     50000,  100000, 100000, 100000, 100000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //5    
    {0, 	100000, 100000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //10    
    {0, 	100000, 100000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //15 
    {0, 	100000, 100000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //20 
    {0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //25 
    {0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //30
    {0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //35 	
	{0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //40 
	{0, 	50000,  150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //45 
	{0, 	50000,  150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //50 
	{0, 	50000,  100000,	100000,	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //55 
	{0, 	50000,  100000,	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //60
	{0, 	0,      0,		0, 		0, 		0, 		0, 		0, 		0, 		0, 		0, 		0, 		0	  },    //65 	
};

/*瞬态10最大放电电流 >= 8   10S脉冲放电数据暂未提供 待更新*/
/*瞬态30最大放电电流 >= 25*/
/*放电电流*/
//#define DSG_CURR_TEMPERATURE_LEVEL		20/*放电电流的温度数量*/
//#define DSG_CURR_SOC_LEVEL			13/*放电电流的SOC*/
//const int8_t  DsgCurrTemperatureIndex[DSG_CURR_TEMPERATURE_LEVEL] = {-30, -25, -20, -15, -10, -5, 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65};
//const uint8_t DsgCurrSocIndex[DSG_CURR_SOC_LEVEL] = {0, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 95, 100};
//uint32_t DsgCurrent10[DSG_CURR_TEMPERATURE_LEVEL][DSG_CURR_SOC_LEVEL];
const uint32_t DsgCurrent10Default[DSG_CURR_TEMPERATURE_LEVEL][DSG_CURR_SOC_LEVEL] =
{
/*SOC:0 	5		10		20      30      40      50      60      70      80      90      95      100*/
	{0,     0,      0,		0, 		20000, 	50000, 	50000, 	50000, 	50000, 	100000, 100000, 100000, 100000},	//-30    
    {0,     0,      0, 		20000, 	30000, 	50000, 	50000, 	50000, 	50000, 	100000, 100000, 100000, 100000},	//-25    
    {0,     0,      20000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-20    
    {0,     20000,  30000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-15    
    {0,     30000,  50000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-10    
    {0,     30000,  50000, 	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //-5    
    {0,     50000,  100000,	100000, 100000, 100000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //0    
    {0,     50000,  100000, 100000, 100000, 100000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //5    
    {0, 	100000, 100000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //10    
    {0, 	100000, 100000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //15 
    {0, 	100000, 100000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //20 
    {0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //25 
    {0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //30
    {0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //35 	
	{0, 	100000, 150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //40 
	{0, 	50000,  150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //45 
	{0, 	50000,  150000,	150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000, 150000},    //50 
	{0, 	50000,  100000,	100000,	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //55 
	{0, 	50000,  100000,	100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000, 100000},    //60
	{0, 	0,      0,		0, 		0, 		0, 		0, 		0, 		0, 		0, 		0, 		0, 		0	  },    //65 	
};

/*充电最大倍率——根据表4.1.2充电模式  得到不同温度与Soc下的的充电倍率大小  具体数值扩大100倍*/
#define CHG_MULTI_TEMPERATURE_LEVEL	 8
#define CHG_MULTI_SOC_LEVEL		     7
const int8_t ChgMultiTemperatureIndex[CHG_MULTI_TEMPERATURE_LEVEL] = {0, 10, 20, 25, 45, 55, 60, 65};
const uint8_t ChgMultiSocIndex[CHG_MULTI_SOC_LEVEL] = {0, 60, 70, 80, 90, 95, 100};
/*100倍，实际使用计算的时候需要除以100*/
//uint8_t ChgMulti[CHG_MULTI_TEMPERATURE_LEVEL][CHG_MULTI_SOC_LEVEL];
const uint8_t ChgMultiDefault[CHG_MULTI_TEMPERATURE_LEVEL][CHG_MULTI_SOC_LEVEL] = 
{
/*SOC 0, 	60, 	70, 	80, 	90, 	95, 	100*/
	{0, 	 0, 	 0, 	0, 		0,		 0, 	 0},   	//0度
	{10, 	10,	 	10, 	10, 	10, 	10, 	 0},   	//10度
    {20, 	20, 	20, 	20, 	20, 	10,	 	10},    //20度
    {50, 	50, 	50, 	50, 	50, 	30,		10},   	//25度
    {100, 	100, 	100, 	100, 	80, 	50,		10},    //45度
    {50, 	50, 	50, 	50, 	50, 	30,		10},    //55度
    {20, 	20, 	20, 	20, 	20, 	20,		10},    //60度
    {0, 	 0,  	0, 		0, 		0, 		0,		10},   	//65度
};


/*循环寿命*/
#define CELL_LOOP_FREQ_LEVEL		41

//uint32_t CellLoopLife[CELL_LOOP_FREQ_LEVEL];
//const uint32_t CellLoopLifeDefault[CELL_LOOP_FREQ_LEVEL] = 
//{
//	80000, 79406, 78551, 78286, 77697, 77179, 76449, 76461, 76113, 76020, 75253, 
//    74677, 74641, 74188, 73919, 73672, 73545, 73198, 73057, 72812, 72436, 
//    72263, 71982, 71766, 71475, 71347, 71034, 70935, 70778, 70542, 70439, 
//    70344, 69724, 69887, 69823, 69579, 69530, 69181, 69205, 68991, 68851, 
//};
/*暂未提供   待更新*/
const uint16_t CellLoopLifeDefault[CELL_LOOP_FREQ_LEVEL] = 
{
    10000, 9926, 9819, 9786, 9712, 9647, 9556, 9558, 9514, 9503, 9407, 
    9335, 9330, 9273, 9240, 9209, 9193, 9150, 9132, 9101, 9055, 
    9033, 8998, 8971, 8934, 8918, 8879, 8867, 8847, 8818, 8805, 
    8793, 8715, 8736, 8728, 8697, 8691, 8648, 8651, 8624, 8606,
};

const uint16_t CellLoopCyclesDefault[CELL_LOOP_FREQ_LEVEL] = 
{
    20,   50,   100,  150,  200,  250,  300,  350,  400,  450, 500,
    550,  600,  650,  700,  750,  800,  850,  900,  950,  1000,
    1050, 1100, 1150, 1200, 1250, 1300, 1350, 1400, 1450, 1500,
    1550, 1600, 1650, 1700, 1750, 1800, 1850, 1900, 1950, 2000,
};

PackStatus_t	gPackStatus;
PackStatus_t    gPackStatus_debug;
static PackStatus_t    sgSoxFlashInfo;
void PackCurrentModeJudge(PackStatus_t *pPackStatus)
{    
	int32_t current = pPackStatus->pack_current;//指针的地址  电流
	if(current < PACK_CURRENT_MIN_THRESHOLD && current > (0-PACK_CURRENT_MIN_THRESHOLD))//若电流处于 500mA >电流>-500mA则此时处于空闲状态
	{
		pPackStatus->mode = EN_PACK_CURRENT_IDLE_MODE;	//判断当前是充电还是放电或是空闲状态
	}
	else if(current > 0)
	{
		if(pPackStatus->empty_charge_flag == 0)//满充标志未置位
		{
			pPackStatus->mode = EN_PACK_CURRENT_DISCHARGE_MODE;//电流大于0，则此时为放电模式。放电为正。
		}
		else
		{            
			pPackStatus->mode = EN_PACK_CURRENT_IDLE_MODE;//如果满充标志位已经置位，则此时处于满充，判断此时处于空闲状态，
		}
	}
	else
	{
//		if(pPackStatus->full_charge_flag == 0)
//		{
			pPackStatus->mode = EN_PACK_CURRENT_CHARGE_MODE;//电流为小于-500,则此时为充电模式。
//		}
//		else
//		{
//			pPackStatus->mode = EN_PACK_CURRENT_IDLE_MODE;
//		}
	}
}



/*线性插值运算, x介于x1和x2之间*/
static float _InterpolationOperation(int32_t x1, int32_t x2, int32_t y1, int32_t y2, int32_t x)
{
	float y;
	if(x1 == x2)
	{
		y = y1;
		return y;
	}
	
	y = (float)(y2 - y1) / (x2 - x1) * (x - x1) + y1;
	return y;
}
static float _InterpolationOperationf(float x1, float x2, float y1, float y2, float x)
{
	float y;
	if(x1 == x2)
	{
		y = y1;
		return y;
	}
	
	y = (float)(y2 - y1) / (x2 - x1) * (x - x1) + y1;
	return y;
}


/*计算列表区间范围索引,有符号*/
static void _GetTableRangeIndex8(int8_t value, const int8_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
	if(value <= p_index_table[0])
	{
		*p_index_d = 0;
		*p_index_u = 0;
	}
	else if(value > p_index_table[table_level - 1])
	{
		*p_index_d = table_level - 1;
		*p_index_u = table_level - 1;
	}
	else
	{
		for(uint8_t i = 1; i < table_level; i++)
		{
			if(value <= p_index_table[i])
			{
				*p_index_d = i - 1;
				*p_index_u = i;
                return;
			}
		}
	}
}
/*计算列表区间范围索引,无符号*/
static void _GetTableRangeIndex8u(uint8_t value, const uint8_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
	if(value <= p_index_table[0])
	{
		*p_index_d = 0;
		*p_index_u = 0;
	}
	else if(value > p_index_table[table_level - 1])
	{
		*p_index_d = table_level - 1;
		*p_index_u = table_level - 1;
	}
	else
	{
		for(uint8_t i = 1; i < table_level; i++)
		{
			if(value <= p_index_table[i])
			{
				*p_index_d = i - 1;
				*p_index_u = i;
                return;
			}
		}
	}
}

static void _GetTableRangeIndex16(int16_t value, const int16_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
	if(value <= p_index_table[0])
	{
		*p_index_d = 0;
		*p_index_u = 0;
	}
	else if(value > p_index_table[table_level - 1])
	{
		*p_index_d = table_level - 1;
		*p_index_u = table_level - 1;
	}
	else
	{
		for(uint8_t i = 1; i < table_level; i++)
		{
			if(value <= p_index_table[i])
			{
				*p_index_d = i - 1;
				*p_index_u = i;
                return;
			}
		}
	}
}

static void _GetTableRangeIndex16u(uint16_t value, const uint16_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    if(p_index_table[0] <= p_index_table[table_level - 1])/*升序*/
    {
        if(value <= p_index_table[0])
        {
            *p_index_d = 0;
            *p_index_u = 0;
        }
        else if(value > p_index_table[table_level - 1])
        {
            *p_index_d = table_level - 1;
            *p_index_u = table_level - 1;
        }
        else
        {
            for(uint8_t i = 1; i < table_level; i++)
            {
                if(value <= p_index_table[i])
                {
                    *p_index_d = i - 1;
                    *p_index_u = i;
                    return;
                }
            }
        }
    }
	else/*降序*/
    {
        if(value >= p_index_table[0])
        {
            *p_index_d = 0;
            *p_index_u = 0;
        }
        else if(value < p_index_table[table_level - 1])
        {
            *p_index_d = table_level - 1;
            *p_index_u = table_level - 1;
        }
        else
        {
            for(uint8_t i = 1; i < table_level; i++)
            {
                if(value >= p_index_table[i])
                {
                    *p_index_d = i - 1;
                    *p_index_u = i;
                    return;
                }
            }
        }
    }
}

static void _GetTableRangeIndex32(int32_t value, const int32_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
	if(value <= p_index_table[0])
	{
		*p_index_d = 0;
		*p_index_u = 0;
	}
	else if(value > p_index_table[table_level - 1])
	{
		*p_index_d = table_level - 1;
		*p_index_u = table_level - 1;
	}
	else
	{
		for(uint8_t i = 1; i < table_level; i++)
		{
			if(value <= p_index_table[i])
			{
				*p_index_d = i - 1;
				*p_index_u = i;
                return;
			}
		}
	}
}

static void _GetTableRangeIndex32u(uint32_t value, const uint32_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
	if(value <= p_index_table[0])
	{
		*p_index_d = 0;
		*p_index_u = 0;
	}
	else if(value > p_index_table[table_level - 1])
	{
		*p_index_d = table_level - 1;
		*p_index_u = table_level - 1;
	}
	else
	{
		for(uint8_t i = 1; i < table_level; i++)
		{
			if(value <= p_index_table[i])
			{
				*p_index_d = i - 1;
				*p_index_u = i;
                return;
			}
		}
	}
}

static void _GetTableRangeIndexf(float value, const float *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
	if(value <= p_index_table[0])
	{
		*p_index_d = 0;
		*p_index_u = 0;
	}
	else if(value > p_index_table[table_level - 1])
	{
		*p_index_d = table_level - 1;
		*p_index_u = table_level - 1;
	}
	else
	{
		for(uint8_t i = 1; i < table_level; i++)
		{
			if(value <= p_index_table[i])
			{
				*p_index_d = i - 1;
				*p_index_u = i;
                return;
			}
		}
	}
}

static void _GetTableRangeIndexf8u(float value, const uint8_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
	if(value <= p_index_table[0])
	{
		*p_index_d = 0;
		*p_index_u = 0;
	}
	else if(value > p_index_table[table_level - 1])
	{
		*p_index_d = table_level - 1;
		*p_index_u = table_level - 1;
	}
	else
	{
		for(uint8_t i = 1; i < table_level; i++)
		{
			if(value <= p_index_table[i])
			{
				*p_index_d = i - 1;
				*p_index_u = i;
                return;
			}
		}
	}
}




/*插值法*/
static float _OCV2SOCWithInterpolation(uint16_t ocv, PackStatus_t *pPackStatus)
{
	const uint16_t *p_ocv_tempd, *p_ocv_tempu;
	uint8_t i = 0, j = 0;
	uint8_t soc_index;
	uint16_t soc_ret1, soc_ret2;
	
	int16_t temp_d, temp_u;
	uint8_t temp_index_d, temp_index_u;
	uint16_t soc_tempd_d, soc_tempd_u, soc_tempu_d, soc_tempu_u;
	uint8_t soc_index_tempd_d, soc_index_tempd_u, soc_index_tempu_d, soc_index_tempu_u;
	
	
	float soc_tempd_oi, soc_tempu_oi, soc_oi;
	uint16_t soc_ret;
	int16_t temperature = pPackStatus->pack_average_temperature / 10;
	
	/*温度目前按插值的方式，后续改成区间的方式*/
	_GetTableRangeIndex8(temperature, OCVTemperatureIndex, OCV_SOC_TEMPERATURE_LEVEL, &temp_index_d, &temp_index_u);
	temp_d = OCVTemperatureIndex[temp_index_d];
	temp_u = OCVTemperatureIndex[temp_index_u];
	
	
	p_ocv_tempd = OCVSOCDefault[temp_index_d];
	p_ocv_tempu = OCVSOCDefault[temp_index_u];
	
	_GetTableRangeIndex16u(ocv, p_ocv_tempd, OCV_SOC_LEVEL, &soc_index_tempd_d, &soc_index_tempd_u);
	soc_tempd_d = OCVSOCIndex[soc_index_tempd_d];
	soc_tempd_u = OCVSOCIndex[soc_index_tempd_u];
	soc_tempd_oi = _InterpolationOperation(p_ocv_tempd[soc_index_tempd_d], p_ocv_tempd[soc_index_tempd_u], soc_tempd_d, soc_tempd_u, ocv);
	
	
	_GetTableRangeIndex16u(ocv, p_ocv_tempu, OCV_SOC_LEVEL, &soc_index_tempu_d, &soc_index_tempu_u);
	soc_tempu_d = OCVSOCIndex[soc_index_tempu_d];
	soc_tempu_u = OCVSOCIndex[soc_index_tempu_u];
	soc_tempu_oi = _InterpolationOperation(p_ocv_tempu[soc_index_tempu_d], p_ocv_tempu[soc_index_tempu_u], soc_tempu_d, soc_tempu_u, ocv);
	
	soc_oi = _InterpolationOperationf(temp_d, temp_u, soc_tempd_oi, soc_tempu_oi, temperature);
	return soc_oi;
}


/*OCV校准，在静置状态下(EN_PACK_CURRENT_IDLE_MODE)持续一定时间后调用*/
void OCVCalibraiton(PackStatus_t *pPackStatus)
{
	uint16_t min_voltage = pPackStatus->cell_min_voltage;
	
	pPackStatus->sox_info.soc_real = _OCV2SOCWithInterpolation(min_voltage, pPackStatus);//根据最低单体电压求线性插值实际SOC
    if(abs((int)(pPackStatus->sox_info.soc_actl - pPackStatus->sox_info.soc_real)) > 20)//soc差值过大
    {
        pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_real;//真实值 给计算续航soc
    }
//    pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_real;
//    pPackStatus->sox_info.soc_disp = pPackStatus->sox_info.soc_actl;

	if(pPackStatus->sox_info.soc_real > pPackStatus->sox_info.soc_max)
	{
		pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max;
		pPackStatus->full_charge_flag = 1;
	}
	if(abs((int)((pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_min) * 10)) < 5)
	{
		pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_min;
		pPackStatus->empty_charge_flag = 1;
        LOG_ERROR(LOG_TAG, "OCVCalibraiton   pPackStatus->sox_info.soc_real = 0 ");
//        SoxFlashInfoSave();
	}
}

/*满充校准，放到安时积分函数里去*/
static void _FullChargeCalibration(PackStatus_t *pPackStatus)
{		
	static uint8_t flg = 0;
	static uint32_t timecount = 0; 
	if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
	{
        /* 3.62V强制校准 */
        if(pPackStatus->cell_max_voltage > CHARGE_MAX_VOLTAGE + 50 + 20)//强制校准的阈值设置  待确认
        {
            pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max;
            pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_max;
            pPackStatus->sox_info.soc_disp = 100.1;
        }
//		if(pPackStatus->cell_max_voltage >= pPackStatus->full_charge_voltage)
        else if((pPackStatus->cell_max_voltage >= CHARGE_MAX_VOLTAGE && (abs(pPackStatus->pack_current) < 10000)) ||\
            pPackStatus->cell_max_voltage >= (CHARGE_MAX_VOLTAGE + 30))//充电最大阈值设置  待确认
		{
			if(flg == 0)
			{
				if(pPackStatus->sox_info.soc_real < pPackStatus->sox_info.soc_max - 1)
				{
					pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max - 1;
				}
                timecount = tim_get_systime();
			}
			flg = 1;
		}
        else 
		{
            /* 防止满充标志清不掉 */
			flg = 0;
		}
        
		if(flg == 1)
		{
			if((uint32_t)(tim_get_systime() - timecount) > 0)
			{
                pPackStatus->full_charge_flag = 1;                
                if(pPackStatus->sox_info.soc_disp < 99)
                {
                    pPackStatus->sox_info.soc_disp += 0.015f;
                }
                else
                {
                    pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max;
                    pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_max;
                    pPackStatus->sox_info.soc_disp = 100.1;
                }
			}
		}
	}
}

/*空放判断，待优化   应加入温度校准*/
static void _EmptyDischargeJudge(PackStatus_t *pPackStatus)
{
    static uint8_t empty_charge_count = 0;
    static uint32_t empty_charge_times = 0;
//如果模式是放电或者是处于待机状态
	if(pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE || pPackStatus->mode == EN_PACK_CURRENT_IDLE_MODE)
	{
//如果电池最小电压<放电最小电压  或者实际SOC与SOC最小值的差异小于0.5
        if(pPackStatus->cell_min_voltage <= DISCHARGE_MIN_VOLTAGE || (abs((int)((pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_min) * 10)) < 5))
        {
            pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_min;
            pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_min;
            pPackStatus->sox_info.soc_disp = 0;
            pPackStatus->empty_charge_flag = 1;
            LOG_ERROR(LOG_TAG, "_EmptyDischargeJudge   pPackStatus->sox_info.soc_real = 0 ");
//          SoxFlashInfoSave();
        }
        else
        {
            if(pPackStatus->sox_info.soc_disp <= 0)
            {
                pPackStatus->empty_charge_flag = 1;
            }
        }
        
        if(pPackStatus->sox_info.soc_disp < 100)
        {
            pPackStatus->full_charge_flag = 0;
        }
        
//		if(tim_difftime_from_now(empty_charge_times) > 100)
//		{
//            empty_charge_times = tim_get_systime();
//            if(pPackStatus->cell_min_voltage <= pPackStatus->empty_charge_voltage)
//            {
//                empty_charge_count ++;		
//            }
//            else
//            {
//                empty_charge_count = 0;
//            }            	
//		}
//                
//        if(empty_charge_count > 5)
//        {
//            pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_min;
//			pPackStatus->empty_charge_flag = 1;
//        }
//        
//		if(pPackStatus->sox_info.soc_real <= pPackStatus->sox_info.soc_min)
//		{
//			pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_min;
//			pPackStatus->empty_charge_flag = 1;
//		}
	}
}


/*滑动滤波*/

static uint32_t _SlidingFilter(uint32_t value, EN_PACK_CURRENT_MODE mode, uint16_t level)//滑动滤波
{
    static EN_PACK_CURRENT_MODE pre_mode = EN_PACK_CURRENT_IDLE_MODE;//预模式为空闲模式
    static uint16_t count = 0;
    static uint16_t pre_level = 0;
//  static uint32_t filter_buf[64];
    static uint32_t filter_value = 0;
    static uint32_t ret;
    static uint32_t avg_value = 0;
    
    if(mode != pre_mode)//如果模式为充电模式或者放电模式
    {
        count = 0;
        pre_mode = mode;//将充电模式或者放电模式给到 pre_mode
        filter_value = 0;//将滤波值赋值为0
    }
    if(level != pre_level)//数量不等于预定义数量
    {
        count = 0;//
        pre_level = level;//将数量赋值给预定义数量
        filter_value = 0;//滤波值给定为0
    }
    
    if(count < level - 1)//总数小于数量-1
    {
        filter_value += value;
        ret = filter_value / (count + 1);
//      filter_value = ret;
        avg_value = ret;
    }
    else                //总数小大于等于（数量-1）
    {
        ret = (avg_value * (level - 1) + value + (level - 1)) / level;
        avg_value = ret;
    }
    
		count++;
		return ret;
}

/*计算当前实际容量，在初始化的时候需要调用一次，确保安时积分算法执行前已获取cr*/
void CurrentCapaCalc(PackStatus_t *pPackStatus)
{
	int16_t temperature = pPackStatus->pack_average_temperature / 10;//温度单位为0.1摄氏度
	int32_t current = pPackStatus->pack_current;//电流单位:mA
    static int32_t recharge_current = 0;//单位:mA
    static uint32_t charge_capa = 0;//充电容量 单位:mAh
    static uint32_t discharge_capa = 0;//放电容量 单位:mAh
	float dsg_multiplier;//放电倍率
    static uint32_t recharge_times = 0;//重新充电时间
    static uint8_t recharge_times_count = 0;
    uint32_t fullcharge_times = 0;//满充时间
    uint32_t recharge_times_temp = 0;//
    
	
	int16_t temp_d, temp_u;
	uint8_t temp_index_d, temp_index_u;
	const uint32_t *p_capa_tempd, *p_capa_tempu;
	float dsg_multiplier_d, dsg_multiplier_u;
	uint8_t dsg_multiplier_index_d, dsg_multiplier_index_u;
	
	uint32_t capa_tempd_d, capa_tempd_u, capa_tempu_d, capa_tempu_u;
	uint32_t capa_tempd_oi, capa_tempu_oi, capa_oi;
	
	if(current < 0)
	{
		current = -current;
	}
    if((abs(current - recharge_current) > 200))
    {
        recharge_current = current;
    }

    
	dsg_multiplier = (float)current / PACK_RATED_CAPACITY;
	
	_GetTableRangeIndex8(temperature, DsgCapaTemperatureIndex, DSG_CAPA_TEMPERATURE_LEVEL, &temp_index_d, &temp_index_u);
	temp_d = DsgCapaTemperatureIndex[temp_index_d];
	temp_u = DsgCapaTemperatureIndex[temp_index_u];
	
	if(temperature > temp_u)
	{
		temperature = temp_u;
	}
	else if(temperature < temp_d)
	{
		temperature = temp_d;
	}
	
	p_capa_tempd = DsgCapacityDefault[temp_index_d];
	p_capa_tempu = DsgCapacityDefault[temp_index_u];
	
	_GetTableRangeIndexf(dsg_multiplier, DsgCapaMultiIndex, DSG_CAPA_MULTI_LEVEL, &dsg_multiplier_index_d, &dsg_multiplier_index_u);
	capa_tempd_d = p_capa_tempd[dsg_multiplier_index_d];
	capa_tempd_u = p_capa_tempd[dsg_multiplier_index_u];
	capa_tempu_d = p_capa_tempu[dsg_multiplier_index_d];
	capa_tempu_u = p_capa_tempu[dsg_multiplier_index_u];
	
	
	dsg_multiplier_d = DsgCapaMultiIndex[dsg_multiplier_index_d];
	dsg_multiplier_u = DsgCapaMultiIndex[dsg_multiplier_index_u];
	
	if(dsg_multiplier > dsg_multiplier_u)
	{
		dsg_multiplier = dsg_multiplier_u;
	}
	else if(dsg_multiplier < dsg_multiplier_d)
	{
		dsg_multiplier = dsg_multiplier_d;
	}
	
	capa_tempd_oi = _InterpolationOperationf(dsg_multiplier_d, dsg_multiplier_u, capa_tempd_d, capa_tempd_u, dsg_multiplier);
	capa_tempu_oi = _InterpolationOperationf(dsg_multiplier_d, dsg_multiplier_u, capa_tempu_d, capa_tempu_u, dsg_multiplier);
	
	capa_oi = _InterpolationOperationf(temp_d, temp_u, capa_tempd_oi, capa_tempu_oi, temperature);
	
	pPackStatus->cr = capa_oi;/*当前最大可用容量通过插值获取*/
	pPackStatus->cr = pPackStatus->cr * pPackStatus->sox_info.soh / 100;
    
    pPackStatus->current_power = (abs(pPackStatus->pack_current) / 1000.0) * (gCell_Info.total_cell_voltage / 1000.0) * pPackStatus->current_power_percentage;   /* 当前功率 */
	
    if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
    {
        /* 预计充满时间 */
//        recharge_times += pPackStatus->cr * (pPackStatus->sox_info.soc_max - pPackStatus->sox_info.soc_real) / 100.0 / \
//                                     recharge_current * 3600;
//        
//        charge_capa = pPackStatus->chg_accumulate_capa_ah;
//        recharge_times_count++;
//        
//        if(recharge_times_count == 15)
//        {
//            recharge_times_count = 0;
//            if(recharge_times / 15 < pPackStatus->recharge_times)
//            {
//                pPackStatus->recharge_times = recharge_times / 15;
//            }            
//            recharge_times = 0;
//        }
        
        if(pPackStatus->sox_info.soc_actl < 100 && pPackStatus->sox_info.soc_real < 99)
        {
            if(recharge_current == 0)
            {
                recharge_current = 1;
            }
            recharge_times_temp = pPackStatus->cr * (pPackStatus->sox_info.soc_max - pPackStatus->sox_info.soc_actl) * 36 / recharge_current;
            
            /* 确保进入涓充时，剩余时间不会随电流减小而增加 */
            if(recharge_times_temp > recharge_times)
            {
                if(_gTrickleChargeFlag || pPackStatus->full_charge_flag)
                {
                    
                }
                else
                {
                    recharge_times = recharge_times_temp;
                }
            }
            pPackStatus->recharge_times = _SlidingFilter(recharge_times, pPackStatus->mode, 5);                 
        }
        else    //不会进入
        {
            if(pPackStatus->sox_info.soc_real > 99 && pPackStatus->sox_info.soc_real < 100)
            {
                fullcharge_times = (5 * 60 * 1000 - tim_difftime_from_now(gFullChargeTimes)) / 1000;
                 if(fullcharge_times > 1 && pPackStatus->cur_max_chg_current != 0)
                 {
                    if(pPackStatus->recharge_times > 5 * 60)
                    {
                        pPackStatus->recharge_times = 5 * 60 - 1;
                    }
                    else if(fullcharge_times <= pPackStatus->recharge_times)
                    {
                        pPackStatus->recharge_times = fullcharge_times;       
                    }
                    else
                    {
                            
                    }
                 }
                 else
                 {
                    pPackStatus->recharge_times = 0;
                 }
            }
            else if(pPackStatus->sox_info.soc_real > 100)
            {
                pPackStatus->recharge_times = 0;
            }
            else
            {
                if(recharge_current == 0)
                {
                    recharge_current = 1;
                }
                recharge_times = pPackStatus->cr * (pPackStatus->sox_info.soc_max - pPackStatus->sox_info.soc_real) * 36 / recharge_current;
                if(recharge_times <= pPackStatus->recharge_times)
                {
                    pPackStatus->recharge_times = recharge_times;
                }
            }            
        }
        
        
    }
    else
    {
        /* 剩余可用时间 */
//        recharge_times += pPackStatus->recharge_times = pPackStatus->cr * (pPackStatus->sox_info.soc_max - pPackStatus->sox_info.soc_real) / 100/ \
//        recharge_current * 3600;
//        
//        discharge_capa = pPackStatus->dsg_accumulate_capa_ah;
//        recharge_times_count++;
//        
//        if(recharge_times_count == 15)
//        {
//            recharge_times_count = 0;
//            if(recharge_times / 15 < pPackStatus->recharge_times)
//            {
//                pPackStatus->recharge_times = recharge_times / 15;
//            }            
//            recharge_times = 0;
//        }
        
        recharge_times = pPackStatus->cr * (pPackStatus->sox_info.soc_actl - pPackStatus->sox_info.soc_min) * 36 / recharge_current;
        pPackStatus->recharge_times = _SlidingFilter(recharge_times, pPackStatus->mode, 5);   
    }
    
}


/*计算当前最大放电电流, mode = 0:10S, mode = 1:30S, mode = 2:60S*/
void MaxDsgCurrentCalc(PackStatus_t *pPackStatus)
{
	float soc = pPackStatus->sox_info.soc_real;
	int16_t temperature = pPackStatus->pack_average_temperature / 10;
	
	int16_t temp_d, temp_u;
	uint8_t temp_index_d, temp_index_u;
	const uint32_t *p_curr_tempd, *p_curr_tempu;
	float soc_d, soc_u;
	uint8_t soc_index_d, soc_index_u;
	
	uint32_t curr_tempd_d, curr_tempd_u, curr_tempu_d, curr_tempu_u;
	uint32_t curr_tempd_oi, curr_tempu_oi, curr_oi;
	
	_GetTableRangeIndex8(temperature, DsgCurrTemperatureIndex, DSG_CURR_TEMPERATURE_LEVEL, &temp_index_d, &temp_index_u);
	temp_d = OCVTemperatureIndex[temp_index_d];
	temp_u = OCVTemperatureIndex[temp_index_u];
	
	if(temperature > temp_u)
	{
		temperature = temp_u;
	}
	else if(temperature < temp_d)
	{
		temperature = temp_d;
	}
	
	_GetTableRangeIndex8u(soc, DsgCurrSocIndex, DSG_CURR_SOC_LEVEL, &soc_index_d, &soc_index_u);
	
	soc_d = DsgCurrSocIndex[soc_index_d];
	soc_u = DsgCurrSocIndex[soc_index_u];
	
	/*10S瞬态*/
	p_curr_tempd = DsgCurrent10Default[temp_index_d];
	p_curr_tempu = DsgCurrent10Default[temp_index_u];
	
	curr_tempd_d = p_curr_tempd[soc_index_d];
	curr_tempd_u = p_curr_tempd[soc_index_u];
	curr_tempu_d = p_curr_tempu[soc_index_d];
	curr_tempu_u = p_curr_tempu[soc_index_u];
	
	curr_tempd_oi = _InterpolationOperation(soc_d, soc_u, curr_tempd_d, curr_tempd_u, soc);
	curr_tempu_oi = _InterpolationOperation(soc_d, soc_u, curr_tempu_d, curr_tempu_u, soc);
	
	curr_oi = _InterpolationOperation(temp_d, temp_u, curr_tempd_oi, curr_tempu_oi, temperature);
	
	pPackStatus->cur_max_dsg_current_10s = curr_oi;
	
	/*30S瞬态*/
	p_curr_tempd = DsgCurrent30Default[temp_index_d];
	p_curr_tempu = DsgCurrent30Default[temp_index_u];
	
	curr_tempd_d = p_curr_tempd[soc_index_d];
	curr_tempd_u = p_curr_tempd[soc_index_u];
	curr_tempu_d = p_curr_tempu[soc_index_d];
	curr_tempu_u = p_curr_tempu[soc_index_u];
	
	curr_tempd_oi = _InterpolationOperation(soc_d, soc_u, curr_tempd_d, curr_tempd_u, soc);
	curr_tempu_oi = _InterpolationOperation(soc_d, soc_u, curr_tempu_d, curr_tempu_u, soc);
	
	curr_oi = _InterpolationOperation(temp_d, temp_u, curr_tempd_oi, curr_tempu_oi, temperature);
	
	pPackStatus->cur_max_dsg_current_30s = curr_oi;
	
	/*60S稳态*/
	p_curr_tempd = DsgCurrent60Default[temp_index_d];
	p_curr_tempu = DsgCurrent60Default[temp_index_u];
	
	curr_tempd_d = p_curr_tempd[soc_index_d];
	curr_tempd_u = p_curr_tempd[soc_index_u];
	curr_tempu_d = p_curr_tempu[soc_index_d];
	curr_tempu_u = p_curr_tempu[soc_index_u];
	
	curr_tempd_oi = _InterpolationOperation(soc_d, soc_u, curr_tempd_d, curr_tempd_u, soc);
	curr_tempu_oi = _InterpolationOperation(soc_d, soc_u, curr_tempu_d, curr_tempu_u, soc);
	
	curr_oi = _InterpolationOperation(temp_d, temp_u, curr_tempd_oi, curr_tempu_oi, temperature);
	
	pPackStatus->cur_max_dsg_current_60s = curr_oi;
    
    if(pPackStatus->empty_charge_flag)
    {
        pPackStatus->cur_max_dsg_current_60s = 0;
        pPackStatus->cur_max_dsg_current_30s = 0;
        pPackStatus->cur_max_dsg_current_10s = 0;
    }
}


/*计算当前最大充电倍率换算成电流*/
void MaxChgMultiCalc(PackStatus_t *pPackStatus)
{
    static uint32_t trickle_charge_count = 0;
	float soc = pPackStatus->sox_info.soc_real;
	int16_t temperature = pPackStatus->pack_average_temperature / 10;
    static uint8_t trickle_charge_flag = 0;    
	
	int16_t temp_d, temp_u;
	uint8_t temp_index_d, temp_index_u;
	const uint8_t *p_multi_tempd, *p_multi_tempu;
	float soc_d, soc_u;
	uint8_t soc_index_d, soc_index_u;
	
	uint8_t multi_tempd_d, multi_tempd_u, multi_tempu_d, multi_tempu_u;
	uint8_t multi_tempd_oi, multi_tempu_oi, multi_oi;
	
	_GetTableRangeIndex8(temperature, ChgMultiTemperatureIndex, CHG_MULTI_TEMPERATURE_LEVEL, &temp_index_d, &temp_index_u);
	temp_d = ChgMultiTemperatureIndex[temp_index_d];
	temp_u = ChgMultiTemperatureIndex[temp_index_u];
	
	if(temperature > temp_u)
	{
		temperature = temp_u;
	}
	else if(temperature < temp_d)
	{
		temperature = temp_d;
	}
	
	p_multi_tempd = ChgMultiDefault[temp_index_d];
	p_multi_tempu = ChgMultiDefault[temp_index_u];
	
	_GetTableRangeIndex8u(soc, ChgMultiSocIndex, CHG_MULTI_SOC_LEVEL, &soc_index_d, &soc_index_u);
	multi_tempd_d = p_multi_tempd[soc_index_d];
	multi_tempd_u = p_multi_tempd[soc_index_u];
	multi_tempu_d = p_multi_tempu[soc_index_d];
	multi_tempu_u = p_multi_tempu[soc_index_u];
	
	soc_d = ChgMultiSocIndex[soc_index_d];
	soc_u = ChgMultiSocIndex[soc_index_u];
	
	multi_tempd_oi = _InterpolationOperation(soc_d, soc_u, multi_tempd_d, multi_tempd_u, soc);
	multi_tempu_oi = _InterpolationOperation(soc_d, soc_u, multi_tempu_d, multi_tempu_u, soc);
	
	multi_oi = _InterpolationOperation(temp_d, temp_u, multi_tempd_oi, multi_tempu_oi, temperature);
	
//    if(pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE)
//    {
//        pPackStatus->full_charge_flag = 0;  
//        trickle_charge_flag = 0;
//    }
//    
//    if(pPackStatus->sox_info.soc_disp < 100)
//    {
//        if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE && pPackStatus->full_charge_flag == 0 && pPackStatus->sox_info.soc_disp >= 99)
//        {
//            pPackStatus->sox_info.soc_disp = 99;
//        }    
//    }    
//    
//    /* real_soc 到99% 10分钟后强制停充 */
//    if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE && pPackStatus->sox_info.soc_real > 99)
//    {
//        trickle_charge_flag = 0;
//        if(pPackStatus->full_charge_flag == 0)
//        {
//            gFullChargeTimes = tim_get_systime();
//            pPackStatus->full_charge_flag = 1;
//        }
//        
//        if(tim_difftime_from_now(gFullChargeTimes) > 5 * 60 * 1000)
//        {
//            pPackStatus->sox_info.soc_real = 100.1;
//            pPackStatus->sox_info.soc_actl = 100.1;
//            pPackStatus->sox_info.soc_disp = 100.1;
//            pPackStatus->cur_max_chg_current = 0;
//            gFullChargeTimes = tim_get_systime();
//        }
//    }
//    
//    /* 达到满充电压 且电芯压差大于100mv触发满充校准 */
//    if(pPackStatus->cell_max_voltage >= CHARGE_MAX_VOLTAGE && gCell_Info.cell_diff_voltage > 100)
//    {        
//        pPackStatus->full_charge_flag = 1;
//        trickle_charge_flag = 0;
//        if(/*pPackStatus->sox_info.soc_real > 99 && */ pPackStatus->sox_info.soc_disp >= 100)
//        {
//            pPackStatus->sox_info.soc_real = 100.1;
//            pPackStatus->sox_info.soc_actl = 100.1;
//            pPackStatus->sox_info.soc_disp = 100.1;
//            pPackStatus->cur_max_chg_current = 0;
//        } 
//        /* 触发满充校准但显示soc小于100，迅速将disp_soc加到100 */
//        else if(pPackStatus->sox_info.soc_disp < 100)
//        {
//            pPackStatus->cur_max_chg_current = 1000;
//            pPackStatus->sox_info.soc_disp += 0.1;
//        }        
//    }
//    /* 第一次到达涓流充电电压 */
//    else if((pPackStatus->cell_max_voltage >= (CHARGE_MAX_VOLTAGE - 50) && pPackStatus->full_charge_flag == 0) || trickle_charge_flag == 1)
//    {
//        trickle_charge_flag = 1;
//        pPackStatus->cur_max_chg_current = 8000;
//    }
//    /* 未达到涓流按照充电map表 */
//    else if((pPackStatus->cell_max_voltage < (CHARGE_MAX_VOLTAGE - 50)) && pPackStatus->sox_info.soc_disp < 100 && pPackStatus->full_charge_flag == 0)
//    {
//        pPackStatus->cur_max_chg_current = multi_oi * PACK_RATED_CAPACITY / 100;/*充电倍率表按照100倍，所以这里要除以100*/    
//    }
    if(pPackStatus->full_charge_flag)
    {
        if(pPackStatus->sox_info.soc_disp < 100)
        {
            pPackStatus->sox_info.soc_real = 99.4;
            pPackStatus->sox_info.soc_actl = 99.4;
            pPackStatus->cur_max_chg_current = 8000;
            
            if(tim_difftime_from_now(trickle_charge_count) > 10000)
            {     
                trickle_charge_count = tim_get_systime();
                if(pPackStatus->sox_info.soc_disp < 99)
                {
                    pPackStatus->sox_info.soc_disp += 0.1f;
                }
            }   
        }
        else
        {
            pPackStatus->cur_max_chg_current = 0;
        }        
    }
    else if(pPackStatus->cell_max_voltage >= CHARGE_MAX_VOLTAGE)
    {
        pPackStatus->cur_max_chg_current = 20000;
        _gTrickleChargeFlag = 1;
    }
    else if(!_gTrickleChargeFlag)
    {
        pPackStatus->cur_max_chg_current = multi_oi * PACK_RATED_CAPACITY / 100;/*充电倍率表按照100倍，所以这里要除以100*/    
    }

}

static uint8_t _R2CorrectCoefJudge(PackStatus_t *pPackStatus)
{
	uint8_t ret = 0;
	static uint32_t timecount = 0; 
	static uint8_t flg = 0;
	
	if(pPackStatus->cell_max_voltage >= CHARGE_MAX_VOLTAGE - 30)
	{
		if(flg == 0)
		{
			timecount = tim_get_systime();
			flg = 1;
		}
	}
	else
	{
		flg = 0;
	}
	
	if(flg == 1)
	{
		if((uint32_t)(tim_get_systime() - timecount) > 10000)
		{
			if(pPackStatus->pack_current < -100 && pPackStatus->sox_info.soc_real < pPackStatus->sox_info.soc_max - 4)
			{
				ret = 1;
			}
		}		
	}
	return ret;
}


uint16_t    soh_rpt_test = 1000;
/*安时积分法，放在定时器中断里，每隔900ms执行一次*/
void SOCAmpereHourIntegral(uint32_t ms, PackStatus_t *pPackStatus)
{
	uint32_t cr;/*当前最大容量，单位:mAh*/
	float r1_act = 1, r1_rpt = 1;
	float r2_act = 1, r2_rpt = 1;
	float r3_rpt = 1;
    	
	int32_t current = pPackStatus->pack_current;
	float soc_max = pPackStatus->sox_info.soc_max;
	float soc_min = pPackStatus->sox_info.soc_min;
	
	float soc_rpt_target;/*soc_disp的修正目标，基于soc_actl结合放电深度*/
	float calcu_temp;
    uint16_t capa_ratio = 0;
    uint8_t pack_cycles_index_tempd_d, pack_cycles_index_tempd_u = 0;
    
    if(_SoxInitFlag == 0)
    {
        return;
    }
	
	/*暂时放在这里，当真实电池容量小于阈值-1，则清除满充标志*/
	if(pPackStatus->sox_info.soc_real <= pPackStatus->sox_info.soc_max - 1)
	{
		pPackStatus->full_charge_flag = 0;
	}
	if(pPackStatus->sox_info.soc_real >= pPackStatus->sox_info.soc_min + 1)
	{
		pPackStatus->empty_charge_flag = 0;
	}
	
//	if(pPackStatus->mode == EN_PACK_CURRENT_IDLE_MODE)
//	{
//		return;
//	}
	
	/*计算当前容量*/
	cr = pPackStatus->cr; 
	
	
	/*计算R修正系数*/
	if(pPackStatus->sox_info.soc_actl < soc_min)
	{
		soc_rpt_target = 0;
	}
	else if(pPackStatus->sox_info.soc_actl > soc_max)
	{
		soc_rpt_target = 100;
	}
	else
	{
		soc_rpt_target = (pPackStatus->sox_info.soc_actl - soc_min) * 100 / (soc_max - soc_min);
	}
	
	switch(pPackStatus->mode)
	{
        case EN_PACK_CURRENT_IDLE_MODE:
		case EN_PACK_CURRENT_DISCHARGE_MODE:
		{
			uint8_t r3_flg;
			/*actl修正系数R1*/
            if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl >= 20)
			{
				r1_act = SPEED_CUT_4 * 0.5f;
			}
            if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl >= 10)
			{
				r1_act = SPEED_CUT_3 * 0.5f;
			}
            else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl >= 5)
			{
				r1_act = SPEED_CUT_2 * 0.5f;
			}
			else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl >= 3)
			{
				r1_act = SPEED_CUT_1 * 0.5f;
			}
			else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl >= 1 && pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl < 3)
			{
				r1_act = SPEED_CUT_0 * 0.5f;
			}
			else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl > -3 && pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl <= -1)
			{
				r1_act = SPEED_UP_0 * 2;
			}
            else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl <= -20)
			{
				r1_act = SPEED_UP_4 * 2;
			}
			else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl <= -10)
			{
				r1_act = SPEED_UP_3 * 2;
			}
            else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl <= -5)
			{
				r1_act = SPEED_UP_2 * 2;
			}
            else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl <= -3)
			{
				r1_act = SPEED_UP_1 * 2;
			}
			else 
			{
				r1_act = 1.0f;
			}
			/*actl修正系数R2*/
			r2_act = 1.0;
			
			/*disp修正系数R1*/
            if(soc_rpt_target - pPackStatus->sox_info.soc_disp >= 20)
			{
				r1_rpt = SPEED_CUT_4;
			}
            else if(soc_rpt_target - pPackStatus->sox_info.soc_disp >= 10)
			{
				r1_rpt = SPEED_CUT_3;
			}
            else if(soc_rpt_target - pPackStatus->sox_info.soc_disp >= 5)
			{
				r1_rpt = SPEED_CUT_2;
			}
			else if(soc_rpt_target - pPackStatus->sox_info.soc_disp >= 3)
			{
				r1_rpt = SPEED_CUT_1;
			}
			else if(soc_rpt_target - pPackStatus->sox_info.soc_disp >= 1 && soc_rpt_target - pPackStatus->sox_info.soc_disp < 3)
			{
				r1_rpt = SPEED_CUT_0;
			}
			else if(soc_rpt_target - pPackStatus->sox_info.soc_disp > -3 && soc_rpt_target - pPackStatus->sox_info.soc_disp <= -1)
			{
				r1_rpt = SPEED_UP_0;
			}
            else if(soc_rpt_target - pPackStatus->sox_info.soc_disp <= -20)
			{
				r1_rpt = SPEED_UP_4;
			}
			else if(soc_rpt_target - pPackStatus->sox_info.soc_disp <= -10)
			{
				r1_rpt = SPEED_UP_3;
			}
            else if(soc_rpt_target - pPackStatus->sox_info.soc_disp <= -5)
			{
				r1_rpt = SPEED_UP_2;
			}
            else if(soc_rpt_target - pPackStatus->sox_info.soc_disp <= -3)
			{
				r1_rpt = SPEED_UP_1;
			}
			else 
			{
				r1_rpt = 1.0f;
			}
			/*disp修正系数R2*/
			r2_rpt = 1.0;
			/*disp修正系数R3*/
			if(pPackStatus->sox_info.soc_real < 30 && soc_rpt_target - pPackStatus->sox_info.soc_disp > 0.5f && pPackStatus->pack_current >= 0)
			{
				r3_rpt = 1.8f;
			}
			else
			{
				r3_rpt = 1.0f;
			}
			break;
		}
		case EN_PACK_CURRENT_CHARGE_MODE:
		{
			uint8_t r2_flg;
			r2_flg = _R2CorrectCoefJudge(pPackStatus);
			/*actl修正系数R1*/
            if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl >= 20)
			{
				r1_act = SPEED_UP_4 * 2;
			}
            if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl >= 10)
			{
				r1_act = SPEED_UP_3 * 2;
			}
            else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl >= 5)
			{
				r1_act = SPEED_UP_2 * 2;
			}
			else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl >= 3)
			{
				r1_act = SPEED_UP_1 * 2;
			}
			else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl >= 1 && pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl < 3)
			{
				r1_act = SPEED_UP_0 * 2;
			}
			else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl > -3 && pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl <= -1)
			{
				r1_act = SPEED_CUT_0 * 0.5f;
			}
            else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl <= -20)
			{
				r1_act = SPEED_CUT_4 * 0.5f;
			}
			else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl <= -10)
			{
				r1_act = SPEED_CUT_3 * 0.5f;
			}
            else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl <= -5)
			{
				r1_act = SPEED_CUT_2 * 0.5f;
			}
            else if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl <= -3)
			{
				r1_act = SPEED_CUT_1 * 0.5f;
			}
			else 
			{
				r1_act = 1.0f;
			}
			/*actl修正系数R2*/
			if(r2_flg == 1)
			{
				if(pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl > 14)
				{
					r2_act = 5.0f;
				}
				else
				{
					r2_act = (pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_actl) / 3 + 1.5f;
					if(r2_act < 0)
					{
						r2_act = 1.0f;
					}
					if(r2_act > 3)
					{
						r2_act = 5.0f;
					}
				}
			}
			else
			{
				r2_act = 1.0;
			}
			
			/*disp修正系数R1*/
            if(soc_rpt_target - pPackStatus->sox_info.soc_disp >= 20)
			{
				r1_rpt = SPEED_UP_4;
			}
            if(soc_rpt_target - pPackStatus->sox_info.soc_disp >= 10)
			{
				r1_rpt = SPEED_UP_3;
			}
            else if(soc_rpt_target - pPackStatus->sox_info.soc_disp >= 5)
			{
				r1_rpt = SPEED_UP_2;
			}
			else if(soc_rpt_target - pPackStatus->sox_info.soc_disp >= 3)
			{
				r1_rpt = SPEED_UP_1;
			}
			else if(soc_rpt_target - pPackStatus->sox_info.soc_disp >= 1 && soc_rpt_target - pPackStatus->sox_info.soc_disp < 3)
			{
				r1_rpt = SPEED_UP_0;
			}
			else if(soc_rpt_target - pPackStatus->sox_info.soc_disp > -3 && soc_rpt_target - pPackStatus->sox_info.soc_disp <= -1)
			{
				r1_rpt = SPEED_CUT_0;
			}
            else if(soc_rpt_target - pPackStatus->sox_info.soc_disp <= -20)
			{
				r1_rpt = SPEED_CUT_4;
			}
            else if(soc_rpt_target - pPackStatus->sox_info.soc_disp <= -10)
			{
				r1_rpt = SPEED_CUT_3;
			}
            else if(soc_rpt_target - pPackStatus->sox_info.soc_disp <= -5)
			{
				r1_rpt = SPEED_CUT_2;
			}
			else if(soc_rpt_target - pPackStatus->sox_info.soc_disp <= -3)
			{
				r1_rpt = SPEED_CUT_1;
			}                       
			else 
			{
				r1_rpt = 1.0;
			}
			/*disp修正系数R2*/
			if(r2_flg == 1)
			{
				if(100 - pPackStatus->sox_info.soc_disp > 14)
				{
					r2_rpt = 5.0f;
				}
				else
				{
					r2_rpt = (100 - pPackStatus->sox_info.soc_disp) / 6 + 1;
					if(r2_rpt < 0)
					{
						r2_rpt = 1.0f;
					}
					if(r2_rpt > 3)
					{
						r2_rpt = 5.0f;
					}
				}
			}
			else
			{
				r2_rpt = 1.0;
			}
            /*disp修正系数R3*/
            r3_rpt = 1.0;
			break;
		}
		default:break;
	}
	if(pPackStatus->sox_info.soc_real < 20 && pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE)
    {
        current = current * 1.2f;
    }
	// calcu_temp = current * ms / 1000 / 3600;
	/*电流，充电为负，放电为正*/
	pPackStatus->sox_info.soc_real -= (float)current * ms / 1000 / 3600 * 100 / cr;//mA, ms转h, 除以(1000 * 3600) * 100(%)
	pPackStatus->sox_info.soc_actl -= r1_act * r2_act * (float)current * ms / 1000 / 3600  / cr * 100;//mA, ms转h, 除以(1000 * 3600)
	
    /* 满充标志存在，待机耗电情况下不计算表显 */
    if(pPackStatus->full_charge_flag && current > 0)
    {
    
    
    }
    else
    {
        pPackStatus->sox_info.soc_disp -= r1_rpt * r2_rpt * r3_rpt * (float)current * ms / 1000 / 3600 * 100 / cr;//mA, ms转h, 除以(1000 * 3600)
	}
    
	if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
	{
		if(pPackStatus->sox_info.soc_real >= pPackStatus->sox_info.soc_max - 0.6f && pPackStatus->full_charge_flag == 0)
		{
			pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max - 0.6f;/*待满充校准触发后，再跳至100.1%*/
		}
        if(pPackStatus->sox_info.soc_actl >= pPackStatus->sox_info.soc_max - 0.6f && pPackStatus->full_charge_flag == 0)
        {
            pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_max - 0.6f;
        }
        if(pPackStatus->sox_info.soc_disp >= pPackStatus->sox_info.soc_max - 0.6f && pPackStatus->full_charge_flag == 0)
        {
            pPackStatus->sox_info.soc_disp = pPackStatus->sox_info.soc_max - 0.6f;
        }
		/*满充校准*/
		_FullChargeCalibration(pPackStatus);
		
		/*计算SOH所需的累积容量*/
		pPackStatus->chg_accumulate_capa_ah += abs(current) * ms / 1000 / 3600 / 1000.0;
        pPackStatus->chg_accumulate_capa_wh += (abs(current) * ms / 1000 / 3600) * (pPackStatus->cell_total_voltage / 1000.0) / 1000.0;
	}
	else if(pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE || pPackStatus->mode == EN_PACK_CURRENT_IDLE_MODE)
	{
        /* 清除涓充标志 */
        _gTrickleChargeFlag = 0;
		/*空放判断*/
		_EmptyDischargeJudge(pPackStatus);
		
		/*计算SOH所需的累积容量*/
		pPackStatus->dsg_accumulate_capa_ah += abs(current) * ms / 1000 / 3600 / 1000.0;
        pPackStatus->dsg_accumulate_capa_wh += (abs(current) * ms / 1000 / 3600) * (pPackStatus->cell_total_voltage / 1000.0) / 1000.0;
	}
    else if(pPackStatus->mode == EN_PACK_CURRENT_IDLE_MODE)
    {
        if(pPackStatus->sox_info.soc_disp >= 100)
        {
            pPackStatus->sox_info.soc_disp = 100.1f;
        }
    }
	/*计算SOH*/
	if(pPackStatus->chg_accumulate_capa_ah >= pPackStatus->dsg_accumulate_capa_ah)
	{
		capa_ratio = pPackStatus->dsg_accumulate_capa_ah / (PACK_RATED_CAPACITY / 1000);
    }
	else
	{
		capa_ratio = pPackStatus->chg_accumulate_capa_ah / (PACK_RATED_CAPACITY / 1000);
    }
    pPackStatus->pack_cycles = capa_ratio;
    
    if(capa_ratio > 20)
    {
        _GetTableRangeIndex16u(capa_ratio, CellLoopCyclesDefault, CELL_LOOP_FREQ_LEVEL, &pack_cycles_index_tempd_d, &pack_cycles_index_tempd_u);
        pPackStatus->sox_info.soh = _InterpolationOperation(CellLoopCyclesDefault[pack_cycles_index_tempd_d], CellLoopCyclesDefault[pack_cycles_index_tempd_d], \
                            CellLoopLifeDefault[pack_cycles_index_tempd_d], CellLoopLifeDefault[pack_cycles_index_tempd_d], capa_ratio) / 100.0f;        
    }
    else
    {
        pPackStatus->sox_info.soh = 100;
    }
    pPackStatus->soh_rpt = pPackStatus->sox_info.soh * 10;
    if(gSoxDebugParam.sox_debug_mode == 1)
    {
        gPackStatus.soh_rpt = 1000;
    }
//        pPackStatus->soh_rpt = soh_rpt_test;
	
	
	/*防止越界*/
	if(pPackStatus->sox_info.soc_real > pPackStatus->sox_info.soc_max)
	{
        pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max;
	}
	if(pPackStatus->sox_info.soc_actl > pPackStatus->sox_info.soc_max)
	{
		pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_max;
	}
	if(pPackStatus->sox_info.soc_disp > 100.1f)
	{
		pPackStatus->sox_info.soc_disp = 100.1f;
	}
	
    if(pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE)
    {
        if(abs((int)((pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_min) * 10)) < 10)
        {
            pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_min;
            LOG_ERROR(LOG_TAG, "SOCAmpereHourIntegral   pPackStatus->sox_info.soc_real = %d  pPackStatus->sox_info.soc_min = %d");
        }
        if(abs((int)((pPackStatus->sox_info.soc_actl - pPackStatus->sox_info.soc_min) * 10)) < 10)
        {
            pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_min;
            LOG_ERROR(LOG_TAG, "SOCAmpereHourIntegral   pPackStatus->sox_info.soc_actl = %d  pPackStatus->sox_info.soc_min = %d");
        }
        if(pPackStatus->sox_info.soc_disp < 0.1f)
        {
            pPackStatus->sox_info.soc_disp = 0;
        }
    }	
}

//void SohSet(uint16_t soh_rpt)
//{
//    soh_rpt_test = soh_rpt;
//}

//FINSH_CMD_EXPORT(SohSet, uint16_t soh_rpt);
/*当前满充、满放单体标定电压计算, 每隔一段时间轮询执行一次，主要是温度变化，不需要太频繁，初始化执行一次*/
void FullEmptyChargeRatedVoltageCalc(PackStatus_t *pPackStatus)
{
	uint16_t full_charge_voltage;
	uint16_t full_charge_voltage_tempd, full_charge_voltage_tempu;
	uint16_t empty_charge_voltage;
	uint16_t empty_charge_voltage_tempd, empty_charge_voltage_tempu;
	int16_t temperature = pPackStatus->pack_average_temperature / 10;
	int16_t temp_d, temp_u;
	uint8_t temp_index_d, temp_index_u;
	uint8_t soc_max_d, soc_max_u;
	uint8_t soc_max_index_d, soc_max_index_u;
	
	uint8_t soc_min_d, soc_min_u;
	uint8_t soc_min_index_d, soc_min_index_u;
	
	
	float soc_max = pPackStatus->sox_info.soc_max;
	float soc_min = pPackStatus->sox_info.soc_min;
	
	_GetTableRangeIndex8(temperature, OCVTemperatureIndex, OCV_SOC_TEMPERATURE_LEVEL, &temp_index_d, &temp_index_u);
	temp_d = OCVTemperatureIndex[temp_index_d];
	temp_u = OCVTemperatureIndex[temp_index_u];
	
	_GetTableRangeIndexf8u(soc_max, OCVSOCIndex, OCV_SOC_LEVEL, &soc_max_index_d, &soc_max_index_u);
	soc_max_d = OCVSOCIndex[soc_max_index_d];
	soc_max_u = OCVSOCIndex[soc_max_index_u];
	_GetTableRangeIndexf8u(soc_min, OCVSOCIndex, OCV_SOC_LEVEL, &soc_min_index_d, &soc_min_index_u);
	soc_min_d = OCVSOCIndex[soc_min_index_d];
	soc_min_u = OCVSOCIndex[soc_min_index_u];
	
	
	full_charge_voltage_tempd = _InterpolationOperation(soc_max_d, soc_max_u, OCVSOCDefault[temp_index_d][soc_max_index_d], OCVSOCDefault[temp_index_d][soc_max_index_u], soc_max);
	full_charge_voltage_tempu = _InterpolationOperation(soc_max_d, soc_max_u, OCVSOCDefault[temp_index_u][soc_max_index_d], OCVSOCDefault[temp_index_u][soc_max_index_u], soc_max);
	full_charge_voltage = _InterpolationOperation(temp_d, temp_u, full_charge_voltage_tempd, full_charge_voltage_tempu, temperature);
	
	empty_charge_voltage_tempd = _InterpolationOperation(soc_min_d, soc_min_u, OCVSOCDefault[temp_index_d][soc_min_index_d], OCVSOCDefault[temp_index_d][soc_min_index_u], soc_min);
	empty_charge_voltage_tempu = _InterpolationOperation(soc_min_d, soc_min_u, OCVSOCDefault[temp_index_u][soc_min_index_d], OCVSOCDefault[temp_index_u][soc_min_index_u], soc_min);
	empty_charge_voltage = _InterpolationOperation(temp_d, temp_u, empty_charge_voltage_tempd, empty_charge_voltage_tempu, temperature);
	
	pPackStatus->full_charge_voltage = full_charge_voltage;
	pPackStatus->empty_charge_voltage = empty_charge_voltage;
	// return full_charge_voltage;
	
}





/*OCV校准条件判断*/
static void _OCVJudge(PackStatus_t *pPackStatus)
{
	if(pPackStatus->mode != EN_PACK_CURRENT_IDLE_MODE)
	{
		pPackStatus->ocv_time_count = RtcGetTime();
	}
	else
	{
		if(RtcGetTime() > pPackStatus->ocv_time_count)
		{
			if(RtcGetTime() - pPackStatus->ocv_time_count > 2 * 60 * 60)/*暂定2小时,后期改成宏定义*/
			{
				pPackStatus->ocv_time_count = RtcGetTime();
				OCVCalibraiton(pPackStatus);/*满足OCV校准条件，进行OCV校准*/
			}
		}
		else
		{
			pPackStatus->ocv_time_count = RtcGetTime();
		}
	}
}

void OCV_Test(void)
{
    PackStatus_t *pPackStatus = NULL;
    
    if(gSoxDebugParam.sox_debug_mode == 0)
    {
        pPackStatus = &gPackStatus; 
    }
    else if(gSoxDebugParam.sox_debug_mode == 1)
    {
        pPackStatus = &gPackStatus_debug; 
    }
    
    OCVCalibraiton(pPackStatus); 
    pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_real;
    pPackStatus->sox_info.soc_disp = (pPackStatus->sox_info.soc_actl - pPackStatus->sox_info.soc_min) * 100 / 
                                         (pPackStatus->sox_info.soc_max - pPackStatus->sox_info.soc_min);
}

MSH_CMD_EXPORT(OCV_Test, ---);

void SoxFlashInfoSave(void)
{
    uint8_t save_flg[4];
    
    memset((uint8_t *)save_flg, 0x5A, 4);
    if(gSoxDebugParam.sox_debug_mode == 0)
    {
        spi_flash_write_bytes_with_erase(SOX_FLASH_INFO_ADDR, (uint8_t*)&gPackStatus, sizeof(PackStatus_t));
    }
    else if(gSoxDebugParam.sox_debug_mode == 1)
    {
        spi_flash_write_bytes_with_erase(SOX_FLASH_INFO_ADDR, (uint8_t*)&gPackStatus_debug, sizeof(PackStatus_t));
    }   
    spi_flash_write_bytes_with_erase(SOX_FLASH_INFO_ADDR + 0x1000, save_flg, 4);

    if(gSoxDebugParam.sox_debug_mode == 0)
    {
        spi_flash_write_bytes_with_erase(SOX_FLASH_BACKUP_INFO_ADDR, (uint8_t*)&gPackStatus, sizeof(PackStatus_t));
    }
    else if(gSoxDebugParam.sox_debug_mode == 1)
    {
        spi_flash_write_bytes_with_erase(SOX_FLASH_BACKUP_INFO_ADDR, (uint8_t*)&gPackStatus_debug, sizeof(PackStatus_t));
    }   
    spi_flash_write_bytes_with_erase(SOX_FLASH_BACKUP_INFO_ADDR + 0x1000, save_flg, 4);
        
}

void SoxFlashInfoLoad(void)
{
    uint8_t save_flg[4];
    
    spi_flash_read_bytes(SOX_FLASH_INFO_ADDR + 0x1000, (uint8_t*)save_flg, 4);
    if(save_flg[0] != 0x5A || save_flg[1] != 0x5A || save_flg[2] != 0x5A || save_flg[3] != 0x5A)
    {
        spi_flash_read_bytes(SOX_FLASH_BACKUP_INFO_ADDR + 0x1000, (uint8_t*)save_flg, 4);
        if(save_flg[0] != 0x5A || save_flg[1] != 0x5A || save_flg[2] != 0x5A || save_flg[3] != 0x5A)
        {
            memset((uint8_t*)&sgSoxFlashInfo, 0, sizeof(PackStatus_t));
            gPackStatus.cell_min_voltage = gCell_Info.cell_min_voltage;
            gPackStatus.pack_average_temperature = gCell_Info.cell_ave_temperature;
            gPackStatus.sox_info.soc_max = 100.1;
            OCVCalibraiton(&gPackStatus);
            sgSoxFlashInfo.sox_info.soc_max = 100.1;     
            sgSoxFlashInfo.sox_info.soc_min = 0;
            sgSoxFlashInfo.sox_info.soc_real = gPackStatus.sox_info.soc_real;  
            gPackStatus.sox_info.soc_actl = gPackStatus.sox_info.soc_real;          
            sgSoxFlashInfo.sox_info.soc_actl = gPackStatus.sox_info.soc_real;
            sgSoxFlashInfo.sox_info.soc_disp = (gPackStatus.sox_info.soc_actl - gPackStatus.sox_info.soc_min) * 100 / 
                                             (gPackStatus.sox_info.soc_max - gPackStatus.sox_info.soc_min);
            sgSoxFlashInfo.sox_info.soh = 100;
            LOG_ERROR(LOG_TAG, "Flash load sox Error! re-OCV!");        
        }
        else
        {
            spi_flash_read_bytes(SOX_FLASH_BACKUP_INFO_ADDR, (uint8_t*)&sgSoxFlashInfo, sizeof(PackStatus_t));
        }
    }
    else
    {
        spi_flash_read_bytes(SOX_FLASH_INFO_ADDR, (uint8_t*)&sgSoxFlashInfo, sizeof(PackStatus_t));
    }
    
    sgSoxFlashInfo.sox_info.soc_max = 100.1;  
//    memset((uint8_t *)save_flg, 0x00, 4);
//    spi_flash_write_bytes_with_erase(SOX_FLASH_INFO_ADDR + 0x1000, save_flg, 4);  
    
}

/*从下电到上电，执行一次，从深度睡眠到唤醒执行一次*/
void SoxInit(PackStatus_t *pPackStatus)
{
	uint8_t rdata = 0;
    
	/*如果是掉电或休眠数据丢失，需要重新从FLASH读取休眠或掉电前存储的数据*/
	SoxFlashInfoLoad();
	/*电芯数据从AFE获取，并赋值到gPackStatus*/
    pPackStatus->current_power_percentage    = 1;
	pPackStatus->cell_average_voltage        = gCell_Info.cell_ave_voltage;
    pPackStatus->cell_max_voltage            = gCell_Info.cell_max_voltage; 
    pPackStatus->cell_min_voltage            = gCell_Info.cell_min_voltage;
    pPackStatus->cell_total_voltage          = gCell_Info.total_cell_voltage;
    pPackStatus->pack_current                = gCell_Info.cell_current;
    pPackStatus->pack_average_temperature    = gCell_Info.cell_ave_temperature;
//    pPackStatus->pack_current                = 100000;
//    pPackStatus->pack_average_temperature    = 25;
    pPackStatus->pack_max_temperature        = gCell_Info.cell_max_temperature;
    pPackStatus->pack_min_temperature        = gCell_Info.cell_min_temperature;
    
//    pPackStatus->sox_info.soh = 100;
//    pPackStatus->soh_rpt = 1000;
    pPackStatus->sox_info.soc_max = sgSoxFlashInfo.sox_info.soc_max;
    pPackStatus->sox_info.soc_min = sgSoxFlashInfo.sox_info.soc_min;
    pPackStatus->sox_info.soc_real = sgSoxFlashInfo.sox_info.soc_real;
    pPackStatus->sox_info.soc_actl = sgSoxFlashInfo.sox_info.soc_actl;
    pPackStatus->sox_info.soc_disp = sgSoxFlashInfo.sox_info.soc_disp;
    pPackStatus->sox_info.soh = sgSoxFlashInfo.sox_info.soh;
    pPackStatus->soh_rpt = sgSoxFlashInfo.soh_rpt;
    pPackStatus->chg_accumulate_capa_ah     = sgSoxFlashInfo.chg_accumulate_capa_ah;
    pPackStatus->chg_accumulate_capa_wh = sgSoxFlashInfo.chg_accumulate_capa_wh;
    pPackStatus->dsg_accumulate_capa_ah     = sgSoxFlashInfo.dsg_accumulate_capa_ah;
    pPackStatus->dsg_accumulate_capa_wh = sgSoxFlashInfo.dsg_accumulate_capa_wh;
    pPackStatus->ocv_time_count = sgSoxFlashInfo.ocv_time_count;
    pPackStatus->full_charge_flag = sgSoxFlashInfo.full_charge_flag;
    pPackStatus->empty_charge_flag = sgSoxFlashInfo.empty_charge_flag;
    
    CurrentCapaCalc(pPackStatus);
	FullEmptyChargeRatedVoltageCalc(pPackStatus);
    
	/*上电或休眠唤醒校准*/
	if(RtcGetTime() > pPackStatus->ocv_time_count)
	{
		if(RtcNormalJudge() == true)
		{
			if(RtcGetTime() - pPackStatus->ocv_time_count > 2 * 60 * 60)/*暂定2小时,后期改成宏定义*/
			{
                if(pPackStatus->cell_max_voltage > 3300 || pPackStatus->cell_max_voltage < 3100)
                {
                    OCVCalibraiton(pPackStatus);
                    pPackStatus->ocv_time_count = RtcGetTime();
                    if(abs((int)(pPackStatus->sox_info.soc_actl - pPackStatus->sox_info.soc_real)) > 30)/*表显与实际差异过大, 强制校正*/
                    {
                        pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_real;
                        pPackStatus->sox_info.soc_disp = (pPackStatus->sox_info.soc_actl - pPackStatus->sox_info.soc_min) * 100 / 
                                                         (pPackStatus->sox_info.soc_max - pPackStatus->sox_info.soc_min);
                    }
                }				
			}
		}
	}
    
    /*系统首次上电 OCV校准 */
    EepromRead(&rdata, SOX_FIRST_FIX_ADDR, 1);
    if(rdata != 0xAA)
    {
        OCVCalibraiton(pPackStatus);
        pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_real;
        pPackStatus->sox_info.soc_disp = (pPackStatus->sox_info.soc_actl - pPackStatus->sox_info.soc_min) * 100 / 
                                         (pPackStatus->sox_info.soc_max - pPackStatus->sox_info.soc_min);
        rdata = 0xAA;
        EepromWrite(&rdata,SOX_FIRST_FIX_ADDR, 1);
    }    
	pPackStatus->ocv_time_count = RtcGetTime();
    
    
    
	
	
	
//    if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
//    {
        /*获取当前最大充电倍率换算成电流*/
        MaxChgMultiCalc(pPackStatus);
//    }
//    else
//    {
        /*获取当前放电最大电流*/
        MaxDsgCurrentCalc(pPackStatus);
        
//    }
	
    /*判断电流方向*/
    PackCurrentModeJudge(pPackStatus);
    
	_SoxInitFlag = 1;
}

void FlagClear(void)
{
    uint8_t data = 0;
    gFlagClear = 1;
    EepromWrite(&data,SOX_FIRST_FIX_ADDR, 1);
    spi_flash_auto_erase_sector(SOX_FLASH_INFO_ADDR + 0x1000, 4);
    spi_flash_auto_erase_sector(SOX_FLASH_BACKUP_INFO_ADDR + 0x1000, 4);
}

MSH_CMD_EXPORT(FlagClear, flag);


//uint32_t count1,count2;
//uint8_t count3 = 100;
/*大轮询调用*/
void SoxProcess(PackStatus_t *pPackStatus)
{
	static uint32_t timecount = 0;
	
	if((uint32_t)(tim_get_systime() - timecount) > 1000)
	{
        timecount = tim_get_systime();
		/*1S执行的非高实时性*/
		CurrentCapaCalc(pPackStatus);
		FullEmptyChargeRatedVoltageCalc(pPackStatus);
//		_OCVJudge(pPackStatus);
	}
    
	pPackStatus->cell_average_voltage        = gCell_Info.cell_ave_voltage;
    pPackStatus->cell_max_voltage            = gCell_Info.cell_max_voltage; 
    pPackStatus->cell_min_voltage            = gCell_Info.cell_min_voltage;
    pPackStatus->cell_total_voltage          = gCell_Info.total_cell_voltage;       
    pPackStatus->pack_average_temperature    = gCell_Info.cell_ave_temperature;      
    pPackStatus->pack_max_temperature        = gCell_Info.cell_max_temperature;    
    pPackStatus->pack_min_temperature        = gCell_Info.cell_min_temperature;    
    
    
    
	PackCurrentModeJudge(pPackStatus);/*通过电流方向判断充放电状态*/
	/*获取当前放电最大电流*/
	MaxDsgCurrentCalc(pPackStatus);
	/*获取当前最大充电倍率换算成电流*/
	MaxChgMultiCalc(pPackStatus);
    
    
    /*test*/
//    count1 = tim_get_systime();
//    for(int i = 0; i < count3; i++)
//    {
//        PackCurrentModeJudge();
//        MaxDsgCurrentCalc();
//        MaxChgMultiCalc();
//        SOCAmpereHourIntegral(10);
//        CurrentCapaCalc();
//		FullEmptyChargeRatedVoltageCalc();
//		_OCVJudge();
//    }
//    
//    count2 = tim_get_systime() - count1;
}




/*当检测到充满或者放空后，标志位置1后，n秒后如果电流依然处于充放电状态，则需要掐断MOS*/












/*
1、首次上电，RTC尚未配置，需要做开路检测；
2、正常运行时，实时获取RTC的值并记录，(考虑定期保存，避免异常掉电丢失)，进入休眠模式前也要保存
3、下次上电，或唤醒，根据检测保存的RTC数值和实际当前RTC数值对比，获取静置时间，并判断是否进行校准
4、系统正常运行过程中，如果未接负载，则定时进行开路检测校准
5、ACT_SOC每变化1%,则对所有SOC信息进行一次存储

*/

static int32_t sox_mode(uint32_t argc, uint8_t **argv) 
{
    
    if(argc != 2)
    {
        LOG_STRING(LOG_TAG, "parma error!\r\n");
        return 1;
    }
    
    gSoxDebugParam.sox_debug_mode = atoi((char *)argv[1]);
    
    if(gSoxDebugParam.sox_debug_mode > 2)
    {
        gSoxDebugParam.sox_debug_mode = 2;
    }

    return 0;
}

static int32_t sox_current(uint32_t argc, uint8_t **argv) 
{
    
    if(argc != 2)
    {
        LOG_STRING(LOG_TAG, "parma error!\r\n");
        return 1;
    }
    
    gSoxDebugParam.cell_current_debug = atoi((char *)argv[1]);
    
    if(gSoxDebugParam.cell_current_debug > 150000)
    {
        gSoxDebugParam.cell_current_debug = 150000;
    }

    return 0;
}
void SocDisplay()
{
    LOG_INFO(LOG_TAG, "Soc_real:%4.2f\t Soc_actl:%4.2f\t Soc_disp:%4.2f\t", gPackStatus.sox_info.soc_real, gPackStatus.sox_info.soc_actl, gPackStatus.sox_info.soc_disp);
}

MSH_CMD_EXPORT(SocDisplay, SocDisplay);


MSH_CMD_EXPORT(sox_mode, 0_normal-1_debug-2_all);
MSH_CMD_EXPORT(sox_current, uint32_current_mA);