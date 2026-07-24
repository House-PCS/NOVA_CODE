/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-13
  * @brief       PV_INPUT信息
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

#define  EM_BACK_ON_ICON_ARROW_SIZE                     (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gstEmWinListPVInputInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    uint16_t pv_ch1_input_curr; // PV1输入电流
    uint16_t pv_ch1_input_volt; // PV1输入电压
    uint16_t pv_ch1_input_power;// PV1输入功率
    uint16_t pv_ch2_input_curr; // PV2输入电流
    uint16_t pv_ch2_input_volt; // PV2输入电压
    uint16_t pv_ch2_input_power;// PV2输入功率
    uint16_t pv_ch3_input_curr; // PV3输入电流
    uint16_t pv_ch3_input_volt; // PV3输入电压
    uint16_t pv_ch3_input_power;// PV3输入功率
    uint16_t pv_ch4_input_curr; // PV4输入电流
    uint16_t pv_ch4_input_volt; // PV4输入电压
    uint16_t pv_ch4_input_power;// PV4输入功率
    
    int32_t select;
} UiCtx_t;

static UiCtx_t _gstUiCtx;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

static emWidgetTextPara_t  _gstTitleTextPara;
//static emWidgetTextPara_t  _gstPressTextPara;
//static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;

static char _garrTitleBuf[] = "Operational State Info < PV Input";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";

/*List控件*/
static emListPara_t _gstEmListPVInputInfoPara;

static char _garrListPVInputInfoLeftTextBuf[12][20];
static char _garrListPVInputInfoRightTextBuf[12][20];
static char _garrListPVInputInfoHeadTextBuf[4][20];
static char * _gparrPvNameList[12] = {"PV1 Voltage", "PV1 Current", "PV1 Power", "PV2 Voltage", "PV2 Current", "PV2 Power",
                                      "PV3 Voltage", "PV3 Current", "PV3 Power", "PV4 Voltage", "PV4 Current", "PV4 Power"};

/*PV Input中的线条信息*/
#define WIN_LIST_PV_INPUT_LINE_SIZE                 (3)
#define WIN_LIST_PV_INPUT_LINE_LEN                  (66)

#define WIN_LIST_PV1_INPUT_LINE_X0POS               (79)
#define WIN_LIST_PV1_INPUT_LINE_Y0POS               (50)
#define WIN_LIST_PV1_INPUT_LINE_X1POS               (79)
#define WIN_LIST_PV1_INPUT_LINE_Y1POS               (50 + 66 -1)

#define WIN_LIST_PV2_INPUT_LINE_X0POS               (79)
#define WIN_LIST_PV2_INPUT_LINE_Y0POS               (146)
#define WIN_LIST_PV2_INPUT_LINE_X1POS               (79)
#define WIN_LIST_PV2_INPUT_LINE_Y1POS               (146 + 66 -1)


#define WIN_LIST_PV_INPUT_INFO_ID                                (GUI_ID_USER + 0x12)
#define WIN_LIST_PV_INPUT_INFO_TITLE_ID                          (GUI_ID_USER + 0x13)
#define WIN_LIST_PV_INPUT_INFO_PRESS_ID                          (GUI_ID_USER + 0x14)
#define WIN_LIST_PV_INPUT_INFO_BACK_ID                           (GUI_ID_USER + 0x15)
#define WIN_LIST_PV_INPUT_INFO_BACK_IMG_ID                       (GUI_ID_USER + 0x16)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListPVInputInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListPVInputInfoWin",    WIN_LIST_PV_INPUT_INFO_ID,       0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_PV_INPUT_INFO_TITLE_ID,         31,  20, 415,  14,   0,   0x0,  0 },
//  { TEXT_CreateIndirect,    "Press",          WIN_LIST_PV_INPUT_INFO_PRESS_ID,        133, 273, 33,   14,   0,   0x0,  0 },
//  { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_PV_INPUT_INFO_BACK_ID,         171-5, 275, 15,   10,   0,   0x0,  0 },
//  { TEXT_CreateIndirect,    "Back",           WIN_LIST_PV_INPUT_INFO_BACK_IMG_ID,     188, 273, 150,  14,   0,   0x0,  0 },
};



