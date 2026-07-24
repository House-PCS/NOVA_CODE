/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_3d_homepage.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-01-31
  * @brief       3D版-首页翻页后的电量统计
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"

#define DEBUG_TEST 				0


#define EM_ELEC_INFO_ICON_GRID_CHARGE_SIZE     (4 * 1024)
#define EM_ELEC_INFO_ICON_GRID_TIED_SIZE       (4 * 1024)
#define EM_ELEC_INFO_ICON_PV_CHARGE_SIZE       (4 * 1024)
#define EM_ELEC_INFO_ICON_LOAD_SIZE            (4 * 1024)

#define EM_ELEC_INFO_MID_TIME_ICON_SIZE					(4* 1024 * 2)
#define EM_ELEC_INFO_BOTTOM_BATTERY_ICON_SIZE			(4* 1024 * 2)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gEmWinElectricityInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
} UiCtx_t;


typedef struct _GRID_CHARGE_T
{
	float elec_today;
	float elec_nov;
	float elec_year;
	float elec_total;
}GRID_CHARGE_T;

typedef struct _GRID_TIED_T
{
	float elec_today;
	float elec_nov;
	float elec_year;
	float elec_total;
	
}GRID_TIED_T;

typedef struct _PV_CHARGE_T
{
	float elec_today;
	float elec_nov;
	float elec_year;
	float elec_total;
	
}PV_CHARGE_T;

typedef struct _BACKUP_LOAD_T
{
	float elec_today;
	float elec_nov;
	float elec_year;
	float elec_total;
	
}BACKUP_LOAD_T;

/*函数声明*/
int32_t SetGridChargeInfo(GRID_CHARGE_T Grid_Charge);//电网发电统计
int32_t SetGridTiedInfo(GRID_TIED_T Grid_Tied);//向电网馈电统计
int32_t SetPVChargeInfo(PV_CHARGE_T PV_Charge);//太阳能充电统计
int32_t SetBackupLoadInfo(BACKUP_LOAD_T Backup_Load);//负载用电统计
int32_t SetRocaOPTTime(int32_t c_time);//设置Roca OPT Time
int32_t SetOnGridTime(int32_t c_time);//设置On Grid Time
int32_t SetBatteryCycNum(int32_t c_nums);//设置电池循环次数
int32_t SetSOHState(int32_t c_precentage);//设置SOH


/*各个子窗口控件句柄及其他相关参数*/
/*顶部电量信息统计子窗口*/
static GUI_HWIN _gWinElecInfoHandle;
static IMAGE_Handle _gImgGridChargeHandle;//电网充电图标句柄
static IMAGE_Handle _gImgGridTiedHandle;//电网馈电图标句柄
static IMAGE_Handle _gImgPVChargeHandle;//太阳能充电图标句柄
static IMAGE_Handle _gImgLoadHandle;//负载图标句柄
static TEXT_Handle  _gTextkWhHandle;
static TEXT_Handle 	_gTextTodayHandle;
static TEXT_Handle	_gTextNovHandle;
static TEXT_Handle 	_gTextYearHandle;
static TEXT_Handle 	_gTextTotalHandle;

/*当天电量统计*/
static TEXT_Handle _gTextGridChargeToadyHandle;
static TEXT_Handle _gTextGridTiedTodayHandle;
static TEXT_Handle _gTextPVchargeTodayHandle;
static TEXT_Handle _gTextLoadTodayHandle;
/*当前电量统计*/
static TEXT_Handle _gTextGridChargeNovHandle;
static TEXT_Handle _gTextGridTiedNovHandle;
static TEXT_Handle _gTextPVchargeNovHandle;
static TEXT_Handle _gTextLoadNovHandle;
/*当年电量统计*/
static TEXT_Handle _gTextGridChargeYearHandle;
static TEXT_Handle _gTextGridTiedYearHandle;
static TEXT_Handle _gTextPVchargeYearHandle;
static TEXT_Handle _gTextLoadYearHandle;
/*总电量统计*/
static TEXT_Handle _gTextGridChargeTotalHandle;
static TEXT_Handle _gTextGridTiedTotalHandle;
static TEXT_Handle _gTextPVChargeTotalHandle;
static TEXT_Handle _gTextLoadTotalHandle;

/*中部区域发电时长*/
static IMAGE_Handle _gImgRocaOptTimeHandle;//时间图标句柄
static TEXT_Handle 	_gTextRocaOPTHandle;
static TEXT_Handle 	_gTextRocaOPTTimeHandle;
static TEXT_Handle 	_gTextOnGirdHandle;
static TEXT_Handle 	_gTextOnGirdTimeHandle;

/*底部区域*/
static IMAGE_Handle _gImgBatteryHandle;//电池图标句柄
static TEXT_Handle 	_gTextBatCyclesHandle;
static TEXT_Handle 	_gTextBatCyclesNumsHandle;
static TEXT_Handle 	_gTextSOHHandle;
static TEXT_Handle 	_gTextSOHStatHandle;


/*顶部电量统计窗口图标参数*/
static emWidgetImgPara_t  _gElecGridChargeImgPara;
static emWidgetImgPara_t  _gElecGridTiedImgPara;
static emWidgetImgPara_t  _gElecPVChargeImgPara;
static emWidgetImgPara_t  _gElecLoadImgPara;
static emWidgetTextPara_t  _gkWhTextPara;
static emWidgetTextPara_t  _gTodayTextPara;
static emWidgetTextPara_t  _gNovTextPara;
static emWidgetTextPara_t  _gYearTextPara;
static emWidgetTextPara_t  _gTotalTextPara;
static char _gkWhTextBuf[20] = "kWh";
static char _gTodayTextBuf[20] = "Today"; 
static char _gNovTextBuf[20] = "Nov"; 
static char _gYearTextBuf[20] = "Year";
static char _gTotalTextBuf[20] = "Total"; 

/*当天电量统计*/
static emWidgetTextPara_t  _gElecGridChargeTodayTextPara;
static emWidgetTextPara_t  _gElecGridTiedTodayTextPara;
static emWidgetTextPara_t  _gElecPVChargeTodayTextPara;
static emWidgetTextPara_t  _gElecLoadTodayTextPara;
static char _gGridChargeToadyTextBuf[20] = {0};
static char _gGridTiedTodayTextBuf[20] = {0}; 
static char _gPVchargeTodayTextBuf[20] = {0}; 
static char _gLoadTodayTextBuf[20] = {0}; 
/*目前电量统计*/
static emWidgetTextPara_t  _gElecGridChargeNovTextPara;
static emWidgetTextPara_t  _gElecGridTiedNovTextPara;
static emWidgetTextPara_t  _gElecPVChargeNovTextPara;
static emWidgetTextPara_t  _gElecLoadNovTextPara;
static char _gGridChargeNovTextBuf[20] = {0};
static char _gGridTiedNovTextBuf[20] = {0}; 
static char _gPVchargeNovTextBuf[20] = {0}; 
static char _gLoadNovTextBuf[20] = {0}; 
/*当年电量统计*/
static emWidgetTextPara_t  _gElecGridChargeYearTextPara;
static emWidgetTextPara_t  _gElecGridTiedYearTextPara;
static emWidgetTextPara_t  _gElecPVChargeYearTextPara;
static emWidgetTextPara_t  _gElecLoadYearTextPara;
static char _gGridChargeYearTextBuf[20] = {0};
static char _gGridTiedYearTextBuf[20] = {0}; 
static char _gPVchargeYearTextBuf[20] = {0}; 
static char _gLoadYearTextBuf[20] = {0}; 
/*总电量统计*/
static emWidgetTextPara_t  _gElecGridChargeTotalTextPara;
static emWidgetTextPara_t  _gElecGridTiedTotalTextPara;
static emWidgetTextPara_t  _gElecPVChargeTotalTextPara;
static emWidgetTextPara_t  _gElecLoadTotalTextPara;
static char _gGridChargeTotalTextBuf[20] = {0};
static char _gGridTiedTotalTextBuf[20] = {0}; 
static char _gPVchargeTotalTextBuf[20] = {0}; 
static char _gLoadTotalTextBuf[20] = {0}; 

