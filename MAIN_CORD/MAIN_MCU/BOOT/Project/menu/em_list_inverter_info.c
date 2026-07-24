/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-11
  * @brief       逆变信息
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
  
  
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListInverterInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t		select;
	int32_t 	key_init;
	
	uint16_t   	r_phase_inverter_volt;                   
	uint16_t   	r_phase_inverter_curr; 
	uint16_t   	r_phase_inverter_freq; 	
	uint16_t   	s_phase_inverter_volt;                    
	uint16_t   	s_phase_inverter_curr; 
	uint16_t   	s_phase_inverter_freq;	
	uint16_t   	t_phase_inverter_volt;                  
	uint16_t   	t_phase_inverter_curr; 
	uint16_t   	t_phase_inverter_freq;  	
	uint16_t   	inverter_apparent_power;        
	uint16_t   	inverter_active_power;          
	uint16_t   	inverter_reactive_power;        
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;

static char _gTitleBuf[] = "Operational State Info < Inverter";

/*List控件*/
static emListPara_t  _gEmListInverterInfoPara;

static char _gListInverterInfoLeftTextBuf[6][20];
static char _gListInverterInfoRightTextBuf[6][20];


#define WIN_LIST_INVERTER_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_INVERTER_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)

static const GUI_WIDGET_CREATE_INFO _aListInverterInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListInverterInfoWin",    WIN_LIST_INVERTER_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_INVERTER_INFO_TITLE_ID,     31,  20, 415,  14,   0,   0x0,  0 },
};

