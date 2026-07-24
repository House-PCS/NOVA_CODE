/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-6-1    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "sox.h"
#include "rtc.h"
#include "math.h"

#define LOG_TAG     "sox"
#define PACK_RATED_CAPACITY                 314000/*mAh*/
#define PACK_RATED_CAPACITY_WH              8038/*Wh*/
#define PACK_CURRENT_MIN_THRESHOLD          5000/*mA*/
#define PACK_ACT_MAX_SOC_THRESHOLD          100
#define PACK_ACT_MIN_SOC_THRESHOLD          5


#define SOX_FLASH_INFO_ADDR                 0x00101000
#define SOX_FLASH_BACKUP_INFO_ADDR          0x00103000

#define SOX_EEPROM_INFO_ADDR                                  0x00000400
#define SOX_EEPROM_BACKUP_INFO_ADDR                           0x00000600

#define SOX_FIRST_FIX_ADDR                                    0x00000800
#define CURRENT_CALIBRATION_FLASH_ADDR                        0x00109000
#define CURRENT_CALIBRATION_BACKUP_FLASH_ADDR                 0x0010B000
#define REST_FACTORY_INFO_ADDR                                0x00100000

#define OCV_CALIBRATION_TIME                7200
#define CHARGE_MAX_VOLTAGE                  3400
#define DISCHARGE_MIN_VOLTAGE               2700
#define SPEED_UP_4                          (10)
#define SPEED_UP_3                          (5)
#define SPEED_UP_2                          (2)
#define SPEED_UP_1                          (1.5f)
#define SPEED_UP_0                          (1.1f)
#define SPEED_CUT_0                         (0.9f)
#define SPEED_CUT_1                         (0.7f)
#define SPEED_CUT_2                         (0.5f)
#define SPEED_CUT_3                         (0.2f)
#define SPEED_CUT_4                         (0.1f)

uint8_t _SoxInitFlag = 0;;
uint8_t grechargeflag = 0;
uint8_t sox_init_flag = 0;
uint32_t max_charge_current = 0;
uint8_t initcurrflag = 0;

PackStatus_t    gPackStatus;
PackStatus_t    gPackStatusBackUp;
PackStatus_t    gPackStatusBase;
PackStatus_t    gPackStatusEEpromBackUp;
PackStatus_t    gPackStatusEEpromBase;
PackStatus_t    gPackStatus_debug;
LinearCurrentController_t gCurrentController;
static PackStatus_t    sgSoxFlashInfo;
static uint8_t _gTrickleChargeFlag = 0;
static uint32_t dispsoctimedely = 0;

SoxDebugParam_t gSoxDebugParam =
{
    .cell_current_debug = 100,
};

/*OCV,单位mV*/
#define OCV_SOC_TEMPERATURE_LEVEL       6
#define OCV_SOC_LEVEL                   21
const int8_t  OCVTemperatureIndex[OCV_SOC_TEMPERATURE_LEVEL] = {0, 15, 25, 35, 45, 55};
const uint8_t OCVSOCIndex[OCV_SOC_LEVEL] = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
const uint16_t OCVSOCDefault[OCV_SOC_TEMPERATURE_LEVEL][OCV_SOC_LEVEL] =
{
    {2796, 3190, 3223, 3246, 3276, 3296, 3301, 3301, 3302, 3303, 3304, 3306, 3310, 3337, 3341, 3341, 3341, 3341, 3340, 3339, 3400},
    {2843, 3195, 3222, 3246, 3275, 3295, 3300, 3301, 3302, 3303, 3304, 3306, 3309, 3338, 3340, 3339, 3339, 3339, 3339, 3338, 3447},
    {2780, 3181, 3221, 3243, 3273, 3294, 3301, 3302, 3303, 3304, 3306, 3307, 3310, 3338, 3339, 3339, 3339, 3339, 3339, 3338, 3495},
    {2842, 3189, 3220, 3244, 3272, 3292, 3302, 3303, 3304, 3305, 3307, 3309, 3312, 3338, 3338, 3338, 3338, 3338, 3338, 3339, 3503},
    {2802, 3179, 3220, 3242, 3271, 3291, 3302, 3305, 3306, 3307, 3309, 3311, 3316, 3338, 3338, 3338, 3338, 3339, 3339, 3340, 3530},
    {2854, 3185, 3220, 3242, 3270, 3290, 3302, 3306, 3308, 3309, 3311, 3313, 3319, 3339, 3340, 3341, 3342, 3342, 3342, 3342, 3481},
};

/*放电容量，单位mAh*/
#define DSG_CAPA_TEMPERATURE_LEVEL      6
#define DSG_CAPA_MULTI_LEVEL            4
const uint32_t DsgCapacityDefault[DSG_CAPA_TEMPERATURE_LEVEL][DSG_CAPA_MULTI_LEVEL] =
{
    {289800, 289800, 289800, 289800},
    {293070, 293070, 293070, 293070},
    {295280, 295280, 295280, 295280},
    {310000, 310000, 310000, 310000},
    {322530, 322530, 322530, 322530},
    {298250, 298250, 298250, 298250},
};
const int8_t DsgCapaTemperatureIndex[DSG_CAPA_TEMPERATURE_LEVEL] = { -20, -10, 0, 10, 25, 45};
const float  DsgCapaMultiIndex[DSG_CAPA_MULTI_LEVEL] = {1 / 4, 1 / 2, 1, 2};
/*放电电流*/
#define DSG_CURR_TEMPERATURE_LEVEL      13
#define DSG_CURR_SOC_LEVEL              13
const int8_t    DsgCurrTemperatureIndex[DSG_CURR_TEMPERATURE_LEVEL] = { -30, -25, -10, 0, 5, 10, 15, 20, 25, 45, 50, 55, 60};
const uint8_t   DsgCurrSocIndex[DSG_CURR_SOC_LEVEL] = {0, 5, 10, 20, 30, 40, 50, 60, 70, 80, 90, 95, 100};

/*长时间60最大放电电流 >=55*/
const uint32_t DsgCurrent60Default[DSG_CURR_TEMPERATURE_LEVEL][DSG_CURR_SOC_LEVEL] =
{
/*SOC:0   5   10    20   30   40    50   60   70   80    90    95   100*/
    {0,   0,   0,    0,   0,   0,    0,   0,   0,   0,    0,    0,   0,},   //-30
    {0,  25,  25,   25,  25,  25,   25,  25,  25,  25,   25,   25,  25,},   //-25
    {0,  35,  35,   50,  50,  50,   50,  50,  50,  50,   50,   50,  50,},   //-10
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //0
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //5
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //10
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //10
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //20
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //25
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //45
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //50
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //55
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //60
};


/*瞬态30最大放电电流 >= 25*/
const uint32_t DsgCurrent30Default[DSG_CURR_TEMPERATURE_LEVEL][DSG_CURR_SOC_LEVEL] =
{
/*SOC:0   5   10    20   30   40    50   60   70   80    90    95   100*/
    {0,   0,   0,    0,   0,   0,    0,   0,   0,   0,    0,    0,   0,},   //-30
    {0,  25,  25,   25,  25,  25,   25,  25,  25,  25,   25,   25,  25,},   //-25
    {0,  35,  35,   50,  50,  50,   50,  50,  50,  50,   50,   50,  50,},   //-10
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //0
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //5
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //10
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //10
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //20
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //25
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //45
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //50
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //55
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //60
};

/*瞬态10最大放电电流 >= 8*/
const uint32_t DsgCurrent10Default[DSG_CURR_TEMPERATURE_LEVEL][DSG_CURR_SOC_LEVEL] =
{
/*SOC:0   5   10    20   30   40    50   60   70   80    90    95   100*/
    {0,   0,   0,    0,   0,   0,    0,   0,   0,   0,    0,    0,   0,},   //-30
    {0,  25,  25,   25,  25,  25,   25,  25,  25,  25,   25,   25,  25,},   //-25
    {0,  35,  35,   50,  50,  50,   50,  50,  50,  50,   50,   50,  50,},   //-10
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //0
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //5
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //10
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //10
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //20
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //25
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //45
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //50
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //55
    {0,  35,  35,   60,  60,  60,   60,  60,  60,  60,   60,   60,  60,},   //60
};

