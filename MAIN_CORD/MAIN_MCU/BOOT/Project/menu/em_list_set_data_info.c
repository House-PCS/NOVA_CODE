/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_set_data_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-19
  * @brief       高级设置->日期设置
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-19    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
#include <time.h>
  
#define		EM_BACK_ON_ICON_ARROW_SIZE                		(4096)  
#define		DATA_DIGIT_NUMS									(12)  
#define		REC_XPOS										(20)
#define 	REC_YPOS										(38)
#define 	REC_XSIZE										(440)
#define 	REC_YSIZE										(34)
#define 	DIALOG_PARA_OFFSET								(9)
#define  	TEST_DYNAMIC									(0)
 
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gEmWinListSetDataInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;

	int8_t num[DATA_DIGIT_NUMS];
	uint16_t data[2];
} UiCtx_t;

typedef struct _data_t
{
	uint16_t date_year;
	uint16_t date_month;
	uint16_t date_day;
} Date_t;

static Date_t date_check = {0};

static UiCtx_t g_ui_ctx = {0};

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

/*data time列表*/
static TEXT_Handle  _gTextDataHandle;//显示data句柄
static TEXT_Handle	_gTextTimeHandle;//显示Time句柄
static TEXT_Handle	_gTextNumHandle[DATA_DIGIT_NUMS];
static TEXT_Handle	_gTextColonHandle[2];//冒号句柄


static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 
static emWidgetTextPara_t  _gDataTextPara;
static emWidgetTextPara_t  _gNumTextPara[DATA_DIGIT_NUMS];
static emWidgetTextPara_t  _gTimeTextPara;
static emWidgetTextPara_t  _gColonTextPara[2];

static char _gTitleBuf[] = "Settings < More Settings < Time";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";
static char _gDataBuf[] = "Date";
static char _gTimeBuf[] = "Time";
static char _gColonBuf[] = " :";

static char _gSetDataNumTextBuf[DATA_DIGIT_NUMS][20];


#define WIN_LIST_SET_DATA_ID                            (GUI_ID_USER + 0x10)
#define WIN_LIST_SET_DATA_TITLE_ID                      (GUI_ID_USER + 0x11)
#define WIN_LIST_SET_DATA_PRESS_ID					    (GUI_ID_USER + 0x12)
#define WIN_LIST_SET_DATA_BACK_ID					    (GUI_ID_USER + 0x13)
#define WIN_LIST_SET_DATA_BACK_IMG_ID					(GUI_ID_USER + 0x14)
#define WIN_MAIN_SET_DATA_ID							(GUI_ID_USER + 0x15)
#define WIN_MAIN_SET_TIME_ID							(GUI_ID_USER + 0x16)
#define WIN_MAIN_COLON1_ID								(GUI_ID_USER + 0x17)
#define WIN_MAIN_COLON2_ID								(GUI_ID_USER + 0x18)

//年
#define WIN_MAIN_NUMBER1_ID								(GUI_ID_USER + 0x20)
#define WIN_MAIN_NUMBER2_ID								(GUI_ID_USER + 0x21)
#define WIN_MAIN_NUMBER3_ID							    (GUI_ID_USER + 0x22)
#define WIN_MAIN_NUMBER4_ID								(GUI_ID_USER + 0x23)
//月
#define WIN_MAIN_NUMBER5_ID								(GUI_ID_USER + 0x24)
//日
#define WIN_MAIN_NUMBER6_ID								(GUI_ID_USER + 0x25)
//小时
#define WIN_MAIN_NUMBER7_ID								(GUI_ID_USER + 0x26)
#define WIN_MAIN_NUMBER8_ID								(GUI_ID_USER + 0x27)
//分钟
#define WIN_MAIN_NUMBER9_ID								(GUI_ID_USER + 0x28)
#define WIN_MAIN_NUMBER10_ID							(GUI_ID_USER + 0x29)
//秒
#define WIN_MAIN_NUMBER11_ID							(GUI_ID_USER + 0x2A)
#define WIN_MAIN_NUMBER12_ID							(GUI_ID_USER + 0x2B)



