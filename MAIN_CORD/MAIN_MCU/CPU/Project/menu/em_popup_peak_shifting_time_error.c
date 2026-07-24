/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_popup_start_and_end_time_error.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-07-03
  * @brief       开始时间大于结束时间错误弹窗
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-07-03    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"


#define  EM_POP_UP_START_AND_END_TIME_ICON_ALERT_SIZE                (4096)

#define WIN_LIST_OPERA_INFO_ID                               (GUI_ID_USER + 0x50)

#define REC_NO_XPOS             (130)
#define REC_NO_YPOS             (60)
#define REC_YES_XPOS            (190)
#define REC_YES_YPOS            (60)


typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
} UiCtx_t;


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstPopUpPeakShiftingTimeErrorWin = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

static UiCtx_t _gstUiCtx;
static emPopUpPara_t _gstEmPopUpStartAndEndTimePara;

static GUI_HMEM _gPopUpErrorStartAndEndTimeTitleMem;
static uint8_t *_pPopUpErrorStartAndEndTimeTitleBuf;
static char _garrPopUpErrorStartAndEndTimeTitleBuf[60];
static char _garrPopUpErrorStartAndEndTimeYESBuf[5];
static char _garrPopUpErrorStartAndEndTimeNOBuf[5];


/**
  * @brief  返回到削峰填谷界面
  * @param  NULL
  * @retval NULL
  */
static void _Err1BackPeakShifiting(void)
{
    emWin_t *pnext = NULL;
    pnext = &gstEmWinListSavedSetTimeInfo;
    if(pCurrentEmWin->win_handle != 0)
    {
        pPrevEmWin = pCurrentEmWin;
        pCurrentEmWin->delete_win();
        pCurrentEmWin = pnext;
        pCurrentEmWin->create_win();
    }
}

/**
  * @brief  返回到工作模式界面
  * @param  NULL
  * @retval NULL
  */
static void _Err1BackWorkMode(void)
{
    emWin_t *pnext = NULL;
    pnext = &gstEmWinListWorkingModeInfo;
    if(pCurrentEmWin->win_handle != 0)
    {
        pPrevEmWin = pCurrentEmWin;
        pCurrentEmWin->delete_win();
        pCurrentEmWin = pnext;
        pCurrentEmWin->create_win();
    }
}


/**
  * @brief  绘制选中框
  * @param  NULL
  * @retval NULL
  */
static void _DrawSelPopUpRecPaint(int32_t sel)
{
    if(sel == 0)
    {
        //no
        GUI_SetColor(0x00FFFFFF);
        GUI_FillRoundedRect(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + 54 - 1,  REC_NO_YPOS + 18 - 1, 3);
        GUI_SetColor(0x00000000);
        GUI_DrawRoundedFrame(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + 54 - 1,  REC_NO_YPOS + 18 - 1, 3, 1);
        //yes
        GUI_SetColor(0x00A4A4A4);
        GUI_FillRoundedRect(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + 54 - 1,  REC_YES_YPOS + 18 - 1, 3);
//      GUI_SetColor(0x00000000);
//      GUI_DrawRoundedFrame(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + 54 - 1,  REC_YES_YPOS + 18 - 1, 3,1);
    }
    else if(sel == 1)
    {
        //no
        GUI_SetColor(0x00A4A4A4);
        GUI_FillRoundedRect(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + 54 - 1,  REC_NO_YPOS + 18 - 1, 3);
//      GUI_SetColor(0x00000000);
//      GUI_DrawRoundedFrame(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + 54 - 1,  REC_NO_YPOS + 18 - 1, 3,1);
        //yes
        GUI_SetColor(0x00FFFFFF);
        GUI_FillRoundedRect(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + 54 - 1,  REC_YES_YPOS + 18 - 1, 3);
        GUI_SetColor(0x00000000);
        GUI_DrawRoundedFrame(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + 54 - 1,  REC_YES_YPOS + 18 - 1, 3, 1);
    }
    return;
}



/**
  * @brief 弹窗Callback
  * @param  NULL
  * @retval NULL
  */
