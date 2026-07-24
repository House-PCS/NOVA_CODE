/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_main_3d_nav.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-12-18
  * @brief       启动界面
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-12-18    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"
#include "em_main.h"
#include <time.h>
#include "firmware_upgrate.h"

#define DEBUG_TEST                              0

#define EM_MAIN_3D_NAV_SELECT_SIZE              (4 * 1024)
#define EM_UPGRADE_SELECT_SIZE                  (4 * 1024)
#define EM_UPGRADE_DEV_STAT_SIZE                (4 * 1024 * 8)

/*6个升级选项ID*/
#define WIN_UPGRADE_ID                      (GUI_ID_USER + EM_UPGRADE_TYPE_NONE)

#define WIN_UPGRADE_MON_IMG_ID              (GUI_ID_USER + EM_UPGRADE_TYPE_MON)
#define WIN_UPGRADE_PCS_IMG_ID              (GUI_ID_USER + EM_UPGRADE_TYPE_PCS)
#define WIN_UPGRADE_BMS_IMG_ID              (GUI_ID_USER + EM_UPGRADE_TYPE_BMS)
#define WIN_UPGRADE_ARC_IMG_ID              (GUI_ID_USER + EM_UPGRADE_TYPE_ARC)
#define WIN_UPGRADE_MON_RES_IMG_IMG_ID      (GUI_ID_USER + EM_UPGRADE_TYPE_MON_RES_IMG)
#define WIN_UPGRADE_MON_RES_FONT_IMG_ID     (GUI_ID_USER + EM_UPGRADE_TYPE_MON_RES_FONT)
#define WIN_UPGRADE_DIRECT_ID               (GUI_ID_USER + EM_UPGRADE_TYPE_MON_RES_FONT + 0x10)
#define WIN_UPGRADE_WAIT_ID                 (GUI_ID_USER + EM_UPGRADE_TYPE_MON_RES_FONT + 0x11)
/*选择三角形*/
#define WIN_MAIN_3D_NAV_SELECT_ID                   (GUI_ID_USER + 0x21)
#define WIN_UPGRADE_T_MON_ID                        (GUI_ID_USER + 0x22)
#define WIN_UPGRADE_T_PCS_ID                        (GUI_ID_USER + 0x23)
#define WIN_UPGRADE_T_BMS_ID                        (GUI_ID_USER + 0x24)
#define WIN_UPGRADE_T_ARC_ID                        (GUI_ID_USER + 0x25)
#define WIN_UPGRADE_T_IMG_ID                        (GUI_ID_USER + 0x26)
#define WIN_UPGRADE_T_FONT_ID                       (GUI_ID_USER + 0x27)
#define WIN_UPGRADE_T_MSG_ID                        (GUI_ID_USER + 0x28)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    uint32_t time;
} UiCtx_t;

typedef enum
{
    EM_UPGRADE_TYPE_NONE = 0,
    EM_UPGRADE_TYPE_MON,            // 1. 监控卡
    EM_UPGRADE_TYPE_PCS,            // 2. PCS
    EM_UPGRADE_TYPE_BMS,            // 3. BMS
    EM_UPGRADE_TYPE_ARC,            // 4. 电弧检测
    EM_UPGRADE_TYPE_MON_RES_IMG,    // 5. 监控卡图库
    EM_UPGRADE_TYPE_MON_RES_FONT,   // 6. 监控卡字库
    EM_UPGRADE_TYPE_MAX,            // 7. 占位符
} EN_EM_UPGRADE_TYPE;

emWin_t gstEmWinUpgrade = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

static UiCtx_t _gstUiCtx;
static char _garrStatBarTimeTextBuf[80] = {0}; // 2024 Jan 1th 00:00:01

/*四个选项句柄和参数*/
static IMAGE_Handle _garrImgUpgradeHandle[EM_UPGRADE_TYPE_MAX]; // 升级选项句柄
static emWidgetImgPara_t  _garrUpgradeImgPara[EM_UPGRADE_TYPE_MAX];

static char _garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_MAX][32] = {0};
static char _garrMsgTextBuf[128] = {0};
static emWidgetTextPara_t _gstStatBarWorkMode1TextPara;

/*选择三角形句柄和参数*/
static IMAGE_Handle _gImgNavSelectHandle;
static emWidgetImgPara_t  _gstImgNavSelectPara;
static TEXT_Handle  _gTextStatBarWorkMode1Handle;// 模块显示句柄
static TEXT_Handle  _gTextMsgHandle;//提示句柄

