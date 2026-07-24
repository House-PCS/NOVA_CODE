/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_main.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-12-18
  * @brief       3D版本导航界面
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-12-18    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"
#include "em_main.h"
#include <time.h>

#define LAYOUT_DEBUG                                0
#define TEST_DYNAMIC                                0
#define TEST_MEMORY                                 1

#define EM_MAIN_ICON_BACKGROUND_SIZE                (452 * 1024)
#define EM_MAIN_ELEC_STAT_ICON_GRID_CHARGE_SIZE     (4 * 1024)
#define EM_MAIN_ELEC_STAT_ICON_GRID_TIED_SIZE       (4 * 1024)
#define EM_MAIN_ELEC_STAT_ICON_PV_CHARGE_SIZE       (4 * 1024)
#define EM_MAIN_ELEC_STAT_ICON_LOAD_SIZE            (4 * 1024)

#define EM_MAIN_TEMP_ICON_SIZE                      (4 * 1024)

#define EM_MAIN_DEVICE_ICON_ROCA_SIZE               (64 * 1024)
#define EM_MAIN_DEVICE_ICON_GRID_CHARGE_SIZE        (4 * 1024)
#define EM_MAIN_DEVICE_ICON_PV_CHARGE_SIZE          (4 * 1024)
#define EM_MAIN_DEVICE_ICON_BACKUP_SIZE             (4 * 1024)
#define EM_MAIN_DEVICE_ICON_INDICATE_SIZE           (4 * 1024)

#define EM_MAIN_CHARGE_ICON_GRID_SIZE               (4 * 1024)
#define EM_MAIN_CHARGE_ICON_PV_SIZE                 (4 * 1024)
#define EM_MAIN_CHARGE_ICON_GRID_PV_SIZE            (4 * 1024)

#define EM_MAIN_STAT_BAR_WIFI_ICON_SIZE             (4 * 1024)

typedef struct gstUiCtx
{
    uint16_t work_mode;
    uint16_t electic_purchase_day;
    uint16_t electic_sale_day;
    uint32_t pv_sum_generation_on_day;
    uint16_t load_cost_day;
    uint16_t soc;
    uint16_t charge_type;
    uint16_t chassis_internal_temp;
    uint16_t net_state;
    uint32_t time;
} UiCtx_t;

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinMain = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

static UiCtx_t _gstUiCtx;
/* 背景图内存 */
static GUI_HMEM _gMemBK;
static uint8_t *_pBKBuf;


/*各个子窗口控件句柄及其他相关参数*/
/*电量统计子窗口*/
static GUI_HWIN _gWinElecStatHandle;//电量统计窗口控件句柄
static TEXT_Handle  _gTextElecStatTitle1Handle;//电量统计之标题1句柄
static TEXT_Handle  _gTextElecStatTitle2Handle;//电量统计之标题2句柄
static IMAGE_Handle _gImgElecStatGridChargeHandle;//电量统计之市电充电图标句柄
static TEXT_Handle  _gTextElecStatGridChargeHandle;//电量统计之市电充电数值句柄
static IMAGE_Handle _gImgElecStatGridTiedHandle;//电量统计之并网图标句柄
static TEXT_Handle  _gTextElecStatGridTiedHandle;//电量统计之并网数值句柄
static IMAGE_Handle _gImgElecStatPVChargeHandle;//电量统计之光伏充电图标句柄
static TEXT_Handle  _gTextElecStatPVChargeHandle;//电量统计之光伏充电数值句柄
static IMAGE_Handle _gImgElecStatLoadHandle;//电量统计之负载图标句柄
static TEXT_Handle  _gTextElecStatLoadHandle;//电量统计之负载数值句柄

/*电量统计窗口图标参数*/
static emWidgetImgPara_t  _gElecStatGridChargeImgPara;
static emWidgetImgPara_t  _gElecStatGridTiedImgPara;
static emWidgetImgPara_t  _gElecStatPVChargeImgPara;
static emWidgetImgPara_t  _gElecStatLoadImgPara;


/*电量统计窗口文字参数*/
static emWidgetTextPara_t  _gElecStatTitle1TextPara;
static emWidgetTextPara_t  _gElecStatTitle2TextPara;
static emWidgetTextPara_t  _gElecStatGridChargeTextPara;
static emWidgetTextPara_t  _gElecStatGridTiedTextPara;
static emWidgetTextPara_t  _gElecStatPVChargeTextPara;
static emWidgetTextPara_t  _gElecStatLoadTextPara;

static char _gElecStatTitle1TextBuf[] = "Today's E";
static char _gElecStatTitle2TextBuf[] = "kWh";
static char _gElecStatGridChargeTextBuf[8];
static char _gElecStatGridTiedTextBuf[8];
static char _gElecStatPVChargeTextBuf[8];
static char _gElecStatLoadTextBuf[8];

/*温度子窗口*/
static GUI_HWIN _gWinTempHandle;//温度句柄
static TEXT_Handle  _gTextTempKeyHandle;//温度标题句柄
static TEXT_Handle  _gTextTempKeyMesureHandle;//温度标题句柄
static TEXT_Handle  _gTextTempValueTextHandle;//温度值句柄
static IMAGE_Handle _gImgTempHandle;//温度图标句柄
/*温度窗口图标参数*/
static emWidgetImgPara_t  _gTempImgPara[3]; // 0:normal, 1:col, 2:hot
/*温度窗口文字参数*/
static emWidgetTextPara_t  _gTempKeyTextPara;
static emWidgetTextPara_t  _gTempKeyMesureTextPara;
static emWidgetTextPara_t  _gTempValueTextPara;

static char _gTempKeyTextBuf[] = "Opt Temp";
static char _gTempKeyMesureTextBuf[8] = "F";
static char _gTempValueTextBuf[8] = "112";

/*设备模型子窗口*/
static GUI_HWIN _gWinDeviceHandle;//设备模型窗口控件句柄
static IMAGE_Handle _gImgDeviceModelHandle;//电量统计之市电充电图标句柄
static IMAGE_Handle _gImgDeviceGridChargeHandle;//电量统计之市电充电图标句柄
static IMAGE_Handle _gImgDevicePvHandle;//电量统计之市电充电图标句柄
static IMAGE_Handle _gImgDeviceBackupHandle;//电量统计之市电充电图标句柄
static IMAGE_Handle _gImgDeviceIndicateHandle[3];//电量统计之负载图标句柄
/*设备模型窗口图标参数*/
static emWidgetImgPara_t  _gDeviceModelImgPara;
static emWidgetImgPara_t  _gDeviceGridChargeImgPara[2]; // 0:disable, 1:enable
static emWidgetImgPara_t  _gDevicePvChargeImgPara[2];   //
static emWidgetImgPara_t  _gDeviceBackupImgPara[2];     //
static emWidgetImgPara_t  _gDeviceIndicateImgPara[2]; // 0:GRID, 1:PV, 2:BACKUP


/*工作模式子窗口*/
static GUI_HWIN _gWinWorkModeHandle;//工作模式句柄
static TEXT_Handle  _gTextWorkModeTitleHandle;//工作模式标题句柄
static TEXT_Handle  _gTextWorkModeContentHandle;//工作模式内容句柄

/*工作模式窗口文字参数*/
static emWidgetTextPara_t  _gWorkModeTitleTextPara;
static emWidgetTextPara_t  _gWorkModeContentTextPara;

static char _gWorkModeTitleTextBuf[] = "Working Mode";
static char _gWorkModeContentTextBuf[32] = {0};

/*LED 状态灯子窗口*/
static GUI_HWIN _gWinLedStatHandle;//工作模式句柄
static TEXT_Handle  _gTextLedStatTitle1Handle;//工作模式标题句柄
static TEXT_Handle  _gTextLedStatTitle2Handle;//工作模式标题句柄
static TEXT_Handle  _gTextLedStatTitle3Handle;//工作模式标题句柄

/*LED 状态灯窗口文字参数*/
static emWidgetTextPara_t  _gLedStatTitle1TextPara;
static emWidgetTextPara_t  _gLedStatTitle2TextPara;
static emWidgetTextPara_t  _gLedStatTitle3TextPara;

static char _gLedStatTitle1TextBuf[] = "FAULT";
static char _gLedStatTitle2TextBuf[] = "STATUS";
static char _gLedStatTitle3TextBuf[] = "GRID";
static GUI_COLOR _gLedStatLed[3] = {0x00AAAAAA, 0x00AAAAAA, 0x00AAAAAA};

/*充电量子窗口*/
EN_CHARGE_TYPE_E _gChargeType = EN_CHARGE_TYPE_NONE;
static GUI_HWIN _gWinChargeHandle;//充电量句柄
static IMAGE_Handle _gImgChargeTypeHandle;//充电类型图标句柄
static TEXT_Handle  _gTextChargePercentHandle;//充电百分比句柄
static TEXT_Handle  _gTextChargePvHandle;//PV充电模式内容句柄
static TEXT_Handle  _gTextChargeGridHandle;//GRID充电模式内容句柄

/*充电窗口文字参数*/
static emWidgetImgPara_t  _gChargeTypeImgPara[3]; // 0:grid, 1:pv, 2:grid&pv
static emWidgetTextPara_t  _gChargePercentTextPara;
static emWidgetTextPara_t  _gChargePvTextPara;
static emWidgetTextPara_t  _gChargeGridTextPara;

static char _gChargePercentTextBuf[8] = "78%";
static char _gChargePvTextBuf[32] = {"PV Charging"};
static char _gChargeGridTextBuf[32] = {"GRID Charging"};

/*状态栏子窗口*/
static GUI_HWIN _gWinStatBarHandle;//状态栏句柄
static IMAGE_Handle _gImgStatBarWifiHandle;//状态栏WIFI图标句柄
static TEXT_Handle  _gTextStatBarTimeHandle;//状态栏时间句柄

/*状态栏窗口文字参数*/
static emWidgetImgPara_t  _gStatBarWifiImgPara[2];      // 0:disconnect, 1:connect
static emWidgetTextPara_t  _gStatBarTimeTextPara;

static char _gStatBarTimeTextBuf[80] = {0}; // 2024 Jan 1th 00:00:01