static int32_t _SetPv1Vol(int8_t ui_update, uint16_t vol)
{
    int16_t vol_tmp = 0;
    vol_tmp = (int16_t)vol;
    float voltage = (float)vol_tmp / 10;
    snprintf(_garrListPVInputInfoRightTextBuf[0], sizeof(_garrListPVInputInfoRightTextBuf[0]),
             "%.1fV", voltage);
    if (ui_update && _gstEmListPVInputInfoPara.list_page == 0)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][0],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}


static int32_t _SetPv1Cur(int8_t ui_update, uint16_t cur)
{
    int16_t cur_tmp = 0;
    cur_tmp = (int16_t)cur;
    float current = (float)cur_tmp / 10;
    snprintf(_garrListPVInputInfoRightTextBuf[1], sizeof(_garrListPVInputInfoRightTextBuf[1]),
             "%.1fA", current);
    if (ui_update && _gstEmListPVInputInfoPara.list_page == 0)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][1],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    }
    else if( ui_update && _gstEmListPVInputInfoPara.list_page == 1)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][0],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetPv1Power(int8_t ui_update, uint16_t power)
{
    int16_t power_tmp = 0;
    power_tmp = (int16_t)power;
    snprintf(_garrListPVInputInfoRightTextBuf[2], sizeof(_garrListPVInputInfoRightTextBuf[2]),
             "%dW", power_tmp);

    if (ui_update && _gstEmListPVInputInfoPara.list_page == 0)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][2],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 1)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][1],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 2)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][0],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetPv2Vol(int8_t ui_update, uint16_t vol)
{
    int16_t vol_tmp = 0;
    vol_tmp = (int16_t)vol;
    float voltage = (float)vol_tmp / 10;
    snprintf(_garrListPVInputInfoRightTextBuf[3], sizeof(_garrListPVInputInfoRightTextBuf[3]),
             "%.1fV", voltage);

    if (ui_update && _gstEmListPVInputInfoPara.list_page == 0)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][3],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 1)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][2],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 2)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][1],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 3)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][0],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetPv2Cur(int8_t ui_update, uint16_t cur)
{
    int16_t cur_tmp = 0;
    cur_tmp = cur;
    float current = (float)cur_tmp / 10;
    snprintf(_garrListPVInputInfoRightTextBuf[4], sizeof(_garrListPVInputInfoRightTextBuf[4]),
             "%.1fA", current);

    if (ui_update && _gstEmListPVInputInfoPara.list_page == 0)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][4],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }        
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 1)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][3],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 2)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][2],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 3)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][1],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 4)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][0],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
    }
    return 0;
}


static int32_t _SetPv2Power(int8_t ui_update, uint16_t power)
{
    int16_t power_tmp = 0;
    power_tmp = (int16_t)power;
    snprintf(_garrListPVInputInfoRightTextBuf[5], sizeof(_garrListPVInputInfoRightTextBuf[5]),
             "%dW", power_tmp);

    if (ui_update && _gstEmListPVInputInfoPara.list_page == 0)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][5],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 1)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][4],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 2)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][3],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 3)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][2],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 4)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][1],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 5)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][0],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetPv3Vol(int8_t ui_update, uint16_t vol)
{
    int16_t vol_tmp = 0;
    vol_tmp = (int16_t)vol;
    float voltage = (float)vol_tmp / 10;
    snprintf(_garrListPVInputInfoRightTextBuf[6], sizeof(_garrListPVInputInfoRightTextBuf[6]),
             "%.1fV", voltage);
    if (ui_update && _gstEmListPVInputInfoPara.list_page == 1)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][5],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 2)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][4],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 3)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][3],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 4)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][2],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 5)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][1],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 6)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][0],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetPv3Cur(int8_t ui_update, uint16_t cur)
{
    int16_t cur_tmp = 0;
    cur_tmp = cur;
    float current = (float)cur_tmp / 10;
    snprintf(_garrListPVInputInfoRightTextBuf[7], sizeof(_garrListPVInputInfoRightTextBuf[7]),
             "%.1fA", current);

    if (ui_update && _gstEmListPVInputInfoPara.list_page == 2)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][5],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 3)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][4],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 4)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][3],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 5)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][2],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 6)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][1],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_content);
    }
    return 0;
}