static __SDRAM GUI_WIDGET_CREATE_INFO _garrUpgradeDialogCreate[] =
{
    { WINDOW_CreateIndirect,     "MainUpgradeWin",      WIN_UPGRADE_ID,                     0,      0,      480,  320,  0,   0x0,  0},
    { CHECKBOX_CreateIndirect,   "monitor",             WIN_UPGRADE_MON_IMG_ID,             30,     100,    30,   30,   0,   0x0,  0},
    { CHECKBOX_CreateIndirect,   "pcs",                 WIN_UPGRADE_PCS_IMG_ID,             110,    100,    30,   30,   0,   0x0,  0},
    { CHECKBOX_CreateIndirect,   "bms",                 WIN_UPGRADE_BMS_IMG_ID,             180,    100,    30,   30,   0,   0x0,  0},
    { CHECKBOX_CreateIndirect,   "arc",                 WIN_UPGRADE_ARC_IMG_ID,             250,    100,    30,   30,   0,   0x0,  0},
    { CHECKBOX_CreateIndirect,   "mon_img",             WIN_UPGRADE_MON_RES_IMG_IMG_ID,     320,    100,    30,   30,   0,   0x0,  0},
    { CHECKBOX_CreateIndirect,   "mon_font",            WIN_UPGRADE_MON_RES_FONT_IMG_ID,    400,    100,    30,   30,   0,   0x0,  0},
    { BUTTON_CreateIndirect,     "Direct",              WIN_UPGRADE_DIRECT_ID,              20,     20,     60,   20,   0,   0x0,  0},
    { BUTTON_CreateIndirect,     "Wait",                WIN_UPGRADE_WAIT_ID,                100,    20,     60,   20,   0,   0x0,  0},
    { IMAGE_CreateIndirect,      "Select",              WIN_MAIN_3D_NAV_SELECT_ID,          0,      150,    25,   25,   0,   0x0,  0},
    { TEXT_CreateIndirect,       "tmonitor",            WIN_UPGRADE_T_MON_ID,               20,     120,    50,   40,   0,   0x0,  0},
    { TEXT_CreateIndirect,       "tpcs",                WIN_UPGRADE_T_PCS_ID,               85,     120,    50,   40,   0,   0x0,  0},
    { TEXT_CreateIndirect,       "tbms",                WIN_UPGRADE_T_BMS_ID,               158,    120,    50,   40,   0,   0x0,  0},
    { TEXT_CreateIndirect,       "tarc",                WIN_UPGRADE_T_ARC_ID,               225,    120,    50,   40,   0,   0x0,  0},
    { TEXT_CreateIndirect,       "img",                 WIN_UPGRADE_T_IMG_ID,               296,    120,    50,   40,   0,   0x0,  0},
    { TEXT_CreateIndirect,       "tfont",               WIN_UPGRADE_T_FONT_ID,              377,    120,    50,   40,   0,   0x0,  0},
    { TEXT_CreateIndirect,       "msg",                 WIN_UPGRADE_T_MSG_ID,               10,     220,    360,  100,  0,   0x0,  0},
};

int32_t _SetUpgradeSelect(uint32_t idx)
{
    int32_t x_idx = 0;
    int32_t y_idx = 0;

    x_idx = -100;
    y_idx = _garrUpgradeDialogCreate[9].y0;
    switch (idx)
    {
        case 0:
        {
            x_idx =  _garrUpgradeDialogCreate[1].x0 + _garrUpgradeDialogCreate[1].xSize / 2 - 10;
            break;
        }
        case 1:
        {
            x_idx =  _garrUpgradeDialogCreate[2].x0 + _garrUpgradeDialogCreate[2].xSize / 2 - 10;
            break;
        }
        case 2:
        {
            x_idx =  _garrUpgradeDialogCreate[3].x0 + _garrUpgradeDialogCreate[3].xSize / 2 - 10;
            break;
        }
        case 3:
        {
            x_idx =  _garrUpgradeDialogCreate[4].x0 + _garrUpgradeDialogCreate[4].xSize / 2 - 10;
            break;
        }
        case 4:
        {
            x_idx =  _garrUpgradeDialogCreate[5].x0 + _garrUpgradeDialogCreate[5].xSize / 2 - 10;
            break;
        }
        case 5:
        {
            x_idx =  _garrUpgradeDialogCreate[6].x0 + _garrUpgradeDialogCreate[6].xSize / 2 - 10;
            break;
        }
        case 6:
        {
            x_idx =  _garrUpgradeDialogCreate[7].x0 + _garrUpgradeDialogCreate[7].xSize / 2 - 10;
            y_idx = _garrUpgradeDialogCreate[7].y0 + _garrUpgradeDialogCreate[7].ySize + 5;
            break;
        }
        case 7:
        {
            x_idx =  _garrUpgradeDialogCreate[8].x0 + _garrUpgradeDialogCreate[8].xSize / 2 - 10;
            y_idx = _garrUpgradeDialogCreate[8].y0 + _garrUpgradeDialogCreate[8].ySize + 5;
            break;
        }
        default:
            break;
    }

    WM_MoveTo(_gImgNavSelectHandle, x_idx, y_idx);
    return 0;
}

