/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_enter_password.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-20
  * @brief       高级设置输入密码
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-20    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"

#define     EM_BACK_ON_ICON_ARROW_SIZE                      (4096)
#define     DIGIT_NUMS                                      (6)
#define     REC_XPOS                                        (20)
#define     REC_YPOS                                        (38)


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinListEnterPassword = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

/*Password列表*/
static TEXT_Handle  _gTextPasswordHandle;//显示Password句柄
static TEXT_Handle  _garrTextNumHandle[DIGIT_NUMS];

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t   _gstReturnImgPara;
static emWidgetTextPara_t  _gstPasswordTextPara;
static emWidgetTextPara_t _garrNumTextPara[DIGIT_NUMS];

static char _garrTitleBuf[] = "Factory Reset";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";
static char _garrPasswordBuf[] = "Password";
static char _garrEnterPasswordNumTextBuf[DIGIT_NUMS][20];

#define WIN_LIST_ENTER_PASSWORD_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_ENTER_PASSWORD_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_ENTER_PASSWORD_PRESS_ID                         (GUI_ID_USER + 0x14)
#define WIN_LIST_ENTER_PASSWORD_BACK_ID                          (GUI_ID_USER + 0x15)
#define WIN_LIST_ENTER_PASSWORD_BACK_IMG_ID                      (GUI_ID_USER + 0x16)

#define WIN_MAIN_PASSWORD_ID                                 (GUI_ID_USER + 0x20)
#define WIN_MAIN_NUMBER1_ID                                  (GUI_ID_USER + 0x21)
#define WIN_MAIN_NUMBER2_ID                                  (GUI_ID_USER + 0x22)
#define WIN_MAIN_NUMBER3_ID                                  (GUI_ID_USER + 0x23)
#define WIN_MAIN_NUMBER4_ID                                  (GUI_ID_USER + 0x24)
#define WIN_MAIN_NUMBER5_ID                                  (GUI_ID_USER + 0x25)
#define WIN_MAIN_NUMBER6_ID                                  (GUI_ID_USER + 0x26)


/*资源列表*/
static const GUI_WIDGET_CREATE_INFO _garrListEnterPasswordDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListEnterPasswordWin",    WIN_LIST_ENTER_PASSWORD_ID,            0,      0,      480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_ENTER_PASSWORD_TITLE_ID,               31,     20,     415,  14,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_ENTER_PASSWORD_PRESS_ID,               133,    273,    33,   14,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_ENTER_PASSWORD_BACK_IMG_ID,            171 - 5,  275,    15,   10,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_ENTER_PASSWORD_BACK_ID,                188,    273,    150,  14,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Password",       WIN_MAIN_PASSWORD_ID,                           31,     48,     114,  17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM1",           WIN_MAIN_NUMBER1_ID,                            356,    45,     9,    17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM2",           WIN_MAIN_NUMBER2_ID,                            372,    45,     9,    17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM3",           WIN_MAIN_NUMBER3_ID,                            388,    45,     9,    17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM4",           WIN_MAIN_NUMBER4_ID,                            404,    45,     9,    17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM5",           WIN_MAIN_NUMBER5_ID,                            420,    45,     9,    17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM6",           WIN_MAIN_NUMBER6_ID,                            436,    45,     9,    17,   0,   0x0,  0 },
};

/**
  * @brief  设置选中数字变化
  * @param  NULL
  * @retval NULL
  */
static void _SetEnterPasswordNumAdd(int32_t index)
{
    _garrNumTextPara[index].digit_value++;
    sprintf(_garrNumTextPara[index].p_content, "%1d", (int)_garrNumTextPara[index].digit_value);
    if(9 < _garrNumTextPara[index].digit_value)
    {
        _garrNumTextPara[index].digit_value = 0;
    }
    TEXT_SetText(_garrTextNumHandle[index], _garrEnterPasswordNumTextBuf[index]);
}

/**
  * @brief  设置选中数字文本的颜色
  * @param  NULL
  * @retval NULL
  */
static void _SetEnterPasswordNumberColor(int32_t index)
{
    static volatile uint8_t arr_nums_sel_falg[DIGIT_NUMS] = {0};
    uint8_t i = 0;

    _SetEnterPasswordNumAdd(index);
    for(i = 0; i < DIGIT_NUMS; i++)
    {
        arr_nums_sel_falg[i] = 0;
        TEXT_SetTextColor(_garrTextNumHandle[i], 0x00FFFFFF);
    }
    arr_nums_sel_falg[index] = 1;
    TEXT_SetTextColor(_garrTextNumHandle[index], 0x00CDF100);
}

