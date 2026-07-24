/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-11
  * @brief       状态信息
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-12    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
  
#define  EM_BACK_ON_ICON_ARROW_SIZE                (4096)  
  
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListStatusInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
	
	uint16_t    system_status;
	uint16_t    invs_status;
	uint16_t    dcdc_status;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

typedef struct _inverter_status_t
{
	uint16_t status_id;
	char status[32];
} InverterStatus_t;

typedef struct _sys_status_t
{
	uint16_t status_id;
	char status[32];
	
} SysStatus_t;

typedef struct _dcdc_status_t
{
	uint16_t status_id;
	char status[32];
} DCDCStatus_t;

static InverterStatus_t inv_status[] = 
{
	{0x00,		"Stand-by"},
	{0x01,		"Off-Grid"},
	{0x02,		"On-Grid"},
	{0x03,		"Off-Grid to On-Grid"},
	{0x04,		"On-Grid to Off-Grid"},
};

static SysStatus_t sys_status[] = 
{
	{0x00,		"Stand-by"},
	{0x01,		"Self-test"},
	{0x02,		"Hybrid grid connection"},
	{0x03,		"Off-Grid"},
	{0x04,		"Bypass"},
};

static DCDCStatus_t dcdc_status[] = 
{
	{0x00,		"Stand-by"},
	{0x01,		"Soft-start"},
	{0x02,		"Charge mode"},
	{0x03,		"Discharge mode"},
};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara;

static char _gTitleBuf[] = "Operational State Info < Status Info";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";


/*List控件*/
static emListPara_t  _gEmListStatusPara;

static char _gListStatusLeftTextBuf[6][20];
static char _gListStatusRightTextBuf[6][20];


#define WIN_LIST_STATUS_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_STATUS_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_STATUS_INFO_PRESS_ID					      (GUI_ID_USER + 0x14)
#define WIN_LIST_STATUS_INFO_BACK_ID					      (GUI_ID_USER + 0x15)
#define WIN_LIST_STATUS_INFO_BACK_IMG_ID					  (GUI_ID_USER + 0x16)

static const GUI_WIDGET_CREATE_INFO _aListGeneratorInputDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListStatusInfoWin",    WIN_LIST_STATUS_INFO_ID,     		0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_STATUS_INFO_TITLE_ID,     			31,  20,  415,  14,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_STATUS_INFO_PRESS_ID,       		133, 273, 33,   14,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_STATUS_INFO_BACK_IMG_ID,    		171-5, 275, 15,   10,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_STATUS_INFO_BACK_ID,       		188, 273, 150,  14,   0,   0x0,  0 },
};

