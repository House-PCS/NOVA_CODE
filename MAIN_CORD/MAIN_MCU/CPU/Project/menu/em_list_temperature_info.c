/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-12
  * @brief       温度信息
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

#define  EM_BACK_ON_ICON_ARROW_SIZE                     (4096)
#define  EM_LIST_TEMP_ICON_ARROW_SIZE                   (4096)

#define  TEST_DEBUG_TMP     0

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinListTemperatureInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t inverter_radiator_temp;
    uint16_t chassis_internal_temp;
    uint16_t battery_temp;
    uint16_t bat_num;
} UiCtx_t;

static UiCtx_t _gstUiCtx;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄
static IMAGE_Handle  _gImgTempratureHandle;//温度图标句柄

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;
static emWidgetImgPara_t  _gTemperatureImgPara[3];  /*0:cold    1:heat  2:normal*/


static char _garrTitleBuf[] = "Operational State Info < Temperature";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";


/*List控件*/
static emListPara_t  _gEmListTemperatureInfoPara;

static char _gListTemperatureInfoLeftTextBuf[6][20];
static char _gListTemperatureInfoRightTextBuf[6][20];


static char _gtemperture[3] = {0xC2,0xB0,0x43};

#define WIN_LIST_TEMPERATURE_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_TEMPERATURE_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_TEMPERATURE_INFO_PRESS_ID                         (GUI_ID_USER + 0x14)
#define WIN_LIST_TEMPERATURE_INFO_BACK_ID                          (GUI_ID_USER + 0x15)
#define WIN_LIST_TEMPERATURE_INFO_BACK_IMG_ID                      (GUI_ID_USER + 0x16)

static __SDRAM GUI_WIDGET_CREATE_INFO _aListGeneratorInputDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListTemperatureInfoWin",    WIN_LIST_TEMPERATURE_INFO_ID,            0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_TEMPERATURE_INFO_TITLE_ID,             31,  20,  415,  14,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_TEMPERATURE_INFO_PRESS_ID,             133, 273, 33,   14,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_TEMPERATURE_INFO_BACK_IMG_ID,          171 - 5, 275, 15,   10,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_TEMPERATURE_INFO_BACK_ID,              188, 273, 150,  14,   0,   0x0,  0 },
};

