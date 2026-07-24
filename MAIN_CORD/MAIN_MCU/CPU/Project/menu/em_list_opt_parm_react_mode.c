/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opt_parm_ongrid_power.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-19
  * @brief       高级设置->设置->more->Opt PARM->React Mode（功率因子,反应模式）
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-19    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"

#define  TEST_DYNAMIC                                       0

#define     EM_BACK_ON_ICON_ARROW_SIZE                      (4096)
#define     DIGIT_NUMS                                      (11 - 2)
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

emWin_t gstEmWinListAdvanSettingOptReactMode = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    int8_t num[DIGIT_NUMS];
    uint16_t L_power_factor;
    uint16_t C_power_factor;
    uint16_t reactive_power;
} UiCtx_t;

static UiCtx_t _gstUiCtx;
/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

/*Power列表*/
static TEXT_Handle  _gTextPowerFactorHandle;//显示PowerFactor句柄
static TEXT_Handle  _gTextReactivePowerHandle;//ReactivePower句柄
static TEXT_Handle  _garrTextNumHandle[DIGIT_NUMS];
static TEXT_Handle  _gTextPowerOneHandle;
static TEXT_Handle  _gTextLetterLHandle;
static TEXT_Handle  _gTextLetterCHandle;
static TEXT_Handle  _gTextPointOneHandle;
static TEXT_Handle  _gTextPointTwoHandle;
static TEXT_Handle  _gTextLineHandle;

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t   _gstReturnImgPara;
static emWidgetTextPara_t  _gstPowerFactorTextPara;
static emWidgetTextPara_t  _gstReactivePowerTextPara;
static emWidgetTextPara_t  _gstLetterLTextPara;
static emWidgetTextPara_t  _gstLetterCTextPara;
static emWidgetTextPara_t  _gstPointOneTextPara;
static emWidgetTextPara_t  _gstPointTwoTextPara;
static emWidgetTextPara_t  _gstLineTextPara;
static emWidgetTextPara_t  _garrNumTextPara[DIGIT_NUMS];
static emWidgetTextPara_t  _gstPowerOneTextPara;

static char _garrTitleBuf[] = "Settings < More < Opt PARM < Reactive Power Mode";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";
static char _garrPowerFactorBuf[] = "Power Factor";
static char _garrReactivePowerBuf[] = "Reactive Power";
static char _garrLetterLBuf[] = "+";
static char _garrLetterCBuf[] = "C";
static char _garrPointBuf[] = ".";
static char _garrLineBuf[] = "-";
static char _garrAmpereOneBuf[] = "Var";
static char _garrReactModeNumTextBuf[DIGIT_NUMS][20];


#define WIN_LIST_ON_GRID_POWER_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_ON_GRID_POWER_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_ON_GRID_POWER_PRESS_ID                         (GUI_ID_USER + 0x14)
#define WIN_LIST_ON_GRID_POWER_BACK_ID                          (GUI_ID_USER + 0x15)
#define WIN_LIST_ON_GRID_POWER_BACK_IMG_ID                      (GUI_ID_USER + 0x16)

#define WIN_MAIN_POWER_FACTOR_ID                                (GUI_ID_USER + 0x20)
#define WIN_MAIN_REACTIVE_POWER_ID                              (GUI_ID_USER + 0x21)
#define WIN_MAIN_LETTER_L_ID                                    (GUI_ID_USER + 0x22)
#define WIN_MAIN_NUMBER1_ID                                     (GUI_ID_USER + 0x23)
#define WIN_MAIN_NUMBER2_ID                                     (GUI_ID_USER + 0x24)
#define WIN_MAIN_NUMBER3_ID                                     (GUI_ID_USER + 0x25)
#define WIN_MAIN_NUMBER4_ID                                     (GUI_ID_USER + 0x26)
#define WIN_MAIN_NUMBER5_ID                                     (GUI_ID_USER + 0x27)
#define WIN_MAIN_NUMBER6_ID                                     (GUI_ID_USER + 0x28)
#define WIN_MAIN_NUMBER7_ID                                     (GUI_ID_USER + 0x29)
#define WIN_MAIN_NUMBER8_ID                                     (GUI_ID_USER + 0x2A)
#define WIN_MAIN_NUMBER9_ID                                     (GUI_ID_USER + 0x2B)
#define WIN_MAIN_NUMBER10_ID                                    (GUI_ID_USER + 0x2C)
#define WIN_MAIN_LETTER_C_ID                                    (GUI_ID_USER + 0x2D)
#define WIN_MAIN_POINT_ONE_ID                                   (GUI_ID_USER + 0x2E)
#define WIN_MAIN_POINT_TWO_ID                                   (GUI_ID_USER + 0x2F)
#define WIN_MAIN_LINE_ID                                        (GUI_ID_USER + 0x30)
#define WIN_MAIN_POWER_ONE_ID                                   (GUI_ID_USER + 0x31)