static uint32_t _SetSysStatus(int32_t ui_update, uint16_t sys_stat)
{
	if(sys_stat == sys_status[0].status_id)
	{
		snprintf(_gListStatusRightTextBuf[1],sizeof(_gListStatusRightTextBuf[1]), "%s", sys_status[0].status);
	}
	else if(sys_stat == sys_status[1].status_id)
	{
		snprintf(_gListStatusRightTextBuf[1],sizeof(_gListStatusRightTextBuf[1]), "%s", sys_status[1].status);
	}
	else if(sys_stat == sys_status[2].status_id)
	{
		snprintf(_gListStatusRightTextBuf[1],sizeof(_gListStatusRightTextBuf[1]), "%s", sys_status[2].status);
	}
	else if(sys_stat == sys_status[3].status_id)
	{
		snprintf(_gListStatusRightTextBuf[1],sizeof(_gListStatusRightTextBuf[1]), "%s", sys_status[3].status);
	}
	else if(sys_stat == sys_status[4].status_id)
	{
		snprintf(_gListStatusRightTextBuf[1],sizeof(_gListStatusRightTextBuf[1]), "%s", sys_status[4].status);
	}
	
	if (ui_update)
	{
		TEXT_SetText(_gEmListStatusPara.handle[1][1],(const char *)_gEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
	}
	return 0;
}

static uint32_t _SetInverterStatus(int32_t ui_update, uint16_t inv_stat)
{
	if(inv_stat == inv_status[0].status_id)
	{
		snprintf(_gListStatusRightTextBuf[0],sizeof(_gListStatusRightTextBuf[0]), "%s", inv_status[0].status);
	}
	else if(inv_stat == inv_status[1].status_id)
	{
		snprintf(_gListStatusRightTextBuf[0],sizeof(_gListStatusRightTextBuf[0]), "%s", inv_status[1].status);
	}
	else if(inv_stat == inv_status[2].status_id)
	{
		snprintf(_gListStatusRightTextBuf[0],sizeof(_gListStatusRightTextBuf[0]), "%s", inv_status[2].status);
	}
	else if(inv_stat == inv_status[3].status_id)
	{
		snprintf(_gListStatusRightTextBuf[0],sizeof(_gListStatusRightTextBuf[0]), "%s", inv_status[3].status);
	}
	else if(inv_stat == inv_status[4].status_id)
	{
		snprintf(_gListStatusRightTextBuf[0],sizeof(_gListStatusRightTextBuf[0]), "%s", inv_status[4].status);
	}
	if (ui_update)
	{
		TEXT_SetText(_gEmListStatusPara.handle[1][0],(const char *)_gEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
	}
	return 0;
}

static uint32_t _SetDCDCStatus(int32_t ui_update, uint16_t dcdc_stat)
{
	if(dcdc_stat == dcdc_status[0].status_id)
	{
		snprintf(_gListStatusRightTextBuf[2],sizeof(_gListStatusRightTextBuf[2]), "%s", dcdc_status[0].status);
	}
	else if(dcdc_stat == dcdc_status[1].status_id)
	{
		snprintf(_gListStatusRightTextBuf[2],sizeof(_gListStatusRightTextBuf[2]), "%s", dcdc_status[1].status);
	}
	else if(dcdc_stat == dcdc_status[2].status_id)
	{
		snprintf(_gListStatusRightTextBuf[2],sizeof(_gListStatusRightTextBuf[2]), "%s", dcdc_status[2].status);
	}
	else if(dcdc_stat == dcdc_status[3].status_id)
	{
		snprintf(_gListStatusRightTextBuf[2],sizeof(_gListStatusRightTextBuf[2]), "%s", dcdc_status[3].status);
	}

	if (ui_update)
	{
		TEXT_SetText(_gEmListStatusPara.handle[1][2],(const char *)_gEmListStatusPara.sub_widget_para[1][2].sub_widget.text_para.p_content);	
	}
	
	return 0;
}


/**
  * @brief ListStatusInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListStatusInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_STATUS_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListStatusPara, pMsg->hWin);
			
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_STATUS_INFO_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_STATUS_INFO_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_STATUS_INFO_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
			
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListStatusPara, _gEmListStatusPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(int i = 0; i < _gEmListStatusPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListStatusPara.sub_region[i].x_pos, 
                                    _gEmListStatusPara.sub_region[i].y_pos, 
                                    _gEmListStatusPara.sub_region[i].x_pos + _gEmListStatusPara.sub_region[i].x_size - 1, 
                                    _gEmListStatusPara.sub_region[i].y_pos + _gEmListStatusPara.sub_region[i].y_size - 1,
                                    _gEmListStatusPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListStatusPara, _gEmListStatusPara.list_sel);
			
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
	pPrevEmWin = &gEmWinListEquipOperaInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListStatusInfo.win_handle = GUI_CreateDialogBox(_aListGeneratorInputDialogCreate, GUI_COUNTOF(_aListGeneratorInputDialogCreate), _cbListStatusInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListStatusInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListStatusInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListStatusInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListStatusPara, gEmWinListStatusInfo.win_handle);
    return gEmWinListStatusInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListStatusInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListStatusInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListStatusInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	g_ui_ctx.system_status = gCollectParaData.pcs_input_front_data.system_status;
	g_ui_ctx.invs_status = gCollectParaData.pcs_input_front_data.invs_status;
	g_ui_ctx.dcdc_status = gCollectParaData.pcs_input_front_data.dcdc_status;

	EmKeyDefaultInfoSet();
    uint8_t max_page;
    
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
	
	
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListStatusPara.list_start_id = WIN_LIST_STATUS_INFO_ID + 0x30;
    _gEmListStatusPara.list_num = 4;
    _gEmListStatusPara.list_display_num = 6;
    if(_gEmListStatusPara.list_num <= _gEmListStatusPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListStatusPara.list_num + 1 - _gEmListStatusPara.list_display_num;
    }
    
    if(_gEmListStatusPara.list_sel > _gEmListStatusPara.list_num - 1)
    {
        _gEmListStatusPara.list_sel = 0;
    }
    if(_gEmListStatusPara.list_page > max_page - 1)
    {
        _gEmListStatusPara.list_page = 0;
    }
    _gEmListStatusPara.list_bk_color = 0x00787878;
	/*List选中框*/
    _gEmListStatusPara.selected_r = 10;
	
    /*3个子列表*/
    _gEmListStatusPara.sub_region[0].x_pos = 20;
    _gEmListStatusPara.sub_region[0].y_pos = 38;
    _gEmListStatusPara.sub_region[0].x_size = 440;
    _gEmListStatusPara.sub_region[0].y_size = 34;
    _gEmListStatusPara.sub_region[1].x_pos = 20;
    _gEmListStatusPara.sub_region[1].y_pos = 80;
    _gEmListStatusPara.sub_region[1].x_size = 440;
    _gEmListStatusPara.sub_region[1].y_size = 34;
    _gEmListStatusPara.sub_region[2].x_pos = 20;
    _gEmListStatusPara.sub_region[2].y_pos = 122;
    _gEmListStatusPara.sub_region[2].x_size = 440;
    _gEmListStatusPara.sub_region[2].y_size = 34;

    /*List控件*/

    //第一行左边
    _gEmListStatusPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListStatusPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListStatusPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListStatusPara.sub_widget_para[0][0].x_size = 158;
    _gEmListStatusPara.sub_widget_para[0][0].y_size = 17;
    _gEmListStatusPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListStatusLeftTextBuf[0];
    _gEmListStatusPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListStatusPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListStatusPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Inverter");
    //第一行右边
    _gEmListStatusPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListStatusPara.sub_widget_para[1][0].x_relative_pos = 218;
    _gEmListStatusPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListStatusPara.sub_widget_para[1][0].x_size = 208;
    _gEmListStatusPara.sub_widget_para[1][0].y_size = 17;
	_gEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListStatusRightTextBuf[0];
    _gEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	 strcpy(_gEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "Bypass");
//    _gEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 220;//测试数据，实际赋值系统数据
//    sprintf(_gEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "%dV", (uint16_t)_gEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.digit_value);
    //第二行左边
    _gEmListStatusPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListStatusPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListStatusPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListStatusPara.sub_widget_para[0][1].x_size = 158;
    _gEmListStatusPara.sub_widget_para[0][1].y_size = 17;
    _gEmListStatusPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListStatusLeftTextBuf[1];
    _gEmListStatusPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListStatusPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListStatusPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "PV Running");
    //第二行右边
    _gEmListStatusPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListStatusPara.sub_widget_para[1][1].x_relative_pos = 218;
    _gEmListStatusPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gEmListStatusPara.sub_widget_para[1][1].x_size = 208;
    _gEmListStatusPara.sub_widget_para[1][1].y_size = 17;
	_gEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListStatusRightTextBuf[1];
    _gEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "Running");
