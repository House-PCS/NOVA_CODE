/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_reconn_grid_time.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-02
  * @brief       高级设置->设置->more settings->battery->Voltage(充放电电压)
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-02    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
  
#define  TEST_DYNAMIC											0 
 
#define		EM_BACK_ON_ICON_ARROW_SIZE                		(4096)  
#define		DIGIT_NUMS										(6)  
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

emWin_t  gEmWinListAdvanSettingBatVoltage = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;

	int8_t num[DIGIT_NUMS];
	uint16_t charge_voltage;
	uint16_t discharge_voltage;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};



/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

/*Voltage列表*/
static TEXT_Handle  _gTextChargeVoltHandle;//显示charge Voltage句柄
static TEXT_Handle	_gTextDischargeVoltHandle;//discharge Voltage句柄
static TEXT_Handle	_gTextNumHandle[DIGIT_NUMS];
static TEXT_Handle	_gTextVoltOneHandle;
static TEXT_Handle  _gTextVoltTwoHandle;
static TEXT_Handle  _gTextPointOneHandle;
static TEXT_Handle  _gTextPointTwoHandle;

static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 
static emWidgetTextPara_t  _gChargeVoltTextPara;
static emWidgetTextPara_t  _gDischargeVoltTextPara;
static emWidgetTextPara_t  _gNumTextPara[DIGIT_NUMS];
static emWidgetTextPara_t  _gVoltOneTextPara;
static emWidgetTextPara_t  _gVoltTwoTextPara;
static emWidgetTextPara_t  _gPointOneTextPara;
static emWidgetTextPara_t  _gPointTwoTextPara;

static char _gTitleBuf[] = "Setings < More Settings < Battery < Voltage";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";
static char _gChargeBuf[] = "Charge Voltage";
static char _gDischargeBuf[] = "Discharge Voltage";
static char _gVoltOneBuf[] = "V";
static char _gVoltTwoBuf[] = "V";
static char _gPointOneBuf[] = ".";
static char _gPointTwoBuf[] = ".";
static char _gBatteryVoltageNumTextBuf[DIGIT_NUMS][20];


#define WIN_LIST_BAT_VOLT_ID                               	(GUI_ID_USER + 0x12)
#define WIN_LIST_BAT_VOLT_TITLE_ID                         	(GUI_ID_USER + 0x13)
#define WIN_LIST_BAT_VOLT_PRESS_ID					      	(GUI_ID_USER + 0x14)
#define WIN_LIST_BAT_VOLT_BACK_ID					      	(GUI_ID_USER + 0x15)
#define WIN_LIST_BAT_VOLT_BACK_IMG_ID					  	(GUI_ID_USER + 0x16)

#define WIN_MAIN_CHARGE_VOLT_ID							  	(GUI_ID_USER + 0x20)
#define WIN_MAIN_DISCHARGE_VOLT_ID						  	(GUI_ID_USER + 0x21)
#define WIN_MAIN_NUMBER1_ID							 		(GUI_ID_USER + 0x22)
#define WIN_MAIN_NUMBER2_ID							 		(GUI_ID_USER + 0x23)
#define WIN_MAIN_NUMBER3_ID							 		(GUI_ID_USER + 0x24)
#define WIN_MAIN_NUMBER4_ID							 		(GUI_ID_USER + 0x25)
#define WIN_MAIN_NUMBER5_ID							 		(GUI_ID_USER + 0x26)
#define WIN_MAIN_NUMBER6_ID							 		(GUI_ID_USER + 0x27)
#define WIN_MAIN_AMPERE_ONE_ID							 	(GUI_ID_USER + 0x28)
#define WIN_MAIN_AMPERE_TWO_ID							 	(GUI_ID_USER + 0x29)
#define WIN_MAIN_POINT_ONE_ID							 	(GUI_ID_USER + 0x2A)
#define WIN_MAIN_POINT_TWO_ID							 	(GUI_ID_USER + 0x2B)


