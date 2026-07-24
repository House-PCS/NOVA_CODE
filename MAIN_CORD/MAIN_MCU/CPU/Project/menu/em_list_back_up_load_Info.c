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


emWin_t gstEmWinListBackUpLoadInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t r_phase_backup_volt;
    int16_t r_phase_backup_curr;
    uint16_t s_phase_backup_volt;
    int16_t s_phase_backup_curr;
    uint16_t t_phase_backup_volt;
    int16_t t_phase_backup_curr;
    int32_t r_backup_active_power;
    int32_t r_backup_reactive_power;
    int32_t r_backup_apparent_power;
    int32_t s_backup_active_power;
    int32_t s_backup_reactive_power;
    int32_t s_backup_apparent_power;
    int32_t t_backup_active_power;
    int32_t t_backup_reactive_power;
    int32_t t_backup_apparent_power;
    
    int32_t total_backup_active_power;
    int32_t total_backup_reactive_power;
    int32_t total_backup_apparent_power;
    
} UiCtx_t;

static UiCtx_t _gstUiCtx;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;

static char _garrTitleBuf[] = "Operational State Info < Backup Load";

/*List控件*/
static emListPara_t  _gstEmListBackUpLoadInfoPara;

static char _garrListBackUpLoadInfoLeftTextBuf[6][32];
static char _garrListBackUpLoadInfoRightTextBuf[6][32];


#define WIN_LIST_BACK_UP_LOAD_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_BACK_UP_LOAD_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListBackUpLoadInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListBackUpLoadInfoWin",    WIN_LIST_BACK_UP_LOAD_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_BACK_UP_LOAD_INFO_TITLE_ID,     31,  20, 415,  14,   0,   0x0,  0 },
};

/**
  * @brief   设置单相负载电压
  * @param   r_volt r相负载电压  s_volt s相负载电压  t_volt t相负载电压
  * @return  None
  * @note    None
  */
