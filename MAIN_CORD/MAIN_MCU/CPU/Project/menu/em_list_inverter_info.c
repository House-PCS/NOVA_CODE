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


emWin_t gstEmWinListInverterInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t     select;
    int32_t     key_init;
    uint16_t    r_phase_inverter_volt;
    uint16_t    r_phase_inverter_curr;
    uint16_t    r_phase_inverter_freq;
    uint16_t    s_phase_inverter_volt;
    uint16_t    s_phase_inverter_curr;
    uint16_t    s_phase_inverter_freq;
    uint16_t    t_phase_inverter_volt;
    uint16_t    t_phase_inverter_curr;
    uint16_t    t_phase_inverter_freq;
    uint16_t    r_phase_inverter_apparent_power;
    uint16_t    r_phase_inverter_active_power;
    uint16_t    r_phase_inverter_reactive_power;
    uint16_t    s_phase_inverter_apparent_power;
    uint16_t    s_phase_inverter_active_power;
    uint16_t    s_phase_inverter_reactive_power;
    uint16_t    t_phase_inverter_apparent_power;
    uint16_t    t_phase_inverter_active_power;
    uint16_t    t_phase_inverter_reactive_power;
} UiCtx_t;

static UiCtx_t _gstUiCtx;


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;

static char _garrTitleBuf[] = "Operational State Info < Inverter";

/*List控件*/
static emListPara_t  _garrEmListInverterInfoPara;

static char _garrListInverterInfoLeftTextBuf[6][32];
static char _garrListInverterInfoRightTextBuf[6][32];


#define WIN_LIST_INVERTER_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_INVERTER_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)

static const GUI_WIDGET_CREATE_INFO _garrListInverterInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListInverterInfoWin",    WIN_LIST_INVERTER_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_INVERTER_INFO_TITLE_ID,     31,  20, 415,  14,   0,   0x0,  0 },
};

