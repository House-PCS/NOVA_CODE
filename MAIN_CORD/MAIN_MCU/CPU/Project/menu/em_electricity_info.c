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

#define DEBUG_TEST              0


#define EM_ELEC_INFO_ICON_GRID_CHARGE_SIZE     (4 * 1024)
#define EM_ELEC_INFO_ICON_GRID_TIED_SIZE       (4 * 1024)
#define EM_ELEC_INFO_ICON_PV_CHARGE_SIZE       (4 * 1024)
#define EM_ELEC_INFO_ICON_LOAD_SIZE            (4 * 1024)

#define EM_ELEC_INFO_MID_TIME_ICON_SIZE                 (4* 1024 * 2)
#define EM_ELEC_INFO_BOTTOM_BATTERY_ICON_SIZE           (4* 1024 * 2)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinElectricityInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t load_elec_cost_today;
    uint32_t load_elec_cost_month;
    uint32_t load_elec_cost_year;
    uint32_t load_elec_cost_total;
    uint16_t electric_purchase_today;
    uint32_t electric_purchase_month;
    uint32_t electric_purchase_year;
    uint32_t electric_purchase_total;
    uint16_t electric_sale_today;
    uint32_t electric_sale_month;
    uint32_t electric_sale_year;
    uint32_t electric_sale_total;
    uint16_t pv_generate_today;
    uint32_t pv_generate_month;
    uint32_t pv_generate_year;
    uint32_t pv_generate_total;
    uint16_t soh;
    uint32_t grid_conn_nums;    //20241125 修改为并离网切换次数寄存器
    uint32_t on_grid_time;
    uint32_t roca_opt_time;

} UiCtx_t;

typedef struct _grid_charge_stru_t
{
    float elec_today;
    float elec_mon;
    float elec_year;
    float elec_total;
} GridCharge_t;

typedef struct _grid_tied_stru_t
{
    float elec_today;
    float elec_mon;
    float elec_year;
    float elec_total;
} GridTied_t;

typedef struct _pv_charge_stru_t
{
    float elec_today;
    float elec_mon;
    float elec_year;
    float elec_total;
} PvCharge_t;

typedef struct _backup_load_stru_t
{
    float elec_today;
    float elec_mon;
    float elec_year;
    float elec_total;
} BackupLoad_t;

/*函数声明*/
int32_t SetGridChargeInfo(GridCharge_t grid_charge);//电网发电统计
int32_t SetGridTiedInfo(GridTied_t grid_tied);//向电网馈电统计
int32_t SetPVChargeInfo(PvCharge_t pv_charge);//太阳能充电统计
int32_t SetBackupLoadInfo(BackupLoad_t backup_load);//负载用电统计
int32_t SetRocaOPTTime(int32_t c_time);//设置Roca OPT Time
int32_t SetOnGridTime(int32_t c_time);//设置On Grid Time
int32_t SetBatteryCycNum(int32_t c_nums);//设置电池循环次数
int32_t SetSOHState(int32_t c_precentage);//设置SOH

static UiCtx_t _gstUiCtx;
/*各个子窗口控件句柄及其他相关参数*/
/*顶部电量信息统计子窗口*/
static GUI_HWIN  _gWinElecInfoHandle;
static IMAGE_Handle _gImgGridChargeHandle;//电网充电图标句柄
static IMAGE_Handle _gImgGridTiedHandle;//电网馈电图标句柄
static IMAGE_Handle _gImgPVChargeHandle;//太阳能充电图标句柄
static IMAGE_Handle _gImgLoadHandle;//负载图标句柄
static TEXT_Handle  _gTextkWhHandle;
static TEXT_Handle  _gTextTodayHandle;
static TEXT_Handle  _gTextMonHandle;
static TEXT_Handle  _gTextYearHandle;
static TEXT_Handle  _gTextTotalHandle;

/*当天电量统计*/
static TEXT_Handle _gTextGridChargeToadyHandle;
static TEXT_Handle _gTextGridTiedTodayHandle;
static TEXT_Handle _gTextPVchargeTodayHandle;
static TEXT_Handle _gTextLoadTodayHandle;
/*当月电量统计*/
static TEXT_Handle  _gTextGridChargeMonHandle;
static TEXT_Handle  _gTextGridTiedMonHandle;
static TEXT_Handle  _gTextPVchargeMonHandle;
static TEXT_Handle  _gTextLoadMonHandle;
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
static TEXT_Handle  _gTextRocaOPTHandle;
static TEXT_Handle  _gTextRocaOPTTimeHandle;
static TEXT_Handle  _gTextOnGirdHandle;
static TEXT_Handle  _gTextOnGirdTimeHandle;

/*底部区域*/
static IMAGE_Handle _gImgBatteryHandle;//电池图标句柄
static TEXT_Handle  _gTextBatCyclesHandle;
static TEXT_Handle  _gTextBatCyclesNumsHandle;
static TEXT_Handle  _gTextSOHHandle;  //soh文本
static TEXT_Handle  _gTextSOHStatHandle; //soh的百分比值


/*顶部电量统计窗口图标参数*/
static emWidgetImgPara_t _gstElecGridChargeImgPara;
static emWidgetImgPara_t _gstElecGridTiedImgPara;
static emWidgetImgPara_t _gstElecPVChargeImgPara;
static emWidgetImgPara_t _gstElecLoadImgPara;
static emWidgetTextPara_t _gstKWhTextPara;
static emWidgetTextPara_t _gstTodayTextPara;
static emWidgetTextPara_t _gstMonTextPara;
static emWidgetTextPara_t _gstYearTextPara;
static emWidgetTextPara_t _gstTotalTextPara;
static char _garrkWhTextBuf[20] = "kWh";
static char _garrTodayTextBuf[20] = "Today";
static char _garrMonTextBuf[20] = "Mon";
static char _garrYearTextBuf[20] = "Year";
static char _garrTotalTextBuf[20] = "Total";

/*当天电量统计*/
static emWidgetTextPara_t _gstElecGridChargeTodayTextPara;
static emWidgetTextPara_t _gstElecGridTiedTodayTextPara;
static emWidgetTextPara_t _gstElecPVChargeTodayTextPara;
static emWidgetTextPara_t _gstElecLoadTodayTextParas;
static char _garrGridChargeToadyTextBuf[20] = {0};
static char _garrGridTiedTodayTextBuf[20] = {0};
static char _garrPVchargeTodayTextBuf[20] = {0};
static char _garrLoadTodayTextBuf[20] = {0};
/*当月电量统计*/
static emWidgetTextPara_t _gstElecGridChargeMonTextPara;
static emWidgetTextPara_t _gstElecGridTiedMonTextPara;
static emWidgetTextPara_t _gstElecPVChargeMonTextPara;
static emWidgetTextPara_t _gstElecLoadMonTextPara;
static char _garrGridChargeMonTextBuf[20] = {0};
static char _garrGridTiedMonTextBuf[20] = {0};
static char _garrPVchargeMonTextBuf[20] = {0};
static char _garrLoadMonTextBuf[20] = {0};
/*当年电量统计*/
static emWidgetTextPara_t _gstElecGridChargeYearTextPara;
static emWidgetTextPara_t _gstElecGridTiedYearTextPara;
static emWidgetTextPara_t _gstElecPVChargeYearTextPara;
static emWidgetTextPara_t _gstElecLoadYearTextPara;
static char _garrGridChargeYearTextBuf[20] = {0};
static char _garrGridTiedYearTextBuf[20] = {0};
static char _garrPVchargeYearTextBuf[20] = {0};
static char _garrLoadYearTextBuf[20] = {0};
/*总电量统计*/
static emWidgetTextPara_t _gstElecGridChargeTotalTextPara;
static emWidgetTextPara_t _gstElecGridTiedTotalTextPara;
static emWidgetTextPara_t _gstElecPVChargeTotalTextPara;
static emWidgetTextPara_t _gstElecLoadTotalTextPara;
static char _garrGridChargeTotalTextBuf[20] = {0};
static char _garrGridTiedTotalTextBuf[20] = {0};
static char _garrPVchargeTotalTextBuf[20] = {0};
static char _garrLoadTotalTextBuf[20] = {0};

/*中间发电时长窗口图标参数*/
static GUI_HWIN _gWinElecMidInfoHandle;
static emWidgetImgPara_t   _gstRocaOptTimeImgPara;
static emWidgetTextPara_t  _gstRocaOPTTextPara;
static emWidgetTextPara_t  _gstRocaOPTTimeTextPara;
static emWidgetTextPara_t  _gstOnGirdTextPara;
static emWidgetTextPara_t  _gstOnGirdTimeTextPara;
static char _garrRocaOPTTextBuf[20] = {0};
static char _garrRocaOPTTimeTextBuf[20] = {0};
static char _garrOnGirdTextBuf[20] = {0};
static char _garrOnGirdTimeTextBuf[20] = {0};
/*底部区域*/
static GUI_HWIN _gWinElecBottomInfoHandle;
static emWidgetImgPara_t   _gstBatteryImgPara;
static emWidgetTextPara_t  _gstBatCyclesTextPara;
static emWidgetTextPara_t  _gstBatCyclesNumsTextPara;
static emWidgetTextPara_t  _gstSOHTextPara;
static emWidgetTextPara_t  _gstSOHStatTextPara;
static char _garrBatCyclesTextBuf[20] = {0};
static char _garrBatCyclesNumsTextBuf[20] = {0};
static char _garrSOHTextBuf[20] = {0};
static char _garrSOHStatTextBuf[20] = {0};



/*顶部状态栏控件ID号*/
#define WIN_ELECTRICITY_INFO_ID                             (GUI_ID_USER + 0x10)
#define WIN_ELEC_INFO_GRID_CHARGE_IMG_ID                    (GUI_ID_USER + 0x11)
#define WIN_ELEC_INFO_GRID_TIED_IMG_ID                      (GUI_ID_USER + 0x12)
#define WIN_ELEC_INFO_PV_CHARGE_IMG_ID                      (GUI_ID_USER + 0x13)
#define WIN_ELEC_INFO_LOAD_IMG_ID                           (GUI_ID_USER + 0x14)
#define WIN_ELEC_INFO_KWH_TEXT_ID                           (GUI_ID_USER + 0x15)
#define WIN_ELEC_INFO_TODAY_TEXT_ID                         (GUI_ID_USER + 0x16)
#define WIN_ELEC_INFO_NOV_TEXT_ID                           (GUI_ID_USER + 0x17)
#define WIN_ELEC_INFO_YEAR_TEXT_ID                          (GUI_ID_USER + 0x18)
#define WIN_ELEC_INFO_TOTAL_TEXT_ID                         (GUI_ID_USER + 0x19)

