/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_popup_time_repetition_error.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-07-03
  * @brief       充电时间与放电时间重叠错误
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


#define  EM_POP_UP_TIME_REPETITION_ICON_ALERT_SIZE                (4096)



static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gPopUpTimeRepetitionErrorWin = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


static emPopUpPara_t  _gEmPopUpTimeRepetitionPara;


static GUI_HMEM _gPopUpErrorTimeRepetitionTitleMem;
static uint8_t *_pPopUpErrorTimeRepetitionTitleBuf;
static char _gPopUpErrorTimeRepetitionTitleBuf[8];
static char _gPopUpErrorTimeRepetitionDescBuf[80];
static char _gPopUpErrorTimeRepetitionCodeBuf[48];


#define WIN_LIST_OPERA_INFO_ID                               (GUI_ID_USER + 0x50)

/**
  * @brief 弹窗Callback
  * @param  NULL
  * @retval NULL
  */
static void _cbPopUpErrorTimeRepetitionWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            EmPopUpSubWidgetCreate(&_gEmPopUpTimeRepetitionPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
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
GUI_HWIN EmPopUpErrorTimeRepetitionCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src)
{
    static GUI_WIDGET_CREATE_INFO  pop_wm[1];
    GUI_HWIN handle;
    pop_wm[0].pfCreateIndirect = WINDOW_CreateIndirect;
    pop_wm[0].pName = "pop_up";
    pop_wm[0].Id = p_widget_para->popup_start_id;
    pop_wm[0].x0 = 0;
    pop_wm[0].y0 = 0;
    pop_wm[0].xSize = p_widget_para->x_size;
    pop_wm[0].ySize = p_widget_para->y_size;
    pop_wm[0].Flags = 0;
    pop_wm[0].Para = 0;
    pop_wm[0].NumExtraBytes = 0;

    handle = GUI_CreateDialogBox(pop_wm, GUI_COUNTOF(pop_wm), _cbPopUpErrorTimeRepetitionWin, h_win_src, p_widget_para->x_pos, p_widget_para->y_pos);
    WINDOW_SetBkColor(handle, p_widget_para->bk_color);
    GUI_Exec();
    return handle;
}


/**
  * @brief 删除弹窗
  * @param  p_widget_para:list控件结构体
  * @retval NULL
  */
void EmPopUpErrorTimeRepetitionDel(emPopUpPara_t *p_widget_para)
{
    if(gPopUpTimeRepetitionErrorWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gPopUpTimeRepetitionErrorWin.win_handle);
    gPopUpTimeRepetitionErrorWin.win_handle = 0;
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
    gPopUpTimeRepetitionErrorWin.win_handle = EmPopUpErrorTimeRepetitionCreate(&_gEmPopUpTimeRepetitionPara, pCurrentEmWin->win_handle);
    pCurrentPopUpSubWin = &gPopUpTimeRepetitionErrorWin;
    return gPopUpTimeRepetitionErrorWin.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gPopUpTimeRepetitionErrorWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gPopUpTimeRepetitionErrorWin.win_handle);
    _Del();
    gPopUpTimeRepetitionErrorWin.win_handle = 0;
    pCurrentPopUpSubWin = NULL;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    _gEmPopUpTimeRepetitionPara.bk_color = 0x006A7105;//后续根据错误还是警告信息填入合适的颜色
    _gEmPopUpTimeRepetitionPara.x_pos = 115;
    _gEmPopUpTimeRepetitionPara.y_pos = 122;
    _gEmPopUpTimeRepetitionPara.x_size = 250;
    _gEmPopUpTimeRepetitionPara.y_size = 90;

    _gEmPopUpTimeRepetitionPara.popup_start_id = WIN_LIST_OPERA_INFO_ID;
    _gEmPopUpTimeRepetitionPara.widget_num = 3;

    _gPopUpErrorTimeRepetitionTitleMem = GUI_ALLOC_AllocZero(EM_POP_UP_TIME_REPETITION_ICON_ALERT_SIZE);
    _pPopUpErrorTimeRepetitionTitleBuf = GUI_ALLOC_h2p(_gPopUpErrorTimeRepetitionTitleMem);
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _pPopUpErrorTimeRepetitionTitleBuf, EM_POP_UP_TIME_REPETITION_ICON_ALERT_SIZE);
    /*子控件参数初始化*/
    _gEmPopUpTimeRepetitionPara.sub_widget_para[0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[0].x_relative_pos = 12;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[0].y_relative_pos = 8;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[0].x_size = 21;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[0].y_size = 21;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[0].sub_widget.img_para.mem_img = _gPopUpErrorTimeRepetitionTitleMem;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[0].sub_widget.img_para.p_img_buf = _pPopUpErrorTimeRepetitionTitleBuf;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[0].sub_widget.img_para.img_size = EM_POP_UP_TIME_REPETITION_ICON_ALERT_SIZE;

    _gEmPopUpTimeRepetitionPara.sub_widget_para[1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[1].x_relative_pos = 36;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[1].y_relative_pos = 12;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[1].x_size = 60;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[1].y_size = 17;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[1].sub_widget.text_para.p_content = _gPopUpErrorTimeRepetitionTitleBuf;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmPopUpTimeRepetitionPara.sub_widget_para[1].sub_widget.text_para.p_content, "Error");//后续根据错误还是警告信息填入

    _gEmPopUpTimeRepetitionPara.sub_widget_para[2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[2].x_relative_pos = 15;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[2].y_relative_pos = 36;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[2].x_size = 220;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[2].y_size = 42;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[2].sub_widget.text_para.p_content = _gPopUpErrorTimeRepetitionDescBuf;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gEmPopUpTimeRepetitionPara.sub_widget_para[2].sub_widget.text_para.p_content, "The charging and discharging \ntime cannot be cross or \nrepeated !");//后续根据错误还是警告信息填入

#if 0
    _gEmPopUpTimeRepetitionPara.sub_widget_para[3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[3].x_relative_pos = 15;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[3].y_relative_pos = 80;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[3].x_size = 200;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[3].y_size = 14;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[3].sub_widget.text_para.p_content = _gPopUpErrorTimeRepetitionCodeBuf;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmPopUpTimeRepetitionPara.sub_widget_para[3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gEmPopUpTimeRepetitionPara.sub_widget_para[3].sub_widget.text_para.p_content, "Code:123456");//后续根据错误还是警告信息填入
#endif

}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gPopUpErrorTimeRepetitionTitleMem);
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
    int32_t event = EmGetKeyEvent(p_msg);

    if (event == EN_KEY_SRC_ESC)
    {
        if(pCurrentEmWin->win_handle != 0)
        {
            emWin_t *pnext = NULL;
            if(p_msg->press_down_time >= gEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
            {
                pnext = &gEmWinHomePage;
            }
            else
            {
                pnext = &gEmWinListSavedSetTimeInfo;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}