static uint32_t _SetInverterVoltage(int8_t ui_update, uint16_t r_vol, uint16_t s_vol, uint16_t t_vol)
{
    float voltage[3] =
    {
        (float)r_vol / 10,
        (float)s_vol / 10,
        (float)t_vol / 10
    };
#if ROCA_5KW
    snprintf(_garrListInverterInfoRightTextBuf[0], sizeof(_garrListInverterInfoRightTextBuf[0]), "%.1fV", voltage[0]);
#else
    snprintf(_garrListInverterInfoRightTextBuf[0], sizeof(_garrListInverterInfoRightTextBuf[0]), "%.1f | %.1f | %.1f V", voltage[0], voltage[1], voltage[2]);
#endif
    if (ui_update)
    {
        TEXT_SetText(_garrEmListInverterInfoPara.handle[1][0], (const char *)_garrEmListInverterInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }

    return 0;
}

static uint32_t _SetInverterCurrent(int8_t ui_update, uint16_t r_cur, uint16_t s_cur, uint16_t t_cur)
{
    int16_t cur_value[3] = { (int16_t)r_cur, (int16_t)s_cur, (int16_t)t_cur };
    float current[3] =
    {
        (float)cur_value[0] / 10,
        (float)cur_value[1] / 10,
        (float)cur_value[2] / 10
    };

#if ROCA_5KW
    snprintf(_garrListInverterInfoRightTextBuf[1], sizeof(_garrListInverterInfoRightTextBuf[1]), "%.1fA", current[0]);
#else
    snprintf(_garrListInverterInfoRightTextBuf[1], sizeof(_garrListInverterInfoRightTextBuf[1]), "%.1f | %.1f | %.1f A", current[0], current[1], current[2]);
#endif

    if (ui_update)
    {
        TEXT_SetText(_garrEmListInverterInfoPara.handle[1][1], (const char *)_garrEmListInverterInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    }
    return 0;
}

static uint32_t _SetInverterFrequency(int8_t ui_update, uint16_t r_freq, uint16_t s_freq, uint16_t t_freq)
{
    float frequency[3] =
    {
        (float)r_freq / 100,
        (float)s_freq / 100,
        (float)t_freq / 100
    };

#if ROCA_5KW
    snprintf(_garrListInverterInfoRightTextBuf[2], sizeof(_garrListInverterInfoRightTextBuf[2]), "%.2fHz", frequency[0]);
#else
    snprintf(_garrListInverterInfoRightTextBuf[2], sizeof(_garrListInverterInfoRightTextBuf[2]), "%.2f | %.2f | %.2f Hz", frequency[0], frequency[1], frequency[2]);
#endif

    if (ui_update)
    {
        TEXT_SetText(_garrEmListInverterInfoPara.handle[1][2], (const char *)_garrEmListInverterInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }

    return 0;
}

static uint32_t _SetInverterActivePower(int8_t ui_update, uint16_t r_active_power, uint16_t s_active_power, uint16_t t_active_power)
{
    int16_t active_power[3] = 
    {
        (int16_t)r_active_power,
        (int16_t)s_active_power,
        (int16_t)t_active_power
    };
    snprintf(_garrListInverterInfoRightTextBuf[3], sizeof(_garrListInverterInfoRightTextBuf[3]), "%d | %d | %d W", active_power[0], active_power[1], active_power[2]);
    if (ui_update)
    {
        TEXT_SetText(_garrEmListInverterInfoPara.handle[1][3], (const char *)_garrEmListInverterInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }

    return 0;
}

static uint32_t _SetInverterReactivePower(int8_t ui_update, uint16_t r_reactive_power, uint16_t s_reactive_power, uint16_t t_reactive_power)
{
    int16_t reactive_power[3] = 
    {
        (int16_t)r_reactive_power,
        (int16_t)s_reactive_power,
        (int16_t)t_reactive_power
    };
    snprintf(_garrListInverterInfoRightTextBuf[4], sizeof(_garrListInverterInfoRightTextBuf[4]), "%d | %d | %d Var", reactive_power[0], reactive_power[1], reactive_power[2]);
    if (ui_update)
    {
        TEXT_SetText(_garrEmListInverterInfoPara.handle[1][4], (const char *)_garrEmListInverterInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }

    return 0;
}

static uint32_t _SetInverterApparentPower(int8_t ui_update, uint16_t r_apparent_power, uint16_t s_apparent_power, uint16_t t_apparent_power)
{
    int16_t apparent_power[3] = 
    {
        (int16_t)r_apparent_power,
        (int16_t)s_apparent_power,
        (int16_t)t_apparent_power
    };
    snprintf(_garrListInverterInfoRightTextBuf[5], sizeof(_garrListInverterInfoRightTextBuf[5]), "%d | %d | %d VA", apparent_power[0], apparent_power[1], apparent_power[2]);

    if (ui_update)
    {
        TEXT_SetText(_garrEmListInverterInfoPara.handle[1][5], (const char *)_garrEmListInverterInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
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
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_INVERTER_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_garrEmListInverterInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_garrEmListInverterInfoPara, _garrEmListInverterInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _garrEmListInverterInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_garrEmListInverterInfoPara.sub_region[i].x_pos,
                                    _garrEmListInverterInfoPara.sub_region[i].y_pos,
                                    _garrEmListInverterInfoPara.sub_region[i].x_pos + _garrEmListInverterInfoPara.sub_region[i].x_size - 1,
                                    _garrEmListInverterInfoPara.sub_region[i].y_pos + _garrEmListInverterInfoPara.sub_region[i].y_size - 1,
                                    _garrEmListInverterInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_garrEmListInverterInfoPara, _garrEmListInverterInfoPara.list_sel);
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
    pPrevEmWin = &gstEmWinListEquipOperaInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListInverterInfo.win_handle = GUI_CreateDialogBox(_garrListInverterInfoDialogCreate, GUI_COUNTOF(_garrListInverterInfoDialogCreate), _cbListInverterInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListInverterInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListInverterInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListInverterInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_garrEmListInverterInfoPara, gstEmWinListInverterInfo.win_handle);
    return gstEmWinListInverterInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListInverterInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListInverterInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListInverterInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;
    EmKeyDefaultInfoSet();

    _gstUiCtx.r_phase_inverter_volt = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_volt;
    _gstUiCtx.s_phase_inverter_volt = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_volt;
    _gstUiCtx.t_phase_inverter_volt = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_volt;
    _gstUiCtx.r_phase_inverter_curr = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_curr;
    _gstUiCtx.s_phase_inverter_curr = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_curr;
    _gstUiCtx.t_phase_inverter_curr = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_curr;
    _gstUiCtx.r_phase_inverter_freq = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_freq;
    _gstUiCtx.s_phase_inverter_freq = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_freq;
    _gstUiCtx.t_phase_inverter_freq = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_freq;

    _gstUiCtx.r_phase_inverter_active_power = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_active_power;
    _gstUiCtx.s_phase_inverter_active_power = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_active_power;
    _gstUiCtx.t_phase_inverter_active_power = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_active_power;

    _gstUiCtx.r_phase_inverter_reactive_power = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_reactive_power;
    _gstUiCtx.s_phase_inverter_reactive_power = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_reactive_power;
    _gstUiCtx.t_phase_inverter_reactive_power = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_reactive_power;

    _gstUiCtx.r_phase_inverter_apparent_power = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_power;
    _gstUiCtx.s_phase_inverter_apparent_power = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_power;
    _gstUiCtx.t_phase_inverter_apparent_power = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_power;


    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00EEEEEE;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _garrEmListInverterInfoPara.list_start_id = WIN_LIST_INVERTER_INFO_ID + 0x30;
    _garrEmListInverterInfoPara.list_num = 6;
    _garrEmListInverterInfoPara.list_display_num = 6;
    if(_garrEmListInverterInfoPara.list_num <= _garrEmListInverterInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _garrEmListInverterInfoPara.list_num + 1 - _garrEmListInverterInfoPara.list_display_num;
    }

    if(_garrEmListInverterInfoPara.list_sel > _garrEmListInverterInfoPara.list_num - 1)
    {
        _garrEmListInverterInfoPara.list_sel = 0;
    }
    if(_garrEmListInverterInfoPara.list_page > max_page - 1)
    {
        _garrEmListInverterInfoPara.list_page = 0;
    }
    _garrEmListInverterInfoPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _garrEmListInverterInfoPara.selected_r = 10;

    /*4个子列表*/
    _garrEmListInverterInfoPara.sub_region[0].x_pos = 20;
    _garrEmListInverterInfoPara.sub_region[0].y_pos = 38;
    _garrEmListInverterInfoPara.sub_region[0].x_size = 440;
    _garrEmListInverterInfoPara.sub_region[0].y_size = 34;
    _garrEmListInverterInfoPara.sub_region[1].x_pos = 20;
    _garrEmListInverterInfoPara.sub_region[1].y_pos = 80;
    _garrEmListInverterInfoPara.sub_region[1].x_size = 440;
    _garrEmListInverterInfoPara.sub_region[1].y_size = 34;
    _garrEmListInverterInfoPara.sub_region[2].x_pos = 20;
    _garrEmListInverterInfoPara.sub_region[2].y_pos = 122;
    _garrEmListInverterInfoPara.sub_region[2].x_size = 440;
    _garrEmListInverterInfoPara.sub_region[2].y_size = 34;
    _garrEmListInverterInfoPara.sub_region[3].x_pos = 20;
    _garrEmListInverterInfoPara.sub_region[3].y_pos = 164;
    _garrEmListInverterInfoPara.sub_region[3].x_size = 440;
    _garrEmListInverterInfoPara.sub_region[3].y_size = 34;
    _garrEmListInverterInfoPara.sub_region[4].x_pos = 20;
    _garrEmListInverterInfoPara.sub_region[4].y_pos = 206;
    _garrEmListInverterInfoPara.sub_region[4].x_size = 440;
    _garrEmListInverterInfoPara.sub_region[4].y_size = 34;
    _garrEmListInverterInfoPara.sub_region[5].x_pos = 20;
    _garrEmListInverterInfoPara.sub_region[5].y_pos = 248;
    _garrEmListInverterInfoPara.sub_region[5].x_size = 440;
    _garrEmListInverterInfoPara.sub_region[5].y_size = 34;

    /*List控件*/

    //第一行左边
    _garrEmListInverterInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _garrEmListInverterInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[0][0].x_size = 158;
    _garrEmListInverterInfoPara.sub_widget_para[0][0].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListInverterInfoLeftTextBuf[0];
    _garrEmListInverterInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_garrEmListInverterInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Inverter Voltage");
    //第一行右边
    _garrEmListInverterInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[1][0].x_relative_pos = 218;
    _garrEmListInverterInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[1][0].x_size = 208;
    _garrEmListInverterInfoPara.sub_widget_para[1][0].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListInverterInfoRightTextBuf[0];
    _garrEmListInverterInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _garrEmListInverterInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 220;//测试数据，实际赋值系统数据
    _SetInverterVoltage(0, _gstUiCtx.r_phase_inverter_volt,
                        _gstUiCtx.s_phase_inverter_volt,
                        _gstUiCtx.t_phase_inverter_volt);
    //第二行左边
    _garrEmListInverterInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _garrEmListInverterInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[0][1].x_size = 158;
    _garrEmListInverterInfoPara.sub_widget_para[0][1].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListInverterInfoLeftTextBuf[1];
    _garrEmListInverterInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_garrEmListInverterInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Inverter Current");
    //第二行右边
    _garrEmListInverterInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[1][1].x_relative_pos = 218;
    _garrEmListInverterInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[1][1].x_size = 208;
    _garrEmListInverterInfoPara.sub_widget_para[1][1].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListInverterInfoRightTextBuf[1];
    _garrEmListInverterInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _garrEmListInverterInfoPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    _SetInverterCurrent(0, _gstUiCtx.r_phase_inverter_curr,
                        _gstUiCtx.s_phase_inverter_curr,
                        _gstUiCtx.t_phase_inverter_curr);
    //第三行左边
    _garrEmListInverterInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _garrEmListInverterInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[0][2].x_size = 158;
    _garrEmListInverterInfoPara.sub_widget_para[0][2].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListInverterInfoLeftTextBuf[2];
    _garrEmListInverterInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_garrEmListInverterInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Frequency");
    //第三行右边
    _garrEmListInverterInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[1][2].x_relative_pos = 218;
    _garrEmListInverterInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[1][2].x_size = 208;
    _garrEmListInverterInfoPara.sub_widget_para[1][2].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListInverterInfoRightTextBuf[2];
    _garrEmListInverterInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _garrEmListInverterInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 60;//测试数据，实际赋值系统数据
    _SetInverterFrequency(0, _gstUiCtx.r_phase_inverter_freq,
                          _gstUiCtx.s_phase_inverter_freq,
                          _gstUiCtx.t_phase_inverter_freq);
    //第四行左边
    _garrEmListInverterInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _garrEmListInverterInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[0][3].x_size = 158;
    _garrEmListInverterInfoPara.sub_widget_para[0][3].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListInverterInfoLeftTextBuf[3];
    _garrEmListInverterInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_garrEmListInverterInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Active Power");
    //第四行右边
    _garrEmListInverterInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[1][3].x_relative_pos = 218;
    _garrEmListInverterInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[1][3].x_size = 208;
    _garrEmListInverterInfoPara.sub_widget_para[1][3].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _garrListInverterInfoRightTextBuf[3];
    _garrEmListInverterInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _garrEmListInverterInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    _SetInverterActivePower(0, _gstUiCtx.r_phase_inverter_active_power, _gstUiCtx.s_phase_inverter_active_power, _gstUiCtx.t_phase_inverter_active_power);
    //第五行左边
    _garrEmListInverterInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _garrEmListInverterInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[0][4].x_size = 158;
    _garrEmListInverterInfoPara.sub_widget_para[0][4].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListInverterInfoLeftTextBuf[4];
    _garrEmListInverterInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_garrEmListInverterInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Reactive Power");
    //第五行右边
    _garrEmListInverterInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[1][4].x_relative_pos = 218;
    _garrEmListInverterInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[1][4].x_size = 208;
    _garrEmListInverterInfoPara.sub_widget_para[1][4].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _garrListInverterInfoRightTextBuf[4];
    _garrEmListInverterInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _garrEmListInverterInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    _SetInverterReactivePower(0, _gstUiCtx.r_phase_inverter_reactive_power, _gstUiCtx.s_phase_inverter_reactive_power, _gstUiCtx.t_phase_inverter_reactive_power);
    //第六行左边
    _garrEmListInverterInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[0][5].x_relative_pos = 11;
    _garrEmListInverterInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[0][5].x_size = 158;
    _garrEmListInverterInfoPara.sub_widget_para[0][5].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListInverterInfoLeftTextBuf[5];
    _garrEmListInverterInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_garrEmListInverterInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Apparent Power");
    //第六行右边
    _garrEmListInverterInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _garrEmListInverterInfoPara.sub_widget_para[1][5].x_relative_pos = 218;
    _garrEmListInverterInfoPara.sub_widget_para[1][5].y_relative_pos = 10;
    _garrEmListInverterInfoPara.sub_widget_para[1][5].x_size = 208;
    _garrEmListInverterInfoPara.sub_widget_para[1][5].y_size = 17;
    _garrEmListInverterInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _garrListInverterInfoRightTextBuf[5];
    _garrEmListInverterInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _garrEmListInverterInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _garrEmListInverterInfoPara.sub_widget_para[1][5].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    _SetInverterApparentPower(0, _gstUiCtx.r_phase_inverter_apparent_power, _gstUiCtx.s_phase_inverter_apparent_power, _gstUiCtx.t_phase_inverter_apparent_power);
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
    uint16_t power[3] =
    {
        gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_active_power
        + gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_active_power
        + gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_active_power,

        gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_reactive_power
        + gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_reactive_power
        + gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_reactive_power,

        gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_power
        + gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_power
        + gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_power
    };

    if( _gstUiCtx.r_phase_inverter_volt != gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_volt || \
            _gstUiCtx.s_phase_inverter_volt != gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_volt || \
            _gstUiCtx.t_phase_inverter_volt != gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_volt   )
    {
        _gstUiCtx.r_phase_inverter_volt = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_volt;
        _gstUiCtx.s_phase_inverter_volt = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_volt;
        _gstUiCtx.t_phase_inverter_volt = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_volt;

        _SetInverterVoltage(1, _gstUiCtx.r_phase_inverter_volt, _gstUiCtx.s_phase_inverter_volt, _gstUiCtx.t_phase_inverter_volt);
    }

    if( _gstUiCtx.r_phase_inverter_curr != gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_curr || \
        _gstUiCtx.s_phase_inverter_curr != gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_curr || \
        _gstUiCtx.t_phase_inverter_curr != gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_curr   )
    {
        _gstUiCtx.r_phase_inverter_curr = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_curr;
        _gstUiCtx.s_phase_inverter_curr = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_curr;
        _gstUiCtx.t_phase_inverter_curr = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_curr;

        _SetInverterCurrent(1, _gstUiCtx.r_phase_inverter_curr, _gstUiCtx.s_phase_inverter_curr, _gstUiCtx.t_phase_inverter_curr);
    }

    if( _gstUiCtx.r_phase_inverter_freq != gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_freq || \
            _gstUiCtx.s_phase_inverter_freq != gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_freq || \
            _gstUiCtx.t_phase_inverter_freq != gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_freq   )
    {
        _gstUiCtx.r_phase_inverter_freq = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_freq;
        _gstUiCtx.s_phase_inverter_freq = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_freq;
        _gstUiCtx.t_phase_inverter_freq = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_freq;

        _SetInverterFrequency(1, _gstUiCtx.r_phase_inverter_freq, _gstUiCtx.s_phase_inverter_freq, _gstUiCtx.t_phase_inverter_freq);
    }

    if( _gstUiCtx.r_phase_inverter_active_power != gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_active_power || \
        _gstUiCtx.s_phase_inverter_active_power != gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_active_power || \
        _gstUiCtx.t_phase_inverter_active_power != gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_active_power )
    {
        _gstUiCtx.r_phase_inverter_active_power = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_active_power;
        _gstUiCtx.s_phase_inverter_active_power = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_active_power;
        _gstUiCtx.t_phase_inverter_active_power = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_active_power;

        _SetInverterActivePower(1, _gstUiCtx.r_phase_inverter_active_power, _gstUiCtx.s_phase_inverter_active_power, _gstUiCtx.t_phase_inverter_active_power);
    }

    if( _gstUiCtx.r_phase_inverter_reactive_power != gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_reactive_power || \
        _gstUiCtx.s_phase_inverter_reactive_power != gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_reactive_power || \
        _gstUiCtx.t_phase_inverter_reactive_power != gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_reactive_power )
    {
        _gstUiCtx.r_phase_inverter_reactive_power = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_reactive_power;
        _gstUiCtx.s_phase_inverter_reactive_power = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_reactive_power;
        _gstUiCtx.t_phase_inverter_reactive_power = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_reactive_power;

        _SetInverterReactivePower(1, _gstUiCtx.r_phase_inverter_reactive_power, _gstUiCtx.s_phase_inverter_reactive_power, _gstUiCtx.t_phase_inverter_reactive_power);
    }

    if( _gstUiCtx.r_phase_inverter_apparent_power != gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_power || \
        _gstUiCtx.s_phase_inverter_apparent_power != gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_power || \
        _gstUiCtx.t_phase_inverter_apparent_power != gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_power)
    {
        _gstUiCtx.r_phase_inverter_apparent_power = gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_power;
        _gstUiCtx.s_phase_inverter_apparent_power = gstCollectParaData.pcs_data.udp_pcs_input_para.s_phase_inverter_power;
        _gstUiCtx.t_phase_inverter_apparent_power = gstCollectParaData.pcs_data.udp_pcs_input_para.t_phase_inverter_power;

        _SetInverterApparentPower(1, _gstUiCtx.r_phase_inverter_apparent_power, _gstUiCtx.s_phase_inverter_apparent_power, _gstUiCtx.t_phase_inverter_apparent_power);
    }

}

/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
    int32_t event = 0;
    event = EmGetKeyEvent(p_msg);
    if (event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }

    if (event == EN_KEY_SRC_ESC)
    {
        if(pCurrentEmWin->win_handle != 0)
        {
            emWin_t *pnext = NULL;
            if(p_msg->press_down_time >= gstEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
            {
                pnext = &gstEmWinHomePage;
            }
            else
            {
                pnext = &gstEmWinListEquipOperaInfo;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}

