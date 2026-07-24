/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_reconn_grid_time.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-01
  * @brief       高级设置->设置->more settings->battery->DOD
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-01    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
  
#define  TEST_DYNAMIC											0 
 
#define		EM_BACK_ON_ICON_ARROW_SIZE                		(4096)  
#define		DIGIT_NUMS										(2)  
#define		REC_XPOS										(20)
#define 	REC_YPOS										(38)
#define 	NUMS_LINE_XPOS									(388)
#define		NUMS_LINE_YPOS									(65)	
#define		NUMS_LINE_LEN									(12)
#define		NUMS_LIN_OFFSET									(16)
 
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gEmWinListAdvanSettingBatDOD = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;

	int8_t num[DIGIT_NUMS];
	uint16_t DOD;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};



/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

/*DOD列表*/
static TEXT_Handle  _gTextDODHandle;//显示DOD句柄
static TEXT_Handle	_gTextNumHandle[DIGIT_NUMS];
static TEXT_Handle	_gTextPercentHandle;


static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 
static emWidgetTextPara_t  _gDODTextPara;
static emWidgetTextPara_t  _gNumTextPara[DIGIT_NUMS];
static emWidgetTextPara_t  _gPercentTextPara;

static char _gTitleBuf[] = "Setings < More Settings < Battery < DOD";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";
static char _gDODBuf[] = "Discharge Of Depth";
static char _gPercentBuf[] = "%";



static char _gDODNumTextBuf[DIGIT_NUMS][20];


#define WIN_LIST_DOD_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_DOD_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_DOD_PRESS_ID					      (GUI_ID_USER + 0x14)
#define WIN_LIST_DOD_BACK_ID					      (GUI_ID_USER + 0x15)
#define WIN_LIST_DOD_BACK_IMG_ID					  (GUI_ID_USER + 0x16)

#define WIN_MAIN_DOD_ID								 (GUI_ID_USER + 0x20)
#define WIN_MAIN_NUMBER1_ID							 (GUI_ID_USER + 0x21)
#define WIN_MAIN_NUMBER2_ID							 (GUI_ID_USER + 0x22)
#define WIN_MAIN_PERCENT_ID							 (GUI_ID_USER + 0x23)



/*资源列表*/
static const GUI_WIDGET_CREATE_INFO _aListDODDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListDODWin",    	WIN_LIST_DOD_ID,     				0,   	0,   	480,	320,	0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_DOD_TITLE_ID,     			31,  	20,  	415,	14,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_DOD_PRESS_ID,       		133, 	273, 	33,   	14,   	0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_DOD_BACK_IMG_ID,    		171-5, 	275, 	15,   	10,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_DOD_BACK_ID,       		188, 	273, 	150,  	14,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "DOD",			WIN_MAIN_DOD_ID,					31, 	48, 	180,  	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM1",			WIN_MAIN_NUMBER1_ID,       			406, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM2",			WIN_MAIN_NUMBER2_ID,       			422, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Percent",		WIN_MAIN_PERCENT_ID,       			437,	48,  	15,    	17,   	0,   0x0,  0 },

};

static int32_t _SetBattDodNumber(int32_t ui_update, uint16_t value )
{
	uint8_t i = 0;
	uint16_t tens  = value / 10;
	uint16_t uints = value % 10;
	
	snprintf(_gDODNumTextBuf[0],sizeof(_gDODNumTextBuf[0]), "%d", tens);
	snprintf(_gDODNumTextBuf[1],sizeof(_gDODNumTextBuf[0]), "%d", uints);
	
	if(ui_update)
	{
		for(i = 0; i < DIGIT_NUMS; ++i)
		{
			TEXT_SetText(_gTextNumHandle[i],(const char *)_gNumTextPara[i].p_content);
		}
	}

	
	return 0;
}


/*函数声明*/
void _SetSavedDODNumberColor(int32_t index);
void _SetDODSelectedNumAndLine(int32_t index);
void _SetDODNumAdd(int32_t index,int32_t v);
void _DrawDODNumUnderLine(int32_t index);

