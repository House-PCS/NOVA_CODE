/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_main_3d_nav.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-12-18
  * @brief       启动界面
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
#include "error_proc.h"
#include <time.h>

#define EM_MAIN_3D_HIDE                             (0)

#define EM_MAIN_3D_NAV_SELECT_SIZE                  (4 * 1024)
#define EM_MAIN_3D_NAV_DEV_STAT_SIZE                (4 * 1024 * 8)
#define EM_MAIN_3D_NAV_SETTING_SIZE                 (4 * 1024 * 8)
#define EM_MAIN_3D_NAV_DEV_INFO_SIZE                (4 * 1024 * 8)
#define EM_MAIN_3D_NAV_FACTORY_RESTORE_SIZE         (4 * 1024 * 8)


#define EM_MAIN_3D_NAV_WIFI_ICON_SIZE               (4 * 1024)
#define EM_MAIN_3D_NAV_WARNING_ICON_SIZE            (4 * 1024)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinMain3DNav = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int8_t select;

    uint16_t sys_state;
    uint16_t work_mode;
    uint16_t electic_purchase_day;
    uint16_t electic_sale_day;
    uint32_t pv_sum_generation_on_day;
    uint16_t load_cost_day;
    uint16_t soc;
    uint16_t charge_type;
    uint16_t chassis_internal_temp;
    uint16_t net_state;
    uint16_t warnning_state;
    uint32_t time;
    uint16_t navbk_flag;
} UiCtx_t;

static UiCtx_t _gstUiCtx;

static __SDRAM uint8_t _garr3DNavPageBKBuf[4][1024 * 32];

static IMAGE_Handle _gImgStatBarWifiHandle;//状态栏WIFI图标句柄
static IMAGE_Handle _gImgStatBarWarningHandle;//状态栏WIFI图标句柄
static TEXT_Handle  _gTextStatBarTimeHandle;//状态栏时间句柄
static TEXT_Handle  _gTextStatBarWorkMode1Handle;//状态栏时间句柄
static TEXT_Handle  _gTextStatBarWorkMode2Handle;//状态栏时间句柄
static TEXT_Handle  _gTextStatBarWarningHandle;//状态栏时间句柄


/*状态栏窗口参数*/
static emWidgetImgPara_t  _garrStatBarWifiImgPara[4];      // 0:OFF 1: ON  2:disconnect 3:connect
static emWidgetImgPara_t  _garrStatBarWarningImgPara[2];       // 0:WARNING, 1ERROR:
static emWidgetTextPara_t  _gstStatBarTimeTextPara;
static emWidgetTextPara_t  _gstStatBarWorkMode1TextPara;
static emWidgetTextPara_t  _gstStatBarWorkMode2TextPara;
static emWidgetTextPara_t  _gstStatBarWarningTextPara;

static char _garrStatBarTimeTextBuf[80] = {0}; // 2024 Jan 1th 00:00:01
static char _garrStatBarWorkMode1TextBuf[32] = {0};
static char _garrStatBarWorkMode2TextBuf[32] = {0};
static char _garrStatBarWarningTextBuf[8] = {0};



/*四个选项句柄和参数*/
static IMAGE_Handle       _gImgDevStatHandle;//电量统计之市电充电图标句柄
static emWidgetImgPara_t  _gstDevStatImgPara;

static IMAGE_Handle       _gImgSettingHandle;//电量统计之市电充电图标句柄
static emWidgetImgPara_t  _gSettingImgPara;

static IMAGE_Handle       _gImgDevInfoHandle;//电量统计之市电充电图标句柄
static emWidgetImgPara_t  _gstDevInfoImgPara;

static IMAGE_Handle       _gImgFactoryRestoreHandle;//电量统计之市电充电图标句柄
static emWidgetImgPara_t  _gstFactoryRestoreImgPara;

/*选择三角形句柄和参数*/
static IMAGE_Handle       _gImgNavSelectHandle;
static emWidgetImgPara_t  _gstImgNavSelectPara;

