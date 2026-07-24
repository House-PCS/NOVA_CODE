/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-01-17
  * @brief       高级设置-电网标准设置
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-17    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define  EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE			(4096)
#define  EM_LIST_GRID_STAND_ICON_DOWN_ARROW_SIZE        (4096)
#define  EM_LIST_GRID_STAND_ICON_BACK_SIZE				(4096)
#define  EM_LIST_ICON_ARROW_SIZE						(4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListGridStand = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_t
{
	int32_t grid_list_show;
	int32_t select;
	int32_t key_init;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

typedef struct _national_idx_t
{
	const int32_t img_addr;
	const char name[16];
} NationalIdx_t;

extern FlagIdx flag_index;
static NationalIdx_t national_idx[] = 
{
	{EM_ICON_GERMAN_FLAG_ADDR,			"German"	},
	{EM_ICON_CHINESE_FLAG_ADDR,			"China"		},
	{EM_ICON_AUSTRALIA_FLAG_ADDR,		"Australia"	},
	{EM_ICON_ITALY_FLAG_ADDR,			"Italy"		},
	{EM_ICON_SPAIN_FLAG_ADDR,			"Spanish"	},
	{EM_ICON_FRANCE_FLAG_ADDR,			"France"	},
	{EM_ICON_GREECE_FLAG_ADDR,			"Greece"	},
	{EM_ICON_HUNGARY_FLAG_ADDR,			"Hungary"	},
	{EM_ICON_BELGIUM_FLAG_ADDR,			"Belgium"	},
	{EM_ICON_THAILAND_FLAG_ADDR,		"Thailand"	},
	{EM_ICON_ENGLAND_FLAG_ADDR,			"England"	},
};

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

static char _gTitleBuf[] = "Settings < Grid Standard";
static char _gGridStandBuf[] = "Grid Standard";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";
static char _gCountryBuf[64] = {"China"};


static TEXT_Handle  _gTextGridStandHandle;//显示GridStandard句柄
static TEXT_Handle  _gTextCountryHandle;//显示Conutry句柄
static IMAGE_Handle  _gImgNationaHandle;//国旗图标句柄 
static IMAGE_Handle  _gImgArrowHandle;//箭头图标句柄 


/*List控件*/
static emListPara_t  _gEmListOperaInfoPara;

static emWidgetTextPara_t  _gGridStandTextPara;
static emWidgetTextPara_t  _gCountryTextPara;
static emWidgetImgPara_t  _gFlagImgPara;
static emWidgetImgPara_t  _gArrowImgPara[2];   //0：右箭头   1：左箭头
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 

#define WIN_LIST_GRID_STAND_ID                               	 (GUI_ID_USER + 0x10)
#define WIN_LIST_GRID_STAND_TITLE_ID                         	 (GUI_ID_USER + 0x11)
#define WIN_LIST_GRID_STAND_TEXT_ID							 	 (GUI_ID_USER + 0x12)
#define WIN_LIST_ARROW_IMG_ID								 	 (GUI_ID_USER + 0x13)
#define WIN_LIST_GRID_STAND_FLAG_PRESS_ID						 (GUI_ID_USER + 0x14)
#define WIN_LIST_GRID_STAND_FLAG_BACK_IMG_ID					 (GUI_ID_USER + 0x15)
#define WIN_LIST_GRID_STAND_FLAG_BACK_ID						 (GUI_ID_USER + 0x16)

#define WIN_LIST_GRID_STAND_FLAG_IMG_ID							 (GUI_ID_USER + 0x17)
#define WIN_LIST_GRID_STAND_COUNTRY_TEXT_ID						 (GUI_ID_USER + 0x18)
#define WIN_LIST_GRID_STAND_ARROW_IMG_ID						 (GUI_ID_USER + 0x19)

int32_t SetCurGridFlag(int32_t idx);

static const GUI_WIDGET_CREATE_INFO _aListOperaInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListOperaInfoWin",    WIN_LIST_GRID_STAND_ID,    	 				0,   	0,  	480,  	320,  	0,   0x0,  	0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_GRID_STAND_TITLE_ID,    					31,  	20, 	415,  	14,   	0,   0x0,  	0 },
	{ TEXT_CreateIndirect,    "Grid Standard",	WIN_LIST_GRID_STAND_TEXT_ID,						31,  	48, 	152,  	17,   	0,   0x0,  	0 },

	{ IMAGE_CreateIndirect,   "Imgfalg",		WIN_LIST_GRID_STAND_FLAG_IMG_ID,
		338,	41,		30,		30,		0,	 0x0,  0 },
	{ TEXT_CreateIndirect,    "COUNTRY",		WIN_LIST_GRID_STAND_COUNTRY_TEXT_ID,
		374,	48,		50,		17,		0,	 0x0,  0 },
	{ IMAGE_CreateIndirect,	  "ARROW",			WIN_LIST_GRID_STAND_ARROW_IMG_ID,
		437,	50,		15,		15,		0,	 0x0,  0 },


	//{ IMAGE_CreateIndirect,	  "Img",			WIN_LIST_ARROW_IMG_ID,				 				434, 	52, 	20,    	20,   	0,   0x0,  	0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_GRID_STAND_FLAG_PRESS_ID,       			133, 	273, 	33,   	14,   	0,   0x0,  	0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_GRID_STAND_FLAG_BACK_IMG_ID,    			171-5, 	275, 	15,   	10,   	0,   0x0,  	0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_GRID_STAND_FLAG_BACK_ID,       			188, 	273, 	150,  	14,   	0,   0x0,  	0 },
};

