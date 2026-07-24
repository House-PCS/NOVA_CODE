/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_system.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-15
  * @brief       高级设置->系统设置信息
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-15    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
 
#define	 TEST_DYNAMIC							 0

#define  EM_LIST_ICON_ARROW_RIGHT_SIZE                 		(4096) 
#define  EM_LIST_ICON_SWITCH_SIZE                			(4096)
#define  EM_LIST_ICON_ARROW_DOWN_SIZE               		(4096)
  
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListAdvanceSystem = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t pv_mode_list_show;
	int32_t select;
	int32_t key_init;

	uint32_t switcher[4];
	uint16_t drm_enable;
	uint16_t anti_reverse_flow_enable;
	uint16_t eps_enable;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static IMAGE_Handle  _gImgSwithHandle;//开关图标句柄 
static IMAGE_Handle _gImgArrowHandle;//箭头图标句柄
	
static emWidgetTextPara_t  _gTitleTextPara;
static char _gTitleBuf[] = "Setting < More Settings < System";

/*List控件*/
static emListPara_t  _gEmListAdvanceSystemPara;

static char _gListAdvanceSystemLeftTextBuf[11][20];
static char _gListAdvanceSystemRightText2Buf[2][20];
static GUI_HMEM _gListAdvanceSystemRightMem;
static uint8_t *_pListAdvanceSystemRightBuf;

/*开关图片参数*/
static emWidgetImgPara_t  _gSwtichImgPara[2];  /*0:关	1:开*/
static emWidgetImgPara_t  _gArrowImgPara[2];  /*0:右	1:下*/

/*开关枚举和函数声明*/
typedef enum _SWITCH_STAT_E
{
	SWITCH_STAT_OFF = 0,
	SWITCH_STAT_ON,
} SWITCH_STAT_E;

typedef struct _pv_input_mode_idx_t
{
	const char mode[16];
} PvInputMode_t;

static PvInputMode_t PvInputMode_idx[] = 
{
	{"Independ"},
	{"Parallel"},
};


#define WIN_LIST_ADVANCED_SYSTEM_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_ADVANCED_SYSTEM_TITLE_ID                         (GUI_ID_USER + 0x11)


static const GUI_WIDGET_CREATE_INFO _aListAdvancedSystemDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListAdvancedSystemWin",    WIN_LIST_ADVANCED_SYSTEM_ID,    	 0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_ADVANCED_SYSTEM_TITLE_ID,     		31,  20,  400,  14,   0,   0x0,  0 },
};

static int32_t _SetAntiBackFlowStatus(uint16_t enable)
{
	emWidgetImgPara_t  *tmp_switch_img_para = &_gSwtichImgPara[0];
	switch (enable)
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
		default:
			break;
	}

	_gEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = tmp_switch_img_para->mem_img;
    _gEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = tmp_switch_img_para->p_img_buf;
    _gEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
	IMAGE_SetDTA(_gEmListAdvanceSystemPara.handle[1][0], tmp_switch_img_para->p_img_buf, 
				 tmp_switch_img_para->img_size); 
	
	return 0;
}

static int32_t _SetDRMStatus(uint16_t enable)
{
	emWidgetImgPara_t  *tmp_switch_img_para = &_gSwtichImgPara[0];
	switch (enable)
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
		default:
			break;
	}

	_gEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = tmp_switch_img_para->mem_img;
    _gEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = tmp_switch_img_para->p_img_buf;
    _gEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
	IMAGE_SetDTA(_gEmListAdvanceSystemPara.handle[1][2], tmp_switch_img_para->p_img_buf, 
				 tmp_switch_img_para->img_size); 
	
	return 0;
}

static int32_t _SetEPSStatus(uint16_t enable)
{
	emWidgetImgPara_t  *tmp_switch_img_para = &_gSwtichImgPara[0];
	switch (enable)
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
		default:
			break;
	}

	_gEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = tmp_switch_img_para->mem_img;
    _gEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = tmp_switch_img_para->p_img_buf;
    _gEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
	IMAGE_SetDTA(_gEmListAdvanceSystemPara.handle[1][3], tmp_switch_img_para->p_img_buf, 
				 tmp_switch_img_para->img_size); 
	
	return 0;
}


/*函数声明*/
int32_t _AdvancedSysSetSwitchStat(uint32_t row,SWITCH_STAT_E stat);
void SetPVInputModeArrowDir(void);
int32_t SetCurPVInputMode(int32_t index);

