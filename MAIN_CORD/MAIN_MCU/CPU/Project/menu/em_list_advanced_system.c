/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_system.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-15
  * @brief       高级设置->系统设置信息
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-15    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"

#define  TEST_DYNAMIC                            0

#define  EM_LIST_ICON_ARROW_RIGHT_SIZE                      (4096)
#define  EM_LIST_ICON_SWITCH_SIZE                           (4096)
#define  EM_LIST_ICON_ARROW_DOWN_SIZE                       (4096)

#define  EN_BTN_NUM                              7

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListAdvanceSystem = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _sys_button_stru_t
{
    uint32_t tmp_switch;
    uint32_t key_stat;
    uint32_t key_time;
} SysButton_t;


typedef struct _ui_ctx_stru_t
{
    int32_t pv_mode_list_show;
    int32_t select;
    int32_t key_init;
    uint32_t switcher[EN_BTN_NUM];
    uint16_t drm_enable;
    uint16_t drm_enable_tmp;
    uint16_t anti_reverse_flow_enable;
    uint16_t backup_enable;
    uint16_t pv_input_mode_idx;
    uint16_t xboost_enable;  //等价于power_pro
    uint16_t zeroms_ups_enable;
    uint16_t generaltor_enable;
    uint16_t shut_down_enable;
    uint16_t shut_down_tmp_enable;
    SysButton_t sys_button[EN_BTN_NUM];
} UiCtx_t;

static UiCtx_t _gstUiCtx;


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static IMAGE_Handle  _gImgSwithHandle;//开关图标句柄
static IMAGE_Handle _gImgArrowHandle;//箭头图标句柄

static emWidgetTextPara_t  _gstTitleTextPara;
static char _garrTitleBuf[] = "Setting < More < System";

/*List控件*/
static emListPara_t  _gstEmListAdvanceSystemPara;

static char _garrListAdvanceSystemLeftTextBuf[11][26];
static char _garrListAdvanceSystemRightText2Buf[2][20];
static GUI_HMEM _gListAdvanceSystemRightMem;
static uint8_t *_pListAdvanceSystemRightBuf;

/*开关图片参数*/
static emWidgetImgPara_t  _garrSwtichImgPara[2];  /*0:关   1:开*/
static emWidgetImgPara_t  _garrArrowImgPara[2];  /*0:右    1:下*/

/*开关枚举和函数声明*/
typedef enum
{
    EN_SWITCH_STAT_OFF = 0,
    EN_SWITCH_STAT_ON,
} EN_SWITCH_STAT_E;

typedef struct _pv_input_mode_idx_stru_t
{
    uint16_t index;
    const char mode[16];
} PvInputMode_t;

static PvInputMode_t PvInputModeIdx[] =
{
    {0, "Independ"},
    {1, "Parallel"},
};


#define WIN_LIST_ADVANCED_SYSTEM_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_ADVANCED_SYSTEM_TITLE_ID                         (GUI_ID_USER + 0x11)


static __SDRAM GUI_WIDGET_CREATE_INFO _garrListAdvancedSystemDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListAdvancedSystemWin",    WIN_LIST_ADVANCED_SYSTEM_ID,       0,     0,      480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_ADVANCED_SYSTEM_TITLE_ID,           31,    20,     400,  14,   0,   0x0,  0 },
};


/*函数声明*/
void SetPVInputModeArrowDir(void);
int32_t SetCurPVInputMode(int32_t index);


static int32_t _SetAntiBackFlowStatus(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];

    _gstUiCtx.anti_reverse_flow_enable = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;

    if(ui_updatae)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][0], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}

static int32_t _SetDRMStatus(uint32_t ui_update, uint16_t enable)
{
    uint16_t enable_tmp = 0;
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];

    //_gstUiCtx.drm_enable = enable;
    if(enable == 0x00 || enable == 0x10)
    {
        enable_tmp = 0x00;
    }
    else
    {
        enable_tmp = 0x01;
        _gstUiCtx.drm_enable = 0x01;
    }
    switch (enable_tmp)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    if(ui_update && _gstEmListAdvanceSystemPara.list_page == 0)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][1], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_update && _gstEmListAdvanceSystemPara.list_page == 1)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][0], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}

static int32_t _SetBackUpLoadStatus(uint32_t ui_update, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];

    _gstUiCtx.backup_enable = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    if(ui_update && _gstEmListAdvanceSystemPara.list_page == 0)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][2], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_update && _gstEmListAdvanceSystemPara.list_page == 1)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][1], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_update && _gstEmListAdvanceSystemPara.list_page == 2)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][0], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}

