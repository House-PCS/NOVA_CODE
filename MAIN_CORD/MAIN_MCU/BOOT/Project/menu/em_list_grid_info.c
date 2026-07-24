/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-11
  * @brief       电网信息
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-11    <td>V1.0.0    <td>           <td>InitialVersion
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


emWin_t  gEmWinListGridInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
    uint16_t r_phase_grid_volt;              //R 相电网电压    //6026
    uint16_t r_phase_grid_freq;              //R 相电网频率 
    uint16_t r_phase_meter_curr;             //R 相电表电流
    uint16_t r_phase_meter_power;            //R 相电表功率
    uint16_t r_phase_meter_active_power;     //R 相电表有功功率
    uint16_t r_phase_meter_reactive_power;   //R 相电表无功功率
    uint16_t s_phase_grid_volt;              //S 相电网电压
    uint16_t s_phase_grid_freq;              //S 相电网频率 
    uint16_t s_phase_meter_curr;             //S 相电表电流
    uint16_t s_phase_meter_power;            //S 相电表功率
    uint16_t s_phase_meter_active_power;     //S 相电表有功功率
    uint16_t s_phase_meter_reactive_power;   //S 相电表无功功率
    uint16_t t_phase_grid_volt;              //T 相电网电压
    uint16_t t_phase_grid_freq;              //T 相电网频率 
    uint16_t t_phase_meter_curr;             //T 相电表电流
    uint16_t t_phase_meter_power;            //T 相电表功率
    uint16_t t_phase_meter_active_power;     //T 相电表有功功率
    uint16_t t_phase_meter_reactive_power;   //T 相电表无功功率
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;

static char _gTitleBuf[] = "Operational State Info < Grid Info";

/*List控件*/
static emListPara_t  _gEmListGridInfoPara;

static char _gListGridInfoLeftTextBuf[6][20];
static char _gListGridInfoRightText2Buf[6][20];


#define WIN_LIST_GRID_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_GRID_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)


static const GUI_WIDGET_CREATE_INFO _aListGridOperaInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListGridOperaInfoWin",    WIN_LIST_GRID_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_GRID_INFO_TITLE_ID,     31,  20, 415,  14,   0,   0x0,  0 },
};