/*资源列表*/
static const GUI_WIDGET_CREATE_INFO _aListReconnGridTimeDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListReconnGridTimeWin",    WIN_LIST_SET_DATA_ID,     				0,   	0,   	480,	320,	0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_SET_DATA_TITLE_ID,     					31,  	20,  	415,	14,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_SET_DATA_PRESS_ID,       					133, 	273, 	33,   	14,   	0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_SET_DATA_BACK_IMG_ID,    					171-5, 	275, 	15,   	10,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_SET_DATA_BACK_ID,       					188, 	273, 	150,  	14,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "DATA",			WIN_MAIN_SET_DATA_ID,								31, 	48, 	180,  	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Time",			WIN_MAIN_SET_TIME_ID,								31, 	90, 	180,  	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "COLON1",			WIN_MAIN_COLON1_ID,       							357, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "COLON2",			WIN_MAIN_COLON2_ID,       							403, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM1",			WIN_MAIN_NUMBER1_ID,       							296, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM2",			WIN_MAIN_NUMBER2_ID,       							312, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM3",			WIN_MAIN_NUMBER3_ID,       							328, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM4",			WIN_MAIN_NUMBER4_ID,       							344, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM5",			WIN_MAIN_NUMBER5_ID,       							380, 	47,  	20,    	17,   	0,   0x0,  0 },
	//{ TEXT_CreateIndirect,    "NUM6",			WIN_MAIN_NUMBER6_ID,       							390, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM7",			WIN_MAIN_NUMBER6_ID,       							426, 	47,  	20,    	17,   	0,   0x0,  0 },
	//{ TEXT_CreateIndirect,    "NUM8",			WIN_MAIN_NUMBER8_ID,       							436, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM9",			WIN_MAIN_NUMBER7_ID,       							328, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM10",			WIN_MAIN_NUMBER8_ID,       							344, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM11",			WIN_MAIN_NUMBER9_ID,       							374, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM12",			WIN_MAIN_NUMBER10_ID,       						390, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM13",			WIN_MAIN_NUMBER11_ID,       						420, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM14",			WIN_MAIN_NUMBER12_ID,       						436, 	89,  	9,    	17,   	0,   0x0,  0 },
};

static int32_t _SetDataInfoNumber(int32_t ui_update, uint16_t *data)
{
	uint8_t i, k = 0;
	uint32_t time = 0;
	memcpy(&time, data, 4);
	
	struct tm* timeinfo;
	
	timeinfo = localtime(&time); // 根据时间戳生成tm结构体对象

	char tmp_data[20] = {0};
	strftime(tmp_data, sizeof(tmp_data), "%Y%m%d%H%M%S", timeinfo);
	
	
	//年
	for(k = 2; k < 4; k++)
	{
		_gSetDataNumTextBuf[k][0] = tmp_data[k];
		snprintf(_gSetDataNumTextBuf[k],sizeof(_gSetDataNumTextBuf[k]),"%c", tmp_data[k]);
	}
	//月
	_gSetDataNumTextBuf[4][0] = tmp_data[4];
	_gSetDataNumTextBuf[4][1] = tmp_data[5];
	snprintf(_gSetDataNumTextBuf[4],sizeof(_gSetDataNumTextBuf[4]),"%c%c", tmp_data[4],tmp_data[5]);

	//日
	_gSetDataNumTextBuf[5][0] = tmp_data[6];
	_gSetDataNumTextBuf[5][1] = tmp_data[7];
	snprintf(_gSetDataNumTextBuf[5],sizeof(_gSetDataNumTextBuf[5]),"%c%c", tmp_data[6],tmp_data[7]);
	//时 分 秒
	for(k = 8; k < 14; k++)
	{
		_gSetDataNumTextBuf[k-2][0] = tmp_data[k];
		snprintf(_gSetDataNumTextBuf[k-2],sizeof(_gSetDataNumTextBuf[k-2]),"%c", tmp_data[k]);
	}
		
	if(ui_update)
	{
		for(i = 2; i < DATA_DIGIT_NUMS; ++i)
		{
			TEXT_SetText(_gTextNumHandle[i], (const char *)_gNumTextPara[i].p_content);
		}		
	}
	
	return 0;
}



/*函数声明*/
void _SetDataNumberColor(int32_t index);
void _DrawUnderLine(int32_t index);
void _SetDataNumAdd(int32_t index, int32_t v);
void SelectedDrawRectPaint(int32_t sel);
Date_t DateCheck(uint16_t year,uint16_t month,uint16_t day,int32_t check_dir);

