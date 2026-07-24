/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_main_3d_nav.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-12-18
  * @brief       启动界面
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
#include "em_main.h"
#include <time.h>

#define EM_MAIN_3D_NAV_SELECT_SIZE					(4 * 1024)
#define EM_MAIN_3D_NAV_DEV_STAT_SIZE			    (4 * 1024 * 8)
#define EM_MAIN_3D_NAV_SETTING_SIZE					(4 * 1024 * 8)
#define EM_MAIN_3D_NAV_DEV_INFO_SIZE				(4 * 1024 * 8)
#define EM_MAIN_3D_NAV_FACTORY_RESTORE_SIZE			(4 * 1024 * 8)


#define EM_MAIN_3D_NAV_WIFI_ICON_SIZE				(4 * 1024)
#define EM_MAIN_3D_NAV_WARNING_ICON_SIZE			(4 * 1024)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gEmWinMain3DNav = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int8_t select;

	uint16_t work_mode;

	uint16_t electic_purchase_day;
	uint16_t electic_sale_day;
	uint32_t pv_sum_generation_on_day;
	uint16_t load_cost_day;

	uint16_t soc;

	uint16_t charge_type;

    uint16_t chassis_internal_temp;

    uint16_t net_state;

    uint32_t time;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

static IMAGE_Handle _gImgStatBarWifiHandle;//状态栏WIFI图标句柄
static IMAGE_Handle _gImgStatBarWarningHandle;//状态栏WIFI图标句柄
static TEXT_Handle  _gTextStatBarTimeHandle;//状态栏时间句柄
static TEXT_Handle  _gTextStatBarWorkMode1Handle;//状态栏时间句柄
static TEXT_Handle  _gTextStatBarWorkMode2Handle;//状态栏时间句柄
static TEXT_Handle  _gTextStatBarWarningHandle;//状态栏时间句柄


/*状态栏窗口参数*/
static emWidgetImgPara_t  _gStatBarWifiImgPara[2];		// 0:disconnect, 1:connect
static emWidgetImgPara_t  _gStatBarWarningImgPara[2];		// 0:WARNING, 1ERROR:
static emWidgetTextPara_t  _gStatBarTimeTextPara;
static emWidgetTextPara_t  _gStatBarWorkMode1TextPara;
static emWidgetTextPara_t  _gStatBarWorkMode2TextPara;
static emWidgetTextPara_t  _gStatBarWarningTextPara;

static char _gStatBarTimeTextBuf[80] = {0}; // 2024 Jan 1th 00:00:01
static char _gStatBarWorkMode1TextBuf[32] = {0}; 
static char _gStatBarWorkMode2TextBuf[32] = {0}; 
static char _gStatBarWarningTextBuf[8] = {0}; 



/*四个选项句柄和参数*/
static IMAGE_Handle _gImgDevStatHandle;//电量统计之市电充电图标句柄
static emWidgetImgPara_t  _gDevStatImgPara;

static IMAGE_Handle _gImgSettingHandle;//电量统计之市电充电图标句柄
static emWidgetImgPara_t  _gSettingImgPara;

static IMAGE_Handle _gImgDevInfoHandle;//电量统计之市电充电图标句柄
static emWidgetImgPara_t  _gDevInfoImgPara;

static IMAGE_Handle _gImgFactoryRestoreHandle;//电量统计之市电充电图标句柄
static emWidgetImgPara_t  _gFactoryRestoreImgPara;

/*选择三角形句柄和参数*/
static IMAGE_Handle _gImgNavSelectHandle;
static emWidgetImgPara_t  _gImgNavSelectPara;

