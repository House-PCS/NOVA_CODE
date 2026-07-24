/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-12-18
  * @brief       启动界面
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-12-18    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define  EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE                (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListEquipOperaInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t bus_voltage;
} UiCtx_t;

static UiCtx_t gstUiCtx;


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;

static char _garrTitleBuf[] = "Operational State Info";

/*List控件*/
static emListPara_t  _gstEmListEquipOperaInfoPara;

static char _garrListEquipOperaInfoLeftTextBuf[11][20];
static char _garrListEquipOperaInfoRightText3Buf[8];
static GUI_HMEM _gListEquipOperaInfoRightMem;
static uint8_t *_pListEquipOperaInfoRightBuf;

#define WIN_LIST_EQUIP_OPERA_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_EQUIP_OPERA_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListEquipOperaInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListEquipOperaInfoWin",    WIN_LIST_EQUIP_OPERA_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_EQUIP_OPERA_INFO_TITLE_ID,     31,  20, 200,  20,   0,   0x0,  0 },  //144, 14
};


static int32_t _SetBusVoltage(int8_t ui_update, uint16_t vol)
{
    int16_t vol_tmp = 0;
    vol_tmp = (int16_t)vol;
    float voltage = (float)vol_tmp / 10;
    snprintf(_garrListEquipOperaInfoRightText3Buf, sizeof(_garrListEquipOperaInfoRightText3Buf), "%.1fV", voltage);

//  value *= 100;
//
//  _gstEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.digit_value = value / 100;
//  sprintf(_gstEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.p_content, "%.1fV", \
//          _gstEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.digit_value);
//
    if(ui_update && _gstEmListEquipOperaInfoPara.list_page == 4)
    {
        TEXT_SetText(_gstEmListEquipOperaInfoPara.handle[1][5], (const char *)_gstEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.text_para.p_content);
    }

    return 0;
}