/*资源列表*/
static __SDRAM GUI_WIDGET_CREATE_INFO _garrListReactModeDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListDODWin",     WIN_LIST_ON_GRID_POWER_ID,                  0,      0,      480,    320,    0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_ON_GRID_POWER_TITLE_ID,            31,     20,     415,    14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_ON_GRID_POWER_PRESS_ID,            133,    273,    33,     14,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_ON_GRID_POWER_BACK_IMG_ID,         171 - 5,  275,    15,     10,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_ON_GRID_POWER_BACK_ID,             188,    273,    150,    14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "factor",         WIN_MAIN_POWER_FACTOR_ID,                   31,     48,     180,    17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "reactive",       WIN_MAIN_REACTIVE_POWER_ID,                 31,     90,     180,    17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "+",              WIN_MAIN_LETTER_L_ID,                       390,    47,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM1",           WIN_MAIN_NUMBER1_ID,                        406,    47,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM2",           WIN_MAIN_NUMBER2_ID,                        422,    47,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM3",           WIN_MAIN_NUMBER3_ID,                        438,    47,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "+/-",            WIN_MAIN_NUMBER4_ID,                        358 - 20,    89,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM5",           WIN_MAIN_NUMBER5_ID,                        374 - 20,    89,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM6",           WIN_MAIN_NUMBER6_ID,                        390 - 20,    89,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM7",           WIN_MAIN_NUMBER7_ID,                        406 - 20,    89,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM8",           WIN_MAIN_NUMBER8_ID,                        422 - 20,    89,     9,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "power",          WIN_MAIN_POWER_ONE_ID,                      437 - 20,    89,     15+15,     17,     0,   0x0,  0 },
//    { TEXT_CreateIndirect,    "POINT1",         WIN_MAIN_POINT_ONE_ID,                      337,    49,     5,      17,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "POINT2",         WIN_MAIN_POINT_TWO_ID,                      416,    49,     5,      17,     0,   0x0,  0 },
};

static int32_t _SetReactModePowerFactor(int32_t ui_update, uint16_t L_power)
{
    uint16_t i = 0;
    int16_t power_factor = 0;
    int16_t symbol;
    uint16_t L_hundred;
    uint16_t L_tens;
    uint16_t L_uints;

    power_factor = (int16_t)L_power;

    if(power_factor >= 0)
    {
        _gstUiCtx.num[0] = 0x0;
        L_hundred  = (power_factor / 100) % 10;
        L_tens     = (power_factor / 10)  % 10;
        L_uints    = (power_factor % 10);
    }
    else
    {
        _gstUiCtx.num[0] = 0x1;
        L_hundred  = abs((power_factor / 100) % 10);
        L_tens     = abs((power_factor / 10)  % 10);
        L_uints    = abs((power_factor % 10));
    }
    sprintf(_garrReactModeNumTextBuf[0], "%s", _gstUiCtx.num[0] % 2 == 0 ? "+" : "-");
    snprintf(_garrReactModeNumTextBuf[1], sizeof(_garrReactModeNumTextBuf[1]), "%d", L_hundred);
    snprintf(_garrReactModeNumTextBuf[2], sizeof(_garrReactModeNumTextBuf[2]), "%d", L_tens);
    snprintf(_garrReactModeNumTextBuf[3], sizeof(_garrReactModeNumTextBuf[3]), "%d", L_uints);

    for(i = 1; i < 4; i++)
    {
        _gstUiCtx.num[i] = atoi((const char *)&_garrReactModeNumTextBuf[i][0]);
    }
    
    if(ui_update)
    {
        for(i = 0; i < 4; ++i)
        {
            TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
        }
    }
    return 0;
}