//    _gEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
//    sprintf(_gEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "%dA", (uint16_t)_gEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.digit_value);
    //第三行左边
    _gEmListStatusPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListStatusPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListStatusPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListStatusPara.sub_widget_para[0][2].x_size = 158;
    _gEmListStatusPara.sub_widget_para[0][2].y_size = 17;
    _gEmListStatusPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListStatusLeftTextBuf[2];
    _gEmListStatusPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListStatusPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListStatusPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "DCDC");
    //第三行右边
    _gEmListStatusPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListStatusPara.sub_widget_para[1][2].x_relative_pos = 218;
    _gEmListStatusPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gEmListStatusPara.sub_widget_para[1][2].x_size = 208;
    _gEmListStatusPara.sub_widget_para[1][2].y_size = 17;
	_gEmListStatusPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListStatusRightTextBuf[2];
    _gEmListStatusPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListStatusPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListStatusPara.sub_widget_para[1][2].sub_widget.text_para.p_content, "Charging");
//    _gEmListStatusPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 50;//测试数据，实际赋值系统数据
//    sprintf(_gEmListStatusPara.sub_widget_para[1][2].sub_widget.text_para.p_content, "%dHz", (uint16_t)_gEmListStatusPara.sub_widget_para[1][2].sub_widget.text_para.digit_value);

	_SetSysStatus(0, g_ui_ctx.system_status);
	_SetInverterStatus(0, g_ui_ctx.invs_status);
	_SetDCDCStatus(0, g_ui_ctx.dcdc_status);
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
	if(g_ui_ctx.system_status != gCollectParaData.pcs_input_front_data.system_status)
	{
		g_ui_ctx.system_status = gCollectParaData.pcs_input_front_data.system_status;
		
		_SetSysStatus(1, g_ui_ctx.system_status);
	}
	
	if(g_ui_ctx.invs_status != gCollectParaData.pcs_input_front_data.invs_status)
	{
		g_ui_ctx.invs_status = gCollectParaData.pcs_input_front_data.invs_status;
		
		_SetInverterStatus(1, g_ui_ctx.invs_status);
	}
	
	if(g_ui_ctx.dcdc_status != gCollectParaData.pcs_input_front_data.dcdc_status)
	{
		g_ui_ctx.dcdc_status = gCollectParaData.pcs_input_front_data.dcdc_status;
		
		_SetDCDCStatus(1, g_ui_ctx.dcdc_status);
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
	if (event < 0 || event > EN_KEY_SRC_NUM)
	{
		return;
	}

	if (event == EN_KEY_SRC_ESC)
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
				pnext = &gEmWinListEquipOperaInfo;
			}
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win();  
		}
		return;
	}
    
}