/**
  * @brief ListEquipOperaInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListEquipOperaInfoWin(WM_MESSAGE* p_msg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (p_msg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(p_msg->hWin, WIN_LIST_EQUIP_OPERA_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListEquipOperaInfoPara, p_msg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListEquipOperaInfoPara, _gstEmListEquipOperaInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色

            for(i = 0; i < _gstEmListEquipOperaInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListEquipOperaInfoPara.sub_region[i].x_pos,
                                    _gstEmListEquipOperaInfoPara.sub_region[i].y_pos,
                                    _gstEmListEquipOperaInfoPara.sub_region[i].x_pos + _gstEmListEquipOperaInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListEquipOperaInfoPara.sub_region[i].y_pos + _gstEmListEquipOperaInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListEquipOperaInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListEquipOperaInfoPara, _gstEmListEquipOperaInfoPara.list_sel);
            break;
        }
        case WM_NOTIFY_PARENT:
        {
            break;
        }
        default:
        {
            WM_DefaultProc(p_msg);
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
    gstUiCtx.key_init = 0;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListEquipOperaInfo.win_handle = GUI_CreateDialogBox(_garrListEquipOperaInfoDialogCreate, GUI_COUNTOF(_garrListEquipOperaInfoDialogCreate),
                                            _cbListEquipOperaInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListEquipOperaInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListEquipOperaInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListEquipOperaInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListEquipOperaInfoPara, gstEmWinListEquipOperaInfo.win_handle);
    return gstEmWinListEquipOperaInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListEquipOperaInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListEquipOperaInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListEquipOperaInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;
    uint8_t i;
    uint8_t j;

    EmKeyDefaultInfoSet();

    gstUiCtx.bus_voltage = gstCollectInfo.power_data.power_value_info.bus_vol;

    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00EEEEEE;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListEquipOperaInfoPara.list_start_id = WIN_LIST_EQUIP_OPERA_INFO_ID + 0x30;
    _gstEmListEquipOperaInfoPara.list_num = 9;//10;//11
    _gstEmListEquipOperaInfoPara.list_display_num = 6;
    if(_gstEmListEquipOperaInfoPara.list_num <= _gstEmListEquipOperaInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListEquipOperaInfoPara.list_num + 1 - _gstEmListEquipOperaInfoPara.list_display_num;
    }


    if(_gstEmListEquipOperaInfoPara.list_sel > _gstEmListEquipOperaInfoPara.list_num - 1)
    {
        _gstEmListEquipOperaInfoPara.list_sel = 0;
    }
    if(_gstEmListEquipOperaInfoPara.list_page > max_page - 1)
    {
        _gstEmListEquipOperaInfoPara.list_page = 0;
    }
    _gstEmListEquipOperaInfoPara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gstEmListEquipOperaInfoPara.sub_region[0].x_pos = 20;
    _gstEmListEquipOperaInfoPara.sub_region[0].y_pos = 38;
    _gstEmListEquipOperaInfoPara.sub_region[0].x_size = 420;
    _gstEmListEquipOperaInfoPara.sub_region[0].y_size = 34;
    _gstEmListEquipOperaInfoPara.sub_region[1].x_pos = 20;
    _gstEmListEquipOperaInfoPara.sub_region[1].y_pos = 80;
    _gstEmListEquipOperaInfoPara.sub_region[1].x_size = 420;
    _gstEmListEquipOperaInfoPara.sub_region[1].y_size = 34;
    _gstEmListEquipOperaInfoPara.sub_region[2].x_pos = 20;
    _gstEmListEquipOperaInfoPara.sub_region[2].y_pos = 122;
    _gstEmListEquipOperaInfoPara.sub_region[2].x_size = 420;
    _gstEmListEquipOperaInfoPara.sub_region[2].y_size = 34;
    _gstEmListEquipOperaInfoPara.sub_region[3].x_pos = 20;
    _gstEmListEquipOperaInfoPara.sub_region[3].y_pos = 164;
    _gstEmListEquipOperaInfoPara.sub_region[3].x_size = 420;
    _gstEmListEquipOperaInfoPara.sub_region[3].y_size = 34;
    _gstEmListEquipOperaInfoPara.sub_region[4].x_pos = 20;
    _gstEmListEquipOperaInfoPara.sub_region[4].y_pos = 206;
    _gstEmListEquipOperaInfoPara.sub_region[4].x_size = 420;
    _gstEmListEquipOperaInfoPara.sub_region[4].y_size = 34;
    _gstEmListEquipOperaInfoPara.sub_region[5].x_pos = 20;
    _gstEmListEquipOperaInfoPara.sub_region[5].y_pos = 248;
    _gstEmListEquipOperaInfoPara.sub_region[5].x_size = 420;
    _gstEmListEquipOperaInfoPara.sub_region[5].y_size = 34;

    /*List控件*/
    _gListEquipOperaInfoRightMem = GUI_ALLOC_AllocZero(EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE);
    _pListEquipOperaInfoRightBuf = GUI_ALLOC_h2p(_gListEquipOperaInfoRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListEquipOperaInfoRightBuf, EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE);


    //第一行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][0].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][0].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[0];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "PV Input");
    //第一行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][0].x_relative_pos = 395;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][0].x_size = 5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][0].y_size = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第二行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][1].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][1].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[1];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Grid");
    //第二行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][1].x_relative_pos = 395;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][1].x_size = 5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][1].y_size = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第三行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][2].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][2].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[2];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Generator");
    //第三行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][2].x_relative_pos = 395;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][2].y_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][2].x_size = 5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][2].y_size = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第四行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][3].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][3].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[3];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Backup Load");
    //第四行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][3].x_relative_pos = 395;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][3].y_relative_pos = 13;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][3].x_size = 5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][3].y_size = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第五行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][4].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][4].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[4];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "On-Grid Load");
    //第五行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][4].x_relative_pos = 395;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][4].y_relative_pos = 13;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][4].x_size = 5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][4].y_size = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第六行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][5].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][5].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[5];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Battery Pack");
    //第六行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][5].x_relative_pos = 395;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][5].y_relative_pos = 13;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][5].x_size = 5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][5].y_size = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
#if 0
    //第七行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[6];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Inverter");
    //第七行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].x_relative_pos = 395;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].y_relative_pos = 13;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].x_size = 5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].y_size = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
#endif
    //第八行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[6];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Temperature");
    //第八行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].x_relative_pos = 395;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].y_relative_pos = 13;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].x_size = 5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].y_size = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第九行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][7].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][7].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][7].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][7].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[7];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Status");
    //第九行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][7].x_relative_pos = 395;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][7].y_relative_pos = 13;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][7].x_size = 5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][7].y_size = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
#if 0    
    //第十行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[8];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_content, "System Settings");
    //第十行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].x_relative_pos = 395;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].y_relative_pos = 13;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].x_size = 5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].y_size = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