/**
  * @brief ListReconnGridTimeWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListReconnGridTimeWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
	static int32_t num = -1;
	static int32_t index = -1;
	static int32_t selected = -1;
	static WM_HTIMER wm_timer_handel_nums;
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SET_DATA_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
			//列表中的文字
			_gTextDataHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_SET_DATA_ID);
            TEXT_SetFont(_gTextDataHandle, _gDataTextPara.p_text_font);
            TEXT_SetText(_gTextDataHandle, (const char *)_gDataTextPara.p_content);
            TEXT_SetTextAlign(_gTextDataHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextDataHandle, _gDataTextPara.text_color); 
			//TIME
			_gTextTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_SET_TIME_ID);
            TEXT_SetFont(_gTextTimeHandle, _gTimeTextPara.p_text_font);
            TEXT_SetText(_gTextTimeHandle, (const char *)_gTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextTimeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextTimeHandle, _gTimeTextPara.text_color);		
			//冒号
			_gTextColonHandle[0] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_COLON1_ID);
            TEXT_SetFont(_gTextColonHandle[0], _gColonTextPara[0].p_text_font);
            TEXT_SetText(_gTextColonHandle[0], (const char *)_gColonTextPara[0].p_content);
            TEXT_SetTextAlign(_gTextColonHandle[0], TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextColonHandle[0], _gColonTextPara[0].text_color);	
			
			_gTextColonHandle[1] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_COLON2_ID);
            TEXT_SetFont(_gTextColonHandle[1], _gColonTextPara[0].p_text_font);
            TEXT_SetText(_gTextColonHandle[1], (const char *)_gColonTextPara[1].p_content);
            TEXT_SetTextAlign(_gTextColonHandle[1], TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextColonHandle[1], _gColonTextPara[1].text_color);				
			
			for(int i = 0; i < DATA_DIGIT_NUMS; i++)
			{
				_gTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_NUMBER1_ID + i);
				TEXT_SetFont(_gTextNumHandle[i], _gNumTextPara[i].p_text_font);
				TEXT_SetText(_gTextNumHandle[i], (const char *)_gNumTextPara[i].p_content);
				TEXT_SetTextAlign(_gTextNumHandle[i], TEXT_CF_HCENTER | TEXT_CF_VCENTER);
				TEXT_SetTextColor(_gTextNumHandle[i], _gNumTextPara[i].text_color); 	
			}
		
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SET_DATA_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SET_DATA_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SET_DATA_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
			
			_SetDataNumberColor(g_ui_ctx.select);

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
			index = num % DATA_DIGIT_NUMS;
			_SetDataNumberColor(index);
			
			WM_Paint(pMsg->hWin);
            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
		}
		
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListReconnGridTimePara, _gEmListReconnGridTimePara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
			GUI_FillRoundedRect(REC_XPOS, REC_YPOS,  REC_XPOS + REC_XSIZE - 1,  REC_YPOS + REC_YSIZE - 1, 10);
			GUI_FillRoundedRect(REC_XPOS, REC_YPOS + 42,  REC_XPOS + REC_XSIZE - 1,  REC_YPOS + 42 + REC_YSIZE - 1, 10);

			SelectedDrawRectPaint(g_ui_ctx.select / (DATA_DIGIT_NUMS / 2));
			
			GUI_SetPenSize(3);
			GUI_SetColor(0x00FFFFFF);
			//日期中的短横线
			GUI_DrawLine(360, 53, 360+10 ,53);  
			GUI_DrawLine(406, 53, 406+10 ,53);
			 
			for(uint8_t i = 0; i < DATA_DIGIT_NUMS; i++)
			{
				if(i == 4 || i == 5)
				{
					GUI_DrawLine(_aListReconnGridTimeDialogCreate[i + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[i + DIALOG_PARA_OFFSET].y0 + 18,\
							_aListReconnGridTimeDialogCreate[i + DIALOG_PARA_OFFSET].x0 - 2 + 24, _aListReconnGridTimeDialogCreate[i + DIALOG_PARA_OFFSET].y0 + 18);
				}
				else
				{
					GUI_DrawLine(_aListReconnGridTimeDialogCreate[i + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[i + DIALOG_PARA_OFFSET].y0 + 18,\
							_aListReconnGridTimeDialogCreate[i + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[i + DIALOG_PARA_OFFSET].y0 + 18);
				}
			}

			_DrawUnderLine(g_ui_ctx.select);	
//            SelectedDrawInWMPaint(&_gEmListReconnGridTimePara, _gEmListReconnGridTimePara.list_sel);
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
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListSetDataInfo.win_handle = GUI_CreateDialogBox(_aListReconnGridTimeDialogCreate, GUI_COUNTOF(_aListReconnGridTimeDialogCreate), _cbListReconnGridTimeWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListSetDataInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListSetDataInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListSetDataInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListReconnGridTimePara, gEmWinListSetDataInfo.win_handle);
    return gEmWinListSetDataInfo.win_handle;
	
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListSetDataInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListSetDataInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListSetDataInfo.win_handle = 0;
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	uint8_t i = 0;
	
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
	_gDataTextPara.p_text_font = EM_TEXT_FONT_5;
	_gDataTextPara.text_color = 0x00FFFFFF;
	_gDataTextPara.p_content = _gDataBuf;
	
	_gTimeTextPara.p_text_font = EM_TEXT_FONT_5;
	_gTimeTextPara.text_color = 0x00FFFFFF;
	_gTimeTextPara.p_content = _gTimeBuf;
	
	_gColonTextPara[0].p_text_font = EM_TEXT_FONT_5;
	_gColonTextPara[0].text_color = 0x00FFFFFF;
	_gColonTextPara[0].p_content = _gColonBuf;
	
	_gColonTextPara[1].p_text_font = EM_TEXT_FONT_5;
	_gColonTextPara[1].text_color = 0x00FFFFFF;
	_gColonTextPara[1].p_content = _gColonBuf;


#if 0
	//初始化年的数字：保持前两位定值
	for( i = 0; i < 4; i++)
	{
		_gNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
		_gNumTextPara[i].text_color = 0x00FFFFFF;
		_gNumTextPara[i].p_content = _gSetDataNumTextBuf[i];
		//前两位固定
		if( i == 0)
		{
			_gNumTextPara[i].digit_value = 2;		
		}
		else if( i == 1)
		{
			_gNumTextPara[i].digit_value = 0;		
		}
		//后两位在0~99之间变化
		else
		{
			_gNumTextPara[i].digit_value = g_ui_ctx.num[i];
		}
		sprintf(_gNumTextPara[i].p_content, "%1d", (int)_gNumTextPara[i].digit_value);
	}
	//初始化月 日：都是单独一个数字
	for( i = 4; i < 6; i++)
	{
		if(g_ui_ctx.num[i] == 0)
		{
			g_ui_ctx.num[i] = 1;
		}
		_gNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
		_gNumTextPara[i].text_color = 0x00FFFFFF;
		_gNumTextPara[i].p_content = _gSetDataNumTextBuf[i];
		_gNumTextPara[i].digit_value = g_ui_ctx.num[i];
		sprintf(_gNumTextPara[i].p_content, "%02d", (int)_gNumTextPara[i].digit_value);
	}
	
	for( i = 6; i < 12; i++)
	{
		_gNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
		_gNumTextPara[i].text_color = 0x00FFFFFF;
		_gNumTextPara[i].p_content = _gSetDataNumTextBuf[i];
		_gNumTextPara[i].digit_value = g_ui_ctx.num[i];
		sprintf(_gNumTextPara[i].p_content, "%1d", (int)_gNumTextPara[i].digit_value);
	}
#endif	
	
	
	for( i = 0; i < 2; i++)
	{
		_gNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
		_gNumTextPara[i].text_color = 0x00FFFFFF;
		_gNumTextPara[i].p_content = _gSetDataNumTextBuf[i];
		//前两位固定
		if( i == 0)
		{
			_gNumTextPara[i].digit_value = 2;		
		}
		else if( i == 1)
		{
			_gNumTextPara[i].digit_value = 0;		
		}
		sprintf(_gNumTextPara[i].p_content, "%1d", (int)_gNumTextPara[i].digit_value);
	}
	for(i = 2; i < DATA_DIGIT_NUMS; ++i)
	{
		_gNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
		_gNumTextPara[i].text_color = 0x00FFFFFF;
		_gNumTextPara[i].p_content = _gSetDataNumTextBuf[i];
	}
	_SetDataInfoNumber(0,gCollectParaData.collect_data.rtc_time);

	
	
	/*标题*/
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00EEEEEE;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
	
    g_ui_ctx.select = 2;
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
	if(!memcmp(g_ui_ctx.data,gCollectParaData.collect_data.rtc_time,4))
	{
		memcpy(g_ui_ctx.data,gCollectParaData.collect_data.rtc_time,4);
		_SetDataInfoNumber(1, g_ui_ctx.data);
	}
	
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
	
	uint16_t year 	= 0;
	uint16_t month 	= 0;
	uint16_t day 	= 0;
	
	if(event == EN_KEY_SRC_LEFT)
	{
		if(g_ui_ctx.select >= DATA_DIGIT_NUMS)
		{
			return;
		}
		//设置年的十位和个位数
		if(g_ui_ctx.select == 2 || g_ui_ctx.select == 3)
		{
			g_ui_ctx.num[g_ui_ctx.select] = --g_ui_ctx.num[g_ui_ctx.select] < 1 ? 9 : g_ui_ctx.num[g_ui_ctx.select];
		}
		//设置月位数
		else if(g_ui_ctx.select == 4)
		{
			g_ui_ctx.num[g_ui_ctx.select] = --g_ui_ctx.num[g_ui_ctx.select] < 1 ? 12 : g_ui_ctx.num[g_ui_ctx.select];
		}
		//设置天的位数
		else if(g_ui_ctx.select == 5)
		{
			g_ui_ctx.num[g_ui_ctx.select] = --g_ui_ctx.num[g_ui_ctx.select] < 1 ? 31 : g_ui_ctx.num[g_ui_ctx.select];
		}
		//设置小时十位
		else if(g_ui_ctx.select == 6)
		{
			g_ui_ctx.num[g_ui_ctx.select] = --g_ui_ctx.num[g_ui_ctx.select] < 0 ? 2 : g_ui_ctx.num[g_ui_ctx.select];
			if(g_ui_ctx.num[g_ui_ctx.select] == 2 && g_ui_ctx.num[7] > 3)
			{
				g_ui_ctx.num[g_ui_ctx.select] = --g_ui_ctx.num[g_ui_ctx.select] < 0 ? 1 : g_ui_ctx.num[g_ui_ctx.select];
			}
		}
		//设置小时的个位
		else if(g_ui_ctx.select == 7)
		{
			if(g_ui_ctx.num[6] == 0 || g_ui_ctx.num[6] == 1)
			{
				g_ui_ctx.num[g_ui_ctx.select] = --g_ui_ctx.num[g_ui_ctx.select] < 0 ? 9 : g_ui_ctx.num[g_ui_ctx.select];
			}
			else
			{
				g_ui_ctx.num[g_ui_ctx.select] = --g_ui_ctx.num[g_ui_ctx.select] < 0 ? 3 : g_ui_ctx.num[g_ui_ctx.select];
			}
		}	
		//设置分钟 秒的十位
		else if(g_ui_ctx.select == 8 || g_ui_ctx.select == 10)
		{
			g_ui_ctx.num[g_ui_ctx.select] = --g_ui_ctx.num[g_ui_ctx.select] < 0 ? 5 : g_ui_ctx.num[g_ui_ctx.select];
		}
		//设置分钟 秒的个位
		else if(g_ui_ctx.select == 9 || g_ui_ctx.select == 11)
		{
			g_ui_ctx.num[g_ui_ctx.select] = --g_ui_ctx.num[g_ui_ctx.select] < 0 ? 9 : g_ui_ctx.num[g_ui_ctx.select];
		}
	}
	else if(event == EN_KEY_SRC_RIGHT)
	{
		if(g_ui_ctx.select >= DATA_DIGIT_NUMS)
		{
			return;
		}
		//设置年份数字变化（0~9）
		if(g_ui_ctx.select == 2 || g_ui_ctx.select == 3)
		{
			g_ui_ctx.num[g_ui_ctx.select] = ++g_ui_ctx.num[g_ui_ctx.select] > 9 ? 1 : g_ui_ctx.num[g_ui_ctx.select];
		}
		//设置月份
		else if(g_ui_ctx.select == 4)
		{
			g_ui_ctx.num[g_ui_ctx.select] = ++g_ui_ctx.num[g_ui_ctx.select] > 12 ? 1 : g_ui_ctx.num[g_ui_ctx.select];
		}
		//设置天数
		else if(g_ui_ctx.select == 5)
		{
			g_ui_ctx.num[g_ui_ctx.select] = ++g_ui_ctx.num[g_ui_ctx.select] > 31 ? 1 : g_ui_ctx.num[g_ui_ctx.select];
		}
		//设置小时十位
		else if(g_ui_ctx.select == 6)
		{
			g_ui_ctx.num[g_ui_ctx.select] = ++(g_ui_ctx.num[g_ui_ctx.select]) > 2 ? 0 : g_ui_ctx.num[g_ui_ctx.select];
			if(g_ui_ctx.num[g_ui_ctx.select] == 2 && g_ui_ctx.num[7] > 3)
			{
				g_ui_ctx.num[g_ui_ctx.select] = ++(g_ui_ctx.num[g_ui_ctx.select]) > 1 ? 0 : g_ui_ctx.num[g_ui_ctx.select];
			}
		}
		//设置小时个位
		else if(g_ui_ctx.select == 7)
		{
			if(g_ui_ctx.num[6] == 0 || g_ui_ctx.num[6] == 1)
			{
				g_ui_ctx.num[g_ui_ctx.select] = ++(g_ui_ctx.num[g_ui_ctx.select]) > 9 ? 0 : g_ui_ctx.num[g_ui_ctx.select];
			}
			else
			{
				g_ui_ctx.num[g_ui_ctx.select] = ++(g_ui_ctx.num[g_ui_ctx.select]) > 3 ? 0 : g_ui_ctx.num[g_ui_ctx.select];
			}
		}
		//设置分钟 秒的十位
		else if(g_ui_ctx.select == 8 || g_ui_ctx.select == 10)
		{
			g_ui_ctx.num[g_ui_ctx.select] = ++(g_ui_ctx.num[g_ui_ctx.select]) > 5 ? 0 : g_ui_ctx.num[g_ui_ctx.select];
		}
		//设置分钟 秒的个位
		else if(g_ui_ctx.select == 9 || g_ui_ctx.select == 11)
		{
			g_ui_ctx.num[g_ui_ctx.select] = ++(g_ui_ctx.num[g_ui_ctx.select]) > 9 ? 0 : g_ui_ctx.num[g_ui_ctx.select];
		}
	}
	else if(event == EN_KEY_SRC_CONFIRM)
	{
		g_ui_ctx.select = ((++g_ui_ctx.select) % DATA_DIGIT_NUMS);
		if(g_ui_ctx.select == 0 || g_ui_ctx.select == 1)
		{
			g_ui_ctx.select = 2;
		}
		_SetDataNumberColor(g_ui_ctx.select);
		
		WM_Paint(gEmWinListSetDataInfo.win_handle);
		return;
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
				pnext = &gEmWinListAdvancedSettings;
			}
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win();
		}
		return;
	}
	
	//获取首次得到的年、月、日
	year = _gNumTextPara[0].digit_value * 1000 + _gNumTextPara[1].digit_value * 100 + g_ui_ctx.num[2] * 10 + g_ui_ctx.num[3];
	month = g_ui_ctx.num[4];
	day = g_ui_ctx.num[5];
	//进行年月日的合法校验
	date_check = DateCheck(year,month,day,event);
	//将校验后的值重新赋值
	g_ui_ctx.num[2] = (date_check.date_year / 10) % 10;
	g_ui_ctx.num[3] = date_check.date_year % 10;
	g_ui_ctx.num[4] = date_check.date_month;
	g_ui_ctx.num[5] = date_check.date_day;
	
	_SetDataNumAdd(g_ui_ctx.select,g_ui_ctx.num[g_ui_ctx.select]);
}
/**
  * @brief  将当前选中数字的下方横线进行标亮
  * @param  index：当前选中的数字
  * @retval NULL
  */
