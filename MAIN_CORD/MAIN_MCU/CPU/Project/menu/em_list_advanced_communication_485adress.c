/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opt_parm_reactive_respon_tim.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-22
  * @brief       高级设置->设置->more->Communication->485 Address（无功响应时间）
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"

#define  TEST_DYNAMIC                                       0

#define     EM_BACK_ON_ICON_ARROW_SIZE                      (4096)
#define     DIGIT_NUMS                                      (1)
#define     REC1_XPOS                                       (20)
#define     REC1_YPOS                                       (38)
#define     DIALOG_NUM_OFFSET                               (6)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinListAdvanSettingCommunicateAddr = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t num[DIGIT_NUMS];
    uint16_t addr_485;
} UiCtx_t;

static UiCtx_t _gstUiCtx = {0, 0, {1}};
/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

/*Power列表*/
static TEXT_Handle  _gText485AddressHandle;//显示485Address句柄
static TEXT_Handle  _garrTextNumHandle[DIGIT_NUMS];

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;
static emWidgetTextPara_t  _gst485AddressTextPara;
static emWidgetTextPara_t  _garrNumTextPara[DIGIT_NUMS];

static char _garrTitleBuf[] = "Setings < More < Communication < 485 Address";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";
static char _garr485AddressBuf[] = "485 Address";
static char _garr485AddressTextBuf[DIGIT_NUMS][20];

#define WIN_LIST_485_ADDRESS_ID                                 (GUI_ID_USER + 0x12)
#define WIN_LIST_485_ADDRESS_TITLE_ID                           (GUI_ID_USER + 0x13)
#define WIN_LIST_485_ADDRESS_PRESS_ID                           (GUI_ID_USER + 0x14)
#define WIN_LIST_485_ADDRESS_BACK_ID                            (GUI_ID_USER + 0x15)
#define WIN_LIST_485_ADDRESS_BACK_IMG_ID                        (GUI_ID_USER + 0x16)

#define WIN_MAIN_485_ADDRESS_ID                                 (GUI_ID_USER + 0x20)
#define WIN_MAIN_NUMBER1_ID                                     (GUI_ID_USER + 0x21)


/*资源列表*/
static __SDRAM GUI_WIDGET_CREATE_INFO _garrList485AddressDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListDODWin",     WIN_LIST_485_ADDRESS_ID,                    0,      0,      480,    320,    0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_485_ADDRESS_TITLE_ID,              31,     20,     415,    14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_485_ADDRESS_PRESS_ID,              133,    273,    33,     14,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_485_ADDRESS_BACK_IMG_ID,           171 - 5,  275,    15,     10,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_485_ADDRESS_BACK_ID,               188,    273,    150,    14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "485 address",    WIN_MAIN_485_ADDRESS_ID,                    31,     48,     300,    17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM1",           WIN_MAIN_NUMBER1_ID,                        411,    47,     32,     17,     0,   0x0,  0 },
};

static int32_t _Set485AddressNumber(int32_t ui_update, uint16_t address)
{
    snprintf(_garr485AddressTextBuf[0], sizeof(_garr485AddressTextBuf[0]), "%d", address);
    _gstUiCtx.addr_485 = atoi(_garr485AddressTextBuf[0]);
    if(ui_update)
    {
        TEXT_SetText(_garrTextNumHandle[0], (const char *)_garr485AddressTextBuf[0]);
    }
    return 0;
}


/**
  * @brief  设置选中数字文本的颜色
  * @param  NULL
  * @retval NULL
  */
static void _Set485AddressNumberColor(int32_t index)
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
static void _Set485AddressNumAdd(int32_t index, int32_t v)
{
    _garrNumTextPara[index].digit_value = v;
    _gstUiCtx.addr_485 = (uint16_t)v;
    sprintf(_garrNumTextPara[index].p_content, "%1d", (int)_garrNumTextPara[index].digit_value);
    if(10 < _garrNumTextPara[index].digit_value)
    {
        _garrNumTextPara[index].digit_value = 10;
    }
    TEXT_SetText(_garrTextNumHandle[index], _garr485AddressTextBuf[index]);
}