static int32_t _SetPvInputMode(int32_t ui_update, int32_t idx)
{
    snprintf(_garrListAdvanceSystemRightText2Buf[0], sizeof(_garrListAdvanceSystemRightText2Buf[0]), "%s", PvInputModeIdx[idx].mode);
    if(ui_update)
    {
        TEXT_SetText(_gstEmListAdvanceSystemPara.handle[2][1], (const char *)_gstEmListAdvanceSystemPara.sub_widget_para[2][1].sub_widget.text_para.p_content);
    }

    return 0;
}

static int32_t _AdvancedSysSetSwitchStat(uint32_t row, EN_SWITCH_STAT_E stat)
{

    emWidgetImgPara_t  *tmp_switch_img_para = &_garrSwtichImgPara[0];
    switch (stat)
    {
        case EN_SWITCH_STAT_OFF:
        {
            tmp_switch_img_para = &_garrSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            tmp_switch_img_para = &_garrSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gstEmListAdvanceSystemPara.sub_widget_para[1][row].sub_widget.img_para.mem_img = tmp_switch_img_para->mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][row].sub_widget.img_para.p_img_buf = tmp_switch_img_para->p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][row].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][row], tmp_switch_img_para->p_img_buf,
                 tmp_switch_img_para->img_size);
    return 0;
}


static int32_t _SetXBoostFunction(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];

    _gstUiCtx.xboost_enable = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;

    if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 0)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][3], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 1)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][2], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 2)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][1], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 3)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][0], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    
    return 0;
}

static int32_t _Set0MsUpsFunction(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];

    _gstUiCtx.zeroms_ups_enable = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;

    if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 0)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][4], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 1)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][3], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 2)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][2], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 3)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][1], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}
static int32_t _SetShutDownEnable(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];

    _gstUiCtx.shut_down_enable = enable;
    
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    
    if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 0)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][5], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 1)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][4], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 2)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][3], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 3)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][2], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}
static int32_t _SetGeneraltorEnable(uint32_t ui_updatae, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];

    _gstUiCtx.generaltor_enable = enable;
    switch (enable)
    {
        case EN_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    
    if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 1)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][5], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 2)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][4], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    else if(ui_updatae && _gstEmListAdvanceSystemPara.list_page == 3)
    {
        IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][3], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}

