
#ifndef __EM_MENU_H_
#define __EM_MENU_H_

#include "config.h"
#include "drv_opt.h"
#include "GUI.h"
#include "em_lib.h"
#include "dev_key.h"
#include "dev_spiflash.h"
#include "systick.h"
#include "slave_collect.h"
#include <time.h>

#define  EM_XSIZE       480
#define  EM_YSIZE       320

#define  ROCA_5KW       0
#define  SINGLE_PHASE   1

#define BigLittleSwap32(A)  ((((uint32_t)(A) & 0xFF000000) >> 24) | (((uint32_t)(A) & 0x00FF0000) >> 8) | \
                             (((uint32_t)(A) & 0x0000FF00) << 8) | (((uint32_t)(A) & 0x000000FF) << 24))

#define BigLittleSwap16(A)  ((((uint16_t)(A) & 0xFF00) >> 8) | ((uint16_t)(A) & 0x00FF) << 8)

#define SHUT_DOWN_TIMEOUT_MAX (30 * 1000)

typedef struct _shut_sown_stru_t
{
    uint32_t shut_down_time_out;
    uint16_t shut_down_status;	    
    uint16_t shut_down_flag[2];
    uint16_t shut_reset_time_flag;	
} ShutDownInfo_t;

extern ShutDownInfo_t gShutDownPara;
/*图标FLASH地址划分*/

#define  EM_ICON_BASE_FLASH_ADDR                    (4096 * 4096)
#define  EM_ICON_START_ADDR                         (EM_ICON_BASE_FLASH_ADDR)
#define  EM_ICON_BACKGROUND_ADDR                    (EM_ICON_BASE_FLASH_ADDR + 113 * 4096)
#define  EM_ICON_ROCA_ADDR                          (EM_ICON_BASE_FLASH_ADDR + 226 * 4096)
#define  EM_ICON_GRID_CHARGE_ADDR                   (EM_ICON_BASE_FLASH_ADDR + 404 * 4096)
#define  EM_ICON_GRID_TIED_ADDR                     (EM_ICON_BASE_FLASH_ADDR + 405 * 4096)
#define  EM_ICON_PV_CHARGE_ADDR                     (EM_ICON_BASE_FLASH_ADDR + 406 * 4096)
#define  EN_ICON_BACKUP_LOAD_ADDR                   (EM_ICON_BASE_FLASH_ADDR + 407 * 4096)
#define  EM_ICON_GRID_STATUS_ACTIVE_ADDR            (EM_ICON_BASE_FLASH_ADDR + 408 * 4096)
#define  EM_ICON_GRID_STATUS_DISABLED_ADDR          (EM_ICON_BASE_FLASH_ADDR + 409 * 4096)
#define  EM_ICON_PV_STATUS_ACTIVE_ADDR              (EM_ICON_BASE_FLASH_ADDR + 410 * 4096)
#define  EM_ICON_PV_STATUS_DISABLED_ADDR            (EM_ICON_BASE_FLASH_ADDR + 411 * 4096)
#define  EM_ICON_BACKUP_LOAD_STATUS_ACTIVE_ADDR     (EM_ICON_BASE_FLASH_ADDR + 412 * 4096)
#define  EM_ICON_BACKUP_LOAD_STATUS_DISABLED_ADDR   (EM_ICON_BASE_FLASH_ADDR + 413 * 4096)
#define  EM_ICON_GRID_MODE_CHARGING_ADDR            (EM_ICON_BASE_FLASH_ADDR + 414 * 4096)
#define  EM_ICON_PV_MODE_CHARGING_ADDR              (EM_ICON_BASE_FLASH_ADDR + 415 * 4096)
#define  EM_ICON_GRID_PV_MODE_CHARGING_ADDR         (EM_ICON_BASE_FLASH_ADDR + 416 * 4096)
#define  EM_ICON_TEMP_COLD_ADDR                     (EM_ICON_BASE_FLASH_ADDR + 417 * 4096)
#define  EM_ICON_TEMP_NORMAL_ADDR                   (EM_ICON_BASE_FLASH_ADDR + 418 * 4096)
#define  EM_ICON_TEMP_HEAT_ADDR                     (EM_ICON_BASE_FLASH_ADDR + 419 * 4096)
#define  EM_ICON_WIFI_OFF_ADDR                      (EM_ICON_BASE_FLASH_ADDR + 420 * 4096)
#define  EM_ICON_WIFI_ON_ADDR                       (EM_ICON_BASE_FLASH_ADDR + 421 * 4096)
#define  EM_ICON_RIGHT_ARROW_ADDR                   (EM_ICON_BASE_FLASH_ADDR + 422 * 4096)
#define  EM_ICON_BACK_ON_ADDR                       (EM_ICON_BASE_FLASH_ADDR + 423 * 4096)
#define  EM_ICON_ALERT_ADDR                         (EM_ICON_BASE_FLASH_ADDR + 424 * 4096)
#define  EM_ICON_INDICATE_ACTIVE_ADDR               (EM_ICON_BASE_FLASH_ADDR + 425 * 4096)
#define  EM_ICON_INDICATE_DISABLED_ADDR             (EM_ICON_BASE_FLASH_ADDR + 426 * 4096)
#define  EM_ICON_WIFI_NOT_FOUND                     (EM_ICON_BASE_FLASH_ADDR + 427 * 4096)
#define  EM_ICON_WIFI_DISCONN_ROUTER_ADDR           (EM_ICON_BASE_FLASH_ADDR + 428 * 4096)
#define  EM_ICON_WIFI_DISCONN_CLOUD_ADDR            (EM_ICON_BASE_FLASH_ADDR + 429 * 4096)
#define  EM_ICON_WIFI_CONN_CLOUD_ADDR               (EM_ICON_BASE_FLASH_ADDR + 430 * 4096)

