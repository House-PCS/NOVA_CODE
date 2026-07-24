/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_communication_baud_rates.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-02-22
  * @brief       高级设置-通信参数-波特率
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-02-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListAdvanSettingCommunicateBaudRate = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;

static char _gTitleBuf[] = " ";


/*List控件*/
static emListPara_t  _gEmListLanguageTypePara;

static char _gListLanguageTypeInfoLeftTextBuf[16][20];


#define WIN_LIST_LANGUAGE_TYPE_ID								(GUI_ID_USER + 0x10)
#define WIN_LIST_LANGUAGE_TYPE_TITLE_ID                         (GUI_ID_USER + 0x11)


static const GUI_WIDGET_CREATE_INFO _aListLanguageTypeDialogCreate[] = 
{
	{ WINDOW_CreateIndirect,  " ",   			 WIN_LIST_LANGUAGE_TYPE_ID,    	 	 			334,    122,     128,    148,    0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Title",        	 WIN_LIST_LANGUAGE_TYPE_TITLE_ID,      			334,    122,     15,     15,     0,   0x0,  0 },
};

/*函数声明*/
extern void SetCommunBuadRateArrowDirection(void);
extern int32_t SetCurCommuBaudRate(int32_t idx);
extern void SetCommuBaudRateListShow(int32_t show);

/**
  * @brief  ListLanguageTypeInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListLanguageTypeInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_LANGUAGE_TYPE_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
            /*LIST*/
            EmListWidgetCreate(&_gEmListLanguageTypePara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListLanguageTypePara, _gEmListLanguageTypePara.list_pre_sel);

			GUI_RECT Rect;
			WM_GetClientRect(&Rect);
			GUI_SetColor(0x00424242);
			GUI_FillRoundedRect(Rect.x0, Rect.y0, Rect.x1, Rect.y1, 10);
			GUI_SetColor(0x00BDC909);
			GUI_DrawRoundedFrame(Rect.x0 + 1, Rect.y0 + 1, Rect.x1 - 1, Rect.y1 - 1,10,3);
			
			#if 0 
            GUI_SetColor(0x00565656);//上色
			
            for(int i = 0; i < _gEmListLanguageTypePara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListLanguageTypePara.sub_region[i].x_pos, 
                                    _gEmListLanguageTypePara.sub_region[i].y_pos, 
                                    _gEmListLanguageTypePara.sub_region[i].x_pos + _gEmListLanguageTypePara.sub_region[i].x_size - 1, 
                                    _gEmListLanguageTypePara.sub_region[i].y_pos + _gEmListLanguageTypePara.sub_region[i].y_size - 1,
                                    _gEmListLanguageTypePara.selected_r);
            }
			#endif   /*此段用于调节位置时使用*/
			
            SelectedDrawInWMPaint(&_gEmListLanguageTypePara, _gEmListLanguageTypePara.list_sel);
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
    GUI_EnableAlpha(1);
	#if 1
    _gEmListLanguageTypePara.list_x_offset = _aListLanguageTypeDialogCreate[0].x0;
    _gEmListLanguageTypePara.list_y_offset = _aListLanguageTypeDialogCreate[0].y0;
