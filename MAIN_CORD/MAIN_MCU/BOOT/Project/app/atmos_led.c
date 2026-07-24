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


#define ATMOS_LED_THEME_GREEN_COLOR				(0xF4005E)
#define ATMOS_LED_THEME_RED_COLOR				(0x17EA88)
#define ATMOS_LED_THEME_YELLOW_COLOR			(0x8CFF00)
#define PACK_LED_FAULT_COLOR					(0x11FF15)



static WorkMode SystemWorkMode;
static PackSta_t packsta[PACK_NUMS];

static TestStatus test;


/********业务**********/
//采集器运行正常
static const DevAtmosCfg_t _AtmosCfgWifiNormal = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//采集器未连接WIFI路由器
static const DevAtmosCfg_t _AtmosCfgWifiRouterUnconnected = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BLINK, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};

//采集器未连接云平台
static const DevAtmosCfg_t _AtmosCfgWifiCloudUnconnected = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36,0xB6FF36},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//采集器未连接监控卡
static const DevAtmosCfg_t _AtmosCfgWifiOffLine = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0x080810,0x101020,0x12122E,0x101020,0x080810,0x070707,0x080810},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};

//系统运行正常
static const DevAtmosCfg_t _AtmosCfgSysNormal = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E},
    //.data[0] = {0x1A0705,0x2C070F,0x3C0708,0x2C070F,0x1A0705,0x070707},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//系统异常（故障）
static const DevAtmosCfg_t _AtmosCfgSysAbnormal = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0x013305,0x01530E,0x018210,0x01530E,0x013305,0x070707,0x013305},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//系统告警
static const DevAtmosCfg_t _AtmosCfgSysWarnning = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0x0D0D00,0x181800,0x3F3F00,0x181800,0x0D0D00,0x070707,0x0D0D00},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//逆变器正常并网发电
static const DevAtmosCfg_t _AtmosCfgInvNormal = 
{
	.effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD,0xF100CD},
    //.data[0] = {0x1A0705,0x2C070F,0x3C0708,0x2C070F,0x1A0705,0x070707},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//逆变器停机
static const DevAtmosCfg_t _AtmosCfgInvHalt = 
{
	.effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0x080810,0x101020,0x12122E,0x101020,0x080810,0x070707,0x080810},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//逆变器待机
static const DevAtmosCfg_t _AtmosCfgInvStandby = 
{
	.effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0x0D0D00,0x181800,0x3F3F00,0x181800,0x0D0D00,0x070707,0x0D0D00},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//逆变器自检
static const DevAtmosCfg_t _AtmosCfgInvSelfCheck = 
{
	.effect_type = EN_ATMOS_EFFECT_TYPE_BREATH, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0x3F3F00,0x3F3F00,0x3F3F00,0x3F3F00,0x3F3F00,0x3F3F00,0x3F3F00},
    //.data[0] = {0x0D0D00,0x181800,0x3F3F00,0x181800,0x0D0D00,0x070707},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};
//逆变器离网运行
static const DevAtmosCfg_t _AtmosCfgInvOFFGridWork = 
{
	.effect_type = EN_ATMOS_EFFECT_TYPE_BREATH, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0x1A0705,0x2C070F,0x3C0708,0x2C070F,0x1A0705,0x070707,0x1A0705},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};
//逆变器旁路模式
static const DevAtmosCfg_t _AtmosCfgInvBypass = 
{
	.effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0x1A0705,0x2C070F,0x3C0708,0x2C070F,0x1A0705,0x070707,0x1A0705},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};

//开机  触摸按键
static const DevAtmosCfg_t _AtmosCfgKeyControl = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_START_BREATH, 
    .led_num = 7,
    .color_change_num = 1,
#if 0
	.data[0] = {0x180014, 0x180014, 0x180014, 0x180014, 0x180014, 0x180014},
    .data[1] = {0x1D0018, 0x1D0018, 0x1D0018, 0x1D0018, 0x1D0018, 0x1D0018},
    .data[2] = {0x3C0033, 0x3C0033, 0x3C0033, 0x3C0033, 0x3C0033, 0x3C0033},
	.data[3] = {0x44003E, 0x44003E, 0x44003E, 0x44003E, 0x44003E, 0x44003E},
	.data[4] = {0x780066, 0x780066, 0x780066, 0x780066, 0x780066, 0x780066},
	.data[5] = {0x8C0079, 0x8C0079, 0x8C0079, 0x8C0079, 0x8C0079, 0x8C0079},
	.data[6] = {0xE900C6, 0xE900C6, 0xE900C6, 0xE900C6, 0xE900C6, 0xE900C6},
	.data[7] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
	.data[8] = {0xA8008F, 0xA8008F, 0xA8008F, 0xA8008F, 0xA8008F, 0xA8008F},
	.data[9] = {0x600052, 0x600052, 0x600052, 0x600052, 0x600052, 0x600052},
	.data[10] = {0x180014, 0x180014, 0x180014, 0x180014, 0x180014, 0x180014},