/*顶部状态栏控件ID号*/
#define WIN_MAIN_3D_NAV_ID								(GUI_ID_USER + 0x10)
#define WIN_MAIN_3D_NAV_WIFI_IMG_ID						(GUI_ID_USER + 0x11)
#define WIN_MAIN_3D_NAV_WARNING_IMG_ID					(GUI_ID_USER + 0x12)
#define WIN_MAIN_3D_NAV_TIME_TEXT_ID					(GUI_ID_USER + 0x13)
#define WIN_MAIN_3D_NAV_WORK_MODE1_TEXT_ID				(GUI_ID_USER + 0x14)
#define WIN_MAIN_3D_NAV_WORK_MODE2_TEXT_ID				(GUI_ID_USER + 0x15)
#define WIN_MAIN_3D_NAV_WARNING_TEXT_ID					(GUI_ID_USER + 0x17)
/*四个选项ID*/
#define WIN_MAIN_3D_NAV_DEV_STAT_IMG_ID					(GUI_ID_USER + 0x30)
#define WIN_MAIN_3D_NAV_SETTING_IMG_ID					(GUI_ID_USER + 0x40)
#define WIN_MAIN_3D_NAV_DEV_INFO_IMG_ID					(GUI_ID_USER + 0x51)
#define WIN_MAIN_3D_NAV_FACTORY_IMG_ID					(GUI_ID_USER + 0x61)
/*选择三角形*/
#define WIN_MAIN_3D_NAV_SELECT_ID						(GUI_ID_USER + 0x21)

static const GUI_WIDGET_CREATE_INFO _aMain3dNavDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "Main3dNavWin",    WIN_MAIN_3D_NAV_ID,         		   0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "time",            WIN_MAIN_3D_NAV_TIME_TEXT_ID,         0,   15, 158,   28,  0,   0x0,  0 },  
	{ TEXT_CreateIndirect,    "mode",            WIN_MAIN_3D_NAV_WORK_MODE1_TEXT_ID,   156, 15,  84,   28,  0,   0x0,  0 },  
	{ TEXT_CreateIndirect,    "preset",          WIN_MAIN_3D_NAV_WORK_MODE2_TEXT_ID,   252, 15,  100,  28,  0,   0x0,  0 },  
	{ TEXT_CreateIndirect,    "TextAlert",		 WIN_MAIN_3D_NAV_WARNING_TEXT_ID,      390, 23,  22,   18,  0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "ImgAlert",		 WIN_MAIN_3D_NAV_WARNING_IMG_ID,       415, 15,  20,   20,  0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "ImgWifi",		 WIN_MAIN_3D_NAV_WIFI_IMG_ID,          439, 15,  28,   28,  0,   0x0,  0 },
	
	{ IMAGE_CreateIndirect,   "OptState",		 WIN_MAIN_3D_NAV_DEV_STAT_IMG_ID,      46,  100, 85,   110,  0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "Setting",		 WIN_MAIN_3D_NAV_SETTING_IMG_ID,       146, 100, 85,   110,  0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "DeviceInfo",		 WIN_MAIN_3D_NAV_DEV_INFO_IMG_ID,      246, 100, 85,   110,  0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "Factory",		 WIN_MAIN_3D_NAV_FACTORY_IMG_ID,       346, 100, 85,   110,  0,   0x0,  0 },
    
    { IMAGE_CreateIndirect,   "Select",		     WIN_MAIN_3D_NAV_SELECT_ID,            0,   204, 25,   25,   0,   0x0,  0 },
};

static int32_t _Set3dNavWifiStat(WIFI_STAT_E stat)
{
	emWidgetImgPara_t  *tmp_wifi_img_para = &_gStatBarWifiImgPara[0];
	switch (stat)
	{
        case WIFI_STAT_DISCONN:
		{
			tmp_wifi_img_para = &_gStatBarWifiImgPara[0];
			break;
		}
        case WIFI_STAT_CONN:
		{
			tmp_wifi_img_para = &_gStatBarWifiImgPara[1];
			break;
		}
		default:break;
	}

	IMAGE_SetDTA(_gImgStatBarWifiHandle, tmp_wifi_img_para->p_img_buf, 
				 tmp_wifi_img_para->img_size); 
	return 0;
}

