/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-11
  * @brief       bac信息
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


emWin_t  gEmWinListBackUpLoadInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
	
	uint16_t r_phase_backup_volt;         
	uint16_t r_phase_backup_curr;          
	uint16_t s_phase_backup_volt;          
	uint16_t s_phase_backup_curr;         
	uint16_t t_phase_backup_volt;         
	uint16_t t_phase_backup_curr;          
	uint16_t backup_frequency;
	uint16_t backup_active_power;
	uint16_t backup_reactive_power;
	uint16_t backup_apparent_power;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;

static char _gTitleBuf[] = "Operational State Info < Back-up Load Output";

/*List控件*/
static emListPara_t  _gEmListBackUpLoadInfoPara;

static char _gListBackUpLoadInfoLeftTextBuf[6][20];
static char _gListBackUpLoadInfoRightTextBuf[6][20];


#define WIN_LIST_BACK_UP_LOAD_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_BACK_UP_LOAD_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)

static const GUI_WIDGET_CREATE_INFO _aListBackUpLoadInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListBackUpLoadInfoWin",    WIN_LIST_BACK_UP_LOAD_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_BACK_UP_LOAD_INFO_TITLE_ID,     31,  20, 415,  14,   0,   0x0,  0 },
};

/**
  * @brief   设置负载电压
  * @param   r_volt r相负载电压  s_volt s相负载电压  t_volt t相负载电压
  * @return  None
  * @note    None
  */