/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
static void _DrawEnterPasswordNumUnderLine(int32_t index)
{
    GUI_SetColor(0x00CDF100);
    switch(index)
    {
        case 0:
        {
            GUI_DrawLine(_garrListEnterPasswordDialogCreate[index + 6].x0 - 2, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18, \
                         _garrListEnterPasswordDialogCreate[index + 6].x0 - 2 + 12, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18);
        }
        break;
        case 1:
        {
            GUI_DrawLine(_garrListEnterPasswordDialogCreate[index + 6].x0 - 2, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18, \
                         _garrListEnterPasswordDialogCreate[index + 6].x0 - 2 + 12, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18);
        }
        break;
        case 2:
        {
            GUI_DrawLine(_garrListEnterPasswordDialogCreate[index + 6].x0 - 2, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18, \
                         _garrListEnterPasswordDialogCreate[index + 6].x0 - 2 + 12, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18);
        }
        break;
        case 3:
        {
            GUI_DrawLine(_garrListEnterPasswordDialogCreate[index + 6].x0 - 2, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18, \
                         _garrListEnterPasswordDialogCreate[index + 6].x0 - 2 + 12, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18);
        }
        break;
        case 4:
        {
            GUI_DrawLine(_garrListEnterPasswordDialogCreate[index + 6].x0 - 2, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18, \
                         _garrListEnterPasswordDialogCreate[index + 6].x0 - 2 + 12, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18);
        }
        break;
        case 5:
        {
            GUI_DrawLine(_garrListEnterPasswordDialogCreate[index + 6].x0 - 2, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18, \
                         _garrListEnterPasswordDialogCreate[index + 6].x0 - 2 + 12, _garrListEnterPasswordDialogCreate[index + 6].y0 + 18);
        }
        break;
        default:
            break;
    }
}

/**
  * @brief ListEnterPasswordWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListEnterPasswordWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    static int32_t num = -1;
    static int32_t index = -1;
    static WM_HTIMER wm_timer_handel_nums;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            wm_timer_handel_nums = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                                  0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                                  1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                                  0);         /* 留待将来使用，应为0 */

            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ENTER_PASSWORD_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            //
            _gTextPasswordHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_PASSWORD_ID);
            TEXT_SetFont(_gTextPasswordHandle, _gstPasswordTextPara.p_text_font);
            TEXT_SetText(_gTextPasswordHandle, (const char *)_gstPasswordTextPara.p_content);
            TEXT_SetTextAlign(_gTextPasswordHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPasswordHandle, _gstPasswordTextPara.text_color);


            for(i = 0; i < DIGIT_NUMS; i++)
            {
                _garrTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_NUMBER1_ID + i);
                TEXT_SetFont(_garrTextNumHandle[i], _garrNumTextPara[i].p_text_font);
                TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
                TEXT_SetTextAlign(_garrTextNumHandle[i], TEXT_CF_RIGHT | TEXT_CF_VCENTER);
                TEXT_SetTextColor(_garrTextNumHandle[i], _garrNumTextPara[i].text_color);
            }

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ENTER_PASSWORD_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ENTER_PASSWORD_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ENTER_PASSWORD_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            break;
        }
        case WM_DELETE:
        {
            WM_DeleteTimer(wm_timer_handel_nums); //删除窗口，会自动删除对应内部的定时器
            break;
        }
        case WM_TIMER:
        {
            ++num;
            index = num % DIGIT_NUMS;
            _SetEnterPasswordNumberColor(index);

            WM_Paint(pMsg->hWin);
            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
            break;
        }

        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListFactoryRestorePara, _gEmListFactoryRestorePara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            GUI_FillRoundedRect(REC_XPOS, REC_YPOS,  REC_XPOS + 440 - 1,  REC_YPOS + 34 - 1, 10);

            GUI_SetPenSize(3);
            GUI_SetColor(0x00D9D9D9);
            for(i = 0; i < DIGIT_NUMS; i++)
            {
                GUI_DrawLine(_garrListEnterPasswordDialogCreate[i + 6].x0 - 2, _garrListEnterPasswordDialogCreate[i + 6].y0 + 18, \
                             _garrListEnterPasswordDialogCreate[i + 6].x0 - 2 + 12, _garrListEnterPasswordDialogCreate[i + 6].y0 + 18);
            }
            _DrawEnterPasswordNumUnderLine(index);

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
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListEnterPassword.win_handle = GUI_CreateDialogBox(_garrListEnterPasswordDialogCreate, GUI_COUNTOF(_garrListEnterPasswordDialogCreate), _cbListEnterPasswordWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListEnterPassword.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListEnterPassword.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListEnterPassword.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListFactoryRestorePara, gstEmWinListEnterPassword.win_handle);
    return gstEmWinListEnterPassword.win_handle;

}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListEnterPassword.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListEnterPassword.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListEnterPassword.win_handle = 0;
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

    /*password num*/
    _gstPasswordTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstPasswordTextPara.text_color = 0x00FFFFFF;
    _gstPasswordTextPara.p_content = _garrPasswordBuf;

    for(i = 0; i < DIGIT_NUMS; i++)
    {
        _garrNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
        _garrNumTextPara[i].text_color = 0x00FFFFFF;
        _garrNumTextPara[i].p_content = _garrEnterPasswordNumTextBuf[i];
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
    GUI_ALLOC_Free(_gstReturnImgPara.mem_img);
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

