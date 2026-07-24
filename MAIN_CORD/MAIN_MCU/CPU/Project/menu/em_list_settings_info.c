/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_LIST_SETTINGS_INFO.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-01-15
  * @brief       Settings界面
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

#define  EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE                (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListSettingsInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
} UiCtx_t;

static UiCtx_t _gstUiCtx;



/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;

static char _garrTitleBuf[] = "Settings";

/*List控件*/
static emListPara_t  _gstEmListSettingsInfoPara;

static char _garrListSettingsInfoLeftTextBuf[8][20];
static char _garrListSettingsInfoRightText3Buf[8];
static GUI_HMEM _gListSettingsInfoRightMem;
static uint8_t *_pListSettingsInfoRightBuf;


#define WIN_LIST_SETTINGS_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_SETTINGS_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListSettingsInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListSettingsInfoWin",    WIN_LIST_SETTINGS_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_SETTINGS_INFO_TITLE_ID,     31,  20, 144,  14,   0,   0x0,  0 },
};


/**
  * @brief ListSettingsInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListSettingsInfoWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0 ;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SETTINGS_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListSettingsInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListSettingsInfoPara, _gstEmListSettingsInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(i = 0; i < _gstEmListSettingsInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListSettingsInfoPara.sub_region[i].x_pos,
                                    _gstEmListSettingsInfoPara.sub_region[i].y_pos,
                                    _gstEmListSettingsInfoPara.sub_region[i].x_pos + _gstEmListSettingsInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListSettingsInfoPara.sub_region[i].y_pos + _gstEmListSettingsInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListSettingsInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListSettingsInfoPara, _gstEmListSettingsInfoPara.list_sel);
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
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListSettingsInfo.win_handle = GUI_CreateDialogBox(_garrListSettingsInfoDialogCreate, GUI_COUNTOF(_garrListSettingsInfoDialogCreate), _cbListSettingsInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListSettingsInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListSettingsInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListSettingsInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListSettingsInfoPara, gstEmWinListSettingsInfo.win_handle);
    return gstEmWinListSettingsInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListSettingsInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListSettingsInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListSettingsInfo.win_handle = 0;
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

    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListSettingsInfoPara.list_start_id = WIN_LIST_SETTINGS_INFO_ID + 0x30;
    _gstEmListSettingsInfoPara.list_num = 3;
    _gstEmListSettingsInfoPara.list_display_num = 6;
    if(_gstEmListSettingsInfoPara.list_num <= _gstEmListSettingsInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListSettingsInfoPara.list_num + 1 - _gstEmListSettingsInfoPara.list_display_num;
    }

    if(_gstEmListSettingsInfoPara.list_sel > _gstEmListSettingsInfoPara.list_num - 1)
    {
        _gstEmListSettingsInfoPara.list_sel = 0;
    }
    if(_gstEmListSettingsInfoPara.list_page > max_page - 1)
    {
        _gstEmListSettingsInfoPara.list_page = 0;
    }
    _gstEmListSettingsInfoPara.list_bk_color = 0x00787878;
    /*3个子列表*/
    for(i = 0; i < 3; i++)
    {
        _gstEmListSettingsInfoPara.sub_region[i].x_pos = 20;
        _gstEmListSettingsInfoPara.sub_region[i].y_pos = 38 + 42 * i;
        _gstEmListSettingsInfoPara.sub_region[i].x_size = 440;
        _gstEmListSettingsInfoPara.sub_region[i].y_size = 34;
    }

    /*List控件*/
    _gListSettingsInfoRightMem = GUI_ALLOC_AllocZero(EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE);
    _pListSettingsInfoRightBuf = GUI_ALLOC_h2p(_gListSettingsInfoRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListSettingsInfoRightBuf, EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE);
    
    //第一行左边
    _gstEmListSettingsInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSettingsInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListSettingsInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListSettingsInfoPara.sub_widget_para[0][0].x_size = 158;
    _gstEmListSettingsInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListSettingsInfoLeftTextBuf[0];
    _gstEmListSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Working Mode");
    //第一行右边
    _gstEmListSettingsInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListSettingsInfoPara.sub_widget_para[1][0].x_relative_pos = 415;
    _gstEmListSettingsInfoPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gstEmListSettingsInfoPara.sub_widget_para[1][0].x_size = 10;
    _gstEmListSettingsInfoPara.sub_widget_para[1][0].y_size = 5;
    _gstEmListSettingsInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gListSettingsInfoRightMem;
    _gstEmListSettingsInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _pListSettingsInfoRightBuf;
    _gstEmListSettingsInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE;
    //第二行左边
    _gstEmListSettingsInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSettingsInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListSettingsInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListSettingsInfoPara.sub_widget_para[0][1].x_size = 158;
    _gstEmListSettingsInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListSettingsInfoLeftTextBuf[1];
    _gstEmListSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "More Settings");
    //第二行右边
    _gstEmListSettingsInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListSettingsInfoPara.sub_widget_para[1][1].x_relative_pos = 415;
    _gstEmListSettingsInfoPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gstEmListSettingsInfoPara.sub_widget_para[1][1].x_size = 10;
    _gstEmListSettingsInfoPara.sub_widget_para[1][1].y_size = 5;
    _gstEmListSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListSettingsInfoRightMem;
    _gstEmListSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListSettingsInfoRightBuf;
    _gstEmListSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE;
    //第3行左边
    _gstEmListSettingsInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSettingsInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListSettingsInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListSettingsInfoPara.sub_widget_para[0][2].x_size = 158;
    _gstEmListSettingsInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListSettingsInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListSettingsInfoLeftTextBuf[2];
    _gstEmListSettingsInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSettingsInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSettingsInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Control Mode"); //无功控制模式
    //第3行右边
    _gstEmListSettingsInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListSettingsInfoPara.sub_widget_para[1][2].x_relative_pos = 415;
    _gstEmListSettingsInfoPara.sub_widget_para[1][2].y_relative_pos = 12;
    _gstEmListSettingsInfoPara.sub_widget_para[1][2].x_size = 10;
    _gstEmListSettingsInfoPara.sub_widget_para[1][2].y_size = 5;
    _gstEmListSettingsInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gListSettingsInfoRightMem;
    _gstEmListSettingsInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _pListSettingsInfoRightBuf;
    _gstEmListSettingsInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE;

    /*List选中框*/
    _gstEmListSettingsInfoPara.selected_unvalid_color = 0xFF000000;
    _gstEmListSettingsInfoPara.selected_valid_color = 0x00D6E400;
    _gstEmListSettingsInfoPara.selected_size = 3;
    _gstEmListSettingsInfoPara.selected_r = 10;
    _gstEmListSettingsInfoPara.selected_x_start_offset = -2;
    _gstEmListSettingsInfoPara.selected_y_start_offset = -2;
    _gstEmListSettingsInfoPara.selected_x_end_offset = 2;
    _gstEmListSettingsInfoPara.selected_y_end_offset = 2;
