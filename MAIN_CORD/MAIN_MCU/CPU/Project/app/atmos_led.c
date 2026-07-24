/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        atmos_led.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-25
  * @brief       氛围灯控制业务
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-25    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "atmos_led.h"
#include "systick.h"
#include "slave_collect.h"
#include "error_proc.h"
#include "dev_mbi502x_led.h"


#define LED_THEME_GREEN             (0xF4005E)
#define LED_THEME_RED               (0x17EA88)
#define LED_THEME_YELLOW            (0x8CFF00)
#define LED_THEME_BLUE              (0xA900FD)
#define LED_YELLOW                  (0xB6FF36)
#define LED_GRAY                    (0x0A0A0A)
#define LED_FAULT_COLOR             (0x1CEC24)
#define PACK_LED_FAULT              (0x11FF15)


#define SIDE_RED                    (0X005500)
#define SIDE_BLUR                   (0X000055)
#define SIDE_GREEN                  (0X550000)
#define SIDE_THEAM_COL              (0X17EA88)


#define ATOMS_LED_NUM       (8)

static RunningStatus_t _gstWorkStatus;
static PackSocSta_t _gstPackSocStatus;

static WorkMode_t _gstSystemWorkMode;
static PackSta_t _garrPackSta[PACK_NUMS];

static TestStatus _gstTest;
static uint8_t _gFrontPanelLedStatus = 0;      // 0 -开  1-关
static uint8_t _gSidePanelLedStatus = 0;       // 0 -开  1-关
/********业务**********/
//采集器运行正常
static const DevAtmosCfg_t _gstAtmosCfgWifiNormal =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//采集器未连接WIFI路由器
static const DevAtmosCfg_t _gstAtmosCfgWifiRouterUnconnected =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BLINK,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
//    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .data[0] = {LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};

//采集器未连接云平台
static const DevAtmosCfg_t _gstAtmosCfgWifiCloudUnconnected =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//采集器未连接监控卡
static const DevAtmosCfg_t _gstAtmosCfgWifiOffLine =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {LED_GRAY, LED_GRAY, LED_GRAY, LED_GRAY, LED_GRAY, LED_GRAY, LED_GRAY, LED_GRAY},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};

//系统运行正常
static const DevAtmosCfg_t _gstAtmosCfgSysNormal =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//系统异常（故障）
static const DevAtmosCfg_t _gstAtmosCfgSysAbnormal =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
//    .data[0] = {0x01530E, 0x01530E, 0x01530E, 0x01530E, 0x01530E, 0x01530E, 0x01530E, 0x01530E},
    .data[0] = {LED_FAULT_COLOR, LED_FAULT_COLOR, LED_FAULT_COLOR, LED_FAULT_COLOR, LED_FAULT_COLOR, LED_FAULT_COLOR, LED_FAULT_COLOR, LED_FAULT_COLOR},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//系统告警
static const DevAtmosCfg_t _gstAtmosCfgSysWarnning =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//逆变器正常并网发电
static const DevAtmosCfg_t _gstAtmosCfgInvNormal =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//逆变器停机
static const DevAtmosCfg_t _gstAtmosCfgInvHalt =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {LED_GRAY, LED_GRAY, LED_GRAY, LED_GRAY, LED_GRAY, LED_GRAY, LED_GRAY, LED_GRAY},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//逆变器待机
static const DevAtmosCfg_t _gstAtmosCfgInvStandby =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
//    .data[0] = {0x3F3F00, 0x3F3F00, 0x3F3F00, 0x3F3F00, 0x3F3F00, 0x3F3F00, 0x3F3F00, 0x3F3F00},
    .data[0] = {LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//逆变器自检
static const DevAtmosCfg_t _gstAtmosCfgInvSelfCheck =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BLINK,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW, LED_THEME_YELLOW},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};
//逆变器离网运行
static const DevAtmosCfg_t _gstAtmosCfgInvOFFGridWork =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BLINK,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN, LED_THEME_GREEN},
    .effect_period = 4000,
    .effect_one_color_times = 1,
};
//逆变器旁路模式
static const DevAtmosCfg_t _gstAtmosCfgInvBypass =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {LED_THEME_BLUE, LED_THEME_BLUE, LED_THEME_BLUE, LED_THEME_BLUE, LED_THEME_BLUE, LED_THEME_BLUE, LED_THEME_BLUE, LED_THEME_BLUE},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};


static const DevAtmosCfg_t _gstAtmosCfgTest =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[1] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[2] = {0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24},
    .data[3] = {0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};

static const DevAtmosCfg_t _gstAtmosCfgTest1 =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 4,
    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .data[1] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[2] = {0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24},
    .data[3] = {0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};

static const DevAtmosCfg_t _gstAtmosCfgTest2 =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BLINK,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 4,
    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .data[1] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[2] = {0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24},
    .data[3] = {0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF},
    .effect_period = 500,
    .effect_one_color_times = 3,
};


static const DevAtmosCfg_t _gstAtmosCfgTest3 =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 4,
    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[1] = {0xF100CD, 0xF100CD, 0xF100CD, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .data[2] = {0x1CEC24, 0x1CEC24, 0x1CEC24, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF},
    .data[3] = {0xC44CFF, 0xC44CFF, 0xC44CFF, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24},
    .effect_period = 500,
    .effect_one_color_times = 3,
};

static const DevAtmosCfg_t _gstAtmosCfgTest4 =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 6,
    .data[0] = {0x7F0000, 0x007F00, 0x00007F, 0x007F7F, 0x7F007F, 0x7F7F00, 0x7F7F00, 0x7F7F00},
    .data[1] = {0x007F00, 0x00007F, 0x007F7F, 0x7F007F, 0x7F7F00, 0x7F0000, 0x7F0000, 0x7F0000},
    .data[2] = {0x00007F, 0x007F7F, 0x7F007F, 0x7F7F00, 0x7F0000, 0x007F00, 0x007F00, 0x007F00},
    .data[3] = {0x007F7F, 0x7F007F, 0x7F7F00, 0x7F0000, 0x007F00, 0x00007F, 0x00007F, 0x00007F},
    .data[4] = {0x7F007F, 0x7F7F00, 0x7F0000, 0x007F00, 0x00007F, 0x007F7F, 0x007F7F, 0x007F7F},
    .data[5] = {0x7F7F00, 0x7F0000, 0x007F00, 0x00007F, 0x007F7F, 0x7F007F, 0x7F007F, 0x7F007F},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};

