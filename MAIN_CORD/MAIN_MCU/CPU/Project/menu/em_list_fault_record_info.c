/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_LIST_BATTERY_PACK__VERSION_INFO.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-25
  * @brief       故障记录
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


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListFaultRecordsInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
} UiCtx_t;



/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;
static char _garrTitleBuf[] = "Basic Info < Operational Records";

/*List控件*/
static emListPara_t  _gstEmListFaultRecordsInfoPara;
static char _garrListFaultRecordsInfoLeftTextBuf[6][20];
static char _garrListFaultRecordsInfoMidTextBuf[6][20];
static char _garrListFaultRecordsInfoRightTextBuf[6][20];


#define WIN_LIST_FAULT_RECORD_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_FAULT_RECORD_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListFaultRecordsInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListFaultRecordsInfoWin",    WIN_LIST_FAULT_RECORD_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_FAULT_RECORD_INFO_TITLE_ID,     31,  20, 415,  17,   0,   0x0,  0 },
};


/**
  * @brief ListFaultRecordsInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListFaultRecordsInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_FAULT_RECORD_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListFaultRecordsInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListFaultRecordsInfoPara, _gstEmListFaultRecordsInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(i = 0; i < _gstEmListFaultRecordsInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListFaultRecordsInfoPara.sub_region[i].x_pos,
                                    _gstEmListFaultRecordsInfoPara.sub_region[i].y_pos,
                                    _gstEmListFaultRecordsInfoPara.sub_region[i].x_pos + _gstEmListFaultRecordsInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListFaultRecordsInfoPara.sub_region[i].y_pos + _gstEmListFaultRecordsInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListFaultRecordsInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListFaultRecordsInfoPara, _gstEmListFaultRecordsInfoPara.list_sel);
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
    gstEmWinListFaultRecordsInfo.win_handle = GUI_CreateDialogBox(_garrListFaultRecordsInfoDialogCreate, GUI_COUNTOF(_garrListFaultRecordsInfoDialogCreate), _cbListFaultRecordsInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListFaultRecordsInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListFaultRecordsInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListFaultRecordsInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListFaultRecordsInfoPara, gstEmWinListFaultRecordsInfo.win_handle);
    return gstEmWinListFaultRecordsInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListFaultRecordsInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListFaultRecordsInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListFaultRecordsInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;

    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListFaultRecordsInfoPara.list_start_id = WIN_LIST_FAULT_RECORD_INFO_ID + 0x30;
    _gstEmListFaultRecordsInfoPara.list_num = 6;
    _gstEmListFaultRecordsInfoPara.list_display_num = 6;
    if(_gstEmListFaultRecordsInfoPara.list_num <= _gstEmListFaultRecordsInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListFaultRecordsInfoPara.list_num + 1 - _gstEmListFaultRecordsInfoPara.list_display_num;
    }

    if(_gstEmListFaultRecordsInfoPara.list_sel > _gstEmListFaultRecordsInfoPara.list_num - 1)
    {
        _gstEmListFaultRecordsInfoPara.list_sel = 0;
    }
    if(_gstEmListFaultRecordsInfoPara.list_page > max_page - 1)
    {
        _gstEmListFaultRecordsInfoPara.list_page = 0;
    }
    _gstEmListFaultRecordsInfoPara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gstEmListFaultRecordsInfoPara.sub_region[0].x_pos = 20;
    _gstEmListFaultRecordsInfoPara.sub_region[0].y_pos = 38;
    _gstEmListFaultRecordsInfoPara.sub_region[0].x_size = 440;
    _gstEmListFaultRecordsInfoPara.sub_region[0].y_size = 34;
    _gstEmListFaultRecordsInfoPara.sub_region[1].x_pos = 20;
    _gstEmListFaultRecordsInfoPara.sub_region[1].y_pos = 80;
    _gstEmListFaultRecordsInfoPara.sub_region[1].x_size = 440;
    _gstEmListFaultRecordsInfoPara.sub_region[1].y_size = 34;
    _gstEmListFaultRecordsInfoPara.sub_region[2].x_pos = 20;
    _gstEmListFaultRecordsInfoPara.sub_region[2].y_pos = 122;
    _gstEmListFaultRecordsInfoPara.sub_region[2].x_size = 440;
    _gstEmListFaultRecordsInfoPara.sub_region[2].y_size = 34;
    _gstEmListFaultRecordsInfoPara.sub_region[3].x_pos = 20;
    _gstEmListFaultRecordsInfoPara.sub_region[3].y_pos = 164;
    _gstEmListFaultRecordsInfoPara.sub_region[3].x_size = 440;
    _gstEmListFaultRecordsInfoPara.sub_region[3].y_size = 34;
    _gstEmListFaultRecordsInfoPara.sub_region[4].x_pos = 20;
    _gstEmListFaultRecordsInfoPara.sub_region[4].y_pos = 206;
    _gstEmListFaultRecordsInfoPara.sub_region[4].x_size = 440;
    _gstEmListFaultRecordsInfoPara.sub_region[4].y_size = 34;
    _gstEmListFaultRecordsInfoPara.sub_region[5].x_pos = 20;
    _gstEmListFaultRecordsInfoPara.sub_region[5].y_pos = 246;
    _gstEmListFaultRecordsInfoPara.sub_region[5].x_size = 440;
    _gstEmListFaultRecordsInfoPara.sub_region[5].y_size = 34;

    /*List控件*/

    //第一行左边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][0].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][0].x_size = 9;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListFaultRecordsInfoLeftTextBuf[0];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "1");
    //第一行中间
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][0].x_relative_pos = 31;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][0].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][0].x_size = 60;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][0].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListFaultRecordsInfoMidTextBuf[0];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "W_001");
    //第一行右边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][0].x_relative_pos = 180;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][0].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][0].x_size = 248;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][0].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content = _garrListFaultRecordsInfoRightTextBuf[0];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content, "2024-12-01 12:00:00");
    //第二行左边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][1].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][1].x_size = 9;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListFaultRecordsInfoLeftTextBuf[1];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "2");
    //第二行中间
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][1].x_relative_pos = 31;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][1].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][1].x_size = 60;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][1].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListFaultRecordsInfoMidTextBuf[1];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "W_001");
    //第二行右边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][1].x_relative_pos = 180;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][1].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][1].x_size = 248;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][1].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content = _garrListFaultRecordsInfoRightTextBuf[1];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content, "2024-12-01 12:00:00");
    //第三行左边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][2].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][2].x_size = 9;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListFaultRecordsInfoLeftTextBuf[2];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "3");
    //第三行中间
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][2].x_relative_pos = 31;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][2].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][2].x_size = 60;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][2].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListFaultRecordsInfoMidTextBuf[2];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content, "W_001");
    //第三行右边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][2].x_relative_pos = 180;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][2].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][2].x_size = 248;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][2].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][2].sub_widget.text_para.p_content = _garrListFaultRecordsInfoRightTextBuf[2];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[2][2].sub_widget.text_para.p_content, "2024-12-01 12:00:00");
    //第四行左边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][3].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][3].x_size = 9;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListFaultRecordsInfoLeftTextBuf[3];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "4");
    //第四行中间
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][3].x_relative_pos = 31;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][3].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][3].x_size = 60;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][3].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _garrListFaultRecordsInfoMidTextBuf[3];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content, "W_001");
    //第四行右边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][3].x_relative_pos = 180;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][3].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][3].x_size = 248;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][3].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][3].sub_widget.text_para.p_content = _garrListFaultRecordsInfoRightTextBuf[3];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[2][3].sub_widget.text_para.p_content, "2024-12-01 12:00:00");
    //第五行左边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][4].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][4].x_size = 9;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListFaultRecordsInfoLeftTextBuf[4];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "5");
    //第五行中间
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][4].x_relative_pos = 31;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][4].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][4].x_size = 60;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][4].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _garrListFaultRecordsInfoMidTextBuf[4];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content, "W_001");
    //第五行右边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][4].x_relative_pos = 180;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][4].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][4].x_size = 248;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][4].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][4].sub_widget.text_para.p_content = _garrListFaultRecordsInfoRightTextBuf[4];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[2][4].sub_widget.text_para.p_content, "2024-12-01 12:00:00");
    //第六行左边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][5].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][5].x_size = 9;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListFaultRecordsInfoLeftTextBuf[5];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "6");
    //第六行中间
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][5].x_relative_pos = 31;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][5].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][5].x_size = 60;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][5].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _garrListFaultRecordsInfoMidTextBuf[5];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content, "W_001");
    //第六行右边
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][5].x_relative_pos = 180;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][5].y_relative_pos = 11;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][5].x_size = 248;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][5].y_size = 17;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][5].sub_widget.text_para.p_content = _garrListFaultRecordsInfoRightTextBuf[5];
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListFaultRecordsInfoPara.sub_widget_para[2][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListFaultRecordsInfoPara.sub_widget_para[2][5].sub_widget.text_para.p_content, "2024-12-01 12:00:00");

    _gstEmListFaultRecordsInfoPara.selected_r = 10;
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