/*中间发电时长窗口图标参数*/
static GUI_HWIN _gWinElecMidInfoHandle;
static emWidgetImgPara_t 	_gRocaOptTimeImgPara;
static emWidgetTextPara_t  	_gRocaOPTTextPara;
static emWidgetTextPara_t  	_gRocaOPTTimeTextPara;
static emWidgetTextPara_t  	_gOnGirdTextPara;
static emWidgetTextPara_t  	_gOnGirdTimeTextPara;
static char _gRocaOPTTextBuf[20] = {0};
static char _gRocaOPTTimeTextBuf[20] = {0}; 
static char _gOnGirdTextBuf[20] = {0}; 
static char _gOnGirdTimeTextBuf[20] = {0}; 
/*底部区域*/
static GUI_HWIN _gWinElecBottomInfoHandle;
static emWidgetImgPara_t 	_gBatteryImgPara;
static emWidgetTextPara_t  	_gBatCyclesTextPara;
static emWidgetTextPara_t  	_gBatCyclesNumsTextPara;
static emWidgetTextPara_t  	_gSOHTextPara;
static emWidgetTextPara_t  	_gSOHStatTextPara;
static char _gBatCyclesTextBuf[20] = {0};
static char _gBatCyclesNumsTextBuf[20] = {0}; 
static char _gSOHTextBuf[20] = {0}; 
static char _gSOHStatTextBuf[20] = {0}; 



/*顶部状态栏控件ID号*/
#define WIN_ELECTRICITY_INFO_ID								(GUI_ID_USER + 0x10)
#define WIN_ELEC_INFO_GRID_CHARGE_IMG_ID					(GUI_ID_USER + 0x11)
#define WIN_ELEC_INFO_GRID_TIED_IMG_ID						(GUI_ID_USER + 0x12)
#define WIN_ELEC_INFO_PV_CHARGE_IMG_ID						(GUI_ID_USER + 0x13)
#define WIN_ELEC_INFO_LOAD_IMG_ID							(GUI_ID_USER + 0x14)
#define WIN_ELEC_INFO_KWH_TEXT_ID							(GUI_ID_USER + 0x15)
#define WIN_ELEC_INFO_TODAY_TEXT_ID							(GUI_ID_USER + 0x16)
#define WIN_ELEC_INFO_NOV_TEXT_ID							(GUI_ID_USER + 0x17)
#define WIN_ELEC_INFO_YEAR_TEXT_ID							(GUI_ID_USER + 0x18)
#define WIN_ELEC_INFO_TOTAL_TEXT_ID							(GUI_ID_USER + 0x19)

#define WIN_ELEC_INFO_GRID_CHARGE_TODAY_TEXT_ID				(GUI_ID_USER + 0x20)
#define WIN_ELEC_INFO_GRID_CHARGE_NOV_TEXT_ID				(GUI_ID_USER + 0x21)
#define WIN_ELEC_INFO_GRID_CHARGE_YEAR_TEXT_ID				(GUI_ID_USER + 0x22)
#define WIN_ELEC_INFO_GRID_CHARGE_TOTAL_TEXT_ID				(GUI_ID_USER + 0x23)
#define WIN_ELEC_INFO_GRID_TIED_TODAY_TEXT_ID				(GUI_ID_USER + 0x24)
#define WIN_ELEC_INFO_GRID_TIED_NOV_TEXT_ID					(GUI_ID_USER + 0x25)
#define WIN_ELEC_INFO_GRID_TIED_YEAR_TEXT_ID				(GUI_ID_USER + 0x26)
#define WIN_ELEC_INFO_GRID_TIED_TOTAL_TEXT_ID				(GUI_ID_USER + 0x27)
#define WIN_ELEC_INFO_PV_CHARGE_TODAY_TEXT_ID				(GUI_ID_USER + 0x28)
#define WIN_ELEC_INFO_PV_CHARGE_NOV_TEXT_ID					(GUI_ID_USER + 0x29)
#define WIN_ELEC_INFO_PV_CHARGE_YEAR_TEXT_ID				(GUI_ID_USER + 0x2A)
#define WIN_ELEC_INFO_PV_CHARGE_TOTAL_TEXT_ID				(GUI_ID_USER + 0x2B)
#define WIN_ELEC_INFO_LOAD_TODAY_TEXT_ID					(GUI_ID_USER + 0x2C)
#define WIN_ELEC_INFO_LOAD_NOV_TEXT_ID						(GUI_ID_USER + 0x2D)
#define WIN_ELEC_INFO_LOAD_YEAR_TEXT_ID						(GUI_ID_USER + 0x2E)
#define WIN_ELEC_INFO_LOAD_TOTAL_TEXT_ID					(GUI_ID_USER + 0x2F)

#define WIN_ELEC_INFO_ROCA_OPT_IMG_ID						(GUI_ID_USER + 0x30)
#define WIN_ELEC_INFO_ROCA_OPT_TEXT_ID						(GUI_ID_USER + 0x31)
#define WIN_ELEC_INFO_ROCA_OPT_TIME_TEXT_ID					(GUI_ID_USER + 0x32)
#define WIN_ELEC_INFO_ON_GRID_TEXT_ID						(GUI_ID_USER + 0x33)
#define WIN_ELEC_INFO_ON_GRID_TIME_TEXT_ID					(GUI_ID_USER + 0x34)
#define WIN_ELEC_INFO_BATTERY_IMG_ID						(GUI_ID_USER + 0x35)
#define WIN_ELEC_INFO_BATTERY_CYC_TEXT_ID					(GUI_ID_USER + 0x36)
#define WIN_ELEC_INFO_BATTERY_CYC_NUMS_TEXT_ID				(GUI_ID_USER + 0x37)
#define WIN_ELEC_INFO_SOH_TEXT_ID							(GUI_ID_USER + 0x38)
#define WIN_ELEC_INFO_SOH_STAT_TEXT_ID						(GUI_ID_USER + 0x39)

#define WIN_ELEC_MID_INFO_ID								(GUI_ID_USER + 0x3A)
#define WIN_ELEC_MID_INFO_TIME_IMG_ID						(GUI_ID_USER + 0x3B)
#define WIN_ELEC_MID_INFO_ROCA_OPT_TEXT_ID					(GUI_ID_USER + 0x3C)
#define WIN_ELEC_MID_INFO_ROCA_OPT_TIME_TEXT_ID				(GUI_ID_USER + 0x3D)
#define WIN_ELEC_MID_INFO_ON_GRID_TEXT_ID					(GUI_ID_USER + 0x3E)
#define WIN_ELEC_MID_INFO_ON_GRID_TIME_TEXT_ID				(GUI_ID_USER + 0x3F)

#define WIN_ELEC_BOTTOM_INFO_ID								(GUI_ID_USER + 0x3A)
#define WIN_ELEC_BOTTOM_INFO_BATTERY_IMG_ID					(GUI_ID_USER + 0x40)
#define WIN_ELEC_BOTTOM_INFO_BAT_CYC_TEXT_ID				(GUI_ID_USER + 0x41)
#define WIN_ELEC_BOTTOM_INFO_BAT_CYC_NUMS_TEXT_ID			(GUI_ID_USER + 0x42)
#define WIN_ELEC_BOTTOM_INFO_SOH_TEXT_ID					(GUI_ID_USER + 0x43)
#define WIN_ELEC_BOTTOM_INFO_SOH_STAT_TEXT_ID				(GUI_ID_USER + 0x44)

/*资源列表*/
/**
 * @brief 顶部状态栏
 */