#define  EM_ICON_GERMAN_FLAG_ADDR                   (EM_ICON_BASE_FLASH_ADDR + 454 * 4096)
#define  EM_ICON_CHINESE_FLAG_ADDR                  (EM_ICON_BASE_FLASH_ADDR + 455 * 4096)
#define  EM_ICON_AUSTRALIA_FLAG_ADDR                (EM_ICON_BASE_FLASH_ADDR + 456 * 4096)
#define  EM_ICON_ITALY_FLAG_ADDR                    (EM_ICON_BASE_FLASH_ADDR + 457 * 4096)
#define  EM_ICON_SPAIN_FLAG_ADDR                    (EM_ICON_BASE_FLASH_ADDR + 458 * 4096)
#define  EM_ICON_FRANCE_FLAG_ADDR                   (EM_ICON_BASE_FLASH_ADDR + 459 * 4096)
#define  EM_ICON_GREECE_FLAG_ADDR                   (EM_ICON_BASE_FLASH_ADDR + 460 * 4096)
#define  EM_ICON_HUNGARY_FLAG_ADDR                  (EM_ICON_BASE_FLASH_ADDR + 461 * 4096)
#define  EM_ICON_BELGIUM_FLAG_ADDR                  (EM_ICON_BASE_FLASH_ADDR + 462 * 4096)
#define  EM_ICON_THAILAND_FLAG_ADDR                 (EM_ICON_BASE_FLASH_ADDR + 463 * 4096)
#define  EM_ICON_ENGLAND_FLAG_ADDR                  (EM_ICON_BASE_FLASH_ADDR + 464 * 4096)
#define  EM_ICON_NEW_ZEALAND_FLAG_ADDR              (EM_ICON_BASE_FLASH_ADDR + 465 * 4096)
#define  EM_ICON_RESERVE_FLAG_ADDR                  (EM_ICON_BASE_FLASH_ADDR + 466 * 4096)

#define  EM_ICON_DOWN_ARROW_ADDR                    (EM_ICON_BASE_FLASH_ADDR + 485 * 4096)
#define  EM_ICON_TICK_ADDR                          (EM_ICON_BASE_FLASH_ADDR + 440 * 4096)
#define  EM_ICON_ERROR_ADDR                         (EM_ICON_BASE_FLASH_ADDR + 443 * 4096)

