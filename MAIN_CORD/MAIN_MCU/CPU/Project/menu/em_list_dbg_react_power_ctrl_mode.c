/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_work_mode_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-25
  * @brief       Debug 认证无功控制模式界面
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-25    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define  TEST_DYNAMIC                                         0
#define  EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE                (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gstEmWinListReactPowerCtrlMode = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _button_stru_t
{
    uint32_t tmp_switch;
    uint32_t key_stat;
    uint32_t key_time;
} ButtonPara_t;

typedef struct _arr_button_stru_t
{
    uint32_t tmp_switch[3];
    uint32_t key_stat[3];
    uint32_t key_time[3];
    uint32_t tmp_mode;
    uint32_t select;
} ArrButtonPara_t;

typedef struct _ui_ctx_t
{
    int32_t select;
    int32_t key_init;

    uint32_t switcher[3];
    uint32_t pv_switch;     // PV开关使能
    uint32_t pf_switch;     // PF开关使能
    uint32_t ct_dir_switch;     // CT Dir
    uint16_t react_power_ctrl_mode;  //无功控制方式
    uint16_t net_is_conn_switch;   // 网络是否断开
    uint16_t hard_export_limit_switch; 
    uint16_t soft_export_limit_switch;   
    uint16_t power_rate_limit_switch; 
    ButtonPara_t pv_button;
    ButtonPara_t ct_dir_button;
    ButtonPara_t pf_button;
    ButtonPara_t net_conn_button;
    ButtonPara_t export_limit_button;
    ButtonPara_t soft_limit_button;
    ButtonPara_t power_limit_button;
    ArrButtonPara_t react_power_ctrl_button;
} UiCtx_t;

static UiCtx_t _gstUiCtx = {0};


typedef enum
{
    EN_SWITCH_STAT_OFF = 0,
    EN_SWITCH_STAT_ON,
} EN_SWITCH_STAT_E;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;
static IMAGE_Handle  _gImgSwithHandle;//开关图标句柄

static char _gTitleBuf[] = "Settings < DBG Power Ctrl Mode";

/*List控件*/
static emListPara_t  _gEmListWorkingModeInfoPara;

static char _gListWorkingModeInfoLeftTextBuf[10][32];

/*开关图片参数*/
static emWidgetImgPara_t  _gSwtichImgPara[2];  /*0:关   1:开*/


#define WIN_LIST_WORKMODE_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_WORKMODE_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)
#define WIN_LIST_SWITCH_ON_IMG_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_SWITCH_OFF_IMG_ID                              (GUI_ID_USER + 0x13)

static __SDRAM GUI_WIDGET_CREATE_INFO _aListWorkingModeInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListWorkingModeInfoWin",    WIN_LIST_WORKMODE_INFO_ID,           0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",              WIN_LIST_WORKMODE_INFO_TITLE_ID,            31,  20, 400,  14,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "SWITON",             WIN_LIST_SWITCH_ON_IMG_ID,                  400, 38,  40,  40,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "SWITCHOFF",          WIN_LIST_SWITCH_OFF_IMG_ID,                 400, 38,  40,  40,   0,   0x0,  0 }
};

static int32_t _SetCtrlMode(uint8_t row, EN_SWITCH_STAT_E stat)
{

    emWidgetImgPara_t  *tmp_switch_img_para = &_gSwtichImgPara[0];
    switch (stat)
    {
        case EN_SWITCH_STAT_OFF:
        {
            tmp_switch_img_para = &_gSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            tmp_switch_img_para = &_gSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gEmListWorkingModeInfoPara.sub_widget_para[1][row].sub_widget.img_para.mem_img = tmp_switch_img_para->mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][row].sub_widget.img_para.p_img_buf = tmp_switch_img_para->p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][row].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
    
    if(_gEmListWorkingModeInfoPara.list_page == 0)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][row], tmp_switch_img_para->p_img_buf,
                tmp_switch_img_para->img_size);
    }
    else if(_gEmListWorkingModeInfoPara.list_page == 1)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][row - 1], tmp_switch_img_para->p_img_buf,
                        tmp_switch_img_para->img_size);
    }
    else if(_gEmListWorkingModeInfoPara.list_page == 2)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][row - 2], tmp_switch_img_para->p_img_buf,
                        tmp_switch_img_para->img_size);
    }
    return 0;
}