static int32_t _SetReactModeReactivePower(int32_t ui_update, uint16_t power)
{
    uint16_t i = 0;
    int16_t react_power = 0;
    uint16_t p_thousand = 0;
    uint16_t p_hundred = 0;
    uint16_t p_ten  = 0;
    uint16_t p_uint = 0;
    
    react_power = (int16_t)power;
    if(react_power >= 0)
    {
        _gstUiCtx.num[4] = 0x0;
        p_thousand =  react_power / 1000;
        p_hundred  = (react_power / 100) % 10;
        p_ten      = (react_power / 10)  % 10;
        p_uint     =  react_power % 10;
    }
    else
    {
        _gstUiCtx.num[4] = 0x1;
        p_thousand = abs(react_power / 1000);
        p_hundred  = abs((react_power / 100) % 10);
        p_ten      = abs((react_power / 10)  % 10);
        p_uint     = abs(react_power % 10);
    }
    sprintf(_garrReactModeNumTextBuf[4], "%s", _gstUiCtx.num[4] % 2 == 0 ? "+" : "-");
    snprintf(_garrReactModeNumTextBuf[5], sizeof(_garrReactModeNumTextBuf[5]),  "%d",   p_thousand);
    snprintf(_garrReactModeNumTextBuf[6], sizeof(_garrReactModeNumTextBuf[6]),  "%d",   p_hundred );
    snprintf(_garrReactModeNumTextBuf[7], sizeof(_garrReactModeNumTextBuf[7]),  "%d",   p_ten     );
    snprintf(_garrReactModeNumTextBuf[8], sizeof(_garrReactModeNumTextBuf[8]),  "%d",   p_uint    );

    for(i = 5; i < DIGIT_NUMS; i++)
    {
        _gstUiCtx.num[i] = atoi((const char *)&_garrReactModeNumTextBuf[i][0]);
    }
    
    if(ui_update)
    {
        for(i = 4; i < DIGIT_NUMS; ++i)
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
void _SetReactModeNumberColor(int32_t index)
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
static void _SetReactModeNumAdd(int32_t index, int32_t v)
{
    _garrNumTextPara[index].digit_value = v;
    if(9 < _garrNumTextPara[index].digit_value)
    {
        _garrNumTextPara[index].digit_value = 0;
    }

    if ((index == 0) || (index == 4))
    {
        sprintf(_garrReactModeNumTextBuf[index], "%s",
                (int)_garrNumTextPara[index].digit_value % 2 == 0 ? "+" : "-");
    }
    else
    {
        sprintf(_garrNumTextPara[index].p_content, "%1d", (int)_garrNumTextPara[index].digit_value);
    }
    TEXT_SetText(_garrTextNumHandle[index], _garrReactModeNumTextBuf[index]);
}

/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
void _DrawReactModeNumUnderLine(int32_t index)
{
    GUI_SetColor(0x00CDF100);
    switch(index)
    {
        case 0: // 正负号
        {
            GUI_DrawLine(_garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
            break;
        }
        case 1:
        {
            GUI_DrawLine(_garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 2:
        {
            GUI_DrawLine(_garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 3:
        {
            GUI_DrawLine(_garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 4:  //无功功率 正负号
        {
            GUI_DrawLine(_garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 5:
        {
            GUI_DrawLine(_garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 6:
        {
            GUI_DrawLine(_garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 7:
        {
            GUI_DrawLine(_garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
        }
        break;
        case 8:
        {
            GUI_DrawLine(_garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
                         _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
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
static void _DrawSelReactModeRecPaint(int32_t sel)
{
    if(sel == 0)
    {
        GUI_SetColor(0x00BDC909);
        GUI_DrawRoundedFrame(REC1_XPOS, REC1_YPOS,  REC1_XPOS + 440 - 1,  REC1_YPOS + 34 - 1, 10, 3);
    }
    else if(sel == 1)
    {
        GUI_SetColor(0x00BDC909);
        GUI_DrawRoundedFrame(REC2_XPOS, REC2_YPOS,  REC2_XPOS + 440 - 1,  REC2_YPOS + 34 - 1, 10, 3);
    }
    return;
}
/**
  * @brief ListReactModeWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListReactModeWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    static int32_t num = -1;
    static int32_t index = -1;
    static int32_t selected = -1;
    static int32_t tmp_sel;
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ON_GRID_POWER_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            //列表中的文字
            //Power Factor
            _gTextPowerFactorHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_POWER_FACTOR_ID);
            TEXT_SetFont(_gTextPowerFactorHandle, _gstPowerFactorTextPara.p_text_font);
            TEXT_SetText(_gTextPowerFactorHandle, (const char *)_gstPowerFactorTextPara.p_content);
            TEXT_SetTextAlign(_gTextPowerFactorHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPowerFactorHandle, _gstPowerFactorTextPara.text_color);
            //Reactive Power
            _gTextReactivePowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_REACTIVE_POWER_ID);
            TEXT_SetFont(_gTextReactivePowerHandle, _gstReactivePowerTextPara.p_text_font);
            TEXT_SetText(_gTextReactivePowerHandle, (const char *)_gstReactivePowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextReactivePowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextReactivePowerHandle, _gstReactivePowerTextPara.text_color);
            //字母L显示
            _gTextLetterLHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_LETTER_L_ID);
            TEXT_SetFont(_gTextLetterLHandle, _gstLetterLTextPara.p_text_font);
            TEXT_SetText(_gTextLetterLHandle, (const char *)_gstLetterLTextPara.p_content);
            TEXT_SetTextAlign(_gTextLetterLHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLetterLHandle, _gstLetterLTextPara.text_color);
//            //字母C显示
//            _gTextLetterCHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_LETTER_C_ID);
//            TEXT_SetFont(_gTextLetterCHandle, _gstLetterCTextPara.p_text_font);
//            TEXT_SetText(_gTextLetterCHandle, (const char *)_gstLetterCTextPara.p_content);
//            TEXT_SetTextAlign(_gTextLetterCHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
//            TEXT_SetTextColor(_gTextLetterCHandle, _gstLetterCTextPara.text_color);
//            //横线的显示
//            _gTextLineHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_LINE_ID);
//            TEXT_SetFont(_gTextLineHandle, _gstLineTextPara.p_text_font);
//            TEXT_SetText(_gTextLineHandle, (const char *)_gstLineTextPara.p_content);
//            TEXT_SetTextAlign(_gTextLineHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
//            TEXT_SetTextColor(_gTextLineHandle, _gstLineTextPara.text_color);
//            //第一个小数点的显示
//            _gTextPointOneHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_POINT_ONE_ID);
//            TEXT_SetFont(_gTextPointOneHandle, _gstPointOneTextPara.p_text_font);
//            TEXT_SetText(_gTextPointOneHandle, (const char *)_gstPointOneTextPara.p_content);
//            TEXT_SetTextAlign(_gTextPointOneHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
//            TEXT_SetTextColor(_gTextPointOneHandle, _gstPointOneTextPara.text_color);
            //第二个小数点的显示
            _gTextPointTwoHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_POINT_TWO_ID);
            TEXT_SetFont(_gTextPointTwoHandle, _gstPointTwoTextPara.p_text_font);
            TEXT_SetText(_gTextPointTwoHandle, (const char *)_gstPointTwoTextPara.p_content);
            TEXT_SetTextAlign(_gTextPointTwoHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPointTwoHandle, _gstPointTwoTextPara.text_color);


            //功率的显示
            //第二行功率单位显示
            _gTextPowerOneHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_POWER_ONE_ID);
            TEXT_SetFont(_gTextPowerOneHandle, _gstPowerOneTextPara.p_text_font);
            TEXT_SetText(_gTextPowerOneHandle, (const char *)_gstPowerOneTextPara.p_content);
            TEXT_SetTextAlign(_gTextPowerOneHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPowerOneHandle, _gstPowerOneTextPara.text_color);

            for(i = 0; i < DIGIT_NUMS; i++)
            {
                _garrTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_LETTER_L_ID + i);
                TEXT_SetFont(_garrTextNumHandle[i], _garrNumTextPara[i].p_text_font);
                TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
                if((i == 0) || (i == 4))
                {
                    TEXT_SetTextAlign(_garrTextNumHandle[i], TEXT_CF_LEFT | TEXT_CF_VCENTER);
                }
                else
                {
                    TEXT_SetTextAlign(_garrTextNumHandle[i], TEXT_CF_RIGHT | TEXT_CF_VCENTER);
                }
                TEXT_SetTextColor(_garrTextNumHandle[i], _garrNumTextPara[i].text_color);
            }
            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ON_GRID_POWER_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ON_GRID_POWER_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ON_GRID_POWER_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            _SetReactModeNumberColor(_gstUiCtx.select);
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
            _SetReactModeNumberColor(index);

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
            //绘制数字底部横线
            for(i = 0; i < DIGIT_NUMS; i++)
            {
                GUI_DrawLine(_garrListReactModeDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2, _garrListReactModeDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18,
                             _garrListReactModeDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2 + 12, _garrListReactModeDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18);
            }
#if TEST_DYNAMIC
            _DrawSelReactModeRecPaint(selected % 2);
            _DrawReactModeNumUnderLine(index);
#else
            tmp_sel = _gstUiCtx.select;
            _DrawSelReactModeRecPaint((tmp_sel < 4 ? 0 : 1));
            _DrawReactModeNumUnderLine(_gstUiCtx.select);
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
    pPrevEmWin = &gstEmWinListOptPARMInfo;

    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListAdvanSettingOptReactMode.win_handle = GUI_CreateDialogBox(_garrListReactModeDialogCreate, GUI_COUNTOF(_garrListReactModeDialogCreate), _cbListReactModeWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListAdvanSettingOptReactMode.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvanSettingOptReactMode.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListAdvanSettingOptReactMode.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListDODPara, gstEmWinListAdvanSettingOptReactMode.win_handle);
    return gstEmWinListAdvanSettingOptReactMode.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvanSettingOptReactMode.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvanSettingOptReactMode.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvanSettingOptReactMode.win_handle = 0;
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t i = 0;

    _gstUiCtx.L_power_factor = gstCollectInfo.power_data.power_set_info.power_factor_set;
    _gstUiCtx.reactive_power = gstCollectInfo.power_data.power_set_info.reactive_set;
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
    //power factor
    _gstPowerFactorTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstPowerFactorTextPara.text_color = 0x00FFFFFF;
    _gstPowerFactorTextPara.p_content = _garrPowerFactorBuf;
    //reactive power
    _gstReactivePowerTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstReactivePowerTextPara.text_color = 0x00FFFFFF;
    _gstReactivePowerTextPara.p_content = _garrReactivePowerBuf;

    //字母L
    _gstLetterLTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstLetterLTextPara.text_color = 0x00FFFFFF;
    _gstLetterLTextPara.p_content = _garrLetterLBuf;
//    //字母C
//    _gstLetterCTextPara.p_text_font = EM_TEXT_FONT_5;
//    _gstLetterCTextPara.text_color = 0x00FFFFFF;
//    _gstLetterCTextPara.p_content = _garrLetterCBuf;
//    //小数点1
//    _gstPointOneTextPara.p_text_font = EM_TEXT_FONT_5;
//    _gstPointOneTextPara.text_color = 0x00FFFFFF;
//    _gstPointOneTextPara.p_content = _garrPointBuf;
    //小数点2
    _gstPointTwoTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstPointTwoTextPara.text_color = 0x00FFFFFF;
    _gstPointTwoTextPara.p_content = _garrPointBuf;
//    //横线
//    _gstLineTextPara.p_text_font = EM_TEXT_FONT_5;
//    _gstLineTextPara.text_color = 0x00FFFFFF;
//    _gstLineTextPara.p_content = _garrLineBuf;

    //power1
    _gstPowerOneTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstPowerOneTextPara.text_color = 0x00FFFFFF;
    _gstPowerOneTextPara.p_content = _garrAmpereOneBuf;

    for(i = 0; i < DIGIT_NUMS; i++)
    {
        _garrNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
        _garrNumTextPara[i].text_color = 0x00FFFFFF;
        _garrNumTextPara[i].p_content = _garrReactModeNumTextBuf[i];
//      _garrNumTextPara[i].digit_value = 0;
//      sprintf(_garrNumTextPara[i].p_content, "%1d", (int)_garrNumTextPara[i].digit_value);
    }
    _SetReactModePowerFactor(0, _gstUiCtx.L_power_factor);
    _SetReactModeReactivePower(0, _gstUiCtx.reactive_power);

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
#if 1
    if(_gstUiCtx.L_power_factor != gstCollectInfo.power_data.power_set_info.power_factor_set)
    {
        _gstUiCtx.L_power_factor = gstCollectInfo.power_data.power_set_info.power_factor_set;
        _SetReactModePowerFactor(1, _gstUiCtx.L_power_factor);
    }

    if(_gstUiCtx.reactive_power != gstCollectInfo.power_data.power_set_info.reactive_set)
    {
        _gstUiCtx.reactive_power = gstCollectInfo.power_data.power_set_info.reactive_set;
        _SetReactModeReactivePower(1, _gstUiCtx.reactive_power);
    }
#endif
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
        _SetReactModeNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        return;
    }
    else if(event == EN_KEY_SRC_LEFT)
    {
        if(_gstUiCtx.select >= DIGIT_NUMS)
        {
            return;
        }
        _gstUiCtx.num[_gstUiCtx.select] = (++(_gstUiCtx.num[_gstUiCtx.select])) % 10;
        _SetReactModeNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        return;
    }
    else if(event == EN_KEY_SRC_CONFIRM)
    {
        _gstUiCtx.select = (++_gstUiCtx.select) % DIGIT_NUMS;
        _SetReactModeNumberColor(_gstUiCtx.select);

        WM_Paint(gstEmWinListAdvanSettingOptReactMode.win_handle);

        // 发送值给PCS
    }
    else if(event == EN_KEY_SRC_ESC)
    {
#if 1
        uint8_t data_buff[16] = {0};
        int16_t L_value = 0;
        int16_t L_reac_power = 0;
        uint16_t reactive_power = 0;
        uint16_t power_factor = 0;

        char L_value_str[5] = {0};
        char power_str[6] = {0};

        snprintf(L_value_str, sizeof(L_value_str), "%c%c%c%c", _garrReactModeNumTextBuf[0][0], _garrReactModeNumTextBuf[1][0], 
                 _garrReactModeNumTextBuf[2][0], _garrReactModeNumTextBuf[3][0]);
        snprintf(power_str,  sizeof(power_str),  "%c%c%c%c%c", _garrReactModeNumTextBuf[4][0], _garrReactModeNumTextBuf[5][0], 
                 _garrReactModeNumTextBuf[6][0], _garrReactModeNumTextBuf[7][0],_garrReactModeNumTextBuf[8][0]);


        L_value = atoi((const char *)L_value_str);
        L_reac_power = atoi((const char *)power_str);
        if(L_value > 0)
        {
            if(L_value > 100)
            {
                L_value = 100;
            }
            else if(L_value < 80)
            {
                L_value = 80;
            } 
        }
        else if(L_value < 0)
        {
            if(L_value > -80)
            {
                L_value = -80;
            }
            else if(L_value < -100)
            {
                L_value = -100;
            }
        }
        else if(L_value == 0)
        {
            L_value = 100;
        }
        
        if(L_reac_power > 3000)
        {
            L_reac_power = 3000;
        }
        else if(L_reac_power < -3000)
        {
            L_reac_power = -3000;
        }
        power_factor = (uint16_t)L_value;
        reactive_power = (uint16_t)L_reac_power;

        power_factor = BigLittleSwap16(power_factor);
        reactive_power = BigLittleSwap16(reactive_power);

        ModbusWriteSingleRemain(data_buff, 0x01, 0x634F, power_factor);
        ModbusWriteSingleRemain(data_buff, 0x01, 0x634C, reactive_power);
#endif
        if(pCurrentEmWin->win_handle != 0)
        {
            emWin_t *pnext = NULL;
            if(p_msg->press_down_time >= gstEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
            {
                pnext = &gstEmWinHomePage;
            }
            else
            {
                pnext = &gstEmWinListOptPARMInfo;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
    }
}