/*充电最大倍率*/
#define CHG_MULTI_TEMPERATURE_LEVEL  11
#define CHG_MULTI_SOC_LEVEL          12
const int8_t ChgMultiTemperatureIndex[CHG_MULTI_TEMPERATURE_LEVEL] = {0, 5, 10, 15, 20, 25, 40, 45, 50, 55, 60};
const uint8_t ChgMultiSocIndex[CHG_MULTI_SOC_LEVEL] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 95, 100};
/*100倍，实际使用计算的时候需要除以100*/
const uint8_t ChgMultiDefault[CHG_MULTI_TEMPERATURE_LEVEL][CHG_MULTI_SOC_LEVEL] =
{
    /*SOC:0    10    20   30  40    50     60   70    80    90   95    100*/
    { 0,    0,   0,   0,   0,    0,    0,   0,    0,    0,    0,    0,},       //0
    {10,   10,   10,  10,  10,   10,   10,  10,   10,   10,   10,    10,},     //5
    {30,   30,   30,  30,  30,   30,   30,  30,   30,   30,   15,   15,},      //10
    {50,   50,   50,  50,  50,   50,   50,  50,   50,   50,   25,   25,},      //15
    {50,   50,   50,  50,  50,   50,   50,  50,   50,   50,   25,   25,},      //20
    {50,   50,   50,  50,  50,   50,   50,  50,   50,   50,   25,   25,},      //25
    {50,   50,   50,  50,  50,   50,   50,  50,   50,   50,   25,   25,},      //40
    {50,   50,   50,  50,  50,   50,   50,  50,   50,   50,   25,   25,},      //45
    {50,   50,   50,  50,  50,   50,   50,  50,   50,   50,   25,   25,},      //50
    {50,   50,   50,  50,  50,   50,   50,  50,   50,   50,   25,   25,},      //55
    {50,   50,   50,  50,  50,   50,   50,  50,   50,   50,   25,   25,},      //60
};


/*循环寿命*/
#define CELL_LOOP_FREQ_LEVEL        41
const uint16_t CellLoopLifeDefault[CELL_LOOP_FREQ_LEVEL] =
{
    10350, 10080, 9940,  9830,  9730,  9630,  9550,  9470,  9390,  9320,
    9250,  9190,  9120,  9060,  9000,  8940,  8880,  8820,  8760,  8710,
    8650,  8590,  8540,  8490,  8430,  8380,  8330,  8270,  8220,  8170,
    8120,  8060,  8010,  7960,  7910,  7850,  7800,  7740,  7601,  7630, 7580,
};

const uint16_t CellLoopCyclesDefault[CELL_LOOP_FREQ_LEVEL] =
{
    1,     201,   401,   601,   801,   1001,  1201,  1401,  1601,  1801,
    2001,  2201,  2401,  2601,  2801,  3001,  3201,  3401,  3601,  3801,
    4001,  4201,  4401,  4601,  4801,  5001,  5201,  5401,  5601,  5801,
    6001,  6201,  6401,  6601,  6801,  7001,  7201,  7401,  7601,  7801,  8001,
};


void PackCurrentModeJudge(PackStatus_t *pPackStatus)
{
    int32_t current = pPackStatus->pack_current;
    static EN_PACK_CURRENT_MODE e_last_mode = EN_PACK_CURRENT_IDLE_MODE;
    if(current < PACK_CURRENT_MIN_THRESHOLD && current > (0 - PACK_CURRENT_MIN_THRESHOLD))
    {
        pPackStatus->mode = EN_PACK_CURRENT_IDLE_MODE;
    }
    else if(current > 0)
    {
        if(pPackStatus->empty_discharge_flag == 0)
        {
            pPackStatus->mode = EN_PACK_CURRENT_DISCHARGE_MODE;
        }
        else
        {
            pPackStatus->mode = EN_PACK_CURRENT_IDLE_MODE;
        }
    }
    else
    {
        pPackStatus->mode = EN_PACK_CURRENT_CHARGE_MODE;
    }

    if(e_last_mode != pPackStatus->mode)
    {
        e_last_mode = pPackStatus->mode;
        pPackStatus->ocv_time_start = RtcGetTime();
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
    if(x < x1)
    {
        y =  y1;
    }
    else if(x > x2)
    {
        y =  y2;
    }
    else
    {
        y = (float)(y2 - y1) / (x2 - x1) * (x - x1) + y1;
    }
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
    if(x < x1)
    {
        y =  y1;
    }
    else if(x > x2)
    {
        y =  y2;
    }
    else
    {
        y = (float)(y2 - y1) / (x2 - x1) * (x - x1) + y1;
    }
    return y;
}


/*计算列表区间范围索引,有符号*/
static void _GetTableRangeIndex8(int8_t value, const int8_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    // 1. 校验所有指针非空 + 索引表长度合法（至少2个元素才存在区间）
    if (p_index_table == NULL || p_index_d == NULL || p_index_u == NULL || table_level < 2)
    {
        *p_index_d = 0;
        *p_index_u = 0;
        return;
    }

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
        // 2. 覆盖极端场景：遍历未找到（表非递增），默认返回最后一个区间
        *p_index_d = table_level - 1;
        *p_index_u = table_level - 1;
    }
}

/*计算列表区间范围索引,无符号*/
static void _GetTableRangeIndex8u(uint8_t value, const uint8_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    // 1. 校验所有指针非空 + 索引表长度合法（至少2个元素才存在区间）
    if (p_index_table == NULL || p_index_d == NULL || p_index_u == NULL || table_level < 2)
    {
        *p_index_d = 0;
        *p_index_u = 0;
        return;
    }
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
        // 2. 覆盖极端场景：遍历未找到（表非递增），默认返回最后一个区间
        *p_index_d = table_level - 1;
        *p_index_u = table_level - 1;
    }
}

static void _GetTableRangeIndex16(int16_t value, const int16_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    // 1. 校验所有指针非空 + 索引表长度合法（至少2个元素才存在区间）
    if (p_index_table == NULL || p_index_d == NULL || p_index_u == NULL || table_level < 2)
    {
        *p_index_d = 0;
        *p_index_u = 0;
        return;
    }
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
        // 2. 覆盖极端场景：遍历未找到（表非递增），默认返回最后一个区间
        *p_index_d = table_level - 1;
        *p_index_u = table_level - 1;
    }
}

static void _GetTableRangeIndex16u(uint16_t value, const uint16_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    // 1. 校验所有指针非空 + 索引表长度合法（至少2个元素才存在区间）
    if (p_index_table == NULL || p_index_d == NULL || p_index_u == NULL || table_level < 2)
    {
        *p_index_d = 0;
        *p_index_u = 0;
        return;
    }
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
            // 2. 覆盖极端场景：遍历未找到（表非递增），默认返回最后一个区间
            *p_index_d = table_level - 1;
            *p_index_u = table_level - 1;
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
            // 2. 覆盖极端场景：遍历未找到（表非递增），默认返回最后一个区间
            *p_index_d = table_level - 1;
            *p_index_u = table_level - 1;
        }
    }
}

static void _GetTableRangeIndex32(int32_t value, const int32_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    // 1. 校验所有指针非空 + 索引表长度合法（至少2个元素才存在区间）
    if (p_index_table == NULL || p_index_d == NULL || p_index_u == NULL || table_level < 2)
    {
        *p_index_d = 0;
        *p_index_u = 0;
        return;
    }
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
        // 2. 覆盖极端场景：遍历未找到（表非递增），默认返回最后一个区间
        *p_index_d = table_level - 1;
        *p_index_u = table_level - 1;
    }
}

static void _GetTableRangeIndex32u(uint32_t value, const uint32_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    // 1. 校验所有指针非空 + 索引表长度合法（至少2个元素才存在区间）
    if (p_index_table == NULL || p_index_d == NULL || p_index_u == NULL || table_level < 2)
    {
        *p_index_d = 0;
        *p_index_u = 0;
        return;
    }
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
        // 2. 覆盖极端场景：遍历未找到（表非递增），默认返回最后一个区间
        *p_index_d = table_level - 1;
        *p_index_u = table_level - 1;
    }
}

