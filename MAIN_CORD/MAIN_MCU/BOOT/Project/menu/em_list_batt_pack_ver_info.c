/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_LIST_BATTERY_PACK__VERSION_INFO.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-25
  * @brief       电池包固件版本信息
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-25   <td>V1.0.0    <td>           <td>InitialVersion
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


emWin_t  gEmWinListBatteryVersionInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;

	uint16_t hw_ver[BMS_MAX_NUM];
	uint16_t sw_ver[BMS_MAX_NUM];
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;

static char _gTitleBuf[] = "Basic Info < Battery Pack Version";

/*List控件*/
static emListPara_t  _gEmListBatteryPackVersionInfoPara;

static char _gListBatteryPackVersionInfoLeftTextBuf[3][20];
static char _gListBatteryPackVersionInfoRightTextBuf[3][20];


#define WIN_LIST_BATTERY_PACK_VERSION_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_BATTERY_PACK_VERSION_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static const GUI_WIDGET_CREATE_INFO _aListBatteryPackVersionInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListBatteryPackVersionInfoWin",    WIN_LIST_BATTERY_PACK_VERSION_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_BATTERY_PACK_VERSION_INFO_TITLE_ID,     31,  20, 415,  17,   0,   0x0,  0 },
};


static int32_t _SetPackVer(int32_t idx, uint16_t hw, uint16_t sw)
{
	if (idx < 0 || idx >= sizeof(_gListBatteryPackVersionInfoRightTextBuf)/
		sizeof(_gListBatteryPackVersionInfoRightTextBuf[0]))
	{
		return -1;
	}
    snprintf(_gListBatteryPackVersionInfoRightTextBuf[idx], 
			 sizeof(_gListBatteryPackVersionInfoRightTextBuf[idx]), "%4X | %4X", hw, sw);
	TEXT_SetText(_gEmListBatteryPackVersionInfoPara.handle[1][idx], 
				(const char *)_gEmListBatteryPackVersionInfoPara.sub_widget_para[1][idx].sub_widget.text_para.p_content);
	return 0;
}

