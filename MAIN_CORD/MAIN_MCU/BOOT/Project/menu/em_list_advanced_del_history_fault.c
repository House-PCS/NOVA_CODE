/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_del_history_fault.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-02-21
  * @brief       高级设置->更多设置->清除历史故障
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-02-21    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"


#define  EM_LIST_DEL_HISTORY_FAULT_ICON_BACK_SIZE						(4096)
#define  EM_LIST_DEL_HISTORY_FAULT_ICON_RIGHT_ARROW_SIZE				(4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListDelHistoryFault = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 
static TEXT_Handle  _gDelHistoryFaultHandle;//清除历史故障文字句柄

static char _gTitleBuf[] = "Settings < More Settings < Delete History Fault";
static char _gDelHistoryFaultBuf[] = "Delete History Fault";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";

static IMAGE_Handle  _gImgArrowHandle;//箭头图标句柄 


/*List控件*/
static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gDelHistoryFaultTextPara;
static emWidgetImgPara_t  _gArrowImgPara; 
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 

#define WIN_LIST_DEL_HISTORY_FAULT_ID                                (GUI_ID_USER + 0x10)
#define WIN_LIST_DEL_HISTORY_FAULT_TITLE_ID                          (GUI_ID_USER + 0x11)
#define WIN_LIST_DEL_HISTORY_FAULT_TEXT_ID							 (GUI_ID_USER + 0x12)
#define WIN_LIST_DEL_HISTORY_FAULT_PRESS_ID							 (GUI_ID_USER + 0x13)
#define WIN_LIST_DEL_HISTORY_FAULT_BACK_IMG_ID					 	 (GUI_ID_USER + 0x14)
#define WIN_LIST_DEL_HISTORY_FAULT_BACK_ID						 	 (GUI_ID_USER + 0x15)
#define WIN_LIST_DEL_HISTORY_FAULT_ARROW_IMG_ID						 (GUI_ID_USER + 0x16)

/*函数声明*/

static const GUI_WIDGET_CREATE_INFO _aListDelHistoryFaultDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListDelHistoryFaultWin",    WIN_LIST_DEL_HISTORY_FAULT_ID,    	 		0,   	0,  	480,  	320,  	0,   0x0,  	0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_DEL_HISTORY_FAULT_TITLE_ID,    				31,  	20, 	415,  	14,   	0,   0x0,  	0 },
	{ TEXT_CreateIndirect,    "lcdbcak",	 	WIN_LIST_DEL_HISTORY_FAULT_TEXT_ID,						31,  	48, 	300,  	17,   	0,   0x0,  	0 },
	{ IMAGE_CreateIndirect,	  "ARROW",			WIN_LIST_DEL_HISTORY_FAULT_ARROW_IMG_ID,   				437,	50,		15,		15,		0,	 0x0,   0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_DEL_HISTORY_FAULT_PRESS_ID,       				133, 	273, 	33,   	14,   	0,   0x0,  	0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_DEL_HISTORY_FAULT_BACK_IMG_ID,    				171-5, 	275, 	15,   	10,   	0,   0x0,  	0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_DEL_HISTORY_FAULT_BACK_ID,       				188, 	273, 	150,  	14,   	0,   0x0,  	0 },
};



/**
  * @brief ListDelHistoryFaultWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListDelHistoryFaultWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_DEL_HISTORY_FAULT_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
			_gDelHistoryFaultHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_DEL_HISTORY_FAULT_TEXT_ID);
            TEXT_SetFont(_gDelHistoryFaultHandle, _gDelHistoryFaultTextPara.p_text_font);
            TEXT_SetText(_gDelHistoryFaultHandle, (const char *)_gDelHistoryFaultTextPara.p_content);
            TEXT_SetTextAlign(_gDelHistoryFaultHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gDelHistoryFaultHandle, _gDelHistoryFaultTextPara.text_color); 
						
			_gImgArrowHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_DEL_HISTORY_FAULT_ARROW_IMG_ID);
            IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara.p_img_buf, _gArrowImgPara.img_size); 
			
						
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_DEL_HISTORY_FAULT_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_DEL_HISTORY_FAULT_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_DEL_HISTORY_FAULT_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
            
//			SetCurGridFlag(g_ui_ctx.select);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListDelHistoryFaultPara, _gEmListDelHistoryFaultPara.list_pre_sel);
			
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
	g_ui_ctx.key_init = 0;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListDelHistoryFault.win_handle = GUI_CreateDialogBox(_aListDelHistoryFaultDialogCreate, GUI_COUNTOF(_aListDelHistoryFaultDialogCreate), _cbListDelHistoryFaultWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListDelHistoryFault.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListDelHistoryFault.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListDelHistoryFault.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    //SelectedDraw(&_gEmListDelHistoryFaultPara, gEmWinListDelHistoryFault.win_handle);
	//gEmWinListDelHistoryFaultFlag.create_win();
    return gEmWinListDelHistoryFault.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListDelHistoryFault.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListDelHistoryFault.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListDelHistoryFault.win_handle = 0;
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
	
	_gDelHistoryFaultTextPara.p_content = _gDelHistoryFaultBuf;
	_gDelHistoryFaultTextPara.text_color = 0x00FFFFFF;
	_gDelHistoryFaultTextPara.p_text_font = EM_TEXT_FONT_5;
	
	/*右箭头*/
	_gArrowImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_DEL_HISTORY_FAULT_ICON_RIGHT_ARROW_SIZE);
	_gArrowImgPara.p_img_buf = GUI_ALLOC_h2p(_gArrowImgPara.mem_img);
	_gArrowImgPara.img_size = EM_LIST_DEL_HISTORY_FAULT_ICON_RIGHT_ARROW_SIZE;
	SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR,_gArrowImgPara.p_img_buf,_gArrowImgPara.img_size);
	
		
	_gPressTextPara.p_content = _gPressBuf;
    _gPressTextPara.text_color = 0x00EEEEEE;
    _gPressTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gBackTextPara.p_content = _gBackBuf;
    _gBackTextPara.text_color = 0x00EEEEEE;
    _gBackTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_DEL_HISTORY_FAULT_ICON_BACK_SIZE);
    _gReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gReturnImgPara.mem_img);
    _gReturnImgPara.img_size = EM_LIST_DEL_HISTORY_FAULT_ICON_BACK_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size);
	
	
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gArrowImgPara.mem_img);
	GUI_ALLOC_Free(_gReturnImgPara.mem_img);
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
	int32_t event = EmGetKeyEvent(p_msg);
	if (event < 0 || event > EN_KEY_SRC_NUM)
	{
		return;
	}
	if (event == EN_KEY_SRC_CONFIRM)
	{
		gDelHistoryfaultPopUpWin.create_win();
		pCurrentEmWin = &gDelHistoryfaultPopUpWin;
		#if 0 
		emWin_t *pnext = NULL;
		pnext = &gDelHistoryfaultPopUpWin;
		if(pnext != NULL && pCurrentEmWin->win_handle != 0)
		{
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win(); 
		}
		#endif
		return;
	}
	else if (event == EN_KEY_SRC_ESC)
	{
		if(pCurrentEmWin->win_handle != 0)
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