#define WIN_ELEC_INFO_GRID_CHARGE_TODAY_TEXT_ID             (GUI_ID_USER + 0x20)
#define WIN_ELEC_INFO_GRID_CHARGE_NOV_TEXT_ID               (GUI_ID_USER + 0x21)
#define WIN_ELEC_INFO_GRID_CHARGE_YEAR_TEXT_ID              (GUI_ID_USER + 0x22)
#define WIN_ELEC_INFO_GRID_CHARGE_TOTAL_TEXT_ID             (GUI_ID_USER + 0x23)
#define WIN_ELEC_INFO_GRID_TIED_TODAY_TEXT_ID               (GUI_ID_USER + 0x24)
#define WIN_ELEC_INFO_GRID_TIED_NOV_TEXT_ID                 (GUI_ID_USER + 0x25)
#define WIN_ELEC_INFO_GRID_TIED_YEAR_TEXT_ID                (GUI_ID_USER + 0x26)
#define WIN_ELEC_INFO_GRID_TIED_TOTAL_TEXT_ID               (GUI_ID_USER + 0x27)
#define WIN_ELEC_INFO_PV_CHARGE_TODAY_TEXT_ID               (GUI_ID_USER + 0x28)
#define WIN_ELEC_INFO_PV_CHARGE_NOV_TEXT_ID                 (GUI_ID_USER + 0x29)
#define WIN_ELEC_INFO_PV_CHARGE_YEAR_TEXT_ID                (GUI_ID_USER + 0x2A)
#define WIN_ELEC_INFO_PV_CHARGE_TOTAL_TEXT_ID               (GUI_ID_USER + 0x2B)
#define WIN_ELEC_INFO_LOAD_TODAY_TEXT_ID                    (GUI_ID_USER + 0x2C)
#define WIN_ELEC_INFO_LOAD_NOV_TEXT_ID                      (GUI_ID_USER + 0x2D)
#define WIN_ELEC_INFO_LOAD_YEAR_TEXT_ID                     (GUI_ID_USER + 0x2E)
#define WIN_ELEC_INFO_LOAD_TOTAL_TEXT_ID                    (GUI_ID_USER + 0x2F)

#define WIN_ELEC_INFO_ROCA_OPT_IMG_ID                       (GUI_ID_USER + 0x30)
#define WIN_ELEC_INFO_ROCA_OPT_TEXT_ID                      (GUI_ID_USER + 0x31)
#define WIN_ELEC_INFO_ROCA_OPT_TIME_TEXT_ID                 (GUI_ID_USER + 0x32)
#define WIN_ELEC_INFO_ON_GRID_TEXT_ID                       (GUI_ID_USER + 0x33)
#define WIN_ELEC_INFO_ON_GRID_TIME_TEXT_ID                  (GUI_ID_USER + 0x34)
#define WIN_ELEC_INFO_BATTERY_IMG_ID                        (GUI_ID_USER + 0x35)
#define WIN_ELEC_INFO_BATTERY_CYC_TEXT_ID                   (GUI_ID_USER + 0x36)
#define WIN_ELEC_INFO_BATTERY_CYC_NUMS_TEXT_ID              (GUI_ID_USER + 0x37)
#define WIN_ELEC_INFO_SOH_TEXT_ID                           (GUI_ID_USER + 0x38)
#define WIN_ELEC_INFO_SOH_STAT_TEXT_ID                      (GUI_ID_USER + 0x39)

#define WIN_ELEC_MID_INFO_ID                                (GUI_ID_USER + 0x3A)
#define WIN_ELEC_MID_INFO_TIME_IMG_ID                       (GUI_ID_USER + 0x3B)
#define WIN_ELEC_MID_INFO_ROCA_OPT_TEXT_ID                  (GUI_ID_USER + 0x3C)
#define WIN_ELEC_MID_INFO_ROCA_OPT_TIME_TEXT_ID             (GUI_ID_USER + 0x3D)
#define WIN_ELEC_MID_INFO_ON_GRID_TEXT_ID                   (GUI_ID_USER + 0x3E)
#define WIN_ELEC_MID_INFO_ON_GRID_TIME_TEXT_ID              (GUI_ID_USER + 0x3F)

#define WIN_ELEC_BOTTOM_INFO_ID                             (GUI_ID_USER + 0x3A)
#define WIN_ELEC_BOTTOM_INFO_BATTERY_IMG_ID                 (GUI_ID_USER + 0x40)
#define WIN_ELEC_BOTTOM_INFO_BAT_CYC_TEXT_ID                (GUI_ID_USER + 0x41)
#define WIN_ELEC_BOTTOM_INFO_BAT_CYC_NUMS_TEXT_ID           (GUI_ID_USER + 0x42)
#define WIN_ELEC_BOTTOM_INFO_SOH_TEXT_ID                    (GUI_ID_USER + 0x43)
#define WIN_ELEC_BOTTOM_INFO_SOH_STAT_TEXT_ID               (GUI_ID_USER + 0x44)

/*资源列表*/
/**
 * @brief 顶部状态栏
 */
static __SDRAM GUI_WIDGET_CREATE_INFO _garrElecInfoStatDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "StatBar",        WIN_ELECTRICITY_INFO_ID,                            0,      0,   EM_XSIZE,  169,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "kwh",            WIN_ELEC_INFO_KWH_TEXT_ID,                          30,     20,     30,   17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "today",          WIN_ELEC_INFO_TODAY_TEXT_ID,                        60 + 16,  20,     48,   17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Mon",            WIN_ELEC_INFO_NOV_TEXT_ID,                          175,    20,     30,   17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "year",           WIN_ELEC_INFO_YEAR_TEXT_ID,                         272,    20,     34,   17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "total",          WIN_ELEC_INFO_TOTAL_TEXT_ID,                        388,    20,     50,   17,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "gridcharge",     WIN_ELEC_INFO_GRID_CHARGE_IMG_ID,                   30,     53,     30,   18,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "gridtied",       WIN_ELEC_INFO_GRID_TIED_IMG_ID,                     30,     80,     30,   18,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "pvcharge",       WIN_ELEC_INFO_PV_CHARGE_IMG_ID,                     32,     110,    15,   15,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "load",           WIN_ELEC_INFO_LOAD_IMG_ID,                          31,     136,    16,   16,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "today1",         WIN_ELEC_INFO_GRID_CHARGE_TODAY_TEXT_ID,            83,     53,     50,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Mon1",           WIN_ELEC_INFO_GRID_CHARGE_NOV_TEXT_ID,              165,    53,     52,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "year1",          WIN_ELEC_INFO_GRID_CHARGE_YEAR_TEXT_ID,             259,    53,     59,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "total1",         WIN_ELEC_INFO_GRID_CHARGE_TOTAL_TEXT_ID,            372,    53,     70,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "today2",         WIN_ELEC_INFO_GRID_TIED_TODAY_TEXT_ID,              83,     81,     50,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Mon2",           WIN_ELEC_INFO_GRID_TIED_NOV_TEXT_ID,                165,    81,     52,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "year2",          WIN_ELEC_INFO_GRID_TIED_YEAR_TEXT_ID,               259,    81,     59,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "total2",         WIN_ELEC_INFO_GRID_TIED_TOTAL_TEXT_ID,              372,    81,     70,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "today3",         WIN_ELEC_INFO_PV_CHARGE_TODAY_TEXT_ID,              83,     109,    50,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Mon3",           WIN_ELEC_INFO_PV_CHARGE_NOV_TEXT_ID,                165,    109,    52,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "year3",          WIN_ELEC_INFO_PV_CHARGE_YEAR_TEXT_ID,               259,    109,    59,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "total3",         WIN_ELEC_INFO_PV_CHARGE_TOTAL_TEXT_ID,              372,    109,    70,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "today4",         WIN_ELEC_INFO_LOAD_TODAY_TEXT_ID,                   83,     137,    50,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Mon4",           WIN_ELEC_INFO_LOAD_NOV_TEXT_ID,                     165,    137,    52,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "year4",          WIN_ELEC_INFO_LOAD_YEAR_TEXT_ID,                    259,    137,    59,   22,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "total4",         WIN_ELEC_INFO_LOAD_TOTAL_TEXT_ID,                   372,    137,    70,   22,   0,   0x0,  0 },
};

/**
 * @brief 中间信息栏
 */
static __SDRAM GUI_WIDGET_CREATE_INFO _garrMiddleTextInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "middle",         WIN_ELEC_MID_INFO_ID,                           0,      177,    EM_XSIZE,  62,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "ImgTime",        WIN_ELEC_MID_INFO_TIME_IMG_ID,                  20,     12,     38,     38,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "roca",           WIN_ELEC_MID_INFO_ROCA_OPT_TEXT_ID,             63,     23,     130,    19,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "roca time",      WIN_ELEC_MID_INFO_ROCA_OPT_TIME_TEXT_ID,        210,    23,     60,     19,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "ongrid",         WIN_ELEC_MID_INFO_ON_GRID_TEXT_ID,              290,    23,     130,    19,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "ongridtime",     WIN_ELEC_MID_INFO_ON_GRID_TIME_TEXT_ID,         400,    23,     60,     19,     0,   0x0,  0 },
};

/**
 * @brief 底部信息栏
 */
static __SDRAM GUI_WIDGET_CREATE_INFO _garrBottomTextInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "bottom",         WIN_ELEC_BOTTOM_INFO_ID,                        0,      247,    EM_XSIZE,  62,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "Imgbat",         WIN_ELEC_BOTTOM_INFO_BATTERY_IMG_ID,            20,     12,     38,     38,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "bat cyc",        WIN_ELEC_BOTTOM_INFO_BAT_CYC_TEXT_ID,           63,     23,     240,    19,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "bat cyc nums",   WIN_ELEC_BOTTOM_INFO_BAT_CYC_NUMS_TEXT_ID,      240,    23,     38,     19,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "SOH",            WIN_ELEC_BOTTOM_INFO_SOH_TEXT_ID,               290,    23,     160,    19,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "SOH Stat",       WIN_ELEC_BOTTOM_INFO_SOH_STAT_TEXT_ID,          400,    23,     60,     19,     0,   0x0,  0 },
};

/**
  * @brief  设置从电网买电量（当天，当月，当年，总量）
  * @param  NULL
  * @retval NULL
  */
