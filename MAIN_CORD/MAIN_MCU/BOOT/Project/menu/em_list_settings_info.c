/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_LIST_SETTINGS_INFO.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-01-15
  * @brief       Settings界面
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

#define  EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE                (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListSettingsInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};



/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;

static char _gTitleBuf[] = "Settings";

/*List控件*/
static emListPara_t  _gEmListSettingsInfoPara;

static char _gListSettingsInfoLeftTextBuf[16][20];
static char _gListSettingsInfoRightText3Buf[8];
static GUI_HMEM _gListSettingsInfoRightMem;
static uint8_t *_pListSettingsInfoRightBuf;

#define WIN_LIST_SETTINGS_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_SETTINGS_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static const GUI_WIDGET_CREATE_INFO _aListSettingsInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListSettingsInfoWin",    WIN_LIST_SETTINGS_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_SETTINGS_INFO_TITLE_ID,     31,  20, 144,  14,   0,   0x0,  0 },
};



/**
  * @brief ListSettingsInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListSettingsInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SETTINGS_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListSettingsInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListSettingsInfoPara, _gEmListSettingsInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(int i = 0; i < _gEmListSettingsInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListSettingsInfoPara.sub_region[i].x_pos, 
                                    _gEmListSettingsInfoPara.sub_region[i].y_pos, 
                                    _gEmListSettingsInfoPara.sub_region[i].x_pos + _gEmListSettingsInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListSettingsInfoPara.sub_region[i].y_pos + _gEmListSettingsInfoPara.sub_region[i].y_size - 1,
                                    _gEmListSettingsInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListSettingsInfoPara, _gEmListSettingsInfoPara.list_sel);
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
    gEmWinListSettingsInfo.win_handle = GUI_CreateDialogBox(_aListSettingsInfoDialogCreate, GUI_COUNTOF(_aListSettingsInfoDialogCreate), _cbListSettingsInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListSettingsInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListSettingsInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListSettingsInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListSettingsInfoPara, gEmWinListSettingsInfo.win_handle);
    return gEmWinListSettingsInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListSettingsInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListSettingsInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListSettingsInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	EmKeyDefaultInfoSet();
    uint8_t max_page;
    
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListSettingsInfoPara.list_start_id = WIN_LIST_SETTINGS_INFO_ID + 0x30;
    _gEmListSettingsInfoPara.list_num = 2;
    _gEmListSettingsInfoPara.list_display_num = 6;
    if(_gEmListSettingsInfoPara.list_num <= _gEmListSettingsInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListSettingsInfoPara.list_num + 1 - _gEmListSettingsInfoPara.list_display_num;
    }
    
    if(_gEmListSettingsInfoPara.list_sel > _gEmListSettingsInfoPara.list_num - 1)
    {
        _gEmListSettingsInfoPara.list_sel = 0;
    }
    if(_gEmListSettingsInfoPara.list_page > max_page - 1)
    {
        _gEmListSettingsInfoPara.list_page = 0;
    }
    _gEmListSettingsInfoPara.list_bk_color = 0x00787878;
    /*2个子列表*/
    _gEmListSettingsInfoPara.sub_region[0].x_pos = 20;
    _gEmListSettingsInfoPara.sub_region[0].y_pos = 38;
    _gEmListSettingsInfoPara.sub_region[0].x_size = 440;
    _gEmListSettingsInfoPara.sub_region[0].y_size = 34;
    _gEmListSettingsInfoPara.sub_region[1].x_pos = 20;
    _gEmListSettingsInfoPara.sub_region[1].y_pos = 80;
    _gEmListSettingsInfoPara.sub_region[1].x_size = 440;
    _gEmListSettingsInfoPara.sub_region[1].y_size = 34;

    /*List控件*/
    _gListSettingsInfoRightMem = GUI_ALLOC_AllocZero(EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE);
    _pListSettingsInfoRightBuf = GUI_ALLOC_h2p(_gListSettingsInfoRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListSettingsInfoRightBuf, EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE);
    //第一行左边
    _gEmListSettingsInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSettingsInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListSettingsInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListSettingsInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListSettingsInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListSettingsInfoLeftTextBuf[0];
    _gEmListSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Working Mode");
    //第一行右边
    _gEmListSettingsInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListSettingsInfoPara.sub_widget_para[1][0].x_relative_pos = 415;
    _gEmListSettingsInfoPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gEmListSettingsInfoPara.sub_widget_para[1][0].x_size = 10;
    _gEmListSettingsInfoPara.sub_widget_para[1][0].y_size = 5;
    _gEmListSettingsInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gListSettingsInfoRightMem;
    _gEmListSettingsInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _pListSettingsInfoRightBuf;
    _gEmListSettingsInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE;
    //第二行左边
    _gEmListSettingsInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSettingsInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListSettingsInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListSettingsInfoPara.sub_widget_para[0][1].x_size = 158;
    _gEmListSettingsInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListSettingsInfoLeftTextBuf[1];
    _gEmListSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "More Setting");
    //第二行右边
    _gEmListSettingsInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListSettingsInfoPara.sub_widget_para[1][1].x_relative_pos = 415;
    _gEmListSettingsInfoPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gEmListSettingsInfoPara.sub_widget_para[1][1].x_size = 10;
    _gEmListSettingsInfoPara.sub_widget_para[1][1].y_size = 5;
    _gEmListSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListSettingsInfoRightMem;
    _gEmListSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListSettingsInfoRightBuf;
    _gEmListSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_SETTINGS_INFO_ICON_ARROW_SIZE;

    
    /*List选中框*/
    _gEmListSettingsInfoPara.selected_unvalid_color = 0xFF000000;
    _gEmListSettingsInfoPara.selected_valid_color = 0x00D6E400;
    _gEmListSettingsInfoPara.selected_size = 3;
    _gEmListSettingsInfoPara.selected_r = 10;
    _gEmListSettingsInfoPara.selected_x_start_offset = -2;
    _gEmListSettingsInfoPara.selected_y_start_offset = -2;
    _gEmListSettingsInfoPara.selected_x_end_offset = 2;
    _gEmListSettingsInfoPara.selected_y_end_offset = 2;
