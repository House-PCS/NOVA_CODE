/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opt_parm_ongrid_power.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-19
  * @brief       高级设置->设置->more settings->Opt PARM->React Mode（反应模式）
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-19    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
  
#define  TEST_DYNAMIC										0 
 
#define		EM_BACK_ON_ICON_ARROW_SIZE                		(4096)  
#define		DIGIT_NUMS										(10)  
#define		REC1_XPOS										(20)
#define 	REC1_YPOS										(38)
#define		REC2_XPOS										(20)
#define 	REC2_YPOS										(80)
#define     DIALOG_NUM_OFFSET								(7)
 
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gEmWinListAdvanSettingOptReactMode = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;

	int8_t num[DIGIT_NUMS];
	uint16_t L_power_factor;
	uint16_t C_power_factor;
	uint16_t reactive_power;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};



/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

/*Power列表*/
static TEXT_Handle  _gTextPowerFactorHandle;//显示PowerFactor句柄
static TEXT_Handle	_gTextReactivePowerHandle;//ReactivePower句柄
static TEXT_Handle	_gTextNumHandle[DIGIT_NUMS];
static TEXT_Handle	_gTextPowerOneHandle;
static TEXT_Handle	_gTextLetterLHandle;
static TEXT_Handle  _gTextLetterCHandle;
static TEXT_Handle  _gTextPointOneHandle;
static TEXT_Handle  _gTextPointTwoHandle;
static TEXT_Handle  _gTextLineHandle;

static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 
static emWidgetTextPara_t  _gPowerFactorTextPara;
static emWidgetTextPara_t  _gReactivePowerTextPara;
static emWidgetTextPara_t  _gLetterLTextPara;
static emWidgetTextPara_t  _gLetterCTextPara;
static emWidgetTextPara_t  _gPointOneTextPara;
static emWidgetTextPara_t  _gPointTwoTextPara;
static emWidgetTextPara_t  _gLineTextPara;
static emWidgetTextPara_t  _gNumTextPara[DIGIT_NUMS];
static emWidgetTextPara_t  _gPowerOneTextPara;

static char _gTitleBuf[] = "Settings < More Settings < Opt PARM < React Mode";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";
static char _gPowerFactorBuf[] = "Power Factor";
static char _gReactivePowerBuf[] = "Reactive Power";
static char _gLetterLBuf[] = "L";
static char _gLetterCBuf[] = "C";
static char _gPointBuf[] = ".";
static char _gLineBuf[] = "-";
static char _gAmpereOneBuf[] = "W";


static char _gReactModeNumTextBuf[DIGIT_NUMS][20];


#define WIN_LIST_ON_GRID_POWER_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_ON_GRID_POWER_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_ON_GRID_POWER_PRESS_ID					      	(GUI_ID_USER + 0x14)
#define WIN_LIST_ON_GRID_POWER_BACK_ID					      	(GUI_ID_USER + 0x15)
#define WIN_LIST_ON_GRID_POWER_BACK_IMG_ID					  	(GUI_ID_USER + 0x16)

#define WIN_MAIN_POWER_FACTOR_ID								(GUI_ID_USER + 0x20)
#define WIN_MAIN_REACTIVE_POWER_ID								(GUI_ID_USER + 0x21)
#define WIN_MAIN_NUMBER1_ID							 			(GUI_ID_USER + 0x22)
#define WIN_MAIN_NUMBER2_ID							 			(GUI_ID_USER + 0x23)
#define WIN_MAIN_NUMBER3_ID							 			(GUI_ID_USER + 0x24)
#define WIN_MAIN_NUMBER4_ID							 			(GUI_ID_USER + 0x25)
#define WIN_MAIN_NUMBER5_ID							 			(GUI_ID_USER + 0x26)
#define WIN_MAIN_NUMBER6_ID							 			(GUI_ID_USER + 0x27)
#define WIN_MAIN_NUMBER7_ID							 			(GUI_ID_USER + 0x28)
#define WIN_MAIN_NUMBER8_ID							 			(GUI_ID_USER + 0x29)
#define WIN_MAIN_NUMBER9_ID							 			(GUI_ID_USER + 0x2A)
#define WIN_MAIN_NUMBER10_ID							 		(GUI_ID_USER + 0x2B)
#define WIN_MAIN_LETTER_L_ID									(GUI_ID_USER + 0x2C)
#define WIN_MAIN_LETTER_C_ID									(GUI_ID_USER + 0x2D)
#define WIN_MAIN_POINT_ONE_ID									(GUI_ID_USER + 0x2E)
#define WIN_MAIN_POINT_TWO_ID									(GUI_ID_USER + 0x2F)
#define WIN_MAIN_LINE_ID										(GUI_ID_USER + 0x30)
#define WIN_MAIN_POWER_ONE_ID							 		(GUI_ID_USER + 0x31)




