/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_LIST_BATTERY_PACK__VERSION_INFO.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-25
  * @brief       电池包固件版本信息
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-25   <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define PACK_VER_DBG        0


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListBatteryVersionInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t pack_hw_ver[BMS_MAX_NUM];
    uint32_t pack_sw_ver[BMS_MAX_NUM];
    uint16_t fpga_hw_ver[BMS_MAX_NUM];
    uint32_t fpga_sw_ver[BMS_MAX_NUM];
    uint16_t bms_num;
} UiCtx_t;

static UiCtx_t _gstUiCtx;


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;
static char _garrTitleBuf[] = "Basic Info < Battery Pack Version";

/*List控件*/
static emListPara_t  _gstEmListBatteryPackVersionInfoPara;
static char _garrListBatteryPackVersionInfoLeftTextBuf[12][20];
static char _garrListBatteryPackVersionInfoRightTextBuf[12][20];


#define WIN_LIST_BATTERY_PACK_VERSION_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_BATTERY_PACK_VERSION_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListBatteryPackVersionInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListBatteryPackVersionInfoWin",    WIN_LIST_BATTERY_PACK_VERSION_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_BATTERY_PACK_VERSION_INFO_TITLE_ID,     31,  20, 415,  17,   0,   0x0,  0 },
};


static int32_t _SetPackVer(uint8_t ui_update, int32_t idx, uint16_t hw, uint32_t sw)
{
    uint16_t hw_highbit8 = (hw >> 8) + 0x41;   //0~25 表示A~Z  需要在此基础上加偏移（显示使用ASCII码值）
    uint16_t hw_lowbit8  = hw & 0x00ff;

    uint8_t sw_highbit8 = sw >> 16;
    uint8_t sw_midbit8  = sw >> 8;
    uint8_t sw_lowbit8  = sw >> 0;

    if (idx < 0 || idx >= sizeof(_garrListBatteryPackVersionInfoRightTextBuf) /
            sizeof(_garrListBatteryPackVersionInfoRightTextBuf[0]))
    {
        return -1;
    }

    snprintf(_garrListBatteryPackVersionInfoRightTextBuf[idx], sizeof(_garrListBatteryPackVersionInfoRightTextBuf[idx]),
             "V%d.%d.%d | %c%d", sw_highbit8, sw_midbit8, sw_lowbit8, hw_highbit8, hw_lowbit8);

    if(ui_update)
    {
        TEXT_SetText(_gstEmListBatteryPackVersionInfoPara.handle[1][idx],
                     (const char *)_gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][idx].sub_widget.text_para.p_content);
    }
    return 0;
}
static int32_t _SetFpgaVer(uint8_t ui_update, int32_t idx, uint16_t hw, uint32_t sw)
{
    uint16_t hw_highbit8 = (hw >> 8) + 0x41;   //0~25 表示A~Z  需要在此基础上加偏移（显示使用ASCII码值）
    uint16_t hw_lowbit8  = hw & 0x00ff;

    uint8_t sw_highbit8 = sw >> 16;
    uint8_t sw_midbit8  = sw >> 8;
    uint8_t sw_lowbit8  = sw >> 0;

    if (idx < 0 || idx >= sizeof(_garrListBatteryPackVersionInfoRightTextBuf) /
            sizeof(_garrListBatteryPackVersionInfoRightTextBuf[0]))
    {
        return -1;
    }

    snprintf(_garrListBatteryPackVersionInfoRightTextBuf[idx], sizeof(_garrListBatteryPackVersionInfoRightTextBuf[idx]),
             "V%d.%d.%d | %c%d", sw_highbit8, sw_midbit8, sw_lowbit8, hw_highbit8, hw_lowbit8);

    if(ui_update)
    {
        TEXT_SetText(_gstEmListBatteryPackVersionInfoPara.handle[1][idx],
                     (const char *)_gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][idx].sub_widget.text_para.p_content);
    }
    return 0;
}