/**
  * @brief ListAdvancedSystemWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbAdvancedSystemWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    static int32_t switch_type = 0;
    static int32_t row = 0;
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_SYSTEM_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListAdvanceSystemPara, pMsg->hWin);
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
            _AdvancedSysSetSwitchStat(0, (EN_SWITCH_STAT_E)(switch_type % 2));

            WM_RestartTimer(pMsg->Data.v, 300);/*暂定300mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListAdvanceSystemPara, _gstEmListAdvanceSystemPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListAdvanceSystemPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListAdvanceSystemPara.sub_region[i].x_pos,
                                    _gstEmListAdvanceSystemPara.sub_region[i].y_pos,
                                    _gstEmListAdvanceSystemPara.sub_region[i].x_pos + _gstEmListAdvanceSystemPara.sub_region[i].x_size - 1,
                                    _gstEmListAdvanceSystemPara.sub_region[i].y_pos + _gstEmListAdvanceSystemPara.sub_region[i].y_size - 1,
                                    _gstEmListAdvanceSystemPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListAdvanceSystemPara, _gstEmListAdvanceSystemPara.list_sel);
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
    pPrevEmWin = &gstEmWinListAdvancedSettings;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListAdvanceSystem.win_handle = GUI_CreateDialogBox(_garrListAdvancedSystemDialogCreate, GUI_COUNTOF(_garrListAdvancedSystemDialogCreate),
                                           _cbAdvancedSystemWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListAdvanceSystem.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvanceSystem.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListAdvanceSystem.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListAdvanceSystemPara, gstEmWinListAdvanceSystem.win_handle);
    return gstEmWinListAdvanceSystem.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvanceSystem.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvanceSystem.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvanceSystem.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;
    uint8_t i = 0;
    EmKeyDefaultInfoSet();

    _gstUiCtx.anti_reverse_flow_enable = gstCollectInfo.power_data.power_set_info.prohibit_reverse_flow_enable;
    _gstUiCtx.drm_enable = gstCollectInfo.power_data.power_set_info.drm_enable;
    _gstUiCtx.backup_enable = gstCollectInfo.power_data.power_set_info.backup_enable;
    _gstUiCtx.xboost_enable = gstCollectInfo.power_data.power_set_info.x_boost_enable; 
    _gstUiCtx.zeroms_ups_enable = gstCollectInfo.power_data.power_set_info.ups_0_delay_enable;  
    _gstUiCtx.generaltor_enable = gstCollectInfo.power_data.power_set_info.gen_mode; 
    _gstUiCtx.shut_down_enable = gstUdpPcsSlavePara.pcs_info.control_info.remote_shutdown;    
    
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    /*开关图标*/
    _garrSwtichImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_SWITCH_SIZE);
    _garrSwtichImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrSwtichImgPara[0].mem_img);
    _garrSwtichImgPara[0].img_size = EM_LIST_ICON_SWITCH_SIZE;
    SpiFlashReadData(EM_ICON_SET_CLOSE_ADDR, _garrSwtichImgPara[0].p_img_buf, _garrSwtichImgPara[0].img_size);

    _garrSwtichImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_SWITCH_SIZE);
    _garrSwtichImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrSwtichImgPara[1].mem_img);
    _garrSwtichImgPara[1].img_size = EM_LIST_ICON_SWITCH_SIZE;
    SpiFlashReadData(EM_ICON_SET_OPEN_ADDR, _garrSwtichImgPara[1].p_img_buf, _garrSwtichImgPara[1].img_size);

    /*右箭头图标*/
    _gListAdvanceSystemRightMem = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_RIGHT_SIZE);
    _pListAdvanceSystemRightBuf = GUI_ALLOC_h2p(_gListAdvanceSystemRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE);


    _gstEmListAdvanceSystemPara.list_start_id = WIN_LIST_ADVANCED_SYSTEM_ID + 0x30;
    _gstEmListAdvanceSystemPara.list_num = 9;
    _gstEmListAdvanceSystemPara.list_display_num = 6;
    if(_gstEmListAdvanceSystemPara.list_num <= _gstEmListAdvanceSystemPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListAdvanceSystemPara.list_num + 1 - _gstEmListAdvanceSystemPara.list_display_num;
    }

    if(_gstEmListAdvanceSystemPara.list_sel > _gstEmListAdvanceSystemPara.list_num - 1)
    {
        _gstEmListAdvanceSystemPara.list_sel = 0;
    }
    if(_gstEmListAdvanceSystemPara.list_page > max_page - 1)
    {
        _gstEmListAdvanceSystemPara.list_page = 0;
    }
    _gstEmListAdvanceSystemPara.list_bk_color = 0x00787878;

    /*List选中框*/
    /*6个子列表*/
    for(i = 0; i < 6; i++)
    {
        _gstEmListAdvanceSystemPara.sub_region[i].x_pos = 20;
        _gstEmListAdvanceSystemPara.sub_region[i].y_pos = 38 + i * 42;
        _gstEmListAdvanceSystemPara.sub_region[i].x_size = 420;
        _gstEmListAdvanceSystemPara.sub_region[i].y_size = 34;
    }  
    
    /*List控件*/

    //第一行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][0].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[0];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Anti-backflow");
    //第一行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].x_relative_pos = 386 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].y_relative_pos = 9;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].x_size = 40;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].y_size = 31;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;  
    //第2行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][1].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[1];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "DRM");
    //第2行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].x_relative_pos = 386 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].y_relative_pos = 9;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].x_size = 40;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].y_size = 31;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第3行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][2].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[2];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Backup");
    //第3行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].x_relative_pos = 386 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].y_relative_pos = 9;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].x_size = 40;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].y_size = 31;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第4行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[3];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Power PRO");
    //第4行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].x_relative_pos = 386 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].y_relative_pos = 9;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].x_size = 40;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].y_size = 31;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第5行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[4];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "0ms UPS");
    //第5行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].x_relative_pos = 386 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].y_relative_pos = 9;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].x_size = 40;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].y_size = 31;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第6行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[5];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Remote Shutdown");
    //第6行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].x_relative_pos = 386 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].y_relative_pos = 9;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].x_size = 40;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].y_size = 31;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第7行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[6];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Generator");
    //第7行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].x_relative_pos = 386 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].y_relative_pos = 9;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].x_size = 40;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].y_size = 31;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第8行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].x_size = 188;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[7];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Generator Max Power");
   //第8行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].x_relative_pos = 394;//415;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].y_relative_pos = 12;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].x_size = 5;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].y_size = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_RIGHT_SIZE;
    //第9行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][8].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][8].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][8].x_size = 260;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][8].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][8].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[8];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][8].sub_widget.text_para.p_content, "Grid Reconnection Time");
    //第9行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][8].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][8].x_relative_pos = 415 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][8].y_relative_pos = 12;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][8].x_size = 5;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][8].y_size = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][8].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][8].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][8].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_RIGHT_SIZE;

