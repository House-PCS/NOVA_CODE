/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_debug_window.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-21
  * @brief       调试窗口
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-21    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"
#include "error_proc.h"


#define  EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE                (4096)

#define  WIN_LIST_RESTORE_FACOTRY_FAULT_POPUP_ID                  (GUI_ID_USER + 0x40)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

typedef struct _ui_ctx_stru_t
{
    int32_t key_init;
} UiCtx_t;

typedef enum
{
    EN_DEBUG_STATUS_NONE = 0,    // 未初始化
    EN_DEBUG_STATUS_INIT,        // 已初始化
    EN_DEBUG_STATUS_HIDE,        // 隐藏
    EN_DEBUG_STATUS_SHOW,        // 显示
} EN_DEBUG_STATUS;

emWin_t *_gPreWin = NULL;
emWin_t  gstDebugPopUpWin = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

static int32_t _gEmDebugWinStatus = 0;
static UiCtx_t _gstUiCtx;

static emPopUpPara_t  _gstDebugEmPopUpPara;
static GUI_HMEM _gDeubPopUpTitleMem;
static uint8_t *_pDebugPopUpTitleBuf;
static char _garrDebugPopUpTitleBuf[128];

/**
  * @brief 弹窗Callback
  * @param  NULL
  * @retval NULL
  */
static void _cbEmDebugPopUpWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            EmPopUpSubWidgetCreate(&_gstDebugEmPopUpPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
            /*默认选No*/
            //NO的边框
            GUI_Clear();
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
GUI_HWIN EmDebugPopUpCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src)
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

    handle = GUI_CreateDialogBox(arr_pop_wm, GUI_COUNTOF(arr_pop_wm), _cbEmDebugPopUpWin, h_win_src, p_widget_para->x_pos, p_widget_para->y_pos);
    WINDOW_SetBkColor(handle, p_widget_para->bk_color);
    GUI_Exec();
    return handle;
}


/**
  * @brief 删除弹窗
  * @param  p_widget_para:list控件结构体
  * @retval NULL
  */