#endif
	.data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
	
    .effect_period = 11000,
    .effect_one_color_times = 1,
};

//唤醒
static const DevAtmosCfg_t _AtmosCfgKeyWakeup = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_HALF_ON, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
	//.data[1] = {0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};
//待机
static const DevAtmosCfg_t _AtmosCfgKeyStandBy = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0x180014, 0x180014, 0x180014, 0x180014, 0x180014, 0x180014, 0x180014},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};

static const DevAtmosCfg_t _AtmosCfgTest = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[1] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[2] = {0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24},
    .data[3] = {0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};

static const DevAtmosCfg_t _AtmosCfgTest1 = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH, 
    .led_num = 7,
    .color_change_num = 4,
    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .data[1] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[2] = {0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24},
    .data[3] = {0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};

static const DevAtmosCfg_t _AtmosCfgTest2 = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BLINK, 
    .led_num = 7,
    .color_change_num = 4,
    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .data[1] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[2] = {0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24},
    .data[3] = {0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF},
    .effect_period = 500,
    .effect_one_color_times = 3,
};


static const DevAtmosCfg_t _AtmosCfgTest3 = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 4,
    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .data[1] = {0xF100CD, 0xF100CD, 0xF100CD, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .data[2] = {0x1CEC24, 0x1CEC24, 0x1CEC24, 0xC44CFF, 0xC44CFF, 0xC44CFF, 0xC44CFF},
    .data[3] = {0xC44CFF, 0xC44CFF, 0xC44CFF, 0x1CEC24, 0x1CEC24, 0x1CEC24, 0x1CEC24},
    .effect_period = 500,
    .effect_one_color_times = 3,
};

static const DevAtmosCfg_t _AtmosCfgTest4 = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 6,
    .data[0] = {0x7F0000, 0x007F00, 0x00007F, 0x007F7F, 0x7F007F, 0x7F7F00, 0x7F7F00},
    .data[1] = {0x007F00, 0x00007F, 0x007F7F, 0x7F007F, 0x7F7F00, 0x7F0000, 0x7F0000},
    .data[2] = {0x00007F, 0x007F7F, 0x7F007F, 0x7F7F00, 0x7F0000, 0x007F00, 0x007F00},
	.data[3] = {0x007F7F, 0x7F007F, 0x7F7F00, 0x7F0000, 0x007F00, 0x00007F, 0x00007F},
    .data[4] = {0x7F007F, 0x7F7F00, 0x7F0000, 0x007F00, 0x00007F, 0x007F7F, 0x007F7F},
    .data[5] = {0x7F7F00, 0x7F0000, 0x007F00, 0x00007F, 0x007F7F, 0x7F007F, 0x7F007F},
    .effect_period = 1000,
    .effect_one_color_times = 1,
};