/**
  * @brief ListBatteryPackVersionInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListBatteryPackVersionInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BATTERY_PACK_VERSION_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListBatteryPackVersionInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListBatteryPackVersionInfoPara, _gstEmListBatteryPackVersionInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(i = 0; i < _gstEmListBatteryPackVersionInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListBatteryPackVersionInfoPara.sub_region[i].x_pos,
                                    _gstEmListBatteryPackVersionInfoPara.sub_region[i].y_pos,
                                    _gstEmListBatteryPackVersionInfoPara.sub_region[i].x_pos + _gstEmListBatteryPackVersionInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListBatteryPackVersionInfoPara.sub_region[i].y_pos + _gstEmListBatteryPackVersionInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListBatteryPackVersionInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListBatteryPackVersionInfoPara, _gstEmListBatteryPackVersionInfoPara.list_sel);
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
    pPrevEmWin = &gstEmWinListDevBaseInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListBatteryVersionInfo.win_handle = GUI_CreateDialogBox(_garrListBatteryPackVersionInfoDialogCreate, GUI_COUNTOF(_garrListBatteryPackVersionInfoDialogCreate), _cbListBatteryPackVersionInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListBatteryVersionInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListBatteryVersionInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListBatteryVersionInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListBatteryPackVersionInfoPara, gstEmWinListBatteryVersionInfo.win_handle);
    return gstEmWinListBatteryVersionInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListBatteryVersionInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListBatteryVersionInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListBatteryVersionInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t k = 0;
    uint8_t idx = 0;
    uint8_t widget_num = 0;

    _gstUiCtx.bms_num = GetBmsNum();
    
#if PACK_VER_DBG
    uint8_t tmp_num = 0;
    _gstUiCtx.bms_num = tmp_num; 
    uint16_t fpga_ver_hw[6];
    uint16_t fpga_ver_sw[6];
#endif

    if(_gstUiCtx.bms_num > BMS_MAX_NUM)
    {
        _gstUiCtx.bms_num = BMS_MAX_NUM;
    }
    
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListBatteryPackVersionInfoPara.list_start_id = WIN_LIST_BATTERY_PACK_VERSION_INFO_ID + 0x30;
    _gstEmListBatteryPackVersionInfoPara.list_num = _gstUiCtx.bms_num * 2;
    _gstEmListBatteryPackVersionInfoPara.list_display_num = 6;
    if(_gstEmListBatteryPackVersionInfoPara.list_num <= _gstEmListBatteryPackVersionInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListBatteryPackVersionInfoPara.list_num + 1 - _gstEmListBatteryPackVersionInfoPara.list_display_num;
    }

    if(_gstEmListBatteryPackVersionInfoPara.list_sel > _gstEmListBatteryPackVersionInfoPara.list_num - 1)
    {
        _gstEmListBatteryPackVersionInfoPara.list_sel = 0;
    }
    if(_gstEmListBatteryPackVersionInfoPara.list_page > max_page - 1)
    {
        _gstEmListBatteryPackVersionInfoPara.list_page = 0;
    }
    _gstEmListBatteryPackVersionInfoPara.list_bk_color = 0x00787878;
    /*4个子列表*/
    //每次初始化之前将列表先清零 防止出现之后框没有版本的问题
    for(i = 0; i < BMS_MAX_NUM * 2; i++)
    {
        _gstEmListBatteryPackVersionInfoPara.sub_region[i].x_pos = 0;
        _gstEmListBatteryPackVersionInfoPara.sub_region[i].y_pos = 0;
        _gstEmListBatteryPackVersionInfoPara.sub_region[i].x_size = 0;
        _gstEmListBatteryPackVersionInfoPara.sub_region[i].y_size = 0;
    }
    
    if(_gstUiCtx.bms_num > 3)
    {
         /*List选中框*/
        _gstEmListBatteryPackVersionInfoPara.selected_unvalid_color = 0xFF000000;
        _gstEmListBatteryPackVersionInfoPara.selected_valid_color = 0x00D6E400;
        _gstEmListBatteryPackVersionInfoPara.selected_size = 3;
        _gstEmListBatteryPackVersionInfoPara.selected_r = 10;
        _gstEmListBatteryPackVersionInfoPara.selected_x_start_offset = -2;
        _gstEmListBatteryPackVersionInfoPara.selected_y_start_offset = -2;
        _gstEmListBatteryPackVersionInfoPara.selected_x_end_offset = 2;
        _gstEmListBatteryPackVersionInfoPara.selected_y_end_offset = 2;
//    _gstEmListBatteryPackVersionInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
        _gstEmListBatteryPackVersionInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

        /*滚动条*/
        _gstEmListBatteryPackVersionInfoPara.scrollbar_enable = true;
        _gstEmListBatteryPackVersionInfoPara.scrollbar_color = 0x00CDF100;
        _gstEmListBatteryPackVersionInfoPara.scrollbar_bk_color = 0x00565656;
        _gstEmListBatteryPackVersionInfoPara.scrollbar_x_pos = 457;
        _gstEmListBatteryPackVersionInfoPara.scrollbar_y_pos = 39;
        _gstEmListBatteryPackVersionInfoPara.scrollbar_x_size = 6;
        _gstEmListBatteryPackVersionInfoPara.scrollbar_y_size = 240;
    }  
    
    if(_gstUiCtx.bms_num > 2)
    {
        widget_num = 6;
    }
    else
    {
        widget_num = _gstUiCtx.bms_num * 2;
        _gstEmListBatteryPackVersionInfoPara.selected_r = 10;
    }
    
    //根据pack包数进行初始化
    for(i = 0; i < widget_num; i++)
    {
        _gstEmListBatteryPackVersionInfoPara.sub_region[i].x_pos = 20;
        _gstEmListBatteryPackVersionInfoPara.sub_region[i].y_pos = 38 + 42 * i;
        _gstEmListBatteryPackVersionInfoPara.sub_region[i].x_size = 420;
        _gstEmListBatteryPackVersionInfoPara.sub_region[i].y_size = 34;
    }

    /*List控件*/
    for(j = 0, k = 0; j < _gstUiCtx.bms_num * 2; j+= 2,k++)
    {
        //第i行左边
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j].widget_type = EM_WIDGET_TYPE_TEXT;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j].x_relative_pos = 11;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j].y_relative_pos = 10;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j].x_size = 158;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j].y_size = 17;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j].sub_widget.text_para.p_content = _garrListBatteryPackVersionInfoLeftTextBuf[j];
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j].sub_widget.text_para.text_color = 0x00FFFFFF;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
        snprintf(_gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j].sub_widget.text_para.p_content, 
                 sizeof(_garrListBatteryPackVersionInfoLeftTextBuf[j]),"Pack %d",k + 1);
        //第i行右边
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j].widget_type = EM_WIDGET_TYPE_TEXT;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j].x_relative_pos = 200;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j].y_relative_pos = 10;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j].x_size = 210;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j].y_size = 17;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j].sub_widget.text_para.p_content = _garrListBatteryPackVersionInfoRightTextBuf[j];
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j].sub_widget.text_para.text_color = 0x00FFFFFF;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
        //第i+1行左边
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j+1].widget_type = EM_WIDGET_TYPE_TEXT;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j+1].x_relative_pos = 11;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j+1].y_relative_pos = 10;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j+1].x_size = 158;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j+1].y_size = 17;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j+1].sub_widget.text_para.p_content = _garrListBatteryPackVersionInfoLeftTextBuf[j+1];
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j+1].sub_widget.text_para.text_color = 0x00FFFFFF;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j+1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
        snprintf(_gstEmListBatteryPackVersionInfoPara.sub_widget_para[0][j+1].sub_widget.text_para.p_content, 
                 sizeof(_garrListBatteryPackVersionInfoLeftTextBuf[j+1]),"FPGA %d",k + 1);
        //第i+1行右边
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j+1].widget_type = EM_WIDGET_TYPE_TEXT;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j+1].x_relative_pos = 200;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j+1].y_relative_pos = 10;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j+1].x_size = 210;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j+1].y_size = 17;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j+1].sub_widget.text_para.p_content = _garrListBatteryPackVersionInfoRightTextBuf[j+1];
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j+1].sub_widget.text_para.text_color = 0x00FFFFFF;
        _gstEmListBatteryPackVersionInfoPara.sub_widget_para[1][j+1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    }

    for (i = 0; i < _gstUiCtx.bms_num; ++i)
    {
        _gstUiCtx.pack_hw_ver[i] = gstPackSlavePara.pack_para[i].bms_info.base_info.hw_ver;//gstCollectInfo.pack_data[i].base_info.hw_ver;
        _gstUiCtx.pack_sw_ver[i] = gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver[0] << 16 | gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver[1];
        _gstUiCtx.fpga_hw_ver[i] = gstPackSlavePara.pack_para[i].dcdc_info.base_info.hw_ver;
        _gstUiCtx.fpga_sw_ver[i] = gstPackSlavePara.pack_para[i].dcdc_info.base_info.sw_ver[0] << 16 | gstPackSlavePara.pack_para[i].dcdc_info.base_info.sw_ver[1];
#if PACK_VER_DBG
        _gstUiCtx.fpga_hw_ver[i] = fpga_ver_hw[i];
        _gstUiCtx.fpga_sw_ver[i] = fpga_ver_sw[i];
#endif
    }
    
    for(i = 0, idx = 0; idx < _gstUiCtx.bms_num * 2; idx += 2, i++)
    {
        _SetPackVer(0, idx, _gstUiCtx.pack_hw_ver[i], _gstUiCtx.pack_sw_ver[i]);
        _SetFpgaVer(0, idx+1, _gstUiCtx.fpga_hw_ver[i],_gstUiCtx.fpga_sw_ver[i]);
    }
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
    int32_t i = 0;
    int32_t idx = 0;
    for (i = 0, idx = 0; i < _gstUiCtx.bms_num; i++, idx += 2)
    {
        //pack
        if (_gstUiCtx.pack_hw_ver[i] != gstPackSlavePara.pack_para[i].bms_info.base_info.hw_ver ||
            _gstUiCtx.pack_sw_ver[i] != (gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver[0] << 16 | gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver[1]))
        {
            _gstUiCtx.pack_hw_ver[i] = gstPackSlavePara.pack_para[i].bms_info.base_info.hw_ver;//gstCollectInfo.pack_data[i].base_info.hw_ver;
            _gstUiCtx.pack_sw_ver[i] = gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver[0] << 16 | gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver[1];
            _SetPackVer(1, idx, _gstUiCtx.pack_hw_ver[i], _gstUiCtx.pack_sw_ver[i]);
        }
        //fpga
        if(_gstUiCtx.fpga_hw_ver[i] != gstPackSlavePara.pack_para[i].dcdc_info.base_info.hw_ver ||
           _gstUiCtx.fpga_sw_ver[i] != (gstPackSlavePara.pack_para[i].dcdc_info.base_info.sw_ver[0] << 16 | gstPackSlavePara.pack_para[i].dcdc_info.base_info.sw_ver[1]))
        {
            _gstUiCtx.fpga_hw_ver[i] = gstPackSlavePara.pack_para[i].dcdc_info.base_info.hw_ver;
            _gstUiCtx.fpga_sw_ver[i] = gstPackSlavePara.pack_para[i].dcdc_info.base_info.sw_ver[0] << 16 | gstPackSlavePara.pack_para[i].dcdc_info.base_info.sw_ver[1];
            _SetFpgaVer(1, idx+1, _gstUiCtx.fpga_hw_ver[i], _gstUiCtx.fpga_sw_ver[i]);
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
    if(event < 0 || event > EN_KEY_SRC_NUM)
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
        EmListUpward(&_gstEmListBatteryPackVersionInfoPara, gstEmWinListBatteryVersionInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListBatteryPackVersionInfoPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListBatteryPackVersionInfoPara, gstEmWinListBatteryVersionInfo.win_handle);
        return;
    }
    else if(event == EN_KEY_SRC_ESC)
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
                pnext = &gstEmWinListDevBaseInfo;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}