static void _GetTableRangeIndexf(float value, const float *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    // 1. 校验所有指针非空 + 索引表长度合法（至少2个元素才存在区间）
    if (p_index_table == NULL || p_index_d == NULL || p_index_u == NULL || table_level < 2)
    {
        *p_index_d = 0;
        *p_index_u = 0;
        return;
    }
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
        // 2. 覆盖极端场景：遍历未找到（表非递增），默认返回最后一个区间
        *p_index_d = table_level - 1;
        *p_index_u = table_level - 1;
    }
}

static void _GetTableRangeIndexf8u(float value, const uint8_t *p_index_table, uint8_t table_level, uint8_t *p_index_d, uint8_t *p_index_u)
{
    // 1. 校验所有指针非空 + 索引表长度合法（至少2个元素才存在区间）
    if (p_index_table == NULL || p_index_d == NULL || p_index_u == NULL || table_level < 2)
    {
        *p_index_d = 0;
        *p_index_u = 0;
        return;
    }

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
        // 2. 覆盖极端场景：遍历未找到（表非递增），默认返回最后一个区间
        *p_index_d = table_level - 1;
        *p_index_u = table_level - 1;
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
    temp_u = OCVTemperatureIndex[temp_index_u];//求温度区间


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

    soc_oi = _InterpolationOperationf(temp_d, temp_u, soc_tempd_oi, soc_tempu_oi, temperature);//温度高低   soc高低
    return soc_oi;
}


/*OCV校准，在静置状态下(EN_PACK_CURRENT_IDLE_MODE)持续一定时间后调用*/
void OCVCalibraiton(PackStatus_t *pPackStatus)
{
    uint16_t min_voltage = gstCellInfo.cell_min_voltage;

    pPackStatus->sox_info.soc_real = _OCV2SOCWithInterpolation(min_voltage, pPackStatus);
    if(abs((int)(pPackStatus->sox_info.soc_actl - pPackStatus->sox_info.soc_real)) > 20)
    {
        pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_real;
    }

    if(pPackStatus->sox_info.soc_real > pPackStatus->sox_info.soc_max)
    {
        pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max;
        pPackStatus->full_charge_flag = 1;
    }
    if(abs((int)((pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_min) * 10)) < 5)
    {
        pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_min;
        pPackStatus->empty_discharge_flag = 1;
    }
}

/*满充校准，放到安时积分函数里去*/
static void _FullChargeCalibration(PackStatus_t *pPackStatus)
{
    /* 3.63V强制校准 */
    if(pPackStatus->cell_max_voltage >= CHARGE_MAX_VOLTAGE + 200)
    {
        pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max;
        pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_max;
        pPackStatus->sox_info.soc_disp = 100.1f;
        dispsoctimedely = TimGetSystime();
    }
    else if(pPackStatus->cell_max_voltage >= (CHARGE_MAX_VOLTAGE + 100) && (pPackStatus->full_charge_flag == 0))
    {
        if(pPackStatus->sox_info.soc_real < pPackStatus->sox_info.soc_max - 1)
        {
            pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max - 1;
        }
        pPackStatus->full_charge_flag = 1;
    }
    else if((pPackStatus->full_charge_flag == 1) && (pPackStatus->cell_max_voltage < CHARGE_MAX_VOLTAGE + 200) && (grechargeflag == 0))
    {
        if(pPackStatus->sox_info.soc_disp <= 100)
        {
            pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max - 0.6f;
            pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_max - 0.6f;
            pPackStatus->sox_info.soc_disp = pPackStatus->sox_info.soc_max - 0.6f;
        }
    }

    if(pPackStatus->sox_info.soc_disp >= 100.0f)
    {
        grechargeflag  = 1;
    }
}

/*空放判断，待优化*/
static void _EmptyDischargeJudge(PackStatus_t *pPackStatus)////
{
    static uint8_t empty_charge_count = 0;
    static uint32_t empty_charge_times = 0;

    if(pPackStatus->sox_info.soc_disp < 95)
    {
        grechargeflag = 0;
    }

    if(!pPackStatus->empty_discharge_flag)
    {
        if(pPackStatus->cell_min_voltage <= DISCHARGE_MIN_VOLTAGE)
        {
            pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_min;
            pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_min;
            pPackStatus->sox_info.soc_disp = 0;
            pPackStatus->empty_discharge_flag = 1;
        }
        else
        {
            if(pPackStatus->sox_info.soc_disp <= 0)
            {
                pPackStatus->empty_discharge_flag = 1;
            }
        }

    }
    if(pPackStatus->empty_discharge_flag && (pPackStatus->sox_info.soc_real < 0.1f || \
            pPackStatus->sox_info.soc_actl < 0.1f || pPackStatus->sox_info.soc_disp < 0.1f))
    {

        pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_min;
        pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_min;
        pPackStatus->sox_info.soc_disp = 0;
    }
}

/*滑动滤波*/
static float _SlidingFilter(float value, EN_PACK_CURRENT_MODE mode, uint16_t level)
{
    static EN_PACK_CURRENT_MODE pre_mode = EN_PACK_CURRENT_IDLE_MODE;
    static uint16_t count = 0;
    static uint16_t pre_level = 0;
    static float filter_value = 0;
    static float ret;
    static uint32_t avg_value = 0;

    if(mode != pre_mode)
    {
        count = 0;
        pre_mode = mode;
        filter_value = 0;
    }
    if(level != pre_level)
    {
        count = 0;
        pre_level = level;
        filter_value = 0;
    }

    if(count < level - 1)
    {
        filter_value += value;
        ret = filter_value / (count + 1);
        avg_value = ret;
    }
    else
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
    int16_t temperature = pPackStatus->pack_average_temperature / 10;
    float current = pPackStatus->pack_current;//单位:mA
    static float recharge_current = 0;//单位:mA
    static int32_t recharge_current_total = 0;//单位:mA
    static int32_t recharge_current_count = 0;
    static uint32_t charge_capa = 0;//单位:mAh
    static uint32_t discharge_capa = 0;//单位:mAh
    float dsg_multiplier;
    static float recharge_times = 0;
    static uint8_t recharge_times_count = 0;
    static uint32_t current_sampe_time = 0;
    uint32_t fullcharge_times = 0;
    float recharge_times_temp = 0;
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
    recharge_current = current;
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

    pPackStatus->cr = capa_oi * pPackStatus->sox_info.soh / 100;/*当前最大可用容量通过插值获取*/
    pPackStatus->res_capa = pPackStatus->cr * pPackStatus->sox_info.soc_real / 100;
    pPackStatus->current_power = (abs(pPackStatus->pack_current) / 1000.0) * (gstCellInfo.total_cell_voltage / 100.0);   /* 当前功率 */
    if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
    {
        /* 预计充满时间 */
        if(pPackStatus->sox_info.soc_actl < 100 && pPackStatus->sox_info.soc_real < 100)
        {
            recharge_current_total += recharge_current;
            recharge_current_count++;

            /* 电流取30次平均值，减小时间跳动过快 */
            if(recharge_current_count >= 30)
            {
                recharge_current = recharge_current_total / recharge_current_count;
                recharge_current_count = 0;
                recharge_current_total = 0;
            }
            if(recharge_current == 0)
            {
                recharge_current = 1;
            }
            recharge_times_temp = (pPackStatus->cr * (pPackStatus->sox_info.soc_max - pPackStatus->sox_info.soc_real)) / 100 / recharge_current;

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
            else
            {
                recharge_times = recharge_times_temp;
            }
            pPackStatus->recharge_times = _SlidingFilter(recharge_times, pPackStatus->mode, 20);
        }
    }
    else
    {
        /* 剩余可用时间 */
        if(recharge_current < 1000)
        {
            recharge_current = 1000;
        }
        recharge_times = (pPackStatus->cr * (pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_min)) / 100 / recharge_current;
        pPackStatus->recharge_times = _SlidingFilter(recharge_times, pPackStatus->mode, 20);
    }
}


