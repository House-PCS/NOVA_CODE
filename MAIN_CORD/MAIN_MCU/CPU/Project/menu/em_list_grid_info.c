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


emWin_t gstEmWinListGridInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    uint16_t r_phase_grid_volt;              //R 相电网电压    //6026
    uint16_t r_phase_grid_freq;              //R 相电网频率
    int16_t r_phase_meter_curr;             //R 相电表电流
    int32_t r_phase_meter_power;            //R 相电表功率
    int32_t r_phase_meter_active_power;     //R 相电表有功功率
    int32_t r_phase_meter_reactive_power;   //R 相电表无功功率
    uint16_t s_phase_grid_volt;              //S 相电网电压
    uint16_t s_phase_grid_freq;              //S 相电网频率
    int16_t s_phase_meter_curr;             //S 相电表电流
    int32_t s_phase_meter_power;            //S 相电表功率
    int32_t s_phase_meter_active_power;     //S 相电表有功功率
    int32_t s_phase_meter_reactive_power;   //S 相电表无功功率
    uint16_t t_phase_grid_volt;              //T 相电网电压
    uint16_t t_phase_grid_freq;              //T 相电网频率
    int16_t t_phase_meter_curr;             //T 相电表电流
    int32_t t_phase_meter_power;            //T 相电表功率
    int32_t t_phase_meter_active_power;     //T 相电表有功功率
    int32_t t_phase_meter_reactive_power;   //T 相电表无功功率
    
    int32_t total_meter_power;
    int32_t total_meter_active_power;
    int32_t total_meter_reactive_power;
} UiCtx_t;

static UiCtx_t gstUiCtx;


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;

static char _garrTitleBuf[] = "Operational State Info < Grid";

/*List控件*/
static emListPara_t  _gstEmListGridInfoPara;

static char _garrListGridInfoLeftTextBuf[6][32];
static char _garrListGridInfoRightText2Buf[6][32];

#define WIN_LIST_GRID_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_GRID_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)


static __SDRAM GUI_WIDGET_CREATE_INFO _garrListGridOperaInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListGridOperaInfoWin",    WIN_LIST_GRID_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_GRID_INFO_TITLE_ID,     31,  20, 415,  14,   0,   0x0,  0 },
};