/*电量统计控件ID号*/
#define WIN_MAIN_ELEC_STAT_ID                               (GUI_ID_USER + 0x10)
#define WIN_MAIN_ELEC_STAT_GRID_CHARGE_IMG_ID               (GUI_ID_USER + 0x11)
#define WIN_MAIN_ELEC_STAT_GRID_CHARGE_TEXT_ID              (GUI_ID_USER + 0x12)
#define WIN_MAIN_ELEC_STAT_GRID_TIED_IMG_ID                 (GUI_ID_USER + 0x13)
#define WIN_MAIN_ELEC_STAT_GRID_TIED_TEXT_ID                (GUI_ID_USER + 0x14)
#define WIN_MAIN_ELEC_STAT_PV_CHARGE_IMG_ID                 (GUI_ID_USER + 0x15)
#define WIN_MAIN_ELEC_STAT_PV_CHARGE_TEXT_ID                (GUI_ID_USER + 0x16)
#define WIN_MAIN_ELEC_STAT_LOAD_IMG_ID                      (GUI_ID_USER + 0x17)
#define WIN_MAIN_ELEC_STAT_LOAD_TEXT_ID                     (GUI_ID_USER + 0x18)
#define WIN_MAIN_ELEC_STAT_TITLE1_TEXT_ID                   (GUI_ID_USER + 0x19)
#define WIN_MAIN_ELEC_STAT_TITLE2_TEXT_ID                   (GUI_ID_USER + 0x1A)
/*温度控件ID号*/
#define WIN_MAIN_TEMP_ID                                    (GUI_ID_USER + 0x30)
#define WIN_MAIN_TEMP_KEY_TEXT_ID                           (GUI_ID_USER + 0x31)
#define WIN_MAIN_TEMP_KEY_MESURE_TEXT_ID                    (GUI_ID_USER + 0x32)
#define WIN_MAIN_TEMP_VALUE_TEXT_ID                         (GUI_ID_USER + 0x33)
#define WIN_MAIN_TEMP_IMG1_ID                               (GUI_ID_USER + 0x34)
/*设备模型控件ID号*/
#define WIN_MAIN_DEVICE_ID                                  (GUI_ID_USER + 0x40)
#define WIN_MAIN_DEVICE_MODEL_ID                            (GUI_ID_USER + 0x41)
#define WIN_MAIN_DEVICE_GRID_CHARGE_ID                      (GUI_ID_USER + 0x42)
#define WIN_MAIN_DEVICE_PV_CHARGE_ID                        (GUI_ID_USER + 0x43)
#define WIN_MAIN_DEVICE_BACKUP_ID                           (GUI_ID_USER + 0x44)
#define WIN_MAIN_DEVICE_GRID_INDICATE_IMG_ID                (GUI_ID_USER + 0x45)
#define WIN_MAIN_DEVICE_PV_INDICATE_IMG_ID                  (GUI_ID_USER + 0x46)
#define WIN_MAIN_DEVICE_BACKUP_INDICATE_IMG_ID              (GUI_ID_USER + 0x47)
/*工作模式控件ID号*/
#define WIN_MAIN_WORK_MODE_ID                               (GUI_ID_USER + 0x50)
#define WIN_MAIN_WORK_MODE_TITLE_TEXT_ID                    (GUI_ID_USER + 0x51)
#define WIN_MAIN_WORK_MODE_CONTENT_TEXT_ID                  (GUI_ID_USER + 0x52)
/*LED 状态灯*/
#define WIN_MAIN_LED_STAT_ID                                (GUI_ID_USER + 0x70)
#define WIN_MAIN_LED_STAT_TITLE1_TEXT_ID                    (GUI_ID_USER + 0x71)
#define WIN_MAIN_LED_STAT_TITLE2_TEXT_ID                    (GUI_ID_USER + 0x72)
#define WIN_MAIN_LED_STAT_TITLE3_TEXT_ID                    (GUI_ID_USER + 0x73)
#define WIN_MAIN_LED_STAT_LED1_ID                           (GUI_ID_USER + 0x74)
#define WIN_MAIN_LED_STAT_LED2_ID                           (GUI_ID_USER + 0x75)
#define WIN_MAIN_LED_STAT_LED3_ID                           (GUI_ID_USER + 0x76)
/*右下充电量控件ID号*/
#define WIN_MAIN_CHARGE_ID                                  (GUI_ID_USER + 0x90)
#define WIN_MAIN_CHARGE_TYPE_IMG_ID                         (GUI_ID_USER + 0x91)
#define WIN_MAIN_CHARGE_PERCENT_TEXT_ID                     (GUI_ID_USER + 0x92)
#define WIN_MAIN_CHARGE_PV_TEXT_ID                          (GUI_ID_USER + 0x93)
#define WIN_MAIN_CHARGE_GRID_TEXT_ID                        (GUI_ID_USER + 0x94)
/*底部状态栏控件ID号*/
#define WIN_MAIN_STAT_BAR_ID                                (GUI_ID_USER + 0xA0)
#define WIN_MAIN_STAT_BAR_WIFI_IMG_ID                       (GUI_ID_USER + 0xA1)
#define WIN_MAIN_STAT_BAR_TIME_TEXT_ID                      (GUI_ID_USER + 0xA2)

/* layout 背景色*/
#define GUI_COLOR_WIN_MAIN_ELEC_STAT                        GUI_MAKE_COLOR(0x00363A23)
#define GUI_COLOR_WIN_MAIN_WORK_MODE                        GUI_MAKE_COLOR(0x00363A23)
#define GUI_COLOR_WIN_MAIN_LED_STAT                         GUI_MAKE_COLOR(0x00363A23)
#define GUI_COLOR_WIN_MAIN_CHARGE                           GUI_MAKE_COLOR(0x00363A23)
#define GUI_COLOR_WIN_MAIN_STAT_BAR                         GUI_MAKE_COLOR(0x00242717)

#define GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_PEN_SIZE           5
#define GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_BORDER            GUI_MAKE_COLOR(0X005A7105)
#define GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_DISABLE            GUI_MAKE_COLOR(0x00949494)
#define GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_ENABLE             GUI_MAKE_COLOR(0X00B2BE00)

#define GUI_COLOR_WIN_MAIN_CHARGE_CIRCLE_PEN_SIZE           10
#define GUI_COLOR_WIN_MAIN_CHARGE_CIRCLE_BG_1               GUI_MAKE_COLOR(0x00363A23)
#define GUI_COLOR_WIN_MAIN_CHARGE_CIRCLE_BG_2               GUI_MAKE_COLOR(0x0036B6FF)



/*资源列表*/
static const GUI_WIDGET_CREATE_INFO _aElecStatDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ElecStatWin",    WIN_MAIN_ELEC_STAT_ID,                   15,   12,  115,  165,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title1",         WIN_MAIN_ELEC_STAT_TITLE1_TEXT_ID,       12,   15,  62,   17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title2",         WIN_MAIN_ELEC_STAT_TITLE2_TEXT_ID,       74,   15,  30,   17,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "ElecStatImg1",   WIN_MAIN_ELEC_STAT_GRID_CHARGE_IMG_ID,   12,   43,  27,   18,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "ElecStatText1",  WIN_MAIN_ELEC_STAT_GRID_CHARGE_TEXT_ID,  64,   43,  38,   22,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "ElecStatImg2",   WIN_MAIN_ELEC_STAT_GRID_TIED_IMG_ID,     12,   72,  31,   19,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "ElecStatText2",  WIN_MAIN_ELEC_STAT_GRID_TIED_TEXT_ID,    64,   73,  38,   22,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "ElecStatImg3",   WIN_MAIN_ELEC_STAT_PV_CHARGE_IMG_ID,     12,   102, 14,   15,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "ElecStatText3",  WIN_MAIN_ELEC_STAT_PV_CHARGE_TEXT_ID,    64,   101, 38,   22,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "ElecStatImg4",   WIN_MAIN_ELEC_STAT_LOAD_IMG_ID,          12,   128, 16,   16,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "ElecStatText4",  WIN_MAIN_ELEC_STAT_LOAD_TEXT_ID,         64,   129, 38,   22,   0,   0x0,  0 },
};

/**
 * @brief 左下角温度
 */
static const GUI_WIDGET_CREATE_INFO _aTempDialogCreate[] =
{
    {
        WINDOW_CreateIndirect,  "Temp",               WIN_MAIN_TEMP_ID,
        0, 0, 115,  60,  0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "TempKey",            WIN_MAIN_TEMP_KEY_TEXT_ID,
        12, 8,  80,   17,   0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "F",                  WIN_MAIN_TEMP_KEY_MESURE_TEXT_ID,
        115 - 55, 8,  40,   17,   0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "TempValue",          WIN_MAIN_TEMP_VALUE_TEXT_ID,
        115 - 55, 60 - 25,  40,   17,   0,   0x0,  0
    },
    {
        IMAGE_CreateIndirect,   "TempImg1",           WIN_MAIN_TEMP_IMG1_ID,
        15, 60 - 30,  40,   40,   0,   0x0,  0
    },
};

/**
 * @brief 设备状态窗口
 */
static const GUI_WIDGET_CREATE_INFO _aDeviceDialogCreate[] =
{
    {
        WINDOW_CreateIndirect,  "Device",     WIN_MAIN_DEVICE_ID,
        0, 0,   EM_XSIZE / 3 + 10,  EM_YSIZE - 140,   0,   0x0,  0
    },
    {
        IMAGE_CreateIndirect,   "DevImgDevice",   WIN_MAIN_DEVICE_MODEL_ID,
        1, 1,   EM_XSIZE / 3 - 40,  EM_YSIZE - 140,   0,   0x0,  0
    },
    {
        IMAGE_CreateIndirect,   "DevImgGrid", WIN_MAIN_DEVICE_GRID_CHARGE_ID,
        EM_XSIZE / 3 - 20, 15,      40, 40, 0,   0x0,  0
    },
    {
        IMAGE_CreateIndirect,   "DevImgPV",   WIN_MAIN_DEVICE_PV_CHARGE_ID,
        EM_XSIZE / 3 - 20, 70,  40, 40, 0,   0x0,  0
    },
    {
        IMAGE_CreateIndirect,   "DevImgBackup",   WIN_MAIN_DEVICE_BACKUP_ID,
        EM_XSIZE / 3 - 20, 130, 40, 40, 0,   0x0,  0
    },

    {
        IMAGE_CreateIndirect,   "DevImgGridLine", WIN_MAIN_DEVICE_GRID_INDICATE_IMG_ID,
        EM_XSIZE / 3 - 50 - 20, 30,   60, 30, 0,   0x0,  0
    },
    {
        IMAGE_CreateIndirect,   "DevImgPvLine",   WIN_MAIN_DEVICE_PV_INDICATE_IMG_ID,
        EM_XSIZE / 3 - 50 - 20, 80,   60, 30, 0,   0x0,  0
    },
    {
        IMAGE_CreateIndirect,   "DevImgBackup",   WIN_MAIN_DEVICE_BACKUP_INDICATE_IMG_ID,
        EM_XSIZE / 3 - 50 - 20, 140,  60, 30, 0,   0x0,  0
    },
};

