/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_work_mode_info.c
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
  * <tr><td>2023-01-15    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define	 TEST_DYNAMIC										  0
#define  EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE                (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListWorkingModeInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;

	uint32_t switcher[3];
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


typedef enum _SWITCH_STAT_E
{
	SWITCH_STAT_OFF = 0,
	SWITCH_STAT_ON,
} SWITCH_STAT_E;

/*函数声明*/
int32_t _SetSwitchStat(uint8_t row,SWITCH_STAT_E stat);

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;
static IMAGE_Handle  _gImgSwithHandle;//开关图标句柄 

static char _gTitleBuf[] = "Settings < Working Mode";

/*List控件*/
static emListPara_t  _gEmListWorkingModeInfoPara;

static char _gListWorkingModeInfoLeftTextBuf[16][20];

/*开关图片参数*/
static emWidgetImgPara_t  _gSwtichImgPara[2];  /*0:关	1:开*/


#define WIN_LIST_WORKMODE_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_WORKMODE_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)
#define WIN_LIST_SWITCH_ON_IMG_ID								(GUI_ID_USER + 0x12)
#define WIN_LIST_SWITCH_OFF_IMG_ID								(GUI_ID_USER + 0x13)

static const GUI_WIDGET_CREATE_INFO _aListWorkingModeInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListWorkingModeInfoWin",    WIN_LIST_WORKMODE_INFO_ID,     		0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         		WIN_LIST_WORKMODE_INFO_TITLE_ID,     		31,  20, 400,  14,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "SWITON",				WIN_LIST_SWITCH_ON_IMG_ID,       			400, 38,  40,  40,   0,   0x0,  0 }, 
	{ IMAGE_CreateIndirect,   "SWITCHOFF",			WIN_LIST_SWITCH_OFF_IMG_ID,       			400, 38,  40,  40,   0,   0x0,  0 }
};

static int32_t _SetSwitchStat(uint8_t row,SWITCH_STAT_E stat)
{

	emWidgetImgPara_t  *tmp_switch_img_para = &_gSwtichImgPara[0];
	switch (stat)
	{
		case SWITCH_STAT_OFF:
			{
				tmp_switch_img_para = &_gSwtichImgPara[0];
				break;
			}
		case SWITCH_STAT_ON:
			{
				tmp_switch_img_para = &_gSwtichImgPara[1];
				break;
			}
		default:break;
	}

	_gEmListWorkingModeInfoPara.sub_widget_para[1][row].sub_widget.img_para.mem_img = tmp_switch_img_para->mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][row].sub_widget.img_para.p_img_buf = tmp_switch_img_para->p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][row].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
	IMAGE_SetDTA(_gEmListWorkingModeInfoPara.handle[1][row], tmp_switch_img_para->p_img_buf, 
				 tmp_switch_img_para->img_size); 
	return 0;
}

static int32_t _UpdateSwitch(int32_t select)
{
	if (select < 0 || select > sizeof(g_ui_ctx.switcher)/sizeof(g_ui_ctx.switcher[0]))
	{
		return -1;
	}

	g_ui_ctx.select = select;
	for (int32_t i = 0; i < sizeof(g_ui_ctx.switcher)/sizeof(g_ui_ctx.switcher[0]); ++i)
	{
		g_ui_ctx.switcher[i] = 0;
		if (g_ui_ctx.select == i && i != 2)
		{
			g_ui_ctx.switcher[i] = (++g_ui_ctx.switcher[i])%2;
		}
		_SetSwitchStat((uint8_t)i, (SWITCH_STAT_E)g_ui_ctx.switcher[i]);
	}
    
    return 0;
}


/**
  * @brief ListWorkingModeInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListWorkingModeInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
	static int     switch_type = 0,row =0;

#if TEST_DYNAMIC
	static WM_HTIMER wm_timer_handle_switch;
#endif 
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
#if TEST_DYNAMIC
			wm_timer_handle_switch = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                             0, 	     /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                             1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                             0);		 /* 留待将来使用，应为0 */ 
