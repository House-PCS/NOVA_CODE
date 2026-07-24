/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_run_setting.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-01-18
  * @brief       高级设置->操作参数设置
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-18    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define  EM_LIST_RUN_SETTING_ICON_ARROW_SIZE                (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListRunSettingInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};



/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;
static char _garrTitleBuf[] = "Setting < Run Setting";

/*List控件*/
static emListPara_t  _gstEmListRunSettingInfoPara;
static char _garrListRunSettingInfoLeftTextBuf[13][28];
static GUI_HMEM _gListRunSettingInfoRightMem;
static uint8_t *_pListRunSettingInfoRightBuf;

#define WIN_LIST_RUN_SETTING_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_RUN_SETTING_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListRunSettingInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListRunSettingInfoWin",    WIN_LIST_RUN_SETTING_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_RUN_SETTING_INFO_TITLE_ID,     31,  20, 200,  20,   0,   0x0,  0 },  //144, 14
};

/**
  * @brief ListRunSettingInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListRunSettingInfoWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    int32_t n_code;
    int32_t id;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_RUN_SETTING_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListRunSettingInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListRunSettingInfoPara, _gstEmListRunSettingInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色

            for(i = 0; i < _gstEmListRunSettingInfoPara.list_display_num ; i++)
            {
                GUI_FillRoundedRect(_gstEmListRunSettingInfoPara.sub_region[i].x_pos,
                                    _gstEmListRunSettingInfoPara.sub_region[i].y_pos,
                                    _gstEmListRunSettingInfoPara.sub_region[i].x_pos + _gstEmListRunSettingInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListRunSettingInfoPara.sub_region[i].y_pos + _gstEmListRunSettingInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListRunSettingInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListRunSettingInfoPara, _gstEmListRunSettingInfoPara.list_sel);
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
    _Init();
#if 1
    _gstEmListRunSettingInfoPara.list_x_offset = 0;
    _gstEmListRunSettingInfoPara.list_y_offset = 0;
#endif
//    GUI_EnableAlpha(1);
    gstEmWinListRunSettingInfo.win_handle = GUI_CreateDialogBox(_garrListRunSettingInfoDialogCreate, GUI_COUNTOF(_garrListRunSettingInfoDialogCreate), _cbListRunSettingInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListRunSettingInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListRunSettingInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListRunSettingInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListRunSettingInfoPara, gstEmWinListRunSettingInfo.win_handle);
    return gstEmWinListRunSettingInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListRunSettingInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListRunSettingInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListRunSettingInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;

    /*标题参数初始化*/
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00A4A4A4;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListRunSettingInfoPara.list_start_id = WIN_LIST_RUN_SETTING_INFO_ID + 0x30;
    _gstEmListRunSettingInfoPara.list_num = 13;
    _gstEmListRunSettingInfoPara.list_display_num = 6;
    if(_gstEmListRunSettingInfoPara.list_num <= _gstEmListRunSettingInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListRunSettingInfoPara.list_num + 1 - _gstEmListRunSettingInfoPara.list_display_num;
    }


    if(_gstEmListRunSettingInfoPara.list_sel > _gstEmListRunSettingInfoPara.list_num - 1)
    {
        _gstEmListRunSettingInfoPara.list_sel = 0;
    }
    if(_gstEmListRunSettingInfoPara.list_page > max_page - 1)
    {
        _gstEmListRunSettingInfoPara.list_page = 0;
    }
    _gstEmListRunSettingInfoPara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gstEmListRunSettingInfoPara.sub_region[0].x_pos = 20;
    _gstEmListRunSettingInfoPara.sub_region[0].y_pos = 38;
    _gstEmListRunSettingInfoPara.sub_region[0].x_size = 420;
    _gstEmListRunSettingInfoPara.sub_region[0].y_size = 34;
    _gstEmListRunSettingInfoPara.sub_region[1].x_pos = 20;
    _gstEmListRunSettingInfoPara.sub_region[1].y_pos = 80;
    _gstEmListRunSettingInfoPara.sub_region[1].x_size = 420;
    _gstEmListRunSettingInfoPara.sub_region[1].y_size = 34;
    _gstEmListRunSettingInfoPara.sub_region[2].x_pos = 20;
    _gstEmListRunSettingInfoPara.sub_region[2].y_pos = 122;
    _gstEmListRunSettingInfoPara.sub_region[2].x_size = 420;
    _gstEmListRunSettingInfoPara.sub_region[2].y_size = 34;
    _gstEmListRunSettingInfoPara.sub_region[3].x_pos = 20;
    _gstEmListRunSettingInfoPara.sub_region[3].y_pos = 164;
    _gstEmListRunSettingInfoPara.sub_region[3].x_size = 420;
    _gstEmListRunSettingInfoPara.sub_region[3].y_size = 34;
    _gstEmListRunSettingInfoPara.sub_region[4].x_pos = 20;
    _gstEmListRunSettingInfoPara.sub_region[4].y_pos = 206;
    _gstEmListRunSettingInfoPara.sub_region[4].x_size = 420;
    _gstEmListRunSettingInfoPara.sub_region[4].y_size = 34;
    _gstEmListRunSettingInfoPara.sub_region[5].x_pos = 20;
    _gstEmListRunSettingInfoPara.sub_region[5].y_pos = 248;
    _gstEmListRunSettingInfoPara.sub_region[5].x_size = 420;
    _gstEmListRunSettingInfoPara.sub_region[5].y_size = 34;

    /*List控件*/
    _gListRunSettingInfoRightMem = GUI_ALLOC_AllocZero(EM_LIST_RUN_SETTING_ICON_ARROW_SIZE);
    _pListRunSettingInfoRightBuf = GUI_ALLOC_h2p(_gListRunSettingInfoRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListRunSettingInfoRightBuf, EM_LIST_RUN_SETTING_ICON_ARROW_SIZE);


    //第一行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][0].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[0];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "On-Grid Power");
    //第一行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][0].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][0].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][0].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
    //第二行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][1].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[1];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "React Mode");
    //第二行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][1].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][1].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][1].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
    //第三行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][2].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[2];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Max Voltage");
    //第三行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][2].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][2].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][2].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][2].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
    //第四行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][3].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[3];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Min Voltage");
    //第四行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][3].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][3].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][3].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][3].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
    //第五行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][4].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[4];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Max Frequency");
    //第五行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][4].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][4].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][4].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][4].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
    //第六行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][5].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[5];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Min Frequency");
    //第六行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][5].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][5].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][5].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][5].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
    //第七行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][6].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][6].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][6].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][6].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[6];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Over Voltage Derating");
    //第七行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][6].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][6].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][6].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][6].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
    //第八行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][7].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][7].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][7].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][7].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[7];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Under Voltage Derating");
    //第八行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][7].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][7].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][7].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][7].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
    //第九行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][8].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][8].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][8].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][8].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[8];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_content, "Over Frequency Derating");
    //第九行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][8].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][8].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][8].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][8].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][8].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][8].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][8].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][8].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
    //第十行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][9].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][9].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][9].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][9].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][9].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][9].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[9];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][9].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][9].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][9].sub_widget.text_para.p_content, "Under Frequency Derating");
    //第十行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][9].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][9].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][9].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][9].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][9].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][9].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][9].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][9].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;

    //第十一行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][10].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][10].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][10].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][10].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][10].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][10].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[10];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][10].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][10].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][10].sub_widget.text_para.p_content, "Reactive Response Time");
    //第十一行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][10].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][10].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][10].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][10].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][10].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][10].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][10].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][10].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