/*顶部状态栏控件ID号*/
#define WIN_MAIN_3D_NAV_ID                              (GUI_ID_USER + 0x10)
#define WIN_MAIN_3D_NAV_WIFI_IMG_ID                     (GUI_ID_USER + 0x11)
#define WIN_MAIN_3D_NAV_WARNING_IMG_ID                  (GUI_ID_USER + 0x12)
#define WIN_MAIN_3D_NAV_TIME_TEXT_ID                    (GUI_ID_USER + 0x13)
#define WIN_MAIN_3D_NAV_WORK_MODE1_TEXT_ID              (GUI_ID_USER + 0x14)
#define WIN_MAIN_3D_NAV_WORK_MODE2_TEXT_ID              (GUI_ID_USER + 0x15)
#define WIN_MAIN_3D_NAV_WARNING_TEXT_ID                 (GUI_ID_USER + 0x17)
/*四个选项ID*/
#define WIN_MAIN_3D_NAV_DEV_STAT_IMG_ID                 (GUI_ID_USER + 0x30)
#define WIN_MAIN_3D_NAV_SETTING_IMG_ID                  (GUI_ID_USER + 0x40)
#define WIN_MAIN_3D_NAV_DEV_INFO_IMG_ID                 (GUI_ID_USER + 0x51)
#define WIN_MAIN_3D_NAV_FACTORY_IMG_ID                  (GUI_ID_USER + 0x61)
/*选择三角形*/
#define WIN_MAIN_3D_NAV_SELECT_ID                       (GUI_ID_USER + 0x21)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrMain3dNavDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "Main3dNavWin",    WIN_MAIN_3D_NAV_ID,                   0,           0,      480,    320,    0,   0x0,  0 },
    { TEXT_CreateIndirect,    "time",            WIN_MAIN_3D_NAV_TIME_TEXT_ID,         0,           15,     158,    28,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "mode",            WIN_MAIN_3D_NAV_WORK_MODE1_TEXT_ID,   148 + 5 ,    2,      80,     58,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "preset",          WIN_MAIN_3D_NAV_WORK_MODE2_TEXT_ID,   148 + 95,    2,      130,    58,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "TextAlert",       WIN_MAIN_3D_NAV_WARNING_TEXT_ID,      380,         23,     30,     18,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "ImgAlert",        WIN_MAIN_3D_NAV_WARNING_IMG_ID,       415,         20,     20,     20,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "ImgWifi",         WIN_MAIN_3D_NAV_WIFI_IMG_ID,          439,         15,     28,     28,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "OptState",        WIN_MAIN_3D_NAV_DEV_STAT_IMG_ID,      46,          100,    85,     110,    0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "Setting",         WIN_MAIN_3D_NAV_SETTING_IMG_ID,       146,         100,    85,     110,    0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "DeviceInfo",      WIN_MAIN_3D_NAV_DEV_INFO_IMG_ID,      246,         100,    85,     110,    0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "Factory",         WIN_MAIN_3D_NAV_FACTORY_IMG_ID,       346,         100,    85,     110,    0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "Select",          WIN_MAIN_3D_NAV_SELECT_ID,            0,           204,    25,     25,     0,   0x0,  0 },
};

static int32_t _Set3dNavWifiStat(int32_t ui_update, EN_WIFI_STAT_E e_stat)
{
    emWidgetImgPara_t  *p_st_tmp_wifi_img_para = &_garrStatBarWifiImgPara[0];
    switch (e_stat)
    {
        case EN_WIFI_STAT_OFF:
        {
            p_st_tmp_wifi_img_para = &_garrStatBarWifiImgPara[0];
            break;
        }
        case EN_WIFI_STAT_ON:
        {
            p_st_tmp_wifi_img_para = &_garrStatBarWifiImgPara[1];
            break;
        }
        case EN_WIFI_STAT_DISCONN:
        {
            p_st_tmp_wifi_img_para = &_garrStatBarWifiImgPara[2];
            break;
        }
        case EN_WIFI_STAT_CONN:
        {
            p_st_tmp_wifi_img_para = &_garrStatBarWifiImgPara[3];
            break;
        }
        default:
            break;
    }
    if(ui_update)
    {
        IMAGE_SetDTA(_gImgStatBarWifiHandle, p_st_tmp_wifi_img_para->p_img_buf,
                     p_st_tmp_wifi_img_para->img_size);
    }
    return 0;
}

static int32_t _Set3dNavTime(uint8_t ui_update, uint32_t time)
{
    
    ShowRtcTime(time,_garrStatBarTimeTextBuf,sizeof(_garrStatBarTimeTextBuf));
    
    if(ui_update)
    {
        TEXT_SetText(_gTextStatBarTimeHandle, (const char *)_gstStatBarTimeTextPara.p_content);
    }
    return 0;
}

