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


#define  EM_LIST_LANGUAGE_ICON_DOWN_ARROW_SIZE        		(4096)
#define  EM_LIST_LANGUAGE_ICON_BACK_SIZE					(4096)
#define  EM_LIST_LANGUAGE_ICON_RIGHT_ARROW_SIZE				(4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListLanguage = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_t
{
	int32_t language_list_show;
	int32_t select;
	int32_t key_init;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

typedef struct _language_idx_t
{
	const char name[16];
} LanguageIdx_t;

static LanguageIdx_t language_idx[] = 
{
	{"English"  },
	{"Chinese"	},
	{"Australia"},
	{"Italian"	},
	{"Spanish"	},
	{"France"	},
	{"German"	},
	{"Greek"	},
	{"Hungary"	},
	{"Belgium"	},
	{"Thailand"	},
	{"England"	},
};

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

static char _gTitleBuf[] = "Settings < More Settings < Language";
static char _gLanguageBuf[] = "Language";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";
static char _gLanguagesTypeBuf[64] = {"English"};


static TEXT_Handle  _gTextLanguageHandle;//显示Language句柄
static TEXT_Handle  _gTextLanguageTypeHandle;//显示国家语言句柄
static IMAGE_Handle  _gImgArrowHandle;//箭头图标句柄 


/*List控件*/
static emWidgetTextPara_t  _gLanguageTextPara;
static emWidgetTextPara_t  _gLanguageTypeTextPara;
static emWidgetImgPara_t  _gArrowImgPara[2]; 
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 

#define WIN_LIST_LANGUAGE_ID                               	 (GUI_ID_USER + 0x10)
#define WIN_LIST_LANGUAGE_TITLE_ID                         	 (GUI_ID_USER + 0x11)
#define WIN_LIST_LANGUAGE_TEXT_ID							 (GUI_ID_USER + 0x12)
#define WIN_LIST_LANGUAGE_PRESS_ID							 (GUI_ID_USER + 0x13)
#define WIN_LIST_LANGUAGE_BACK_IMG_ID					 	 (GUI_ID_USER + 0x14)
#define WIN_LIST_LANGUAGE_BACK_ID						 	 (GUI_ID_USER + 0x15)

#define WIN_LIST_LANGUAGE_IMG_ID							 (GUI_ID_USER + 0x16)
#define WIN_LIST_LANGUAGE_COUNTRY_TEXT_ID					 (GUI_ID_USER + 0x17)
#define WIN_LIST_LANGUAGE_ARROW_IMG_ID						 (GUI_ID_USER + 0x18)

/*函数声明*/
int32_t SetCurLanguageType(int32_t idx);
void SetArrowDirection(void);

static const GUI_WIDGET_CREATE_INFO _aListLanguageDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListLanguageWin",    WIN_LIST_LANGUAGE_ID,    	 			0,   	0,  	480,  	320,  	0,   0x0,  	0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_LANGUAGE_TITLE_ID,    				31,  	20, 	415,  	14,   	0,   0x0,  	0 },
	{ TEXT_CreateIndirect,    "LANGUAGE",	 	WIN_LIST_LANGUAGE_TEXT_ID,					31,  	48, 	152,  	17,   	0,   0x0,  	0 },
	{ TEXT_CreateIndirect,    "COUNTRY",		WIN_LIST_LANGUAGE_COUNTRY_TEXT_ID,			343,	48,		100,	17,		0,	 0x0,   0 },
	{ IMAGE_CreateIndirect,	  "ARROW",			WIN_LIST_LANGUAGE_ARROW_IMG_ID,   			437,	50,		15,		15,		0,	 0x0,   0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_LANGUAGE_PRESS_ID,       			133, 	273, 	33,   	14,   	0,   0x0,  	0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_LANGUAGE_BACK_IMG_ID,    			171-5, 	275, 	15,   	10,   	0,   0x0,  	0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_LANGUAGE_BACK_ID,       			188, 	273, 	150,  	14,   	0,   0x0,  	0 },
};