/**
 * @brief 右上工作模式
 */
static const GUI_WIDGET_CREATE_INFO _aWorkModeDialogCreate[] =
{
    {
        WINDOW_CreateIndirect,  "WorkModeWin",    WIN_MAIN_WORK_MODE_ID,
        0, 0,  125,  60,  0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "Title",          WIN_MAIN_WORK_MODE_TITLE_TEXT_ID,
        15,   12,  110,   17,   0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "Content",        WIN_MAIN_WORK_MODE_CONTENT_TEXT_ID,
        15,   37,  110,   17,   0,   0x0,  0
    },
};


/**
 * @brief 右侧中间状态灯
 */
static const GUI_WIDGET_CREATE_INFO _aLedStatDialogCreate[] =
{
    {
        WINDOW_CreateIndirect,  "LedStat",        WIN_MAIN_LED_STAT_ID,
        0, 0, 125,  35,  0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "Fault",          WIN_MAIN_LED_STAT_TITLE1_TEXT_ID,
        0,    0,  40,   17,   0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "Status",         WIN_MAIN_LED_STAT_TITLE2_TEXT_ID,
        45,   0,  40,   17,   0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "Grid",           WIN_MAIN_LED_STAT_TITLE3_TEXT_ID,
        85,   0,  40,   17,   0,   0x0,  0
    },

};

/**
 * @brief 右侧中下电量
 */
static const GUI_WIDGET_CREATE_INFO _aChargeDialogCreate[] =
{
    {
        WINDOW_CreateIndirect,  "",               WIN_MAIN_CHARGE_ID,
        0, 0, 125,  148,  0,   0x0,  0
    },
    {
        IMAGE_CreateIndirect,   "ImgChargeType",  WIN_MAIN_CHARGE_TYPE_IMG_ID,
        55,   32,  40,   55,   0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "%",          WIN_MAIN_CHARGE_PERCENT_TEXT_ID,
        5,  76 / 2 + 20,  120,   25,   0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "ChargePV",       WIN_MAIN_CHARGE_PV_TEXT_ID,
        5,  110,  125,   17,   0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "ChargeGrid",     WIN_MAIN_CHARGE_GRID_TEXT_ID,
        5,  128,  125,   17,   0,   0x0,  0
    },

};

/**
 * @brief 底部状态栏
 */
static const GUI_WIDGET_CREATE_INFO _aStatBarDialogCreate[] =
{
    {
        WINDOW_CreateIndirect,  "StatBar",        WIN_MAIN_STAT_BAR_ID,
        0, 0, EM_XSIZE,  55,  0,   0x0,  0
    },
    {
        IMAGE_CreateIndirect,   "ImgWifi",        WIN_MAIN_STAT_BAR_WIFI_IMG_ID,
        30,   15,  40,   55,   0,   0x0,  0
    },
    {
        TEXT_CreateIndirect,    "time",           WIN_MAIN_STAT_BAR_TIME_TEXT_ID,
        EM_XSIZE / 2 - 15,    0,  EM_XSIZE / 2 - 15,   55,   0,   0x0,  0
    },
};


/**
 * @brief 设置当天电网用电量
 *
 * @param kwh
 *
 * @return
 */
static int32_t _SetElecStatGridTied(uint16_t purchase)
{
    float kwh = (float)purchase / 10;
    if (kwh < (float)0.000001)
    {
        return -1;
    }

    snprintf(_gElecStatGridTiedTextBuf, sizeof(_gElecStatGridTiedTextBuf),
             "%.2f", kwh);

    TEXT_SetText(_gTextElecStatGridTiedHandle, (const char *)_gElecStatGridTiedTextPara.p_content);
    return 0;
}

/**
 * @brief 设置当天卖电量
 *
 * @param kwh
 *
 * @return
 */
static int32_t _SetElecStatGridCharge(uint16_t sale)
{
    float kwh = (float)sale / 10;
    if (kwh < (float)0.000001)
    {
        return -1;
    }
    snprintf(_gElecStatGridChargeTextBuf, sizeof(_gElecStatGridChargeTextBuf),
             "%.2f", kwh);

    TEXT_SetText(_gTextElecStatGridChargeHandle, (const char *)_gElecStatGridChargeTextPara.p_content);
    return 0;
}

/**
 * @brief 设置当天PV发电量
 *
 * @param kwh
 *
 * @return
 */
static int32_t _SetElectStatPvCharge(uint32_t sum)
{
    float kwh = (float)sum / 10;
    if (kwh < (float)0.000001)
    {
        return -1;
    }
    snprintf(_gElecStatPVChargeTextBuf, sizeof(_gElecStatPVChargeTextBuf),
             "%.2f", kwh);

    TEXT_SetText(_gTextElecStatPVChargeHandle, (const char *)_gElecStatPVChargeTextPara.p_content);
    return 0;
}


/**
 * @brief 设置当天负载使用电量
 *
 * @param float
 * @param kwh
 * @param force_update
 *
 * @return
 */
static int32_t _SetElecStatLoadCost(uint16_t load)
{
    float kwh = (float)load / 10;
    if (kwh < (float)0.000001)
    {
        return -1;
    }

    snprintf(_gElecStatLoadTextBuf, sizeof(_gElecStatLoadTextBuf),
             "%.2f", kwh);

    TEXT_SetText(_gTextElecStatLoadHandle, (const char *)_gElecStatLoadTextPara.p_content);
    return 0;
}


/**
 * @brief 设置温度
 *
 * @param temp
 *
 * @return
 */
static int32_t _SetTemperature(int16_t temp)
{
    emWidgetImgPara_t  *temp_img_para = NULL; // 0:normal, 1:col, 2:hot
    snprintf(_gTempValueTextBuf, sizeof(_gTempValueTextBuf),
             "%d", temp);

    if (temp < 0)
    {
        // cold
        _gTempValueTextPara.text_color = 0x00FFC44C;
        temp_img_para = &_gTempImgPara[1];
    }
    else if (temp > 0 && temp < 60)
    {
        // normal
        _gTempValueTextPara.text_color = 0x0010FF10;
        temp_img_para = &_gTempImgPara[0];
    }
    else
    {
        // hot
        _gTempValueTextPara.text_color = 0x001010FF;
        temp_img_para = &_gTempImgPara[2];
    }

    TEXT_SetText(_gTextTempValueTextHandle, (const char *)_gTempValueTextPara.p_content);
    TEXT_SetTextColor(_gTextTempValueTextHandle, _gTempValueTextPara.text_color);

    IMAGE_SetDTA(_gImgTempHandle, temp_img_para->p_img_buf,
                 temp_img_para->img_size);
    return 0;
}

/**
 * @brief 设置工作模式
 *
 * @param mode
 *
 * @return
 */
static int32_t _SetPcsWorkMode(EN_WORK_MODE_E mode)
{
    switch (mode)
    {
        case EN_WORK_MODE_BAT_PRIORITY:
        {
            snprintf(_gWorkModeContentTextBuf, sizeof(_gWorkModeContentTextBuf), "%s", "BAT Priority");
            break;
        }
        default:
        {
            snprintf(_gWorkModeContentTextBuf, sizeof(_gWorkModeContentTextBuf), "%s", "N/A");
            break;
        }
    }

    TEXT_SetText(_gTextWorkModeContentHandle, (const char *)_gWorkModeContentTextPara.p_content);
    return 0;
}


static int32_t _SetLedStat(EN_LED_TYPE_E type, EN_LED_STAT_E stat)
{
    uint32_t led_color = 0x00AAAAAA;

    switch (stat)
    {
        case EN_LED_STAT_NONE:
        {
            break;
        }
        case EN_LED_STAT_ERR:
        {
            led_color = 0X00281EDA;
            break;
        }
        case EN_LED_STAT_OK:
        {
            led_color = 0X00C1F100;
            break;
        }
    }
    _gLedStatLed[type] = led_color;
    WM_Paint(_gWinLedStatHandle);

    return 0;
}

/**
 * @brief 设置充电类型
 *
 * @param type
 *
 * @return
 */
static int32_t _SetChargeType(EN_CHARGE_TYPE_E type)
{
    int32_t flag = WM_IsVisible(_gImgChargeTypeHandle);

    switch(type)
    {
        case EN_CHARGE_TYPE_GRID:
        {
            IMAGE_SetDTA(_gImgChargeTypeHandle, _gChargeTypeImgPara[0].p_img_buf,
                         _gChargeTypeImgPara[0].img_size);
            if (flag == 0)
            {
                WM_ShowWindow(_gImgChargeTypeHandle);
                WM_Paint(_gWinChargeHandle);
            }
            break;
        }
        case EN_CHARGE_TYPE_PV:
        {
            IMAGE_SetDTA(_gImgChargeTypeHandle, _gChargeTypeImgPara[1].p_img_buf,
                         _gChargeTypeImgPara[1].img_size);
            if (flag == 0)
            {
                WM_ShowWindow(_gImgChargeTypeHandle);
                WM_Paint(_gWinChargeHandle);
            }
            break;
        }
        case EN_CHARGE_TYPE_GRID_PV:
        {
            IMAGE_SetDTA(_gImgChargeTypeHandle, _gChargeTypeImgPara[2].p_img_buf,
                         _gChargeTypeImgPara[2].img_size);
            if (flag == 0)
            {
                WM_ShowWindow(_gImgChargeTypeHandle);
                WM_Paint(_gWinChargeHandle);
            }
            break;
        }
        default:
        {
            if (flag != 0)
            {
                WM_HideWindow(_gImgChargeTypeHandle);
                WM_Paint(_gWinChargeHandle);
            }
            break;
        }

    }

    return 0;
}

static int32_t _SetSoc(uint16_t soc)
{
    if (soc > 100)
    {
        soc = 100;
    }


    snprintf(_gChargePercentTextBuf, sizeof(_gChargePercentTextBuf), "%d%%", soc);
    TEXT_SetText(_gTextChargePercentHandle, (const char *)_gChargePercentTextPara.p_content);
    WM_Paint(_gWinChargeHandle);
    return 0;
}