/*函数声明*/
void SetGridStandArrowDirection(void);



/**
  * @brief ListOperaInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListOperaInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
			_gTextGridStandHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_TEXT_ID);
            TEXT_SetFont(_gTextGridStandHandle, _gGridStandTextPara.p_text_font);
            TEXT_SetText(_gTextGridStandHandle, (const char *)_gGridStandTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridStandHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridStandHandle, _gGridStandTextPara.text_color); 

			_gTextCountryHandle = WM_GetDialogItem(pMsg->hWin, 
									 WIN_LIST_GRID_STAND_COUNTRY_TEXT_ID);
			TEXT_SetFont(_gTextCountryHandle, _gCountryTextPara.p_text_font);
            TEXT_SetText(_gTextCountryHandle, (const char *)_gCountryTextPara.p_content);
            TEXT_SetTextAlign(_gTextCountryHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextCountryHandle, _gCountryTextPara.text_color); 
			
			_gImgNationaHandle = WM_GetDialogItem(pMsg->hWin, 
												  WIN_LIST_GRID_STAND_FLAG_IMG_ID);
            IMAGE_SetDTA(_gImgNationaHandle, _gFlagImgPara.p_img_buf, _gFlagImgPara.img_size); 			
			_gImgArrowHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_ARROW_IMG_ID);
            IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara[0].p_img_buf, _gArrowImgPara[0].img_size); 
			
						
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_FLAG_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_FLAG_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_FLAG_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
            
			SetCurGridFlag(flag_index.flag_idx);
//			SetCurGridFlag(g_ui_ctx.select);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListOperaInfoPara, _gEmListOperaInfoPara.list_pre_sel);
			
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
	g_ui_ctx.grid_list_show = 0;
	g_ui_ctx.key_init = 0;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListGridStand.win_handle = GUI_CreateDialogBox(_aListOperaInfoDialogCreate, GUI_COUNTOF(_aListOperaInfoDialogCreate), _cbListOperaInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListGridStand.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListGridStand.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListGridStand.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    //SelectedDraw(&_gEmListOperaInfoPara, gEmWinListGridStand.win_handle);
	//gEmWinListGridStandFlag.create_win();
    return gEmWinListGridStand.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListGridStand.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListGridStand.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListGridStand.win_handle = 0;
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
	
	
	_gGridStandTextPara.p_content = _gGridStandBuf;
	_gGridStandTextPara.text_color = 0x00FFFFFF;
	_gGridStandTextPara.p_text_font = EM_TEXT_FONT_5;


	/*country*/
	_gCountryTextPara.p_text_font = EM_TEXT_FONT_5;
	_gCountryTextPara.text_color = 0x00FFFFFF;
	_gCountryTextPara.p_content = _gCountryBuf;
	
	/*国旗*/
	_gFlagImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE);
	_gFlagImgPara.p_img_buf = GUI_ALLOC_h2p(_gFlagImgPara.mem_img);
	_gFlagImgPara.img_size = EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE;
	SpiFlashReadData(EM_ICON_CHINESE_FLAG_ADDR,_gFlagImgPara.p_img_buf,_gFlagImgPara.img_size);
	
	/*右箭头*/
	_gArrowImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_SIZE);
	_gArrowImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gArrowImgPara[0].mem_img);
	_gArrowImgPara[0].img_size = EM_LIST_ICON_ARROW_SIZE;
	SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR,_gArrowImgPara[0].p_img_buf,_gArrowImgPara[0].img_size);
	/*下箭头*/
	_gArrowImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_GRID_STAND_ICON_DOWN_ARROW_SIZE);
    _gArrowImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gArrowImgPara[1].mem_img);
	_gArrowImgPara[1].img_size = EM_LIST_GRID_STAND_ICON_DOWN_ARROW_SIZE;
	SpiFlashReadData(EM_ICON_DOWN_ARROW_ADDR, _gArrowImgPara[1].p_img_buf, _gArrowImgPara[1].img_size);
	
		
	_gPressTextPara.p_content = _gPressBuf;
    _gPressTextPara.text_color = 0x00EEEEEE;
    _gPressTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gBackTextPara.p_content = _gBackBuf;
    _gBackTextPara.text_color = 0x00EEEEEE;
    _gBackTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_GRID_STAND_ICON_BACK_SIZE);
    _gReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gReturnImgPara.mem_img);
    _gReturnImgPara.img_size = EM_LIST_GRID_STAND_ICON_BACK_SIZE;
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
	GUI_ALLOC_Free(_gFlagImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
	if (g_ui_ctx.grid_list_show)
	{
		gEmWinListGridStandFlag.polling();
	}
}

