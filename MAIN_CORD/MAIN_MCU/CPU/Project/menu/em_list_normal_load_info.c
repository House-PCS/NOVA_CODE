/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-11
  * @brief       Normal负载输出信息
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


emWin_t gstEmWinListNormalLoadInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t r_phase_load_volt;
    int16_t r_phase_load_curr;
    uint16_t r_phase_load_freq;             
    uint16_t s_phase_load_volt;
    int16_t s_phase_load_curr;
    uint16_t s_phase_load_freq;   
    uint16_t t_phase_load_volt;
    int16_t t_phase_load_curr;
    uint16_t t_phase_load_freq;   
    int32_t r_load_active_power;
    int32_t r_load_reactive_power;
    int32_t r_load_apparent_power;
    int32_t s_load_active_power;
    int32_t s_load_reactive_power;
    int32_t s_load_apparent_power;
    int32_t t_load_active_power;
    int32_t t_load_reactive_power;
    int32_t t_load_apparent_power;

    int32_t total_load_active_power;
    int32_t total_load_reactive_power;
    int32_t total_load_apparent_power;
} UiCtx_t;

static UiCtx_t _gstUiCtx;


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
//static TEXT_Handle  _gPressHandle;//Press句柄
//static TEXT_Handle  _gBackHandle;//return back句柄
//static IMAGE_Handle  _gImgReturnHandle;//图标句柄

static emWidgetTextPara_t  _gstTitleTextPara;
//static emWidgetTextPara_t  _gstPressTextPara;
//static emWidgetTextPara_t  _gstBackTextPara;
//static emWidgetImgPara_t  _gstReturnImgPara;

static char _garrTitleBuf[] = "Operational State Info < On-Grid Load";
//static char _garrPressBuf[] = "Press";
//static char _garrBackBuf[] = "return back to previous page.";


/*List控件*/
static emListPara_t _gstEmListNormalLoadPara;

static char _garrListNormalLoadLeftTextBuf[6][20];
static char _garrListNormalLoadRightTextBuf[6][20];


#define WIN_LIST_NORMAL_LOAD_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_NORMAL_LOAD_TITLE_ID                         (GUI_ID_USER + 0x13)
//#define WIN_LIST_NORMAL_LOAD_PRESS_ID                         (GUI_ID_USER + 0x14)
//#define WIN_LIST_NORMAL_LOAD_BACK_ID                          (GUI_ID_USER + 0x15)
//#define WIN_LIST_NORMAL_LOAD_BACK_IMG_ID                      (GUI_ID_USER + 0x16)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListGeneratorInputDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListNormalLoadInfoWin",    WIN_LIST_NORMAL_LOAD_ID,          0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_NORMAL_LOAD_TITLE_ID,              31,  20,  415,  14,   0,   0x0,  0 },
//    { TEXT_CreateIndirect,    "Press",          WIN_LIST_NORMAL_LOAD_PRESS_ID,              133, 273, 33,   14,   0,   0x0,  0 },
//    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_NORMAL_LOAD_BACK_IMG_ID,           171 - 5, 275, 15,   10,   0,   0x0,  0 },
//    { TEXT_CreateIndirect,    "Back",           WIN_LIST_NORMAL_LOAD_BACK_ID,               188, 273, 150,  14,   0,   0x0,  0 },
};

