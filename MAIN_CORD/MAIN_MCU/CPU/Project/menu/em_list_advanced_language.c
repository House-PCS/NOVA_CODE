/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_language.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-02-20
  * @brief       高级设置->更多设置->语种设置
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-02-20    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"


#define  EM_LIST_LANGUAGE_ICON_DOWN_ARROW_SIZE              (4096)
#define  EM_LIST_LANGUAGE_ICON_BACK_SIZE                    (4096)
#define  EM_LIST_LANGUAGE_ICON_RIGHT_ARROW_SIZE             (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListLanguage = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_stru_t
{
    int32_t language_list_show;
    int32_t select;
    int32_t key_init;
    uint16_t language_type;
} UiCtx_t;

typedef struct _language_idx_t
{
    uint16_t index;
    const char name[16];
} LanguageIdx_t;

static UiCtx_t _gstUiCtx;
static LanguageIdx_t _gLanguageIdx[] =
{
    {0,  "English"  },
    {1,  "Chinese"  },
    {2,  "Australia"},
    {3,  "Italian"  },
    {4,  "Spanish"  },
    {5,  "France"   },
    {6,  "German"   },
    {7,  "Greek"    },
    {8,  "Hungary"  },
    {9,  "Belgium"  },
    {10, "Thailand" },
    {11, "England"  },
};

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

static char _garrTitleBuf[] = "Settings < More < Language";
static char _garrLanguageBuf[] = "Language";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";
static char _garrLanguagesTypeBuf[64] = {0};

static TEXT_Handle  _gTextLanguageHandle;//显示Language句柄
static TEXT_Handle  _gTextLanguageTypeHandle;//显示国家语言句柄
static IMAGE_Handle  _gImgArrowHandle;//箭头图标句柄

/*List控件*/
static emWidgetTextPara_t  _gstLanguageTextPara;
static emWidgetTextPara_t  _gstLanguageTypeTextPara;
static emWidgetImgPara_t  _garrArrowImgPara[2];
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;

#define WIN_LIST_LANGUAGE_ID                                 (GUI_ID_USER + 0x10)
#define WIN_LIST_LANGUAGE_TITLE_ID                           (GUI_ID_USER + 0x11)
#define WIN_LIST_LANGUAGE_TEXT_ID                            (GUI_ID_USER + 0x12)
#define WIN_LIST_LANGUAGE_PRESS_ID                           (GUI_ID_USER + 0x13)
#define WIN_LIST_LANGUAGE_BACK_IMG_ID                        (GUI_ID_USER + 0x14)
#define WIN_LIST_LANGUAGE_BACK_ID                            (GUI_ID_USER + 0x15)

#define WIN_LIST_LANGUAGE_IMG_ID                             (GUI_ID_USER + 0x16)
#define WIN_LIST_LANGUAGE_COUNTRY_TEXT_ID                    (GUI_ID_USER + 0x17)
#define WIN_LIST_LANGUAGE_ARROW_IMG_ID                       (GUI_ID_USER + 0x18)

/*函数声明*/
int32_t SetCurLanguageType(int32_t idx);
void SetArrowDirection(void);

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListLanguageDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListLanguageWin",    WIN_LIST_LANGUAGE_ID,                   0,      0,      480,    320,    0,   0x0,   0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_LANGUAGE_TITLE_ID,                 31,     20,     415,    14,     0,   0x0,   0 },
    { TEXT_CreateIndirect,    "LANGUAGE",       WIN_LIST_LANGUAGE_TEXT_ID,                  31,     48,     152,    17,     0,   0x0,   0 },
    { TEXT_CreateIndirect,    "COUNTRY",        WIN_LIST_LANGUAGE_COUNTRY_TEXT_ID,          343,    48,     100,    17,     0,   0x0,   0 },
    { IMAGE_CreateIndirect,   "ARROW",          WIN_LIST_LANGUAGE_ARROW_IMG_ID,             437,    50,     15,     15,     0,   0x0,   0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_LANGUAGE_PRESS_ID,                 133,    273,    33,     14,     0,   0x0,   0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_LANGUAGE_BACK_IMG_ID,              171 - 5,  275,    15,     10,     0,   0x0,   0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_LANGUAGE_BACK_ID,                  188,    273,    150,    14,     0,   0x0,   0 },
};

static int32_t _SetLanguageType(int32_t ui_update, uint16_t languageidx)
{
    snprintf(_garrLanguagesTypeBuf, sizeof(_garrLanguagesTypeBuf), "%s", _gLanguageIdx[languageidx].name);
    if(ui_update)
    {
        TEXT_SetText(_gTextLanguageTypeHandle, (const char *)_gstLanguageTypeTextPara.p_content);
    }
    return 0;
}



