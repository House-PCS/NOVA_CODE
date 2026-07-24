/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_del_history_fault_popup.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-21
  * @brief       高级设置->更多设置->清除历史故障(界面中的弹窗)
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


#define  EM_DEL_HISTORY_POP_UP_ICON_ALERT_SIZE                (4096)


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gDelHistoryfaultPopUpWin = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


static emPopUpPara_t  _gDelHistoryEmPopUpPara;


static GUI_HMEM _gDelHistoryPopUpTitleMem;
static uint8_t *_pDelHistoryPopUpTitleBuf;
static char _gDelHistoryPopUpTitleBuf[60];
static char _gDelHistoryPopUpYESBuf[5];
static char _gDelHistoryPopUpNOBuf[5];


#define WIN_LIST_DEL_HISTORY_FAULT_POPUP_ID                               (GUI_ID_USER + 0x40)

/*函数声明*/
static void _DrawSelPopUpRecPaint(int32_t sel);

/**
  * @brief 弹窗Callback
  * @param  NULL
  * @retval NULL
  */ 
static void _cbDelHistoryPopUpWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            EmPopUpSubWidgetCreate(&_gDelHistoryEmPopUpPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
			/*默认选No*/
			//NO的边框
			GUI_SetColor(0x00FFFFFF);
			GUI_FillRoundedRect(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3);
			GUI_SetColor(0x00000000);
			GUI_DrawRoundedFrame(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3,1);
			//yes的边框
			GUI_SetColor(0x00A4A4A4);
			GUI_FillRoundedRect(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3);
//			GUI_SetColor(0x00000000);
//			GUI_DrawRoundedFrame(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3,1);
			
			_DrawSelPopUpRecPaint(g_ui_ctx.select);
			
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
  * @brief 创建弹窗
  * @param  p_widget_para:list控件结构体
  * @param  h_win_src:List的父控件
  * @retval NULL
  */ 
GUI_HWIN EmDelHistoryPopUpCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src)
{
    static GUI_WIDGET_CREATE_INFO  pop_wm[1];
    GUI_HWIN handle;
    pop_wm[0].pfCreateIndirect = WINDOW_CreateIndirect;
    pop_wm[0].pName = "pop_up";
    pop_wm[0].Id = p_widget_para->popup_start_id;
    pop_wm[0].x0 = 0;
    pop_wm[0].y0 = 0;
    pop_wm[0].xSize = p_widget_para->x_size;
    pop_wm[0].ySize = p_widget_para->y_size;
    pop_wm[0].Flags = 0;
    pop_wm[0].Para = 0;
    pop_wm[0].NumExtraBytes = 0;
    
    handle = GUI_CreateDialogBox(pop_wm, GUI_COUNTOF(pop_wm), _cbDelHistoryPopUpWin, h_win_src, p_widget_para->x_pos, p_widget_para->y_pos);
    WINDOW_SetBkColor(handle, p_widget_para->bk_color);
    GUI_Exec();
    return handle;
}
  
  
/**
  * @brief 删除弹窗
  * @param  p_widget_para:list控件结构体
  * @retval NULL
  */   
void EmDelHistoryPopUpDel(emPopUpPara_t *p_widget_para)
{
    if(gDelHistoryfaultPopUpWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gDelHistoryfaultPopUpWin.win_handle);
    gDelHistoryfaultPopUpWin.win_handle = 0;
}





