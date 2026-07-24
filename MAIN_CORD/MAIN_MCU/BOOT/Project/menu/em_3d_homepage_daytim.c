/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_3d_homepage.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-01-20
  * @brief       3D版-首页白天版本
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-20    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"
#include "em_main.h"
#include <time.h>

#define LAYOUT_DEBUG								1
#define DEBUG_TIMER									1

#define EM_HOMEPAGE_DAYTIME_ICON_BACKGROUND_SIZE                (452 * 1024)

#if 0
#define EM_MAIN3D_NAV_SELECT_SIZE					(4 * 1024)
#define EM_MAIN3D_NAV_DEV_STAT_SIZE					(4 * 1024 * 8)
#define EM_MAIN3D_NAV_SETTING_SIZE					(4 * 1024 * 8)
#define EM_MAIN3D_NAV_DEV_INFO_SIZE					(4 * 1024 * 8)
#define EM_MAIN3D_NAV_FACTORY_RESTORE_SIZE			(4 * 1024 * 8)
#endif

#define EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE				(4 * 1024)
#define EM_HOME_PAGE_STATE_BAR_WARNING_ICON_SIZE			(4 * 1024)

#define EM_HOME_PAGE_BOTTOM_HOME_ACTIVE_ICON_SIZE			(4 * 1024 * 16)
#define EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE			(4 * 1024 * 6)
#define EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE			(4 * 1024)

#define EM_HOME_PAGE_BOTTOM_TEMP_ICON_SIZE					(4096)
#define EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE				(4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gEmWinHomePageDayTime = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;

	uint32_t switcher[3];
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};



/* 背景图内存 */
static GUI_HMEM _gMemBK;
static uint8_t *_pBKBuf;


/*各个子窗口控件句柄及其他相关参数*/
/*顶部状态栏子窗口*/
static GUI_HWIN _gWinStateBarHandle;//状态栏句柄
static IMAGE_Handle _gImgStateBarWifiHandle;//状态栏WIFI图标句柄
static IMAGE_Handle _gImgStateBarWarningHandle;//状态栏警告图标句柄
static TEXT_Handle  _gTextStateBarTimeHandle;//状态栏时间句柄
static TEXT_Handle  _gTextStateBarWorkMode1Handle;//状态栏工作模式句柄
static TEXT_Handle  _gTextStateBarWorkMode2Handle;//状态栏工作模式句柄
static TEXT_Handle  _gTextStateBarWarningHandle; //状态栏警告文字句柄

/*状态栏窗口文字参数*/
static emWidgetImgPara_t  _gStateBarWifiImgPara[4];		// 0:disconnect, 1:connect
static emWidgetImgPara_t  _gStateBarWarningImgPara[2];		// 0:WARNING, 1ERROR:
static emWidgetTextPara_t  _gStateBarTimeTextPara;
static emWidgetTextPara_t  _gStateBarWorkMode1TextPara;
static emWidgetTextPara_t  _gStateBarWorkMode2TextPara;
static emWidgetTextPara_t  _gStateBarWarningTextPara;
static char _gStateBarTimeTextBuf[80] = {0}; // 2024 Jan 1th 00:00:01
static char _gStateBarWorkMode1TextBuf[32] = {0}; 
static char _gStateBarWorkMode2TextBuf[32] = {0}; 
static char _gStateBarWarningTextBuf[8] = {0}; 


/*底部控件子窗口*/
static GUI_HWIN _gWinBottomHandle;//底部子窗口句柄
static IMAGE_Handle _gImgHomeActiveHandle;//底部流线图1图标句柄
static IMAGE_Handle _gImgGridActiveHandle;//底部流线图2图标句柄
static IMAGE_Handle _gImgSolarActiveHandle;//底部流线图3图标句柄
/*底部左1*/
static TEXT_Handle _gTextBottomHomeHandle; //底部左1HOME句柄
static TEXT_Handle _gTextBottomHomePowerHandle; //底部左1HOME 功率句柄
/*底部中上*/
static TEXT_Handle _gTextBottomSolarHandle;
static TEXT_Handle _gTextBottomSolarPowerHandle;
/*底部右中*/
static TEXT_Handle _gTextBottomGridHandle;
static TEXT_Handle _gTextBottomGridPowerHandle;
/*底部中间温度显示*/
static TEXT_Handle _gTextBottomTempHandle;
static IMAGE_Handle _gImgBottomTempHandle;//底部中间温度图标句柄
/*底部中下*/
static TEXT_Handle _gTextBottomBatteryHandle;
static IMAGE_Handle _gImgBottomBatteryHandle;//底部中下电池图标句柄 
static TEXT_Handle _gTextBottomRocaHandle;
static TEXT_Handle _gTextBottomRocaPowerHandle;
static TEXT_Handle _gTextBottomChargeHandle;


/*底部控件文字参数*/
static emWidgetImgPara_t  _gHomeActiveImgPara[2];	//0:active 1:disabled	
static emWidgetImgPara_t  _gGridActiveImgPara[3];	//0:in   1:out  2:disabled	
static emWidgetImgPara_t  _gSolarActiveImgPara[3];	//0:active 1:disabled 2:daytime
/*底部左1*/
static emWidgetTextPara_t _gBottomHomeTextPara;
static emWidgetTextPara_t _gBottomHomePowerTextPara;
/*底部中上*/
static emWidgetTextPara_t _gBottomSolarTextPara;
static emWidgetTextPara_t _gBottomSolarPowerTextPara;
/*底部右中*/
static emWidgetTextPara_t _gBottomGridTextPara;
static emWidgetTextPara_t _gBottomGridPowerTextPara;
/*底部中间温度显示*/
static emWidgetTextPara_t _gBottomTempTextPara;
static emWidgetImgPara_t  _gBottomTempImgPara;	
/*底部中下*/
static emWidgetTextPara_t _gBottomBatteryTextPara;
static emWidgetImgPara_t  _gBottomBatteryImgPara[4]; //0:100%  1：80%~99%   2：20%~79%  3:0%~19%	
static emWidgetTextPara_t _gBottomRocaTextPara;
static emWidgetTextPara_t _gBottomRocaPowerTextPara;
static emWidgetTextPara_t _gBottomChargeTextPara;

static char _gBottomHomeTextBuf[20] = {0}; 
static char _gBottomHomePowerTextBuf[20] = {0}; 
static char _gBottomSolarTextBuf[20] = {0}; 
static char _gBottomSolarPowerTextBuf[20] = {0}; 
static char _gBottomGridTextBuf[20] = {0}; 
static char _gBottomGridPowerTextBuf[20] = {0}; 
static char _gBottomTempTextBuf[20] = {0}; 
static char _gBottomBatteryTextBuf[20] = {0}; 
static char _gBottomRocaTextBuf[20] = {0}; 
static char _gBottomRocaPowerTextBuf[20] = {0}; 
static char _gBottomChargeTextBuf[20] = {0}; 


/*顶部状态栏控件ID号*/
#define WIN_MAIN_STATE_BAR_ID								(GUI_ID_USER + 0x10)
#define WIN_MAIN_STATE_BAR_WIFI_IMG_ID						(GUI_ID_USER + 0x11)
#define WIN_MAIN_STATE_BAR_WARNING_IMG_ID					(GUI_ID_USER + 0x12)
#define WIN_MAIN_STATE_BAR_TIME_TEXT_ID						(GUI_ID_USER + 0x13)
#define WIN_MAIN_STATE_BAR_WORK_MODE1_TEXT_ID				(GUI_ID_USER + 0x14)
#define WIN_MAIN_STATE_BAR_WORK_MODE2_TEXT_ID				(GUI_ID_USER + 0x15)
#define WIN_MAIN_STATE_BAR_WORK_MODE3_PEN_ID				(GUI_ID_USER + 0x16)
#define WIN_MAIN_STATE_BAR_WARNING_TEXT_ID					(GUI_ID_USER + 0x17)