static int32_t _SetBackupVoltage(int8_t ui_update, uint16_t is_single_phase, uint16_t r_volt, uint16_t s_volt, uint16_t t_volt)
{
    int16_t volt[3] = {(int16_t)r_volt, (int16_t)s_volt, (int16_t)t_volt};
    float voltage[3] =
    {
        (float)volt[0] / 10,
        (float)volt[1] / 10,
        (float)volt[2] / 10
    };
    if (is_single_phase)
    {
        snprintf(_garrListBackUpLoadInfoRightTextBuf[0], sizeof(_garrListBackUpLoadInfoRightTextBuf[0]),
                 "%.1fV", voltage[0]);
    }
    else
    {
        snprintf(_garrListBackUpLoadInfoRightTextBuf[0], sizeof(_garrListBackUpLoadInfoRightTextBuf[0]),
                 "%.1f | %.1f | %.1f V", voltage[0], voltage[1], voltage[2]);
    }
    if (ui_update)
    {
        TEXT_SetText(_gstEmListBackUpLoadInfoPara.handle[1][0], (const char *)_gstEmListBackUpLoadInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief   设置负载电流
  * @param   r_curr r相负载电流  s_curr s相负载电流  t_curr t相负载电流
  * @return  None
  * @note    None
  */
static int32_t _SetBackupCurrent(int8_t ui_update, uint16_t is_single_phase, uint16_t r_cur, uint16_t s_cur, uint16_t t_cur)
{
    int16_t curr[3] = {(int16_t)r_cur, (int16_t)s_cur, (int16_t)t_cur};
    float current[3] =
    {
        (float)curr[0] / 10,
        (float)curr[1] / 10,
        (float)curr[2] / 10,
    };
    if (is_single_phase)
    {
        snprintf(_garrListBackUpLoadInfoRightTextBuf[1], sizeof(_garrListBackUpLoadInfoRightTextBuf[1]),
                 "%.1fA", current[0]);
    }
    else
    {
        snprintf(_garrListBackUpLoadInfoRightTextBuf[1], sizeof(_garrListBackUpLoadInfoRightTextBuf[1]),
                 "%.1f | %.1f | %.1f A", current[0], current[1], current[2]);
    }
    
    if (ui_update)
    {
        TEXT_SetText(_gstEmListBackUpLoadInfoPara.handle[1][1], (const char *)_gstEmListBackUpLoadInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief   设置负载有功功率
  * @param   active_power 有功功率的真实值
  * @return  None
  * @note    None
  */
static int32_t _SetBackupActivePower(int8_t ui_update, uint32_t r_power, uint32_t s_power, uint32_t t_power)
{
    int32_t power[3] = {(int32_t)r_power, (int32_t)s_power, (int32_t)t_power};
    int32_t total_power = power[0] + power[1] + power[2];

    snprintf(_garrListBackUpLoadInfoRightTextBuf[2], sizeof(_garrListBackUpLoadInfoRightTextBuf[2]),
             "%dW", total_power);
    
    if (ui_update)
    {
        TEXT_SetText(_gstEmListBackUpLoadInfoPara.handle[1][2], (const char *)_gstEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief   设置负载无功功率
  * @param   reactive_power 无功功率的真实值
  * @return  None
  * @note    None
  */
static int32_t _SetBackupReactivePower(int8_t ui_update, uint32_t r_power, uint32_t s_power, uint32_t t_power)
{
    int32_t power[3] = {(int32_t)r_power, (int32_t)s_power, (int32_t)t_power};
    int32_t total_power = power[0] + power[1] + power[2];
    
    snprintf(_garrListBackUpLoadInfoRightTextBuf[3], sizeof(_garrListBackUpLoadInfoRightTextBuf[3]),
             "%dVar", total_power);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListBackUpLoadInfoPara.handle[1][3], (const char *)_gstEmListBackUpLoadInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief   设置负载视在功率
  * @param   apparent_power 视在功率的真实值
  * @return  None
  * @note    None
  */
static int32_t _SetBackupApparentPower(int8_t ui_update, uint32_t r_power, uint32_t s_power, uint32_t t_power)
{
    int32_t power[3] = {(int32_t)r_power, (int32_t)s_power, (int32_t)t_power};
    int32_t total_power = power[0] + power[1] + power[2];

    snprintf(_garrListBackUpLoadInfoRightTextBuf[4], sizeof(_garrListBackUpLoadInfoRightTextBuf[4]),
             "%dVA", total_power);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListBackUpLoadInfoPara.handle[1][4], (const char *)_gstEmListBackUpLoadInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }
    return 0;
}


/**
  * @brief ListBackUpLoadInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListBackUpLoadInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BACK_UP_LOAD_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListBackUpLoadInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListBackUpLoadInfoPara, _gstEmListBackUpLoadInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListBackUpLoadInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListBackUpLoadInfoPara.sub_region[i].x_pos,
                                    _gstEmListBackUpLoadInfoPara.sub_region[i].y_pos,
                                    _gstEmListBackUpLoadInfoPara.sub_region[i].x_pos + _gstEmListBackUpLoadInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListBackUpLoadInfoPara.sub_region[i].y_pos + _gstEmListBackUpLoadInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListBackUpLoadInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListBackUpLoadInfoPara, _gstEmListBackUpLoadInfoPara.list_sel);
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
    gstEmWinListBackUpLoadInfo.win_handle = GUI_CreateDialogBox(_garrListBackUpLoadInfoDialogCreate, GUI_COUNTOF(_garrListBackUpLoadInfoDialogCreate), _cbListBackUpLoadInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListBackUpLoadInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListBackUpLoadInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListBackUpLoadInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListBackUpLoadInfoPara, gstEmWinListBackUpLoadInfo.win_handle);
    return gstEmWinListBackUpLoadInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListBackUpLoadInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListBackUpLoadInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListBackUpLoadInfo.win_handle = 0;
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

    _gstUiCtx.r_phase_backup_volt = gstCollectInfo.power_data.power_value_info.r_backup_vol;
    _gstUiCtx.s_phase_backup_volt = gstCollectInfo.power_data.power_value_info.s_backup_vol;
    _gstUiCtx.t_phase_backup_volt = gstCollectInfo.power_data.power_value_info.t_backup_vol;
    _gstUiCtx.r_phase_backup_curr = gstCollectInfo.power_data.power_value_info.r_backup_current;
    _gstUiCtx.s_phase_backup_curr = gstCollectInfo.power_data.power_value_info.s_backup_current;
    _gstUiCtx.t_phase_backup_curr = gstCollectInfo.power_data.power_value_info.t_backup_current;
#if 0
    _gstUiCtx.total_backup_active_power = gstCollectInfo.power_data.power_value_info.r_backup_act_power +
                                          gstCollectInfo.power_data.power_value_info.s_backup_act_power +
                                          gstCollectInfo.power_data.power_value_info.t_backup_act_power;
    
    _gstUiCtx.total_backup_reactive_power = gstCollectInfo.power_data.power_value_info.r_backup_react_power +
                                            gstCollectInfo.power_data.power_value_info.s_backup_react_power +
                                            gstCollectInfo.power_data.power_value_info.t_backup_react_power;
    
    _gstUiCtx.total_backup_apparent_power = gstCollectInfo.power_data.power_value_info.r_backup_apparent_power +
                                            gstCollectInfo.power_data.power_value_info.s_backup_apparent_power +
                                            gstCollectInfo.power_data.power_value_info.t_backup_apparent_power;
#else
    _gstUiCtx.r_backup_active_power = gstCollectInfo.power_data.power_value_info.r_backup_act_power;
    _gstUiCtx.s_backup_active_power = gstCollectInfo.power_data.power_value_info.s_backup_act_power;
    _gstUiCtx.t_backup_active_power = gstCollectInfo.power_data.power_value_info.t_backup_act_power;

    _gstUiCtx.r_backup_reactive_power = gstCollectInfo.power_data.power_value_info.r_backup_react_power;
    _gstUiCtx.s_backup_reactive_power = gstCollectInfo.power_data.power_value_info.s_backup_react_power;
    _gstUiCtx.t_backup_reactive_power = gstCollectInfo.power_data.power_value_info.t_backup_react_power;

    _gstUiCtx.r_backup_apparent_power = gstCollectInfo.power_data.power_value_info.r_backup_apparent_power;
    _gstUiCtx.s_backup_apparent_power = gstCollectInfo.power_data.power_value_info.s_backup_apparent_power;
    _gstUiCtx.t_backup_apparent_power = gstCollectInfo.power_data.power_value_info.t_backup_apparent_power;
#endif

    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00EEEEEE;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListBackUpLoadInfoPara.list_start_id = WIN_LIST_BACK_UP_LOAD_INFO_ID + 0x30;
    _gstEmListBackUpLoadInfoPara.list_num = 5;
    _gstEmListBackUpLoadInfoPara.list_display_num = 6;
    if(_gstEmListBackUpLoadInfoPara.list_num <= _gstEmListBackUpLoadInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListBackUpLoadInfoPara.list_num + 1 - _gstEmListBackUpLoadInfoPara.list_display_num;
    }

    if(_gstEmListBackUpLoadInfoPara.list_sel > _gstEmListBackUpLoadInfoPara.list_num - 1)
    {
        _gstEmListBackUpLoadInfoPara.list_sel = 0;
    }
    if(_gstEmListBackUpLoadInfoPara.list_page > max_page - 1)
    {
        _gstEmListBackUpLoadInfoPara.list_page = 0;
    }
    _gstEmListBackUpLoadInfoPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _gstEmListBackUpLoadInfoPara.selected_r = 10;

    /*4个子列表*/
    _gstEmListBackUpLoadInfoPara.sub_region[0].x_pos = 20;
    _gstEmListBackUpLoadInfoPara.sub_region[0].y_pos = 38;
    _gstEmListBackUpLoadInfoPara.sub_region[0].x_size = 440;
    _gstEmListBackUpLoadInfoPara.sub_region[0].y_size = 34;
    _gstEmListBackUpLoadInfoPara.sub_region[1].x_pos = 20;
    _gstEmListBackUpLoadInfoPara.sub_region[1].y_pos = 80;
    _gstEmListBackUpLoadInfoPara.sub_region[1].x_size = 440;
    _gstEmListBackUpLoadInfoPara.sub_region[1].y_size = 34;
    _gstEmListBackUpLoadInfoPara.sub_region[2].x_pos = 20;
    _gstEmListBackUpLoadInfoPara.sub_region[2].y_pos = 122;
    _gstEmListBackUpLoadInfoPara.sub_region[2].x_size = 440;
    _gstEmListBackUpLoadInfoPara.sub_region[2].y_size = 34;
    _gstEmListBackUpLoadInfoPara.sub_region[3].x_pos = 20;
    _gstEmListBackUpLoadInfoPara.sub_region[3].y_pos = 164;
    _gstEmListBackUpLoadInfoPara.sub_region[3].x_size = 440;
    _gstEmListBackUpLoadInfoPara.sub_region[3].y_size = 34;
    _gstEmListBackUpLoadInfoPara.sub_region[4].x_pos = 20;
    _gstEmListBackUpLoadInfoPara.sub_region[4].y_pos = 206;
    _gstEmListBackUpLoadInfoPara.sub_region[4].x_size = 440;
    _gstEmListBackUpLoadInfoPara.sub_region[4].y_size = 34;


    /*List控件*/

    //第一行左边
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][0].x_size = 148;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListBackUpLoadInfoLeftTextBuf[0];
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBackUpLoadInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "BackUp Voltage");
    //第一行右边
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][0].x_relative_pos = 200;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][0].x_size = 225;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][0].y_size = 17;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListBackUpLoadInfoRightTextBuf[0];
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 220;//测试数据，实际赋值系统数据
    _SetBackupVoltage(0, SINGLE_PHASE, _gstUiCtx.r_phase_backup_volt, _gstUiCtx.s_phase_backup_volt, _gstUiCtx.t_phase_backup_volt);
    
    //第二行左边
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][1].x_size = 148;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListBackUpLoadInfoLeftTextBuf[1];
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBackUpLoadInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "BackUp Current");
    //第二行右边
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][1].x_relative_pos = 200;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][1].x_size = 225;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][1].y_size = 17;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListBackUpLoadInfoRightTextBuf[1];
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    _SetBackupCurrent(0, SINGLE_PHASE, _gstUiCtx.r_phase_backup_curr, _gstUiCtx.s_phase_backup_curr, _gstUiCtx.t_phase_backup_curr);
    
    //第三行左边
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][2].x_size = 148;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListBackUpLoadInfoLeftTextBuf[2];
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBackUpLoadInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Active Power");
    //第三行右边
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][2].x_relative_pos = 200;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][2].x_size = 225;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][2].y_size = 17;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListBackUpLoadInfoRightTextBuf[2];
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    _SetBackupActivePower(0, _gstUiCtx.r_backup_active_power, _gstUiCtx.s_backup_active_power, _gstUiCtx.t_backup_active_power);
//    _SetBackupActivePower(0, _gstUiCtx.total_backup_active_power);
    //第四行左边
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][3].x_size = 148;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListBackUpLoadInfoLeftTextBuf[3];
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBackUpLoadInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Reactive Power");
    //第四行右边
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][3].x_relative_pos = 200;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][3].x_size = 225;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][3].y_size = 17;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _garrListBackUpLoadInfoRightTextBuf[3];
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    _SetBackupReactivePower(0, _gstUiCtx.r_backup_reactive_power, _gstUiCtx.s_backup_reactive_power, _gstUiCtx.t_backup_reactive_power);
//    _SetBackupReactivePower(0, _gstUiCtx.total_backup_reactive_power);
    //第五行左边
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][4].x_size = 148;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListBackUpLoadInfoLeftTextBuf[4];
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBackUpLoadInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Apparent Power");
    //第五行右边
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][4].x_relative_pos = 200;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][4].x_size = 225;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][4].y_size = 17;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _garrListBackUpLoadInfoRightTextBuf[4];
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBackUpLoadInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
    _SetBackupApparentPower(0, _gstUiCtx.r_backup_apparent_power, _gstUiCtx.s_backup_apparent_power, _gstUiCtx.t_backup_apparent_power);