/*计算当前最大放电电流*/
void MaxDsgCurrentCalc(PackStatus_t *pPackStatus)
{
    static uint32_t dischargetime = 0;
    float soc = pPackStatus->sox_info.soc_real;
    if(gSoxDebugParam.sox_debug_mode == 1)
    {
        pPackStatus->sox_info.soc_real = 25;
        pPackStatus->sox_info.soc_actl = 25;
        pPackStatus->sox_info.soc_disp = 25;
        float soc = pPackStatus->sox_info.soc_real;
    }
    int16_t temperature = pPackStatus->pack_average_temperature / 10;

    int16_t temp_d, temp_u;
    uint8_t temp_index_d, temp_index_u;
    const uint32_t *p_curr_tempd, *p_curr_tempu;
    float soc_d, soc_u;
    uint8_t soc_index_d, soc_index_u;

    uint32_t curr_tempd_d, curr_tempd_u, curr_tempu_d, curr_tempu_u;
    uint32_t curr_tempd_oi, curr_tempu_oi, curr_oi;

    _GetTableRangeIndex8(temperature, DsgCurrTemperatureIndex, DSG_CURR_TEMPERATURE_LEVEL, &temp_index_d, &temp_index_u);
    temp_d = DsgCurrTemperatureIndex[temp_index_d];
    temp_u = DsgCurrTemperatureIndex[temp_index_u];
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

    pPackStatus->cur_max_dsg_current = curr_oi * PACK_RATED_CAPACITY / 100;;

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
    pPackStatus->cur_max_dsg_current_30s = curr_oi * PACK_RATED_CAPACITY / 100;

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
    pPackStatus->cur_max_dsg_current_60s = curr_oi * PACK_RATED_CAPACITY / 100; //放电电流倍率与电池安时数的积分

    if(pPackStatus->empty_discharge_flag)
    {
        pPackStatus->cur_max_dsg_current_60s = 0;
        pPackStatus->cur_max_dsg_current_30s = 0;
        pPackStatus->cur_max_dsg_current = 0;
    }
    set_target_coeff(pPackStatus->current_power_percentage);
    adjust_coeff();
    pPackStatus->cur_max_dsg_current_60s = gCurrentController.current_coeff * pPackStatus->cur_max_dsg_current_60s;
    pPackStatus->cur_max_dsg_current_30s = gCurrentController.current_coeff * pPackStatus->cur_max_dsg_current_30s;
    pPackStatus->cur_max_dsg_current = gCurrentController.current_coeff * pPackStatus->cur_max_dsg_current;
    if(gFaultAnalysisFlag.chargemos_enable_flag == 0 && gFaultAnalysisFlag.chargemos_enable_flag == 0)
    {
        pPackStatus->cur_max_dsg_current_60s = 0;
        pPackStatus->cur_max_dsg_current_30s = 0;
        pPackStatus->cur_max_dsg_current = 0;
    }
    if(gSoxDebugParam.sox_debug_mode == 1)
    {
        pPackStatus->cur_max_dsg_current_60s = 188000;
        pPackStatus->cur_max_dsg_current_30s = 188000;
        pPackStatus->cur_max_dsg_current = 188000;
    }	
    if( (pPackStatus->cur_max_dsg_current_60s < 50000) && (!initcurrflag))
    {
        if((TimeDiffFromNow(dischargetime) < 120000))
        {
            pPackStatus->cur_max_dsg_current_60s = 100000;
            pPackStatus->cur_max_dsg_current_30s = 100000;
            pPackStatus->cur_max_dsg_current = 100000;
        }
		else
		{
			initcurrflag = 1;
		}
    }	
}

/****************************************
 * 函数实现：SOC差值动态限流系数计算
 * 规则：
 * 1. SOC差值绝对值 ≤ 1 ：系数=1.0（无限流）
 * 2. SOC差值绝对值 ≥ 10：系数=0.1（最大限流）
 * 3. 1 < 差值 < 10      ：线性插值计算系数
 ****************************************/
static float _SOC_Diff_Coefficient(int16_t soc_diff)
{
    // 步骤1：取绝对值，仅关注偏差大小，不关注正负
    int16_t diff_abs = (soc_diff < 0) ? (-soc_diff) : soc_diff;

    // 步骤2：边界值判断
    if (diff_abs <= 1)
    {
        // 偏差极小，充电电流无限制
        return 1.0f;
    }
    else if (diff_abs >= 10)
    {
        // 偏差过大，严格限制充电电流
        return 0.1f;
    }
    else
    {
        // 步骤3：中间区间线性插值计算
        // 插值公式：y = (y2-y1)/(x2-x1) * (x-x1) + y1
        // 映射关系：x(1)=1, y=1.0；x(10)=10, y=0.1
        return (0.1f - 1.0f) / (10.0f - 1.0f) * (diff_abs - 1.0f) + 1.0f;
    }
}

/*计算当前最大充电倍率换算成电流*/
void MaxChgMultiCalc(PackStatus_t *pPackStatus)
{
    static uint32_t trickle_charge_count = 0;
    float soc = pPackStatus->sox_info.soc_real;
    int16_t soc_diff =  pPackStatus->sox_info.soc_real - pPackStatus->sox_info.soc_real;
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

    if((grechargeflag == 1) || (pPackStatus->sox_info.soc_disp >= 100) || (pPackStatus->sox_info.soc_real >= 100)) //复冲标志为1 请求电流为0
    {
        pPackStatus->cur_max_chg_current = 0;
    }
    else if(pPackStatus->full_charge_flag)//满充标志为1 3500mv
    {
        if(pPackStatus->sox_info.soc_disp < 100)
        {
            pPackStatus->cur_max_chg_current = _InterpolationOperation(3500, 3600, 0.8 * max_charge_current, 20000, pPackStatus->cell_max_voltage);
        }
        else
        {
            pPackStatus->cur_max_chg_current = 0;
        }
    }
    else if((pPackStatus->cell_max_voltage >= CHARGE_MAX_VOLTAGE) && (!_gTrickleChargeFlag)) //最大电压大于3400mV 则降低电流
    {
        _gTrickleChargeFlag = 1;
        pPackStatus->cur_max_chg_current = multi_oi * PACK_RATED_CAPACITY / 100;/*充电倍率表按照100倍，所以这里要除以100*/
        max_charge_current = pPackStatus->cur_max_chg_current;
    }
    else if(!_gTrickleChargeFlag)
    {
        pPackStatus->cur_max_chg_current = multi_oi * PACK_RATED_CAPACITY / 100;/*充电倍率表按照100倍，所以这里要除以100*/
        max_charge_current = pPackStatus->cur_max_chg_current;
    }
    else if(_gTrickleChargeFlag)
    {
        pPackStatus->cur_max_chg_current = _InterpolationOperation(3400, 3500, max_charge_current, 0.8 * max_charge_current, pPackStatus->cell_max_voltage);
    }
    set_target_coeff(pPackStatus->current_power_percentage);
    adjust_coeff();
    pPackStatus->cur_max_chg_current = gCurrentController.current_coeff * pPackStatus->cur_max_chg_current;
    if(gFaultAnalysisFlag.chargemos_enable_flag == 0 && gFaultAnalysisFlag.chargemos_enable_flag == 0)
    {
        pPackStatus->cur_max_chg_current = 0;
    }

//    float chg_current_coeff = _SOC_Diff_Coefficient(soc_diff);
//    pPackStatus->cur_max_chg_current = pPackStatus->cur_max_chg_current * chg_current_coeff;
}

static uint8_t _R2CorrectCoefJudge(PackStatus_t *pPackStatus)
{
    uint8_t ret = 0;
    static uint32_t timecount = 0;
    static uint8_t flg = 0;

    if(pPackStatus->cell_max_voltage >= CHARGE_MAX_VOLTAGE + 100)
    {
        if(flg == 0)
        {
            timecount = TimGetSystime();
            flg = 1;
        }
    }
    else
    {
        flg = 0;
    }

    if(flg == 1)
    {
        if(TimeDiffFromNow(timecount) > 1000)
        {
            if(pPackStatus->pack_current < -100 && pPackStatus->sox_info.soc_real < pPackStatus->sox_info.soc_max - 4)
            {
                ret = 1;
            }
        }
    }
    return ret;
}