//紫红
static const DevAtmosCfg_t _AtmosCfgHorseLamp1 = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 6,
    .data[0] = {0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x12B96C, 0x17EA88, 0x17EA88},
    .data[1] = {0x17EA88, 0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x12B96C, 0x12B96C},
    .data[2] = {0x12B96C, 0x17EA88, 0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x0D8B51},
	.data[3] = {0x0D8B51, 0x12B96C, 0x17EA88, 0x02170D, 0x042E1B, 0x095C36, 0x095C36},
    .data[4] = {0x095C36, 0x0D8B51, 0x12B96C, 0x17EA88, 0x02170D, 0x042E1B, 0x042E1B},
    .data[5] = {0x042E1B, 0x095C36, 0x0D8B51, 0x12B96C, 0x17EA88, 0x02170D, 0x02170D},
    .effect_period = 200,
    .effect_one_color_times = 1,
};
//黄色
static const DevAtmosCfg_t _AtmosCfgHorseLamp2 = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 6,
    .data[0] = {0x121905, 0x24330A, 0x486615, 0x6D9920, 0x91CC2B, 0xB6FF36, 0xB6FF36},
    .data[1] = {0xB6FF36, 0x121905, 0x24330A, 0x486615, 0x6D9920, 0x91CC2B, 0x91CC2B},
    .data[2] = {0x91CC2B, 0xB6FF36, 0x121905, 0x24330A, 0x486615, 0x6D9920, 0x6D9920},
	.data[3] = {0x6D9920, 0x91CC2B, 0xB6FF36, 0x121905, 0x24330A, 0x486615, 0x486615},
    .data[4] = {0x486615, 0x6D9920, 0x91CC2B, 0xB6FF36, 0x121905, 0x24330A, 0x24330A},
    .data[5] = {0x24330A, 0x486615, 0x6D9920, 0x91CC2B, 0xB6FF36, 0x121905, 0x121905},
    .effect_period = 200,
    .effect_one_color_times = 1,
};
//蓝色
static const DevAtmosCfg_t _AtmosCfgHorseLamp3 = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 6,
	.data[0] = {0x180014, 0x3A0031, 0x44003E, 0x780066, 0xD300A6, 0xF100CD, 0xF100CD},
    .data[1] = {0xF100CD, 0x180014, 0x3A0031, 0x44003E, 0x780066, 0xD300A6, 0xD300A6},
    .data[2] = {0xD300A6, 0xF100CD, 0x180014, 0x3A0031, 0x44003E, 0x780066, 0x780066},
	.data[3] = {0x780066, 0xD300A6, 0xF100CD, 0x180014, 0x3A0031, 0x44003E, 0x44003E},
    .data[4] = {0x44003E, 0x780066, 0xD300A6, 0xF100CD, 0x180014, 0x3A0031, 0x3A0031},
    .data[5] = {0x3A0031, 0x44003E, 0x780066, 0xD300A6, 0xF100CD, 0x180014, 0x180014},
    .effect_period = 200,
    .effect_one_color_times = 1,
};

//拖尾
static const DevAtmosCfg_t _AtmosCfgTrail =
{
	.effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 10,
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
    .effect_period = 100,
    .effect_one_color_times = 1,
};
//正常
static const DevAtmosCfg_t _AtmosCfgNormal = 
{
	.effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .effect_period = 200,
    .effect_one_color_times = 1,
};

//故障
static const DevAtmosCfg_t _AtmosCfgFAULT = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15, 0x11FF15},
    .effect_period = 200,
    .effect_one_color_times = 1,
};

//绿色
static const DevAtmosCfg_t _AtmosCfgHorseGreen = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E, 0xF4005E},
    .effect_period = 500,
    .effect_one_color_times = 1,
};

//蓝色
static const DevAtmosCfg_t _AtmosCfgHorseBlue = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .effect_period = 500,
    .effect_one_color_times = 1,
};
//黄色
static const DevAtmosCfg_t _AtmosCfgHorseYellow = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
//    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
//    .data[0] = {0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E},	
	.data[0] = {0x8CFF00, 0x8CFF00, 0x8CFF00, 0x8CFF00, 0x8CFF00, 0x8CFF00, 0x8CFF00},
    .effect_period = 500,
    .effect_one_color_times = 1,
};
//紫红
static const DevAtmosCfg_t _AtmosCfgHorseRed = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88},
    .effect_period = 500,
    .effect_one_color_times = 1,
};

//蓝色呼吸
static const DevAtmosCfg_t _AtmosCfgHorseBluebreath = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};
//黄色呼吸
static const DevAtmosCfg_t _AtmosCfgHorseYellowbreath = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH, 
    .led_num = 7,
    .color_change_num = 1,
//    .data[0] = {0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36, 0xB6FF36},
    .data[0] = {0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E, 0xC6F81E},	
    .effect_period = 2000,
    .effect_one_color_times = 1,
};
//紫红呼吸
static const DevAtmosCfg_t _AtmosCfgHorseRedbreath = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_BREATH, 
    .led_num = 7,
    .color_change_num = 1,
    .data[0] = {0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88},
    .effect_period = 2000,
    .effect_one_color_times = 1,
};

