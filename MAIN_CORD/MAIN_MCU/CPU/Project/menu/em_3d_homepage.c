/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_3d_homepage.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-01-20
  * @brief       3D版-首页
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-20    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"
#include "em_main.h"
#include "error_proc.h"
#include <time.h>

#define LAYOUT_DEBUG                                1
#define DEBUG_TIMER                                 1
#define TEMPERATURE_DEL                             0
#define BACK_ICON                                   0
#define OTHER_ICON_HIEDE                            0       
#define UNIFIED_UNIT_W                              0                

#define EM_MAIN_ICON_BACKGROUND_SIZE                (452 * 1024)
#define MAG                                         (3)
#define ON_GRID_LINE_COLOR                          (0x00B2BE00)//(0x009EFE0E)
#define OFF_GRID_LINE_COLOR                         (0x00D8D8D8)


#if 0
#define EM_MAIN3D_NAV_SELECT_SIZE                   (4 * 1024)
#define EM_MAIN3D_NAV_DEV_STAT_SIZE                 (4 * 1024 * 8)
#define EM_MAIN3D_NAV_SETTING_SIZE                  (4 * 1024 * 8)
#define EM_MAIN3D_NAV_DEV_INFO_SIZE                 (4 * 1024 * 8)
#define EM_MAIN3D_NAV_FACTORY_RESTORE_SIZE          (4 * 1024 * 8)
#endif

#define EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE               (4 * 1024)
#define EM_HOME_PAGE_STATE_BAR_WARNING_ICON_SIZE            (4 * 1024)

#define EM_HOME_PAGE_BOTTOM_HOME_ACTIVE_ICON_SIZE           (4 * 1024 * 16)
#define EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE           (4 * 1024 * 6)
#define EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE          (4 * 1024)
#define EM_HOME_PAGE_BOTTOM_SOLAR_PLATE_ICON_SIZE           (4 * 1024 * 20)

#define EM_HOME_PAGE_BOTTOM_TEMP_ICON_SIZE                  (4096)
#define EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE               (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinHomePage = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;

    uint32_t switcher[3];
    uint16_t net_state;
    uint32_t rtc_time;
    uint16_t sys_status;
    uint16_t work_mode;
    uint16_t warnning_state;
    uint32_t solar_power;
    uint32_t grid_power;
    uint32_t home_power;
    uint32_t ev_power;
    uint16_t roca_power;
    uint16_t temperature;
    uint16_t soc;
    uint16_t is_charge;
    uint16_t backup_state;
    uint16_t solar_state;
    uint16_t grid_state;
    uint16_t grid_curr;
    uint16_t ev_curr;
    uint32_t generator_power;
    uint16_t generator_enable;
    uint16_t bg_flag;
    uint16_t start_is_generator;
} UiCtx_t;

static UiCtx_t _gstUiCtx;
static uint16_t _gReadAllBmsInfoOkFlag;

/* 背景图内存 */
static GUI_HMEM _gHomeMemBK;
static uint8_t *_pHomePageBKBuf;
static __SDRAM uint8_t _garrHomePageBKBuf[1024 * 512];

/*各个子窗口控件句柄及其他相关参数*/
/*顶部状态栏子窗口*/
static GUI_HWIN _gWinStateBarHandle;//状态栏句柄
static IMAGE_Handle _gImgStateBarWifiHandle;//状态栏WIFI图标句柄
static IMAGE_Handle _gImgStateBarWarningHandle;//状态栏警告图标句柄
static TEXT_Handle  _gTextStateBarTimeHandle;//状态栏时间句柄
static TEXT_Handle  _gTextStateBarWorkMode1Handle;//状态栏工作模式句柄
static TEXT_Handle  _gTextStateBarWorkMode2Handle;//状态栏工作模式句柄
static TEXT_Handle  _gTextStateBarWarningHandle; //状态栏警告文字句柄

/*状态栏窗口文字参数*/
static emWidgetImgPara_t _garrStateBarWifiImgPara[4];     // 0:OFF 1: ON  2:disconnect 3:connect
static emWidgetImgPara_t _garrStateBarWarningImgPara[2];      // 0:WARNING, 1ERROR:
static emWidgetTextPara_t _gStateBarTimeTextPara;
static emWidgetTextPara_t _gStateBarWorkMode1TextPara;
static emWidgetTextPara_t _gStateBarWorkMode2TextPara;
static emWidgetTextPara_t _gStateBarWarningTextPara;
static char _garrStateBarTimeTextBuf[80] = {0}; // 2024 Jan 1th 00:00:01
static char _garrStateBarWorkMode1TextBuf[32] = {0};
static char _garrStateBarWorkMode2TextBuf[32] = {0};
static char _garrStateBarWarningTextBuf[8] = {0};


/*底部控件子窗口*/
static GUI_HWIN _gWinBottomHandle;//底部子窗口句柄
static IMAGE_Handle _gImgHomeActiveHandle;//底部流线图1图标句柄
static IMAGE_Handle _gImgGridActiveHandle;//底部流线图2图标句柄
static IMAGE_Handle _gImgSolarActiveHandle;//底部流线图3图标句柄
static IMAGE_Handle _gImgNewSolarActiveHandle;
static IMAGE_Handle _gImgSolarPlateHandle1;
static IMAGE_Handle _gImgSolarPlateHandle2;
/*底部左1*/
static TEXT_Handle _gTextBottomHomeHandle; //底部左1HOME句柄
static TEXT_Handle _gTextBottomHomePowerHandle; //底部左1HOME 功率句柄
/*底部中上*/
static TEXT_Handle _gTextBottomSolarHandle;
static TEXT_Handle _gTextBottomSolarPowerHandle;
/*底部右中*/
static TEXT_Handle _gTextBottomGridHandle;
static TEXT_Handle _gTextBottomGridPowerHandle;

/*底部右中*/
static TEXT_Handle _gTextBottomEVHandle;
static TEXT_Handle _gTextBottomEVPowerHandle;

/*底部右中*/
static TEXT_Handle _gTextBottomGenHandle;
static TEXT_Handle _gTextBottomGenPowerHandle;

/*底部中间温度显示*/
static TEXT_Handle _gTextBottomTempHandle;
static IMAGE_Handle _gImgBottomTempHandle;//底部中间温度图标句柄
/*底部中下*/
static TEXT_Handle _gTextBottomBatteryHandle;
static IMAGE_Handle _gImgBottomBatteryHandle;//底部中下电池图标句柄
static TEXT_Handle _gTextBottomRocaHandle;
static TEXT_Handle _gTextBottomRocaPowerHandle;
static TEXT_Handle _gTextBottomChargeHandle;

#if OTHER_ICON_HIDE
/*底部控件文字参数*/
static emWidgetImgPara_t  _garrHomeActiveImgPara[2];   //0:disabled 1:active
static emWidgetImgPara_t  _garrGridActiveImgPara[3];   //0:disabled 1:in grid  2:out grid
static emWidgetImgPara_t  _garrSolarActiveImgPara[3];  //0:disabled 1:active   2:daytime
static emWidgetImgPara_t  _gstNewSolarActiveImgPara;
static emWidgetImgPara_t  _gstNewSolarPlateImgPara;
#endif
/*底部左1*/
static emWidgetTextPara_t _gstBottomHomeTextPara;
static emWidgetTextPara_t _gstBottomHomePowerTextPara;
/*底部中上*/
static emWidgetTextPara_t _gstBottomSolarTextPara;
static emWidgetTextPara_t _gstBottomSolarPowerTextPara;
/*底部右中*/
static emWidgetTextPara_t _gstBottomGridTextPara;
static emWidgetTextPara_t _gstBottomGridPowerTextPara;
/*底部左下 ev*/
static emWidgetTextPara_t _gstBottomEVTextPara;
static emWidgetTextPara_t _gstBottomEVPowerTextPara;
/*底部右下 gen*/
static emWidgetTextPara_t _gstBottomGenTextPara;
static emWidgetTextPara_t _gstBottomGenPowerTextPara;
/*底部中间温度显示*/
static emWidgetTextPara_t _gstBottomTempTextPara;
static emWidgetImgPara_t  _gstBottomTempImgPara;
/*底部中下*/
static emWidgetTextPara_t _gstBottomBatteryTextPara;
static emWidgetImgPara_t  _garrBottomBatteryImgPara[4]; //0:100%  1：80%~99%   2：20%~79%  3:0%~19%
static emWidgetTextPara_t _gstBottomRocaTextPara;
static emWidgetTextPara_t _gstBottomRocaPowerTextPara;
static emWidgetTextPara_t _gstBottomChargeTextPara;

static char _garrBottomHomeTextBuf[20] = {0};
static char _garrBottomHomePowerTextBuf[20] = {0};
static char _garrBottomSolarTextBuf[20] = {0};
static char _garrBottomSolarPowerTextBuf[20] = {0};
static char _garrBottomGridTextBuf[20] = {0};
static char _garrBottomGridPowerTextBuf[20] = {0};
static char _garrBottomEVTextBuf[20] = {0};
static char _garrBottomEVPowerTextBuf[20] = {0};
static char _garrBottomGenTextBuf[20] = {0};
static char _garrBottomGenPowerTextBuf[20] = {0};
static char _garrBottomTempTextBuf[20] = {0};
static char _garrBottomBatteryTextBuf[20] = {0};
static char _garrBottomRocaTextBuf[20] = {0};
static char _garrBottomRocaPowerTextBuf[20] = {0};
static char _garrBottomChargeTextBuf[20] = {0};


/*顶部状态栏控件ID号*/
#define WIN_MAIN_STATE_BAR_ID                               (GUI_ID_USER + 0x10)
#define WIN_MAIN_STATE_BAR_WIFI_IMG_ID                      (GUI_ID_USER + 0x11)
#define WIN_MAIN_STATE_BAR_WARNING_IMG_ID                   (GUI_ID_USER + 0x12)
#define WIN_MAIN_STATE_BAR_TIME_TEXT_ID                     (GUI_ID_USER + 0x13)
#define WIN_MAIN_STATE_BAR_WORK_MODE1_TEXT_ID               (GUI_ID_USER + 0x14)
#define WIN_MAIN_STATE_BAR_WORK_MODE2_TEXT_ID               (GUI_ID_USER + 0x15)
#define WIN_MAIN_STATE_BAR_WORK_MODE3_PEN_ID                (GUI_ID_USER + 0x16)
#define WIN_MAIN_STATE_BAR_WARNING_TEXT_ID                  (GUI_ID_USER + 0x17)


/*底部控件ID号*/
#define WIN_HOME_PAGE_BOTTOM_GEN_ID                         (GUI_ID_USER + 0x1E)
#define WIN_HOME_PAGE_BOTTOM_GEN_POWER_ID                   (GUI_ID_USER + 0x1F)
#define WIN_HOME_PAGE_BOTTOM_HOME_ID                        (GUI_ID_USER + 0x20)
#define WIN_HOME_PAGE_BOTTOM_HOME_POWER_ID                  (GUI_ID_USER + 0x21)
#define WIN_HOME_PAGE_BOTTOM_SOLAR_ID                       (GUI_ID_USER + 0x22)
#define WIN_HOME_PAGE_BOTTOM_SOLAR_POWER_ID                 (GUI_ID_USER + 0x23)
#define WIN_HOME_PAGE_BOTTOM_GRID_ID                        (GUI_ID_USER + 0x24)
#define WIN_HOME_PAGE_BOTTOM_GRID_POWER_ID                  (GUI_ID_USER + 0x25)
#define WIN_HOME_PAGE_BOTTOM_TEMP_TEXT_ID                   (GUI_ID_USER + 0x26)
#define WIN_HOME_PAGE_BOTTOM_TEMP_IMG_ID                    (GUI_ID_USER + 0x27)
#define WIN_HOME_PAGE_BOTTOM_BATTERY_ID                     (GUI_ID_USER + 0x28)
#define WIN_HOME_PAGE_BOTTOM_BATTERY_IMG_ID                 (GUI_ID_USER + 0x29)
#define WIN_HOME_PAGE_BOTTOM_ROCA_ID                        (GUI_ID_USER + 0x2A)
#define WIN_HOME_PAGE_BOTTOM_ROCA_POWER_ID                  (GUI_ID_USER + 0x2B)
#define WIN_HOME_PAGE_BOTTOM_CHARGE_ID                      (GUI_ID_USER + 0x2C)
#define WIN_HOME_PAGE_BOTTOM_EV_ID                          (GUI_ID_USER + 0x2D)
#define WIN_HOME_PAGE_BOTTOM_EV_POWER_ID                    (GUI_ID_USER + 0x2E)


#define WIN_HOME_PAGE_STREAM_LINE_ID                        (GUI_ID_USER + 0x30)
#define WIN_HOME_PAGE_GRID_LINE_ID                          (GUI_ID_USER + 0x31)
//#define WIN_HOME_PAGE_SOLAR_LINE_ID                         (GUI_ID_USER + 0x32)
#define WIN_HOME_PAGE_NEW_SOLAR_LINE_ID                     (GUI_ID_USER + 0x33)
#define WIN_HOME_PAGE_SOLAR_PLATE1_ID                       (GUI_ID_USER + 0x34)
#define WIN_HOME_PAGE_SOLAR_PLATE2_ID                       (GUI_ID_USER + 0x35)



/* layout 背景色*/
#define GUI_COLOR_WIN_MAIN_STAT_BAR_LAYOUT                      GUI_MAKE_COLOR(0x001030A0)
#define GUI_COLOR_WIN_HOMEPAGE_STATE_BAR                        GUI_MAKE_COLOR(0x00434343)
#define GUI_COLOR_WIN_HOMEPAGE_STATE_BAR_PEN                    GUI_MAKE_COLOR(0x00797979)
#define GUI_COLOR_WIN_MAIN_STAT_BAR_WORK_MODE                   GUI_MAKE_COLOR(0x00434343)
#define GUI_COLOR_WIN_MAIN_ELEC_STAT                            GUI_MAKE_COLOR(0x00363A23)