/*底部控件ID号*/
#define WIN_HOME_PAGE_BOTTOM_HOME_ID						(GUI_ID_USER + 0x20)
#define WIN_HOME_PAGE_BOTTOM_HOME_POWER_ID					(GUI_ID_USER + 0x21)
#define WIN_HOME_PAGE_BOTTOM_SOLAR_ID						(GUI_ID_USER + 0x22)
#define WIN_HOME_PAGE_BOTTOM_SOLAR_POWER_ID					(GUI_ID_USER + 0x23)
#define WIN_HOME_PAGE_BOTTOM_GRID_ID						(GUI_ID_USER + 0x24)
#define WIN_HOME_PAGE_BOTTOM_GRID_POWER_ID					(GUI_ID_USER + 0x25)
#define WIN_HOME_PAGE_BOTTOM_TEMP_TEXT_ID					(GUI_ID_USER + 0x26)
#define	WIN_HOME_PAGE_BOTTOM_TEMP_IMG_ID					(GUI_ID_USER + 0x27)
#define WIN_HOME_PAGE_BOTTOM_BATTERY_ID						(GUI_ID_USER + 0x28)
#define WIN_HOME_PAGE_BOTTOM_BATTERY_IMG_ID					(GUI_ID_USER + 0x29)
#define WIN_HOME_PAGE_BOTTOM_ROCA_ID						(GUI_ID_USER + 0x2A)
#define	WIN_HOME_PAGE_BOTTOM_ROCA_POWER_ID					(GUI_ID_USER + 0x2B)
#define WIN_HOME_PAGE_BOTTOM_CHARGE_ID						(GUI_ID_USER + 0x2C)

#define WIN_HOME_PAGE_STREAM_LINE_ID						(GUI_ID_USER + 0x30)
#define WIN_HOME_PAGE_GRID_LINE_ID							(GUI_ID_USER + 0x31)
#define WIN_HOME_PAGE_SOLAR_LINE_ID							(GUI_ID_USER + 0x32)






/* layout 背景色*/
#define GUI_COLOR_WIN_MAIN_STAT_BAR_LAYOUT						GUI_MAKE_COLOR(0x001030A0)
#define GUI_COLOR_WIN_HOMEPAGE_STATE_BAR						GUI_MAKE_COLOR(0x00434343)
#define GUI_COLOR_WIN_HOMEPAGE_STATE_BAR_PEN					GUI_MAKE_COLOR(0x00797979)
#define GUI_COLOR_WIN_MAIN_STAT_BAR_WORK_MODE					GUI_MAKE_COLOR(0x00434343)
#define GUI_COLOR_WIN_MAIN_ELEC_STAT                        	GUI_MAKE_COLOR(0x00363A23)

/*函数声明*/
int32_t SetHomePageDayTimeWifiStat(WIFI_STAT_E stat);
int32_t SetHomePageDayTimeWarning(FAULT_STAT_E stat, uint32_t num);
int32_t SetHomePageDayTimePcsStat(PCS_STAT_E mode);
int32_t SetHomePageDayTimeWorkMode(WORK_MODE_E mode);
int32_t SetHomePageDayTimeTime(uint32_t t);
int32_t SetHomePowerDayTime(float value_kw);
int32_t SetSolarPowerDayTime(float value_kw);
int32_t SetGridPowerDayTime(float value_kw);
int32_t SetRocaPowerDayTime(float value_kw);
int32_t SetIsChargeDayTime(int32_t value_charge);
int32_t SetGridPowerstatDayTime(GRID_STAT_E stat);
int32_t SetHomePowerstatDayTime(HOME_STAT_E stat);
int32_t SetSolarPowerstatDayTime(SOLAR_STAT_E stat);
int32_t SetTempStaBarDayTime(float value_temp);


/*资源列表*/
/**
 * @brief 顶部状态栏
 */
static const GUI_WIDGET_CREATE_INFO _aStateBarDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "StatBar",		WIN_MAIN_STATE_BAR_ID, 							0, 0, EM_XSIZE,  58,  0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "time",			WIN_MAIN_STATE_BAR_TIME_TEXT_ID,       			15, 5,  120,   58,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "stand",			WIN_MAIN_STATE_BAR_WORK_MODE1_TEXT_ID,       	158+5, 5,  80,   58,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "prioity",		WIN_MAIN_STATE_BAR_WORK_MODE2_TEXT_ID,       	158+95, 5,  100,   58,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "warningsize",	WIN_MAIN_STATE_BAR_WARNING_TEXT_ID,       		EM_XSIZE-100, 0,  28,   58,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "ImgWarning",		WIN_MAIN_STATE_BAR_WARNING_IMG_ID,       		EM_XSIZE-66,	  15,  20,   55,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "ImgWifi",		WIN_MAIN_STATE_BAR_WIFI_IMG_ID,       			EM_XSIZE-42,	  8,  40,   55,   0,   0x0,  0 },
};

/**
 * @brief 中间及底层信息栏
 */
static const GUI_WIDGET_CREATE_INFO _aBottomTextInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "StatBar",		WIN_MAIN_STATE_BAR_ID,        				0,  	58, 	EM_XSIZE,  EM_YSIZE-58,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "Imghome",		WIN_HOME_PAGE_STREAM_LINE_ID,      			37,		172-58,  	276,   69,      0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "Imggrid",		WIN_HOME_PAGE_GRID_LINE_ID,          		330,	183-58,  	120,   70,      0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "Imgsolar",		WIN_HOME_PAGE_SOLAR_LINE_ID,          		325,	 97-58,  	3,     107,     0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "HOME",			WIN_HOME_PAGE_BOTTOM_HOME_ID,       		110,	274-58,  	45,   	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "HOMEPOWER",		WIN_HOME_PAGE_BOTTOM_HOME_POWER_ID,       	102,	250-58,  	60,   	22,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "SOLAR",			WIN_HOME_PAGE_BOTTOM_SOLAR_ID,       		282,	123-58,  	40,   	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "SOLARPOWER",		WIN_HOME_PAGE_BOTTOM_SOLAR_POWER_ID,       	268,	99-58,  	54,   	22,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "GRID",			WIN_HOME_PAGE_BOTTOM_GRID_ID,       		393,	274-58,  	40,   	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "GRIDPOWER",		WIN_HOME_PAGE_BOTTOM_GRID_POWER_ID,       	383,	250-58,  	54,   	22,   	0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "ImgTEMP",		WIN_HOME_PAGE_BOTTOM_TEMP_IMG_ID,          	342,	200-58,  	16,     16,     0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "TEMP",			WIN_HOME_PAGE_BOTTOM_TEMP_TEXT_ID,       	358,	201-58,  	44,   	17,   	0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "Imgbat",			WIN_HOME_PAGE_BOTTOM_BATTERY_IMG_ID,        212,	250-58,  	7,      14,     0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "battext",		WIN_HOME_PAGE_BOTTOM_BATTERY_ID,       		223,	250-58,  	45,   	22,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "roca",			WIN_HOME_PAGE_BOTTOM_ROCA_ID,       		286,	274-58,  	50,   	17,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "rocapower",		WIN_HOME_PAGE_BOTTOM_ROCA_POWER_ID,       	270,	250-58,  	55,   	22,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "charge",			WIN_HOME_PAGE_BOTTOM_CHARGE_ID,       		254,	293-58,  	78,   	17,   	0,   0x0,  0 },
};


static void _ShowLayoutStatBar()
{
#if LAYOUT_DEBUG

	GUI_SetColor(GUI_COLOR_WIN_MAIN_STAT_BAR_LAYOUT);//上色
	GUI_FillRoundedRect(_aStateBarDialogCreate[1].x0, 
						_aStateBarDialogCreate[1].y0, 
						_aStateBarDialogCreate[1].x0 + _aStateBarDialogCreate[1].xSize - 1, 
						_aStateBarDialogCreate[1].y0 + _aStateBarDialogCreate[1].ySize - 1,
						0);


	GUI_FillRoundedRect(_aStateBarDialogCreate[2].x0, 
						_aStateBarDialogCreate[2].y0, 
						_aStateBarDialogCreate[2].x0 + _aStateBarDialogCreate[2].xSize - 1, 
						_aStateBarDialogCreate[2].y0 + _aStateBarDialogCreate[2].ySize - 1,
						0);


	GUI_FillRoundedRect(_aStateBarDialogCreate[3].x0, 
						_aStateBarDialogCreate[3].y0, 
						_aStateBarDialogCreate[3].x0 + _aStateBarDialogCreate[3].xSize - 1, 
						_aStateBarDialogCreate[3].y0 + _aStateBarDialogCreate[3].ySize - 1,
						0);


	GUI_FillRoundedRect(_aStateBarDialogCreate[4].x0, 
						_aStateBarDialogCreate[4].y0, 
						_aStateBarDialogCreate[4].x0 + _aStateBarDialogCreate[4].xSize - 1, 
						_aStateBarDialogCreate[4].y0 + _aStateBarDialogCreate[4].ySize - 1,
						0);


	GUI_FillRoundedRect(_aStateBarDialogCreate[5].x0, 
						_aStateBarDialogCreate[5].y0, 
						_aStateBarDialogCreate[5].x0 + _aStateBarDialogCreate[5].xSize - 1, 
						_aStateBarDialogCreate[5].y0 + _aStateBarDialogCreate[5].ySize - 1,
						0);
#endif 
}