#endif
    //第十一行左边
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].x_relative_pos = 12;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].y_relative_pos = 10;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].x_size = 200;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_content = _garrListEquipOperaInfoLeftTextBuf[8];
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_content, "Bus Voltage");
    //第十一行右边
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].x_relative_pos = 203;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].y_relative_pos = 11;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].x_size = 197;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].y_size = 17;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.text_para.p_content = _garrListEquipOperaInfoRightText3Buf;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.text_para.text_color = 0x00E5E5E5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gstEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.text_para.digit_value = 200;//测试数据，实际赋值系统数据
    _SetBusVoltage(0, gstUiCtx.bus_voltage);

    /*List选中框*/
    _gstEmListEquipOperaInfoPara.selected_unvalid_color = 0xFF000000;
    _gstEmListEquipOperaInfoPara.selected_valid_color = 0x00D6E400;
    _gstEmListEquipOperaInfoPara.selected_size = 3;
    _gstEmListEquipOperaInfoPara.selected_r = 10;
    _gstEmListEquipOperaInfoPara.selected_x_start_offset = -2;
    _gstEmListEquipOperaInfoPara.selected_y_start_offset = -2;
    _gstEmListEquipOperaInfoPara.selected_x_end_offset = 2;
    _gstEmListEquipOperaInfoPara.selected_y_end_offset = 2;
//    _gstEmListEquipOperaInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListEquipOperaInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    /*滚动条*/
    _gstEmListEquipOperaInfoPara.scrollbar_enable = true;
    _gstEmListEquipOperaInfoPara.scrollbar_color = 0x00CDF100;
    _gstEmListEquipOperaInfoPara.scrollbar_bk_color = 0x00565656;
    _gstEmListEquipOperaInfoPara.scrollbar_x_pos = 457;
    _gstEmListEquipOperaInfoPara.scrollbar_y_pos = 39;
    _gstEmListEquipOperaInfoPara.scrollbar_x_size = 6;
    _gstEmListEquipOperaInfoPara.scrollbar_y_size = 240;
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gListEquipOperaInfoRightMem);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    static uint32_t timecount = 0;
    static uint8_t dir = 0;
    if(gstEmWinListEquipOperaInfo.win_handle == 0)
    {
        return;
    }

    if(gstUiCtx.bus_voltage != gstCollectInfo.power_data.power_value_info.bus_vol)
    {
        gstUiCtx.bus_voltage = gstCollectInfo.power_data.power_value_info.bus_vol;
        _SetBusVoltage(1, gstUiCtx.bus_voltage);
    }


#if 0
    if(GetSystick() - timecount > 100)    //30
    {
        timecount = GetSystick();
        if(_gstEmListEquipOperaInfoPara.list_sel + _gstEmListEquipOperaInfoPara.list_page >= _gstEmListEquipOperaInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListEquipOperaInfoPara.list_sel + _gstEmListEquipOperaInfoPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListEquipOperaInfoPara, gstEmWinListEquipOperaInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListEquipOperaInfoPara, gstEmWinListEquipOperaInfo.win_handle);
        }
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

    if (event == EN_KEY_SRC_LEFT)
    {
        --gstUiCtx.select;
        if (gstUiCtx.select < 0)
        {
            ++gstUiCtx.select;
            return;
        }
        EmListUpward(&_gstEmListEquipOperaInfoPara, gstEmWinListEquipOperaInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++gstUiCtx.select;
        if (gstUiCtx.select >= _gstEmListEquipOperaInfoPara.list_num)
        {
            --gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListEquipOperaInfoPara, gstEmWinListEquipOperaInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        emWin_t *pnext = NULL;
        switch(gstUiCtx.select)
        {
            case 0:
                pnext = &gstEmWinListPVInputInfo;
                break;
            case 1:     
                gVoltCurrPowerInfoIdx = 0;                
                pnext = &gstEmWinListVoltCurrPowerInfo;
                break;
            case 2:
                gVoltCurrPowerInfoIdx = 1;
                pnext = &gstEmWinListVoltCurrPowerInfo;
                break;
            case 3:
                gVoltCurrPowerInfoIdx = 2;
                pnext = &gstEmWinListVoltCurrPowerInfo;
                break;
            case 4:
                gVoltCurrPowerInfoIdx = 3;
                pnext = &gstEmWinListVoltCurrPowerInfo;
                break;
            case 5:
                pnext = &gstEmWinListBatteryInfo;
                break;
#if 0
            case 6:
                pnext = &gstEmWinListInverterInfo;
                break;
#endif
            case 6:
                pnext = &gstEmWinListTemperatureInfo;
                break;
            case 7:
                pnext = &gstEmWinListStatusInfo;
                break;
#if 0
            case 8:
                pnext = &gstEmWinListSystemSettingsInfo;
                break;
#endif
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
                pnext = &gstEmWinMain3DNav;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}