//紫红
static const DevAtmosCfg_t _gstAtmosCfgHorseLamp1 =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 6,
    .data[0] = {0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x12B96C, 0x17EA88, 0x17EA88, 0x17EA88 },
    .data[1] = {0x17EA88, 0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x12B96C, 0x12B96C, 0x12B96C },
    .data[2] = {0x12B96C, 0x17EA88, 0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x0D8B51, 0x0D8B51 },
    .data[3] = {0x0D8B51, 0x12B96C, 0x17EA88, 0x02170D, 0x042E1B, 0x095C36, 0x095C36, 0x095C36 },
    .data[4] = {0x095C36, 0x0D8B51, 0x12B96C, 0x17EA88, 0x02170D, 0x042E1B, 0x042E1B, 0x042E1B },
    .data[5] = {0x042E1B, 0x095C36, 0x0D8B51, 0x12B96C, 0x17EA88, 0x02170D, 0x02170D, 0x02170D },
    .effect_period = 200,
    .effect_one_color_times = 1,
};
//黄色
static const DevAtmosCfg_t _gstAtmosCfgHorseLamp2 =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 6,
    .data[0] = {0x121905, 0x24330A, 0x486615, 0x6D9920, 0x91CC2B, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .data[1] = {0xB6FF36, 0x121905, 0x24330A, 0x486615, 0x6D9920, 0x91CC2B, 0x91CC2B, 0x91CC2B},
    .data[2] = {0x91CC2B, 0xB6FF36, 0x121905, 0x24330A, 0x486615, 0x6D9920, 0x6D9920, 0x6D9920},
    .data[3] = {0x6D9920, 0x91CC2B, 0xB6FF36, 0x121905, 0x24330A, 0x486615, 0x486615, 0x486615},
    .data[4] = {0x486615, 0x6D9920, 0x91CC2B, 0xB6FF36, 0x121905, 0x24330A, 0x24330A, 0x24330A},
    .data[5] = {0x24330A, 0x486615, 0x6D9920, 0x91CC2B, 0xB6FF36, 0x121905, 0x121905, 0x121905},
    .effect_period = 200,
    .effect_one_color_times = 1,
};
//蓝色
static const DevAtmosCfg_t _gstAtmosCfgHorseLamp3 =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 6,
    .data[0] = {0x180014, 0x3A0031, 0x44003E, 0x780066, 0xD300A6, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[1] = {0xF100CD, 0x180014, 0x3A0031, 0x44003E, 0x780066, 0xD300A6, 0xD300A6, 0xD300A6},
    .data[2] = {0xD300A6, 0xF100CD, 0x180014, 0x3A0031, 0x44003E, 0x780066, 0x780066, 0x780066},
    .data[3] = {0x780066, 0xD300A6, 0xF100CD, 0x180014, 0x3A0031, 0x44003E, 0x44003E, 0x44003E},
    .data[4] = {0x44003E, 0x780066, 0xD300A6, 0xF100CD, 0x180014, 0x3A0031, 0x3A0031, 0x3A0031},
    .data[5] = {0x3A0031, 0x44003E, 0x780066, 0xD300A6, 0xF100CD, 0x180014, 0x180014, 0x180014},
    .effect_period = 200,
    .effect_one_color_times = 1,
};

//拖尾
static const DevAtmosCfg_t _gstAtmosCfgTrail =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATMOS_CH_6_NUM,
    .color_change_num = 11,
    .data[0] = {
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
    },
    .data[1] = {
        0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
        0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
        0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
    },
    .data[2] = {
        0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
        0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
        0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
    },
    .data[3] = {
        0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
        0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
        0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
    },
    .data[4] = {
        0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
        0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
        0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A,
    },
    .data[5] = {
        0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A,
        0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A,
        0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A, 0x0A0A0A,
    },
    .data[6] = {
        0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A,
        0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A,
        0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0x0A0A0A,
    },
    .data[7] = {
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD,
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD,
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD,
    },
    .data[8] = {
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6,
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6,
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031, 0xD300A6,
    },
    .data[9] = {
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031,
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031,
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014, 0x3A0031,
    },
    .data[10] = {
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014,
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014,
        0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x0A0A0A, 0x180014,
    },
#if 0
    .data[0] = {0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA},
    .data[1] = {0xF100CD, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA},
    .data[2] = {0xD300A6, 0xF100CD, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA},
    .data[3] = {0x3A0031, 0xD300A6, 0xF100CD, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA},
    .data[4] = {0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA},
    .data[5] = {0xAAAAAA, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0xAAAAAA, 0xAAAAAA},
    .data[6] = {0xAAAAAA, 0xAAAAAA, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD, 0xAAAAAA},
    .data[7] = {0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0x180014, 0x3A0031, 0xD300A6, 0xF100CD},
    .data[8] = {0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0x180014, 0x3A0031, 0xD300A6},
    .data[9] = {0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0x180014, 0x3A0031},
    .data[10] = {0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0xAAAAAA, 0x180014},
#endif
    .effect_period = 200,
    .effect_one_color_times = 1,
};



//OFF
static const DevAtmosCfg_t _gstAtmosCfgHorseOff =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_OFF,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
    .effect_period = 500,
    .effect_one_color_times = 1,
};


//正常
static const DevAtmosCfg_t _gstAtmosCfgNormal =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .effect_period = 200,
    .effect_one_color_times = 1,
};

//故障
static const DevAtmosCfg_t _gstAtmosCfgFAULT =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15},
    .effect_period = 200,
    .effect_one_color_times = 1,
};