#define  EM_ICON_BATTERY1_ADDR                      (EM_ICON_BASE_FLASH_ADDR + 484 * 4096)
#define  EM_ICON_BATTERY2_ADDR                      (EM_ICON_BASE_FLASH_ADDR + 486 * 4096)
#define  EM_ICON_BATTERY3_ADDR                      (EM_ICON_BASE_FLASH_ADDR + 487 * 4096)
#define  EM_ICON_BATTERY4_ADDR                      (EM_ICON_BASE_FLASH_ADDR + 488 * 4096)
#define  EM_ICON_FLUENT_TEMP_ADDR                   (EM_ICON_BASE_FLASH_ADDR + 489 * 4096)
#define  EM_ICON_ELEC_BATTERY_ADDR                  (EM_ICON_BASE_FLASH_ADDR + 490 * 4096)
#define  EM_ICON_ELEC_TIME_ADDR                     (EM_ICON_BASE_FLASH_ADDR + 492 * 4096)

#define  EM_ICON_SET_OPEN_ADDR                      (EM_ICON_BASE_FLASH_ADDR + 444 * 4096)
#define  EM_ICON_SET_CLOSE_ADDR                     (EM_ICON_BASE_FLASH_ADDR + 445 * 4096)


#define  EM_ICON_3D_NAV_SELECT_ADDR                 (EM_ICON_BASE_FLASH_ADDR + 439 * 4096)
#define  EM_ICON_3D_DEV_STAT_ADDR                   (EM_ICON_BASE_FLASH_ADDR + 504 * 4096)
#define  EM_ICON_3D_DEV_SETTING_ADDR                (EM_ICON_BASE_FLASH_ADDR + 512 * 4096)
#define  EM_ICON_3D_DEV_INFO_ADDR                   (EM_ICON_BASE_FLASH_ADDR + 520 * 4096)
#define  EM_ICON_3D_FACTORY_RESTORE_ADDR            (EM_ICON_BASE_FLASH_ADDR + 528 * 4096)
#define  EM_ICON_UP_ARROW_ADDR                      (EM_ICON_BASE_FLASH_ADDR + 536 * 4096)

#define EM_ICON_HOME_PAGE_ADDR                      (EM_ICON_BASE_FLASH_ADDR + 544 * 4096)
#define EM_ICON_HOME_ACTIVE_PAGE_ADDR               (EM_ICON_BASE_FLASH_ADDR + 667 * 4096)
#define EM_ICON_HOME_DISABLED_PAGE_ADDR             (EM_ICON_BASE_FLASH_ADDR + 685 * 4096)
#define EM_ICON_GRID_ACTIVE_IN_PAGE_ADDR            (EM_ICON_BASE_FLASH_ADDR + 703 * 4096)
#define EM_ICON_GRID_ACTIVE_OUT_PAGE_ADDR           (EM_ICON_BASE_FLASH_ADDR + 710 * 4096)
#define EM_ICON_GRID_DISABLED_PAGE_ADDR             (EM_ICON_BASE_FLASH_ADDR + 717 * 4096)
#define EM_ICON_SOLAR_ACTIVE_PAGE_ADDR              (EM_ICON_BASE_FLASH_ADDR + 724 * 4096)
#define EM_ICON_SOLAR_DISABLED_PAGE_ADDR            (EM_ICON_BASE_FLASH_ADDR + 725 * 4096)
#define EM_ICON_SOLAR_LINE_DAYTIME_PAGE_ADDR        (EM_ICON_BASE_FLASH_ADDR + 726 * 4096)

#define EM_ICON_NEW_SOLAR_ACTIVE_PAGE_ADDR          (EM_ICON_BASE_FLASH_ADDR + 727 * 4096)
#define EM_ICON_SOLAR_PLATE_PAGE_ADDR               (EM_ICON_BASE_FLASH_ADDR + 728 * 4096)


#define  EM_FONT_BASE_FLASH_ADDR                    (6144 * 4096)
#define  EM_FONT_ITCAVANT_BOLD_ADDR                 (EM_FONT_BASE_FLASH_ADDR)
#define  EM_FONT_ITCAVANT_DEMI_ADDR                 (EM_FONT_BASE_FLASH_ADDR + 8 * 4096)
#define  EM_FONT_ITCAVANT_MD_ADDR                   (EM_FONT_BASE_FLASH_ADDR + 16 * 4096)