static  int32_t _SetElectricPurchase(uint8_t ui_update, uint16_t today_elec, uint32_t month_elec, uint32_t year_elec, uint32_t total_elec)
{
#if 0
    float today_tmp = (float)today_elec / 10;
    uint32_t month_tmp = (int32_t)month_elec / 10;
    uint32_t year_tmp  = (int32_t)year_elec / 10;
    uint32_t total_tmp = (int32_t)total_elec / 10;
    
    snprintf(_garrGridChargeToadyTextBuf, sizeof(_garrGridChargeToadyTextBuf), "%.1f", today_tmp);
    snprintf(_garrGridChargeMonTextBuf,   sizeof(_garrGridChargeMonTextBuf),   "%d",   month_tmp);
    snprintf(_garrGridChargeYearTextBuf,  sizeof(_garrGridChargeYearTextBuf),  "%d",   year_tmp);
    snprintf(_garrGridChargeTotalTextBuf, sizeof(_garrGridChargeTotalTextBuf), "%d",   total_tmp);
#else
    float today_tmp = (float)today_elec / 10.0f;
    float month_tmp = (float)month_elec / 10.0f;
    float year_tmp  = (float)year_elec / 10.0f;
    float total_tmp = (float)total_elec / 10.0f;
    
    snprintf(_garrGridChargeToadyTextBuf, sizeof(_garrGridChargeToadyTextBuf), "%.1f",   today_tmp);
    snprintf(_garrGridChargeMonTextBuf,   sizeof(_garrGridChargeMonTextBuf),   "%.1f",   month_tmp);
    snprintf(_garrGridChargeYearTextBuf,  sizeof(_garrGridChargeYearTextBuf),  "%.1f",   year_tmp);
    snprintf(_garrGridChargeTotalTextBuf, sizeof(_garrGridChargeTotalTextBuf), "%.1f",   total_tmp);
#endif
    
    
    if(ui_update)
    {
        TEXT_SetText(_gTextGridChargeToadyHandle, (const char *)_gstElecGridChargeTodayTextPara.p_content);
        TEXT_SetText(_gTextGridChargeMonHandle,  (const char *)_gstElecGridChargeMonTextPara.p_content);
        TEXT_SetText(_gTextGridChargeYearHandle, (const char *)_gstElecGridChargeYearTextPara.p_content);
        TEXT_SetText(_gTextGridChargeTotalHandle, (const char *)_gstElecGridChargeTotalTextPara.p_content);
    }
    return 0;
}
/**
  * @brief  设置向电网的卖电量（当天，当月，当年，总量）
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetElectricSale(uint8_t ui_update, uint16_t today_elec, uint32_t month_elec, uint32_t year_elec, uint32_t total_elec)
{
#if 0
    float today_tmp = (float)today_elec / 10;
    uint32_t month_tmp = (int32_t)month_elec / 10;
    uint32_t year_tmp  = (int32_t)year_elec / 10;
    uint32_t total_tmp = (int32_t)total_elec / 10;

    snprintf(_garrGridTiedTodayTextBuf, sizeof(_garrGridTiedTodayTextBuf), "%.1f", today_tmp);
    snprintf(_garrGridTiedMonTextBuf,   sizeof(_garrGridTiedMonTextBuf),   "%d",   month_tmp);
    snprintf(_garrGridTiedYearTextBuf,  sizeof(_garrGridTiedYearTextBuf),  "%d",   year_tmp);
    snprintf(_garrGridTiedTotalTextBuf, sizeof(_garrGridTiedTotalTextBuf), "%d",   total_tmp);
#else 
    float today_tmp = (float)today_elec / 10.0f;
    float month_tmp = (float)month_elec / 10.0f;
    float year_tmp  = (float)year_elec / 10.0f;
    float total_tmp = (float)total_elec / 10.0f;
    
    snprintf(_garrGridTiedTodayTextBuf, sizeof(_garrGridTiedTodayTextBuf), "%.1f",   today_tmp);
    snprintf(_garrGridTiedMonTextBuf,   sizeof(_garrGridTiedMonTextBuf),   "%.1f",   month_tmp);
    snprintf(_garrGridTiedYearTextBuf,  sizeof(_garrGridTiedYearTextBuf),  "%.1f",   year_tmp);
    snprintf(_garrGridTiedTotalTextBuf, sizeof(_garrGridTiedTotalTextBuf), "%.1f",   total_tmp);
#endif   
    if(ui_update)
    {
        TEXT_SetText(_gTextGridTiedTodayHandle, (const char *)_gstElecGridTiedTodayTextPara.p_content);
        TEXT_SetText(_gTextGridTiedMonHandle,  (const char *)_gstElecGridTiedMonTextPara.p_content);
        TEXT_SetText(_gTextGridTiedYearHandle, (const char *)_gstElecGridTiedYearTextPara.p_content);
        TEXT_SetText(_gTextGridTiedTotalHandle, (const char *)_gstElecGridTiedTotalTextPara.p_content);
    }
    return 0;
}
/**
  * @brief  设置负载用电量（当天，当月，当年，总量）
  * @param  NULL
  * @retval NULL
  */
static  int32_t _SetLoadElectric(uint8_t ui_update, uint16_t today_elec, uint32_t month_elec, uint32_t year_elec, uint32_t total_elec)
{
#if 0
    float today_tmp = (float)today_elec / 10;
    uint32_t month_tmp = (int32_t)month_elec / 10;
    uint32_t year_tmp  = (int32_t)year_elec / 10;
    uint32_t total_tmp = (int32_t)total_elec / 10;

    snprintf(_garrLoadTodayTextBuf, sizeof(_garrLoadTodayTextBuf), "%.1f", today_tmp);
    snprintf(_garrLoadMonTextBuf,   sizeof(_garrLoadMonTextBuf),   "%d",   month_tmp);
    snprintf(_garrLoadYearTextBuf,  sizeof(_garrLoadYearTextBuf),  "%d",   year_tmp);
    snprintf(_garrLoadTotalTextBuf, sizeof(_garrLoadTotalTextBuf), "%d",   total_tmp);
#else
    float today_tmp = (float)today_elec / 10.0f;
    float month_tmp = (float)month_elec / 10.0f;
    float year_tmp  = (float)year_elec / 10.0f;
    float total_tmp = (float)total_elec / 10.0f;
    
    snprintf(_garrLoadTodayTextBuf, sizeof(_garrLoadTodayTextBuf), "%.1f",   today_tmp);
    snprintf(_garrLoadMonTextBuf,   sizeof(_garrLoadMonTextBuf),   "%.1f",   month_tmp);
    snprintf(_garrLoadYearTextBuf,  sizeof(_garrLoadYearTextBuf),  "%.1f",   year_tmp);
    snprintf(_garrLoadTotalTextBuf, sizeof(_garrLoadTotalTextBuf), "%.1f",   total_tmp);
    
#endif
    if(ui_update)
    {
        TEXT_SetText(_gTextLoadTodayHandle, (const char *)_gstElecLoadTodayTextParas.p_content);
        TEXT_SetText(_gTextLoadMonHandle,   (const char *)_gstElecLoadMonTextPara.p_content);
        TEXT_SetText(_gTextLoadYearHandle,  (const char *)_gstElecLoadYearTextPara.p_content);
        TEXT_SetText(_gTextLoadTotalHandle, (const char *)_gstElecLoadTotalTextPara.p_content);
    }
    return 0;
}
/**
  * @brief  设置光伏发电量（当天，当月，当年，总量）
  * @param  NULL
  * @retval NULL
  */