static int32_t SetMsg(const char *p_msg)
{
    snprintf(_garrMsgTextBuf, sizeof(_garrMsgTextBuf), "%s", p_msg);
    if (_gTextMsgHandle == NULL)
    {
        return -1;
    }
    TEXT_SetFont(_gTextMsgHandle, _gstStatBarWorkMode1TextPara.p_text_font);
    TEXT_SetTextAlign(_gTextMsgHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
    TEXT_SetTextColor(_gTextMsgHandle, _gstStatBarWorkMode1TextPara.text_color);
    TEXT_SetText(_gTextMsgHandle, (const char *)_garrMsgTextBuf);

    return 0;
}

/**
 * @brief main Win callback
 * @param  NULL
 * @retval NULL
 */
static void _cbUpgradeWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    static time_t cur_time = 1704067200;

    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*upgrade show*/
            _garrImgUpgradeHandle[EM_UPGRADE_TYPE_MON] = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_MON_IMG_ID);
            _garrImgUpgradeHandle[EM_UPGRADE_TYPE_PCS] = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_PCS_IMG_ID);
            _garrImgUpgradeHandle[EM_UPGRADE_TYPE_BMS] = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_BMS_IMG_ID);
            _garrImgUpgradeHandle[EM_UPGRADE_TYPE_ARC] = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_ARC_IMG_ID);
            _garrImgUpgradeHandle[EM_UPGRADE_TYPE_MON_RES_IMG] = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_MON_RES_IMG_IMG_ID);
            _garrImgUpgradeHandle[EM_UPGRADE_TYPE_MON_RES_FONT] = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_MON_RES_FONT_IMG_ID);

            _gTextMsgHandle = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_T_MSG_ID);

            _gTextStatBarWorkMode1Handle = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_T_MON_ID);
            TEXT_SetFont(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWorkMode1Handle, (const char *)_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_MON]);
            TEXT_SetTextAlign(_gTextStatBarWorkMode1Handle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.text_color);

            _gTextStatBarWorkMode1Handle = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_T_PCS_ID);
            TEXT_SetFont(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWorkMode1Handle, (const char *)_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_PCS]);
            TEXT_SetTextAlign(_gTextStatBarWorkMode1Handle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.text_color);

            _gTextStatBarWorkMode1Handle = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_T_BMS_ID);
            TEXT_SetFont(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWorkMode1Handle, (const char *)_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_BMS]);
            TEXT_SetTextAlign(_gTextStatBarWorkMode1Handle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.text_color);

            _gTextStatBarWorkMode1Handle = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_T_ARC_ID);
            TEXT_SetFont(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWorkMode1Handle, (const char *)_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_ARC]);
            TEXT_SetTextAlign(_gTextStatBarWorkMode1Handle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.text_color);

            _gTextStatBarWorkMode1Handle = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_T_IMG_ID);
            TEXT_SetFont(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWorkMode1Handle, (const char *)_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_MON_RES_IMG]);
            TEXT_SetTextAlign(_gTextStatBarWorkMode1Handle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.text_color);

            _gTextStatBarWorkMode1Handle = WM_GetDialogItem(pMsg->hWin, WIN_UPGRADE_T_FONT_ID);
            TEXT_SetFont(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWorkMode1Handle, (const char *)_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_MON_RES_FONT]);
            TEXT_SetTextAlign(_gTextStatBarWorkMode1Handle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWorkMode1Handle, _gstStatBarWorkMode1TextPara.text_color);

            /*Img Select*/
            _gImgNavSelectHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_SELECT_ID);
            IMAGE_SetDTA(_gImgNavSelectHandle, _gstImgNavSelectPara.p_img_buf, _gstImgNavSelectPara.img_size);

            _SetUpgradeSelect((uint32_t)_gstUiCtx.select);
            break;
        }
        case WM_PAINT:
        {
            // GUI_SetColor(0x434343);
            // GUI_FillRoundedRect(158, 15, 158 + 200 - 1, 15 + 28 - 1, 6);
            // GUI_SetColor(0x797979);
            // GUI_SetPenSize(2);
            // GUI_DrawLine(246, 22, 246, 22 + 13 - 1);
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
    GUI_EnableAlpha(0);
    /*创建一个基于桌面窗口的子窗口*/
    gstEmWinUpgrade.win_handle = GUI_CreateDialogBox(_garrUpgradeDialogCreate,
                                 GUI_COUNTOF(_garrUpgradeDialogCreate),
                                 _cbUpgradeWin,
                                 pCurrentEmWin->win_handle,
                                 0, 0);
    //WM_SetTransState(_gWinStatBarHandle, WM_CF_HASTRANS);

    pCurrentEmWin = &gstEmWinUpgrade;

    GUI_Exec();
    return gstEmWinUpgrade.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinUpgrade.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinUpgrade.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinUpgrade.win_handle = 0;
}



