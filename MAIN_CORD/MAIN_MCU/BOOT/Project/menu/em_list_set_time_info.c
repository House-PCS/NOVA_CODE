/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-18
  * @brief       削峰填谷时间设置-未保存
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-18    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
 
#define  TEST_DYNAMIC									0
#define  EM_BACK_ON_ICON_ARROW_SIZE                		(4096)   

#define REC_SAVE_XPOS				(370)
#define REC_SAVE_YPOS				(285)
#define REC_SAVE_XSIZE				(90)
#define	REC_SAVE_YSIZE				(25)
#define	DIGIT_NUMS					(32)  

#define	NUMS_LINE_LEN				(12)
#define	NUMS_LIN_X_OFFSET			(16)
#define NUMS_LIN_Y_OFFFST			(27)

/*函数声明*/
void _SetPeakTimeNumberColor(int32_t index);
void _SetPeakTimeNumAdd(int32_t index);
void _DrawPeakTimeNumUnderLine(int32_t index);
void _DrawSelRectPaint(int32_t sel);
  
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListSetTimeInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t num_idx;
	int32_t key_init;

	uint32_t switcher[3];
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//底部按下文本句柄 
static TEXT_Handle  _gSaveHandle;//save句柄
static TEXT_Handle	_gTextNumHandle[DIGIT_NUMS];

static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gSaveTextPara;
static emWidgetTextPara_t _gNumTextPara[DIGIT_NUMS];


static char _gTitleBuf[] = "Settings < System < Working Mode < Set Time";
static char _gPressBuf[] = "Press save button after edit.";
static char _gSaveBuf[]  = "Save";

static char _gSetTimeNumTextBuf[DIGIT_NUMS][20];

/*List控件*/
static emListPara_t  _gEmListSetTimeInfoPara;

static char _gListSetTimeInfoLeftTextBuf[8][25];
static char _gListSetTimeInfoRightTextBuf[8][20];

static char _gListSetTimeInfoHeadTextBuf[2][20];

/*PV Input中的线条信息*/
#define WIN_LIST_SET_TIME_LINE_SIZE					(3)
#define DIALOG_PARA_OFFSET							(4)

#define WIN_LIST_SET_TIME1_LINE_X0POS				(79)
#define WIN_LIST_SET_TIME1_LINE_Y0POS				(50)
#define WIN_LIST_SET_TIME1_LINE_X1POS				(79) 
#define WIN_LIST_SET_TIME1_LINE_Y1POS				(50 + 91 -1)

#define WIN_LIST_SET_TIME2_LINE_X0POS				(79)
#define WIN_LIST_SET_TIME2_LINE_Y0POS				(175)
#define WIN_LIST_SET_TIME2_LINE_X1POS				(79)
#define WIN_LIST_SET_TIME2_LINE_Y1POS				(175 + 91 -1)


#define WIN_LIST_SET_TIME_INFO_ID                              	 	 (GUI_ID_USER + 0x12)
#define WIN_LIST_SET_TIME_INFO_TITLE_ID                        	 	 (GUI_ID_USER + 0x13)
#define WIN_LIST_SET_TIME_INFO_PRESS_ID					      	 	 (GUI_ID_USER + 0x14)
#define WIN_LIST_SET_TIME_INFO_SAVE_ID					      		 (GUI_ID_USER + 0x15)