static int32_t _Set3dNavTime(uint32_t t)
{
	struct tm* timeinfo;

	timeinfo = localtime(&t); // 根据时间戳生成tm结构体对象
	//strftime(_gStatBarTimeTextBuf, sizeof(_gStatBarTimeTextBuf), "%Y %b %eth %H:%M:%S", 
	strftime(_gStatBarTimeTextBuf, sizeof(_gStatBarTimeTextBuf), "%b %eth %H:%M:%S", 
			 timeinfo); // 按指定格式将时间信息写入字符串
	TEXT_SetText(_gTextStatBarTimeHandle, (const char *)_gStatBarTimeTextPara.p_content);

	return 0;
}

int32_t _Set3dNavSelect(uint32_t idx)
{
	int32_t x_idx = -100;

	switch (idx)
	{
		case 0:
			x_idx =  _aMain3dNavDialogCreate[7].x0 -_aMain3dNavDialogCreate[11].xSize / 2 + _aMain3dNavDialogCreate[7].xSize / 2;
			break;
		case 1:
			x_idx =  _aMain3dNavDialogCreate[8].x0 -_aMain3dNavDialogCreate[11].xSize / 2 + _aMain3dNavDialogCreate[8].xSize / 2;
			break;
		case 2:
			x_idx =  _aMain3dNavDialogCreate[9].x0 -_aMain3dNavDialogCreate[11].xSize / 2 + _aMain3dNavDialogCreate[9].xSize / 2;
			break;
		case 3:
			x_idx =  _aMain3dNavDialogCreate[10].x0 -_aMain3dNavDialogCreate[11].xSize / 2 + _aMain3dNavDialogCreate[10].xSize / 2;
			break;
		default:
			break;

	}

	WM_MoveTo(_gImgNavSelectHandle, x_idx, _aMain3dNavDialogCreate[11].y0);
	return 0;
}


int32_t _Set3DNavWarning(uint32_t type, uint32_t num)
{
	if (type == 0)
	{
		_gStatBarWarningTextPara.text_color = 0X0000BEF7;
	}
	else if (type == 1)
	{
		_gStatBarWarningTextPara.text_color = 0X006A62FF;
	}
	else 
	{
		return -1;
	}


	if (num > 999)
	{
		return -2;
	}
	IMAGE_SetDTA(_gImgStatBarWarningHandle, _gStatBarWarningImgPara[type].p_img_buf, 
				 _gStatBarWarningImgPara[type].img_size); 

	snprintf(_gStatBarWarningTextBuf, sizeof(_gStatBarWarningTextBuf), "%.3d", num);
	TEXT_SetTextColor(_gTextStatBarWarningHandle, _gStatBarWarningTextPara.text_color); 
	TEXT_SetText(_gTextStatBarWarningHandle, (const char *)_gStatBarWarningTextPara.p_content);

	return 0;
}

int32_t _Set3DNavPcsStat(PCS_STAT_E mode)
{
	switch (mode)
	{
		case PCS_STAT_SHUTDOWN:
			{
				snprintf(_gStatBarWorkMode1TextBuf, sizeof(_gStatBarWorkMode1TextBuf), 
						 "%s", "Shutdown");
				break;
			}
		case PCS_STAT_STAND_BY:
			{
				snprintf(_gStatBarWorkMode1TextBuf, sizeof(_gStatBarWorkMode1TextBuf), 
						 "%s", "Stand-by");
				break;
			}
		case PCS_STAT_SELF_TEST:
			{
				snprintf(_gStatBarWorkMode1TextBuf, sizeof(_gStatBarWorkMode1TextBuf), 
						 "%s", "Self-test");
				break;
			}
		case PCS_STAT_BY_PASS:
			{
				snprintf(_gStatBarWorkMode1TextBuf, sizeof(_gStatBarWorkMode1TextBuf), 
						 "%s", "Bypass");
				break;
			}
		case PCS_STAT_OFF_GRID:
			{
				snprintf(_gStatBarWorkMode1TextBuf, sizeof(_gStatBarWorkMode1TextBuf), 
						 "%s", "Off Grid");
				break;
			}
		case PCS_STAT_ON_GRID:
			{
				snprintf(_gStatBarWorkMode1TextBuf, sizeof(_gStatBarWorkMode1TextBuf), 
						 "%s", "On Grid");
				break;
			}
		default:
			break;
	}

	TEXT_SetText(_gTextStatBarWorkMode1Handle, 
				 (const char *)_gStatBarWorkMode1TextPara.p_content);
	return 0;
}