#endif 
			
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_WORKMODE_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListWorkingModeInfoPara, pMsg->hWin);
            break;
        }
		case WM_DELETE:
		{
#if TEST_DYNAMIC
            WM_DeleteTimer(wm_timer_handle_switch);//；删除窗口，会自动删除对应内部的定时器
#endif 
			break;
		}
		case WM_TIMER:
        {
#if TEST_DYNAMIC
			++switch_type;
			++row;
			_SetSwitchStat(row%3,(SWITCH_STAT_E)(switch_type%2));

            WM_RestartTimer(pMsg->Data.v, 300);/*暂定300mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif 
			break;
		}
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListWorkingModeInfoPara, _gEmListWorkingModeInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(int i = 0; i < _gEmListWorkingModeInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListWorkingModeInfoPara.sub_region[i].x_pos, 
                                    _gEmListWorkingModeInfoPara.sub_region[i].y_pos, 
                                    _gEmListWorkingModeInfoPara.sub_region[i].x_pos + _gEmListWorkingModeInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListWorkingModeInfoPara.sub_region[i].y_pos + _gEmListWorkingModeInfoPara.sub_region[i].y_size - 1,
                                    _gEmListWorkingModeInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListWorkingModeInfoPara, _gEmListWorkingModeInfoPara.list_sel);

			for (int32_t i = 0; i < sizeof(g_ui_ctx.switcher)/sizeof(g_ui_ctx.switcher[0]); ++i)
			{
				_SetSwitchStat((uint8_t)i, (SWITCH_STAT_E)g_ui_ctx.switcher[i]);
			}
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
	pPrevEmWin = &gEmWinListSettingsInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListWorkingModeInfo.win_handle = GUI_CreateDialogBox(_aListWorkingModeInfoDialogCreate, GUI_COUNTOF(_aListWorkingModeInfoDialogCreate), _cbListWorkingModeInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListWorkingModeInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListWorkingModeInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListWorkingModeInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListWorkingModeInfoPara, gEmWinListWorkingModeInfo.win_handle);
    return gEmWinListWorkingModeInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListWorkingModeInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListWorkingModeInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListWorkingModeInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	EmKeyDefaultInfoSet();
	int32_t i = 0, j = sizeof(g_ui_ctx.switcher)/sizeof(g_ui_ctx.switcher[0]);
	for (i = 0; i < j; ++i)
	{
		if (g_ui_ctx.switcher[i])
		{
			break;
		}
	}
	if (i == j)
	{
		g_ui_ctx.switcher[0] = 1;
	}

    uint8_t max_page;
    
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;	
	
	_gSwtichImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE);
    _gSwtichImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gSwtichImgPara[0].mem_img);
	_gSwtichImgPara[0].img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
	SpiFlashReadData(EM_ICON_SET_CLOSE_ADDR, _gSwtichImgPara[0].p_img_buf, _gSwtichImgPara[0].img_size);
	
	_gSwtichImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE);
    _gSwtichImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gSwtichImgPara[1].mem_img);
	_gSwtichImgPara[1].img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
	SpiFlashReadData(EM_ICON_SET_OPEN_ADDR, _gSwtichImgPara[1].p_img_buf, _gSwtichImgPara[1].img_size);
	
	
    _gEmListWorkingModeInfoPara.list_start_id = WIN_LIST_WORKMODE_INFO_ID + 0x30;
    _gEmListWorkingModeInfoPara.list_num = 3;
    _gEmListWorkingModeInfoPara.list_display_num = 6;
    if(_gEmListWorkingModeInfoPara.list_num <= _gEmListWorkingModeInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListWorkingModeInfoPara.list_num + 1 - _gEmListWorkingModeInfoPara.list_display_num;
    }
    
    if(_gEmListWorkingModeInfoPara.list_sel > _gEmListWorkingModeInfoPara.list_num - 1)
    {
        _gEmListWorkingModeInfoPara.list_sel = 0;
    }
    if(_gEmListWorkingModeInfoPara.list_page > max_page - 1)
    {
        _gEmListWorkingModeInfoPara.list_page = 0;
    }
    _gEmListWorkingModeInfoPara.list_bk_color = 0x00787878;
    /*2个子列表*/
    _gEmListWorkingModeInfoPara.sub_region[0].x_pos = 20;
    _gEmListWorkingModeInfoPara.sub_region[0].y_pos = 38;
    _gEmListWorkingModeInfoPara.sub_region[0].x_size = 440;
    _gEmListWorkingModeInfoPara.sub_region[0].y_size = 34;
    _gEmListWorkingModeInfoPara.sub_region[1].x_pos = 20;
    _gEmListWorkingModeInfoPara.sub_region[1].y_pos = 80;
    _gEmListWorkingModeInfoPara.sub_region[1].x_size = 440;
    _gEmListWorkingModeInfoPara.sub_region[1].y_size = 34;
	_gEmListWorkingModeInfoPara.sub_region[2].x_pos = 20;
    _gEmListWorkingModeInfoPara.sub_region[2].y_pos = 122;
    _gEmListWorkingModeInfoPara.sub_region[2].x_size = 440;
    _gEmListWorkingModeInfoPara.sub_region[2].y_size = 34;

    /*List控件*/
    //第一行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].x_size = 200;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[0];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Self-consumption");
    //第一行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
    //第二行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].x_size = 200;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[1];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Battery-priority");
    //第二行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gSwtichImgPara[1].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _gSwtichImgPara[1].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
	//第三行左边
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].x_size = 200;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListWorkingModeInfoLeftTextBuf[2];
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Peak Load Banlance");
    //第三行右边
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].x_relative_pos = 380;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].y_relative_pos = 9;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].x_size = 40;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].y_size = 31;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListWorkingModeInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;

    
    /*List选中框*/
    _gEmListWorkingModeInfoPara.selected_unvalid_color = 0xFF000000;
    _gEmListWorkingModeInfoPara.selected_valid_color = 0x00D6E400;
    _gEmListWorkingModeInfoPara.selected_size = 3;
    _gEmListWorkingModeInfoPara.selected_r = 10;
    _gEmListWorkingModeInfoPara.selected_x_start_offset = -2;
    _gEmListWorkingModeInfoPara.selected_y_start_offset = -2;
    _gEmListWorkingModeInfoPara.selected_x_end_offset = 2;
    _gEmListWorkingModeInfoPara.selected_y_end_offset = 2;