/*资源列表*/
static const GUI_WIDGET_CREATE_INFO _aListBatVoltDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListDODWin",    	WIN_LIST_BAT_VOLT_ID,     				0,   	0,   	480,	320,	0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_BAT_VOLT_TITLE_ID,     		31,  	20,  	415,	14,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_BAT_VOLT_PRESS_ID,       		133, 	273, 	33,   	14,   	0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_BAT_VOLT_BACK_IMG_ID,    		171-5, 	275, 	15,   	10,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_BAT_VOLT_BACK_ID,       		188, 	273, 	150,  	14,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "cha_cur",		WIN_MAIN_CHARGE_VOLT_ID,				31, 	48, 	180,  	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "discha_cur",		WIN_MAIN_DISCHARGE_VOLT_ID,				31, 	90, 	180,  	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM1",			WIN_MAIN_NUMBER1_ID,       				390, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM1",			WIN_MAIN_NUMBER2_ID,       				406, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM2",			WIN_MAIN_NUMBER3_ID,       				422, 	47,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM3",			WIN_MAIN_NUMBER4_ID,       				390, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM4",			WIN_MAIN_NUMBER5_ID,       				406, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "NUM5",			WIN_MAIN_NUMBER6_ID,       				422, 	89,  	9,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "ampere1",		WIN_MAIN_AMPERE_ONE_ID,       			437,	48,  	15,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "ampere2",		WIN_MAIN_AMPERE_TWO_ID,       			437,	90,  	15,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "point1",			WIN_MAIN_POINT_ONE_ID,       			416,	48,  	5,    	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "point2",			WIN_MAIN_POINT_TWO_ID,       			416,	90,  	5,    	17,   	0,   0x0,  0 },

};