/*函数声明*/
int32_t SetHomePageWifiStat(uint8_t ui_update, EN_WIFI_STAT_E e_stat);
int32_t SetHomePageWarning(uint8_t ui_update, EN_FAULT_STAT_E e_stat, uint32_t num);
int32_t SetHomePageSysStatus(uint8_t ui_update, EN_SYS_STATUS_E e_mode);
int32_t SetHomePageWorkMode(uint8_t ui_update, EN_WORK_MODE_E e_mode);
int32_t SetHomePageTime(uint8_t ui_update, uint32_t t);
int32_t SetHomePower(uint8_t ui_update, uint32_t value_w);
int32_t SetSolarPower(uint8_t ui_update, uint32_t value_w);
int32_t SetGridPower(uint8_t ui_update, uint32_t value_w, uint16_t value_curr);
int32_t SetBatteryPower(uint8_t ui_update, int16_t value_w);
int32_t SetIsCharge(uint8_t ui_update, uint16_t value_charge);
int32_t SetGridPowerstat(uint8_t ui_update, EN_GRID_STAT_E e_stat);
int32_t SetHomePowerstat(uint8_t ui_update, EN_HOME_STAT_E e_stat);
int32_t SetSolarPowerstat(uint8_t ui_update, EN_SOLAR_STAT_E e_stat);


/*资源列表*/
/**
 * @brief 顶部状态栏
 */
static __SDRAM GUI_WIDGET_CREATE_INFO _garrStateBarDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "StatBar",        WIN_MAIN_STATE_BAR_ID,                          0,                  0,      EM_XSIZE,   50,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "time",           WIN_MAIN_STATE_BAR_TIME_TEXT_ID,                15,                 0,      140,        50,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "stand",          WIN_MAIN_STATE_BAR_WORK_MODE1_TEXT_ID,          148 + 5,            0,      80,         50,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "prioity",        WIN_MAIN_STATE_BAR_WORK_MODE2_TEXT_ID,          148 + 95,           0,      130,        50,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "warningsize",    WIN_MAIN_STATE_BAR_WARNING_TEXT_ID,             EM_XSIZE - 104,     0,      38,         50,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "ImgWarning",     WIN_MAIN_STATE_BAR_WARNING_IMG_ID,              EM_XSIZE - 60,      15,     20,         55,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "ImgWifi",        WIN_MAIN_STATE_BAR_WIFI_IMG_ID,                 EM_XSIZE - 38,      8,      40,         55,   0,   0x0,  0 },
};

/**
 * @brief 中间及底层信息栏
 */
static __SDRAM GUI_WIDGET_CREATE_INFO  _garrBottomTextInfoDialogCreate[] =
{

    { WINDOW_CreateIndirect,  "StatBar",        WIN_MAIN_STATE_BAR_ID,                      0,      58,     EM_XSIZE,  EM_YSIZE - 58,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "Imghome",        WIN_HOME_PAGE_STREAM_LINE_ID,               37,     172 - 58,     280,   72,      0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "Imggrid",        WIN_HOME_PAGE_GRID_LINE_ID,                 330,    183 - 58,     120,   70,      0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "Imgsolar",       WIN_HOME_PAGE_NEW_SOLAR_LINE_ID,            323,    130 - 58,     12,     73,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "HOME",           WIN_HOME_PAGE_BOTTOM_HOME_ID,               50,     83 - 58,     52,     17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "HOMEPOWER",      WIN_HOME_PAGE_BOTTOM_HOME_POWER_ID,         50,     60 - 58,     100,     22,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "SOLAR",          WIN_HOME_PAGE_BOTTOM_SOLAR_ID,              230,    83  - 58,     100,     17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "SOLARPOWER",     WIN_HOME_PAGE_BOTTOM_SOLAR_POWER_ID,        230,    60  - 58,     100,     22,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "GRID",           WIN_HOME_PAGE_BOTTOM_GRID_ID,               390,    83  - 58,     100,     17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "GRIDPOWER",      WIN_HOME_PAGE_BOTTOM_GRID_POWER_ID,         390,    60  - 58,     100,     22,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "EV",             WIN_HOME_PAGE_BOTTOM_EV_ID,                 150,    283 - 58,     100,     17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "EVPOWER",        WIN_HOME_PAGE_BOTTOM_EV_POWER_ID,           150,    260 - 58,     100,     22,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Gen",            WIN_HOME_PAGE_BOTTOM_GEN_ID,                390,    275 - 58,     100,     17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Genpower",       WIN_HOME_PAGE_BOTTOM_GEN_POWER_ID,          390,    252 - 58,     100,     22,     0,   0x0,  0  },
//    { IMAGE_CreateIndirect,   "ImgTEMP",        WIN_HOME_PAGE_BOTTOM_TEMP_IMG_ID,           265,    280 - 58,     16,     16,     0,   0x0,  0 },
//    { TEXT_CreateIndirect,    "TEMP",           WIN_HOME_PAGE_BOTTOM_TEMP_TEXT_ID,          280,    280 - 58,     60,     17,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "Imgbat",         WIN_HOME_PAGE_BOTTOM_BATTERY_IMG_ID,        275,    260 - 58,      7,     14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "battext",        WIN_HOME_PAGE_BOTTOM_BATTERY_ID,            287,    258 - 58,     60,     22,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "solarplate1",     WIN_HOME_PAGE_SOLAR_PLATE1_ID,             257,     84 - 58,     90,     64,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "solarplate2",     WIN_HOME_PAGE_SOLAR_PLATE2_ID,             189,     84 - 58,     90,     64,     0,   0x0,  0 },
//    { TEXT_CreateIndirect,    "roca",           WIN_HOME_PAGE_BOTTOM_ROCA_ID,               286,    274 - 58,     50,     17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "rocapower",      WIN_HOME_PAGE_BOTTOM_ROCA_POWER_ID,         280,    283 - 58,     100,     22,     0,   0x0,  0 },
//    { TEXT_CreateIndirect,    "charge",         WIN_HOME_PAGE_BOTTOM_CHARGE_ID,             254,    293 - 58,     78,     17,     0,   0x0,  0 },
};

/**
  * @brief  PV状态线
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetPvStatusLine(uint32_t pv_value)
{
    int32_t value_tmp = 0;
    value_tmp = (int32_t)pv_value;
    value_tmp = abs(value_tmp);

    if(value_tmp != 0)
    {
        GUI_SetPenSize(3);
        GUI_SetColor(ON_GRID_LINE_COLOR);
        /* 使能高分辨率 */
        GUI_AA_EnableHiRes();

        GUI_AA_SetFactor(MAG);
        GUI_AA_DrawLine(280 * MAG, (126 - 58) * MAG, 280 * MAG, (126 - 58 + 66) * MAG);
        // pv箭头
        GUI_AA_DrawLine(275 * MAG, (142 - 58) * MAG, 279 * MAG, (146 - 58) * MAG);
        GUI_AA_DrawLine(281 * MAG, (146 - 58) * MAG, 285 * MAG, (142 - 58) * MAG);

        GUI_AA_DrawLine(275 * MAG, (142 - 58 + 26) * MAG, 279 * MAG, (146 - 58 + 26) * MAG);
        GUI_AA_DrawLine(281 * MAG, (146 - 58 + 26) * MAG, 285 * MAG, (142 - 58 + 26) * MAG);
    }
    else
    {
        //灰色线条
        GUI_SetPenSize(3);
        GUI_SetColor(OFF_GRID_LINE_COLOR);

        /* 使能高分辨率 */
        GUI_AA_EnableHiRes();

        GUI_AA_SetFactor(MAG);
        GUI_AA_DrawLine(280 * MAG, (126 - 58) * MAG, 280 * MAG, (126 - 58 + 66) * MAG);
    }
    return 0;
}
/**
  * @brief  Back_up状态线
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetHomePowerLine(uint32_t home_power)
{
    int32_t value_tmp = 0;
    value_tmp = (int32_t)home_power;
    value_tmp = abs(value_tmp);

    if(value_tmp != 0)
    {
        if(value_tmp > 100)
        {
            //负载
            GUI_SetPenSize(3);
            GUI_SetColor(ON_GRID_LINE_COLOR);
            /* 使能高分辨率 */
            GUI_AA_EnableHiRes();

            GUI_AA_SetFactor(MAG);
            GUI_AA_DrawLine(270 * MAG, (190 - 58 - 4) * MAG, 270 * MAG, (194 - 58) * MAG);
            GUI_AA_DrawLine(270 * MAG, (190 - 58 - 4) * MAG, (270 - 30) * MAG, (190 - 58) * MAG);
            GUI_AA_DrawLine((270 - 30) * MAG, (190 - 58) * MAG, 168 * MAG, (181 - 58) * MAG);
            GUI_AA_DrawLine(168 * MAG, (181 - 58) * MAG, 168 * MAG, (190 - 58) * MAG);

            //负载箭头1
            GUI_AA_DrawLine(200 * MAG, (187 - 58) * MAG, 203 * MAG, (191 - 58) * MAG);
            GUI_AA_DrawLine(200 * MAG, (184 - 58) * MAG, 204 * MAG, (181 - 58) * MAG);
            //负载箭头2
            GUI_AA_DrawLine(253 * MAG, (187 - 58) * MAG, 257 * MAG, (183 - 58) * MAG);
            GUI_AA_DrawLine(253 * MAG, (189 - 58) * MAG, 258 * MAG, (193 - 58) * MAG);
        }
        else
        {
            //负载
            GUI_SetPenSize(3);
            GUI_SetColor(OFF_GRID_LINE_COLOR);
            /* 使能高分辨率 */
            GUI_AA_EnableHiRes();

            GUI_AA_SetFactor(MAG);
            GUI_AA_DrawLine(270 * MAG, (190 - 58 - 4) * MAG, 270 * MAG, (194 - 58) * MAG);
            GUI_AA_DrawLine(270 * MAG, (190 - 58 - 4) * MAG, (270 - 30) * MAG, (190 - 58) * MAG);
            GUI_AA_DrawLine((270 - 30) * MAG, (190 - 58) * MAG, 168 * MAG, (181 - 58) * MAG);
            GUI_AA_DrawLine(168 * MAG, (181 - 58) * MAG, 168 * MAG, (190 - 58) * MAG);
        }
    }
    else
    {
        //负载
        GUI_SetPenSize(3);
        GUI_SetColor(OFF_GRID_LINE_COLOR);
        /* 使能高分辨率 */
        GUI_AA_EnableHiRes();

        GUI_AA_SetFactor(MAG);
        GUI_AA_DrawLine(270 * MAG, (190 - 58 - 4) * MAG, 270 * MAG, (194 - 58) * MAG);
        GUI_AA_DrawLine(270 * MAG, (190 - 58 - 4) * MAG, (270 - 30) * MAG, (190 - 58) * MAG);
        GUI_AA_DrawLine((270 - 30) * MAG, (190 - 58) * MAG, 168 * MAG, (181 - 58) * MAG);
        GUI_AA_DrawLine(168 * MAG, (181 - 58) * MAG, 168 * MAG, (190 - 58) * MAG);
    }
    return 0;
}
/**
  * @brief  ON GRID状态线
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetGridPowerLine(uint32_t grid_power, uint16_t grid_curr)
{
    int32_t value_tmp = 0;
    int16_t tmp_curr = 0;
    value_tmp = (int32_t)grid_power;
    value_tmp = abs(value_tmp);
    tmp_curr = (int16_t)grid_curr;

    if(value_tmp != 0)
    {
        if(tmp_curr < 0)
        {
            //电网
            GUI_SetPenSize(3);
            GUI_SetColor(ON_GRID_LINE_COLOR);
            /* 使能高分辨率 */
            GUI_AA_EnableHiRes();
            GUI_AA_SetFactor(MAG);
            GUI_AA_DrawLine(340 * MAG, (65  - 58)* MAG, (340) * MAG, (170 - 58) * MAG);
            GUI_AA_DrawLine(340 * MAG, (170 - 58)* MAG, (291) * MAG, (173 - 58) * MAG);
            GUI_AA_DrawLine(291 * MAG, (173 - 58)* MAG, (291) * MAG, (191 - 58) * MAG);

            GUI_AA_DrawLine(339 * MAG, (112 - 58) * MAG, 335 * MAG, (116 - 58) * MAG);
            GUI_AA_DrawLine(341 * MAG, (112 - 58) * MAG, 345 * MAG, (116 - 58) * MAG);

            GUI_AA_DrawLine(339 * MAG, (142 - 58) * MAG, 335 * MAG, (146 - 58) * MAG);
            GUI_AA_DrawLine(341 * MAG, (142 - 58) * MAG, 345 * MAG, (146 - 58) * MAG);

            GUI_AA_DrawLine(319 * MAG, (166 - 58) * MAG, 323 * MAG, (170 - 58) * MAG);
            GUI_AA_DrawLine(319 * MAG, (176 - 58) * MAG, 323 * MAG, (172 - 58) * MAG); //okk    to grid
        }
        else
        {
            //电网
            GUI_SetPenSize(3);
            GUI_SetColor(ON_GRID_LINE_COLOR);
            /* 使能高分辨率 */
            GUI_AA_EnableHiRes();

            GUI_AA_SetFactor(MAG);
            GUI_AA_DrawLine(340 * MAG, (65  - 58)* MAG, (340) * MAG, (170 - 58) * MAG);
            GUI_AA_DrawLine(340 * MAG, (170 - 58)* MAG, (291) * MAG, (173 - 58) * MAG);
            GUI_AA_DrawLine(291 * MAG, (173 - 58)* MAG, (291) * MAG, (191 - 58) * MAG);
            //箭头
            GUI_AA_DrawLine(335 * MAG, (112 - 58) * MAG, 339 * MAG, (116 - 58) * MAG);
            GUI_AA_DrawLine(345 * MAG, (112 - 58) * MAG, 341 * MAG, (116 - 58) * MAG);

            GUI_AA_DrawLine(335 * MAG, (142 - 58) * MAG, 339 * MAG, (146 - 58) * MAG);
            GUI_AA_DrawLine(345 * MAG, (142 - 58) * MAG, 341 * MAG, (146 - 58) * MAG);

            GUI_AA_DrawLine(319 * MAG, (170 - 58) * MAG, 323 * MAG, (166 - 58) * MAG);
            GUI_AA_DrawLine(319 * MAG, (172 - 58) * MAG, 323 * MAG, (176 - 58) * MAG); //okk    to pcs
        }
    }
    else
    {
        //电网
        GUI_SetPenSize(3);
        GUI_SetColor(OFF_GRID_LINE_COLOR);
        /* 使能高分辨率 */
        GUI_AA_EnableHiRes();

        GUI_AA_SetFactor(MAG);
        GUI_AA_DrawLine(340 * MAG, (65  - 58)* MAG, (340) * MAG, (170 - 58) * MAG);
        GUI_AA_DrawLine(340 * MAG, (170 - 58)* MAG, (291) * MAG, (173 - 58) * MAG);
        GUI_AA_DrawLine(291 * MAG, (173 - 58)* MAG, (291) * MAG, (191 - 58) * MAG);
    }
    return 0;
}
/**
  * @brief  油机接入状态线
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetGeneratorPowerLine(uint32_t power, uint16_t gen_enable)
{
    int32_t value_tmp = 0;
    value_tmp = (int32_t)power;
    value_tmp = abs(value_tmp);

    if(value_tmp != 0 && (gen_enable == 1))
    {
        GUI_SetPenSize(3);
        GUI_SetColor(ON_GRID_LINE_COLOR);
        /* 使能高分辨率 */
        GUI_AA_EnableHiRes();

        GUI_AA_SetFactor(MAG);
        GUI_AA_DrawLine(300 * MAG, (214 - 58)* MAG, (316) * MAG, (212 - 58) * MAG);
        GUI_AA_DrawLine(316 * MAG, (212 - 58)* MAG, (316) * MAG, (245 - 58) * MAG);
        GUI_AA_DrawLine(316 * MAG, (245 - 58)* MAG, (386) * MAG, (237 - 58) * MAG);
        //箭头1
        GUI_AA_DrawLine(340 * MAG, (241 - 58) * MAG, 344 * MAG, (236 - 58) * MAG);
        GUI_AA_DrawLine(340 * MAG, (243 - 58) * MAG, 345 * MAG, (247 - 58) * MAG);
        //箭头2
        GUI_AA_DrawLine(370 * MAG, (238 - 58) * MAG, 374 * MAG, (233 - 58) * MAG);
        GUI_AA_DrawLine(370 * MAG, (239 - 58) * MAG, 375 * MAG, (243 - 58) * MAG);  //okk to pcs
    }
    else
    {
        GUI_SetPenSize(3);
        GUI_SetColor(OFF_GRID_LINE_COLOR);
        /* 使能高分辨率 */
        GUI_AA_EnableHiRes();

        GUI_AA_SetFactor(MAG);
        GUI_AA_DrawLine(300 * MAG, (214 - 58)* MAG, (316) * MAG, (212 - 58) * MAG);
        GUI_AA_DrawLine(316 * MAG, (212 - 58)* MAG, (316) * MAG, (245 - 58) * MAG);
        GUI_AA_DrawLine(316 * MAG, (245 - 58)* MAG, (386) * MAG, (237 - 58) * MAG);
    }
    return 0;
}
/**
  * @brief  EV接入状态线
  * @param  NULL
  * @retval NULL
  */