static  int32_t _SetLoadVoltage(int8_t ui_update, uint16_t is_single_phase, uint16_t r_volt, uint16_t s_volt, uint16_t t_volt)
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
        snprintf(_garrListNormalLoadRightTextBuf[0], sizeof(_garrListNormalLoadRightTextBuf[0]),
                 "%.1fV", voltage[0]);
    }
    else
    {
        snprintf(_garrListNormalLoadRightTextBuf[0], sizeof(_garrListNormalLoadRightTextBuf[0]),
                 "%.1f | %.1f | %.1f V", voltage[0], voltage[1], voltage[2]);
    }
    if (ui_update)
    {
        TEXT_SetText(_gstEmListNormalLoadPara.handle[1][0], (const char *)_gstEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}

static  int32_t _SetLoadCurrent(int8_t ui_update, uint16_t is_single_phase, uint16_t r_cur, uint16_t s_cur, uint16_t t_cur)
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
        snprintf(_garrListNormalLoadRightTextBuf[1], sizeof(_garrListNormalLoadRightTextBuf[1]),
                 "%.1fA", current[0]);
    }
    else
    {
        snprintf(_garrListNormalLoadRightTextBuf[1], sizeof(_garrListNormalLoadRightTextBuf[1]),
                 "%.1f | %.1f | %.1f A", current[0], current[1], current[2]);
    }

    if (ui_update)
    {
        TEXT_SetText(_gstEmListNormalLoadPara.handle[1][1], (const char *)_gstEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief   设置远端频率
  * @param   active_power 有功功率的真实值
  * @return  None
  * @note    None
  */
static  int32_t _SetLoadFrequency(int8_t ui_update, uint16_t frq_r)
{
    float frequency = (float)frq_r / 100;

    snprintf(_garrListNormalLoadRightTextBuf[2], sizeof(_garrListNormalLoadRightTextBuf[2]),
             "%.2fHz", frequency);
    
    if(ui_update)
    {
        TEXT_SetText(_gstEmListNormalLoadPara.handle[1][2],
                     (const char *)_gstEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief   设置负载有功功率
  * @param   active_power 有功功率的真实值
  * @return  None
  * @note    None
  */
static int32_t _SetLoadActivePower(int8_t ui_update, uint32_t r_power, uint32_t s_power, uint32_t t_power)
{
    int32_t power[3] = {(int32_t)r_power, (int32_t)s_power, (int32_t)t_power};
    int32_t total_power = power[0] + power[1] + power[2];

    snprintf(_garrListNormalLoadRightTextBuf[3], sizeof(_garrListNormalLoadRightTextBuf[3]),
             "%dW", total_power);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListNormalLoadPara.handle[1][3], (const char *)_gstEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief   设置负载无功功率
  * @param   无功功率的真实值
  * @return  None
  * @note    None
  */
static int32_t _SetLoadReactivePower(int8_t ui_update, uint32_t r_power, uint32_t s_power, uint32_t t_power)
{
    int32_t power[3] = {(int32_t)r_power, (int32_t)s_power, (int32_t)t_power};
    int32_t total_power = power[0] + power[1] + power[2];

    snprintf(_garrListNormalLoadRightTextBuf[4], sizeof(_garrListNormalLoadRightTextBuf[4]),
             "%dVar", total_power);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListNormalLoadPara.handle[1][4], (const char *)_gstEmListNormalLoadPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief   设置负载视在功率
  * @param   apparent_power 视在功率的真实值
  * @return  None
  * @note    None
  */
static  int32_t _SetLoadApparentPower(int8_t ui_update, uint32_t r_power, uint32_t s_power, uint32_t t_power)
{
    int32_t power[3] = {(int32_t)r_power, (int32_t)s_power, (int32_t)t_power};
    int32_t total_power = power[0] + power[1] + power[2];

    snprintf(_garrListNormalLoadRightTextBuf[5], sizeof(_garrListNormalLoadRightTextBuf[5]),
             "%dVA", total_power);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListNormalLoadPara.handle[1][5], (const char *)_gstEmListNormalLoadPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
    }
    return 0;
}

/**
  * @brief ListNormalLoadInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListNormalLoadInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_NORMAL_LOAD_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListNormalLoadPara, pMsg->hWin);

//            /*底部return back*/
//            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_NORMAL_LOAD_BACK_ID);
//            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
//            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
//            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
//            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
//            /*底部Press*/
//            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_NORMAL_LOAD_PRESS_ID);
//            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
//            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
//            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
//            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
//            /*底部图标*/
//            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_NORMAL_LOAD_BACK_IMG_ID);
//            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListNormalLoadPara, _gstEmListNormalLoadPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListNormalLoadPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListNormalLoadPara.sub_region[i].x_pos,
                                    _gstEmListNormalLoadPara.sub_region[i].y_pos,
                                    _gstEmListNormalLoadPara.sub_region[i].x_pos + _gstEmListNormalLoadPara.sub_region[i].x_size - 1,
                                    _gstEmListNormalLoadPara.sub_region[i].y_pos + _gstEmListNormalLoadPara.sub_region[i].y_size - 1,
                                    _gstEmListNormalLoadPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListNormalLoadPara, _gstEmListNormalLoadPara.list_sel);

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
    gstEmWinListNormalLoadInfo.win_handle = GUI_CreateDialogBox(_garrListGeneratorInputDialogCreate, GUI_COUNTOF(_garrListGeneratorInputDialogCreate), _cbListNormalLoadInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListNormalLoadInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListNormalLoadInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListNormalLoadInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListNormalLoadPara, gstEmWinListNormalLoadInfo.win_handle);
    return gstEmWinListNormalLoadInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListNormalLoadInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListNormalLoadInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListNormalLoadInfo.win_handle = 0;
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

#if 0
    _gstUiCtx.load_power = gstCollectParaData.collect_data.remot_load_power;    //监控卡端计算远端负载
#else
    _gstUiCtx.r_phase_load_volt = gstCollectInfo.power_data.power_value_info.r_nload_vol;
    _gstUiCtx.s_phase_load_volt = gstCollectInfo.power_data.power_value_info.s_nload_vol;
    _gstUiCtx.t_phase_load_volt = gstCollectInfo.power_data.power_value_info.t_nload_vol;
    _gstUiCtx.r_phase_load_curr = gstCollectInfo.power_data.power_value_info.r_nload_current;
    _gstUiCtx.s_phase_load_curr = gstCollectInfo.power_data.power_value_info.s_nload_current;
    _gstUiCtx.t_phase_load_curr = gstCollectInfo.power_data.power_value_info.t_nload_current;
    
    _gstUiCtx.r_phase_load_freq = gstCollectInfo.power_data.power_value_info.r_nload_freq;

    _gstUiCtx.r_load_active_power = gstCollectInfo.power_data.power_value_info.r_nload_act_power;
    _gstUiCtx.s_load_active_power = gstCollectInfo.power_data.power_value_info.s_nload_act_power;
    _gstUiCtx.t_load_active_power = gstCollectInfo.power_data.power_value_info.t_nload_act_power;

    _gstUiCtx.r_load_reactive_power = gstCollectInfo.power_data.power_value_info.r_nload_react_power;
    _gstUiCtx.s_load_reactive_power = gstCollectInfo.power_data.power_value_info.s_nload_react_power;
    _gstUiCtx.t_load_reactive_power = gstCollectInfo.power_data.power_value_info.t_nload_react_power;

    _gstUiCtx.r_load_apparent_power = gstCollectInfo.power_data.power_value_info.r_nload_apparent_power;
    _gstUiCtx.s_load_apparent_power = gstCollectInfo.power_data.power_value_info.s_nload_apparent_power;
    _gstUiCtx.t_load_apparent_power = gstCollectInfo.power_data.power_value_info.t_nload_apparent_power;

#endif

#if 0
    _gstUiCtx.load_volt = gstCollectParaData;
    _gstUiCtx.load_curr = gstCollectParaData;
#endif

    /*底部back的参数*/
//    _gstPressTextPara.p_content = _garrPressBuf;
//    _gstPressTextPara.text_color = 0x00EEEEEE;
//    _gstPressTextPara.p_text_font = EM_TEXT_FONT_1;

//    _gstBackTextPara.p_content = _garrBackBuf;
//    _gstBackTextPara.text_color = 0x00EEEEEE;
//    _gstBackTextPara.p_text_font = EM_TEXT_FONT_1;

//    _gstReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_BACK_ON_ICON_ARROW_SIZE);
//    _gstReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gstReturnImgPara.mem_img);
//    _gstReturnImgPara.img_size = EM_BACK_ON_ICON_ARROW_SIZE;
//    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);


    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListNormalLoadPara.list_start_id = WIN_LIST_NORMAL_LOAD_ID + 0x30;
    _gstEmListNormalLoadPara.list_num = 6;
    _gstEmListNormalLoadPara.list_display_num = 6;
    if(_gstEmListNormalLoadPara.list_num <= _gstEmListNormalLoadPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListNormalLoadPara.list_num + 1 - _gstEmListNormalLoadPara.list_display_num;
    }

    if(_gstEmListNormalLoadPara.list_sel > _gstEmListNormalLoadPara.list_num - 1)
    {
        _gstEmListNormalLoadPara.list_sel = 0;
    }
    if(_gstEmListNormalLoadPara.list_page > max_page - 1)
    {
        _gstEmListNormalLoadPara.list_page = 0;
    }
    _gstEmListNormalLoadPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _gstEmListNormalLoadPara.selected_r = 10;
    
    /*6个子列表*/
    for(int8_t i = 0; i < 6; i++)
    {
        _gstEmListNormalLoadPara.sub_region[i].x_pos = 20;
        _gstEmListNormalLoadPara.sub_region[i].y_pos = 38 + i * 42;
        _gstEmListNormalLoadPara.sub_region[i].x_size = 440;
        _gstEmListNormalLoadPara.sub_region[i].y_size = 34;
    }

    /*List控件*/
#if 1
    //第一行左边
    _gstEmListNormalLoadPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListNormalLoadPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[0][0].x_size = 158;
    _gstEmListNormalLoadPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListNormalLoadLeftTextBuf[0];
    _gstEmListNormalLoadPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListNormalLoadPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "On-Grid Voltage");
    //第一行右边
    _gstEmListNormalLoadPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[1][0].x_relative_pos = 220;
    _gstEmListNormalLoadPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[1][0].x_size = 208;
    _gstEmListNormalLoadPara.sub_widget_para[1][0].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListNormalLoadRightTextBuf[0];
    _gstEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 220;//测试数据，实际赋值系统数据
//    sprintf(_gstEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "%dV", (uint16_t)_gstEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.digit_value);
    _SetLoadVoltage(0, SINGLE_PHASE, _gstUiCtx.r_phase_load_volt, _gstUiCtx.s_phase_load_volt, _gstUiCtx.t_phase_load_volt);
    //第二行左边
    _gstEmListNormalLoadPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListNormalLoadPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[0][1].x_size = 158;
    _gstEmListNormalLoadPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListNormalLoadLeftTextBuf[1];
    _gstEmListNormalLoadPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListNormalLoadPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "On-Grid Current");
    //第二行右边
    _gstEmListNormalLoadPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[1][1].x_relative_pos = 220;
    _gstEmListNormalLoadPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[1][1].x_size = 208;
    _gstEmListNormalLoadPara.sub_widget_para[1][1].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListNormalLoadRightTextBuf[1];
    _gstEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
//    sprintf(_gstEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "%dA", (uint16_t)_gstEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.digit_value);
    _SetLoadCurrent(0, SINGLE_PHASE, _gstUiCtx.r_phase_load_curr, _gstUiCtx.s_phase_load_curr, _gstUiCtx.t_phase_load_curr);
#endif
    //第三行左边
    _gstEmListNormalLoadPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListNormalLoadPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[0][2].x_size = 158;
    _gstEmListNormalLoadPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListNormalLoadLeftTextBuf[2];
    _gstEmListNormalLoadPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListNormalLoadPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Frequency");
    //第三行右边
    _gstEmListNormalLoadPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[1][2].x_relative_pos = 220;
    _gstEmListNormalLoadPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[1][2].x_size = 208;
    _gstEmListNormalLoadPara.sub_widget_para[1][2].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListNormalLoadRightTextBuf[2];
    _gstEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
//    sprintf(_gstEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.p_content, "%dW", (uint16_t)_gstEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.digit_value);
    _SetLoadFrequency(0, _gstUiCtx.r_phase_load_freq);

    //第4行左边
    _gstEmListNormalLoadPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListNormalLoadPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[0][3].x_size = 158;
    _gstEmListNormalLoadPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListNormalLoadLeftTextBuf[3];
    _gstEmListNormalLoadPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListNormalLoadPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Active Power");
    //第4行右边
    _gstEmListNormalLoadPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[1][3].x_relative_pos = 220;
    _gstEmListNormalLoadPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[1][3].x_size = 208;
    _gstEmListNormalLoadPara.sub_widget_para[1][3].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _garrListNormalLoadRightTextBuf[3];
    _gstEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
//    sprintf(_gstEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.p_content, "%dW", (uint16_t)_gstEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.digit_value);
    _SetLoadActivePower(0, _gstUiCtx.r_load_active_power, _gstUiCtx.s_load_active_power, _gstUiCtx.t_load_active_power);

    //第5行左边
    _gstEmListNormalLoadPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListNormalLoadPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[0][4].x_size = 158;
    _gstEmListNormalLoadPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListNormalLoadLeftTextBuf[4];
    _gstEmListNormalLoadPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListNormalLoadPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Reactive Power");
    //第5行右边
    _gstEmListNormalLoadPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[1][4].x_relative_pos = 220;
    _gstEmListNormalLoadPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[1][4].x_size = 208;
    _gstEmListNormalLoadPara.sub_widget_para[1][4].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _garrListNormalLoadRightTextBuf[4];
    _gstEmListNormalLoadPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListNormalLoadPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
//    sprintf(_gstEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.p_content, "%dW", (uint16_t)_gstEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.digit_value);
    _SetLoadReactivePower(0, _gstUiCtx.r_load_reactive_power, _gstUiCtx.s_load_reactive_power, _gstUiCtx.t_load_reactive_power);
    
    //第6行左边
    _gstEmListNormalLoadPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gstEmListNormalLoadPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[0][5].x_size = 158;
    _gstEmListNormalLoadPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListNormalLoadLeftTextBuf[5];
    _gstEmListNormalLoadPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListNormalLoadPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Apparent Power");
    //第6行右边
    _gstEmListNormalLoadPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListNormalLoadPara.sub_widget_para[1][5].x_relative_pos = 220;
    _gstEmListNormalLoadPara.sub_widget_para[1][5].y_relative_pos = 10;
    _gstEmListNormalLoadPara.sub_widget_para[1][5].x_size = 208;
    _gstEmListNormalLoadPara.sub_widget_para[1][5].y_size = 17;
    _gstEmListNormalLoadPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _garrListNormalLoadRightTextBuf[5];
    _gstEmListNormalLoadPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListNormalLoadPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListNormalLoadPara.sub_widget_para[1][5].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
//    sprintf(_gstEmListNormalLoadPara.sub_widget_para[1][5].sub_widget.text_para.p_content, "%dW", (uint16_t)_gstEmListNormalLoadPara.sub_widget_para[1][5].sub_widget.text_para.digit_value);
    _SetLoadApparentPower(0, _gstUiCtx.r_load_apparent_power, _gstUiCtx.s_load_apparent_power, _gstUiCtx.t_load_apparent_power);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
//    GUI_ALLOC_Free(_gstReturnImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    if(_gstUiCtx.r_phase_load_volt != gstCollectInfo.power_data.power_value_info.r_nload_vol ||
       _gstUiCtx.s_phase_load_volt != gstCollectInfo.power_data.power_value_info.s_nload_vol ||
       _gstUiCtx.t_phase_load_volt != gstCollectInfo.power_data.power_value_info.t_nload_vol)
    {
        _gstUiCtx.r_phase_load_volt = gstCollectInfo.power_data.power_value_info.r_nload_vol;
        _gstUiCtx.s_phase_load_volt = gstCollectInfo.power_data.power_value_info.s_nload_vol;
        _gstUiCtx.t_phase_load_volt = gstCollectInfo.power_data.power_value_info.t_nload_vol;
        _SetLoadVoltage(1, SINGLE_PHASE, _gstUiCtx.r_phase_load_volt, _gstUiCtx.s_phase_load_volt, _gstUiCtx.t_phase_load_volt);
    }

    if(_gstUiCtx.r_phase_load_curr != gstCollectInfo.power_data.power_value_info.r_nload_current ||
       _gstUiCtx.s_phase_load_curr != gstCollectInfo.power_data.power_value_info.s_nload_current ||
       _gstUiCtx.t_phase_load_curr != gstCollectInfo.power_data.power_value_info.t_nload_current)
    {
        _gstUiCtx.r_phase_load_curr = gstCollectInfo.power_data.power_value_info.r_nload_current;
        _gstUiCtx.s_phase_load_curr = gstCollectInfo.power_data.power_value_info.s_nload_current;
        _gstUiCtx.t_phase_load_curr = gstCollectInfo.power_data.power_value_info.t_nload_current;
        _SetLoadCurrent(1, SINGLE_PHASE, _gstUiCtx.r_phase_load_curr, _gstUiCtx.s_phase_load_curr, _gstUiCtx.t_phase_load_curr);
    }
    
    if(_gstUiCtx.r_phase_load_freq != gstCollectInfo.power_data.power_value_info.r_nload_freq)
    {
        _gstUiCtx.r_phase_load_freq = gstCollectInfo.power_data.power_value_info.r_nload_freq;
        _SetLoadFrequency(1, _gstUiCtx.r_phase_load_freq);
    }

    if(_gstUiCtx.r_load_active_power != gstCollectInfo.power_data.power_value_info.r_nload_act_power ||
       _gstUiCtx.s_load_active_power != gstCollectInfo.power_data.power_value_info.s_nload_act_power ||
       _gstUiCtx.t_load_active_power != gstCollectInfo.power_data.power_value_info.t_nload_act_power)
    {
        _gstUiCtx.r_load_active_power = gstCollectInfo.power_data.power_value_info.r_nload_act_power;
        _gstUiCtx.s_load_active_power = gstCollectInfo.power_data.power_value_info.s_nload_act_power;
        _gstUiCtx.t_load_active_power = gstCollectInfo.power_data.power_value_info.t_nload_act_power;
        _SetLoadActivePower(1, _gstUiCtx.r_load_active_power, _gstUiCtx.s_load_active_power, _gstUiCtx.t_load_active_power);
    }
    if(_gstUiCtx.r_load_reactive_power != gstCollectInfo.power_data.power_value_info.r_nload_react_power ||
       _gstUiCtx.s_load_reactive_power != gstCollectInfo.power_data.power_value_info.s_nload_react_power ||
       _gstUiCtx.t_load_reactive_power != gstCollectInfo.power_data.power_value_info.t_nload_react_power)
    {
        _gstUiCtx.r_load_reactive_power = gstCollectInfo.power_data.power_value_info.r_nload_react_power;
        _gstUiCtx.s_load_reactive_power = gstCollectInfo.power_data.power_value_info.s_nload_react_power;
        _gstUiCtx.t_load_reactive_power = gstCollectInfo.power_data.power_value_info.t_nload_react_power;
        _SetLoadReactivePower(1, _gstUiCtx.r_load_reactive_power, _gstUiCtx.s_load_reactive_power, _gstUiCtx.t_load_reactive_power);
    }

    if(_gstUiCtx.r_load_apparent_power != gstCollectInfo.power_data.power_value_info.r_nload_apparent_power ||
       _gstUiCtx.s_load_apparent_power != gstCollectInfo.power_data.power_value_info.s_nload_apparent_power ||
       _gstUiCtx.t_load_apparent_power != gstCollectInfo.power_data.power_value_info.t_nload_apparent_power)
    {
        _gstUiCtx.r_load_apparent_power = gstCollectInfo.power_data.power_value_info.r_nload_apparent_power;
        _gstUiCtx.s_load_apparent_power = gstCollectInfo.power_data.power_value_info.s_nload_apparent_power;
        _gstUiCtx.t_load_apparent_power = gstCollectInfo.power_data.power_value_info.t_nload_apparent_power;
        _SetLoadApparentPower(1, _gstUiCtx.r_load_apparent_power, _gstUiCtx.s_load_apparent_power, _gstUiCtx.t_load_apparent_power);
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

