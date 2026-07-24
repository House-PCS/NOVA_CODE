/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_restore_factory_popup.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2026-01-21
  * @brief       关机界面弹窗
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2026-01-21    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"


#define EM_SHUT_DOWN_POP_UP_ICON_ALERT_SIZE               (4096)

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

emWin_t gstShutDownPopUpWin = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    int32_t shut_down;
    int32_t count;
    uint32_t pv_total_volt;
    uint32_t gen_volt;
    uint32_t turn_off_timeout;
} UiCtx_t;

static UiCtx_t _gstUiCtx;

static emPopUpPara_t _gstShutDownEmPopUpPara;

static GUI_HMEM _gShutDownPopUpTitleMem;
static uint8_t *_pShutDownPopUpTitleBuf;
static char _garrShutDownPopUpTitleBuf[80];
static char _garrShutDownPopUpYESBuf[5];
static char _garrShutDownPopUpNOBuf[5];

#define MSG_SHUTTING_DOWN        "The machine is\nshutting down !"
#define MSG_PV_NOT_OFF           "PV is not turned\noff; shut down is\nnot possible !"
#define MSG_GEN_NOT_OFF          "Generator not\nturned off; cannot\nshut down !"
#define MSG_PV_GEN_NOT_OFF       "PV and Generator\n not turned off;\ncannot shut down !"

#define SHUTSOWN_TIMEOUT_MS             (60 * 1000)
#define PV_VOLT_THRESHOLD_V             (700)
#define GEN_VOLT_THRESHOLD_V            (1000)

#define WIN_LIST_SHUT_DOWN_POPUP_ID                     (GUI_ID_USER + 0x40)


static int32_t _BackShutDown(int32_t status)
{
//    emWin_t *pnext = NULL;
////    pnext = &gstEmWinListAdvanSettingFactoryRestore;
//    pnext = &gstEmWinMain3DNav;
//    if(pCurrentEmWin->win_handle != 0 && status)
//    {
//        pPrevEmWin = pCurrentEmWin;
//        pCurrentEmWin->delete_win();
//        pCurrentEmWin = pnext;
//        pCurrentEmWin->create_win();
//    }
    return 0;
}


static int32_t _SetShutDownStatus(void)
{
    static uint32_t time_out;
    static uint16_t tim_flag;
    static uint16_t is_tomeout;
    uint16_t pv_on = 0;
    uint16_t gen_on = 0;

#if 1    
    _gstUiCtx.pv_total_volt  = gstCollectInfo.power_data.power_value_info.pv_1_vol + gstCollectInfo.power_data.power_value_info.pv_2_vol +
                               gstCollectInfo.power_data.power_value_info.pv_3_vol + gstCollectInfo.power_data.power_value_info.pv_4_vol;

    _gstUiCtx.gen_volt = gstCollectInfo.power_data.power_value_info.r_gen_vol + 
                         gstCollectInfo.power_data.power_value_info.s_gen_vol +
                         gstCollectInfo.power_data.power_value_info.t_gen_vol;
#else
    static uint16_t pv_volt = 0;
    static uint16_t gen_volt = 0;
    
    _gstUiCtx.pv_total_volt = pv_volt;
    _gstUiCtx.gen_volt = gen_volt;
#endif
    
    if(gShutDownPara.shut_reset_time_flag == 0)
    {
        time_out = tim_get_systime();
        gShutDownPara.shut_reset_time_flag = 1;
    }

    is_tomeout = (tim_get_systime() - time_out > SHUTSOWN_TIMEOUT_MS) ? 1:0;

    if(is_tomeout)
    {
        pv_on = (_gstUiCtx.pv_total_volt > PV_VOLT_THRESHOLD_V) ? 1 : 0;
        gen_on = (_gstUiCtx.gen_volt > GEN_VOLT_THRESHOLD_V) ? 1 : 0;

        if(pv_on && gen_on)
        {
            snprintf(_garrShutDownPopUpTitleBuf, sizeof(_garrShutDownPopUpTitleBuf),
                            "%s", MSG_PV_GEN_NOT_OFF);
        }
        else if(pv_on)
        {
            snprintf(_garrShutDownPopUpTitleBuf, sizeof(_garrShutDownPopUpTitleBuf),
                            "%s", MSG_PV_NOT_OFF);
        }
        else if(gen_on)
        {
            snprintf(_garrShutDownPopUpTitleBuf, sizeof(_garrShutDownPopUpTitleBuf), 
                            "%s", MSG_GEN_NOT_OFF);
        }
    }             

    TEXT_SetText(_gstShutDownEmPopUpPara.wid_handle[1],
                 (const char *)_gstShutDownEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content);
    return 0;

}