static int32_t _SetEVPowerLine(uint32_t power, uint16_t ev_curr)
{
    int32_t value_tmp = 0;
    int16_t tmp_curr = 0;
    value_tmp = (int32_t)power;
    value_tmp = abs(value_tmp);
    tmp_curr = (int16_t)ev_curr;

    if(value_tmp != 0)
    {
        if(tmp_curr < 0)
        {
            GUI_SetPenSize(3);
            GUI_SetColor(ON_GRID_LINE_COLOR);
            /* 使能高分辨率 */
            GUI_AA_EnableHiRes();

            GUI_AA_SetFactor(MAG);
            GUI_AA_DrawLine(264 * MAG, (212 - 58 - 3) * MAG, 240 * MAG, (212 - 58) * MAG);
            //箭头
            GUI_AA_DrawLine(252 * MAG, (209 - 58) * MAG, 256 * MAG, (205 - 58) * MAG);
            GUI_AA_DrawLine(252 * MAG, (211 - 58) * MAG, 257 * MAG, (215 - 58) * MAG);  //to ev
        }
        else
        {
            GUI_SetPenSize(3);
            GUI_SetColor(ON_GRID_LINE_COLOR);
            /* 使能高分辨率 */
            GUI_AA_EnableHiRes();

            GUI_AA_SetFactor(MAG);
            GUI_AA_DrawLine(264 * MAG, (212 - 58 - 3) * MAG, 240 * MAG, (212 - 58) * MAG);
            //箭头
            GUI_AA_DrawLine(251 * MAG, (205 - 58) * MAG, 256 * MAG, (209 - 58) * MAG);
            GUI_AA_DrawLine(252 * MAG, (215 - 58) * MAG, 256 * MAG, (211 - 58) * MAG);  //to pcs
        }
    }
    else
    {
        GUI_SetPenSize(3);
        GUI_SetColor(OFF_GRID_LINE_COLOR);
        /* 使能高分辨率 */
        GUI_AA_EnableHiRes();

        GUI_AA_SetFactor(MAG);
        GUI_AA_DrawLine(264 * MAG, (212 - 58 - 3) * MAG, 240 * MAG, (212 - 58) * MAG);
    }
    return 0;
}
/**
  * @brief  Wifi状态变换
  * @param  NULL
  * @retval NULL
  */
int32_t SetHomePageWifiStat(uint8_t ui_update, EN_WIFI_STAT_E e_stat)
{
    emWidgetImgPara_t  *p_st_tmp_wifi_img_para = &_garrStateBarWifiImgPara[0];
    switch (e_stat)
    {
        case EN_WIFI_STAT_DISCONN:
        {
            p_st_tmp_wifi_img_para = &_garrStateBarWifiImgPara[0];
            break;
        }
        case EN_WIFI_STAT_CONN:
        {
            p_st_tmp_wifi_img_para = &_garrStateBarWifiImgPara[1];
            break;
        }
        case EN_WIFI_STAT_ON:
        {
            p_st_tmp_wifi_img_para = &_garrStateBarWifiImgPara[2];
            break;
        }
        case EN_WIFI_STAT_OFF:
        {
            p_st_tmp_wifi_img_para = &_garrStateBarWifiImgPara[3];
            break;
        }
        default:
            break;
    }
    if(ui_update)
    {
        IMAGE_SetDTA(_gImgStateBarWifiHandle, p_st_tmp_wifi_img_para->p_img_buf,
                     p_st_tmp_wifi_img_para->img_size);
    }
    return 0;
}



/**
  * @brief  警告图标变换
  * @param  stat:故障类型
  * @param  num:故障类型码  0：正常  非零：对应故障类型
  * @retval 0:success
  */
int32_t SetHomePageWarning(uint8_t ui_update, EN_FAULT_STAT_E e_stat, uint32_t num)
{
    emWidgetImgPara_t  *p_st_tmp_fault_img_para = &_garrStateBarWarningImgPara[0];
    if(e_stat > EN_FAULT_STAT)
    {
        return -1;
    }

    if (num > 2999)
    {
        return -2;
    }

    if(num >= 1000)
    {
        snprintf(_garrStateBarWarningTextBuf, sizeof(_garrStateBarWarningTextBuf), "%.4d", num);
    }
    else if(num == 0xFFFF)
    {
        snprintf(_garrStateBarWarningTextBuf, sizeof(_garrStateBarWarningTextBuf), "%s", " ");
    }
    else
    {
        snprintf(_garrStateBarWarningTextBuf, sizeof(_garrStateBarWarningTextBuf), "%.3d", num);
    }

    switch (e_stat)
    {
        case EN_WARNNING_STAT:
        {
            _gStateBarWarningTextPara.text_color = 0X0000BEF7;
            p_st_tmp_fault_img_para = &_garrStateBarWarningImgPara[0];
            break;
        }
        case EN_FAULT_STAT:
        {
            _gStateBarWarningTextPara.text_color = 0X006A62FF;
            p_st_tmp_fault_img_para = &_garrStateBarWarningImgPara[1];
            /*IMAGE_SetDTA(_gImgStateBarWarningHandle, tmp_fault_img_para->p_img_buf,
                         tmp_fault_img_para->img_size);*/
            break;
        }
        case EN_NORMAL_STAT:
        {
            snprintf(_garrStateBarWarningTextBuf, sizeof(_garrStateBarWarningTextBuf), "%s", " ");
            p_st_tmp_fault_img_para = NULL;
            break;
        }
        default:
            break;
    }

    if(ui_update)
    {
        TEXT_SetTextColor(_gTextStateBarWarningHandle, _gStateBarWarningTextPara.text_color);
        TEXT_SetText(_gTextStateBarWarningHandle, (const char *)_gStateBarWarningTextPara.p_content);
        if (e_stat == EN_NORMAL_STAT)
        {
            WM_HideWindow(_gImgStateBarWarningHandle);
        }
        else
        {
            WM_ShowWindow(_gImgStateBarWarningHandle);
            IMAGE_SetDTA(_gImgStateBarWarningHandle, p_st_tmp_fault_img_para->p_img_buf,
                         p_st_tmp_fault_img_para->img_size);
        }
    }
    return 0;
}

/**
  * @brief  工作模式
  * @param  NULL
  * @retval NULL
  */
int32_t SetHomePageSysStatus(uint8_t ui_update, EN_SYS_STATUS_E e_mode)
{
    switch (e_mode)
    {
        case EN_SYS_STAND_BY:
        {
            _gStateBarWorkMode1TextPara.text_color = 0X0036B6FF;
            snprintf(_garrStateBarWorkMode1TextBuf, sizeof(_garrStateBarWorkMode1TextBuf),
                     "%s", "Stand-by");
            break;
        }
        case EN_SYS_OFF_GRID:
        {
            _gStateBarWorkMode1TextPara.text_color = 0X00D9D9D9;
            snprintf(_garrStateBarWorkMode1TextBuf, sizeof(_garrStateBarWorkMode1TextBuf),
                     "%s", "Off Grid");
            break;
        }
        case EN_SYS_ON_GRID:
        {
            _gStateBarWorkMode1TextPara.text_color = 0X00B2BE00;
            snprintf(_garrStateBarWorkMode1TextBuf, sizeof(_garrStateBarWorkMode1TextBuf),
                     "%s", "On Grid");
            break;
        }
        case EN_SYS_SELF_TEST:
        {
            _gStateBarWorkMode1TextPara.text_color = 0X0036B6FF;
            snprintf(_garrStateBarWorkMode1TextBuf, sizeof(_garrStateBarWorkMode1TextBuf),
                     "%s", "Self-test");
            break;
        }
        case EN_SYS_BY_PASS:
        {
            _gStateBarWorkMode1TextPara.text_color = 0X00FFC44C;
            snprintf(_garrStateBarWorkMode1TextBuf, sizeof(_garrStateBarWorkMode1TextBuf),
                     "%s", "Bypass");
            break;
        }
        case EN_SYS_SHUTDOWN:
        {
            _gStateBarWorkMode1TextPara.text_color = 0X00D9D9D9;
            snprintf(_garrStateBarWorkMode1TextBuf, sizeof(_garrStateBarWorkMode1TextBuf),
                     "%s", "Shutdown");
            break;
        }
        default:
            break;
    }

    if(ui_update)
    {
        TEXT_SetTextColor(_gTextStateBarWorkMode1Handle, _gStateBarWorkMode1TextPara.text_color);
        TEXT_SetText(_gTextStateBarWorkMode1Handle, (const char *)_gStateBarWorkMode1TextPara.p_content);
    }
    return 0;
}

/**
  * @brief  PCS工作模式
  * @param  NULL
  * @retval NULL
  */
int32_t SetHomePageWorkMode(uint8_t ui_update, EN_WORK_MODE_E e_mode)
{
    switch(e_mode)
    {
        case EN_WORK_MODE_SELF_CONSUMP:
        {
            snprintf(_garrStateBarWorkMode2TextBuf, sizeof(_garrStateBarWorkMode2TextBuf),
                     "%s", "Self Consump");
            break;
        }
        case EN_WORK_MODE_BAT_PRIORITY:
        {
            snprintf(_garrStateBarWorkMode2TextBuf, sizeof(_garrStateBarWorkMode2TextBuf),
                     "%s", "BAT Priority");
            break;
        }
        case EN_WORK_MODE_PL_BALANCE:
        {
            snprintf(_garrStateBarWorkMode2TextBuf, sizeof(_garrStateBarWorkMode2TextBuf),
                     "%s", "Peak Shifting");
            break;
        }
        default:
            break;
    }

    if(ui_update)
    {
        TEXT_SetText(_gTextStateBarWorkMode2Handle, (const char *)_gStateBarWorkMode2TextPara.p_content);
    }
    return 0;
}


/**
  * @brief  显示时间
  * @param  NULL
  * @retval NULL
  */
int32_t SetHomePageTime(uint8_t ui_update, uint32_t time)
{
    ShowRtcTime(time, _garrStateBarTimeTextBuf, sizeof(_garrStateBarTimeTextBuf));

    if(ui_update)
    {
        TEXT_SetText(_gTextStateBarTimeHandle, (const char *)_gStateBarTimeTextPara.p_content);
    }
    return 0;
}

/**
  * @brief  设置界面中家庭功率文本
  * @param  value_kw:功率值
  * @param
  * @retval 0:success
  */