static int32_t _SetPv3Power(int8_t ui_update, uint16_t power)
{
    int16_t power_tmp = 0;
    power_tmp = (int16_t)power;
    snprintf(_garrListPVInputInfoRightTextBuf[8], sizeof(_garrListPVInputInfoRightTextBuf[8]),
             "%dW", power_tmp);

    if (ui_update && _gstEmListPVInputInfoPara.list_page == 3)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][5],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][8].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 4)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][4],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][8].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 5)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][3],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][8].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 6)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][2],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][8].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetPv4Vol(int8_t ui_update, uint16_t vol)
{
    int16_t vol_tmp = 0;
    vol_tmp = (int16_t)vol;
    float voltage = (float)vol_tmp / 10;
    snprintf(_garrListPVInputInfoRightTextBuf[9], sizeof(_garrListPVInputInfoRightTextBuf[9]),
             "%.1fV", voltage);
    if (ui_update && _gstEmListPVInputInfoPara.list_page == 4)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][5],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][9].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 5)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][4],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][9].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 6)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][3],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][9].sub_widget.text_para.p_content);
    }
    return 0;
}
static int32_t _SetPv4Cur(int8_t ui_update, uint16_t cur)
{
    int16_t cur_tmp = 0;
    cur_tmp = cur;
    float current = (float)cur_tmp / 10;
    snprintf(_garrListPVInputInfoRightTextBuf[10], sizeof(_garrListPVInputInfoRightTextBuf[10]),
             "%.1fA", current);

    if (ui_update && _gstEmListPVInputInfoPara.list_page == 5)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][5],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][10].sub_widget.text_para.p_content);
    }
    else if (ui_update && _gstEmListPVInputInfoPara.list_page == 6)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][4],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][10].sub_widget.text_para.p_content);
    }
    return 0;
}