static int32_t _Set3dNavSelect(uint32_t idx)
{
    int32_t x_idx = -100;

    switch (idx)
    {
        case 0:
            x_idx =  _garrMain3dNavDialogCreate[7].x0 - _garrMain3dNavDialogCreate[11].xSize / 2 + _garrMain3dNavDialogCreate[7].xSize / 2;
            break;
        case 1:
            x_idx =  _garrMain3dNavDialogCreate[8].x0 - _garrMain3dNavDialogCreate[11].xSize / 2 + _garrMain3dNavDialogCreate[8].xSize / 2;
            break;
        case 2:
            x_idx =  _garrMain3dNavDialogCreate[9].x0 - _garrMain3dNavDialogCreate[11].xSize / 2 + _garrMain3dNavDialogCreate[9].xSize / 2;
            break;
        case 3:
            x_idx =  _garrMain3dNavDialogCreate[10].x0 - _garrMain3dNavDialogCreate[11].xSize / 2 + _garrMain3dNavDialogCreate[10].xSize / 2;
            break;
        default:
            break;
    }

    WM_MoveTo(_gImgNavSelectHandle, x_idx, _garrMain3dNavDialogCreate[11].y0);
    return 0;
}


static int32_t _Set3DNavWarning(uint8_t ui_update, EN_FAULT_STAT_E e_stat, uint32_t num)
{
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
        snprintf(_garrStatBarWarningTextBuf, sizeof(_garrStatBarWarningTextBuf), "%.4d", num);
    }
    else if(num == 0xFFFF)
    {
        snprintf(_garrStatBarWarningTextBuf, sizeof(_garrStatBarWarningTextBuf), "%s", " ");
    }
    else
    {
        snprintf(_garrStatBarWarningTextBuf, sizeof(_garrStatBarWarningTextBuf), "%.3d", num);
    }
    
    emWidgetImgPara_t  *p_st_tmp_fault_img_para = &_garrStatBarWarningImgPara[0];
    switch ((int32_t)e_stat)
    {
        case EN_WARNNING_STAT:
        {
            _gstStatBarWarningTextPara.text_color = 0X0000BEF7;
            p_st_tmp_fault_img_para = &_garrStatBarWarningImgPara[0];

            break;
        }
        case EN_FAULT_STAT:
        {
            _gstStatBarWarningTextPara.text_color = 0X006A62FF;
            p_st_tmp_fault_img_para = &_garrStatBarWarningImgPara[1];
            break;
        }
        case EN_NORMAL_STAT:
        {
            snprintf(_garrStatBarWarningTextBuf, sizeof(_garrStatBarWarningTextBuf), "%s", " ");
            p_st_tmp_fault_img_para = NULL;
            break;
        }

        default:
            break;
    }

    if(ui_update)
    {
        //IMAGE_SetDTA(_gImgStatBarWarningHandle, _garrStatBarWarningImgPara[type].p_img_buf,
        //_garrStatBarWarningImgPara[type].img_size);
        TEXT_SetTextColor(_gTextStatBarWarningHandle, _gstStatBarWarningTextPara.text_color);
        TEXT_SetText(_gTextStatBarWarningHandle, (const char *)_gstStatBarWarningTextPara.p_content);

        if ((int32_t)e_stat == EN_NORMAL_STAT)
        {
            WM_HideWindow(_gImgStatBarWarningHandle);
        }
        else
        {
            WM_ShowWindow(_gImgStatBarWarningHandle);
            IMAGE_SetDTA(_gImgStatBarWarningHandle, p_st_tmp_fault_img_para->p_img_buf,
                         p_st_tmp_fault_img_para->img_size);
        }
    }
    return 0;
}