#define WIN_SET_TIME_NUMBER1_ID									 	 (GUI_ID_USER + 0x20)
#define WIN_SET_TIME_NUMBER2_ID									 	 (GUI_ID_USER + 0x21)
#define WIN_SET_TIME_NUMBER3_ID									 	 (GUI_ID_USER + 0x22)
#define WIN_SET_TIME_NUMBER4_ID									 	 (GUI_ID_USER + 0x23)
#define WIN_SET_TIME_NUMBER5_ID										 (GUI_ID_USER + 0x24)
#define WIN_SET_TIME_NUMBER6_ID									 	 (GUI_ID_USER + 0x25)
#define WIN_SET_TIME_NUMBER7_ID									 	 (GUI_ID_USER + 0x26)
#define WIN_SET_TIME_NUMBER8_ID									 	 (GUI_ID_USER + 0x27)
#define WIN_SET_TIME_NUMBER9_ID									 	 (GUI_ID_USER + 0x28)
#define WIN_SET_TIME_NUMBER10_ID									 (GUI_ID_USER + 0x29)
#define WIN_SET_TIME_NUMBER11_ID									 (GUI_ID_USER + 0x2A)
#define WIN_SET_TIME_NUMBER12_ID									 (GUI_ID_USER + 0x2B)
#define WIN_SET_TIME_NUMBER13_ID									 (GUI_ID_USER + 0x2C)
#define WIN_SET_TIME_NUMBER14_ID									 (GUI_ID_USER + 0x2D)
#define WIN_SET_TIME_NUMBER15_ID									 (GUI_ID_USER + 0x2E)
#define WIN_SET_TIME_NUMBER16_ID									 (GUI_ID_USER + 0x2F)
#define WIN_SET_TIME_NUMBER17_ID									 (GUI_ID_USER + 0x30)
#define WIN_SET_TIME_NUMBER18_ID									 (GUI_ID_USER + 0x31)
#define WIN_SET_TIME_NUMBER19_ID									 (GUI_ID_USER + 0x32)
#define WIN_SET_TIME_NUMBER20_ID									 (GUI_ID_USER + 0x33)
#define WIN_SET_TIME_NUMBER21_ID									 (GUI_ID_USER + 0x34)
#define WIN_SET_TIME_NUMBER22_ID									 (GUI_ID_USER + 0x35)
#define WIN_SET_TIME_NUMBER23_ID									 (GUI_ID_USER + 0x36)
#define WIN_SET_TIME_NUMBER24_ID									 (GUI_ID_USER + 0x37)
#define WIN_SET_TIME_NUMBER25_ID									 (GUI_ID_USER + 0x38)
#define WIN_SET_TIME_NUMBER26_ID									 (GUI_ID_USER + 0x39)
#define WIN_SET_TIME_NUMBER27_ID									 (GUI_ID_USER + 0x3A)
#define WIN_SET_TIME_NUMBER28_ID									 (GUI_ID_USER + 0x3B)
#define WIN_SET_TIME_NUMBER29_ID									 (GUI_ID_USER + 0x3C)
#define WIN_SET_TIME_NUMBER30_ID									 (GUI_ID_USER + 0x3D)
#define WIN_SET_TIME_NUMBER31_ID									 (GUI_ID_USER + 0x3E)
#define WIN_SET_TIME_NUMBER32_ID									 (GUI_ID_USER + 0x3F)