/**
  * @brief  HomePageStatSubWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbStateBarSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;

	static time_t cur_time = 1704067200;
    static int     wifi_type = 0;
    static WM_HTIMER wm_timer_handle_wifi;
	static int 		pcs_stat = 0;
	static int 		workmode = 0;;

    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:   
        {

            wm_timer_handle_wifi = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                             0, 	     /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                             1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                             0);		 /* 留待将来使用，应为0 */ 



            /*初始化各个子控件*/
			_gImgStateBarWifiHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_WIFI_IMG_ID);
			IMAGE_SetDTA(_gImgStateBarWifiHandle, _gStateBarWifiImgPara[wifi_type].p_img_buf, 
						 _gStateBarWifiImgPara[wifi_type].img_size); 

			_gImgStateBarWarningHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_WARNING_IMG_ID);
			/*IMAGE_SetDTA(_gImgStateBarWarningHandle, _gStateBarWarningImgPara.p_img_buf, 
						 _gStateBarWarningImgPara.img_size); */			 

            _gTextStateBarTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_TIME_TEXT_ID);
            TEXT_SetFont(_gTextStateBarTimeHandle, _gStateBarTimeTextPara.p_text_font);
            TEXT_SetText(_gTextStateBarTimeHandle, (const char *)_gStateBarTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextStateBarTimeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStateBarTimeHandle, _gStateBarTimeTextPara.text_color); 

            _gTextStateBarWorkMode1Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_WORK_MODE1_TEXT_ID);
            TEXT_SetFont(_gTextStateBarWorkMode1Handle, _gStateBarWorkMode1TextPara.p_text_font);
            TEXT_SetText(_gTextStateBarWorkMode1Handle, (const char *)_gStateBarWorkMode1TextPara.p_content);
            TEXT_SetTextAlign(_gTextStateBarWorkMode1Handle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStateBarWorkMode1Handle, _gStateBarWorkMode1TextPara.text_color); 

            _gTextStateBarWorkMode2Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_WORK_MODE2_TEXT_ID);
            TEXT_SetFont(_gTextStateBarWorkMode2Handle, _gStateBarWorkMode2TextPara.p_text_font);
            TEXT_SetText(_gTextStateBarWorkMode2Handle, (const char *)_gStateBarWorkMode2TextPara.p_content);
            TEXT_SetTextAlign(_gTextStateBarWorkMode2Handle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStateBarWorkMode2Handle, _gStateBarWorkMode2TextPara.text_color); 

            _gTextStateBarWarningHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_STATE_BAR_WARNING_TEXT_ID);
            TEXT_SetFont(_gTextStateBarWarningHandle, _gStateBarWarningTextPara.p_text_font);
            TEXT_SetText(_gTextStateBarWarningHandle, (const char *)_gStateBarWarningTextPara.p_content);
            TEXT_SetTextAlign(_gTextStateBarWarningHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStateBarWarningHandle, _gStateBarWarningTextPara.text_color); 
            
            break;
        }
        case WM_DELETE:
		{
            WM_DeleteTimer(wm_timer_handle_wifi);//；删除窗口，会自动删除对应内部的定时器
			break;
		}
        case WM_TIMER:
        {
#if DEBUG_TIMER
			++wifi_type;
			++cur_time;
			++pcs_stat;
			++workmode;

			SetHomePageDayTimeTime(cur_time);
			SetHomePageDayTimeWifiStat((WIFI_STAT_E)(wifi_type%4));
			//SetHomePageDayTimeWarning((FAULT_STAT_E)(cur_time%3), cur_time%1000);
//			SetHomePageDayTimeWarning(WARNNING_STAT,222);
//			SetHomePageDayTimeWarning(FAULT_STAT,223);
			SetHomePageDayTimeWarning(NORMAL_STAT,0);
			
            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
			break;

#endif 
		}
        case WM_PAINT:
        {
            GUI_SetColor(0x00EEEEEE);//上色
            GUI_FillRoundedRect(158, 
                                15, 
                                158+200, 
								10+38,
                                5);

			GUI_SetPenSize(2);
            GUI_SetColor(0x00FFFFFF);
			GUI_DrawLine(247, 23, 247, 23+18);
			
			SetHomePageDayTimeWorkMode((WORK_MODE_E)(workmode % 3));
			SetHomePageDayTimePcsStat((PCS_STAT_E)(pcs_stat % 6));
			//_ShowLayoutStatBar();

			SetSolarPowerstatDayTime(SOLAR_DAYTIME);
			SetHomePowerstatDayTime(HOME_DISABLED);
			SetGridPowerstatDayTime(GRID_ACTIVE_IN);
			SetIsChargeDayTime(1);


            break;
        }
        case WM_NOTIFY_PARENT:/*基于该控件窗口的子控件触发时，如虚拟按键*/
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
  * @brief Bottom Win callback
  * @param  NULL
  * @retval NULL
  */
