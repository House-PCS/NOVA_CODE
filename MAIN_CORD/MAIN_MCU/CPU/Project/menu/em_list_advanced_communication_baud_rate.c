/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_communication_baud_rates.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-02-22
  * @brief       高级设置-通信参数-波特率
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-02-22    <td>V1.0.0    <td>           <td>InitialVersion
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


emWin_t gstEmWinListAdvanSettingCommunicateBaudRate = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


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
static emListPara_t _gstEmListLanguageTypePara;
static char _garrListLanguageTypeInfoLeftTextBuf[16][20];

#define WIN_LIST_LANGUAGE_TYPE_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_LANGUAGE_TYPE_TITLE_ID                         (GUI_ID_USER + 0x11)


static __SDRAM GUI_WIDGET_CREATE_INFO _garrListLanguageTypeDialogCreate[] =
{
    { WINDOW_CreateIndirect,  " ",               WIN_LIST_LANGUAGE_TYPE_ID,                     324,    122,     138,    148,    0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",           WIN_LIST_LANGUAGE_TYPE_TITLE_ID,               334,    122,     15,     15,     0,   0x0,  0 },
};

/*函数声明*/
extern void SetCommunBuadRateArrowDirection(void);
extern int32_t SetCurCommuBaudRate(int32_t idx);
extern void SetCommuBaudRateListShow(int32_t show);

/**
  * @brief  ListLanguageTypeInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListLanguageTypeInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_TYPE_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListLanguageTypePara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListLanguageTypePara, _gstEmListLanguageTypePara.list_pre_sel);

            GUI_RECT Rect;
            WM_GetClientRect(&Rect);
            GUI_SetColor(0x00424242);
            GUI_FillRoundedRect(Rect.x0, Rect.y0, Rect.x1, Rect.y1, 10);
            GUI_SetColor(0x00BDC909);
            GUI_DrawRoundedFrame(Rect.x0 + 1, Rect.y0 + 1, Rect.x1 - 1, Rect.y1 - 1, 10, 3);
#if 0
            GUI_SetColor(0x00565656);//上色

            for(i = 0; i < _gstEmListLanguageTypePara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListLanguageTypePara.sub_region[i].x_pos,
                                    _gstEmListLanguageTypePara.sub_region[i].y_pos,
                                    _gstEmListLanguageTypePara.sub_region[i].x_pos + _gstEmListLanguageTypePara.sub_region[i].x_size - 1,
                                    _gstEmListLanguageTypePara.sub_region[i].y_pos + _gstEmListLanguageTypePara.sub_region[i].y_size - 1,
                                    _gstEmListLanguageTypePara.selected_r);
            }
#endif   /*此段用于调节位置时使用*/
            SelectedDrawInWMPaint(&_gstEmListLanguageTypePara, _gstEmListLanguageTypePara.list_sel);
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
    _gstEmListLanguageTypePara.list_x_offset = _garrListLanguageTypeDialogCreate[0].x0;
    _gstEmListLanguageTypePara.list_y_offset = _garrListLanguageTypeDialogCreate[0].y0;
    gstEmWinListAdvanSettingCommunicateBaudRate.win_handle = GUI_CreateDialogBox(_garrListLanguageTypeDialogCreate, GUI_COUNTOF(_garrListLanguageTypeDialogCreate), _cbListLanguageTypeInfoWin,
            pCurrentEmWin->win_handle, 0, 0);
    //WINDOW_SetBkColor(gstEmWinListAdvanSettingCommunicateBaudRate.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvanSettingCommunicateBaudRate.win_handle, 0x00000000);
    WM_SetTransState(gstEmWinListAdvanSettingCommunicateBaudRate.win_handle, WM_CF_HASTRANS);
    SelectedDraw(&_gstEmListLanguageTypePara, gstEmWinListAdvanSettingCommunicateBaudRate.win_handle);
    GUI_Exec();
    return gstEmWinListAdvanSettingCommunicateBaudRate.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvanSettingCommunicateBaudRate.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvanSettingCommunicateBaudRate.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvanSettingCommunicateBaudRate.win_handle = 0;
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

    _gstEmListLanguageTypePara.list_start_id = WIN_LIST_LANGUAGE_TYPE_ID + 0x30;
    _gstEmListLanguageTypePara.list_num = 5;
    _gstEmListLanguageTypePara.list_display_num = 4;
    if(_gstEmListLanguageTypePara.list_num <= _gstEmListLanguageTypePara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListLanguageTypePara.list_num + 1 - _gstEmListLanguageTypePara.list_display_num;
    }

    if(_gstEmListLanguageTypePara.list_sel > _gstEmListLanguageTypePara.list_num - 1)
    {
        _gstEmListLanguageTypePara.list_sel = 0;
    }
    if(_gstEmListLanguageTypePara.list_page > max_page - 1)
    {
        _gstEmListLanguageTypePara.list_page = 0;
    }

    //_gstEmListLanguageTypePara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gstEmListLanguageTypePara.sub_region[0].x_pos = 12;
    _gstEmListLanguageTypePara.sub_region[0].y_pos = 12;
    _gstEmListLanguageTypePara.sub_region[0].x_size = 108;
    _gstEmListLanguageTypePara.sub_region[0].y_size = 25;
    _gstEmListLanguageTypePara.sub_region[1].x_pos = 12;
    _gstEmListLanguageTypePara.sub_region[1].y_pos = 44;
    _gstEmListLanguageTypePara.sub_region[1].x_size = 108;
    _gstEmListLanguageTypePara.sub_region[1].y_size = 25;
    _gstEmListLanguageTypePara.sub_region[2].x_pos = 12;
    _gstEmListLanguageTypePara.sub_region[2].y_pos = 76;
    _gstEmListLanguageTypePara.sub_region[2].x_size = 108;
    _gstEmListLanguageTypePara.sub_region[2].y_size = 25;
    _gstEmListLanguageTypePara.sub_region[3].x_pos = 12;
    _gstEmListLanguageTypePara.sub_region[3].y_pos = 108;
    _gstEmListLanguageTypePara.sub_region[3].x_size = 108;
    _gstEmListLanguageTypePara.sub_region[3].y_size = 25;
    _gstEmListLanguageTypePara.sub_region[4].x_pos = 12;
    _gstEmListLanguageTypePara.sub_region[4].y_pos = 140;
    _gstEmListLanguageTypePara.sub_region[4].x_size = 108;
    _gstEmListLanguageTypePara.sub_region[4].y_size = 25;

    /*List控件*/
    //第1行左边
    _gstEmListLanguageTypePara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListLanguageTypePara.sub_widget_para[0][0].x_relative_pos = 10;
    _gstEmListLanguageTypePara.sub_widget_para[0][0].y_relative_pos = 6;
    _gstEmListLanguageTypePara.sub_widget_para[0][0].x_size = 110;
    _gstEmListLanguageTypePara.sub_widget_para[0][0].y_size = 17;
    _gstEmListLanguageTypePara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListLanguageTypeInfoLeftTextBuf[0];
    _gstEmListLanguageTypePara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListLanguageTypePara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListLanguageTypePara.sub_widget_para[0][0].sub_widget.text_para.p_content, "9600bps");
    //第2行左边
    _gstEmListLanguageTypePara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListLanguageTypePara.sub_widget_para[0][1].x_relative_pos = 10;
    _gstEmListLanguageTypePara.sub_widget_para[0][1].y_relative_pos = 6;
    _gstEmListLanguageTypePara.sub_widget_para[0][1].x_size = 110;
    _gstEmListLanguageTypePara.sub_widget_para[0][1].y_size = 17;
    _gstEmListLanguageTypePara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListLanguageTypeInfoLeftTextBuf[1];
    _gstEmListLanguageTypePara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListLanguageTypePara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListLanguageTypePara.sub_widget_para[0][1].sub_widget.text_para.p_content, "19200bps");
    //第3行左边
    _gstEmListLanguageTypePara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListLanguageTypePara.sub_widget_para[0][2].x_relative_pos = 10;
    _gstEmListLanguageTypePara.sub_widget_para[0][2].y_relative_pos = 6;
    _gstEmListLanguageTypePara.sub_widget_para[0][2].x_size = 110;
    _gstEmListLanguageTypePara.sub_widget_para[0][2].y_size = 17;
    _gstEmListLanguageTypePara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListLanguageTypeInfoLeftTextBuf[2];
    _gstEmListLanguageTypePara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListLanguageTypePara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListLanguageTypePara.sub_widget_para[0][2].sub_widget.text_para.p_content, "38400bps");
    //第4行左边
    _gstEmListLanguageTypePara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListLanguageTypePara.sub_widget_para[0][3].x_relative_pos = 10;
    _gstEmListLanguageTypePara.sub_widget_para[0][3].y_relative_pos = 6;
    _gstEmListLanguageTypePara.sub_widget_para[0][3].x_size = 110;
    _gstEmListLanguageTypePara.sub_widget_para[0][3].y_size = 17;
    _gstEmListLanguageTypePara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListLanguageTypeInfoLeftTextBuf[3];
    _gstEmListLanguageTypePara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListLanguageTypePara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListLanguageTypePara.sub_widget_para[0][3].sub_widget.text_para.p_content, "57600bps");
    //第5行左边
    _gstEmListLanguageTypePara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListLanguageTypePara.sub_widget_para[0][4].x_relative_pos = 10;
    _gstEmListLanguageTypePara.sub_widget_para[0][4].y_relative_pos = 6;
    _gstEmListLanguageTypePara.sub_widget_para[0][4].x_size = 110;
    _gstEmListLanguageTypePara.sub_widget_para[0][4].y_size = 17;
    _gstEmListLanguageTypePara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListLanguageTypeInfoLeftTextBuf[4];
    _gstEmListLanguageTypePara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListLanguageTypePara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListLanguageTypePara.sub_widget_para[0][4].sub_widget.text_para.p_content, "115200bps");

    /*List选中框*/
    _gstEmListLanguageTypePara.selected_unvalid_color = 0xFF000000;
    _gstEmListLanguageTypePara.selected_valid_color = 0x00D6E400;
    _gstEmListLanguageTypePara.selected_size = 3;
    _gstEmListLanguageTypePara.selected_r = 5;
    _gstEmListLanguageTypePara.selected_x_start_offset = - 2;
    _gstEmListLanguageTypePara.selected_y_start_offset = -2;
    _gstEmListLanguageTypePara.selected_x_end_offset = 2;
    _gstEmListLanguageTypePara.selected_y_end_offset = 2;
    _gstEmListLanguageTypePara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    //_gstEmListLanguageTypePara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