//开机 单色
static const DevAtmosCfg_t _AtmosCfgHorseStar = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON, 
    .led_num = 7,
    .color_change_num = 6,
    .data[0] = {0x24001E, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
	.data[1] = {0x24001E, 0x600052, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[2] = {0x24001E, 0x600052, 0xA8008F, 0x000000, 0x000000, 0x000000},
    .data[3] = {0x24001E, 0x600052, 0xA8008F, 0xF100CD, 0x000000, 0x000000},
	.data[4] = {0x24001E, 0x600052, 0xA8008F, 0xF100CD, 0xA8008F, 0x000000},
	.data[5] = {0x24001E, 0x600052, 0xA8008F, 0xF100CD, 0xA8008F, 0x600052},
    .effect_period = 80,
    .effect_one_color_times = 1,
};
//开机 多色
static const DevAtmosCfg_t _AtmosCfgHorseStarB = 
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
static const DevAtmosCfg_t _AtmosCfgHorseStarR = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON, 
    .led_num = 7,
    .color_change_num = 6,
    .data[0] = {0x02170D, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
	.data[1] = {0x02170D, 0x042E1B, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[2] = {0x02170D, 0x042E1B, 0x095C36, 0x000000, 0x000000, 0x000000},
    .data[3] = {0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x000000, 0x000000},
	.data[4] = {0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x12B96C, 0x000000},
	.data[5] = {0x02170D, 0x042E1B, 0x095C36, 0x0D8B51, 0x12B96C, 0x17E888},
    .effect_period = 80,
    .effect_one_color_times = 1,
};
//开机 多色
static const DevAtmosCfg_t _AtmosCfgHorseStarY = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON, 
    .led_num = 7,
    .color_change_num = 6,
    .data[0] = {0x121905, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000},
	.data[1] = {0x121905, 0x24330A, 0x000000, 0x000000, 0x000000, 0x000000},
    .data[2] = {0x121905, 0x24330A, 0x486615, 0x000000, 0x000000, 0x000000},
    .data[3] = {0x121905, 0x24330A, 0x486615, 0x6D9920, 0x000000, 0x000000},
	.data[4] = {0x121905, 0x24330A, 0x486615, 0x6D9920, 0x91CC2B, 0x000000},
	.data[5] = {0x121905, 0x24330A, 0x486615, 0x6D9920, 0x91CC2B, 0xB6FF36},
    .effect_period = 80,
    .effect_one_color_times = 1,
};

//20%紫红
static const DevAtmosCfg_t _AtmosCfgHorseRed20 = 
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_THIRD_BREATH, 
    .led_num = 7,
    .color_change_num = 1,
	.data[0] = {0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88, 0x17EA88},  //紫红
//	.data[0] = {0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD, 0xF100CD},	//蓝绿色
    .effect_period = 2600,
    .effect_one_color_times = 1,
	.effect_time_count = 3,
};