//    _SetBackupApparentPower(0, _gstUiCtx.total_backup_apparent_power);
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
        gstCollectInfo.power_data.power_value_info.r_backup_act_power +
        gstCollectInfo.power_data.power_value_info.s_backup_act_power +
        gstCollectInfo.power_data.power_value_info.t_backup_act_power,
        
        gstCollectInfo.power_data.power_value_info.r_backup_react_power +
        gstCollectInfo.power_data.power_value_info.s_backup_react_power +
        gstCollectInfo.power_data.power_value_info.t_backup_react_power,
        
        gstCollectInfo.power_data.power_value_info.r_backup_apparent_power +
        gstCollectInfo.power_data.power_value_info.s_backup_apparent_power +
        gstCollectInfo.power_data.power_value_info.t_backup_apparent_power, 
    };
#endif
    
    if(_gstUiCtx.r_phase_backup_volt != gstCollectInfo.power_data.power_value_info.r_backup_vol || \
       _gstUiCtx.s_phase_backup_volt != gstCollectInfo.power_data.power_value_info.s_backup_vol || \
       _gstUiCtx.t_phase_backup_volt != gstCollectInfo.power_data.power_value_info.t_backup_vol)
    {
        _gstUiCtx.r_phase_backup_volt = gstCollectInfo.power_data.power_value_info.r_backup_vol;
        _gstUiCtx.s_phase_backup_volt = gstCollectInfo.power_data.power_value_info.s_backup_vol;
        _gstUiCtx.t_phase_backup_volt = gstCollectInfo.power_data.power_value_info.t_backup_vol;
        _SetBackupVoltage(1, SINGLE_PHASE, _gstUiCtx.r_phase_backup_volt, _gstUiCtx.s_phase_backup_volt, _gstUiCtx.t_phase_backup_volt);
    }

    if(_gstUiCtx.r_phase_backup_curr != gstCollectInfo.power_data.power_value_info.r_backup_current || \
       _gstUiCtx.s_phase_backup_curr != gstCollectInfo.power_data.power_value_info.s_backup_current || \
       _gstUiCtx.t_phase_backup_curr != gstCollectInfo.power_data.power_value_info.t_backup_current)
    {
        _gstUiCtx.r_phase_backup_curr = gstCollectInfo.power_data.power_value_info.r_backup_current;
        _gstUiCtx.s_phase_backup_curr = gstCollectInfo.power_data.power_value_info.s_backup_current;
        _gstUiCtx.t_phase_backup_curr = gstCollectInfo.power_data.power_value_info.t_backup_current;
        _SetBackupCurrent(1, SINGLE_PHASE, _gstUiCtx.r_phase_backup_curr, _gstUiCtx.s_phase_backup_curr, _gstUiCtx.t_phase_backup_curr);
    }
