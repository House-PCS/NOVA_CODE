/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_restore_factory.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-21
  * @brief       高级设置->恢复出厂
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

#define     TEST_DYNAMIC                                    0

#define     EM_BACK_ON_ICON_ARROW_SIZE                      (4096)
#define     DIGIT_NUMS                                      (6)
#define     REC_XPOS                                        (20)
#define     REC_YPOS                                        (38)
#define     DIALOG_NUM_OFFSET                               (4)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinListAdvanSettingFactoryRestore = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    int8_t num[DIGIT_NUMS];
} UiCtx_t;

static UiCtx_t _gstUiCtx;
/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gBottomHandle;//底部文字句柄

/*Password列表*/
static TEXT_Handle  _gTextRestoreFactoryHandle;//显示Restore Factory句柄
static TEXT_Handle  _garrTextNumHandle[DIGIT_NUMS];

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstBottomTextPara;
static emWidgetTextPara_t  _gstRestoreFactoryTextPara;
static emWidgetTextPara_t _garrNumTextPara[DIGIT_NUMS];

static char _garrTitleBuf[] = "Settings < More < Restore Factory";
static char _garrBottomBuf[] = "* Use this function with caution! Maintenance personnel must request the dynamic 6-digit password from customer service.";
static char _garrRestoreFactoryBuf[] = "Restore Factory";
static char _garrFactoryRestoreNumTextBuf[DIGIT_NUMS][20];


#define WIN_LIST_ADVANCED_FACTORYRESTORE_ID                                  (GUI_ID_USER + 0x12)
#define WIN_LIST_ADVANCED_FACTORYRESTORE_TITLE_ID                            (GUI_ID_USER + 0x13)
#define WIN_LIST_ADVANCED_FACTORYRESTORE_BOTTOM_ID                           (GUI_ID_USER + 0x14)

#define WIN_MAIN_RESTORE_FACTORY_ID                          (GUI_ID_USER + 0x20)
#define WIN_MAIN_NUMBER1_ID                                  (GUI_ID_USER + 0x21)
#define WIN_MAIN_NUMBER2_ID                                  (GUI_ID_USER + 0x22)
#define WIN_MAIN_NUMBER3_ID                                  (GUI_ID_USER + 0x23)
#define WIN_MAIN_NUMBER4_ID                                  (GUI_ID_USER + 0x24)
#define WIN_MAIN_NUMBER5_ID                                  (GUI_ID_USER + 0x25)
#define WIN_MAIN_NUMBER6_ID                                  (GUI_ID_USER + 0x26)


/*资源列表*/
static const GUI_WIDGET_CREATE_INFO _garrListFactoryRestoreDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListFactoryRestoreWin",    WIN_LIST_ADVANCED_FACTORYRESTORE_ID,          0,      0,      480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_ADVANCED_FACTORYRESTORE_TITLE_ID,              31,     20,     415,  14,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "bottom",         WIN_LIST_ADVANCED_FACTORYRESTORE_BOTTOM_ID,             37,     244,    404,  44,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "FACTORY",        WIN_MAIN_RESTORE_FACTORY_ID,                            31,     48,     160,  17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM1",           WIN_MAIN_NUMBER1_ID,                            356,    45,     9,    17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM2",           WIN_MAIN_NUMBER2_ID,                            372,    45,     9,    17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM3",           WIN_MAIN_NUMBER3_ID,                            388,    45,     9,    17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM4",           WIN_MAIN_NUMBER4_ID,                            404,    45,     9,    17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM5",           WIN_MAIN_NUMBER5_ID,                            420,    45,     9,    17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM6",           WIN_MAIN_NUMBER6_ID,                            436,    45,     9,    17,   0,   0x0,  0 },
};

/**
  * @brief  设置选中数字文本的颜色
  * @param  NULL
  * @retval NULL
  */