/**
  * @brief ListBatteryPackVersionInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListBatteryPackVersionInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BATTERY_PACK_VERSION_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListBatteryPackVersionInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListBatteryPackVersionInfoPara, _gEmListBatteryPackVersionInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(int i = 0; i < _gEmListBatteryPackVersionInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListBatteryPackVersionInfoPara.sub_region[i].x_pos, 
                                    _gEmListBatteryPackVersionInfoPara.sub_region[i].y_pos, 
                                    _gEmListBatteryPackVersionInfoPara.sub_region[i].x_pos + _gEmListBatteryPackVersionInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListBatteryPackVersionInfoPara.sub_region[i].y_pos + _gEmListBatteryPackVersionInfoPara.sub_region[i].y_size - 1,
                                    _gEmListBatteryPackVersionInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListBatteryPackVersionInfoPara, _gEmListBatteryPackVersionInfoPara.list_sel);
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
	pPrevEmWin = &gEmWinListDevBaseInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListBatteryVersionInfo.win_handle = GUI_CreateDialogBox(_aListBatteryPackVersionInfoDialogCreate, GUI_COUNTOF(_aListBatteryPackVersionInfoDialogCreate), _cbListBatteryPackVersionInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListBatteryVersionInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListBatteryVersionInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListBatteryVersionInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListBatteryPackVersionInfoPara, gEmWinListBatteryVersionInfo.win_handle);
    return gEmWinListBatteryVersionInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListBatteryVersionInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListBatteryVersionInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListBatteryVersionInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;

	for (int32_t i = 0; i < BMS_MAX_NUM; ++i)
	{
		if (g_ui_ctx.hw_ver[i] != gCollectParaData.bms_data[i].bms_front_para.hw_version
			|| g_ui_ctx.sw_ver[i] != gCollectParaData.bms_data[i].bms_front_para.sw_version)
		{
			g_ui_ctx.hw_ver[i] = gCollectParaData.bms_data[i].bms_front_para.hw_version;
			g_ui_ctx.sw_ver[i] = gCollectParaData.bms_data[i].bms_front_para.sw_version;
		}

	}
    
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListBatteryPackVersionInfoPara.list_start_id = WIN_LIST_BATTERY_PACK_VERSION_INFO_ID + 0x30;
    _gEmListBatteryPackVersionInfoPara.list_num = 3;
    _gEmListBatteryPackVersionInfoPara.list_display_num = 6;
    if(_gEmListBatteryPackVersionInfoPara.list_num <= _gEmListBatteryPackVersionInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListBatteryPackVersionInfoPara.list_num + 1 - _gEmListBatteryPackVersionInfoPara.list_display_num;
    }
    
    if(_gEmListBatteryPackVersionInfoPara.list_sel > _gEmListBatteryPackVersionInfoPara.list_num - 1)
    {
        _gEmListBatteryPackVersionInfoPara.list_sel = 0;
    }
    if(_gEmListBatteryPackVersionInfoPara.list_page > max_page - 1)
    {
        _gEmListBatteryPackVersionInfoPara.list_page = 0;
    }
    _gEmListBatteryPackVersionInfoPara.list_bk_color = 0x00787878;
    /*3个子列表*/
    _gEmListBatteryPackVersionInfoPara.sub_region[0].x_pos = 20;
    _gEmListBatteryPackVersionInfoPara.sub_region[0].y_pos = 38;
    _gEmListBatteryPackVersionInfoPara.sub_region[0].x_size = 440;
    _gEmListBatteryPackVersionInfoPara.sub_region[0].y_size = 34;
    _gEmListBatteryPackVersionInfoPara.sub_region[1].x_pos = 20;
    _gEmListBatteryPackVersionInfoPara.sub_region[1].y_pos = 80;
    _gEmListBatteryPackVersionInfoPara.sub_region[1].x_size = 440;
    _gEmListBatteryPackVersionInfoPara.sub_region[1].y_size = 34;
	_gEmListBatteryPackVersionInfoPara.sub_region[2].x_pos = 20;
    _gEmListBatteryPackVersionInfoPara.sub_region[2].y_pos = 122;
    _gEmListBatteryPackVersionInfoPara.sub_region[2].x_size = 440;
    _gEmListBatteryPackVersionInfoPara.sub_region[2].y_size = 34;

    /*List控件*/

    //第一行左边
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListBatteryPackVersionInfoLeftTextBuf[0];
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBatteryPackVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Pack1");
    //第一行右边
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][0].x_relative_pos = 218;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][0].x_size = 210;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][0].y_size = 17;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListBatteryPackVersionInfoRightTextBuf[0];
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    snprintf(_gListBatteryPackVersionInfoRightTextBuf[0], 
			sizeof(_gListBatteryPackVersionInfoRightTextBuf[0]), "%4X | %4X", 
			g_ui_ctx.hw_ver[0], g_ui_ctx.sw_ver[0]);
    //第二行左边
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][1].x_size = 158;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListBatteryPackVersionInfoLeftTextBuf[1];
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBatteryPackVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Pack2");
    //第二行右边
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][1].x_relative_pos = 218;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][1].x_size = 210;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][1].y_size = 17;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListBatteryPackVersionInfoRightTextBuf[1];
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    snprintf(_gListBatteryPackVersionInfoRightTextBuf[1], 
			sizeof(_gListBatteryPackVersionInfoRightTextBuf[1]), "%4X | %4X", 
			g_ui_ctx.hw_ver[1], g_ui_ctx.sw_ver[1]);
	//第三行左边
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][2].x_size = 158;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListBatteryPackVersionInfoLeftTextBuf[2];
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBatteryPackVersionInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Pack3");
    //第三行右边
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][2].x_relative_pos = 218;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][2].x_size = 210;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][2].y_size = 17;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListBatteryPackVersionInfoRightTextBuf[2];
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryPackVersionInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    snprintf(_gListBatteryPackVersionInfoRightTextBuf[2], 
			sizeof(_gListBatteryPackVersionInfoRightTextBuf[2]), "%4X | %4X", 
			g_ui_ctx.hw_ver[2], g_ui_ctx.sw_ver[2]);
    
	
	_gEmListBatteryPackVersionInfoPara.selected_r = 10;
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
	for (int32_t i = 0; i < BMS_MAX_NUM; ++i)
	{
		if (g_ui_ctx.hw_ver[i] != gCollectParaData.bms_data[i].bms_front_para.hw_version
			|| g_ui_ctx.sw_ver[i] != gCollectParaData.bms_data[i].bms_front_para.sw_version)
		{
			g_ui_ctx.hw_ver[i] = gCollectParaData.bms_data[i].bms_front_para.hw_version;
			g_ui_ctx.sw_ver[i] = gCollectParaData.bms_data[i].bms_front_para.sw_version;
			_SetPackVer(i, g_ui_ctx.hw_ver[i], g_ui_ctx.sw_ver[i]);
		}

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
	
	if(event == EN_KEY_SRC_ESC)
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
				pnext = &gEmWinListDevBaseInfo;
			}
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win(); 
		}
		return;
	}
    
}