#if 0   
    if(_gstUiCtx.total_backup_active_power != power[0])
    {
        _gstUiCtx.total_backup_active_power = power[0];
        _SetBackupActivePower(0, _gstUiCtx.total_backup_active_power);
    }
    
    if(_gstUiCtx.total_backup_reactive_power != power[1])
    {
        _gstUiCtx.total_backup_reactive_power = power[1];
        _SetBackupReactivePower(1, _gstUiCtx.total_backup_reactive_power);
    }
    
    if(_gstUiCtx.total_backup_apparent_power != power[2])
    {
        _gstUiCtx.total_backup_apparent_power = power[2];
        _SetBackupApparentPower(1, _gstUiCtx.total_backup_apparent_power);
    }

#else
    if(_gstUiCtx.r_backup_active_power != gstCollectInfo.power_data.power_value_info.r_backup_act_power || \
       _gstUiCtx.s_backup_active_power != gstCollectInfo.power_data.power_value_info.s_backup_act_power || \
       _gstUiCtx.t_backup_active_power != gstCollectInfo.power_data.power_value_info.t_backup_act_power)
    {
        _gstUiCtx.r_backup_active_power = gstCollectInfo.power_data.power_value_info.r_backup_act_power;
        _gstUiCtx.s_backup_active_power = gstCollectInfo.power_data.power_value_info.s_backup_act_power;
        _gstUiCtx.t_backup_active_power = gstCollectInfo.power_data.power_value_info.t_backup_act_power;

        _SetBackupActivePower(1, _gstUiCtx.r_backup_active_power, _gstUiCtx.s_backup_active_power, _gstUiCtx.t_backup_active_power);
    }

    if(_gstUiCtx.r_backup_reactive_power != gstCollectInfo.power_data.power_value_info.r_backup_react_power || \
       _gstUiCtx.s_backup_reactive_power != gstCollectInfo.power_data.power_value_info.s_backup_react_power || \
       _gstUiCtx.t_backup_reactive_power != gstCollectInfo.power_data.power_value_info.t_backup_react_power)
    {
        _gstUiCtx.r_backup_reactive_power = gstCollectInfo.power_data.power_value_info.r_backup_react_power;
        _gstUiCtx.s_backup_reactive_power = gstCollectInfo.power_data.power_value_info.s_backup_react_power;
        _gstUiCtx.t_backup_reactive_power = gstCollectInfo.power_data.power_value_info.t_backup_react_power;
        _SetBackupReactivePower(1, _gstUiCtx.r_backup_reactive_power, _gstUiCtx.s_backup_reactive_power, _gstUiCtx.t_backup_reactive_power);
    }

    if(_gstUiCtx.r_backup_apparent_power != gstCollectInfo.power_data.power_value_info.r_backup_apparent_power ||
       _gstUiCtx.s_backup_apparent_power != gstCollectInfo.power_data.power_value_info.s_backup_apparent_power ||
       _gstUiCtx.t_backup_apparent_power != gstCollectInfo.power_data.power_value_info.t_backup_apparent_power)
    {
        _gstUiCtx.r_backup_apparent_power = gstCollectInfo.power_data.power_value_info.r_backup_apparent_power;
        _gstUiCtx.s_backup_apparent_power = gstCollectInfo.power_data.power_value_info.s_backup_apparent_power;
        _gstUiCtx.t_backup_apparent_power = gstCollectInfo.power_data.power_value_info.t_backup_apparent_power;
        _SetBackupApparentPower(1, _gstUiCtx.r_backup_apparent_power, _gstUiCtx.s_backup_apparent_power, _gstUiCtx.t_backup_apparent_power);
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