/**
  * @brief ListAdvancedSystemWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbAdvancedSystemWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_SYSTEM_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListAdvanceSystemPara, pMsg->hWin);
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
			_AdvancedSysSetSwitchStat(0,(SWITCH_STAT_E)(switch_type%2));

            WM_RestartTimer(pMsg->Data.v, 300);/*暂定300mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif 
			break;
		}
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListAdvanceSystemPara, _gEmListAdvanceSystemPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(int i = 0; i < _gEmListAdvanceSystemPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListAdvanceSystemPara.sub_region[i].x_pos, 
                                    _gEmListAdvanceSystemPara.sub_region[i].y_pos, 
                                    _gEmListAdvanceSystemPara.sub_region[i].x_pos + _gEmListAdvanceSystemPara.sub_region[i].x_size - 1, 
                                    _gEmListAdvanceSystemPara.sub_region[i].y_pos + _gEmListAdvanceSystemPara.sub_region[i].y_size - 1,
                                    _gEmListAdvanceSystemPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListAdvanceSystemPara, _gEmListAdvanceSystemPara.list_sel);
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
	pPrevEmWin = &gEmWinListAdvancedSettings;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListAdvanceSystem.win_handle = GUI_CreateDialogBox(_aListAdvancedSystemDialogCreate, GUI_COUNTOF(_aListAdvancedSystemDialogCreate), _cbAdvancedSystemWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListAdvanceSystem.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListAdvanceSystem.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListAdvanceSystem.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListAdvanceSystemPara, gEmWinListAdvanceSystem.win_handle);
    return gEmWinListAdvanceSystem.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListAdvanceSystem.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListAdvanceSystem.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListAdvanceSystem.win_handle = 0;
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
	
	/*开关图标*/
	_gSwtichImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_SWITCH_SIZE);
    _gSwtichImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gSwtichImgPara[0].mem_img);
	_gSwtichImgPara[0].img_size = EM_LIST_ICON_SWITCH_SIZE;
	SpiFlashReadData(EM_ICON_SET_CLOSE_ADDR, _gSwtichImgPara[0].p_img_buf, _gSwtichImgPara[0].img_size);
	
	_gSwtichImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_SWITCH_SIZE);
    _gSwtichImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gSwtichImgPara[1].mem_img);
	_gSwtichImgPara[1].img_size = EM_LIST_ICON_SWITCH_SIZE;
	SpiFlashReadData(EM_ICON_SET_OPEN_ADDR, _gSwtichImgPara[1].p_img_buf, _gSwtichImgPara[1].img_size);
	
	/*右箭头图标*/	
	_gListAdvanceSystemRightMem = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_RIGHT_SIZE);
    _pListAdvanceSystemRightBuf = GUI_ALLOC_h2p(_gListAdvanceSystemRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE);

    
    _gEmListAdvanceSystemPara.list_start_id = WIN_LIST_ADVANCED_SYSTEM_ID + 0x30;
    _gEmListAdvanceSystemPara.list_num = 5;
    _gEmListAdvanceSystemPara.list_display_num = 6;
    if(_gEmListAdvanceSystemPara.list_num <= _gEmListAdvanceSystemPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListAdvanceSystemPara.list_num + 1 - _gEmListAdvanceSystemPara.list_display_num;
    }
    
    if(_gEmListAdvanceSystemPara.list_sel > _gEmListAdvanceSystemPara.list_num - 1)
    {
        _gEmListAdvanceSystemPara.list_sel = 0;
    }
    if(_gEmListAdvanceSystemPara.list_page > max_page - 1)
    {
        _gEmListAdvanceSystemPara.list_page = 0;
    }
    _gEmListAdvanceSystemPara.list_bk_color = 0x00787878;

	/*List选中框*/
    /*5个子列表*/
    _gEmListAdvanceSystemPara.sub_region[0].x_pos = 20;
    _gEmListAdvanceSystemPara.sub_region[0].y_pos = 38;
    _gEmListAdvanceSystemPara.sub_region[0].x_size = 440;
    _gEmListAdvanceSystemPara.sub_region[0].y_size = 34;
    _gEmListAdvanceSystemPara.sub_region[1].x_pos = 20;
    _gEmListAdvanceSystemPara.sub_region[1].y_pos = 80;
    _gEmListAdvanceSystemPara.sub_region[1].x_size = 440;
    _gEmListAdvanceSystemPara.sub_region[1].y_size = 34;
    _gEmListAdvanceSystemPara.sub_region[2].x_pos = 20;
    _gEmListAdvanceSystemPara.sub_region[2].y_pos = 122;
    _gEmListAdvanceSystemPara.sub_region[2].x_size = 440;
    _gEmListAdvanceSystemPara.sub_region[2].y_size = 34;
    _gEmListAdvanceSystemPara.sub_region[3].x_pos = 20;
    _gEmListAdvanceSystemPara.sub_region[3].y_pos = 164;
    _gEmListAdvanceSystemPara.sub_region[3].x_size = 440;
    _gEmListAdvanceSystemPara.sub_region[3].y_size = 34;
    _gEmListAdvanceSystemPara.sub_region[4].x_pos = 20;
    _gEmListAdvanceSystemPara.sub_region[4].y_pos = 206;
    _gEmListAdvanceSystemPara.sub_region[4].x_size = 440;
    _gEmListAdvanceSystemPara.sub_region[4].y_size = 34;
    /*List控件*/

    //第一行左边
    _gEmListAdvanceSystemPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvanceSystemPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListAdvanceSystemPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListAdvanceSystemPara.sub_widget_para[0][0].x_size = 158;
    _gEmListAdvanceSystemPara.sub_widget_para[0][0].y_size = 17;
    _gEmListAdvanceSystemPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListAdvanceSystemLeftTextBuf[0];
    _gEmListAdvanceSystemPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvanceSystemPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvanceSystemPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Anti-backflow");
    //第一行右边
    _gEmListAdvanceSystemPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvanceSystemPara.sub_widget_para[1][0].x_relative_pos = 386;
    _gEmListAdvanceSystemPara.sub_widget_para[1][0].y_relative_pos = 9;
    _gEmListAdvanceSystemPara.sub_widget_para[1][0].x_size = 40;
    _gEmListAdvanceSystemPara.sub_widget_para[1][0].y_size = 31;
	_gEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListAdvanceSystemPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
	//第二行左边
    _gEmListAdvanceSystemPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvanceSystemPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListAdvanceSystemPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListAdvanceSystemPara.sub_widget_para[0][1].x_size = 158;
    _gEmListAdvanceSystemPara.sub_widget_para[0][1].y_size = 17;
    _gEmListAdvanceSystemPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListAdvanceSystemLeftTextBuf[1];
    _gEmListAdvanceSystemPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvanceSystemPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvanceSystemPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "PV Input Mode");
    //第二行右边
    _gEmListAdvanceSystemPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvanceSystemPara.sub_widget_para[1][1].x_relative_pos = 415;
    _gEmListAdvanceSystemPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gEmListAdvanceSystemPara.sub_widget_para[1][1].x_size = 5;
    _gEmListAdvanceSystemPara.sub_widget_para[1][1].y_size = 10;
	_gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
    _gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
    _gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_RIGHT_SIZE;
	//第二行中间文字
	_gEmListAdvanceSystemPara.sub_widget_para[2][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvanceSystemPara.sub_widget_para[2][1].x_relative_pos = 328;
    _gEmListAdvanceSystemPara.sub_widget_para[2][1].y_relative_pos = 10;
    _gEmListAdvanceSystemPara.sub_widget_para[2][1].x_size = 80;
    _gEmListAdvanceSystemPara.sub_widget_para[2][1].y_size = 17;
    _gEmListAdvanceSystemPara.sub_widget_para[2][1].sub_widget.text_para.p_content = _gListAdvanceSystemRightText2Buf[0];
    _gEmListAdvanceSystemPara.sub_widget_para[2][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvanceSystemPara.sub_widget_para[2][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvanceSystemPara.sub_widget_para[2][1].sub_widget.text_para.p_content, " ");
    //第三行左边
    _gEmListAdvanceSystemPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvanceSystemPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListAdvanceSystemPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListAdvanceSystemPara.sub_widget_para[0][2].x_size = 158;
    _gEmListAdvanceSystemPara.sub_widget_para[0][2].y_size = 17;
    _gEmListAdvanceSystemPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListAdvanceSystemLeftTextBuf[2];
    _gEmListAdvanceSystemPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvanceSystemPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvanceSystemPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "DRM");
    //第三行右边
    _gEmListAdvanceSystemPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvanceSystemPara.sub_widget_para[1][2].x_relative_pos = 386;
    _gEmListAdvanceSystemPara.sub_widget_para[1][2].y_relative_pos = 9;
    _gEmListAdvanceSystemPara.sub_widget_para[1][2].x_size = 40;
    _gEmListAdvanceSystemPara.sub_widget_para[1][2].y_size = 31;
	_gEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListAdvanceSystemPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第四行左边
    _gEmListAdvanceSystemPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvanceSystemPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListAdvanceSystemPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListAdvanceSystemPara.sub_widget_para[0][3].x_size = 158;
    _gEmListAdvanceSystemPara.sub_widget_para[0][3].y_size = 17;
    _gEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListAdvanceSystemLeftTextBuf[3];
    _gEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvanceSystemPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "EPS");
    //第四行右边
    _gEmListAdvanceSystemPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvanceSystemPara.sub_widget_para[1][3].x_relative_pos = 386;
    _gEmListAdvanceSystemPara.sub_widget_para[1][3].y_relative_pos = 9;
    _gEmListAdvanceSystemPara.sub_widget_para[1][3].x_size = 40;
    _gEmListAdvanceSystemPara.sub_widget_para[1][3].y_size = 31;
	_gEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListAdvanceSystemPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第五行左边
    _gEmListAdvanceSystemPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvanceSystemPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListAdvanceSystemPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListAdvanceSystemPara.sub_widget_para[0][4].x_size = 200;
    _gEmListAdvanceSystemPara.sub_widget_para[0][4].y_size = 17;
    _gEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListAdvanceSystemLeftTextBuf[4];
    _gEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvanceSystemPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Reconnect Grid Time");
    //第五行右边
    _gEmListAdvanceSystemPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvanceSystemPara.sub_widget_para[1][4].x_relative_pos = 415;
    _gEmListAdvanceSystemPara.sub_widget_para[1][4].y_relative_pos = 12;
    _gEmListAdvanceSystemPara.sub_widget_para[1][4].x_size = 5;
    _gEmListAdvanceSystemPara.sub_widget_para[1][4].y_size = 10;
	_gEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
    _gEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
    _gEmListAdvanceSystemPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_RIGHT_SIZE;
	
	  /*List选中框*/
    _gEmListAdvanceSystemPara.selected_unvalid_color = 0xFF000000;
    _gEmListAdvanceSystemPara.selected_valid_color = 0x00D6E400;
    _gEmListAdvanceSystemPara.selected_size = 3;
    _gEmListAdvanceSystemPara.selected_r = 10;
    _gEmListAdvanceSystemPara.selected_x_start_offset = -2;
    _gEmListAdvanceSystemPara.selected_y_start_offset = -2;
    _gEmListAdvanceSystemPara.selected_x_end_offset = 2;
    _gEmListAdvanceSystemPara.selected_y_end_offset = 2;
//    _gEmListAdvanceSystemPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gEmListAdvanceSystemPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
	GUI_ALLOC_Free(_gListAdvanceSystemRightMem);
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
    if(gEmWinListAdvanceSystem.win_handle == 0)
    {
        return;
    }
	
	if(g_ui_ctx.anti_reverse_flow_enable != gCollectParaData.pcs_hold_data.anti_reverse_flow_enable)
	{
		g_ui_ctx.anti_reverse_flow_enable = gCollectParaData.pcs_hold_data.anti_reverse_flow_enable;
		_SetAntiBackFlowStatus(g_ui_ctx.anti_reverse_flow_enable);
	}
	
	if(g_ui_ctx.drm_enable != gCollectParaData.pcs_hold_data.drm_enable)
	{
		g_ui_ctx.drm_enable = gCollectParaData.pcs_hold_data.drm_enable;
		_SetDRMStatus(g_ui_ctx.drm_enable);
	}
#if 0
	if(g_ui_ctx.eps_enable != gCollectParaData.pcs_hold_data.eps_enable)
	{
		g_ui_ctx.eps_enable = gCollectParaData.pcs_hold_data.eps_enable;
		_SetDRMStatus(g_ui_ctx.eps_enable);
	}
#endif
	
#if 0
    if(GetSystick() - timecount > 100)    //30
    {
        timecount = GetSystick();
        if(_gEmListAdvanceSystemPara.list_sel + _gEmListAdvanceSystemPara.list_page >= _gEmListAdvanceSystemPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListAdvanceSystemPara.list_sel + _gEmListAdvanceSystemPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListAdvanceSystemPara, gEmWinListAdvanceSystem.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListAdvanceSystemPara, gEmWinListAdvanceSystem.win_handle);
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
	if(g_ui_ctx.pv_mode_list_show)
	{
		gEmWinListSysPVInputMode.key_handler(p_msg);
		
		if(!g_ui_ctx.pv_mode_list_show)
		{
			gEmWinListSysPVInputMode.delete_win();
		}
		return;
	}
	
	
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
		EmListUpward(&_gEmListAdvanceSystemPara, gEmWinListAdvanceSystem.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		++g_ui_ctx.select;
		if(g_ui_ctx.select >= _gEmListAdvanceSystemPara.list_num)
		{
			--g_ui_ctx.select;
			return;
		}
		EmListDownward(&_gEmListAdvanceSystemPara,gEmWinListAdvancedSettings.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		emWin_t *pnext = NULL;
		switch(g_ui_ctx.select)
		{
			case 0:
				g_ui_ctx.switcher[0] = (++g_ui_ctx.switcher[0])%2;
				_AdvancedSysSetSwitchStat(g_ui_ctx.select, 
										  (SWITCH_STAT_E)g_ui_ctx.switcher[0]);
				break;
			case 1:
				{
					if(!g_ui_ctx.pv_mode_list_show)
					{
						gEmWinListSysPVInputMode.create_win();
						g_ui_ctx.pv_mode_list_show = 1;
					}
					SetPVInputModeArrowDir();
				}
				break;
			case 2:
				g_ui_ctx.switcher[1] = (++g_ui_ctx.switcher[1])%2;
				_AdvancedSysSetSwitchStat(g_ui_ctx.select, (SWITCH_STAT_E)g_ui_ctx.switcher[1]);
				break;
			case 3:
				g_ui_ctx.switcher[2] = (++g_ui_ctx.switcher[2])%2;
				_AdvancedSysSetSwitchStat(g_ui_ctx.select, (SWITCH_STAT_E)g_ui_ctx.switcher[2]);
				break;
			case 4:
				pnext = &gEmWinListReconnGridTime;
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
				pnext = &gEmWinListAdvancedSettings;
			}
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win();			
		}
	}
}

