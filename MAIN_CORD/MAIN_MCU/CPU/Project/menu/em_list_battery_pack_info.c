/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-11
  * @brief       电池信息
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


emWin_t gstEmWinListBatteryPackInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t     select;
    int32_t     key_init;
    uint16_t    batt_volt;
    uint16_t    pack_power;
    uint16_t    batt_current;
    uint32_t    rated_energy;
    uint16_t    batt_soh;
    uint16_t    batt_soc;
} UiCtx_t;

static UiCtx_t _gstUiCtx;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

static emWidgetTextPara_t  _gstTitleTextPara;

static char _garrTitleBuf[] = "Operational State Info < Battery < Batttery Pack";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";


/*List控件*/
static emListPara_t  _gstEmListBatteryPackInfoPara;

static char _garrListBatteryPackInfoLeftTextBuf[6][20];
static char _garrListBatteryPackInfoRightTextBuf[6][20];


#define WIN_LIST_BATTERY_PACK_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_BATTERY_PACK_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListBatteryPackDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListBatteryPackInfoWin",    WIN_LIST_BATTERY_PACK_INFO_ID,            0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_BATTERY_PACK_INFO_TITLE_ID,             31,  20,  415,  14,   0,   0x0,  0 },
};

static int32_t _SetBatteryVoltage(int8_t ui_update, uint16_t vol)
{
    int16_t vol_tmp = 0;
    vol_tmp = (int16_t)vol;
    float voltage = (float)vol_tmp / 100;
    snprintf(_garrListBatteryPackInfoRightTextBuf[0], sizeof(_garrListBatteryPackInfoRightTextBuf[0]), "%.1fV", voltage);
    if (ui_update)
    {
        TEXT_SetText(_gstEmListBatteryPackInfoPara.handle[1][0], (const char *)_gstEmListBatteryPackInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetBatteryCurrent(int8_t ui_update, uint16_t cur)
{
    int16_t cur_value = (int16_t)cur;
    float current = (float)cur_value / 10;
    snprintf(_garrListBatteryPackInfoRightTextBuf[1], sizeof(_garrListBatteryPackInfoRightTextBuf[1]), "%.1fA", current);
    if (ui_update)
    {
        TEXT_SetText(_gstEmListBatteryPackInfoPara.handle[1][1], (const char *)_gstEmListBatteryPackInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    }
    return 0;
}

//额定能量
static int32_t _SetBatteryPackRatedEnergy(int8_t ui_update, uint32_t rate_energy)
{
    int32_t cap_tmp = 0;
    cap_tmp = (int32_t)rate_energy;
    float capacity_value = (float)cap_tmp;   //已在外围计算除十
    
    if(capacity_value >= 0.0f && capacity_value < 1000.0f)
    {
        snprintf(_garrListBatteryPackInfoRightTextBuf[2], sizeof(_garrListBatteryPackInfoRightTextBuf[2]), "%.1fWh", capacity_value);
    }
    else if(capacity_value >= 1000.0f)
    {
        capacity_value /= 1000;
        snprintf(_garrListBatteryPackInfoRightTextBuf[2], sizeof(_garrListBatteryPackInfoRightTextBuf[2]), "%.2fkWh", capacity_value);
    }

    if (ui_update)
    {
        TEXT_SetText(_gstEmListBatteryPackInfoPara.handle[1][2], (const char *)_gstEmListBatteryPackInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetBatteryPackSOC(int8_t ui_update, uint16_t soc)
{
    int16_t soc_value = 0;

    soc_value = soc / 10;
    
    snprintf(_garrListBatteryPackInfoRightTextBuf[3], sizeof(_garrListBatteryPackInfoRightTextBuf[3]), "%d%%", soc_value);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListBatteryPackInfoPara.handle[1][3], (const char *)_gstEmListBatteryPackInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetBatteryPackSOH(int8_t ui_update, uint16_t soh)
{
    int16_t soh_value = soh / 10;
    snprintf(_garrListBatteryPackInfoRightTextBuf[4], sizeof(_garrListBatteryPackInfoRightTextBuf[4]), "%d%%", soh_value);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListBatteryPackInfoPara.handle[1][4], (const char *)_gstEmListBatteryPackInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }
    return 0;
}
static int32_t _SetPackPower(int8_t ui_update, uint16_t power)
{
    int16_t bat_power = (int16_t)power;

    snprintf(_garrListBatteryPackInfoRightTextBuf[5], sizeof(_garrListBatteryPackInfoRightTextBuf[5]), "%dW", bat_power);

    if(ui_update)
    {    
        TEXT_SetText(_gstEmListBatteryPackInfoPara.handle[1][5], (const char *)_gstEmListBatteryPackInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
    }
    return 0;
}

/**
  * @brief ListBatteryPackInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListBatteryPackInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BATTERY_PACK_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListBatteryPackInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListBatteryPackInfoPara, _gstEmListBatteryPackInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListBatteryPackInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListBatteryPackInfoPara.sub_region[i].x_pos,
                                    _gstEmListBatteryPackInfoPara.sub_region[i].y_pos,
                                    _gstEmListBatteryPackInfoPara.sub_region[i].x_pos + _gstEmListBatteryPackInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListBatteryPackInfoPara.sub_region[i].y_pos + _gstEmListBatteryPackInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListBatteryPackInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListBatteryPackInfoPara, _gstEmListBatteryPackInfoPara.list_sel);

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
    pPrevEmWin = &gstEmWinListBatteryInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListBatteryPackInfo.win_handle = GUI_CreateDialogBox(_garrListBatteryPackDialogCreate, GUI_COUNTOF(_garrListBatteryPackDialogCreate), _cbListBatteryPackInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListBatteryPackInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListBatteryPackInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListBatteryPackInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListBatteryPackInfoPara, gstEmWinListBatteryPackInfo.win_handle);
    return gstEmWinListBatteryPackInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListBatteryPackInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListBatteryPackInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListBatteryPackInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page, i;
    EmKeyDefaultInfoSet();
#if 0
    uint16_t batt_current = 0;
    uint16_t power_temp   = 0;  
    for(i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        batt_current += gstPackSlavePara.pack_para[i].dcdc_info.power_info.output_curr;
        power_temp   += gstPackSlavePara.pack_para[i].dcdc_info.power_info.output_power;
    }
    
    _gstUiCtx.batt_volt      = gstPackSlavePara.pack_para[i].dcdc_info.power_info.output_vol;
    _gstUiCtx.batt_current   = batt_current;
    _gstUiCtx.batt_power     = power_temp;
    _gstUiCtx.rated_energy   = gstCollectInfo.maincard_data.total_pack_info.rated_energy;
    _gstUiCtx.batt_soh       = gstCollectInfo.maincard_data.total_pack_info.soh;
    _gstUiCtx.batt_soc       = gstCollectInfo.maincard_data.total_pack_info.soc;
#endif
    switch(gPackInfoIdx)
    {
        case 0:
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[0].bms_info.status_info.total_vol;
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[0].bms_info.status_info.total_curr;
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[0].bms_info.status_info.design_cap / 10) * 25.6f;
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[0].bms_info.status_info.soc_disp;
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[0].bms_info.status_info.soh;
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[0].dcdc_info.power_info.output_power;
            break;
        }
        case 1:
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[1].bms_info.status_info.total_vol;
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[1].bms_info.status_info.total_curr;
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[1].bms_info.status_info.design_cap / 10) * 25.6f;
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[1].bms_info.status_info.soc_disp;
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[1].bms_info.status_info.soh;
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[1].dcdc_info.power_info.output_power;
            
            break;
        }
        case 2:
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[2].bms_info.status_info.total_vol;
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[2].bms_info.status_info.total_curr;
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[2].bms_info.status_info.design_cap / 10) * 25.6f;
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[2].bms_info.status_info.soc_disp;
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[2].bms_info.status_info.soh;
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[2].dcdc_info.power_info.output_power;
            break;
        }
        case 3:
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[3].bms_info.status_info.total_vol;
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[3].bms_info.status_info.total_curr;
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[3].bms_info.status_info.design_cap / 10) * 25.6f;
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[3].bms_info.status_info.soc_disp;
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[3].bms_info.status_info.soh;
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[3].dcdc_info.power_info.output_power;
            break;
        }
        case 4:
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[4].bms_info.status_info.total_vol;
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[4].bms_info.status_info.total_curr;
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[4].bms_info.status_info.design_cap / 10) * 25.6f;
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[4].bms_info.status_info.soc_disp;
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[4].bms_info.status_info.soh;
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[4].dcdc_info.power_info.output_power;
            break;
        }
        case 5:
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[5].bms_info.status_info.total_vol;
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[5].bms_info.status_info.total_curr;
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[5].bms_info.status_info.design_cap / 10) * 25.6f;
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[5].bms_info.status_info.soc_disp;
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[5].bms_info.status_info.soh;
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[5].dcdc_info.power_info.output_power;
            break;
        }
        default:
            break;
    }
    
    
    _gstTitleTextPara.p_content   = _garrTitleBuf;
    _gstTitleTextPara.text_color  = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListBatteryPackInfoPara.list_start_id = WIN_LIST_BATTERY_PACK_INFO_ID + 0x30;
    _gstEmListBatteryPackInfoPara.list_num = 6;
    _gstEmListBatteryPackInfoPara.list_display_num = 6;
    if(_gstEmListBatteryPackInfoPara.list_num <= _gstEmListBatteryPackInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListBatteryPackInfoPara.list_num + 1 - _gstEmListBatteryPackInfoPara.list_display_num;
    }

    if(_gstEmListBatteryPackInfoPara.list_sel > _gstEmListBatteryPackInfoPara.list_num - 1)
    {
        _gstEmListBatteryPackInfoPara.list_sel = 0;
    }
    if(_gstEmListBatteryPackInfoPara.list_page > max_page - 1)
    {
        _gstEmListBatteryPackInfoPara.list_page = 0;
    }
    _gstEmListBatteryPackInfoPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _gstEmListBatteryPackInfoPara.selected_r = 10;

    /*4个子列表*/
    _gstEmListBatteryPackInfoPara.sub_region[0].x_pos = 20;
    _gstEmListBatteryPackInfoPara.sub_region[0].y_pos = 38;
    _gstEmListBatteryPackInfoPara.sub_region[0].x_size = 440;
    _gstEmListBatteryPackInfoPara.sub_region[0].y_size = 34;
    _gstEmListBatteryPackInfoPara.sub_region[1].x_pos = 20;
    _gstEmListBatteryPackInfoPara.sub_region[1].y_pos = 80;
    _gstEmListBatteryPackInfoPara.sub_region[1].x_size = 440;
    _gstEmListBatteryPackInfoPara.sub_region[1].y_size = 34;
    _gstEmListBatteryPackInfoPara.sub_region[2].x_pos = 20;
    _gstEmListBatteryPackInfoPara.sub_region[2].y_pos = 122;
    _gstEmListBatteryPackInfoPara.sub_region[2].x_size = 440;
    _gstEmListBatteryPackInfoPara.sub_region[2].y_size = 34;
    _gstEmListBatteryPackInfoPara.sub_region[3].x_pos = 20;
    _gstEmListBatteryPackInfoPara.sub_region[3].y_pos = 164;
    _gstEmListBatteryPackInfoPara.sub_region[3].x_size = 440;
    _gstEmListBatteryPackInfoPara.sub_region[3].y_size = 34;
    _gstEmListBatteryPackInfoPara.sub_region[4].x_pos = 20;
    _gstEmListBatteryPackInfoPara.sub_region[4].y_pos = 206;
    _gstEmListBatteryPackInfoPara.sub_region[4].x_size = 440;
    _gstEmListBatteryPackInfoPara.sub_region[4].y_size = 34;
    _gstEmListBatteryPackInfoPara.sub_region[5].x_pos = 20;
    _gstEmListBatteryPackInfoPara.sub_region[5].y_pos = 248;
    _gstEmListBatteryPackInfoPara.sub_region[5].x_size = 440;
    _gstEmListBatteryPackInfoPara.sub_region[5].y_size = 34;

    /*List控件*/

    //第一行左边
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][0].x_size = 188;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListBatteryPackInfoLeftTextBuf[0];
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBatteryPackInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Battery Voltage");
    //第一行右边
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][0].x_relative_pos = 220;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][0].x_size = 208;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][0].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListBatteryPackInfoRightTextBuf[0];
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 50;//测试数据，实际赋值系统数据
    _SetBatteryVoltage(0, _gstUiCtx.batt_volt);
    //第二行左边
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][1].x_size = 188;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListBatteryPackInfoLeftTextBuf[1];
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBatteryPackInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Battery Current");
    //第二行右边
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][1].x_relative_pos = 220;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][1].x_size = 208;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][1].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListBatteryPackInfoRightTextBuf[1];
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    _SetBatteryCurrent(0, _gstUiCtx.batt_current);
    //第三行左边
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][2].x_size = 188;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListBatteryPackInfoLeftTextBuf[2];
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBatteryPackInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Rated Energy");
    //第三行右边
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][2].x_relative_pos = 220;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][2].x_size = 208;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][2].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListBatteryPackInfoRightTextBuf[2];
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 100;//测试数据，实际赋值系统数据
    _SetBatteryPackRatedEnergy(0, _gstUiCtx.rated_energy);
    //第四行左边
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][3].x_size = 188;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListBatteryPackInfoLeftTextBuf[3];
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBatteryPackInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "SOC");
    //第四行右边
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][3].x_relative_pos = 220;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][3].x_size = 208;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][3].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _garrListBatteryPackInfoRightTextBuf[3];
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 100;//测试数据，实际赋值系统数据
    _SetBatteryPackSOC(0, _gstUiCtx.batt_soc);
    //第五行左边
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][4].x_size = 188;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListBatteryPackInfoLeftTextBuf[4];
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBatteryPackInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "SOH");
    //第五行右边
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][4].x_relative_pos = 220;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][4].x_size = 208;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][4].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _garrListBatteryPackInfoRightTextBuf[4];
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 85;//测试数据，实际赋值系统数据
    _SetBatteryPackSOH(0, _gstUiCtx.batt_soh);
    //第6行左边
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][5].x_size = 188;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListBatteryPackInfoLeftTextBuf[5];
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListBatteryPackInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Pack Power");
    //第6行右边
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][5].x_relative_pos = 220;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][5].y_relative_pos = 10;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][5].x_size = 208;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][5].y_size = 17;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _garrListBatteryPackInfoRightTextBuf[5];
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListBatteryPackInfoPara.sub_widget_para[1][5].sub_widget.text_para.digit_value = 85;//测试数据，实际赋值系统数据 
    _SetPackPower(0, _gstUiCtx.pack_power);
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
    if(0 == gPackInfoIdx)
    {
        if (_gstUiCtx.batt_volt != gstPackSlavePara.pack_para[0].bms_info.status_info.total_vol)
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[0].bms_info.status_info.total_vol;
            _SetBatteryVoltage(1, _gstUiCtx.batt_volt);
        }

        if (_gstUiCtx.batt_current != gstPackSlavePara.pack_para[0].bms_info.status_info.total_curr)
        {
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[0].bms_info.status_info.total_curr;;
            _SetBatteryCurrent(1, _gstUiCtx.batt_current);
        }

        if (_gstUiCtx.rated_energy != (gstPackSlavePara.pack_para[0].bms_info.status_info.design_cap / 10) * 25.6f)
        {
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[0].bms_info.status_info.design_cap / 10) * 25.6f;
            _SetBatteryPackRatedEnergy(1, _gstUiCtx.rated_energy);
        }

        if (_gstUiCtx.batt_soh != gstPackSlavePara.pack_para[0].bms_info.status_info.soh)
        {
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[0].bms_info.status_info.soh;
            _SetBatteryPackSOH(1, _gstUiCtx.batt_soh);
        }

        if (_gstUiCtx.batt_soc != gstPackSlavePara.pack_para[0].bms_info.status_info.soc_disp)
        {
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[0].bms_info.status_info.soc_disp;
            _SetBatteryPackSOC(1, _gstUiCtx.batt_soc);
        }
                
        if(_gstUiCtx.pack_power != gstPackSlavePara.pack_para[0].dcdc_info.power_info.output_power)
        {
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[0].dcdc_info.power_info.output_power;;
            _SetPackPower(1, _gstUiCtx.pack_power);
        }
    }
    else if(1 == gPackInfoIdx)
    {
        if (_gstUiCtx.batt_volt != gstPackSlavePara.pack_para[1].bms_info.status_info.total_vol)
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[1].bms_info.status_info.total_vol;
            _SetBatteryVoltage(1, _gstUiCtx.batt_volt);
        }

        if (_gstUiCtx.batt_current != gstPackSlavePara.pack_para[1].bms_info.status_info.total_curr)
        {
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[1].bms_info.status_info.total_curr;;
            _SetBatteryCurrent(1, _gstUiCtx.batt_current);
        }

        if (_gstUiCtx.rated_energy != (gstPackSlavePara.pack_para[1].bms_info.status_info.design_cap / 10) * 25.6f)
        {
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[1].bms_info.status_info.design_cap / 10) * 25.6f;
            _SetBatteryPackRatedEnergy(1, _gstUiCtx.rated_energy);
        }

        if (_gstUiCtx.batt_soh != gstPackSlavePara.pack_para[1].bms_info.status_info.soh)
        {
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[1].bms_info.status_info.soh;
            _SetBatteryPackSOH(1, _gstUiCtx.batt_soh);
        }

        if (_gstUiCtx.batt_soc != gstPackSlavePara.pack_para[1].bms_info.status_info.soc_disp)
        {
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[1].bms_info.status_info.soc_disp;
            _SetBatteryPackSOC(1, _gstUiCtx.batt_soc);
        }
                
        if(_gstUiCtx.pack_power != gstPackSlavePara.pack_para[1].dcdc_info.power_info.output_power)
        {
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[1].dcdc_info.power_info.output_power;;
            _SetPackPower(1, _gstUiCtx.pack_power);
        }
    }
    else if(2 == gPackInfoIdx)
    {
        if (_gstUiCtx.batt_volt != gstPackSlavePara.pack_para[2].bms_info.status_info.total_vol)
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[2].bms_info.status_info.total_vol;
            _SetBatteryVoltage(1, _gstUiCtx.batt_volt);
        }

        if (_gstUiCtx.batt_current != gstPackSlavePara.pack_para[2].bms_info.status_info.total_curr)
        {
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[2].bms_info.status_info.total_curr;;
            _SetBatteryCurrent(1, _gstUiCtx.batt_current);
        }

        if (_gstUiCtx.rated_energy != (gstPackSlavePara.pack_para[2].bms_info.status_info.design_cap / 10) * 25.6f)
        {
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[2].bms_info.status_info.design_cap / 10) * 25.6f;
            _SetBatteryPackRatedEnergy(1, _gstUiCtx.rated_energy);
        }

        if (_gstUiCtx.batt_soh != gstPackSlavePara.pack_para[2].bms_info.status_info.soh)
        {
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[2].bms_info.status_info.soh;
            _SetBatteryPackSOH(1, _gstUiCtx.batt_soh);
        }

        if (_gstUiCtx.batt_soc != gstPackSlavePara.pack_para[2].bms_info.status_info.soc_disp)
        {
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[2].bms_info.status_info.soc_disp;
            _SetBatteryPackSOC(1, _gstUiCtx.batt_soc);
        }
                
        if(_gstUiCtx.pack_power != gstPackSlavePara.pack_para[2].dcdc_info.power_info.output_power)
        {
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[2].dcdc_info.power_info.output_power;;
            _SetPackPower(1, _gstUiCtx.pack_power);
        }
    }
    else if(3 == gPackInfoIdx)
    {
        if (_gstUiCtx.batt_volt != gstPackSlavePara.pack_para[3].bms_info.status_info.total_vol)
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[3].bms_info.status_info.total_vol;
            _SetBatteryVoltage(1, _gstUiCtx.batt_volt);
        }

        if (_gstUiCtx.batt_current != gstPackSlavePara.pack_para[3].bms_info.status_info.total_curr)
        {
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[3].bms_info.status_info.total_curr;;
            _SetBatteryCurrent(1, _gstUiCtx.batt_current);
        }

        if (_gstUiCtx.rated_energy != (gstPackSlavePara.pack_para[3].bms_info.status_info.design_cap / 10) * 25.6f)
        {
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[3].bms_info.status_info.design_cap / 10) * 25.6f;
            _SetBatteryPackRatedEnergy(1, _gstUiCtx.rated_energy);
        }

        if (_gstUiCtx.batt_soh != gstPackSlavePara.pack_para[3].bms_info.status_info.soh)
        {
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[3].bms_info.status_info.soh;
            _SetBatteryPackSOH(1, _gstUiCtx.batt_soh);
        }

        if (_gstUiCtx.batt_soc != gstPackSlavePara.pack_para[3].bms_info.status_info.soc_disp)
        {
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[3].bms_info.status_info.soc_disp;
            _SetBatteryPackSOC(1, _gstUiCtx.batt_soc);
        }
                
        if(_gstUiCtx.pack_power != gstPackSlavePara.pack_para[3].dcdc_info.power_info.output_power)
        {
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[3].dcdc_info.power_info.output_power;;
            _SetPackPower(1, _gstUiCtx.pack_power);
        }
    }
    else if(4 == gPackInfoIdx)
    {
        if (_gstUiCtx.batt_volt != gstPackSlavePara.pack_para[4].bms_info.status_info.total_vol)
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[4].bms_info.status_info.total_vol;
            _SetBatteryVoltage(1, _gstUiCtx.batt_volt);
        }

        if (_gstUiCtx.batt_current != gstPackSlavePara.pack_para[4].bms_info.status_info.total_curr)
        {
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[4].bms_info.status_info.total_curr;;
            _SetBatteryCurrent(1, _gstUiCtx.batt_current);
        }

        if (_gstUiCtx.rated_energy != (gstPackSlavePara.pack_para[4].bms_info.status_info.design_cap / 10) * 25.6f)
        {
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[4].bms_info.status_info.design_cap / 10) * 25.6f;
            _SetBatteryPackRatedEnergy(1, _gstUiCtx.rated_energy);
        }

        if (_gstUiCtx.batt_soh != gstPackSlavePara.pack_para[4].bms_info.status_info.soh)
        {
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[4].bms_info.status_info.soh;
            _SetBatteryPackSOH(1, _gstUiCtx.batt_soh);
        }

        if (_gstUiCtx.batt_soc != gstPackSlavePara.pack_para[4].bms_info.status_info.soc_disp)
        {
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[4].bms_info.status_info.soc_disp;
            _SetBatteryPackSOC(1, _gstUiCtx.batt_soc);
        }
                
        if(_gstUiCtx.pack_power != gstPackSlavePara.pack_para[4].dcdc_info.power_info.output_power)
        {
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[4].dcdc_info.power_info.output_power;;
            _SetPackPower(1, _gstUiCtx.pack_power);
        }
    }
    else if(5 == gPackInfoIdx)
    {
        if (_gstUiCtx.batt_volt != gstPackSlavePara.pack_para[5].bms_info.status_info.total_vol)
        {
            _gstUiCtx.batt_volt = gstPackSlavePara.pack_para[5].bms_info.status_info.total_vol;
            _SetBatteryVoltage(1, _gstUiCtx.batt_volt);
        }

        if (_gstUiCtx.batt_current != gstPackSlavePara.pack_para[5].bms_info.status_info.total_curr)
        {
            _gstUiCtx.batt_current = gstPackSlavePara.pack_para[5].bms_info.status_info.total_curr;;
            _SetBatteryCurrent(1, _gstUiCtx.batt_current);
        }

        if (_gstUiCtx.rated_energy != (gstPackSlavePara.pack_para[5].bms_info.status_info.design_cap / 10) * 25.6f)
        {
            _gstUiCtx.rated_energy = (gstPackSlavePara.pack_para[5].bms_info.status_info.design_cap / 10) * 25.6f;
            _SetBatteryPackRatedEnergy(1, _gstUiCtx.rated_energy);
        }

        if (_gstUiCtx.batt_soh != gstPackSlavePara.pack_para[5].bms_info.status_info.soh)
        {
            _gstUiCtx.batt_soh = gstPackSlavePara.pack_para[5].bms_info.status_info.soh;
            _SetBatteryPackSOH(1, _gstUiCtx.batt_soh);
        }

        if (_gstUiCtx.batt_soc != gstPackSlavePara.pack_para[5].bms_info.status_info.soc_disp)
        {
            _gstUiCtx.batt_soc = gstPackSlavePara.pack_para[5].bms_info.status_info.soc_disp;
            _SetBatteryPackSOC(1, _gstUiCtx.batt_soc);
        }
                
        if(_gstUiCtx.pack_power != gstPackSlavePara.pack_para[5].dcdc_info.power_info.output_power)
        {
            _gstUiCtx.pack_power = gstPackSlavePara.pack_para[5].dcdc_info.power_info.output_power;;
            _SetPackPower(1, _gstUiCtx.pack_power);
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
                pnext = &gstEmWinListBatteryInfo;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}

