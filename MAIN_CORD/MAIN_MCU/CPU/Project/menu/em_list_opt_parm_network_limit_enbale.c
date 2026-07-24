/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_system.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-15
  * @brief       高级设置->设置->more->Opt PARM->入户测功率限制
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

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListAdvanSettingHouseholdLimitEnable = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _sys_button_stru_t
{
    uint32_t tmp_switch;
    uint32_t key_stat;
    uint32_t key_time;
} SysButton_t;


typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint32_t switcher[1];

    uint16_t household_power_enable;
    uint16_t household_enable_last_status;
    SysButton_t sys_button[1];
} UiCtx_t;

static UiCtx_t _gstUiCtx;


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static IMAGE_Handle  _gImgSwithHandle;//开关图标句柄
static IMAGE_Handle _gImgArrowHandle;//箭头图标句柄

static emWidgetTextPara_t  _gstTitleTextPara;
static char _garrTitleBuf[] = "Settings < More < Opt PARM < Import Limitation";

/*List控件*/
static emListPara_t  _gstEmListAdvanOptHouseholdLimitPara;

static char _garrListAdvanOptHouseholdLimitLeftTextBuf[2][26];
static char _garrListAdvanOptHouseholdLimitRightText2Buf[2][20];


/*开关图片参数*/
static emWidgetImgPara_t  _garrSwtichImgPara[2];  /*0:关   1:开*/
static emWidgetImgPara_t  _garrArrowImgPara[2];  /*0:右    1:下*/

/*开关枚举和函数声明*/
typedef enum
{
    EN_SWITCH_STAT_OFF = 0,
    EN_SWITCH_STAT_ON,
} EN_SWITCH_STAT_E;


#define WIN_LIST_ADVANCED_NETWORK_LIMIT_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_ADVANCED_NETWORK_LIMIT_TITLE_ID                         (GUI_ID_USER + 0x11)


static const GUI_WIDGET_CREATE_INFO _garrListAdvanOptHouseholdLimitDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListAdvanOptHouseholdLimitWin",    WIN_LIST_ADVANCED_NETWORK_LIMIT_ID,       0,     0,      480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_ADVANCED_NETWORK_LIMIT_TITLE_ID,           31,    20,     400,  14,   0,   0x0,  0 },
};




static int32_t _SetNetworkPowerLimitEnable(uint8_t ui_update, uint16_t enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];

    _gstUiCtx.household_power_enable = enable;
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

    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;

    if(ui_update)
    {
        IMAGE_SetDTA(_gstEmListAdvanOptHouseholdLimitPara.handle[1][0], p_st_tmp_switch_img_para->p_img_buf,
                     p_st_tmp_switch_img_para->img_size);
    }
    return 0;
}