//    _gEmListSettingsInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gEmListSettingsInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
    /*滚动条*/
//    _gEmListSettingsInfoPara.scrollbar_enable = true;
//    _gEmListSettingsInfoPara.scrollbar_color = 0x00D6E400;
//    _gEmListSettingsInfoPara.scrollbar_bk_color = 0x00565656;
//    _gEmListSettingsInfoPara.scrollbar_x_pos = 454;
//    _gEmListSettingsInfoPara.scrollbar_y_pos = 39;
//    _gEmListSettingsInfoPara.scrollbar_x_size = 6;
//    _gEmListSettingsInfoPara.scrollbar_y_size = 241;
    
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gListSettingsInfoRightMem);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    static uint32_t timecount = 0;
    static uint8_t dir = 0;
    static uint8_t pop_up_stat = 0;
    static uint32_t pop_timecount = 0;
    if(gEmWinListSettingsInfo.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gEmListSettingsInfoPara.list_sel + _gEmListSettingsInfoPara.list_page >= _gEmListSettingsInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListSettingsInfoPara.list_sel + _gEmListSettingsInfoPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListSettingsInfoPara, gEmWinListSettingsInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListSettingsInfoPara, gEmWinListSettingsInfo.win_handle);
        }
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
	if (event < 0 || event > EN_KEY_SRC_NUM)
	{
		return;
	}

	if (event == EN_KEY_SRC_LEFT)
	{
		--g_ui_ctx.select;
		if (g_ui_ctx.select < 0)
		{
			++g_ui_ctx.select;
			return;
		}
		EmListUpward(&_gEmListSettingsInfoPara, gEmWinListSettingsInfo.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		++g_ui_ctx.select;
		if (g_ui_ctx.select >= _gEmListSettingsInfoPara.list_num)
		{
			--g_ui_ctx.select;
			return;
		}
		EmListDownward(&_gEmListSettingsInfoPara, gEmWinListSettingsInfo.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		emWin_t *pnext = NULL;
		switch(g_ui_ctx.select)
		{
			case 0:
				pnext = &gEmWinListWorkingModeInfo;
				break;
			case 1:
				pnext = &gEmWinListADVSetPassword;
				break;
			default:
				return;
		}
		if(pnext != NULL && pCurrentEmWin->win_handle != 0)
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
				pnext = &gEmWinMain3DNav;
			}
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win();  
		}
		return;
	}
}