/*资源列表*/
static const GUI_WIDGET_CREATE_INFO _aListReactModeDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListDODWin",    	WIN_LIST_ON_GRID_POWER_ID,     				0,   	0,   	480,	320,	0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_ON_GRID_POWER_TITLE_ID,     			31,  	20,  	415,	14,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_ON_GRID_POWER_PRESS_ID,       		133, 	273, 	33,   	14,   	0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_ON_GRID_POWER_BACK_IMG_ID,    		171-5, 	275, 	15,   	10,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_ON_GRID_POWER_BACK_ID,       		188, 	273, 	150,  	14,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "factor",			WIN_MAIN_POWER_FACTOR_ID,					31, 	48, 	180,  	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "reactive",		WIN_MAIN_REACTIVE_POWER_ID,					31, 	90, 	180,  	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM1",			WIN_MAIN_NUMBER1_ID,       				327, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM2",			WIN_MAIN_NUMBER2_ID,       				343, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM3",			WIN_MAIN_NUMBER3_ID,       				359, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM4",			WIN_MAIN_NUMBER4_ID,       				406, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM5",			WIN_MAIN_NUMBER5_ID,       				422, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM6",			WIN_MAIN_NUMBER6_ID,       				438, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM7",			WIN_MAIN_NUMBER7_ID,       				374, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM8",			WIN_MAIN_NUMBER8_ID,       				390, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM9",			WIN_MAIN_NUMBER9_ID,       				406, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM10",			WIN_MAIN_NUMBER10_ID,       			422, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "power",			WIN_MAIN_POWER_ONE_ID,       			437,	89,  	15,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "POINT1",			WIN_MAIN_POINT_ONE_ID,       			337, 	49,  	5,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "POINT2",			WIN_MAIN_POINT_TWO_ID,       			416, 	49,  	5,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "L",				WIN_MAIN_LETTER_L_ID,       			314, 	47,  	15,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "C",				WIN_MAIN_LETTER_C_ID,       			389,	47,  	15,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "line",			WIN_MAIN_LINE_ID,       				377,	47,  	9,    	17,   	0,   0x0,  0 },
};

static int32_t _SetReactModePowerFactor(int32_t ui_update, uint16_t L_power, uint16_t C_power)
{
	uint16_t i = 0;
	uint16_t L_hundred  = (L_power / 100) % 10;
	uint16_t L_tens     = (L_power / 10)  % 10;
	uint16_t L_uints    = (L_power % 10);
 	
	uint16_t C_hundred  = (C_power / 100) % 10;
	uint16_t C_tens     = (C_power / 10)  % 10;
	uint16_t C_uints    = (C_power % 10);
	
	snprintf(_gReactModeNumTextBuf[0], sizeof(_gReactModeNumTextBuf[0]), "%d", L_hundred);
	snprintf(_gReactModeNumTextBuf[1], sizeof(_gReactModeNumTextBuf[1]), "%d", L_tens);
	snprintf(_gReactModeNumTextBuf[2], sizeof(_gReactModeNumTextBuf[2]), "%d", L_uints);
	
	snprintf(_gReactModeNumTextBuf[3], sizeof(_gReactModeNumTextBuf[3]), "%d", C_hundred);
	snprintf(_gReactModeNumTextBuf[4], sizeof(_gReactModeNumTextBuf[4]), "%d", C_tens);
	snprintf(_gReactModeNumTextBuf[5], sizeof(_gReactModeNumTextBuf[5]), "%d", C_uints );

	if(ui_update)
	{		
		for(i = 0; i < 6; ++i)
		{
			TEXT_SetText(_gTextNumHandle[i],(const char *)_gNumTextPara[i].p_content);
		}
	}

	return 0;
}


static int32_t _SetReactModeReactivePower(int32_t ui_update, uint16_t power)
{
	uint16_t i = 0;
	uint16_t thousand =  power / 1000;
	uint16_t hundred  = (power / 100) % 10;
	uint16_t ten 	  = (power / 10)  % 10;
	uint16_t uint     =  power % 10;

	snprintf(_gReactModeNumTextBuf[6], sizeof(_gReactModeNumTextBuf[6]), "%d", thousand);
	snprintf(_gReactModeNumTextBuf[7], sizeof(_gReactModeNumTextBuf[7]), "%d", hundred );
	snprintf(_gReactModeNumTextBuf[8], sizeof(_gReactModeNumTextBuf[8]), "%d", ten     );
	snprintf(_gReactModeNumTextBuf[9], sizeof(_gReactModeNumTextBuf[9]), "%d", uint    );
	
	if(ui_update)
	{
		for(i = 6; i < DIGIT_NUMS; ++i)
		{
			TEXT_SetText(_gTextNumHandle[i],(const char *)_gNumTextPara[i].p_content);
		}
	}
	
	return 0;
}