/**
  * @brief ListAdvanOptHouseholdLimitWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbAdvanOptHouseholdLimitWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_NETWORK_LIMIT_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListAdvanOptHouseholdLimitPara, pMsg->hWin);
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
//            SelectedDel2(&_gstEmListAdvanOptHouseholdLimitPara, _gstEmListAdvanOptHouseholdLimitPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListAdvanOptHouseholdLimitPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListAdvanOptHouseholdLimitPara.sub_region[i].x_pos,
                                    _gstEmListAdvanOptHouseholdLimitPara.sub_region[i].y_pos,
                                    _gstEmListAdvanOptHouseholdLimitPara.sub_region[i].x_pos + _gstEmListAdvanOptHouseholdLimitPara.sub_region[i].x_size - 1,
                                    _gstEmListAdvanOptHouseholdLimitPara.sub_region[i].y_pos + _gstEmListAdvanOptHouseholdLimitPara.sub_region[i].y_size - 1,
                                    _gstEmListAdvanOptHouseholdLimitPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListAdvanOptHouseholdLimitPara, _gstEmListAdvanOptHouseholdLimitPara.list_sel);
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
    pPrevEmWin = &gstEmWinListOptPARMInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle = GUI_CreateDialogBox(_garrListAdvanOptHouseholdLimitDialogCreate, GUI_COUNTOF(_garrListAdvanOptHouseholdLimitDialogCreate),
                                           _cbAdvanOptHouseholdLimitWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListAdvanOptHouseholdLimitPara, gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle);
    return gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle = 0;
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
     
    //_gstUiCtx.household_power_enable = gstCollectInfo.power_data.inside_info.network_power_limit_enable;
    
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



    _gstEmListAdvanOptHouseholdLimitPara.list_start_id = WIN_LIST_ADVANCED_NETWORK_LIMIT_ID + 0x30;
    _gstEmListAdvanOptHouseholdLimitPara.list_num = 1;
    _gstEmListAdvanOptHouseholdLimitPara.list_display_num = 6;
    if(_gstEmListAdvanOptHouseholdLimitPara.list_num <= _gstEmListAdvanOptHouseholdLimitPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListAdvanOptHouseholdLimitPara.list_num + 1 - _gstEmListAdvanOptHouseholdLimitPara.list_display_num;
    }

    if(_gstEmListAdvanOptHouseholdLimitPara.list_sel > _gstEmListAdvanOptHouseholdLimitPara.list_num - 1)
    {
        _gstEmListAdvanOptHouseholdLimitPara.list_sel = 0;
    }
    if(_gstEmListAdvanOptHouseholdLimitPara.list_page > max_page - 1)
    {
        _gstEmListAdvanOptHouseholdLimitPara.list_page = 0;
    }
    _gstEmListAdvanOptHouseholdLimitPara.list_bk_color = 0x00787878;

    /*List选中框*/
    /*6个子列表*/
    for(i = 0; i < 1; i++)
    {
        _gstEmListAdvanOptHouseholdLimitPara.sub_region[i].x_pos = 20;
        _gstEmListAdvanOptHouseholdLimitPara.sub_region[i].y_pos = 38 + i * 42;
        _gstEmListAdvanOptHouseholdLimitPara.sub_region[i].x_size = 420;
        _gstEmListAdvanOptHouseholdLimitPara.sub_region[i].y_size = 34;
    }  
    
    /*List控件*/

    //第一行左边
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[0][0].x_size = 260;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListAdvanOptHouseholdLimitLeftTextBuf[0];
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Import Limitation Enable");
    //第一行右边
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].x_relative_pos = 386 - 21;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].y_relative_pos = 9;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].x_size = 40;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].y_size = 31;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvanOptHouseholdLimitPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    
    /*List选中框*/
    _gstEmListAdvanOptHouseholdLimitPara.selected_unvalid_color = 0xFF000000;
    _gstEmListAdvanOptHouseholdLimitPara.selected_valid_color = 0x00D6E400;
    _gstEmListAdvanOptHouseholdLimitPara.selected_size = 3;
    _gstEmListAdvanOptHouseholdLimitPara.selected_r = 10;
    _gstEmListAdvanOptHouseholdLimitPara.selected_x_start_offset = -2;
    _gstEmListAdvanOptHouseholdLimitPara.selected_y_start_offset = -2;
    _gstEmListAdvanOptHouseholdLimitPara.selected_x_end_offset = 2;
    _gstEmListAdvanOptHouseholdLimitPara.selected_y_end_offset = 2;
//    _gstEmListAdvanOptHouseholdLimitPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListAdvanOptHouseholdLimitPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    /*滚动条*/
    _gstEmListAdvanOptHouseholdLimitPara.scrollbar_enable = true;
    _gstEmListAdvanOptHouseholdLimitPara.scrollbar_color = 0x00D6E400;
    _gstEmListAdvanOptHouseholdLimitPara.scrollbar_bk_color = 0x00565656;
    _gstEmListAdvanOptHouseholdLimitPara.scrollbar_x_pos = 457;
    _gstEmListAdvanOptHouseholdLimitPara.scrollbar_y_pos = 39;
    _gstEmListAdvanOptHouseholdLimitPara.scrollbar_x_size = 6;
    _gstEmListAdvanOptHouseholdLimitPara.scrollbar_y_size = 240;


    _SetNetworkPowerLimitEnable(0, _gstUiCtx.household_power_enable);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
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
    if(gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle == 0)
    {
        return;
    }