static uint32_t _SetInverterVoltage(int32_t ui_update, uint16_t r_vol, uint16_t s_vol, uint16_t t_vol)
{
	snprintf(_gListInverterInfoRightTextBuf[0], sizeof(_gListInverterInfoRightTextBuf[0]), "%d | %d | %d",r_vol,s_vol, t_vol);
	if (ui_update)
	{
		TEXT_SetText(_gEmListInverterInfoPara.handle[1][0],(const char *)_gEmListInverterInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
	}
	
	return 0;
}

static uint32_t _SetInverterCurrent(int32_t ui_update, uint16_t r_curr, uint16_t s_curr, uint16_t t_curr)
{
	snprintf(_gListInverterInfoRightTextBuf[1], sizeof(_gListInverterInfoRightTextBuf[1]), "%d | %d | %d", r_curr, s_curr, t_curr);
	if (ui_update)
	{
		TEXT_SetText(_gEmListInverterInfoPara.handle[1][1],(const char *)_gEmListInverterInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
	}
	return 0;
}

static uint32_t _SetInverterFrequency(int32_t ui_update, uint16_t r_freq, uint16_t s_freq, uint16_t t_freq)
{
	snprintf(_gListInverterInfoRightTextBuf[2], sizeof(_gListInverterInfoRightTextBuf[2]), "%d | %d | %d",r_freq, s_freq, t_freq);
	if (ui_update)
	{
		TEXT_SetText(_gEmListInverterInfoPara.handle[1][2],(const char *)_gEmListInverterInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
	}
	
	return 0;
}

static uint32_t _SetInverterActivePower(int32_t ui_update, uint16_t active_power)
{
	snprintf(_gListInverterInfoRightTextBuf[3], sizeof(_gListInverterInfoRightTextBuf[3]), "%d",active_power);
	if (ui_update)
	{
		TEXT_SetText(_gEmListInverterInfoPara.handle[1][3],(const char *)_gEmListInverterInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
	}

	return 0;
}

static uint32_t _SetInverterReactivePower(int32_t ui_update, uint16_t reactive_power)
{
	snprintf(_gListInverterInfoRightTextBuf[4], sizeof(_gListInverterInfoRightTextBuf[4]), "%d",reactive_power);
	if (ui_update)
	{
		TEXT_SetText(_gEmListInverterInfoPara.handle[1][4],(const char *)_gEmListInverterInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
	}
	
	return 0;
}

static uint32_t _SetInverterApparentPower(int32_t ui_update, uint16_t apparent_power)
{
	snprintf(_gListInverterInfoRightTextBuf[5], sizeof(_gListInverterInfoRightTextBuf[5]), "%d",apparent_power);
	if (ui_update)
	{
		TEXT_SetText(_gEmListInverterInfoPara.handle[1][5],(const char *)_gEmListInverterInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
	}
	
	return 0;
}

/**
  * @brief ListInverterInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListInverterInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_INVERTER_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListInverterInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListInverterInfoPara, _gEmListInverterInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(int i = 0; i < _gEmListInverterInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListInverterInfoPara.sub_region[i].x_pos, 
                                    _gEmListInverterInfoPara.sub_region[i].y_pos, 
                                    _gEmListInverterInfoPara.sub_region[i].x_pos + _gEmListInverterInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListInverterInfoPara.sub_region[i].y_pos + _gEmListInverterInfoPara.sub_region[i].y_size - 1,
                                    _gEmListInverterInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListInverterInfoPara, _gEmListInverterInfoPara.list_sel);
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
    gEmWinListInverterInfo.win_handle = GUI_CreateDialogBox(_aListInverterInfoDialogCreate, GUI_COUNTOF(_aListInverterInfoDialogCreate), _cbListInverterInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListInverterInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListInverterInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListInverterInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListInverterInfoPara, gEmWinListInverterInfo.win_handle);
    return gEmWinListInverterInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListInverterInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListInverterInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListInverterInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	g_ui_ctx.r_phase_inverter_volt = gCollectParaData.pcs_input_front_data.r_phase_inverter_volt;
	g_ui_ctx.s_phase_inverter_volt = gCollectParaData.pcs_input_front_data.s_phase_inverter_volt;
	g_ui_ctx.t_phase_inverter_volt = gCollectParaData.pcs_input_front_data.t_phase_inverter_volt;
	g_ui_ctx.r_phase_inverter_curr = gCollectParaData.pcs_input_front_data.r_phase_inverter_curr;
	g_ui_ctx.s_phase_inverter_curr = gCollectParaData.pcs_input_front_data.s_phase_inverter_curr;
	g_ui_ctx.t_phase_inverter_curr = gCollectParaData.pcs_input_front_data.t_phase_inverter_curr;
	g_ui_ctx.r_phase_inverter_freq = gCollectParaData.pcs_input_front_data.r_phase_inverter_freq;
	g_ui_ctx.s_phase_inverter_freq = gCollectParaData.pcs_input_front_data.s_phase_inverter_freq;
	g_ui_ctx.t_phase_inverter_freq = gCollectParaData.pcs_input_front_data.t_phase_inverter_freq;
	g_ui_ctx.inverter_active_power = gCollectParaData.pcs_input_front_data.inverter_active_power;
	g_ui_ctx.inverter_reactive_power = gCollectParaData.pcs_input_front_data.inverter_reactive_power;
	g_ui_ctx.inverter_apparent_power = gCollectParaData.pcs_input_front_data.inverter_apparent_power;

	EmKeyDefaultInfoSet();
    uint8_t max_page;
    
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00EEEEEE;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListInverterInfoPara.list_start_id = WIN_LIST_INVERTER_INFO_ID + 0x30;
    _gEmListInverterInfoPara.list_num = 6;
    _gEmListInverterInfoPara.list_display_num = 6;
    if(_gEmListInverterInfoPara.list_num <= _gEmListInverterInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListInverterInfoPara.list_num + 1 - _gEmListInverterInfoPara.list_display_num;
    }
    
    if(_gEmListInverterInfoPara.list_sel > _gEmListInverterInfoPara.list_num - 1)
    {
        _gEmListInverterInfoPara.list_sel = 0;
    }
    if(_gEmListInverterInfoPara.list_page > max_page - 1)
    {
        _gEmListInverterInfoPara.list_page = 0;
    }
    _gEmListInverterInfoPara.list_bk_color = 0x00787878;
	/*List选中框*/
    _gEmListInverterInfoPara.selected_r = 10;
	
    /*4个子列表*/
    _gEmListInverterInfoPara.sub_region[0].x_pos = 20;
    _gEmListInverterInfoPara.sub_region[0].y_pos = 38;
    _gEmListInverterInfoPara.sub_region[0].x_size = 440;
    _gEmListInverterInfoPara.sub_region[0].y_size = 34;
    _gEmListInverterInfoPara.sub_region[1].x_pos = 20;
    _gEmListInverterInfoPara.sub_region[1].y_pos = 80;
    _gEmListInverterInfoPara.sub_region[1].x_size = 440;
    _gEmListInverterInfoPara.sub_region[1].y_size = 34;
    _gEmListInverterInfoPara.sub_region[2].x_pos = 20;
    _gEmListInverterInfoPara.sub_region[2].y_pos = 122;
    _gEmListInverterInfoPara.sub_region[2].x_size = 440;
    _gEmListInverterInfoPara.sub_region[2].y_size = 34;
    _gEmListInverterInfoPara.sub_region[3].x_pos = 20;
    _gEmListInverterInfoPara.sub_region[3].y_pos = 164;
    _gEmListInverterInfoPara.sub_region[3].x_size = 440;
    _gEmListInverterInfoPara.sub_region[3].y_size = 34;
    _gEmListInverterInfoPara.sub_region[4].x_pos = 20;
    _gEmListInverterInfoPara.sub_region[4].y_pos = 206;
    _gEmListInverterInfoPara.sub_region[4].x_size = 440;
    _gEmListInverterInfoPara.sub_region[4].y_size = 34;
    _gEmListInverterInfoPara.sub_region[5].x_pos = 20;
    _gEmListInverterInfoPara.sub_region[5].y_pos = 248;
    _gEmListInverterInfoPara.sub_region[5].x_size = 440;
    _gEmListInverterInfoPara.sub_region[5].y_size = 34;

    /*List控件*/

    //第一行左边
    _gEmListInverterInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListInverterInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListInverterInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListInverterInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListInverterInfoLeftTextBuf[0];
    _gEmListInverterInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListInverterInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Inverter Voltage");
    //第一行右边
    _gEmListInverterInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[1][0].x_relative_pos = 218;
    _gEmListInverterInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[1][0].x_size = 208;
    _gEmListInverterInfoPara.sub_widget_para[1][0].y_size = 17;
	_gEmListInverterInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListInverterInfoRightTextBuf[0];
    _gEmListInverterInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListInverterInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 220;//测试数据，实际赋值系统数据
    //第二行左边
    _gEmListInverterInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListInverterInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[0][1].x_size = 158;
    _gEmListInverterInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListInverterInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListInverterInfoLeftTextBuf[1];
    _gEmListInverterInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListInverterInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Inverter Current");
    //第二行右边
    _gEmListInverterInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[1][1].x_relative_pos = 218;
    _gEmListInverterInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[1][1].x_size = 208;
    _gEmListInverterInfoPara.sub_widget_para[1][1].y_size = 17;
	_gEmListInverterInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListInverterInfoRightTextBuf[1];
    _gEmListInverterInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListInverterInfoPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    //第三行左边
    _gEmListInverterInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListInverterInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[0][2].x_size = 158;
    _gEmListInverterInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListInverterInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListInverterInfoLeftTextBuf[2];
    _gEmListInverterInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListInverterInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Frequency");
    //第三行右边
    _gEmListInverterInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[1][2].x_relative_pos = 218;
    _gEmListInverterInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[1][2].x_size = 208;
    _gEmListInverterInfoPara.sub_widget_para[1][2].y_size = 17;
	_gEmListInverterInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListInverterInfoRightTextBuf[2];
    _gEmListInverterInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListInverterInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 60;//测试数据，实际赋值系统数据
    //第四行左边
    _gEmListInverterInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListInverterInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[0][3].x_size = 158;
    _gEmListInverterInfoPara.sub_widget_para[0][3].y_size = 17;
    _gEmListInverterInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListInverterInfoLeftTextBuf[3];
    _gEmListInverterInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListInverterInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Active Power");
    //第四行右边
    _gEmListInverterInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[1][3].x_relative_pos = 218;
    _gEmListInverterInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[1][3].x_size = 208;
    _gEmListInverterInfoPara.sub_widget_para[1][3].y_size = 17;
	_gEmListInverterInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _gListInverterInfoRightTextBuf[3];
    _gEmListInverterInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListInverterInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
        //第五行左边
    _gEmListInverterInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListInverterInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[0][4].x_size = 158;
    _gEmListInverterInfoPara.sub_widget_para[0][4].y_size = 17;
    _gEmListInverterInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListInverterInfoLeftTextBuf[4];
    _gEmListInverterInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListInverterInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Reactive Power");
    //第五行右边
    _gEmListInverterInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[1][4].x_relative_pos = 218;
    _gEmListInverterInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[1][4].x_size = 208;
    _gEmListInverterInfoPara.sub_widget_para[1][4].y_size = 17;
	_gEmListInverterInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _gListInverterInfoRightTextBuf[4];
    _gEmListInverterInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListInverterInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    //第六行左边
    _gEmListInverterInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gEmListInverterInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[0][5].x_size = 158;
    _gEmListInverterInfoPara.sub_widget_para[0][5].y_size = 17;
    _gEmListInverterInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListInverterInfoLeftTextBuf[5];
    _gEmListInverterInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListInverterInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Apparent Power");
    //第六行右边
    _gEmListInverterInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListInverterInfoPara.sub_widget_para[1][5].x_relative_pos = 218;
    _gEmListInverterInfoPara.sub_widget_para[1][5].y_relative_pos = 10;
    _gEmListInverterInfoPara.sub_widget_para[1][5].x_size = 208;
    _gEmListInverterInfoPara.sub_widget_para[1][5].y_size = 17;
	_gEmListInverterInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _gListInverterInfoRightTextBuf[5];
    _gEmListInverterInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListInverterInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListInverterInfoPara.sub_widget_para[1][5].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据

	_SetInverterVoltage(0, g_ui_ctx.r_phase_inverter_volt, g_ui_ctx.s_phase_inverter_volt, g_ui_ctx.t_phase_inverter_volt);
	_SetInverterCurrent(0, g_ui_ctx.r_phase_inverter_curr, g_ui_ctx.s_phase_inverter_curr, g_ui_ctx.t_phase_inverter_curr);
	_SetInverterFrequency(0, g_ui_ctx.r_phase_inverter_freq, g_ui_ctx.s_phase_inverter_freq, g_ui_ctx.t_phase_inverter_freq);
	_SetInverterActivePower(0, g_ui_ctx.inverter_active_power);
	_SetInverterReactivePower(0, g_ui_ctx.inverter_reactive_power);
	_SetInverterApparentPower(0, g_ui_ctx.inverter_apparent_power);
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
	if( g_ui_ctx.r_phase_inverter_volt != gCollectParaData.pcs_input_front_data.r_phase_inverter_volt ||\
	    g_ui_ctx.s_phase_inverter_volt != gCollectParaData.pcs_input_front_data.s_phase_inverter_volt ||\
	    g_ui_ctx.t_phase_inverter_volt != gCollectParaData.pcs_input_front_data.t_phase_inverter_volt   )
	{
		g_ui_ctx.r_phase_inverter_volt = gCollectParaData.pcs_input_front_data.r_phase_inverter_volt;
		g_ui_ctx.s_phase_inverter_volt = gCollectParaData.pcs_input_front_data.s_phase_inverter_volt;
		g_ui_ctx.t_phase_inverter_volt = gCollectParaData.pcs_input_front_data.t_phase_inverter_volt;
		
		_SetInverterVoltage(1, g_ui_ctx.r_phase_inverter_volt, g_ui_ctx.s_phase_inverter_volt, g_ui_ctx.t_phase_inverter_volt);
	}
	
	if( g_ui_ctx.r_phase_inverter_curr != gCollectParaData.pcs_input_front_data.r_phase_inverter_curr ||\
	    g_ui_ctx.s_phase_inverter_curr != gCollectParaData.pcs_input_front_data.s_phase_inverter_curr ||\
	    g_ui_ctx.t_phase_inverter_curr != gCollectParaData.pcs_input_front_data.t_phase_inverter_curr   )
	{
		g_ui_ctx.r_phase_inverter_curr = gCollectParaData.pcs_input_front_data.r_phase_inverter_curr;
		g_ui_ctx.s_phase_inverter_curr = gCollectParaData.pcs_input_front_data.s_phase_inverter_curr;
		g_ui_ctx.t_phase_inverter_curr = gCollectParaData.pcs_input_front_data.t_phase_inverter_curr;
		
		_SetInverterCurrent(1, g_ui_ctx.r_phase_inverter_curr, g_ui_ctx.s_phase_inverter_curr, g_ui_ctx.t_phase_inverter_curr);
	}
	
	if( g_ui_ctx.r_phase_inverter_freq != gCollectParaData.pcs_input_front_data.r_phase_inverter_freq ||\
	    g_ui_ctx.s_phase_inverter_freq != gCollectParaData.pcs_input_front_data.s_phase_inverter_freq ||\
	    g_ui_ctx.t_phase_inverter_freq != gCollectParaData.pcs_input_front_data.t_phase_inverter_freq   )
	{
		g_ui_ctx.r_phase_inverter_freq = gCollectParaData.pcs_input_front_data.r_phase_inverter_freq;
		g_ui_ctx.s_phase_inverter_freq = gCollectParaData.pcs_input_front_data.s_phase_inverter_freq;
		g_ui_ctx.t_phase_inverter_freq = gCollectParaData.pcs_input_front_data.t_phase_inverter_freq;
		
		_SetInverterFrequency(1, g_ui_ctx.r_phase_inverter_freq, g_ui_ctx.s_phase_inverter_freq, g_ui_ctx.t_phase_inverter_freq);
	}
	
	if( g_ui_ctx.inverter_active_power != gCollectParaData.pcs_input_front_data.inverter_active_power)
	{
		 g_ui_ctx.inverter_active_power = gCollectParaData.pcs_input_front_data.inverter_active_power;
		
		_SetInverterActivePower(1, g_ui_ctx.inverter_active_power);
	}
	
	if( g_ui_ctx.inverter_reactive_power != gCollectParaData.pcs_input_front_data.inverter_reactive_power)
	{
		 g_ui_ctx.inverter_reactive_power = gCollectParaData.pcs_input_front_data.inverter_reactive_power;
		
		_SetInverterReactivePower(1, g_ui_ctx.inverter_reactive_power);
	}
	
	if( g_ui_ctx.inverter_apparent_power != gCollectParaData.pcs_input_front_data.inverter_apparent_power)
	{
		 g_ui_ctx.inverter_apparent_power = gCollectParaData.pcs_input_front_data.inverter_apparent_power;
		
		_SetInverterApparentPower(1, g_ui_ctx.inverter_apparent_power);
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