int32_t SetHomePower(uint8_t ui_update, uint32_t value_w)
{
    int32_t value_tmp = 0;
    value_tmp = (int32_t)value_w;
    value_tmp = abs(value_tmp);
    float temp_value = ((float)value_tmp) / 1000;
#if OTHER_ICON_HIEDE
    emWidgetImgPara_t *p_st_tmp_ImgPara = &_garrHomeActiveImgPara[0];
#endif

#if UNIFIED_UNIT_W
    if (value_tmp < 1000)
    {
        if(value_tmp < 100)
        {
            snprintf(_garrBottomHomePowerTextBuf, sizeof(_garrBottomHomePowerTextBuf), "%dW", 0);
        }
        else
        {
            snprintf(_garrBottomHomePowerTextBuf, sizeof(_garrBottomHomePowerTextBuf), "%dW", value_tmp);
        }
    }
    else
    {
        snprintf(_garrBottomHomePowerTextBuf, sizeof(_garrBottomHomePowerTextBuf), "%.2fkW", temp_value);
    }
#else 
    snprintf(_garrBottomHomePowerTextBuf, sizeof(_garrBottomHomePowerTextBuf), "%.1fkW", temp_value);

#endif
    if(ui_update)
    {
        TEXT_SetText(_gTextBottomHomePowerHandle, (const char *)_gstBottomHomePowerTextPara.p_content);
#if OTHER_ICON_HIEDE
        // 根据功率正负，切换流向
        if (value_tmp != 0)
        {
            p_st_tmp_ImgPara = &_garrHomeActiveImgPara[1];
        }
        IMAGE_SetDTA(_gImgHomeActiveHandle, p_st_tmp_ImgPara->p_img_buf,
                     p_st_tmp_ImgPara->img_size);
#endif
    }
    return 0;
}
/**
  * @brief  设置界面中太阳能功率文本
  * @param  value_w:功率值
  * @param
  * @retval 0:success
  */
int32_t SetSolarPower(uint8_t ui_update, uint32_t value_w)
{
    float temp_value = 0.0f;
    int32_t value_tmp = (int32_t)value_w;
    value_tmp = abs(value_tmp);
    temp_value = (float)(value_tmp) / 1000;
#if OTHER_ICON_HIEDE   
    emWidgetImgPara_t *p_st_tmp_ImgPara = &_garrSolarActiveImgPara[0];
#endif

#if UNIFIED_UNIT_W
    if (value_tmp < 1000)
    {
        snprintf(_garrBottomSolarPowerTextBuf, sizeof(_garrBottomSolarPowerTextBuf), "%dW", value_tmp);
    }
    else
    {
        snprintf(_garrBottomSolarPowerTextBuf, sizeof(_garrBottomSolarPowerTextBuf), "%.2fkW", temp_value);
    }
#else
    snprintf(_garrBottomSolarPowerTextBuf, sizeof(_garrBottomSolarPowerTextBuf), "%.1fkW", temp_value);
#endif

    if(ui_update)
    {
        TEXT_SetText(_gTextBottomSolarPowerHandle, (const char *)_gstBottomSolarPowerTextPara.p_content);
#if OTHER_ICON_HIEDE        
        // 根据电网功率正负，切换流向
        if (value_tmp != 0)
        {
            p_st_tmp_ImgPara = &_garrSolarActiveImgPara[1];
        }

        IMAGE_SetDTA(_gImgSolarActiveHandle, p_st_tmp_ImgPara->p_img_buf,
                     p_st_tmp_ImgPara->img_size);
#endif
    }
    return 0;
}
/**
  * @brief  设置界面中电网功率文本
  * @param  value_w:功率值
  * @param
  * @retval 0:success
  */
int32_t SetGridPower(uint8_t ui_update, uint32_t value_w, uint16_t gen_enable)
{
    int32_t value_tmp = 0;
    float temp_value = 0.0f;
//    int16_t curr = 0;
//    float curr_tmp = 0.0f;

//    curr = (int16_t)value_curr;
//    curr_tmp = (float)curr / 10;
    value_tmp = (int32_t)value_w;
    value_tmp = abs(value_tmp);
    temp_value = (float)(value_tmp) / 1000;
#if OTHER_ICON_HIEDE
    emWidgetImgPara_t *p_st_tmp_ImgPara = &_garrGridActiveImgPara[0];
#endif

#if UNIFIED_UNIT_W
    if (value_tmp < 1000)
    {
        snprintf(_garrBottomGridPowerTextBuf, sizeof(_garrBottomGridPowerTextBuf), "%dW", value_tmp);
    }
    else
    {
        snprintf(_garrBottomGridPowerTextBuf, sizeof(_garrBottomGridPowerTextBuf), "%.2fkW", temp_value);
    }
#else
    snprintf(_garrBottomGridPowerTextBuf, sizeof(_garrBottomGridPowerTextBuf), "%.1fkW", temp_value);
#endif

    if(ui_update)
    {
        TEXT_SetText(_gTextBottomGridPowerHandle, (const char *)_gstBottomGridPowerTextPara.p_content);
        if(_gstUiCtx.sys_status == 0x01)   //离网状态下 送显为0W 流向不显示
        {
            snprintf(_garrBottomGridPowerTextBuf, sizeof(_garrBottomGridPowerTextBuf), "%dkW", 0);
        }
        if(1 == gen_enable)
        {
            WM_HideWindow(_gTextBottomGridHandle);
            WM_HideWindow(_gTextBottomGridPowerHandle);
        }
#if OTHER_ICON_HIEDE        
        if(_gstUiCtx.sys_status == 0x01)   //离网状态下 送显为0W 流向不显示
        {
            snprintf(_garrBottomGridPowerTextBuf, sizeof(_garrBottomGridPowerTextBuf), "%dW", 0);
            p_st_tmp_ImgPara = &_garrGridActiveImgPara[0];
        }
        else
        {
            if(value_tmp != 0)
            {
                if(curr_tmp < 0)
                {
                    p_st_tmp_ImgPara = &_garrGridActiveImgPara[1];
                }
                else
                {
                    p_st_tmp_ImgPara = &_garrGridActiveImgPara[2];
                }
            }
        }
        IMAGE_SetDTA(_gImgGridActiveHandle, p_st_tmp_ImgPara->p_img_buf,
                     p_st_tmp_ImgPara->img_size);
#endif                     
    }

    return 0;
}
/**
  * @brief  设置界面中油机功率文本
  * @param  value_w:功率值
  * @param
  * @retval 0:success
  */
int32_t SetGeneratorPower(uint8_t ui_update, uint32_t value_w, uint16_t gen_enable)
{
    int32_t value_tmp = 0;
    float temp_value = 0.0f;

    value_tmp = (int32_t)value_w;
    value_tmp = abs(value_tmp);
    temp_value = (float)(value_tmp) / 1000;

#if UNIFIED_UNIT_W
    if (value_tmp < 1000)
    {
        snprintf(_garrBottomGenPowerTextBuf, sizeof(_garrBottomGenPowerTextBuf), "%dW", value_tmp);
    }
    else
    {
        snprintf(_garrBottomGenPowerTextBuf, sizeof(_garrBottomGenPowerTextBuf), "%.2fkW", temp_value);
    }
#else
    snprintf(_garrBottomGenPowerTextBuf, sizeof(_garrBottomGenPowerTextBuf), "%.1fkW", temp_value); 
#endif

    if(ui_update)
    {
        TEXT_SetText(_gTextBottomGenPowerHandle, (const char *)_gstBottomGenPowerTextPara.p_content);
        if(0 == gen_enable)
        {
            WM_HideWindow(_gTextBottomGenHandle);
            WM_HideWindow(_gTextBottomGenPowerHandle);
        }
    }
    return 0;
}

int32_t SetGridTextChange(uint8_t ui_update, uint16_t value)
{
    if(value == 0)  //0电网
    {
        snprintf(_garrBottomGridTextBuf, sizeof(_garrBottomGridTextBuf), "%s", "Grid");
    }
    else   //1 油机
    {
        snprintf(_garrBottomGridTextBuf, sizeof(_garrBottomGridTextBuf), "%s", "Generator");
    }

    if(ui_update)
    {
        if(value == 0)  //0电网
        {
            TEXT_SetText(_gTextBottomGridHandle, (const char *)_gstBottomGridTextPara.p_content);
        }
        else   //1 油机
        {
            TEXT_SetText(_gTextBottomGridHandle, (const char *)_gstBottomGridTextPara.p_content);
        }
    }

    return 0;
}

/**
  * @brief  设置界面中Roca功率文本(电池包总功率)
  * @param  value_w:功率值
  * @param
  * @retval 0:success
  */
int32_t SetBatteryPower(uint8_t ui_update, int16_t value_w)
{
    int16_t value_tmp = 0;
    float temp_value = 0.0f;

    value_tmp = (int16_t)value_w;
    temp_value = (float)(value_tmp) / 1000;

#if UNIFIED_UNIT_W
    if (abs(value_tmp) < 1000)
    {
        snprintf(_garrBottomRocaPowerTextBuf, sizeof(_garrBottomRocaPowerTextBuf), "%dW", value_tmp);
    }
    else
    {
        snprintf(_garrBottomRocaPowerTextBuf, sizeof(_garrBottomRocaPowerTextBuf), "%.2fkW", temp_value);
    }
#else
    snprintf(_garrBottomRocaPowerTextBuf, sizeof(_garrBottomRocaPowerTextBuf), "%.1fkW", temp_value);
#endif

    if(ui_update)
    {
        TEXT_SetText(_gTextBottomRocaPowerHandle, (const char *)_gstBottomRocaPowerTextPara.p_content);
    }
    return 0;
}
/**
  * @brief  设置界面底部是否充电文本
  * @param  value_charge:0——充电（显示文本）  非0——未充电（不显示）
  * @param
  * @retval 0:success
  */
int32_t SetIsCharge(uint8_t ui_update, uint16_t value_charge)
{
    switch(value_charge)
    {
        case 0:
        case 1:
        case 3:
        {
            snprintf(_garrBottomChargeTextBuf, sizeof(_garrBottomChargeTextBuf), "%s", " ");
            break;
        }
        case 2:
        {
            snprintf(_garrBottomChargeTextBuf, sizeof(_garrBottomChargeTextBuf), "%s", "Charging");
            break;
        }
        default:
            break;
    }

    if(ui_update)
    {
        TEXT_SetText(_gTextBottomChargeHandle, (const char *)_gstBottomChargeTextPara.p_content);
    }
    return 0;
}

/**
  * @brief  设置界面中电池图标改变以及电量百分比文本显示
  * @param  value_elec:电量占比
  * @param
  * @retval 0:success
  * @notes  电池电量显示切换
  */
int32_t SetElecChange(uint8_t ui_update, uint16_t value_elec)
{
    float temp_value = 0.0f;
    int16_t value_tmp = 0;

    value_tmp = (int16_t)value_elec;
    temp_value = (float)value_tmp / 10;
    emWidgetImgPara_t  *p_st_tmp_battery_img_para = &_garrBottomBatteryImgPara[0];
    _gReadAllBmsInfoOkFlag = GetAllBmsReadOverStatus();
    if(temp_value > 100.0f)
    {
        return -1;
    }

    /*根据电量选择显示那个电池图标  0-满电  1-更多电 2-一半电 3-低电量*/
    if(temp_value == 100.0f)
    {
        p_st_tmp_battery_img_para = &_garrBottomBatteryImgPara[0];
        _gstBottomBatteryTextPara.text_color = 0x00B2BE00;
    }
    else if(temp_value > 80.0f && temp_value < 100.0f)
    {
        p_st_tmp_battery_img_para = &_garrBottomBatteryImgPara[1];
        _gstBottomBatteryTextPara.text_color = 0x00B2BE00;
    }
    else if(temp_value > 20.0f && temp_value <= 80.0f)
    {
        p_st_tmp_battery_img_para = &_garrBottomBatteryImgPara[2];
        _gstBottomBatteryTextPara.text_color = 0x00B2BE00;
    }
    else if(temp_value >= 0.0f && temp_value <= 20.0f)
    {
        p_st_tmp_battery_img_para = &_garrBottomBatteryImgPara[3];

        /*低电量时，电量百分比文本变成红色；其余状态时，电量百分比文本正常显示*/
        _gstBottomBatteryTextPara.text_color = 0x006A62FF;
    }

    /*低电量时，电量百分比文本变成红色；其余状态时，电量百分比文本正常显示*/
//    if(value_elec > 0 && value_elec < 200)
//    {
//        _gstBottomBatteryTextPara.text_color = 0x006A62FF;
//    }
//    _gstBottomBatteryTextPara.digit_value = value_elec;
//    sprintf(_gstBottomBatteryTextPara.p_content, "%d%%", (uint16_t)_gstBottomBatteryTextPara.digit_value);

    if(_gReadAllBmsInfoOkFlag)
    {
        snprintf(_garrBottomBatteryTextBuf, sizeof(_garrBottomBatteryTextBuf), "%.1f%%", temp_value);
    }
    else
    {
        snprintf(_garrBottomBatteryTextBuf, sizeof(_garrBottomBatteryTextBuf), "%s", "-.-");
        _gstUiCtx.soc = 0;
    }


    if(ui_update)
    {
        TEXT_SetTextColor(_gTextBottomBatteryHandle, _gstBottomBatteryTextPara.text_color);
        TEXT_SetText(_gTextBottomBatteryHandle, (const char *)_gstBottomBatteryTextPara.p_content);

        IMAGE_SetDTA(_gImgBottomBatteryHandle, p_st_tmp_battery_img_para->p_img_buf, p_st_tmp_battery_img_para->img_size);
    }
    return 0;
}

/**
  * @brief  设置界面中EV功率文本
  * @param  value_kw:功率值
  * @param
  * @retval 0:success
  */