static int32_t _GetShutDown()
{
    return _gstUiCtx.shut_down;
}

static int32_t _CheckShutDown()
{
    return 0;
}

/**
  * @brief  绘制选中框
  * @param  NULL
  * @retval NULL
  */
static void _DrawSelPopUpRecPaint(int32_t sel)
{
//    if (!_GetShutDown())
    if(1)
    {
        if(sel == 0)
        {
            //no
            if (_garrShutDownPopUpYESBuf[0] != 'O')
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
            if (_garrShutDownPopUpYESBuf[0] != 'O')
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
static void _cbShutDownPopUpWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            EmPopUpSubWidgetCreate(&_gstShutDownEmPopUpPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
            /*默认选No*/
            //NO的边框
            GUI_Clear();
//            if (2 == _GetShutDown())
//            if(1)
//            {
//                if (_garrShutDownPopUpYESBuf[0] != 'O')
//                {
//                    GUI_SetColor(0x00FFFFFF);
//                    GUI_FillRoundedRect(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + REC_X_SIZE,  REC_NO_YPOS + REC_Y_SIZE, 3);
//                    GUI_SetColor(0x00000000);
//                    GUI_DrawRoundedFrame(REC_NO_XPOS, REC_NO_YPOS,  REC_NO_XPOS + REC_X_SIZE,  REC_NO_YPOS + REC_Y_SIZE, 3, 2);
//                }
            //yes的边框
            // GUI_SetColor(0x00A4A4A4);
            // GUI_FillRoundedRect(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + REC_X_SIZE,  REC_YES_YPOS + REC_Y_SIZE, 3);
            //          GUI_SetColor(0x00000000);
            //          GUI_DrawRoundedFrame(REC_YES_XPOS, REC_YES_YPOS,  REC_YES_XPOS + REC_X_SIZE - 1,  REC_YES_YPOS + REC_Y_SIZE, 3, 2);
//            }
            // _DrawSelPopUpRecPaint(_gstUiCtx.select);
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
GUI_HWIN EmShutDownPopUpCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src)
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

    handle = GUI_CreateDialogBox(arr_pop_wm, GUI_COUNTOF(arr_pop_wm), _cbShutDownPopUpWin, h_win_src, p_widget_para->x_pos, p_widget_para->y_pos);
    WINDOW_SetBkColor(handle, p_widget_para->bk_color);
    GUI_Exec();
    return handle;
}


/**
  * @brief 删除弹窗
  * @param  p_widget_para:list控件结构体
  * @retval NULL
  */
void EmShutDownPopUpDel(emPopUpPara_t *p_widget_para)
{
    if(gstShutDownPopUpWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstShutDownPopUpWin.win_handle);
    gstShutDownPopUpWin.win_handle = 0;
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
    gstShutDownPopUpWin.win_handle = EmShutDownPopUpCreate(&_gstShutDownEmPopUpPara, pCurrentEmWin->win_handle);
    pCurrentPopUpSubWin = &gstShutDownPopUpWin;
    return gstShutDownPopUpWin.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstShutDownPopUpWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstShutDownPopUpWin.win_handle);
    _Del();
    gstShutDownPopUpWin.win_handle = 0;
    pCurrentPopUpSubWin = NULL;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    _gstUiCtx.turn_off_timeout = tim_get_systime();
    _gstUiCtx.shut_down = 0;
    _gstShutDownEmPopUpPara.bk_color = 0x006A7105;//后续根据错误还是警告信息填入合适的颜色
    _gstShutDownEmPopUpPara.x_pos = 115;
    _gstShutDownEmPopUpPara.y_pos = 122;
    _gstShutDownEmPopUpPara.x_size = 240;
    _gstShutDownEmPopUpPara.y_size = 95;

    _gstShutDownEmPopUpPara.popup_start_id = WIN_LIST_SHUT_DOWN_POPUP_ID;
    _gstShutDownEmPopUpPara.widget_num = 4;

    _gShutDownPopUpTitleMem = GUI_ALLOC_AllocZero(EM_SHUT_DOWN_POP_UP_ICON_ALERT_SIZE);
    _pShutDownPopUpTitleBuf = GUI_ALLOC_h2p(_gShutDownPopUpTitleMem);
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _pShutDownPopUpTitleBuf, EM_SHUT_DOWN_POP_UP_ICON_ALERT_SIZE);
    /*子控件参数初始化*/
    _gstShutDownEmPopUpPara.sub_widget_para[0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstShutDownEmPopUpPara.sub_widget_para[0].x_relative_pos = 12;
    _gstShutDownEmPopUpPara.sub_widget_para[0].y_relative_pos = 13;
    _gstShutDownEmPopUpPara.sub_widget_para[0].x_size = 21;
    _gstShutDownEmPopUpPara.sub_widget_para[0].y_size = 21;
    _gstShutDownEmPopUpPara.sub_widget_para[0].sub_widget.img_para.mem_img = _gShutDownPopUpTitleMem;
    _gstShutDownEmPopUpPara.sub_widget_para[0].sub_widget.img_para.p_img_buf = _pShutDownPopUpTitleBuf;
    _gstShutDownEmPopUpPara.sub_widget_para[0].sub_widget.img_para.img_size = EM_SHUT_DOWN_POP_UP_ICON_ALERT_SIZE;

    _gstShutDownEmPopUpPara.sub_widget_para[1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstShutDownEmPopUpPara.sub_widget_para[1].x_relative_pos = 53;
    _gstShutDownEmPopUpPara.sub_widget_para[1].y_relative_pos = 10;
    _gstShutDownEmPopUpPara.sub_widget_para[1].x_size = 202;
    _gstShutDownEmPopUpPara.sub_widget_para[1].y_size = 50;
    _gstShutDownEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content = _garrShutDownPopUpTitleBuf;
    _gstShutDownEmPopUpPara.sub_widget_para[1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstShutDownEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    strcpy(_gstShutDownEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content, "The machine is\nshutting down!");
    strcpy(_gstShutDownEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content, MSG_SHUTTING_DOWN);
#if 0
    _gstShutDownEmPopUpPara.sub_widget_para[2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstShutDownEmPopUpPara.sub_widget_para[2].x_relative_pos = 149;
    _gstShutDownEmPopUpPara.sub_widget_para[2].y_relative_pos = 72;
    _gstShutDownEmPopUpPara.sub_widget_para[2].x_size = 50;
    _gstShutDownEmPopUpPara.sub_widget_para[2].y_size = 17;
    _gstShutDownEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_content = _garrShutDownPopUpNOBuf;
    _gstShutDownEmPopUpPara.sub_widget_para[2].sub_widget.text_para.text_color = 0x00000000;
    _gstShutDownEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gstShutDownEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_content, "No");//后续根据错误还是警告信息填入

    _gstShutDownEmPopUpPara.sub_widget_para[3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstShutDownEmPopUpPara.sub_widget_para[3].x_relative_pos = 205;
    _gstShutDownEmPopUpPara.sub_widget_para[3].y_relative_pos = 72;
    _gstShutDownEmPopUpPara.sub_widget_para[3].x_size = 50;
    _gstShutDownEmPopUpPara.sub_widget_para[3].y_size = 17;
    _gstShutDownEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content = _garrShutDownPopUpYESBuf;
    _gstShutDownEmPopUpPara.sub_widget_para[3].sub_widget.text_para.text_color = 0x00000000;
    _gstShutDownEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gstShutDownEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content, "Yes");//后续根据错误还是警告信息填入
#endif
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gShutDownPopUpTitleMem);
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */


extern uint16_t gTestShutDown;
extern uint16_t gTestTurnOffFlag;

static void _Polling()
{
    _SetShutDownStatus();


#if 0
    if(tim_get_systime() - _gstUiCtx.turn_off_timeout > 1000 * 5)
    {
        if(pCurrentEmWin->win_handle != 0)
        {
            emWin_t *pnext = NULL;
            pnext = &gstEmWinHomePage;
            //        gstShutDownPopUpWin.delete_win();
//            pPrevEmWin = pCurrentEmWin;
//            pCurrentEmWin->delete_win();
            GUI_EndDialog(gstShutDownPopUpWin.win_handle, 0);
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
//           WM_HideWindow(gstShutDownPopUpWin.win_handle);

        }
    }
    _CheckShutDown();
#endif
}

/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
#if 0
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
        WM_Paint(gstShutDownPopUpWin.win_handle);
        return;
    }
    if (event == EN_KEY_SRC_RIGHT)
    {
        if(_gstUiCtx.select)
        {
            _gstUiCtx.select = 0;
        }
        WM_Paint(gstShutDownPopUpWin.win_handle);
        return;
    }
    if (event == EN_KEY_SRC_CONFIRM)
    {
        if (_gstUiCtx.select)
        {
            uint8_t arr_data_buff[16];
            uint16_t tmp = 0;
            memset(arr_data_buff, 0, sizeof(arr_data_buff));
            _gstUiCtx.shut_down = 0x01;
            //0x01准备恢复出厂设置
            tmp = BigLittleSwap16(_gstUiCtx.shut_down);
//            ModbusWriteSingleRemain(arr_data_buff, 0x01, 0x5241, tmp);

            return ;
        }
        _BackShutDown(1);
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
#endif
}