static void _cbBottomSubWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;


    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:   
        {
            /*初始化各个子控件*/
			//家用供电线图标
			_gImgHomeActiveHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_STREAM_LINE_ID);
			IMAGE_SetDTA(_gImgHomeActiveHandle, _gHomeActiveImgPara[0].p_img_buf, _gHomeActiveImgPara[0].img_size); 
			//电网供电线图标
			_gImgGridActiveHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_GRID_LINE_ID);
			IMAGE_SetDTA(_gImgGridActiveHandle, _gGridActiveImgPara[1].p_img_buf, _gGridActiveImgPara[1].img_size); 
			//太阳能供电线图标
			_gImgSolarActiveHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_SOLAR_LINE_ID);
			IMAGE_SetDTA(_gImgSolarActiveHandle, _gSolarActiveImgPara[0].p_img_buf, _gSolarActiveImgPara[0].img_size); 			 
			//温度图标
			_gImgBottomTempHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_TEMP_IMG_ID);
			IMAGE_SetDTA(_gImgBottomTempHandle, _gBottomTempImgPara.p_img_buf, _gBottomTempImgPara.img_size); 	
			//电池电量图标
			_gImgBottomBatteryHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_BATTERY_IMG_ID);
			IMAGE_SetDTA(_gImgBottomBatteryHandle, _gBottomBatteryImgPara[2].p_img_buf, _gBottomBatteryImgPara[2].img_size); 
			//家用文字标识
            _gTextBottomHomeHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_HOME_ID);
            TEXT_SetFont(_gTextBottomHomeHandle, _gBottomHomeTextPara.p_text_font);
            TEXT_SetText(_gTextBottomHomeHandle, (const char *)_gBottomHomeTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomHomeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomHomeHandle, _gBottomHomeTextPara.text_color); 
			//家用功率
            _gTextBottomHomePowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_HOME_POWER_ID);
            TEXT_SetFont(_gTextBottomHomePowerHandle, _gBottomHomePowerTextPara.p_text_font);
            TEXT_SetText(_gTextBottomHomePowerHandle, (const char *)_gBottomHomePowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomHomePowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomHomePowerHandle, _gBottomHomePowerTextPara.text_color); 
			//太阳能文字标识	
            _gTextBottomSolarHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_SOLAR_ID);
            TEXT_SetFont(_gTextBottomSolarHandle, _gBottomSolarTextPara.p_text_font);
            TEXT_SetText(_gTextBottomSolarHandle, (const char *)_gBottomSolarTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomSolarHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomSolarHandle, _gBottomSolarTextPara.text_color); 
			//太阳能功率
            _gTextBottomSolarPowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_SOLAR_POWER_ID);
            TEXT_SetFont(_gTextBottomSolarPowerHandle, _gBottomSolarPowerTextPara.p_text_font);
            TEXT_SetText(_gTextBottomSolarPowerHandle, (const char *)_gBottomSolarPowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomSolarPowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomSolarPowerHandle, _gBottomSolarPowerTextPara.text_color); 
			//电网文字标识
			_gTextBottomGridHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_GRID_ID);
            TEXT_SetFont(_gTextBottomGridHandle, _gBottomGridTextPara.p_text_font);
            TEXT_SetText(_gTextBottomGridHandle, (const char *)_gBottomGridTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomGridHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomGridHandle, _gBottomGridTextPara.text_color); 
			//电网功率
			_gTextBottomGridPowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_GRID_POWER_ID);
            TEXT_SetFont(_gTextBottomGridPowerHandle, _gBottomGridPowerTextPara.p_text_font);
            TEXT_SetText(_gTextBottomGridPowerHandle, (const char *)_gBottomGridPowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomGridPowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomGridPowerHandle, _gBottomGridPowerTextPara.text_color); 
			//温度TEXT
			_gTextBottomTempHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_TEMP_TEXT_ID);
            TEXT_SetFont(_gTextBottomTempHandle, _gBottomTempTextPara.p_text_font);
            TEXT_SetText(_gTextBottomTempHandle, (const char *)_gBottomTempTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomTempHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomTempHandle, _gBottomTempTextPara.text_color); 
			//电池电量TEXT
			_gTextBottomBatteryHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_BATTERY_ID);
            TEXT_SetFont(_gTextBottomBatteryHandle, _gBottomBatteryTextPara.p_text_font);
            TEXT_SetText(_gTextBottomBatteryHandle, (const char *)_gBottomBatteryTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomBatteryHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomBatteryHandle, _gBottomBatteryTextPara.text_color); 
            //Roca text
			_gTextBottomRocaHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_ROCA_ID);
            TEXT_SetFont(_gTextBottomRocaHandle, _gBottomRocaTextPara.p_text_font);
            TEXT_SetText(_gTextBottomRocaHandle, (const char *)_gBottomRocaTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomRocaHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomRocaHandle, _gBottomRocaTextPara.text_color); 
			//Roca功率
			_gTextBottomRocaPowerHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_ROCA_POWER_ID);
            TEXT_SetFont(_gTextBottomRocaPowerHandle, _gBottomRocaPowerTextPara.p_text_font);
            TEXT_SetText(_gTextBottomRocaPowerHandle, (const char *)_gBottomRocaPowerTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomRocaPowerHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomRocaPowerHandle, _gBottomRocaPowerTextPara.text_color); 
			//charge text
			_gTextBottomChargeHandle = WM_GetDialogItem(pMsg->hWin, WIN_HOME_PAGE_BOTTOM_CHARGE_ID);
            TEXT_SetFont(_gTextBottomChargeHandle, _gBottomChargeTextPara.p_text_font);
            TEXT_SetText(_gTextBottomChargeHandle, (const char *)_gBottomChargeTextPara.p_content);
            TEXT_SetTextAlign(_gTextBottomChargeHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextBottomChargeHandle, _gBottomChargeTextPara.text_color); 			
            break;
        }
       
        case WM_PAINT:
        {
//            GUI_SetColor(0x00B2BE00);//上色
//            GUI_FillRoundedRect(226, 198-58,226+77,198-58 + 20, 10);
			//int32_t SetTempStaBarDayTime(int32_t value_temp);
            SetTempStaBarDayTime(120);      

			int32_t SetElecChangeDayTime(int32_t value_elec);
			SetElecChangeDayTime(30);
			
//			int32_t SetHomePageDayTimeElec(int32_t value_elec);
//			SetHomePageDayTimeElec(10);
			
			SetHomePowerDayTime(4.2);
            break;
        }
        case WM_NOTIFY_PARENT:/*基于该控件窗口的子控件触发时，如虚拟按键*/
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
  * @brief Root Win callback
  * @param  NULL
  * @retval NULL
  */
static void _cbMainWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld;
    switch (pMsg->MsgId) 
    {
        case WM_CREATE:
        {
            break;
        }
        case WM_DELETE:
        {
            break;
        }
        case WM_PAINT:
        {
            GUI_DrawStreamedBitmapAuto(_pBKBuf, 0, 0);
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
  * @brief Create stat bar sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateStateBarSubWin()
{
    _gWinStateBarHandle = GUI_CreateDialogBox(_aStateBarDialogCreate, 
											  GUI_COUNTOF(_aStateBarDialogCreate), 
											  _cbStateBarSubWin, 
											  pCurrentEmWin->win_handle, 
											  0, 0);
											  //EM_YSIZE - _aStateBarDialogCreate[0].ySize);
    WM_SetTransState(_gWinStateBarHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinStateBarHandle;
}

/**
  * @brief Create bottom sub win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateBottomSubWin()
{
    _gWinBottomHandle = GUI_CreateDialogBox(_aBottomTextInfoDialogCreate, 
											  GUI_COUNTOF(_aBottomTextInfoDialogCreate), 
											  _cbBottomSubWin, 
											  pCurrentEmWin->win_handle, 
											  0, 0);
											  //EM_YSIZE - _aStateBarDialogCreate[0].ySize);
    WM_SetTransState(_gWinBottomHandle, WM_CF_HASTRANS);
//    GUI_Exec();
    return _gWinBottomHandle;
}


  
/**
  * @brief Create current win
  * @param  NULL
  * @retval NULL
  */
static GUI_HWIN _CreateWin(void)
{
    _Init();
    GUI_EnableAlpha(1);
    /*创建一个基于桌面窗口的子窗口*/
    gEmWinHomePageDayTime.win_handle = WM_CreateWindow(0, 0, EM_XSIZE, EM_YSIZE, WM_CF_SHOW, _cbMainWin, 0);
    pCurrentEmWin = &gEmWinHomePageDayTime;
    GUI_Exec();

	_CreateStateBarSubWin();
	_CreateBottomSubWin();


    return gEmWinHomePageDayTime.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinHomePageDayTime.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinHomePageDayTime.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinHomePageDayTime.win_handle = 0;
}



/**
  * @brief  顶部状态栏初始化
  * @param  NULL
  * @retval NULL
  */
static void _InitHomepageStatBar()
{
	/*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为StatBar区域子控件申请内存*/
	_gStateBarWifiImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE);
    _gStateBarWifiImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gStateBarWifiImgPara[0].mem_img);
    _gStateBarWifiImgPara[0].img_size = EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_ON_ADDR, _gStateBarWifiImgPara[0].p_img_buf, 
					 _gStateBarWifiImgPara[0].img_size);

    _gStateBarWifiImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE);
    _gStateBarWifiImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gStateBarWifiImgPara[1].mem_img);
    _gStateBarWifiImgPara[1].img_size = EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_DISCONN_CLOUD_ADDR, _gStateBarWifiImgPara[1].p_img_buf, 
					 _gStateBarWifiImgPara[1].img_size);

	_gStateBarWifiImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE);
    _gStateBarWifiImgPara[2].p_img_buf = GUI_ALLOC_h2p(_gStateBarWifiImgPara[2].mem_img);
    _gStateBarWifiImgPara[2].img_size = EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_OFF_ADDR, _gStateBarWifiImgPara[2].p_img_buf, 
					 _gStateBarWifiImgPara[2].img_size);
	
	_gStateBarWifiImgPara[3].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE);
    _gStateBarWifiImgPara[3].p_img_buf = GUI_ALLOC_h2p(_gStateBarWifiImgPara[3].mem_img);
    _gStateBarWifiImgPara[3].img_size = EM_HOME_PAGE_STATE_BAR_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_ABNORMAL_ADDR, _gStateBarWifiImgPara[3].p_img_buf, 
					 _gStateBarWifiImgPara[3].img_size);
	

    _gStateBarWarningImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WARNING_ICON_SIZE);
    _gStateBarWarningImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gStateBarWarningImgPara[0].mem_img);
    _gStateBarWarningImgPara[0].img_size = EM_HOME_PAGE_STATE_BAR_WARNING_ICON_SIZE;
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _gStateBarWarningImgPara[0].p_img_buf, 
					 _gStateBarWarningImgPara[0].img_size);

    _gStateBarWarningImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_STATE_BAR_WARNING_ICON_SIZE);
    _gStateBarWarningImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gStateBarWarningImgPara[1].mem_img);
    _gStateBarWarningImgPara[1].img_size = EM_HOME_PAGE_STATE_BAR_WARNING_ICON_SIZE;
    SpiFlashReadData(EM_ICON_ERROR_ADDR, _gStateBarWarningImgPara[1].p_img_buf, 
					 _gStateBarWarningImgPara[1].img_size);


    _gStateBarTimeTextPara.p_content = _gStateBarTimeTextBuf;
    _gStateBarTimeTextPara.text_color = 0X00D9D9D9;
    _gStateBarTimeTextPara.p_text_font = EM_TEXT_FONT_7;

    _gStateBarWorkMode1TextPara.p_content = _gStateBarWorkMode1TextBuf;
    _gStateBarWorkMode1TextPara.text_color = 0X00D9D9D9;
    _gStateBarWorkMode1TextPara.p_text_font = EM_TEXT_FONT_7;

    _gStateBarWorkMode2TextPara.p_content = _gStateBarWorkMode2TextBuf;
    _gStateBarWorkMode2TextPara.text_color = 0X00D9D9D9;
    _gStateBarWorkMode2TextPara.p_text_font = EM_TEXT_FONT_7;

    _gStateBarWarningTextPara.p_content = _gStateBarWarningTextBuf;
    _gStateBarWarningTextPara.text_color = 0X00000000;
    _gStateBarWarningTextPara.p_text_font = EM_TEXT_FONT_7;

	snprintf(_gStateBarWorkMode1TextBuf, sizeof(_gStateBarWorkMode1TextBuf), "%s", "Off Grid");
	snprintf(_gStateBarWorkMode2TextBuf, sizeof(_gStateBarWorkMode2TextBuf), "%s", "PL-Balance");
}