static  int32_t _SetPVElectric(uint8_t ui_update, uint16_t today_elec, uint32_t month_elec, uint32_t year_elec, uint32_t total_elec)
{
#if 0
    float today_tmp = (float)today_elec / 10;
    uint32_t month_tmp = (int32_t)month_elec / 10;
    uint32_t year_tmp  = (int32_t)year_elec / 10;
    uint32_t total_tmp = (int32_t)total_elec / 10;

    snprintf(_garrPVchargeTodayTextBuf, sizeof(_garrPVchargeTodayTextBuf), "%.1f", today_tmp);
    snprintf(_garrPVchargeMonTextBuf,   sizeof(_garrPVchargeMonTextBuf),   "%d",   month_tmp);
    snprintf(_garrPVchargeYearTextBuf,  sizeof(_garrPVchargeYearTextBuf),  "%d",   year_tmp);
    snprintf(_garrPVchargeTotalTextBuf, sizeof(_garrPVchargeTotalTextBuf), "%d",   total_tmp);
#else
    float today_tmp = (float)today_elec / 10.0f;
    float month_tmp = (float)month_elec / 10.0f;
    float year_tmp  = (float)year_elec / 10.0f;
    float total_tmp = (float)total_elec / 10.0f;
    
    snprintf(_garrPVchargeTodayTextBuf, sizeof(_garrPVchargeTodayTextBuf), "%.1f",   today_tmp);
    snprintf(_garrPVchargeMonTextBuf,   sizeof(_garrPVchargeMonTextBuf),   "%.1f",   month_tmp);
    snprintf(_garrPVchargeYearTextBuf,  sizeof(_garrPVchargeYearTextBuf),  "%.1f",   year_tmp);
    snprintf(_garrPVchargeTotalTextBuf, sizeof(_garrPVchargeTotalTextBuf), "%.1f",   total_tmp);
    
#endif
    if(ui_update)
    {
        TEXT_SetText(_gTextPVchargeTodayHandle, (const char *)_gstElecPVChargeTodayTextPara.p_content);
        TEXT_SetText(_gTextPVchargeMonHandle,   (const char *)_gstElecPVChargeMonTextPara.p_content);
        TEXT_SetText(_gTextPVchargeYearHandle,  (const char *)_gstElecPVChargeYearTextPara.p_content);
        TEXT_SetText(_gTextPVChargeTotalHandle, (const char *)_gstElecPVChargeTotalTextPara.p_content);
    }
    return 0;
}
/**
  * @brief  设置SOH
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetSOHValue(uint8_t ui_update, uint16_t soh)
{
    float bat_soh = (float)soh / 10;

    snprintf(_garrSOHStatTextBuf, sizeof(_garrSOHStatTextBuf), "%.1f%%", bat_soh);

    if( ui_update)
    {
        TEXT_SetText(_gTextSOHStatHandle, (const char *)_gstSOHStatTextPara.p_content);
    }
    return 0;
}


/**
  * @brief  设置Roca OPT Time
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetRocaOPTTime(uint8_t ui_update, uint32_t c_time)
{
    static float time_show = 0.0f;

    if(c_time < 60)
    {
        snprintf(_garrRocaOPTTimeTextBuf, sizeof(_garrRocaOPTTimeTextBuf), "%ds", c_time);
    }
    else if(c_time >= 60 && c_time < 3600)
    {
        time_show  = (float)c_time / 60;
        snprintf(_garrRocaOPTTimeTextBuf, sizeof(_garrRocaOPTTimeTextBuf), "%.1fmin", time_show);
    }
    else if (c_time >= 3600)
    {
        time_show  = (float)c_time / 3600;
        snprintf(_garrRocaOPTTimeTextBuf, sizeof(_garrRocaOPTTimeTextBuf), "%.2fh", time_show);
    }

    if(ui_update)
    {
        TEXT_SetText(_gTextRocaOPTTimeHandle, (const char *)_gstRocaOPTTimeTextPara.p_content);
    }
    return 0;
}
/**
  * @brief  设置On Grid Time
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetOnGridTime(uint8_t ui_update, uint32_t c_time)
{
    float time_show = 0.0f;

    if(c_time < 60)
    {
        snprintf(_garrOnGirdTimeTextBuf, sizeof(_garrOnGirdTimeTextBuf), "%ds", c_time);
    }
    else if(c_time >= 60 && c_time < 3600)
    {
        time_show  = (float)c_time / 60;
        snprintf(_garrOnGirdTimeTextBuf, sizeof(_garrOnGirdTimeTextBuf), "%.1fmin", time_show);
    }
    else if (c_time >= 3600)
    {
        time_show  = (float)c_time / 3600;
        snprintf(_garrOnGirdTimeTextBuf, sizeof(_garrOnGirdTimeTextBuf), "%.2fh", time_show);
    }

    if(ui_update)
    {
        TEXT_SetText(_gTextOnGirdTimeHandle, (const char *)_gstOnGirdTimeTextPara.p_content);
    }
    return 0;
}

/**
  * @brief  设置电池循环次数
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetPackCylceNum(uint8_t ui_update, uint16_t cycle)
{
    snprintf(_garrBatCyclesNumsTextBuf, sizeof(_garrBatCyclesNumsTextBuf), "%d", cycle);

    if(ui_update)
    {
        TEXT_SetText(_gTextBatCyclesNumsHandle, (const char *)_gstBatCyclesNumsTextPara.p_content);
    }
    return 0;
}



/**
  * @brief  ElecInfoTopStatSubWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbElecInfoTopStatSubWin(WM_MESSAGE* pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*初始化各个子控件*/
            /*电网充电*/
            _gImgGridChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_CHARGE_IMG_ID);
            IMAGE_SetDTA(_gImgGridChargeHandle, _gstElecGridChargeImgPara.p_img_buf, _gstElecGridChargeImgPara.img_size);
            //电网馈电
            _gImgGridTiedHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_TIED_IMG_ID);
            IMAGE_SetDTA(_gImgGridTiedHandle, _gstElecGridTiedImgPara.p_img_buf, _gstElecGridTiedImgPara.img_size);
            //太阳能充电
            _gImgPVChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_PV_CHARGE_IMG_ID);
            IMAGE_SetDTA(_gImgPVChargeHandle, _gstElecPVChargeImgPara.p_img_buf, _gstElecPVChargeImgPara.img_size);
            //负载
            _gImgLoadHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_LOAD_IMG_ID);
            IMAGE_SetDTA(_gImgLoadHandle, _gstElecLoadImgPara.p_img_buf, _gstElecLoadImgPara.img_size);

            //kwh
            _gTextkWhHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_KWH_TEXT_ID);
            TEXT_SetFont(_gTextkWhHandle, _gstKWhTextPara.p_text_font);
            TEXT_SetText(_gTextkWhHandle, (const char *)_gstKWhTextPara.p_content);
            TEXT_SetTextAlign(_gTextkWhHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextkWhHandle, _gstKWhTextPara.text_color);
            //today
            _gTextTodayHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_TODAY_TEXT_ID);
            TEXT_SetFont(_gTextTodayHandle, _gstTodayTextPara.p_text_font);
            TEXT_SetText(_gTextTodayHandle, (const char *)_gstTodayTextPara.p_content);
            TEXT_SetTextAlign(_gTextTodayHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextTodayHandle, _gstTodayTextPara.text_color);
            //Mon
            _gTextMonHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_NOV_TEXT_ID);
            TEXT_SetFont(_gTextMonHandle, _gstMonTextPara.p_text_font);
            TEXT_SetText(_gTextMonHandle, (const char *)_gstMonTextPara.p_content);
            TEXT_SetTextAlign(_gTextMonHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextMonHandle, _gstMonTextPara.text_color);
            //year
            _gTextYearHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_YEAR_TEXT_ID);
            TEXT_SetFont(_gTextYearHandle, _gstYearTextPara.p_text_font);
            TEXT_SetText(_gTextYearHandle, (const char *)_gstYearTextPara.p_content);
            TEXT_SetTextAlign(_gTextYearHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextYearHandle, _gstYearTextPara.text_color);
            //total
            _gTextTotalHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_TOTAL_TEXT_ID);
            TEXT_SetFont(_gTextTotalHandle, _gstTotalTextPara.p_text_font);
            TEXT_SetText(_gTextTotalHandle, (const char *)_gstTotalTextPara.p_content);
            TEXT_SetTextAlign(_gTextTotalHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextTotalHandle, _gstTotalTextPara.text_color);
            //grid charge today
            _gTextGridChargeToadyHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_CHARGE_TODAY_TEXT_ID);
            TEXT_SetFont(_gTextGridChargeToadyHandle, _gstElecGridChargeTodayTextPara.p_text_font);
            TEXT_SetText(_gTextGridChargeToadyHandle, (const char *)_gstElecGridChargeTodayTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridChargeToadyHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridChargeToadyHandle, _gstElecGridChargeTodayTextPara.text_color);
            //grid charge Mon
            _gTextGridChargeMonHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_CHARGE_NOV_TEXT_ID);
            TEXT_SetFont(_gTextGridChargeMonHandle, _gstElecGridChargeMonTextPara.p_text_font);
            TEXT_SetText(_gTextGridChargeMonHandle, (const char *)_gstElecGridChargeMonTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridChargeMonHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridChargeMonHandle, _gstElecGridChargeMonTextPara.text_color);
            //grid charge year
            _gTextGridChargeYearHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_CHARGE_YEAR_TEXT_ID);
            TEXT_SetFont(_gTextGridChargeYearHandle, _gstElecGridChargeYearTextPara.p_text_font);
            TEXT_SetText(_gTextGridChargeYearHandle, (const char *)_gstElecGridChargeYearTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridChargeYearHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridChargeYearHandle, _gstElecGridChargeYearTextPara.text_color);
            //grid charge total
            _gTextGridChargeTotalHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_CHARGE_TOTAL_TEXT_ID);
            TEXT_SetFont(_gTextGridChargeTotalHandle, _gstElecGridChargeTotalTextPara.p_text_font);
            TEXT_SetText(_gTextGridChargeTotalHandle, (const char *)_gstElecGridChargeTotalTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridChargeTotalHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridChargeTotalHandle, _gstElecGridChargeTotalTextPara.text_color);
            //grid tied today
            _gTextGridTiedTodayHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_TIED_TODAY_TEXT_ID);
            TEXT_SetFont(_gTextGridTiedTodayHandle, _gstElecGridTiedTodayTextPara.p_text_font);
            TEXT_SetText(_gTextGridTiedTodayHandle, (const char *)_gstElecGridTiedTodayTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridTiedTodayHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridTiedTodayHandle, _gstElecGridTiedTodayTextPara.text_color);
            //grid tied Mon
            _gTextGridTiedMonHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_TIED_NOV_TEXT_ID);
            TEXT_SetFont(_gTextGridTiedMonHandle, _gstElecGridTiedMonTextPara.p_text_font);
            TEXT_SetText(_gTextGridTiedMonHandle, (const char *)_gstElecGridTiedMonTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridTiedMonHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridTiedMonHandle, _gstElecGridTiedMonTextPara.text_color);
            //grid tied year
            _gTextGridTiedYearHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_TIED_YEAR_TEXT_ID);
            TEXT_SetFont(_gTextGridTiedYearHandle, _gstElecGridTiedYearTextPara.p_text_font);
            TEXT_SetText(_gTextGridTiedYearHandle, (const char *)_gstElecGridTiedYearTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridTiedYearHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridTiedYearHandle, _gstElecGridTiedYearTextPara.text_color);
            //grid tied total
            _gTextGridTiedTotalHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_GRID_TIED_TOTAL_TEXT_ID);
            TEXT_SetFont(_gTextGridTiedTotalHandle, _gstElecGridTiedTotalTextPara.p_text_font);
            TEXT_SetText(_gTextGridTiedTotalHandle, (const char *)_gstElecGridTiedTotalTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridTiedTotalHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridTiedTotalHandle, _gstElecGridTiedTotalTextPara.text_color);
            //pv charge today
            _gTextPVchargeTodayHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_PV_CHARGE_TODAY_TEXT_ID);
            TEXT_SetFont(_gTextPVchargeTodayHandle, _gstElecPVChargeTodayTextPara.p_text_font);
            TEXT_SetText(_gTextPVchargeTodayHandle, (const char *)_gstElecPVChargeTodayTextPara.p_content);
            TEXT_SetTextAlign(_gTextPVchargeTodayHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPVchargeTodayHandle, _gstElecPVChargeTodayTextPara.text_color);
            //pv charge Mon
            _gTextPVchargeMonHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_PV_CHARGE_NOV_TEXT_ID);
            TEXT_SetFont(_gTextPVchargeMonHandle, _gstElecPVChargeMonTextPara.p_text_font);
            TEXT_SetText(_gTextPVchargeMonHandle, (const char *)_gstElecPVChargeMonTextPara.p_content);
            TEXT_SetTextAlign(_gTextPVchargeMonHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPVchargeMonHandle, _gstElecPVChargeMonTextPara.text_color);
            //pv charge year
            _gTextPVchargeYearHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_PV_CHARGE_YEAR_TEXT_ID);
            TEXT_SetFont(_gTextPVchargeYearHandle, _gstElecPVChargeYearTextPara.p_text_font);
            TEXT_SetText(_gTextPVchargeYearHandle, (const char *)_gstElecPVChargeYearTextPara.p_content);
            TEXT_SetTextAlign(_gTextPVchargeYearHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPVchargeYearHandle, _gstElecPVChargeYearTextPara.text_color);
            //pv charge total
            _gTextPVChargeTotalHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_PV_CHARGE_TOTAL_TEXT_ID);
            TEXT_SetFont(_gTextPVChargeTotalHandle, _gstElecPVChargeTotalTextPara.p_text_font);
            TEXT_SetText(_gTextPVChargeTotalHandle, (const char *)_gstElecPVChargeTotalTextPara.p_content);
            TEXT_SetTextAlign(_gTextPVChargeTotalHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPVChargeTotalHandle, _gstElecPVChargeTotalTextPara.text_color);
            //load today
            _gTextLoadTodayHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_LOAD_TODAY_TEXT_ID);
            TEXT_SetFont(_gTextLoadTodayHandle, _gstElecLoadTodayTextParas.p_text_font);
            TEXT_SetText(_gTextLoadTodayHandle, (const char *)_gstElecLoadTodayTextParas.p_content);
            TEXT_SetTextAlign(_gTextLoadTodayHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLoadTodayHandle, _gstElecLoadTodayTextParas.text_color);
            //load Mon
            _gTextLoadMonHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_LOAD_NOV_TEXT_ID);
            TEXT_SetFont(_gTextLoadMonHandle, _gstElecLoadMonTextPara.p_text_font);
            TEXT_SetText(_gTextLoadMonHandle, (const char *)_gstElecLoadMonTextPara.p_content);
            TEXT_SetTextAlign(_gTextLoadMonHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLoadMonHandle, _gstElecLoadMonTextPara.text_color);
            //load year
            _gTextLoadYearHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_LOAD_YEAR_TEXT_ID);
            TEXT_SetFont(_gTextLoadYearHandle, _gstElecLoadYearTextPara.p_text_font);
            TEXT_SetText(_gTextLoadYearHandle, (const char *)_gstElecLoadYearTextPara.p_content);
            TEXT_SetTextAlign(_gTextLoadYearHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLoadYearHandle, _gstElecLoadYearTextPara.text_color);
            //load total
            _gTextLoadTotalHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_INFO_LOAD_TOTAL_TEXT_ID);
            TEXT_SetFont(_gTextLoadTotalHandle, _gstElecLoadTotalTextPara.p_text_font);
            TEXT_SetText(_gTextLoadTotalHandle, (const char *)_gstElecLoadTotalTextPara.p_content);
            TEXT_SetTextAlign(_gTextLoadTotalHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLoadTotalHandle, _gstElecLoadTotalTextPara.text_color);
            break;
        }
        case WM_PAINT:
        {
            GUI_SetColor(0x00424242);//上色
            GUI_FillRect(12, 8, 12 + 456, 8 + 161);

            GUI_SetPenSize(2);
            GUI_SetColor(0x00797979);
            GUI_DrawLine(139, 53, 139, 53 + 102);
            GUI_DrawLine(229, 53, 229, 53 + 102);
            GUI_DrawLine(339, 53, 339, 53 + 102);
#if DEBUG_TEST
            GRID_CHARGE_T grid_charge = {2, 34, 666, 3434};
            GridTied_t ttt = {3, 46, 346, 5454.2};
            PvCharge_t pv = {1.2, 45, 466, 4546.3};
            BackupLoad_t lll = {2, 55, 325.3, 6435.2};

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
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*初始化各个子控件*/
            //时间图标
            _gImgRocaOptTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_MID_INFO_TIME_IMG_ID);
            IMAGE_SetDTA(_gImgRocaOptTimeHandle, _gstRocaOptTimeImgPara.p_img_buf, _gstRocaOptTimeImgPara.img_size);

            //roca opt
            _gTextRocaOPTHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_MID_INFO_ROCA_OPT_TEXT_ID);
            TEXT_SetFont(_gTextRocaOPTHandle, _gstRocaOPTTextPara.p_text_font);
            TEXT_SetText(_gTextRocaOPTHandle, (const char *)_gstRocaOPTTextPara.p_content);
            TEXT_SetTextAlign(_gTextRocaOPTHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextRocaOPTHandle, _gstRocaOPTTextPara.text_color);
            //roca opt time
            _gTextRocaOPTTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_MID_INFO_ROCA_OPT_TIME_TEXT_ID);
            TEXT_SetFont(_gTextRocaOPTTimeHandle, _gstRocaOPTTimeTextPara.p_text_font);
            TEXT_SetText(_gTextRocaOPTTimeHandle, (const char *)_gstRocaOPTTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextRocaOPTTimeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextRocaOPTTimeHandle, _gstRocaOPTTimeTextPara.text_color);
            //on grid
            _gTextOnGirdHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_MID_INFO_ON_GRID_TEXT_ID);
            TEXT_SetFont(_gTextOnGirdHandle, _gstOnGirdTextPara.p_text_font);
            TEXT_SetText(_gTextOnGirdHandle, (const char *)_gstOnGirdTextPara.p_content);
            TEXT_SetTextAlign(_gTextOnGirdHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextOnGirdHandle, _gstOnGirdTextPara.text_color);
            //on grid time
            _gTextOnGirdTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_MID_INFO_ON_GRID_TIME_TEXT_ID);
            TEXT_SetFont(_gTextOnGirdTimeHandle, _gstOnGirdTimeTextPara.p_text_font);
            TEXT_SetText(_gTextOnGirdTimeHandle, (const char *)_gstOnGirdTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextOnGirdTimeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextOnGirdTimeHandle, _gstOnGirdTimeTextPara.text_color);
            break;
        }
        case WM_PAINT:
        {
            GUI_SetColor(0x00424242);//上色
            GUI_FillRect(12, 0, 12 + 456, 62);
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
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*初始化各个子控件*/
            //电池图标
            _gImgBatteryHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_BOTTOM_INFO_BATTERY_IMG_ID);
            IMAGE_SetDTA(_gImgBatteryHandle, _gstBatteryImgPara.p_img_buf, _gstBatteryImgPara.img_size);

            //battery cycles
            _gTextBatCyclesHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_BOTTOM_INFO_BAT_CYC_TEXT_ID);
            TEXT_SetFont(_gTextBatCyclesHandle, _gstBatCyclesTextPara.p_text_font);
            TEXT_SetText(_gTextBatCyclesHandle, (const char *)_gstBatCyclesTextPara.p_content);
            TEXT_SetTextAlign(_gTextBatCyclesHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBatCyclesHandle, _gstBatCyclesTextPara.text_color);
            //battery cycles nums
            _gTextBatCyclesNumsHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_BOTTOM_INFO_BAT_CYC_NUMS_TEXT_ID);
            TEXT_SetFont(_gTextBatCyclesNumsHandle, _gstBatCyclesNumsTextPara.p_text_font);
            TEXT_SetText(_gTextBatCyclesNumsHandle, (const char *)_gstBatCyclesNumsTextPara.p_content);
            TEXT_SetTextAlign(_gTextBatCyclesNumsHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBatCyclesNumsHandle, _gstBatCyclesNumsTextPara.text_color);
            //SOH
            _gTextSOHHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_BOTTOM_INFO_SOH_TEXT_ID);
            TEXT_SetFont(_gTextSOHHandle, _gstSOHTextPara.p_text_font);
            TEXT_SetText(_gTextSOHHandle, (const char *)_gstSOHTextPara.p_content);
            TEXT_SetTextAlign(_gTextSOHHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextSOHHandle, _gstSOHTextPara.text_color);
            //SOH STAT
            _gTextSOHStatHandle = WM_GetDialogItem(pMsg->hWin, WIN_ELEC_BOTTOM_INFO_SOH_STAT_TEXT_ID);
            TEXT_SetFont(_gTextSOHStatHandle, _gstSOHStatTextPara.p_text_font);
            TEXT_SetText(_gTextSOHStatHandle, (const char *)_gstSOHStatTextPara.p_content);
            TEXT_SetTextAlign(_gTextSOHStatHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextSOHStatHandle, _gstSOHStatTextPara.text_color);
            break;
        }
        case WM_PAINT:
        {
            GUI_SetColor(0x00424242);//上色
            GUI_FillRect(12, 0, 12 + 456, 62);
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
            GUI_SetColor(0x00353535);
            GUI_FillRect(0, 0, EM_XSIZE, EM_YSIZE);
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
    _gWinElecInfoHandle = GUI_CreateDialogBox(_garrElecInfoStatDialogCreate,
                          GUI_COUNTOF(_garrElecInfoStatDialogCreate),
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
    _gWinElecMidInfoHandle = GUI_CreateDialogBox(_garrMiddleTextInfoDialogCreate,
                             GUI_COUNTOF(_garrMiddleTextInfoDialogCreate),
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
    _gWinElecBottomInfoHandle = GUI_CreateDialogBox(_garrBottomTextInfoDialogCreate,
                                GUI_COUNTOF(_garrBottomTextInfoDialogCreate),
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
    pPrevEmWin = &gstEmWinHomePage;

    GUI_EnableAlpha(1);
    /*创建一个基于桌面窗口的子窗口*/
    gstEmWinElectricityInfo.win_handle = WM_CreateWindow(0, 0, EM_XSIZE, EM_YSIZE, WM_CF_SHOW, _cbMainWin, 0);
    pCurrentEmWin = &gstEmWinElectricityInfo;
    GUI_Exec();

    _CreateElecInfoStatSubWin();
    _CreateMiddleSubWin();
    _CreateBottomSubWin();

    return gstEmWinElectricityInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinElectricityInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinElectricityInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinElectricityInfo.win_handle = 0;
}

/**
  * @brief  顶部状态栏初始化
  * @param  NULL
  * @retval NULL
  */
static void _InitElecInfoTopStat()
{
    uint32_t pv_generate_month = 0;
    uint32_t pv_generate_year  = 0;
    uint32_t pv_generate_total = 0;

    //买电
    _gstUiCtx.electric_purchase_today = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day;
    _gstUiCtx.electric_purchase_month = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month;
    _gstUiCtx.electric_purchase_year  = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year;
    _gstUiCtx.electric_purchase_total = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_total;
    //卖电
    _gstUiCtx.electric_sale_today = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day;
    _gstUiCtx.electric_sale_month = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month;
    _gstUiCtx.electric_sale_year  = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year;
    _gstUiCtx.electric_sale_total = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_total;
    //负载
    _gstUiCtx.load_elec_cost_today = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day;
    _gstUiCtx.load_elec_cost_month = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month;
    _gstUiCtx.load_elec_cost_year  = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year;
    _gstUiCtx.load_elec_cost_total = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_total;
    //光伏发电
    _gstUiCtx.pv_generate_today = gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day;
    _gstUiCtx.pv_generate_month = gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month;
    _gstUiCtx.pv_generate_year  = gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year;
    _gstUiCtx.pv_generate_total = gstCollectInfo.maincard_data.elec_stats_info.pv_elec_total;

//    memcpy(_gstUiCtx.pv_generate_month, gstCollectParaData.pcs_data.udp_pcs_input_para.pv_sum_generation_on_month, sizeof(_gstUiCtx.pv_generate_month));
//    memcpy(_gstUiCtx.pv_generate_year, gstCollectParaData.pcs_data.udp_pcs_input_para.pv_sum_generation_on_year, sizeof(_gstUiCtx.pv_generate_year));
//    memcpy(_gstUiCtx.pv_generate_total, gstCollectParaData.pcs_data.udp_pcs_input_para.pv_sum_generation_total,   sizeof(_gstUiCtx.pv_generate_total));
//    pv_generate_month = _gstUiCtx.pv_generate_month[0] << 16 | _gstUiCtx.pv_generate_month[1];
//    pv_generate_year  = _gstUiCtx.pv_generate_year[0]  << 16 | _gstUiCtx.pv_generate_year[1];
//    pv_generate_total = _gstUiCtx.pv_generate_total[0] << 16 | _gstUiCtx.pv_generate_total[1];


    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为StatBar区域子控件申请内存*/
    _gstElecGridChargeImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_ICON_GRID_CHARGE_SIZE);
    _gstElecGridChargeImgPara.p_img_buf = GUI_ALLOC_h2p(_gstElecGridChargeImgPara.mem_img);
    _gstElecGridChargeImgPara.img_size = EM_ELEC_INFO_ICON_GRID_CHARGE_SIZE;
    SpiFlashReadData(EM_ICON_GRID_CHARGE_ADDR, _gstElecGridChargeImgPara.p_img_buf, _gstElecGridChargeImgPara.img_size);

    _gstElecGridTiedImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_ICON_GRID_TIED_SIZE);
    _gstElecGridTiedImgPara.p_img_buf = GUI_ALLOC_h2p(_gstElecGridTiedImgPara.mem_img);
    _gstElecGridTiedImgPara.img_size = EM_ELEC_INFO_ICON_GRID_TIED_SIZE;
    SpiFlashReadData(EM_ICON_GRID_TIED_ADDR, _gstElecGridTiedImgPara.p_img_buf, _gstElecGridTiedImgPara.img_size);

    _gstElecPVChargeImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_ICON_PV_CHARGE_SIZE);
    _gstElecPVChargeImgPara.p_img_buf = GUI_ALLOC_h2p(_gstElecPVChargeImgPara.mem_img);
    _gstElecPVChargeImgPara.img_size = EM_ELEC_INFO_ICON_PV_CHARGE_SIZE;
    SpiFlashReadData(EM_ICON_PV_CHARGE_ADDR, _gstElecPVChargeImgPara.p_img_buf, _gstElecPVChargeImgPara.img_size);

    _gstElecLoadImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_ICON_LOAD_SIZE);
    _gstElecLoadImgPara.p_img_buf = GUI_ALLOC_h2p(_gstElecLoadImgPara.mem_img);
    _gstElecLoadImgPara.img_size = EM_ELEC_INFO_ICON_LOAD_SIZE;
    SpiFlashReadData(EN_ICON_BACKUP_LOAD_ADDR, _gstElecLoadImgPara.p_img_buf, _gstElecLoadImgPara.img_size);

    _gstKWhTextPara.p_content = _garrkWhTextBuf;
    _gstKWhTextPara.text_color = 0x00B2BE00;
    _gstKWhTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstTodayTextPara.p_content = _garrTodayTextBuf;
    _gstTodayTextPara.text_color = 0x00FFFFFF;
    _gstTodayTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstMonTextPara.p_content = _garrMonTextBuf;
    _gstMonTextPara.text_color = 0x00FFFFFF;
    _gstMonTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstYearTextPara.p_content = _garrYearTextBuf;
    _gstYearTextPara.text_color = 0x00FFFFFF;
    _gstYearTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstTotalTextPara.p_content = _garrTotalTextBuf;
    _gstTotalTextPara.text_color = 0x00FFFFFF;
    _gstTotalTextPara.p_text_font = EM_TEXT_FONT_7;

    /*GRID CHARGE TODAY*/
    _gstElecGridChargeTodayTextPara.p_content = _garrGridChargeToadyTextBuf;
    _gstElecGridChargeTodayTextPara.text_color = 0x00FFFFFF;
    _gstElecGridChargeTodayTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecGridChargeTodayTextPara.digit_value = 3.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecGridChargeTodayTextPara.p_content, "%.2f", _gstElecGridChargeTodayTextPara.digit_value);