static const GUI_WIDGET_CREATE_INFO _aElecInfoStatDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "StatBar",		WIN_ELECTRICITY_INFO_ID, 							0,  	0,	 EM_XSIZE,  169,  0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "kwh",			WIN_ELEC_INFO_KWH_TEXT_ID,       					30, 	20, 	30,   17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "today",			WIN_ELEC_INFO_TODAY_TEXT_ID,       					60+16, 	20, 	48,   17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "nov",			WIN_ELEC_INFO_NOV_TEXT_ID,       					175, 	20,  	27,   17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "year",			WIN_ELEC_INFO_YEAR_TEXT_ID,       					272, 	20,  	34,   17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "total",			WIN_ELEC_INFO_TOTAL_TEXT_ID,       					388,	20,  	40,   17,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "gridcharge",		WIN_ELEC_INFO_GRID_CHARGE_IMG_ID,       			30,	  	53,  	30,   18,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "gridtied",		WIN_ELEC_INFO_GRID_TIED_IMG_ID,       				30,	  	80,  	30,   18,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "pvcharge",		WIN_ELEC_INFO_PV_CHARGE_IMG_ID,       				32,	  	110,  	15,   15,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "load",			WIN_ELEC_INFO_LOAD_IMG_ID,       					31,	  	136,  	16,   16,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "today1",			WIN_ELEC_INFO_GRID_CHARGE_TODAY_TEXT_ID,       		83, 	53,  	38,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "nov1",			WIN_ELEC_INFO_GRID_CHARGE_NOV_TEXT_ID,       		165, 	53,     48,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "year1",			WIN_ELEC_INFO_GRID_CHARGE_YEAR_TEXT_ID,       		259, 	53,  	59,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "total1",			WIN_ELEC_INFO_GRID_CHARGE_TOTAL_TEXT_ID,       		372,	53,  	70,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "today2",			WIN_ELEC_INFO_GRID_TIED_TODAY_TEXT_ID,       		83, 	81,  	38,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "nov2",			WIN_ELEC_INFO_GRID_TIED_NOV_TEXT_ID,       			165, 	81,  	48,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "year2",			WIN_ELEC_INFO_GRID_TIED_YEAR_TEXT_ID,       		259, 	81,  	59,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "total2",			WIN_ELEC_INFO_GRID_TIED_TOTAL_TEXT_ID,       		372,	81,  	70,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "today3",			WIN_ELEC_INFO_PV_CHARGE_TODAY_TEXT_ID,       		83, 	109,  	38,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "nov3",			WIN_ELEC_INFO_PV_CHARGE_NOV_TEXT_ID,       			165, 	109,  	48,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "year3",			WIN_ELEC_INFO_PV_CHARGE_YEAR_TEXT_ID,       		259, 	109,  	59,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "total3",			WIN_ELEC_INFO_PV_CHARGE_TOTAL_TEXT_ID,       		372,	109,  	70,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "today4",			WIN_ELEC_INFO_LOAD_TODAY_TEXT_ID,       			83, 	137,  	38,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "nov4",			WIN_ELEC_INFO_LOAD_NOV_TEXT_ID,       				165, 	137,  	48,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "year4",			WIN_ELEC_INFO_LOAD_YEAR_TEXT_ID,       				259,	137,  	59,   22,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "total4",			WIN_ELEC_INFO_LOAD_TOTAL_TEXT_ID,       			372,	137, 	70,   22,   0,   0x0,  0 },
};

/**
 * @brief 中间信息栏
 */
static const GUI_WIDGET_CREATE_INFO _aMiddleTextInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "middle",			WIN_ELEC_MID_INFO_ID,        					0,  	177, 	EM_XSIZE,  62,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "ImgTime",		WIN_ELEC_MID_INFO_TIME_IMG_ID,      			30,		12,  	38,   	38,     0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "roca",			WIN_ELEC_MID_INFO_ROCA_OPT_TEXT_ID,       		83,		23,  	130,   	19,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "roca time",		WIN_ELEC_MID_INFO_ROCA_OPT_TIME_TEXT_ID,       	210,	23,  	40,   	19,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "ongrid",			WIN_ELEC_MID_INFO_ON_GRID_TEXT_ID,       		280,	23,  	130,   	19,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "ongridtime",		WIN_ELEC_MID_INFO_ON_GRID_TIME_TEXT_ID,       	400,	23,  	40,   	19,   	0,   0x0,  0 },
};

/**
 * @brief 底部信息栏
 */
static const GUI_WIDGET_CREATE_INFO _aBottomTextInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "bottom",			WIN_ELEC_BOTTOM_INFO_ID,        				0,  	247, 	EM_XSIZE,  62,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "Imgbat",			WIN_ELEC_BOTTOM_INFO_BATTERY_IMG_ID,      		30,		12,  	38,   	38,     0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "bat cyc",		WIN_ELEC_BOTTOM_INFO_BAT_CYC_TEXT_ID,       	83,		23,  	130,   	19,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "bat cyc nums",	WIN_ELEC_BOTTOM_INFO_BAT_CYC_NUMS_TEXT_ID,      220,	23,  	38,   	19,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "SOH",			WIN_ELEC_BOTTOM_INFO_SOH_TEXT_ID,       		280,	23,  	130,   	19,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "SOH Stat",		WIN_ELEC_BOTTOM_INFO_SOH_STAT_TEXT_ID,       	400,	23,  	44,   	19,   	0,   0x0,  0 },
};