/**
  * @brief  底部子窗口初始化
  * @param  NULL
  * @retval NULL
  */
static void _InitHomepageBottom()
{
	/*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/

	_gHomeActiveImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_HOME_ACTIVE_ICON_SIZE);
	_gHomeActiveImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gHomeActiveImgPara[0].mem_img);
	_gHomeActiveImgPara[0].img_size = EM_HOME_PAGE_BOTTOM_HOME_ACTIVE_ICON_SIZE;
	SpiFlashReadData(EM_ICON_HOME_ACTIVE_PAGE_ADDR, _gHomeActiveImgPara[0].p_img_buf, 
					 _gHomeActiveImgPara[0].img_size);
	
	_gHomeActiveImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_HOME_ACTIVE_ICON_SIZE);
	_gHomeActiveImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gHomeActiveImgPara[1].mem_img);
	_gHomeActiveImgPara[1].img_size = EM_HOME_PAGE_BOTTOM_HOME_ACTIVE_ICON_SIZE;
	SpiFlashReadData(EM_ICON_HOME_DISABLED_PAGE_ADDR, _gHomeActiveImgPara[1].p_img_buf, 
					 _gHomeActiveImgPara[1].img_size);
	
	_gGridActiveImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE);
	_gGridActiveImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gGridActiveImgPara[0].mem_img);
	_gGridActiveImgPara[0].img_size = EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE;
	SpiFlashReadData(EM_ICON_GRID_ACTIVE_IN_PAGE_ADDR, _gGridActiveImgPara[0].p_img_buf, 
					 _gGridActiveImgPara[0].img_size);
	
	_gGridActiveImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE);
	_gGridActiveImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gGridActiveImgPara[1].mem_img);
	_gGridActiveImgPara[1].img_size = EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE;
	SpiFlashReadData(EM_ICON_GRID_ACTIVE_OUT_PAGE_ADDR, _gGridActiveImgPara[1].p_img_buf, 
					 _gGridActiveImgPara[1].img_size);
	
	_gGridActiveImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE);
	_gGridActiveImgPara[2].p_img_buf = GUI_ALLOC_h2p(_gGridActiveImgPara[2].mem_img);
	_gGridActiveImgPara[2].img_size = EM_HOME_PAGE_BOTTOM_GRID_ACTIVE_ICON_SIZE;
	SpiFlashReadData(EM_ICON_GRID_DISABLED_PAGE_ADDR, _gGridActiveImgPara[2].p_img_buf, 
					 _gGridActiveImgPara[2].img_size);
	
	_gSolarActiveImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE);
	_gSolarActiveImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gSolarActiveImgPara[0].mem_img);
	_gSolarActiveImgPara[0].img_size = EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE;
	SpiFlashReadData(EM_ICON_SOLAR_ACTIVE_PAGE_ADDR, _gSolarActiveImgPara[0].p_img_buf, 
					 _gSolarActiveImgPara[0].img_size);
					 
	_gSolarActiveImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE);
	_gSolarActiveImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gSolarActiveImgPara[1].mem_img);
	_gSolarActiveImgPara[1].img_size = EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE;
	SpiFlashReadData(EM_ICON_SOLAR_DISABLED_PAGE_ADDR, _gSolarActiveImgPara[1].p_img_buf, 
					 _gSolarActiveImgPara[1].img_size);		

	_gSolarActiveImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE);
	_gSolarActiveImgPara[2].p_img_buf = GUI_ALLOC_h2p(_gSolarActiveImgPara[2].mem_img);
	_gSolarActiveImgPara[2].img_size = EM_HOME_PAGE_BOTTOM_SOLAR_ACTIVE_ICON_SIZE;
	SpiFlashReadData(EM_ICON_SOLAR_LINE_DAYTIME_PAGE_ADDR, _gSolarActiveImgPara[2].p_img_buf, 
					 _gSolarActiveImgPara[2].img_size);	
	
	_gBottomTempImgPara.mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_TEMP_ICON_SIZE);
	_gBottomTempImgPara.p_img_buf = GUI_ALLOC_h2p(_gBottomTempImgPara.mem_img);
	_gBottomTempImgPara.img_size = EM_HOME_PAGE_BOTTOM_TEMP_ICON_SIZE;
	SpiFlashReadData(EM_ICON_FLUENT_TEMP_ADDR, _gBottomTempImgPara.p_img_buf, 
					 _gBottomTempImgPara.img_size);
	
	_gBottomBatteryImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE);
	_gBottomBatteryImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gBottomBatteryImgPara[0].mem_img);
	_gBottomBatteryImgPara[0].img_size = EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE;
	SpiFlashReadData(EM_ICON_BATTERY1_ADDR, _gBottomBatteryImgPara[0].p_img_buf, 
					 _gBottomBatteryImgPara[0].img_size);

	_gBottomBatteryImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE);
	_gBottomBatteryImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gBottomBatteryImgPara[1].mem_img);
	_gBottomBatteryImgPara[1].img_size = EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE;
	SpiFlashReadData(EM_ICON_BATTERY2_ADDR, _gBottomBatteryImgPara[1].p_img_buf,  _gBottomBatteryImgPara[1].img_size);

	_gBottomBatteryImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE);
	_gBottomBatteryImgPara[2].p_img_buf = GUI_ALLOC_h2p(_gBottomBatteryImgPara[2].mem_img);
	_gBottomBatteryImgPara[2].img_size = EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE;
	SpiFlashReadData(EM_ICON_BATTERY3_ADDR, _gBottomBatteryImgPara[2].p_img_buf,
					_gBottomBatteryImgPara[2].img_size);	
					 
	_gBottomBatteryImgPara[3].mem_img = GUI_ALLOC_AllocZero(EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE);
	_gBottomBatteryImgPara[3].p_img_buf = GUI_ALLOC_h2p(_gBottomBatteryImgPara[3].mem_img);
	_gBottomBatteryImgPara[3].img_size = EM_HOME_PAGE_BOTTOM_BATTERY_ICON_SIZE;
	SpiFlashReadData(EM_ICON_BATTERY4_ADDR, _gBottomBatteryImgPara[3].p_img_buf,
					_gBottomBatteryImgPara[3].img_size);					 					 
					 	
	_gBottomHomeTextPara.p_content = _gBottomHomeTextBuf;
	_gBottomHomeTextPara.text_color = 0x00C1C1C1;
    _gBottomHomeTextPara.p_text_font = EM_TEXT_FONT_7;
	snprintf(_gBottomHomeTextBuf, sizeof(_gBottomHomeTextBuf), "%s", "Home");				 

    _gBottomHomePowerTextPara.p_content =	_gBottomHomePowerTextBuf;
    _gBottomHomePowerTextPara.text_color = 0X00FFFFFF;
    _gBottomHomePowerTextPara.p_text_font = EM_TEXT_FONT_5;
	_gBottomHomePowerTextPara.digit_value = 2.0;//测试数据，实际赋值系统数据
    sprintf(_gBottomHomePowerTextPara.p_content, "%.1fkW", _gBottomHomePowerTextPara.digit_value);
	
	
	_gBottomSolarTextPara.p_content = _gBottomSolarTextBuf;
	_gBottomSolarTextPara.text_color = 0x00C1C1C1;
    _gBottomSolarTextPara.p_text_font = EM_TEXT_FONT_7;
	snprintf(_gBottomSolarTextBuf, sizeof(_gBottomSolarTextBuf), "%s", "Solar");				 

    _gBottomSolarPowerTextPara.p_content =	_gBottomSolarPowerTextBuf;
    _gBottomSolarPowerTextPara.text_color = 0X00FFFFFF;
    _gBottomSolarPowerTextPara.p_text_font = EM_TEXT_FONT_5;
	_gBottomSolarPowerTextPara.digit_value = 4.0;//测试数据，实际赋值系统数据
    sprintf(_gBottomSolarPowerTextPara.p_content, "%.1fkW", _gBottomSolarPowerTextPara.digit_value);


	_gBottomGridTextPara.p_content = _gBottomGridTextBuf;
	_gBottomGridTextPara.text_color = 0x00C1C1C1;
    _gBottomGridTextPara.p_text_font = EM_TEXT_FONT_7;
	snprintf(_gBottomGridTextBuf, sizeof(_gBottomGridTextBuf), "%s", "Grid");				 

    _gBottomGridPowerTextPara.p_content =	_gBottomGridPowerTextBuf;
    _gBottomGridPowerTextPara.text_color = 0X00FFFFFF;
    _gBottomGridPowerTextPara.p_text_font = EM_TEXT_FONT_5;
	_gBottomGridPowerTextPara.digit_value = 2.3;//测试数据，实际赋值系统数据
    sprintf(_gBottomGridPowerTextPara.p_content, "%.1fkW", _gBottomGridPowerTextPara.digit_value);
	
    
	_gBottomTempTextPara.p_content = _gBottomTempTextBuf;
	_gBottomTempTextPara.text_color = 0x00FFFFFF;
    _gBottomTempTextPara.p_text_font = EM_TEXT_FONT_7;
	_gBottomTempTextPara.digit_value = 112.5;
	sprintf(_gBottomTempTextPara.p_content, "%.1fF", _gBottomTempTextPara.digit_value);
	
	_gBottomBatteryTextPara.p_content = _gBottomBatteryTextBuf;
	_gBottomBatteryTextPara.text_color = 0x00B2BE00;
    _gBottomBatteryTextPara.p_text_font = EM_TEXT_FONT_5;
	_gBottomBatteryTextPara.digit_value = 60;
	sprintf(_gBottomBatteryTextPara.p_content, "%d%%", (uint16_t)_gBottomBatteryTextPara.digit_value);
	
	_gBottomRocaTextPara.p_content = _gBottomRocaTextBuf;
	_gBottomRocaTextPara.text_color = 0x00C1C1C1;
    _gBottomRocaTextPara.p_text_font = EM_TEXT_FONT_7;
	snprintf(_gBottomRocaTextBuf, sizeof(_gBottomRocaTextBuf), "%s", "Roca");	
	
	_gBottomRocaPowerTextPara.p_content = _gBottomRocaPowerTextBuf;
	_gBottomRocaPowerTextPara.text_color = 0x00FFFFFF;
    _gBottomRocaPowerTextPara.p_text_font = EM_TEXT_FONT_5;
	_gBottomRocaPowerTextPara.digit_value = 4.0;
	sprintf(_gBottomRocaPowerTextPara.p_content, "%.1fkW", _gBottomRocaPowerTextPara.digit_value);
	
	_gBottomChargeTextPara.p_content = _gBottomChargeTextBuf;
	_gBottomChargeTextPara.text_color = 0x00B2BE00;
    _gBottomChargeTextPara.p_text_font = EM_TEXT_FONT_7;
	snprintf(_gBottomChargeTextBuf, sizeof(_gBottomChargeTextBuf), "%s", "Charging");	
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    /*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/
    /*为背景图申请内存*/
    _gMemBK = GUI_ALLOC_AllocZero(EM_HOMEPAGE_DAYTIME_ICON_BACKGROUND_SIZE);
    _pBKBuf = GUI_ALLOC_h2p(_gMemBK);
    SpiFlashReadData(EM_ICON_HOME_PAGE_ADDR, _pBKBuf, EM_HOMEPAGE_DAYTIME_ICON_BACKGROUND_SIZE);

	_InitHomepageStatBar();
	_InitHomepageBottom();
#if 0
	_InitSelect();

	_InitDevStat();
	_InitSetting();
	_InitDevInfo();
	_InitFactorRestore();
#endif
}