//    snprintf(_garrGridChargeToadyTextBuf, sizeof(_garrGridChargeToadyTextBuf), "%.2f", (float)_gstUiCtx.electric_purchase_today);

    /*GRID CHARGE NOV*/
    _gstElecGridChargeMonTextPara.p_content = _garrGridChargeMonTextBuf;
    _gstElecGridChargeMonTextPara.text_color = 0x00FFFFFF;
    _gstElecGridChargeMonTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecGridChargeMonTextPara.digit_value = 30.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecGridChargeMonTextPara.p_content, "%.2f", _gstElecGridChargeMonTextPara.digit_value);
//    snprintf(_garrGridChargeMonTextBuf, sizeof(_garrGridChargeMonTextBuf), "%.2f", (float)_gstUiCtx.electric_purchase_month);


    /*GRID CHARGE YEAR*/
    _gstElecGridChargeYearTextPara.p_content = _garrGridChargeYearTextBuf;
    _gstElecGridChargeYearTextPara.text_color = 0x00FFFFFF;
    _gstElecGridChargeYearTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecGridChargeYearTextPara.digit_value = 300.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecGridChargeYearTextPara.p_content, "%.2f", _gstElecGridChargeYearTextPara.digit_value);
//    snprintf(_garrGridChargeYearTextBuf, sizeof(_garrGridChargeYearTextBuf), "%.2f", (float)_gstUiCtx.electric_purchase_year);
    /*GRID CHARGE TOTAL*/
    _gstElecGridChargeTotalTextPara.p_content = _garrGridChargeTotalTextBuf;
    _gstElecGridChargeTotalTextPara.text_color = 0x00FFFFFF;
    _gstElecGridChargeTotalTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecGridChargeTotalTextPara.digit_value = 3000.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecGridChargeTotalTextPara.p_content, "%.2f", _gstElecGridChargeTotalTextPara.digit_value);