/**
  * @brief  ElecInfoTopStatSubWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbElecInfoTopStatSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;

    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:   
        {
            /*初始化各个子控件*/
			/*电网充电*/
			_gImgGridChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_CHARGE_IMG_ID);
			IMAGE_SetDTA(_gImgGridChargeHandle, _gElecGridChargeImgPara.p_img_buf, _gElecGridChargeImgPara.img_size); 
			//电网馈电
			_gImgGridTiedHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_TIED_IMG_ID);
			IMAGE_SetDTA(_gImgGridTiedHandle, _gElecGridTiedImgPara.p_img_buf, _gElecGridTiedImgPara.img_size); 
			//太阳能充电
			_gImgPVChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_PV_CHARGE_IMG_ID);
			IMAGE_SetDTA(_gImgPVChargeHandle, _gElecPVChargeImgPara.p_img_buf, _gElecPVChargeImgPara.img_size);
			//负载
			_gImgLoadHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_LOAD_IMG_ID);
			IMAGE_SetDTA(_gImgLoadHandle, _gElecLoadImgPara.p_img_buf, _gElecLoadImgPara.img_size);
			
			//kwh
            _gTextkWhHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_KWH_TEXT_ID);
            TEXT_SetFont(_gTextkWhHandle, _gkWhTextPara.p_text_font);
            TEXT_SetText(_gTextkWhHandle, (const char *)_gkWhTextPara.p_content);
            TEXT_SetTextAlign(_gTextkWhHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextkWhHandle, _gkWhTextPara.text_color); 
			//today
            _gTextTodayHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_TODAY_TEXT_ID);
            TEXT_SetFont(_gTextTodayHandle, _gTodayTextPara.p_text_font);
            TEXT_SetText(_gTextTodayHandle, (const char *)_gTodayTextPara.p_content);
            TEXT_SetTextAlign(_gTextTodayHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextTodayHandle, _gTodayTextPara.text_color); 
			//nov
            _gTextNovHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_NOV_TEXT_ID);
            TEXT_SetFont(_gTextNovHandle, _gNovTextPara.p_text_font);
            TEXT_SetText(_gTextNovHandle, (const char *)_gNovTextPara.p_content);
            TEXT_SetTextAlign(_gTextNovHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextNovHandle, _gNovTextPara.text_color); 
			//year
            _gTextYearHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_YEAR_TEXT_ID);
            TEXT_SetFont(_gTextYearHandle, _gYearTextPara.p_text_font);
            TEXT_SetText(_gTextYearHandle, (const char *)_gYearTextPara.p_content);
            TEXT_SetTextAlign(_gTextYearHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextYearHandle, _gYearTextPara.text_color); 
			//total
            _gTextTotalHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_TOTAL_TEXT_ID);
            TEXT_SetFont(_gTextTotalHandle, _gTotalTextPara.p_text_font);
            TEXT_SetText(_gTextTotalHandle, (const char *)_gTotalTextPara.p_content);
            TEXT_SetTextAlign(_gTextTotalHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextTotalHandle, _gTotalTextPara.text_color); 
			//grid charge today
            _gTextGridChargeToadyHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_CHARGE_TODAY_TEXT_ID);
            TEXT_SetFont(_gTextGridChargeToadyHandle, _gElecGridChargeTodayTextPara.p_text_font);
            TEXT_SetText(_gTextGridChargeToadyHandle, (const char *)_gElecGridChargeTodayTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridChargeToadyHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridChargeToadyHandle, _gElecGridChargeTodayTextPara.text_color); 
			//grid charge nov
            _gTextGridChargeNovHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_CHARGE_NOV_TEXT_ID);
            TEXT_SetFont(_gTextGridChargeNovHandle, _gElecGridChargeNovTextPara.p_text_font);
            TEXT_SetText(_gTextGridChargeNovHandle, (const char *)_gElecGridChargeNovTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridChargeNovHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridChargeNovHandle, _gElecGridChargeNovTextPara.text_color); 
			//grid charge year
            _gTextGridChargeYearHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_CHARGE_YEAR_TEXT_ID);
            TEXT_SetFont(_gTextGridChargeYearHandle, _gElecGridChargeYearTextPara.p_text_font);
            TEXT_SetText(_gTextGridChargeYearHandle, (const char *)_gElecGridChargeYearTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridChargeYearHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridChargeYearHandle, _gElecGridChargeYearTextPara.text_color); 
			//grid charge total
            _gTextGridChargeTotalHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_CHARGE_TOTAL_TEXT_ID);
            TEXT_SetFont(_gTextGridChargeTotalHandle, _gElecGridChargeTotalTextPara.p_text_font);
            TEXT_SetText(_gTextGridChargeTotalHandle, (const char *)_gElecGridChargeTotalTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridChargeTotalHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridChargeTotalHandle, _gElecGridChargeTotalTextPara.text_color); 
			//grid tied today
            _gTextGridTiedTodayHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_TIED_TODAY_TEXT_ID);
            TEXT_SetFont(_gTextGridTiedTodayHandle, _gElecGridTiedTodayTextPara.p_text_font);
            TEXT_SetText(_gTextGridTiedTodayHandle, (const char *)_gElecGridTiedTodayTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridTiedTodayHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridTiedTodayHandle, _gElecGridTiedTodayTextPara.text_color); 
			//grid tied nov
            _gTextGridTiedNovHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_TIED_NOV_TEXT_ID);
            TEXT_SetFont(_gTextGridTiedNovHandle, _gElecGridChargeNovTextPara.p_text_font);
            TEXT_SetText(_gTextGridTiedNovHandle, (const char *)_gElecGridChargeNovTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridTiedNovHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridTiedNovHandle, _gElecGridChargeNovTextPara.text_color); 
			//grid tied year
            _gTextGridTiedYearHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_TIED_YEAR_TEXT_ID);
            TEXT_SetFont(_gTextGridTiedYearHandle, _gElecGridTiedYearTextPara.p_text_font);
            TEXT_SetText(_gTextGridTiedYearHandle, (const char *)_gElecGridTiedYearTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridTiedYearHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridTiedYearHandle, _gElecGridTiedYearTextPara.text_color); 
			//grid tied total
            _gTextGridTiedTotalHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_TIED_TOTAL_TEXT_ID);
            TEXT_SetFont(_gTextGridTiedTotalHandle, _gElecGridTiedTotalTextPara.p_text_font);
            TEXT_SetText(_gTextGridTiedTotalHandle, (const char *)_gElecGridTiedTotalTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridTiedTotalHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridTiedTotalHandle, _gElecGridTiedTotalTextPara.text_color); 
			//pv charge today
            _gTextPVchargeTodayHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_PV_CHARGE_TODAY_TEXT_ID);
            TEXT_SetFont(_gTextPVchargeTodayHandle, _gElecPVChargeTodayTextPara.p_text_font);
            TEXT_SetText(_gTextPVchargeTodayHandle, (const char *)_gElecPVChargeTodayTextPara.p_content);
            TEXT_SetTextAlign(_gTextPVchargeTodayHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPVchargeTodayHandle, _gElecPVChargeTodayTextPara.text_color); 
			//pv charge nov
            _gTextPVchargeNovHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_PV_CHARGE_NOV_TEXT_ID);
            TEXT_SetFont(_gTextPVchargeNovHandle, _gElecPVChargeNovTextPara.p_text_font);
            TEXT_SetText(_gTextPVchargeNovHandle, (const char *)_gElecPVChargeNovTextPara.p_content);
            TEXT_SetTextAlign(_gTextPVchargeNovHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPVchargeNovHandle, _gElecPVChargeNovTextPara.text_color); 
			//pv charge year
            _gTextPVchargeYearHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_PV_CHARGE_YEAR_TEXT_ID);
            TEXT_SetFont(_gTextPVchargeYearHandle, _gElecPVChargeYearTextPara.p_text_font);
            TEXT_SetText(_gTextPVchargeYearHandle, (const char *)_gElecPVChargeYearTextPara.p_content);
            TEXT_SetTextAlign(_gTextPVchargeYearHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPVchargeYearHandle, _gElecPVChargeYearTextPara.text_color); 
			//pv charge total
            _gTextPVChargeTotalHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_PV_CHARGE_TOTAL_TEXT_ID);
            TEXT_SetFont(_gTextPVChargeTotalHandle, _gElecPVChargeTotalTextPara.p_text_font);
            TEXT_SetText(_gTextPVChargeTotalHandle, (const char *)_gElecPVChargeTotalTextPara.p_content);
            TEXT_SetTextAlign(_gTextPVChargeTotalHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPVChargeTotalHandle, _gElecPVChargeTotalTextPara.text_color); 
			//load today
            _gTextLoadTodayHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_LOAD_TODAY_TEXT_ID);
            TEXT_SetFont(_gTextLoadTodayHandle, _gElecLoadTodayTextPara.p_text_font);
            TEXT_SetText(_gTextLoadTodayHandle, (const char *)_gElecLoadTodayTextPara.p_content);
            TEXT_SetTextAlign(_gTextLoadTodayHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLoadTodayHandle, _gElecLoadTodayTextPara.text_color); 
			//load nov
            _gTextLoadNovHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_LOAD_NOV_TEXT_ID);
            TEXT_SetFont(_gTextLoadNovHandle, _gElecLoadNovTextPara.p_text_font);
            TEXT_SetText(_gTextLoadNovHandle, (const char *)_gElecLoadNovTextPara.p_content);
            TEXT_SetTextAlign(_gTextLoadNovHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLoadNovHandle, _gElecLoadNovTextPara.text_color); 
			//load year
            _gTextLoadYearHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_LOAD_YEAR_TEXT_ID);
            TEXT_SetFont(_gTextLoadYearHandle, _gElecLoadYearTextPara.p_text_font);
            TEXT_SetText(_gTextLoadYearHandle, (const char *)_gElecLoadYearTextPara.p_content);
            TEXT_SetTextAlign(_gTextLoadYearHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLoadYearHandle, _gElecLoadYearTextPara.text_color); 
			//load total
            _gTextLoadTotalHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_LOAD_TOTAL_TEXT_ID);
            TEXT_SetFont(_gTextLoadTotalHandle, _gElecLoadTotalTextPara.p_text_font);
            TEXT_SetText(_gTextLoadTotalHandle, (const char *)_gElecLoadTotalTextPara.p_content);
            TEXT_SetTextAlign(_gTextLoadTotalHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLoadTotalHandle, _gElecLoadTotalTextPara.text_color); 
            break;
        }
        case WM_PAINT:
        {
            GUI_SetColor(0x00424242);//上色
			GUI_FillRect(12,8,12+456,8+161);

			GUI_SetPenSize(2);
            GUI_SetColor(0x00797979);
			GUI_DrawLine(139, 53, 139, 53+102);
			GUI_DrawLine(239, 53, 239, 53+102);
			GUI_DrawLine(339, 53, 339, 53+102);
#if DEBUG_TEST	
			GRID_CHARGE_T grid_charge = {2,34,666,3434};
			GRID_TIED_T ttt = {3,46,346,5454.2}; 
			PV_CHARGE_T pv = {1.2,45,466,4546.3};
			BACKUP_LOAD_T lll = {2,55,325.3,6435.2};
			
			SetGridChargeInfo(grid_charge);
			SetGridTiedInfo(ttt);
			SetPVChargeInfo(pv);
			SetBackupLoadInfo(lll);
#endif			
            break;
        }
        case WM_NOTIFY_PARENT:/*基于该控件窗口的子控件触发时，如虚拟按键*/
        {
            break;
        }
        default:
        {
            WM_DefaultProc(pMsg);
            break;
        }
    }
}

/**
  * @brief middle Win callback
  * @param  NULL
  * @retval NULL
  */ 
