/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_system_pv_input_mode.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-02-19
  * @brief       高级设置-系统设置-PV_input_mode
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-02-19    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinListSysPVInputMode = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
} UiCtx_t;

static UiCtx_t _gstUiCtx;
/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;
static char _garrTitleBuf[] = " ";

/*List控件*/
static emListPara_t  _gstEmListSysPVInputModePara;
static char _garrListSysPVInputModeInfoLeftTextBuf[2][20];
static char _garrListSysPVInputModeInfoRightText3Buf[8];


#define WIN_LIST_SYS_PV_INPUT_MODE_ID                              (GUI_ID_USER + 0x10)
#define WIN_LIST_SYS_PV_INPUT_MODE_TITLE_ID                        (GUI_ID_USER + 0x11)


static __SDRAM GUI_WIDGET_CREATE_INFO _garrListSysPVInputModeDialogCreate[] =
{
    { WINDOW_CreateIndirect,  " ",               WIN_LIST_SYS_PV_INPUT_MODE_ID,                     315,    120,     149,    98,    0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",           WIN_LIST_SYS_PV_INPUT_MODE_TITLE_ID,               311,    114,     15,     15,    0,   0x0,  0 },
};

/*函数声明*/
extern void SetPVInputModeListShow(int32_t show);
extern int32_t SetCurPVInputMode(int32_t index);
extern void SetPVInputModeArrowDir(void);

/**
  * @brief  ListSysPVInputModeInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListSysPVInputModeInfoWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SYS_PV_INPUT_MODE_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListSysPVInputModePara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListSysPVInputModePara, _gstEmListSysPVInputModePara.list_pre_sel);
            GUI_RECT Rect;
            WM_GetClientRect(&Rect);
            GUI_SetColor(0x00424242);
            GUI_FillRoundedRect(Rect.x0, Rect.y0, Rect.x1, Rect.y1, 10);
            GUI_SetColor(0x00BDC909);
            GUI_DrawRoundedFrame(Rect.x0 + 1, Rect.y0 + 1, Rect.x1 - 1, Rect.y1 - 1, 10, 3);
#if 0
            GUI_SetColor(0x00565656);//上色

            for(i = 0; i < _gstEmListSysPVInputModePara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListSysPVInputModePara.sub_region[i].x_pos,
                                    _gstEmListSysPVInputModePara.sub_region[i].y_pos,
                                    _gstEmListSysPVInputModePara.sub_region[i].x_pos + _gstEmListSysPVInputModePara.sub_region[i].x_size - 1,
                                    _gstEmListSysPVInputModePara.sub_region[i].y_pos + _gstEmListSysPVInputModePara.sub_region[i].y_size - 1,
                                    _gstEmListSysPVInputModePara.selected_r);
            }
#endif   /*此段用于调节位置时使用*/
            SelectedDrawInWMPaint(&_gstEmListSysPVInputModePara, _gstEmListSysPVInputModePara.list_sel);
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
    GUI_EnableAlpha(1);
#if 1
    _gstEmListSysPVInputModePara.list_x_offset = _garrListSysPVInputModeDialogCreate[0].x0;
    _gstEmListSysPVInputModePara.list_y_offset = _garrListSysPVInputModeDialogCreate[0].y0;
#endif
    gstEmWinListSysPVInputMode.win_handle = GUI_CreateDialogBox(_garrListSysPVInputModeDialogCreate, GUI_COUNTOF(_garrListSysPVInputModeDialogCreate), _cbListSysPVInputModeInfoWin,
                                          pCurrentEmWin->win_handle, 0, 0);
    //WINDOW_SetBkColor(gstEmWinListSysPVInputMode.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListSysPVInputMode.win_handle, 0x00000000);
    WM_SetTransState(gstEmWinListSysPVInputMode.win_handle, WM_CF_HASTRANS);
    SelectedDraw(&_gstEmListSysPVInputModePara, gstEmWinListSysPVInputMode.win_handle);
    GUI_Exec();
    return gstEmWinListSysPVInputMode.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListSysPVInputMode.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListSysPVInputMode.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListSysPVInputMode.win_handle = 0;
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

    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstEmListSysPVInputModePara.list_start_id = WIN_LIST_SYS_PV_INPUT_MODE_ID + 0x30;
    _gstEmListSysPVInputModePara.list_num = 2;
    _gstEmListSysPVInputModePara.list_display_num = 2;
    if(_gstEmListSysPVInputModePara.list_num <= _gstEmListSysPVInputModePara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListSysPVInputModePara.list_num + 1 - _gstEmListSysPVInputModePara.list_display_num;
    }

    if(_gstEmListSysPVInputModePara.list_sel > _gstEmListSysPVInputModePara.list_num - 1)
    {
        _gstEmListSysPVInputModePara.list_sel = 0;
    }
    if(_gstEmListSysPVInputModePara.list_page > max_page - 1)
    {
        _gstEmListSysPVInputModePara.list_page = 0;
    }

    //_gstEmListSysPVInputModePara.list_bk_color = 0x00787878;
    /*2个子列表*/
    _gstEmListSysPVInputModePara.sub_region[0].x_pos = 12;
    _gstEmListSysPVInputModePara.sub_region[0].y_pos = 10;
    _gstEmListSysPVInputModePara.sub_region[0].x_size = 125;
    _gstEmListSysPVInputModePara.sub_region[0].y_size = 25;
    _gstEmListSysPVInputModePara.sub_region[1].x_pos = 12;
    _gstEmListSysPVInputModePara.sub_region[1].y_pos = 52;
    _gstEmListSysPVInputModePara.sub_region[1].x_size = 125;
    _gstEmListSysPVInputModePara.sub_region[1].y_size = 25;
    /*List控件*/

    //第1行左边
    _gstEmListSysPVInputModePara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSysPVInputModePara.sub_widget_para[0][0].x_relative_pos = 7;
    _gstEmListSysPVInputModePara.sub_widget_para[0][0].y_relative_pos = 6;
    _gstEmListSysPVInputModePara.sub_widget_para[0][0].x_size = 104;
    _gstEmListSysPVInputModePara.sub_widget_para[0][0].y_size = 17;
    _gstEmListSysPVInputModePara.sub_widget_para[0][0].align = EM_TEXT_USER_ALIGN;
    _gstEmListSysPVInputModePara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListSysPVInputModeInfoLeftTextBuf[0];
    _gstEmListSysPVInputModePara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSysPVInputModePara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSysPVInputModePara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Independ");

    //第2行左边
    _gstEmListSysPVInputModePara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSysPVInputModePara.sub_widget_para[0][1].x_relative_pos = 7;
    _gstEmListSysPVInputModePara.sub_widget_para[0][1].y_relative_pos = 6;
    _gstEmListSysPVInputModePara.sub_widget_para[0][1].x_size = 104;
    _gstEmListSysPVInputModePara.sub_widget_para[0][1].y_size = 17;
    _gstEmListSysPVInputModePara.sub_widget_para[0][1].align = EM_TEXT_USER_ALIGN;
    _gstEmListSysPVInputModePara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListSysPVInputModeInfoLeftTextBuf[1];
    _gstEmListSysPVInputModePara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSysPVInputModePara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSysPVInputModePara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Parallel");

    /*List选中框*/
    _gstEmListSysPVInputModePara.selected_unvalid_color = 0xFF000000;
    _gstEmListSysPVInputModePara.selected_valid_color = 0x00D6E400;
    _gstEmListSysPVInputModePara.selected_size = 3;
    _gstEmListSysPVInputModePara.selected_r = 5;
    _gstEmListSysPVInputModePara.selected_x_start_offset = 2;
    _gstEmListSysPVInputModePara.selected_y_start_offset = -2;
    _gstEmListSysPVInputModePara.selected_x_end_offset = -2;
    _gstEmListSysPVInputModePara.selected_y_end_offset = 2;
    _gstEmListSysPVInputModePara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    //_gstEmListSysPVInputModePara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

//    /*滚动条*/
//    _gstEmListSysPVInputModePara.scrollbar_enable = true;
//    _gstEmListSysPVInputModePara.scrollbar_color = 0x00D6E400;
//    _gstEmListSysPVInputModePara.scrollbar_bk_color = 0x00565656;
//    _gstEmListSysPVInputModePara.scrollbar_x_pos = 454;
//    _gstEmListSysPVInputModePara.scrollbar_y_pos = 39;
//    _gstEmListSysPVInputModePara.scrollbar_x_size = 6;
//    _gstEmListSysPVInputModePara.scrollbar_y_size = 241;
}