void AtmosLedFuncTest(void)
{
#if 0
    gDevAtmosCfg[0] = _AtmosCfgWifiCloudUnconnected;
    gDevAtmosCfg[1] = _AtmosCfgTest1;
    gDevAtmosCfg[2] = _AtmosCfgTest3;
#endif
	
#if 0
    gDevAtmosCfg[0] = _AtmosCfgHorseStarB;
    gDevAtmosCfg[1] = _AtmosCfgHorseStarR;
    gDevAtmosCfg[2] = _AtmosCfgHorseStarY;                                                                                  
#endif
	
#if 0
	gDevAtmosCfg[0] = _AtmosCfgHorseGreen;
    gDevAtmosCfg[1] = _AtmosCfgHorseYellow;
    gDevAtmosCfg[2] = _AtmosCfgHorseRed;
#endif	
	
#if 0
	gDevAtmosCfg[0] = _AtmosCfgHorseGreen;
    gDevAtmosCfg[1] = _AtmosCfgHorseRed;
    gDevAtmosCfg[2] = _AtmosCfgHorseYellow;
#endif	

#if 0 
	packsta[0].pack_sta = 0;
	packsta[1].pack_sta = 0;
	packsta[2].pack_sta = 0;
	
#endif

	
#if 0
	test.sys_sta = 0;
	test.inv_sta = 0;
	test.wifi_sta = 0;
#endif
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
				packsta[0].pack_sta = 0;
				packsta[1].pack_sta = 1;
				packsta[2].pack_sta = 1;
				break;
			}
			case 1:
			{
				packsta[0].pack_sta = 1;
				packsta[1].pack_sta = 0;
				packsta[2].pack_sta = 1;
				break;
			}
			case 2:
			{
				packsta[0].pack_sta = 1;
				packsta[1].pack_sta = 1;
				packsta[2].pack_sta = 0;
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
    //根据实际业务判断是否更新gDevAtmosCfg
	static uint32_t count = 0;
	uint16_t poll_period = TimerPollingHandle.timer_cfg->period / 3000;
	
	
	if(count % 3000 == 0)
	{
		switch(count / 3000)
		{
			case 0:
			{
				gDevAtmosCfg[0] = _AtmosCfgHorseGreen;
				gDevAtmosCfg[1] = _AtmosCfgHorseGreen;
				gDevAtmosCfg[2] = _AtmosCfgHorseGreen;
				break;
			}
			case 1:
			{
				gDevAtmosCfg[0] = _AtmosCfgHorseBlue;
				gDevAtmosCfg[1] = _AtmosCfgHorseYellow;
				gDevAtmosCfg[2] = _AtmosCfgHorseRed;
				break;
			}
			case 2:
			{
				gDevAtmosCfg[0] = _AtmosCfgTest;
				gDevAtmosCfg[1] = _AtmosCfgTest2;
				gDevAtmosCfg[2] = _AtmosCfgFAULT;
				break;
			}
			case 3:
			{
				gDevAtmosCfg[0] = _AtmosCfgHorseBluebreath;
				gDevAtmosCfg[1] = _AtmosCfgHorseYellowbreath;
				gDevAtmosCfg[2] = _AtmosCfgHorseRedbreath;
				break;
			}
			default:break;
		}
	}
	count++;
	if(count / 3000 > 3)
	{
		count = 0;
	}
}


void AtmosLedFuncUpdate(void)
{
    //根据实际业务判断是否更新gDevAtmosCfg

#if 0
	InvStatus();
	WifiStatus();
	SysStatus();
#endif
	
#if 1
	gDevAtmosCfg[0] = _AtmosCfgHorseGreen;
	gDevAtmosCfg[1] = _AtmosCfgHorseRed;
	gDevAtmosCfg[2] = _AtmosCfgHorseYellow;
#endif
	
#if 0
	SideLightSta();
	
#endif
	
}

void InvStatus(void)
{
	if(test.inv_sta == 0)  //正常状态
	{
		SystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_NORMAL;
	}
	else if(test.inv_sta ==1)  //逆变器停机
	{
		SystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_CLOSE;
	}
	else if(test.inv_sta == 2)  //逆变器起待机
	{
		SystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_STANDBY;
	}
	else if(test.inv_sta == 3)   //逆变器自检
	{
		SystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_CHECK;
	}
	else if(test.inv_sta == 4)   //逆变器离网运行
	{
		SystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_OFFGRID;
	}
	else if(test.inv_sta == 5)   //逆变器旁路模式
	{
		SystemWorkMode._AtmostatusInv = EN_ATMOS_STA_INV_BYPASS;
	}
	
	
	if(SystemWorkMode._AtmostatusInv != SystemWorkMode._AtmostatusInvPre)
	{
		switch(SystemWorkMode._AtmostatusInv)
		{
			case EN_ATMOS_STA_INV_NORMAL:
			{
				gDevAtmosCfg[0] = _AtmosCfgSysNormal;
				break;
			}
			case EN_ATMOS_STA_INV_CLOSE:
			{
				gDevAtmosCfg[0] = _AtmosCfgInvHalt;
				break;
			}
			case EN_ATMOS_STA_INV_STANDBY:
			{
				gDevAtmosCfg[0] = _AtmosCfgInvStandby;
				break;
			}
			case EN_ATMOS_STA_INV_CHECK:
			{
				gDevAtmosCfg[0] = _AtmosCfgInvSelfCheck;
				break;
			}
			case EN_ATMOS_STA_INV_OFFGRID:
			{
				gDevAtmosCfg[0] = _AtmosCfgInvOFFGridWork;
				break;
			}
			case EN_ATMOS_STA_INV_BYPASS:
			{
				gDevAtmosCfg[0] = _AtmosCfgInvBypass;
				break;
			}
			default:break;
		}
		SystemWorkMode._AtmostatusInvPre = SystemWorkMode._AtmostatusInv;
	}
	
}