/**
  * @brief ListLanguageWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListLanguageWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
			_gTextLanguageHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_TEXT_ID);
            TEXT_SetFont(_gTextLanguageHandle, _gLanguageTextPara.p_text_font);
            TEXT_SetText(_gTextLanguageHandle, (const char *)_gLanguageTextPara.p_content);
            TEXT_SetTextAlign(_gTextLanguageHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLanguageHandle, _gLanguageTextPara.text_color); 

			_gTextLanguageTypeHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_COUNTRY_TEXT_ID);
			TEXT_SetFont(_gTextLanguageTypeHandle, _gLanguageTypeTextPara.p_text_font);
            TEXT_SetText(_gTextLanguageTypeHandle, (const char *)_gLanguageTypeTextPara.p_content);
            TEXT_SetTextAlign(_gTextLanguageTypeHandle, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLanguageTypeHandle, _gLanguageTypeTextPara.text_color); 
						
			_gImgArrowHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_ARROW_IMG_ID);
            IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara[0].p_img_buf, _gArrowImgPara[0].img_size); 
			
						
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
            
//			SetCurGridFlag(g_ui_ctx.select);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListLanguagePara, _gEmListLanguagePara.list_pre_sel);
			
			GUI_SetColor(0x00424242);//上色
			GUI_FillRoundedRect(20, 38,  20+440-1,  38+34-1, 10);
			GUI_SetColor(0x00BDC909);
			GUI_DrawRoundedFrame(20, 38,  20+440-1,  38+34-1,10,3);
			
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
	g_ui_ctx.language_list_show = 0;
	g_ui_ctx.key_init = 0;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListLanguage.win_handle = GUI_CreateDialogBox(_aListLanguageDialogCreate, GUI_COUNTOF(_aListLanguageDialogCreate), _cbListLanguageWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListLanguage.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListLanguage.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListLanguage.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    //SelectedDraw(&_gEmListLanguagePara, gEmWinListLanguage.win_handle);
	//gEmWinListLanguageFlag.create_win();
    return gEmWinListLanguage.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListLanguage.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListLanguage.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListLanguage.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	EmKeyDefaultInfoSet();
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
	
	_gLanguageTextPara.p_content = _gLanguageBuf;
	_gLanguageTextPara.text_color = 0x00FFFFFF;
	_gLanguageTextPara.p_text_font = EM_TEXT_FONT_5;

	/*语言类型*/
	_gLanguageTypeTextPara.p_text_font = EM_TEXT_FONT_5;
	_gLanguageTypeTextPara.text_color = 0x00FFFFFF;
	_gLanguageTypeTextPara.p_content = _gLanguagesTypeBuf;
	
	/*右箭头*/
	_gArrowImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_LANGUAGE_ICON_RIGHT_ARROW_SIZE);
	_gArrowImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gArrowImgPara[0].mem_img);
	_gArrowImgPara[0].img_size = EM_LIST_LANGUAGE_ICON_RIGHT_ARROW_SIZE;
	SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR,_gArrowImgPara[0].p_img_buf,_gArrowImgPara[0].img_size);
	/*下箭头*/
	_gArrowImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_LANGUAGE_ICON_DOWN_ARROW_SIZE);
    _gArrowImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gArrowImgPara[1].mem_img);
	_gArrowImgPara[1].img_size = EM_LIST_LANGUAGE_ICON_DOWN_ARROW_SIZE;
	SpiFlashReadData(EM_ICON_DOWN_ARROW_ADDR, _gArrowImgPara[1].p_img_buf, _gArrowImgPara[1].img_size);
	
		
	_gPressTextPara.p_content = _gPressBuf;
    _gPressTextPara.text_color = 0x00EEEEEE;
    _gPressTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gBackTextPara.p_content = _gBackBuf;
    _gBackTextPara.text_color = 0x00EEEEEE;
    _gBackTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_LANGUAGE_ICON_BACK_SIZE);
    _gReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gReturnImgPara.mem_img);
    _gReturnImgPara.img_size = EM_LIST_LANGUAGE_ICON_BACK_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size);
	
	
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gArrowImgPara[0].mem_img);
    GUI_ALLOC_Free(_gArrowImgPara[1].mem_img);
	GUI_ALLOC_Free(_gReturnImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
	if (g_ui_ctx.language_list_show)
	{
		gEmWinListLanguageType.polling();
	}
}

/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
	if (g_ui_ctx.language_list_show)
	{
		gEmWinListLanguageType.key_handler(p_msg);

		if (!g_ui_ctx.language_list_show)
		{
			gEmWinListLanguageType.delete_win();
		}
		return;
	}
	int32_t event = EmGetKeyEvent(p_msg);
	if (event < 0 || event > EN_KEY_SRC_NUM)
	{
		return;
	}
	if (event == EN_KEY_SRC_CONFIRM)
	{
		if (!g_ui_ctx.language_list_show)
		{
			gEmWinListLanguageType.create_win();
			g_ui_ctx.language_list_show = 1;
		}
		SetArrowDirection();
		return;
	}
	else if (event == EN_KEY_SRC_ESC)
	{
		if(!g_ui_ctx.language_list_show && pCurrentEmWin->win_handle != 0)
		{
			emWin_t *pnext = NULL;
			if(p_msg->press_down_time >= gEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
			{
				pnext = &gEmWinHomePage;
			}
			else
			{
				pnext = &gEmWinListAdvancedSettings;
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
	g_ui_ctx.language_list_show = show;
}

void SetArrowDirection(void)
{
	if(g_ui_ctx.language_list_show)
	{
		//箭头向下
		IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara[1].p_img_buf, _gArrowImgPara[1].img_size); 
	}
	else
	{
		//箭头向右
		IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara[0].p_img_buf, _gArrowImgPara[0].img_size); 
	}
}	

int32_t SetCurLanguageType(int32_t idx)
{
	if (idx < EN_NATIONALLIST_DE || idx >= EN_NATIONALLIST_MAX)
	{
		return -1;
	}

	snprintf(_gLanguagesTypeBuf, sizeof(_gLanguagesTypeBuf), "%s", language_idx[idx].name);

	TEXT_SetText(_gTextLanguageTypeHandle, (const char *)_gLanguageTypeTextPara.p_content);
	IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara[0].p_img_buf, _gArrowImgPara[0].img_size); 
	return 0;
}