static void _SetADVResFactoryNumberColor(int32_t index)
{
    static volatile uint8_t arr_nums_sel_falg[DIGIT_NUMS] = {0};
    uint8_t i = 0;
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
static void _SetADVResFactoryNumAdd(int32_t index, int32_t v)
{
    _garrNumTextPara[index].digit_value = v;
    sprintf(_garrNumTextPara[index].p_content, "%1d", (int)_garrNumTextPara[index].digit_value);
    if(9 < _garrNumTextPara[index].digit_value)
    {
        _garrNumTextPara[index].digit_value = 0;
    }
    TEXT_SetText(_garrTextNumHandle[index], _garrFactoryRestoreNumTextBuf[index]);
}

/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
static void _DrawADVResFactoryNumUnderLine(int32_t index)
{
    GUI_SetColor(0x00CDF100);
    switch(index)
    {
        case 0:
        {
            GUI_DrawLine(_garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18, \
                         _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 1:
        {
            GUI_DrawLine(_garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18, \
                         _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 2:
        {
            GUI_DrawLine(_garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18, \
                         _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 3:
        {
            GUI_DrawLine(_garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18, \
                         _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 4:
        {
            GUI_DrawLine(_garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18, \
                         _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 5:
        {
            GUI_DrawLine(_garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18, \
                         _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListFactoryRestoreDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        default:
            break;
    }
}

/**
  * @brief ListFactoryRestoreWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListFactoryRestoreWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_FACTORYRESTORE_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            //
            _gTextRestoreFactoryHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_RESTORE_FACTORY_ID);
            TEXT_SetFont(_gTextRestoreFactoryHandle, _gstRestoreFactoryTextPara.p_text_font);
            TEXT_SetText(_gTextRestoreFactoryHandle, (const char *)_gstRestoreFactoryTextPara.p_content);
            TEXT_SetTextAlign(_gTextRestoreFactoryHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextRestoreFactoryHandle, _gstRestoreFactoryTextPara.text_color);

            for(i = 0; i < DIGIT_NUMS; i++)
            {
                _garrTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_NUMBER1_ID + i);
                TEXT_SetFont(_garrTextNumHandle[i], _garrNumTextPara[i].p_text_font);
                TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
                TEXT_SetTextAlign(_garrTextNumHandle[i], TEXT_CF_RIGHT | TEXT_CF_VCENTER);
                TEXT_SetTextColor(_garrTextNumHandle[i], _garrNumTextPara[i].text_color);
            }
            /*底部文字*/
            _gBottomHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_FACTORYRESTORE_BOTTOM_ID);
            TEXT_SetFont(_gBottomHandle, _gstBottomTextPara.p_text_font);
            TEXT_SetText(_gBottomHandle, (const char *)_gstBottomTextPara.p_content);
            TEXT_SetTextAlign(_gBottomHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBottomHandle, _gstBottomTextPara.text_color);

            _SetADVResFactoryNumberColor(_gstUiCtx.select);
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
            _SetADVResFactoryNumberColor(index);

            WM_Paint(pMsg->hWin);
            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListFactoryRestorePara, _gEmListFactoryRestorePara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            GUI_FillRoundedRect(20, 38,  20 + 440 - 1,  38 + 34 - 1, 10);
            GUI_SetColor(0x00BDC909);
            GUI_DrawRoundedFrame(20, 38,  20 + 440 - 1,  38 + 34 - 1, 10, 3);
            
            GUI_SetPenSize(3);
            GUI_SetColor(0x00D9D9D9);
            for(i = 0; i < DIGIT_NUMS; i++)
            {
                GUI_DrawLine(_garrListFactoryRestoreDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2, _garrListFactoryRestoreDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18, \
                             _garrListFactoryRestoreDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListFactoryRestoreDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18);
            }
            _DrawADVResFactoryNumUnderLine(_gstUiCtx.select);
//            SelectedDrawInWMPaint(&_gEmListFactoryRestorePara, _gEmListFactoryRestorePara.list_sel);
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
    gstEmWinListAdvanSettingFactoryRestore.win_handle = GUI_CreateDialogBox(_garrListFactoryRestoreDialogCreate, GUI_COUNTOF(_garrListFactoryRestoreDialogCreate), _cbListFactoryRestoreWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListAdvanSettingFactoryRestore.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvanSettingFactoryRestore.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListAdvanSettingFactoryRestore.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListFactoryRestorePara, gstEmWinListAdvanSettingFactoryRestore.win_handle);
    return gstEmWinListAdvanSettingFactoryRestore.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvanSettingFactoryRestore.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvanSettingFactoryRestore.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvanSettingFactoryRestore.win_handle = 0;
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t i = 0;
    EmKeyDefaultInfoSet();
    /*底部文字的参数*/
    _gstBottomTextPara.p_content = _garrBottomBuf;
    _gstBottomTextPara.text_color = 0x00EEEEEE;
    _gstBottomTextPara.p_text_font = EM_TEXT_FONT_1;


    /*password num*/
    _gstRestoreFactoryTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstRestoreFactoryTextPara.text_color = 0x00FFFFFF;
    _gstRestoreFactoryTextPara.p_content = _garrRestoreFactoryBuf;

    for(i = 0; i < DIGIT_NUMS; i++)
    {
        _garrNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
        _garrNumTextPara[i].text_color = 0x00FFFFFF;
        _garrNumTextPara[i].p_content = _garrFactoryRestoreNumTextBuf[i];
        _garrNumTextPara[i].digit_value = 0;
        sprintf(_garrNumTextPara[i].p_content, "%1d", (int)_garrNumTextPara[i].digit_value);
    }

    /*标题*/
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{

}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{

}

static int32_t CheckRestoreFacotyPwd()
{
    uint16_t arr_pwd[4] = {0};
    for (int32_t i = 0, j = 0; i < DIGIT_NUMS; i += 2)
    {
        arr_pwd[j++] = _gstUiCtx.num[i] << 8 | _gstUiCtx.num[i + 1];
    }

    if (gstCollectInfo.maincard_data.base_info.user_type == 0)
    {
        if (!memcmp(gstCollectInfo.maincard_data.base_info.normal_password, arr_pwd, sizeof(arr_pwd)))
        {
            return 1;
        }
    }
    else if (gstCollectInfo.maincard_data.base_info.user_type == 1)
    {
        if (!memcmp(gstCollectInfo.maincard_data.base_info.manager_password, arr_pwd, sizeof(arr_pwd)))
        {
            return 1;
        }
    }
    else if (gstCollectInfo.maincard_data.base_info.user_type == 2)
    {
        if (!memcmp(gstCollectInfo.maincard_data.base_info.installer_password, arr_pwd, sizeof(arr_pwd)))
        {
            return 1;
        }
    }
    else if (gstCollectInfo.maincard_data.base_info.user_type == 3)
    {
        if (!memcmp(gstCollectInfo.maincard_data.base_info.internal_password, arr_pwd, sizeof(arr_pwd)))
        {
            return 1;
        }
    }
    return 0;
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
        if(_gstUiCtx.select >= DIGIT_NUMS)
        {
            return;
        }
        _gstUiCtx.num[_gstUiCtx.select] = abs(10 + (--(_gstUiCtx.num[_gstUiCtx.select]))) % 10;
        _SetADVResFactoryNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        return;
    }
    else if (event == EN_KEY_SRC_LEFT)
    {
        if(_gstUiCtx.select >= DIGIT_NUMS)
        {
            return;
        }
        _gstUiCtx.num[_gstUiCtx.select] = (++(_gstUiCtx.num[_gstUiCtx.select])) % 10;
        _SetADVResFactoryNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        int32_t check_passwd = 0;
        if (++_gstUiCtx.select > DIGIT_NUMS - 1)
        {
            // check passwd is ok
            check_passwd = CheckRestoreFacotyPwd();
            if (check_passwd)
            {
                _gstUiCtx.select = DIGIT_NUMS - 1;
                // pop wind
                gstRestoreFacotryPopUpWin.create_win();
                pCurrentEmWin = &gstRestoreFacotryPopUpWin;
            }
            else
            {
                _gstUiCtx.select = 0;
            }
        }
        // _gstUiCtx.select = (++_gstUiCtx.select) % DIGIT_NUMS;
        _SetADVResFactoryNumberColor(_gstUiCtx.select);
        WM_Paint(gstEmWinListAdvanSettingFactoryRestore.win_handle);
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
                pnext = &gstEmWinListAdvancedSettings;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}