/**
  * @brief  界面控件等初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    _gstImgNavSelectPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_SELECT_SIZE);
    _gstImgNavSelectPara.p_img_buf = GUI_ALLOC_h2p(_gstImgNavSelectPara.mem_img);
    _gstImgNavSelectPara.img_size = EM_MAIN_3D_NAV_SELECT_SIZE;
    SpiFlashReadData(EM_ICON_3D_NAV_SELECT_ADDR, _gstImgNavSelectPara.p_img_buf, _gstImgNavSelectPara.img_size);

    _gstStatBarWorkMode1TextPara.p_content = _garrStatBarWorkMode1TextBuf[0];
    _gstStatBarWorkMode1TextPara.text_color = 0X0036B6FF;
    _gstStatBarWorkMode1TextPara.p_text_font = EM_TEXT_FONT_3;//EM_TEXT_FONT_5;

    snprintf(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_MON],
             sizeof(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_MON]), "%s", "monitor");
    snprintf(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_PCS],
             sizeof(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_PCS]), "%s", "pcs");
    snprintf(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_BMS],
             sizeof(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_BMS]), "%s", "bms");
    snprintf(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_ARC],
             sizeof(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_ARC]), "%s", "arc");
    snprintf(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_MON_RES_IMG],
             sizeof(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_MON_RES_IMG]), "%s", "img");
    snprintf(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_MON_RES_FONT],
             sizeof(_garrStatBarWorkMode1TextBuf[EM_UPGRADE_TYPE_MON_RES_FONT]), "%s", "font");

    EmKeyDefaultInfoSet();
}


/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gstImgNavSelectPara.mem_img);
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
//  int32_t event = GetKeyEvent(p_msg, &_gstUiCtx.key_init);
    int32_t event = 0;
    event = EmGetKeyEvent(p_msg);
    if (event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }
    if (event == EN_KEY_SRC_LEFT)
    {
        _gstUiCtx.select = (--_gstUiCtx.select) < 0 ? 0 : _gstUiCtx.select;
        _SetUpgradeSelect((uint32_t)_gstUiCtx.select);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        _gstUiCtx.select = (++_gstUiCtx.select) > 7 ? 7 : _gstUiCtx.select;
        _SetUpgradeSelect((uint32_t)_gstUiCtx.select);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        int32_t i = 0;
        int32_t j = 0;
        int32_t k = 0;
        int16_t r = 0;
        int16_t t = 0;
        int16_t n = 0;
        if (_gstUiCtx.select >= 0 && _gstUiCtx.select <= 5)
        {
            CHECKBOX_SetState(_garrImgUpgradeHandle[_gstUiCtx.select + 1],
                              (CHECKBOX_GetState(_garrImgUpgradeHandle[_gstUiCtx.select + 1]) + 1) % 2);
            return;
        }


        for (i = 0, j = 0, k = 0; i < EM_UPGRADE_TYPE_MON_RES_FONT; ++i)
        {
            j = CHECKBOX_GetState(_garrImgUpgradeHandle[i + 1]);
            k = j << i;
            r  |= k;
        }
        t = _gstUiCtx.select == 6 ? 1 : 0;

        char arr_buf[128] = {0};
#if DEBUG_TEST
        snprintf(buf, sizeof(buf), "update: %d, %s, is test no start", r,
                 t == 1 ? "direct" : "wait");
#else
        n = SetUpdataFirmwareType((uint16_t)r, (uint8_t)t);
        snprintf(arr_buf, sizeof(arr_buf), "update: %d, %s, result: %d", r,
                 t == 1 ? "direct" : "wait", n);
#endif /* DEBUG_TEST */
        SetMsg((const char *)arr_buf);
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
    {
        if(pCurrentEmWin->win_handle != 0)
        {
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
//            pCurrentEmWin = &gstEmWinMain;
            pCurrentEmWin->create_win();
        }
        return;
    }
}