static const GUI_WIDGET_CREATE_INFO _aListSetTimeInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListSetTimeInfoWin",    WIN_LIST_SET_TIME_INFO_ID,     		0,   	0,  	480,  	320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_SET_TIME_INFO_TITLE_ID,     		31,  	20, 	415,  	14,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_SET_TIME_INFO_PRESS_ID,       		122, 	292, 	210,   	14,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,	  "save", 			WIN_LIST_SET_TIME_INFO_SAVE_ID,		 		397, 	291, 	45, 	15,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM1",			WIN_SET_TIME_NUMBER1_ID,       					381, 	46,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM2",			WIN_SET_TIME_NUMBER2_ID,       					397, 	46,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM3",			WIN_SET_TIME_NUMBER3_ID,       					420, 	46,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM4",			WIN_SET_TIME_NUMBER4_ID,       					436, 	46,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM5",			WIN_SET_TIME_NUMBER5_ID,       					381, 	74,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM6",			WIN_SET_TIME_NUMBER6_ID,       					397, 	74,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM7",			WIN_SET_TIME_NUMBER7_ID,       					420, 	74,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM8",			WIN_SET_TIME_NUMBER8_ID,       					436, 	74,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM9",			WIN_SET_TIME_NUMBER9_ID,       					381, 	100,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM10",			WIN_SET_TIME_NUMBER10_ID,       				397, 	100,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM11",			WIN_SET_TIME_NUMBER11_ID,       				420, 	100,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM12",			WIN_SET_TIME_NUMBER12_ID,       				436, 	100,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM13",			WIN_SET_TIME_NUMBER13_ID,       				381, 	128,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM14",			WIN_SET_TIME_NUMBER14_ID,       				397, 	128,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM15",			WIN_SET_TIME_NUMBER15_ID,       				420, 	128,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM16",			WIN_SET_TIME_NUMBER16_ID,       				436, 	128,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM17",			WIN_SET_TIME_NUMBER17_ID,       				381, 	171,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM18",			WIN_SET_TIME_NUMBER18_ID,       				397, 	171,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM19",			WIN_SET_TIME_NUMBER19_ID,       				420, 	171,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM20",			WIN_SET_TIME_NUMBER20_ID,       				436, 	171,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM21",			WIN_SET_TIME_NUMBER21_ID,       				381, 	199,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM22",			WIN_SET_TIME_NUMBER22_ID,       				397, 	199,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM23",			WIN_SET_TIME_NUMBER23_ID,       				420, 	199,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM24",			WIN_SET_TIME_NUMBER24_ID,       				436, 	199,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM25",			WIN_SET_TIME_NUMBER25_ID,       				381, 	225,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM26",			WIN_SET_TIME_NUMBER26_ID,       				397, 	225,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM27",			WIN_SET_TIME_NUMBER27_ID,       				420, 	225,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM28",			WIN_SET_TIME_NUMBER28_ID,       				436, 	225,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM29",			WIN_SET_TIME_NUMBER29_ID,       				381, 	253,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM30",			WIN_SET_TIME_NUMBER30_ID,       				397, 	253,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM31",			WIN_SET_TIME_NUMBER31_ID,       				420, 	253,  	9,    	17,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM32",			WIN_SET_TIME_NUMBER32_ID,       				436, 	253,  	9,    	17,   0,   0x0,  0 },
};


/**
  * @brief ListSetTimeInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListSetTimeInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
	static int32_t num = -1;
    static int32_t index = -1;
	static int32_t selected = -1;
#if TEST_DYNAMIC
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SET_TIME_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
			/*底部文本*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin,WIN_LIST_SET_TIME_INFO_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			
			/*保存文本*/
			_gSaveHandle = WM_GetDialogItem(pMsg->hWin,WIN_LIST_SET_TIME_INFO_SAVE_ID);
            TEXT_SetFont(_gSaveHandle, _gSaveTextPara.p_text_font);
            TEXT_SetText(_gSaveHandle, (const char *)_gSaveTextPara.p_content);
            TEXT_SetTextAlign(_gSaveHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gSaveHandle, _gSaveTextPara.text_color); 	
			
			for(int i = 0; i < DIGIT_NUMS; i++)
			{
				_gTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_SET_TIME_NUMBER1_ID + i);
				TEXT_SetFont(_gTextNumHandle[i], _gNumTextPara[i].p_text_font);
				TEXT_SetText(_gTextNumHandle[i], (const char *)_gNumTextPara[i].p_content);
				TEXT_SetTextAlign(_gTextNumHandle[i], TEXT_CF_RIGHT | TEXT_CF_VCENTER);
				TEXT_SetTextColor(_gTextNumHandle[i], _gNumTextPara[i].text_color); 	
			}
			
            /*LIST*/
            EmListWidgetCreate(&_gEmListSetTimeInfoPara, pMsg->hWin);
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
			++selected;
			index = num % DIGIT_NUMS;
			_SetPeakTimeNumberColor(index);
			
			WM_Paint(pMsg->hWin);
            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif 
            break;
		}
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListSetTimeInfoPara, _gEmListSetTimeInfoPara.list_pre_sel);
			GUI_SetColor(0x00424242);//上色
			for(int i = 0; i < _gEmListSetTimeInfoPara.list_display_num; i++)
			{
			   GUI_FillRoundedRect(_gEmListSetTimeInfoPara.sub_region[i].x_pos, 
								   _gEmListSetTimeInfoPara.sub_region[i].y_pos, 
								   _gEmListSetTimeInfoPara.sub_region[i].x_pos + _gEmListSetTimeInfoPara.sub_region[i].x_size - 1, 
								   _gEmListSetTimeInfoPara.sub_region[i].y_pos + _gEmListSetTimeInfoPara.sub_region[i].y_size - 1,
								   _gEmListSetTimeInfoPara.selected_r);
			}

			/*右下角保存框的填充*/
			GUI_SetColor(0x00424242);//上色
			GUI_FillRoundedRect(REC_SAVE_XPOS, REC_SAVE_YPOS,  REC_SAVE_XPOS + REC_SAVE_XSIZE - 1,  REC_SAVE_YPOS + REC_SAVE_YSIZE - 1, 10);