void _DrawUnderLine(int32_t index)
{
	GUI_SetColor(0x00CDF100);
	switch(index)
	{
#if 0		
		case 0: 
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 1:
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
#endif
		case 2:
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 3: 
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 4:
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 24, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		case 5:
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 24, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;		
		case 6: 
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 7:
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		case 8:
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		case 9:
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		case 10:
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;		
		case 11: 
		{
			GUI_DrawLine(_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2,_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18,\
			_aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _aListReconnGridTimeDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
		}
		break;
		default: 
		break;
	}	
}

/**
  * @brief  设置选中数字文本的颜色
  * @param  index:当前选中的数字
  * @retval NULL
  */
void _SetDataNumberColor(int32_t index)
{
	//_SetDataNumAdd(index);
	uint8_t i = 0;
	static volatile uint8_t nums_sel_falg[DATA_DIGIT_NUMS] = {0};
	for(i = 0; i < DATA_DIGIT_NUMS; i++)
	{
		nums_sel_falg[i] = 0;
		TEXT_SetTextColor(_gTextNumHandle[i],0x00FFFFFF);
	}
	
	if(index > 1)
	{
		nums_sel_falg[index] = 1;
		TEXT_SetTextColor(_gTextNumHandle[index],0x00CDF100);
	}
	
}

/**
  * @brief  设置选中数字变化
  * @param  NULL
  * @retval NULL
  */
void _SetDataNumAdd(int32_t index, int32_t v)
{
	if(index < 2)
	{
		return;
	}
	_gNumTextPara[index].digit_value = v;
	//月份 日 显示两位
	if(index == 4 || index == 5)
	{
		sprintf(_gNumTextPara[index].p_content, "%02d", (int)_gNumTextPara[index].digit_value);
	}
	else
	{
		sprintf(_gNumTextPara[index].p_content, "%1d", (int)_gNumTextPara[index].digit_value);
	}
	TEXT_SetText(_gTextNumHandle[index],_gSetDataNumTextBuf[index]);
	
	//显示经过校验后的天数
	if(_gNumTextPara[5].digit_value >= 28 && index == 4)
	{
		_gNumTextPara[5].digit_value = date_check.date_day;
		sprintf(_gNumTextPara[5].p_content, "%02d", (int)_gNumTextPara[5].digit_value);
		TEXT_SetText(_gTextNumHandle[5],_gSetDataNumTextBuf[5]);
	}
}

/**
  * @brief  绘制选中框
  * @param  NULL
  * @retval NULL
  */
void SelectedDrawRectPaint(int32_t sel)
{
	if(sel == 0)
	{
		GUI_SetColor(0x00BDC909);
		GUI_DrawRoundedFrame(REC_XPOS, REC_YPOS,  REC_XPOS + REC_XSIZE - 1,  REC_YPOS + REC_YSIZE - 1, 10,3);
	}
	else if(sel == 1)
	{
		GUI_SetColor(0x00BDC909);
		GUI_DrawRoundedFrame(REC_XPOS, REC_YPOS + 42,  REC_XPOS + REC_XSIZE - 1,  REC_YPOS + 42 + REC_YSIZE - 1, 10,3);
	}
}


/**
  * @brief  判定时间的标准
  * @param  check_dir:判断其加计数还是减计数
  * @retval NULL
  */
Date_t DateCheck(uint16_t year,uint16_t month,uint16_t day,int32_t check_dir)
{
	Date_t ret = {0};
	
	if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 12)
	{

	}
	else if(month == 4 || month == 6 || month == 9 || month == 11)
	{
		if (check_dir == EN_KEY_SRC_RIGHT)
		{
			day = day > 30 ? 1:day;
		}
		else 
		{
			day = day > 30 ? 30: day;
		}
	}
	else if (month == 2)
	{
		// 闰年
		int r = ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
		if (check_dir == EN_KEY_SRC_RIGHT)
		{
			if (r)
			{
				day = day > 29 ? 1:day;
			}
			else 
			{
				day = day > 28 ? 1:day;
			}
		}
		else 
		{
			if (r)
			{
				day = day > 29 ? 29:day;
			}
			else 
			{
				day = day > 28 ? 28:day;
			}
		}
		
	}
	
	ret.date_year = year;
	ret.date_month = month;
	ret.date_day = day;
	
	return ret;
}