static int32_t _SetGridVol(int8_t ui_update, uint16_t is_single_phase, uint16_t vol_r, uint16_t vol_s, uint16_t vol_t)
{
    int16_t volt[3] = {(int16_t)vol_r, (int16_t)vol_s, (int16_t)vol_t};

    float voltage[3] =
    {
        (float)volt[0] / 10,
        (float)volt[1] / 10,
        (float)volt[2] / 10
    };
    
    if(is_single_phase)
    {
        snprintf(_garrListGridInfoRightText2Buf[0], sizeof(_garrListGridInfoRightText2Buf[0]),
             "%.1fV", voltage[0]);
    }
    else
    {
        snprintf(_garrListGridInfoRightText2Buf[0], sizeof(_garrListGridInfoRightText2Buf[0]),
             "%.1f | %.1f | %.1f V", voltage[0], voltage[1], voltage[2]);
    }

    if (ui_update)
    {
        TEXT_SetText(_gstEmListGridInfoPara.handle[1][0],
                     (const char *)_gstEmListGridInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetGridCur(int8_t ui_update, uint16_t is_single_phase, uint16_t cur_r, uint16_t cur_s, uint16_t cur_t)
{
    int16_t cur[3] = {(int16_t)cur_r, (int16_t)cur_s, (int16_t)cur_t};
    float current[3] =
    {
        (float)cur[0] / 10,
        (float)cur[1] / 10,
        (float)cur[2] / 10
    };
    
    if(is_single_phase)
    {
        snprintf(_garrListGridInfoRightText2Buf[1], sizeof(_garrListGridInfoRightText2Buf[1]),
                 "%.1fA", current[0]);
    }
    else
    {
        snprintf(_garrListGridInfoRightText2Buf[1], sizeof(_garrListGridInfoRightText2Buf[1]),
                 "%.1f | %.1f | %.1f A", current[0], current[1], current[2]);
    }

    if (ui_update)
    {
        TEXT_SetText(_gstEmListGridInfoPara.handle[1][1],
                     (const char *)_gstEmListGridInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    }
    return 0;
}

//static int32_t _SetFrequency(int8_t ui_update, uint16_t frq_r, uint16_t frq_s, uint16_t frq_t)
static int32_t _SetFrequency(int8_t ui_update, uint16_t frq_r)
{
    float frequency = (float)frq_r / 100;

    snprintf(_garrListGridInfoRightText2Buf[2], sizeof(_garrListGridInfoRightText2Buf[2]),
             "%.2fHz", frequency);
    
    if(ui_update)
    {
        TEXT_SetText(_gstEmListGridInfoPara.handle[1][2],
                     (const char *)_gstEmListGridInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetActivePower(int8_t ui_update, uint32_t r_power, uint32_t s_power, uint32_t t_power)
{
    int32_t power[3] = {(int32_t)r_power, (int32_t)s_power, (int32_t)t_power};

    int32_t total_power = power[0] + power[1] + power[2];
    
    snprintf(_garrListGridInfoRightText2Buf[3], sizeof(_garrListGridInfoRightText2Buf[3]),
             "%dW", total_power);
    
    if (ui_update)
    {
        TEXT_SetText(_gstEmListGridInfoPara.handle[1][3],
                     (const char *)_gstEmListGridInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetReactivePower(int8_t ui_update, uint32_t r_power, uint32_t s_power, uint32_t t_power)
{
    int32_t power[3] = {(int32_t)r_power, (int32_t)s_power, (int32_t)t_power};
    int32_t reactive_power = power[0] + power[1] + power[2];

    snprintf(_garrListGridInfoRightText2Buf[4], sizeof(_garrListGridInfoRightText2Buf[4]),
             "%dVar", reactive_power);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListGridInfoPara.handle[1][4],
                     (const char *)_gstEmListGridInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetApparentPower(int8_t ui_update, uint32_t r_power, uint32_t s_power, uint32_t t_power)
{
    int32_t power[3] = {(int32_t)r_power, (int32_t)s_power, (int32_t)t_power};
    int32_t apparent_power = power[0] + power[1] + power[2];

    snprintf(_garrListGridInfoRightText2Buf[5], sizeof(_garrListGridInfoRightText2Buf[5]),
             "%dVA", apparent_power);

    if(ui_update)
    {
        TEXT_SetText(_gstEmListGridInfoPara.handle[1][5],
                     (const char *)_gstEmListGridInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
    }

    return 0;
}

/**
  * @brief ListGridOperaInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListGridOperaInfoWin(WM_MESSAGE * pMsg)
{
    uint32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListGridInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListGridInfoPara, _gstEmListGridInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListGridInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListGridInfoPara.sub_region[i].x_pos,
                                    _gstEmListGridInfoPara.sub_region[i].y_pos,
                                    _gstEmListGridInfoPara.sub_region[i].x_pos + _gstEmListGridInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListGridInfoPara.sub_region[i].y_pos + _gstEmListGridInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListGridInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListGridInfoPara, _gstEmListGridInfoPara.list_sel);
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
    gstEmWinListGridInfo.win_handle = GUI_CreateDialogBox(_garrListGridOperaInfoDialogCreate, GUI_COUNTOF(_garrListGridOperaInfoDialogCreate), _cbListGridOperaInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListGridInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListGridInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListGridInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListGridInfoPara, gstEmWinListGridInfo.win_handle);
    return gstEmWinListGridInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListGridInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListGridInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListGridInfo.win_handle = 0;
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

    gstUiCtx.r_phase_grid_volt = gstCollectInfo.power_data.power_value_info.r_grid_vol;
    gstUiCtx.s_phase_grid_volt = gstCollectInfo.power_data.power_value_info.s_grid_vol;
    gstUiCtx.t_phase_grid_volt = gstCollectInfo.power_data.power_value_info.t_grid_vol;

    gstUiCtx.r_phase_meter_curr = gstCollectInfo.power_data.power_value_info.r_meter_current;
    gstUiCtx.s_phase_meter_curr = gstCollectInfo.power_data.power_value_info.s_meter_current;
    gstUiCtx.t_phase_meter_curr = gstCollectInfo.power_data.power_value_info.t_meter_current;

    gstUiCtx.r_phase_grid_freq = gstCollectInfo.power_data.power_value_info.r_grid_freq;
    gstUiCtx.s_phase_grid_freq = gstCollectInfo.power_data.power_value_info.s_grid_freq;
    gstUiCtx.t_phase_grid_freq = gstCollectInfo.power_data.power_value_info.t_grid_freq;

    gstUiCtx.r_phase_meter_active_power = gstCollectInfo.power_data.power_value_info.r_meter_act_power;
    gstUiCtx.s_phase_meter_active_power = gstCollectInfo.power_data.power_value_info.s_meter_act_power;
    gstUiCtx.t_phase_meter_active_power = gstCollectInfo.power_data.power_value_info.t_meter_act_power;

    gstUiCtx.r_phase_meter_reactive_power = gstCollectInfo.power_data.power_value_info.r_meter_react_power;
    gstUiCtx.s_phase_meter_reactive_power = gstCollectInfo.power_data.power_value_info.s_meter_react_power;
    gstUiCtx.t_phase_meter_reactive_power = gstCollectInfo.power_data.power_value_info.t_meter_react_power;

    gstUiCtx.r_phase_meter_power = gstCollectInfo.power_data.power_value_info.r_meter_apparent_power;
    gstUiCtx.s_phase_meter_power = gstCollectInfo.power_data.power_value_info.s_meter_apparent_power;
    gstUiCtx.t_phase_meter_power = gstCollectInfo.power_data.power_value_info.t_meter_apparent_power;
                                       
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListGridInfoPara.list_start_id = WIN_LIST_GRID_INFO_ID + 0x30;
    _gstEmListGridInfoPara.list_num = 6;
    _gstEmListGridInfoPara.list_display_num = 6;
    if(_gstEmListGridInfoPara.list_num <= _gstEmListGridInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListGridInfoPara.list_num + 1 - _gstEmListGridInfoPara.list_display_num;
    }

    if(_gstEmListGridInfoPara.list_sel > _gstEmListGridInfoPara.list_num - 1)
    {
        _gstEmListGridInfoPara.list_sel = 0;
    }
    if(_gstEmListGridInfoPara.list_page > max_page - 1)
    {
        _gstEmListGridInfoPara.list_page = 0;
    }
    _gstEmListGridInfoPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _gstEmListGridInfoPara.selected_r = 10;

    /*6个子列表*/
    _gstEmListGridInfoPara.sub_region[0].x_pos = 20;
    _gstEmListGridInfoPara.sub_region[0].y_pos = 38;
    _gstEmListGridInfoPara.sub_region[0].x_size = 440;
    _gstEmListGridInfoPara.sub_region[0].y_size = 34;
    _gstEmListGridInfoPara.sub_region[1].x_pos = 20;
    _gstEmListGridInfoPara.sub_region[1].y_pos = 80;
    _gstEmListGridInfoPara.sub_region[1].x_size = 440;
    _gstEmListGridInfoPara.sub_region[1].y_size = 34;
    _gstEmListGridInfoPara.sub_region[2].x_pos = 20;
    _gstEmListGridInfoPara.sub_region[2].y_pos = 122;
    _gstEmListGridInfoPara.sub_region[2].x_size = 440;
    _gstEmListGridInfoPara.sub_region[2].y_size = 34;
    _gstEmListGridInfoPara.sub_region[3].x_pos = 20;
    _gstEmListGridInfoPara.sub_region[3].y_pos = 164;
    _gstEmListGridInfoPara.sub_region[3].x_size = 440;
    _gstEmListGridInfoPara.sub_region[3].y_size = 34;
    _gstEmListGridInfoPara.sub_region[4].x_pos = 20;
    _gstEmListGridInfoPara.sub_region[4].y_pos = 206;
    _gstEmListGridInfoPara.sub_region[4].x_size = 440;
    _gstEmListGridInfoPara.sub_region[4].y_size = 34;
    _gstEmListGridInfoPara.sub_region[5].x_pos = 20;
    _gstEmListGridInfoPara.sub_region[5].y_pos = 248;
    _gstEmListGridInfoPara.sub_region[5].x_size = 440;
    _gstEmListGridInfoPara.sub_region[5].y_size = 34;
    /*List控件*/

    //第一行左边
    _gstEmListGridInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListGridInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[0][0].x_size = 148;
    _gstEmListGridInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListGridInfoLeftTextBuf[0];
    _gstEmListGridInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Grid Voltage");
    //第一行右边
    _gstEmListGridInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[1][0].x_relative_pos = 200;
    _gstEmListGridInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[1][0].x_size = 225;
    _gstEmListGridInfoPara.sub_widget_para[1][0].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListGridInfoRightText2Buf[0];
    _gstEmListGridInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListGridInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 220;//测试数据，实际赋值系统数据
    _SetGridVol(0, SINGLE_PHASE, gstUiCtx.r_phase_grid_volt, gstUiCtx.s_phase_grid_volt, gstUiCtx.t_phase_grid_volt);

    //第二行左边
    _gstEmListGridInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListGridInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[0][1].x_size = 148;
    _gstEmListGridInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListGridInfoLeftTextBuf[1];
    _gstEmListGridInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Grid Current");
    //第二行右边
    _gstEmListGridInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[1][1].x_relative_pos = 200;
    _gstEmListGridInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[1][1].x_size = 225;
    _gstEmListGridInfoPara.sub_widget_para[1][1].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListGridInfoRightText2Buf[1];
    _gstEmListGridInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListGridInfoPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    _SetGridCur(0, SINGLE_PHASE, gstUiCtx.r_phase_meter_curr, gstUiCtx.s_phase_meter_curr, gstUiCtx.t_phase_meter_curr);
    
    //第三行左边
    _gstEmListGridInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListGridInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[0][2].x_size = 148;
    _gstEmListGridInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListGridInfoLeftTextBuf[2];
    _gstEmListGridInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Frequency");
    //第三行右边
    _gstEmListGridInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[1][2].x_relative_pos = 200;
    _gstEmListGridInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[1][2].x_size = 225;
    _gstEmListGridInfoPara.sub_widget_para[1][2].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListGridInfoRightText2Buf[2];
    _gstEmListGridInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListGridInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 50;//测试数据，实际赋值系统数据
    _SetFrequency(0, gstUiCtx.r_phase_grid_freq);
    //第四行左边
    _gstEmListGridInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListGridInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[0][3].x_size = 148;
    _gstEmListGridInfoPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListGridInfoLeftTextBuf[3];
    _gstEmListGridInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Active Power");
    //第四行右边
    _gstEmListGridInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[1][3].x_relative_pos = 200;
    _gstEmListGridInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[1][3].x_size = 225;
    _gstEmListGridInfoPara.sub_widget_para[1][3].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _garrListGridInfoRightText2Buf[3];
    _gstEmListGridInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListGridInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    _SetActivePower(0, gstUiCtx.r_phase_meter_active_power, gstUiCtx.s_phase_meter_active_power, gstUiCtx.t_phase_meter_active_power);

    //第五行左边
    _gstEmListGridInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListGridInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[0][4].x_size = 148;
    _gstEmListGridInfoPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListGridInfoLeftTextBuf[4];
    _gstEmListGridInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Reactive Power");
    //第五行右边
    _gstEmListGridInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[1][4].x_relative_pos = 200;
    _gstEmListGridInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[1][4].x_size = 225;
    _gstEmListGridInfoPara.sub_widget_para[1][4].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _garrListGridInfoRightText2Buf[4];
    _gstEmListGridInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListGridInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    _SetReactivePower(0, gstUiCtx.r_phase_meter_reactive_power, gstUiCtx.s_phase_meter_reactive_power, gstUiCtx.t_phase_meter_reactive_power);
    
    //第六行左边
    _gstEmListGridInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gstEmListGridInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[0][5].x_size = 148;
    _gstEmListGridInfoPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListGridInfoLeftTextBuf[5];
    _gstEmListGridInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Apparent Power");
    //第六行右边
    _gstEmListGridInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridInfoPara.sub_widget_para[1][5].x_relative_pos = 200;
    _gstEmListGridInfoPara.sub_widget_para[1][5].y_relative_pos = 10;
    _gstEmListGridInfoPara.sub_widget_para[1][5].x_size = 225;
    _gstEmListGridInfoPara.sub_widget_para[1][5].y_size = 17;
    _gstEmListGridInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _garrListGridInfoRightText2Buf[5];
    _gstEmListGridInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListGridInfoPara.sub_widget_para[1][5].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    _SetApparentPower(0, gstUiCtx.r_phase_meter_power, gstUiCtx.s_phase_meter_power, gstUiCtx.t_phase_meter_power);
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
#if 0
    uint32_t power[3] = 
    {
        gstCollectInfo.power_data.power_value_info.r_meter_act_power +
        gstCollectInfo.power_data.power_value_info.s_meter_act_power +
        gstCollectInfo.power_data.power_value_info.t_meter_act_power,
        
        gstCollectInfo.power_data.power_value_info.r_meter_react_power +
        gstCollectInfo.power_data.power_value_info.s_meter_react_power +
        gstCollectInfo.power_data.power_value_info.t_meter_react_power,
        
        gstCollectInfo.power_data.power_value_info.r_meter_apparent_power +
        gstCollectInfo.power_data.power_value_info.s_meter_apparent_power +
        gstCollectInfo.power_data.power_value_info.t_meter_apparent_power,
    };
#endif
    if (gstUiCtx.r_phase_grid_volt != gstCollectInfo.power_data.power_value_info.r_grid_vol
     || gstUiCtx.s_phase_grid_volt != gstCollectInfo.power_data.power_value_info.s_grid_vol
     || gstUiCtx.t_phase_grid_volt != gstCollectInfo.power_data.power_value_info.t_grid_vol)
    {
        gstUiCtx.r_phase_grid_volt = gstCollectInfo.power_data.power_value_info.r_grid_vol;
        gstUiCtx.s_phase_grid_volt = gstCollectInfo.power_data.power_value_info.s_grid_vol;
        gstUiCtx.t_phase_grid_volt = gstCollectInfo.power_data.power_value_info.t_grid_vol;
        _SetGridVol(1, SINGLE_PHASE, gstUiCtx.r_phase_grid_volt, gstUiCtx.s_phase_grid_volt, gstUiCtx.t_phase_grid_volt);
    }
    

    if (gstUiCtx.r_phase_meter_curr != gstCollectInfo.power_data.power_value_info.r_meter_current
     || gstUiCtx.s_phase_meter_curr != gstCollectInfo.power_data.power_value_info.s_meter_current
     || gstUiCtx.t_phase_meter_curr != gstCollectInfo.power_data.power_value_info.t_meter_current)
    {
        gstUiCtx.r_phase_meter_curr = gstCollectInfo.power_data.power_value_info.r_meter_current;
        gstUiCtx.s_phase_meter_curr = gstCollectInfo.power_data.power_value_info.s_meter_current;
        gstUiCtx.t_phase_meter_curr = gstCollectInfo.power_data.power_value_info.t_meter_current;
        _SetGridCur(1, SINGLE_PHASE, gstUiCtx.r_phase_meter_curr, gstUiCtx.s_phase_meter_curr,
                    gstUiCtx.t_phase_meter_curr);
    }
    
    if (gstUiCtx.r_phase_grid_freq != gstCollectInfo.power_data.power_value_info.r_grid_freq)
    {
        gstUiCtx.r_phase_grid_freq = gstCollectInfo.power_data.power_value_info.r_grid_freq;
        _SetFrequency(1, gstUiCtx.r_phase_grid_freq);
    }

    if (gstUiCtx.r_phase_meter_active_power != gstCollectInfo.power_data.power_value_info.r_meter_act_power
     || gstUiCtx.s_phase_meter_active_power != gstCollectInfo.power_data.power_value_info.s_meter_act_power
     || gstUiCtx.t_phase_meter_active_power != gstCollectInfo.power_data.power_value_info.t_meter_act_power)
    {
        gstUiCtx.r_phase_meter_active_power = gstCollectInfo.power_data.power_value_info.r_meter_act_power;
        gstUiCtx.s_phase_meter_active_power = gstCollectInfo.power_data.power_value_info.s_meter_act_power;
        gstUiCtx.t_phase_meter_active_power = gstCollectInfo.power_data.power_value_info.t_meter_act_power;
        _SetActivePower(1, gstUiCtx.r_phase_meter_active_power, gstUiCtx.s_phase_meter_active_power,
                        gstUiCtx.t_phase_meter_active_power);
    }
    
    if (gstUiCtx.r_phase_meter_reactive_power != gstCollectInfo.power_data.power_value_info.r_meter_react_power
     || gstUiCtx.s_phase_meter_reactive_power != gstCollectInfo.power_data.power_value_info.s_meter_react_power
     || gstUiCtx.t_phase_meter_reactive_power != gstCollectInfo.power_data.power_value_info.t_meter_react_power)
    {
        gstUiCtx.r_phase_meter_reactive_power = gstCollectInfo.power_data.power_value_info.r_meter_react_power;
        gstUiCtx.s_phase_meter_reactive_power = gstCollectInfo.power_data.power_value_info.s_meter_react_power;
        gstUiCtx.t_phase_meter_reactive_power = gstCollectInfo.power_data.power_value_info.t_meter_react_power;
        _SetReactivePower(1, gstUiCtx.r_phase_meter_reactive_power, gstUiCtx.s_phase_meter_reactive_power,
                   gstUiCtx.t_phase_meter_reactive_power);
    }
    
    if (gstUiCtx.r_phase_meter_power != gstCollectInfo.power_data.power_value_info.r_meter_apparent_power
     || gstUiCtx.s_phase_meter_power != gstCollectInfo.power_data.power_value_info.s_meter_apparent_power
     || gstUiCtx.t_phase_meter_power != gstCollectInfo.power_data.power_value_info.t_meter_apparent_power)
    {
        gstUiCtx.r_phase_meter_power = gstCollectInfo.power_data.power_value_info.r_meter_apparent_power;
        gstUiCtx.s_phase_meter_power = gstCollectInfo.power_data.power_value_info.s_meter_apparent_power;
        gstUiCtx.t_phase_meter_power = gstCollectInfo.power_data.power_value_info.t_meter_apparent_power;
        _SetApparentPower(1, gstUiCtx.r_phase_meter_power, gstUiCtx.s_phase_meter_power,
                              gstUiCtx.t_phase_meter_power);
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

