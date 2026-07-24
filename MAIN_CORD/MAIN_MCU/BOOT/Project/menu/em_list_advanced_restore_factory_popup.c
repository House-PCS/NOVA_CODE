/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_restore_factory_popup.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-21
  * @brief       高级设置->更多设置->恢复出厂(界面中的弹窗)
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


#define  EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE                (4096)


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gRestoreFacotryPopUpWin = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;

	int32_t restore_factory;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


static emPopUpPara_t  _gRestoreFacotryEmPopUpPara;


static GUI_HMEM _gRestoreFacotryPopUpTitleMem;
static uint8_t *_pRestoreFacotryPopUpTitleBuf;
static char _gRestoreFacotryPopUpTitleBuf[60];
static char _gRestoreFacotryPopUpYESBuf[5];
static char _gRestoreFacotryPopUpNOBuf[5];


#define WIN_LIST_RESTORE_FACOTRY_FAULT_POPUP_ID                     (GUI_ID_USER + 0x40)

/*函数声明*/
static void _DrawSelPopUpRecPaint(int32_t sel);

static int32_t _BackRestoreFactory(int32_t status)
{
	emWin_t *pnext = NULL;
	pnext = &gEmWinListAdvanSettingFactoryRestore;
	if(pCurrentEmWin->win_handle != 0 && status)
	{
		pPrevEmWin = pCurrentEmWin;
		pCurrentEmWin->delete_win();
		pCurrentEmWin = pnext;
		pCurrentEmWin->create_win(); 
	}
	return 0;
}

static int32_t _SetRestoringFactoryStatus(int32_t status)
{
	int32_t tmp = g_ui_ctx.restore_factory%3000;
	if (status > 0)
	{
		static int32_t tmp1 = 0;
		if (tmp == 0)
		{
			tmp = ++tmp1%4;
			if (tmp == 0)
			{
				snprintf(_gRestoreFacotryPopUpTitleBuf, sizeof(_gRestoreFacotryPopUpTitleBuf), 
						 "%s", "Restoring Factory");
			}
			else if (tmp == 1)
			{
				snprintf(_gRestoreFacotryPopUpTitleBuf, sizeof(_gRestoreFacotryPopUpTitleBuf), 
						 "%s", "Restoring Factory .");
			}
			else if (tmp == 2)
			{
				snprintf(_gRestoreFacotryPopUpTitleBuf, sizeof(_gRestoreFacotryPopUpTitleBuf), 
						 "%s", "Restoring Factory ..");
			}
			else if (tmp == 3)
			{
				snprintf(_gRestoreFacotryPopUpTitleBuf, sizeof(_gRestoreFacotryPopUpTitleBuf), 
						 "%s", "Restoring Factory ...");
			}

			WM_HideWindow(_gRestoreFacotryEmPopUpPara.wid_handle[2]);
			WM_HideWindow(_gRestoreFacotryEmPopUpPara.wid_handle[3]);
			TEXT_SetText(_gRestoreFacotryEmPopUpPara.wid_handle[1], 
						 (const char *)_gRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content);
		}
		++g_ui_ctx.restore_factory;
	} 
	else if (status == 0)
	{
		snprintf(_gRestoreFacotryPopUpTitleBuf, sizeof(_gRestoreFacotryPopUpTitleBuf), "%s",
				 "Restore Factory complete");
		snprintf(_gRestoreFacotryPopUpYESBuf, sizeof(_gRestoreFacotryPopUpYESBuf), "%s",
				 "OK");
		WM_HideWindow(_gRestoreFacotryEmPopUpPara.wid_handle[2]);
		WM_ShowWindow(_gRestoreFacotryEmPopUpPara.wid_handle[3]);
		TEXT_SetText(_gRestoreFacotryEmPopUpPara.wid_handle[1], 
					 (const char *)_gRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content);
		TEXT_SetText(_gRestoreFacotryEmPopUpPara.wid_handle[3], 
					 (const char *)_gRestoreFacotryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content);
		g_ui_ctx.restore_factory = 0;
	}

	//WM_Paint(gRestoreFacotryPopUpWin.win_handle);

	return 0;
}

