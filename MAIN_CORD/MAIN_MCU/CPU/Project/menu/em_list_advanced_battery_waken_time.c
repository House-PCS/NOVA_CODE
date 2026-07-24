/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_reconn_grid_time.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-02
  * @brief       高级设置->设置->more->battery->waken time
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-02    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"

#define  TEST_DYNAMIC                                           0

#define     EM_BACK_ON_ICON_ARROW_SIZE                      (4096)
#define     DIGIT_NUMS                                      (2)
#define     REC_XPOS                                        (20)
#define     REC_YPOS                                        (38)


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinListAdvanSettingBatWakTime = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    int8_t num[DIGIT_NUMS];
    uint16_t wakn_time;
} UiCtx_t;

static UiCtx_t _gstUiCtx;
static int32_t _gWakeTime;
/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

/*Waken time列表*/
static TEXT_Handle  _gTextWakenTimeHandle;//显示Waken time句柄
static TEXT_Handle  _garrTextNumHandle[DIGIT_NUMS];
static TEXT_Handle  _gTextMinutesHandle;

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;
static emWidgetTextPara_t  _gstWakenTimeTextPara;
static emWidgetTextPara_t  _garrNumTextPara[DIGIT_NUMS];
static emWidgetTextPara_t  _gstMinutesTextPara;

static char _garrTitleBuf[] = "Setings < More < Battery < Waken";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";
static char _garrWakenTimeBuf[] = "Waken Time";
static char _garrMinutesBuf[] = "Min";
static char _garrWakenTimeNumTextBuf[DIGIT_NUMS][20];

#define WIN_LIST_WAKEN_TIME_ID                            (GUI_ID_USER + 0x12)
#define WIN_LIST_WAKEN_TIME_TITLE_ID                      (GUI_ID_USER + 0x13)
#define WIN_LIST_WAKEN_TIME_PRESS_ID                      (GUI_ID_USER + 0x14)
#define WIN_LIST_WAKEN_TIME_BACK_IMG_ID                   (GUI_ID_USER + 0x15)
#define WIN_LIST_WAKEN_TIME_BACK_ID                       (GUI_ID_USER + 0x16)


#define WIN_MAIN_WAKEN_TIME_ID                            (GUI_ID_USER + 0x20)
#define WIN_MAIN_NUMBER1_TEXT_ID                          (GUI_ID_USER + 0x21)
#define WIN_MAIN_NUMBER2_TEXT_ID                          (GUI_ID_USER + 0x22)
#define WIN_MAIN_MINUTES_TEXT_ID                          (GUI_ID_USER + 0x23)



/*资源列表*/
static const GUI_WIDGET_CREATE_INFO _garrListWakenTimeDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListWakenTimeWin",       WIN_LIST_WAKEN_TIME_ID,                     0,      0,      480,    320,    0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_WAKEN_TIME_TITLE_ID,               31,     20,     415,    14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_WAKEN_TIME_PRESS_ID,               133,    273,    33,     14,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_WAKEN_TIME_BACK_IMG_ID,            171 - 5,  275,    15,     10,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_WAKEN_TIME_BACK_ID,                188,    273,    150,    14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "WAKEN",          WIN_MAIN_WAKEN_TIME_ID,                     31,     48,     180,    17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM1",           WIN_MAIN_NUMBER1_TEXT_ID,                   390,    47,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM2",           WIN_MAIN_NUMBER2_TEXT_ID,                   406,    47,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "MINTUES",        WIN_MAIN_MINUTES_TEXT_ID,                   420,    48,     30,     17,     0,   0x0,  0 },

};