static int32_t _SetGridVol(uint16_t vol_r, uint16_t vol_s, uint16_t vol_t)
{
	float _vol[3] = {
		(float)vol_r/10,
		(float)vol_s/10,
		(float)vol_t/10
	};
    snprintf(_gListGridInfoRightText2Buf[0], sizeof(_gListGridInfoRightText2Buf[0]), 
			"%.1fV | %.1fV | %.1fV", _vol[0], _vol[1], _vol[2]);
	TEXT_SetText(_gEmListGridInfoPara.handle[1][0], 
				(const char *)_gEmListGridInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetGridCur(uint16_t cur_r, uint16_t cur_s, uint16_t cur_t)
{
	float _cur[3] = {
		(float)cur_r/10,
		(float)cur_s/10,
		(float)cur_t/10
	};
    snprintf(_gListGridInfoRightText2Buf[1], sizeof(_gListGridInfoRightText2Buf[1]), 
			"%.1fA | %.1fA | %.1fA", _cur[0], _cur[1], _cur[2]);
	TEXT_SetText(_gEmListGridInfoPara.handle[1][1], 
				(const char *)_gEmListGridInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetFrequency(uint16_t frq_r, uint16_t frq_s, uint16_t frq_t)
{
	float _frq[3] = {
		(float)frq_r/100,
		(float)frq_s/100,
		(float)frq_t/100
	};
    snprintf(_gListGridInfoRightText2Buf[2], sizeof(_gListGridInfoRightText2Buf[2]), 
			"%.1fHz | %.1fHz | %.1fHz", _frq[0], _frq[1], _frq[2]);
	TEXT_SetText(_gEmListGridInfoPara.handle[1][2], 
				(const char *)_gEmListGridInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetActivePower(uint16_t power_r, uint16_t power_s, uint16_t power_t)
{
    snprintf(_gListGridInfoRightText2Buf[3], sizeof(_gListGridInfoRightText2Buf[3]), 
			"%dW | %dW | %dW", power_r, power_s, power_t);
	TEXT_SetText(_gEmListGridInfoPara.handle[1][3], 
				(const char *)_gEmListGridInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetReactivePower(uint16_t power_r, uint16_t power_s, uint16_t power_t)
{
    snprintf(_gListGridInfoRightText2Buf[4], sizeof(_gListGridInfoRightText2Buf[4]), 
			"%dW | %dW | %dW", power_r, power_s, power_t);
	TEXT_SetText(_gEmListGridInfoPara.handle[1][4], 
				(const char *)_gEmListGridInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetApparentPower(uint16_t power_r, uint16_t power_s, uint16_t power_t)
{
    snprintf(_gListGridInfoRightText2Buf[5], sizeof(_gListGridInfoRightText2Buf[5]), 
			"%dW | %dW | %dW", power_r, power_s, power_t);
	TEXT_SetText(_gEmListGridInfoPara.handle[1][5], 
				(const char *)_gEmListGridInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
	return 0;
}

/**
  * @brief ListGridOperaInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListGridOperaInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListGridInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListGridInfoPara, _gEmListGridInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(int i = 0; i < _gEmListGridInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListGridInfoPara.sub_region[i].x_pos, 
                                    _gEmListGridInfoPara.sub_region[i].y_pos, 
                                    _gEmListGridInfoPara.sub_region[i].x_pos + _gEmListGridInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListGridInfoPara.sub_region[i].y_pos + _gEmListGridInfoPara.sub_region[i].y_size - 1,
                                    _gEmListGridInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListGridInfoPara, _gEmListGridInfoPara.list_sel);
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
    gEmWinListGridInfo.win_handle = GUI_CreateDialogBox(_aListGridOperaInfoDialogCreate, GUI_COUNTOF(_aListGridOperaInfoDialogCreate), _cbListGridOperaInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListGridInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListGridInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListGridInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListGridInfoPara, gEmWinListGridInfo.win_handle);
    return gEmWinListGridInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListGridInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListGridInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListGridInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	g_ui_ctx.r_phase_grid_volt = gCollectParaData.pcs_input_front_data.r_phase_grid_volt;
	g_ui_ctx.s_phase_grid_volt = gCollectParaData.pcs_input_front_data.s_phase_grid_volt;
	g_ui_ctx.t_phase_grid_volt = gCollectParaData.pcs_input_front_data.t_phase_grid_volt;

	EmKeyDefaultInfoSet();
    uint8_t max_page;
    
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListGridInfoPara.list_start_id = WIN_LIST_GRID_INFO_ID + 0x30;
    _gEmListGridInfoPara.list_num = 6;
    _gEmListGridInfoPara.list_display_num = 6;
    if(_gEmListGridInfoPara.list_num <= _gEmListGridInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListGridInfoPara.list_num + 1 - _gEmListGridInfoPara.list_display_num;
    }
    
    if(_gEmListGridInfoPara.list_sel > _gEmListGridInfoPara.list_num - 1)
    {
        _gEmListGridInfoPara.list_sel = 0;
    }
    if(_gEmListGridInfoPara.list_page > max_page - 1)
    {
        _gEmListGridInfoPara.list_page = 0;
    }
    _gEmListGridInfoPara.list_bk_color = 0x00787878;
	/*List选中框*/
    _gEmListGridInfoPara.selected_r = 10;
	
    /*6个子列表*/
    _gEmListGridInfoPara.sub_region[0].x_pos = 20;
    _gEmListGridInfoPara.sub_region[0].y_pos = 38;
    _gEmListGridInfoPara.sub_region[0].x_size = 440;
    _gEmListGridInfoPara.sub_region[0].y_size = 34;
    _gEmListGridInfoPara.sub_region[1].x_pos = 20;
    _gEmListGridInfoPara.sub_region[1].y_pos = 80;
    _gEmListGridInfoPara.sub_region[1].x_size = 440;
    _gEmListGridInfoPara.sub_region[1].y_size = 34;
    _gEmListGridInfoPara.sub_region[2].x_pos = 20;
    _gEmListGridInfoPara.sub_region[2].y_pos = 122;
    _gEmListGridInfoPara.sub_region[2].x_size = 440;
    _gEmListGridInfoPara.sub_region[2].y_size = 34;
    _gEmListGridInfoPara.sub_region[3].x_pos = 20;
    _gEmListGridInfoPara.sub_region[3].y_pos = 164;
    _gEmListGridInfoPara.sub_region[3].x_size = 440;
    _gEmListGridInfoPara.sub_region[3].y_size = 34;
    _gEmListGridInfoPara.sub_region[4].x_pos = 20;
    _gEmListGridInfoPara.sub_region[4].y_pos = 206;
    _gEmListGridInfoPara.sub_region[4].x_size = 440;
    _gEmListGridInfoPara.sub_region[4].y_size = 34;
    _gEmListGridInfoPara.sub_region[5].x_pos = 20;
    _gEmListGridInfoPara.sub_region[5].y_pos = 248;
    _gEmListGridInfoPara.sub_region[5].x_size = 440;
    _gEmListGridInfoPara.sub_region[5].y_size = 34;
    /*List控件*/

    //第一行左边
    _gEmListGridInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListGridInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListGridInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListGridInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListGridInfoLeftTextBuf[0];
    _gEmListGridInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListGridInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Grid Voltage");
    //第一行右边
    _gEmListGridInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[1][0].x_relative_pos = 220;
    _gEmListGridInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[1][0].x_size = 208;
    _gEmListGridInfoPara.sub_widget_para[1][0].y_size = 17;
	_gEmListGridInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListGridInfoRightText2Buf[0];
    _gEmListGridInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListGridInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 220;//测试数据，实际赋值系统数据
    snprintf(_gListGridInfoRightText2Buf[0], sizeof(_gListGridInfoRightText2Buf[0]), 
			"%.1fV | %.1fV | %.1fV", (float)(g_ui_ctx.r_phase_grid_volt/10), 
			(float)(g_ui_ctx.s_phase_grid_volt/10), (float)(g_ui_ctx.t_phase_grid_volt/10));
    //第二行左边
    _gEmListGridInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListGridInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[0][1].x_size = 158;
    _gEmListGridInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListGridInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListGridInfoLeftTextBuf[1];
    _gEmListGridInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListGridInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Grid Current");
    //第二行右边
    _gEmListGridInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[1][1].x_relative_pos = 220;
    _gEmListGridInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[1][1].x_size = 208;
    _gEmListGridInfoPara.sub_widget_para[1][1].y_size = 17;
	_gEmListGridInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListGridInfoRightText2Buf[1];
    _gEmListGridInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListGridInfoPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    snprintf(_gListGridInfoRightText2Buf[1], sizeof(_gListGridInfoRightText2Buf[1]), 
			"%.1fA | %.1fA | %.1fA", (float)(g_ui_ctx.r_phase_meter_curr/10), 
			(float)(g_ui_ctx.s_phase_meter_curr/10), (float)(g_ui_ctx.t_phase_meter_curr/10));
    //第三行左边
    _gEmListGridInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListGridInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[0][2].x_size = 158;
    _gEmListGridInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListGridInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListGridInfoLeftTextBuf[2];
    _gEmListGridInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListGridInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Frequency");
    //第三行右边
    _gEmListGridInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[1][2].x_relative_pos = 220;
    _gEmListGridInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[1][2].x_size = 208;
    _gEmListGridInfoPara.sub_widget_para[1][2].y_size = 17;
	_gEmListGridInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListGridInfoRightText2Buf[2];
    _gEmListGridInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListGridInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 50;//测试数据，实际赋值系统数据
    snprintf(_gListGridInfoRightText2Buf[2], sizeof(_gListGridInfoRightText2Buf[2]), 
			"%.1fHz | %.1fHz | %.1fHz", (float)(g_ui_ctx.r_phase_grid_freq/10), 
			(float)(g_ui_ctx.s_phase_grid_freq/10), (float)(g_ui_ctx.t_phase_grid_freq/10));
    //第四行左边
    _gEmListGridInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListGridInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[0][3].x_size = 158;
    _gEmListGridInfoPara.sub_widget_para[0][3].y_size = 17;
    _gEmListGridInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListGridInfoLeftTextBuf[3];
    _gEmListGridInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListGridInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Active Power");
    //第四行右边
    _gEmListGridInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[1][3].x_relative_pos = 220;
    _gEmListGridInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[1][3].x_size = 208;
    _gEmListGridInfoPara.sub_widget_para[1][3].y_size = 17;
	_gEmListGridInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _gListGridInfoRightText2Buf[3];
    _gEmListGridInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListGridInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    snprintf(_gListGridInfoRightText2Buf[3], sizeof(_gListGridInfoRightText2Buf[3]), 
			"%dW | %dW | %dW", g_ui_ctx.r_phase_meter_active_power, 
			g_ui_ctx.s_phase_meter_active_power, g_ui_ctx.t_phase_meter_active_power);
    //第五行左边
    _gEmListGridInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListGridInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[0][4].x_size = 158;
    _gEmListGridInfoPara.sub_widget_para[0][4].y_size = 17;
    _gEmListGridInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListGridInfoLeftTextBuf[4];
    _gEmListGridInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListGridInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Reactive Power");
    //第五行右边
    _gEmListGridInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[1][4].x_relative_pos = 220;
    _gEmListGridInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[1][4].x_size = 208;
    _gEmListGridInfoPara.sub_widget_para[1][4].y_size = 17;
	_gEmListGridInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _gListGridInfoRightText2Buf[4];
    _gEmListGridInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListGridInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    snprintf(_gListGridInfoRightText2Buf[4], sizeof(_gListGridInfoRightText2Buf[4]), 
			"%dW | %dW | %dW", g_ui_ctx.r_phase_meter_reactive_power, 
			g_ui_ctx.s_phase_meter_reactive_power, g_ui_ctx.t_phase_meter_reactive_power);
    //第六行左边
    _gEmListGridInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gEmListGridInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[0][5].x_size = 158;
    _gEmListGridInfoPara.sub_widget_para[0][5].y_size = 17;
    _gEmListGridInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListGridInfoLeftTextBuf[5];
    _gEmListGridInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListGridInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Apparent Power");
    //第六行右边
    _gEmListGridInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridInfoPara.sub_widget_para[1][5].x_relative_pos = 220;
    _gEmListGridInfoPara.sub_widget_para[1][5].y_relative_pos = 10;
    _gEmListGridInfoPara.sub_widget_para[1][5].x_size = 208;
    _gEmListGridInfoPara.sub_widget_para[1][5].y_size = 17;
	_gEmListGridInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _gListGridInfoRightText2Buf[5];
    _gEmListGridInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListGridInfoPara.sub_widget_para[1][5].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    snprintf(_gListGridInfoRightText2Buf[5], sizeof(_gListGridInfoRightText2Buf[5]), 
			"%dW | %dW | %dW", g_ui_ctx.r_phase_meter_power, 
			g_ui_ctx.s_phase_meter_power, g_ui_ctx.t_phase_meter_power);
    
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
	if (g_ui_ctx.r_phase_grid_volt != gCollectParaData.pcs_input_front_data.r_phase_grid_volt
		|| g_ui_ctx.s_phase_grid_volt != gCollectParaData.pcs_input_front_data.s_phase_grid_volt
		|| g_ui_ctx.t_phase_grid_volt != gCollectParaData.pcs_input_front_data.t_phase_grid_volt)
	{
		g_ui_ctx.r_phase_grid_volt = gCollectParaData.pcs_input_front_data.r_phase_grid_volt;
		g_ui_ctx.s_phase_grid_volt = gCollectParaData.pcs_input_front_data.s_phase_grid_volt;
		g_ui_ctx.t_phase_grid_volt = gCollectParaData.pcs_input_front_data.t_phase_grid_volt;
		_SetGridVol(g_ui_ctx.r_phase_grid_volt, g_ui_ctx.s_phase_grid_volt, 
					g_ui_ctx.t_phase_grid_volt);
	}

	if (g_ui_ctx.r_phase_meter_curr != gCollectParaData.pcs_input_front_data.r_phase_meter_curr
		|| g_ui_ctx.s_phase_meter_curr != gCollectParaData.pcs_input_front_data.s_phase_meter_curr
		|| g_ui_ctx.t_phase_meter_curr != gCollectParaData.pcs_input_front_data.t_phase_meter_curr)
	{
		g_ui_ctx.r_phase_meter_curr = gCollectParaData.pcs_input_front_data.r_phase_meter_curr;
		g_ui_ctx.s_phase_meter_curr = gCollectParaData.pcs_input_front_data.s_phase_meter_curr;
		g_ui_ctx.t_phase_meter_curr = gCollectParaData.pcs_input_front_data.t_phase_meter_curr;
		_SetGridCur(g_ui_ctx.r_phase_meter_curr, g_ui_ctx.s_phase_meter_curr, 
					g_ui_ctx.t_phase_meter_curr);
	}

	if (g_ui_ctx.r_phase_grid_freq != gCollectParaData.pcs_input_front_data.r_phase_grid_freq
		|| g_ui_ctx.s_phase_grid_freq != gCollectParaData.pcs_input_front_data.s_phase_grid_freq
		|| g_ui_ctx.t_phase_grid_freq != gCollectParaData.pcs_input_front_data.t_phase_grid_freq)
	{
		g_ui_ctx.r_phase_grid_freq = gCollectParaData.pcs_input_front_data.r_phase_grid_freq;
		g_ui_ctx.s_phase_grid_freq = gCollectParaData.pcs_input_front_data.s_phase_grid_freq;
		g_ui_ctx.t_phase_grid_freq = gCollectParaData.pcs_input_front_data.t_phase_grid_freq;
		_SetFrequency(g_ui_ctx.r_phase_grid_freq, g_ui_ctx.s_phase_grid_freq, 
					g_ui_ctx.t_phase_grid_freq);
	}

	if (g_ui_ctx.r_phase_meter_active_power != gCollectParaData.pcs_input_front_data.r_phase_meter_active_power
		|| g_ui_ctx.s_phase_meter_active_power != gCollectParaData.pcs_input_front_data.s_phase_meter_active_power
		|| g_ui_ctx.t_phase_meter_active_power != gCollectParaData.pcs_input_front_data.t_phase_meter_active_power)
	{
		g_ui_ctx.r_phase_meter_active_power = gCollectParaData.pcs_input_front_data.r_phase_meter_active_power;
		g_ui_ctx.s_phase_meter_active_power = gCollectParaData.pcs_input_front_data.s_phase_meter_active_power;
		g_ui_ctx.t_phase_meter_active_power = gCollectParaData.pcs_input_front_data.t_phase_meter_active_power;
		_SetActivePower(g_ui_ctx.r_phase_meter_active_power, g_ui_ctx.s_phase_meter_active_power, 
					g_ui_ctx.t_phase_meter_active_power);
	}

	if (g_ui_ctx.r_phase_meter_reactive_power != gCollectParaData.pcs_input_front_data.r_phase_meter_reactive_power
		|| g_ui_ctx.s_phase_meter_reactive_power != gCollectParaData.pcs_input_front_data.s_phase_meter_reactive_power
		|| g_ui_ctx.t_phase_meter_reactive_power != gCollectParaData.pcs_input_front_data.t_phase_meter_reactive_power)
	{
		g_ui_ctx.r_phase_meter_reactive_power = gCollectParaData.pcs_input_front_data.r_phase_meter_reactive_power;
		g_ui_ctx.s_phase_meter_reactive_power = gCollectParaData.pcs_input_front_data.s_phase_meter_reactive_power;
		g_ui_ctx.t_phase_meter_reactive_power = gCollectParaData.pcs_input_front_data.t_phase_meter_reactive_power;
		_SetReactivePower(g_ui_ctx.r_phase_meter_reactive_power, g_ui_ctx.s_phase_meter_reactive_power, 
					g_ui_ctx.t_phase_meter_reactive_power);
	}

	if (g_ui_ctx.r_phase_meter_power != gCollectParaData.pcs_input_front_data.r_phase_meter_power
		|| g_ui_ctx.s_phase_meter_power != gCollectParaData.pcs_input_front_data.s_phase_meter_power
		|| g_ui_ctx.t_phase_meter_power != gCollectParaData.pcs_input_front_data.t_phase_meter_power)
	{
		g_ui_ctx.r_phase_meter_power = gCollectParaData.pcs_input_front_data.r_phase_meter_power;
		g_ui_ctx.s_phase_meter_power = gCollectParaData.pcs_input_front_data.s_phase_meter_power;
		g_ui_ctx.t_phase_meter_power = gCollectParaData.pcs_input_front_data.t_phase_meter_power;
		_SetApparentPower(g_ui_ctx.r_phase_meter_power, g_ui_ctx.s_phase_meter_power, 
					g_ui_ctx.t_phase_meter_power);
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