//绿色
static const DevAtmosCfg_t _gstAtmosCfgHorseGreen =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xAA0032},
    .effect_period = 500,
    .effect_one_color_times = 1,
};
//绿色加报警灯
static const DevAtmosCfg_t _gstAtmosCfgHorseGreenAndAlram =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0x8CFF00},
    .effect_period = 500,
    .effect_one_color_times = 1,
};
//蓝色
static const DevAtmosCfg_t _gstAtmosCfgHorseBlue =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .effect_period = 500,
    .effect_one_color_times = 1,
};
//黄色
static const DevAtmosCfg_t _gstAtmosCfgHorseYellow =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
//    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
//    .data[0] = {0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E},
    .data[0] = {0x8CFF00, 0x8CFF00, 0x8CFF00, 0x8CFF00, 0x8CFF00, 0x8CFF00, 0x8CFF00, 0x91CC2B},

    .effect_period = 500,
    .effect_one_color_times = 1,
};
//紫红
static const DevAtmosCfg_t _gstAtmosCfgHorseRed =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0xAA0032},
    .effect_period = 500,
    .effect_one_color_times = 1,
};

//青色呼吸
static const DevAtmosCfg_t _gstAtmosCfgHorseNormalbreath =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};
//黄色呼吸
static const DevAtmosCfg_t _gstAtmosCfgHorseYellowbreath =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
//    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
//    .data[0] = {0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E},
    .data[0] = {0xFFFF00, 0xFFFF00, 0xFFFF00, 0xFFFF00, 0xFFFF00, 0xFFFF00, 0xFFFF00, 0xFFFF00},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};
//紫红呼吸
static const DevAtmosCfg_t _gstAtmosCfgHorseRedbreath =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};
//红呼吸
static const DevAtmosCfg_t _gstAtmosCfgHorseRedBreath100 =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};


//绿色闪烁
static const DevAtmosCfg_t _gstAtmosCfgHorseNormalBlink =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BLINK,
    .led_num = ATOMS_LED_NUM - 1,
    .color_change_num = 1,
    .data[0] = {0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//黄色闪烁
static const DevAtmosCfg_t _gstAtmosCfgHorseYellowBlink =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATOMS_LED_NUM - 1,
    .color_change_num = 1,
//    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .data[0] = {0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E},
    .effect_period = 500,
    .effect_one_color_times = 1,
};


//紫红闪烁
static const DevAtmosCfg_t _gstAtmosCfgHorseRedBlink =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATOMS_LED_NUM - 1,
    .color_change_num = 1,
    .data[0] = {0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88},
    .effect_period = 500,
    .effect_one_color_times = 1,
};

//正面呼吸开机
static const DevAtmosCfg_t _gstAtmosCfgStartBreath =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_THIRD_BREATH,
    .led_num = ATOMS_LED_NUM,
    .color_change_num = 1,
    .data[0] = {0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E},
    .effect_period = 2600,
    .effect_one_color_times = 1,
    .effect_time_count = 3,
};