typedef enum _EN_NationalList
{
    EN_NATIONALLIST_CN = 0,             //0 中国
    EN_NATIONALLIST_DE,                 //1 德国
    EN_NATIONALLIST_AU_A,               //2 澳大利亚
    EN_NATIONALLIST_AU_B,               //3 澳大利亚
    EN_NATIONALLIST_AU_C,               //4 澳大利亚
    EN_NATIONALLIST_AU_NZ,              //5 新西兰
    EN_NATIONALLIST_IT,                 //6 意大利
    EN_NATIONALLIST_ES,                 //7 西班牙
    EN_NATIONALLIST_GB,                 //8 英国
    EN_NATIONALLIST_HU,                 //9 匈牙利
    EN_NATIONALLIST_BE,                 //10 比利时
    EN_NATIONALLIST_GR,                 //11 希腊
    EN_NATIONALLIST_FR,                 //12 法国
    EN_NATIONALLIST_THA,                //13 泰国
    EN_NATIONALLIST_LOCAL,              //14 本地
    EN_NATIONALLIST_60HZ,               //15 60赫兹
    EN_NATIONALLIST_MAX                 // 限定符
} EN_NATIONALLIST;

typedef struct _national_idx_stru_t
{
    uint16_t idx;
    int32_t img_addr;	// 国旗地址
    char name[20];		// 国家名称
} NationalIdx_t;
extern NationalIdx_t _garrNationalIdx[EN_NATIONALLIST_MAX];

extern GUI_FONT gFontITCAvantBold16;
extern GUI_FONT gFontITCAvantBold13;
extern GUI_FONT gFontITCAvantDemi16;
extern GUI_FONT gFontITCAvantMd16;


#define  EM_TEXT_FONT_1             GUI_FONT_13_1
#define  EM_TEXT_FONT_2             GUI_FONT_10S_1
#define  EM_TEXT_FONT_3             GUI_FONT_20_1
#define  EM_TEXT_FONT_4             GUI_FONT_20B_1
#define  EM_TEXT_FONT_5             &gFontITCAvantBold16
//#define  EM_TEXT_FONT_5             GUI_FONT_16B_1
#define  EM_TEXT_FONT_6             GUI_FONT_16_1
//#define  EM_TEXT_FONT_7             GUI_FONT_13_1
#define  EM_TEXT_FONT_7             &gFontITCAvantBold13


typedef struct _em_win_stru_t
{
    GUI_HWIN        win_handle;
    void            *pAttri;
    GUI_HWIN        (*create_win)();
    void            (*delete_win)();
    void            (*key_handler)(KeyNewMsg_t *);
    void            (*polling)();
} emWin_t;




/*部分常用控件的输入参数信息*/
typedef struct _em_widget_text_para_stru_t
{
    GUI_CONST_STORAGE GUI_FONT        *p_text_font;
    GUI_COLOR                         text_color;
    char                              *p_content;
    float                             digit_value;//仅当文本是数字时使用
} emWidgetTextPara_t;

typedef struct _em_widget_img_para_stru_t
{
    GUI_HMEM        mem_img;
    uint8_t         *p_img_buf;
    uint32_t        img_size;
} emWidgetImgPara_t;

typedef struct _em_widget_progbar_para_stru_t
{
    GUI_CONST_STORAGE GUI_FONT        *p_text_font;
//    GUI_COLOR                         text_color;//暂时固定一种颜色
    GUI_COLOR                         bar_color;
    uint16_t                          value;
} emWidgetProgbarPara_t;

typedef enum
{
    EM_WIDGET_TYPE_NULL,
    EM_WIDGET_TYPE_TEXT,
    EM_WIDGET_TYPE_IMG,
    EM_WIDGET_TYPE_PROGBAR,
} EM_WIDGET_TYPE;

typedef enum
{
    EM_TEXT_DEFAULT_ALIGN = 0,
    EM_TEXT_USER_ALIGN,
} EM_ALIGN_TYPE;

typedef struct _em_sub_region_t
{
    uint16_t        x_pos;
    uint16_t        y_pos;
    uint16_t        x_size;
    uint16_t        y_size;
} emSubRegion_t;


typedef struct _em_sub_widget_para_t
{
    EM_WIDGET_TYPE  widget_type;
    uint16_t        x_relative_pos;
    uint16_t        y_relative_pos;
    uint16_t        x_size;
    uint16_t        y_size;
    uint32_t        align;
    union           sub_widget_uni_t
    {
        emWidgetTextPara_t      text_para;
        emWidgetImgPara_t       img_para;
        emWidgetProgbarPara_t   progbar_para;
    } sub_widget;
} emSubWidgetPara_t;