#endif 
    gEmWinListAdvanSettingCommunicateBaudRate.win_handle = GUI_CreateDialogBox(_aListLanguageTypeDialogCreate, GUI_COUNTOF(_aListLanguageTypeDialogCreate), _cbListLanguageTypeInfoWin, 
	pCurrentEmWin->win_handle, 0, 0);
    //WINDOW_SetBkColor(gEmWinListAdvanSettingCommunicateBaudRate.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListAdvanSettingCommunicateBaudRate.win_handle, 0x00000000);
    WM_SetTransState(gEmWinListAdvanSettingCommunicateBaudRate.win_handle, WM_CF_HASTRANS);
    SelectedDraw(&_gEmListLanguageTypePara, gEmWinListAdvanSettingCommunicateBaudRate.win_handle);
    GUI_Exec();
    return gEmWinListAdvanSettingCommunicateBaudRate.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListAdvanSettingCommunicateBaudRate.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListAdvanSettingCommunicateBaudRate.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListAdvanSettingCommunicateBaudRate.win_handle = 0;
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
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_1;

	
	_gEmListLanguageTypePara.list_start_id = WIN_LIST_LANGUAGE_TYPE_ID + 0x30;
    _gEmListLanguageTypePara.list_num = 7;
    _gEmListLanguageTypePara.list_display_num = 4;
    if(_gEmListLanguageTypePara.list_num <= _gEmListLanguageTypePara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListLanguageTypePara.list_num + 1 - _gEmListLanguageTypePara.list_display_num;
    }
    
    if(_gEmListLanguageTypePara.list_sel > _gEmListLanguageTypePara.list_num - 1)
    {
        _gEmListLanguageTypePara.list_sel = 0;
    }
    if(_gEmListLanguageTypePara.list_page > max_page - 1)
    {
        _gEmListLanguageTypePara.list_page = 0;
    }
	
	
    //_gEmListLanguageTypePara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gEmListLanguageTypePara.sub_region[0].x_pos = 12;
    _gEmListLanguageTypePara.sub_region[0].y_pos = 12;
    _gEmListLanguageTypePara.sub_region[0].x_size = 102;
    _gEmListLanguageTypePara.sub_region[0].y_size = 25;
    _gEmListLanguageTypePara.sub_region[1].x_pos = 12;
    _gEmListLanguageTypePara.sub_region[1].y_pos = 44;
    _gEmListLanguageTypePara.sub_region[1].x_size = 102;
    _gEmListLanguageTypePara.sub_region[1].y_size = 25;
    _gEmListLanguageTypePara.sub_region[2].x_pos = 12;
    _gEmListLanguageTypePara.sub_region[2].y_pos = 76;
    _gEmListLanguageTypePara.sub_region[2].x_size = 102;
    _gEmListLanguageTypePara.sub_region[2].y_size = 25;
    _gEmListLanguageTypePara.sub_region[3].x_pos = 12;
    _gEmListLanguageTypePara.sub_region[3].y_pos = 108;
    _gEmListLanguageTypePara.sub_region[3].x_size = 102;
    _gEmListLanguageTypePara.sub_region[3].y_size = 25;
    _gEmListLanguageTypePara.sub_region[4].x_pos = 12;
    _gEmListLanguageTypePara.sub_region[4].y_pos = 140;
    _gEmListLanguageTypePara.sub_region[4].x_size = 102;
    _gEmListLanguageTypePara.sub_region[4].y_size = 25;
    _gEmListLanguageTypePara.sub_region[5].x_pos = 12;
    _gEmListLanguageTypePara.sub_region[5].y_pos = 172;
    _gEmListLanguageTypePara.sub_region[5].x_size = 102;
    _gEmListLanguageTypePara.sub_region[5].y_size = 25;
    /*List控件*/

    //第1行左边
    _gEmListLanguageTypePara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListLanguageTypePara.sub_widget_para[0][0].x_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][0].y_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][0].x_size = 74;
    _gEmListLanguageTypePara.sub_widget_para[0][0].y_size = 17;
    _gEmListLanguageTypePara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListLanguageTypeInfoLeftTextBuf[0];
    _gEmListLanguageTypePara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListLanguageTypePara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListLanguageTypePara.sub_widget_para[0][0].sub_widget.text_para.p_content, "9600bps");
    //第2行左边
    _gEmListLanguageTypePara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListLanguageTypePara.sub_widget_para[0][1].x_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][1].y_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][1].x_size = 74;
    _gEmListLanguageTypePara.sub_widget_para[0][1].y_size = 17;
    _gEmListLanguageTypePara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListLanguageTypeInfoLeftTextBuf[1];
    _gEmListLanguageTypePara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListLanguageTypePara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListLanguageTypePara.sub_widget_para[0][1].sub_widget.text_para.p_content, "19200bps");
    //第3行左边
    _gEmListLanguageTypePara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListLanguageTypePara.sub_widget_para[0][2].x_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][2].y_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][2].x_size = 100;
    _gEmListLanguageTypePara.sub_widget_para[0][2].y_size = 17;
    _gEmListLanguageTypePara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListLanguageTypeInfoLeftTextBuf[2];
    _gEmListLanguageTypePara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListLanguageTypePara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListLanguageTypePara.sub_widget_para[0][2].sub_widget.text_para.p_content, "38400bps");
    //第4行左边
    _gEmListLanguageTypePara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListLanguageTypePara.sub_widget_para[0][3].x_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][3].y_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][3].x_size = 74;
    _gEmListLanguageTypePara.sub_widget_para[0][3].y_size = 17;
    _gEmListLanguageTypePara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListLanguageTypeInfoLeftTextBuf[3];
    _gEmListLanguageTypePara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListLanguageTypePara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListLanguageTypePara.sub_widget_para[0][3].sub_widget.text_para.p_content, "57600bps");
    //第5行左边
    _gEmListLanguageTypePara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListLanguageTypePara.sub_widget_para[0][4].x_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][4].y_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][4].x_size = 74;
    _gEmListLanguageTypePara.sub_widget_para[0][4].y_size = 17;
    _gEmListLanguageTypePara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListLanguageTypeInfoLeftTextBuf[4];
    _gEmListLanguageTypePara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListLanguageTypePara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListLanguageTypePara.sub_widget_para[0][4].sub_widget.text_para.p_content, "115200bps");
    //第6行左边
    _gEmListLanguageTypePara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListLanguageTypePara.sub_widget_para[0][5].x_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][5].y_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][5].x_size = 74;
    _gEmListLanguageTypePara.sub_widget_para[0][5].y_size = 17;
    _gEmListLanguageTypePara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListLanguageTypeInfoLeftTextBuf[5];
    _gEmListLanguageTypePara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListLanguageTypePara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListLanguageTypePara.sub_widget_para[0][5].sub_widget.text_para.p_content, "256000bps");
    //第7行左边
    _gEmListLanguageTypePara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListLanguageTypePara.sub_widget_para[0][6].x_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][6].y_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][6].x_size = 74;
    _gEmListLanguageTypePara.sub_widget_para[0][6].y_size = 17;
    _gEmListLanguageTypePara.sub_widget_para[0][6].sub_widget.text_para.p_content = _gListLanguageTypeInfoLeftTextBuf[6];
    _gEmListLanguageTypePara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListLanguageTypePara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListLanguageTypePara.sub_widget_para[0][6].sub_widget.text_para.p_content, "460800bps");