int32_t _Set3DNavWorkMode(WORK_MODE_E mode)
{
	switch (mode)
	{
		case WORK_MODE_PL_BALANCE:
			{
				snprintf(_gStatBarWorkMode2TextBuf, sizeof(_gStatBarWorkMode2TextBuf), 
						 "%s", "PL-Balance");
				break;
			}
		case WORK_MODE_SELF_CONSUMP:
			{
				snprintf(_gStatBarWorkMode2TextBuf, sizeof(_gStatBarWorkMode2TextBuf), 
						 "%s", "Self Consump");
				break;
			}
		case WORK_MODE_BAT:
			{
				snprintf(_gStatBarWorkMode2TextBuf, sizeof(_gStatBarWorkMode2TextBuf), 
						 "%s", "BAT Prioity");
				break;
			}
		default:
			break;
	}

	TEXT_SetText(_gTextStatBarWorkMode2Handle, 
				 (const char *)_gStatBarWorkMode2TextPara.p_content);
	return 0;
}



/**
  * @brief main Win callback
  * @param  NULL
  * @retval NULL
  */
static void _cbMain3dNavWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
	
	static time_t cur_time = 1704067200;
    static int     wifi_type = 0;
	
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
			/*时间*/
			_gTextStatBarTimeHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_TIME_TEXT_ID);
            TEXT_SetFont(_gTextStatBarTimeHandle, _gStatBarTimeTextPara.p_text_font);
            TEXT_SetText(_gTextStatBarTimeHandle, (const char *)_gStatBarTimeTextPara.p_content);
            TEXT_SetTextAlign(_gTextStatBarTimeHandle, TEXT_CF_HCENTER | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarTimeHandle, _gStatBarTimeTextPara.text_color); 
			
			/*Mode*/
			_gTextStatBarWorkMode1Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_WORK_MODE1_TEXT_ID);
            TEXT_SetFont(_gTextStatBarWorkMode1Handle, _gStatBarWorkMode1TextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWorkMode1Handle, (const char *)_gStatBarWorkMode1TextPara.p_content);
            TEXT_SetTextAlign(_gTextStatBarWorkMode1Handle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWorkMode1Handle, _gStatBarWorkMode1TextPara.text_color); 
			
			/*Preset*/
			_gTextStatBarWorkMode2Handle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_WORK_MODE2_TEXT_ID);
            TEXT_SetFont(_gTextStatBarWorkMode2Handle, _gStatBarWorkMode2TextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWorkMode2Handle, (const char *)_gStatBarWorkMode2TextPara.p_content);
            TEXT_SetTextAlign(_gTextStatBarWorkMode2Handle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWorkMode2Handle, _gStatBarWorkMode2TextPara.text_color); 
			
			/*Test Alert*/
			_gTextStatBarWarningHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_WARNING_TEXT_ID);
            TEXT_SetFont(_gTextStatBarWarningHandle, _gStatBarWarningTextPara.p_text_font);
            TEXT_SetText(_gTextStatBarWarningHandle, (const char *)_gStatBarWarningTextPara.p_content);
            TEXT_SetTextAlign(_gTextStatBarWarningHandle, TEXT_CF_RIGHT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextStatBarWarningHandle, _gStatBarWarningTextPara.text_color); 
			
			/*Img Alert*/
			_gImgStatBarWarningHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_WARNING_IMG_ID);
			IMAGE_SetDTA(_gImgStatBarWarningHandle, _gStatBarWarningImgPara[0].p_img_buf, _gStatBarWarningImgPara[0].img_size); 
			
			/*Img Wifi*/
			_gImgStatBarWifiHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_WIFI_IMG_ID);
			IMAGE_SetDTA(_gImgStatBarWifiHandle, _gStatBarWifiImgPara[wifi_type].p_img_buf, _gStatBarWifiImgPara[wifi_type].img_size); 
			
			/*Img OptState*/
			_gImgDevStatHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_DEV_STAT_IMG_ID);
            IMAGE_SetDTA(_gImgDevStatHandle, _gDevStatImgPara.p_img_buf, _gDevStatImgPara.img_size); 
			
			/*Img Settings*/
			_gImgSettingHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_SETTING_IMG_ID);
            IMAGE_SetDTA(_gImgSettingHandle, _gSettingImgPara.p_img_buf, _gSettingImgPara.img_size); 
			
			/*Img DeviceInfo*/
			_gImgDevInfoHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_DEV_INFO_IMG_ID);
            IMAGE_SetDTA(_gImgDevInfoHandle, _gDevInfoImgPara.p_img_buf, _gDevInfoImgPara.img_size); 
			
			/*Img Factory*/
			_gImgFactoryRestoreHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_FACTORY_IMG_ID);
            IMAGE_SetDTA(_gImgFactoryRestoreHandle, _gFactoryRestoreImgPara.p_img_buf, _gFactoryRestoreImgPara.img_size); 
			
			/*Img Select*/
			_gImgNavSelectHandle = WM_GetDialogItem(pMsg->hWin, WIN_MAIN_3D_NAV_SELECT_ID);
			IMAGE_SetDTA(_gImgNavSelectHandle, _gImgNavSelectPara.p_img_buf, _gImgNavSelectPara.img_size); 
            
			_Set3dNavSelect((uint32_t)g_ui_ctx.select);
            _Set3dNavTime(cur_time);
            break;
        }
        case WM_PAINT:
        {
            //GUI_DrawStreamedBitmapAuto(_pBKBuf, 0, 0);
            GUI_SetColor(0x434343);
			GUI_FillRoundedRect(158, 15, 158 + 200 - 1, 15 + 28 - 1, 6);
            GUI_SetColor(0x797979);      
            GUI_SetPenSize(2);
            GUI_DrawLine(246, 22, 246, 22 + 13 - 1);                                
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
    GUI_EnableAlpha(0);
    /*创建一个基于桌面窗口的子窗口*/
	gEmWinMain3DNav.win_handle = GUI_CreateDialogBox(_aMain3dNavDialogCreate, 
													 GUI_COUNTOF(_aMain3dNavDialogCreate), 
													 _cbMain3dNavWin, 
													 pCurrentEmWin->win_handle, 
													 0, 0);
    //WM_SetTransState(_gWinStatBarHandle, WM_CF_HASTRANS);

    pCurrentEmWin = &gEmWinMain3DNav;

    GUI_Exec();
    return gEmWinMain3DNav.win_handle;
}