//开机 单色
static const DevAtmosCfg_t _gstAtmosCfgHorseStar =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON,
    .led_num = 7,
    .color_change_num = 7,
    .data[0] = {0x300012, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[1] = {0x300012, 0x610025, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[2] = {0x300012, 0x610025, 0xAA0032, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[3] = {0x300012, 0x610025, 0xAA0032, 0xF4005E, 0x000000, 0x000000, 0x000000},
    .data[4] = {0x300012, 0x610025, 0xAA0032, 0xF4005E, 0xAA0032, 0x000000, 0x000000},
    .data[5] = {0x300012, 0x610025, 0xAA0032, 0xF4005E, 0xAA0032, 0x610025, 0x000000},
    .data[6] = {0x300012, 0x610025, 0xAA0032, 0xF4005E, 0xAA0032, 0x610025, 0x300012},
    .effect_period = 80,
    .effect_one_color_times = 1,
};
//开机 多色
static const DevAtmosCfg_t _gstAtmosCfgHorseStarB =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON,
    .led_num = 7,
    .color_change_num = 6,
    .data[0] = {0x24001E, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[1] = {0x24001E, 0x600052, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[2] = {0x24001E, 0x600052, 0xA8008F, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[3] = {0x24001E, 0x600052, 0xA8008F, 0xF100CD, 0x000000, 0x000000, 0x000000},
    .data[4] = {0x24001E, 0x600052, 0xA8008F, 0xF100CD, 0xA8008F, 0x000000, 0x000000},
    .data[5] = {0x24001E, 0x600052, 0xA8008F, 0xF100CD, 0xA8008F, 0x600052, 0x000000},
    .data[6] = {0x24001E, 0x24001E, 0x600052, 0xA8008F, 0xF100CD, 0xA8008F, 0x600052},
    .effect_period = 80,
    .effect_one_color_times = 1,
};
//开机 多色
static const DevAtmosCfg_t _gstAtmosCfgHorseStarR =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON,
    .led_num = 7,
    .color_change_num = 6,
    .data[0] = {0x02170D, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[1] = {0x02170D, 0x042E1B, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[2] = {0x02170D, 0x042E1B, 0x095C36, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[3] = {0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x000000, 0x000000, 0x000000},
    .data[4] = {0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x12B96C, 0x000000, 0x000000},
    .data[5] = {0x02170D, 0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x12B96C, 0x17E888},
    .effect_period = 80,
    .effect_one_color_times = 1,
};
//开机 多色
static const DevAtmosCfg_t _gstAtmosCfgHorseStarY =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON,
    .led_num = 7,
    .color_change_num = 6,
    .data[0] = {0x121905, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[1] = {0x121905, 0x24330A, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[2] = {0x121905, 0x24330A, 0x486615, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[3] = {0x121905, 0x24330A, 0x486615, 0x6D9920, 0x000000, 0x000000, 0x000000},
    .data[4] = {0x121905, 0x24330A, 0x486615, 0x6D9920, 0x91CC2B, 0x000000, 0x000000},
    .data[5] = {0x121905, 0x121905, 0x24330A, 0x486615, 0x6D9920, 0x91CC2B, 0xB6FF36},
    .effect_period = 80,
    .effect_one_color_times = 1,
};

//20%紫红
static const DevAtmosCfg_t _gstAtmosCfgHorseRed20 =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_THIRD_BREATH,
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88},  //紫红
//  .data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},  //蓝绿色
    .effect_period = 2600,
    .effect_one_color_times = 1,
    .effect_time_count = 3,
};

//侧灯板多色呼吸
static const DevAtmosCfg_t _gstAtmosCfgSideBreath =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATMOS_CH_6_NUM,
    .color_change_num = 3,
    .data[0] = {
        0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88,
        0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88,
        0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88,
    },

    .data[1] = {
        0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E,
        0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E,
        0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E,
    },

    .data[2] = {
        0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15,
        0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15,
        0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15,
    },
    .effect_period = 2000,
    .effect_one_color_times = 1,
};
//侧灯板绿色呼吸
static const DevAtmosCfg_t _gstAtmosCfgSideBreathGreen =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATMOS_CH_6_NUM,
    .color_change_num = 1,
    .data[0] = {
        0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E,
        0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E,
        0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E,
    },
    .effect_period = 2000,
    .effect_one_color_times = 1,
};
//侧灯板蓝色呼吸
static const DevAtmosCfg_t _gstAtmosCfgSideBreathYellow =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH,
    .led_num = ATMOS_CH_6_NUM,
    .color_change_num = 1,
    .data[0] = {
        0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000,
        0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000,
        0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000, 0xFF0000,
    },
    .effect_period = 2000,
    .effect_one_color_times = 1,
};

//侧灯板多色常亮紫红
static const DevAtmosCfg_t _gstAtmosCfgSideOnPurple =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATMOS_CH_6_NUM,
    .color_change_num = 1,
    .data[0] = {
        0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88,
        0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88,
        0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88,
    },
    .effect_period = 300,
    .effect_one_color_times = 1,
};
//侧灯板多色常亮绿色
static const DevAtmosCfg_t _gstAtmosCfgSideOnGreen =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATMOS_CH_6_NUM,
    .color_change_num = 1,
    .data[0] = {
        0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E,
        0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E,
        0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E,
    },
    .effect_period = 300,
    .effect_one_color_times = 1,
};
//侧灯板多色常亮红色
static const DevAtmosCfg_t _gstAtmosCfgSideOnRed =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATMOS_CH_6_NUM,
    .color_change_num = 1,
    .data[0] = {
        0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500,
        0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500,
        0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500,
    },
    .effect_period = 300,
    .effect_one_color_times = 1,
};
//侧灯板异常
static const DevAtmosCfg_t _gstAtmosCfgSideFault =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = ATMOS_CH_6_NUM,
    .color_change_num = 1,
    .data[0] = {
        0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500,
        0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500,
        0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500,
    },
    .effect_period = 300,
    .effect_one_color_times = 1,
};
//侧灯板多色闪烁蓝色
static const DevAtmosCfg_t _gstAtmosCfgSideBlinkBlue =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BLINK,
    .led_num = ATMOS_CH_6_NUM,
    .color_change_num = 3,
    .data[0] = {
        0x000055, 0x000055, 0x000055, 0x000055, 0x000055, 0x000055, 0x000055,
        0x000055, 0x000055, 0x000055, 0x000055, 0x000055, 0x000055, 0x000055,
        0x000055, 0x000055, 0x000055, 0x000055, 0x000055, 0x000055, 0x000055,
    },

    .data[1] = {
        0x550000, 0x550000, 0x550000, 0x550000, 0x550000, 0x550000, 0x550000,
        0x550000, 0x550000, 0x550000, 0x550000, 0x550000, 0x550000, 0x550000,
        0x550000, 0x550000, 0x550000, 0x550000, 0x550000, 0x550000, 0x550000,
    },

    .data[2] = {
        0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500,
        0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500,
        0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500, 0x005500,
    },
    .effect_period = 500,
    .effect_one_color_times = 1,
};



void PowerOnAtmosLedInit(void)
{
    garrDevAtmosCfg[0] = _gstAtmosCfgHorseStar;
    garrDevAtmosCfg[1] = _gstAtmosCfgHorseStar;
    garrDevAtmosCfg[2] = _gstAtmosCfgHorseStar;
}

void AtmosLedPackUpdateTest(void)
{
    static uint32_t pack_count = 0;
    uint16_t pack_poll_period = TimerPollingHandle.timer_cfg->period / 1000;
    if(pack_count % 1000 == 0)
    {
        switch(pack_count / 1000)
        {
            case 0:
            {
                _garrPackSta[0].pack_sta = 0;
                _garrPackSta[1].pack_sta = 1;
                _garrPackSta[2].pack_sta = 1;
                break;
            }
            case 1:
            {
                _garrPackSta[0].pack_sta = 1;
                _garrPackSta[1].pack_sta = 0;
                _garrPackSta[2].pack_sta = 1;
                break;
            }
            case 2:
            {
                _garrPackSta[0].pack_sta = 1;
                _garrPackSta[1].pack_sta = 1;
                _garrPackSta[2].pack_sta = 0;
                break;
            }
            default:
                break;
        }
    }
    pack_count++;
    if(pack_count / 1000 > 2)
    {
        pack_count = 0;
    }
}