#if 0
    //第8行左边
    _gEmListLanguageTypePara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListLanguageTypePara.sub_widget_para[0][7].x_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][7].y_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][7].x_size = 74;
    _gEmListLanguageTypePara.sub_widget_para[0][7].y_size = 17;
    _gEmListLanguageTypePara.sub_widget_para[0][7].sub_widget.text_para.p_content = _gListLanguageTypeInfoLeftTextBuf[7];
    _gEmListLanguageTypePara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListLanguageTypePara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListLanguageTypePara.sub_widget_para[0][7].sub_widget.text_para.p_content, "256000bps");
	//第9行左边
    _gEmListLanguageTypePara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListLanguageTypePara.sub_widget_para[0][8].x_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][8].y_relative_pos = 6;
    _gEmListLanguageTypePara.sub_widget_para[0][8].x_size = 74;
    _gEmListLanguageTypePara.sub_widget_para[0][8].y_size = 17;
    _gEmListLanguageTypePara.sub_widget_para[0][8].sub_widget.text_para.p_content = _gListLanguageTypeInfoLeftTextBuf[8];
    _gEmListLanguageTypePara.sub_widget_para[0][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListLanguageTypePara.sub_widget_para[0][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListLanguageTypePara.sub_widget_para[0][8].sub_widget.text_para.p_content, "460800bps");
#endif


    /*List选中框*/
    _gEmListLanguageTypePara.selected_unvalid_color = 0xFF000000;
    _gEmListLanguageTypePara.selected_valid_color = 0x00D6E400;
    _gEmListLanguageTypePara.selected_size = 3;
    _gEmListLanguageTypePara.selected_r = 5;
    _gEmListLanguageTypePara.selected_x_start_offset = - 2;
    _gEmListLanguageTypePara.selected_y_start_offset = -2;
    _gEmListLanguageTypePara.selected_x_end_offset = 2;
    _gEmListLanguageTypePara.selected_y_end_offset = 2;
    _gEmListLanguageTypePara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    //_gEmListLanguageTypePara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
//    /*滚动条*/
//    _gEmListLanguageTypePara.scrollbar_enable = true;
//    _gEmListLanguageTypePara.scrollbar_color = 0x00D6E400;
//    _gEmListLanguageTypePara.scrollbar_bk_color = 0x00565656;
//    _gEmListLanguageTypePara.scrollbar_x_pos = 454;
//    _gEmListLanguageTypePara.scrollbar_y_pos = 39;
//    _gEmListLanguageTypePara.scrollbar_x_size = 6;
//    _gEmListLanguageTypePara.scrollbar_y_size = 241;


}


/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{

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
    if(gEmWinListAdvanSettingCommunicateBaudRate.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gEmListLanguageTypePara.list_sel + _gEmListLanguageTypePara.list_page >= _gEmListLanguageTypePara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListLanguageTypePara.list_sel + _gEmListLanguageTypePara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListLanguageTypePara, gEmWinListAdvanSettingCommunicateBaudRate.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListLanguageTypePara, gEmWinListAdvanSettingCommunicateBaudRate.win_handle);
        }
    }
    
//    if(pop_up_stat == 0)
//    {
//        if(GetSystick() - pop_timecount > 1000)
//        {
//            pop_up_stat = 1;
//            pop_timecount = GetSystick();
//            gPopUpWinError.create_win();
//        }
//    }
//    else
//    {
//        if(GetSystick() - pop_timecount > 1000)
//        {
//            pop_up_stat = 0;
//            pop_timecount = GetSystick();
//            gPopUpWinError.delete_win();
//        }
//    }
    
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
		EmListUpward(&_gEmListLanguageTypePara, gEmWinListAdvanSettingCommunicateBaudRate.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		++g_ui_ctx.select;
		if (g_ui_ctx.select >= _gEmListLanguageTypePara.list_num)
		{
			--g_ui_ctx.select;
			return;
		}
		EmListDownward(&_gEmListLanguageTypePara, gEmWinListAdvanSettingCommunicateBaudRate.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		SetCurCommuBaudRate(g_ui_ctx.select);
		
		SetCommuBaudRateListShow(0);
		SetCommunBuadRateArrowDirection();
		return;
	}
	else if (event == EN_KEY_SRC_ESC)
	{
		SetCommuBaudRateListShow(0);
		SetCommunBuadRateArrowDirection();
		return;
	}
}