/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinMain3DNav.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinMain3DNav.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinMain3DNav.win_handle = 0;
}



/**
  * @brief  界面控件等初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	/*UI内存申请，当内存足够大时，界面初始化全部申请好，界面关闭时再释放，如果内存不足，则用时再申请，用完释放*/	
	
	_gStatBarTimeTextPara.p_content = _gStatBarTimeTextBuf;
    _gStatBarTimeTextPara.text_color = 0X00B2BE00;
    _gStatBarTimeTextPara.p_text_font = EM_TEXT_FONT_7;
	
	_gStatBarWorkMode1TextPara.p_content = _gStatBarWorkMode1TextBuf;
    _gStatBarWorkMode1TextPara.text_color = 0X0036B6FF;
    _gStatBarWorkMode1TextPara.p_text_font = EM_TEXT_FONT_5;
	
	_gStatBarWorkMode2TextPara.p_content = _gStatBarWorkMode2TextBuf;
    _gStatBarWorkMode2TextPara.text_color = 0X00D9D9D9;
    _gStatBarWorkMode2TextPara.p_text_font = EM_TEXT_FONT_5;
	
	snprintf(_gStatBarWorkMode1TextBuf, sizeof(_gStatBarWorkMode1TextBuf), "%s", "stand-by");
	snprintf(_gStatBarWorkMode2TextBuf, sizeof(_gStatBarWorkMode2TextBuf), "%s", "BAT Prioity");
    snprintf(_gStatBarWarningTextBuf, sizeof(_gStatBarWarningTextBuf), "%s", "009");
	
	_gStatBarWarningImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_WARNING_ICON_SIZE);
    _gStatBarWarningImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gStatBarWarningImgPara[0].mem_img);
    _gStatBarWarningImgPara[0].img_size = EM_MAIN_3D_NAV_WARNING_ICON_SIZE;
    SpiFlashReadData(EM_ICON_ALERT_ADDR, _gStatBarWarningImgPara[0].p_img_buf, _gStatBarWarningImgPara[0].img_size);
	
	_gStatBarWarningImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_WARNING_ICON_SIZE);
    _gStatBarWarningImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gStatBarWarningImgPara[1].mem_img);
    _gStatBarWarningImgPara[1].img_size = EM_MAIN_3D_NAV_WARNING_ICON_SIZE;
    SpiFlashReadData(EM_ICON_ERROR_ADDR, _gStatBarWarningImgPara[1].p_img_buf, _gStatBarWarningImgPara[1].img_size);
	
	_gStatBarWifiImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_WIFI_ICON_SIZE);
    _gStatBarWifiImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gStatBarWifiImgPara[0].mem_img);
    _gStatBarWifiImgPara[0].img_size = EM_MAIN_3D_NAV_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_OFF_ADDR, _gStatBarWifiImgPara[0].p_img_buf, _gStatBarWifiImgPara[0].img_size);
	
	_gStatBarWifiImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_WIFI_ICON_SIZE);
    _gStatBarWifiImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gStatBarWifiImgPara[1].mem_img);
    _gStatBarWifiImgPara[1].img_size = EM_MAIN_3D_NAV_WIFI_ICON_SIZE;
    SpiFlashReadData(EM_ICON_WIFI_ON_ADDR, _gStatBarWifiImgPara[1].p_img_buf, _gStatBarWifiImgPara[1].img_size);
	
	_gDevStatImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_DEV_STAT_SIZE);
    _gDevStatImgPara.p_img_buf = GUI_ALLOC_h2p(_gDevStatImgPara.mem_img);
    _gDevStatImgPara.img_size = EM_MAIN_3D_NAV_DEV_STAT_SIZE;
    SpiFlashReadData(EM_ICON_3D_DEV_STAT_ADDR, _gDevStatImgPara.p_img_buf, _gDevStatImgPara.img_size);
	
	_gSettingImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_SETTING_SIZE);
    _gSettingImgPara.p_img_buf = GUI_ALLOC_h2p(_gSettingImgPara.mem_img);
    _gSettingImgPara.img_size = EM_MAIN_3D_NAV_SETTING_SIZE;
    SpiFlashReadData(EM_ICON_3D_DEV_SETTING_ADDR, _gSettingImgPara.p_img_buf, _gSettingImgPara.img_size);
	
	_gDevInfoImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_DEV_INFO_SIZE);
    _gDevInfoImgPara.p_img_buf = GUI_ALLOC_h2p(_gDevInfoImgPara.mem_img);
    _gDevInfoImgPara.img_size = EM_MAIN_3D_NAV_DEV_INFO_SIZE;
    SpiFlashReadData(EM_ICON_3D_DEV_INFO_ADDR, _gDevInfoImgPara.p_img_buf, _gDevInfoImgPara.img_size);
	
	_gFactoryRestoreImgPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_FACTORY_RESTORE_SIZE);
    _gFactoryRestoreImgPara.p_img_buf = GUI_ALLOC_h2p(_gFactoryRestoreImgPara.mem_img);
    _gFactoryRestoreImgPara.img_size = EM_MAIN_3D_NAV_FACTORY_RESTORE_SIZE;
    SpiFlashReadData(EM_ICON_3D_FACTORY_RESTORE_ADDR, _gFactoryRestoreImgPara.p_img_buf, _gFactoryRestoreImgPara.img_size);
	
	_gImgNavSelectPara.mem_img = GUI_ALLOC_AllocZero(EM_MAIN_3D_NAV_SELECT_SIZE);
	_gImgNavSelectPara.p_img_buf = GUI_ALLOC_h2p(_gImgNavSelectPara.mem_img);
	_gImgNavSelectPara.img_size = EM_MAIN_3D_NAV_SELECT_SIZE;
	SpiFlashReadData(EM_ICON_3D_NAV_SELECT_ADDR, _gImgNavSelectPara.p_img_buf, _gImgNavSelectPara.img_size);
	
	EmKeyDefaultInfoSet();
}