int32_t SetEVPower(uint8_t ui_update, uint32_t value_w)
{
    int32_t value_tmp = 0;
    float temp_value = 0.0f;

    value_tmp = (int32_t)value_w;
    temp_value = (float)(abs(value_tmp)) / 1000;
#if UNIFIED_UNIT_W
    if (abs(value_tmp) < 1000)
    {
        snprintf(_garrBottomEVPowerTextBuf, sizeof(_garrBottomEVPowerTextBuf), "%dW", abs(value_tmp));
    }
    else
    {
        snprintf(_garrBottomEVPowerTextBuf, sizeof(_garrBottomEVPowerTextBuf), "%.2fkW", temp_value);
    }
#else
    snprintf(_garrBottomEVPowerTextBuf, sizeof(_garrBottomEVPowerTextBuf), "%.1fkW", temp_value);
#endif

    if(ui_update)
    {
        TEXT_SetText(_gTextBottomEVPowerHandle, (const char *)_gstBottomEVPowerTextPara.p_content);
        //科2先隐藏掉
        WM_HideWindow(_gTextBottomEVPowerHandle);
        WM_HideWindow(_gTextBottomEVHandle);
    }
    return 0;
}



/**
  * @brief  设置界面中温度文本以及其显示的状态条
  * @param  value_temp:温度范围  <0 蓝色（cold） >0 && <60 青色(normal)  else 红色（hot）
  * @param
  * @retval 0:success
  * @notes  none
  */
int32_t SetTempStaBar(uint8_t ui_update, uint16_t value_temp)
{
    int16_t value_tmp = 0;
    float temp_value = 0.0f;
    value_tmp = (int16_t)value_temp;
    temp_value = (float)value_tmp / 10;

    GUI_Clear();
    if(value_tmp < 0)
    {
        //cold
        GUI_SetColor(0x00FDA900);//上色
    }
    else if(value_tmp >= 0 && value_tmp < 600)
    {
        //normal
        GUI_SetColor(0x00B2BE00);//上色
    }
    else
    {
        //hot
        GUI_SetColor(0x00453BFA);//上色
    }
    GUI_FillRoundedRect(260, 276 - 58, 260 + 77, 276 - 58 + 20, 10);

    WM_Paint(_gWinBottomHandle);
    snprintf(_garrBottomTempTextBuf, sizeof(_garrBottomTempTextBuf), "%.1f C", temp_value);
    if(ui_update)
    {
        TEXT_SetText(_gTextBottomTempHandle, (const char *)_gstBottomTempTextPara.p_content);
    }
    return 0;
}
#if 0
/**
  * @brief  设置界面中设备与电网之间的图标流向，馈电、用电、停用
  * @param  stat:用电状态
  * @param
  * @retval 0:success
  * @notes  none
  */
int32_t SetGridPowerstat(uint8_t ui_update, EN_GRID_STAT_E e_stat)
{
    if(e_stat > EN_GRID_STAT_NUM)
    {
        return -1;
    }

    emWidgetImgPara_t *p_st_tmp_gridstat_img_para = &_garrGridActiveImgPara[0];

    switch(e_stat)
    {
        case EN_GRID_DISABLED:
        {
            p_st_tmp_gridstat_img_para = &_garrGridActiveImgPara[0];
            break;
        }
        case EN_GRID_ACTIVE_IN:
        {
            p_st_tmp_gridstat_img_para = &_garrGridActiveImgPara[1];
            break;
        }
        case EN_GRID_ACTIVE_OUT:
        {
            p_st_tmp_gridstat_img_para = &_garrGridActiveImgPara[2];
            break;
        }
        default:
            break;
    }
    if( ui_update)
    {
        IMAGE_SetDTA(_gImgGridActiveHandle, p_st_tmp_gridstat_img_para->p_img_buf, p_st_tmp_gridstat_img_para->img_size);
    }
    return 0;
}
/**
  * @brief  设置界面中设备与家庭之间的图标关系，用电、停电
  * @param  stat:用电状态
  * @param
  * @retval 0:success
  * @notes  none
  */
int32_t SetHomePowerstat(uint8_t ui_update, EN_HOME_STAT_E e_stat)
{
    emWidgetImgPara_t  *p_st_tmp_homestat_img_para = &_garrHomeActiveImgPara[0];
    if(e_stat > EN_HOME_STAT_NUM)
    {
        return -1;
    }

    switch (e_stat)
    {
        case EN_HOME_DISABLED:
        {
            p_st_tmp_homestat_img_para = &_garrHomeActiveImgPara[0];
            break;
        }
        case EN_HOME_ACTIVE:
        {
            p_st_tmp_homestat_img_para = &_garrHomeActiveImgPara[1];
            break;
        }
        default:
            break;
    }
    if(ui_update)
    {
        IMAGE_SetDTA(_gImgHomeActiveHandle, p_st_tmp_homestat_img_para->p_img_buf,  p_st_tmp_homestat_img_para->img_size);
    }
    return 0;
}

/**
  * @brief  设置界面中太阳能功率状态流线图，发电、停电
  * @param  stat:用电状态
  * @param
  * @retval 0:success
  * @notes  none
  */
int32_t SetSolarPowerstat(uint8_t ui_update, EN_SOLAR_STAT_E e_stat)
{
    emWidgetImgPara_t  *p_st_tmp_solarstat_img_para = &_garrSolarActiveImgPara[0];
    if(e_stat > EN_SOLAR_STAT_NUM)
    {
        return -1;
    }

    switch (e_stat)
    {
        case EN_SOLAR_DISABLED:
        {
            p_st_tmp_solarstat_img_para = &_garrSolarActiveImgPara[0];
            break;
        }
        case EN_SOLAR_ACTIVE:
        {
            p_st_tmp_solarstat_img_para = &_garrSolarActiveImgPara[1];
            break;
        }
        case EN_SOLAR_DAYTIME:
        {
            p_st_tmp_solarstat_img_para = &_garrSolarActiveImgPara[2];
            break;
        }
        default:
            break;
    }
    if(ui_update)
    {
        IMAGE_SetDTA(_gImgSolarActiveHandle, p_st_tmp_solarstat_img_para->p_img_buf,  p_st_tmp_solarstat_img_para->img_size);
    }
    return 0;
}
#endif
/**
  * @brief  设置Wifi状态变换
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetHomePageWifiStat(int32_t ui_update, uint16_t stat)
{
    emWidgetImgPara_t  *p_st_tmp_wifi_img_para = &_garrStateBarWifiImgPara[0];
    switch (stat)
    {
        case 0:
        {
            p_st_tmp_wifi_img_para = &_garrStateBarWifiImgPara[0];
            break;
        }
        case 1:
        {
            p_st_tmp_wifi_img_para = &_garrStateBarWifiImgPara[1];
            break;
        }
        case 2:
        {
            p_st_tmp_wifi_img_para = &_garrStateBarWifiImgPara[2];
            break;
        }
        case 3:
        {
            p_st_tmp_wifi_img_para = &_garrStateBarWifiImgPara[3];
            break;
        }
        default:
            break;
    }

    if(ui_update)
    {
        IMAGE_SetDTA(_gImgStateBarWifiHandle, p_st_tmp_wifi_img_para->p_img_buf,
                     p_st_tmp_wifi_img_para->img_size);
    }
    return 0;
}


static void _ShowLayoutStatBar()
{
#if LAYOUT_DEBUG

    GUI_SetColor(GUI_COLOR_WIN_MAIN_STAT_BAR_LAYOUT);//上色
    GUI_FillRoundedRect(_garrStateBarDialogCreate[1].x0,
                        _garrStateBarDialogCreate[1].y0,
                        _garrStateBarDialogCreate[1].x0 + _garrStateBarDialogCreate[1].xSize - 1,
                        _garrStateBarDialogCreate[1].y0 + _garrStateBarDialogCreate[1].ySize - 1,
                        0);


    GUI_FillRoundedRect(_garrStateBarDialogCreate[2].x0,
                        _garrStateBarDialogCreate[2].y0,
                        _garrStateBarDialogCreate[2].x0 + _garrStateBarDialogCreate[2].xSize - 1,
                        _garrStateBarDialogCreate[2].y0 + _garrStateBarDialogCreate[2].ySize - 1,
                        0);


    GUI_FillRoundedRect(_garrStateBarDialogCreate[3].x0,
                        _garrStateBarDialogCreate[3].y0,
                        _garrStateBarDialogCreate[3].x0 + _garrStateBarDialogCreate[3].xSize - 1,
                        _garrStateBarDialogCreate[3].y0 + _garrStateBarDialogCreate[3].ySize - 1,
                        0);


    GUI_FillRoundedRect(_garrStateBarDialogCreate[4].x0,
                        _garrStateBarDialogCreate[4].y0,
                        _garrStateBarDialogCreate[4].x0 + _garrStateBarDialogCreate[4].xSize - 1,
                        _garrStateBarDialogCreate[4].y0 + _garrStateBarDialogCreate[4].ySize - 1,
                        0);


    GUI_FillRoundedRect(_garrStateBarDialogCreate[5].x0,
                        _garrStateBarDialogCreate[5].y0,
                        _garrStateBarDialogCreate[5].x0 + _garrStateBarDialogCreate[5].xSize - 1,
                        _garrStateBarDialogCreate[5].y0 + _garrStateBarDialogCreate[5].ySize - 1,
                        0);
#endif
}

/**
  * @brief  HomePageStatSubWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbStateBarSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    static time_t cur_time = 1704067200;
//    static uint32_t     wifi_type = 0;
    static uint32_t  pcs_stat = 0;
    static uint32_t  workmode = 0;;
    SysWarningModuleDispMap_t *show_err_code;
    uint32_t arr_errcode[4] =
    {
        GetErrId(),         // 错误码
        0,                  // 临时错误类型
        0,                  // 错误源
        0                   // 错误值
    };

    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
#if 0
            WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                           0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                           1000,         /* 周期，此周期过后指定窗口应收到消息*/
                           0);         /* 留待将来使用，应为0 */
#endif


            /*初始化各个子控件*/
            _gImgStateBarWifiHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_WIFI_IMG_ID);
            _SetHomePageWifiStat(1, _gstUiCtx.net_state);
            /*IMAGE_SetDTA(_gImgStateBarWifiHandle, _garrStateBarWifiImgPara[0].p_img_buf,
                         _garrStateBarWifiImgPara[0].img_size); */

            _gTextStateBarTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_TIME_TEXT_ID);
            TEXT_SetFont(_gTextStateBarTimeHandle, _gStateBarTimeTextPara.p_text_font);
            TEXT_SetText(_gTextStateBarTimeHandle, (const char *)_gStateBarTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextStateBarTimeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStateBarTimeHandle, _gStateBarTimeTextPara.text_color);

            _gTextStateBarWorkMode1Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_WORK_MODE1_TEXT_ID);
            TEXT_SetFont(_gTextStateBarWorkMode1Handle, _gStateBarWorkMode1TextPara.p_text_font);
            TEXT_SetText(_gTextStateBarWorkMode1Handle, (const char *)_gStateBarWorkMode1TextPara.p_content);
            TEXT_SetTextAlign(_gTextStateBarWorkMode1Handle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStateBarWorkMode1Handle, _gStateBarWorkMode1TextPara.text_color);

            _gTextStateBarWorkMode2Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_WORK_MODE2_TEXT_ID);
            TEXT_SetFont(_gTextStateBarWorkMode2Handle, _gStateBarWorkMode2TextPara.p_text_font);
            TEXT_SetText(_gTextStateBarWorkMode2Handle, (const char *)_gStateBarWorkMode2TextPara.p_content);
            TEXT_SetTextAlign(_gTextStateBarWorkMode2Handle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStateBarWorkMode2Handle, _gStateBarWorkMode2TextPara.text_color);

            _gTextStateBarWarningHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_WARNING_TEXT_ID);
            TEXT_SetFont(_gTextStateBarWarningHandle, _gStateBarWarningTextPara.p_text_font);
//            TEXT_SetText(_gTextStateBarWarningHandle, (const char *)_gStateBarWarningTextPara.p_content);
            TEXT_SetTextAlign(_gTextStateBarWarningHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
//            TEXT_SetTextColor(_gTextStateBarWarningHandle, _gStateBarWarningTextPara.text_color);

            _gImgStateBarWarningHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_WARNING_IMG_ID);
            SetHomePageWarning(1, (EN_FAULT_STAT_E)_gstUiCtx.warnning_state, 0);
            /*IMAGE_SetDTA(_gImgStateBarWarningHandle, _garrStateBarWarningImgPara.p_img_buf,
                         _garrStateBarWarningImgPara.img_size); */
            break;
        }
        case WM_DELETE:
        {
            break;
        }
        case WM_TIMER:
        {
#if DEBUG_TIMER
            show_err_code = GetErrorID();
            SetHomePageWarning(1, (EN_FAULT_STAT_E)0, 0);
            if(show_err_code->serial_num == 0x00 || show_err_code->serial_num > 0x00)
            {
                SetHomePageWarning(1, (EN_FAULT_STAT_E)show_err_code->type, show_err_code->serial_num);
            }

            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
            break;
#endif
        }
        case WM_PAINT:
        {
//            GUI_SetColor(GUI_COLOR_WIN_HOMEPAGE_STATE_BAR);//上色
//            GUI_FillRoundedRect(148, 8, 148 + 220, 8 + 38, 5);

            GUI_SetPenSize(2);
            GUI_SetColor(GUI_COLOR_WIN_HOMEPAGE_STATE_BAR_PEN);
            GUI_DrawLine(237, 14, 237, 14 + 18);

//            _ShowLayoutStatBar();
#if 0
            SetHomePageSysStatus(1, (EN_SYS_STATUS_E)(workmode % 3));
            SetHomePageWorkMode(1, (EN_WORK_MODE_E)(pcs_stat % 6));


            SetSolarPowerstat(1, EN_SOLAR_DISABLED);
            SetHomePowerstat(1, EN_HOME_DISABLED);
            SetGridPowerstat(1, EN_GRID_ACTIVE_IN);
            SetIsCharge(1, 0);
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
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
#if OTHER_ICON_HIDE
            /*初始化各个子控件*/
            //家用供电线图标
            _gImgHomeActiveHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_STREAM_LINE_ID);
            /*IMAGE_SetDTA(_gImgHomeActiveHandle, _garrHomeActiveImgPara[0].p_img_buf, _garrHomeActiveImgPara[0].img_size);*/