//    _gstEmListSettingsInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListSettingsInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    /*滚动条*/
//    _gstEmListSettingsInfoPara.scrollbar_enable = true;
//    _gstEmListSettingsInfoPara.scrollbar_color = 0x00D6E400;
//    _gstEmListSettingsInfoPara.scrollbar_bk_color = 0x00565656;
//    _gstEmListSettingsInfoPara.scrollbar_x_pos = 454;
//    _gstEmListSettingsInfoPara.scrollbar_y_pos = 39;
//    _gstEmListSettingsInfoPara.scrollbar_x_size = 6;
//    _gstEmListSettingsInfoPara.scrollbar_y_size = 241;

}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gListSettingsInfoRightMem);
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
    if(gstEmWinListSettingsInfo.win_handle == 0)
    {
        return;
    }
    
#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gstEmListSettingsInfoPara.list_sel + _gstEmListSettingsInfoPara.list_page >= _gstEmListSettingsInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListSettingsInfoPara.list_sel + _gstEmListSettingsInfoPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListSettingsInfoPara, gstEmWinListSettingsInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListSettingsInfoPara, gstEmWinListSettingsInfo.win_handle);
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
        EmListUpward(&_gstEmListSettingsInfoPara, gstEmWinListSettingsInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListSettingsInfoPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListSettingsInfoPara, gstEmWinListSettingsInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        emWin_t *pnext = NULL;
        switch(_gstUiCtx.select)
        {
            case 0:
            {
                pnext = &gstEmWinListWorkingModeInfo;
                break;
            }
            case 1:
            {
                pnext = &gstEmWinListADVSetPassword;
                break;
            }
            case 2:
            {
                pnext = &gstEmWinListReactPowerCtrlMode;
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
                pnext = &gstEmWinMain3DNav;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}