static void _cbPopUpErrorStartAndEndTimeWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            EmPopUpSubWidgetCreate(&_gstEmPopUpStartAndEndTimePara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
            /*默认选No*/
            //NO的边框
            GUI_SetColor(0x00FFFFFF);
            GUI_FillRoundedRect(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + 54 - 1,  REC_NO_YPOS + 18 - 1, 3);
            GUI_SetColor(0x00000000);
            GUI_DrawRoundedFrame(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + 54 - 1,  REC_NO_YPOS + 18 - 1, 3, 1);
            //yes的边框
            GUI_SetColor(0x00A4A4A4);
            GUI_FillRoundedRect(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + 54 - 1,  REC_YES_YPOS + 18 - 1, 3);

            _DrawSelPopUpRecPaint(_gstUiCtx.select);
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
  * @brief 创建弹窗
  * @param  p_widget_para:list控件结构体
  * @param  h_win_src:List的父控件
  * @retval NULL
  */
GUI_HWIN EmPopUpErrorStartAndEndTimeCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src)
{
    static GUI_WIDGET_CREATE_INFO  arr_pop_wm[1];
    GUI_HWIN handle;
    arr_pop_wm[0].pfCreateIndirect = WINDOW_CreateIndirect;
    arr_pop_wm[0].pName = "pop_up";
    arr_pop_wm[0].Id = p_widget_para->popup_start_id;
    arr_pop_wm[0].x0 = 0;
    arr_pop_wm[0].y0 = 0;
    arr_pop_wm[0].xSize = p_widget_para->x_size;
    arr_pop_wm[0].ySize = p_widget_para->y_size;
    arr_pop_wm[0].Flags = 0;
    arr_pop_wm[0].Para = 0;
    arr_pop_wm[0].NumExtraBytes = 0;

    handle = GUI_CreateDialogBox(arr_pop_wm, GUI_COUNTOF(arr_pop_wm), _cbPopUpErrorStartAndEndTimeWin, h_win_src, p_widget_para->x_pos, p_widget_para->y_pos);
    WINDOW_SetBkColor(handle, p_widget_para->bk_color);
    GUI_Exec();
    return handle;
}


/**
  * @brief 删除弹窗
  * @param  p_widget_para:list控件结构体
  * @retval NULL
  */
void EmPopUpStartAndEndTimeErrorDel(emPopUpPara_t *p_widget_para)
{
    if(gstPopUpPeakShiftingTimeErrorWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstPopUpPeakShiftingTimeErrorWin.win_handle);
    gstPopUpPeakShiftingTimeErrorWin.win_handle = 0;
}