int32_t _AdvancedSysSetSwitchStat(uint32_t row,SWITCH_STAT_E stat)
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

	_gEmListAdvanceSystemPara.sub_widget_para[1][row].sub_widget.img_para.mem_img = tmp_switch_img_para->mem_img;
    _gEmListAdvanceSystemPara.sub_widget_para[1][row].sub_widget.img_para.p_img_buf = tmp_switch_img_para->p_img_buf;
    _gEmListAdvanceSystemPara.sub_widget_para[1][row].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
	IMAGE_SetDTA(_gEmListAdvanceSystemPara.handle[1][row], tmp_switch_img_para->p_img_buf, 
				 tmp_switch_img_para->img_size); 
	return 0;
}

void SetPVInputModeArrowDir(void)
{
	if(g_ui_ctx.pv_mode_list_show)
	{
		/*向下箭头图标*/
//		_gListAdvanceSystemRightMem = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_RIGHT_SIZE);
//		_pListAdvanceSystemRightBuf = GUI_ALLOC_h2p(_gListAdvanceSystemRightMem);
		SpiFlashReadData(EM_ICON_DOWN_ARROW_ADDR, _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE);	
		
		
		_gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
		_gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
		_gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_DOWN_SIZE;
		IMAGE_SetDTA(_gEmListAdvanceSystemPara.handle[1][1], _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE); 
	}
	else
	{
		SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE);	
		
		
		_gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
		_gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
		_gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_DOWN_SIZE;
		IMAGE_SetDTA(_gEmListAdvanceSystemPara.handle[1][1], _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE); 
	}
	/*向下箭头图标*/