static int32_t _SetPv4Power(int8_t ui_update, uint16_t power)
{
    int16_t power_tmp = 0;
    power_tmp = (int16_t)power;
    snprintf(_garrListPVInputInfoRightTextBuf[11], sizeof(_garrListPVInputInfoRightTextBuf[11]),
             "%dW", power_tmp);

    if (ui_update && _gstEmListPVInputInfoPara.list_page == 6)
    {
        TEXT_SetText(_gstEmListPVInputInfoPara.handle[1][5],
                     (const char *)_gstEmListPVInputInfoPara.sub_widget_para[1][11].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief ListPVInputInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListPVInputInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_PV_INPUT_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListPVInputInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListPVInputInfoPara, _gstEmListPVInputInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListPVInputInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListPVInputInfoPara.sub_region[i].x_pos,
                                    _gstEmListPVInputInfoPara.sub_region[i].y_pos,
                                    _gstEmListPVInputInfoPara.sub_region[i].x_pos + _gstEmListPVInputInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListPVInputInfoPara.sub_region[i].y_pos + _gstEmListPVInputInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListPVInputInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListPVInputInfoPara, _gstEmListPVInputInfoPara.list_sel);
//            GUI_SetPenSize(WIN_LIST_PV_INPUT_LINE_SIZE);
//            GUI_SetColor(0x00A2A856);
//            GUI_DrawLine(WIN_LIST_PV1_INPUT_LINE_X0POS, WIN_LIST_PV1_INPUT_LINE_Y0POS, WIN_LIST_PV1_INPUT_LINE_X1POS, WIN_LIST_PV1_INPUT_LINE_Y1POS);
//            GUI_DrawLine(WIN_LIST_PV2_INPUT_LINE_X0POS, WIN_LIST_PV2_INPUT_LINE_Y0POS, WIN_LIST_PV2_INPUT_LINE_X1POS, WIN_LIST_PV2_INPUT_LINE_Y1POS);
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
    gstEmWinListPVInputInfo.win_handle = GUI_CreateDialogBox(_garrListPVInputInfoDialogCreate, GUI_COUNTOF(_garrListPVInputInfoDialogCreate), _cbListPVInputInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListPVInputInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListPVInputInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListPVInputInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListPVInputInfoPara, gstEmWinListPVInputInfo.win_handle);
    return gstEmWinListPVInputInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListPVInputInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListPVInputInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListPVInputInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t i = 0;
    
    _gstUiCtx.pv_ch1_input_curr  = gstCollectInfo.power_data.power_value_info.pv_1_current;
    _gstUiCtx.pv_ch1_input_volt  = gstCollectInfo.power_data.power_value_info.pv_1_vol;
    _gstUiCtx.pv_ch1_input_power = gstCollectInfo.power_data.power_value_info.pv_1_power;
    _gstUiCtx.pv_ch2_input_curr  = gstCollectInfo.power_data.power_value_info.pv_2_current;
    _gstUiCtx.pv_ch2_input_volt  = gstCollectInfo.power_data.power_value_info.pv_2_vol;
    _gstUiCtx.pv_ch2_input_power = gstCollectInfo.power_data.power_value_info.pv_2_power;
    _gstUiCtx.pv_ch3_input_curr  = gstCollectInfo.power_data.power_value_info.pv_3_current;
    _gstUiCtx.pv_ch3_input_volt  = gstCollectInfo.power_data.power_value_info.pv_3_vol;
    _gstUiCtx.pv_ch3_input_power = gstCollectInfo.power_data.power_value_info.pv_3_power;
    _gstUiCtx.pv_ch4_input_curr  = gstCollectInfo.power_data.power_value_info.pv_4_current;
    _gstUiCtx.pv_ch4_input_volt  = gstCollectInfo.power_data.power_value_info.pv_4_vol;
    _gstUiCtx.pv_ch4_input_power = gstCollectInfo.power_data.power_value_info.pv_4_power;
    EmKeyDefaultInfoSet();
    /*底部back的参数*/
    //_gstPressTextPara.p_content = _garrPressBuf;
    //_gstPressTextPara.text_color = 0x00EEEEEE;
    //_gstPressTextPara.p_text_font = EM_TEXT_FONT_1;

    //_gstBackTextPara.p_content = _garrBackBuf;
    //_gstBackTextPara.text_color = 0x00EEEEEE;
    //_gstBackTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_BACK_ON_ICON_ARROW_SIZE);
    _gstReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gstReturnImgPara.mem_img);
    _gstReturnImgPara.img_size = EM_BACK_ON_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

    /*标题相关参数*/
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00EEEEEE;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListPVInputInfoPara.list_start_id = WIN_LIST_PV_INPUT_INFO_ID + 0x30;
    _gstEmListPVInputInfoPara.list_num = 12;
    _gstEmListPVInputInfoPara.list_display_num = 6;

    if(_gstEmListPVInputInfoPara.list_sel > _gstEmListPVInputInfoPara.list_num - 1)
    {
        _gstEmListPVInputInfoPara.list_sel = 0;
    }
    _gstEmListPVInputInfoPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _gstEmListPVInputInfoPara.selected_r = 10;

    /*6个子列表*/
    for(i = 0; i < 6; i++)
    {
        _gstEmListPVInputInfoPara.sub_region[i].x_pos = 20;
        _gstEmListPVInputInfoPara.sub_region[i].y_pos = 38 + (i * 42);
        _gstEmListPVInputInfoPara.sub_region[i].x_size = 420;
        _gstEmListPVInputInfoPara.sub_region[i].y_size = 34;
    }
#if 0
    _gstEmListPVInputInfoPara.sub_region[0].x_pos = 20;
    _gstEmListPVInputInfoPara.sub_region[0].y_pos = 38;
    _gstEmListPVInputInfoPara.sub_region[0].x_size = 440;
    _gstEmListPVInputInfoPara.sub_region[0].y_size = 88;
    _gstEmListPVInputInfoPara.sub_region[1].x_pos = 20;
    _gstEmListPVInputInfoPara.sub_region[1].y_pos = 38;
    _gstEmListPVInputInfoPara.sub_region[1].x_size = 440;
    _gstEmListPVInputInfoPara.sub_region[1].y_size = 88;
    _gstEmListPVInputInfoPara.sub_region[2].x_pos = 20;
    _gstEmListPVInputInfoPara.sub_region[2].y_pos = 38;
    _gstEmListPVInputInfoPara.sub_region[2].x_size = 440;
    _gstEmListPVInputInfoPara.sub_region[2].y_size = 88;
    _gstEmListPVInputInfoPara.sub_region[3].x_pos = 20;
    _gstEmListPVInputInfoPara.sub_region[3].y_pos = 134;
    _gstEmListPVInputInfoPara.sub_region[3].x_size = 440;
    _gstEmListPVInputInfoPara.sub_region[3].y_size = 88;
    _gstEmListPVInputInfoPara.sub_region[4].x_pos = 20;
    _gstEmListPVInputInfoPara.sub_region[4].y_pos = 134;
    _gstEmListPVInputInfoPara.sub_region[4].x_size = 440;
    _gstEmListPVInputInfoPara.sub_region[4].y_size = 88;
    _gstEmListPVInputInfoPara.sub_region[5].x_pos = 20;
    _gstEmListPVInputInfoPara.sub_region[5].y_pos = 134;
    _gstEmListPVInputInfoPara.sub_region[5].x_size = 440;
    _gstEmListPVInputInfoPara.sub_region[5].y_size = 88;
#endif

    /*List控件*/

    for(i = 0; i < 12; i++)
    {
        //第i行左边
        _gstEmListPVInputInfoPara.sub_widget_para[0][i].widget_type = EM_WIDGET_TYPE_TEXT;
        _gstEmListPVInputInfoPara.sub_widget_para[0][i].x_relative_pos = 12;
        _gstEmListPVInputInfoPara.sub_widget_para[0][i].y_relative_pos = 10;
        _gstEmListPVInputInfoPara.sub_widget_para[0][i].x_size = 158;
        _gstEmListPVInputInfoPara.sub_widget_para[0][i].y_size = 17;
        _gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content = _garrListPVInputInfoLeftTextBuf[i];
        _gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.text_color = 0x00FFFFFF;
        _gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
        
        strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, _gparrPvNameList[i]);
#if 0       
        if(i == 0)
        {
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV1 Current");
        }
        if(i == 1)
        {
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV1 Voltage");
        }
        if(i == 2)
        {
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV1 Power");
        }
        if(i == 3)
        {
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV2 Current");
        }                                                                                          
        if(i == 4)                                                                         
        {                                                                                          
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV2 Voltage");
        }                                                                                          
        if(i == 5)                                                                                 
        {                                                                                          
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV2 Power");
        }
        if(i == 6)
        {
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV3 Current");
        }                                                                                          
        if(i == 7)                                                                                 
        {                                                                                          
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV3 Voltage");
        }                                                                                          
        if(i == 8)                                                                         
        {                                                                                          
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV3 Power");
        }
        if(i == 9)
        {
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV4 Current");
        }
        if(i == 10)
        {
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV4 Voltage");
        }
        if(i == 11)
        {
            strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][i].sub_widget.text_para.p_content, "PV4 Power");
        }