static int32_t _Set3DNavSysStatus(uint8_t ui_update, EN_SYS_STATUS_E e_mode)
{
    switch (e_mode)
    {
        case EN_SYS_STAND_BY:
        {
            _gstStatBarWorkMode1TextPara.text_color = 0X0036B6FF;
            snprintf(_garrStatBarWorkMode1TextBuf, sizeof(_garrStatBarWorkMode1TextBuf),
                     "%s", "Stand-by");
            break;
        }
        case EN_SYS_OFF_GRID:
        {
            _gstStatBarWorkMode1TextPara.text_color = 0X00D9D9D9;
            snprintf(_garrStatBarWorkMode1TextBuf, sizeof(_garrStatBarWorkMode1TextBuf),
                     "%s", "Off Grid");
            break;
        }
        case EN_SYS_ON_GRID:
        {
            _gstStatBarWorkMode1TextPara.text_color = 0X00B2BE00;
            snprintf(_garrStatBarWorkMode1TextBuf, sizeof(_garrStatBarWorkMode1TextBuf),
                     "%s", "On Grid");
            break;
        }
        case EN_SYS_SELF_TEST:
        {
            _gstStatBarWorkMode1TextPara.text_color = 0X0036B6FF;
            snprintf(_garrStatBarWorkMode1TextBuf, sizeof(_garrStatBarWorkMode1TextBuf),
                     "%s", "Self-test");
            break;
        }
        case EN_SYS_BY_PASS:
        {
            _gstStatBarWorkMode1TextPara.text_color = 0X00FFC44C;
            snprintf(_garrStatBarWorkMode1TextBuf, sizeof(_garrStatBarWorkMode1TextBuf),
                     "%s", "Bypass");
            break;
        }
        case EN_SYS_SHUTDOWN:
        {
            _gstStatBarWorkMode1TextPara.text_color = 0X00D9D9D9;
            snprintf(_garrStatBarWorkMode1TextBuf, sizeof(_garrStatBarWorkMode1TextBuf),
                     "%s", "Shutdown");
            break;
        }
        default:
            break;
    }
    if(ui_update)
    {
        TEXT_SetTextColor(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.text_color);
        TEXT_SetText(_gTextStatBarWorkMode1Handle,
                     (const char *)_gstStatBarWorkMode1TextPara.p_content);
    }
    return 0;
}

static int32_t _Set3DNavPcsWorkMode(uint8_t ui_update, EN_WORK_MODE_E e_mode)
{
    switch (e_mode)
    {
        case EN_WORK_MODE_SELF_CONSUMP:
        {
            snprintf(_garrStatBarWorkMode2TextBuf, sizeof(_garrStatBarWorkMode2TextBuf),
                     "%s", "Self Consump");
            break;
        }
        case EN_WORK_MODE_BAT_PRIORITY:
        {
            snprintf(_garrStatBarWorkMode2TextBuf, sizeof(_garrStatBarWorkMode2TextBuf),
                     "%s", "BAT Prioity");
            break;
        }
        case EN_WORK_MODE_PL_BALANCE:
        {
            snprintf(_garrStatBarWorkMode2TextBuf, sizeof(_garrStatBarWorkMode2TextBuf),
                     "%s", "Peak Shifting");
            break;
        }
        default:
            break;
    }

    if(ui_update)
    {
        TEXT_SetText(_gTextStatBarWorkMode2Handle,
                     (const char *)_gstStatBarWorkMode2TextPara.p_content);
    }
    return 0;
}



/**
  * @brief main Win callback
  * @param  NULL
  * @retval NULL
  */
static void _cbMain3dNavWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;

    SysWarningModuleDispMap_t *show_err_code;
    
    uint32_t arr_errcode[4] =
    {
        GetErrId(),         // 错误码
        0,                  // 临时错误类型
        0,                  // 错误源
        0                   // 错误值
    };
    static time_t cur_time  = 1704067200;
    static int    wifi_type = 0;

    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {

            /*时间*/
            _gTextStatBarTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_TIME_TEXT_ID);
            TEXT_SetFont(_gTextStatBarTimeHandle, _gstStatBarTimeTextPara.p_text_font);
            TEXT_SetText(_gTextStatBarTimeHandle, (const char *)_gstStatBarTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextStatBarTimeHandle, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarTimeHandle, _gstStatBarTimeTextPara.text_color);

            /*Mode*/
            _gTextStatBarWorkMode1Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_WORK_MODE1_TEXT_ID);
            TEXT_SetFont(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWorkMode1Handle, (const char *)_gstStatBarWorkMode1TextPara.p_content);
            TEXT_SetTextAlign(_gTextStatBarWorkMode1Handle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.text_color);

            /*Preset*/
            _gTextStatBarWorkMode2Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_WORK_MODE2_TEXT_ID);
            TEXT_SetFont(_gTextStatBarWorkMode2Handle, _gstStatBarWorkMode2TextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWorkMode2Handle, (const char *)_gstStatBarWorkMode2TextPara.p_content);
            TEXT_SetTextAlign(_gTextStatBarWorkMode2Handle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWorkMode2Handle, _gstStatBarWorkMode2TextPara.text_color);

            /*Test Alert*/
            _gTextStatBarWarningHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_WARNING_TEXT_ID);
            TEXT_SetFont(_gTextStatBarWarningHandle, _gstStatBarWarningTextPara.p_text_font);