/**
  * @brief Create current win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWin(void)
{
    if(pCurrentEmWin->win_handle == 0)
    {
        return 0;
    }
    _Init();
    gstPopUpPeakShiftingTimeErrorWin.win_handle = EmPopUpErrorStartAndEndTimeCreate(&_gstEmPopUpStartAndEndTimePara, pCurrentEmWin->win_handle);
    pCurrentPopUpSubWin = &gstPopUpPeakShiftingTimeErrorWin;
    return gstPopUpPeakShiftingTimeErrorWin.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstPopUpPeakShiftingTimeErrorWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstPopUpPeakShiftingTimeErrorWin.win_handle);
    _Del();
    gstPopUpPeakShiftingTimeErrorWin.win_handle = 0;
    pCurrentPopUpSubWin = NULL;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    snprintf(_garrPopUpErrorStartAndEndTimeTitleBuf, sizeof(_garrPopUpErrorStartAndEndTimeTitleBuf), "%s", GetPeakShiftingErr());

    _gstEmPopUpStartAndEndTimePara.bk_color = 0x006A7105;//后续根据错误还是警告信息填入合适的颜色
    _gstEmPopUpStartAndEndTimePara.x_pos = 115;
    _gstEmPopUpStartAndEndTimePara.y_pos = 122;
    _gstEmPopUpStartAndEndTimePara.x_size = 250;
    _gstEmPopUpStartAndEndTimePara.y_size = 90;

    _gstEmPopUpStartAndEndTimePara.popup_start_id = WIN_LIST_OPERA_INFO_ID;
    _gstEmPopUpStartAndEndTimePara.widget_num = 4;

    _gPopUpErrorStartAndEndTimeTitleMem = GUI_ALLOC_AllocZero(EM_POP_UP_START_AND_END_TIME_ICON_ALERT_SIZE);
    _pPopUpErrorStartAndEndTimeTitleBuf = GUI_ALLOC_h2p(_gPopUpErrorStartAndEndTimeTitleMem);
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _pPopUpErrorStartAndEndTimeTitleBuf, EM_POP_UP_START_AND_END_TIME_ICON_ALERT_SIZE);
    /*子控件参数初始化*/
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[0].x_relative_pos = 8;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[0].y_relative_pos = 8;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[0].x_size = 21;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[0].y_size = 21;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[0].sub_widget.img_para.mem_img = _gPopUpErrorStartAndEndTimeTitleMem;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[0].sub_widget.img_para.p_img_buf = _pPopUpErrorStartAndEndTimeTitleBuf;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[0].sub_widget.img_para.img_size = EM_POP_UP_START_AND_END_TIME_ICON_ALERT_SIZE;

    _gstEmPopUpStartAndEndTimePara.sub_widget_para[1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[1].x_relative_pos = 35;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[1].y_relative_pos = 8;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[1].x_size = 210;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[1].y_size = 50;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[1].sub_widget.text_para.p_content = _garrPopUpErrorStartAndEndTimeTitleBuf;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    strcpy(_gstEmPopUpStartAndEndTimePara.sub_widget_para[1].sub_widget.text_para.p_content, "The start time is greater\nthan the end time !");//后续根据错误还是警告信息填入

    _gstEmPopUpStartAndEndTimePara.sub_widget_para[2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[2].x_relative_pos = 149;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[2].y_relative_pos = 62;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[2].x_size = 50;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[2].y_size = 17;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[2].sub_widget.text_para.p_content = _garrPopUpErrorStartAndEndTimeNOBuf;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[2].sub_widget.text_para.text_color = 0x00000000;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gstEmPopUpStartAndEndTimePara.sub_widget_para[2].sub_widget.text_para.p_content, "No");//后续根据错误还是警告信息填入

    _gstEmPopUpStartAndEndTimePara.sub_widget_para[3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[3].x_relative_pos = 205;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[3].y_relative_pos = 62;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[3].x_size = 50;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[3].y_size = 17;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[3].sub_widget.text_para.p_content = _garrPopUpErrorStartAndEndTimeYESBuf;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[3].sub_widget.text_para.text_color = 0x00000000;
    _gstEmPopUpStartAndEndTimePara.sub_widget_para[3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gstEmPopUpStartAndEndTimePara.sub_widget_para[3].sub_widget.text_para.p_content, "Yes");//后续根据错误还是警告信息填入
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gPopUpErrorStartAndEndTimeTitleMem);
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{

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
    if (event == EN_KEY_SRC_RIGHT)
    {
        if(!_gstUiCtx.select)
        {
            _gstUiCtx.select = 1;
        }
        WM_Paint(gstPopUpPeakShiftingTimeErrorWin.win_handle);
        return;
    }
    if (event == EN_KEY_SRC_LEFT)
    {
        if(_gstUiCtx.select)
        {
            _gstUiCtx.select = 0;
        }
        WM_Paint(gstPopUpPeakShiftingTimeErrorWin.win_handle);
        return;
    }
    if (event == EN_KEY_SRC_CONFIRM)
    {
        if(_gstUiCtx.select)
        {
            //选中yes重新进行设置,返回到削峰填谷设置时间界面
            _Err1BackPeakShifiting();
        }
        else
        {
            //选中No 返回工作模式界面，保留上一次设置的正确时间段
            _Err1BackWorkMode();
        }
        return;
    }
#if 0
    if (event == EN_KEY_SRC_ESC)
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
                pnext = &gstEmWinListSavedSetTimeInfo;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
#endif
}