/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
	GUI_ALLOC_Free(_gStatBarWarningImgPara[0].mem_img);
	GUI_ALLOC_Free(_gStatBarWarningImgPara[1].mem_img);
	GUI_ALLOC_Free(_gStatBarWifiImgPara[0].mem_img);
    GUI_ALLOC_Free(_gStatBarWifiImgPara[1].mem_img);
	GUI_ALLOC_Free(_gDevStatImgPara.mem_img);
	GUI_ALLOC_Free(_gSettingImgPara.mem_img);
	GUI_ALLOC_Free(_gDevInfoImgPara.mem_img);
	GUI_ALLOC_Free(_gFactoryRestoreImgPara.mem_img);
	GUI_ALLOC_Free(_gImgNavSelectPara.mem_img);
}



/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
	if (gCollectParaData.collect_data.work_mode != g_ui_ctx.work_mode)
	{
		g_ui_ctx.work_mode = gCollectParaData.collect_data.work_mode;
		_Set3DNavWorkMode((WORK_MODE_E)g_ui_ctx.work_mode);
	}

	if (g_ui_ctx.net_state)
	{
		_Set3dNavWifiStat((WIFI_STAT_E)g_ui_ctx.net_state);
	}
    
	if (g_ui_ctx.time > 0)
	{
		_Set3dNavTime(g_ui_ctx.time);
	}

}


/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
//	int32_t event = GetKeyEvent(p_msg, &g_ui_ctx.key_init);
    int32_t event = EmGetKeyEvent(p_msg);
	if (event < 0 || event > EN_KEY_SRC_NUM)
	{
		return;
	}
	if (event == EN_KEY_SRC_LEFT)
	{
		g_ui_ctx.select = (--g_ui_ctx.select) < 0 ? 0 : g_ui_ctx.select;
		_Set3dNavSelect((uint32_t)g_ui_ctx.select);
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		g_ui_ctx.select = (++g_ui_ctx.select) > 3 ? 3 : g_ui_ctx.select;
		_Set3dNavSelect((uint32_t)g_ui_ctx.select);
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		emWin_t *pnext = NULL;
		switch(g_ui_ctx.select)
		{
			case 0:
				pnext = &gEmWinListEquipOperaInfo;
				break;
			case 1:
				pnext = &gEmWinListSettingsInfo;
				break;
			case 2:
				pnext = &gEmWinListDevBaseInfo;
				break;
			case 3:
				pnext = &gEmWinListFactoryRestore;
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
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = &gEmWinHomePage;
			pCurrentEmWin->create_win(); 
		}
		return;
	}
}