static int32_t _SetWifiStat(EN_WIFI_STAT_E stat)
{

    emWidgetImgPara_t  *tmp_wifi_img_para = &_gStatBarWifiImgPara[0];
    switch (stat)
    {
        case EN_WIFI_STAT_DISCONN:
        {
            tmp_wifi_img_para = &_gStatBarWifiImgPara[0];
            break;
        }
        case EN_WIFI_STAT_CONN:
        {
            tmp_wifi_img_para = &_gStatBarWifiImgPara[1];
            break;
        }
        default:
            break;
    }

    IMAGE_SetDTA(_gImgStatBarWifiHandle, tmp_wifi_img_para->p_img_buf,
                 tmp_wifi_img_para->img_size);
    return 0;
}

static int32_t _SetTime(uint32_t t)
{
    struct tm* timeinfo;

    timeinfo = localtime(&t); // 根据时间戳生成tm结构体对象
    strftime(_gStatBarTimeTextBuf, sizeof(_gStatBarTimeTextBuf), "%Y %b %eth %H:%M:%S",
             timeinfo); // 按指定格式将时间信息写入字符串
    TEXT_SetText(_gTextStatBarTimeHandle, (const char *)_gStatBarTimeTextPara.p_content);

    return 0;
}

static void _ElecStatSubWinPaint(WM_MESSAGE * pMsg)
{

    GUI_SetColor(GUI_COLOR_WIN_MAIN_ELEC_STAT);//上色
    GUI_FillRoundedRect(0,
                        0,
                        _aElecStatDialogCreate[0].xSize - 1,
                        _aElecStatDialogCreate[0].ySize - 1,
                        10);

    TEXT_SetText(_gTextElecStatGridChargeHandle, (const char *)_gElecStatGridChargeTextPara.p_content);
    TEXT_SetText(_gTextElecStatGridTiedHandle, (const char *)_gElecStatGridTiedTextPara.p_content);
    TEXT_SetText(_gTextElecStatPVChargeHandle, (const char *)_gElecStatPVChargeTextPara.p_content);
    TEXT_SetText(_gTextElecStatLoadHandle, (const char *)_gElecStatLoadTextPara.p_content);
}

/*左上角电量统计子窗口业务函数*/

/**
  * @brief ElecStatSubWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbElecStatSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*初始化各个子控件*/
            _gTextElecStatTitle1Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_ELEC_STAT_TITLE1_TEXT_ID);
            TEXT_SetFont(_gTextElecStatTitle1Handle, _gElecStatTitle1TextPara.p_text_font);
            TEXT_SetText(_gTextElecStatTitle1Handle, (const char *)_gElecStatTitle1TextPara.p_content);
            TEXT_SetTextAlign(_gTextElecStatTitle1Handle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextElecStatTitle1Handle, _gElecStatTitle1TextPara.text_color);

            _gTextElecStatTitle2Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_ELEC_STAT_TITLE2_TEXT_ID);
            TEXT_SetFont(_gTextElecStatTitle2Handle, _gElecStatTitle2TextPara.p_text_font);
            TEXT_SetText(_gTextElecStatTitle2Handle, (const char *)_gElecStatTitle2TextPara.p_content);
            TEXT_SetTextAlign(_gTextElecStatTitle2Handle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextElecStatTitle2Handle, _gElecStatTitle2TextPara.text_color);

            _gImgElecStatGridChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_ELEC_STAT_GRID_CHARGE_IMG_ID);
            IMAGE_SetDTA(_gImgElecStatGridChargeHandle, _gElecStatGridChargeImgPara.p_img_buf, _gElecStatGridChargeImgPara.img_size);

            _gTextElecStatGridChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_ELEC_STAT_GRID_CHARGE_TEXT_ID);
            TEXT_SetFont(_gTextElecStatGridChargeHandle, _gElecStatGridChargeTextPara.p_text_font);
            TEXT_SetText(_gTextElecStatGridChargeHandle, (const char *)_gElecStatGridChargeTextPara.p_content);
            TEXT_SetTextAlign(_gTextElecStatGridChargeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextElecStatGridChargeHandle, _gElecStatGridChargeTextPara.text_color);

            _gImgElecStatGridTiedHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_ELEC_STAT_GRID_TIED_IMG_ID);
            IMAGE_SetDTA(_gImgElecStatGridTiedHandle, _gElecStatGridTiedImgPara.p_img_buf, _gElecStatGridTiedImgPara.img_size);

            _gTextElecStatGridTiedHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_ELEC_STAT_GRID_TIED_TEXT_ID);
            TEXT_SetFont(_gTextElecStatGridTiedHandle, _gElecStatGridTiedTextPara.p_text_font);
            TEXT_SetText(_gTextElecStatGridTiedHandle, (const char *)_gElecStatGridTiedTextPara.p_content);
            TEXT_SetTextAlign(_gTextElecStatGridTiedHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextElecStatGridTiedHandle, _gElecStatGridTiedTextPara.text_color);

            _gImgElecStatPVChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_ELEC_STAT_PV_CHARGE_IMG_ID);
            IMAGE_SetDTA(_gImgElecStatPVChargeHandle, _gElecStatPVChargeImgPara.p_img_buf, _gElecStatPVChargeImgPara.img_size);

            _gTextElecStatPVChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_ELEC_STAT_PV_CHARGE_TEXT_ID);
            TEXT_SetFont(_gTextElecStatPVChargeHandle, _gElecStatPVChargeTextPara.p_text_font);
            TEXT_SetText(_gTextElecStatPVChargeHandle, (const char *)_gElecStatPVChargeTextPara.p_content);
            TEXT_SetTextAlign(_gTextElecStatPVChargeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextElecStatPVChargeHandle, _gElecStatPVChargeTextPara.text_color);

            _gImgElecStatLoadHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_ELEC_STAT_LOAD_IMG_ID);
            IMAGE_SetDTA(_gImgElecStatLoadHandle, _gElecStatLoadImgPara.p_img_buf, _gElecStatLoadImgPara.img_size);

            _gTextElecStatLoadHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_ELEC_STAT_LOAD_TEXT_ID);
            TEXT_SetFont(_gTextElecStatLoadHandle, _gElecStatLoadTextPara.p_text_font);
            TEXT_SetText(_gTextElecStatLoadHandle, (const char *)_gElecStatLoadTextPara.p_content);
            TEXT_SetTextAlign(_gTextElecStatLoadHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextElecStatLoadHandle, _gElecStatLoadTextPara.text_color);
            break;
        }
        case WM_PAINT:
        {
            _ElecStatSubWinPaint(pMsg);
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
  * @brief Temp callback
  * @param  NULL
  * @retval NULL
  */
static void _cbTempSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;

    static int     temp_type = 0;

#if TEST_DYNAMIC
    static WM_HTIMER wm_timer_handle_temp;
    static int ttt = 0;
#endif

    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*初始化各个子控件*/

#if TEST_DYNAMIC
            wm_timer_handle_temp = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                                  0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                                  1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                                  0);         /* 留待将来使用，应为0 */