void AtmosLedFuncUpdateTest()
{
    //根据实际业务判断是否更新garrDevAtmosCfg
    static uint32_t count = 0;
    uint16_t poll_period = TimerPollingHandle.timer_cfg->period / 3000;


    if(count % 3000 == 0)
    {
        switch(count / 3000)
        {
            case 0:
            {
                garrDevAtmosCfg[0] = _gstAtmosCfgHorseStar;
                garrDevAtmosCfg[1] = _gstAtmosCfgHorseStar;
                garrDevAtmosCfg[2] = _gstAtmosCfgHorseStar;

                garrDevAtmosCfg[4] = _gstAtmosCfgSideOnGreen;
                garrDevAtmosCfg[5] = _gstAtmosCfgSideOnGreen;
                break;
            }
            case 1:
            {
                garrDevAtmosCfg[0] = _gstAtmosCfgHorseNormalbreath;
                garrDevAtmosCfg[1] = _gstAtmosCfgHorseYellowbreath;
//              garrDevAtmosCfg[1] = _gstAtmosCfgHorseRedbreath;
                garrDevAtmosCfg[2] = _gstAtmosCfgHorseRedBreath100;

                garrDevAtmosCfg[4] = _gstAtmosCfgSideBreathGreen;
                garrDevAtmosCfg[5] = _gstAtmosCfgSideBreathGreen;
                break;
            }
            case 2:
            {
                garrDevAtmosCfg[0] = _gstAtmosCfgStartBreath;
                garrDevAtmosCfg[1] = _gstAtmosCfgStartBreath;
                garrDevAtmosCfg[2] = _gstAtmosCfgStartBreath;

                garrDevAtmosCfg[4] = _gstAtmosCfgSideBreath;
                garrDevAtmosCfg[5] = _gstAtmosCfgSideBreath;
                break;
            }
            default:
                break;
        }
    }
    count++;
    if(count / 3000 > 2)
    {
        count = 0;
    }
}


void AtmosLedFuncUpdate(void)
{
    //根据实际业务判断是否更新garrDevAtmosCfg
#if 0
    garrDevAtmosCfg[0] = _gstAtmosCfgHorseGreen;
    garrDevAtmosCfg[1] = _gstAtmosCfgHorseRed;
    garrDevAtmosCfg[2] = _gstAtmosCfgHorseYellow;

    garrDevAtmosCfg[4] = _gstAtmosCfgSideOnGreen;
    garrDevAtmosCfg[5] = _gstAtmosCfgSideOnGreen;

//  garrDevAtmosCfg[4] = _gstAtmosCfgTrail;
//  garrDevAtmosCfg[5] = _gstAtmosCfgTrail;
#endif


#if 1
    static uint32_t count = 0;
    uint16_t poll_period = TimerPollingHandle.timer_cfg->period / 1500;

    if(0 == gstCollectParaData.collect_data.device_running_status)
    {
        garrDevAtmosCfg[0] = _gstAtmosCfgHorseStar;
        garrDevAtmosCfg[1] = _gstAtmosCfgHorseStar;
        garrDevAtmosCfg[2] = _gstAtmosCfgHorseStar;
    }
    else
    {
        if(count % 1500 == 0)
        {
            switch(count / 1500)
            {
                case 0:
                {
                    if(_gFrontPanelLedStatus == 0)
                    {
                        garrDevAtmosCfg[0] = _gstAtmosCfgHorseGreen;
                        garrDevAtmosCfg[1] = _gstAtmosCfgHorseGreen;
                        garrDevAtmosCfg[2] = _gstAtmosCfgHorseGreen;
                    }
                    else
                    {
                        garrDevAtmosCfg[0] = _gstAtmosCfgHorseOff;
                        garrDevAtmosCfg[1] = _gstAtmosCfgHorseOff;
                        garrDevAtmosCfg[2] = _gstAtmosCfgHorseOff;
                    }

                    if(_gSidePanelLedStatus == 0)
                    {
                        garrDevAtmosCfg[4] = _gstAtmosCfgSideOnGreen;
                        garrDevAtmosCfg[5] = _gstAtmosCfgSideOnGreen;
                    }
                    else
                    {
                        garrDevAtmosCfg[4] = _gstAtmosCfgHorseOff;
                        garrDevAtmosCfg[5] = _gstAtmosCfgHorseOff;
                    }
                    break;
                }
                case 1:
                {
                    if(_gFrontPanelLedStatus == 0)
                    {
                        garrDevAtmosCfg[0] = _gstAtmosCfgHorseNormalbreath;
                        garrDevAtmosCfg[1] = _gstAtmosCfgHorseNormalbreath;
                        garrDevAtmosCfg[2] = _gstAtmosCfgHorseNormalbreath;
                    }
                    else
                    {
                        garrDevAtmosCfg[0] = _gstAtmosCfgHorseOff;
                        garrDevAtmosCfg[1] = _gstAtmosCfgHorseOff;
                        garrDevAtmosCfg[2] = _gstAtmosCfgHorseOff;
                    }

                    if(_gSidePanelLedStatus == 0)
                    {
                        garrDevAtmosCfg[4] = _gstAtmosCfgSideBreathGreen;
                        garrDevAtmosCfg[5] = _gstAtmosCfgSideBreathGreen;
                    }
                    else
                    {
                        garrDevAtmosCfg[4] = _gstAtmosCfgHorseOff;
                        garrDevAtmosCfg[5] = _gstAtmosCfgHorseOff;
                    }
                    break;
                }
                case 2:
                {
                    _gstTest.inv_sta = 2;
                    _gstTest.wifi_sta = 2;
                    _gstTest.sys_sta = 2;

                    InvStatus();
                    WifiStatus();
                    SysStatus();
                }
                case 3:
                {
                    _gstTest.inv_sta = 2;
                    _gstTest.wifi_sta = 0;
                    _gstTest.sys_sta = 1;

                    InvStatus();
                    WifiStatus();
                    SysStatus();
                }
                default:
                    break;
            }
        }
        count++;
        if(count / 1500 > 1)
        {
            count = 0;
        }
    }
#endif

#if 0
    InvStatus();
    WifiStatus();
    SysStatus();
#endif
}


/**
  * @brief   设置采集器的状态
  * @param   None
  * @return  None
  * @note    None
  */