#if 0
    /*滚动条*/
    _gstEmListLanguageTypePara.scrollbar_enable = true;
    _gstEmListLanguageTypePara.scrollbar_color = 0x00D6E400;
    _gstEmListLanguageTypePara.scrollbar_bk_color = 0x00565656;
    _gstEmListLanguageTypePara.scrollbar_x_pos = 454;
    _gstEmListLanguageTypePara.scrollbar_y_pos = 39;
    _gstEmListLanguageTypePara.scrollbar_x_size = 6;
    _gstEmListLanguageTypePara.scrollbar_y_size = 241;
#endif
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
    if(gstEmWinListAdvanSettingCommunicateBaudRate.win_handle == 0)
    {
        return;
    }
#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gstEmListLanguageTypePara.list_sel + _gstEmListLanguageTypePara.list_page >= _gstEmListLanguageTypePara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListLanguageTypePara.list_sel + _gstEmListLanguageTypePara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListLanguageTypePara, gstEmWinListAdvanSettingCommunicateBaudRate.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListLanguageTypePara, gstEmWinListAdvanSettingCommunicateBaudRate.win_handle);
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
        EmListUpward(&_gstEmListLanguageTypePara, gstEmWinListAdvanSettingCommunicateBaudRate.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListLanguageTypePara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListLanguageTypePara, gstEmWinListAdvanSettingCommunicateBaudRate.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        uint8_t data_buff[256 + 8] = {0};
        uint16_t tmp = 0;
        SetCurCommuBaudRate(_gstUiCtx.select);
        SetCommuBaudRateListShow(0);
        SetCommunBuadRateArrowDirection();
        tmp = BigLittleSwap16(_gstUiCtx.select);
        ModbusWriteSingleRemain(data_buff, 0x01, 0x502C, tmp);
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
    {
        SetCommuBaudRateListShow(0);
        SetCommunBuadRateArrowDirection();
        return;
    }
}