#if 1
    //第十二行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][11].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][11].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][11].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][11].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][11].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][11].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[11];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][11].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][11].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][11].sub_widget.text_para.p_content, "Grid Drop Switching Method");
    //第十二行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][11].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][11].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][11].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][11].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][11].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][11].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][11].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][11].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;

    //第十三行左边
    _gstEmListRunSettingInfoPara.sub_widget_para[0][12].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][12].x_relative_pos = 11;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][12].y_relative_pos = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][12].x_size = 280;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][12].y_size = 17;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][12].sub_widget.text_para.p_content = _garrListRunSettingInfoLeftTextBuf[12];
    _gstEmListRunSettingInfoPara.sub_widget_para[0][12].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListRunSettingInfoPara.sub_widget_para[0][12].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListRunSettingInfoPara.sub_widget_para[0][12].sub_widget.text_para.p_content, "Power Conversion Ratio");
    //第十三行右边
    _gstEmListRunSettingInfoPara.sub_widget_para[1][12].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][12].x_relative_pos = 394;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][12].y_relative_pos = 12;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][12].x_size = 5;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][12].y_size = 10;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][12].sub_widget.img_para.mem_img = _gListRunSettingInfoRightMem;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][12].sub_widget.img_para.p_img_buf = _pListRunSettingInfoRightBuf;
    _gstEmListRunSettingInfoPara.sub_widget_para[1][12].sub_widget.img_para.img_size = EM_LIST_RUN_SETTING_ICON_ARROW_SIZE;
#endif

    /*List选中框*/
    _gstEmListRunSettingInfoPara.selected_unvalid_color = 0xFF000000;
    _gstEmListRunSettingInfoPara.selected_valid_color = 0x00D6E400;
    _gstEmListRunSettingInfoPara.selected_size = 3;
    _gstEmListRunSettingInfoPara.selected_r = 10;
    _gstEmListRunSettingInfoPara.selected_x_start_offset = -2;
    _gstEmListRunSettingInfoPara.selected_y_start_offset = -2;
    _gstEmListRunSettingInfoPara.selected_x_end_offset = 2;
    _gstEmListRunSettingInfoPara.selected_y_end_offset = 2;
//    _gstEmListRunSettingInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListRunSettingInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    /*滚动条*/
    _gstEmListRunSettingInfoPara.scrollbar_enable = true;
    _gstEmListRunSettingInfoPara.scrollbar_color = 0x00CDF100;
    _gstEmListRunSettingInfoPara.scrollbar_bk_color = 0x00565656;
    _gstEmListRunSettingInfoPara.scrollbar_x_pos = 457;
    _gstEmListRunSettingInfoPara.scrollbar_y_pos = 39;
    _gstEmListRunSettingInfoPara.scrollbar_x_size = 6;
    _gstEmListRunSettingInfoPara.scrollbar_y_size = 240;

}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gListRunSettingInfoRightMem);
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
    if(gstEmWinListRunSettingInfo.win_handle == 0)
    {
        return;
    }
    if(GetSystick() - timecount > 500)    //30
    {
        timecount = GetSystick();
        if(_gstEmListRunSettingInfoPara.list_sel + _gstEmListRunSettingInfoPara.list_page >= _gstEmListRunSettingInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListRunSettingInfoPara.list_sel + _gstEmListRunSettingInfoPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListRunSettingInfoPara, gstEmWinListRunSettingInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListRunSettingInfoPara, gstEmWinListRunSettingInfo.win_handle);
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

}