/*安时积分法，放在定时器中断里，每隔900ms执行一次*/
void SOCAmpereHourIntegral(uint32_t ms, PackStatus_t *pPackStatus)
{
    uint32_t cr;/*当前最大容量，单位:mAh*/
    float r1_act = 1, r1_rpt = 1;
    float r2_act = 1, r2_rpt = 1;
    float r3_rpt = 1;
    float current = pPackStatus->pack_current;
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
        pPackStatus->empty_discharge_flag = 0;
    }
    /*计算当前容量*/
    cr = pPackStatus->cr;
    if(gSoxDebugParam.sox_debug_mode)
    {
        cr = 314000;
    }
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
            /*用于计算时长的SOC修正系数*/
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

            /*表显SOC的修正*/
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
            /*用于计算时长的SOC修正系数*/
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
            /*表显SOC的修正*/
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
        default:
            break;
    }

    /*电流，充电为负，放电为正*/
    if(pPackStatus->sox_info.soc_real >= 0)
    {
        pPackStatus->sox_info.soc_real -= (float)current * ms / 1000 / 3600 * 100 / cr;//mA, ms转h, 除以(1000 * 3600) * 100(%)
        if(pPackStatus->sox_info.soc_real <= 0)
        {
            pPackStatus->sox_info.soc_real = 0;
        }
    }
	
    if(pPackStatus->sox_info.soc_actl >= 0)
    {
        pPackStatus->sox_info.soc_actl -= r1_act * r2_act * (float)current * ms / 1000 / 3600  / cr * 100;//mA, ms转h, 除以(1000 * 3600)
        if(pPackStatus->sox_info.soc_actl <= 0)
        {
            pPackStatus->sox_info.soc_actl = 0;
        }
    }

    if(pPackStatus->sox_info.soc_disp >= 0)
    {
        if(pPackStatus->sox_info.soc_disp >= 100)
        {
            if((TimeDiffFromNow(dispsoctimedely) > 60000))
            {
                pPackStatus->sox_info.soc_disp -= r1_rpt * r2_rpt * r3_rpt * (float)current * ms / 1000 / 3600 * 100 / cr;//mA, ms转h, 除以(1000 * 3600)
                if(pPackStatus->sox_info.soc_disp <= 0)
                {
                    pPackStatus->sox_info.soc_disp = 0;
                }
            }
        }
        else
        {
            pPackStatus->sox_info.soc_disp -= r1_rpt * r2_rpt * r3_rpt * (float)current * ms / 1000 / 3600 * 100 / cr;//mA, ms转h, 除以(1000 * 3600)
            if(pPackStatus->sox_info.soc_disp <= 0)
            {
                pPackStatus->sox_info.soc_disp = 0;
            }
        }
    }

    /* 满充等待 */
    if(pPackStatus->mode == EN_PACK_CURRENT_CHARGE_MODE)
    {
        pPackStatus->empty_discharge_flag = 0;

        if(pPackStatus->sox_info.soc_real >= pPackStatus->sox_info.soc_max - 0.6f && pPackStatus->full_charge_flag == 0)
        {
            pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max - 0.6f;/*待满充校准触发后，再跳至100.1%*/
        }
        if((pPackStatus->sox_info.soc_actl >= pPackStatus->sox_info.soc_max - 0.6f && pPackStatus->full_charge_flag == 0) || \
                (pPackStatus->cell_max_voltage > (CHARGE_MAX_VOLTAGE + 50) && (pPackStatus->full_charge_flag == 0)))
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
        pPackStatus->chg_accumulate_capa_ah += fabs(current) * ms / 100 / 3600 / 1000.0;
        pPackStatus->chg_accumulate_capa_wh += (fabs(current / 1000.0f) * ms / 1000 / 3600) * (pPackStatus->cell_total_voltage);
    }
    /* 放空等待 */
    else if(pPackStatus->mode == EN_PACK_CURRENT_DISCHARGE_MODE)
    {
        pPackStatus->full_charge_flag = 0;
        /* 清除涓充标志 */
        _gTrickleChargeFlag = 0;
        if(pPackStatus->sox_info.soc_real < pPackStatus->sox_info.soc_min + 1.0f && pPackStatus->empty_discharge_flag == 0)
        {
            pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_min + 1.0f;/*待满充校准触发后，再跳至100.1%*/
        }
        if((pPackStatus->sox_info.soc_actl < pPackStatus->sox_info.soc_min + 1.0f && pPackStatus->empty_discharge_flag == 0) || \
                (pPackStatus->cell_min_voltage < (DISCHARGE_MIN_VOLTAGE + 300)))
        {
            pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_min + 1.0f;
        }

        if(pPackStatus->sox_info.soc_disp < pPackStatus->sox_info.soc_min + 1.0f && pPackStatus->empty_discharge_flag == 0)
        {
            pPackStatus->sox_info.soc_disp = pPackStatus->sox_info.soc_min + 1.0f;
        }
        /*放空判断*/
        _EmptyDischargeJudge(pPackStatus);
        /*计算SOH所需的累积容量*/
        pPackStatus->dsg_accumulate_capa_ah += (fabs(current) * ms) / 100 / 3600 / 1000.0;
        pPackStatus->dsg_accumulate_capa_wh += (fabs(current / 1000.0f) * ms / 1000 / 3600) * (pPackStatus->cell_total_voltage) ;
    }
    else if(pPackStatus->mode == EN_PACK_CURRENT_IDLE_MODE)
    {
        /* 清除涓充标志 */
        _gTrickleChargeFlag = 0;
        /*计算SOH所需的累积容量*/
        pPackStatus->dsg_accumulate_capa_ah += (fabs(current) * ms) / 100 / 3600 / 1000.0;
        pPackStatus->dsg_accumulate_capa_wh += (fabs(current / 1000.0f) * ms / 1000 / 3600) * (pPackStatus->cell_total_voltage) ;
    }

    /*计算电池循环次数和SOH*/
    if(pPackStatus->chg_accumulate_capa_ah >= pPackStatus->dsg_accumulate_capa_ah)
    {
        capa_ratio = pPackStatus->dsg_accumulate_capa_wh / PACK_RATED_CAPACITY_WH / 10;
    }
    else
    {
        capa_ratio = pPackStatus->chg_accumulate_capa_wh / PACK_RATED_CAPACITY_WH / 10;
    }
    pPackStatus->pack_cycles = capa_ratio;

    if(capa_ratio > 200)
    {
        _GetTableRangeIndex16u(capa_ratio, CellLoopCyclesDefault, CELL_LOOP_FREQ_LEVEL, &pack_cycles_index_tempd_d, &pack_cycles_index_tempd_u);
        pPackStatus->sox_info.soh = _InterpolationOperation(CellLoopCyclesDefault[pack_cycles_index_tempd_d], CellLoopCyclesDefault[pack_cycles_index_tempd_d], \
                                    CellLoopLifeDefault[pack_cycles_index_tempd_d], CellLoopLifeDefault[pack_cycles_index_tempd_d], capa_ratio) / 100.0f;
    }
    else
    {
        pPackStatus->sox_info.soh = 100;
    }
    if(pPackStatus->sox_info.soh > 100)
    {
        pPackStatus->sox_info.soh = 100;
    }
    pPackStatus->soh_rpt = pPackStatus->sox_info.soh * 10;
    if(gSoxDebugParam.sox_debug_mode == 1)
    {
        gPackStatus.soh_rpt = 1000;
    }
    /*防止越界*/

    if(pPackStatus->sox_info.soc_real >= pPackStatus->sox_info.soc_max)
    {
        pPackStatus->sox_info.soc_real = pPackStatus->sox_info.soc_max;
    }
    if(pPackStatus->sox_info.soc_actl >= pPackStatus->sox_info.soc_max)
    {
        pPackStatus->sox_info.soc_actl = pPackStatus->sox_info.soc_max;
    }
    if(pPackStatus->sox_info.soc_disp >= 100.0f)
    {
        pPackStatus->sox_info.soc_disp = 100.1f;
    }
}

