/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_popup_error.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-1-9
  * @brief       错误弹窗界面处理
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


#define  EM_POP_UP_ICON_ALERT_SIZE                (4096)



static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstPopUpWinError = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


static emPopUpPara_t  _gEmPopUpPara;


static GUI_HMEM _gPopUpErrorTitleMem;
static uint8_t *_pPopUpErrorTitleBuf;
static char _gPopUpErrorTitleBuf[8];
static char _gPopUpErrorDescBuf[48];
static char _gPopUpErrorCodeBuf[60];


#define WIN_LIST_OPERA_INFO_ID                               (GUI_ID_USER + 0x50)

/**
  * @brief 弹窗Callback
  * @param  NULL
  * @retval NULL
  */
static void _cbPopUpErrorWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            EmPopUpSubWidgetCreate(&_gEmPopUpPara, pMsg->hWin);
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
GUI_HWIN EmPopUpErrorCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src)
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

    handle = GUI_CreateDialogBox(pop_wm, GUI_COUNTOF(pop_wm), _cbPopUpErrorWin, h_win_src, p_widget_para->x_pos, p_widget_para->y_pos);
    WINDOW_SetBkColor(handle, p_widget_para->bk_color);
    GUI_Exec();
    return handle;
}


/**
  * @brief 删除弹窗
  * @param  p_widget_para:list控件结构体
  * @retval NULL
  */
void EmPopUpErrorDel(emPopUpPara_t *p_widget_para)
{
    if(gstPopUpWinError.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstPopUpWinError.win_handle);
    gstPopUpWinError.win_handle = 0;
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
    gstPopUpWinError.win_handle = EmPopUpErrorCreate(&_gEmPopUpPara, pCurrentEmWin->win_handle);
    pCurrentPopUpSubWin = &gstPopUpWinError;
    return gstPopUpWinError.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstPopUpWinError.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstPopUpWinError.win_handle);
    _Del();
    gstPopUpWinError.win_handle = 0;
    pCurrentPopUpSubWin = NULL;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    _gEmPopUpPara.bk_color = GUI_RED;//后续根据错误还是警告信息填入合适的颜色
    _gEmPopUpPara.x_pos = 115;
    _gEmPopUpPara.y_pos = 122;
    _gEmPopUpPara.x_size = 250;
    _gEmPopUpPara.y_size = 75;

    _gEmPopUpPara.popup_start_id = WIN_LIST_OPERA_INFO_ID;
    _gEmPopUpPara.widget_num = 4;

    _gPopUpErrorTitleMem = GUI_ALLOC_AllocZero(EM_POP_UP_ICON_ALERT_SIZE);
    _pPopUpErrorTitleBuf = GUI_ALLOC_h2p(_gPopUpErrorTitleMem);
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _pPopUpErrorTitleBuf, EM_POP_UP_ICON_ALERT_SIZE);
    /*子控件参数初始化*/
    _gEmPopUpPara.sub_widget_para[0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmPopUpPara.sub_widget_para[0].x_relative_pos = 12;
    _gEmPopUpPara.sub_widget_para[0].y_relative_pos = 8;
    _gEmPopUpPara.sub_widget_para[0].x_size = 21;
    _gEmPopUpPara.sub_widget_para[0].y_size = 21;
    _gEmPopUpPara.sub_widget_para[0].sub_widget.img_para.mem_img = _gPopUpErrorTitleMem;
    _gEmPopUpPara.sub_widget_para[0].sub_widget.img_para.p_img_buf = _pPopUpErrorTitleBuf;
    _gEmPopUpPara.sub_widget_para[0].sub_widget.img_para.img_size = EM_POP_UP_ICON_ALERT_SIZE;

    _gEmPopUpPara.sub_widget_para[1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmPopUpPara.sub_widget_para[1].x_relative_pos = 36;
    _gEmPopUpPara.sub_widget_para[1].y_relative_pos = 12;
    _gEmPopUpPara.sub_widget_para[1].x_size = 60;
    _gEmPopUpPara.sub_widget_para[1].y_size = 17;
    _gEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content = _gPopUpErrorTitleBuf;
    _gEmPopUpPara.sub_widget_para[1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content, "FAULT");//后续根据错误还是警告信息填入

    _gEmPopUpPara.sub_widget_para[2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmPopUpPara.sub_widget_para[2].x_relative_pos = 15;
    _gEmPopUpPara.sub_widget_para[2].y_relative_pos = 36;
    _gEmPopUpPara.sub_widget_para[2].x_size = 200;
    _gEmPopUpPara.sub_widget_para[2].y_size = 14;
    _gEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_content = _gPopUpErrorDescBuf;
    _gEmPopUpPara.sub_widget_para[2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_content, "Error message and suggests.");//后续根据错误还是警告信息填入

    _gEmPopUpPara.sub_widget_para[3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmPopUpPara.sub_widget_para[3].x_relative_pos = 15;
    _gEmPopUpPara.sub_widget_para[3].y_relative_pos = 50;
    _gEmPopUpPara.sub_widget_para[3].x_size = 200;
    _gEmPopUpPara.sub_widget_para[3].y_size = 14;
    _gEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content = _gPopUpErrorCodeBuf;
    _gEmPopUpPara.sub_widget_para[3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content, "Code:123456");//后续根据错误还是警告信息填入


}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gPopUpErrorTitleMem);
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

}