//    _gEmListWorkingModeInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gEmListWorkingModeInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
    /*滚动条*/
//    _gEmListWorkingModeInfoPara.scrollbar_enable = true;
//    _gEmListWorkingModeInfoPara.scrollbar_color = 0x00D6E400;
//    _gEmListWorkingModeInfoPara.scrollbar_bk_color = 0x00565656;
//    _gEmListWorkingModeInfoPara.scrollbar_x_pos = 454;
//    _gEmListWorkingModeInfoPara.scrollbar_y_pos = 39;
//    _gEmListWorkingModeInfoPara.scrollbar_x_size = 6;
//    _gEmListWorkingModeInfoPara.scrollbar_y_size = 241;
    
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
	GUI_ALLOC_Free(_gSwtichImgPara[0].mem_img);
	GUI_ALLOC_Free(_gSwtichImgPara[1].mem_img);
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
    if(gEmWinListWorkingModeInfo.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gEmListWorkingModeInfoPara.list_sel + _gEmListWorkingModeInfoPara.list_page >= _gEmListWorkingModeInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListWorkingModeInfoPara.list_sel + _gEmListWorkingModeInfoPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListWorkingModeInfoPara, gEmWinListWorkingModeInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListWorkingModeInfoPara, gEmWinListWorkingModeInfo.win_handle);
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
		EmListUpward(&_gEmListWorkingModeInfoPara, gEmWinListWorkingModeInfo.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		++g_ui_ctx.select;
		if (g_ui_ctx.select >= _gEmListWorkingModeInfoPara.list_num)
		{
			--g_ui_ctx.select;
			return;
		}
		EmListDownward(&_gEmListWorkingModeInfoPara, gEmWinListWorkingModeInfo.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		_UpdateSwitch(g_ui_ctx.select);
		if (g_ui_ctx.select == 2) 
		{ 
			// 肖峰填谷
			if(pCurrentEmWin->win_handle != 0)
			{
				pPrevEmWin = pCurrentEmWin;
				pCurrentEmWin->delete_win();
				pCurrentEmWin = &gEmWinListSavedSetTimeInfo;
				pCurrentEmWin->create_win(); 
			}
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
				pnext = &gEmWinListSettingsInfo;
			}
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win(); 			 
		}
		return;
	}
    
}


int32_t SetPeakLoadBalacne(uint32_t on)
{
	for (int32_t i = 0; i < sizeof(g_ui_ctx.switcher)/sizeof(g_ui_ctx.switcher[0]); ++i)
	{
		g_ui_ctx.switcher[i] = 0;
	}
	g_ui_ctx.switcher[2] = on;
	return 0;
}