/**
  * @brief ListDODWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListDODWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
	static int32_t num = -1;
	static int32_t index = -1;
#if	TEST_DYNAMIC
	static WM_HTIMER wm_timer_handel_nums;
#endif
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
#if TEST_DYNAMIC
			wm_timer_handel_nums = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                             0, 	     /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                             1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                             0);		 /* 留待将来使用，应为0 */ 
#endif
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_DOD_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
			//列表中的文字
			_gTextDODHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_DOD_ID);
            TEXT_SetFont(_gTextDODHandle, _gDODTextPara.p_text_font);
            TEXT_SetText(_gTextDODHandle, (const char *)_gDODTextPara.p_content);
            TEXT_SetTextAlign(_gTextDODHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextDODHandle, _gDODTextPara.text_color); 
			//百分号的显示
			_gTextPercentHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_PERCENT_ID);
            TEXT_SetFont(_gTextPercentHandle, _gPercentTextPara.p_text_font);
            TEXT_SetText(_gTextPercentHandle, (const char *)_gPercentTextPara.p_content);
            TEXT_SetTextAlign(_gTextPercentHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPercentHandle, _gPercentTextPara.text_color);			
			
			for(int i = 0; i < DIGIT_NUMS; i++)
			{
				_gTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_NUMBER1_ID + i);
				TEXT_SetFont(_gTextNumHandle[i], _gNumTextPara[i].p_text_font);
				TEXT_SetText(_gTextNumHandle[i], (const char *)_gNumTextPara[i].p_content);
				TEXT_SetTextAlign(_gTextNumHandle[i], TEXT_CF_RIGHT | TEXT_CF_VCENTER);
				TEXT_SetTextColor(_gTextNumHandle[i], _gNumTextPara[i].text_color); 	
			}
		
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_DOD_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_DOD_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_DOD_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
			
			_SetSavedDODNumberColor(g_ui_ctx.select);
			
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
			_SetDODNumberColor(index);
			
			WM_Paint(pMsg->hWin);
            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
		}
		
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListDODPara, _gEmListDODPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
			GUI_FillRoundedRect(REC_XPOS, REC_YPOS,  REC_XPOS + 440 - 1,  REC_YPOS + 34 - 1, 10);
			GUI_SetColor(0x00BDC909);
			GUI_DrawRoundedFrame(REC_XPOS, REC_YPOS,  REC_XPOS + 440 - 1,  REC_YPOS + 34 - 1, 10,3);

			GUI_SetPenSize(3);
			GUI_SetColor(0x00D9D9D9);
			for(int i = 0; i < DIGIT_NUMS; i++)
			{
				GUI_DrawLine(_aListDODDialogCreate[i + 6].x0 - 2,_aListDODDialogCreate[i + 6].y0 + 18,
							 _aListDODDialogCreate[i + 6].x0 - 2 + 12,_aListDODDialogCreate[i + 6].y0 + 18);
			}
			
			
			_DrawDODNumUnderLine(g_ui_ctx.select);