#endif

            _gTextTempKeyHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_TEMP_KEY_TEXT_ID);
            TEXT_SetFont(_gTextTempKeyHandle, _gTempKeyTextPara.p_text_font);
            TEXT_SetText(_gTextTempKeyHandle, (const char *)_gTempKeyTextPara.p_content);
            TEXT_SetTextAlign(_gTextTempKeyHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextTempKeyHandle, _gTempKeyTextPara.text_color);

            _gTextTempKeyMesureHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_TEMP_KEY_MESURE_TEXT_ID);
            TEXT_SetFont(_gTextTempKeyMesureHandle, _gTempKeyMesureTextPara.p_text_font);
            TEXT_SetText(_gTextTempKeyMesureHandle, (const char *)_gTempKeyMesureTextPara.p_content);
            TEXT_SetTextAlign(_gTextTempKeyMesureHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextTempKeyMesureHandle, _gTempKeyMesureTextPara.text_color);

            _gTextTempValueTextHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_TEMP_VALUE_TEXT_ID);
            TEXT_SetFont(_gTextTempValueTextHandle, _gTempValueTextPara.p_text_font);
            TEXT_SetText(_gTextTempValueTextHandle, (const char *)_gTempValueTextPara.p_content);
            TEXT_SetTextAlign(_gTextTempValueTextHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextTempValueTextHandle, _gTempValueTextPara.text_color);


            _gImgTempHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_TEMP_IMG1_ID);
            IMAGE_SetDTA(_gImgTempHandle, _gTempImgPara[temp_type].p_img_buf,
                         _gTempImgPara[temp_type].img_size);

            break;
        }
        case WM_DELETE:
        {

#if TEST_DYNAMIC
            WM_DeleteTimer(wm_timer_handle_temp);//；删除窗口，会自动删除对应内部的定时器
#endif
            break;
        }
        case WM_TIMER:
        {

#if TEST_DYNAMIC
            _SetElecStatGridTied(0.12 + (++ttt));

            ++temp_type;

            if (temp_type % 3 == 0)
            {
                _SetTemperature(40);
            }
            else if (temp_type % 3 == 1)
            {
                _SetTemperature(-10);
            }
            else if (temp_type % 3 == 2)
            {
                _SetTemperature(90);
            }

            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
            GUI_SetColor(GUI_COLOR_WIN_MAIN_WORK_MODE);//上色
            GUI_FillRoundedRect(0,
                                0,
                                _aTempDialogCreate[0].xSize - 1,
                                _aTempDialogCreate[0].ySize - 1,
                                10);
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

static void _DrawDeviceElecType(int32_t electype)
{
    const int32_t x[3][2] =
    {
        {EM_XSIZE / 3 - 50 - 20,          EM_XSIZE / 3 - 30},
        {EM_XSIZE / 3 - 50 - 20,          EM_XSIZE / 3 - 30},
        {EM_XSIZE / 3 - 50 - 20,          EM_XSIZE / 3 - 30}
    };
    const int32_t y[3][3] =
    {
        {30,            35,         40},
        {80,            85,         90},
        {140,           145,        150}
    } ;
    //GUI_Clear();
    GUI_SetPenSize(1);
    GUI_SetColor(GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_BORDER);
    GUI_DrawLine(x[0][0], y[0][0], x[0][1], y[0][0]);
    GUI_DrawLine(x[0][0], y[0][2], x[0][1], y[0][2]);

    GUI_DrawLine(x[1][0], y[1][0], x[1][1], y[1][0]);
    GUI_DrawLine(x[1][0], y[1][2], x[1][1], y[1][2]);

    GUI_DrawLine(x[2][0], y[2][0], x[2][1], y[2][0]);
    GUI_DrawLine(x[2][0], y[2][2], x[2][1], y[2][2]);

    GUI_SetPenSize(GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_PEN_SIZE);
    if (electype & 0x01) //device <- GRID
    {
        GUI_SetColor(GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_ENABLE);
    }
    else if (electype & 0x02)
    {
        GUI_SetColor(GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_ENABLE);
    }
    else
    {
        GUI_SetColor(GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_DISABLE);
    }
    GUI_DrawLine(x[0][0], y[0][1], x[0][1], y[0][1]);

    if (electype & 0x04) // device <- PV
    {
        GUI_SetColor(GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_ENABLE);
    }
    else
    {
        GUI_SetColor(GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_DISABLE);
    }
    GUI_DrawLine(x[1][0], y[1][1], x[1][1], y[1][1]);

    if (electype & 0x08) // device <- Backup
    {
        GUI_SetColor(GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_ENABLE);
    }
    else
    {
        GUI_SetColor(GUI_COLOR_WIN_MAIN_DEVICE_CHARGE_DISABLE);
    }
    GUI_DrawLine(x[2][0], y[2][1], x[2][1], y[2][1]);

}

/**
  * @brief Temp callback
  * @param  NULL
  * @retval NULL
  */
static void _cbDeviceSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    int     grid_type = 0, pv_type = 0, backup_type = 0;

    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {

            _gImgDeviceModelHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_DEVICE_MODEL_ID);
            IMAGE_SetDTA(_gImgDeviceModelHandle, _gDeviceModelImgPara.p_img_buf, _gDeviceModelImgPara.img_size);

            _gImgDeviceGridChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_DEVICE_GRID_CHARGE_ID);
            IMAGE_SetDTA(_gImgDeviceGridChargeHandle, _gDeviceGridChargeImgPara[grid_type].p_img_buf,
                         _gDeviceGridChargeImgPara[grid_type].img_size);

            _gImgDevicePvHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_DEVICE_PV_CHARGE_ID);
            IMAGE_SetDTA(_gImgDevicePvHandle, _gDevicePvChargeImgPara[pv_type].p_img_buf,
                         _gDevicePvChargeImgPara[pv_type].img_size);

            _gImgDeviceBackupHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_DEVICE_BACKUP_ID);
            IMAGE_SetDTA(_gImgDeviceBackupHandle, _gDeviceBackupImgPara[backup_type].p_img_buf,
                         _gDeviceBackupImgPara[backup_type].img_size);

            _gImgDeviceIndicateHandle[0] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_DEVICE_GRID_INDICATE_IMG_ID);
            IMAGE_SetDTA(_gImgDeviceIndicateHandle[0], _gDeviceIndicateImgPara[0].p_img_buf,
                         _gDeviceIndicateImgPara[0].img_size);

            _gImgDeviceIndicateHandle[1] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_DEVICE_PV_INDICATE_IMG_ID);
            IMAGE_SetDTA(_gImgDeviceIndicateHandle[1], _gDeviceIndicateImgPara[0].p_img_buf,
                         _gDeviceIndicateImgPara[0].img_size);

            _gImgDeviceIndicateHandle[2] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_DEVICE_BACKUP_INDICATE_IMG_ID);
            IMAGE_SetDTA(_gImgDeviceIndicateHandle[2], _gDeviceIndicateImgPara[0].p_img_buf,
                         _gDeviceIndicateImgPara[0].img_size);

            break;
        }
        case WM_PAINT:
        {
#if LAYOUT_DEBUG
            GUI_SetColor(GUI_COLOR_WIN_MAIN_WORK_MODE);//上色
            GUI_FillRoundedRect(0,
                                0,
                                _aDeviceDialogCreate[0].xSize - 1,
                                _aDeviceDialogCreate[0].ySize - 1,
                                10);
            //_DrawDeviceElecType(1);
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
  * @brief ElecStatSubWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbWorkModeSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*初始化各个子控件*/
            _gTextWorkModeTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_WORK_MODE_TITLE_TEXT_ID);
            TEXT_SetFont(_gTextWorkModeTitleHandle, _gWorkModeTitleTextPara.p_text_font);
            TEXT_SetText(_gTextWorkModeTitleHandle, (const char *)_gWorkModeTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTextWorkModeTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextWorkModeTitleHandle, _gWorkModeTitleTextPara.text_color);

            _gTextWorkModeContentHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_WORK_MODE_CONTENT_TEXT_ID);
            TEXT_SetFont(_gTextWorkModeContentHandle, _gWorkModeContentTextPara.p_text_font);
            TEXT_SetTextAlign(_gTextWorkModeContentHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextWorkModeContentHandle, _gWorkModeContentTextPara.text_color);

            _SetPcsWorkMode(EN_WORK_MODE_BAT_PRIORITY);
            break;
        }
        case WM_PAINT:
        {
            GUI_SetColor(GUI_COLOR_WIN_MAIN_WORK_MODE);//上色
            GUI_FillRoundedRect(0,
                                0,
                                _aWorkModeDialogCreate[0].xSize - 1,
                                _aWorkModeDialogCreate[0].ySize - 1,
                                10);
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
  * @brief ElecStatSubWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbLedStatSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;

    static int ledstat = 0;

#if TEST_DYNAMIC
    static WM_HTIMER wm_timer_handle_led;
#endif

    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*初始化各个子控件*/

#if TEST_DYNAMIC
            wm_timer_handle_led = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                                 0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                                 1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                                 0);         /* 留待将来使用，应为0 */

#endif

            _gTextLedStatTitle1Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_LED_STAT_TITLE1_TEXT_ID);
            TEXT_SetFont(_gTextLedStatTitle1Handle, _gLedStatTitle1TextPara.p_text_font);
            TEXT_SetText(_gTextLedStatTitle1Handle, (const char *)_gLedStatTitle1TextPara.p_content);
            TEXT_SetTextAlign(_gTextLedStatTitle1Handle, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLedStatTitle1Handle, _gLedStatTitle1TextPara.text_color);



            _gTextLedStatTitle2Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_LED_STAT_TITLE2_TEXT_ID);
            TEXT_SetFont(_gTextLedStatTitle2Handle, _gLedStatTitle2TextPara.p_text_font);
            TEXT_SetText(_gTextLedStatTitle2Handle, (const char *)_gLedStatTitle2TextPara.p_content);
            TEXT_SetTextAlign(_gTextLedStatTitle2Handle, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLedStatTitle2Handle, _gLedStatTitle2TextPara.text_color);


            _gTextLedStatTitle3Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_LED_STAT_TITLE3_TEXT_ID);
            TEXT_SetFont(_gTextLedStatTitle3Handle, _gLedStatTitle3TextPara.p_text_font);
            TEXT_SetText(_gTextLedStatTitle3Handle, (const char *)_gLedStatTitle3TextPara.p_content);
            TEXT_SetTextAlign(_gTextLedStatTitle3Handle, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLedStatTitle3Handle, _gLedStatTitle3TextPara.text_color);


            break;
        }
        case WM_DELETE:
        {

#if TEST_DYNAMIC
            WM_DeleteTimer(wm_timer_handle_led);//；删除窗口，会自动删除对应内部的定时器
#endif
            break;
        }
        case WM_TIMER:
        {
#if TEST_DYNAMIC
            ++ledstat;
            _SetLedStat(EN_LED_TYPE_FAULT, (EN_LED_STAT_E)(ledstat % 3));

            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
#if LAYOUT_DEBUG
            GUI_SetColor(GUI_COLOR_WIN_MAIN_WORK_MODE);//上色
            GUI_FillRoundedRect(0,
                                0,
                                _aLedStatDialogCreate[0].xSize - 1,
                                _aLedStatDialogCreate[0].ySize - 1,
                                10);
#endif

            // LED1
            GUI_SetColor(_gLedStatLed[0]);
            GUI_FillCircle(_aLedStatDialogCreate[1].x0
                           + _aLedStatDialogCreate[1].xSize / 2,
                           _aLedStatDialogCreate[1].y0
                           + _aLedStatDialogCreate[1].ySize + 8,
                           6);

            // LED2
            GUI_SetColor(_gLedStatLed[1]);
            GUI_FillCircle(_aLedStatDialogCreate[2].x0
                           + _aLedStatDialogCreate[2].xSize / 2,
                           _aLedStatDialogCreate[2].y0
                           + _aLedStatDialogCreate[2].ySize + 8,
                           6);

            // LED3
            GUI_SetColor(_gLedStatLed[2]);
            GUI_FillCircle(_aLedStatDialogCreate[3].x0
                           + _aLedStatDialogCreate[3].xSize / 2,
                           _aLedStatDialogCreate[3].y0
                           + _aLedStatDialogCreate[3].ySize + 8,
                           6);


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

static int32_t _GetArcStartEnd90symmetry(int32_t percent, int32_t *s, int32_t *e)
{
    int32_t _p = 0;

    _p = 360 * (100 - percent) / 100 / 2;
    *s = 0 + 90 + _p;
    *e = 360 + 90 - _p;

    return 0;
}

static int32_t _GetArcStartEnd0(int32_t percent, int32_t *s, int32_t *e)
{
    int32_t _p = 0;

    _p = 360 * (100 - percent) / 100;
    *s =  _p;
    *e = 360;

    return 0;
}

static void _DrawChargeSubWinCharge(int32_t percent)
{
    int32_t arc_s = 0, arc_e = 0;
    GUI_Clear();
    GUI_SetPenSize(GUI_COLOR_WIN_MAIN_CHARGE_CIRCLE_PEN_SIZE);
    GUI_SetColor(GUI_COLOR_WIN_MAIN_CHARGE_CIRCLE_BG_1);
    GUI_DrawArc((_aChargeDialogCreate[0].xSize + 1) / 2, 76 / 2 + 20, 40, 40, 0, 360);
    GUI_SetColor(GUI_COLOR_WIN_MAIN_CHARGE_CIRCLE_BG_2);
    _GetArcStartEnd0(percent, &arc_s, &arc_e);
    GUI_DrawArc((_aChargeDialogCreate[0].xSize + 1) / 2, 76 / 2 + 20, 40, 40, arc_s, arc_e);
}

/**
  * @brief ElecStatSubWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbChargeSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;

    static int     charge_type = 0;
    static int     charge_percent = 0;

#if TEST_DYNAMIC
    static WM_HTIMER wm_timer_handle_charge_percent;
#endif

    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*初始化各个子控件*/

#if TEST_DYNAMIC
            wm_timer_handle_charge_percent = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                             0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                             100,         /* 周期，此周期过后指定窗口应收到消息*/
                                             0);         /* 留待将来使用，应为0 */