/*列表控件信息*/
typedef enum
{
    EN_LIST_SELECTED_TYPE_FRAME,
    EN_LIST_SELECTED_TYPE_FILLED,
} EN_LIST_SELECTED_TYPE;

#define EM_LIST_ONE_LINE_MAX_WIDGET_NUM     3
#define EM_LIST_MAX_DISPLAY_NUM             8
#define EM_LIST_MAX_SUB_LIST                20



typedef struct _em_list_para_stru_t
{
    uint32_t                list_start_id;//控件起始ID

    uint8_t                 list_num;
    uint8_t                 list_display_num;
    uint8_t                 list_page;
    uint8_t                 list_sel;//如果为0xFF，则表示当前界面无需选中，仅供浏览
    uint8_t                 list_pre_sel;
    GUI_COLOR               list_bk_color;

    int16_t                 list_x_offset;//相对于SubRegion的y_pos + y_size - 1的偏移，可以是负值
    int16_t                 list_y_offset;//相对于SubRegion的y_pos + y_size - 1的偏移，可以是负值

    emSubRegion_t           sub_region[EM_LIST_MAX_DISPLAY_NUM];

    WM_HMEM                 handle[EM_LIST_ONE_LINE_MAX_WIDGET_NUM][EM_LIST_MAX_DISPLAY_NUM];
    emSubWidgetPara_t       sub_widget_para[EM_LIST_ONE_LINE_MAX_WIDGET_NUM][EM_LIST_MAX_SUB_LIST];

    /*List 选中框*/
    GUI_COLOR               selected_unvalid_color;
    GUI_COLOR               selected_valid_color;
    uint16_t                selected_size;
    uint16_t                selected_r;
    int16_t                 selected_x_start_offset;//相对于SubRegion的x_pos的偏移，可以是负值
    int16_t                 selected_y_start_offset;//相对于SubRegion的y_pos的偏移，可以是负值
    int16_t                 selected_x_end_offset;//相对于SubRegion的x_pos + x_size - 1的偏移，可以是负值
    int16_t                 selected_y_end_offset;//相对于SubRegion的y_pos + y_size - 1的偏移，可以是负值
    EN_LIST_SELECTED_TYPE   selected_type;//选中框类型，包括空心框和填充框

    /*滚动条*/
    bool                    scrollbar_enable;
    WM_HMEM                 scrollbar_handle;
    GUI_COLOR               scrollbar_color;
    GUI_COLOR               scrollbar_bk_color;
    uint16_t                scrollbar_x_pos;
    uint16_t                scrollbar_y_pos;
    uint16_t                scrollbar_x_size;
    uint16_t                scrollbar_y_size;
} emListPara_t;


#define EM_POP_UP_MAX_SUB_WIDGET    8

typedef struct _em_popup_para_stru_t
{
    uint32_t                popup_start_id;
    uint16_t                x_pos;
    uint16_t                y_pos;
    uint16_t                x_size;
    uint16_t                y_size;

    GUI_COLOR               bk_color;
    uint8_t                 widget_num;
//    WM_HMEM                 handle;
    WM_HMEM                 wid_handle[EM_POP_UP_MAX_SUB_WIDGET];
    emSubWidgetPara_t       sub_widget_para[EM_POP_UP_MAX_SUB_WIDGET];
} emPopUpPara_t;


/*选中国旗的索引*/
typedef struct _flag_idx_stru_t
{
    int32_t flag_idx;
} FlagIdx_t;

extern FlagIdx_t gstFlagIndex;
extern uint16_t gSwitchIconStates;
extern uint32_t gWaitTime;
extern uint16_t gSinglePhase;
extern uint16_t gPackInfoIdx;
extern uint16_t gVoltCurrPowerInfoIdx;

/*按键信息*/
typedef enum
{
    EN_EM_KEY_PRESSED_UP,
    EN_EM_KEY_PRESSED_DOWN
} EN_EM_KEY_PRESSED_STATUS;


