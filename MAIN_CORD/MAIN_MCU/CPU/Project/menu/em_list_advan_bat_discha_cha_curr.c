/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_reconn_grid_time.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-01
  * @brief       高级设置->设置->more->battery->current(充放电电流)
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-01    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"

#define  TEST_DYNAMIC                                           0

#define     EM_BACK_ON_ICON_ARROW_SIZE                      (4096)
#define     DIGIT_NUMS                                      (5)
#define     REC1_XPOS                                       (20)
#define     REC1_YPOS                                       (38)
#define     REC2_XPOS                                       (20)
#define     REC2_YPOS                                       (80)
#define     DIALOG_NUM_OFFSET                               (7)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gstEmWinListAdvanSettingBatCurrent = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    int16_t num[DIGIT_NUMS];
    uint16_t charge_curr;
    uint16_t discharge_curr;
} UiCtx_t;

static UiCtx_t _gstUiCtx;
/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

/*Current列表*/
static TEXT_Handle  _gTextChargeCurHandle;//显示charge Current句柄
static TEXT_Handle  _gTextDischargeCurHandle;//discharge current句柄
static TEXT_Handle  _garrTextNumHandle[DIGIT_NUMS];
static TEXT_Handle  _gTextAmpereOneHandle;
static TEXT_Handle  _gTextAmpereTwoHandle;

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;
static emWidgetTextPara_t  _gstChargeCurTextPara;
static emWidgetTextPara_t  _gstDischargeCurTextPara;
static emWidgetTextPara_t  _garrNumTextPara[DIGIT_NUMS];
static emWidgetTextPara_t  _gstAmpereOneTextPara;
static emWidgetTextPara_t  _gstAmpereTwoTextPara;

static char _garrTitleBuf[] = "Setings < More < Battery < Current";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";
static char _garrChargeBuf[] = "Max Charge Current";
static char _garrDischargeBuf[] = "Max Discharge Current";
static char _garrAmpereOneBuf[] = "A";
static char _garrAmpereTwoBuf[] = "A";
static char _garrBatteryCurrentNumTextBuf[DIGIT_NUMS][20];


#define WIN_LIST_BAT_CUR_ID                                 (GUI_ID_USER + 0x12)
#define WIN_LIST_BAT_CUR_TITLE_ID                           (GUI_ID_USER + 0x13)
#define WIN_LIST_BAT_CUR_PRESS_ID                           (GUI_ID_USER + 0x14)
#define WIN_LIST_BAT_CUR_BACK_ID                            (GUI_ID_USER + 0x15)
#define WIN_LIST_BAT_CUR_BACK_IMG_ID                        (GUI_ID_USER + 0x16)

#define WIN_MAIN_CHARGE_CUR_ID                              (GUI_ID_USER + 0x20)
#define WIN_MAIN_DISCHARGE_CUR_ID                           (GUI_ID_USER + 0x21)
#define WIN_MAIN_NUMBER1_ID                                 (GUI_ID_USER + 0x22)
#define WIN_MAIN_NUMBER2_ID                                 (GUI_ID_USER + 0x23)
#define WIN_MAIN_NUMBER3_ID                                 (GUI_ID_USER + 0x24)
#define WIN_MAIN_NUMBER4_ID                                 (GUI_ID_USER + 0x25)
#define WIN_MAIN_NUMBER5_ID                                 (GUI_ID_USER + 0x26)
#define WIN_MAIN_AMPERE_ONE_ID                              (GUI_ID_USER + 0x27)
#define WIN_MAIN_AMPERE_TWO_ID                              (GUI_ID_USER + 0x28)