static int32_t _SetBatteryWakenTimeNumber(int32_t ui_update, uint16_t time)
{
    uint8_t i = 0;
    uint16_t time_tens  = (time / 10) % 10;
    uint16_t time_uints = (time % 10);

    snprintf(_garrWakenTimeNumTextBuf[0], sizeof(_garrWakenTimeNumTextBuf[0]), "%d", time_tens);
    snprintf(_garrWakenTimeNumTextBuf[1], sizeof(_garrWakenTimeNumTextBuf[1]), "%d", time_uints);

    if( ui_update)
    {
        for(i = 0; i < DIGIT_NUMS; ++i)
        {
            TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
        }
    }
    return 0;
}
/**
  * @brief  设置选中数字文本的颜色
  * @param  NULL
  * @retval NULL
  */
void _SetWakenTimeNumberColor(int32_t index)
{
    uint8_t i = 0;
    static volatile uint8_t arr_nums_sel_falg[DIGIT_NUMS] = {0};
    //_SetWakenTimeNumAdd(index);
    if(index < 0 || index >= DIGIT_NUMS)
    {
        return;
    }
    for(i = 0; i < DIGIT_NUMS; i++)
    {
        arr_nums_sel_falg[i] = 0;
        TEXT_SetTextColor(_garrTextNumHandle[i], 0x00FFFFFF);
    }
    arr_nums_sel_falg[index] = 1;
    TEXT_SetTextColor(_garrTextNumHandle[index], 0x00CDF100);
}

/**
  * @brief  设置选中数字变化
  * @param  NULL
  * @retval NULL
  */
void _SetWakenTimeNumAdd(int32_t index, int32_t v)
{
    _garrNumTextPara[index].digit_value = v;
    sprintf(_garrNumTextPara[index].p_content, "%1d", (int)_garrNumTextPara[index].digit_value);
    if(9 < _garrNumTextPara[index].digit_value)
    {
        _garrNumTextPara[index].digit_value = 0;
    }
    TEXT_SetText(_garrTextNumHandle[index], _garrWakenTimeNumTextBuf[index]);
}

/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
void _DrawWakenTimeNumUnderLine(int32_t index)
{
    GUI_SetColor(0x00CDF100);
    switch(index)
    {
        case 0:
        {
            GUI_DrawLine(_garrListWakenTimeDialogCreate[index + 6].x0 - 2, _garrListWakenTimeDialogCreate[index + 6].y0 + 18,
                         _garrListWakenTimeDialogCreate[index + 6].x0 - 2 + 12, _garrListWakenTimeDialogCreate[index + 6].y0 + 18);
        }
        break;
        case 1:
        {
            GUI_DrawLine(_garrListWakenTimeDialogCreate[index + 6].x0 - 2, _garrListWakenTimeDialogCreate[index + 6].y0 + 18,
                         _garrListWakenTimeDialogCreate[index + 6].x0 - 2 + 12, _garrListWakenTimeDialogCreate[index + 6].y0 + 18);
        }
        break;
        default:
            break;
    }
}

/**
  * @brief ListWakenTimeWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListWakenTimeWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    static int32_t num = -1;
    static int32_t index = -1;
#if TEST_DYNAMIC
    static WM_HTIMER wm_timer_handel_nums;
#endif
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
#if TEST_DYNAMIC
            wm_timer_handel_nums = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                                  0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                                  1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                                  0);         /* 留待将来使用，应为0 */