//            TEXT_SetText(_gTextStatBarWarningHandle, (const char *)_gstStatBarWarningTextPara.p_content);
            TEXT_SetTextAlign(_gTextStatBarWarningHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
//            TEXT_SetTextColor(_gTextStatBarWarningHandle, _gstStatBarWarningTextPara.text_color);

            /*Img Alert*/
            _gImgStatBarWarningHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_WARNING_IMG_ID);
            _Set3DNavWarning(1, (EN_FAULT_STAT_E)_gstUiCtx.warnning_state, 0);
            /*IMAGE_SetDTA(_gImgStatBarWarningHandle, _garrStatBarWarningImgPara[0].p_img_buf, _garrStatBarWarningImgPara[0].img_size); */

            /*Img Wifi*/
            _gImgStatBarWifiHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_WIFI_IMG_ID);
            _Set3dNavWifiStat(1, (EN_WIFI_STAT_E)_gstUiCtx.net_state);
            /*IMAGE_SetDTA(_gImgStatBarWifiHandle, _garrStatBarWifiImgPara[wifi_type].p_img_buf, _garrStatBarWifiImgPara[wifi_type].img_size);*/

            /*Img OptState*/
            _gImgDevStatHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_DEV_STAT_IMG_ID);
//            IMAGE_SetDTA(_gImgDevStatHandle, _gstDevStatImgPara.p_img_buf, _gstDevStatImgPara.img_size);
            IMAGE_SetDTA(_gImgDevStatHandle, _garr3DNavPageBKBuf[0], EM_MAIN_3D_NAV_DEV_STAT_SIZE);

            /*Img Settings*/
            _gImgSettingHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_SETTING_IMG_ID);
//            IMAGE_SetDTA(_gImgSettingHandle, _gSettingImgPara.p_img_buf, _gSettingImgPara.img_size);
            IMAGE_SetDTA(_gImgSettingHandle, _garr3DNavPageBKBuf[1], EM_MAIN_3D_NAV_SETTING_SIZE);

            /*Img DeviceInfo*/
            _gImgDevInfoHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_DEV_INFO_IMG_ID);
//            IMAGE_SetDTA(_gImgDevInfoHandle, _gstDevInfoImgPara.p_img_buf, _gstDevInfoImgPara.img_size);
            IMAGE_SetDTA(_gImgDevInfoHandle, _garr3DNavPageBKBuf[2], EM_MAIN_3D_NAV_DEV_INFO_SIZE);

            /*Img Factory*/
            _gImgFactoryRestoreHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_FACTORY_IMG_ID);
//            IMAGE_SetDTA(_gImgFactoryRestoreHandle, _gstFactoryRestoreImgPara.p_img_buf, _gstFactoryRestoreImgPara.img_size);
            IMAGE_SetDTA(_gImgFactoryRestoreHandle, _garr3DNavPageBKBuf[3], EM_MAIN_3D_NAV_FACTORY_RESTORE_SIZE);

            /*Img Select*/
            _gImgNavSelectHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_SELECT_ID);
            IMAGE_SetDTA(_gImgNavSelectHandle, _gstImgNavSelectPara.p_img_buf, _gstImgNavSelectPara.img_size);      

            _Set3dNavSelect((uint32_t)_gstUiCtx.select);