static void _SetWifiStatus(uint16_t status)
{
    switch(status)
    {
        case EN_ATMOS_STATUS_WIFI_NORMAL:
        {
            garrDevAtmosCfg[1] = _gstAtmosCfgWifiNormal;
            break;
        }
        case EN_ATMOS_STATUS_WIFI_DISCONN:
        {
            garrDevAtmosCfg[1] = _gstAtmosCfgWifiRouterUnconnected;
            break;
        }
        case EN_ATMOS_STATUS_WIFI_DISCONN_CLOUD:
        {
            garrDevAtmosCfg[1] = _gstAtmosCfgWifiCloudUnconnected;
            break;
        }
        case EN_ATMOS_STATUS_WIFI_DISCONN_MONITOR:
        {
            garrDevAtmosCfg[1] = _gstAtmosCfgWifiOffLine;//_gstAtmosCfgWifiRouterUnconnected;//_gstAtmosCfgWifiOffLine;
            break;
        }
        default:
            break;
    }
}

/**
  * @brief   设置系统报警和错误
  * @param   None
  * @return  None
  * @note    None
  */
static void _SetSysErrAlarmStatus(uint16_t status)
{
    switch(status)
    {
        case EN_ATMOS_STATUS_SYS_NORMAL:
        {
            garrDevAtmosCfg[0] = _gstAtmosCfgSysNormal;
            break;
        }
        case EN_ATMOS_STATUS_SYS_WARNNING:
        {
            garrDevAtmosCfg[0] = _gstAtmosCfgSysWarnning;
            break;
        }
        case EN_ATMOS_STATUS_SYS_ERR:
        {
            garrDevAtmosCfg[0] = _gstAtmosCfgSysAbnormal;
            break;
        }
        default:
            break;
    }
}

/**
  * @brief   设置逆变器报警和错误
  * @param   None
  * @return  None
  * @note    None
  */
static void _SetInvterErrAlarmStatus(uint16_t status)
{
    switch(status)
    {
        case EN_ATMOS_STATUS_INV_CLOSE:
        {
            garrDevAtmosCfg[2] = _gstAtmosCfgInvHalt;
            break;
        }
        case EN_ATMOS_STATUS_INV_STANDBY:
        {
            garrDevAtmosCfg[2] = _gstAtmosCfgInvStandby;//_gstAtmosCfgInvOFFGridWork;//_gstAtmosCfgInvSelfCheck;//_gstAtmosCfgInvStandby;
            break;
        }
        case EN_ATMOS_STATUS_INV_CHECK:
        {
            garrDevAtmosCfg[2] = _gstAtmosCfgInvSelfCheck;
            break;
        }
        case EN_ATMOS_STATUS_INV_NORMAL:
        {
            garrDevAtmosCfg[2] = _gstAtmosCfgInvNormal;
            break;
        }
        case EN_ATMOS_STATUS_INV_OFFGRID:
        {
            garrDevAtmosCfg[2] = _gstAtmosCfgInvOFFGridWork;
            break;
        }
        case EN_ATMOS_STATUS_INV_BYPASS:
        {
            garrDevAtmosCfg[2] = _gstAtmosCfgInvBypass;
            break;
        }
        default:
            break;
    }
}

/**
  * @brief   设置系统报警和错误
  * @param   None
  * @return  None
  * @note    None
  */
static void _SetPackErrAlarmStatus(uint16_t status)
{
    switch(status)
    {
        case EN_ATMOS_STATUS_PACK_NORMAL:
        {
            garrDevAtmosCfg[4] = _gstAtmosCfgSideOnGreen;
            garrDevAtmosCfg[5] = _gstAtmosCfgSideOnGreen;
            break;
        }
        case EN_ATMOS_STATUS_PACK_ERR:
        {
            garrDevAtmosCfg[4] = _gstAtmosCfgSideFault;
            garrDevAtmosCfg[5] = _gstAtmosCfgSideFault;
            break;
        }
        default:
            break;
    }
}

/**
  * @brief   获取SOC的值，并下发到数码管中
  * @param   None
  * @return  None
  * @note    None
  */
static void _SetPackSoc(uint16_t soc)
{
    DevDightLedCtl(soc, 1);
}

static void _SetPackIsCharge(uint16_t value)
{
    DevDightChargeLedCtl(value);
}

/**
  * @brief   采集器状态轮询处理
  * @param   None
  * @return  None
  * @note    None
  */
static void _WifiStatusPolling()
{
    if(_gstWorkStatus.wifi_sta != gstCollectParaData.collect_data.net_state)
    {
        _gstWorkStatus.wifi_sta = gstCollectParaData.collect_data.net_state;
        _SetWifiStatus(_gstWorkStatus.wifi_sta);
    }
}

/**
  * @brief   系统报警、错误轮询处理
  * @param   None
  * @return  None
  * @note    None
  */
static void _SysErrAlarmPolling()
{
    uint32_t err_code;
    err_code = GetErrId();

    uint32_t errcode[4] =
    {
        err_code,       //错误码
        0,              //临时错误类型
        0,              //错误源
        0               //错误值
    };

    if(errcode[0] > 0)  //存在警告和错误
    {
        errcode[1] = ((errcode[0] >> 30) & 0xFF);
//        errcode[2] = ((errcode[0] >> 16) & 0x0F) * 100;
//        errcode[3] = (errcode[0] & 0xFFFF) + errcode[2];
    }
    else
    {
        //不存在警告和错误
        errcode[1] = 0;
    }

    if(_gstWorkStatus.sys_sta != (uint16_t)errcode[1])
    {
        _gstWorkStatus.sys_sta = (uint16_t)errcode[1];
        _SetSysErrAlarmStatus(_gstWorkStatus.sys_sta);
    }
}

/**
  * @brief   逆变器报警、错误轮询处理
  * @param   None
  * @return  None
  * @note    None
  */