//    snprintf(_garrGridChargeTotalTextBuf, sizeof(_garrGridChargeTotalTextBuf), "%.2f", (float)_gstUiCtx.electric_purchase_total);
    /*GRID TIED TODAY*/
    _gstElecGridTiedTodayTextPara.p_content = _garrGridTiedTodayTextBuf;
    _gstElecGridTiedTodayTextPara.text_color = 0x00FFFFFF;
    _gstElecGridTiedTodayTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecGridTiedTodayTextPara.digit_value = 4.2;//测试数据，实际赋值系统数据
//    sprintf(_gstElecGridTiedTodayTextPara.p_content, "%.2f", _gstElecGridTiedTodayTextPara.digit_value);
//    snprintf(_garrGridTiedTodayTextBuf, sizeof(_garrGridTiedTodayTextBuf), "%.2f", (float)_gstUiCtx.electric_sale_today);
    /*GRID TIED NOV*/
    _gstElecGridTiedMonTextPara.p_content = _garrGridTiedMonTextBuf;
    _gstElecGridTiedMonTextPara.text_color = 0x00FFFFFF;
    _gstElecGridTiedMonTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecGridTiedMonTextPara.digit_value = 30.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecGridTiedMonTextPara.p_content, "%.2f", _gstElecGridTiedMonTextPara.digit_value);
//    snprintf(_garrGridTiedMonTextBuf, sizeof(_garrGridTiedMonTextBuf), "%.2f", (float)_gstUiCtx.electric_sale_month);
    /*GRID TIED YEAR*/
    _gstElecGridTiedYearTextPara.p_content = _garrGridTiedYearTextBuf;
    _gstElecGridTiedYearTextPara.text_color = 0x00FFFFFF;
    _gstElecGridTiedYearTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecGridTiedYearTextPara.digit_value = 300.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecGridTiedYearTextPara.p_content, "%.2f", _gstElecGridTiedYearTextPara.digit_value);
//    snprintf(_garrGridTiedYearTextBuf, sizeof(_garrGridTiedYearTextBuf), "%.2f", (float)_gstUiCtx.electric_sale_year);
    /*GRID TIED TOTAL*/
    _gstElecGridTiedTotalTextPara.p_content = _garrGridTiedTotalTextBuf;
    _gstElecGridTiedTotalTextPara.text_color = 0x00FFFFFF;
    _gstElecGridTiedTotalTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecGridTiedTotalTextPara.digit_value = 3000.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecGridTiedTotalTextPara.p_content, "%.2f", _gstElecGridTiedTotalTextPara.digit_value);
//    snprintf(_garrGridTiedTotalTextBuf, sizeof(_garrGridTiedTotalTextBuf), "%.2f", (float)_gstUiCtx.electric_sale_total);

    /*PV CHARGE TODAY*/
    _gstElecPVChargeTodayTextPara.p_content = _garrPVchargeTodayTextBuf;
    _gstElecPVChargeTodayTextPara.text_color = 0x00FFFFFF;
    _gstElecPVChargeTodayTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecPVChargeTodayTextPara.digit_value = 3.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecPVChargeTodayTextPara.p_content, "%.2f", _gstElecPVChargeTodayTextPara.digit_value);
//    snprintf(_garrPVchargeTodayTextBuf, sizeof(_garrPVchargeTodayTextBuf), "%.2f", (float)_gstUiCtx.pv_generate_today);
    /*PV CHARGE NOV*/
    _gstElecPVChargeMonTextPara.p_content = _garrPVchargeMonTextBuf;
    _gstElecPVChargeMonTextPara.text_color = 0x00FFFFFF;
    _gstElecPVChargeMonTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecPVChargeMonTextPara.digit_value = 30.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecPVChargeMonTextPara.p_content, "%.2f", _gstElecPVChargeMonTextPara.digit_value);
//    snprintf(_garrPVchargeMonTextBuf, sizeof(_garrPVchargeMonTextBuf), "%.2f", (float)pv_generate_month);
    /*PV CHARGE YEAR*/
    _gstElecPVChargeYearTextPara.p_content = _garrPVchargeYearTextBuf;
    _gstElecPVChargeYearTextPara.text_color = 0x00FFFFFF;
    _gstElecPVChargeYearTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecPVChargeYearTextPara.digit_value = 300.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecPVChargeYearTextPara.p_content, "%.2f", _gstElecPVChargeYearTextPara.digit_value);
//    snprintf(_garrPVchargeYearTextBuf, sizeof(_garrPVchargeYearTextBuf), "%.2f", (float)pv_generate_year);
    /*PV CHARGE TOTAL*/
    _gstElecPVChargeTotalTextPara.p_content = _garrPVchargeTotalTextBuf;
    _gstElecPVChargeTotalTextPara.text_color = 0x00FFFFFF;
    _gstElecPVChargeTotalTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecPVChargeTotalTextPara.digit_value = 3000.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecPVChargeTotalTextPara.p_content, "%.2f", _gstElecPVChargeTotalTextPara.digit_value);
//    snprintf(_garrPVchargeTotalTextBuf, sizeof(_garrPVchargeTotalTextBuf), "%.2f", (float)pv_generate_total);
    /*LOAD TODAY*/
    _gstElecLoadTodayTextParas.p_content = _garrLoadTodayTextBuf;
    _gstElecLoadTodayTextParas.text_color = 0x00FFFFFF;
    _gstElecLoadTodayTextParas.p_text_font = EM_TEXT_FONT_7;
    _gstElecLoadTodayTextParas.digit_value = 3.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecLoadTodayTextParas.p_content, "%.2f", _gstElecLoadTodayTextParas.digit_value);
//    snprintf(_garrLoadTodayTextBuf, sizeof(_garrLoadTodayTextBuf), "%.2f", (float)_gstUiCtx.load_elec_cost_today);
    /*LOAD MON*/
    _gstElecLoadMonTextPara.p_content = _garrLoadMonTextBuf;
    _gstElecLoadMonTextPara.text_color = 0x00FFFFFF;
    _gstElecLoadMonTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecLoadMonTextPara.digit_value = 30.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecLoadMonTextPara.p_content, "%.2f", _gstElecLoadMonTextPara.digit_value);
//    snprintf(_garrLoadMonTextBuf, sizeof(_garrLoadMonTextBuf), "%.2f", (float)_gstUiCtx.load_elec_cost_month);
    /*LOAD YEAR*/
    _gstElecLoadYearTextPara.p_content = _garrLoadYearTextBuf;
    _gstElecLoadYearTextPara.text_color = 0x00FFFFFF;
    _gstElecLoadYearTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecLoadYearTextPara.digit_value = 300.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecLoadYearTextPara.p_content, "%.2f", _gstElecLoadYearTextPara.digit_value);