//            _Set3dNavTime(cur_time);
            break;
        }
        case WM_TIMER:
        {
            show_err_code = GetErrorID();
            _Set3DNavWarning(1,(EN_FAULT_STAT_E)0, 0);
            if(show_err_code->serial_num == 0x00 || show_err_code->serial_num > 0x00)
            {
                _Set3DNavWarning(1,(EN_FAULT_STAT_E)show_err_code->type, show_err_code->serial_num);
            }

            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
            break;
        }
        case WM_PAINT:
        {
            //GUI_DrawStreamedBitmapAuto(_pBKBuf, 0, 0);
            GUI_SetColor(0x434343);
            GUI_FillRoundedRect(148, 8, 148 + 220, 8 + 38, 5);
            GUI_SetColor(0x797979);
            GUI_SetPenSize(2);
            GUI_DrawLine(237, 20, 237, 20 + 18);
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
    GUI_EnableAlpha(0);
    /*创建一个基于桌面窗口的子窗口*/
    gstEmWinMain3DNav.win_handle = GUI_CreateDialogBox(_garrMain3dNavDialogCreate,
                                   GUI_COUNTOF(_garrMain3dNavDialogCreate),
                                   _cbMain3dNavWin,
                                   pCurrentEmWin->win_handle,
                                   0, 0);
    //WM_SetTransState(_gWinStatBarHandle, WM_CF_HASTRANS);

	WM_CreateTimer(gstEmWinMain3DNav.win_handle, /* 接受信息的窗口的句柄 */
				   0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
				   1000,         /* 周期，此周期过后指定窗口应收到消息*/
				   0);         /* 留待将来使用，应为0 */

    pCurrentEmWin = &gstEmWinMain3DNav;
    GUI_Exec();
    return gstEmWinMain3DNav.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinMain3DNav.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinMain3DNav.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinMain3DNav.win_handle = 0;
}