void WifiStatus(void)
{
	if(test.wifi_sta == 0)  //正常状态
	{
		SystemWorkMode._AtmostatusWifi = EN_ATMOS_STA_WIFI_NORMAL;
	}
	else if(test.wifi_sta == 1)  //未连接状态
	{
		SystemWorkMode._AtmostatusWifi = EN_ATMOS_STA_WIFI_DISCONN;
	}
	else if(test.wifi_sta == 2)  //未连接云平台
	{
		SystemWorkMode._AtmostatusWifi = EN_ATMOS_STA_WIFI_DISCONN_CLOUD;
	}
	else if(test.wifi_sta == 3)   //未连接监控卡
	{
		SystemWorkMode._AtmostatusWifi = EN_ATMOS_STA_WIFI_DISCONN_MONITOR;
	}
	
	if(SystemWorkMode._AtmostatusWifi != SystemWorkMode._AtmostatusWifiPre)
	{
		switch(SystemWorkMode._AtmostatusWifi)
		{
			case EN_ATMOS_STA_WIFI_NORMAL:
			{
				gDevAtmosCfg[1] = _AtmosCfgWifiNormal;
				break;
			}
			case EN_ATMOS_STA_WIFI_DISCONN:
			{
				gDevAtmosCfg[1] = _AtmosCfgWifiRouterUnconnected;
				break;
			}
			case EN_ATMOS_STA_WIFI_DISCONN_CLOUD:
			{
				gDevAtmosCfg[1] = _AtmosCfgWifiCloudUnconnected;
				break;
			}
			case EN_ATMOS_STA_WIFI_DISCONN_MONITOR:
			{
				gDevAtmosCfg[1] = _AtmosCfgWifiOffLine;
				break;
			}
			default:break;
		}
		SystemWorkMode._AtmostatusWifiPre = SystemWorkMode._AtmostatusWifi;
	}
	
	
}

void SysStatus(void)
{
	if(test.sys_sta == 0)  //正常状态
	{
		SystemWorkMode._AtmostatusSys = EN_ATMOS_STA_SYS_NORMAL;
	}
	else if(test.sys_sta == 1)  //系统故障
	{
		SystemWorkMode._AtmostatusSys = EN_ATMOS_STA_SYS_ERR;
	}
	else if(test.sys_sta == 2)  //系统告警
	{
		SystemWorkMode._AtmostatusSys = EN_ATMOS_STA_SYS_WARNNING;
	}
	
	if(SystemWorkMode._AtmostatusSys != SystemWorkMode._AtmostatusSysPre)
	{
		switch(SystemWorkMode._AtmostatusSys)
		{
			case EN_ATMOS_STA_SYS_NORMAL:
			{
				gDevAtmosCfg[2] = _AtmosCfgSysNormal;
				break;
			}
			case EN_ATMOS_STA_SYS_ERR:
			{
				gDevAtmosCfg[2] = _AtmosCfgSysAbnormal;
				break;
			}
			case EN_ATMOS_STA_SYS_WARNNING:
			{
				gDevAtmosCfg[2] = _AtmosCfgSysWarnning;
				break;
			}
			default:break;
		}
		SystemWorkMode._AtmostatusSysPre = SystemWorkMode._AtmostatusSys;
	}

}


//获取pack包的状态
void SideLightSta(void)
{
	uint32_t i = 0;
	for(i = 0; i < PACK_NUMS; ++i)
	{
		if(packsta[i].pack_sta == 0)  //PACK正常
		{
			packsta[i].AtmostatusPack = EN_ATMOS_STA_PACK_NORMAL;
		}
		else if(packsta[i].pack_sta == 1)  //pack异常
		{
			packsta[i].AtmostatusPack = EN_ATMOS_STA_PACK_ERR;
		}

		if(packsta[i].AtmostatusPack != packsta[i].AtmostatusPackPre)
		{
			//刷新对应灯的颜色
			switch(packsta[i].AtmostatusPack)
			{
				case EN_ATMOS_STA_PACK_NORMAL:
				{
					gDevAtmosCfg[i] = _AtmosCfgNormal;
					break;
				}
				case EN_ATMOS_STA_PACK_ERR:
				{
					gDevAtmosCfg[i] = _AtmosCfgFAULT;
					break;
				}
				default:
					break;
			}
		}
		//刷新状态
		packsta[i].AtmostatusPackPre = packsta[i].AtmostatusPack;
	}	
}