#endif

            _gImgChargeTypeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_CHARGE_TYPE_IMG_ID);
#if 0
            IMAGE_SetDTA(_gImgChargeTypeHandle, _gChargeTypeImgPara[charge_type].p_img_buf,
                         _gChargeTypeImgPara[charge_type].img_size);
#endif

            _gTextChargePercentHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_CHARGE_PERCENT_TEXT_ID);
            TEXT_SetFont(_gTextChargePercentHandle, _gChargePercentTextPara.p_text_font);
            TEXT_SetText(_gTextChargePercentHandle, (const char *)_gChargePercentTextPara.p_content);
            TEXT_SetTextAlign(_gTextChargePercentHandle, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextChargePercentHandle, _gChargePercentTextPara.text_color);


            _gTextChargePvHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_CHARGE_PV_TEXT_ID);
            TEXT_SetFont(_gTextChargePvHandle, _gChargePvTextPara.p_text_font);
            TEXT_SetText(_gTextChargePvHandle, (const char *)_gChargePvTextPara.p_content);
            TEXT_SetTextAlign(_gTextChargePvHandle, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextChargePvHandle, _gChargePvTextPara.text_color);


            _gTextChargeGridHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_CHARGE_GRID_TEXT_ID);
            TEXT_SetFont(_gTextChargeGridHandle, _gChargeGridTextPara.p_text_font);
            TEXT_SetText(_gTextChargeGridHandle, (const char *)_gChargeGridTextPara.p_content);
            TEXT_SetTextAlign(_gTextChargeGridHandle, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextChargeGridHandle, _gChargeGridTextPara.text_color);

            break;
        }
        case WM_DELETE:
        {
#if TEST_DYNAMIC
            WM_DeleteTimer(wm_timer_handle_charge_percent);//；删除窗口，会自动删除对应内部的定时器
#endif
            break;
        }
        case WM_TIMER:
        {
#if TEST_DYNAMIC
            ++charge_percent;
            if (charge_percent > 100)
            {
                charge_percent = 0;
            }

            if (charge_percent % 10 == 0)
            {
                ++charge_type;
            }

            _SetChargeType((EN_CHARGE_TYPE_E)(charge_type % 4));
            _SetSoc(charge_percent);

            WM_RestartTimer(pMsg->Data.v, 100);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
#if LAYOUT_DEBUG

            GUI_SetColor(GUI_COLOR_WIN_MAIN_CHARGE);//上色
            GUI_FillRoundedRect(0,
                                0,
                                _aChargeDialogCreate[0].xSize - 1,
                                _aChargeDialogCreate[0].ySize - 1,
                                10);
#endif

            _DrawChargeSubWinCharge(charge_percent);

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
  * @brief ElecStatSubWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbStatBarSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;

    static time_t cur_time = 1704067200;
    static int     wifi_type = 0;

#if TEST_DYNAMIC
    static WM_HTIMER wm_timer_handle_wifi;
#endif

    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {

#if TEST_DYNAMIC
            wm_timer_handle_wifi = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                                  0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                                  1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                                  0);         /* 留待将来使用，应为0 */
#endif

            /*初始化各个子控件*/
            _gImgStatBarWifiHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STAT_BAR_WIFI_IMG_ID);
            IMAGE_SetDTA(_gImgStatBarWifiHandle, _gStatBarWifiImgPara[wifi_type].p_img_buf,
                         _gStatBarWifiImgPara[wifi_type].img_size);

            _gTextStatBarTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STAT_BAR_TIME_TEXT_ID);
            TEXT_SetFont(_gTextStatBarTimeHandle, _gStatBarTimeTextPara.p_text_font);
            TEXT_SetText(_gTextStatBarTimeHandle, (const char *)_gStatBarTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextStatBarTimeHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarTimeHandle, _gStatBarTimeTextPara.text_color);

            break;
        }
        case WM_DELETE:
        {
#if TEST_DYNAMIC
            WM_DeleteTimer(wm_timer_handle_wifi);//；删除窗口，会自动删除对应内部的定时器
#endif
            break;
        }
        case WM_TIMER:
        {

#if TEST_DYNAMIC
            ++wifi_type;
            ++cur_time;

            _SetTime(cur_time);
            _SetWifiStat((WIFI_STAT_E)(wifi_type % 4));

            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
            GUI_SetColor(GUI_COLOR_WIN_MAIN_STAT_BAR);//上色
            GUI_FillRoundedRect(0,
                                0,
                                _aStatBarDialogCreate[0].xSize - 1,
                                _aStatBarDialogCreate[0].ySize - 1,
                                5);

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
  * @brief Create ElecStat sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateElecStatSubWin()
{
    _gWinElecStatHandle = GUI_CreateDialogBox(_aElecStatDialogCreate, GUI_COUNTOF(_aElecStatDialogCreate), _cbElecStatSubWin, pCurrentEmWin->win_handle, 0, 0);
    WM_SetTransState(_gWinElecStatHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinElecStatHandle;
}

/**
  * @brief Create working mode sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateTempSubWin()
{
    _gWinTempHandle = GUI_CreateDialogBox(_aTempDialogCreate,
                                          GUI_COUNTOF(_aTempDialogCreate),
                                          _cbTempSubWin,
                                          pCurrentEmWin->win_handle,
                                          15,
                                          _aElecStatDialogCreate[0].ySize
                                          + 12 + 12);
    WM_SetTransState(_gWinTempHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinWorkModeHandle;
}

/**
  * @brief Device sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateDeviceSubWin()
{
    _gWinDeviceHandle = GUI_CreateDialogBox(_aDeviceDialogCreate,
                                            GUI_COUNTOF(_aDeviceDialogCreate),
                                            _cbDeviceSubWin,
                                            pCurrentEmWin->win_handle,
                                            15 + _aElecStatDialogCreate[0].xSize + 15,
                                            67);
    WM_SetTransState(_gWinDeviceHandle, WM_CF_HASTRANS);
    return _gWinDeviceHandle;
}

/**
  * @brief Create working mode sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWorkModSubWin()
{
    _gWinWorkModeHandle = GUI_CreateDialogBox(_aWorkModeDialogCreate,
                          GUI_COUNTOF(_aWorkModeDialogCreate),
                          _cbWorkModeSubWin,
                          pCurrentEmWin->win_handle,
                          EM_XSIZE -  _aElecStatDialogCreate[0].xSize
                          - _aElecStatDialogCreate[0].x0 - 15,
                          12);
    WM_SetTransState(_gWinWorkModeHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinWorkModeHandle;
}

/**
  * @brief Create LED stat sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateLedStatSubWin()
{
    _gWinLedStatHandle = GUI_CreateDialogBox(_aLedStatDialogCreate,
                         GUI_COUNTOF(_aLedStatDialogCreate),
                         _cbLedStatSubWin,
                         pCurrentEmWin->win_handle,
                         EM_XSIZE -  _aElecStatDialogCreate[0].xSize
                         - _aElecStatDialogCreate[0].x0 - 15,
                         _aWorkModeDialogCreate[0].y0
                         + _aWorkModeDialogCreate[0].ySize
                         + 12 + 10);
    WM_SetTransState(_gWinLedStatHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinLedStatHandle;
}

/**
  * @brief Create stat bar sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateChargeSubWin()
{
    _gWinChargeHandle = GUI_CreateDialogBox(_aChargeDialogCreate,
                                            GUI_COUNTOF(_aChargeDialogCreate),
                                            _cbChargeSubWin,
                                            pCurrentEmWin->win_handle,
                                            EM_XSIZE -  _aElecStatDialogCreate[0].xSize
                                            - _aElecStatDialogCreate[0].x0 - 15,
                                            12 + _aWorkModeDialogCreate[0].ySize
                                            + 10 + _aLedStatDialogCreate[0].ySize
                                           );
    WM_SetTransState(_gWinChargeHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinChargeHandle;
}

/**
  * @brief Create stat bar sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateStatBarSubWin()
{
    _gWinStatBarHandle = GUI_CreateDialogBox(_aStatBarDialogCreate,
                         GUI_COUNTOF(_aStatBarDialogCreate),
                         _cbStatBarSubWin,
                         pCurrentEmWin->win_handle,
                         0,
                         EM_YSIZE - _aStatBarDialogCreate[0].ySize);
    WM_SetTransState(_gWinStatBarHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinStatBarHandle;
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
            GUI_DrawStreamedBitmapAuto(_pBKBuf, 0, 0);
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
  * @brief Create current win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWin(void)
{
    _Init();
    GUI_EnableAlpha(1);
    /*创建一个基于桌面窗口的子窗口*/
    gstEmWinMain.win_handle = WM_CreateWindow(0, 0, EM_XSIZE, EM_YSIZE, WM_CF_SHOW, _cbMainWin, 0);
//    gstEmWinMain.win_handle = WM_CreateWindowAsChild(0, 0, EM_XSIZE, EM_YSIZE, WM_HBKWIN, WM_CF_SHOW, _cbMainWin, 0);
    pCurrentEmWin = &gstEmWinMain;
    GUI_Exec();

    _CreateElecStatSubWin();
    _CreateTempSubWin();
    _CreateDeviceSubWin();
    _CreateWorkModSubWin();
    _CreateLedStatSubWin();
    _CreateChargeSubWin();
    _CreateStatBarSubWin();
    return gstEmWinMain.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinMain.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinMain.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinMain.win_handle = 0;
}

static void _InitElecStat()
{
    /*为ElecStat区域子控件申请内存*/
    _gElecStatGridChargeImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_ELEC_STAT_ICON_GRID_CHARGE_SIZE);
    _gElecStatGridChargeImgPara.p_img_buf = GUI_ALLOC_h2p(_gElecStatGridChargeImgPara.mem_img);
    _gElecStatGridChargeImgPara.img_size = EM_MAIN_ELEC_STAT_ICON_GRID_CHARGE_SIZE;
    SpiFlashReadData(EM_ICON_GRID_CHARGE_ADDR, _gElecStatGridChargeImgPara.p_img_buf, _gElecStatGridChargeImgPara.img_size);
    _gElecStatGridTiedImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_ELEC_STAT_ICON_GRID_TIED_SIZE);
    _gElecStatGridTiedImgPara.p_img_buf = GUI_ALLOC_h2p(_gElecStatGridTiedImgPara.mem_img);
    _gElecStatGridTiedImgPara.img_size = EM_MAIN_ELEC_STAT_ICON_GRID_TIED_SIZE;
    SpiFlashReadData(EM_ICON_GRID_TIED_ADDR, _gElecStatGridTiedImgPara.p_img_buf, _gElecStatGridTiedImgPara.img_size);
    _gElecStatPVChargeImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_ELEC_STAT_ICON_PV_CHARGE_SIZE);
    _gElecStatPVChargeImgPara.p_img_buf = GUI_ALLOC_h2p(_gElecStatPVChargeImgPara.mem_img);
    _gElecStatPVChargeImgPara.img_size = EM_MAIN_ELEC_STAT_ICON_PV_CHARGE_SIZE;
    SpiFlashReadData(EM_ICON_PV_CHARGE_ADDR, _gElecStatPVChargeImgPara.p_img_buf, _gElecStatPVChargeImgPara.img_size);
    _gElecStatLoadImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_ELEC_STAT_ICON_LOAD_SIZE);
    _gElecStatLoadImgPara.p_img_buf = GUI_ALLOC_h2p(_gElecStatLoadImgPara.mem_img);
    _gElecStatLoadImgPara.img_size = EM_MAIN_ELEC_STAT_ICON_LOAD_SIZE;
    SpiFlashReadData(EN_ICON_BACKUP_LOAD_ADDR, _gElecStatLoadImgPara.p_img_buf, _gElecStatLoadImgPara.img_size);

    _gElecStatTitle1TextPara.p_content = _gElecStatTitle1TextBuf;
    _gElecStatTitle1TextPara.text_color = 0x00FFFFFF;
    _gElecStatTitle1TextPara.p_text_font = EM_TEXT_FONT_1;
    _gElecStatTitle2TextPara.p_content = _gElecStatTitle2TextBuf;
    _gElecStatTitle2TextPara.text_color = 0x00B2BE00;
    _gElecStatTitle2TextPara.p_text_font = EM_TEXT_FONT_2;
    _gElecStatGridChargeTextPara.p_content = _gElecStatGridChargeTextBuf;
    _gElecStatGridChargeTextPara.text_color = 0x00FFFFFF;
    _gElecStatGridChargeTextPara.p_text_font = EM_TEXT_FONT_3;
    _gElecStatGridTiedTextPara.p_content = _gElecStatGridTiedTextBuf;
    _gElecStatGridTiedTextPara.text_color = 0x00FFFFFF;
    _gElecStatGridTiedTextPara.p_text_font = EM_TEXT_FONT_3;
    _gElecStatPVChargeTextPara.p_content = _gElecStatPVChargeTextBuf;
    _gElecStatPVChargeTextPara.text_color = 0x00FFFFFF;
    _gElecStatPVChargeTextPara.p_text_font = EM_TEXT_FONT_3;
    _gElecStatLoadTextPara.p_content = _gElecStatLoadTextBuf;
    _gElecStatLoadTextPara.text_color = 0x00FFFFFF;
    _gElecStatLoadTextPara.p_text_font = EM_TEXT_FONT_3;

    _gElecStatGridChargeTextPara.digit_value = 3.25;//测试数据，实际赋值系统数据
    sprintf(_gElecStatGridChargeTextPara.p_content, "%.2f", _gElecStatGridChargeTextPara.digit_value);
    _gElecStatGridTiedTextPara.digit_value = 4.20;//测试数据，实际赋值系统数据
    sprintf(_gElecStatGridTiedTextPara.p_content, "%.2f", _gElecStatGridTiedTextPara.digit_value);
    _gElecStatPVChargeTextPara.digit_value = 3.25;//测试数据，实际赋值系统数据
    sprintf(_gElecStatPVChargeTextPara.p_content, "%.2f", _gElecStatPVChargeTextPara.digit_value);
    _gElecStatLoadTextPara.digit_value = 4.20;//测试数据，实际赋值系统数据
    sprintf(_gElecStatLoadTextPara.p_content, "%.2f", _gElecStatLoadTextPara.digit_value);

}