static uint32_t _SetInverterRadiatorTemp(int8_t ui_update, uint16_t temp)
{
    int16_t __temp = (int16_t)temp;
    float _temp = (float)__temp / 10;
    snprintf(_gListTemperatureInfoRightTextBuf[0], sizeof(_gListTemperatureInfoRightTextBuf[0]), "%.1f", _temp);
    strcat(_gListTemperatureInfoRightTextBuf[0], _gtemperture);

    if (!ui_update)
    {
        return 0;
    }

    _gEmListTemperatureInfoPara.sub_widget_para[2][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gTemperatureImgPara[2].mem_img;
    _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[2].p_img_buf;

    if (_temp < 10)
    {
        _gEmListTemperatureInfoPara.sub_widget_para[2][0].sub_widget.text_para.text_color = 0x00FFC44C;
        _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gTemperatureImgPara[0].mem_img;
        _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[0].p_img_buf;
    }
    else if (_temp > 50)
    {
        _gEmListTemperatureInfoPara.sub_widget_para[2][0].sub_widget.text_para.text_color = 0x006A62FF;
        _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gTemperatureImgPara[1].mem_img;
        _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[1].p_img_buf;
    }

    IMAGE_SetDTA(_gEmListTemperatureInfoPara.handle[1][0], _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf, _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size);
    TEXT_SetTextColor(_gEmListTemperatureInfoPara.handle[2][0], _gEmListTemperatureInfoPara.sub_widget_para[2][0].sub_widget.text_para.text_color);
    TEXT_SetText(_gEmListTemperatureInfoPara.handle[2][0], (const char *)_gEmListTemperatureInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content);
    return 0;
}

static uint32_t _SetInternalEnviromentTemp(int8_t ui_update, uint16_t temp)
{
    int16_t __temp = (int16_t)temp;
    float _temp = (float)__temp / 10;
    snprintf(_gListTemperatureInfoRightTextBuf[1], sizeof(_gListTemperatureInfoRightTextBuf[1]), "%.1f", _temp);
    strcat(_gListTemperatureInfoRightTextBuf[1], _gtemperture);

    if (!ui_update)
    {
        return 0;
    }

    _gEmListTemperatureInfoPara.sub_widget_para[2][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gTemperatureImgPara[2].mem_img;
    _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[2].p_img_buf;

    if (_temp < 10)
    {
        _gEmListTemperatureInfoPara.sub_widget_para[2][1].sub_widget.text_para.text_color = 0x00FFC44C;
        _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gTemperatureImgPara[0].mem_img;
        _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[0].p_img_buf;
    }
    else if (_temp > 50)
    {
        _gEmListTemperatureInfoPara.sub_widget_para[2][1].sub_widget.text_para.text_color = 0x006A62FF;
        _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gTemperatureImgPara[1].mem_img;
        _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[1].p_img_buf;
    }

    IMAGE_SetDTA(_gEmListTemperatureInfoPara.handle[1][1], _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf, _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size);
    TEXT_SetTextColor(_gEmListTemperatureInfoPara.handle[2][1], _gEmListTemperatureInfoPara.sub_widget_para[2][1].sub_widget.text_para.text_color);
    TEXT_SetText(_gEmListTemperatureInfoPara.handle[2][1], (const char *)_gEmListTemperatureInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content);
    return 0;
}

static uint32_t _SetBatteryTemp(int8_t ui_update, uint16_t temp)
{
    int16_t __temp = (int16_t)temp;
    float _temp = (float)__temp / 10;
    snprintf(_gListTemperatureInfoRightTextBuf[2], sizeof(_gListTemperatureInfoRightTextBuf[2]), "%.1f", _temp);
    strcat(_gListTemperatureInfoRightTextBuf[2], _gtemperture);

    if (!ui_update)
    {
        return 0;
    }

    _gEmListTemperatureInfoPara.sub_widget_para[2][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gTemperatureImgPara[2].mem_img;
    _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[2].p_img_buf;

    if (_temp < 10)
    {
        _gEmListTemperatureInfoPara.sub_widget_para[2][2].sub_widget.text_para.text_color = 0x00FFC44C;
        _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gTemperatureImgPara[0].mem_img;
        _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[0].p_img_buf;
    }
    else if (_temp > 50)
    {
        _gEmListTemperatureInfoPara.sub_widget_para[2][2].sub_widget.text_para.text_color = 0x006A62FF;
        _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gTemperatureImgPara[1].mem_img;
        _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[1].p_img_buf;
    }

    IMAGE_SetDTA(_gEmListTemperatureInfoPara.handle[1][2], _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf, _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size);
    TEXT_SetTextColor(_gEmListTemperatureInfoPara.handle[2][2], _gEmListTemperatureInfoPara.sub_widget_para[2][2].sub_widget.text_para.text_color);
    TEXT_SetText(_gEmListTemperatureInfoPara.handle[2][2], (const char *)_gEmListTemperatureInfoPara.sub_widget_para[2][2].sub_widget.text_para.p_content);
    return 0;
}

/**
  * @brief ListTemperatureInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListTemperatureInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gEmListTemperatureInfoPara, pMsg->hWin);

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListTemperatureInfoPara, _gEmListTemperatureInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(int i = 0; i < _gEmListTemperatureInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListTemperatureInfoPara.sub_region[i].x_pos,
                                    _gEmListTemperatureInfoPara.sub_region[i].y_pos,
                                    _gEmListTemperatureInfoPara.sub_region[i].x_pos + _gEmListTemperatureInfoPara.sub_region[i].x_size - 1,
                                    _gEmListTemperatureInfoPara.sub_region[i].y_pos + _gEmListTemperatureInfoPara.sub_region[i].y_size - 1,
                                    _gEmListTemperatureInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListTemperatureInfoPara, _gEmListTemperatureInfoPara.list_sel);

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
    gstEmWinListTemperatureInfo.win_handle = GUI_CreateDialogBox(_aListGeneratorInputDialogCreate, GUI_COUNTOF(_aListGeneratorInputDialogCreate), _cbListTemperatureInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListTemperatureInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListTemperatureInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListTemperatureInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListTemperatureInfoPara, gstEmWinListTemperatureInfo.win_handle);
    return gstEmWinListTemperatureInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListTemperatureInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListTemperatureInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListTemperatureInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t i = 0;
    _gstUiCtx.bat_num = GetBmsNum();
    if(_gstUiCtx.bat_num > BMS_MAX_NUM)
    {
        _gstUiCtx.bat_num = BMS_MAX_NUM;
    }
    _gstUiCtx.inverter_radiator_temp = gstCollectInfo.power_data.model_info.inv_radiator_temp;
    _gstUiCtx.chassis_internal_temp  = gstCollectInfo.power_data.model_info.inv_box_temp;
    _gstUiCtx.battery_temp = gstCollectInfo.pack_data[0].state_info.cell_ave_temp;
    
    for(i = 0; i < _gstUiCtx.bat_num; i++)
    {
        if(_gstUiCtx.battery_temp < gstCollectInfo.pack_data[i].state_info.cell_ave_temp)
        {
            _gstUiCtx.battery_temp = gstCollectInfo.pack_data[i].state_info.cell_ave_temp;
        }
    }
    
#if TEST_DEBUG_TMP
    _gstUiCtx.inverter_radiator_temp = 234;
    _gstUiCtx.chassis_internal_temp = 23;
    _gstUiCtx.battery_temp = 645;
#endif

    EmKeyDefaultInfoSet();
    uint8_t max_page;

    /*底部back的参数*/
    _gstPressTextPara.p_content = _garrPressBuf;
    _gstPressTextPara.text_color = 0x00EEEEEE;
    _gstPressTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstBackTextPara.p_content = _garrBackBuf;
    _gstBackTextPara.text_color = 0x00EEEEEE;
    _gstBackTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_BACK_ON_ICON_ARROW_SIZE);
    _gstReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gstReturnImgPara.mem_img);
    _gstReturnImgPara.img_size = EM_BACK_ON_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);
    /*温度图标的参数*/
    _gTemperatureImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_TEMP_ICON_ARROW_SIZE);
    _gTemperatureImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gTemperatureImgPara[0].mem_img);
    _gTemperatureImgPara[0].img_size = EM_LIST_TEMP_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_TEMP_COLD_ADDR, _gTemperatureImgPara[0].p_img_buf, _gTemperatureImgPara[0].img_size);

    _gTemperatureImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_TEMP_ICON_ARROW_SIZE);
    _gTemperatureImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gTemperatureImgPara[1].mem_img);
    _gTemperatureImgPara[1].img_size = EM_LIST_TEMP_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_TEMP_HEAT_ADDR, _gTemperatureImgPara[1].p_img_buf, _gTemperatureImgPara[1].img_size);

    _gTemperatureImgPara[2].mem_img = GUI_ALLOC_AllocZero(EM_LIST_TEMP_ICON_ARROW_SIZE);
    _gTemperatureImgPara[2].p_img_buf = GUI_ALLOC_h2p(_gTemperatureImgPara[2].mem_img);
    _gTemperatureImgPara[2].img_size = EM_LIST_TEMP_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_TEMP_NORMAL_ADDR, _gTemperatureImgPara[2].p_img_buf, _gTemperatureImgPara[2].img_size);

    /*标题*/
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gEmListTemperatureInfoPara.list_start_id = WIN_LIST_TEMPERATURE_INFO_ID + 0x30;
    _gEmListTemperatureInfoPara.list_num = 3;
    _gEmListTemperatureInfoPara.list_display_num = 6;
    if(_gEmListTemperatureInfoPara.list_num <= _gEmListTemperatureInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListTemperatureInfoPara.list_num + 1 - _gEmListTemperatureInfoPara.list_display_num;
    }

    if(_gEmListTemperatureInfoPara.list_sel > _gEmListTemperatureInfoPara.list_num - 1)
    {
        _gEmListTemperatureInfoPara.list_sel = 0;
    }
    if(_gEmListTemperatureInfoPara.list_page > max_page - 1)
    {
        _gEmListTemperatureInfoPara.list_page = 0;
    }
    _gEmListTemperatureInfoPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _gEmListTemperatureInfoPara.selected_r = 10;

    /*4个子列表*/
    _gEmListTemperatureInfoPara.sub_region[0].x_pos = 20;
    _gEmListTemperatureInfoPara.sub_region[0].y_pos = 38;
    _gEmListTemperatureInfoPara.sub_region[0].x_size = 440;
    _gEmListTemperatureInfoPara.sub_region[0].y_size = 34;
    _gEmListTemperatureInfoPara.sub_region[1].x_pos = 20;
    _gEmListTemperatureInfoPara.sub_region[1].y_pos = 80;
    _gEmListTemperatureInfoPara.sub_region[1].x_size = 440;
    _gEmListTemperatureInfoPara.sub_region[1].y_size = 34;
    _gEmListTemperatureInfoPara.sub_region[2].x_pos = 20;
    _gEmListTemperatureInfoPara.sub_region[2].y_pos = 122;
    _gEmListTemperatureInfoPara.sub_region[2].x_size = 440;
    _gEmListTemperatureInfoPara.sub_region[2].y_size = 34;

    /*List控件*/
    //第一行左边
    _gEmListTemperatureInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListTemperatureInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListTemperatureInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListTemperatureInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListTemperatureInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListTemperatureInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListTemperatureInfoLeftTextBuf[0];
    _gEmListTemperatureInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListTemperatureInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListTemperatureInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Inverter Heatsink");
    //第一行中间图标
    _gEmListTemperatureInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListTemperatureInfoPara.sub_widget_para[1][0].x_relative_pos = 356;
    _gEmListTemperatureInfoPara.sub_widget_para[1][0].y_relative_pos = 7;
    _gEmListTemperatureInfoPara.sub_widget_para[1][0].x_size = 18;
    _gEmListTemperatureInfoPara.sub_widget_para[1][0].y_size = 16;
    _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gTemperatureImgPara[2].mem_img;
    _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[2].p_img_buf;
    _gEmListTemperatureInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_TEMP_ICON_ARROW_SIZE;
    //第一行右边
    _gEmListTemperatureInfoPara.sub_widget_para[2][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListTemperatureInfoPara.sub_widget_para[2][0].x_relative_pos = 218;
    _gEmListTemperatureInfoPara.sub_widget_para[2][0].y_relative_pos = 10;
    _gEmListTemperatureInfoPara.sub_widget_para[2][0].x_size = 208;
    _gEmListTemperatureInfoPara.sub_widget_para[2][0].y_size = 17;
    _gEmListTemperatureInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content = _gListTemperatureInfoRightTextBuf[0];
    _gEmListTemperatureInfoPara.sub_widget_para[2][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListTemperatureInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListTemperatureInfoPara.sub_widget_para[2][0].sub_widget.text_para.digit_value = 114;//测试数据，实际赋值系统数据
    _SetInverterRadiatorTemp(0, _gstUiCtx.inverter_radiator_temp);
    //第二行左边
    _gEmListTemperatureInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListTemperatureInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListTemperatureInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListTemperatureInfoPara.sub_widget_para[0][1].x_size = 170;
    _gEmListTemperatureInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListTemperatureInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListTemperatureInfoLeftTextBuf[1];
    _gEmListTemperatureInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListTemperatureInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListTemperatureInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Internal Temp");
    //第二行中间图标
    _gEmListTemperatureInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListTemperatureInfoPara.sub_widget_para[1][1].x_relative_pos = 356;
    _gEmListTemperatureInfoPara.sub_widget_para[1][1].y_relative_pos = 7;
    _gEmListTemperatureInfoPara.sub_widget_para[1][1].x_size = 18;
    _gEmListTemperatureInfoPara.sub_widget_para[1][1].y_size = 16;
    _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gTemperatureImgPara[2].mem_img;
    _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[2].p_img_buf;
    _gEmListTemperatureInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_TEMP_ICON_ARROW_SIZE;
    //第二行右边
    _gEmListTemperatureInfoPara.sub_widget_para[2][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListTemperatureInfoPara.sub_widget_para[2][1].x_relative_pos = 218;
    _gEmListTemperatureInfoPara.sub_widget_para[2][1].y_relative_pos = 10;
    _gEmListTemperatureInfoPara.sub_widget_para[2][1].x_size = 208;
    _gEmListTemperatureInfoPara.sub_widget_para[2][1].y_size = 17;
    _gEmListTemperatureInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content = _gListTemperatureInfoRightTextBuf[1];
    _gEmListTemperatureInfoPara.sub_widget_para[2][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListTemperatureInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListTemperatureInfoPara.sub_widget_para[2][1].sub_widget.text_para.digit_value = 124;//测试数据，实际赋值系统数据
    _SetInternalEnviromentTemp(0, _gstUiCtx.chassis_internal_temp);
    //第三行左边
    _gEmListTemperatureInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListTemperatureInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListTemperatureInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListTemperatureInfoPara.sub_widget_para[0][2].x_size = 158;
    _gEmListTemperatureInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListTemperatureInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListTemperatureInfoLeftTextBuf[2];
    _gEmListTemperatureInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListTemperatureInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListTemperatureInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Battery Temp");
    //第三行中间图标
    _gEmListTemperatureInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListTemperatureInfoPara.sub_widget_para[1][2].x_relative_pos = 356;
    _gEmListTemperatureInfoPara.sub_widget_para[1][2].y_relative_pos = 7;
    _gEmListTemperatureInfoPara.sub_widget_para[1][2].x_size = 6;
    _gEmListTemperatureInfoPara.sub_widget_para[1][2].y_size = 16;
    _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gTemperatureImgPara[2].mem_img;
    _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _gTemperatureImgPara[2].p_img_buf;
    _gEmListTemperatureInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_TEMP_ICON_ARROW_SIZE;
    //第三行右边
    _gEmListTemperatureInfoPara.sub_widget_para[2][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListTemperatureInfoPara.sub_widget_para[2][2].x_relative_pos = 218;
    _gEmListTemperatureInfoPara.sub_widget_para[2][2].y_relative_pos = 10;
    _gEmListTemperatureInfoPara.sub_widget_para[2][2].x_size = 208;
    _gEmListTemperatureInfoPara.sub_widget_para[2][2].y_size = 17;
    _gEmListTemperatureInfoPara.sub_widget_para[2][2].sub_widget.text_para.p_content = _gListTemperatureInfoRightTextBuf[2];
    _gEmListTemperatureInfoPara.sub_widget_para[2][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListTemperatureInfoPara.sub_widget_para[2][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListTemperatureInfoPara.sub_widget_para[2][2].sub_widget.text_para.digit_value = 120;//测试数据，实际赋值系统数据
    _SetBatteryTemp(0, _gstUiCtx.battery_temp);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{

    GUI_ALLOC_Free(_gstReturnImgPara.mem_img);
    GUI_ALLOC_Free(_gTemperatureImgPara[0].mem_img);
    GUI_ALLOC_Free(_gTemperatureImgPara[1].mem_img);
    GUI_ALLOC_Free(_gTemperatureImgPara[2].mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
#if TEST_DEBUG_TMP
    _gstUiCtx.inverter_radiator_temp = 0;
    _gstUiCtx.chassis_internal_temp = 0;
    _gstUiCtx.battery_temp = 0;
#endif
    uint8_t i = 0;
    uint16_t cell_temp = gstCollectInfo.pack_data[0].state_info.cell_ave_temp;
    for(i = 0; i < _gstUiCtx.bat_num; i++)
    {
        if(cell_temp < gstCollectInfo.pack_data[i].state_info.cell_ave_temp)
        {
            cell_temp = gstCollectInfo.pack_data[i].state_info.cell_ave_temp;
        }
    }
    
    if(_gstUiCtx.inverter_radiator_temp != gstCollectInfo.power_data.model_info.inv_radiator_temp)
    {
        _gstUiCtx.inverter_radiator_temp = gstCollectInfo.power_data.model_info.inv_radiator_temp;
//        _gstUiCtx.inverter_radiator_temp = 90;
        _SetInverterRadiatorTemp(1, _gstUiCtx.inverter_radiator_temp);
    }

    if(_gstUiCtx.chassis_internal_temp != gstCollectInfo.power_data.model_info.inv_box_temp)
    {
        _gstUiCtx.chassis_internal_temp = gstCollectInfo.power_data.model_info.inv_box_temp;
//        _gstUiCtx.chassis_internal_temp = 200;
        _SetInternalEnviromentTemp(1, _gstUiCtx.chassis_internal_temp);
    }

    if(_gstUiCtx.battery_temp != cell_temp)
    {
        _gstUiCtx.battery_temp = cell_temp;
//        _gstUiCtx.battery_temp = 600;
        _SetBatteryTemp(1, _gstUiCtx.battery_temp);
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

