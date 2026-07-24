/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_restore_factory_popup.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-21
  * @brief       高级设置->更多设置->恢复出厂(界面中的弹窗)
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


#define EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE               (4096)

#define REC_NO_XPOS                                             (130)
#define REC_NO_YPOS                                             (70)
#define REC_YES_XPOS                                            (190)
#define REC_YES_YPOS                                            (70)
#define REC_X_SIZE                                              (54)
#define REC_Y_SIZE                                              (18)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstRestoreFacotryPopUpWin = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    int32_t restore_factory;
    int32_t count;
} UiCtx_t;

static UiCtx_t _gstUiCtx;

static emPopUpPara_t _gstRestoreFacotryEmPopUpPara;
int32_t gTestResetFactory = 0;
int32_t gTestReseting = 0;

static GUI_HMEM _gRestoreFacotryPopUpTitleMem;
static uint8_t *_pRestoreFacotryPopUpTitleBuf;
static char _garrRestoreFacotryPopUpTitleBuf[80];
static char _garrRestoreFacotryPopUpYESBuf[5];
static char _garrRestoreFacotryPopUpNOBuf[5];


#define WIN_LIST_RESTORE_FACOTRY_FAULT_POPUP_ID                     (GUI_ID_USER + 0x40)


static int32_t _BackRestoreFactory(int32_t status)
{
    emWin_t *pnext = NULL;
//    pnext = &gstEmWinListAdvanSettingFactoryRestore;
    pnext = &gstEmWinMain3DNav;
    if(pCurrentEmWin->win_handle != 0 && status)
    {
        pPrevEmWin = pCurrentEmWin;
        pCurrentEmWin->delete_win();
        pCurrentEmWin = pnext;
        pCurrentEmWin->create_win();
    }
    return 0;
}

static int32_t _SetRestoringFactoryStatus(int32_t status)
{
    static int32_t tmp1;
    int32_t tmp = 0;

    tmp = _gstUiCtx.count % 4000;
    if (1 == status)
    {
        if(tmp == 0)
        {
            tmp = ++tmp1 % 4;
            if(tmp == 0)
            {
                snprintf(_garrRestoreFacotryPopUpTitleBuf, sizeof(_garrRestoreFacotryPopUpTitleBuf),
                         "%s", "Factory Reset");
//                snprintf(_garrRestoreFacotryPopUpTitleBuf, sizeof(_garrRestoreFacotryPopUpTitleBuf),
//                         "%s", "Restoring Factory");
            }
            else if (tmp == 1)
            {
                snprintf(_garrRestoreFacotryPopUpTitleBuf, sizeof(_garrRestoreFacotryPopUpTitleBuf),
                        "%s", "Factory Reset .");
//                snprintf(_garrRestoreFacotryPopUpTitleBuf, sizeof(_garrRestoreFacotryPopUpTitleBuf),
//                         "%s", "Restoring Factory .");
            }
            else if (tmp == 2)
            {
                snprintf(_garrRestoreFacotryPopUpTitleBuf, sizeof(_garrRestoreFacotryPopUpTitleBuf),
                         "%s", "Factory Reset .."); 
//                snprintf(_garrRestoreFacotryPopUpTitleBuf, sizeof(_garrRestoreFacotryPopUpTitleBuf),
//                         "%s", "Restoring Factory ..");
            }
            else if (tmp == 3)
            {
                snprintf(_garrRestoreFacotryPopUpTitleBuf, sizeof(_garrRestoreFacotryPopUpTitleBuf),
                         "%s", "Factory Reset ...");
//                snprintf(_garrRestoreFacotryPopUpTitleBuf, sizeof(_garrRestoreFacotryPopUpTitleBuf),
//                         "%s", "Restoring Factory ...");
            }

            WM_HideWindow(_gstRestoreFacotryEmPopUpPara.wid_handle[2]);
            WM_HideWindow(_gstRestoreFacotryEmPopUpPara.wid_handle[3]);
            TEXT_SetText(_gstRestoreFacotryEmPopUpPara.wid_handle[1],
                         (const char *)_gstRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content);
        }
        ++_gstUiCtx.count;
    }
    else if (status == 2)
    {
        snprintf(_garrRestoreFacotryPopUpTitleBuf, sizeof(_garrRestoreFacotryPopUpTitleBuf), "%s",
                 "Factory Reset Complete");
//        snprintf(_garrRestoreFacotryPopUpTitleBuf, sizeof(_garrRestoreFacotryPopUpTitleBuf), "%s",
//                 "Restore Factory complete");
        snprintf(_garrRestoreFacotryPopUpYESBuf, sizeof(_garrRestoreFacotryPopUpYESBuf), "%s",
                 "OK");
        WM_HideWindow(_gstRestoreFacotryEmPopUpPara.wid_handle[2]);
        WM_ShowWindow(_gstRestoreFacotryEmPopUpPara.wid_handle[3]);
        TEXT_SetText(_gstRestoreFacotryEmPopUpPara.wid_handle[1],
                     (const char *)_gstRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content);
        TEXT_SetText(_gstRestoreFacotryEmPopUpPara.wid_handle[3],
                     (const char *)_gstRestoreFacotryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content);
        _gstUiCtx.count = 0;
    }
    //WM_Paint(gstRestoreFacotryPopUpWin.win_handle);
    return 0;
}