static void _InitTemp()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为Temp区域子控件申请内存*/
    _gTempImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_TEMP_ICON_SIZE);
    _gTempImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gTempImgPara[0].mem_img);
    _gTempImgPara[0].img_size = EM_MAIN_TEMP_ICON_SIZE;
    SpiFlashReadData(EM_ICON_TEMP_NORMAL_ADDR, _gTempImgPara[0].p_img_buf, _gTempImgPara[0].img_size);

    _gTempImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_TEMP_ICON_SIZE);
    _gTempImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gTempImgPara[1].mem_img);
    _gTempImgPara[1].img_size = EM_MAIN_TEMP_ICON_SIZE;
    SpiFlashReadData(EM_ICON_TEMP_COLD_ADDR, _gTempImgPara[1].p_img_buf, _gTempImgPara[1].img_size);

    _gTempImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_TEMP_ICON_SIZE);
    _gTempImgPara[2].p_img_buf = GUI_ALLOC_h2p(_gTempImgPara[2].mem_img);
    _gTempImgPara[2].img_size = EM_MAIN_TEMP_ICON_SIZE;
    SpiFlashReadData(EM_ICON_TEMP_HEAT_ADDR, _gTempImgPara[2].p_img_buf, _gTempImgPara[2].img_size);

    _gTempKeyTextPara.p_content = _gTempKeyTextBuf;
    _gTempKeyTextPara.text_color = 0x00FFFFFF;
    _gTempKeyTextPara.p_text_font = GUI_FONT_16_ASCII;

    _gTempKeyMesureTextPara.p_content = _gTempKeyMesureTextBuf;
    _gTempKeyMesureTextPara.text_color = 0x00FFFFFF;
    _gTempKeyMesureTextPara.p_text_font = GUI_FONT_16_ASCII;

    _gTempValueTextPara.p_content = _gTempValueTextBuf;;
    _gTempValueTextPara.text_color = 0x00CDF100;
    _gTempValueTextPara.p_text_font = GUI_FONT_20_ASCII;
}

static void _InitDevice()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为设备区域子控件申请内存*/
    _gDeviceModelImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_DEVICE_ICON_ROCA_SIZE);
    _gDeviceModelImgPara.p_img_buf = GUI_ALLOC_h2p(_gDeviceModelImgPara.mem_img);
    _gDeviceModelImgPara.img_size = EM_MAIN_DEVICE_ICON_ROCA_SIZE;
    SpiFlashReadData(EM_ICON_ROCA_ADDR, _gDeviceModelImgPara.p_img_buf, _gDeviceModelImgPara.img_size);

    _gDeviceGridChargeImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_DEVICE_ICON_GRID_CHARGE_SIZE);
    _gDeviceGridChargeImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gDeviceGridChargeImgPara[0].mem_img);
    _gDeviceGridChargeImgPara[0].img_size = EM_MAIN_DEVICE_ICON_GRID_CHARGE_SIZE;
    SpiFlashReadData(EM_ICON_GRID_STATUS_DISABLED_ADDR, _gDeviceGridChargeImgPara[0].p_img_buf,
                     _gDeviceGridChargeImgPara[0].img_size);

    _gDeviceGridChargeImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_DEVICE_ICON_GRID_CHARGE_SIZE);
    _gDeviceGridChargeImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gDeviceGridChargeImgPara[1].mem_img);
    _gDeviceGridChargeImgPara[1].img_size = EM_MAIN_DEVICE_ICON_GRID_CHARGE_SIZE;
    SpiFlashReadData(EM_ICON_GRID_STATUS_ACTIVE_ADDR, _gDeviceGridChargeImgPara[1].p_img_buf,
                     _gDeviceGridChargeImgPara[1].img_size);

    _gDevicePvChargeImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_DEVICE_ICON_PV_CHARGE_SIZE);
    _gDevicePvChargeImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gDevicePvChargeImgPara[0].mem_img);
    _gDevicePvChargeImgPara[0].img_size = EM_MAIN_DEVICE_ICON_PV_CHARGE_SIZE;
    SpiFlashReadData(EM_ICON_PV_STATUS_DISABLED_ADDR, _gDevicePvChargeImgPara[0].p_img_buf,
                     _gDevicePvChargeImgPara[0].img_size);

    _gDevicePvChargeImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_DEVICE_ICON_PV_CHARGE_SIZE);
    _gDevicePvChargeImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gDevicePvChargeImgPara[1].mem_img);
    _gDevicePvChargeImgPara[1].img_size = EM_MAIN_DEVICE_ICON_PV_CHARGE_SIZE;
    SpiFlashReadData(EM_ICON_PV_STATUS_ACTIVE_ADDR, _gDevicePvChargeImgPara[1].p_img_buf,
                     _gDevicePvChargeImgPara[1].img_size);

    _gDeviceBackupImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_DEVICE_ICON_BACKUP_SIZE);
    _gDeviceBackupImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gDeviceBackupImgPara[0].mem_img);
    _gDeviceBackupImgPara[0].img_size = EM_MAIN_DEVICE_ICON_BACKUP_SIZE;
    SpiFlashReadData(EM_ICON_BACKUP_LOAD_STATUS_DISABLED_ADDR, _gDeviceBackupImgPara[0].p_img_buf,
                     _gDeviceBackupImgPara[0].img_size);

    _gDeviceBackupImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_DEVICE_ICON_BACKUP_SIZE);
    _gDeviceBackupImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gDeviceBackupImgPara[1].mem_img);
    _gDeviceBackupImgPara[1].img_size = EM_MAIN_DEVICE_ICON_BACKUP_SIZE;
    SpiFlashReadData(EM_ICON_BACKUP_LOAD_STATUS_ACTIVE_ADDR, _gDeviceBackupImgPara[1].p_img_buf,
                     _gDeviceBackupImgPara[1].img_size);

    _gDeviceIndicateImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_DEVICE_ICON_INDICATE_SIZE);
    _gDeviceIndicateImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gDeviceIndicateImgPara[0].mem_img);
    _gDeviceIndicateImgPara[0].img_size = EM_MAIN_DEVICE_ICON_INDICATE_SIZE;
    SpiFlashReadData(EM_ICON_INDICATE_ACTIVE_ADDR, _gDeviceIndicateImgPara[0].p_img_buf,
                     _gDeviceIndicateImgPara[0].img_size);

    _gDeviceIndicateImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_DEVICE_ICON_INDICATE_SIZE);
    _gDeviceIndicateImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gDeviceIndicateImgPara[1].mem_img);
    _gDeviceIndicateImgPara[1].img_size = EM_MAIN_DEVICE_ICON_INDICATE_SIZE;
    SpiFlashReadData(EM_ICON_INDICATE_DISABLED_ADDR, _gDeviceIndicateImgPara[1].p_img_buf,
                     _gDeviceIndicateImgPara[1].img_size);
}