#endif       
        
        //第i行右边
        _gstEmListPVInputInfoPara.sub_widget_para[1][i].widget_type = EM_WIDGET_TYPE_TEXT;
        _gstEmListPVInputInfoPara.sub_widget_para[1][i].x_relative_pos = 180;
        _gstEmListPVInputInfoPara.sub_widget_para[1][i].y_relative_pos = 10;
        _gstEmListPVInputInfoPara.sub_widget_para[1][i].x_size = 220;
        _gstEmListPVInputInfoPara.sub_widget_para[1][i].y_size = 17;
        _gstEmListPVInputInfoPara.sub_widget_para[1][i].sub_widget.text_para.p_content = _garrListPVInputInfoRightTextBuf[i];
        _gstEmListPVInputInfoPara.sub_widget_para[1][i].sub_widget.text_para.text_color = 0x00FFFFFF;
        _gstEmListPVInputInfoPara.sub_widget_para[1][i].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
        _gstEmListPVInputInfoPara.sub_widget_para[1][i].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据

        _SetPv1Vol(0, _gstUiCtx.pv_ch1_input_volt);
        _SetPv1Cur(0, _gstUiCtx.pv_ch1_input_curr);
        _SetPv1Power(0, _gstUiCtx.pv_ch1_input_power);
        _SetPv2Vol(0, _gstUiCtx.pv_ch2_input_volt);
        _SetPv2Cur(0, _gstUiCtx.pv_ch2_input_curr);
        _SetPv2Power(0, _gstUiCtx.pv_ch2_input_power);
        _SetPv3Vol(0, _gstUiCtx.pv_ch3_input_volt);
        _SetPv3Cur(0, _gstUiCtx.pv_ch3_input_curr);
        _SetPv3Power(0, _gstUiCtx.pv_ch3_input_power);
        _SetPv4Vol(0, _gstUiCtx.pv_ch4_input_volt);
        _SetPv4Cur(0, _gstUiCtx.pv_ch4_input_curr);
        _SetPv4Power(0, _gstUiCtx.pv_ch4_input_power);
    }