/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{

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
    if(gstEmWinListSysPVInputMode.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gstEmListSysPVInputModePara.list_sel + _gstEmListSysPVInputModePara.list_page >= _gstEmListSysPVInputModePara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListSysPVInputModePara.list_sel + _gstEmListSysPVInputModePara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListSysPVInputModePara, gstEmWinListSysPVInputMode.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListSysPVInputModePara, gstEmWinListSysPVInputMode.win_handle);
        }
    }

//    if(pop_up_stat == 0)
//    {
//        if(GetSystick() - pop_timecount > 1000)
//        {
//            pop_up_stat = 1;
//            pop_timecount = GetSystick();
//            gstPopUpWinError.create_win();
//        }
//    }
//    else
//    {
//        if(GetSystick() - pop_timecount > 1000)
//        {
//            pop_up_stat = 0;
//            pop_timecount = GetSystick();
//            gstPopUpWinError.delete_win();
//        }
//    }

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
        EmListUpward(&_gstEmListSysPVInputModePara, gstEmWinListSysPVInputMode.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListSysPVInputModePara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListSysPVInputModePara, gstEmWinListSysPVInputMode.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        uint8_t arr_data_buff[256 + 8] = {0};
        uint16_t tmp = 0;

        SetCurPVInputMode(_gstUiCtx.select);
        SetPVInputModeListShow(0);
        SetPVInputModeArrowDir();

        tmp = BigLittleSwap16(_gstUiCtx.select);
        ModbusWriteSingleRemain(arr_data_buff, 0x01, 0x6322, tmp);
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
    {
        SetPVInputModeListShow(0);
        SetPVInputModeArrowDir();
        return;
    }
}