static void _cbMiddleSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;

    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:   
        {
            /*初始化各个子控件*/
			//时间图标
			_gImgRocaOptTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_MID_INFO_TIME_IMG_ID);
			IMAGE_SetDTA(_gImgRocaOptTimeHandle, _gRocaOptTimeImgPara.p_img_buf, _gRocaOptTimeImgPara.img_size); 
			
			//roca opt
            _gTextRocaOPTHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_MID_INFO_ROCA_OPT_TEXT_ID);
            TEXT_SetFont(_gTextRocaOPTHandle, _gRocaOPTTextPara.p_text_font);
            TEXT_SetText(_gTextRocaOPTHandle, (const char *)_gRocaOPTTextPara.p_content);
            TEXT_SetTextAlign(_gTextRocaOPTHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextRocaOPTHandle, _gRocaOPTTextPara.text_color); 
			//roca opt time
            _gTextRocaOPTTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_MID_INFO_ROCA_OPT_TIME_TEXT_ID);
            TEXT_SetFont(_gTextRocaOPTTimeHandle, _gRocaOPTTimeTextPara.p_text_font);
            TEXT_SetText(_gTextRocaOPTTimeHandle, (const char *)_gRocaOPTTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextRocaOPTTimeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextRocaOPTTimeHandle, _gRocaOPTTimeTextPara.text_color); 
			//on grid
            _gTextOnGirdHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_MID_INFO_ON_GRID_TEXT_ID);
            TEXT_SetFont(_gTextOnGirdHandle, _gOnGirdTextPara.p_text_font);
            TEXT_SetText(_gTextOnGirdHandle, (const char *)_gOnGirdTextPara.p_content);
            TEXT_SetTextAlign(_gTextOnGirdHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextOnGirdHandle, _gOnGirdTextPara.text_color); 
			//on grid time
            _gTextOnGirdTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_MID_INFO_ON_GRID_TIME_TEXT_ID);
            TEXT_SetFont(_gTextOnGirdTimeHandle, _gOnGirdTimeTextPara.p_text_font);
            TEXT_SetText(_gTextOnGirdTimeHandle, (const char *)_gOnGirdTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextOnGirdTimeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextOnGirdTimeHandle, _gOnGirdTimeTextPara.text_color); 
				
            break;
        }
       
        case WM_PAINT:
        {
            GUI_SetColor(0x00424242);//上色
            GUI_FillRect(12, 0,12+456,62);
#if DEBUG_TEST
			SetRocaOPTTime(211);
			SetOnGridTime(256);
#endif		
            break;
        }
        case WM_NOTIFY_PARENT:/*基于该控件窗口的子控件触发时，如虚拟按键*/
        {
            break;
        }
        default:
        {
            WM_DefaultProc(pMsg);
            break;
        }
    }
}

/**
  * @brief Bottom Win callback
  * @param  NULL
  * @retval NULL
  */
static void _cbBottomSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;


    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:   
        {
            /*初始化各个子控件*/
			//电池图标
			_gImgBatteryHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_BOTTOM_INFO_BATTERY_IMG_ID);
			IMAGE_SetDTA(_gImgBatteryHandle, _gBatteryImgPara.p_img_buf, _gBatteryImgPara.img_size); 
			
			//battery cycles
            _gTextBatCyclesHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_BOTTOM_INFO_BAT_CYC_TEXT_ID);
            TEXT_SetFont(_gTextBatCyclesHandle, _gBatCyclesTextPara.p_text_font);
            TEXT_SetText(_gTextBatCyclesHandle, (const char *)_gBatCyclesTextPara.p_content);
            TEXT_SetTextAlign(_gTextBatCyclesHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBatCyclesHandle, _gBatCyclesTextPara.text_color); 
			//battery cycles nums
            _gTextBatCyclesNumsHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_BOTTOM_INFO_BAT_CYC_NUMS_TEXT_ID);
            TEXT_SetFont(_gTextBatCyclesNumsHandle, _gBatCyclesNumsTextPara.p_text_font);
            TEXT_SetText(_gTextBatCyclesNumsHandle, (const char *)_gBatCyclesNumsTextPara.p_content);
            TEXT_SetTextAlign(_gTextBatCyclesNumsHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBatCyclesNumsHandle, _gBatCyclesNumsTextPara.text_color); 
			//SOH	
            _gTextSOHHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_BOTTOM_INFO_SOH_TEXT_ID);
            TEXT_SetFont(_gTextSOHHandle, _gSOHTextPara.p_text_font);
            TEXT_SetText(_gTextSOHHandle, (const char *)_gSOHTextPara.p_content);
            TEXT_SetTextAlign(_gTextSOHHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextSOHHandle, _gSOHTextPara.text_color); 
			//SOH STAT
            _gTextSOHStatHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_BOTTOM_INFO_SOH_STAT_TEXT_ID);
            TEXT_SetFont(_gTextSOHStatHandle, _gSOHStatTextPara.p_text_font);
            TEXT_SetText(_gTextSOHStatHandle, (const char *)_gSOHStatTextPara.p_content);
            TEXT_SetTextAlign(_gTextSOHStatHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextSOHStatHandle, _gSOHStatTextPara.text_color); 
						
            break;
        }
       
        case WM_PAINT:
        {
            GUI_SetColor(0x00424242);//上色
            GUI_FillRect(12, 0,12+456,62);
#if DEBUG_TEST
			SetBatteryCycNum(18);
			SetSOHState(99);
#endif		
            break;
        }
        case WM_NOTIFY_PARENT:/*基于该控件窗口的子控件触发时，如虚拟按键*/
        {
            break;
        }
        default:
        {
            WM_DefaultProc(pMsg);
            break;
        }
    }
}
/**
  * @brief Root Win callback
  * @param  NULL
  * @retval NULL
  */
static void _cbMainWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld;
    switch (pMsg->MsgId) 
    {
        case WM_CREATE:
        {
            break;
        }
        case WM_DELETE:
        {
            break;
        }
        case WM_PAINT:
        {
			GUI_SetColor(0x00353535);
            GUI_FillRect(0,0,EM_XSIZE,EM_YSIZE);              
            break;
        }
        default:
        {
            WM_DefaultProc(pMsg);
            break;
        }
    }
}



/**
  * @brief Create stat bar sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateElecInfoStatSubWin()
{
    _gWinElecInfoHandle = GUI_CreateDialogBox(_aElecInfoStatDialogCreate, 
											  GUI_COUNTOF(_aElecInfoStatDialogCreate), 
											  _cbElecInfoTopStatSubWin, 
											  pCurrentEmWin->win_handle, 
											  0, 0);
											  //EM_YSIZE - _aStateBarDialogCreate[0].ySize);
    WM_SetTransState(_gWinElecInfoHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinElecInfoHandle;
}

/**
  * @brief Create middle sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateMiddleSubWin()
{
    _gWinElecMidInfoHandle = GUI_CreateDialogBox(_aMiddleTextInfoDialogCreate, 
											  GUI_COUNTOF(_aMiddleTextInfoDialogCreate), 
											  _cbMiddleSubWin, 
											  pCurrentEmWin->win_handle, 
											  0, 0);
											  //EM_YSIZE - _aStateBarDialogCreate[0].ySize);
    WM_SetTransState(_gWinElecMidInfoHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinElecMidInfoHandle;
}



/**
  * @brief Create bottom sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateBottomSubWin()
{
    _gWinElecBottomInfoHandle = GUI_CreateDialogBox(_aBottomTextInfoDialogCreate, 
											  GUI_COUNTOF(_aBottomTextInfoDialogCreate), 
											  _cbBottomSubWin, 
											  pCurrentEmWin->win_handle, 
											  0, 0);
											  //EM_YSIZE - _aStateBarDialogCreate[0].ySize);
    WM_SetTransState(_gWinElecBottomInfoHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinElecBottomInfoHandle;
}


  
/**
  * @brief Create current win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWin(void)
{
    _Init();
    GUI_EnableAlpha(1);
    /*创建一个基于桌面窗口的子窗口*/
    gEmWinElectricityInfo.win_handle = WM_CreateWindow(0, 0, EM_XSIZE, EM_YSIZE, WM_CF_SHOW, _cbMainWin, 0);
    pCurrentEmWin = &gEmWinElectricityInfo;
    GUI_Exec();
	
	_CreateElecInfoStatSubWin();
	_CreateMiddleSubWin();
	_CreateBottomSubWin();

    return gEmWinElectricityInfo.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinElectricityInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinElectricityInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinElectricityInfo.win_handle = 0;
}



/**
  * @brief  顶部状态栏初始化
  * @param  NULL
  * @retval NULL
  */