//            SetHomePowerstat(1, (EN_HOME_STAT_E)_gstUiCtx.backup_state);
            //电网供电线图标
            _gImgGridActiveHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_GRID_LINE_ID);
            /*IMAGE_SetDTA(_gImgGridActiveHandle, _garrGridActiveImgPara[1].p_img_buf, _garrGridActiveImgPara[1].img_size);*/
            /*SetGridPowerstat(1, (EN_GRID_STAT_E)_gstUiCtx.grid_state);*/
            //太阳能供电线图标
            _gImgSolarActiveHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_NEW_SOLAR_LINE_ID);
            /*IMAGE_SetDTA(_gImgSolarActiveHandle, _garrSolarActiveImgPara[0].p_img_buf, _garrSolarActiveImgPara[0].img_size);*/
//            SetSolarPowerstat(1, (EN_SOLAR_STAT_E)_gstUiCtx.solar_state);
#if TEMPERATURE_DEL
            //温度图标
            _gImgBottomTempHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_TEMP_IMG_ID);
            IMAGE_SetDTA(_gImgBottomTempHandle, _gstBottomTempImgPara.p_img_buf, _gstBottomTempImgPara.img_size);
#endif
            //太阳能板图标
            _gImgSolarPlateHandle1 = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_SOLAR_PLATE1_ID);
//            IMAGE_SetDTA(_gImgSolarPlateHandle1, _gstNewSolarPlateImgPara.p_img_buf, _gstNewSolarPlateImgPara.img_size);

            _gImgSolarPlateHandle2 = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_SOLAR_PLATE2_ID);
//            IMAGE_SetDTA(_gImgSolarPlateHandle2, _gstNewSolarPlateImgPara.p_img_buf, _gstNewSolarPlateImgPara.img_size);
#endif
            //家用文字标识
            _gTextBottomHomeHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_HOME_ID);
            TEXT_SetFont(_gTextBottomHomeHandle, _gstBottomHomeTextPara.p_text_font);
            TEXT_SetText(_gTextBottomHomeHandle, (const char *)_gstBottomHomeTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomHomeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomHomeHandle, _gstBottomHomeTextPara.text_color);
            //家用功率
            _gTextBottomHomePowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_HOME_POWER_ID);
            TEXT_SetFont(_gTextBottomHomePowerHandle, _gstBottomHomePowerTextPara.p_text_font);
            TEXT_SetText(_gTextBottomHomePowerHandle, (const char *)_gstBottomHomePowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomHomePowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomHomePowerHandle, _gstBottomHomePowerTextPara.text_color);
            //太阳能文字标识
            _gTextBottomSolarHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_SOLAR_ID);
            TEXT_SetFont(_gTextBottomSolarHandle, _gstBottomSolarTextPara.p_text_font);
            TEXT_SetText(_gTextBottomSolarHandle, (const char *)_gstBottomSolarTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomSolarHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomSolarHandle, _gstBottomSolarTextPara.text_color);
            //太阳能功率
            _gTextBottomSolarPowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_SOLAR_POWER_ID);
            TEXT_SetFont(_gTextBottomSolarPowerHandle, _gstBottomSolarPowerTextPara.p_text_font);
            TEXT_SetText(_gTextBottomSolarPowerHandle, (const char *)_gstBottomSolarPowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomSolarPowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomSolarPowerHandle, _gstBottomSolarPowerTextPara.text_color);
            //电网文字标识
            _gTextBottomGridHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_GRID_ID);
            TEXT_SetFont(_gTextBottomGridHandle, _gstBottomGridTextPara.p_text_font);
            TEXT_SetText(_gTextBottomGridHandle, (const char *)_gstBottomGridTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomGridHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomGridHandle, _gstBottomGridTextPara.text_color);
            //电网功率
            _gTextBottomGridPowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_GRID_POWER_ID);
            TEXT_SetFont(_gTextBottomGridPowerHandle, _gstBottomGridPowerTextPara.p_text_font);
            TEXT_SetText(_gTextBottomGridPowerHandle, (const char *)_gstBottomGridPowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomGridPowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomGridPowerHandle, _gstBottomGridPowerTextPara.text_color);
            //EV文字标识
            _gTextBottomEVHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_EV_ID);
            TEXT_SetFont(_gTextBottomEVHandle, _gstBottomEVTextPara.p_text_font);
            TEXT_SetText(_gTextBottomEVHandle, (const char *)_gstBottomEVTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomEVHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomEVHandle, _gstBottomEVTextPara.text_color);
            //EV功率
            _gTextBottomEVPowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_EV_POWER_ID);
            TEXT_SetFont(_gTextBottomEVPowerHandle, _gstBottomEVPowerTextPara.p_text_font);
            TEXT_SetText(_gTextBottomEVPowerHandle, (const char *)_gstBottomEVPowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomEVPowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomEVPowerHandle, _gstBottomEVPowerTextPara.text_color);
            //Gen文字标识
            _gTextBottomGenHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_GEN_ID);
            TEXT_SetFont(_gTextBottomGenHandle, _gstBottomEVTextPara.p_text_font);
            TEXT_SetText(_gTextBottomGenHandle, (const char *)_gstBottomGenTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomGenHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomGenHandle, _gstBottomGenTextPara.text_color);
            //GEN功率
            _gTextBottomGenPowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_GEN_POWER_ID);
            TEXT_SetFont(_gTextBottomGenPowerHandle, _gstBottomGenPowerTextPara.p_text_font);
            TEXT_SetText(_gTextBottomGenPowerHandle, (const char *)_gstBottomGenPowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomGenPowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomGenPowerHandle, _gstBottomGenPowerTextPara.text_color);
#if TEMPERATURE_DEL  //屏蔽主界面温度显示
            //温度TEXT
            _gTextBottomTempHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_TEMP_TEXT_ID);
            TEXT_SetFont(_gTextBottomTempHandle, _gstBottomTempTextPara.p_text_font);
            TEXT_SetText(_gTextBottomTempHandle, (const char *)_gstBottomTempTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomTempHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomTempHandle, _gstBottomTempTextPara.text_color);
#endif

            //电池电量TEXT
            _gTextBottomBatteryHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_BATTERY_ID);
            TEXT_SetFont(_gTextBottomBatteryHandle, _gstBottomBatteryTextPara.p_text_font);
            TEXT_SetText(_gTextBottomBatteryHandle, (const char *)_gstBottomBatteryTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomBatteryHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomBatteryHandle, _gstBottomBatteryTextPara.text_color);
            //电池电量图标
            _gImgBottomBatteryHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_BATTERY_IMG_ID);
            /*IMAGE_SetDTA(_gImgBottomBatteryHandle, _garrBottomBatteryImgPara[2].p_img_buf, _garrBottomBatteryImgPara[2].img_size);*/
            SetElecChange(1, _gstUiCtx.soc);


            //Roca text
            _gTextBottomRocaHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_ROCA_ID);
            TEXT_SetFont(_gTextBottomRocaHandle, _gstBottomRocaTextPara.p_text_font);
            TEXT_SetText(_gTextBottomRocaHandle, (const char *)_gstBottomRocaTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomRocaHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomRocaHandle, _gstBottomRocaTextPara.text_color);
            //Roca功率
            _gTextBottomRocaPowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_ROCA_POWER_ID);
            TEXT_SetFont(_gTextBottomRocaPowerHandle, _gstBottomRocaPowerTextPara.p_text_font);
            TEXT_SetText(_gTextBottomRocaPowerHandle, (const char *)_gstBottomRocaPowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomRocaPowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomRocaPowerHandle, _gstBottomRocaPowerTextPara.text_color);
            //charge text
            _gTextBottomChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_CHARGE_ID);
            TEXT_SetFont(_gTextBottomChargeHandle, _gstBottomChargeTextPara.p_text_font);
            TEXT_SetText(_gTextBottomChargeHandle, (const char *)_gstBottomChargeTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomChargeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomChargeHandle, _gstBottomChargeTextPara.text_color);


            SetHomePower(1, _gstUiCtx.home_power);
            SetGridPower(1, _gstUiCtx.grid_power, _gstUiCtx.generator_enable);
            SetBatteryPower(1, _gstUiCtx.roca_power);
            SetSolarPower(1, _gstUiCtx.solar_power);
            SetEVPower(1, _gstUiCtx.ev_power);
            SetGeneratorPower(1, _gstUiCtx.generator_power, _gstUiCtx.generator_enable);
            break;
        }
        case WM_PAINT:
        {
            _SetPvStatusLine(_gstUiCtx.solar_power);
            _SetHomePowerLine(_gstUiCtx.home_power);
            _SetEVPowerLine(_gstUiCtx.ev_power, _gstUiCtx.ev_curr);
            _SetGridPowerLine(_gstUiCtx.grid_power, _gstUiCtx.grid_curr);
            _SetGeneratorPowerLine(_gstUiCtx.generator_power, _gstUiCtx.generator_enable);

#if TEMPERATURE_DEL
            SetTempStaBar(1, _gstUiCtx.temperature);
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
static void _cbHomePageWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
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
#if BACK_ICON
            GUI_DrawStreamedBitmapAuto(_pHomePageBKBuf, 0, 0);
#else
            GUI_DrawStreamedBitmapAuto(_garrHomePageBKBuf, 0, 0);
#endif
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
static GUI_HWIN _CreateStateBarSubWin()
{
    _gWinStateBarHandle = GUI_CreateDialogBox(_garrStateBarDialogCreate,
                          GUI_COUNTOF(_garrStateBarDialogCreate),
                          _cbStateBarSubWin,
                          pCurrentEmWin->win_handle,
                          0, 0);
    //EM_YSIZE - _garrStateBarDialogCreate[0].ySize);
    WM_SetTransState(_gWinStateBarHandle, WM_CF_HASTRANS);

#if DEBUG_TIMER
    WM_CreateTimer(_gWinStateBarHandle, /* 接受信息的窗口的句柄 */
                   0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                   1000,         /* 周期，此周期过后指定窗口应收到消息*/
                   0);         /* 留待将来使用，应为0 */
#endif

//    GUI_Exec();
    return _gWinStateBarHandle;
}

/**
  * @brief Create bottom sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateBottomSubWin()
{
    _gWinBottomHandle = GUI_CreateDialogBox(_garrBottomTextInfoDialogCreate,
                                            GUI_COUNTOF(_garrBottomTextInfoDialogCreate),
                                            _cbBottomSubWin,
                                            pCurrentEmWin->win_handle,
                                            0, 0);
    //EM_YSIZE - _garrStateBarDialogCreate[0].ySize);
    WM_SetTransState(_gWinBottomHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinBottomHandle;
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
    gstEmWinHomePage.win_handle = WM_CreateWindow(0, 0, EM_XSIZE, EM_YSIZE, WM_CF_SHOW, _cbHomePageWin, 0);
    pCurrentEmWin = &gstEmWinHomePage;
//    GUI_Exec();
    _CreateStateBarSubWin();
    _CreateBottomSubWin();
    return gstEmWinHomePage.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinHomePage.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinHomePage.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinHomePage.win_handle = 0;
}



/**
  * @brief  顶部状态栏初始化
  * @param  NULL
  * @retval NULL
  */
static void _InitHomepageStatBar()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为StatBar区域子控件申请内存*/
    _gstUiCtx.net_state = gstCollectInfo.maincard_data.sys_info.net_conn_state;
//    _gstUiCtx.sys_status = gstCollectParaData.pcs_data.udp_pcs_input_para.system_status;
    _gstUiCtx.sys_status = gstCollectInfo.power_data.model_info.on_grid_state;
    _gstUiCtx.rtc_time = gstCollectInfo.maincard_data.sys_info.time_stamp;
    _gstUiCtx.work_mode = gstCollectInfo.power_data.power_set_info.work_mode;

    //wifi状态图标
    _garrStateBarWifiImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE);
    _garrStateBarWifiImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrStateBarWifiImgPara[0].mem_img);
    _garrStateBarWifiImgPara[0].img_size = EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_NOT_FOUND, _garrStateBarWifiImgPara[0].p_img_buf,
                     _garrStateBarWifiImgPara[0].img_size);

    _garrStateBarWifiImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE);
    _garrStateBarWifiImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrStateBarWifiImgPara[1].mem_img);
    _garrStateBarWifiImgPara[1].img_size = EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_OFF_ADDR, _garrStateBarWifiImgPara[1].p_img_buf,
                     _garrStateBarWifiImgPara[1].img_size);

    _garrStateBarWifiImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE);
    _garrStateBarWifiImgPara[2].p_img_buf = GUI_ALLOC_h2p(_garrStateBarWifiImgPara[2].mem_img);
    _garrStateBarWifiImgPara[2].img_size = EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_DISCONN_CLOUD_ADDR, _garrStateBarWifiImgPara[2].p_img_buf,
                     _garrStateBarWifiImgPara[2].img_size);

    _garrStateBarWifiImgPara[3].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE);
    _garrStateBarWifiImgPara[3].p_img_buf = GUI_ALLOC_h2p(_garrStateBarWifiImgPara[3].mem_img);
    _garrStateBarWifiImgPara[3].img_size = EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_ON_ADDR, _garrStateBarWifiImgPara[3].p_img_buf,
                     _garrStateBarWifiImgPara[3].img_size);


    _garrStateBarWarningImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WARNING_ICON_SIZE);
    _garrStateBarWarningImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrStateBarWarningImgPara[0].mem_img);
    _garrStateBarWarningImgPara[0].img_size = EM_HOME_PAGE_STATE_BAR_WARNING_ICON_SIZE;
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _garrStateBarWarningImgPara[0].p_img_buf,
                     _garrStateBarWarningImgPara[0].img_size);

    _garrStateBarWarningImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WARNING_ICON_SIZE);
    _garrStateBarWarningImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrStateBarWarningImgPara[1].mem_img);
    _garrStateBarWarningImgPara[1].img_size = EM_HOME_PAGE_STATE_BAR_WARNING_ICON_SIZE;
    SpiFlashReadData(EM_ICON_ERROR_ADDR, _garrStateBarWarningImgPara[1].p_img_buf,
                     _garrStateBarWarningImgPara[1].img_size);

    _gStateBarTimeTextPara.p_content = _garrStateBarTimeTextBuf;
    _gStateBarTimeTextPara.text_color = 0X00D9D9D9;
    _gStateBarTimeTextPara.p_text_font = EM_TEXT_FONT_7;

    _gStateBarWorkMode1TextPara.p_content = _garrStateBarWorkMode1TextBuf;
    _gStateBarWorkMode1TextPara.text_color = 0X00D9D9D9;
    _gStateBarWorkMode1TextPara.p_text_font = EM_TEXT_FONT_5;

    _gStateBarWorkMode2TextPara.p_content = _garrStateBarWorkMode2TextBuf;
    _gStateBarWorkMode2TextPara.text_color = 0X00D9D9D9;
    _gStateBarWorkMode2TextPara.p_text_font = EM_TEXT_FONT_5;

    _gStateBarWarningTextPara.p_content = _garrStateBarWarningTextBuf;
    _gStateBarWarningTextPara.text_color = 0X00000000;
    _gStateBarWarningTextPara.p_text_font = EM_TEXT_FONT_7;

    /*初始化时加载的时间*/
    SetHomePageTime(0, _gstUiCtx.rtc_time);
    /*初始化时加载的系统工作模式*/
    SetHomePageSysStatus(0, (EN_SYS_STATUS_E)_gstUiCtx.sys_status);
    /*初始化时加载的WiFi图标*/
    _SetHomePageWifiStat(0, _gstUiCtx.net_state);
    /*初始化时加载的warnning图标*/
    SetHomePageWarning(0, (EN_FAULT_STAT_E)_gstUiCtx.warnning_state, 0);
    /*初始化时加载的pcs工作模式图标*/
    SetHomePageWorkMode(0, (EN_WORK_MODE_E)_gstUiCtx.work_mode);
}