static void _InvterErrAlarmPolling()
{
    uint16_t status;
    uint16_t sys_status;
    uint16_t inv_status;

    sys_status = gstCollectParaData.pcs_input_front_data.system_status;  //0-待机  1-自检  2-混合并网 3-离网 4-旁路 5-关机
    inv_status = gstCollectParaData.pcs_input_front_data.invs_status;    //1-离网  2-并网

    if(sys_status < 2 || sys_status == 5 || sys_status == 4)  //使用系统状态的待机 自检 旁路 关机作为逆变器的状态
    {
        status = sys_status;
    }
    else if(sys_status >= 2 && sys_status < 5)
    {
        // 系统状态和逆变器状态(离网 或 并网) 同时选中 进行下发
        if(inv_status == 1)
        {
            status = EN_ATMOS_STATUS_INV_OFFGRID;
        }
        else
        {
            status = EN_ATMOS_STATUS_INV_NORMAL;
        }
    }
#if 0
    if(status != 5)
    {
        status = 5;
    }
#endif

    if(_gstWorkStatus.inv_sta != status)
    {
        _gstWorkStatus.inv_sta = status;
        _SetInvterErrAlarmStatus(_gstWorkStatus.inv_sta);
    }
}

/**
  * @brief   Pack报警、错误轮询处理
  * @param   None
  * @return  None
  * @note    None
  */
static void _PackErrAlarmPolling()
{
    uint16_t pack_sta;
    uint32_t err_code;
    err_code = GetErrId();

    uint32_t errcode[4] =
    {
        err_code,       //错误码
        0,              //临时错误类型
        0,              //错误源
        0               //错误值
    };

    if(errcode[0] > 0)  //存在警告和错误
    {
        errcode[1] = ((errcode[0] >> 30) & 0xFF);
        errcode[2] = ((errcode[0] >> 16) & 0x0F);
    }

    pack_sta = errcode[2];   //判断是不是BMS的错误  1代表BMS错误

    if((pack_sta == 1) & (errcode[1] == 2)) //错误码以及BMS的错误
    {
        pack_sta = EN_ATMOS_STATUS_PACK_ERR;
    }
    else
    {
        pack_sta = EN_ATMOS_STATUS_PACK_NORMAL;
    }

    if(_gstWorkStatus.pack_sta != pack_sta)
    {
        _gstWorkStatus.pack_sta = pack_sta;
        _SetPackErrAlarmStatus(_gstWorkStatus.pack_sta);
    }
}

/**
  * @brief   soc值获取轮询处理
  * @param   None
  * @return  None
  * @note    None
  */
static void _PackSocGetPolling(void)
{
    if(_gstPackSocStatus.soc != gstCollectParaData.collect_data.batt_soc)
    {
        _gstPackSocStatus.soc = gstCollectParaData.collect_data.batt_soc;
        _gstPackSocStatus.soc = _gstPackSocStatus.soc / 10;
        _SetPackSoc(_gstPackSocStatus.soc);
    }

    if(_gstPackSocStatus.is_charge != gstCollectParaData.collect_data.cur_direct_status.bits.batt_cur_direct)
    {
        _gstPackSocStatus.is_charge = gstCollectParaData.collect_data.cur_direct_status.bits.batt_cur_direct;
        _SetPackIsCharge(_gstPackSocStatus.is_charge);
    }
}


/**
  * @brief   WiFi 逆变器 系统 pack状态轮询处理
  * @param   None
  * @return  None
  * @note    None
  */
void AtmosLedErrAlarmPolling(void)
{ 
    if(1 == gstCollectParaData.collect_data.device_running_status)  //1--初始化中
    {
        AtmosLedErrAlarmInit();
    }
    if(2 == gstCollectParaData.collect_data.device_running_status)  //2--准备就绪
    {
        _WifiStatusPolling();
        _SysErrAlarmPolling();
        _InvterErrAlarmPolling();
        _PackSocGetPolling();
    }
}
/**
  * @brief   侧灯板状态轮询
  * @param   None
  * @return  None
  * @note    None
  */
void SideAtomsPolling(void)
{
    static uint32_t count = 0;
    uint16_t poll_period = TimerPollingHandle.timer_cfg->period / 3000;

    if(count % 3000 == 0)
    {
        switch(count / 3000)
        {
            case 0:
            {
                garrDevAtmosCfg[4] = _gstAtmosCfgSideBreathGreen;
                garrDevAtmosCfg[5] = _gstAtmosCfgSideBreathGreen;
                break;
            }
            default:
                break;
        }
    }
    count++;
    if(count / 3000 > 1)
    {
        count = 0;
    }
}

/**
  * @brief   WiFi 逆变器 系统 pack状态初始化
  * @param   None
  * @return  None
  * @note    None
  */
void AtmosLedErrAlarmInit(void)
{
    _SetWifiStatus(_gstWorkStatus.wifi_sta);
    _SetSysErrAlarmStatus(_gstWorkStatus.sys_sta);
    _SetInvterErrAlarmStatus(_gstWorkStatus.inv_sta);
    _SetPackSoc(_gstPackSocStatus.soc);
    _SetPackIsCharge(_gstPackSocStatus.is_charge);
}


void InvStatus(void)
{
    if(_gstTest.inv_sta == 0)  //正常状态
    {
        _gstSystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_NORMAL;
    }
    else if(_gstTest.inv_sta == 1) //逆变器停机
    {
        _gstSystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_CLOSE;
    }
    else if(_gstTest.inv_sta == 2)  //逆变器起待机
    {
        _gstSystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_STANDBY;
    }
    else if(_gstTest.inv_sta == 3)   //逆变器自检
    {
        _gstSystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_CHECK;
    }
    else if(_gstTest.inv_sta == 4)   //逆变器离网运行
    {
        _gstSystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_OFFGRID;
    }
    else if(_gstTest.inv_sta == 5)   //逆变器旁路模式
    {
        _gstSystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_BYPASS;
    }


    if(_gstSystemWorkMode._AtmostatusInv != _gstSystemWorkMode._AtmostatusInvPre)
    {
        switch(_gstSystemWorkMode._AtmostatusInv)
        {
            case EN_ATMOS_STA_INV_NORMAL:
            {
                garrDevAtmosCfg[0] = _gstAtmosCfgSysNormal;
                break;
            }
            case EN_ATMOS_STA_INV_CLOSE:
            {
                garrDevAtmosCfg[0] = _gstAtmosCfgInvHalt;
                break;
            }
            case EN_ATMOS_STA_INV_STANDBY:
            {
                garrDevAtmosCfg[0] = _gstAtmosCfgInvStandby;
                break;
            }
            case EN_ATMOS_STA_INV_CHECK:
            {
                garrDevAtmosCfg[0] = _gstAtmosCfgInvSelfCheck;
                break;
            }
            case EN_ATMOS_STA_INV_OFFGRID:
            {
                garrDevAtmosCfg[0] = _gstAtmosCfgInvOFFGridWork;
                break;
            }
            case EN_ATMOS_STA_INV_BYPASS:
            {
                garrDevAtmosCfg[0] = _gstAtmosCfgInvBypass;
                break;
            }
            default:
                break;
        }
        _gstSystemWorkMode._AtmostatusInvPre = _gstSystemWorkMode._AtmostatusInv;
    }
}


