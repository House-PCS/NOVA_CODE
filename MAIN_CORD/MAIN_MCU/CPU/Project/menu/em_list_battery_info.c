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


#define  EM_LIST_BATTERY_INFO_ICON_ARROW_SIZE                (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


uint16_t gPackInfoIdx;

emWin_t gstEmWinListBatteryInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t     select;
    int32_t     key_init;
    uint16_t    batt_volt;
    uint16_t    batt_power;
    uint16_t    batt_current;
    uint32_t    rated_energy;
    uint16_t    batt_soh;
    uint16_t    batt_soc;
    
    uint16_t    bms_num;
} UiCtx_t;

static UiCtx_t _gstUiCtx;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

static emWidgetTextPara_t  _gstTitleTextPara;

static char _garrTitleBuf[] = "Operational State Info < Battery Pack";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";


/*List控件*/
static emListPara_t  _gstEmListBatteryInfoPara;
static GUI_HMEM _gListBatteryInfoRightMem;
static uint8_t *_pListBatteryInfoRightBuf;


static char _garrListBatteryInfoLeftTextBuf[6][20];
static char _garrListBatteryInfoRightTextBuf[6][20];


#define WIN_LIST_BATTERY_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_BATTERY_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListGeneratorInputDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListBatteryInfoWin",    WIN_LIST_BATTERY_INFO_ID,            0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_BATTERY_INFO_TITLE_ID,             31,  20,  415,  14,   0,   0x0,  0 },
};