/*  
  //第8行中间文字
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].x_relative_pos = 328 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].x_size = 80;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].sub_widget.text_para.p_content = _garrListAdvanceSystemRightText2Buf[0];
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[2][1].sub_widget.text_para.p_content, " ");
    snprintf(_garrListAdvanceSystemRightText2Buf[0], sizeof(_garrListAdvanceSystemRightText2Buf[0]), "%s", PvInputModeIdx[_gstUiCtx.pv_input_mode_idx].mode);    
*/
 
#if 0
    //第4行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].x_size = 200;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[3];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Time To Reconnect Grid");
    //第4行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].x_relative_pos = 415 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].y_relative_pos = 12;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].x_size = 5;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].y_size = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_RIGHT_SIZE;
    //第5行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[4];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Power PRO");
    //第5行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].x_relative_pos = 386 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].y_relative_pos = 9;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].x_size = 40;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].y_size = 31;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第6行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[5];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "0ms UPS");
    //第6行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].x_relative_pos = 386 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].y_relative_pos = 9;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].x_size = 40;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].y_size = 31;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第7行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[6];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Generator");
    //第7行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].x_relative_pos = 386 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].y_relative_pos = 9;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].x_size = 40;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].y_size = 31;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第8行左边
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].x_relative_pos = 11;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].x_size = 158;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _garrListAdvanceSystemLeftTextBuf[7];
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Generator Power");
   //第8行右边
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].x_relative_pos = 394;//415;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].y_relative_pos = 12;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].x_size = 5;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].y_size = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
    _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_RIGHT_SIZE;
/*  
  //第8行中间文字
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].x_relative_pos = 328 - 21;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].y_relative_pos = 10;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].x_size = 80;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].y_size = 17;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].sub_widget.text_para.p_content = _garrListAdvanceSystemRightText2Buf[0];
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceSystemPara.sub_widget_para[2][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    strcpy(_gstEmListAdvanceSystemPara.sub_widget_para[2][1].sub_widget.text_para.p_content, " ");
    snprintf(_garrListAdvanceSystemRightText2Buf[0], sizeof(_garrListAdvanceSystemRightText2Buf[0]), "%s", PvInputModeIdx[_gstUiCtx.pv_input_mode_idx].mode);    
*/

#endif
    /*List选中框*/
    _gstEmListAdvanceSystemPara.selected_unvalid_color = 0xFF000000;
    _gstEmListAdvanceSystemPara.selected_valid_color = 0x00D6E400;
    _gstEmListAdvanceSystemPara.selected_size = 3;
    _gstEmListAdvanceSystemPara.selected_r = 10;
    _gstEmListAdvanceSystemPara.selected_x_start_offset = -2;
    _gstEmListAdvanceSystemPara.selected_y_start_offset = -2;
    _gstEmListAdvanceSystemPara.selected_x_end_offset = 2;
    _gstEmListAdvanceSystemPara.selected_y_end_offset = 2;
//    _gstEmListAdvanceSystemPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListAdvanceSystemPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    /*滚动条*/
    _gstEmListAdvanceSystemPara.scrollbar_enable = true;
    _gstEmListAdvanceSystemPara.scrollbar_color = 0x00D6E400;
    _gstEmListAdvanceSystemPara.scrollbar_bk_color = 0x00565656;
    _gstEmListAdvanceSystemPara.scrollbar_x_pos = 457;
    _gstEmListAdvanceSystemPara.scrollbar_y_pos = 39;
    _gstEmListAdvanceSystemPara.scrollbar_x_size = 6;
    _gstEmListAdvanceSystemPara.scrollbar_y_size = 240;


    _SetAntiBackFlowStatus(0, _gstUiCtx.anti_reverse_flow_enable);
    _SetDRMStatus(0, _gstUiCtx.drm_enable);
    _SetBackUpLoadStatus(0, _gstUiCtx.backup_enable);
    _SetXBoostFunction(0, _gstUiCtx.xboost_enable);
    _Set0MsUpsFunction(0, _gstUiCtx.zeroms_ups_enable);
    _SetGeneraltorEnable(0, _gstUiCtx.generaltor_enable);
    _SetShutDownEnable(0, _gstUiCtx.shut_down_enable);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gListAdvanceSystemRightMem);
    GUI_ALLOC_Free(_garrSwtichImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrSwtichImgPara[1].mem_img);
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
    if(gstEmWinListAdvanceSystem.win_handle == 0)
    {
        return;
    }

    //anti
    if(_gstUiCtx.sys_button[0].key_stat == 1)
    {
        if(_gstUiCtx.anti_reverse_flow_enable != _gstUiCtx.sys_button[0].tmp_switch)
        {
            _gstUiCtx.anti_reverse_flow_enable = _gstUiCtx.sys_button[0].tmp_switch;
            _SetAntiBackFlowStatus(1, _gstUiCtx.anti_reverse_flow_enable);
        }
        if(tim_get_systime() - _gstUiCtx.sys_button[0].key_time > 3000)
        {
            _gstUiCtx.sys_button[0].key_stat = 0;
        }
    }
    else
    {
        if(_gstUiCtx.anti_reverse_flow_enable != gstCollectInfo.power_data.power_set_info.prohibit_reverse_flow_enable)
        {
            _gstUiCtx.anti_reverse_flow_enable = gstCollectInfo.power_data.power_set_info.prohibit_reverse_flow_enable;
            _SetAntiBackFlowStatus(1, _gstUiCtx.anti_reverse_flow_enable);
        }
    }

    //drm
    if(_gstUiCtx.sys_button[1].key_stat == 1)
    {
        if(_gstUiCtx.drm_enable != _gstUiCtx.sys_button[1].tmp_switch)
        {
            _gstUiCtx.drm_enable = _gstUiCtx.sys_button[1].tmp_switch;
            _SetDRMStatus(1, _gstUiCtx.drm_enable);
        }
        if(tim_get_systime() - _gstUiCtx.sys_button[1].key_time > 3000)
        {
            _gstUiCtx.sys_button[1].key_stat = 0;
        }
    }
    else
    {
        if(_gstUiCtx.drm_enable_tmp != gstCollectInfo.power_data.power_set_info.drm_enable)
        {
            _gstUiCtx.drm_enable_tmp = gstCollectInfo.power_data.power_set_info.drm_enable;
            _SetDRMStatus(1, _gstUiCtx.drm_enable_tmp);
        }
    }

    //back_up
    if(_gstUiCtx.sys_button[2].key_stat == 1)
    {
        if(_gstUiCtx.backup_enable != _gstUiCtx.sys_button[2].tmp_switch)
        {
            _gstUiCtx.backup_enable = _gstUiCtx.sys_button[2].tmp_switch;
            _SetBackUpLoadStatus(1, _gstUiCtx.backup_enable);
        }
        if(tim_get_systime() - _gstUiCtx.sys_button[2].key_time > 3000)
        {
            _gstUiCtx.sys_button[2].key_stat = 0;
        }
    }
    else
    {
        if(_gstUiCtx.backup_enable != gstCollectInfo.power_data.power_set_info.backup_enable)
        {
            _gstUiCtx.backup_enable = gstCollectInfo.power_data.power_set_info.backup_enable;
            _SetBackUpLoadStatus(1, _gstUiCtx.backup_enable);
        }
    }

    if(_gstUiCtx.pv_input_mode_idx != gstCollectInfo.power_data.power_set_info.pv_input_mode)
    {
        _gstUiCtx.pv_input_mode_idx = gstCollectInfo.power_data.power_set_info.pv_input_mode;
        _SetPvInputMode(1, _gstUiCtx.pv_input_mode_idx);
    } 

    
    //XBOOST
    if(_gstUiCtx.sys_button[3].key_stat == 1)
    {
        if(_gstUiCtx.xboost_enable != _gstUiCtx.sys_button[3].tmp_switch)
        {
            _gstUiCtx.xboost_enable = _gstUiCtx.sys_button[3].tmp_switch;
            _SetXBoostFunction(1, _gstUiCtx.xboost_enable);
        }
        if(tim_get_systime() - _gstUiCtx.sys_button[3].key_time > 3000)
        {
            _gstUiCtx.sys_button[3].key_stat = 0;
        }
    }
    else
    {
        if(_gstUiCtx.xboost_enable != gstCollectInfo.power_data.power_set_info.x_boost_enable)
        {
            _gstUiCtx.xboost_enable = gstCollectInfo.power_data.power_set_info.x_boost_enable; 
            _SetXBoostFunction(1, _gstUiCtx.xboost_enable);
        }
    }

    //0ms ups
    if(_gstUiCtx.sys_button[4].key_stat == 1)
    {
        if(_gstUiCtx.zeroms_ups_enable != _gstUiCtx.sys_button[4].tmp_switch)
        {
            _gstUiCtx.zeroms_ups_enable = _gstUiCtx.sys_button[4].tmp_switch;
            _Set0MsUpsFunction(1, _gstUiCtx.zeroms_ups_enable);
        }
        if(tim_get_systime() - _gstUiCtx.sys_button[4].key_time > 3000)
        {
            _gstUiCtx.sys_button[4].key_stat = 0;
        }
    }
    else
    {
        if(_gstUiCtx.zeroms_ups_enable != gstCollectInfo.power_data.power_set_info.ups_0_delay_enable)
        {
            _gstUiCtx.zeroms_ups_enable = gstCollectInfo.power_data.power_set_info.ups_0_delay_enable;  
            _Set0MsUpsFunction(1, _gstUiCtx.zeroms_ups_enable);
        }
    }
    //远程关机
    if(_gstUiCtx.sys_button[5].key_stat == 1)
    {
        if(_gstUiCtx.shut_down_enable != _gstUiCtx.sys_button[5].tmp_switch)
        {
            _gstUiCtx.shut_down_enable = _gstUiCtx.sys_button[5].tmp_switch;
            _SetShutDownEnable(1, _gstUiCtx.shut_down_enable);
        }
        if(tim_get_systime() - _gstUiCtx.sys_button[5].key_time > 3000)
        {
            _gstUiCtx.sys_button[5].key_stat = 0;
        }
    }
    else
    {
        if(_gstUiCtx.shut_down_enable != gstUdpPcsSlavePara.pcs_info.control_info.remote_shutdown)
        {
            _gstUiCtx.shut_down_enable = gstUdpPcsSlavePara.pcs_info.control_info.remote_shutdown;
            _SetShutDownEnable(1, _gstUiCtx.shut_down_enable);
        }
    }
    //油机
    if(_gstUiCtx.sys_button[6].key_stat == 1)
    {
        if(_gstUiCtx.generaltor_enable != _gstUiCtx.sys_button[6].tmp_switch)
        {
            _gstUiCtx.generaltor_enable = _gstUiCtx.sys_button[6].tmp_switch;
            _SetGeneraltorEnable(1, _gstUiCtx.generaltor_enable);
        }
        if(tim_get_systime() - _gstUiCtx.sys_button[6].key_time > 3000)
        {
            _gstUiCtx.sys_button[6].key_stat = 0;
        }
    }
    else
    {
        if(_gstUiCtx.generaltor_enable != gstCollectInfo.power_data.power_set_info.gen_mode)
        {
            _gstUiCtx.generaltor_enable = gstCollectInfo.power_data.power_set_info.gen_mode;
            _SetGeneraltorEnable(1, _gstUiCtx.generaltor_enable);
        }
    }