static int32_t _GetRestoreFacoty()
{
    return _gstUiCtx.restore_factory;
}

static int32_t _CheckRestoreFacoty()
{
    if(1 == gTestResetFactory)
    {
        _SetRestoringFactoryStatus(1);
        if(_gstUiCtx.count > 20000)
        {
            gTestResetFactory = 0;
            _gstUiCtx.select = 0;
            _SetRestoringFactoryStatus(2);
        }
    }
    return 0;
}

/**
  * @brief  绘制选中框
  * @param  NULL
  * @retval NULL
  */
static void _DrawSelPopUpRecPaint(int32_t sel)
{
//    if (!_GetRestoreFacoty())
    if(0 == gTestResetFactory)
    {
        if(sel == 0)
        {
            //no
            if (_garrRestoreFacotryPopUpYESBuf[0] != 'O')
            {
                GUI_SetColor(0x00FFFFFF);
                GUI_FillRoundedRect(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + REC_X_SIZE,  REC_NO_YPOS + REC_Y_SIZE, 3);
                GUI_SetColor(0x00000000);
                GUI_DrawRoundedFrame(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + REC_X_SIZE,  REC_NO_YPOS + REC_Y_SIZE, 3, 2);
            }
            //yes
            GUI_SetColor(0x00A4A4A4);
            GUI_FillRoundedRect(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + REC_X_SIZE,  REC_YES_YPOS + REC_Y_SIZE, 3);
            //      GUI_SetColor(0x00000000);
            //      GUI_DrawRoundedFrame(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + REC_X_SIZE,  REC_YES_YPOS + REC_Y_SIZE, 3, 2);

        }
        else if(sel == 1)
        {
            //no
            if (_garrRestoreFacotryPopUpYESBuf[0] != 'O')
            {
                GUI_SetColor(0x00A4A4A4);
                GUI_FillRoundedRect(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + REC_X_SIZE,  REC_NO_YPOS + REC_Y_SIZE, 3);
                //      GUI_SetColor(0x00000000);
                //      GUI_DrawRoundedFrame(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + REC_X_SIZE,  REC_NO_YPOS + REC_Y_SIZE, 3,2);
            }
            //yes
            GUI_SetColor(0x00FFFFFF);
            GUI_FillRoundedRect(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + REC_X_SIZE,  REC_YES_YPOS + REC_Y_SIZE, 3);
            GUI_SetColor(0x00000000);
            GUI_DrawRoundedFrame(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + REC_X_SIZE,  REC_YES_YPOS + REC_Y_SIZE, 3, 2);
        }
    }
    return;
}

/**
  * @brief 弹窗Callback
  * @param  NULL
  * @retval NULL
  */
static void _cbRestoreFacotryPopUpWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            EmPopUpSubWidgetCreate(&_gstRestoreFacotryEmPopUpPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
            /*默认选No*/
            //NO的边框
            GUI_Clear();
//            if (2 == _GetRestoreFacoty())
            if(0 == gTestResetFactory)
            {
                if (_garrRestoreFacotryPopUpYESBuf[0] != 'O')
                {
                    GUI_SetColor(0x00FFFFFF);
                    GUI_FillRoundedRect(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + REC_X_SIZE,  REC_NO_YPOS + REC_Y_SIZE, 3);
                    GUI_SetColor(0x00000000);
                    GUI_DrawRoundedFrame(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + REC_X_SIZE,  REC_NO_YPOS + REC_Y_SIZE, 3, 2);
                }
                //yes的边框
                GUI_SetColor(0x00A4A4A4);
                GUI_FillRoundedRect(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + REC_X_SIZE,  REC_YES_YPOS + REC_Y_SIZE, 3);
                //          GUI_SetColor(0x00000000);
                //          GUI_DrawRoundedFrame(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + REC_X_SIZE - 1,  REC_YES_YPOS + REC_Y_SIZE, 3, 2);
            }
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
GUI_HWIN EmRestoreFacotryPopUpCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src)
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

    handle = GUI_CreateDialogBox(arr_pop_wm, GUI_COUNTOF(arr_pop_wm), _cbRestoreFacotryPopUpWin, h_win_src, p_widget_para->x_pos, p_widget_para->y_pos);
    WINDOW_SetBkColor(handle, p_widget_para->bk_color);
    GUI_Exec();
    return handle;
}


/**
  * @brief 删除弹窗
  * @param  p_widget_para:list控件结构体
  * @retval NULL
  */