/*资源列表*/
static __SDRAM GUI_WIDGET_CREATE_INFO _garrListBatCurDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListDODWin",     WIN_LIST_BAT_CUR_ID,                    0,      0,      480,    320,    0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_BAT_CUR_TITLE_ID,              31,     20,     415,    14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_BAT_CUR_PRESS_ID,              133,    273,    33,     14,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_BAT_CUR_BACK_IMG_ID,           171 - 5,  275,    15,     10,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_BAT_CUR_BACK_ID,               188,    273,    150,    14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "cha_cur",        WIN_MAIN_CHARGE_CUR_ID,                 31,     48,     220,    17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "discha_cur",     WIN_MAIN_DISCHARGE_CUR_ID,              31,     90,     220,    17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM1",           WIN_MAIN_NUMBER1_ID,                    406,    47,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM2",           WIN_MAIN_NUMBER2_ID,                    422,    47,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM3",           WIN_MAIN_NUMBER3_ID,                    390,    89,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM4",           WIN_MAIN_NUMBER4_ID,                    406,    89,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM5",           WIN_MAIN_NUMBER5_ID,                    422,    89,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "ampere1",        WIN_MAIN_AMPERE_ONE_ID,                 437,    48,     15,     17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "ampere2",        WIN_MAIN_AMPERE_TWO_ID,                 437,    90,     15,     17,     0,   0x0,  0 },

};