static void EmDebugPopUpDel(emPopUpPara_t *p_widget_para)
{
    if(gstDebugPopUpWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstDebugPopUpWin.win_handle);
    gstDebugPopUpWin.win_handle = 0;
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
    gstDebugPopUpWin.win_handle =
        EmDebugPopUpCreate(&_gstDebugEmPopUpPara, pCurrentEmWin->win_handle);
    return gstDebugPopUpWin.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstDebugPopUpWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstDebugPopUpWin.win_handle);
    _Del();
    gstDebugPopUpWin.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    _gstDebugEmPopUpPara.bk_color = 0x006A7105;//后续根据错误还是警告信息填入合适的颜色
    _gstDebugEmPopUpPara.x_pos = 20;
    _gstDebugEmPopUpPara.y_pos = 20;
    _gstDebugEmPopUpPara.x_size = EM_XSIZE - _gstDebugEmPopUpPara.x_pos * 2;
    _gstDebugEmPopUpPara.y_size = EM_YSIZE - _gstDebugEmPopUpPara.y_pos * 2;

    _gstDebugEmPopUpPara.x_size = _gstDebugEmPopUpPara.x_size > 0 ? _gstDebugEmPopUpPara.x_size : 0;
    _gstDebugEmPopUpPara.y_size = _gstDebugEmPopUpPara.y_size > 0 ? _gstDebugEmPopUpPara.y_size : 0;

    _gstDebugEmPopUpPara.popup_start_id = WIN_LIST_RESTORE_FACOTRY_FAULT_POPUP_ID;
    _gstDebugEmPopUpPara.widget_num = 2;

    _gDeubPopUpTitleMem = GUI_ALLOC_AllocZero(EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE);
    _pDebugPopUpTitleBuf = GUI_ALLOC_h2p(_gDeubPopUpTitleMem);
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _pDebugPopUpTitleBuf, EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE);
    /*子控件参数初始化*/
    _gstDebugEmPopUpPara.sub_widget_para[0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstDebugEmPopUpPara.sub_widget_para[0].x_relative_pos = 12;
    _gstDebugEmPopUpPara.sub_widget_para[0].y_relative_pos = 13;
    _gstDebugEmPopUpPara.sub_widget_para[0].x_size = 21;
    _gstDebugEmPopUpPara.sub_widget_para[0].y_size = 21;
    _gstDebugEmPopUpPara.sub_widget_para[0].sub_widget.img_para.mem_img = _gDeubPopUpTitleMem;
    _gstDebugEmPopUpPara.sub_widget_para[0].sub_widget.img_para.p_img_buf = _pDebugPopUpTitleBuf;
    _gstDebugEmPopUpPara.sub_widget_para[0].sub_widget.img_para.img_size = EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE;

    _gstDebugEmPopUpPara.sub_widget_para[1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstDebugEmPopUpPara.sub_widget_para[1].x_relative_pos = 10;
    _gstDebugEmPopUpPara.sub_widget_para[1].y_relative_pos = 10;
    _gstDebugEmPopUpPara.sub_widget_para[1].x_size = _gstDebugEmPopUpPara.x_size - _gstDebugEmPopUpPara.sub_widget_para[1].x_relative_pos * 2;
    _gstDebugEmPopUpPara.sub_widget_para[1].y_size = _gstDebugEmPopUpPara.y_size - _gstDebugEmPopUpPara.sub_widget_para[1].y_relative_pos * 2;
    _gstDebugEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content = _garrDebugPopUpTitleBuf;
    _gstDebugEmPopUpPara.sub_widget_para[1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstDebugEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gstDebugEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content, "Note that restoring the \nfactory will delete all \ncurrent configurations !");//后续根据错误还是警告信息填入
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gDeubPopUpTitleMem);
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    bool alarm = GetSysAlarm();
    bool syserr = GetSysError();
    uint8_t pcs_com_stat = GetPcsCommStatus();

    snprintf(_garrDebugPopUpTitleBuf, sizeof(_garrDebugPopUpTitleBuf),
             "12j3lkfjlkdsjflkdslkjflkjdslkjfkldslkfjsdjfkjlkdsjafkldsj jflkdsk lfjkdj fj    fdlksjf lkjds  \n"
             "alarm[%d], \n"
             "syserr[%d], \n"
             "pcs_com_stat[%d]",
             alarm,
             syserr,
             pcs_com_stat);

    TEXT_SetWrapMode(_gstDebugEmPopUpPara.wid_handle[1], GUI_WRAPMODE_WORD);
    TEXT_SetText(_gstDebugEmPopUpPara.wid_handle[1], (const char *)_gstDebugEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content);
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
        return;
    }
    if (event == EN_KEY_SRC_RIGHT)
    {
        return;
    }
    if (event == EN_KEY_SRC_CONFIRM)
    {
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
    {
        EmDebugWinHide();
        return;
    }
}

static int32_t _EmDebugWinGetStatus()
{
    return _gEmDebugWinStatus;
}

static int32_t _EmDebugWinSetStatus(int32_t status)
{
    switch (status)
    {
        case EN_DEBUG_STATUS_NONE:
        {
            break;
        }
        case EN_DEBUG_STATUS_INIT:
        {
            if (_EmDebugWinGetStatus() <= EN_DEBUG_STATUS_NONE)
            {
                gstDebugPopUpWin.create_win();
            }
            break;
        }
        case EN_DEBUG_STATUS_HIDE:
        {
            break;
        }
        case EN_DEBUG_STATUS_SHOW:
        {
            if (_EmDebugWinGetStatus() <= EN_DEBUG_STATUS_NONE)
            {
                gstDebugPopUpWin.create_win();
            }
            pCurrentEmWin = &gstDebugPopUpWin;
            break;
        }
    }
    return 0;
}

int32_t EmDebugWinHide(void)
{
    if (!_gPreWin)
    {
        return -1;
    }

    if(pCurrentEmWin->win_handle == 0)
    {
        return -2;
    }

    _EmDebugWinSetStatus(EN_DEBUG_STATUS_NONE);

    pCurrentEmWin->delete_win();
    pCurrentEmWin = _gPreWin;
    pCurrentEmWin->create_win();
    _gPreWin = NULL;

    return 0;
}

int32_t EmDebugWinShow(emWin_t *pre)
{
    _gPreWin = pre;
    _EmDebugWinSetStatus(EN_DEBUG_STATUS_SHOW);
    return 0;
}