//	_gListAdvanceSystemRightMem = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_RIGHT_SIZE);
//    _pListAdvanceSystemRightBuf = GUI_ALLOC_h2p(_gListAdvanceSystemRightMem);
//    SpiFlashReadData(EM_ICON_DOWN_ARROW_ADDR, _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE);	
//	
//	
//	_gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListAdvanceSystemRightMem;
//    _gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListAdvanceSystemRightBuf;
//    _gEmListAdvanceSystemPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_DOWN_SIZE;
//	IMAGE_SetDTA(_gEmListAdvanceSystemPara.handle[1][1], _pListAdvanceSystemRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE); 
}

void SetPVInputModeListShow(int32_t show)
{
	g_ui_ctx.pv_mode_list_show = show;
}	

int32_t SetCurPVInputMode(int32_t index)
{
	snprintf(_gEmListAdvanceSystemPara.sub_widget_para[2][1].sub_widget.text_para.p_content, 
			sizeof(_gListAdvanceSystemRightText2Buf[0]), "%s", PvInputMode_idx[index].mode);
	
	TEXT_SetText(_gEmListAdvanceSystemPara.handle[2][1], (const char *)_gEmListAdvanceSystemPara.sub_widget_para[2][1].sub_widget.text_para.p_content);
	
	return 0;
}