void EmRestoreFacotryPopUpDel(emPopUpPara_t *p_widget_para)
{
    if(gstRestoreFacotryPopUpWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstRestoreFacotryPopUpWin.win_handle);
    gstRestoreFacotryPopUpWin.win_handle = 0;
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
//    pPrevEmWin = &gstEmWinListAdvanSettingFactoryRestore;
    pPrevEmWin = &gstEmWinMain3DNav;
    _Init();
    gstRestoreFacotryPopUpWin.win_handle = EmRestoreFacotryPopUpCreate(&_gstRestoreFacotryEmPopUpPara, pCurrentEmWin->win_handle);
    pCurrentPopUpSubWin = &gstRestoreFacotryPopUpWin;
    return gstRestoreFacotryPopUpWin.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstRestoreFacotryPopUpWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstRestoreFacotryPopUpWin.win_handle);
    _Del();
    gstRestoreFacotryPopUpWin.win_handle = 0;
    pCurrentPopUpSubWin = NULL;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    _gstUiCtx.restore_factory = 0;
    _gstRestoreFacotryEmPopUpPara.bk_color = 0x006A7105;//后续根据错误还是警告信息填入合适的颜色
    _gstRestoreFacotryEmPopUpPara.x_pos = 115;
    _gstRestoreFacotryEmPopUpPara.y_pos = 122;
    _gstRestoreFacotryEmPopUpPara.x_size = 250;
    _gstRestoreFacotryEmPopUpPara.y_size = 95;

    _gstRestoreFacotryEmPopUpPara.popup_start_id = WIN_LIST_RESTORE_FACOTRY_FAULT_POPUP_ID;
    _gstRestoreFacotryEmPopUpPara.widget_num = 4;

    _gRestoreFacotryPopUpTitleMem = GUI_ALLOC_AllocZero(EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE);
    _pRestoreFacotryPopUpTitleBuf = GUI_ALLOC_h2p(_gRestoreFacotryPopUpTitleMem);
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _pRestoreFacotryPopUpTitleBuf, EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE);
    /*子控件参数初始化*/
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[0].x_relative_pos = 12;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[0].y_relative_pos = 13;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[0].x_size = 21;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[0].y_size = 21;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[0].sub_widget.img_para.mem_img = _gRestoreFacotryPopUpTitleMem;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[0].sub_widget.img_para.p_img_buf = _pRestoreFacotryPopUpTitleBuf;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[0].sub_widget.img_para.img_size = EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE;

    _gstRestoreFacotryEmPopUpPara.sub_widget_para[1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[1].x_relative_pos = 43;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[1].y_relative_pos = 10;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[1].x_size = 192;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[1].y_size = 50;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content = _garrRestoreFacotryPopUpTitleBuf;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gstRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content, "Note that factory reset\nwill delete all current\nconfigurations !");//后续根据错误还是警告信息填入

    _gstRestoreFacotryEmPopUpPara.sub_widget_para[2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[2].x_relative_pos = 149;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[2].y_relative_pos = 72;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[2].x_size = 50;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[2].y_size = 17;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_content = _garrRestoreFacotryPopUpNOBuf;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.text_color = 0x00000000;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gstRestoreFacotryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_content, "No");//后续根据错误还是警告信息填入

    _gstRestoreFacotryEmPopUpPara.sub_widget_para[3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[3].x_relative_pos = 205;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[3].y_relative_pos = 72;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[3].x_size = 50;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[3].y_size = 17;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content = _garrRestoreFacotryPopUpYESBuf;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.text_color = 0x00000000;
    _gstRestoreFacotryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gstRestoreFacotryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content, "Yes");//后续根据错误还是警告信息填入
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gRestoreFacotryPopUpTitleMem);
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    _CheckRestoreFacoty();
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
        if(!_gstUiCtx.select)
        {
            _gstUiCtx.select = 1;
        }
        WM_Paint(gstRestoreFacotryPopUpWin.win_handle);
        return;
    }
    if (event == EN_KEY_SRC_RIGHT)
    {
        if(_gstUiCtx.select)
        {
            _gstUiCtx.select = 0;
        }
        WM_Paint(gstRestoreFacotryPopUpWin.win_handle);
        return;
    }
    if (event == EN_KEY_SRC_CONFIRM)
    {
        if (_gstUiCtx.select)
        {
            uint8_t arr_data_buff[16];
            uint16_t tmp = 0;
            memset(arr_data_buff, 0, sizeof(arr_data_buff));
            _gstUiCtx.restore_factory = 0x01;
            //0x01准备恢复出厂设置
            tmp = BigLittleSwap16(_gstUiCtx.restore_factory);
            ModbusWriteSingleRemain(arr_data_buff, 0x01, 0x5241, tmp);
            _gstUiCtx.restore_factory = 0x02;
            //0x02开始恢复出厂设置
            tmp = BigLittleSwap16(_gstUiCtx.restore_factory);
            ModbusWriteSingleRemain(arr_data_buff, 0x01, 0x5241, tmp);

            //test
            gTestResetFactory = 1;
            return ;
        }
        _BackRestoreFactory(1);
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
//                pnext = &gstEmWinListAdvanSettingFactoryRestore;
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