#endif
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_WAKEN_TIME_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            //列表中的文字
            _gTextWakenTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_WAKEN_TIME_ID);
            TEXT_SetFont(_gTextWakenTimeHandle, _gstWakenTimeTextPara.p_text_font);
            TEXT_SetText(_gTextWakenTimeHandle, (const char *)_gstWakenTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextWakenTimeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextWakenTimeHandle, _gstWakenTimeTextPara.text_color);
            //分钟的显示
            _gTextMinutesHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_MINUTES_TEXT_ID);
            TEXT_SetFont(_gTextMinutesHandle, _gstMinutesTextPara.p_text_font);
            TEXT_SetText(_gTextMinutesHandle, (const char *)_gstMinutesTextPara.p_content);
            TEXT_SetTextAlign(_gTextMinutesHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextMinutesHandle, _gstMinutesTextPara.text_color);

            for(i = 0; i < DIGIT_NUMS; i++)
            {
                _garrTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_NUMBER1_TEXT_ID + i);
                TEXT_SetFont(_garrTextNumHandle[i], _garrNumTextPara[i].p_text_font);
                TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
                TEXT_SetTextAlign(_garrTextNumHandle[i], TEXT_CF_RIGHT | TEXT_CF_VCENTER);
                TEXT_SetTextColor(_garrTextNumHandle[i], _garrNumTextPara[i].text_color);
            }

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_WAKEN_TIME_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_WAKEN_TIME_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_WAKEN_TIME_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            _SetWakenTimeNumberColor(_gstUiCtx.select);
            break;
        }
        case WM_DELETE:
        {
#if TEST_DYNAMIC
            WM_DeleteTimer(wm_timer_handel_nums); //删除窗口，会自动删除对应内部的定时器

#endif
            break;
        }
        case WM_TIMER:
        {
#if TEST_DYNAMIC
            ++num;
            index = num % DIGIT_NUMS;
            _SetDODNumberColor(index);

            WM_Paint(pMsg->hWin);
            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListWakenTimePara, _gEmListWakenTimePara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            GUI_FillRoundedRect(REC_XPOS, REC_YPOS,  REC_XPOS + 440 - 1,  REC_YPOS + 34 - 1, 10);
            GUI_SetColor(0x00BDC909);
            GUI_DrawRoundedFrame(REC_XPOS, REC_YPOS,  REC_XPOS + 440 - 1,  REC_YPOS + 34 - 1, 10, 3);

            GUI_SetPenSize(3);
            GUI_SetColor(0x00D9D9D9);
            for(i = 0; i < DIGIT_NUMS; i++)
            {
                GUI_DrawLine(_garrListWakenTimeDialogCreate[i + 6].x0 - 2, _garrListWakenTimeDialogCreate[i + 6].y0 + 18,
                             _garrListWakenTimeDialogCreate[i + 6].x0 - 2 + 12, _garrListWakenTimeDialogCreate[i + 6].y0 + 18);
            }
            _DrawWakenTimeNumUnderLine(_gstUiCtx.select);
//          _SetWakenTimeSelectedNumAndLine(index);
//            SelectedDrawInWMPaint(&_gEmListWakenTimePara, _gEmListWakenTimePara.list_sel);
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
  * @brief Create Factory Restore win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateFactoryRestoreWin()
{
    gstEmWinListAdvanSettingBatWakTime.win_handle = GUI_CreateDialogBox(_garrListWakenTimeDialogCreate,
            GUI_COUNTOF(_garrListWakenTimeDialogCreate),
            _cbListWakenTimeWin,
            pCurrentEmWin->win_handle,
            0,
            0);
    WM_SetTransState(gstEmWinListAdvanSettingBatWakTime.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    return gstEmWinListAdvanSettingBatWakTime.win_handle;
}


/**
  * @brief Create current win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWin(void)
{
    _gstUiCtx.key_init = 0;
    pPrevEmWin = &gstEmWinListAdvancedBattery;

    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListAdvanSettingBatWakTime.win_handle = GUI_CreateDialogBox(_garrListWakenTimeDialogCreate, GUI_COUNTOF(_garrListWakenTimeDialogCreate), _cbListWakenTimeWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListAdvanSettingBatWakTime.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvanSettingBatWakTime.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListAdvanSettingBatWakTime.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListWakenTimePara, gstEmWinListAdvanSettingBatWakTime.win_handle);
    return gstEmWinListAdvanSettingBatWakTime.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvanSettingBatWakTime.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvanSettingBatWakTime.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvanSettingBatWakTime.win_handle = 0;
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t i = 0;
    /*底部back的参数*/
    _gstPressTextPara.p_content = _garrPressBuf;
    _gstPressTextPara.text_color = 0x00EEEEEE;
    _gstPressTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstBackTextPara.p_content = _garrBackBuf;
    _gstBackTextPara.text_color = 0x00EEEEEE;
    _gstBackTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_BACK_ON_ICON_ARROW_SIZE);
    _gstReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gstReturnImgPara.mem_img);
    _gstReturnImgPara.img_size = EM_BACK_ON_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

    /*列表中的文字*/
    _gstWakenTimeTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstWakenTimeTextPara.text_color = 0x00FFFFFF;
    _gstWakenTimeTextPara.p_content = _garrWakenTimeBuf;

    _gstMinutesTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstMinutesTextPara.text_color = 0x00FFFFFF;
    _gstMinutesTextPara.p_content = _garrMinutesBuf;

    for(i = 0; i < DIGIT_NUMS; i++)
    {
        _garrNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
        _garrNumTextPara[i].text_color = 0x00FFFFFF;
        _garrNumTextPara[i].p_content = _garrWakenTimeNumTextBuf[i];
//      _garrNumTextPara[i].digit_value = 0;
//      sprintf(_garrNumTextPara[i].p_content, "%1d", (int)_garrNumTextPara[i].digit_value);
    }
    _SetBatteryWakenTimeNumber(0, gstCollectParaData.collect_data.batt_wakeup_time);

    /*标题*/
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00EEEEEE;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gstReturnImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    if(_gstUiCtx.wakn_time != gstCollectParaData.collect_data.batt_wakeup_time)
    {
        _gstUiCtx.wakn_time = gstCollectParaData.collect_data.batt_wakeup_time;
        _SetBatteryWakenTimeNumber(1, _gstUiCtx.wakn_time);
    }
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
    if(event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }

    if(event == EN_KEY_SRC_RIGHT)
    {
        if(_gstUiCtx.select >= DIGIT_NUMS)
        {
            return;
        }
        _gstUiCtx.num[_gstUiCtx.select] = abs(10 + (--(_gstUiCtx.num[_gstUiCtx.select]))) % 10;
        _SetWakenTimeNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        return;
    }
    else if(event == EN_KEY_SRC_LEFT )
    {
        if(_gstUiCtx.select >= DIGIT_NUMS)
        {
            return;
        }
        _gstUiCtx.num[_gstUiCtx.select] = (++(_gstUiCtx.num[_gstUiCtx.select])) % 10;
        _SetWakenTimeNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        return;
    }
    else if(event == EN_KEY_SRC_CONFIRM)
    {
        _gstUiCtx.select = (++_gstUiCtx.select) % DIGIT_NUMS;
        _SetWakenTimeNumberColor(_gstUiCtx.select);

        WM_Paint(gstEmWinListAdvanSettingBatWakTime.win_handle);
    }
    else if(event == EN_KEY_SRC_ESC)
    {
        uint8_t arr_data_buff[256 + 8] = {0};
        uint16_t waken_time = 0;
        char arr_waken_time_str[8] = {0};

        snprintf(arr_waken_time_str, sizeof(arr_waken_time_str), "%c%c", _garrWakenTimeNumTextBuf[0][0], _garrWakenTimeNumTextBuf[1][0]);
        waken_time = atoi((const char *)arr_waken_time_str);
        waken_time = BigLittleSwap16(waken_time);
        ModbusWriteSingleRemain(arr_data_buff, 0x01, COLLECT_BATT_WAKE_TIME_ADDR, waken_time);

        if(pCurrentEmWin->win_handle != 0)
        {
            emWin_t *pnext = NULL;
            if(p_msg->press_down_time >= gstEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
            {
                pnext = &gstEmWinHomePage;
            }
            else
            {
                pnext = &gstEmWinListAdvancedBattery;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
    }
}

/**
  * @brief  获取两个数字对应的十进制数
  * @param  NULL
  * @retval NULL
  */
int32_t GetWakeTimeValue(void)
{
    _gWakeTime = (int32_t)(_garrNumTextPara[0].digit_value) * 10 + _garrNumTextPara[1].digit_value;
    return _gWakeTime;
}
