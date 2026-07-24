/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_LIST_ARM_VERSION_INFO.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-25
  * @brief       ARM DSP版本
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-25    <td>V1.0.0    <td>           <td>InitialVersion
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


emWin_t gstEmWinListArmVersionInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t arm_hw_ver;
    uint32_t arm_sw_ver;
    uint16_t dsp_hw_ver;
    uint32_t dsp_sw_ver;
    uint16_t arc_hw_ver;
    uint32_t arc_sw_ver;
    uint16_t arm_cop_hw_ver;
    uint32_t arm_cop_sw_ver;
    uint16_t dsp_cop_hw_ver;
    uint32_t dsp_cop_sw_ver;
} UiCtx_t;

static UiCtx_t _gstUiCtx;


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;
static char _garrTitleBuf[] = "Basic Info < Inverter Version";

/*List控件*/
static emListPara_t  _gstEmListArmVersionInfoPara;
static char _garrListArmVersionInfoLeftTextBuf[5][20];
static char _garrListArmVersionInfoRightTextBuf[5][20];


#define WIN_LIST_ARM_VERSION_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_ARM_VERSION_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListArmVersionInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListArmVersionInfoWin",    WIN_LIST_ARM_VERSION_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_ARM_VERSION_INFO_TITLE_ID,     31,  20, 415,  17,   0,   0x0,  0 },
};