static int32_t _UpdateSwitch(int32_t select)
{
    if (select < 0 || select > 3)
    {
        return -1;
    }


//        _gstUiCtx.select = select;
    for (int32_t i = 0; i < 3 && i < sizeof(_gstUiCtx.switcher) / sizeof(_gstUiCtx.switcher[0]); ++i)
    {
        _gstUiCtx.switcher[i] = 0;
        _gstUiCtx.react_power_ctrl_button.key_stat[i] = 0;
        _gstUiCtx.react_power_ctrl_button.tmp_switch[i] = 0;
//            if (_gstUiCtx.select == i && i < 3)
        if(select == i && i < 3)
        {
            _gstUiCtx.switcher[i] = (++_gstUiCtx.switcher[i]) % 2;
            _gstUiCtx.react_power_ctrl_button.tmp_switch[i] = _gstUiCtx.switcher[i];
            _gstUiCtx.react_power_ctrl_button.key_stat[i] = 1;
            _gstUiCtx.react_power_ctrl_button.key_time[i] = tim_get_systime();
            _gstUiCtx.react_power_ctrl_button.tmp_mode = i;
        }
        _SetCtrlMode((uint8_t)i, (EN_SWITCH_STAT_E)_gstUiCtx.react_power_ctrl_button.tmp_switch[i]);
    }
#if 0
    else
    {
        switch (select)
        {
            case 3:
            {
                _gstUiCtx.pv_switch = (++_gstUiCtx.pv_switch) % 2;
                _SetCtrlMode((uint8_t)select, (EN_SWITCH_STAT_E)(_gstUiCtx.pv_switch));
                break;
            }
            case 4:
            {
                _gstUiCtx.pf_switch = (++_gstUiCtx.pf_switch) % 2;
                _SetCtrlMode((uint8_t)select, (EN_SWITCH_STAT_E)(_gstUiCtx.pf_switch));
                break;
            }
            case 5:
            {
                _gstUiCtx.ct_dir_switch = (++_gstUiCtx.ct_dir_switch) % 2;
                _SetCtrlMode((uint8_t)select, (EN_SWITCH_STAT_E)(_gstUiCtx.ct_dir_switch));
                break;
            }
        }
    }
#endif
    return 0;
}
/**
  * @brief 设置PV开关使能
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetPVSwitchEnable(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_gSwtichImgPara[0];

    _gstUiCtx.pv_switch = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;


    if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 0)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][3], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 1)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][2], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 2)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][1], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 3)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][0], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    
    return 0;
}
/**
  * @brief 设置PF开关使能
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetPFSwitchEnable(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_gSwtichImgPara[0];

    _gstUiCtx.pf_switch = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 0)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][4], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 1)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][3], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 2)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][2], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 3)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][1], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 4)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][0], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}
/**
  * @brief 设置电流方向
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetCtDirectionStatus(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_gSwtichImgPara[0];

    _gstUiCtx.ct_dir_switch = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 0)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][5], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 1)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][4], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 2)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][3], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 3)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][2], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 4)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][1], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}

/**
  * @brief 设置网络断开开关使能
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetNetIsConnEnbale(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_gSwtichImgPara[0];

    _gstUiCtx.net_is_conn_switch = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;


    if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 1)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][5], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 2)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][4], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 3)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][3], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 4)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][2], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}
/**
  * @brief 设置电网输出口限制开关使能
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetHardExportLimitEnbale(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_gSwtichImgPara[0];

    _gstUiCtx.hard_export_limit_switch = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;


    if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 2)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][5], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 3)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][4], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 4)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][3], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}

/**
  * @brief 设置软限制开关使能
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetSoftExportLimitEnbale(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_gSwtichImgPara[0];

    _gstUiCtx.soft_export_limit_switch = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;


    if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 3)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][5], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 4)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][4], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}

/**
  * @brief 设置功率比开关使能
  * @param  NULL
  * @retval NULL
  */
static int32_t _SetPowerRateLimitEnbale(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_gSwtichImgPara[0];

    _gstUiCtx.power_rate_limit_switch = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_gSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    if(ui_updatae && _gEmListWorkingModeInfoPara.list_page == 4)
    {
        IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][5], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}

/**
  * @brief ListWorkingModeInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListWorkingModeInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    static int switch_type = 0, row = 0;

#if TEST_DYNAMIC
    static WM_HTIMER wm_timer_handle_switch;
#endif
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
#if TEST_DYNAMIC
            wm_timer_handle_switch = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                                    0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                                    1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                                    0);         /* 留待将来使用，应为0 */