//			GUI_SetColor(0xFF000000);
//			GUI_DrawRoundedFrame(REC_SAVE_XPOS, REC_SAVE_YPOS,  REC_SAVE_XPOS + REC_SAVE_XSIZE - 1,  REC_SAVE_YPOS + REC_SAVE_YSIZE - 1, 10,3);
			
			GUI_SetPenSize(WIN_LIST_SET_TIME_LINE_SIZE);
			GUI_SetColor(0x00FFFFFF);
			for(uint8_t i = 0; i < DIGIT_NUMS; i++)
			{
				GUI_DrawLine(_aListSetTimeInfoDialogCreate[i + 4].x0 - 2,_aListSetTimeInfoDialogCreate[i + 4].y0 + 18,\
							 _aListSetTimeInfoDialogCreate[i + 4].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[i + 4].y0 + 18);
			}
			
			
			GUI_SetPenSize(WIN_LIST_SET_TIME_LINE_SIZE);
			GUI_SetColor(0x00A2A856);
			GUI_DrawLine(79,50,79,50+91);
			GUI_DrawLine(79,175,79,175+91);
//			GUI_DrawLine(WIN_LIST_SET_TIME1_LINE_X0POS,WIN_LIST_SET_TIME1_LINE_Y0POS,WIN_LIST_SET_TIME1_LINE_X1POS,WIN_LIST_SET_TIME1_LINE_Y1POS);
//			GUI_DrawLine(WIN_LIST_SET_TIME2_LINE_X0POS,WIN_LIST_SET_TIME2_LINE_Y0POS,WIN_LIST_SET_TIME2_LINE_X1POS,WIN_LIST_SET_TIME2_LINE_Y1POS);
			
			_DrawPeakTimeNumUnderLine(index);
			
			_DrawSelRectPaint(g_ui_ctx.select % 2);
			//SelectedDrawInWMPaint(&_gEmListSetTimeInfoPara, _gEmListSetTimeInfoPara.list_sel);
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
	pPrevEmWin = &gEmWinListWorkingModeInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListSetTimeInfo.win_handle = GUI_CreateDialogBox(_aListSetTimeInfoDialogCreate, GUI_COUNTOF(_aListSetTimeInfoDialogCreate), _cbListSetTimeInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListSetTimeInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListSetTimeInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListSetTimeInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListSetTimeInfoPara, gEmWinListSetTimeInfo.win_handle);
    return gEmWinListSetTimeInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListSetTimeInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListSetTimeInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListSetTimeInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	EmKeyDefaultInfoSet();
	/*底部文本的参数*/
	_gPressTextPara.p_content = _gPressBuf;
    _gPressTextPara.text_color = 0x00EEEEEE;
    _gPressTextPara.p_text_font = EM_TEXT_FONT_7;
	/*保存文本参数*/
	_gSaveTextPara.p_content = _gSaveBuf;
	_gSaveTextPara.text_color = 0x00FFFFFF;
	_gSaveTextPara.p_text_font = EM_TEXT_FONT_5;
	
	for(uint8_t i = 0; i < DIGIT_NUMS; i++)
	{
		_gNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
		_gNumTextPara[i].text_color = 0x00FFFFFF;
		_gNumTextPara[i].p_content = _gSetTimeNumTextBuf[i];
		_gNumTextPara[i].digit_value = 0;
		sprintf(_gNumTextPara[i].p_content, "%1d", (int)_gNumTextPara[i].digit_value);
	}
	
	
	/*标题相关参数*/
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00EEEEEE;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListSetTimeInfoPara.list_start_id = WIN_LIST_SET_TIME_INFO_ID + 0x60;
    _gEmListSetTimeInfoPara.list_num = 8;
    _gEmListSetTimeInfoPara.list_display_num = 8;
    
    if(_gEmListSetTimeInfoPara.list_sel > _gEmListSetTimeInfoPara.list_num - 1)
    {
        _gEmListSetTimeInfoPara.list_sel = 0;
    }
    _gEmListSetTimeInfoPara.list_bk_color = 0x00787878;
	/*List选中框*/
    _gEmListSetTimeInfoPara.selected_r = 10;
	
    /*4个子列表*/
    _gEmListSetTimeInfoPara.sub_region[0].x_pos = 20;
    _gEmListSetTimeInfoPara.sub_region[0].y_pos = 38;
    _gEmListSetTimeInfoPara.sub_region[0].x_size = 440;
    _gEmListSetTimeInfoPara.sub_region[0].y_size = 117;
    _gEmListSetTimeInfoPara.sub_region[1].x_pos = 20;
    _gEmListSetTimeInfoPara.sub_region[1].y_pos = 38;
    _gEmListSetTimeInfoPara.sub_region[1].x_size = 440;
    _gEmListSetTimeInfoPara.sub_region[1].y_size = 117;
	_gEmListSetTimeInfoPara.sub_region[2].x_pos = 20;
    _gEmListSetTimeInfoPara.sub_region[2].y_pos = 38;
    _gEmListSetTimeInfoPara.sub_region[2].x_size = 440;
    _gEmListSetTimeInfoPara.sub_region[2].y_size = 117;
    _gEmListSetTimeInfoPara.sub_region[3].x_pos = 20;
    _gEmListSetTimeInfoPara.sub_region[3].y_pos = 38;
    _gEmListSetTimeInfoPara.sub_region[3].x_size = 440;
    _gEmListSetTimeInfoPara.sub_region[3].y_size = 117;
	_gEmListSetTimeInfoPara.sub_region[4].x_pos = 20;
    _gEmListSetTimeInfoPara.sub_region[4].y_pos = 163;
    _gEmListSetTimeInfoPara.sub_region[4].x_size = 440;
    _gEmListSetTimeInfoPara.sub_region[4].y_size = 117;
    _gEmListSetTimeInfoPara.sub_region[5].x_pos = 20;
    _gEmListSetTimeInfoPara.sub_region[5].y_pos = 163;
    _gEmListSetTimeInfoPara.sub_region[5].x_size = 440;
    _gEmListSetTimeInfoPara.sub_region[5].y_size = 117;
	_gEmListSetTimeInfoPara.sub_region[6].x_pos = 20;
    _gEmListSetTimeInfoPara.sub_region[6].y_pos = 163;
    _gEmListSetTimeInfoPara.sub_region[6].x_size = 440;
    _gEmListSetTimeInfoPara.sub_region[7].y_size = 117;
    _gEmListSetTimeInfoPara.sub_region[7].x_pos = 20;
    _gEmListSetTimeInfoPara.sub_region[7].y_pos = 163;
    _gEmListSetTimeInfoPara.sub_region[7].x_size = 440;
    _gEmListSetTimeInfoPara.sub_region[7].y_size = 117;


    /*List控件*/

    //第一行左边
    _gEmListSetTimeInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[0][0].x_relative_pos = 73;
    _gEmListSetTimeInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListSetTimeInfoPara.sub_widget_para[0][0].x_size = 186;
    _gEmListSetTimeInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListSetTimeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListSetTimeInfoLeftTextBuf[0];
    _gEmListSetTimeInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Charge Start-time");
	//第一行右边
    _gEmListSetTimeInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[1][0].x_relative_pos = 390;
    _gEmListSetTimeInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListSetTimeInfoPara.sub_widget_para[1][0].x_size = 5;
    _gEmListSetTimeInfoPara.sub_widget_para[1][0].y_size = 10;
	_gEmListSetTimeInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListSetTimeInfoRightTextBuf[0];
    _gEmListSetTimeInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListSetTimeInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content, ":");  
    //第二行左边
    _gEmListSetTimeInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[0][1].x_relative_pos = 73;
    _gEmListSetTimeInfoPara.sub_widget_para[0][1].y_relative_pos = 37;
    _gEmListSetTimeInfoPara.sub_widget_para[0][1].x_size = 186;
    _gEmListSetTimeInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListSetTimeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListSetTimeInfoLeftTextBuf[1];
    _gEmListSetTimeInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Charge End-time");
    //第二行右边
    _gEmListSetTimeInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[1][1].x_relative_pos = 390;
    _gEmListSetTimeInfoPara.sub_widget_para[1][1].y_relative_pos = 38;
    _gEmListSetTimeInfoPara.sub_widget_para[1][1].x_size = 5;
    _gEmListSetTimeInfoPara.sub_widget_para[1][1].y_size = 10;
	_gEmListSetTimeInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListSetTimeInfoRightTextBuf[1];
    _gEmListSetTimeInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListSetTimeInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content, ":");
	//第三行左边
    _gEmListSetTimeInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[0][2].x_relative_pos = 73;
    _gEmListSetTimeInfoPara.sub_widget_para[0][2].y_relative_pos = 67;
    _gEmListSetTimeInfoPara.sub_widget_para[0][2].x_size = 186;
    _gEmListSetTimeInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListSetTimeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListSetTimeInfoLeftTextBuf[2];
    _gEmListSetTimeInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Discharge Start-time");
    //第三行右边
    _gEmListSetTimeInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[1][2].x_relative_pos = 390;
    _gEmListSetTimeInfoPara.sub_widget_para[1][2].y_relative_pos = 64;
    _gEmListSetTimeInfoPara.sub_widget_para[1][2].x_size = 5;
    _gEmListSetTimeInfoPara.sub_widget_para[1][2].y_size = 10;
	_gEmListSetTimeInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListSetTimeInfoRightTextBuf[2];
    _gEmListSetTimeInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content, ":");
	//第四行左边
    _gEmListSetTimeInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[0][3].x_relative_pos = 73;
    _gEmListSetTimeInfoPara.sub_widget_para[0][3].y_relative_pos = 91;
    _gEmListSetTimeInfoPara.sub_widget_para[0][3].x_size = 186;
    _gEmListSetTimeInfoPara.sub_widget_para[0][3].y_size = 17;
    _gEmListSetTimeInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListSetTimeInfoLeftTextBuf[3];
    _gEmListSetTimeInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Discharge End-time");
	//第四行右边
    _gEmListSetTimeInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[1][3].x_relative_pos = 390;
    _gEmListSetTimeInfoPara.sub_widget_para[1][3].y_relative_pos = 92;
    _gEmListSetTimeInfoPara.sub_widget_para[1][3].x_size = 5;
    _gEmListSetTimeInfoPara.sub_widget_para[1][3].y_size = 10;
	_gEmListSetTimeInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _gListSetTimeInfoRightTextBuf[3];
    _gEmListSetTimeInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content, ":");
    //第五行左边
    _gEmListSetTimeInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[0][4].x_relative_pos = 73;
    _gEmListSetTimeInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListSetTimeInfoPara.sub_widget_para[0][4].x_size = 186;
    _gEmListSetTimeInfoPara.sub_widget_para[0][4].y_size = 17;
    _gEmListSetTimeInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListSetTimeInfoLeftTextBuf[4];
    _gEmListSetTimeInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Charge Start-time");
    //第五行右边
    _gEmListSetTimeInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[1][4].x_relative_pos = 390;
    _gEmListSetTimeInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gEmListSetTimeInfoPara.sub_widget_para[1][4].x_size = 5;
    _gEmListSetTimeInfoPara.sub_widget_para[1][4].y_size = 10;
	_gEmListSetTimeInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _gListSetTimeInfoRightTextBuf[4];
    _gEmListSetTimeInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content, ":");
	
	//第六行左边
    _gEmListSetTimeInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[0][5].x_relative_pos = 73;
    _gEmListSetTimeInfoPara.sub_widget_para[0][5].y_relative_pos = 37;
    _gEmListSetTimeInfoPara.sub_widget_para[0][5].x_size = 186;
    _gEmListSetTimeInfoPara.sub_widget_para[0][5].y_size = 17;
    _gEmListSetTimeInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListSetTimeInfoLeftTextBuf[5];
    _gEmListSetTimeInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Charge End-time");
    //第六行右边
    _gEmListSetTimeInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[1][5].x_relative_pos = 390;
    _gEmListSetTimeInfoPara.sub_widget_para[1][5].y_relative_pos = 38;
    _gEmListSetTimeInfoPara.sub_widget_para[1][5].x_size = 5;
    _gEmListSetTimeInfoPara.sub_widget_para[1][5].y_size = 10;
	_gEmListSetTimeInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _gListSetTimeInfoRightTextBuf[5];
    _gEmListSetTimeInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content, ":");
	//第七行左边
    _gEmListSetTimeInfoPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[0][6].x_relative_pos = 73;
    _gEmListSetTimeInfoPara.sub_widget_para[0][6].y_relative_pos = 64;
    _gEmListSetTimeInfoPara.sub_widget_para[0][6].x_size = 186;
    _gEmListSetTimeInfoPara.sub_widget_para[0][6].y_size = 17;
    _gEmListSetTimeInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _gListSetTimeInfoLeftTextBuf[6];
    _gEmListSetTimeInfoPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Discharge Start-time");
	//第七行右边
    _gEmListSetTimeInfoPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[1][6].x_relative_pos = 390;
    _gEmListSetTimeInfoPara.sub_widget_para[1][6].y_relative_pos = 64;
    _gEmListSetTimeInfoPara.sub_widget_para[1][6].x_size = 5;
    _gEmListSetTimeInfoPara.sub_widget_para[1][6].y_size = 10;
	_gEmListSetTimeInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_content = _gListSetTimeInfoRightTextBuf[6];
    _gEmListSetTimeInfoPara.sub_widget_para[1][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListSetTimeInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_content, ":");
	//第八行左边
    _gEmListSetTimeInfoPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[0][7].x_relative_pos = 73;
    _gEmListSetTimeInfoPara.sub_widget_para[0][7].y_relative_pos = 91;
    _gEmListSetTimeInfoPara.sub_widget_para[0][7].x_size = 186;
    _gEmListSetTimeInfoPara.sub_widget_para[0][7].y_size = 17;
    _gEmListSetTimeInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _gListSetTimeInfoLeftTextBuf[7];
    _gEmListSetTimeInfoPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Discharge End-time");
	//第八行右边
    _gEmListSetTimeInfoPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSetTimeInfoPara.sub_widget_para[1][7].x_relative_pos = 390;
    _gEmListSetTimeInfoPara.sub_widget_para[1][7].y_relative_pos = 90;
    _gEmListSetTimeInfoPara.sub_widget_para[1][7].x_size = 5;
    _gEmListSetTimeInfoPara.sub_widget_para[1][7].y_size = 10;
	_gEmListSetTimeInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_content = _gListSetTimeInfoRightTextBuf[7];
    _gEmListSetTimeInfoPara.sub_widget_para[1][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListSetTimeInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_content, ":");

    //Time1标题
	_gEmListSetTimeInfoPara.sub_widget_para[2][0].widget_type = EM_WIDGET_TYPE_TEXT;
	_gEmListSetTimeInfoPara.sub_widget_para[2][0].x_relative_pos = 3;
    _gEmListSetTimeInfoPara.sub_widget_para[2][0].y_relative_pos = 35;
    _gEmListSetTimeInfoPara.sub_widget_para[2][0].x_size = 50;
    _gEmListSetTimeInfoPara.sub_widget_para[2][0].y_size = 31;
    _gEmListSetTimeInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content = _gListSetTimeInfoHeadTextBuf[0];
    _gEmListSetTimeInfoPara.sub_widget_para[2][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content, "Time\n1    ");
	//Time2标题
	_gEmListSetTimeInfoPara.sub_widget_para[2][4].widget_type = EM_WIDGET_TYPE_TEXT;
	_gEmListSetTimeInfoPara.sub_widget_para[2][4].x_relative_pos = 3;
    _gEmListSetTimeInfoPara.sub_widget_para[2][4].y_relative_pos = 35;
    _gEmListSetTimeInfoPara.sub_widget_para[2][4].x_size = 50;
    _gEmListSetTimeInfoPara.sub_widget_para[2][4].y_size = 31;
    _gEmListSetTimeInfoPara.sub_widget_para[2][4].sub_widget.text_para.p_content = _gListSetTimeInfoHeadTextBuf[1];
    _gEmListSetTimeInfoPara.sub_widget_para[2][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSetTimeInfoPara.sub_widget_para[2][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSetTimeInfoPara.sub_widget_para[2][4].sub_widget.text_para.p_content, "Time\n2    ");
	
	
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
		g_ui_ctx.select = (--g_ui_ctx.select) < 0? 0:g_ui_ctx.select;
		if (g_ui_ctx.select >= 0)
		{
			_SetPeakTimeNumberColor(g_ui_ctx.select);
		}
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		g_ui_ctx.select = (++g_ui_ctx.select) > 4? 4:g_ui_ctx.select;
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		emWin_t *pnext = NULL;
		switch(g_ui_ctx.select)
		{
		case 0:
			break;
		case 1:
			break;
		case 2: // save

			break;
		default:
			return;
		}
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
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pPrevEmWin;
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
void _DrawSelRectPaint(int32_t sel)
{
	if(sel == 0)
	{
		GUI_SetColor(0x00BDC909);
		GUI_DrawRoundedFrame(20, 38, 20+440, 38+117-1,10,3);
	}
	else
	{
		GUI_SetColor(0x00BDC909);
		GUI_DrawRoundedFrame(20, 163, 20+440, 163+117-1,10,3);
	}
}

/**
  * @brief  设置选中数字文本的颜色
  * @param  NULL
  * @retval NULL
  */
void _SetPeakTimeNumberColor(int32_t index)
{
	_SetPeakTimeNumAdd(index);
	
	static volatile uint8_t nums_sel_falg[DIGIT_NUMS] = {0};
	uint8_t i = 0;
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
void _SetPeakTimeNumAdd(int32_t index)
{
	_gNumTextPara[index].digit_value++;
	sprintf(_gNumTextPara[index].p_content, "%1d", (int)_gNumTextPara[index].digit_value);
	if(9 < _gNumTextPara[index].digit_value)
	{
		_gNumTextPara[index].digit_value = 0;
	}
	TEXT_SetText(_gTextNumHandle[index],_gSetTimeNumTextBuf[index]);
}


/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
void _DrawPeakTimeNumUnderLine(int32_t index)
{
	GUI_SetColor(0x00CDF100);
	switch(index)
	{
		case 0:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 1:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 2:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 3:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 4:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 5:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 6:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 7:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 8:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 9:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		case 10:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 11:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 12:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 13:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 14:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 15:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 16:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 17:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 18:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 19:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		case 20:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 21:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 22:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 23:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 24:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 25:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 26:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 27:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 28:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 29:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		case 30:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 31:
		{
			GUI_DrawLine(_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
						 _aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12,_aListSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		default:
			break;
	}
}