//			_SetDODSelectedNumAndLine(index);
			
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
  * @brief Create Factory Restore win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateFactoryRestoreWin()
{
    gEmWinListAdvanSettingBatDOD.win_handle = GUI_CreateDialogBox(_aListDODDialogCreate, 
											  GUI_COUNTOF(_aListDODDialogCreate), 
											  _cbListDODWin, 
											  pCurrentEmWin->win_handle, 
											  0, 
											  0);
    WM_SetTransState(gEmWinListAdvanSettingBatDOD.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    return gEmWinListAdvanSettingBatDOD.win_handle;
}



/**
  * @brief Create current win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWin(void)
{
	g_ui_ctx.key_init = 0;
	pPrevEmWin = &gEmWinListAdvancedBattery;
	
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListAdvanSettingBatDOD.win_handle = GUI_CreateDialogBox(_aListDODDialogCreate, GUI_COUNTOF(_aListDODDialogCreate), _cbListDODWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListAdvanSettingBatDOD.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListAdvanSettingBatDOD.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListAdvanSettingBatDOD.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListDODPara, gEmWinListAdvanSettingBatDOD.win_handle);
    return gEmWinListAdvanSettingBatDOD.win_handle;
	
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListAdvanSettingBatDOD.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListAdvanSettingBatDOD.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListAdvanSettingBatDOD.win_handle = 0;
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	/*底部back的参数*/
	_gPressTextPara.p_content = _gPressBuf;
    _gPressTextPara.text_color = 0x00EEEEEE;
    _gPressTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gBackTextPara.p_content = _gBackBuf;
    _gBackTextPara.text_color = 0x00EEEEEE;
    _gBackTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_BACK_ON_ICON_ARROW_SIZE);
    _gReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gReturnImgPara.mem_img);
    _gReturnImgPara.img_size = EM_BACK_ON_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size);
	
	/*列表中的文字*/
	_gDODTextPara.p_text_font = EM_TEXT_FONT_5;
	_gDODTextPara.text_color = 0x00FFFFFF;
	_gDODTextPara.p_content = _gDODBuf;
	
	_gPercentTextPara.p_text_font = EM_TEXT_FONT_5;
	_gPercentTextPara.text_color = 0x00FFFFFF;
	_gPercentTextPara.p_content = _gPercentBuf;
	
	for(uint8_t i = 0; i < DIGIT_NUMS; i++)
	{
		_gNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
		_gNumTextPara[i].text_color = 0x00FFFFFF;
		_gNumTextPara[i].p_content = _gDODNumTextBuf[i];
//		_gNumTextPara[i].digit_value = 0;
//		sprintf(_gNumTextPara[i].p_content, "%1d", (int)_gNumTextPara[i].digit_value);
	}
#if 0
	_SetBattDodNumber(0,gCollectParaData);
#endif
	
	/*标题*/
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00EEEEEE;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gReturnImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
#if 0
	if(g_ui_ctx.DOD != gCollectParaData.)
	{
		g_ui_ctx.DOD != gCollectParaData;
		_SetBattDodNumber(1,g_ui_ctx.DOD);
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
	int32_t event = EmGetKeyEvent(p_msg);
    if(event < 0 || event > EN_KEY_SRC_NUM)
	{
		return;
	}
	
	if(event == EN_KEY_SRC_LEFT)
	{
		if(g_ui_ctx.select >= DIGIT_NUMS)
		{
			return;
		}
		g_ui_ctx.num[g_ui_ctx.select] = abs(10 + (--(g_ui_ctx.num[g_ui_ctx.select])))%10;
		_SetDODNumAdd(g_ui_ctx.select,g_ui_ctx.num[g_ui_ctx.select]);
		return;
	}
	else if(event == EN_KEY_SRC_RIGHT)
	{
		if(g_ui_ctx.select >= DIGIT_NUMS)
		{
			return;
		}
		g_ui_ctx.num[g_ui_ctx.select] = (++(g_ui_ctx.num[g_ui_ctx.select]))%10;
		_SetDODNumAdd(g_ui_ctx.select,g_ui_ctx.num[g_ui_ctx.select]);
		return;
	}
	else if(event == EN_KEY_SRC_CONFIRM)
	{
		g_ui_ctx.select = (++g_ui_ctx.select) % DIGIT_NUMS;
		_SetSavedDODNumberColor(g_ui_ctx.select);
		
		WM_Paint(gEmWinListAdvanSettingBatDOD.win_handle);
		
	}
	else if(event == EN_KEY_SRC_ESC)
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
				pnext = &gEmWinListAdvancedBattery;
			}
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win();
		}
	}
	
}

/**
  * @brief  设置选中数字文本的颜色
  * @param  NULL
  * @retval NULL
  */
