/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_popup_complete_time_period.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-07-01
  * @brief       高级设置->削峰填谷->时间合理判断弹窗
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-07-01    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"


#define  EM_POP_UP_TIME_PERIOD_ICON_ALERT_SIZE                (4096)



static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gPopUpCompleteTimePeriodErrorWin = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


static emPopUpPara_t  _gEmPopUpCompleteTimePeriodPara;


static GUI_HMEM _gPopUpErrorTimePeriodTitleMem;
static uint8_t *_pPopUpErrorTimePeriodTitleBuf;
static char _gPopUpErrorTimePeriodTitleBuf[8];
static char _gPopUpErrorTimePeriodDescBuf[80];
static char _gPopUpErrorTimePeriodCodeBuf[48];


#define WIN_LIST_POP_TIME_PERIOD_ID                               (GUI_ID_USER + 0x51)

/**
  * @brief 弹窗Callback
  * @param  NULL
  * @retval NULL
  */
static void _cbPopUpErrorTimePeriodWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            EmPopUpSubWidgetCreate(&_gEmPopUpCompleteTimePeriodPara, pMsg->hWin);
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
GUI_HWIN EmPopUpErrorTimePeriodCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src)
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

    handle = GUI_CreateDialogBox(pop_wm, GUI_COUNTOF(pop_wm), _cbPopUpErrorTimePeriodWin, h_win_src, p_widget_para->x_pos, p_widget_para->y_pos);
    WINDOW_SetBkColor(handle, p_widget_para->bk_color);
    GUI_Exec();
    return handle;
}


/**
  * @brief 删除弹窗
  * @param  p_widget_para:list控件结构体
  * @retval NULL
  */
void EmPopUpErrorTimePeriodDel(emPopUpPara_t *p_widget_para)
{
    if(gPopUpCompleteTimePeriodErrorWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gPopUpCompleteTimePeriodErrorWin.win_handle);
    gPopUpCompleteTimePeriodErrorWin.win_handle = 0;
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
    gPopUpCompleteTimePeriodErrorWin.win_handle = EmPopUpErrorTimePeriodCreate(&_gEmPopUpCompleteTimePeriodPara, pCurrentEmWin->win_handle);
    pCurrentPopUpSubWin = &gPopUpCompleteTimePeriodErrorWin;
    return gPopUpCompleteTimePeriodErrorWin.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gPopUpCompleteTimePeriodErrorWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gPopUpCompleteTimePeriodErrorWin.win_handle);
    _Del();
    gPopUpCompleteTimePeriodErrorWin.win_handle = 0;
    pCurrentPopUpSubWin = NULL;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    _gEmPopUpCompleteTimePeriodPara.bk_color = 0x006A7105;//后续根据错误还是警告信息填入合适的颜色
    _gEmPopUpCompleteTimePeriodPara.x_pos = 115;
    _gEmPopUpCompleteTimePeriodPara.y_pos = 122;
    _gEmPopUpCompleteTimePeriodPara.x_size = 250;
    _gEmPopUpCompleteTimePeriodPara.y_size = 90;

    _gEmPopUpCompleteTimePeriodPara.popup_start_id = WIN_LIST_POP_TIME_PERIOD_ID;
    _gEmPopUpCompleteTimePeriodPara.widget_num = 3;

    _gPopUpErrorTimePeriodTitleMem = GUI_ALLOC_AllocZero(EM_POP_UP_TIME_PERIOD_ICON_ALERT_SIZE);
    _pPopUpErrorTimePeriodTitleBuf = GUI_ALLOC_h2p(_gPopUpErrorTimePeriodTitleMem);
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _pPopUpErrorTimePeriodTitleBuf, EM_POP_UP_TIME_PERIOD_ICON_ALERT_SIZE);
    /*子控件参数初始化*/
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[0].x_relative_pos = 12;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[0].y_relative_pos = 8;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[0].x_size = 21;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[0].y_size = 21;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[0].sub_widget.img_para.mem_img = _gPopUpErrorTimePeriodTitleMem;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[0].sub_widget.img_para.p_img_buf = _pPopUpErrorTimePeriodTitleBuf;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[0].sub_widget.img_para.img_size = EM_POP_UP_TIME_PERIOD_ICON_ALERT_SIZE;

    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[1].x_relative_pos = 36;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[1].y_relative_pos = 12;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[1].x_size = 60;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[1].y_size = 17;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[1].sub_widget.text_para.p_content = _gPopUpErrorTimePeriodTitleBuf;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmPopUpCompleteTimePeriodPara.sub_widget_para[1].sub_widget.text_para.p_content, "Error");//后续根据错误还是警告信息填入

    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[2].x_relative_pos = 15;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[2].y_relative_pos = 36;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[2].x_size = 220;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[2].y_size = 42;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[2].sub_widget.text_para.p_content = _gPopUpErrorTimePeriodDescBuf;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gEmPopUpCompleteTimePeriodPara.sub_widget_para[2].sub_widget.text_para.p_content, "At least set a complete \ncharging and discharging \ntime period !");//后续根据错误还是警告信息填入

#if 0
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[3].x_relative_pos = 15;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[3].y_relative_pos = 80;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[3].x_size = 200;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[3].y_size = 14;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[3].sub_widget.text_para.p_content = _gPopUpErrorTimePeriodCodeBuf;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmPopUpCompleteTimePeriodPara.sub_widget_para[3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gEmPopUpCompleteTimePeriodPara.sub_widget_para[3].sub_widget.text_para.p_content, "Code:123456");//后续根据错误还是警告信息填入
#endif

}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gPopUpErrorTimePeriodTitleMem);
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



