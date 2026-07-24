/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-18
  * @brief       高级设置->电网标准确认
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-18    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"

#define     EM_BACK_ON_ICON_ARROW_SIZE                      (4096)
#define     EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE           (4096)
#define     EM_LIST_ICON_ARROW_SIZE                         (4096)

#define     REC_XPOS            (20)
#define     REC_YPOS            (38)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinListGridStandConfirm = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

/*GridStandard列表*/
static TEXT_Handle  _gTextGridStandardHandle;//显示GridStandard句柄
static TEXT_Handle  _gTextCountryHandle;//显示GridStandard句柄
static IMAGE_Handle  _gImgNationaHandle;//国旗图标句柄
static IMAGE_Handle  _gImgArrowHandle;//箭头图标句柄


static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;

static emWidgetTextPara_t  _gstGridStandardTextPara;
static emWidgetTextPara_t  _gstCountryTextPara;
static emWidgetImgPara_t  _gstFlagImgPara;
static emWidgetImgPara_t  _gstArrowImgPara;


static char _garrTitleBuf[] = "Setting < Grid Standard";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";
static char _garrPasswordBuf[] = "Password";
static char _garrGridStandardBuf[] = "Grid Standard";
static char _garrCountryBuf[] = "China";


#define WIN_LIST_GRID_STAND_CONFIRM_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_GRID_STAND_CONFIRM_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_GRID_STAND_CONFIRM_PRESS_ID                         (GUI_ID_USER + 0x14)
#define WIN_LIST_GRID_STAND_CONFIRM_BACK_ID                          (GUI_ID_USER + 0x15)
#define WIN_LIST_GRID_STAND_CONFIRM_BACK_IMG_ID                      (GUI_ID_USER + 0x16)

#define WIN_LIST_GRID_STAND_CONFIRM_TEXT_ID                          (GUI_ID_USER + 0x20)
#define WIN_LIST_GRID_STAND_CONGIRM_FLAG_IMG_ID                      (GUI_ID_USER + 0x21)
#define WIN_LIST_GRID_STAND_CONFIRM_COUNTRY_TEXT_ID                  (GUI_ID_USER + 0x22)
#define WIN_LIST_GRID_STAND_CONFIRM_ARROW_IMG_ID                     (GUI_ID_USER + 0x23)


/*资源列表*/
static __SDRAM GUI_WIDGET_CREATE_INFO _garrListGridStandConfirmDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListGridStandConfirmWin",    WIN_LIST_GRID_STAND_CONFIRM_ID,                     0,      0,      480,    320,    0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",                      WIN_LIST_GRID_STAND_CONFIRM_TITLE_ID,               31,     20,     415,    14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Press",                      WIN_LIST_GRID_STAND_CONFIRM_PRESS_ID,               133,    273,    33,     14,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "BackImg",                    WIN_LIST_GRID_STAND_CONFIRM_BACK_IMG_ID,            171 - 5,  275,    15,     10,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Back",                       WIN_LIST_GRID_STAND_CONFIRM_BACK_ID,                188,    273,    150,    14,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "GRID",                       WIN_LIST_GRID_STAND_CONFIRM_TEXT_ID,                31,     48,     152,    17,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "Imgfalg",                    WIN_LIST_GRID_STAND_CONGIRM_FLAG_IMG_ID,            338,    41,     30,     30,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "COUNTRY",                    WIN_LIST_GRID_STAND_CONFIRM_COUNTRY_TEXT_ID,        374,    48,     50,     17,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "ARROW",                      WIN_LIST_GRID_STAND_CONFIRM_ARROW_IMG_ID,           437,    50,     15,     15,     0,   0x0,  0 }
};



/**
  * @brief ListGridStandConfirmWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListGridStandConfirmWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;

    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*中间的列表*/
            _gTextGridStandardHandle =  WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_TEXT_ID);
            TEXT_SetFont(_gTextGridStandardHandle, _gstGridStandardTextPara.p_text_font);
            TEXT_SetText(_gTextGridStandardHandle, (const char *)_gstGridStandardTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridStandardHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridStandardHandle, _gstGridStandardTextPara.text_color);

            _gTextCountryHandle =   WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_COUNTRY_TEXT_ID);
            TEXT_SetFont(_gTextCountryHandle, _gstCountryTextPara.p_text_font);
            TEXT_SetText(_gTextCountryHandle, (const char *)_gstCountryTextPara.p_content);
            TEXT_SetTextAlign(_gTextCountryHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextCountryHandle, _gstCountryTextPara.text_color);

            _gImgNationaHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONGIRM_FLAG_IMG_ID);
            IMAGE_SetDTA(_gImgNationaHandle, _gstFlagImgPara.p_img_buf, _gstFlagImgPara.img_size);

            _gImgArrowHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_ARROW_IMG_ID);
            IMAGE_SetDTA(_gImgArrowHandle, _gstArrowImgPara.p_img_buf, _gstArrowImgPara.img_size);

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListGridStandConfirmPara, _gEmListGridStandConfirmPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            GUI_FillRoundedRect(REC_XPOS, REC_YPOS,  REC_XPOS + 440 - 1,  REC_YPOS + 34 - 1, 10);
            GUI_SetColor(0x00BDC909);
            GUI_DrawRoundedFrame(REC_XPOS, REC_YPOS,  REC_XPOS + 440 - 1,  REC_YPOS + 34 - 1, 10, 3);

//            SelectedDrawInWMPaint(&_gEmListGridStandConfirmPara, _gEmListGridStandConfirmPara.list_sel);
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
    gstEmWinListGridStandConfirm.win_handle = GUI_CreateDialogBox(_garrListGridStandConfirmDialogCreate, GUI_COUNTOF(_garrListGridStandConfirmDialogCreate), _cbListGridStandConfirmWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListGridStandConfirm.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListGridStandConfirm.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListGridStandConfirm.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListGridStandConfirmPara, gstEmWinListGridStandConfirm.win_handle);
    return gstEmWinListGridStandConfirm.win_handle;

}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListGridStandConfirm.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListGridStandConfirm.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListGridStandConfirm.win_handle = 0;
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
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

    /*gridstand*/
    _gstGridStandardTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstGridStandardTextPara.text_color = 0x00FFFFFF;
    _gstGridStandardTextPara.p_content = _garrGridStandardBuf;

    /*country*/
    _gstCountryTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstCountryTextPara.text_color = 0x00FFFFFF;
    _gstCountryTextPara.p_content = _garrCountryBuf;

    /*国旗*/
    _gstFlagImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE);
    _gstFlagImgPara.p_img_buf = GUI_ALLOC_h2p(_gstFlagImgPara.mem_img);
    _gstFlagImgPara.img_size = EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_CHINESE_FLAG_ADDR, _gstFlagImgPara.p_img_buf, _gstFlagImgPara.img_size);
    /*箭头*/
    _gstArrowImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_SIZE);
    _gstArrowImgPara.p_img_buf = GUI_ALLOC_h2p(_gstArrowImgPara.mem_img);
    _gstArrowImgPara.img_size = EM_LIST_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _gstArrowImgPara.p_img_buf, _gstArrowImgPara.img_size);

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
    GUI_ALLOC_Free(_gstArrowImgPara.mem_img);
    GUI_ALLOC_Free(_gstFlagImgPara.mem_img);
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