static int32_t _SetArmVer(uint8_t ui_update, uint16_t hw, uint32_t sw)
{
    uint16_t hw_highbit8 = (hw >> 8) + 'A';
    uint16_t hw_lowbit8  = hw & 0x00ff;

    uint8_t sw_highbit8 = sw >> 16;
    uint8_t sw_midbit8  = sw >> 8;
    uint8_t sw_lowbit8  = sw >> 0;

    snprintf(_garrListArmVersionInfoRightTextBuf[0], sizeof(_garrListArmVersionInfoRightTextBuf[0]),
             "V%d.%d.%d | %c%d", sw_highbit8, sw_midbit8, sw_lowbit8, hw_highbit8, hw_lowbit8);
    if(ui_update)
    {
        TEXT_SetText(_gstEmListArmVersionInfoPara.handle[1][0],
                     (const char *)_gstEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetDspVer(uint8_t ui_update, uint16_t hw, uint32_t sw)
{
    uint16_t hw_highbit8 = (hw >> 8) + 'A';
    uint16_t hw_lowbit8  = hw & 0xff;

    uint8_t sw_highbit8 = sw >> 16;
    uint8_t sw_midbit8  = sw >> 8;
    uint8_t sw_lowbit8  = sw >> 0;

    snprintf(_garrListArmVersionInfoRightTextBuf[1], sizeof(_garrListArmVersionInfoRightTextBuf[1]),
             "V%d.%d.%d | %c%d", sw_highbit8, sw_midbit8, sw_lowbit8, hw_highbit8, hw_lowbit8);
    if(ui_update)
    {
        TEXT_SetText(_gstEmListArmVersionInfoPara.handle[1][1],
                     (const char *)_gstEmListArmVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetArcVer(uint8_t ui_update, uint16_t hw, uint32_t sw)
{
    uint16_t hw_highbit8 = (hw >> 8) + 'A';
    uint16_t hw_lowbit8  = hw & 0xff;

    uint8_t sw_highbit8 = sw >> 16;
    uint8_t sw_midbit8  = sw >> 8;
    uint8_t sw_lowbit8  = sw >> 0;

    snprintf(_garrListArmVersionInfoRightTextBuf[2], sizeof(_garrListArmVersionInfoRightTextBuf[2]),
             "V%d.%d.%d | %c%d", sw_highbit8, sw_midbit8, sw_lowbit8, hw_highbit8, hw_lowbit8);
    if(ui_update)
    {
        TEXT_SetText(_gstEmListArmVersionInfoPara.handle[1][2],
                     (const char *)_gstEmListArmVersionInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetArmCopVer(uint8_t ui_update, uint16_t hw, uint32_t sw)
{
    uint16_t hw_highbit8 = (hw >> 8) + 'A';
    uint16_t hw_lowbit8  = hw & 0xff;

    uint8_t sw_highbit8 = sw >> 16;
    uint8_t sw_midbit8  = sw >> 8;
    uint8_t sw_lowbit8  = sw >> 0;

    snprintf(_garrListArmVersionInfoRightTextBuf[3], sizeof(_garrListArmVersionInfoRightTextBuf[3]),
             "V%d.%d.%d | %c%d", sw_highbit8, sw_midbit8, sw_lowbit8, hw_highbit8, hw_lowbit8);
    if(ui_update)
    {
        TEXT_SetText(_gstEmListArmVersionInfoPara.handle[1][3],
                     (const char *)_gstEmListArmVersionInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetDspCopVer(uint8_t ui_update, uint16_t hw, uint32_t sw)
{
    uint16_t hw_highbit8 = (hw >> 8) + 'A';
    uint16_t hw_lowbit8  = hw & 0xff;

    uint8_t sw_highbit8 = sw >> 16;
    uint8_t sw_midbit8  = sw >> 8;
    uint8_t sw_lowbit8  = sw >> 0;

    snprintf(_garrListArmVersionInfoRightTextBuf[4], sizeof(_garrListArmVersionInfoRightTextBuf[4]),
             "V%d.%d.%d | %c%d", sw_highbit8, sw_midbit8, sw_lowbit8, hw_highbit8, hw_lowbit8);
    if(ui_update)
    {
        TEXT_SetText(_gstEmListArmVersionInfoPara.handle[1][4],
                     (const char *)_gstEmListArmVersionInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetInverterVer(uint8_t ui_update, uint16_t sw)
{
    uint16_t sw_highbit4 = sw >> 12;
    uint16_t sw_midbit6  = (sw & 0x0FC0) >> 6;
    uint16_t sw_lowbit6  = (sw & 0x003F);

    snprintf(_garrListArmVersionInfoRightTextBuf[0], sizeof(_garrListArmVersionInfoRightTextBuf[0]),
             "V%d.%d.%d", sw_highbit4, sw_midbit6, sw_lowbit6);
    if(ui_update)
    {
        TEXT_SetText(_gstEmListArmVersionInfoPara.handle[1][0],
                     (const char *)_gstEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief ListArmVersionInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListArmVersionInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ARM_VERSION_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListArmVersionInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListArmVersionInfoPara, _gstEmListArmVersionInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(i = 0; i < _gstEmListArmVersionInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListArmVersionInfoPara.sub_region[i].x_pos,
                                    _gstEmListArmVersionInfoPara.sub_region[i].y_pos,
                                    _gstEmListArmVersionInfoPara.sub_region[i].x_pos + _gstEmListArmVersionInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListArmVersionInfoPara.sub_region[i].y_pos + _gstEmListArmVersionInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListArmVersionInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListArmVersionInfoPara, _gstEmListArmVersionInfoPara.list_sel);
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
    gstEmWinListArmVersionInfo.win_handle = GUI_CreateDialogBox(_garrListArmVersionInfoDialogCreate, GUI_COUNTOF(_garrListArmVersionInfoDialogCreate), _cbListArmVersionInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListArmVersionInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListArmVersionInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListArmVersionInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListArmVersionInfoPara, gstEmWinListArmVersionInfo.win_handle);
    return gstEmWinListArmVersionInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListArmVersionInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListArmVersionInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListArmVersionInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;

    _gstUiCtx.arm_hw_ver = gstCollectInfo.maincard_data.base_info.hw_ver;
    _gstUiCtx.arm_sw_ver = gstCollectInfo.maincard_data.base_info.sw_ver;     //?????

    _gstUiCtx.dsp_hw_ver = gstCollectInfo.power_data.pcs_base_info.hw_ver;
    _gstUiCtx.dsp_sw_ver = gstCollectInfo.power_data.pcs_base_info.sw_ver;

    _gstUiCtx.arc_hw_ver = gstCollectInfo.power_data.arc_base_info.hw_ver;
    _gstUiCtx.arc_sw_ver = gstCollectInfo.power_data.arc_base_info.sw_ver;

    _gstUiCtx.arm_cop_hw_ver = gstCollectInfo.maincard_data.base_info.hw_ver;
    _gstUiCtx.arm_cop_sw_ver = gstCollectInfo.cop_data.cop_device1_info.cop_sw_version;

    _gstUiCtx.dsp_cop_hw_ver = gstCollectInfo.power_data.pcs_base_info.hw_ver;
    _gstUiCtx.dsp_cop_sw_ver = gstCollectInfo.cop_data.cop_device2_info.cop_sw_version;


    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListArmVersionInfoPara.list_start_id = WIN_LIST_ARM_VERSION_INFO_ID + 0x30;
    _gstEmListArmVersionInfoPara.list_num = 5;
    _gstEmListArmVersionInfoPara.list_display_num = 6;
    if(_gstEmListArmVersionInfoPara.list_num <= _gstEmListArmVersionInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListArmVersionInfoPara.list_num + 1 - _gstEmListArmVersionInfoPara.list_display_num;
    }

    if(_gstEmListArmVersionInfoPara.list_sel > _gstEmListArmVersionInfoPara.list_num - 1)
    {
        _gstEmListArmVersionInfoPara.list_sel = 0;
    }
    if(_gstEmListArmVersionInfoPara.list_page > max_page - 1)
    {
        _gstEmListArmVersionInfoPara.list_page = 0;
    }
    _gstEmListArmVersionInfoPara.list_bk_color = 0x00787878;
    /*5个子列表*/
    _gstEmListArmVersionInfoPara.sub_region[0].x_pos = 20;
    _gstEmListArmVersionInfoPara.sub_region[0].y_pos = 38;
    _gstEmListArmVersionInfoPara.sub_region[0].x_size = 440;
    _gstEmListArmVersionInfoPara.sub_region[0].y_size = 34;    
    _gstEmListArmVersionInfoPara.sub_region[1].x_pos = 20;
    _gstEmListArmVersionInfoPara.sub_region[1].y_pos = 80;
    _gstEmListArmVersionInfoPara.sub_region[1].x_size = 440;
    _gstEmListArmVersionInfoPara.sub_region[1].y_size = 34;
    _gstEmListArmVersionInfoPara.sub_region[2].x_pos = 20;
    _gstEmListArmVersionInfoPara.sub_region[2].y_pos = 122;
    _gstEmListArmVersionInfoPara.sub_region[2].x_size = 440;
    _gstEmListArmVersionInfoPara.sub_region[2].y_size = 34;
    _gstEmListArmVersionInfoPara.sub_region[3].x_pos = 20;
    _gstEmListArmVersionInfoPara.sub_region[3].y_pos = 164;
    _gstEmListArmVersionInfoPara.sub_region[3].x_size = 440;
    _gstEmListArmVersionInfoPara.sub_region[3].y_size = 34;
    _gstEmListArmVersionInfoPara.sub_region[4].x_pos = 20;
    _gstEmListArmVersionInfoPara.sub_region[4].y_pos = 206;
    _gstEmListArmVersionInfoPara.sub_region[4].x_size = 440;
    _gstEmListArmVersionInfoPara.sub_region[4].y_size = 34;

    /*List控件*/
#if 1
    //第一行左边
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].x_size = 158;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListArmVersionInfoLeftTextBuf[0];
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "ARM Version");
    //第一行右边
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].x_relative_pos = 198;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].x_size = 230;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListArmVersionInfoRightTextBuf[0];
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    snprintf(_garrListArmVersionInfoRightTextBuf[0], sizeof(_garrListArmVersionInfoRightTextBuf[0]),
//          "%4X | %4X", _gstUiCtx.arm_hw_ver, _gstUiCtx.arm_sw_ver);
    _SetArmVer(0, _gstUiCtx.arm_hw_ver, _gstUiCtx.arm_sw_ver);

    //第二行左边
    _gstEmListArmVersionInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][1].x_size = 158;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListArmVersionInfoLeftTextBuf[1];
    _gstEmListArmVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListArmVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "DSP Version");
    //第二行右边
    _gstEmListArmVersionInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][1].x_relative_pos = 198;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][1].x_size = 230;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][1].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListArmVersionInfoRightTextBuf[1];
    _gstEmListArmVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    snprintf(_garrListArmVersionInfoRightTextBuf[1], sizeof(_garrListArmVersionInfoRightTextBuf[1]),
//          "%4X | %4X", _gstUiCtx.dsp_hw_ver, _gstUiCtx.dsp_sw_ver);
    _SetDspVer(0, _gstUiCtx.dsp_hw_ver, _gstUiCtx.dsp_sw_ver);

    //第3行左边
    _gstEmListArmVersionInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][2].x_size = 158;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListArmVersionInfoLeftTextBuf[2];
    _gstEmListArmVersionInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListArmVersionInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "ARC Version");
    //第3行右边
    _gstEmListArmVersionInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][2].x_relative_pos = 198;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][2].x_size = 230;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][2].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListArmVersionInfoRightTextBuf[2];
    _gstEmListArmVersionInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    snprintf(_garrListArmVersionInfoRightTextBuf[2], sizeof(_garrListArmVersionInfoRightTextBuf[2]),
//          "%4X | %4X", _gstUiCtx.dsp_hw_ver, _gstUiCtx.dsp_sw_ver);
    _SetArcVer(0, _gstUiCtx.arc_hw_ver, _gstUiCtx.arc_sw_ver);

    //第4行左边
    _gstEmListArmVersionInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][3].x_size = 158;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListArmVersionInfoLeftTextBuf[3];
    _gstEmListArmVersionInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListArmVersionInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "ARM Cop Version");
    //第4行右边
    _gstEmListArmVersionInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][3].x_relative_pos = 198;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][3].x_size = 230;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][3].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _garrListArmVersionInfoRightTextBuf[3];
    _gstEmListArmVersionInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    snprintf(_garrListArmVersionInfoRightTextBuf[3], sizeof(_garrListArmVersionInfoRightTextBuf[3]),
//          "%4X | %4X", _gstUiCtx.dsp_hw_ver, _gstUiCtx.dsp_sw_ver);
    _SetArmCopVer(0, _gstUiCtx.arm_cop_hw_ver, _gstUiCtx.arm_cop_sw_ver);

    //第5行左边
    _gstEmListArmVersionInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][4].x_size = 158;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListArmVersionInfoLeftTextBuf[4];
    _gstEmListArmVersionInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListArmVersionInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "DSP Cop Version");
    //第5行右边
    _gstEmListArmVersionInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][4].x_relative_pos = 198;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][4].x_size = 230;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][4].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _garrListArmVersionInfoRightTextBuf[4];
    _gstEmListArmVersionInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    snprintf(_garrListArmVersionInfoRightTextBuf[4], sizeof(_garrListArmVersionInfoRightTextBuf[4]),
//          "%4X | %4X", _gstUiCtx.dsp_hw_ver, _gstUiCtx.dsp_sw_ver);
    _SetDspCopVer(0, _gstUiCtx.dsp_cop_hw_ver, _gstUiCtx.dsp_cop_sw_ver);

#else
    //第一行左边
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].x_size = 158;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListArmVersionInfoLeftTextBuf[0];
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Inverter Version");
    //第一行右边
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].x_relative_pos = 198;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].x_size = 230;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].y_size = 17;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListArmVersionInfoRightTextBuf[0];
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    snprintf(_garrListArmVersionInfoRightTextBuf[0], sizeof(_garrListArmVersionInfoRightTextBuf[0]),
//          "%4X | %4X", _gstUiCtx.arm_hw_ver, _gstUiCtx.arm_sw_ver);
    _SetInverterVer(0, _gstUiCtx.arm_sw_ver);

#endif
    _gstEmListArmVersionInfoPara.selected_r = 10;
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
#if 1
    uint8_t arr_hw_buff[256 + 8] = {0};
    uint8_t arr_sw_buff[256 + 8] = {0};
    uint16_t hw_num = 0;
    uint16_t sw_num = 0;

    if (_gstUiCtx.arm_hw_ver != gstCollectInfo.maincard_data.base_info.hw_ver
            || _gstUiCtx.arm_sw_ver != gstCollectInfo.maincard_data.base_info.hw_ver)
    {

        _gstUiCtx.arm_hw_ver = gstCollectInfo.maincard_data.base_info.hw_ver;
        _gstUiCtx.arm_sw_ver = gstCollectInfo.maincard_data.base_info.sw_ver;
        _SetArmVer(1, _gstUiCtx.arm_hw_ver, _gstUiCtx.arm_sw_ver);

//        hw_num = BigLittleSwap16(_gstUiCtx.arm_hw_ver);
//        sw_num = BigLittleSwap16(_gstUiCtx.arm_sw_ver);
//        ModbusWriteSingleRemain(arr_hw_buff, 0x01, COLLECT_HW_VER_ADDR, hw_num);
//        ModbusWriteSingleRemain(arr_sw_buff, 0x01, COLLECT_SW_VER_ADDR, sw_num);
    }

    if (_gstUiCtx.dsp_hw_ver != gstCollectInfo.power_data.pcs_base_info.hw_ver
     || _gstUiCtx.dsp_sw_ver != gstCollectInfo.power_data.pcs_base_info.sw_ver)
    {
        _gstUiCtx.dsp_hw_ver = gstCollectInfo.power_data.pcs_base_info.hw_ver;
        _gstUiCtx.dsp_sw_ver = gstCollectInfo.power_data.pcs_base_info.sw_ver;
        _SetDspVer(1, _gstUiCtx.dsp_hw_ver, _gstUiCtx.dsp_sw_ver);
    }

    if(_gstUiCtx.arc_hw_ver != gstCollectInfo.power_data.arc_base_info.hw_ver
            || _gstUiCtx.arc_sw_ver != gstCollectInfo.power_data.arc_base_info.sw_ver)
    {
        _gstUiCtx.arc_hw_ver = gstCollectInfo.power_data.arc_base_info.hw_ver;
        _gstUiCtx.arc_sw_ver = gstCollectInfo.power_data.arc_base_info.sw_ver;
        _SetArcVer(1, _gstUiCtx.arc_hw_ver, _gstUiCtx.arc_sw_ver);
    }

    if(_gstUiCtx.arm_cop_hw_ver != gstCollectInfo.maincard_data.base_info.hw_ver
            || _gstUiCtx.arm_cop_sw_ver != gstCollectInfo.cop_data.cop_device1_info.cop_sw_version)
    {
        _gstUiCtx.arm_cop_hw_ver = gstCollectInfo.maincard_data.base_info.hw_ver;
         _gstUiCtx.arm_cop_sw_ver = gstCollectInfo.cop_data.cop_device1_info.cop_sw_version;
        _SetArmCopVer(1, _gstUiCtx.arm_cop_hw_ver, _gstUiCtx.arm_cop_sw_ver);
    }

    if(_gstUiCtx.dsp_cop_hw_ver != gstCollectInfo.power_data.pcs_base_info.hw_ver
            || _gstUiCtx.dsp_cop_sw_ver != gstCollectInfo.cop_data.cop_device2_info.cop_sw_version)
    {
        _gstUiCtx.dsp_cop_hw_ver = gstCollectInfo.power_data.pcs_base_info.hw_ver;
        _gstUiCtx.dsp_cop_sw_ver = gstCollectInfo.cop_data.cop_device2_info.cop_sw_version;
        _SetDspCopVer(1, _gstUiCtx.dsp_cop_hw_ver, _gstUiCtx.dsp_cop_sw_ver);
    }


#else
    if (_gstUiCtx.arm_sw_ver != gstCollectInfo.maincard_data.base_info.sw_ver)      //?????更新版本号
    {
        _gstUiCtx.arm_sw_ver = gstCollectInfo.maincard_data.base_info.sw_ver;
        _SetInverterVer(1, _gstUiCtx.arm_sw_ver);
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
    if(event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }

    if(event == EN_KEY_SRC_ESC)
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