#endif

            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_WORKMODE_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gEmListWorkingModeInfoPara, pMsg->hWin);
            break;
        }
        case WM_DELETE:
        {
#if TEST_DYNAMIC
            WM_DeleteTimer(wm_timer_handle_switch);//；删除窗口，会自动删除对应内部的定时器
#endif
            break;
        }
        case WM_TIMER:
        {
#if TEST_DYNAMIC
            ++switch_type;
            ++row;
            _SetCtrlMode(row % 3, (EN_SWITCH_STAT_E)(switch_type % 2));

            WM_RestartTimer(pMsg->Data.v, 300);/*暂定300mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListWorkingModeInfoPara, _gEmListWorkingModeInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(int i = 0; i < _gEmListWorkingModeInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListWorkingModeInfoPara.sub_region[i].x_pos,
                                    _gEmListWorkingModeInfoPara.sub_region[i].y_pos,
                                    _gEmListWorkingModeInfoPara.sub_region[i].x_pos + _gEmListWorkingModeInfoPara.sub_region[i].x_size - 1,
                                    _gEmListWorkingModeInfoPara.sub_region[i].y_pos + _gEmListWorkingModeInfoPara.sub_region[i].y_size - 1,
                                    _gEmListWorkingModeInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListWorkingModeInfoPara, _gEmListWorkingModeInfoPara.list_sel);

            for (int32_t i = 0; i < sizeof(_gstUiCtx.switcher) / sizeof(_gstUiCtx.switcher[0]); ++i)
            {
                _SetCtrlMode((uint8_t)i, (EN_SWITCH_STAT_E)_gstUiCtx.switcher[i]);
            }
//            _SetCtrlMode(3, (EN_SWITCH_STAT_E)(_gstUiCtx.pv_switch % 2));
//            _SetCtrlMode(4, (EN_SWITCH_STAT_E)(_gstUiCtx.pf_switch % 2));
//            _SetCtrlMode(5, (EN_SWITCH_STAT_E)(_gstUiCtx.ct_dir_switch % 2));          
            break;
        }
        case WM_NOTIFY_PARENT:
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
  * @brief Create current win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWin(void)
{
    _gstUiCtx.key_init = 0;
    pPrevEmWin = &gstEmWinListSettingsInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListReactPowerCtrlMode.win_handle = GUI_CreateDialogBox(_aListWorkingModeInfoDialogCreate, GUI_COUNTOF(_aListWorkingModeInfoDialogCreate), _cbListWorkingModeInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListReactPowerCtrlMode.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListReactPowerCtrlMode.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListReactPowerCtrlMode.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListWorkingModeInfoPara, gstEmWinListReactPowerCtrlMode.win_handle);
    return gstEmWinListReactPowerCtrlMode.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListReactPowerCtrlMode.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListReactPowerCtrlMode.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListReactPowerCtrlMode.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;
    EmKeyDefaultInfoSet();
    //每次设置前将图标清零
    memset(_gstUiCtx.switcher, 0, sizeof(_gstUiCtx.switcher));
    _gstUiCtx.pv_switch = 0;
    _gstUiCtx.pf_switch = 0;
    _gstUiCtx.ct_dir_switch = 0;
//    _gstUiCtx.react_power_ctrl_mode = gstCollectParaData.pcs_hold_data.reactive_power_ctrl_method;
    _gstUiCtx.react_power_ctrl_mode = _gstUiCtx.react_power_ctrl_button.tmp_mode;

    int32_t i = 0;
    int32_t j = sizeof(_gstUiCtx.switcher) / sizeof(_gstUiCtx.switcher[0]);
    if (gstCollectInfo.power_data.power_set_info.reactive_ctrl_type < 3)  //无功控制方式0 1 2
    {
        _gstUiCtx.switcher[_gstUiCtx.react_power_ctrl_mode] = 1;
//        _gstUiCtx.switcher[gstCollectParaData.pcs_hold_data.reactive_power_ctrl_method] = 1;
    }
    for (i = 0; i < j; ++i)
    {
        if (_gstUiCtx.switcher[i])
        {
            break;
        }
    }
    if (i == j)
    {
        _gstUiCtx.switcher[0] = 1;
    }
    _gstUiCtx.pv_switch     = gstCollectInfo.power_data.inside_info.pv_switch;
    _gstUiCtx.pf_switch     = gstCollectInfo.power_data.inside_info.pf_switch;
    _gstUiCtx.ct_dir_switch = gstCollectInfo.power_data.power_set_info.ct_dir;

    _gstUiCtx.net_is_conn_switch = 0;//gstCollectInfo.power_data.power_set_info.  & 0x01;
    _gstUiCtx.hard_export_limit_switch = 0;//gstCollectParaData.pcs_hold_data.certification_test_switch & 0x02;
    _gstUiCtx.soft_export_limit_switch = 0;//gstCollectParaData.pcs_hold_data.certification_test_switch & 0x04;
    _gstUiCtx.power_rate_limit_switch = 0;//gstCollectParaData.pcs_hold_data.certification_test_switch & 0x08;

    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gSwtichImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE);
    _gSwtichImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gSwtichImgPara[0].mem_img);
    _gSwtichImgPara[0].img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_SET_CLOSE_ADDR, _gSwtichImgPara[0].p_img_buf, _gSwtichImgPara[0].img_size);

    _gSwtichImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE);
    _gSwtichImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gSwtichImgPara[1].mem_img);
    _gSwtichImgPara[1].img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_SET_OPEN_ADDR, _gSwtichImgPara[1].p_img_buf, _gSwtichImgPara[1].img_size);


    _gEmListWorkingModeInfoPara.list_start_id = WIN_LIST_WORKMODE_INFO_ID + 0x30;
    _gEmListWorkingModeInfoPara.list_num = 10;
    _gEmListWorkingModeInfoPara.list_display_num = 6;
    if(_gEmListWorkingModeInfoPara.list_num <= _gEmListWorkingModeInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListWorkingModeInfoPara.list_num + 1 - _gEmListWorkingModeInfoPara.list_display_num;
    }

    if(_gEmListWorkingModeInfoPara.list_sel > _gEmListWorkingModeInfoPara.list_num - 1)
    {
        _gEmListWorkingModeInfoPara.list_sel = 0;
    }
    if(_gEmListWorkingModeInfoPara.list_page > max_page - 1)
    {
        _gEmListWorkingModeInfoPara.list_page = 0;
    }
    _gEmListWorkingModeInfoPara.list_bk_color = 0x00787878;
    /*2个子列表*/
    _gEmListWorkingModeInfoPara.sub_region[0].x_pos = 20;
    _gEmListWorkingModeInfoPara.sub_region[0].y_pos = 38;
    _gEmListWorkingModeInfoPara.sub_region[0].x_size = 440;
    _gEmListWorkingModeInfoPara.sub_region[0].y_size = 34;
    _gEmListWorkingModeInfoPara.sub_region[1].x_pos = 20;
    _gEmListWorkingModeInfoPara.sub_region[1].y_pos = 80;
    _gEmListWorkingModeInfoPara.sub_region[1].x_size = 440;
    _gEmListWorkingModeInfoPara.sub_region[1].y_size = 34;
    _gEmListWorkingModeInfoPara.sub_region[2].x_pos = 20;
    _gEmListWorkingModeInfoPara.sub_region[2].y_pos = 122;
    _gEmListWorkingModeInfoPara.sub_region[2].x_size = 440;
    _gEmListWorkingModeInfoPara.sub_region[2].y_size = 34;
    _gEmListWorkingModeInfoPara.sub_region[3].x_pos = 20;
    _gEmListWorkingModeInfoPara.sub_region[3].y_pos = 164;
    _gEmListWorkingModeInfoPara.sub_region[3].x_size = 440;
    _gEmListWorkingModeInfoPara.sub_region[3].y_size = 34;
    _gEmListWorkingModeInfoPara.sub_region[4].x_pos = 20;
    _gEmListWorkingModeInfoPara.sub_region[4].y_pos = 206;
    _gEmListWorkingModeInfoPara.sub_region[4].x_size = 440;
    _gEmListWorkingModeInfoPara.sub_region[4].y_size = 34;
    _gEmListWorkingModeInfoPara.sub_region[5].x_pos = 20;
    _gEmListWorkingModeInfoPara.sub_region[5].y_pos = 248;
    _gEmListWorkingModeInfoPara.sub_region[5].x_size = 440;
    _gEmListWorkingModeInfoPara.sub_region[5].y_size = 34;

    /*List控件*/
    //第一行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].x_size = 240;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[0];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Power Factor Ctrl.");
    //第一行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
    //第二行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].x_size = 240;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[1];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Reactive Power Ctrl.");
    //第二行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gSwtichImgPara[1].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _gSwtichImgPara[1].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    //第三行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].x_size = 240;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[2];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Volt-Var Ctrl.");
    //第三行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    //第4行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][3].x_size = 240;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][3].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[3];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Volt-Watt Ctrl.");
    //第4行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    //第5行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][4].x_size = 240;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][4].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[4];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Freq-Watt Ctrl.");
    //第5行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    //第6行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][5].x_size = 200;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][5].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[5];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "CT Reverse");
    //第6行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    //第7行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][6].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][6].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][6].x_size = 240;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][6].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[6];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Comm. Forced Disconnect");
    //第7行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    //第8行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][7].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][7].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][7].x_size = 200;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][7].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[7];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Hard Export Limit");
    //第8行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    //第9行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][8].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][8].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][8].x_size = 200;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][8].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[8];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_content, "Soft Export Limit");
    //第9行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][8].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    //第10行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][9].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][9].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][9].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][9].x_size = 200;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][9].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][9].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[9];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][9].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][9].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][9].sub_widget.text_para.p_content, "Power Rate Limit");
    //第10行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][9].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;


    /*List选中框*/
    _gEmListWorkingModeInfoPara.selected_unvalid_color = 0xFF000000;
    _gEmListWorkingModeInfoPara.selected_valid_color = 0x00D6E400;
    _gEmListWorkingModeInfoPara.selected_size = 3;
    _gEmListWorkingModeInfoPara.selected_r = 10;
    _gEmListWorkingModeInfoPara.selected_x_start_offset = -2;
    _gEmListWorkingModeInfoPara.selected_y_start_offset = -2;
    _gEmListWorkingModeInfoPara.selected_x_end_offset = 2;
    _gEmListWorkingModeInfoPara.selected_y_end_offset = 2;
//    _gEmListWorkingModeInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gEmListWorkingModeInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    /*滚动条*/
//    _gEmListWorkingModeInfoPara.scrollbar_enable = true;
//    _gEmListWorkingModeInfoPara.scrollbar_color = 0x00D6E400;
//    _gEmListWorkingModeInfoPara.scrollbar_bk_color = 0x00565656;
//    _gEmListWorkingModeInfoPara.scrollbar_x_pos = 454;
//    _gEmListWorkingModeInfoPara.scrollbar_y_pos = 39;
//    _gEmListWorkingModeInfoPara.scrollbar_x_size = 6;
//    _gEmListWorkingModeInfoPara.scrollbar_y_size = 241;

    _SetPVSwitchEnable(0, _gstUiCtx.pv_switch);
    _SetPFSwitchEnable(0, _gstUiCtx.pf_switch);
    _SetCtDirectionStatus(0, _gstUiCtx.ct_dir_switch);
    _SetNetIsConnEnbale(0, _gstUiCtx.net_is_conn_switch);
    _SetHardExportLimitEnbale(0, _gstUiCtx.hard_export_limit_switch);
    _SetSoftExportLimitEnbale(0, _gstUiCtx.soft_export_limit_switch);
    _SetPowerRateLimitEnbale(0, _gstUiCtx.power_rate_limit_switch);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gSwtichImgPara[0].mem_img);
    GUI_ALLOC_Free(_gSwtichImgPara[1].mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    static uint32_t timecount = 0;
    static uint8_t dir = 0;
    static uint8_t pop_up_stat = 0;
    static uint32_t pop_timecount = 0;
    uint8_t i = 0;
    if(gstEmWinListReactPowerCtrlMode.win_handle == 0)
    {
        return;
    }

    if(_gstUiCtx.react_power_ctrl_button.key_stat[_gstUiCtx.react_power_ctrl_button.select] == 1)
    {
        if(_gstUiCtx.react_power_ctrl_mode !=_gstUiCtx.react_power_ctrl_button.tmp_mode)
//        if(_gstUiCtx.switcher[_gstUiCtx.react_power_ctrl_button.select] != _gstUiCtx.react_power_ctrl_button.tmp_switch[_gstUiCtx.react_power_ctrl_button.select])
        {
            _gstUiCtx.react_power_ctrl_mode = _gstUiCtx.react_power_ctrl_button.tmp_mode;
            _UpdateSwitch(_gstUiCtx.react_power_ctrl_mode);
        }
        if(tim_get_systime() - _gstUiCtx.react_power_ctrl_button.key_time[_gstUiCtx.react_power_ctrl_button.select] > 3000)
        {
            _gstUiCtx.react_power_ctrl_button.key_stat[_gstUiCtx.react_power_ctrl_button.select] = 0;
            _gstUiCtx.react_power_ctrl_button.key_time[_gstUiCtx.react_power_ctrl_button.select] = tim_get_systime();
        }
    }
    else
    {
        if(_gstUiCtx.react_power_ctrl_mode != gstCollectInfo.power_data.power_set_info.reactive_ctrl_type)
        {
            _gstUiCtx.react_power_ctrl_mode = gstCollectInfo.power_data.power_set_info.reactive_ctrl_type;
            _UpdateSwitch(_gstUiCtx.react_power_ctrl_mode);
        }
    }
   
    //pv_switch
    if(_gstUiCtx.pv_button.key_stat == 1)
    {
        if(_gstUiCtx.pv_switch != _gstUiCtx.pv_button.tmp_switch)
        {
            _gstUiCtx.pv_switch = _gstUiCtx.pv_button.tmp_switch;
            _SetPVSwitchEnable(1, _gstUiCtx.pv_switch);
//            _SetCtrlMode(3, (EN_SWITCH_STAT_E)_gstUiCtx.pv_switch);
        }
        if(tim_get_systime() - _gstUiCtx.pv_button.key_time > 3000)
        {
            _gstUiCtx.pv_button.key_stat = 0;
        }
    }
    else
    {
        if(_gstUiCtx.pv_switch != gstUdpPcsSlavePara.pcs_info.control_info.pv_switch)
        {
            _gstUiCtx.pv_switch = gstUdpPcsSlavePara.pcs_info.control_info.pv_switch;
            _SetPVSwitchEnable(1, _gstUiCtx.pv_switch);
//            _SetCtrlMode(3, (EN_SWITCH_STAT_E)_gstUiCtx.pv_switch);
        }
    }
    //pf_switch
    if(_gstUiCtx.pf_button.key_stat == 1)
    {
        if(_gstUiCtx.pf_switch != _gstUiCtx.pf_button.tmp_switch)
        {
            _gstUiCtx.pf_switch = _gstUiCtx.pf_button.tmp_switch;
            _SetPFSwitchEnable(1, _gstUiCtx.pf_switch);
//            _SetCtrlMode(4, (EN_SWITCH_STAT_E)_gstUiCtx.pf_switch);
        }
        if(tim_get_systime() - _gstUiCtx.pf_button.key_time > 3000)
        {
           _gstUiCtx.pf_button.key_stat = 0; 
        }
    }
    else
    {
        if(_gstUiCtx.pf_switch != gstUdpPcsSlavePara.pcs_info.control_info.pf_switch)
        {
            _gstUiCtx.pf_switch = gstUdpPcsSlavePara.pcs_info.control_info.pf_switch;
            _SetPFSwitchEnable(1, _gstUiCtx.pf_switch);
//            _SetCtrlMode(4, (EN_SWITCH_STAT_E)_gstUiCtx.pf_switch);
        }
    }
    //ct dir
    if(_gstUiCtx.ct_dir_button.key_stat == 1)
    {
        if(_gstUiCtx.ct_dir_switch != _gstUiCtx.ct_dir_button.tmp_switch)
        {
            _gstUiCtx.ct_dir_switch = _gstUiCtx.ct_dir_button.tmp_switch;
            _SetCtDirectionStatus(1, _gstUiCtx.ct_dir_switch);
//            _SetCtrlMode(5, (EN_SWITCH_STAT_E)_gstUiCtx.ct_dir_switch);
        }
        if(tim_get_systime() - _gstUiCtx.ct_dir_button.key_time > 3000)
        {
            _gstUiCtx.ct_dir_button.key_stat = 0;
        }
    }
    else
    {
        if(_gstUiCtx.ct_dir_switch != gstCollectInfo.power_data.power_set_info.ct_dir)
        {
            _gstUiCtx.ct_dir_switch = gstCollectInfo.power_data.power_set_info.ct_dir;
            _SetCtDirectionStatus(1, _gstUiCtx.ct_dir_switch);
//            _SetCtrlMode(5, (EN_SWITCH_STAT_E)_gstUiCtx.ct_dir_switch);
        }
    }

    //net_conn
    if(_gstUiCtx.net_conn_button.key_stat == 1)
    {
        if(_gstUiCtx.net_is_conn_switch != _gstUiCtx.net_conn_button.tmp_switch)
        {
            _gstUiCtx.net_is_conn_switch = _gstUiCtx.net_conn_button.tmp_switch;
            _SetNetIsConnEnbale(1, _gstUiCtx.net_is_conn_switch);
        }
        if(tim_get_systime() - _gstUiCtx.net_conn_button.key_time > 3000)
        {
           _gstUiCtx.net_conn_button.key_stat = 0; 
        }
    }
    else
    {
        if(_gstUiCtx.net_is_conn_switch != (gstCollectInfo.power_data.power_set_info.certification_test_switch & 0x01))
        {
            _gstUiCtx.net_is_conn_switch = gstCollectInfo.power_data.power_set_info.certification_test_switch & 0x01;
            _SetNetIsConnEnbale(1, _gstUiCtx.net_is_conn_switch);
        }
    }
    //export_limit_enable
    if(_gstUiCtx.export_limit_button.key_stat == 1)
    {
        if(_gstUiCtx.hard_export_limit_switch != _gstUiCtx.export_limit_button.tmp_switch)
        {
            _gstUiCtx.hard_export_limit_switch = _gstUiCtx.export_limit_button.tmp_switch;
            _SetHardExportLimitEnbale(1, _gstUiCtx.hard_export_limit_switch);
        }
        if(tim_get_systime() - _gstUiCtx.export_limit_button.key_time > 3000)
        {
           _gstUiCtx.export_limit_button.key_stat = 0; 
        }
    }
    else
    {
        if(_gstUiCtx.hard_export_limit_switch != (gstCollectInfo.power_data.power_set_info.certification_test_switch & 0x02) >> 1)
        {
            _gstUiCtx.hard_export_limit_switch = (gstCollectInfo.power_data.power_set_info.certification_test_switch & 0x02) >> 1;
            _SetHardExportLimitEnbale(1, _gstUiCtx.hard_export_limit_switch);
        }
    }

    //soft_limit_enable
    if(_gstUiCtx.soft_limit_button.key_stat == 1)
    {
        if(_gstUiCtx.soft_export_limit_switch != _gstUiCtx.soft_limit_button.tmp_switch)
        {
            _gstUiCtx.soft_export_limit_switch = _gstUiCtx.soft_limit_button.tmp_switch;
            _SetSoftExportLimitEnbale(1, _gstUiCtx.soft_export_limit_switch);
        }
        if(tim_get_systime() - _gstUiCtx.soft_limit_button.key_time > 3000)
        {
           _gstUiCtx.soft_limit_button.key_stat = 0; 
        }
    }
    else
    {
        if(_gstUiCtx.soft_export_limit_switch != (gstCollectInfo.power_data.power_set_info.certification_test_switch & 0x04) >> 2)
        {
            _gstUiCtx.soft_export_limit_switch = (gstCollectInfo.power_data.power_set_info.certification_test_switch & 0x04) >> 2;
            _SetSoftExportLimitEnbale(1, _gstUiCtx.soft_export_limit_switch);
        }
    }

    //power_rate_enable
    if(_gstUiCtx.power_limit_button.key_stat == 1)
    {
        if(_gstUiCtx.power_rate_limit_switch != _gstUiCtx.power_limit_button.tmp_switch)
        {
            _gstUiCtx.power_rate_limit_switch = _gstUiCtx.power_limit_button.tmp_switch;
            _SetPowerRateLimitEnbale(1, _gstUiCtx.power_rate_limit_switch);
        }
        if(tim_get_systime() - _gstUiCtx.power_limit_button.key_time > 3000)
        {
           _gstUiCtx.power_limit_button.key_stat = 0; 
        }
    }
    else
    {
        if(_gstUiCtx.power_rate_limit_switch != (gstCollectInfo.power_data.power_set_info.certification_test_switch & 0x08) >> 3)
        {
            _gstUiCtx.power_rate_limit_switch = (gstCollectInfo.power_data.power_set_info.certification_test_switch & 0x08) >> 3;
            _SetPowerRateLimitEnbale(1, _gstUiCtx.power_rate_limit_switch);
        }
    }

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gEmListWorkingModeInfoPara.list_sel + _gEmListWorkingModeInfoPara.list_page >= _gEmListWorkingModeInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListWorkingModeInfoPara.list_sel + _gEmListWorkingModeInfoPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListWorkingModeInfoPara, gstEmWinListReactPowerCtrlMode.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListWorkingModeInfoPara, gstEmWinListReactPowerCtrlMode.win_handle);
        }
    }