//    snprintf(_garrLoadYearTextBuf, sizeof(_garrLoadYearTextBuf), "%.2f", (float)_gstUiCtx.load_elec_cost_year);
    /*LOAD TOTAL*/
    _gstElecLoadTotalTextPara.p_content = _garrLoadTotalTextBuf;
    _gstElecLoadTotalTextPara.text_color = 0x00FFFFFF;
    _gstElecLoadTotalTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstElecLoadTotalTextPara.digit_value = 3000.25;//测试数据，实际赋值系统数据
//    sprintf(_gstElecLoadTotalTextPara.p_content, "%.2f", _gstElecLoadTotalTextPara.digit_value);
//    snprintf(_garrLoadTotalTextBuf, sizeof(_garrLoadTotalTextBuf), "%.2f", (float)_gstUiCtx.load_elec_cost_total);

    _SetElectricPurchase(0, _gstUiCtx.electric_purchase_today, _gstUiCtx.electric_purchase_month, _gstUiCtx.electric_purchase_year, _gstUiCtx.electric_purchase_total);
    _SetElectricSale(0, _gstUiCtx.electric_sale_today, _gstUiCtx.electric_sale_month, _gstUiCtx.electric_sale_year, _gstUiCtx.electric_sale_total);
    _SetLoadElectric(0, _gstUiCtx.load_elec_cost_today, _gstUiCtx.load_elec_cost_month, _gstUiCtx.load_elec_cost_year, _gstUiCtx.load_elec_cost_total);
    _SetPVElectric(0, _gstUiCtx.pv_generate_today, _gstUiCtx.pv_generate_month, _gstUiCtx.pv_generate_year, _gstUiCtx.pv_generate_total);
}


/**
  * @brief  中间子窗口初始化
  * @param  NULL
  * @retval NULL
  */
static void _InitElecInfoMid()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    _gstUiCtx.roca_opt_time = gstCollectInfo.maincard_data.sys_info.run_time;
    _gstUiCtx.on_grid_time  = gstCollectInfo.maincard_data.sys_info.on_grid_time; //GetOnGridTime();

    _gstRocaOptTimeImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_MID_TIME_ICON_SIZE);
    _gstRocaOptTimeImgPara.p_img_buf = GUI_ALLOC_h2p(_gstRocaOptTimeImgPara.mem_img);
    _gstRocaOptTimeImgPara.img_size = EM_ELEC_INFO_MID_TIME_ICON_SIZE;
    SpiFlashReadData(EM_ICON_ELEC_TIME_ADDR, _gstRocaOptTimeImgPara.p_img_buf, _gstRocaOptTimeImgPara.img_size);

    _gstRocaOPTTextPara.p_content = _garrRocaOPTTextBuf;
    _gstRocaOPTTextPara.text_color = 0x00FFFFFF;
    _gstRocaOPTTextPara.p_text_font = EM_TEXT_FONT_7;
    snprintf(_garrRocaOPTTextBuf, sizeof(_garrRocaOPTTextBuf), "%s", "Roca OPT Time");

    _gstRocaOPTTimeTextPara.p_content =   _garrRocaOPTTimeTextBuf;
    _gstRocaOPTTimeTextPara.text_color = 0x00B2BE00;
    _gstRocaOPTTimeTextPara.p_text_font = EM_TEXT_FONT_7;
//    _gstRocaOPTTimeTextPara.digit_value = 300;//测试数据，实际赋值系统数据
//    sprintf(_gstRocaOPTTimeTextPara.p_content, "%dh", (uint16_t)_gstRocaOPTTimeTextPara.digit_value);

    _gstOnGirdTextPara.p_content = _garrOnGirdTextBuf;
    _gstOnGirdTextPara.text_color = 0x00FFFFFF;
    _gstOnGirdTextPara.p_text_font = EM_TEXT_FONT_7;
    snprintf(_garrOnGirdTextBuf, sizeof(_garrOnGirdTextBuf), "%s", "On Grid Time");

    _gstOnGirdTimeTextPara.p_content =    _garrOnGirdTimeTextBuf;
    _gstOnGirdTimeTextPara.text_color = 0x00B2BE00;
    _gstOnGirdTimeTextPara.p_text_font = EM_TEXT_FONT_7;
//    _gstOnGirdTimeTextPara.digit_value = 1000;//测试数据，实际赋值系统数据
//    sprintf(_gstOnGirdTimeTextPara.p_content, "%dh", (uint16_t)_gstOnGirdTimeTextPara.digit_value);

    _SetRocaOPTTime(0, _gstUiCtx.roca_opt_time);
    _SetOnGridTime(0, _gstUiCtx.on_grid_time);
}


/**
  * @brief  底部子窗口初始化
  * @param  NULL
  * @retval NULL
  */
static void _InitElecInfoBottom()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    //soh
    _gstUiCtx.soh = gstCollectInfo.maincard_data.total_pack_info.soh;
    _gstUiCtx.grid_conn_nums = gstCollectInfo.maincard_data.sys_info.on_grid_num;

    _gstBatteryImgPara.mem_img = GUI_ALLOC_AllocZero(EM_ELEC_INFO_BOTTOM_BATTERY_ICON_SIZE);
    _gstBatteryImgPara.p_img_buf = GUI_ALLOC_h2p(_gstBatteryImgPara.mem_img);
    _gstBatteryImgPara.img_size = EM_ELEC_INFO_BOTTOM_BATTERY_ICON_SIZE;
    SpiFlashReadData(EM_ICON_ELEC_BATTERY_ADDR, _gstBatteryImgPara.p_img_buf, _gstBatteryImgPara.img_size);

    _gstBatCyclesTextPara.p_content = _garrBatCyclesTextBuf;
    _gstBatCyclesTextPara.text_color = 0x00FFFFFF;
    _gstBatCyclesTextPara.p_text_font = EM_TEXT_FONT_7;
    snprintf(_garrBatCyclesTextBuf, sizeof(_garrBatCyclesTextBuf), "%s", "Grid Connect Number");

    _gstBatCyclesNumsTextPara.p_content = _garrBatCyclesNumsTextBuf;
    _gstBatCyclesNumsTextPara.text_color = 0x00B2BE00;
    _gstBatCyclesNumsTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstBatCyclesNumsTextPara.digit_value = 23;//测试数据，实际赋值系统数据
//    sprintf(_gstBatCyclesNumsTextPara.p_content, "%d", (uint16_t)_gstBatCyclesNumsTextPara.digit_value);
//    snprintf(_garrBatCyclesNumsTextBuf, sizeof(_garrBatCyclesNumsTextBuf), "%d", _gstUiCtx.grid_conn_nums);

    _gstSOHTextPara.p_content = _garrSOHTextBuf;
    _gstSOHTextPara.text_color = 0x00FFFFFF;
    _gstSOHTextPara.p_text_font = EM_TEXT_FONT_7;
    snprintf(_garrSOHTextBuf, sizeof(_garrSOHTextBuf), "%s", "State of health");

    _gstSOHStatTextPara.p_content = _garrSOHStatTextBuf;
    _gstSOHStatTextPara.text_color = 0x00B2BE00;
    _gstSOHStatTextPara.p_text_font = EM_TEXT_FONT_7;
    _gstSOHStatTextPara.digit_value = 100;//测试数据，实际赋值系统数据
//    sprintf(_gstSOHStatTextPara.p_content, "%d%%", (uint16_t)_gstSOHStatTextPara.digit_value);
//    snprintf(_garrSOHStatTextBuf, sizeof(_garrSOHStatTextBuf), "%.1f%%", (float)_gstUiCtx.soh);

    _SetSOHValue(0, _gstUiCtx.soh);
    _SetPackCylceNum(0, _gstUiCtx.grid_conn_nums);
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    _InitElecInfoTopStat();
    _InitElecInfoMid();
    _InitElecInfoBottom();
}


static void _DelElecInfoTopStat()
{
    GUI_ALLOC_Free(_gstElecGridChargeImgPara.mem_img);
    GUI_ALLOC_Free(_gstElecGridTiedImgPara.mem_img);
    GUI_ALLOC_Free(_gstElecPVChargeImgPara.mem_img);
    GUI_ALLOC_Free(_gstElecLoadImgPara.mem_img);
}

static void _DelMiddle()
{
    GUI_ALLOC_Free(_gstRocaOptTimeImgPara.mem_img);
}