#if 0
    //第一行左边
    _gstEmListPVInputInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[0][0].x_relative_pos = 12;
    _gstEmListPVInputInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[0][0].x_size = 158;
    _gstEmListPVInputInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListPVInputInfoLeftTextBuf[0];
    _gstEmListPVInputInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "PV1 Current");
    //第一行右边
    _gstEmListPVInputInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[1][0].x_relative_pos = 200;
    _gstEmListPVInputInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[1][0].x_size = 220;
    _gstEmListPVInputInfoPara.sub_widget_para[1][0].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListPVInputInfoRightTextBuf[0];
    _gstEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    _SetPv1Cur(0, _gstUiCtx.pv_ch1_input_curr);
    //第二行左边
    _gstEmListPVInputInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[0][1].x_relative_pos = 12;
    _gstEmListPVInputInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[0][1].x_size = 158;
    _gstEmListPVInputInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListPVInputInfoLeftTextBuf[1];
    _gstEmListPVInputInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "PV1 Voltage");
    //第二行右边
    _gstEmListPVInputInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[1][1].x_relative_pos = 200;
    _gstEmListPVInputInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[1][1].x_size = 220;
    _gstEmListPVInputInfoPara.sub_widget_para[1][1].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListPVInputInfoRightTextBuf[1];
    _gstEmListPVInputInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListPVInputInfoPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    _SetPv1Vol(0, _gstUiCtx.pv_ch1_input_volt);
    //第三行左边
    _gstEmListPVInputInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[0][2].x_relative_pos = 12;
    _gstEmListPVInputInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[0][2].x_size = 158;
    _gstEmListPVInputInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListPVInputInfoLeftTextBuf[2];
    _gstEmListPVInputInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Power");
    //第三行右边
    _gstEmListPVInputInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[1][2].x_relative_pos = 200;
    _gstEmListPVInputInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[1][2].x_size = 220;
    _gstEmListPVInputInfoPara.sub_widget_para[1][2].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListPVInputInfoRightTextBuf[2];
    _gstEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 3000;//测试数据，实际赋值系统数据
    _SetPv1Power(0, _gstUiCtx.pv_ch1_input_power);

    //第四行左边
    _gstEmListPVInputInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[0][3].x_relative_pos = 12;
    _gstEmListPVInputInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[0][3].x_size = 158;
    _gstEmListPVInputInfoPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListPVInputInfoLeftTextBuf[3];
    _gstEmListPVInputInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Current");
    //第四行右边
    _gstEmListPVInputInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[1][3].x_relative_pos = 200;
    _gstEmListPVInputInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[1][3].x_size = 220;
    _gstEmListPVInputInfoPara.sub_widget_para[1][3].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _garrListPVInputInfoRightTextBuf[3];
    _gstEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    _SetPv2Cur(0, _gstUiCtx.pv_ch2_input_curr);

    //第五行左边
    _gstEmListPVInputInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[0][4].x_relative_pos = 12;
    _gstEmListPVInputInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[0][4].x_size = 158;
    _gstEmListPVInputInfoPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListPVInputInfoLeftTextBuf[4];
    _gstEmListPVInputInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Voltage");
    //第五行右边
    _gstEmListPVInputInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[1][4].x_relative_pos = 200;
    _gstEmListPVInputInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[1][4].x_size = 220;
    _gstEmListPVInputInfoPara.sub_widget_para[1][4].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _garrListPVInputInfoRightTextBuf[4];
    _gstEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    _SetPv2Vol(0, _gstUiCtx.pv_ch2_input_volt);

    //第六行左边
    _gstEmListPVInputInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[0][5].x_relative_pos = 12;
    _gstEmListPVInputInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[0][5].x_size = 158;
    _gstEmListPVInputInfoPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListPVInputInfoLeftTextBuf[5];
    _gstEmListPVInputInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListPVInputInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Power");
    //第六行右边
    _gstEmListPVInputInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[1][5].x_relative_pos = 200;
    _gstEmListPVInputInfoPara.sub_widget_para[1][5].y_relative_pos = 10;
    _gstEmListPVInputInfoPara.sub_widget_para[1][5].x_size = 220;
    _gstEmListPVInputInfoPara.sub_widget_para[1][5].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _garrListPVInputInfoRightTextBuf[5];
    _gstEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.digit_value = 3000;//测试数据，实际赋值系统数据
    _SetPv2Power(0, _gstUiCtx.pv_ch2_input_power);


    //PV1标题
    _gstEmListPVInputInfoPara.sub_widget_para[2][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[2][0].x_relative_pos = 15;
    _gstEmListPVInputInfoPara.sub_widget_para[2][0].y_relative_pos = 37;
    _gstEmListPVInputInfoPara.sub_widget_para[2][0].x_size = 35;
    _gstEmListPVInputInfoPara.sub_widget_para[2][0].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content = _garrListPVInputInfoHeadTextBuf[0];
    _gstEmListPVInputInfoPara.sub_widget_para[2][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListPVInputInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content, "PV1");
    //PV2标题
    _gstEmListPVInputInfoPara.sub_widget_para[2][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListPVInputInfoPara.sub_widget_para[2][3].x_relative_pos = 15;
    _gstEmListPVInputInfoPara.sub_widget_para[2][3].y_relative_pos = 37;
    _gstEmListPVInputInfoPara.sub_widget_para[2][3].x_size = 35;
    _gstEmListPVInputInfoPara.sub_widget_para[2][3].y_size = 17;
    _gstEmListPVInputInfoPara.sub_widget_para[2][3].sub_widget.text_para.p_content = _garrListPVInputInfoHeadTextBuf[1];
    _gstEmListPVInputInfoPara.sub_widget_para[2][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListPVInputInfoPara.sub_widget_para[2][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListPVInputInfoPara.sub_widget_para[2][3].sub_widget.text_para.p_content, "PV2");
#endif
            /*List选中框*/
    _gstEmListPVInputInfoPara.selected_unvalid_color = 0xFF000000;
    _gstEmListPVInputInfoPara.selected_valid_color = 0x00D6E400;
    _gstEmListPVInputInfoPara.selected_size = 3;
    _gstEmListPVInputInfoPara.selected_r = 10;
    _gstEmListPVInputInfoPara.selected_x_start_offset = -2;
    _gstEmListPVInputInfoPara.selected_y_start_offset = -2;
    _gstEmListPVInputInfoPara.selected_x_end_offset = 2;
    _gstEmListPVInputInfoPara.selected_y_end_offset = 2;
//    _gstEmListPVInputInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListPVInputInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
        /*滚动条*/
    _gstEmListPVInputInfoPara.scrollbar_enable = true;
    _gstEmListPVInputInfoPara.scrollbar_color = 0x00CDF100;
    _gstEmListPVInputInfoPara.scrollbar_bk_color = 0x00565656;
    _gstEmListPVInputInfoPara.scrollbar_x_pos = 457;
    _gstEmListPVInputInfoPara.scrollbar_y_pos = 39;
    _gstEmListPVInputInfoPara.scrollbar_x_size = 6;
    _gstEmListPVInputInfoPara.scrollbar_y_size = 240; 
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gstReturnImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    //pv1
    if (_gstUiCtx.pv_ch1_input_volt != gstCollectInfo.power_data.power_value_info.pv_1_vol)
    {
        _gstUiCtx.pv_ch1_input_volt = gstCollectInfo.power_data.power_value_info.pv_1_vol;
        _SetPv1Vol(1, _gstUiCtx.pv_ch1_input_volt);
    }
    if (_gstUiCtx.pv_ch1_input_curr != gstCollectInfo.power_data.power_value_info.pv_1_current)
    {
        _gstUiCtx.pv_ch1_input_curr = gstCollectInfo.power_data.power_value_info.pv_1_current;
        _SetPv1Cur(1, _gstUiCtx.pv_ch1_input_curr);
    }
    if (_gstUiCtx.pv_ch1_input_power != gstCollectInfo.power_data.power_value_info.pv_1_power)
    {
        _gstUiCtx.pv_ch1_input_power = gstCollectInfo.power_data.power_value_info.pv_1_power;
        _SetPv1Power(1, _gstUiCtx.pv_ch1_input_power);
    }
    //pv2
    if (_gstUiCtx.pv_ch2_input_volt != gstCollectInfo.power_data.power_value_info.pv_2_vol)
    { 
        _gstUiCtx.pv_ch2_input_volt = gstCollectInfo.power_data.power_value_info.pv_2_vol;
        _SetPv2Vol(1, _gstUiCtx.pv_ch2_input_volt);
    }
    if (_gstUiCtx.pv_ch2_input_curr != gstCollectInfo.power_data.power_value_info.pv_2_current)
    {
        _gstUiCtx.pv_ch2_input_curr = gstCollectInfo.power_data.power_value_info.pv_2_current;
        _SetPv2Cur(1, _gstUiCtx.pv_ch2_input_curr);
    }
    if (_gstUiCtx.pv_ch2_input_power != gstCollectInfo.power_data.power_value_info.pv_2_power)
    {
        _gstUiCtx.pv_ch2_input_power = gstCollectInfo.power_data.power_value_info.pv_2_power;
        _SetPv2Power(1, _gstUiCtx.pv_ch2_input_power);
    }
    //pv3
    if (_gstUiCtx.pv_ch3_input_volt != gstCollectInfo.power_data.power_value_info.pv_3_vol)
    {
        _gstUiCtx.pv_ch3_input_volt = gstCollectInfo.power_data.power_value_info.pv_3_vol;
        _SetPv3Vol(1, _gstUiCtx.pv_ch3_input_volt);
    }
    if (_gstUiCtx.pv_ch3_input_curr != gstCollectInfo.power_data.power_value_info.pv_3_current)
    {
        _gstUiCtx.pv_ch3_input_curr = gstCollectInfo.power_data.power_value_info.pv_3_current;
        _SetPv3Cur(1, _gstUiCtx.pv_ch3_input_curr);
    }
    if (_gstUiCtx.pv_ch3_input_power != gstCollectInfo.power_data.power_value_info.pv_3_power)
    {
        _gstUiCtx.pv_ch3_input_power = gstCollectInfo.power_data.power_value_info.pv_3_power;
        _SetPv3Power(1, _gstUiCtx.pv_ch3_input_power);
    }
    //pv4
    if (_gstUiCtx.pv_ch4_input_volt != gstCollectInfo.power_data.power_value_info.pv_4_vol)
    { 
        _gstUiCtx.pv_ch4_input_volt = gstCollectInfo.power_data.power_value_info.pv_4_vol;
        _SetPv4Vol(1, _gstUiCtx.pv_ch4_input_volt);
    }    
    if (_gstUiCtx.pv_ch4_input_curr != gstCollectInfo.power_data.power_value_info.pv_4_current)
    {
        _gstUiCtx.pv_ch4_input_curr = gstCollectInfo.power_data.power_value_info.pv_4_current;
        _SetPv4Cur(1, _gstUiCtx.pv_ch4_input_curr);
    }
    if (_gstUiCtx.pv_ch4_input_power != gstCollectInfo.power_data.power_value_info.pv_4_power)
    {
        _gstUiCtx.pv_ch4_input_power = gstCollectInfo.power_data.power_value_info.pv_4_power;
        _SetPv4Power(1, _gstUiCtx.pv_ch4_input_power);
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
        EmListUpward(&_gstEmListPVInputInfoPara, gstEmWinListPVInputInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListPVInputInfoPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListPVInputInfoPara, gstEmWinListPVInputInfo.win_handle);
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