#endif

}

/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
    uint8_t data_buff[256] = {0};
    uint16_t certification_switch = 0;
    uint16_t tmp = 0;
    int32_t event = EmGetKeyEvent(p_msg);
    if (event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }

    if (event == EN_KEY_SRC_LEFT)
    {
        --_gstUiCtx.select;
        if (_gstUiCtx.select < 0)
        {
            ++_gstUiCtx.select;
            return;
        }
        EmListUpward(&_gEmListWorkingModeInfoPara, gstEmWinListReactPowerCtrlMode.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gEmListWorkingModeInfoPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gEmListWorkingModeInfoPara, gstEmWinListReactPowerCtrlMode.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        switch(_gstUiCtx.select)
        {
            case 0:
            case 1:
            case 2:
            {
                _gstUiCtx.react_power_ctrl_button.select = _gstUiCtx.select;
                _UpdateSwitch(_gstUiCtx.select);

                tmp = BigLittleSwap16((uint16_t)_gstUiCtx.select);
                ModbusWriteSingleRemain(data_buff, 0x01, 0x634D, tmp);   //无功控制方式
                break;
            }
            case 3:
            {
                _gstUiCtx.pv_switch = (++_gstUiCtx.pv_switch) % 2;
                _gstUiCtx.pv_button.tmp_switch = _gstUiCtx.pv_switch;
                _gstUiCtx.pv_button.key_stat = 1;
                _gstUiCtx.pv_button.key_time = tim_get_systime();
                _SetPVSwitchEnable(1, _gstUiCtx.pv_switch);
//                _SetCtrlMode((uint8_t)_gstUiCtx.select, (EN_SWITCH_STAT_E)(_gstUiCtx.pv_button.tmp_switch));
                
                //tmp = BigLittleSwap16((uint16_t)_gstUiCtx.pv_switch);
                tmp = (uint16_t)_gstUiCtx.pv_switch;
                SetPcsActivePowerVoltCtrl(tmp);
                //ModbusWriteSingleRemain(data_buff, 0x01, 0x6238, tmp);   //PV开关使能
                break;
            }
            case 4:
            {
                _gstUiCtx.pf_switch = (++_gstUiCtx.pf_switch) % 2;
                _gstUiCtx.pf_button.tmp_switch = _gstUiCtx.pf_switch;
                _gstUiCtx.pf_button.key_stat = 1;    
                _gstUiCtx.pf_button.key_time = tim_get_systime();
                _SetPFSwitchEnable(1, _gstUiCtx.pf_switch);
//                _SetCtrlMode((uint8_t)_gstUiCtx.select, (EN_SWITCH_STAT_E)_gstUiCtx.pf_button.tmp_switch);

                //tmp = BigLittleSwap16((uint16_t)_gstUiCtx.pf_switch);
                tmp = (uint16_t)_gstUiCtx.pf_switch;
                SetPcsActivePowerFreqCtrl(tmp);
                //ModbusWriteSingleRemain(data_buff, 0x01, 0x6239, tmp);   //PF开关使能
                break;
            }
            case 5:
            {
                _gstUiCtx.ct_dir_switch = (++_gstUiCtx.ct_dir_switch) % 2;
                _gstUiCtx.ct_dir_button.tmp_switch = _gstUiCtx.ct_dir_switch;
                _gstUiCtx.ct_dir_button.key_stat = 1;
                _gstUiCtx.ct_dir_button.key_time = tim_get_systime();
                _SetCtDirectionStatus(1, _gstUiCtx.ct_dir_switch);
//                _SetCtrlMode((uint8_t)_gstUiCtx.select, (EN_SWITCH_STAT_E)(_gstUiCtx.ct_dir_button.tmp_switch));   
                
                tmp = BigLittleSwap16((uint16_t)_gstUiCtx.ct_dir_switch);
                ModbusWriteSingleRemain(data_buff, 0x01, 0x6339, tmp);   //CT DIR
                break;
            }
            case 6:
            {
                _gstUiCtx.net_is_conn_switch = (++_gstUiCtx.net_is_conn_switch) % 2;
                _gstUiCtx.net_conn_button.tmp_switch = _gstUiCtx.net_is_conn_switch;
                _gstUiCtx.net_conn_button.key_stat = 1;
                _gstUiCtx.net_conn_button.key_time = tim_get_systime();
                _SetNetIsConnEnbale(1, _gstUiCtx.net_is_conn_switch);

                certification_switch = gstCollectInfo.power_data.power_set_info.certification_test_switch;
                certification_switch &= ~(1<<0);
                certification_switch |= (uint16_t)(_gstUiCtx.net_is_conn_switch << 0);

                tmp = BigLittleSwap16(certification_switch);
                ModbusWriteSingleRemain(data_buff, 0x01, 0x63F0, tmp);
                break;
            }
            case 7:
            {
                _gstUiCtx.hard_export_limit_switch = (++_gstUiCtx.hard_export_limit_switch) % 2;
                _gstUiCtx.export_limit_button.tmp_switch = _gstUiCtx.hard_export_limit_switch;
                _gstUiCtx.export_limit_button.key_stat = 1;
                _gstUiCtx.export_limit_button.key_time = tim_get_systime();
                _SetHardExportLimitEnbale(1, _gstUiCtx.hard_export_limit_switch);

                certification_switch = gstCollectInfo.power_data.power_set_info.certification_test_switch;
                certification_switch &= ~(1<<1);
                certification_switch |= (uint16_t)(_gstUiCtx.hard_export_limit_switch << 1);

                tmp = BigLittleSwap16(certification_switch);
                ModbusWriteSingleRemain(data_buff, 0x01, 0x63F0, tmp);
                break;
            }
            case 8:
            {
                _gstUiCtx.soft_export_limit_switch = (++_gstUiCtx.soft_export_limit_switch) % 2;
                _gstUiCtx.soft_limit_button.tmp_switch = _gstUiCtx.soft_export_limit_switch;
                _gstUiCtx.soft_limit_button.key_stat = 1;
                _gstUiCtx.soft_limit_button.key_time = tim_get_systime();
                _SetSoftExportLimitEnbale(1, _gstUiCtx.soft_export_limit_switch);

                certification_switch = gstCollectInfo.power_data.power_set_info.certification_test_switch;
                certification_switch &= ~(1<<2);
                certification_switch |= (uint16_t)(_gstUiCtx.soft_export_limit_switch << 2);

                tmp = BigLittleSwap16(certification_switch);
                ModbusWriteSingleRemain(data_buff, 0x01, 0x63F0, tmp);
                break;
            }
            case 9:
            {
                _gstUiCtx.power_rate_limit_switch = (++_gstUiCtx.power_rate_limit_switch) % 2;
                _gstUiCtx.power_limit_button.tmp_switch = _gstUiCtx.power_rate_limit_switch;
                _gstUiCtx.power_limit_button.key_stat = 1;
                _gstUiCtx.power_limit_button.key_time = tim_get_systime();
                _SetPowerRateLimitEnbale(1, _gstUiCtx.power_rate_limit_switch);

                certification_switch = gstCollectInfo.power_data.power_set_info.certification_test_switch;
                certification_switch &= ~(1<<3);
                certification_switch |= (uint16_t)(_gstUiCtx.power_rate_limit_switch << 3);

                tmp = BigLittleSwap16(certification_switch);
                ModbusWriteSingleRemain(data_buff, 0x01, 0x63F0, tmp);
                break;
            }
            default:
                break;
        }
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
    {
        if(pCurrentEmWin->win_handle != 0)
        {
            emWin_t *pnext = NULL;
            if(p_msg->press_down_time >= gstEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
            {
                pnext = &gstEmWinHomePage;
            }
            else
            {
                pnext = &gstEmWinListSettingsInfo;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}