static void _DelBottom()
{
    GUI_ALLOC_Free(_gstBatteryImgPara.mem_img);
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
    uint32_t on_grid_time = 0;
    uint32_t grid_conn_nums = 0;
    grid_conn_nums = gstCollectInfo.maincard_data.sys_info.on_grid_num;
    on_grid_time = gstCollectInfo.maincard_data.sys_info.on_grid_time;//GetOnGridTime();
    //买电
    if(_gstUiCtx.electric_purchase_today != gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day   ||
       _gstUiCtx.electric_purchase_month != gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month ||
       _gstUiCtx.electric_purchase_year  != gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year  ||
       _gstUiCtx.electric_purchase_total != gstCollectInfo.maincard_data.elec_stats_info.buy_elec_total  )
    {
        _gstUiCtx.electric_purchase_today = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day;
        _gstUiCtx.electric_purchase_month = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month;
        _gstUiCtx.electric_purchase_year  = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year;
        _gstUiCtx.electric_purchase_total = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_total;

        _SetElectricPurchase(1, _gstUiCtx.electric_purchase_today, _gstUiCtx.electric_purchase_month,
                             _gstUiCtx.electric_purchase_year, _gstUiCtx.electric_purchase_total);
    }
    //卖电
    if(_gstUiCtx.electric_sale_today != gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day   ||
       _gstUiCtx.electric_sale_month != gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month ||
       _gstUiCtx.electric_sale_year  != gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year  ||
       _gstUiCtx.electric_sale_total != gstCollectInfo.maincard_data.elec_stats_info.sell_elec_total )
    {
        _gstUiCtx.electric_sale_today = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day;
        _gstUiCtx.electric_sale_month = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month;
        _gstUiCtx.electric_sale_year  = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year;
        _gstUiCtx.electric_sale_total = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_total;

        _SetElectricSale(1, _gstUiCtx.electric_sale_today, _gstUiCtx.electric_sale_month,
                         _gstUiCtx.electric_sale_year, _gstUiCtx.electric_sale_total);
    }
    //负载用电
    if(_gstUiCtx.load_elec_cost_today != gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day   ||
       _gstUiCtx.load_elec_cost_month != gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month ||
       _gstUiCtx.load_elec_cost_year  != gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year  ||
       _gstUiCtx.load_elec_cost_total != gstCollectInfo.maincard_data.elec_stats_info.eps_elec_total )
    {                                    
        _gstUiCtx.load_elec_cost_today = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day;
        _gstUiCtx.load_elec_cost_month = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month;
        _gstUiCtx.load_elec_cost_year  = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year;
        _gstUiCtx.load_elec_cost_total = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_total;

        _SetLoadElectric(1, _gstUiCtx.load_elec_cost_today, _gstUiCtx.load_elec_cost_month,
                         _gstUiCtx.load_elec_cost_year, _gstUiCtx.load_elec_cost_total);
    }
    //pv发电
    if(_gstUiCtx.pv_generate_today != gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day      ||
       _gstUiCtx.pv_generate_month != gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month    ||
       _gstUiCtx.pv_generate_year  != gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year     ||
       _gstUiCtx.pv_generate_total != gstCollectInfo.maincard_data.elec_stats_info.pv_elec_total)
    {
        _gstUiCtx.pv_generate_today = gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day;
        _gstUiCtx.pv_generate_month = gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month;
        _gstUiCtx.pv_generate_year  = gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year;
        _gstUiCtx.pv_generate_total = gstCollectInfo.maincard_data.elec_stats_info.pv_elec_total;

        _SetPVElectric(1, _gstUiCtx.pv_generate_today, _gstUiCtx.pv_generate_month, _gstUiCtx.pv_generate_year, _gstUiCtx.pv_generate_total);
    }
    //soh
    if(_gstUiCtx.soh != gstCollectInfo.maincard_data.total_pack_info.soh)
    {
        _gstUiCtx.soh = gstCollectInfo.maincard_data.total_pack_info.soh;
        _SetSOHValue(1, _gstUiCtx.soh);
    }
    //pack cycle
    if(_gstUiCtx.grid_conn_nums != grid_conn_nums)
    {
        _gstUiCtx.grid_conn_nums = grid_conn_nums;
        _SetPackCylceNum(1, _gstUiCtx.grid_conn_nums);
    }
    // opt time
    if(_gstUiCtx.roca_opt_time != gstCollectInfo.maincard_data.sys_info.run_time)
    {
        _gstUiCtx.roca_opt_time = gstCollectInfo.maincard_data.sys_info.run_time;
        _SetRocaOPTTime(1, _gstUiCtx.roca_opt_time);
    }
    //on gird time
    if(_gstUiCtx.on_grid_time != on_grid_time)
    {
        _gstUiCtx.on_grid_time = on_grid_time;
        _SetOnGridTime(1, _gstUiCtx.on_grid_time);
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
    event = EmGetKeyEvent(p_msg);
    if(event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }

    if(event == EN_KEY_SRC_LEFT)
    {
        if(pCurrentEmWin->win_handle != 0)
        {
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = &gstEmWinHomePage;
            pCurrentEmWin->create_win();
        }
        return;
    }

    if(event == EN_KEY_SRC_ESC)
    {
        if(pCurrentEmWin->win_handle != 0)
        {
            emWin_t * pnext = NULL;
            pnext = &gstEmWinHomePage;
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}

//电网发电统计
int32_t SetGridChargeInfo(GridCharge_t st_grid_charge)
{
    //today
    _gstElecGridChargeTodayTextPara.digit_value = st_grid_charge.elec_today;
    sprintf(_gstElecGridChargeTodayTextPara.p_content, "%.2f", _gstElecGridChargeTodayTextPara.digit_value);
    TEXT_SetText(_gTextGridChargeToadyHandle, (const char *)_gstElecGridChargeTodayTextPara.p_content);
    //Mon
    _gstElecGridChargeMonTextPara.digit_value = st_grid_charge.elec_mon;
    sprintf(_gstElecGridChargeMonTextPara.p_content, "%.2f", _gstElecGridChargeMonTextPara.digit_value);
    TEXT_SetText(_gTextGridChargeMonHandle, (const char *)_gstElecGridChargeMonTextPara.p_content);
    //year
    _gstElecGridChargeYearTextPara.digit_value = st_grid_charge.elec_year;
    sprintf(_gstElecGridChargeYearTextPara.p_content, "%.2f", _gstElecGridChargeYearTextPara.digit_value);
    TEXT_SetText(_gTextGridChargeYearHandle, (const char *)_gstElecGridChargeYearTextPara.p_content);
    //total
    _gstElecGridChargeTotalTextPara.digit_value = st_grid_charge.elec_total;
    sprintf(_gstElecGridChargeTotalTextPara.p_content, "%.2f", _gstElecGridChargeTotalTextPara.digit_value);
    TEXT_SetText(_gTextGridChargeTotalHandle, (const char *)_gstElecGridChargeTotalTextPara.p_content);

    return 0;
}
//向电网馈电统计
int32_t SetGridTiedInfo(GridTied_t st_grid_charge)
{
    //today
    _gstElecGridTiedTodayTextPara.digit_value = st_grid_charge.elec_today;
    sprintf(_gstElecGridTiedTodayTextPara.p_content, "%.2f", _gstElecGridTiedTodayTextPara.digit_value);
    TEXT_SetText(_gTextGridTiedTodayHandle, (const char *)_gstElecGridTiedTodayTextPara.p_content);
    //Mon
    _gstElecGridTiedMonTextPara.digit_value = st_grid_charge.elec_mon;
    sprintf(_gstElecGridTiedMonTextPara.p_content, "%.2f", _gstElecGridTiedMonTextPara.digit_value);
    TEXT_SetText(_gTextGridTiedMonHandle, (const char *)_gstElecGridTiedMonTextPara.p_content);
    //year
    _gstElecGridTiedYearTextPara.digit_value = st_grid_charge.elec_year;
    sprintf(_gstElecGridTiedYearTextPara.p_content, "%.2f", _gstElecGridTiedYearTextPara.digit_value);
    TEXT_SetText(_gTextGridTiedYearHandle, (const char *)_gstElecGridTiedYearTextPara.p_content);
    //total
    _gstElecGridTiedTotalTextPara.digit_value = st_grid_charge.elec_total;
    sprintf(_gstElecGridTiedTotalTextPara.p_content, "%.2f", _gstElecGridTiedTotalTextPara.digit_value);
    TEXT_SetText(_gTextGridTiedTotalHandle, (const char *)_gstElecGridTiedTotalTextPara.p_content);

    return 0;
}
//太阳能充电统计
int32_t SetPVChargeInfo(PvCharge_t st_pv_charge)
{
    //today
    _gstElecPVChargeTodayTextPara.digit_value = st_pv_charge.elec_today;
    sprintf(_gstElecPVChargeTodayTextPara.p_content, "%.2f", _gstElecPVChargeTodayTextPara.digit_value);
    TEXT_SetText(_gTextPVchargeTodayHandle, (const char *)_gstElecPVChargeTodayTextPara.p_content);
    //Mon
    _gstElecPVChargeMonTextPara.digit_value = st_pv_charge.elec_mon;
    sprintf(_gstElecPVChargeMonTextPara.p_content, "%.2f", _gstElecPVChargeMonTextPara.digit_value);
    TEXT_SetText(_gTextPVchargeMonHandle, (const char *)_gstElecPVChargeMonTextPara.p_content);
    //year
    _gstElecPVChargeYearTextPara.digit_value = st_pv_charge.elec_year;
    sprintf(_gstElecPVChargeYearTextPara.p_content, "%.2f", _gstElecPVChargeYearTextPara.digit_value);
    TEXT_SetText(_gTextPVchargeYearHandle, (const char *)_gstElecPVChargeYearTextPara.p_content);
    //total
    _gstElecPVChargeTotalTextPara.digit_value = st_pv_charge.elec_total;
    sprintf(_gstElecPVChargeTotalTextPara.p_content, "%.2f", _gstElecPVChargeTotalTextPara.digit_value);
    TEXT_SetText(_gTextPVChargeTotalHandle, (const char *)_gstElecPVChargeTotalTextPara.p_content);

    return 0;
}
//负载用电统计
int32_t SetBackupLoadInfo(BackupLoad_t st_backup_load)
{
    //today
    _gstElecLoadTodayTextParas.digit_value = st_backup_load.elec_today;
    sprintf(_gstElecLoadTodayTextParas.p_content, "%.2f", _gstElecLoadTodayTextParas.digit_value);
    TEXT_SetText(_gTextLoadTodayHandle, (const char *)_gstElecLoadTodayTextParas.p_content);
    //Mon
    _gstElecLoadMonTextPara.digit_value = st_backup_load.elec_mon;
    sprintf(_gstElecLoadMonTextPara.p_content, "%.2f", _gstElecLoadMonTextPara.digit_value);
    TEXT_SetText(_gTextLoadMonHandle, (const char *)_gstElecLoadMonTextPara.p_content);
    //year
    _gstElecLoadYearTextPara.digit_value = st_backup_load.elec_year;
    sprintf(_gstElecLoadYearTextPara.p_content, "%.2f", _gstElecLoadYearTextPara.digit_value);
    TEXT_SetText(_gTextLoadYearHandle, (const char *)_gstElecLoadYearTextPara.p_content);
    //total
    _gstElecLoadTotalTextPara.digit_value = st_backup_load.elec_total;
    sprintf(_gstElecLoadTotalTextPara.p_content, "%.2f", _gstElecLoadTotalTextPara.digit_value);
    TEXT_SetText(_gTextLoadTotalHandle, (const char *)_gstElecLoadTotalTextPara.p_content);

    return 0;
}
//设置Roca OPT Time
int32_t SetRocaOPTTime(int32_t c_time)
{
    _gstRocaOPTTimeTextPara.digit_value = c_time;
    sprintf(_gstRocaOPTTimeTextPara.p_content, "%d", (uint16_t)_gstRocaOPTTimeTextPara.digit_value);
    TEXT_SetText(_gTextRocaOPTTimeHandle, (const char *)_gstRocaOPTTimeTextPara.p_content);
    return 0;
}
//设置On Grid Time
int32_t SetOnGridTime(int32_t c_time)
{
    _gstOnGirdTimeTextPara.digit_value = c_time;
    sprintf(_gstOnGirdTimeTextPara.p_content, "%d", (uint16_t)_gstOnGirdTimeTextPara.digit_value);
    TEXT_SetText(_gTextOnGirdTimeHandle, (const char *)_gstOnGirdTimeTextPara.p_content);
    return 0;
}
//设置电池循环次数
int32_t SetBatteryCycNum(int32_t c_nums)
{
    _gstBatCyclesNumsTextPara.digit_value = c_nums;
    sprintf(_gstBatCyclesNumsTextPara.p_content, "%d", (uint16_t)_gstBatCyclesNumsTextPara.digit_value);
    TEXT_SetText(_gTextBatCyclesNumsHandle, (const char *)_gstBatCyclesNumsTextPara.p_content);
    return 0;
}
//设置SOH
int32_t SetSOHState(int32_t c_precentage)
{
    _gstSOHStatTextPara.digit_value = c_precentage;
    sprintf(_gstSOHStatTextPara.p_content, "%d", (uint16_t)_gstSOHStatTextPara.digit_value);
    TEXT_SetText(_gTextSOHStatHandle, (const char *)_gstSOHStatTextPara.p_content);
    return 0;
}