static int32_t _SetChargeAndDischVoltage(int32_t ui_update, uint16_t charge_vol, uint16_t discharge_vol)
{
	uint8_t i = 0;
	uint16_t ch_hundred  = (charge_vol / 100 ) % 10;
	uint16_t ch_tens     = (charge_vol / 10  ) % 10;
	uint16_t ch_uints    = (charge_vol % 10);
	
	uint16_t disch_hundred  = (discharge_vol / 100 ) % 10;
	uint16_t disch_tens     = (discharge_vol / 10  ) % 10;
	uint16_t disch_uints    = (discharge_vol % 10);
	
	snprintf(_gBatteryVoltageNumTextBuf[0],sizeof(_gBatteryVoltageNumTextBuf[0]),"%d", ch_hundred);
	snprintf(_gBatteryVoltageNumTextBuf[1],sizeof(_gBatteryVoltageNumTextBuf[1]),"%d", ch_tens);
	snprintf(_gBatteryVoltageNumTextBuf[2],sizeof(_gBatteryVoltageNumTextBuf[2]),"%d", ch_uints);
	
	snprintf(_gBatteryVoltageNumTextBuf[3],sizeof(_gBatteryVoltageNumTextBuf[3]),"%d", disch_hundred);
	snprintf(_gBatteryVoltageNumTextBuf[4],sizeof(_gBatteryVoltageNumTextBuf[4]),"%d", disch_tens);
	snprintf(_gBatteryVoltageNumTextBuf[5],sizeof(_gBatteryVoltageNumTextBuf[5]),"%d", disch_uints);

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
void _SetBatVoltNumberColor(int32_t index);
void _SetBatVoltSelectedNumAndLine(int32_t index);
void _SetBatVoltNumAdd(int32_t index,int32_t v);
void _DrawBatVoltNumUnderLine(int32_t index);
void _DrawSelBatVoltRecPaint(int32_t sel);

/**
  * @brief ListBatVoltWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListBatVoltWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BAT_VOLT_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
			//列表中的文字
			//charge voltage
			_gTextChargeVoltHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_CHARGE_VOLT_ID);
            TEXT_SetFont(_gTextChargeVoltHandle, _gChargeVoltTextPara.p_text_font);
            TEXT_SetText(_gTextChargeVoltHandle, (const char *)_gChargeVoltTextPara.p_content);
            TEXT_SetTextAlign(_gTextChargeVoltHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextChargeVoltHandle, _gChargeVoltTextPara.text_color); 
			//discharge voltage
			_gTextDischargeVoltHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_DISCHARGE_VOLT_ID);
            TEXT_SetFont(_gTextDischargeVoltHandle, _gDischargeVoltTextPara.p_text_font);
            TEXT_SetText(_gTextDischargeVoltHandle, (const char *)_gDischargeVoltTextPara.p_content);
            TEXT_SetTextAlign(_gTextDischargeVoltHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextDischargeVoltHandle, _gDischargeVoltTextPara.text_color); 
			
			//安培的显示
			//第一行电压
			_gTextVoltOneHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_AMPERE_ONE_ID);
            TEXT_SetFont(_gTextVoltOneHandle, _gVoltOneTextPara.p_text_font);
            TEXT_SetText(_gTextVoltOneHandle, (const char *)_gVoltOneTextPara.p_content);
            TEXT_SetTextAlign(_gTextVoltOneHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextVoltOneHandle, _gVoltOneTextPara.text_color);			
			//第二行电压
			_gTextVoltTwoHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_AMPERE_TWO_ID);
            TEXT_SetFont(_gTextVoltTwoHandle, _gVoltTwoTextPara.p_text_font);
            TEXT_SetText(_gTextVoltTwoHandle, (const char *)_gVoltTwoTextPara.p_content);
            TEXT_SetTextAlign(_gTextVoltTwoHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextVoltTwoHandle, _gVoltTwoTextPara.text_color);	
			
			//小数点的显示
			_gTextPointOneHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_POINT_ONE_ID);
			TEXT_SetFont(_gTextPointOneHandle, _gPointOneTextPara.p_text_font);
            TEXT_SetText(_gTextPointOneHandle, (const char *)_gPointOneTextPara.p_content);
            TEXT_SetTextAlign(_gTextPointOneHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPointOneHandle, _gPointOneTextPara.text_color);	
			
			_gTextPointTwoHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_POINT_TWO_ID);
			TEXT_SetFont(_gTextPointTwoHandle, _gPointTwoTextPara.p_text_font);
            TEXT_SetText(_gTextPointTwoHandle, (const char *)_gPointTwoTextPara.p_content);
            TEXT_SetTextAlign(_gTextPointTwoHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextPointTwoHandle, _gPointTwoTextPara.text_color);	
			
			for(int i = 0; i < DIGIT_NUMS; i++)
			{
				_gTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_NUMBER1_ID + i);
				TEXT_SetFont(_gTextNumHandle[i], _gNumTextPara[i].p_text_font);
				TEXT_SetText(_gTextNumHandle[i], (const char *)_gNumTextPara[i].p_content);
				TEXT_SetTextAlign(_gTextNumHandle[i], TEXT_CF_RIGHT | TEXT_CF_VCENTER);
				TEXT_SetTextColor(_gTextNumHandle[i], _gNumTextPara[i].text_color); 	
			}
		
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BAT_VOLT_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BAT_VOLT_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BAT_VOLT_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
			
			_SetBatVoltNumberColor(g_ui_ctx.select);
			
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
			_SetBatVoltNumberColor(index);
			
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
			for(int i = 0; i < DIGIT_NUMS; i++)
			{
				GUI_DrawLine(_aListBatVoltDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2,_aListBatVoltDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18,
							 _aListBatVoltDialogCreate[i + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListBatVoltDialogCreate[i + DIALOG_NUM_OFFSET].y0 + 18);
			}
			
			
			
#if TEST_DYNAMIC
			_DrawSelBatVoltRecPaint(selected%2);
			_DrawBatVoltNumUnderLine(index);
#else
			_DrawSelBatVoltRecPaint(g_ui_ctx.select / (DIGIT_NUMS / 2));
			_DrawBatVoltNumUnderLine(g_ui_ctx.select);
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
  * @brief Create Factory Restore win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateFactoryRestoreWin()
{
    gEmWinListAdvanSettingBatVoltage.win_handle = GUI_CreateDialogBox(_aListBatVoltDialogCreate, 
											  GUI_COUNTOF(_aListBatVoltDialogCreate), 
											  _cbListBatVoltWin, 
											  pCurrentEmWin->win_handle, 
											  0, 
											  0);
    WM_SetTransState(gEmWinListAdvanSettingBatVoltage.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    return gEmWinListAdvanSettingBatVoltage.win_handle;
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
    gEmWinListAdvanSettingBatVoltage.win_handle = GUI_CreateDialogBox(_aListBatVoltDialogCreate, GUI_COUNTOF(_aListBatVoltDialogCreate), _cbListBatVoltWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListAdvanSettingBatVoltage.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListAdvanSettingBatVoltage.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListAdvanSettingBatVoltage.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListDODPara, gEmWinListAdvanSettingBatVoltage.win_handle);
    return gEmWinListAdvanSettingBatVoltage.win_handle;
	
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListAdvanSettingBatVoltage.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListAdvanSettingBatVoltage.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListAdvanSettingBatVoltage.win_handle = 0;
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
	//charge cur
	_gChargeVoltTextPara.p_text_font = EM_TEXT_FONT_5;
	_gChargeVoltTextPara.text_color = 0x00FFFFFF;
	_gChargeVoltTextPara.p_content = _gChargeBuf;
	//discharge cur
	_gDischargeVoltTextPara.p_text_font = EM_TEXT_FONT_5;
	_gDischargeVoltTextPara.text_color = 0x00FFFFFF;
	_gDischargeVoltTextPara.p_content = _gDischargeBuf;
	//Voltage1 
	_gVoltOneTextPara.p_text_font = EM_TEXT_FONT_5;
	_gVoltOneTextPara.text_color = 0x00FFFFFF;
	_gVoltOneTextPara.p_content = _gVoltOneBuf;
	//Voltage2
	_gVoltTwoTextPara.p_text_font = EM_TEXT_FONT_5;
	_gVoltTwoTextPara.text_color = 0x00FFFFFF;
	_gVoltTwoTextPara.p_content = _gVoltTwoBuf;

	//Point1 
	_gPointOneTextPara.p_text_font = EM_TEXT_FONT_5;
	_gPointOneTextPara.text_color = 0x00FFFFFF;
	_gPointOneTextPara.p_content = _gPointOneBuf;
	//Point2
	_gPointTwoTextPara.p_text_font = EM_TEXT_FONT_5;
	_gPointTwoTextPara.text_color = 0x00FFFFFF;
	_gPointTwoTextPara.p_content = _gPointTwoBuf;
	
	for(uint8_t i = 0; i < DIGIT_NUMS; i++)
	{
		_gNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
		_gNumTextPara[i].text_color = 0x00FFFFFF;
		_gNumTextPara[i].p_content = _gBatteryVoltageNumTextBuf[i];
//		_gNumTextPara[i].digit_value = 0;
//		sprintf(_gNumTextPara[i].p_content, "%1d", (int)_gNumTextPara[i].digit_value);
	}

#if 0
	_SetChargeAndDischVoltage(gCollectParaData,0);
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
	if(g_ui_ctx.charge_voltage != gCollectParaData. || g_ui_ctx.discharge_voltage != gCollectParaData.)
	{
		g_ui_ctx.charge_voltage = gCollectParaData;
		g_ui_ctx.discharge_voltage = gCollectParaData;
		_SetChargeAndDischVoltage(g_ui_ctx.charge_voltage, g_ui_ctx.discharge_voltage, 1);
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
		_SetBatVoltNumAdd(g_ui_ctx.select,g_ui_ctx.num[g_ui_ctx.select]);
		return;
	}
	else if(event == EN_KEY_SRC_RIGHT)
	{
		if(g_ui_ctx.select >= DIGIT_NUMS)
		{
			return;
		}
		g_ui_ctx.num[g_ui_ctx.select] = (++(g_ui_ctx.num[g_ui_ctx.select]))%10;
		_SetBatVoltNumAdd(g_ui_ctx.select,g_ui_ctx.num[g_ui_ctx.select]);
		return;
	}
	else if(event == EN_KEY_SRC_CONFIRM)
	{
		g_ui_ctx.select = (++g_ui_ctx.select) % DIGIT_NUMS;
		_SetBatVoltNumberColor(g_ui_ctx.select);

		WM_Paint(gEmWinListAdvanSettingBatVoltage.win_handle);
		
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
void _SetBatVoltNumberColor(int32_t index)
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
void _SetBatVoltNumAdd(int32_t index, int32_t v)
{
	_gNumTextPara[index].digit_value = v;
	sprintf(_gNumTextPara[index].p_content, "%1d", (int)_gNumTextPara[index].digit_value);
	if(9 < _gNumTextPara[index].digit_value)
	{
		_gNumTextPara[index].digit_value = 0;
	}
	TEXT_SetText(_gTextNumHandle[index],_gBatteryVoltageNumTextBuf[index]);
}

/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
void _DrawBatVoltNumUnderLine(int32_t index)
{
	GUI_SetColor(0x00CDF100);
	switch(index)
	{
		case 0:
		{
			GUI_DrawLine(_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 1:
		{
			GUI_DrawLine(_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 2:
		{
			GUI_DrawLine(_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 3:
		{
			GUI_DrawLine(_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 4:
		{
			GUI_DrawLine(_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
		}
		break;
		case 5:
		{
			GUI_DrawLine(_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18,
						 _aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].x0 - 2 + 12,_aListBatVoltDialogCreate[index + DIALOG_NUM_OFFSET].y0 + 18);
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
void _DrawSelBatVoltRecPaint(int32_t sel)
{
	if(sel == 0)
	{
		GUI_SetColor(0x00BDC909);
		GUI_DrawRoundedFrame(REC1_XPOS, REC1_YPOS,  REC1_XPOS + 440 - 1,  REC1_YPOS + 34 - 1, 10,3);
	}
	else if(sel == 1 || sel == 2)
	{
		GUI_SetColor(0x00BDC909);
		GUI_DrawRoundedFrame(REC2_XPOS, REC2_YPOS,  REC2_XPOS + 440 - 1,  REC2_YPOS + 34 - 1, 10,3);
	}
	return;        
}