/**
  * @brief ListLanguageWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListLanguageWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            _gTextLanguageHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_TEXT_ID);
            TEXT_SetFont(_gTextLanguageHandle, _gstLanguageTextPara.p_text_font);
            TEXT_SetText(_gTextLanguageHandle, (const char *)_gstLanguageTextPara.p_content);
            TEXT_SetTextAlign(_gTextLanguageHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLanguageHandle, _gstLanguageTextPara.text_color);

            _gTextLanguageTypeHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_COUNTRY_TEXT_ID);
            TEXT_SetFont(_gTextLanguageTypeHandle, _gstLanguageTypeTextPara.p_text_font);
            TEXT_SetText(_gTextLanguageTypeHandle, (const char *)_gstLanguageTypeTextPara.p_content);
            TEXT_SetTextAlign(_gTextLanguageTypeHandle, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLanguageTypeHandle, _gstLanguageTypeTextPara.text_color);

            _gImgArrowHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_ARROW_IMG_ID);
            IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListLanguagePara, _gEmListLanguagePara.list_pre_sel);

            GUI_SetColor(0x00424242);//上色
            GUI_FillRoundedRect(20, 38,  20 + 440 - 1,  38 + 34 - 1, 10);
            GUI_SetColor(0x00BDC909);
            GUI_DrawRoundedFrame(20, 38,  20 + 440 - 1,  38 + 34 - 1, 10, 3);
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
    _gstUiCtx.language_list_show = 0;
    _gstUiCtx.key_init = 0;
    pPrevEmWin = &gstEmWinListAdvancedSettings;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListLanguage.win_handle = GUI_CreateDialogBox(_garrListLanguageDialogCreate, GUI_COUNTOF(_garrListLanguageDialogCreate), _cbListLanguageWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListLanguage.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListLanguage.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListLanguage.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    //SelectedDraw(&_gEmListLanguagePara, gstEmWinListLanguage.win_handle);
    //gEmWinListLanguageFlag.create_win();
    return gstEmWinListLanguage.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListLanguage.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListLanguage.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListLanguage.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    EmKeyDefaultInfoSet();
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstLanguageTextPara.p_content = _garrLanguageBuf;
    _gstLanguageTextPara.text_color = 0x00FFFFFF;
    _gstLanguageTextPara.p_text_font = EM_TEXT_FONT_5;

    /*语言类型*/
    _gstLanguageTypeTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstLanguageTypeTextPara.text_color = 0x00FFFFFF;
    _gstLanguageTypeTextPara.p_content = _garrLanguagesTypeBuf;

    /*右箭头*/
    _garrArrowImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_LANGUAGE_ICON_RIGHT_ARROW_SIZE);
    _garrArrowImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrArrowImgPara[0].mem_img);
    _garrArrowImgPara[0].img_size = EM_LIST_LANGUAGE_ICON_RIGHT_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);
    /*下箭头*/
    _garrArrowImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_LANGUAGE_ICON_DOWN_ARROW_SIZE);
    _garrArrowImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrArrowImgPara[1].mem_img);
    _garrArrowImgPara[1].img_size = EM_LIST_LANGUAGE_ICON_DOWN_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_DOWN_ARROW_ADDR, _garrArrowImgPara[1].p_img_buf, _garrArrowImgPara[1].img_size);

    _gstPressTextPara.p_content = _garrPressBuf;
    _gstPressTextPara.text_color = 0x00EEEEEE;
    _gstPressTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstBackTextPara.p_content = _garrBackBuf;
    _gstBackTextPara.text_color = 0x00EEEEEE;
    _gstBackTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_LANGUAGE_ICON_BACK_SIZE);
    _gstReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gstReturnImgPara.mem_img);
    _gstReturnImgPara.img_size = EM_LIST_LANGUAGE_ICON_BACK_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

    _SetLanguageType(0, gstCollectInfo.maincard_data.device_info.lcd_languege);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_garrArrowImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrArrowImgPara[1].mem_img);
    GUI_ALLOC_Free(_gstReturnImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    if(_gstUiCtx.language_type != gstCollectInfo.maincard_data.device_info.lcd_languege)
    {
        _gstUiCtx.language_type = gstCollectInfo.maincard_data.device_info.lcd_languege;
        _SetLanguageType(1, _gstUiCtx.language_type);
    }
//  if (_gstUiCtx.language_list_show)
//  {
//      gstEmWinListLanguageType.polling();
//  }
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
    if (_gstUiCtx.language_list_show)
    {
        gstEmWinListLanguageType.key_handler(p_msg);

        if (!_gstUiCtx.language_list_show)
        {
            gstEmWinListLanguageType.delete_win();
        }
        return;
    }
    if (event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }
    if (event == EN_KEY_SRC_CONFIRM)
    {
        if (!_gstUiCtx.language_list_show)
        {
            gstEmWinListLanguageType.create_win();
            _gstUiCtx.language_list_show = 1;
        }
        SetArrowDirection();
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
    {
        if(!_gstUiCtx.language_list_show && pCurrentEmWin->win_handle != 0)
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

void SetLanguageListShow(int32_t show)
{
    _gstUiCtx.language_list_show = show;
}

void SetArrowDirection(void)
{
    if(_gstUiCtx.language_list_show)
    {
        //箭头向下
        IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[1].p_img_buf, _garrArrowImgPara[1].img_size);
    }
    else
    {
        //箭头向右
        IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);
    }
}

int32_t SetCurLanguageType(int32_t idx)
{
    if (idx < EN_NATIONALLIST_CN || idx >= EN_NATIONALLIST_MAX)
    {
        return -1;
    }

    snprintf(_garrLanguagesTypeBuf, sizeof(_garrLanguagesTypeBuf), "%s", _gLanguageIdx[idx].name);

    TEXT_SetText(_gTextLanguageTypeHandle, (const char *)_gstLanguageTypeTextPara.p_content);
    IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);
    return 0;
}