/*函数声明*/
void _SetReactModeNumberColor(int32_t index);
void _SetReactModeNumAdd(int32_t index,int32_t v);
void _DrawReactModeNumUnderLine(int32_t index);
void _DrawSelReactModeRecPaint(int32_t sel);
/**
  * @brief ListReactModeWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListReactModeWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
	static int32_t num = -1;
	static int32_t index = -1;
	static int32_t selected = -1;
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ON_GRID_POWER_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
			//列表中的文字
			//Power Factor
			_gTextPowerFactorHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_POWER_FACTOR_ID);
            TEXT_SetFont(_gTextPowerFactorHandle, _gPowerFactorTextPara.p_text_font);
            TEXT_SetText(_gTextPowerFactorHandle, (const char *)_gPowerFactorTextPara.p_content);
            TEXT_SetTextAlign(_gTextPowerFactorHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPowerFactorHandle, _gPowerFactorTextPara.text_color); 
			//Reactive Power
			_gTextReactivePowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_REACTIVE_POWER_ID);
            TEXT_SetFont(_gTextReactivePowerHandle, _gReactivePowerTextPara.p_text_font);
            TEXT_SetText(_gTextReactivePowerHandle, (const char *)_gReactivePowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextReactivePowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextReactivePowerHandle, _gReactivePowerTextPara.text_color);
			//字母L显示
			_gTextLetterLHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_LETTER_L_ID);
            TEXT_SetFont(_gTextLetterLHandle, _gLetterLTextPara.p_text_font);
            TEXT_SetText(_gTextLetterLHandle, (const char *)_gLetterLTextPara.p_content);
            TEXT_SetTextAlign(_gTextLetterLHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLetterLHandle, _gLetterLTextPara.text_color);
			//字母C显示
			_gTextLetterCHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_LETTER_C_ID);
            TEXT_SetFont(_gTextLetterCHandle, _gLetterCTextPara.p_text_font);
            TEXT_SetText(_gTextLetterCHandle, (const char *)_gLetterCTextPara.p_content);
            TEXT_SetTextAlign(_gTextLetterCHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLetterCHandle, _gLetterCTextPara.text_color);
			//横线的显示
			_gTextLineHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_LINE_ID);
            TEXT_SetFont(_gTextLineHandle, _gLineTextPara.p_text_font);
            TEXT_SetText(_gTextLineHandle, (const char *)_gLineTextPara.p_content);
            TEXT_SetTextAlign(_gTextLineHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextLineHandle, _gLineTextPara.text_color);
			//第一个小数点的显示
			_gTextPointOneHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_POINT_ONE_ID);
            TEXT_SetFont(_gTextPointOneHandle, _gPointOneTextPara.p_text_font);
            TEXT_SetText(_gTextPointOneHandle, (const char *)_gPointOneTextPara.p_content);
            TEXT_SetTextAlign(_gTextPointOneHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPointOneHandle, _gPointOneTextPara.text_color);
			//第二个小数点的显示
			_gTextPointTwoHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_POINT_TWO_ID);
            TEXT_SetFont(_gTextPointTwoHandle, _gPointTwoTextPara.p_text_font);
            TEXT_SetText(_gTextPointTwoHandle, (const char *)_gPointTwoTextPara.p_content);
            TEXT_SetTextAlign(_gTextPointTwoHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPointTwoHandle, _gPointTwoTextPara.text_color);
			
			
			//功率的显示
			//第二行功率单位显示
			_gTextPowerOneHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_POWER_ONE_ID);
            TEXT_SetFont(_gTextPowerOneHandle, _gPowerOneTextPara.p_text_font);
            TEXT_SetText(_gTextPowerOneHandle, (const char *)_gPowerOneTextPara.p_content);
            TEXT_SetTextAlign(_gTextPowerOneHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPowerOneHandle, _gPowerOneTextPara.text_color);	
			
			for(int i = 0; i < DIGIT_NUMS; i++)
			{
				_gTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_NUMBER1_ID + i);
				TEXT_SetFont(_gTextNumHandle[i], _gNumTextPara[i].p_text_font);
				TEXT_SetText(_gTextNumHandle[i], (const char *)_gNumTextPara[i].p_content);
				TEXT_SetTextAlign(_gTextNumHandle[i], TEXT_CF_RIGHT | TEXT_CF_VCENTER);
				TEXT_SetTextColor(_gTextNumHandle[i], _gNumTextPara[i].text_color); 	
			}
		
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ON_GRID_POWER_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ON_GRID_POWER_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ON_GRID_POWER_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
			
			_SetReactModeNumberColor(g_ui_ctx.select);
			
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
			_SetReactModeNumberColor(index);
			
			WM_Paint(pMsg->hWin);
            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
		}
		
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListDODPara, _gEmListDODPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
			GUI_FillRoundedRect(REC1_XPOS, REC1_YPOS,  REC1_XPOS + 440 - 1,  REC1_YPOS + 34 - 1, 10);
			GUI_FillRoundedRect(REC2_XPOS, REC2_YPOS,  REC2_XPOS + 440 - 1,  REC2_YPOS + 34 - 1, 10);
			
			GUI_SetPenSize(3);
			GUI_SetColor(0x00D9D9D9);
			//绘制数字底部横线
			for(int i = 0; i < DIGIT_NUMS; i++)
			{
				GUI_DrawLine(_aListReactModeDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18,
							 _aListReactModeDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18);
			}
			
			
			
#if TEST_DYNAMIC
			_DrawSelReactModeRecPaint(selected%2);
			_DrawReactModeNumUnderLine(index);
#else
			_DrawSelReactModeRecPaint(g_ui_ctx.select / ((DIGIT_NUMS + 2) / 2));
			_DrawReactModeNumUnderLine(g_ui_ctx.select);
#endif
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
  * @brief Create current win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWin(void)
{
	g_ui_ctx.key_init = 0;
	pPrevEmWin = &gEmWinListOptPARMInfo;
	
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListAdvanSettingOptReactMode.win_handle = GUI_CreateDialogBox(_aListReactModeDialogCreate, GUI_COUNTOF(_aListReactModeDialogCreate), _cbListReactModeWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListAdvanSettingOptReactMode.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListAdvanSettingOptReactMode.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListAdvanSettingOptReactMode.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListDODPara, gEmWinListAdvanSettingOptReactMode.win_handle);
    return gEmWinListAdvanSettingOptReactMode.win_handle;
	
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListAdvanSettingOptReactMode.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListAdvanSettingOptReactMode.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListAdvanSettingOptReactMode.win_handle = 0;
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
	//power factor
	_gPowerFactorTextPara.p_text_font = EM_TEXT_FONT_5;
	_gPowerFactorTextPara.text_color = 0x00FFFFFF;
	_gPowerFactorTextPara.p_content = _gPowerFactorBuf;
	//reactive power
	_gReactivePowerTextPara.p_text_font = EM_TEXT_FONT_5;
	_gReactivePowerTextPara.text_color = 0x00FFFFFF;
	_gReactivePowerTextPara.p_content = _gReactivePowerBuf;
	
	//字母L
	_gLetterLTextPara.p_text_font = EM_TEXT_FONT_5;
	_gLetterLTextPara.text_color = 0x00FFFFFF;
	_gLetterLTextPara.p_content = _gLetterLBuf;
	//字母C
	_gLetterCTextPara.p_text_font = EM_TEXT_FONT_5;
	_gLetterCTextPara.text_color = 0x00FFFFFF;
	_gLetterCTextPara.p_content = _gLetterCBuf;
	//小数点1
	_gPointOneTextPara.p_text_font = EM_TEXT_FONT_5;
	_gPointOneTextPara.text_color = 0x00FFFFFF;
	_gPointOneTextPara.p_content = _gPointBuf;
	//小数点2
	_gPointTwoTextPara.p_text_font = EM_TEXT_FONT_5;
	_gPointTwoTextPara.text_color = 0x00FFFFFF;
	_gPointTwoTextPara.p_content = _gPointBuf;
	//横线
	_gLineTextPara.p_text_font = EM_TEXT_FONT_5;
	_gLineTextPara.text_color = 0x00FFFFFF;
	_gLineTextPara.p_content = _gLineBuf;
	
	//power1 
	_gPowerOneTextPara.p_text_font = EM_TEXT_FONT_5;
	_gPowerOneTextPara.text_color = 0x00FFFFFF;
	_gPowerOneTextPara.p_content = _gAmpereOneBuf;

	
	for(uint8_t i = 0; i < DIGIT_NUMS; i++)
	{
		_gNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
		_gNumTextPara[i].text_color = 0x00FFFFFF;
		_gNumTextPara[i].p_content = _gReactModeNumTextBuf[i];
//		_gNumTextPara[i].digit_value = 0;
//		sprintf(_gNumTextPara[i].p_content, "%1d", (int)_gNumTextPara[i].digit_value);
	}

#if 1
	_SetReactModePowerFactor(0,gCollectParaData.pcs_hold_data.power_factor_setting_value,gCollectParaData.pcs_hold_data.power_factor_setting_value);	
	_SetReactModeReactivePower(0,gCollectParaData.pcs_hold_data.reactive_power_setting_value);
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
#if 1
	if(g_ui_ctx.L_power_factor != gCollectParaData.pcs_hold_data.power_factor_setting_value || g_ui_ctx.C_power_factor != gCollectParaData.pcs_hold_data.power_factor_setting_value)
	{
		g_ui_ctx.L_power_factor = gCollectParaData.pcs_hold_data.power_factor_setting_value;
		g_ui_ctx.C_power_factor = gCollectParaData.pcs_hold_data.power_factor_setting_value;
		
		_SetReactModePowerFactor(1,g_ui_ctx.L_power_factor, g_ui_ctx.C_power_factor);
	}
	
	if(g_ui_ctx.reactive_power != gCollectParaData.pcs_hold_data.reactive_power_setting_value)
	{
		g_ui_ctx.reactive_power = gCollectParaData.pcs_hold_data.reactive_power_setting_value;
		_SetReactModeReactivePower(1,g_ui_ctx.reactive_power);
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
		_SetReactModeNumAdd(g_ui_ctx.select,g_ui_ctx.num[g_ui_ctx.select]);
		return;
	}
	else if(event == EN_KEY_SRC_RIGHT)
	{
		if(g_ui_ctx.select >= DIGIT_NUMS)
		{
			return;
		}
		g_ui_ctx.num[g_ui_ctx.select] = (++(g_ui_ctx.num[g_ui_ctx.select]))%10;
		_SetReactModeNumAdd(g_ui_ctx.select,g_ui_ctx.num[g_ui_ctx.select]);
		return;
	}
	else if(event == EN_KEY_SRC_CONFIRM)
	{
		g_ui_ctx.select = (++g_ui_ctx.select) % DIGIT_NUMS;
		_SetReactModeNumberColor(g_ui_ctx.select);

		WM_Paint(gEmWinListAdvanSettingOptReactMode.win_handle);
		
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
				pnext = &gEmWinListOptPARMInfo;
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
void _SetReactModeNumberColor(int32_t index)
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
void _SetReactModeNumAdd(int32_t index, int32_t v)
{
	_gNumTextPara[index].digit_value = v;
	sprintf(_gNumTextPara[index].p_content, "%1d", (int)_gNumTextPara[index].digit_value);
	if(9 < _gNumTextPara[index].digit_value)
	{
		_gNumTextPara[index].digit_value = 0;
	}
	TEXT_SetText(_gTextNumHandle[index],_gReactModeNumTextBuf[index]);
}

/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
void _DrawReactModeNumUnderLine(int32_t index)
{
	GUI_SetColor(0x00CDF100);
	switch(index)
	{
		case 0:
		{
			GUI_DrawLine(_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 1:
		{
			GUI_DrawLine(_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 2:
		{
			GUI_DrawLine(_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 3:
		{
			GUI_DrawLine(_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 4:
		{
			GUI_DrawLine(_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 5:
		{
			GUI_DrawLine(_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 6:
		{
			GUI_DrawLine(_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 7:
		{
			GUI_DrawLine(_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 8:
		{
			GUI_DrawLine(_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 9:
		{
			GUI_DrawLine(_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListReactModeDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		default:
			break;
	}
}
/**
  * @brief  绘制选中框
  * @param  NULL
  * @retval NULL
  */
void _DrawSelReactModeRecPaint(int32_t sel)
{
	if(sel == 0)
	{
		GUI_SetColor(0x00BDC909);
		GUI_DrawRoundedFrame(REC1_XPOS, REC1_YPOS,  REC1_XPOS + 440 - 1,  REC1_YPOS + 34 - 1, 10,3);
	}
	else if(sel == 1)
	{
		GUI_SetColor(0x00BDC909);
		GUI_DrawRoundedFrame(REC2_XPOS, REC2_YPOS,  REC2_XPOS + 440 - 1,  REC2_YPOS + 34 - 1, 10,3);
	}
	return;          
}