#if 0
    if(GetSystick() - timecount > 100)    //30
    {
        timecount = GetSystick();
        if(_gstEmListAdvanceSystemPara.list_sel + _gstEmListAdvanceSystemPara.list_page >= _gstEmListAdvanceSystemPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListAdvanceSystemPara.list_sel + _gstEmListAdvanceSystemPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListAdvanceSystemPara, gstEmWinListAdvanceSystem.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListAdvanceSystemPara, gstEmWinListAdvanceSystem.win_handle);
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
    int32_t event = 0;
    event = EmGetKeyEvent(p_msg);
    if(_gstUiCtx.pv_mode_list_show)
    {
        gstEmWinListSysPVInputMode.key_handler(p_msg);

        if(!_gstUiCtx.pv_mode_list_show)
        {
            gstEmWinListSysPVInputMode.delete_win();
        }
        return;
    }

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
        EmListUpward(&_gstEmListAdvanceSystemPara, gstEmWinListAdvanceSystem.win_handle);

        if (_gstEmListAdvanceSystemPara.list_num - _gstUiCtx.select - 1
                == _gstEmListAdvanceSystemPara.list_display_num)
        {
            WM_Paint(gstEmWinListAdvanceSystem.win_handle);
        } 
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if(_gstUiCtx.select >= _gstEmListAdvanceSystemPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListAdvanceSystemPara, gstEmWinListAdvanceSystem.win_handle);
        
        if (_gstUiCtx.select == _gstEmListAdvanceSystemPara.list_num - 1)
        {
            WM_Paint(gstEmWinListAdvanceSystem.win_handle);
        }
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        emWin_t *pnext = NULL;
        switch(_gstUiCtx.select)
        {
            case 0:
            {
                _gstUiCtx.switcher[0] = (++_gstUiCtx.switcher[0]) % 2;
                _gstUiCtx.sys_button[0].tmp_switch = _gstUiCtx.switcher[0];
                _gstUiCtx.sys_button[0].key_stat = 1;
                _gstUiCtx.sys_button[0].key_time = tim_get_systime();
//                _AdvancedSysSetSwitchStat(_gstUiCtx.select,(EN_SWITCH_STAT_E)_gstUiCtx.switcher[0]);
                _SetAntiBackFlowStatus(1, (EN_SWITCH_STAT_E)_gstUiCtx.switcher[0]);
                uint8_t data_anti[256 + 8] = {0};
                uint16_t anti_enable = 0;
                anti_enable = _gstUiCtx.switcher[0];
                anti_enable = BigLittleSwap16(anti_enable);
                ModbusWriteSingleRemain(data_anti, 0x01, 0x6323, anti_enable);
            }
            break;
            case 1:
            {
                _gstUiCtx.switcher[1] = (++_gstUiCtx.switcher[1]) % 2;
                _gstUiCtx.sys_button[1].tmp_switch = _gstUiCtx.switcher[1];
                _gstUiCtx.sys_button[1].key_stat = 1;
                _gstUiCtx.sys_button[1].key_time = tim_get_systime();
//                _AdvancedSysSetSwitchStat(_gstUiCtx.select, (EN_SWITCH_STAT_E)_gstUiCtx.switcher[1]);
                if(_gstUiCtx.switcher[1] == 0x01)
                {
                    _gstUiCtx.drm_enable_tmp = 0xFF;
                }
                else
                {
                    _gstUiCtx.drm_enable_tmp = 0x00;
                }
                _SetDRMStatus(1, _gstUiCtx.drm_enable_tmp);
                uint8_t data_drm[256 + 8] = {0};
                uint16_t drm_enable = 0;
                drm_enable = _gstUiCtx.drm_enable_tmp;
                drm_enable = BigLittleSwap16(drm_enable);
                ModbusWriteSingleRemain(data_drm, 0x01, 0x6331, drm_enable);
            }
            break;
            case 2:
            {
                _gstUiCtx.switcher[2] = (++_gstUiCtx.switcher[2]) % 2;
                _gstUiCtx.sys_button[2].tmp_switch = _gstUiCtx.switcher[2];
                _gstUiCtx.sys_button[2].key_stat = 1;
                _gstUiCtx.sys_button[2].key_time = tim_get_systime();
//                _AdvancedSysSetSwitchStat(_gstUiCtx.select, (EN_SWITCH_STAT_E)_gstUiCtx.switcher[2]);
                _SetBackUpLoadStatus(1, (EN_SWITCH_STAT_E)_gstUiCtx.switcher[2]);
                uint8_t data_esp[256 + 8] = {0};
                uint16_t esp_enable = 0;
                esp_enable = _gstUiCtx.switcher[2];
                esp_enable = BigLittleSwap16(esp_enable);
                ModbusWriteSingleRemain(data_esp, 0x01, 0x6328, esp_enable);
            }
            break;
            case 3:
            {
                _gstUiCtx.switcher[3] = (++_gstUiCtx.switcher[3]) % 2;
                _gstUiCtx.sys_button[3].tmp_switch = _gstUiCtx.switcher[3];
                _gstUiCtx.sys_button[3].key_stat = 1;
                _gstUiCtx.sys_button[3].key_time = tim_get_systime();
                _SetXBoostFunction(1, (EN_SWITCH_STAT_E)_gstUiCtx.switcher[3]);

                uint8_t data_anti[256 + 8] = {0};
                uint16_t xboost = 0;
                xboost = _gstUiCtx.switcher[3];
                xboost = BigLittleSwap16(xboost);
                ModbusWriteSingleRemain(data_anti, 0x01, 0x6326, xboost);
            }
            break;
            case 4:
            {
                _gstUiCtx.switcher[4] = (++_gstUiCtx.switcher[4]) % 2;
                _gstUiCtx.sys_button[4].tmp_switch = _gstUiCtx.switcher[4];
                _gstUiCtx.sys_button[4].key_stat = 1;
                _gstUiCtx.sys_button[4].key_time = tim_get_systime();
                _Set0MsUpsFunction(1, (EN_SWITCH_STAT_E)_gstUiCtx.switcher[4]);

                uint8_t data_anti[256 + 8] = {0};
                uint16_t delay_ups_0ms = 0;
                delay_ups_0ms = _gstUiCtx.switcher[4];
                delay_ups_0ms = BigLittleSwap16(delay_ups_0ms);
                ModbusWriteSingleRemain(data_anti, 0x01, 0x6327, delay_ups_0ms);
            }
            break;
            case 5:
            {
                _gstUiCtx.switcher[5] = (++_gstUiCtx.switcher[5]) % 2;
                _gstUiCtx.sys_button[5].tmp_switch = _gstUiCtx.switcher[5];
                _gstUiCtx.sys_button[5].key_stat = 1;
                _gstUiCtx.sys_button[5].key_time = tim_get_systime();
                _SetShutDownEnable(1, (EN_SWITCH_STAT_E)_gstUiCtx.switcher[5]);
                uint8_t data_tmp[256 + 8] = {0};
                uint16_t shutdown_enable = 0;
                shutdown_enable = _gstUiCtx.switcher[5];
//                shutdown_enable = BigLittleSwap16(shutdown_enable);
                SetPcsRemoteShutdown(shutdown_enable);
                // ModbusWriteSingleRemain(data_tmp, 0x01, 0x6301, generaltor_enable);
                break;
            }
            case 6:
            {
                _gstUiCtx.switcher[6] = (++_gstUiCtx.switcher[6]) % 2;
                _gstUiCtx.sys_button[6].tmp_switch = _gstUiCtx.switcher[6];
                _gstUiCtx.sys_button[6].key_stat = 1;
                _gstUiCtx.sys_button[6].key_time = tim_get_systime();
                _SetGeneraltorEnable(1, (EN_SWITCH_STAT_E)_gstUiCtx.switcher[6]);
                uint8_t data_tmp[256 + 8] = {0};
                uint16_t generaltor_enable = 0;
                generaltor_enable = _gstUiCtx.switcher[6];
                generaltor_enable = BigLittleSwap16(generaltor_enable);
                ModbusWriteSingleRemain(data_tmp, 0x01, 0x6301, generaltor_enable);
                break;
            }
            case 7:
            {
                pnext = &gstEmWinListSystemGeneratorPower;
                #if 0
                if(!_gstUiCtx.pv_mode_list_show)
                {
                    gstEmWinListSysPVInputMode.create_win();
                    _gstUiCtx.pv_mode_list_show = 1;
                }
                SetPVInputModeArrowDir();
                #endif
                break;
            }
            case 8:
            {
                pnext = &gstEmWinListReconnGridTime;
                break;
            }
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
#if 0
        uint8_t data_anti[256 + 8] = {0};
        uint8_t data_drm[256 + 8] = {0};
        uint8_t data_esp[256 + 8] = {0};

        uint16_t anti_enable = 0;
        uint16_t drm_enable = 0;
        uint16_t esp_enable = 0;

        anti_enable = _gstUiCtx.switcher[0];
        drm_enable = _gstUiCtx.switcher[1];
        esp_enable = _gstUiCtx.switcher[2];


        anti_enable = BigLittleSwap16(anti_enable);
        ModbusWriteSingleRemain(data_anti, 0x01, 0x6204, anti_enable);

        drm_enable = BigLittleSwap16(drm_enable);
        ModbusWriteSingleRemain(data_drm, 0x01, 0x6203, drm_enable);

        esp_enable = BigLittleSwap16(esp_enable);
        ModbusWriteSingleRemain(data_esp, 0x01, 0x620C, esp_enable);

#endif

        if(pCurrentEmWin->win_handle != 0)
        {
            emWin_t *pnext = NULL;
            if(p_msg->press_down_time >= gstEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
            {
                pnext = &gstEmWinHomePage;
            }
            else
            {
                pnext = &gstEmWinListAdvancedSettings;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
    }
}


void SetPVInputModeArrowDir(void)
{
    if(_gstUiCtx.pv_mode_list_show)
    {
        /*向上箭头图标*/
        if(_gstEmListAdvanceSystemPara.list_page == 2)
        {
            SpiFlashReadData(EM_ICON_UP_ARROW_ADDR, _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE);

            _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
            _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
            _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_DOWN_SIZE;
            IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][5], _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE);
        }
    }
    else
    {
        /*向右箭头图标*/
        if(_gstEmListAdvanceSystemPara.list_page == 2)
        {
            SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE);

            _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
            _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
            _gstEmListAdvanceSystemPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_DOWN_SIZE;
            IMAGE_SetDTA(_gstEmListAdvanceSystemPara.handle[1][5], _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE);
        }
    }
}

void SetPVInputModeListShow(int32_t show)
{
    _gstUiCtx.pv_mode_list_show = show;
}

int32_t SetCurPVInputMode(int32_t index)
{
    if(_gstEmListAdvanceSystemPara.list_page == 2)
    {
        snprintf(_gstEmListAdvanceSystemPara.sub_widget_para[2][7].sub_widget.text_para.p_content,
                 sizeof(_garrListAdvanceSystemRightText2Buf[0]), "%s", PvInputModeIdx[index].mode);

        TEXT_SetText(_gstEmListAdvanceSystemPara.handle[2][5], (const char *)_gstEmListAdvanceSystemPara.sub_widget_para[2][7].sub_widget.text_para.p_content);
    }
    return 0;
}