/*当前满充、满放单体标定电压计算, 每隔一段时间轮询执行一次，主要是温度变化，不需要太频繁，初始化执行一次*/
void FullEmptyChargeRatedVoltageCalc(PackStatus_t *pPackStatus)
{
    uint16_t full_charge_voltage;
    uint16_t full_charge_voltage_tempd, full_charge_voltage_tempu;
    uint16_t empty_discharge_voltage;
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
    empty_discharge_voltage = _InterpolationOperation(temp_d, temp_u, empty_charge_voltage_tempd, empty_charge_voltage_tempu, temperature);

    pPackStatus->full_charge_voltage = full_charge_voltage;
    pPackStatus->empty_discharge_voltage = empty_discharge_voltage;
}

/*OCV校准条件判断*/
static void _OCVJudge(PackStatus_t *pPackStatus)
{
    if(pPackStatus->mode != EN_PACK_CURRENT_IDLE_MODE)
    {
        pPackStatus->ocv_time_start = RtcGetTime();
    }
    else
    {
        if(RtcGetTime() > pPackStatus->ocv_time_start)
        {
            if(RtcGetTime() - pPackStatus->ocv_time_start > 2 * 60 * 60)/*暂定2小时,后期改成宏定义*/
            {
                pPackStatus->ocv_time_start = RtcGetTime();
                OCVCalibraiton(pPackStatus);/*满足OCV校准条件，进行OCV校准*/
            }
        }
        else
        {
            pPackStatus->ocv_time_count = RtcGetTime();
        }
    }
}

/*更新RTC时间*/
void UpdateRtcTime(PackStatus_t *pPackStatus)
{
    pPackStatus->ocv_time_count = RtcGetTime();
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

void SoxFlashInfoSave(void)
{
    uint8_t save_flg[4];
    uint8_t save_backup_flg[4];
    uint8_t load_flg[4];
    uint8_t load_backup_flg[4];
    memset((uint8_t *)save_flg, 0x5A, 4);
    memset((uint8_t *)save_backup_flg, 0x5A, 4);
    gPackStatusBase = gPackStatus;
    gPackStatusBackUp = gPackStatus;
    if((gSoxDebugParam.sox_debug_mode == 0) || (gSoxDebugParam.sox_debug_mode == 1))
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            spi_flash_write_bytes_with_erase(SOX_FLASH_INFO_ADDR + 0x1000, save_flg, 4);
            spi_flash_write_bytes_with_erase(SOX_FLASH_INFO_ADDR, (uint8_t*)&gPackStatus, sizeof(PackStatus_t));//
            spi_flash_read_bytes(SOX_FLASH_INFO_ADDR, (uint8_t*)&sgSoxFlashInfo, sizeof(PackStatus_t));
            spi_flash_read_bytes(SOX_FLASH_INFO_ADDR + 0x1000, (uint8_t*)load_flg, 4);
            if((load_flg[0] == 0x5A) && (load_flg[1] == 0x5A) && (load_flg[2] == 0x5A) && (load_flg[3] == 0x5A))
            {
                if(memcmp(&gPackStatusBase, &sgSoxFlashInfo, sizeof(PackStatus_t)) == 0)
                {
                    break;
                }
            }
        }
    }

    /*备份存储*/
    if((gSoxDebugParam.sox_debug_mode == 0) || (gSoxDebugParam.sox_debug_mode == 1))
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            spi_flash_write_bytes_with_erase(SOX_FLASH_BACKUP_INFO_ADDR + 0x1000, save_backup_flg, 4);
            spi_flash_write_bytes_with_erase(SOX_FLASH_BACKUP_INFO_ADDR, (uint8_t*)&gPackStatus, sizeof(PackStatus_t));//
            spi_flash_read_bytes(SOX_FLASH_BACKUP_INFO_ADDR, (uint8_t*)&sgSoxFlashInfo, sizeof(PackStatus_t));
            spi_flash_read_bytes(SOX_FLASH_BACKUP_INFO_ADDR + 0x1000, (uint8_t*)load_backup_flg, 4);
            if((load_backup_flg[0] == 0x5A) && (load_backup_flg[1] == 0x5A) && (load_backup_flg[2] == 0x5A) && (load_backup_flg[3] == 0x5A))
            {
                if(memcmp(&gPackStatusBackUp, &sgSoxFlashInfo, sizeof(PackStatus_t)) == 0)
                {
                    break;
                }
            }

        }
    }
}

void SoxEepromInfoSave(void)
{
    uint8_t eeprom_save_flg[4];
    uint8_t eeprom_save_backup_flg[4];
    uint8_t eeprom_load_flg[4];
    uint8_t eeprom_load_backup_flg[4];
    memset((uint8_t *)eeprom_save_flg, 0x5A, 4);
    memset((uint8_t *)eeprom_save_backup_flg, 0x5A, 4);
    gPackStatusEEpromBase = gPackStatus;
    gPackStatusEEpromBackUp = gPackStatus;
    if((gSoxDebugParam.sox_debug_mode == 0) || (gSoxDebugParam.sox_debug_mode == 1))
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            EepromWrite((uint8_t*)&gPackStatus, SOX_EEPROM_INFO_ADDR, sizeof(PackStatus_t));
            EepromWrite((uint8_t *)eeprom_save_flg, 0x5F0, 4);

            EepromRead((uint8_t*)&sgSoxFlashInfo, SOX_EEPROM_INFO_ADDR, sizeof(PackStatus_t));//
            EepromRead((uint8_t*)eeprom_load_flg, 0x5F0, 4);

            if((eeprom_load_flg[0] == 0x5A) && (eeprom_load_flg[1] == 0x5A) && (eeprom_load_flg[2] == 0x5A) && (eeprom_load_flg[3] == 0x5A))
            {
                if(memcmp(&gPackStatusEEpromBase, &sgSoxFlashInfo, sizeof(PackStatus_t)) == 0)
                {
                    break;
                }
            }
        }
    }

    /*备份存储*/
    if((gSoxDebugParam.sox_debug_mode == 0) || (gSoxDebugParam.sox_debug_mode == 1))
    {
        for(uint8_t i = 0; i < 3; i++)
        {
            EepromWrite((uint8_t*)&gPackStatus, SOX_EEPROM_BACKUP_INFO_ADDR, sizeof(PackStatus_t));
            EepromWrite((uint8_t *)eeprom_save_backup_flg, 0x7F0, 4);

            EepromRead((uint8_t*)&sgSoxFlashInfo, SOX_EEPROM_INFO_ADDR, sizeof(PackStatus_t));
            EepromRead((uint8_t*)eeprom_load_backup_flg, 0x7F0, 4);

            if((eeprom_load_backup_flg[0] == 0x5A) && (eeprom_load_backup_flg[1] == 0x5A) && (eeprom_load_backup_flg[2] == 0x5A) && (eeprom_load_backup_flg[3] == 0x5A))
            {
                if(memcmp(&gPackStatusEEpromBackUp, &sgSoxFlashInfo, sizeof(PackStatus_t)) == 0)
                {
                    break;
                }
            }
        }
    }
}

void Current_Calibration_Factor_Save()
{
    uint8_t save_flg[4];
    memset((uint8_t *)save_flg, 0x5A, 4);
    spi_flash_write_bytes_with_erase(CURRENT_CALIBRATION_FLASH_ADDR, (uint8_t*)&gPackStatus.pack_current_calibration_factor, 4);
    spi_flash_write_bytes_with_erase(CURRENT_CALIBRATION_FLASH_ADDR + 0x1000, save_flg, 4);
    spi_flash_write_bytes_with_erase(CURRENT_CALIBRATION_BACKUP_FLASH_ADDR, (uint8_t*)&gPackStatus.pack_current_calibration_factor, 4);
    spi_flash_write_bytes_with_erase(CURRENT_CALIBRATION_BACKUP_FLASH_ADDR + 0x1000, save_flg, 4);
}