static int32_t _SetBackupVoltage(uint16_t r_volt, uint16_t s_volt, uint16_t t_volt)
{
	float _vol[3] = {
		(float)r_volt/10,
		(float)s_volt/10,
		(float)t_volt/10
	};
	snprintf(_gListBackUpLoadInfoLeftTextBuf[0], sizeof(_gListBackUpLoadInfoLeftTextBuf[0]),
			"%.1fV | %.1fV | %.1fV", _vol[0], _vol[1], _vol[2]);
	
	TEXT_SetText(_gEmListBackUpLoadInfoPara.handle[1][0], (const char *)_gEmListBackUpLoadInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
	return 0;
}
/**
  * @brief   设置负载电流
  * @param   r_curr r相负载电流  s_curr s相负载电流  t_curr t相负载电流
  * @return  None
  * @note    None
  */
static int32_t _SetBackupCurrent(uint16_t r_cur, uint16_t s_cur, uint16_t t_cur)
{
	float _cur[3] = {
		(float)r_cur/10,
		(float)s_cur/10,
		(float)t_cur/10,
	};
	snprintf(_gListBackUpLoadInfoLeftTextBuf[1], sizeof(_gListBackUpLoadInfoLeftTextBuf[1]),
			"%.1f | %.1f | %.1f", _cur[0], _cur[1], _cur[2]);
	
	TEXT_SetText(_gEmListBackUpLoadInfoPara.handle[1][1], (const char *)_gEmListBackUpLoadInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
	return 0;
}
/**
  * @brief   设置负载频率
  * @param   freq 负载频率真实值
  * @return  None
  * @note    None
  */
static int32_t _SetBackupFrequecy(uint16_t freq)
{
	snprintf(_gListBackUpLoadInfoLeftTextBuf[2], sizeof(_gListBackUpLoadInfoLeftTextBuf[2]),"%d", freq);
	
	TEXT_SetText(_gEmListBackUpLoadInfoPara.handle[1][2], (const char *)_gEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
	return 0;
}
/**
  * @brief   设置负载有功功率
  * @param   active_power 有功功率的真实值
  * @return  None
  * @note    None
  */
static int32_t _SetBackupActivePower(uint16_t active_power)
{
	snprintf(_gListBackUpLoadInfoLeftTextBuf[3], sizeof(_gListBackUpLoadInfoLeftTextBuf[3]),
			"%d",active_power);
	
	TEXT_SetText(_gEmListBackUpLoadInfoPara.handle[1][3], (const char *)_gEmListBackUpLoadInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
	return 0;
}
/**
  * @brief   设置负载无功功率
  * @param   reactive_power 无功功率的真实值
  * @return  None
  * @note    None
  */
static int32_t _SetBackupReactivePower(uint16_t reactive_power)
{
	snprintf(_gListBackUpLoadInfoLeftTextBuf[4], sizeof(_gListBackUpLoadInfoLeftTextBuf[4]),
			"%d",reactive_power);
	
	TEXT_SetText(_gEmListBackUpLoadInfoPara.handle[1][4], (const char *)_gEmListBackUpLoadInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
	return 0;
}
/**
  * @brief   设置负载视在功率
  * @param   apparent_power 视在功率的真实值
  * @return  None
  * @note    None
  */
static int32_t _SetBackupApparentPower(uint16_t apparent_power)
{
	snprintf(_gListBackUpLoadInfoLeftTextBuf[5], sizeof(_gListBackUpLoadInfoLeftTextBuf[5]),
			"%d",apparent_power);
	
	TEXT_SetText(_gEmListBackUpLoadInfoPara.handle[1][5], (const char *)_gEmListBackUpLoadInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
	return 0;
}


/**
  * @brief ListBackUpLoadInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListBackUpLoadInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BACK_UP_LOAD_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListBackUpLoadInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListBackUpLoadInfoPara, _gEmListBackUpLoadInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(int i = 0; i < _gEmListBackUpLoadInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListBackUpLoadInfoPara.sub_region[i].x_pos, 
                                    _gEmListBackUpLoadInfoPara.sub_region[i].y_pos, 
                                    _gEmListBackUpLoadInfoPara.sub_region[i].x_pos + _gEmListBackUpLoadInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListBackUpLoadInfoPara.sub_region[i].y_pos + _gEmListBackUpLoadInfoPara.sub_region[i].y_size - 1,
                                    _gEmListBackUpLoadInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListBackUpLoadInfoPara, _gEmListBackUpLoadInfoPara.list_sel);
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
    gEmWinListBackUpLoadInfo.win_handle = GUI_CreateDialogBox(_aListBackUpLoadInfoDialogCreate, GUI_COUNTOF(_aListBackUpLoadInfoDialogCreate), _cbListBackUpLoadInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListBackUpLoadInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListBackUpLoadInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListBackUpLoadInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListBackUpLoadInfoPara, gEmWinListBackUpLoadInfo.win_handle);
    return gEmWinListBackUpLoadInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListBackUpLoadInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListBackUpLoadInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListBackUpLoadInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	g_ui_ctx.r_phase_backup_volt = gCollectParaData.pcs_input_front_data.r_phase_backup_volt;
	g_ui_ctx.s_phase_backup_volt = gCollectParaData.pcs_input_front_data.s_phase_backup_volt;
	g_ui_ctx.t_phase_backup_volt = gCollectParaData.pcs_input_front_data.t_phase_backup_volt;
	g_ui_ctx.r_phase_backup_curr = gCollectParaData.pcs_input_front_data.r_phase_backup_curr;
	g_ui_ctx.s_phase_backup_curr = gCollectParaData.pcs_input_front_data.s_phase_backup_curr;
	g_ui_ctx.t_phase_backup_curr = gCollectParaData.pcs_input_front_data.t_phase_backup_curr;
	g_ui_ctx.backup_active_power = gCollectParaData.pcs_input_front_data.backup_active_power;
	g_ui_ctx.backup_reactive_power = gCollectParaData.pcs_input_front_data.backup_reactive_power;
	g_ui_ctx.backup_apparent_power = gCollectParaData.pcs_input_front_data.backup_apparent_power;
	
	EmKeyDefaultInfoSet();
    uint8_t max_page;
    
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00EEEEEE;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListBackUpLoadInfoPara.list_start_id = WIN_LIST_BACK_UP_LOAD_INFO_ID + 0x30;
    _gEmListBackUpLoadInfoPara.list_num = 6;
    _gEmListBackUpLoadInfoPara.list_display_num = 6;
    if(_gEmListBackUpLoadInfoPara.list_num <= _gEmListBackUpLoadInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListBackUpLoadInfoPara.list_num + 1 - _gEmListBackUpLoadInfoPara.list_display_num;
    }
    
    if(_gEmListBackUpLoadInfoPara.list_sel > _gEmListBackUpLoadInfoPara.list_num - 1)
    {
        _gEmListBackUpLoadInfoPara.list_sel = 0;
    }
    if(_gEmListBackUpLoadInfoPara.list_page > max_page - 1)
    {
        _gEmListBackUpLoadInfoPara.list_page = 0;
    }
    _gEmListBackUpLoadInfoPara.list_bk_color = 0x00787878;
	/*List选中框*/
    _gEmListBackUpLoadInfoPara.selected_r = 10;
	
    /*4个子列表*/
    _gEmListBackUpLoadInfoPara.sub_region[0].x_pos = 20;
    _gEmListBackUpLoadInfoPara.sub_region[0].y_pos = 38;
    _gEmListBackUpLoadInfoPara.sub_region[0].x_size = 440;
    _gEmListBackUpLoadInfoPara.sub_region[0].y_size = 34;
    _gEmListBackUpLoadInfoPara.sub_region[1].x_pos = 20;
    _gEmListBackUpLoadInfoPara.sub_region[1].y_pos = 80;
    _gEmListBackUpLoadInfoPara.sub_region[1].x_size = 440;
    _gEmListBackUpLoadInfoPara.sub_region[1].y_size = 34;
    _gEmListBackUpLoadInfoPara.sub_region[2].x_pos = 20;
    _gEmListBackUpLoadInfoPara.sub_region[2].y_pos = 122;
    _gEmListBackUpLoadInfoPara.sub_region[2].x_size = 440;
    _gEmListBackUpLoadInfoPara.sub_region[2].y_size = 34;
    _gEmListBackUpLoadInfoPara.sub_region[3].x_pos = 20;
    _gEmListBackUpLoadInfoPara.sub_region[3].y_pos = 164;
    _gEmListBackUpLoadInfoPara.sub_region[3].x_size = 440;
    _gEmListBackUpLoadInfoPara.sub_region[3].y_size = 34;
    _gEmListBackUpLoadInfoPara.sub_region[4].x_pos = 20;
    _gEmListBackUpLoadInfoPara.sub_region[4].y_pos = 206;
    _gEmListBackUpLoadInfoPara.sub_region[4].x_size = 440;
    _gEmListBackUpLoadInfoPara.sub_region[4].y_size = 34;
    _gEmListBackUpLoadInfoPara.sub_region[5].x_pos = 20;
    _gEmListBackUpLoadInfoPara.sub_region[5].y_pos = 248;
    _gEmListBackUpLoadInfoPara.sub_region[5].x_size = 440;
    _gEmListBackUpLoadInfoPara.sub_region[5].y_size = 34;

    /*List控件*/

    //第一行左边
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListBackUpLoadInfoLeftTextBuf[0];
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBackUpLoadInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Back Up Voltage");
    //第一行右边
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][0].x_relative_pos = 218;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][0].x_size = 208;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][0].y_size = 17;
	_gEmListBackUpLoadInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListBackUpLoadInfoRightTextBuf[0];
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 220;//测试数据，实际赋值系统数据
	snprintf(_gListBackUpLoadInfoLeftTextBuf[0], sizeof(_gListBackUpLoadInfoLeftTextBuf[0]),
			"%.1fV | %.1fV | %.1fV", ((float)g_ui_ctx.r_phase_backup_volt)/10, 
			((float)g_ui_ctx.s_phase_backup_volt)/10, ((float)g_ui_ctx.t_phase_backup_volt)/10);
    //第二行左边
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][1].x_size = 158;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListBackUpLoadInfoLeftTextBuf[1];
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBackUpLoadInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Back Up Current");
    //第二行右边
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][1].x_relative_pos = 218;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][1].x_size = 208;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][1].y_size = 17;
	_gEmListBackUpLoadInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListBackUpLoadInfoRightTextBuf[1];
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
	snprintf(_gListBackUpLoadInfoLeftTextBuf[1], sizeof(_gListBackUpLoadInfoLeftTextBuf[1]),
			"%.1f | %.1f | %.1f", ((float)g_ui_ctx.r_phase_backup_curr)/10, 
			((float)g_ui_ctx.s_phase_backup_curr)/10, ((float)g_ui_ctx.t_phase_backup_curr)/10);
    //第三行左边
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][2].x_size = 158;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListBackUpLoadInfoLeftTextBuf[2];
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBackUpLoadInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Frequency");
    //第三行右边
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][2].x_relative_pos = 218;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][2].x_size = 208;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][2].y_size = 17;
	_gEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListBackUpLoadInfoRightTextBuf[2];
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 50;//测试数据，实际赋值系统数据
    sprintf(_gEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content, "%dHz", (uint16_t)_gEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value);
    //第四行左边
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][3].x_size = 158;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][3].y_size = 17;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListBackUpLoadInfoLeftTextBuf[3];
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBackUpLoadInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Active Power");
    //第四行右边
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][3].x_relative_pos = 218;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][3].x_size = 208;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][3].y_size = 17;
	_gEmListBackUpLoadInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _gListBackUpLoadInfoRightTextBuf[3];
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
	snprintf(_gListBackUpLoadInfoLeftTextBuf[3], sizeof(_gListBackUpLoadInfoLeftTextBuf[3]),
			"%d", g_ui_ctx.backup_active_power);
        //第五行左边
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][4].x_size = 158;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][4].y_size = 17;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListBackUpLoadInfoLeftTextBuf[4];
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	snprintf(_gListBackUpLoadInfoLeftTextBuf[4], sizeof(_gListBackUpLoadInfoLeftTextBuf[4]),
			"%d", g_ui_ctx.backup_reactive_power);
    //第五行右边
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][4].x_relative_pos = 218;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][4].x_size = 208;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][4].y_size = 17;
	_gEmListBackUpLoadInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _gListBackUpLoadInfoRightTextBuf[4];
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
	snprintf(_gListBackUpLoadInfoLeftTextBuf[5], sizeof(_gListBackUpLoadInfoLeftTextBuf[5]),
			"%d", g_ui_ctx.backup_apparent_power);
    //第六行左边
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][5].x_size = 158;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][5].y_size = 17;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListBackUpLoadInfoLeftTextBuf[5];
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBackUpLoadInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Apparent Power");
    //第六行右边
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][5].x_relative_pos = 218;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][5].y_relative_pos = 10;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][5].x_size = 208;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][5].y_size = 17;
	_gEmListBackUpLoadInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _gListBackUpLoadInfoRightTextBuf[5];
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBackUpLoadInfoPara.sub_widget_para[1][5].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
	snprintf(_gListBackUpLoadInfoLeftTextBuf[5], sizeof(_gListBackUpLoadInfoLeftTextBuf[5]),
			"%d", g_ui_ctx.backup_apparent_power);
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
	if(g_ui_ctx.r_phase_backup_volt != gCollectParaData.pcs_input_front_data.r_phase_backup_volt || 
	   g_ui_ctx.s_phase_backup_volt != gCollectParaData.pcs_input_front_data.s_phase_backup_volt || 
	   g_ui_ctx.t_phase_backup_volt != gCollectParaData.pcs_input_front_data.t_phase_backup_volt)
	{
		g_ui_ctx.r_phase_backup_volt = gCollectParaData.pcs_input_front_data.r_phase_backup_volt;
	    g_ui_ctx.s_phase_backup_volt = gCollectParaData.pcs_input_front_data.s_phase_backup_volt;
		g_ui_ctx.t_phase_backup_volt = gCollectParaData.pcs_input_front_data.t_phase_backup_volt;
		_SetBackupVoltage(g_ui_ctx.r_phase_backup_volt, g_ui_ctx.s_phase_backup_volt, g_ui_ctx.t_phase_backup_volt);
	}
	
	if(g_ui_ctx.r_phase_backup_curr != gCollectParaData.pcs_input_front_data.r_phase_backup_curr || 
	   g_ui_ctx.s_phase_backup_curr != gCollectParaData.pcs_input_front_data.s_phase_backup_curr || 
	   g_ui_ctx.t_phase_backup_curr != gCollectParaData.pcs_input_front_data.t_phase_backup_curr)
	{
		g_ui_ctx.r_phase_backup_curr = gCollectParaData.pcs_input_front_data.r_phase_backup_curr;
	    g_ui_ctx.s_phase_backup_curr = gCollectParaData.pcs_input_front_data.s_phase_backup_curr;
		g_ui_ctx.t_phase_backup_curr = gCollectParaData.pcs_input_front_data.t_phase_backup_curr;
		_SetBackupCurrent(g_ui_ctx.r_phase_backup_curr, g_ui_ctx.s_phase_backup_curr, g_ui_ctx.t_phase_backup_curr);
	}
	/*缺少频率*/
	
	if(g_ui_ctx.backup_active_power != gCollectParaData.pcs_input_front_data.backup_active_power)
	{
		g_ui_ctx.backup_active_power = gCollectParaData.pcs_input_front_data.backup_active_power;
		_SetBackupActivePower(g_ui_ctx.backup_active_power);
	}
	
	if(g_ui_ctx.backup_reactive_power != gCollectParaData.pcs_input_front_data.backup_reactive_power)
	{
		g_ui_ctx.backup_reactive_power = gCollectParaData.pcs_input_front_data.backup_reactive_power;
		_SetBackupReactivePower(g_ui_ctx.backup_reactive_power);
	}
	
	if(g_ui_ctx.backup_apparent_power != gCollectParaData.pcs_input_front_data.backup_apparent_power)
	{
		g_ui_ctx.backup_apparent_power = gCollectParaData.pcs_input_front_data.backup_apparent_power;
		_SetBackupApparentPower(g_ui_ctx.backup_apparent_power);
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