/**
  * @brief Create current win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWin(void)
{
    if(pCurrentEmWin->win_handle == 0)
    {
        return 0;
    }
	pPrevEmWin = &gEmWinListDelHistoryFault;
    _Init();
    gDelHistoryfaultPopUpWin.win_handle = EmDelHistoryPopUpCreate(&_gDelHistoryEmPopUpPara, pCurrentEmWin->win_handle);
    pCurrentPopUpSubWin = &gDelHistoryfaultPopUpWin;
    return gDelHistoryfaultPopUpWin.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gDelHistoryfaultPopUpWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gDelHistoryfaultPopUpWin.win_handle);
    _Del();
    gDelHistoryfaultPopUpWin.win_handle = 0;
    pCurrentPopUpSubWin = NULL;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    _gDelHistoryEmPopUpPara.bk_color = 0x006A7105;//后续根据错误还是警告信息填入合适的颜色
    _gDelHistoryEmPopUpPara.x_pos = 115;
    _gDelHistoryEmPopUpPara.y_pos = 122;
    _gDelHistoryEmPopUpPara.x_size = 250;
    _gDelHistoryEmPopUpPara.y_size = 75;
    
    _gDelHistoryEmPopUpPara.popup_start_id = WIN_LIST_DEL_HISTORY_FAULT_POPUP_ID;
    _gDelHistoryEmPopUpPara.widget_num = 4;
    
    _gDelHistoryPopUpTitleMem = GUI_ALLOC_AllocZero(EM_DEL_HISTORY_POP_UP_ICON_ALERT_SIZE);
    _pDelHistoryPopUpTitleBuf = GUI_ALLOC_h2p(_gDelHistoryPopUpTitleMem);
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _pDelHistoryPopUpTitleBuf, EM_DEL_HISTORY_POP_UP_ICON_ALERT_SIZE);
    /*子控件参数初始化*/
    _gDelHistoryEmPopUpPara.sub_widget_para[0].widget_type = EM_WIDGET_TYPE_IMG;
    _gDelHistoryEmPopUpPara.sub_widget_para[0].x_relative_pos = 12;
    _gDelHistoryEmPopUpPara.sub_widget_para[0].y_relative_pos = 13;
    _gDelHistoryEmPopUpPara.sub_widget_para[0].x_size = 21;
    _gDelHistoryEmPopUpPara.sub_widget_para[0].y_size = 21;
    _gDelHistoryEmPopUpPara.sub_widget_para[0].sub_widget.img_para.mem_img = _gDelHistoryPopUpTitleMem;
    _gDelHistoryEmPopUpPara.sub_widget_para[0].sub_widget.img_para.p_img_buf = _pDelHistoryPopUpTitleBuf;
    _gDelHistoryEmPopUpPara.sub_widget_para[0].sub_widget.img_para.img_size = EM_DEL_HISTORY_POP_UP_ICON_ALERT_SIZE;
    
    _gDelHistoryEmPopUpPara.sub_widget_para[1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gDelHistoryEmPopUpPara.sub_widget_para[1].x_relative_pos = 43;
    _gDelHistoryEmPopUpPara.sub_widget_para[1].y_relative_pos = 10;
    _gDelHistoryEmPopUpPara.sub_widget_para[1].x_size = 192;
    _gDelHistoryEmPopUpPara.sub_widget_para[1].y_size = 34;
    _gDelHistoryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content = _gDelHistoryPopUpTitleBuf;
    _gDelHistoryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gDelHistoryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gDelHistoryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content, "Please confirm whether to\ndelete the record !");//后续根据错误还是警告信息填入
    
    _gDelHistoryEmPopUpPara.sub_widget_para[2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gDelHistoryEmPopUpPara.sub_widget_para[2].x_relative_pos = 149;
    _gDelHistoryEmPopUpPara.sub_widget_para[2].y_relative_pos = 52;
    _gDelHistoryEmPopUpPara.sub_widget_para[2].x_size = 50;
    _gDelHistoryEmPopUpPara.sub_widget_para[2].y_size = 17;
    _gDelHistoryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_content = _gDelHistoryPopUpNOBuf;
    _gDelHistoryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.text_color = 0x00000000;
    _gDelHistoryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gDelHistoryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_content, "No");//后续根据错误还是警告信息填入
    
    _gDelHistoryEmPopUpPara.sub_widget_para[3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gDelHistoryEmPopUpPara.sub_widget_para[3].x_relative_pos = 205;
    _gDelHistoryEmPopUpPara.sub_widget_para[3].y_relative_pos = 52;
    _gDelHistoryEmPopUpPara.sub_widget_para[3].x_size = 50;
    _gDelHistoryEmPopUpPara.sub_widget_para[3].y_size = 17;
    _gDelHistoryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content = _gDelHistoryPopUpYESBuf;
    _gDelHistoryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.text_color = 0x00000000;
    _gDelHistoryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gDelHistoryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content, "Yes");//后续根据错误还是警告信息填入
    
    
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gDelHistoryPopUpTitleMem);
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
	if (event == EN_KEY_SRC_LEFT)
	{
		if(!g_ui_ctx.select)
		{
			g_ui_ctx.select = 1;
		}
		WM_Paint(gDelHistoryfaultPopUpWin.win_handle);
		return;
	}
	if (event == EN_KEY_SRC_RIGHT)
	{
		if(g_ui_ctx.select)
		{
			g_ui_ctx.select = 0;
		}
		WM_Paint(gDelHistoryfaultPopUpWin.win_handle);
		return;
	}
	if (event == EN_KEY_SRC_CONFIRM)
	{
		emWin_t *pnext = NULL;
		pnext = &gEmWinListDelHistoryFault;
		if(pCurrentEmWin->win_handle != 0)
		{
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win(); 
		}
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
				pnext = &gEmWinListDelHistoryFault;
			}
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win(); 
		}
		return;
	}
}

/**
  * @brief  绘制选中框
  * @param  NULL
  * @retval NULL
  */
static void _DrawSelPopUpRecPaint(int32_t sel)
{
	if(sel == 0)
	{
		//no
		GUI_SetColor(0x00FFFFFF);
		GUI_FillRoundedRect(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3);
		GUI_SetColor(0x00000000);
		GUI_DrawRoundedFrame(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3,1);
		//yes
		GUI_SetColor(0x00A4A4A4);
		GUI_FillRoundedRect(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3);
//		GUI_SetColor(0x00000000);
//		GUI_DrawRoundedFrame(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3,1);
		
	}
	else if(sel == 1)
	{
		//no
		GUI_SetColor(0x00A4A4A4);
		GUI_FillRoundedRect(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3);
//		GUI_SetColor(0x00000000);
//		GUI_DrawRoundedFrame(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3,1);
		//yes
		GUI_SetColor(0x00FFFFFF);
		GUI_FillRoundedRect(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3);
		GUI_SetColor(0x00000000);
		GUI_DrawRoundedFrame(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3,1);
	}
	return;          
}

  