/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
static void _Draw485AddressNumUnderLine(int32_t index)
{
    GUI_SetColor(0x00CDF100);
    switch(index)
    {
        case 0:
        {
            GUI_DrawLine(_garrList485AddressDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrList485AddressDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrList485AddressDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 36, _garrList485AddressDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
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
static void _DrawSel485AddressRecPaint(void)
{
    GUI_SetColor(0x00BDC909);
    GUI_DrawRoundedFrame(REC1_XPOS, REC1_YPOS,  REC1_XPOS + 440 - 1,  REC1_YPOS + 34 - 1, 10, 3);
    return;
}

/**
  * @brief  List485AddressWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbList485AddressWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_485_ADDRESS_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            //列表中的文字
            //485 address
            _gText485AddressHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_485_ADDRESS_ID);
            TEXT_SetFont(_gText485AddressHandle, _gst485AddressTextPara.p_text_font);
            TEXT_SetText(_gText485AddressHandle, (const char *)_gst485AddressTextPara.p_content);
            TEXT_SetTextAlign(_gText485AddressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gText485AddressHandle, _gst485AddressTextPara.text_color);

            for(i = 0; i < DIGIT_NUMS; i++)
            {
                _garrTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_NUMBER1_ID + i);
                TEXT_SetFont(_garrTextNumHandle[i], _garrNumTextPara[i].p_text_font);
                TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
                TEXT_SetTextAlign(_garrTextNumHandle[i], TEXT_CF_HCENTER | TEXT_CF_VCENTER);
                TEXT_SetTextColor(_garrTextNumHandle[i], _garrNumTextPara[i].text_color);
            }

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_485_ADDRESS_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_485_ADDRESS_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_485_ADDRESS_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            _Set485AddressNumberColor(_gstUiCtx.select);
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
            _Set485AddressNumberColor(index);

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

            GUI_SetPenSize(3);
            GUI_SetColor(0x00D9D9D9);
            //绘制数字底部横线
            for(i = 0; i < DIGIT_NUMS; i++)
            {
                GUI_DrawLine(_garrList485AddressDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2, _garrList485AddressDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18,
                             _garrList485AddressDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2 + 36, _garrList485AddressDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18);
            }
#if TEST_DYNAMIC
            _DrawSel485AddressRecPaint(selected % 2);
            _Draw485AddressNumUnderLine(index);
#else
            _DrawSel485AddressRecPaint();
            _Draw485AddressNumUnderLine(_gstUiCtx.select);
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
  * @brief Create current win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWin(void)
{
    _gstUiCtx.key_init = 0;
    pPrevEmWin = &gstEmWinListAdvancedSettings;

    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListAdvanSettingCommunicateAddr.win_handle = GUI_CreateDialogBox(_garrList485AddressDialogCreate, GUI_COUNTOF(_garrList485AddressDialogCreate), _cbList485AddressWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListAdvanSettingCommunicateAddr.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvanSettingCommunicateAddr.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListAdvanSettingCommunicateAddr.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListDODPara, gstEmWinListAdvanSettingCommunicateAddr.win_handle);
    return gstEmWinListAdvanSettingCommunicateAddr.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvanSettingCommunicateAddr.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvanSettingCommunicateAddr.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvanSettingCommunicateAddr.win_handle = 0;
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t i = 0;
    _gstUiCtx.addr_485 = gstCollectInfo.maincard_data.base_info.modbus_addr;
    
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

    /*标题*/
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00EEEEEE;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;


    /*列表中的文字*/
    //485address
    _gst485AddressTextPara.p_text_font = EM_TEXT_FONT_5;
    _gst485AddressTextPara.text_color = 0x00FFFFFF;
    _gst485AddressTextPara.p_content = _garr485AddressBuf;

    for(i = 0; i < DIGIT_NUMS; i++)
    {
        _garrNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
        _garrNumTextPara[i].text_color = 0x00FFFFFF;
        _garrNumTextPara[i].p_content = _garr485AddressTextBuf[i];
//      _garrNumTextPara[i].digit_value = 0;
//      sprintf(_garrNumTextPara[i].p_content, "%1d", (int)_garrNumTextPara[i].digit_value);
    }
    _Set485AddressNumber(0, _gstUiCtx.addr_485);
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
    if(_gstUiCtx.addr_485 != gstCollectInfo.maincard_data.base_info.modbus_addr)
    {
        _gstUiCtx.addr_485 = gstCollectInfo.maincard_data.base_info.modbus_addr;
        _Set485AddressNumber(1, _gstUiCtx.addr_485);
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
        _gstUiCtx.num[_gstUiCtx.select] = (--_gstUiCtx.num[_gstUiCtx.select]) < 1 ? 254 : _gstUiCtx.num[_gstUiCtx.select];
        _Set485AddressNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        return;
    }
    else if(event == EN_KEY_SRC_LEFT)
    {
        if(_gstUiCtx.select >= DIGIT_NUMS)
        {
            return;
        }
        _gstUiCtx.num[_gstUiCtx.select] = (++_gstUiCtx.num[_gstUiCtx.select]) > 254 ? 1 : _gstUiCtx.num[_gstUiCtx.select];
        _Set485AddressNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        return;
    }
    else if(event == EN_KEY_SRC_CONFIRM)
    {
        _gstUiCtx.select = (++_gstUiCtx.select) % DIGIT_NUMS;
        _Set485AddressNumberColor(_gstUiCtx.select);
        WM_Paint(gstEmWinListAdvanSettingCommunicateAddr.win_handle);
    }
    else if(event == EN_KEY_SRC_ESC)
    {
        uint8_t arr_data_buff[256] = {0};
        uint16_t tmp = 0;

        tmp = BigLittleSwap16(_gstUiCtx.addr_485);
        ModbusWriteSingleRemain(arr_data_buff, 0x01, 0x502B, tmp);

        if(pCurrentEmWin->win_handle != 0)
        {
            emWin_t *pnext = NULL;
            if(p_msg->press_down_time >= gstEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
            {
                pnext = &gstEmWinHomePage;
            }
            else
            {
                pnext = &gstEmWinListAdvanceCommunication;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
    }
}