static void _InitElecInfoTopStat()
{
	/*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为StatBar区域子控件申请内存*/
	_gElecGridChargeImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_ICON_GRID_CHARGE_SIZE);
    _gElecGridChargeImgPara.p_img_buf = GUI_ALLOC_h2p(_gElecGridChargeImgPara.mem_img);
	_gElecGridChargeImgPara.img_size = EM_ELEC_INFO_ICON_GRID_CHARGE_SIZE;
    SpiFlashReadData(EM_ICON_GRID_CHARGE_ADDR, _gElecGridChargeImgPara.p_img_buf, _gElecGridChargeImgPara.img_size);

    _gElecGridTiedImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_ICON_GRID_TIED_SIZE);
    _gElecGridTiedImgPara.p_img_buf = GUI_ALLOC_h2p(_gElecGridTiedImgPara.mem_img);
    _gElecGridTiedImgPara.img_size = EM_ELEC_INFO_ICON_GRID_TIED_SIZE;
    SpiFlashReadData(EM_ICON_GRID_TIED_ADDR, _gElecGridTiedImgPara.p_img_buf, _gElecGridTiedImgPara.img_size);

	_gElecPVChargeImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_ICON_PV_CHARGE_SIZE);
    _gElecPVChargeImgPara.p_img_buf = GUI_ALLOC_h2p(_gElecPVChargeImgPara.mem_img);
    _gElecPVChargeImgPara.img_size = EM_ELEC_INFO_ICON_PV_CHARGE_SIZE;
    SpiFlashReadData(EM_ICON_PV_CHARGE_ADDR, _gElecPVChargeImgPara.p_img_buf, _gElecPVChargeImgPara.img_size);
	
	_gElecLoadImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_ICON_LOAD_SIZE);
    _gElecLoadImgPara.p_img_buf = GUI_ALLOC_h2p(_gElecLoadImgPara.mem_img);
    _gElecLoadImgPara.img_size = EM_ELEC_INFO_ICON_LOAD_SIZE;
    SpiFlashReadData(EN_ICON_BACKUP_LOAD_ADDR, _gElecLoadImgPara.p_img_buf, _gElecLoadImgPara.img_size);
	

    _gkWhTextPara.p_content = _gkWhTextBuf;
    _gkWhTextPara.text_color = 0x00B2BE00;
    _gkWhTextPara.p_text_font = EM_TEXT_FONT_7;

    _gTodayTextPara.p_content = _gTodayTextBuf;
    _gTodayTextPara.text_color = 0x00FFFFFF;
    _gTodayTextPara.p_text_font = EM_TEXT_FONT_7;

    _gNovTextPara.p_content = _gNovTextBuf;
    _gNovTextPara.text_color = 0x00FFFFFF;
    _gNovTextPara.p_text_font = EM_TEXT_FONT_7;

    _gYearTextPara.p_content = _gYearTextBuf;
    _gYearTextPara.text_color = 0x00FFFFFF;
    _gYearTextPara.p_text_font = EM_TEXT_FONT_7;

    _gTotalTextPara.p_content = _gTotalTextBuf;
    _gTotalTextPara.text_color = 0x00FFFFFF;
    _gTotalTextPara.p_text_font = EM_TEXT_FONT_7;

	/*GRID CHARGE TODAY*/
	_gElecGridChargeTodayTextPara.p_content = _gGridChargeToadyTextBuf;
    _gElecGridChargeTodayTextPara.text_color = 0x00FFFFFF;
    _gElecGridChargeTodayTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecGridChargeTodayTextPara.digit_value = 3.25;//测试数据，实际赋值系统数据
    sprintf(_gElecGridChargeTodayTextPara.p_content, "%.2f", _gElecGridChargeTodayTextPara.digit_value);
	/*GRID CHARGE NOV*/
	_gElecGridChargeNovTextPara.p_content = _gGridChargeNovTextBuf;
    _gElecGridChargeNovTextPara.text_color = 0x00FFFFFF;
    _gElecGridChargeNovTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecGridChargeNovTextPara.digit_value = 30.25;//测试数据，实际赋值系统数据
    sprintf(_gElecGridChargeNovTextPara.p_content, "%.2f", _gElecGridChargeNovTextPara.digit_value);
	/*GRID CHARGE YEAR*/
	_gElecGridChargeYearTextPara.p_content = _gGridChargeYearTextBuf;
    _gElecGridChargeYearTextPara.text_color = 0x00FFFFFF;
    _gElecGridChargeYearTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecGridChargeYearTextPara.digit_value = 300.25;//测试数据，实际赋值系统数据
    sprintf(_gElecGridChargeYearTextPara.p_content, "%.2f", _gElecGridChargeYearTextPara.digit_value);
	/*GRID CHARGE TOTAL*/
	_gElecGridChargeTotalTextPara.p_content = _gGridChargeTotalTextBuf;
    _gElecGridChargeTotalTextPara.text_color = 0x00FFFFFF;
    _gElecGridChargeTotalTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecGridChargeTotalTextPara.digit_value = 3000.25;//测试数据，实际赋值系统数据
    sprintf(_gElecGridChargeTotalTextPara.p_content, "%.2f", _gElecGridChargeTotalTextPara.digit_value);
	/*GRID TIED TODAY*/
	_gElecGridTiedTodayTextPara.p_content = _gGridTiedTodayTextBuf;
    _gElecGridTiedTodayTextPara.text_color = 0x00FFFFFF;
    _gElecGridTiedTodayTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecGridTiedTodayTextPara.digit_value = 4.2;//测试数据，实际赋值系统数据
    sprintf(_gElecGridTiedTodayTextPara.p_content, "%.2f", _gElecGridTiedTodayTextPara.digit_value);
	/*GRID TIED NOV*/
	_gElecGridTiedNovTextPara.p_content = _gGridTiedNovTextBuf;
    _gElecGridTiedNovTextPara.text_color = 0x00FFFFFF;
    _gElecGridTiedNovTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecGridTiedNovTextPara.digit_value = 30.25;//测试数据，实际赋值系统数据
    sprintf(_gElecGridTiedNovTextPara.p_content, "%.2f", _gElecGridTiedNovTextPara.digit_value);
	/*GRID TIED YEAR*/
	_gElecGridTiedYearTextPara.p_content = _gGridTiedYearTextBuf;
    _gElecGridTiedYearTextPara.text_color = 0x00FFFFFF;
    _gElecGridTiedYearTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecGridTiedYearTextPara.digit_value = 300.25;//测试数据，实际赋值系统数据
    sprintf(_gElecGridTiedYearTextPara.p_content, "%.2f", _gElecGridTiedYearTextPara.digit_value);
	/*GRID TIED TOTAL*/
	_gElecGridTiedTotalTextPara.p_content = _gGridTiedTotalTextBuf;
    _gElecGridTiedTotalTextPara.text_color = 0x00FFFFFF;
    _gElecGridTiedTotalTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecGridTiedTotalTextPara.digit_value = 3000.25;//测试数据，实际赋值系统数据
    sprintf(_gElecGridTiedTotalTextPara.p_content, "%.2f", _gElecGridTiedTotalTextPara.digit_value);
	
	/*PV CHARGE TODAY*/
	_gElecPVChargeTodayTextPara.p_content = _gPVchargeTodayTextBuf;
    _gElecPVChargeTodayTextPara.text_color = 0x00FFFFFF;
    _gElecPVChargeTodayTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecPVChargeTodayTextPara.digit_value = 3.25;//测试数据，实际赋值系统数据
    sprintf(_gElecPVChargeTodayTextPara.p_content, "%.2f", _gElecPVChargeTodayTextPara.digit_value);
	/*PV CHARGE NOV*/
	_gElecPVChargeNovTextPara.p_content = _gPVchargeNovTextBuf;
    _gElecPVChargeNovTextPara.text_color = 0x00FFFFFF;
    _gElecPVChargeNovTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecPVChargeNovTextPara.digit_value = 30.25;//测试数据，实际赋值系统数据
    sprintf(_gElecPVChargeNovTextPara.p_content, "%.2f", _gElecPVChargeNovTextPara.digit_value);
	/*PV CHARGE YEAR*/
	_gElecPVChargeYearTextPara.p_content = _gPVchargeYearTextBuf;
    _gElecPVChargeYearTextPara.text_color = 0x00FFFFFF;
    _gElecPVChargeYearTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecPVChargeYearTextPara.digit_value = 300.25;//测试数据，实际赋值系统数据
    sprintf(_gElecPVChargeYearTextPara.p_content, "%.2f", _gElecPVChargeYearTextPara.digit_value);
	/*PV CHARGE TOTAL*/
	_gElecPVChargeTotalTextPara.p_content = _gPVchargeTotalTextBuf;
    _gElecPVChargeTotalTextPara.text_color = 0x00FFFFFF;
    _gElecPVChargeTotalTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecPVChargeTotalTextPara.digit_value = 3000.25;//测试数据，实际赋值系统数据
    sprintf(_gElecPVChargeTotalTextPara.p_content, "%.2f", _gElecPVChargeTotalTextPara.digit_value);
	/*LOAD TODAY*/
	_gElecLoadTodayTextPara.p_content = _gLoadTodayTextBuf;
    _gElecLoadTodayTextPara.text_color = 0x00FFFFFF;
    _gElecLoadTodayTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecLoadTodayTextPara.digit_value = 3.25;//测试数据，实际赋值系统数据
    sprintf(_gElecLoadTodayTextPara.p_content, "%.2f", _gElecLoadTodayTextPara.digit_value);
	/*LOAD NOV*/
	_gElecLoadNovTextPara.p_content = _gLoadNovTextBuf;
    _gElecLoadNovTextPara.text_color = 0x00FFFFFF;
    _gElecLoadNovTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecLoadNovTextPara.digit_value = 30.25;//测试数据，实际赋值系统数据
    sprintf(_gElecLoadNovTextPara.p_content, "%.2f", _gElecLoadNovTextPara.digit_value);
	/*LOAD YEAR*/
	_gElecLoadYearTextPara.p_content = _gLoadYearTextBuf;
    _gElecLoadYearTextPara.text_color = 0x00FFFFFF;
    _gElecLoadYearTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecLoadYearTextPara.digit_value = 300.25;//测试数据，实际赋值系统数据
    sprintf(_gElecLoadYearTextPara.p_content, "%.2f", _gElecLoadYearTextPara.digit_value);
	/*LOAD TOTAL*/
	_gElecLoadTotalTextPara.p_content = _gLoadTotalTextBuf;
    _gElecLoadTotalTextPara.text_color = 0x00FFFFFF;
    _gElecLoadTotalTextPara.p_text_font = EM_TEXT_FONT_5;
	_gElecLoadTotalTextPara.digit_value = 3000.25;//测试数据，实际赋值系统数据
    sprintf(_gElecLoadTotalTextPara.p_content, "%.2f", _gElecLoadTotalTextPara.digit_value);

}