/**
  * @brief  界面控件等初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    _gstUiCtx.net_state = gstCollectInfo.maincard_data.sys_info.net_conn_state;
    _gstUiCtx.sys_state = gstCollectInfo.power_data.model_info.on_grid_state;
    _gstUiCtx.work_mode = gstCollectInfo.power_data.power_set_info.work_mode;
    _gstUiCtx.time = gstCollectInfo.maincard_data.sys_info.time_stamp;


    _gstStatBarTimeTextPara.p_content = _garrStatBarTimeTextBuf;
    _gstStatBarTimeTextPara.text_color = 0X00B2BE00;
    _gstStatBarTimeTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstStatBarWorkMode1TextPara.p_content = _garrStatBarWorkMode1TextBuf;
    _gstStatBarWorkMode1TextPara.text_color = 0X0036B6FF;
    _gstStatBarWorkMode1TextPara.p_text_font = EM_TEXT_FONT_5;

    _gstStatBarWorkMode2TextPara.p_content = _garrStatBarWorkMode2TextBuf;
    _gstStatBarWorkMode2TextPara.text_color = 0X00D9D9D9;
    _gstStatBarWorkMode2TextPara.p_text_font = EM_TEXT_FONT_5;

    _gstStatBarWarningTextPara.p_content = _garrStatBarWarningTextBuf;
    _gstStatBarWarningTextPara.text_color = 0X00000000;
    _gstStatBarWarningTextPara.p_text_font = EM_TEXT_FONT_7;

//  snprintf(_garrStatBarWorkMode1TextBuf, sizeof(_garrStatBarWorkMode1TextBuf), "%s", "stand-by");
//  snprintf(_garrStatBarWorkMode2TextBuf, sizeof(_garrStatBarWorkMode2TextBuf), "%s", "BAT Prioity");
//    snprintf(_garrStatBarWarningTextBuf, sizeof(_garrStatBarWarningTextBuf), "%s", "009");

    _garrStatBarWarningImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_WARNING_ICON_SIZE);
    _garrStatBarWarningImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrStatBarWarningImgPara[0].mem_img);
    _garrStatBarWarningImgPara[0].img_size = EM_MAIN_3D_NAV_WARNING_ICON_SIZE;
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _garrStatBarWarningImgPara[0].p_img_buf, _garrStatBarWarningImgPara[0].img_size);

    _garrStatBarWarningImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_WARNING_ICON_SIZE);
    _garrStatBarWarningImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrStatBarWarningImgPara[1].mem_img);
    _garrStatBarWarningImgPara[1].img_size = EM_MAIN_3D_NAV_WARNING_ICON_SIZE;
    SpiFlashReadData(EM_ICON_ERROR_ADDR, _garrStatBarWarningImgPara[1].p_img_buf, _garrStatBarWarningImgPara[1].img_size);

    _garrStatBarWifiImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_WIFI_ICON_SIZE);
    _garrStatBarWifiImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrStatBarWifiImgPara[0].mem_img);
    _garrStatBarWifiImgPara[0].img_size = EM_MAIN_3D_NAV_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_NOT_FOUND, _garrStatBarWifiImgPara[0].p_img_buf, _garrStatBarWifiImgPara[0].img_size);

    _garrStatBarWifiImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_WIFI_ICON_SIZE);
    _garrStatBarWifiImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrStatBarWifiImgPara[1].mem_img);
    _garrStatBarWifiImgPara[1].img_size = EM_MAIN_3D_NAV_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_OFF_ADDR, _garrStatBarWifiImgPara[1].p_img_buf, _garrStatBarWifiImgPara[1].img_size);

    _garrStatBarWifiImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_WIFI_ICON_SIZE);
    _garrStatBarWifiImgPara[2].p_img_buf = GUI_ALLOC_h2p(_garrStatBarWifiImgPara[2].mem_img);
    _garrStatBarWifiImgPara[2].img_size = EM_MAIN_3D_NAV_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_DISCONN_CLOUD_ADDR, _garrStatBarWifiImgPara[2].p_img_buf, _garrStatBarWifiImgPara[2].img_size);

    _garrStatBarWifiImgPara[3].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_WIFI_ICON_SIZE);
    _garrStatBarWifiImgPara[3].p_img_buf = GUI_ALLOC_h2p(_garrStatBarWifiImgPara[3].mem_img);
    _garrStatBarWifiImgPara[3].img_size = EM_MAIN_3D_NAV_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_ON_ADDR, _garrStatBarWifiImgPara[3].p_img_buf, _garrStatBarWifiImgPara[3].img_size);

#if EM_MAIN_3D_HIDE
    _gstDevStatImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_DEV_STAT_SIZE);
    _gstDevStatImgPara.p_img_buf = GUI_ALLOC_h2p(_gstDevStatImgPara.mem_img);
    _gstDevStatImgPara.img_size = EM_MAIN_3D_NAV_DEV_STAT_SIZE;
    SpiFlashReadData(EM_ICON_3D_DEV_STAT_ADDR, _gstDevStatImgPara.p_img_buf, _gstDevStatImgPara.img_size);

    _gSettingImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_SETTING_SIZE);
    _gSettingImgPara.p_img_buf = GUI_ALLOC_h2p(_gSettingImgPara.mem_img);
    _gSettingImgPara.img_size = EM_MAIN_3D_NAV_SETTING_SIZE;
    SpiFlashReadData(EM_ICON_3D_DEV_SETTING_ADDR, _gSettingImgPara.p_img_buf, _gSettingImgPara.img_size);

    _gstDevInfoImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_DEV_INFO_SIZE);
    _gstDevInfoImgPara.p_img_buf = GUI_ALLOC_h2p(_gstDevInfoImgPara.mem_img);
    _gstDevInfoImgPara.img_size = EM_MAIN_3D_NAV_DEV_INFO_SIZE;
    SpiFlashReadData(EM_ICON_3D_DEV_INFO_ADDR, _gstDevInfoImgPara.p_img_buf, _gstDevInfoImgPara.img_size);

    _gstFactoryRestoreImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_FACTORY_RESTORE_SIZE);
    _gstFactoryRestoreImgPara.p_img_buf = GUI_ALLOC_h2p(_gstFactoryRestoreImgPara.mem_img);
    _gstFactoryRestoreImgPara.img_size = EM_MAIN_3D_NAV_FACTORY_RESTORE_SIZE;
    SpiFlashReadData(EM_ICON_3D_FACTORY_RESTORE_ADDR, _gstFactoryRestoreImgPara.p_img_buf, _gstFactoryRestoreImgPara.img_size);
#else
    if(_gstUiCtx.navbk_flag == 0)
    {
        SpiFlashReadData(EM_ICON_3D_DEV_STAT_ADDR, _garr3DNavPageBKBuf[0], EM_MAIN_3D_NAV_DEV_STAT_SIZE);
        SpiFlashReadData(EM_ICON_3D_DEV_SETTING_ADDR, _garr3DNavPageBKBuf[1], EM_MAIN_3D_NAV_SETTING_SIZE);
        SpiFlashReadData(EM_ICON_3D_DEV_INFO_ADDR, _garr3DNavPageBKBuf[2], EM_MAIN_3D_NAV_DEV_INFO_SIZE);
        SpiFlashReadData(EM_ICON_3D_FACTORY_RESTORE_ADDR, _garr3DNavPageBKBuf[3], EM_MAIN_3D_NAV_FACTORY_RESTORE_SIZE);
        _gstUiCtx.navbk_flag = 1;
    }
#endif
    _gstImgNavSelectPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_SELECT_SIZE);
    _gstImgNavSelectPara.p_img_buf = GUI_ALLOC_h2p(_gstImgNavSelectPara.mem_img);
    _gstImgNavSelectPara.img_size = EM_MAIN_3D_NAV_SELECT_SIZE;
    SpiFlashReadData(EM_ICON_3D_NAV_SELECT_ADDR, _gstImgNavSelectPara.p_img_buf, _gstImgNavSelectPara.img_size);

    _Set3dNavTime(0, _gstUiCtx.time);
    _Set3DNavSysStatus(0, (EN_SYS_STATUS_E)_gstUiCtx.sys_state);
    _Set3DNavPcsWorkMode(0, (EN_WORK_MODE_E)_gstUiCtx.work_mode);
    _Set3DNavWarning(0, (EN_FAULT_STAT_E)_gstUiCtx.warnning_state, 0);

    EmKeyDefaultInfoSet();
}


/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_garrStatBarWarningImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrStatBarWarningImgPara[1].mem_img);
    GUI_ALLOC_Free(_garrStatBarWifiImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrStatBarWifiImgPara[1].mem_img);
    GUI_ALLOC_Free(_garrStatBarWifiImgPara[2].mem_img);
    GUI_ALLOC_Free(_garrStatBarWifiImgPara[3].mem_img);
#if EM_MAIN_3D_HIDE   
    GUI_ALLOC_Free(_gstDevStatImgPara.mem_img);
    GUI_ALLOC_Free(_gSettingImgPara.mem_img);
    GUI_ALLOC_Free(_gstDevInfoImgPara.mem_img);
    GUI_ALLOC_Free(_gstFactoryRestoreImgPara.mem_img);
#endif
    GUI_ALLOC_Free(_gstImgNavSelectPara.mem_img);
}



/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    //rtc时间
    uint32_t _rtctime = gstCollectInfo.maincard_data.sys_info.time_stamp;
    if (_gstUiCtx.sys_state != gstCollectInfo.power_data.model_info.on_grid_state)
    {
        if(gstCollectInfo.power_data.model_info.on_grid_state >= 3)
        {
            return;
        }
        _gstUiCtx.sys_state = gstCollectInfo.power_data.model_info.on_grid_state;
        _Set3DNavSysStatus(1, (EN_SYS_STATUS_E)_gstUiCtx.sys_state);
    }
    if (_gstUiCtx.work_mode != gstCollectInfo.power_data.power_set_info.work_mode)
    {
        _gstUiCtx.work_mode = gstCollectInfo.power_data.power_set_info.work_mode;
        _Set3DNavPcsWorkMode(1, (EN_WORK_MODE_E)_gstUiCtx.work_mode);
    }
    if (_gstUiCtx.net_state)
    {
        _Set3dNavWifiStat(1, (EN_WIFI_STAT_E)_gstUiCtx.net_state);
    }
    if (_gstUiCtx.time != _rtctime)
    {
        _gstUiCtx.time = _rtctime;
        _Set3dNavTime(1, _gstUiCtx.time);
    }
}


/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
//  int32_t event = GetKeyEvent(p_msg, &_gstUiCtx.key_init);
    int32_t event = 0;
    event = EmGetKeyEvent(p_msg);
    if (event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }
    if (event == EN_KEY_SRC_LEFT)
    {
        _gstUiCtx.select = (--_gstUiCtx.select) < 0 ? 0 : _gstUiCtx.select;
        _Set3dNavSelect((uint32_t)_gstUiCtx.select);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        _gstUiCtx.select = (++_gstUiCtx.select) > 3 ? 3 : _gstUiCtx.select;
        _Set3dNavSelect((uint32_t)_gstUiCtx.select);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        emWin_t *pnext = NULL;
        switch(_gstUiCtx.select)
        {
            case 0:
                pnext = &gstEmWinListEquipOperaInfo;
                break;
            case 1:
                pnext = &gstEmWinListSettingsInfo;
                break;
            case 2:
                pnext = &gstEmWinListDevBaseInfo;
                break;
            case 3:
                pnext = &gstEmWinListFactoryRestore;
                break;
            default:
                return;
        }
        if(pnext != NULL && pCurrentEmWin->win_handle != 0)
        {
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
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
}