//    //network power limit
//    if(_gstUiCtx.sys_button[0].key_stat == 1)
//    {
//        if(_gstUiCtx.household_power_enable != _gstUiCtx.sys_button[0].tmp_switch)
//        {
//            _gstUiCtx.household_power_enable = _gstUiCtx.sys_button[0].tmp_switch;
//            _SetNetworkPowerLimitEnable(1, _gstUiCtx.household_power_enable);
//        }
//        if(tim_get_systime() - _gstUiCtx.sys_button[0].key_time > 3000)
//        {
//            _gstUiCtx.sys_button[0].key_stat = 0;
//        }
//    }
//    else
//    {
//        if(_gstUiCtx.household_power_enable != gstCollectInfo.power_data.inside_info.network_power_limit_enable)
//        {
//            _gstUiCtx.household_power_enable = gstCollectInfo.power_data.inside_info.network_power_limit_enable;
//            _SetNetworkPowerLimitEnable(1, _gstUiCtx.household_power_enable);
//        }
//    }

#if 0
    if(GetSystick() - timecount > 100)    //30
    {
        timecount = GetSystick();
        if(_gstEmListAdvanOptHouseholdLimitPara.list_sel + _gstEmListAdvanOptHouseholdLimitPara.list_page >= _gstEmListAdvanOptHouseholdLimitPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListAdvanOptHouseholdLimitPara.list_sel + _gstEmListAdvanOptHouseholdLimitPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListAdvanOptHouseholdLimitPara, gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListAdvanOptHouseholdLimitPara, gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle);
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
        EmListUpward(&_gstEmListAdvanOptHouseholdLimitPara, gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle);

        if (_gstEmListAdvanOptHouseholdLimitPara.list_num - _gstUiCtx.select - 1
                == _gstEmListAdvanOptHouseholdLimitPara.list_display_num)
        {
            WM_Paint(gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle);
        }
        
        
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if(_gstUiCtx.select >= _gstEmListAdvanOptHouseholdLimitPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListAdvanOptHouseholdLimitPara, gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle);
        
        if (_gstUiCtx.select == _gstEmListAdvanOptHouseholdLimitPara.list_num - 1)
        {
            WM_Paint(gstEmWinListAdvanSettingHouseholdLimitEnable.win_handle);
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
                _gstUiCtx.household_enable_last_status = _gstUiCtx.switcher[0];
                _gstUiCtx.switcher[0] = (++_gstUiCtx.switcher[0]) % 2;
                _gstUiCtx.sys_button[0].tmp_switch = _gstUiCtx.switcher[0];
                _gstUiCtx.sys_button[0].key_stat = 1;
                _gstUiCtx.sys_button[0].key_time = tim_get_systime();
                
                _SetNetworkPowerLimitEnable(1, (EN_SWITCH_STAT_E)_gstUiCtx.switcher[0]);
                uint8_t data_anti[256 + 8] = {0};
                uint16_t household_power_enable = 0;
                household_power_enable = _gstUiCtx.switcher[0];
                household_power_enable = BigLittleSwap16(household_power_enable);
               // ModbusWriteSingleRemain(data_anti, 0x01, 0x620B, household_power_enable);
                            
                if(_gstUiCtx.household_enable_last_status == 0)
                {
                    pPrevEmWin = pCurrentEmWin;
                    pCurrentEmWin->delete_win();
                    pCurrentEmWin = &gstEmWinListAdvanSettingHouseholdGridPower;
                    pCurrentEmWin->create_win();
                }
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
        if(pCurrentEmWin->win_handle != 0)
        {
            emWin_t *pnext = NULL;
            if(p_msg->press_down_time >= gstEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
            {
                pnext = &gstEmWinHomePage;
            }
            else
            {
                pnext = &gstEmWinListOptPARMInfo;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
    }
}