static void _InitWorkMode()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为WorkMode_t区域子控件申请内存*/

    _gWorkModeTitleTextPara.p_content = _gWorkModeTitleTextBuf;
    _gWorkModeTitleTextPara.text_color = 0x00FFFFFF;
    _gWorkModeTitleTextPara.p_text_font = GUI_FONT_16_ASCII;
    _gWorkModeContentTextPara.p_content = _gWorkModeContentTextBuf;;
    _gWorkModeContentTextPara.text_color = 0X00B2BE00;
    _gWorkModeContentTextPara.p_text_font = GUI_FONT_16B_ASCII;
}

static void _InitLedStat()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为LedStat区域子控件申请内存*/

    _gLedStatTitle1TextPara.p_content = _gLedStatTitle1TextBuf;
    _gLedStatTitle1TextPara.text_color = 0x00FFFFFF;
    _gLedStatTitle1TextPara.p_text_font = GUI_FONT_13_ASCII;


    _gLedStatTitle2TextPara.p_content = _gLedStatTitle2TextBuf;
    _gLedStatTitle2TextPara.text_color = 0x00FFFFFF;
    _gLedStatTitle2TextPara.p_text_font = GUI_FONT_13_ASCII;

    _gLedStatTitle3TextPara.p_content = _gLedStatTitle3TextBuf;
    _gLedStatTitle3TextPara.text_color = 0x00FFFFFF;
    _gLedStatTitle3TextPara.p_text_font = GUI_FONT_13_ASCII;
}

static void _InitCharge()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为Charge区域子控件申请内存*/

    _gChargeTypeImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_CHARGE_ICON_GRID_SIZE);
    _gChargeTypeImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gChargeTypeImgPara[0].mem_img);
    _gChargeTypeImgPara[0].img_size = EM_MAIN_CHARGE_ICON_GRID_SIZE;
    SpiFlashReadData(EM_ICON_GRID_MODE_CHARGING_ADDR, _gChargeTypeImgPara[0].p_img_buf,
                     _gChargeTypeImgPara[0].img_size);

    _gChargeTypeImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_CHARGE_ICON_PV_SIZE);
    _gChargeTypeImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gChargeTypeImgPara[1].mem_img);
    _gChargeTypeImgPara[1].img_size = EM_MAIN_CHARGE_ICON_PV_SIZE;
    SpiFlashReadData(EM_ICON_PV_MODE_CHARGING_ADDR, _gChargeTypeImgPara[1].p_img_buf,
                     _gChargeTypeImgPara[0].img_size);

    _gChargeTypeImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_CHARGE_ICON_GRID_PV_SIZE);
    _gChargeTypeImgPara[2].p_img_buf = GUI_ALLOC_h2p(_gChargeTypeImgPara[2].mem_img);
    _gChargeTypeImgPara[2].img_size = EM_MAIN_CHARGE_ICON_GRID_PV_SIZE;
    SpiFlashReadData(EM_ICON_GRID_PV_MODE_CHARGING_ADDR, _gChargeTypeImgPara[2].p_img_buf,
                     _gChargeTypeImgPara[2].img_size);

    _gChargePercentTextPara.p_content = _gChargePercentTextBuf;
    _gChargePercentTextPara.text_color = 0x00FFFFFF;
    _gChargePercentTextPara.p_text_font = GUI_FONT_24_ASCII;


    _gChargePvTextPara.p_content = _gChargePvTextBuf;
    _gChargePvTextPara.text_color = 0X00CDF100;
    _gChargePvTextPara.p_text_font = GUI_FONT_13_ASCII;

    _gChargeGridTextPara.p_content = _gChargeGridTextBuf;
    _gChargeGridTextPara.text_color = 0x00FFC44C;
    _gChargeGridTextPara.p_text_font = GUI_FONT_13_ASCII;
}

static void _InitStatBar()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为StatBar区域子控件申请内存*/

    _gStatBarWifiImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_STAT_BAR_WIFI_ICON_SIZE);
    _gStatBarWifiImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gStatBarWifiImgPara[0].mem_img);
    _gStatBarWifiImgPara[0].img_size = EM_MAIN_STAT_BAR_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_OFF_ADDR, _gStatBarWifiImgPara[0].p_img_buf,
                     _gStatBarWifiImgPara[0].img_size);

    _gStatBarWifiImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_STAT_BAR_WIFI_ICON_SIZE);
    _gStatBarWifiImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gStatBarWifiImgPara[1].mem_img);
    _gStatBarWifiImgPara[1].img_size = EM_MAIN_STAT_BAR_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_ON_ADDR, _gStatBarWifiImgPara[1].p_img_buf,
                     _gStatBarWifiImgPara[1].img_size);

    _gStatBarTimeTextPara.p_content = _gStatBarTimeTextBuf;
    _gStatBarTimeTextPara.text_color = 0X00B2BE00;
    _gStatBarTimeTextPara.p_text_font = GUI_FONT_20_ASCII;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    EmKeyDefaultInfoSet();
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为背景图申请内存*/
    _gMemBK = GUI_ALLOC_AllocZero(EM_MAIN_ICON_BACKGROUND_SIZE);
    _pBKBuf = GUI_ALLOC_h2p(_gMemBK);
    SpiFlashReadData(EM_ICON_BACKGROUND_ADDR, _pBKBuf, EM_MAIN_ICON_BACKGROUND_SIZE);

    _InitElecStat();
    _InitTemp();

    _InitDevice();

    _InitWorkMode();
    _InitLedStat();
    _InitCharge();

    _InitStatBar();
}


static void _DelElecStat()
{
    GUI_ALLOC_Free(_gElecStatGridChargeImgPara.mem_img);
    GUI_ALLOC_Free(_gElecStatGridTiedImgPara.mem_img);
    GUI_ALLOC_Free(_gElecStatPVChargeImgPara.mem_img);
    GUI_ALLOC_Free(_gElecStatLoadImgPara.mem_img);
}

static void _DelTemp()
{
    GUI_ALLOC_Free(_gTempImgPara[0].mem_img);
    GUI_ALLOC_Free(_gTempImgPara[1].mem_img);
    GUI_ALLOC_Free(_gTempImgPara[2].mem_img);
}

static void _DelDevice()
{
    GUI_ALLOC_Free(_gDeviceModelImgPara.mem_img);
    GUI_ALLOC_Free(_gDeviceGridChargeImgPara[0].mem_img);
    GUI_ALLOC_Free(_gDeviceGridChargeImgPara[1].mem_img);
    GUI_ALLOC_Free(_gDevicePvChargeImgPara[0].mem_img);
    GUI_ALLOC_Free(_gDevicePvChargeImgPara[1].mem_img);
    GUI_ALLOC_Free(_gDeviceBackupImgPara[0].mem_img);
    GUI_ALLOC_Free(_gDeviceBackupImgPara[1].mem_img);
    GUI_ALLOC_Free(_gDeviceIndicateImgPara[0].mem_img);
    GUI_ALLOC_Free(_gDeviceIndicateImgPara[1].mem_img);
}

static void _DelWorkMode()
{
}

static void _DelLedStat()
{
}

static void _DelCharge()
{
    GUI_ALLOC_Free(_gChargeTypeImgPara[0].mem_img);
    GUI_ALLOC_Free(_gChargeTypeImgPara[1].mem_img);
    GUI_ALLOC_Free(_gChargeTypeImgPara[2].mem_img);
}

static void _DelStatBar()
{
    GUI_ALLOC_Free(_gStatBarWifiImgPara[0].mem_img);
    GUI_ALLOC_Free(_gStatBarWifiImgPara[1].mem_img);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    _DelElecStat();
    _DelTemp();
    _DelDevice();
    _DelWorkMode();
    _DelLedStat();
    _DelCharge();
    _DelStatBar();
    GUI_ALLOC_Free(_gMemBK);
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    if (gstCollectInfo.power_data.power_set_info.work_mode != _gstUiCtx.work_mode)
    {
        _gstUiCtx.work_mode = gstCollectInfo.power_data.power_set_info.work_mode;
        _SetPcsWorkMode((EN_WORK_MODE_E)_gstUiCtx.work_mode);
    }

    if (gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day != _gstUiCtx.electic_purchase_day)
    {
        _SetElecStatGridTied(_gstUiCtx.electic_purchase_day);
    }

    if (gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day != _gstUiCtx.electic_sale_day)
    {
        _SetElecStatGridCharge(_gstUiCtx.electic_sale_day);
    }

    if (gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day != _gstUiCtx.pv_sum_generation_on_day)
    {
        _SetElectStatPvCharge(_gstUiCtx.pv_sum_generation_on_day);
    }

    if (gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day != _gstUiCtx.load_cost_day)
    {
        _SetElecStatLoadCost(_gstUiCtx.load_cost_day);
    }

    if (_gstUiCtx.soc > 0)
    {
        _SetSoc(_gstUiCtx.soc);
    }

    if (_gstUiCtx.charge_type > 0)
    {
        _SetChargeType((EN_CHARGE_TYPE_E)_gstUiCtx.charge_type);
    }
    if (_gstUiCtx.charge_type & 0x01)
    {
        _SetLedStat(EN_LED_TYPE_GRID, EN_LED_STAT_OK);
    }
    else
    {
        _SetLedStat(EN_LED_TYPE_GRID, EN_LED_STAT_NONE);
    }

    if (_gstUiCtx.chassis_internal_temp > 0)
    {
        _SetTemperature(_gstUiCtx.chassis_internal_temp);
    }

    if (_gstUiCtx.net_state)
    {
        _SetWifiStat((EN_WIFI_STAT_E)_gstUiCtx.net_state);
    }

    if (_gstUiCtx.time > 0)
    {
        _SetTime(_gstUiCtx.time);
    }
}

/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
    int32_t event = EmGetKeyEvent(p_msg);
    if (event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }

    if (event == EN_KEY_SRC_CONFIRM)
    {
        if(pCurrentEmWin->win_handle != 0)
        {
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = &gstEmWinMain3DNav;
            pCurrentEmWin->create_win();
        }
        return;
    }
}