/**
  * @brief  底部子窗口初始化      
  * @param  NULL                
  * @retval NULL                
  */
static void _InitHomepageBottom()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    _gstUiCtx.home_power  = gstCollectInfo.power_data.power_value_info.r_backup_act_power +
                            gstCollectInfo.power_data.power_value_info.s_backup_act_power +
                            gstCollectInfo.power_data.power_value_info.t_backup_act_power + 
                            gstCollectInfo.power_data.power_value_info.r_nload_act_power +
                            gstCollectInfo.power_data.power_value_info.s_nload_act_power +
                            gstCollectInfo.power_data.power_value_info.t_nload_act_power;
    _gstUiCtx.grid_power  = gstCollectInfo.power_data.power_value_info.r_meter_act_power +
                            gstCollectInfo.power_data.power_value_info.s_meter_act_power +
                            gstCollectInfo.power_data.power_value_info.t_meter_act_power;

    uint16_t power_temp = 0;
    for(uint16_t i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        power_temp   += gstPackSlavePara.pack_para[i].dcdc_info.power_info.output_power;
    }
    _gstUiCtx.roca_power = power_temp;
    _gstUiCtx.solar_power = gstCollectInfo.power_data.power_value_info.pv_1_power + gstCollectInfo.power_data.power_value_info.pv_2_power +
                            gstCollectInfo.power_data.power_value_info.pv_3_power + gstCollectInfo.power_data.power_value_info.pv_4_power;
    _gstUiCtx.solar_power += (uint32_t)gstCollectInfo.power_data.power_value_info.r_third_party_act_power;
    
    _gstUiCtx.soc         = gstCollectInfo.maincard_data.total_pack_info.soc;
    _gstUiCtx.temperature = gstCollectInfo.power_data.model_info.inv_box_temp;
    _gstUiCtx.is_charge   = gstCollectInfo.power_data.model_info.dcdc_run_state;
    _gstUiCtx.grid_curr   = gstCollectInfo.power_data.power_value_info.r_meter_current;
    _gstUiCtx.generator_enable = gstCollectInfo.power_data.power_set_info.gen_mode;
    _gstUiCtx.generator_power  = gstCollectInfo.power_data.power_value_info.r_gen_act_power + 
                                 gstCollectInfo.power_data.power_value_info.s_gen_act_power + 
                                 gstCollectInfo.power_data.power_value_info.t_gen_act_power;
    _gstUiCtx.ev_power = gstCollectInfo.power_data.power_value_info.inv_ev_power;
    _gstUiCtx.ev_curr  = gstCollectInfo.power_data.power_value_info.inv_ev_current;

#if OTHER_ICON_HIDE    
    //负载能量流向
    _garrHomeActiveImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_HOME_ACTIVE_ICON_SIZE);
    _garrHomeActiveImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrHomeActiveImgPara[0].mem_img);
    _garrHomeActiveImgPara[0].img_size = EM_HOME_PAGE_BOTTOM_HOME_ACTIVE_ICON_SIZE;
//    SpiFlashReadData(EM_ICON_HOME_DISABLED_PAGE_ADDR, _garrHomeActiveImgPara[0].p_img_buf,
//                     _garrHomeActiveImgPara[0].img_size);

    _garrHomeActiveImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_HOME_ACTIVE_ICON_SIZE);
    _garrHomeActiveImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrHomeActiveImgPara[1].mem_img);
    _garrHomeActiveImgPara[1].img_size = EM_HOME_PAGE_BOTTOM_HOME_ACTIVE_ICON_SIZE;
//    SpiFlashReadData(EM_ICON_HOME_ACTIVE_PAGE_ADDR, _garrHomeActiveImgPara[1].p_img_buf,
//                     _garrHomeActiveImgPara[1].img_size);
    //电网能量流向
    _garrGridActiveImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE);
    _garrGridActiveImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrGridActiveImgPara[0].mem_img);
    _garrGridActiveImgPara[0].img_size = EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE;
//    SpiFlashReadData(EM_ICON_GRID_DISABLED_PAGE_ADDR, _garrGridActiveImgPara[0].p_img_buf,
//                     _garrGridActiveImgPara[0].img_size);

    _garrGridActiveImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE);
    _garrGridActiveImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrGridActiveImgPara[1].mem_img);
    _garrGridActiveImgPara[1].img_size = EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE;
//    SpiFlashReadData(EM_ICON_GRID_ACTIVE_IN_PAGE_ADDR, _garrGridActiveImgPara[1].p_img_buf,
//                     _garrGridActiveImgPara[1].img_size);

    _garrGridActiveImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE);
    _garrGridActiveImgPara[2].p_img_buf = GUI_ALLOC_h2p(_garrGridActiveImgPara[2].mem_img);
    _garrGridActiveImgPara[2].img_size = EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE;
//    SpiFlashReadData(EM_ICON_GRID_ACTIVE_OUT_PAGE_ADDR, _garrGridActiveImgPara[2].p_img_buf,
//                     _garrGridActiveImgPara[2].img_size);
    //PV能量流向
    _garrSolarActiveImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE);
    _garrSolarActiveImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrSolarActiveImgPara[0].mem_img);
    _garrSolarActiveImgPara[0].img_size = EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE;
//    SpiFlashReadData(EM_ICON_SOLAR_DISABLED_PAGE_ADDR, _garrSolarActiveImgPara[0].p_img_buf,
//                     _garrSolarActiveImgPara[0].img_size);

    _garrSolarActiveImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE);
    _garrSolarActiveImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrSolarActiveImgPara[1].mem_img);
    _garrSolarActiveImgPara[1].img_size = EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE;
//    SpiFlashReadData(EM_ICON_NEW_SOLAR_ACTIVE_PAGE_ADDR, _garrSolarActiveImgPara[1].p_img_buf,
//                     _garrSolarActiveImgPara[1].img_size);

    _garrSolarActiveImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE);
    _garrSolarActiveImgPara[2].p_img_buf = GUI_ALLOC_h2p(_garrSolarActiveImgPara[2].mem_img);
    _garrSolarActiveImgPara[2].img_size = EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE;
//    SpiFlashReadData(EM_ICON_SOLAR_LINE_DAYTIME_PAGE_ADDR, _garrSolarActiveImgPara[2].p_img_buf,
//                     _garrSolarActiveImgPara[2].img_size);
#if TEMPERATURE_DEL
    //温度图标
    _gstBottomTempImgPara.mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_TEMP_ICON_SIZE);
    _gstBottomTempImgPara.p_img_buf = GUI_ALLOC_h2p(_gstBottomTempImgPara.mem_img);
    _gstBottomTempImgPara.img_size = EM_HOME_PAGE_BOTTOM_TEMP_ICON_SIZE;
    SpiFlashReadData(EM_ICON_FLUENT_TEMP_ADDR, _gstBottomTempImgPara.p_img_buf,
                     _gstBottomTempImgPara.img_size);
#endif
    //光伏板图标
    _gstNewSolarPlateImgPara.mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_SOLAR_PLATE_ICON_SIZE);
    _gstNewSolarPlateImgPara.p_img_buf = GUI_ALLOC_h2p(_gstNewSolarPlateImgPara.mem_img);
    _gstNewSolarPlateImgPara.img_size = EM_HOME_PAGE_BOTTOM_SOLAR_PLATE_ICON_SIZE;
//    SpiFlashReadData(EM_ICON_SOLAR_PLATE_PAGE_ADDR, _gstNewSolarPlateImgPara.p_img_buf, _gstNewSolarPlateImgPara.img_size);
#endif
    //电池图标
    _garrBottomBatteryImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE);
    _garrBottomBatteryImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrBottomBatteryImgPara[0].mem_img);
    _garrBottomBatteryImgPara[0].img_size = EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE;
    SpiFlashReadData(EM_ICON_BATTERY1_ADDR, _garrBottomBatteryImgPara[0].p_img_buf,
                     _garrBottomBatteryImgPara[0].img_size);

    _garrBottomBatteryImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE);
    _garrBottomBatteryImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrBottomBatteryImgPara[1].mem_img);
    _garrBottomBatteryImgPara[1].img_size = EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE;
    SpiFlashReadData(EM_ICON_BATTERY2_ADDR, _garrBottomBatteryImgPara[1].p_img_buf,  _garrBottomBatteryImgPara[1].img_size);

    _garrBottomBatteryImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE);
    _garrBottomBatteryImgPara[2].p_img_buf = GUI_ALLOC_h2p(_garrBottomBatteryImgPara[2].mem_img);
    _garrBottomBatteryImgPara[2].img_size = EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE;
    SpiFlashReadData(EM_ICON_BATTERY3_ADDR, _garrBottomBatteryImgPara[2].p_img_buf,
                     _garrBottomBatteryImgPara[2].img_size);

    _garrBottomBatteryImgPara[3].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE);
    _garrBottomBatteryImgPara[3].p_img_buf = GUI_ALLOC_h2p(_garrBottomBatteryImgPara[3].mem_img);
    _garrBottomBatteryImgPara[3].img_size = EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE;
    SpiFlashReadData(EM_ICON_BATTERY4_ADDR, _garrBottomBatteryImgPara[3].p_img_buf,
                     _garrBottomBatteryImgPara[3].img_size);

    //底部HOME字样
    _gstBottomHomeTextPara.p_content = _garrBottomHomeTextBuf;
    _gstBottomHomeTextPara.text_color = 0x00C1C1C1;
    _gstBottomHomeTextPara.p_text_font = EM_TEXT_FONT_5;
    snprintf(_garrBottomHomeTextBuf, sizeof(_garrBottomHomeTextBuf), "%s", "Home");
    //负载功率
    _gstBottomHomePowerTextPara.p_content =   _garrBottomHomePowerTextBuf;
    _gstBottomHomePowerTextPara.text_color = 0X00FFFFFF;
    _gstBottomHomePowerTextPara.p_text_font = EM_TEXT_FONT_5;
//    _gstBottomHomePowerTextPara.digit_value = 2.0;//测试数据，实际赋值系统数据
//    sprintf(_gstBottomHomePowerTextPara.p_content, "%.1fkW", _gstBottomHomePowerTextPara.digit_value);

    //底部Solar字样
    _gstBottomSolarTextPara.p_content = _garrBottomSolarTextBuf;
    _gstBottomSolarTextPara.text_color = 0x00C1C1C1;
    _gstBottomSolarTextPara.p_text_font = EM_TEXT_FONT_5;
    snprintf(_garrBottomSolarTextBuf, sizeof(_garrBottomSolarTextBuf), "%s", "Solar");
    //PV总和的有功功率
    _gstBottomSolarPowerTextPara.p_content =  _garrBottomSolarPowerTextBuf;
    _gstBottomSolarPowerTextPara.text_color = 0X00FFFFFF;
    _gstBottomSolarPowerTextPara.p_text_font = EM_TEXT_FONT_5;
//    _gstBottomSolarPowerTextPara.digit_value = 4.0;//测试数据，实际赋值系统数据
//    sprintf(_gstBottomSolarPowerTextPara.p_content, "%.1fkW", _gstBottomSolarPowerTextPara.digit_value);

    //底部Grid字样
    _gstBottomGridTextPara.p_content = _garrBottomGridTextBuf;
    _gstBottomGridTextPara.text_color = 0x00C1C1C1;
    _gstBottomGridTextPara.p_text_font = EM_TEXT_FONT_5;
    snprintf(_garrBottomGridTextBuf, sizeof(_garrBottomGridTextBuf), "%s", "Grid");

    //电网的有功功率
    _gstBottomGridPowerTextPara.p_content = _garrBottomGridPowerTextBuf;
    _gstBottomGridPowerTextPara.text_color = 0X00FFFFFF;
    _gstBottomGridPowerTextPara.p_text_font = EM_TEXT_FONT_5;