static int32_t _GetRestoreFacoty()
{
	return g_ui_ctx.restore_factory;
}

static int32_t _CheckRestoreFacoty()
{
	if (!g_ui_ctx.restore_factory)
	{
		return -1;
	}

	_SetRestoringFactoryStatus(1);
	if (g_ui_ctx.restore_factory > 50000)
	{
		_SetRestoringFactoryStatus(0);
		g_ui_ctx.select = 0;
		_BackRestoreFactory(0);
	}

	return 0;
}

/**
  * @brief 弹窗Callback
  * @param  NULL
  * @retval NULL
  */ 
static void _cbRestoreFacotryPopUpWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            EmPopUpSubWidgetCreate(&_gRestoreFacotryEmPopUpPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
			/*默认选No*/
			//NO的边框
			GUI_Clear();
			if (!_GetRestoreFacoty())
			{
				if (_gRestoreFacotryPopUpYESBuf[0] != 'O')
				{
					GUI_SetColor(0x00FFFFFF);
					GUI_FillRoundedRect(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3);
					GUI_SetColor(0x00000000);
					GUI_DrawRoundedFrame(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3,1);
				}
				//yes的边框
				GUI_SetColor(0x00A4A4A4);
				GUI_FillRoundedRect(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3);
	//			GUI_SetColor(0x00000000);
	//			GUI_DrawRoundedFrame(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3,1);
			}
			
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
GUI_HWIN EmRestoreFacotryPopUpCreate(emPopUpPara_t *p_widget_para, WM_HWIN h_win_src)
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
    
    handle = GUI_CreateDialogBox(pop_wm, GUI_COUNTOF(pop_wm), _cbRestoreFacotryPopUpWin, h_win_src, p_widget_para->x_pos, p_widget_para->y_pos);
    WINDOW_SetBkColor(handle, p_widget_para->bk_color);
    GUI_Exec();
    return handle;
}
  
  
/**
  * @brief 删除弹窗
  * @param  p_widget_para:list控件结构体
  * @retval NULL
  */   