/**
  * @brief  中间子窗口初始化
  * @param  NULL
  * @retval NULL
  */
static void _InitElecInfoMid()
{
	/*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/

	_gRocaOptTimeImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_MID_TIME_ICON_SIZE);
	_gRocaOptTimeImgPara.p_img_buf = GUI_ALLOC_h2p(_gRocaOptTimeImgPara.mem_img);
	_gRocaOptTimeImgPara.img_size = EM_ELEC_INFO_MID_TIME_ICON_SIZE;
	SpiFlashReadData(EM_ICON_ELEC_TIME_ADDR, _gRocaOptTimeImgPara.p_img_buf, _gRocaOptTimeImgPara.img_size);		 					 
					 	
	_gRocaOPTTextPara.p_content = _gRocaOPTTextBuf;
	_gRocaOPTTextPara.text_color = 0x00FFFFFF;
    _gRocaOPTTextPara.p_text_font = EM_TEXT_FONT_7;
	snprintf(_gRocaOPTTextBuf, sizeof(_gRocaOPTTextBuf), "%s", "Roca OPT Time");				 

    _gRocaOPTTimeTextPara.p_content =	_gRocaOPTTimeTextBuf;
    _gRocaOPTTimeTextPara.text_color = 0x00B2BE00;
    _gRocaOPTTimeTextPara.p_text_font = EM_TEXT_FONT_5;
	_gRocaOPTTimeTextPara.digit_value = 300;//测试数据，实际赋值系统数据
    sprintf(_gRocaOPTTimeTextPara.p_content, "%dh", (uint16_t)_gRocaOPTTimeTextPara.digit_value);
	
	
	_gOnGirdTextPara.p_content = _gOnGirdTextBuf;
	_gOnGirdTextPara.text_color = 0x00FFFFFF;
    _gOnGirdTextPara.p_text_font = EM_TEXT_FONT_7;
	snprintf(_gOnGirdTextBuf, sizeof(_gOnGirdTextBuf), "%s", "On Grid Time");				 

    _gOnGirdTimeTextPara.p_content =	_gOnGirdTimeTextBuf;
    _gOnGirdTimeTextPara.text_color = 0x00B2BE00;
    _gOnGirdTimeTextPara.p_text_font = EM_TEXT_FONT_5;
	_gOnGirdTimeTextPara.digit_value = 120;//测试数据，实际赋值系统数据
    sprintf(_gOnGirdTimeTextPara.p_content, "%dh", (uint16_t)_gOnGirdTimeTextPara.digit_value);
	 
}


/**
  * @brief  底部子窗口初始化
  * @param  NULL
  * @retval NULL
  */
static void _InitElecInfoBottom()
{
	/*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/

	_gBatteryImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_BOTTOM_BATTERY_ICON_SIZE);
	_gBatteryImgPara.p_img_buf = GUI_ALLOC_h2p(_gBatteryImgPara.mem_img);
	_gBatteryImgPara.img_size = EM_ELEC_INFO_BOTTOM_BATTERY_ICON_SIZE;
	SpiFlashReadData(EM_ICON_ELEC_BATTERY_ADDR, _gBatteryImgPara.p_img_buf, _gBatteryImgPara.img_size);		 					 
					 	
	_gBatCyclesTextPara.p_content = _gBatCyclesTextBuf;
	_gBatCyclesTextPara.text_color = 0x00FFFFFF;
    _gBatCyclesTextPara.p_text_font = EM_TEXT_FONT_7;
	snprintf(_gBatCyclesTextBuf, sizeof(_gBatCyclesTextBuf), "%s", "Battery cycles");				 

    _gBatCyclesNumsTextPara.p_content =	_gBatCyclesNumsTextBuf;
    _gBatCyclesNumsTextPara.text_color = 0x00B2BE00;
    _gBatCyclesNumsTextPara.p_text_font = EM_TEXT_FONT_5;
	_gBatCyclesNumsTextPara.digit_value = 23;//测试数据，实际赋值系统数据
    sprintf(_gBatCyclesNumsTextPara.p_content, "%d", (uint16_t)_gBatCyclesNumsTextPara.digit_value);
	
	
	_gSOHTextPara.p_content = _gSOHTextBuf;
	_gSOHTextPara.text_color = 0x00FFFFFF;
    _gSOHTextPara.p_text_font = EM_TEXT_FONT_7;
	snprintf(_gSOHTextBuf, sizeof(_gSOHTextBuf), "%s", "State of health");				 

    _gSOHStatTextPara.p_content = _gSOHStatTextBuf;
    _gSOHStatTextPara.text_color = 0x00B2BE00;
    _gSOHStatTextPara.p_text_font = EM_TEXT_FONT_5;
	_gSOHStatTextPara.digit_value = 100;//测试数据，实际赋值系统数据
    sprintf(_gSOHStatTextPara.p_content, "%d%%", (uint16_t)_gSOHStatTextPara.digit_value);
	 	
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为背景图申请内存*/

	_InitElecInfoTopStat();
	_InitElecInfoMid();
	_InitElecInfoBottom();
}


static void _DelElecInfoTopStat()
{
    GUI_ALLOC_Free(_gElecGridChargeImgPara.mem_img);
    GUI_ALLOC_Free(_gElecGridTiedImgPara.mem_img);
    GUI_ALLOC_Free(_gElecPVChargeImgPara.mem_img);
    GUI_ALLOC_Free(_gElecLoadImgPara.mem_img);
}

static void _DelMiddle()
{
	GUI_ALLOC_Free(_gRocaOptTimeImgPara.mem_img);
}

static void _DelBottom()
{
	GUI_ALLOC_Free(_gBatteryImgPara.mem_img);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
	_DelElecInfoTopStat();
	_DelMiddle();
	_DelBottom();
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{

}

/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
    int32_t event = EmGetKeyEvent(p_msg);
	if(event < 0 || event > EN_KEY_SRC_NUM)
	{
		return;
	}
	
	if(event == EN_KEY_SRC_ESC)
	{
		if(pCurrentEmWin->win_handle != 0)
		{
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pPrevEmWin;
			pCurrentEmWin->create_win();
		}
		return;
	}
}

//电网发电统计
int32_t SetGridChargeInfo(GRID_CHARGE_T Grid_Charge)
{
	//today
	_gElecGridChargeTodayTextPara.digit_value = Grid_Charge.elec_today;
	sprintf(_gElecGridChargeTodayTextPara.p_content, "%.2f", _gElecGridChargeTodayTextPara.digit_value);
	TEXT_SetText(_gTextGridChargeToadyHandle, (const char *)_gElecGridChargeTodayTextPara.p_content);
	//nov
	_gElecGridChargeNovTextPara.digit_value = Grid_Charge.elec_nov;
	sprintf(_gElecGridChargeNovTextPara.p_content, "%.2f", _gElecGridChargeNovTextPara.digit_value);
	TEXT_SetText(_gTextGridChargeNovHandle, (const char *)_gElecGridChargeNovTextPara.p_content);
	//year
	_gElecGridChargeYearTextPara.digit_value = Grid_Charge.elec_year;
	sprintf(_gElecGridChargeYearTextPara.p_content, "%.2f", _gElecGridChargeYearTextPara.digit_value);
	TEXT_SetText(_gTextGridChargeYearHandle, (const char *)_gElecGridChargeYearTextPara.p_content);
	//total
	_gElecGridChargeTotalTextPara.digit_value = Grid_Charge.elec_total;
	sprintf(_gElecGridChargeTotalTextPara.p_content, "%.2f", _gElecGridChargeTotalTextPara.digit_value);
	TEXT_SetText(_gTextGridChargeTotalHandle, (const char *)_gElecGridChargeTotalTextPara.p_content);
	
	return 0;
}
//向电网馈电统计
int32_t SetGridTiedInfo(GRID_TIED_T Grid_Tied)
{
	//today
	_gElecGridTiedTodayTextPara.digit_value = Grid_Tied.elec_today;
	sprintf(_gElecGridTiedTodayTextPara.p_content, "%.2f", _gElecGridTiedTodayTextPara.digit_value);
	TEXT_SetText(_gTextGridTiedTodayHandle, (const char *)_gElecGridTiedTodayTextPara.p_content);
	//nov
	_gElecGridTiedNovTextPara.digit_value = Grid_Tied.elec_nov;
	sprintf(_gElecGridTiedNovTextPara.p_content, "%.2f", _gElecGridTiedNovTextPara.digit_value);
	TEXT_SetText(_gTextGridChargeNovHandle, (const char *)_gElecGridChargeNovTextPara.p_content);
	//year
	_gElecGridTiedYearTextPara.digit_value = Grid_Tied.elec_year;
	sprintf(_gElecGridTiedYearTextPara.p_content, "%.2f", _gElecGridTiedYearTextPara.digit_value);
	TEXT_SetText(_gTextGridTiedYearHandle, (const char *)_gElecGridTiedYearTextPara.p_content);
	//total
	_gElecGridTiedTotalTextPara.digit_value = Grid_Tied.elec_total;
	sprintf(_gElecGridTiedTotalTextPara.p_content, "%.2f", _gElecGridTiedTotalTextPara.digit_value);
	TEXT_SetText(_gTextGridTiedTotalHandle, (const char *)_gElecGridTiedTotalTextPara.p_content);
	
	return 0;
}
//太阳能充电统计
int32_t SetPVChargeInfo(PV_CHARGE_T PV_Charge)
{
	//today
	_gElecPVChargeTodayTextPara.digit_value = PV_Charge.elec_today;
	sprintf(_gElecPVChargeTodayTextPara.p_content, "%.2f", _gElecPVChargeTodayTextPara.digit_value);
	TEXT_SetText(_gTextPVchargeTodayHandle, (const char *)_gElecPVChargeTodayTextPara.p_content);
	//nov
	_gElecPVChargeNovTextPara.digit_value = PV_Charge.elec_nov;
	sprintf(_gElecPVChargeNovTextPara.p_content, "%.2f", _gElecPVChargeNovTextPara.digit_value);
	TEXT_SetText(_gTextPVchargeNovHandle, (const char *)_gElecPVChargeNovTextPara.p_content);
	//year
	_gElecPVChargeYearTextPara.digit_value = PV_Charge.elec_year;
	sprintf(_gElecPVChargeYearTextPara.p_content, "%.2f", _gElecPVChargeYearTextPara.digit_value);
	TEXT_SetText(_gTextPVchargeYearHandle, (const char *)_gElecPVChargeYearTextPara.p_content);
	//total
	_gElecPVChargeTotalTextPara.digit_value = PV_Charge.elec_total;
	sprintf(_gElecPVChargeTotalTextPara.p_content, "%.2f", _gElecPVChargeTotalTextPara.digit_value);
	TEXT_SetText(_gTextPVChargeTotalHandle, (const char *)_gElecPVChargeTotalTextPara.p_content);
	
	return 0;
}
//负载用电统计
int32_t SetBackupLoadInfo(BACKUP_LOAD_T Backup_Load)
{
	//today
	_gElecLoadTodayTextPara.digit_value = Backup_Load.elec_today;
	sprintf(_gElecLoadTodayTextPara.p_content, "%.2f", _gElecLoadTodayTextPara.digit_value);
	TEXT_SetText(_gTextLoadTodayHandle, (const char *)_gElecLoadTodayTextPara.p_content);
	//nov
	_gElecLoadNovTextPara.digit_value = Backup_Load.elec_nov;
	sprintf(_gElecLoadNovTextPara.p_content, "%.2f", _gElecLoadNovTextPara.digit_value);
	TEXT_SetText(_gTextLoadNovHandle, (const char *)_gElecLoadNovTextPara.p_content);
	//year
	_gElecLoadYearTextPara.digit_value = Backup_Load.elec_year;
	sprintf(_gElecLoadYearTextPara.p_content, "%.2f", _gElecLoadYearTextPara.digit_value);
	TEXT_SetText(_gTextLoadYearHandle, (const char *)_gElecLoadYearTextPara.p_content);
	//total
	_gElecLoadTotalTextPara.digit_value = Backup_Load.elec_total;
	sprintf(_gElecLoadTotalTextPara.p_content, "%.2f", _gElecLoadTotalTextPara.digit_value);
	TEXT_SetText(_gTextLoadTotalHandle, (const char *)_gElecLoadTotalTextPara.p_content);
	
	return 0;
}
//设置Roca OPT Time
int32_t SetRocaOPTTime(int32_t c_time)
{
	_gRocaOPTTimeTextPara.digit_value = c_time;
	sprintf(_gRocaOPTTimeTextPara.p_content, "%d", (uint16_t)_gRocaOPTTimeTextPara.digit_value);
	TEXT_SetText(_gTextRocaOPTTimeHandle, (const char *)_gRocaOPTTimeTextPara.p_content);
	return 0;
}
//设置On Grid Time
int32_t SetOnGridTime(int32_t c_time)
{
	_gOnGirdTimeTextPara.digit_value = c_time;
	sprintf(_gOnGirdTimeTextPara.p_content, "%d", (uint16_t)_gOnGirdTimeTextPara.digit_value);
	TEXT_SetText(_gTextOnGirdTimeHandle, (const char *)_gOnGirdTimeTextPara.p_content);
	return 0;
}
//设置电池循环次数
int32_t SetBatteryCycNum(int32_t c_nums)
{
	_gBatCyclesNumsTextPara.digit_value = c_nums;
	sprintf(_gBatCyclesNumsTextPara.p_content, "%d", (uint16_t)_gBatCyclesNumsTextPara.digit_value);
	TEXT_SetText(_gTextBatCyclesNumsHandle, (const char *)_gBatCyclesNumsTextPara.p_content);
	return 0;
}
//设置SOH
int32_t SetSOHState(int32_t c_precentage)
{
	_gSOHStatTextPara.digit_value = c_precentage;
	sprintf(_gSOHStatTextPara.p_content, "%d", (uint16_t)_gSOHStatTextPara.digit_value);
	TEXT_SetText(_gTextSOHStatHandle, (const char *)_gSOHStatTextPara.p_content);
	return 0;
}