static int32_t _SetChargeAndDischCurr(int32_t ui_update, uint16_t charge_cur, uint16_t dischar_curr)
{
    uint16_t i = 0;
    uint16_t j = 0;

    uint16_t current = charge_cur / 10;
    uint16_t ch_tens  = current / 10;
    uint16_t ch_uints = current % 10;

    current = dischar_curr / 10;
    uint16_t disch_hundred = (current / 100);
    uint16_t disch_tens    = (current / 10) % 10;
    uint16_t disch_uints   = (current % 10);
    
    snprintf(_garrBatteryCurrentNumTextBuf[0], sizeof(_garrBatteryCurrentNumTextBuf[0]), "%d", ch_tens);
    snprintf(_garrBatteryCurrentNumTextBuf[1], sizeof(_garrBatteryCurrentNumTextBuf[1]), "%d", ch_uints);

    snprintf(_garrBatteryCurrentNumTextBuf[2], sizeof(_garrBatteryCurrentNumTextBuf[2]), "%d", disch_hundred);
    snprintf(_garrBatteryCurrentNumTextBuf[3], sizeof(_garrBatteryCurrentNumTextBuf[3]), "%d", disch_tens);
    snprintf(_garrBatteryCurrentNumTextBuf[4], sizeof(_garrBatteryCurrentNumTextBuf[4]), "%d", disch_uints);

    for(j = 0; j < DIGIT_NUMS; j++)
    {
        _gstUiCtx.num[j] = atoi((const char *)&_garrBatteryCurrentNumTextBuf[j][0]);
    }

    if(ui_update)
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
static void _SetBatCurNumberColor(int32_t index)
{
    uint8_t i = 0;
    static volatile uint8_t arr_nums_sel_falg[DIGIT_NUMS] = {0};
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
static void _SetBatCurNumAdd(int32_t index, int32_t v)
{
    _garrNumTextPara[index].digit_value = v;
    sprintf(_garrNumTextPara[index].p_content, "%1d", (int)_garrNumTextPara[index].digit_value);
    if(9 < _garrNumTextPara[index].digit_value)
    {
        _garrNumTextPara[index].digit_value = 0;
    }
    TEXT_SetText(_garrTextNumHandle[index], _garrBatteryCurrentNumTextBuf[index]);
}

/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
static void _DrawBatCurNumUnderLine(int32_t index)
{
    GUI_SetColor(0x00CDF100);
    switch(index)
    {
        case 0:
        {
            GUI_DrawLine(_garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 1:
        {
            GUI_DrawLine(_garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 2:
        {
            GUI_DrawLine(_garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 3:
        {
            GUI_DrawLine(_garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 4:
        {
            GUI_DrawLine(_garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListBatCurDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        default:
            break;
    }
}
/**
  * @brief  绘制选中框
  * @param  NULL
  * @retval NULL
  */
static void _DrawSelBatCurRecPaint(int32_t sel)
{
    if(sel == 0)
    {
        GUI_SetColor(0x00BDC909);
        GUI_DrawRoundedFrame(REC1_XPOS, REC1_YPOS,  REC1_XPOS + 440 - 1,  REC1_YPOS + 34 - 1, 10, 3);
    }
    else if(sel == 1 || sel == 2)
    {
        GUI_SetColor(0x00BDC909);
        GUI_DrawRoundedFrame(REC2_XPOS, REC2_YPOS,  REC2_XPOS + 440 - 1,  REC2_YPOS + 34 - 1, 10, 3);
    }
    return;
}

/**
  * @brief ListBatCurWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListBatCurWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    static int32_t num = -1;
    static int32_t index = -1;
    static int32_t selected = -1;
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BAT_CUR_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            //列表中的文字
            //charge current
            _gTextChargeCurHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_CHARGE_CUR_ID);
            TEXT_SetFont(_gTextChargeCurHandle, _gstChargeCurTextPara.p_text_font);
            TEXT_SetText(_gTextChargeCurHandle, (const char *)_gstChargeCurTextPara.p_content);
            TEXT_SetTextAlign(_gTextChargeCurHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextChargeCurHandle, _gstChargeCurTextPara.text_color);
            //discharge current
            _gTextDischargeCurHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_DISCHARGE_CUR_ID);
            TEXT_SetFont(_gTextDischargeCurHandle, _gstDischargeCurTextPara.p_text_font);
            TEXT_SetText(_gTextDischargeCurHandle, (const char *)_gstDischargeCurTextPara.p_content);
            TEXT_SetTextAlign(_gTextDischargeCurHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextDischargeCurHandle, _gstDischargeCurTextPara.text_color);

            //安培的显示
            //第一行安培
            _gTextAmpereOneHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_AMPERE_ONE_ID);
            TEXT_SetFont(_gTextAmpereOneHandle, _gstAmpereOneTextPara.p_text_font);
            TEXT_SetText(_gTextAmpereOneHandle, (const char *)_gstAmpereOneTextPara.p_content);
            TEXT_SetTextAlign(_gTextAmpereOneHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextAmpereOneHandle, _gstAmpereOneTextPara.text_color);
            //第二行安培
            _gTextAmpereTwoHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_AMPERE_TWO_ID);
            TEXT_SetFont(_gTextAmpereTwoHandle, _gstAmpereTwoTextPara.p_text_font);
            TEXT_SetText(_gTextAmpereTwoHandle, (const char *)_gstAmpereTwoTextPara.p_content);
            TEXT_SetTextAlign(_gTextAmpereTwoHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextAmpereTwoHandle, _gstAmpereTwoTextPara.text_color);
            for(i = 0; i < DIGIT_NUMS; i++)
            {
                _garrTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_NUMBER1_ID + i);
                TEXT_SetFont(_garrTextNumHandle[i], _garrNumTextPara[i].p_text_font);
                TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
                TEXT_SetTextAlign(_garrTextNumHandle[i], TEXT_CF_RIGHT | TEXT_CF_VCENTER);
                TEXT_SetTextColor(_garrTextNumHandle[i], _garrNumTextPara[i].text_color);
            }

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BAT_CUR_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BAT_CUR_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BAT_CUR_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            _SetBatCurNumberColor(_gstUiCtx.select);
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
            ++selected;
            index = num % DIGIT_NUMS;
            _SetBatCurNumberColor(index);

            WM_Paint(pMsg->hWin);
            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListDODPara, _gEmListDODPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            GUI_FillRoundedRect(REC1_XPOS, REC1_YPOS,  REC1_XPOS + 440 - 1,  REC1_YPOS + 34 - 1, 10);
            GUI_FillRoundedRect(REC2_XPOS, REC2_YPOS,  REC2_XPOS + 440 - 1,  REC2_YPOS + 34 - 1, 10);

            GUI_SetPenSize(3);
            GUI_SetColor(0x00D9D9D9);
            for(i = 0; i < DIGIT_NUMS; i++)
            {
                GUI_DrawLine(_garrListBatCurDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2, _garrListBatCurDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18,
                             _garrListBatCurDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListBatCurDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18);
            }
#if TEST_DYNAMIC
            _DrawSelBatCurRecPaint(selected % 2);
            _DrawBatCurNumUnderLine(index);
#else
            _DrawSelBatCurRecPaint(_gstUiCtx.select / (DIGIT_NUMS / 2));
            _DrawBatCurNumUnderLine(_gstUiCtx.select);
#endif
//          _SetDODSelectedNumAndLine(index);

//            SelectedDrawInWMPaint(&_gEmListDODPara, _gEmListDODPara.list_sel);
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
    gstEmWinListAdvanSettingBatCurrent.win_handle = GUI_CreateDialogBox(_garrListBatCurDialogCreate,
            GUI_COUNTOF(_garrListBatCurDialogCreate),
            _cbListBatCurWin,
            pCurrentEmWin->win_handle,
            0,
            0);
    WM_SetTransState(gstEmWinListAdvanSettingBatCurrent.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    return gstEmWinListAdvanSettingBatCurrent.win_handle;
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
    gstEmWinListAdvanSettingBatCurrent.win_handle = GUI_CreateDialogBox(_garrListBatCurDialogCreate, GUI_COUNTOF(_garrListBatCurDialogCreate), _cbListBatCurWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListAdvanSettingBatCurrent.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvanSettingBatCurrent.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListAdvanSettingBatCurrent.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListDODPara, gstEmWinListAdvanSettingBatCurrent.win_handle);
    return gstEmWinListAdvanSettingBatCurrent.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvanSettingBatCurrent.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvanSettingBatCurrent.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvanSettingBatCurrent.win_handle = 0;
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
    //charge cur
    _gstChargeCurTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstChargeCurTextPara.text_color = 0x00FFFFFF;
    _gstChargeCurTextPara.p_content = _garrChargeBuf;
    //discharge cur
    _gstDischargeCurTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstDischargeCurTextPara.text_color = 0x00FFFFFF;
    _gstDischargeCurTextPara.p_content = _garrDischargeBuf;
    //ampere1
    _gstAmpereOneTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstAmpereOneTextPara.text_color = 0x00FFFFFF;
    _gstAmpereOneTextPara.p_content = _garrAmpereOneBuf;
    //ampere2
    _gstAmpereTwoTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstAmpereTwoTextPara.text_color = 0x00FFFFFF;
    _gstAmpereTwoTextPara.p_content = _garrAmpereTwoBuf;

    for(i = 0; i < DIGIT_NUMS; i++)
    {
        _garrNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
        _garrNumTextPara[i].text_color = 0x00FFFFFF;
        _garrNumTextPara[i].p_content = _garrBatteryCurrentNumTextBuf[i];
//      _garrNumTextPara[i].digit_value = 0;
//      sprintf(_garrNumTextPara[i].p_content, "%1d", (int)_garrNumTextPara[i].digit_value);
    }

    _SetChargeAndDischCurr(0, gstCollectInfo.maincard_data.sys_info.max_chg_curr_set,
                              gstCollectInfo.maincard_data.sys_info.max_dsg_curr_set);

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
    if(_gstUiCtx.charge_curr    != gstCollectInfo.maincard_data.sys_info.max_chg_curr_set  || 
       _gstUiCtx.discharge_curr != gstCollectInfo.maincard_data.sys_info.max_dsg_curr_set)
    {
        _gstUiCtx.charge_curr    = gstCollectInfo.maincard_data.sys_info.max_chg_curr_set;
        _gstUiCtx.discharge_curr = gstCollectInfo.maincard_data.sys_info.max_dsg_curr_set;

        _SetChargeAndDischCurr(1, _gstUiCtx.charge_curr, _gstUiCtx.discharge_curr);
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
        #if 0
        _gstUiCtx.num[_gstUiCtx.select] = abs(10 + (--(_gstUiCtx.num[_gstUiCtx.select]))) % 10;
        _SetBatCurNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        #else
        //充电
        if(_gstUiCtx.select < 2)
        {
            _gstUiCtx.num[_gstUiCtx.select] = abs(10 + (--(_gstUiCtx.num[_gstUiCtx.select]))) % 10;
            _SetBatCurNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        }
        //放电
        if(_gstUiCtx.select >= 2)
        {
            //百位
            if(_gstUiCtx.select == 2)
            {
                if((_gstUiCtx.num[3] == 0) && (_gstUiCtx.num[4] == 0))
                {
                    _gstUiCtx.num[_gstUiCtx.select] = --(_gstUiCtx.num[_gstUiCtx.select]) < 0 ? 1 : _gstUiCtx.num[_gstUiCtx.select];
                }
                else
                {
                    _gstUiCtx.num[_gstUiCtx.select] = 0;
                } 
                _SetBatCurNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
            }
            else  //其他位
            {
                if(_gstUiCtx.num[2] == 1)
                {
                    _gstUiCtx.num[3] = 0;
                    _gstUiCtx.num[4] = 0;
                    _SetBatCurNumAdd(3, _gstUiCtx.num[3]);
                    _SetBatCurNumAdd(4, _gstUiCtx.num[4]);
                }
                else
                {
                    _gstUiCtx.num[_gstUiCtx.select] = abs(10 + (--(_gstUiCtx.num[_gstUiCtx.select]))) % 10;
                    _SetBatCurNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
                }
            }
        }
        #endif
        return;
    }
    else if(event == EN_KEY_SRC_LEFT)
    {
        if(_gstUiCtx.select >= DIGIT_NUMS)
        {
            return;
        }
        #if 0
        _gstUiCtx.num[_gstUiCtx.select] = (++(_gstUiCtx.num[_gstUiCtx.select])) % 10;
        _SetBatCurNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        #else
        //充电
        if(_gstUiCtx.select < 2)
        {
            _gstUiCtx.num[_gstUiCtx.select] = (++(_gstUiCtx.num[_gstUiCtx.select])) % 10;
            _SetBatCurNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        }
        //放电
        if(_gstUiCtx.select >= 2)
        {
            //百位
            if(_gstUiCtx.select == 2)
            {
                if((_gstUiCtx.num[3] == 0) && (_gstUiCtx.num[4] == 0))
                {
                    _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 1 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
                }
                else
                {
                    _gstUiCtx.num[_gstUiCtx.select] = 0;
                } 
                _SetBatCurNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
            }
            else  //其他位
            {
                if(_gstUiCtx.num[2] == 1)
                {
                    _gstUiCtx.num[3] = 0;
                    _gstUiCtx.num[4] = 0;
                    _SetBatCurNumAdd(3, _gstUiCtx.num[3]);
                    _SetBatCurNumAdd(4, _gstUiCtx.num[4]);
                }
                else
                {
                    _gstUiCtx.num[_gstUiCtx.select] = abs(10 + (++(_gstUiCtx.num[_gstUiCtx.select]))) % 10;
                    _SetBatCurNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
                }
            }
        }
        #endif
        return;
    }
    else if(event == EN_KEY_SRC_CONFIRM)
    {
        _gstUiCtx.select = (++_gstUiCtx.select) % DIGIT_NUMS;
        _SetBatCurNumberColor(_gstUiCtx.select);
        WM_Paint(gstEmWinListAdvanSettingBatCurrent.win_handle);
    }
    else if(event == EN_KEY_SRC_ESC)
    {
        uint8_t arr_data_chg[256 + 8] = {0};
        uint8_t arr_data_dsg[256 + 8] = {0};
        uint16_t chg_curr = 0;
        uint16_t dischg_curr = 0;
        char arr_chg_curr_str[8] = {0};
        char arr_dsg_curr_str[8] = {0};

        snprintf(arr_chg_curr_str, sizeof(arr_chg_curr_str), "%c%c", _garrBatteryCurrentNumTextBuf[0][0], _garrBatteryCurrentNumTextBuf[1][0]);
        snprintf(arr_dsg_curr_str, sizeof(arr_dsg_curr_str), "%c%c%c", _garrBatteryCurrentNumTextBuf[2][0], _garrBatteryCurrentNumTextBuf[3][0], _garrBatteryCurrentNumTextBuf[4][0]);
        chg_curr = atoi((const char *)arr_chg_curr_str) * 10;
        dischg_curr = atoi((const char *)arr_dsg_curr_str) * 10;
        chg_curr = BigLittleSwap16(chg_curr);
        dischg_curr = BigLittleSwap16(dischg_curr);
        ModbusWriteSingleRemain(arr_data_chg, 0x01, 0x5236, chg_curr);
        ModbusWriteSingleRemain(arr_data_dsg, 0x01, 0x5237, dischg_curr);

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