//    _gstBottomGridPowerTextPara.digit_value = 2.3;//测试数据，实际赋值系统数据
//    sprintf(_gstBottomGridPowerTextPara.p_content, "%.1fkW", _gstBottomGridPowerTextPara.digit_value);

    //底部Gen字样
    _gstBottomGenTextPara.p_content = _garrBottomGenTextBuf;
    _gstBottomGenTextPara.text_color = 0x00C1C1C1;
    _gstBottomGenTextPara.p_text_font = EM_TEXT_FONT_5;
    snprintf(_garrBottomGenTextBuf, sizeof(_garrBottomGenTextBuf), "%s", "Gen");

    //gen的有功功率
    _gstBottomGenPowerTextPara.p_content = _garrBottomGenPowerTextBuf;
    _gstBottomGenPowerTextPara.text_color = 0X00FFFFFF;
    _gstBottomGenPowerTextPara.p_text_font = EM_TEXT_FONT_5;
//    _gstBottomGenPowerTextPara.digit_value = 64362.3;//测试数据，实际赋值系统数据
//    sprintf(_gstBottomGenPowerTextPara.p_content, "%.1fkW", _gstBottomGenPowerTextPara.digit_value);

    //底部EV字样
    _gstBottomEVTextPara.p_content = _garrBottomEVTextBuf;
    _gstBottomEVTextPara.text_color = 0x00C1C1C1;
    _gstBottomEVTextPara.p_text_font = EM_TEXT_FONT_5;
    snprintf(_garrBottomEVTextBuf, sizeof(_garrBottomEVTextBuf), "%s", "EV");

    //EV的有功功率
    _gstBottomEVPowerTextPara.p_content = _garrBottomEVPowerTextBuf;
    _gstBottomEVPowerTextPara.text_color = 0X00FFFFFF;
    _gstBottomEVPowerTextPara.p_text_font = EM_TEXT_FONT_5;
//    _gstBottomEVPowerTextPara.digit_value = 3324.3;//测试数据，实际赋值系统数据
//    sprintf(_gstBottomEVPowerTextPara.p_content, "%.1fkW", _gstBottomEVPowerTextPara.digit_value);


#if 0
    //温度值
    _gstBottomTempTextPara.p_content = _garrBottomTempTextBuf;
    _gstBottomTempTextPara.text_color = 0x00FFFFFF;
    _gstBottomTempTextPara.p_text_font = EM_TEXT_FONT_7;
//    _gstBottomTempTextPara.digit_value = 112.5;
//    sprintf(_gstBottomTempTextPara.p_content, "%.1fF", _gstBottomTempTextPara.digit_value);
#endif
    //电池soc值
    _gstBottomBatteryTextPara.p_content = _garrBottomBatteryTextBuf;
    _gstBottomBatteryTextPara.text_color = 0x00B2BE00;
    _gstBottomBatteryTextPara.p_text_font = EM_TEXT_FONT_5;
//    _gstBottomBatteryTextPara.digit_value = 60;
//    sprintf(_gstBottomBatteryTextPara.p_content, "%d%%", (uint16_t)_gstBottomBatteryTextPara.digit_value);
    //底部Roca字样
    _gstBottomRocaTextPara.p_content = _garrBottomRocaTextBuf;
    _gstBottomRocaTextPara.text_color = 0x00C1C1C1;
    _gstBottomRocaTextPara.p_text_font = EM_TEXT_FONT_7;
    snprintf(_garrBottomRocaTextBuf, sizeof(_garrBottomRocaTextBuf), "%s", "Roca");
    //电池包 总有功功率
    _gstBottomRocaPowerTextPara.p_content = _garrBottomRocaPowerTextBuf;
    _gstBottomRocaPowerTextPara.text_color = 0x00FFFFFF;
    _gstBottomRocaPowerTextPara.p_text_font = EM_TEXT_FONT_5;
//    _gstBottomRocaPowerTextPara.digit_value = 4.0;
//    sprintf(_gstBottomRocaPowerTextPara.p_content, "%.1fkW", _gstBottomRocaPowerTextPara.digit_value);
    //是否充电的字样
    _gstBottomChargeTextPara.p_content = _garrBottomChargeTextBuf;
    _gstBottomChargeTextPara.text_color = 0x00B2BE00;
    _gstBottomChargeTextPara.p_text_font = EM_TEXT_FONT_7;
    snprintf(_garrBottomChargeTextBuf, sizeof(_garrBottomChargeTextBuf), "%s", "Charging");

    SetHomePower(0, _gstUiCtx.home_power);
    SetSolarPower(0, _gstUiCtx.solar_power);
    SetGridPower(0, _gstUiCtx.grid_power, _gstUiCtx.generator_enable);
    SetGeneratorPower(0, _gstUiCtx.generator_power, _gstUiCtx.generator_enable);
    SetBatteryPower(0, _gstUiCtx.roca_power);
    SetIsCharge(0, _gstUiCtx.is_charge);
    SetElecChange(0, _gstUiCtx.soc);
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    // reset user_type =normal
    ResetPrivileged(); // add by guch

    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为背景图申请内存*/
#if BACK_ICON
    _gHomeMemBK = GUI_ALLOC_AllocZero(EM_MAIN_ICON_BACKGROUND_SIZE);
    _pHomePageBKBuf = GUI_ALLOC_h2p(_gHomeMemBK);
    SpiFlashReadData(EM_ICON_HOME_PAGE_ADDR, _pHomePageBKBuf, EM_MAIN_ICON_BACKGROUND_SIZE);
#else
    if(_gstUiCtx.bg_flag == 0)
    {
        SpiFlashReadData(EM_ICON_HOME_PAGE_ADDR, _garrHomePageBKBuf, EM_MAIN_ICON_BACKGROUND_SIZE);
        _gstUiCtx.bg_flag = 1;
    }
#endif

    _InitHomepageStatBar();
    _InitHomepageBottom();
}


static void _DelHomePageStatBar()
{
    GUI_ALLOC_Free(_garrStateBarWarningImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrStateBarWarningImgPara[1].mem_img);
    
    GUI_ALLOC_Free(_garrStateBarWifiImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrStateBarWifiImgPara[1].mem_img);
    GUI_ALLOC_Free(_garrStateBarWifiImgPara[2].mem_img);
    GUI_ALLOC_Free(_garrStateBarWifiImgPara[3].mem_img);
}

static void _DelHomePageBottom()
{
#if OTHER_ICON_HIDE
    GUI_ALLOC_Free(_garrHomeActiveImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrHomeActiveImgPara[1].mem_img);

    GUI_ALLOC_Free(_garrGridActiveImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrGridActiveImgPara[1].mem_img);
    GUI_ALLOC_Free(_garrGridActiveImgPara[2].mem_img);

    GUI_ALLOC_Free(_garrSolarActiveImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrSolarActiveImgPara[1].mem_img);
    GUI_ALLOC_Free(_garrSolarActiveImgPara[2].mem_img);
    GUI_ALLOC_Free(_gstNewSolarPlateImgPara.mem_img);
#endif
    
#if TEMPERATURE_DEL
    GUI_ALLOC_Free(_gstBottomTempImgPara.mem_img);
#endif
    GUI_ALLOC_Free(_garrBottomBatteryImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrBottomBatteryImgPara[1].mem_img);
    GUI_ALLOC_Free(_garrBottomBatteryImgPara[2].mem_img);
    GUI_ALLOC_Free(_garrBottomBatteryImgPara[3].mem_img);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    _DelHomePageStatBar();
    _DelHomePageBottom();
#if BACK_ICON
    GUI_ALLOC_Free(_gHomeMemBK);
#endif
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    _gReadAllBmsInfoOkFlag = GetAllBmsReadOverStatus();
    uint32_t rtctime = gstCollectInfo.maincard_data.sys_info.time_stamp;
    uint32_t bat_power = 0;
    uint32_t solar_power = gstCollectInfo.power_data.power_value_info.pv_1_power + gstCollectInfo.power_data.power_value_info.pv_2_power +
                           gstCollectInfo.power_data.power_value_info.pv_3_power + gstCollectInfo.power_data.power_value_info.pv_4_power;
    solar_power += (uint32_t)gstCollectInfo.power_data.power_value_info.r_third_party_act_power;
    
    for(uint16_t i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        bat_power += gstPackSlavePara.pack_para[i].dcdc_info.power_info.output_power;
    }
    
    uint32_t arr_power[3] =
    {
        //负载有功功率
        gstCollectInfo.power_data.power_value_info.r_backup_act_power +
        gstCollectInfo.power_data.power_value_info.s_backup_act_power +
        gstCollectInfo.power_data.power_value_info.t_backup_act_power +
        gstCollectInfo.power_data.power_value_info.r_nload_act_power +
        gstCollectInfo.power_data.power_value_info.s_nload_act_power +
        gstCollectInfo.power_data.power_value_info.t_nload_act_power,
        //电网有功功率
        gstCollectInfo.power_data.power_value_info.r_meter_act_power +
        gstCollectInfo.power_data.power_value_info.s_meter_act_power +
        gstCollectInfo.power_data.power_value_info.t_meter_act_power,
        //油机有功功率
        gstCollectInfo.power_data.power_value_info.r_gen_act_power +
        gstCollectInfo.power_data.power_value_info.s_gen_act_power +
        gstCollectInfo.power_data.power_value_info.t_gen_act_power,
    };

    static int32_t i_count = 0;
    int32_t i = GUI_Exec();
    if (i == 0)
    {
        ++i_count;
    }
    else
    {
        i_count = 0;
    }
    if (i_count > 100)
    {
        if (gstEmWinHomePage.win_handle)
        {
            WM_Paint(gstEmWinHomePage.win_handle);
        }
        i_count = 0;
    }
    //WM_BringToTop(gstEmWinListOperaInfo.win_handle);
    //gstEmWinListOperaInfo.polling();
    //网络

    if(_gstUiCtx.net_state != gstCollectInfo.maincard_data.sys_info.net_conn_state)
    {
        _gstUiCtx.net_state = gstCollectInfo.maincard_data.sys_info.net_conn_state;
        _SetHomePageWifiStat(1, _gstUiCtx.net_state);
    }
    //系统状态
    if(_gstUiCtx.sys_status != gstCollectInfo.power_data.model_info.on_grid_state)
    {
        if(gstCollectInfo.power_data.model_info.on_grid_state >= 3)
        {
            gstCollectInfo.power_data.model_info.on_grid_state = 1;
            // return;
        }
        _gstUiCtx.sys_status = gstCollectInfo.power_data.model_info.on_grid_state;
        SetHomePageSysStatus(1, (EN_SYS_STATUS_E)_gstUiCtx.sys_status);
    }
    //pcs状态
    if(_gstUiCtx.work_mode != gstCollectInfo.power_data.power_set_info.work_mode)
    {
        _gstUiCtx.work_mode = gstCollectInfo.power_data.power_set_info.work_mode;
        SetHomePageWorkMode(1, (EN_WORK_MODE_E)_gstUiCtx.work_mode);
    }

    //rtc时间
    if (_gstUiCtx.rtc_time != rtctime)
    {
        _gstUiCtx.rtc_time = rtctime;
        SetHomePageTime(1, _gstUiCtx.rtc_time);
    }
    //电网功率
    if(_gstUiCtx.grid_power != arr_power[1])
    {
        _gstUiCtx.grid_power = arr_power[1];
        _gstUiCtx.grid_curr = gstCollectInfo.power_data.power_value_info.r_meter_current;
        SetGridPower(1, _gstUiCtx.grid_power, _gstUiCtx.generator_enable);
    }
    //负载功率
    if(_gstUiCtx.home_power != arr_power[0])
    {
        _gstUiCtx.home_power = arr_power[0];
        SetHomePower(1, _gstUiCtx.home_power);
    }

    //电池功率
    if(_gstUiCtx.roca_power != bat_power)
    {
        _gstUiCtx.roca_power = bat_power;
        SetBatteryPower(1, _gstUiCtx.roca_power);
    }
    //太阳能功率
    if(_gstUiCtx.solar_power != solar_power)
    {
        _gstUiCtx.solar_power = solar_power;
        SetSolarPower(1, _gstUiCtx.solar_power);
    }
    //ev
    if(_gstUiCtx.ev_power != gstCollectInfo.power_data.power_value_info.inv_ev_power)
    {
        _gstUiCtx.ev_power = gstCollectInfo.power_data.power_value_info.inv_ev_power;
        SetEVPower(1, _gstUiCtx.ev_power);
    }
    //油机
    if(_gstUiCtx.generator_power != arr_power[2])
    {
        _gstUiCtx.generator_power = arr_power[2]; 
        SetGeneratorPower(1, _gstUiCtx.generator_power, _gstUiCtx.generator_enable);
    }
    //soc
    if(_gReadAllBmsInfoOkFlag)  //未获取到全部电池包的信息前不进行polling  soc显示-.-
    {
        if(_gstUiCtx.soc != gstCollectInfo.maincard_data.total_pack_info.soc)
        {
            _gstUiCtx.soc = gstCollectInfo.maincard_data.total_pack_info.soc;
            SetElecChange(1, _gstUiCtx.soc);
        }
    }
    //温度
    if(_gstUiCtx.temperature != gstCollectInfo.power_data.model_info.inv_box_temp)
    {
        _gstUiCtx.temperature = gstCollectInfo.power_data.model_info.inv_box_temp;
        SetTempStaBar(1, _gstUiCtx.temperature);
    }
    //是否充电
    if(_gstUiCtx.is_charge != gstCollectInfo.power_data.model_info.dcdc_run_state)
    {
        _gstUiCtx.is_charge = gstCollectInfo.power_data.model_info.dcdc_run_state;
        SetIsCharge(1, _gstUiCtx.is_charge);
    }
}

/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
    int32_t event = 0;
    static int32_t show = 0;
    event = EmGetKeyEvent(p_msg);
    if(event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }

    if (event == EN_KEY_SRC_ESC)
    {
        if(pCurrentEmWin == NULL)
        {
            return;
        }
//      EmDebugWinShow(pCurrentEmWin);
        return;
    }

    if(event == EN_KEY_SRC_CONFIRM)
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

    if(event == EN_KEY_SRC_RIGHT)
    {
        if(pCurrentEmWin->win_handle != 0)
        {
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = &gstEmWinElectricityInfo;
            pCurrentEmWin->create_win();
        }
        return;
    }
}