static int32_t _SetBatteryVoltage(int8_t ui_update, uint16_t vol)
{
    int16_t vol_tmp = 0;
    vol_tmp = (int16_t)vol;
    float voltage = (float)vol_tmp / 10;
    snprintf(_garrListBatteryInfoRightTextBuf[0], sizeof(_garrListBatteryInfoRightTextBuf[0]), "%.1fV", voltage);
    if (ui_update)
    {
        TEXT_SetText(_gstEmListBatteryInfoPara.handle[1][0], (const char *)_gstEmListBatteryInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetBatteryCurrent(int8_t ui_update, uint16_t cur)
{
    int16_t cur_value = (int16_t)cur;
    float current = (float)cur_value / 10;
    snprintf(_garrListBatteryInfoRightTextBuf[1], sizeof(_garrListBatteryInfoRightTextBuf[1]), "%.1fA", current);
    if (ui_update)
    {
        TEXT_SetText(_gstEmListBatteryInfoPara.handle[1][1], (const char *)_gstEmListBatteryInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetBatteryPower(int8_t ui_update, uint16_t power)
{
    int16_t bat_power = (int16_t)power;

    snprintf(_garrListBatteryInfoRightTextBuf[2], sizeof(_garrListBatteryInfoRightTextBuf[2]), "%dW", bat_power);

    if(ui_update)
    {    
        TEXT_SetText(_gstEmListBatteryInfoPara.handle[1][2], (const char *)_gstEmListBatteryInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }
    return 0;
}
//额定能量
static int32_t _SetBatteryRatedEnergy(int8_t ui_update, uint32_t rate_energy)
{
    int32_t cap_tmp = 0;
    cap_tmp = (int32_t)rate_energy;
    float capacity_value = (float)cap_tmp;   //已在外围计算除十
    
    if(capacity_value >= 0.0f && capacity_value < 1000.0f)
    {
        snprintf(_garrListBatteryInfoRightTextBuf[3], sizeof(_garrListBatteryInfoRightTextBuf[3]), "%.1fWh", capacity_value);
    }
    else if(capacity_value >= 1000.0f)
    {
        capacity_value /= 1000;
        snprintf(_garrListBatteryInfoRightTextBuf[3], sizeof(_garrListBatteryInfoRightTextBuf[3]), "%.2fkWh", capacity_value);
    }

    if (ui_update)
    {
        TEXT_SetText(_gstEmListBatteryInfoPara.handle[1][3], (const char *)_gstEmListBatteryInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetBatterySOH(int8_t ui_update, uint16_t soh)
{
    float soh_value = (float)soh / 10;
    snprintf(_garrListBatteryInfoRightTextBuf[4], sizeof(_garrListBatteryInfoRightTextBuf[4]), "%.1f%%", soh_value);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListBatteryInfoPara.handle[1][4], (const char *)_gstEmListBatteryInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetBatterySOC(int8_t ui_update, uint16_t soc)
{
    float soc_value = 0.0f;
    int16_t temp = 0;
    
    temp = (int16_t)soc;
    soc_value = (float)temp / 10;
    
    snprintf(_garrListBatteryInfoRightTextBuf[5], sizeof(_garrListBatteryInfoRightTextBuf[5]), "%.1f%%", soc_value);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListBatteryInfoPara.handle[1][5], (const char *)_gstEmListBatteryInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief ListBatteryInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListBatteryInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BATTERY_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListBatteryInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListBatteryInfoPara, _gstEmListBatteryInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListBatteryInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListBatteryInfoPara.sub_region[i].x_pos,
                                    _gstEmListBatteryInfoPara.sub_region[i].y_pos,
                                    _gstEmListBatteryInfoPara.sub_region[i].x_pos + _gstEmListBatteryInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListBatteryInfoPara.sub_region[i].y_pos + _gstEmListBatteryInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListBatteryInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListBatteryInfoPara, _gstEmListBatteryInfoPara.list_sel);

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
    gstEmWinListBatteryInfo.win_handle = GUI_CreateDialogBox(_garrListGeneratorInputDialogCreate, GUI_COUNTOF(_garrListGeneratorInputDialogCreate), _cbListBatteryInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListBatteryInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListBatteryInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListBatteryInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListBatteryInfoPara, gstEmWinListBatteryInfo.win_handle);
    return gstEmWinListBatteryInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListBatteryInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListBatteryInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListBatteryInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page, i, j;
    EmKeyDefaultInfoSet();
    uint8_t widget_num = 0;
    
    _gstUiCtx.bms_num = GetBmsNum();
    
#if 0 
    uint8_t tmp_num = 0;
    _gstUiCtx.bms_num = tmp_num;

#endif     
    
    if(_gstUiCtx.bms_num > BMS_MAX_NUM)
    {
        _gstUiCtx.bms_num = BMS_MAX_NUM;
    }

    _gstTitleTextPara.p_content   = _garrTitleBuf;
    _gstTitleTextPara.text_color  = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListBatteryInfoPara.list_start_id = WIN_LIST_BATTERY_INFO_ID + 0x30;
    _gstEmListBatteryInfoPara.list_num = _gstUiCtx.bms_num;
    _gstEmListBatteryInfoPara.list_display_num = 6;
    if(_gstEmListBatteryInfoPara.list_num <= _gstEmListBatteryInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListBatteryInfoPara.list_num + 1 - _gstEmListBatteryInfoPara.list_display_num;
    }

    if(_gstEmListBatteryInfoPara.list_sel > _gstEmListBatteryInfoPara.list_num - 1)
    {
        _gstEmListBatteryInfoPara.list_sel = 0;
    }
    if(_gstEmListBatteryInfoPara.list_page > max_page - 1)
    {
        _gstEmListBatteryInfoPara.list_page = 0;
    }
    _gstEmListBatteryInfoPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _gstEmListBatteryInfoPara.selected_r = 10;
    
    
    /*List控件*/
    _gListBatteryInfoRightMem = GUI_ALLOC_AllocZero(EM_LIST_BATTERY_INFO_ICON_ARROW_SIZE);
    _pListBatteryInfoRightBuf = GUI_ALLOC_h2p(_gListBatteryInfoRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListBatteryInfoRightBuf, EM_LIST_BATTERY_INFO_ICON_ARROW_SIZE);
    

    /*4个子列表*/
    //每次初始化之前将列表先清零 防止出现之后框没有版本的问题
    for(i = 0; i < BMS_MAX_NUM * 2; i++)
    {
        _gstEmListBatteryInfoPara.sub_region[i].x_pos = 0;
        _gstEmListBatteryInfoPara.sub_region[i].y_pos = 0;
        _gstEmListBatteryInfoPara.sub_region[i].x_size = 0;
        _gstEmListBatteryInfoPara.sub_region[i].y_size = 0;
    }
    
    
        
     /*List选中框*/
    _gstEmListBatteryInfoPara.selected_unvalid_color = 0xFF000000;
    _gstEmListBatteryInfoPara.selected_valid_color = 0x00D6E400;
    _gstEmListBatteryInfoPara.selected_size = 3;
    _gstEmListBatteryInfoPara.selected_r = 10;
    _gstEmListBatteryInfoPara.selected_x_start_offset = -2;
    _gstEmListBatteryInfoPara.selected_y_start_offset = -2;
    _gstEmListBatteryInfoPara.selected_x_end_offset = 2;
    _gstEmListBatteryInfoPara.selected_y_end_offset = 2;
//    _gstEmListBatteryInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListBatteryInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
    if(_gstUiCtx.bms_num > 6)
    {
        /*滚动条*/
        _gstEmListBatteryInfoPara.scrollbar_enable = true;
        _gstEmListBatteryInfoPara.scrollbar_color = 0x00CDF100;
        _gstEmListBatteryInfoPara.scrollbar_bk_color = 0x00565656;
        _gstEmListBatteryInfoPara.scrollbar_x_pos = 457;
        _gstEmListBatteryInfoPara.scrollbar_y_pos = 39;
        _gstEmListBatteryInfoPara.scrollbar_x_size = 6;
        _gstEmListBatteryInfoPara.scrollbar_y_size = 240;
    }
    
    for(i = 0; i < _gstUiCtx.bms_num; i++)
    {
        _gstEmListBatteryInfoPara.sub_region[i].x_pos = 20;
        _gstEmListBatteryInfoPara.sub_region[i].y_pos = 38 + 42 * i;
        _gstEmListBatteryInfoPara.sub_region[i].x_size = 420;
        _gstEmListBatteryInfoPara.sub_region[i].y_size = 34;
    }

    /*List控件*/
    for(j = 0; j < _gstUiCtx.bms_num; j++)
    {
        //第i行左边
        _gstEmListBatteryInfoPara.sub_widget_para[0][j].widget_type = EM_WIDGET_TYPE_TEXT;
        _gstEmListBatteryInfoPara.sub_widget_para[0][j].x_relative_pos = 11;
        _gstEmListBatteryInfoPara.sub_widget_para[0][j].y_relative_pos = 10;
        _gstEmListBatteryInfoPara.sub_widget_para[0][j].x_size = 158;
        _gstEmListBatteryInfoPara.sub_widget_para[0][j].y_size = 17;
        _gstEmListBatteryInfoPara.sub_widget_para[0][j].sub_widget.text_para.p_content = _garrListBatteryInfoLeftTextBuf[j];
        _gstEmListBatteryInfoPara.sub_widget_para[0][j].sub_widget.text_para.text_color = 0x00FFFFFF;
        _gstEmListBatteryInfoPara.sub_widget_para[0][j].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
        snprintf(_gstEmListBatteryInfoPara.sub_widget_para[0][j].sub_widget.text_para.p_content, 
                 sizeof(_garrListBatteryInfoLeftTextBuf[j]),"Pack %d",j + 1);
        
        //第i行右边
        _gstEmListBatteryInfoPara.sub_widget_para[1][j].widget_type = EM_WIDGET_TYPE_IMG;
        _gstEmListBatteryInfoPara.sub_widget_para[1][j].x_relative_pos = 395;
        _gstEmListBatteryInfoPara.sub_widget_para[1][j].y_relative_pos = 12;
        _gstEmListBatteryInfoPara.sub_widget_para[1][j].x_size = 5;
        _gstEmListBatteryInfoPara.sub_widget_para[1][j].y_size = 10;
        _gstEmListBatteryInfoPara.sub_widget_para[1][j].sub_widget.img_para.mem_img = _gListBatteryInfoRightMem;
        _gstEmListBatteryInfoPara.sub_widget_para[1][j].sub_widget.img_para.p_img_buf = _pListBatteryInfoRightBuf;
        _gstEmListBatteryInfoPara.sub_widget_para[1][j].sub_widget.img_para.img_size = EM_LIST_BATTERY_INFO_ICON_ARROW_SIZE;
    }
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gListBatteryInfoRightMem);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    if(gstEmWinListBatteryInfo.win_handle == 0)
    {
        return;
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

    if (event == EN_KEY_SRC_LEFT)
    {
        --_gstUiCtx.select;
        if (_gstUiCtx.select < 0)
        {
            ++_gstUiCtx.select;
            return;
        }
        EmListUpward(&_gstEmListBatteryInfoPara, gstEmWinListBatteryInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListBatteryInfoPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListBatteryInfoPara, gstEmWinListBatteryInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        emWin_t *pnext = NULL;
        switch(_gstUiCtx.select)
        {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            {
                gPackInfoIdx = _gstUiCtx.select;
                pnext = &gstEmWinListBatteryPackInfo;
                break;
            }
            default:
                return;
        }
        if(pnext != NULL && pCurrentEmWin->win_handle != 0)
        {
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
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