typedef enum
{
    EN_EM_KEY_TRIGGER_TYPE_ONLY_SHORT_PRESS,
    EN_EM_KEY_TRIGGER_TYPE_LONG_CONTINUOUS_PRESS,
    EN_EM_KEY_TRIGGER_TYPE_LONG_SINGLE_PRESS
} EN_EM_KEY_TRIGGER_TYPE;



typedef struct _em_key_info_stru_t
{
    EN_EM_KEY_TRIGGER_TYPE      type;
    EN_EM_KEY_PRESSED_STATUS    status;
    bool                        key_func_valid;//切换至新页面后，默认按键功能禁止，只有收到按键按下，且时长为0的消息时，才开启按键功能
    uint32_t                    long_continuous_start_time;//长按连续的按键，停顿时长
    uint32_t                    long_continuous_period;//长按连续触发后，多长时间执行一次连续动作
    uint32_t                    long_single_time;//长按单次功能的按键，区分长按和短按的阈值

} EmKeyInfo_t;

extern EmKeyInfo_t gstEmKeyInfo[];
extern void EmKeyDefaultInfoSet(void);
extern int32_t EmGetKeyEvent(KeyNewMsg_t *p_msg);



extern emWin_t *pCurrentEmWin;
extern emWin_t *pPrevEmWin;
extern emWin_t *pCurrentPopUpSubWin;

extern GUI_LOGPALETTE      Palette565;/*565*/
extern GUI_LOGPALETTE      Palette888;/*888*/
extern GUI_BITMAP          Bitmap;

extern emWin_t  gstEmWinStartup;
extern emWin_t  gstEmWinMain;
extern emWin_t  gstEmWinListOperaInfo;

extern emWin_t  gstEmWinMain3DNav;

extern emWin_t  gstEmWinListEquipOperaInfo;
extern emWin_t  gstEmWinListPVInputInfo;
extern emWin_t  gstEmWinListGridInfo;
extern emWin_t  gstEmWinListGeneratorInput;
extern emWin_t  gstEmWinListBackUpLoadInfo;
extern emWin_t  gstEmWinListNormalLoadInfo;
extern emWin_t  gstEmWinListBatteryInfo;
extern emWin_t  gstEmWinListInverterInfo;
extern emWin_t  gstEmWinListTemperatureInfo;
extern emWin_t  gstEmWinListStatusInfo;
extern emWin_t  gstEmWinListSystemSettingsInfo;
extern emWin_t  gstEmWinListFactoryRestore;
extern emWin_t  gstEmWinListSettingsInfo;
extern emWin_t  gstEmWinListWorkingModeInfo;
extern emWin_t  gstEmWinListAdvanceSystem;
extern emWin_t  gstEmWinListAdvancedSettings;
extern emWin_t  gstEmWinListAdvancedBattery;
extern emWin_t  gstEmWinListGridStand;
extern emWin_t  gstEmWinListGridStandFlag;
extern emWin_t  gstEmWinListGridStandConfirm;
extern emWin_t  gstEmWinListReconnGridTime;
extern emWin_t  gstEmWinListRunSettingInfo;
extern emWin_t  gstEmWinListSetTimeInfo;
extern emWin_t  gstEmWinListSetDataInfo;
extern emWin_t  gstEmWinListPopConfirmInfo;
extern emWin_t  gstEmWinListEnterPassword;
extern emWin_t  gstEmWinListSavedSetTimeInfo;
extern emWin_t  gstEmWinListDevBaseInfo;
extern emWin_t  gstEmWinListArmVersionInfo;
extern emWin_t  gstEmWinListFaultRecordsInfo;
extern emWin_t  gstEmWinListBatteryVersionInfo;
extern emWin_t  gstEmWinListAdvanSettingBatDOD;
extern emWin_t  gstEmWinListAdvanSettingBatCurrent;
extern emWin_t  gstEmWinListAdvanSettingBatPower;
extern emWin_t  gstEmWinListAdvanSettingBatVoltage;
extern emWin_t  gstEmWinListAdvanSettingBatWakTime;
extern emWin_t  gstEmWinListOptPARMInfo;
extern emWin_t  gstEmWinListSysPVInputMode;
extern emWin_t  gstEmWinListAdvanSettingOptGridPower;
extern emWin_t  gstEmWinListAdvanSettingOptReactMode;
extern emWin_t  gstEmWinListAdvanSettingOptGridOVP;
extern emWin_t  gstEmWinListAdvanSettingOptInverterUVP;
extern emWin_t  gstEmWinListAdvanSettingOptOFPPoint;
extern emWin_t  gstEmWinListAdvanSettingOptUFPPoint;
extern emWin_t  gstEmWinListAdvanSettingOptOVDPoint;
extern emWin_t  gstEmWinListAdvanSettingOptUVDPoint;
extern emWin_t  gstEmWinListAdvanSettingOptOFDPoint;
extern emWin_t  gstEmWinListAdvanSettingOptUFDPoint;
extern emWin_t  gstEmWinListAdvanSettingOptReactvRespTim;
extern emWin_t  gstEmWinListAdvaSettingOptGridDropSwitch;
extern emWin_t  gstEmWinListLanguage;
extern emWin_t  gstEmWinListLanguageType;
extern emWin_t  gstEmWinListLCDBacklight;
extern emWin_t  gstEmWinListAdvanSettingOptLCDBKLTime;
extern emWin_t  gstEmWinListAdvanSettingFactoryRestore;
extern emWin_t  gstRestoreFacotryPopUpWin;
extern emWin_t  gstEmWinListADVSetPassword;
extern emWin_t  gstEmWinListDelHistoryFault;
extern emWin_t  gstDelHistoryfaultPopUpWin;
extern emWin_t  gstEmWinListAdvanceCommunication;
extern emWin_t  gstEmWinListAdvanSettingCommunicateAddr;
extern emWin_t  gstEmWinListAdvanSettingCommunicateBaudRate;
extern emWin_t  gstEmWinListReactPowerCtrlMode;
extern emWin_t  gstEmWinListMeterBrandInfo;
extern emWin_t gstEmWinListBatteryPackInfo;
extern emWin_t gstEmWinListVoltCurrPowerInfo;