void WifiStatus(void)
{
    if(_gstTest.wifi_sta == 0)  //正常状态
    {
        _gstSystemWorkMode._AtmostatusWifi = EN_ATMOS_STA_WIFI_NORMAL;
    }
    else if(_gstTest.wifi_sta == 1)  //未连接状态
    {
        _gstSystemWorkMode._AtmostatusWifi = EN_ATMOS_STA_WIFI_DISCONN;
    }
    else if(_gstTest.wifi_sta == 2)  //未连接云平台
    {
        _gstSystemWorkMode._AtmostatusWifi = EN_ATMOS_STA_WIFI_DISCONN_CLOUD;
    }
    else if(_gstTest.wifi_sta == 3)   //未连接监控卡
    {
        _gstSystemWorkMode._AtmostatusWifi = EN_ATMOS_STA_WIFI_DISCONN_MONITOR;
    }

    if(_gstSystemWorkMode._AtmostatusWifi != _gstSystemWorkMode._AtmostatusWifiPre)
    {
        switch(_gstSystemWorkMode._AtmostatusWifi)
        {
            case EN_ATMOS_STA_WIFI_NORMAL:
            {
                garrDevAtmosCfg[1] = _gstAtmosCfgWifiNormal;
                break;
            }
            case EN_ATMOS_STA_WIFI_DISCONN:
            {
                garrDevAtmosCfg[1] = _gstAtmosCfgWifiRouterUnconnected;
                break;
            }
            case EN_ATMOS_STA_WIFI_DISCONN_CLOUD:
            {
                garrDevAtmosCfg[1] = _gstAtmosCfgWifiCloudUnconnected;
                break;
            }
            case EN_ATMOS_STA_WIFI_DISCONN_MONITOR:
            {
                garrDevAtmosCfg[1] = _gstAtmosCfgWifiOffLine;
                break;
            }
            default:
                break;
        }
        _gstSystemWorkMode._AtmostatusWifiPre = _gstSystemWorkMode._AtmostatusWifi;
    }
}

void SysStatus(void)
{
    if(_gstTest.sys_sta == 0)  //正常状态
    {
        _gstSystemWorkMode._AtmostatusSys = EN_ATMOS_STA_SYS_NORMAL;
    }
    else if(_gstTest.sys_sta == 1)  //系统故障
    {
        _gstSystemWorkMode._AtmostatusSys = EN_ATMOS_STA_SYS_ERR;
    }
    else if(_gstTest.sys_sta == 2)  //系统告警
    {
        _gstSystemWorkMode._AtmostatusSys = EN_ATMOS_STA_SYS_WARNNING;
    }

    if(_gstSystemWorkMode._AtmostatusSys != _gstSystemWorkMode._AtmostatusSysPre)
    {
        switch(_gstSystemWorkMode._AtmostatusSys)
        {
            case EN_ATMOS_STA_SYS_NORMAL:
            {
                garrDevAtmosCfg[2] = _gstAtmosCfgSysNormal;
                break;
            }
            case EN_ATMOS_STA_SYS_ERR:
            {
                garrDevAtmosCfg[2] = _gstAtmosCfgSysAbnormal;
                break;
            }
            case EN_ATMOS_STA_SYS_WARNNING:
            {
                garrDevAtmosCfg[2] = _gstAtmosCfgSysWarnning;
                break;
            }
            default:
                break;
        }
        _gstSystemWorkMode._AtmostatusSysPre = _gstSystemWorkMode._AtmostatusSys;
    }
}


//获取pack包的状态
void SideLightSta(void)
{
    uint32_t i = 0;
    for(i = 0; i < PACK_NUMS; ++i)
    {
        if(_garrPackSta[i].pack_sta == 0)  //PACK正常
        {
            _garrPackSta[i].AtmostatusPack = EN_ATMOS_STA_PACK_NORMAL;
        }
        else if(_garrPackSta[i].pack_sta == 1)  //pack异常
        {
            _garrPackSta[i].AtmostatusPack = EN_ATMOS_STA_PACK_ERR;
        }

        if(_garrPackSta[i].AtmostatusPack != _garrPackSta[i].AtmostatusPackPre)
        {
            //刷新对应灯的颜色
            switch(_garrPackSta[i].AtmostatusPack)
            {
                case EN_ATMOS_STA_PACK_NORMAL:
                {
                    garrDevAtmosCfg[i] = _gstAtmosCfgNormal;
                    break;
                }
                case EN_ATMOS_STA_PACK_ERR:
                {
                    garrDevAtmosCfg[i] = _gstAtmosCfgFAULT;
                    break;
                }
                default:
                    break;
            }
        }
        //刷新状态
        _garrPackSta[i].AtmostatusPackPre = _garrPackSta[i].AtmostatusPack;
    }
}


void SetLedSwitchStatus(uint8_t front_switch, uint8_t side_led_switch)
{
    _gFrontPanelLedStatus = front_switch;
    _gSidePanelLedStatus = side_led_switch;
}