static void _DelStatBar()
{
    GUI_ALLOC_Free(_gStateBarWifiImgPara[0].mem_img);
    GUI_ALLOC_Free(_gStateBarWifiImgPara[1].mem_img);
	GUI_ALLOC_Free(_gStateBarWifiImgPara[2].mem_img);
    GUI_ALLOC_Free(_gStateBarWifiImgPara[3].mem_img);
}

static void _DelBottom()
{
	GUI_ALLOC_Free(_gHomeActiveImgPara[0].mem_img);
	GUI_ALLOC_Free(_gHomeActiveImgPara[1].mem_img);
	
	GUI_ALLOC_Free(_gGridActiveImgPara[0].mem_img);
	GUI_ALLOC_Free(_gGridActiveImgPara[1].mem_img);
	GUI_ALLOC_Free(_gGridActiveImgPara[2].mem_img);		
	
	GUI_ALLOC_Free(_gSolarActiveImgPara[0].mem_img);
	GUI_ALLOC_Free(_gSolarActiveImgPara[1].mem_img);
	GUI_ALLOC_Free(_gSolarActiveImgPara[2].mem_img);
	
	GUI_ALLOC_Free(_gBottomTempImgPara.mem_img);
	GUI_ALLOC_Free(_gBottomBatteryImgPara[0].mem_img);
	GUI_ALLOC_Free(_gBottomBatteryImgPara[1].mem_img);
	GUI_ALLOC_Free(_gBottomBatteryImgPara[2].mem_img);
	GUI_ALLOC_Free(_gBottomBatteryImgPara[3].mem_img);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{

	_DelStatBar();
	_DelBottom();
	#if 0	
	_DelSelect();
	_DelDevStat();
	_DelSetting();
	_DelDevInfo();
	_DelFactoryRestore();
    GUI_ALLOC_Free(_gMemBK);
	#endif
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
	//WM_BringToTop(gEmWinListOperaInfo.win_handle);
    //gEmWinListOperaInfo.polling();
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
	
	if(event == EN_KEY_SRC_CONFIRM)
	{
		if(pCurrentEmWin->win_handle != 0)
		{
			pPrevEmWin = pCurrentEmWin;
		}
		pCurrentEmWin->delete_win();
		pCurrentEmWin = &gEmWinElectricityInfo;
		pCurrentEmWin->create_win();
		return;
	}
}

/**
  * @brief  Wifi状态变换
  * @param  NULL
  * @retval NULL
  */
int32_t SetHomePageDayTimeWifiStat(WIFI_STAT_E stat)
{
	emWidgetImgPara_t  *tmp_wifi_img_para = &_gStateBarWifiImgPara[0];
	switch ((int32_t)stat)
	{
        case WIFI_STAT_DISCONN:
		{
			tmp_wifi_img_para = &_gStateBarWifiImgPara[0];
			break;
		}
        case WIFI_STAT_CONN:
		{
			tmp_wifi_img_para = &_gStateBarWifiImgPara[1];
			break;
		}
		case WIFI_STAT_ON:
		{
			tmp_wifi_img_para = &_gStateBarWifiImgPara[2];
			break;
		}
		case WIFI_STAT_OFF:
		{
			tmp_wifi_img_para = &_gStateBarWifiImgPara[3];
			break;
		}
		default:break;
	}

	IMAGE_SetDTA(_gImgStateBarWifiHandle, tmp_wifi_img_para->p_img_buf, 
				 tmp_wifi_img_para->img_size); 
	return 0;
}



/**
  * @brief  警告图标变换
  * @param  stat:故障类型
  * @param  num:故障类型码  0：正常  非零：对应故障类型
  * @retval 0:success
  */
int32_t SetHomePageDayTimeWarning(FAULT_STAT_E stat, uint32_t num)
{
	if(stat > FAULT_STAT_NUM)
	{
		return -1;
	}
	
	if (num > 999)
	{
		return -2;
	}
	
	emWidgetImgPara_t  *tmp_fault_img_para = &_gStateBarWarningImgPara[0];
	switch ((int32_t)stat)
	{
        case FAULT_STAT:
		{
			_gStateBarWarningTextPara.text_color = 0X0000BEF7;
			tmp_fault_img_para = &_gStateBarWarningImgPara[0];
			IMAGE_SetDTA(_gImgStateBarWarningHandle, tmp_fault_img_para->p_img_buf, 
						 tmp_fault_img_para->img_size); 
			break;
		}
        case WARNNING_STAT:
		{
			_gStateBarWarningTextPara.text_color = 0X006A62FF;
			tmp_fault_img_para = &_gStateBarWarningImgPara[1];
			IMAGE_SetDTA(_gImgStateBarWarningHandle, tmp_fault_img_para->p_img_buf, 
						 tmp_fault_img_para->img_size); 
			break;
		}
		case NORMAL_STAT:
		{
			tmp_fault_img_para = NULL;
			break;
		}

		default:break;
	}

	if(0 == num)
	{
		snprintf(_gStateBarWarningTextBuf, sizeof(_gStateBarWarningTextBuf), "%s", " ");
	}
	else
	{
		snprintf(_gStateBarWarningTextBuf, sizeof(_gStateBarWarningTextBuf), "%.3d", num);
	}
	TEXT_SetTextColor(_gTextStateBarWarningHandle, _gStateBarWarningTextPara.text_color); 
	TEXT_SetText(_gTextStateBarWarningHandle, (const char *)_gStateBarWarningTextPara.p_content);

	return 0;
}

/**
  * @brief  PCS状态变换
  * @param  NULL
  * @retval NULL
  */
int32_t SetHomePageDayTimePcsStat(PCS_STAT_E mode)
{
	switch (mode)
	{
		case PCS_STAT_SHUTDOWN:
			{
				_gStateBarWorkMode1TextPara.text_color = 0X00D9D9D9;
				snprintf(_gStateBarWorkMode1TextBuf, sizeof(_gStateBarWorkMode1TextBuf), 
						 "%s", "Shutdown");
		break;
			}
		case PCS_STAT_STAND_BY:
			{
				_gStateBarWorkMode1TextPara.text_color = 0X0036B6FF;
				snprintf(_gStateBarWorkMode1TextBuf, sizeof(_gStateBarWorkMode1TextBuf), 
						 "%s", "Stand-by");
		break;
			}
		case PCS_STAT_SELF_TEST:
			{
				_gStateBarWorkMode1TextPara.text_color = 0X0036B6FF;
				snprintf(_gStateBarWorkMode1TextBuf, sizeof(_gStateBarWorkMode1TextBuf), 
						 "%s", "Self-test");
		break;
			}
		case PCS_STAT_BY_PASS:
			{
				_gStateBarWorkMode1TextPara.text_color = 0X00FFC44C;
				snprintf(_gStateBarWorkMode1TextBuf, sizeof(_gStateBarWorkMode1TextBuf), 
						 "%s", "Bypass");
		break;
			}
		case PCS_STAT_OFF_GRID:
			{
				_gStateBarWorkMode1TextPara.text_color = 0X00D9D9D9;
				snprintf(_gStateBarWorkMode1TextBuf, sizeof(_gStateBarWorkMode1TextBuf), 
						 "%s", "Off Grid");
		break;
			}
		case PCS_STAT_ON_GRID:
			{
				_gStateBarWorkMode1TextPara.text_color = 0X00B2BE00;
				snprintf(_gStateBarWorkMode1TextBuf, sizeof(_gStateBarWorkMode1TextBuf), 
						 "%s", "On Grid");
		break;
			}
		default:
		break;
	}
	TEXT_SetTextColor(_gTextStateBarWorkMode1Handle, _gStateBarWorkMode1TextPara.text_color); 
	TEXT_SetText(_gTextStateBarWorkMode1Handle, (const char *)_gStateBarWorkMode1TextPara.p_content);
	return 0;
}

/**
  * @brief  工作模式变换
  * @param  NULL
  * @retval NULL
  */
int32_t SetHomePageDayTimeWorkMode(WORK_MODE_E mode)
{
	switch(mode)
	{
		case WORK_MODE_PL_BALANCE:
			{
				snprintf(_gStateBarWorkMode2TextBuf, sizeof(_gStateBarWorkMode2TextBuf), 
						 "%s", "PL-Balance");
		break;
			}
		case WORK_MODE_SELF_CONSUMP:
			{
				snprintf(_gStateBarWorkMode2TextBuf, sizeof(_gStateBarWorkMode2TextBuf), 
						 "%s", "Self Consump");
		break;
			}
		case WORK_MODE_BAT:
			{
				snprintf(_gStateBarWorkMode2TextBuf, sizeof(_gStateBarWorkMode2TextBuf), 
						 "%s", "BAT Prioity");
		break;
			}
		default:
		break;
	}

	TEXT_SetText(_gTextStateBarWorkMode2Handle, 
				 (const char *)_gStateBarWorkMode2TextPara.p_content);
	return 0;
}


/**
  * @brief  显示时间
  * @param  NULL
  * @retval NULL
  */
int32_t SetHomePageDayTimeTime(uint32_t t)
{
	struct tm* timeinfo;
	timeinfo = localtime(&t); // 根据时间戳生成tm结构体对象
	//strftime(_gStateBarTimeTextBuf, sizeof(_gStateBarTimeTextBuf), "%Y %b %eth %H:%M:%S", 
	strftime(_gStateBarTimeTextBuf, sizeof(_gStateBarTimeTextBuf), "%b %eth %H:%M:%S", 
			 timeinfo); // 按指定格式将时间信息写入字符串
	TEXT_SetText(_gTextStateBarTimeHandle, (const char *)_gStateBarTimeTextPara.p_content);

	return 0;
}

/**
  * @brief  设置界面中家庭功率文本
  * @param  value_kw:功率值
  * @param  
  * @retval 0:success
  */
int32_t SetHomePowerDayTime(float value_kw)
{
	if (value_kw < (float)0.000001)
	{
		return -1;
	}
	
	_gBottomHomePowerTextPara.digit_value = value_kw;
	sprintf(_gBottomHomePowerTextPara.p_content, "%.1fkW", _gBottomHomePowerTextPara.digit_value);
	TEXT_SetText(_gTextBottomHomePowerHandle, (const char *)_gBottomHomePowerTextPara.p_content);
	return 0;
}
/**
  * @brief  设置界面中太阳能功率文本
  * @param  value_kw:功率值
  * @param  
  * @retval 0:success
  */
int32_t SetSolarPowerDayTime(float value_kw)
{
	if (value_kw < (float)0.000001)
	{
		return -1;
	}
	
	_gBottomSolarPowerTextPara.digit_value = value_kw;
    sprintf(_gBottomSolarPowerTextPara.p_content, "%.1fkW", _gBottomSolarPowerTextPara.digit_value);
	TEXT_SetText(_gTextBottomSolarPowerHandle, (const char *)_gBottomSolarPowerTextPara.p_content);
	return 0;
}
/**
  * @brief  设置界面中电网功率文本
  * @param  value_kw:功率值
  * @param  
  * @retval 0:success
  */
int32_t SetGridPowerDayTime(float value_kw)
{
	if(value_kw < (float)0.000001)
	{
		return -1;
	}
	
	_gBottomGridPowerTextPara.digit_value = value_kw;
    sprintf(_gBottomGridPowerTextPara.p_content, "%.1fkW", _gBottomGridPowerTextPara.digit_value);
	TEXT_SetText(_gTextBottomGridPowerHandle, (const char *)_gBottomGridPowerTextPara.p_content);
	return 0;
}
/**
  * @brief  设置界面中Roca功率文本
  * @param  value_kw:功率值
  * @param  
  * @retval 0:success
  */
int32_t SetRocaPowerDayTime(float value_kw)
{
	if(value_kw < (float)0.000001)
	{
		return -1;
	}
	
	_gBottomRocaPowerTextPara.digit_value = value_kw;
	sprintf(_gBottomRocaPowerTextPara.p_content, "%.1fkW", _gBottomRocaPowerTextPara.digit_value);
	TEXT_SetText(_gTextBottomRocaPowerHandle, (const char *)_gBottomRocaPowerTextPara.p_content);
	return 0;
}
#if 0
/**
  * @brief  设置界面中温度文本
  * @param  value_temp:温度值
  * @param  
  * @retval 0:success
  */
int32_t SetHomePageDayTimeTemperature(float value_temp)
{
	if(value_temp < (float)0.000001)
	{
		return -1;
	}

	_gBottomTempTextPara.digit_value = value_temp;
	sprintf(_gBottomTempTextPara.p_content, "%.1fF", _gBottomTempTextPara.digit_value);
	TEXT_SetText(_gTextBottomTempHandle, (const char *)_gBottomTempTextPara.p_content);
	return 0;
}

/**
  * @brief  设置界面中电量百分比文本
  * @param  value_elec:电量百分比
  * @param  
  * @retval 0:success
  */
int32_t SetHomePageDayTimeElec(int32_t value_elec)
{
	if(value_elec < 0 || value_elec > 100)
	{
		return -1;
	}
	
	if(value_elec > 0 && value_elec < 20)
	{
		_gBottomBatteryTextPara.text_color = 0x006A62FF;
	}
	_gBottomBatteryTextPara.digit_value = value_elec;
	sprintf(_gBottomBatteryTextPara.p_content, "%d%%", (uint16_t)_gBottomBatteryTextPara.digit_value);
	TEXT_SetTextColor(_gTextBottomBatteryHandle,_gBottomBatteryTextPara.text_color);
	TEXT_SetText(_gTextBottomBatteryHandle, (const char *)_gBottomBatteryTextPara.p_content);
	return 0;
}
#endif
/**
  * @brief  设置界面底部是否充电文本
  * @param  value_charge:0——充电（显示文本）  非0——未充电（不显示）
  * @param  
  * @retval 0:success
  */
int32_t SetIsChargeDayTime(int32_t value_charge)
{
	if(!value_charge)
	{
		snprintf(_gBottomChargeTextBuf, sizeof(_gBottomChargeTextBuf), "%s", "Charging");
	}
	else
	{
		snprintf(_gBottomChargeTextBuf, sizeof(_gBottomChargeTextBuf), "%s", " ");
	}
	TEXT_SetText(_gTextBottomChargeHandle, (const char *)_gBottomChargeTextPara.p_content);
	
	return 0;
}
/**
  * @brief  设置界面中电池图标改变以及电量百分比文本显示
  * @param  value_elec:电量占比
  * @param  
  * @retval 0:success
  * @notes  电池电量显示切换
  */
int32_t SetElecChangeDayTime(int32_t value_elec)
{
	emWidgetImgPara_t  *tmp_battery_img_para = &_gBottomBatteryImgPara[2];
	
	if(0 > value_elec | value_elec > 100)
	{
		return -1;
	}

	/*根据电量选择显示那个电池图标  0-满电  1-更多电 2-一半电 3-低电量*/ 
	if(value_elec == 100)
	{
		tmp_battery_img_para = &_gBottomBatteryImgPara[0];
	}
	else if(value_elec > 79 && value_elec < 100)
	{
		tmp_battery_img_para = &_gBottomBatteryImgPara[1];
	}
	else if(value_elec > 19 && value_elec < 80)
	{
		tmp_battery_img_para = &_gBottomBatteryImgPara[2];
	}
	else if(value_elec >= 0 && value_elec < 20)
	{
		tmp_battery_img_para = &_gBottomBatteryImgPara[3];
	}
	
	/*低电量时，电量百分比文本变成红色；其余状态时，电量百分比文本正常显示*/
	if(value_elec > 0 && value_elec < 20)
	{
		_gBottomBatteryTextPara.text_color = 0x006A62FF;
	}
	_gBottomBatteryTextPara.digit_value = value_elec;
	sprintf(_gBottomBatteryTextPara.p_content, "%d%%", (uint16_t)_gBottomBatteryTextPara.digit_value);
	TEXT_SetTextColor(_gTextBottomBatteryHandle,_gBottomBatteryTextPara.text_color);
	TEXT_SetText(_gTextBottomBatteryHandle, (const char *)_gBottomBatteryTextPara.p_content);
	
	IMAGE_SetDTA(_gImgBottomBatteryHandle, tmp_battery_img_para->p_img_buf,tmp_battery_img_para->img_size); 	
	return 0;
}
/**
  * @brief  设置界面中温度文本以及其显示的状态条
  * @param  value_temp:温度范围  <0 蓝色（cold） >0 && <60 青色(normal)  else 红色（hot）
  * @param  
  * @retval 0:success
  * @notes  none
  */
int32_t SetTempStaBarDayTime(float value_temp)
{
	if(value_temp < (float)0.000001)
	{
		return -1;
	}

	if(value_temp < 0)
	{
		//cold
		GUI_SetColor(0x00FDA900);//上色
		GUI_FillRoundedRect(336, 198-58,336+77,198-58 + 20, 10);
	}
	else if(value_temp > 0 && value_temp < 60)
	{
		//normal
		GUI_SetColor(0x00B2BE00);//上色
		GUI_FillRoundedRect(336, 198-58,336+77,198-58 + 20, 10);
	}
	else
	{
		//hot
		GUI_SetColor(0x00453BFA);//上色
		GUI_FillRoundedRect(336, 198-58,336+77,198-58 + 20, 10);
	}
	
	_gBottomTempTextPara.digit_value = value_temp;
	sprintf(_gBottomTempTextPara.p_content, "%.1fF", _gBottomTempTextPara.digit_value);
	TEXT_SetText(_gTextBottomTempHandle, (const char *)_gBottomTempTextPara.p_content);
	return 0;
}
/**
  * @brief  设置界面中设备与电网之间的图标流向，馈电、用电、停用
  * @param  stat:用电状态
  * @param  
  * @retval 0:success
  * @notes  none
  */
int32_t SetGridPowerstatDayTime(GRID_STAT_E stat)
{
	if(stat > GRID_STAT_NUM)
	{
		return -1;
	}
	
	emWidgetImgPara_t  *tmp_gridstat_img_para = &_gGridActiveImgPara[0];
	
	switch((int32_t)stat)
	{
		case GRID_ACTIVE_IN:
		{
			tmp_gridstat_img_para = &_gGridActiveImgPara[0];
			break;
		}
		case GRID_ACTIVE_OUT:
		{
			tmp_gridstat_img_para = &_gGridActiveImgPara[1];
			break;
		}
		case GRID_DISABLED:
		{
			tmp_gridstat_img_para = &_gGridActiveImgPara[2];
			break;
		}
		default:break;
	}
	IMAGE_SetDTA(_gImgGridActiveHandle,tmp_gridstat_img_para->p_img_buf, tmp_gridstat_img_para->img_size);
	return 0;
}
/**
  * @brief  设置界面中设备与家庭之间的图标关系，用电、停电
  * @param  stat:用电状态
  * @param  
  * @retval 0:success
  * @notes  none
  */
int32_t SetHomePowerstatDayTime(HOME_STAT_E stat)
{
	if(stat > HOME_STAT_NUM)
	{
		return -1;
	}
	
	emWidgetImgPara_t  *tmp_homestat_img_para = &_gHomeActiveImgPara[0];
	switch ((int32_t)stat)
	{
        case HOME_ACTIVE:
		{
			tmp_homestat_img_para = &_gHomeActiveImgPara[0];
			break;
		}
        case HOME_DISABLED:
		{
			tmp_homestat_img_para = &_gHomeActiveImgPara[1];
			break;
		}
		default:break;
	}

	IMAGE_SetDTA(_gImgHomeActiveHandle, tmp_homestat_img_para->p_img_buf,  tmp_homestat_img_para->img_size); 
	
	return 0;
}
/**
  * @brief  设置界面中太阳能功率状态流线图，发电、停电
  * @param  stat:用电状态
  * @param  
  * @retval 0:success
  * @notes  none
  */
int32_t SetSolarPowerstatDayTime(SOLAR_STAT_E stat)
{
	if(stat > SOLAR_STAT_NUM)
	{
		return -1;
	}
	
	emWidgetImgPara_t  *tmp_solarstat_img_para = &_gSolarActiveImgPara[0];
	switch ((int32_t)stat)
	{
        case SOLAR_ACTIVE:
		{
			tmp_solarstat_img_para = &_gSolarActiveImgPara[0];
			break;
		}
        case SOLAR_DISABLED:
		{
			tmp_solarstat_img_para = &_gSolarActiveImgPara[1];
			break;
		}
		case SOLAR_DAYTIME:
		{
			tmp_solarstat_img_para = &_gSolarActiveImgPara[2];
			break;
		}
		default:break;
	}

	IMAGE_SetDTA(_gImgSolarActiveHandle, tmp_solarstat_img_para->p_img_buf,  tmp_solarstat_img_para->img_size); 
	
	return 0;
}