void CurrFlashInfoSave(void)
{

    int16_t pack_current_calibration_value = 0x0000;
    pack_current_calibration_value = garrBMSReg.bms_hold_para.current_calibrat_value;
    if(fabs(pack_current_calibration_value - (gPackStatus.pack_current_uncalibrated * gPackStatus.pack_current_calibration_factor)) < 300)
    {
        gPackStatus.pack_current_calibration_factor = (float)pack_current_calibration_value / gPackStatus.pack_current_uncalibrated;
        if((gPackStatus.pack_current_calibration_factor < 0) || (gPackStatus.pack_current_calibration_factor > 2))
        {
            gPackStatus.pack_current_calibration_factor = 0.9;
        }
        Current_Calibration_Factor_Save();
    }
}

void CurrFlashInfoLoad(void)
{
    uint8_t calibration_save_flag[4];
    uint8_t calibration_save_back_flag[4];
    spi_flash_read_bytes(CURRENT_CALIBRATION_FLASH_ADDR + 0x1000, (uint8_t*)calibration_save_flag, 4);
    if((calibration_save_flag[0] == 0x5A) && (calibration_save_flag[1] == 0x5A) && (calibration_save_flag[2] == 0x5A) && (calibration_save_flag[3] == 0x5A))
    {
        spi_flash_read_bytes(CURRENT_CALIBRATION_FLASH_ADDR, (uint8_t*)&gPackStatus.pack_current_calibration_factor, 4);
    }
    else
    {
        spi_flash_read_bytes(CURRENT_CALIBRATION_BACKUP_FLASH_ADDR + 0x1000, (uint8_t*)calibration_save_back_flag, 4);
        if((calibration_save_back_flag[0] == 0x5A) && (calibration_save_back_flag[1] == 0x5A) && (calibration_save_back_flag[2] == 0x5A) && (calibration_save_back_flag[3] == 0x5A))
        {
            spi_flash_read_bytes(CURRENT_CALIBRATION_BACKUP_FLASH_ADDR, (uint8_t*)&gPackStatus.pack_current_calibration_factor, 4);
        }
        else
        {
            gPackStatus.pack_current_calibration_factor = 1;
        }
    }
}

void SoxInfoLoad(void)
{
    uint8_t save_flg[4];
    EepromRead((uint8_t*)save_flg, 0x5F0, 4);
    LOG_INFO_HEX(LOG_TAG, (uint8_t*)&save_flg, 4);
    if(save_flg[0] != 0x5A || save_flg[1] != 0x5A || save_flg[2] != 0x5A || save_flg[3] != 0x5A)
    {
        EepromRead((uint8_t*)save_flg, 0x7F0, 4);
        if(save_flg[0] != 0x5A || save_flg[1] != 0x5A || save_flg[2] != 0x5A || save_flg[3] != 0x5A)
        {
            SoxFlashInfoLoad();//eeprom连续读取错误则从flash读取程序
        }
        else
        {
            EepromRead((uint8_t*)&sgSoxFlashInfo, SOX_EEPROM_BACKUP_INFO_ADDR, sizeof(PackStatus_t));
        }
    }
    else
    {
        EepromRead((uint8_t*)&sgSoxFlashInfo, SOX_EEPROM_INFO_ADDR, sizeof(PackStatus_t));
    }
    sgSoxFlashInfo.sox_info.soc_max = 100.1;
}