/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
	if (g_ui_ctx.grid_list_show)
	{
		gEmWinListGridStandFlag.key_handler(p_msg);

		if (!g_ui_ctx.grid_list_show)
		{
			gEmWinListGridStandFlag.delete_win();
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
		if (!g_ui_ctx.grid_list_show)
		{
			gEmWinListGridStandFlag.create_win();
			g_ui_ctx.grid_list_show = 1;
		}
		SetGridStandArrowDirection();
		return;
	}
	else if (event == EN_KEY_SRC_ESC)
	{
		if(!g_ui_ctx.grid_list_show && pCurrentEmWin->win_handle != 0)
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

void SetGridStandArrowDirection(void)
{
	if(g_ui_ctx.grid_list_show)
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


void SetGridListShow(int32_t show)
{
	g_ui_ctx.grid_list_show = show;
}

int32_t SetCurGridFlag(int32_t idx)
{
	if (idx < EN_NATIONALLIST_DE || idx >= EN_NATIONALLIST_MAX)
	{
		return -1;
	}

	snprintf(_gCountryBuf, sizeof(_gCountryBuf), "%s", national_idx[idx].name);
	SpiFlashReadData(national_idx[idx].img_addr,_gFlagImgPara.p_img_buf,
					 _gFlagImgPara.img_size);
	TEXT_SetText(_gTextCountryHandle, (const char *)_gCountryTextPara.p_content);
	IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara[0].p_img_buf, _gArrowImgPara[0].img_size); 
	return 0;
}