void EmRestoreFacotryPopUpDel(emPopUpPara_t *p_widget_para)
{
    if(gRestoreFacotryPopUpWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gRestoreFacotryPopUpWin.win_handle);
    gRestoreFacotryPopUpWin.win_handle = 0;
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
	pPrevEmWin = &gEmWinListAdvanSettingFactoryRestore;
    _Init();
    gRestoreFacotryPopUpWin.win_handle = 
		EmRestoreFacotryPopUpCreate(&_gRestoreFacotryEmPopUpPara, pCurrentEmWin->win_handle);
    pCurrentPopUpSubWin = &gRestoreFacotryPopUpWin;
    return gRestoreFacotryPopUpWin.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gRestoreFacotryPopUpWin.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gRestoreFacotryPopUpWin.win_handle);
    _Del();
    gRestoreFacotryPopUpWin.win_handle = 0;
    pCurrentPopUpSubWin = NULL;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	g_ui_ctx.restore_factory = 0;
    _gRestoreFacotryEmPopUpPara.bk_color = 0x006A7105;//后续根据错误还是警告信息填入合适的颜色
    _gRestoreFacotryEmPopUpPara.x_pos = 115;
    _gRestoreFacotryEmPopUpPara.y_pos = 122;
    _gRestoreFacotryEmPopUpPara.x_size = 250;
    _gRestoreFacotryEmPopUpPara.y_size = 75;
    
    _gRestoreFacotryEmPopUpPara.popup_start_id = WIN_LIST_RESTORE_FACOTRY_FAULT_POPUP_ID;
    _gRestoreFacotryEmPopUpPara.widget_num = 4;
    
    _gRestoreFacotryPopUpTitleMem = GUI_ALLOC_AllocZero(EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE);
    _pRestoreFacotryPopUpTitleBuf = GUI_ALLOC_h2p(_gRestoreFacotryPopUpTitleMem);
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _pRestoreFacotryPopUpTitleBuf, EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE);
    /*子控件参数初始化*/
    _gRestoreFacotryEmPopUpPara.sub_widget_para[0].widget_type = EM_WIDGET_TYPE_IMG;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[0].x_relative_pos = 12;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[0].y_relative_pos = 13;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[0].x_size = 21;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[0].y_size = 21;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[0].sub_widget.img_para.mem_img = _gRestoreFacotryPopUpTitleMem;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[0].sub_widget.img_para.p_img_buf = _pRestoreFacotryPopUpTitleBuf;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[0].sub_widget.img_para.img_size = EM_RESTORE_FACOTRY_POP_UP_ICON_ALERT_SIZE;
    
    _gRestoreFacotryEmPopUpPara.sub_widget_para[1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[1].x_relative_pos = 43;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[1].y_relative_pos = 10;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[1].x_size = 192;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[1].y_size = 34;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content = _gRestoreFacotryPopUpTitleBuf;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gRestoreFacotryEmPopUpPara.sub_widget_para[1].sub_widget.text_para.p_content, "Note that restoring the factory will delete all current configurations !");//后续根据错误还是警告信息填入
    
    _gRestoreFacotryEmPopUpPara.sub_widget_para[2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[2].x_relative_pos = 149;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[2].y_relative_pos = 52;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[2].x_size = 50;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[2].y_size = 17;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_content = _gRestoreFacotryPopUpNOBuf;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.text_color = 0x00000000;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gRestoreFacotryEmPopUpPara.sub_widget_para[2].sub_widget.text_para.p_content, "No");//后续根据错误还是警告信息填入
    
    _gRestoreFacotryEmPopUpPara.sub_widget_para[3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[3].x_relative_pos = 205;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[3].y_relative_pos = 52;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[3].x_size = 50;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[3].y_size = 17;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content = _gRestoreFacotryPopUpYESBuf;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.text_color = 0x00000000;
    _gRestoreFacotryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
    strcpy(_gRestoreFacotryEmPopUpPara.sub_widget_para[3].sub_widget.text_para.p_content, "Yes");//后续根据错误还是警告信息填入
    
    
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gRestoreFacotryPopUpTitleMem);
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    _CheckRestoreFacoty();
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
		WM_Paint(gRestoreFacotryPopUpWin.win_handle);
		return;
	}
	if (event == EN_KEY_SRC_RIGHT)
	{
		if(g_ui_ctx.select)
		{
			g_ui_ctx.select = 0;
		}
		WM_Paint(gRestoreFacotryPopUpWin.win_handle);
		return;
	}
	if (event == EN_KEY_SRC_CONFIRM)
	{
		if (g_ui_ctx.select)
		{
			_SetRestoringFactoryStatus(1);
			return ;
		}
		_BackRestoreFactory(1);
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
				pnext = &gEmWinListAdvanSettingFactoryRestore;
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
	if (!_GetRestoreFacoty())
	{
		if(sel == 0)
		{
			//no
			if (_gRestoreFacotryPopUpYESBuf[0] != 'O')
			{
				GUI_SetColor(0x00FFFFFF);
				GUI_FillRoundedRect(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3);
				GUI_SetColor(0x00000000);
				GUI_DrawRoundedFrame(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3,1);
			}
			//yes
			GUI_SetColor(0x00A4A4A4);
			GUI_FillRoundedRect(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3);
	//		GUI_SetColor(0x00000000);
	//		GUI_DrawRoundedFrame(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3,1);
			
		}
		else if(sel == 1)
		{
			//no
			if (_gRestoreFacotryPopUpYESBuf[0] != 'O')
			{
			GUI_SetColor(0x00A4A4A4);
			GUI_FillRoundedRect(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3);
	//		GUI_SetColor(0x00000000);
	//		GUI_DrawRoundedFrame(130, 50,  130 + 54 - 1,  50 + 18 - 1, 3,1);
			}
			//yes
			GUI_SetColor(0x00FFFFFF);
			GUI_FillRoundedRect(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3);
			GUI_SetColor(0x00000000);
			GUI_DrawRoundedFrame(190, 50,  188 + 54 - 1,  50 + 18 - 1, 3,1);
		}
	}
	return;          
}

  