void SoxFlashInfoLoad(void)
{
    uint8_t flash_save_flg[4];
    spi_flash_read_bytes(SOX_FLASH_INFO_ADDR + 0x1000, (uint8_t*)flash_save_flg, 4);
    if(flash_save_flg[0] != 0x5A || flash_save_flg[1] != 0x5A || flash_save_flg[2] != 0x5A || flash_save_flg[3] != 0x5A)
    {
        spi_flash_read_bytes(SOX_FLASH_BACKUP_INFO_ADDR + 0x1000, (uint8_t*)flash_save_flg, 4);
        if(flash_save_flg[0] != 0x5A || flash_save_flg[1] != 0x5A || flash_save_flg[2] != 0x5A || flash_save_flg[3] != 0x5A)
        {
            memset((uint8_t*)&sgSoxFlashInfo, 0, sizeof(PackStatus_t));
            gPackStatus.cell_min_voltage = gstCellInfo.cell_min_voltage;
            gPackStatus.pack_average_temperature = gstCellInfo.cell_ave_temperature;
            gPackStatus.sox_info.soc_max = 100.1;
            OCVCalibraiton(&gPackStatus);
            if((gPackStatus.sox_info.soc_disp <= 0) && (gstCellInfo.cell_min_voltage >= 3200))
            {
                OCV_Test();//读取错误强制校准
            }
            sgSoxFlashInfo.sox_info.soc_max = 100.1;
            sgSoxFlashInfo.sox_info.soc_min = 0;
            sgSoxFlashInfo.sox_info.soc_real = gPackStatus.sox_info.soc_real;
            gPackStatus.sox_info.soc_actl = gPackStatus.sox_info.soc_real;
            sgSoxFlashInfo.sox_info.soc_actl = gPackStatus.sox_info.soc_real;
            sgSoxFlashInfo.sox_info.soc_disp = (gPackStatus.sox_info.soc_actl - gPackStatus.sox_info.soc_min) * 100 /
                                               (gPackStatus.sox_info.soc_max - gPackStatus.sox_info.soc_min);
            sgSoxFlashInfo.sox_info.soh = 100;
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
}

/*********************************************************************
 * 函数名：ResetFactory
 * 功能：恢复出厂设置（清除循环次数、累计容量，重置SOH）
 * 参数：
 *      pPackStatus：电池包状态结构体指针
 * 返回值：无
 * 说明：
 *      1. 重置后SOH=1000（100.0%），循环次数=0，累计充放电容量=0
 *      2. 需调用Flash/Eeprom保存函数，确保断电后参数不丢失
 *********************************************************************/
void ResetFactory(PackStatus_t *pPackStatus)
{
    uint8_t resetfactoryflg[4];
    memset((uint8_t *)resetfactoryflg, 0x5C, 4);
    spi_flash_write_bytes_with_erase(REST_FACTORY_INFO_ADDR, resetfactoryflg, 4);
    //接收恢复出厂设置命令  立即恢复
    garrBMSReg.bms_only_read_para.pack_ov_lvl_low = 296;   // 包过压低等级阈值（29.6V）
    garrBMSReg.bms_only_read_para.pack_ov_lvl_high = 0;    // 包过压高等级阈值（预留，未启用）
    garrBMSReg.bms_only_read_para.pack_uv_lvl_low = 208;   // 包欠压低等级阈值（20.8V）
    garrBMSReg.bms_only_read_para.pack_uv_lvl_high = 0;    // 包欠压高等级阈值（预留）
    //下次初始化保存
    pPackStatus->pack_discharge_voltage_low  = 208;
    pPackStatus->pack_discharge_voltage_high = 0;
    pPackStatus->pack_charge_voltage_low  = 296;
    pPackStatus->pack_charge_voltage_high = 0;
    SoxFlashInfoSave();  // 保存SOX相关参数到Flash（非易失性存储）
    SoxEepromInfoSave(); // 保存SOX相关参数到Eeprom（非易失性存储）
}


/*从下电到上电，执行一次，从深度睡眠到唤醒执行一次*/
void SoxInit(PackStatus_t *pPackStatus)
{
    uint8_t rdata = 0;
    CurrFlashInfoLoad();
    SoxInfoLoad();
    /*电芯数据从AFE获取，并赋值到gPackStatus*/
    uint8_t resetfactory_read_flg[4];
    spi_flash_read_bytes(REST_FACTORY_INFO_ADDR, (uint8_t*)resetfactory_read_flg, 4);
    init_controller(1.0f, 0.0015);
    if((resetfactory_read_flg[0] == 0x5C) && (resetfactory_read_flg[1] == 0x5C) && (resetfactory_read_flg[2] == 0x5C) && (resetfactory_read_flg[3] == 0x5C))
    {
        garrBMSReg.bms_only_read_para.pack_ov_lvl_high  = sgSoxFlashInfo.pack_charge_voltage_high;
        garrBMSReg.bms_only_read_para.pack_ov_lvl_low   = sgSoxFlashInfo.pack_charge_voltage_low;
        garrBMSReg.bms_only_read_para.pack_uv_lvl_high  = sgSoxFlashInfo.pack_discharge_voltage_high;
        garrBMSReg.bms_only_read_para.pack_uv_lvl_low   = sgSoxFlashInfo.pack_discharge_voltage_low;
        spi_flash_auto_erase_sector(REST_FACTORY_INFO_ADDR, 4);
    }
    pPackStatus->current_power_percentage    = 1;
    pPackStatus->cell_average_voltage        = gstCellInfo.cell_ave_voltage;
    pPackStatus->cell_max_voltage            = gstCellInfo.cell_max_voltage;
    pPackStatus->cell_min_voltage            = gstCellInfo.cell_min_voltage;
    pPackStatus->cell_total_voltage          = gstCellInfo.total_cell_voltage;
    pPackStatus->pack_current                = gstCellInfo.cell_current;
    pPackStatus->pack_average_temperature    = gstCellInfo.cell_ave_temperature;
    pPackStatus->pack_max_temperature        = gstCellInfo.cell_max_temperature;
    pPackStatus->pack_min_temperature        = gstCellInfo.cell_min_temperature;
    pPackStatus->sox_info.soc_max            = sgSoxFlashInfo.sox_info.soc_max;
    pPackStatus->sox_info.soc_min            = 0;
    pPackStatus->sox_info.soc_real           = sgSoxFlashInfo.sox_info.soc_real;
    pPackStatus->sox_info.soc_actl           = sgSoxFlashInfo.sox_info.soc_actl;
    pPackStatus->sox_info.soc_disp           = sgSoxFlashInfo.sox_info.soc_disp;
    pPackStatus->sox_info.soh                = sgSoxFlashInfo.sox_info.soh;
    pPackStatus->chg_accumulate_capa_ah      = sgSoxFlashInfo.chg_accumulate_capa_ah;
    pPackStatus->chg_accumulate_capa_wh      = sgSoxFlashInfo.chg_accumulate_capa_wh;
    pPackStatus->dsg_accumulate_capa_ah      = sgSoxFlashInfo.dsg_accumulate_capa_ah;
    pPackStatus->dsg_accumulate_capa_wh      = sgSoxFlashInfo.dsg_accumulate_capa_wh;
    pPackStatus->ocv_time_start              = sgSoxFlashInfo.ocv_time_start;
    pPackStatus->ocv_time_count              = RtcGetTime() - sgSoxFlashInfo.ocv_time_start;
    pPackStatus->full_charge_flag            = 0;
    pPackStatus->empty_discharge_flag        = 0;

    CurrentCapaCalc(pPackStatus);
    FullEmptyChargeRatedVoltageCalc(pPackStatus);
    /*上电或休眠唤醒校准*/
    if(pPackStatus->ocv_time_count > 0)
    {
        if(RtcNormalJudge() == true)
        {
            if(pPackStatus->ocv_time_count >  OCV_CALIBRATION_TIME)
            {
                if(pPackStatus->cell_min_voltage > 3340 || pPackStatus->cell_min_voltage < 3100)
                {
                    OCVCalibraiton(pPackStatus);
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
        EepromWrite(&rdata, SOX_FIRST_FIX_ADDR, 1);
    }
    pPackStatus->ocv_time_start = RtcGetTime();
    /*获取当前最大充电倍率换算成电流*/
    MaxChgMultiCalc(pPackStatus);
    /*获取当前放电最大电流*/
    MaxDsgCurrentCalc(pPackStatus);
    PackCurrentModeJudge(pPackStatus);
    _SoxInitFlag = 1;
}
/**
 * 初始化线性电流控制器
 * @param controller 控制器指针
 * @param base_current 基础电流
 * @param initial_coeff 初始系数
 * @param step 调节步长
 * @param interval_ms 调节间隔（毫秒）
 */
void init_controller(float initial_coeff, float step)
{
    gCurrentController.current_coeff = initial_coeff;
    gCurrentController.target_coeff = initial_coeff;
    gCurrentController.step = step;
}

/**
 * 设置目标充电系数（限制0~1范围）
 * @param controller 控制器指针
 * @param target_coeff 目标系数
 */
void set_target_coeff(float target_coeff)
{
    if (target_coeff >= 0.0f && target_coeff <= 1.0f)
    {
        gCurrentController.target_coeff = target_coeff;
    }
    else
    {
        return ;
    }
}

/**
 * 线性调节系数到目标值
 * @param controller 控制器指针
 * @return 1-调节完成，0-未完成
 */
int adjust_coeff(void)
{
    // 计算当前系数与目标系数的差值绝对值
    static uint32_t current_time = 0;
    float diff = fabs(gCurrentController.current_coeff - gCurrentController.target_coeff);
    // 差值大于步长时，逐步调整
    if (diff > gCurrentController.step)
    {
        if(TimeDiffFromNow(current_time) > 10)
        {
            current_time = TimGetSystime();
            if (gCurrentController.current_coeff < gCurrentController.target_coeff)
            {
                gCurrentController.current_coeff += gCurrentController.step;  // 系数上升
            }
            else
            {
                gCurrentController.current_coeff -= gCurrentController.step;  // 系数下降
            }
            // 四舍五入保留2位小数（避免浮点精度问题）
            gCurrentController.current_coeff = roundf(gCurrentController.current_coeff * 1000) / 1000;
        }
        return 0;
    }
    else
    {
        // 最终对齐目标系数
        gCurrentController.current_coeff = gCurrentController.target_coeff;
        return 1;
    }
}

/*大轮询调用*/
void SoxProcess(PackStatus_t *pPackStatus)
{
    static uint32_t timecount = 0;
    if(TimeDiffFromNow(timecount) > 1000)
    {
        timecount = TimGetSystime();
        /*1S执行的非高实时性*/
        CurrentCapaCalc(pPackStatus);
        /* 更新静态时间 */
        pPackStatus->ocv_time_count = RtcGetTime() - pPackStatus->ocv_time_start;
    }
    pPackStatus->cell_average_voltage        = gstCellInfo.cell_ave_voltage;
    pPackStatus->cell_max_voltage            = gstCellInfo.cell_max_voltage;
    pPackStatus->cell_min_voltage            = gstCellInfo.cell_min_voltage;
    pPackStatus->cell_total_voltage          = gstCellInfo.total_cell_voltage;
    pPackStatus->pack_average_temperature    = gstCellInfo.cell_ave_temperature;
    pPackStatus->pack_max_temperature        = gstCellInfo.cell_max_temperature;
    pPackStatus->pack_min_temperature        = gstCellInfo.cell_min_temperature;
    pPackStatus->current_power_percentage    = gPackStatus.current_power_percentage;
    PackCurrentModeJudge(pPackStatus);          /*通过电流方向判断充放电状态*/
    /*获取当前放电最大电流*/
    MaxDsgCurrentCalc(pPackStatus);
    /*获取当前最大充电倍率换算成电流 ——获取当前充电最大电流*/
    MaxChgMultiCalc(pPackStatus);
}

void ChargeDischargeControl()
{
    static uint32_t init_count = 0;
    if(AFEFirstSample() == true)
    {
        if(!sox_init_flag)
        {
            init_count ++;
            /* 开始AFE采集 */
            if(init_count > 50)
            {
                if(gSoxDebugParam.sox_debug_mode == 0)
                {
                    sox_init_flag = 1;
                    SoxInit(&gPackStatus);
                    OpenChargeMos();
                    OpenDisChargeMos();
                    StartAFESample();
                }
                else if(gSoxDebugParam.sox_debug_mode == 1)
                {
                    sox_init_flag = 1;
                    SoxInit(&gPackStatus_debug);
                    OpenChargeMos();
                    OpenDisChargeMos();
                    StartAFESample();
                }
                else if(gSoxDebugParam.sox_debug_mode == 2)
                {
                    SoxInit(&gPackStatus);
                    SoxInit(&gPackStatus_debug);
                }
            }
        }
        BMSRunStateProcess();
        if(gSoxDebugParam.sox_debug_mode == 0)
        {
            SoxProcess(&gPackStatus);
        }
        else if(gSoxDebugParam.sox_debug_mode == 1)
        {
            SoxProcess(&gPackStatus_debug);
        }
    }
}