void _SetSavedDODNumberColor(int32_t index)
{
	//_SetDODNumAdd(index);
	if(index < 0 || index >= DIGIT_NUMS)
	{
		return;
	}
	
	uint8_t i = 0;
	static volatile uint8_t nums_sel_falg[DIGIT_NUMS] = {0};
	for(i= 0; i < DIGIT_NUMS; i++)
	{
		nums_sel_falg[i] = 0;
		TEXT_SetTextColor(_gTextNumHandle[i],0x00FFFFFF);
	}
	nums_sel_falg[index] = 1;
	TEXT_SetTextColor(_gTextNumHandle[index],0x00CDF100);
}

/**
  * @brief  设置选中数字变化
  * @param  NULL
  * @retval NULL
  */
void _SetDODNumAdd(int32_t index,int32_t v)
{
	_gNumTextPara[index].digit_value = v;
	sprintf(_gNumTextPara[index].p_content, "%1d", (int)_gNumTextPara[index].digit_value);
	if(9 < _gNumTextPara[index].digit_value)
	{
		_gNumTextPara[index].digit_value = 0;
	}
	TEXT_SetText(_gTextNumHandle[index],_gDODNumTextBuf[index]);
}

/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
void _DrawDODNumUnderLine(int32_t index)
{
	GUI_SetColor(0x00CDF100);
	switch(index)
	{
		case 0:
		{
			GUI_DrawLine(_aListDODDialogCreate[index + 6].x0 - 2,_aListDODDialogCreate[index + 6].y0 + 18,
						 _aListDODDialogCreate[index + 6].x0 - 2 + 12,_aListDODDialogCreate[index + 6].y0 + 18);
		}
		break;
		case 1:
		{
			GUI_DrawLine(_aListDODDialogCreate[index + 6].x0 - 2,_aListDODDialogCreate[index + 6].y0 + 18,
						 _aListDODDialogCreate[index + 6].x0 - 2 + 12,_aListDODDialogCreate[index + 6].y0 + 18);
		}
		break;
		default:
			break;
	}
}
#if 0
/**
  * @brief  设置选中数字文本和线的值
  * @param  NULL
  * @retval NULL
  */
void _SetDODSelectedNumAndLine(int32_t index)
{
	if(index == 0)
	{
		GUI_SetColor(0x00CDF100);
		GUI_DrawLine(388,65,388+12,65);
		_gNumTextPara[index].digit_value++;
		sprintf(_gNumTextPara[index].p_content, "%1d", (int)_gNumTextPara[index].digit_value);
		if(9 < _gNumTextPara[index].digit_value)
		{
			_gNumTextPara[index].digit_value = 0;
		}
		TEXT_SetText(_gTextNumHandle[index],_gDODNumTextBuf[index]);
	}
	else
	{
		GUI_SetColor(0x00D9D9D9);
		GUI_DrawLine(388,65,388+12,65);
	}
	if(index == 1)
	{
		GUI_SetColor(0x00CDF100);
		GUI_DrawLine(404,65,404+12,65);
		_gNumTextPara[index].digit_value++;
		sprintf(_gNumTextPara[index].p_content, "%1d", (int)_gNumTextPara[index].digit_value);
		if(9 < _gNumTextPara[index].digit_value)
		{
			_gNumTextPara[index].digit_value = 0;
		}
		TEXT_SetText(_gTextNumHandle[index],_gDODNumTextBuf[index]);
	}
	else
	{
		GUI_SetColor(0x00D9D9D9);
		GUI_DrawLine(404,65,404+12,65);
	}
	if(index == 2)
	{
		GUI_SetColor(0x00CDF100);
		GUI_DrawLine(420,65,420+12,65);
		_gNumTextPara[index].digit_value++;
		sprintf(_gNumTextPara[index].p_content, "%1d", (int)_gNumTextPara[index].digit_value);
		if(9 < _gNumTextPara[index].digit_value)
		{
			_gNumTextPara[index].digit_value = 0;
		}
		TEXT_SetText(_gTextNumHandle[index],_gDODNumTextBuf[index]);
	}
	else
	{
		GUI_SetColor(0x00D9D9D9);
		GUI_DrawLine(420,65,420+12,65);
	}
}
	#endif