extern emWin_t  gstEmWinHomePage;
extern emWin_t  gstEmWinElectricityInfo;
extern emWin_t  gEmWinHomePageDayTime;

extern emWin_t  gstPopUpWinError;
extern emWin_t  gstEmWinUpgrade;

extern emWin_t  gstPopUpPeakShiftingTimeErrorWin;
extern emWin_t  gstEmWinListSystemGeneratorPower;
extern emWin_t  gstEmWinListAdvanSettingHouseholdLimitEnable;
extern emWin_t  gstEmWinListAdvanSettingHouseholdGridPower;
extern emWin_t  gstShutDownPopUpWin;

void EmMenuInit(void);
void MenuPolling(void);

void EmListWidgetCreate(emListPara_t *p_widget_para, WM_HWIN h_win_src);
void EmListDownward(emListPara_t *p_widget_para, WM_HWIN h_win_src);
void EmListUpward(emListPara_t *p_widget_para, WM_HWIN h_win_src);
void SelectedDel(emListPara_t *p_widget_para, WM_HWIN h_win_src);
void SelectedDraw(emListPara_t *p_widget_para, WM_HWIN h_win_src);
void SelectedDelInWMPaint(emListPara_t *p_widget_para,  uint8_t list_sel);
void SelectedDrawInWMPaint(emListPara_t *p_widget_para,  uint8_t list_sel);

void EmPopUpSubWidgetCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src);

void MenuSwitch(emWin_t *p_next_win);

int32_t GetKeyEvent(KeyNewMsg_t *p_msg, int32_t *init);

void EmTestMemory(void);

/**
 * @brief 提权到指定用户
 *
 * @param type 0:NORMAL_USER, 1:MANAGER_USER, 2:ADJUST_USER, 3:INTERNAL_USER
 *
 * @return 0:success, others failed
 */
int32_t GrantPrivileged(uint16_t type);

int32_t ResetPrivileged(void);


int32_t EmDebugWinHide(void);
int32_t EmDebugWinShow(emWin_t *pre);
int32_t ShowEmUpgrading(int32_t flag);

char *GetPeakShiftingErr(void);
int32_t ShowRtcTime(uint32_t time, char* txt_buff, int size);
void SetLcdBackLigthStatus(uint8_t status);
void TurnOffWinPop(void);
#endif




