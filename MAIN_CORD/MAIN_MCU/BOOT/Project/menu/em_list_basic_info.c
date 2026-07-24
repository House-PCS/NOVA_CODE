/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-11
  * @brief       List 设备基本信息
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

#define  EM_LIST_BASIC_INFO_ICON_ARROW_SIZE                (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init();
static void _Del();
static void _Polling();
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListBasicInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_t
{
	int8_t select;

	uint16_t work_mode;

	uint16_t electic_purchase_day;
	uint16_t electic_sale_day;
	uint32_t pv_sum_generation_on_day;
	uint16_t load_cost_day;

	uint16_t soc;

	uint16_t charge_type;

    uint16_t chassis_internal_temp;

    uint16_t net_state;

    uint32_t time;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};



/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;

static char _gTitleBuf[] = "Operational State Info";

/*List控件*/
static emListPara_t  _gEmListOperaInfoPara;

static char _gListOperaInfoLeftTextBuf[6][20];
static char _gListOperaInfoRightText3Buf[8];
static GUI_HMEM _gListOperaInfoRightMem;
static uint8_t *_pListOperaInfoRightBuf;

#define WIN_LIST_OPERA_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_OPERA_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static const GUI_WIDGET_CREATE_INFO _aListOperaInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListOperaInfoWin",    WIN_LIST_OPERA_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_OPERA_INFO_TITLE_ID,     31,  20, 144,  14,   0,   0x0,  0 },
};



/**
  * @brief ListOperaInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListOperaInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    int     n_code;
    int     id;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_OPERA_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListOperaInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListOperaInfoPara, _gEmListOperaInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(int i = 0; i < _gEmListOperaInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListOperaInfoPara.sub_region[i].x_pos, 
                                    _gEmListOperaInfoPara.sub_region[i].y_pos, 
                                    _gEmListOperaInfoPara.sub_region[i].x_pos + _gEmListOperaInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListOperaInfoPara.sub_region[i].y_pos + _gEmListOperaInfoPara.sub_region[i].y_size - 1,
                                    _gEmListOperaInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListOperaInfoPara, _gEmListOperaInfoPara.list_sel);
            break;
        }
        case WM_NOTIFY_PARENT:
        {
            id    = WM_GetId(pMsg->hWinSrc);
            n_code = pMsg->Data.v;
            break;
            // USER START (Optionally insert additional message handling)
            // USER END
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
//    GUI_EnableAlpha(1);
    gEmWinListBasicInfo.win_handle = GUI_CreateDialogBox(_aListOperaInfoDialogCreate, GUI_COUNTOF(_aListOperaInfoDialogCreate), _cbListOperaInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListBasicInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListBasicInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListBasicInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListOperaInfoPara, gEmWinListBasicInfo.win_handle);
    return gEmWinListBasicInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListBasicInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListBasicInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListBasicInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;
    
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_1;
    
    _gEmListOperaInfoPara.list_start_id = WIN_LIST_OPERA_INFO_ID + 0x30;
    _gEmListOperaInfoPara.list_num = 6;
    _gEmListOperaInfoPara.list_display_num = 6;
    if(_gEmListOperaInfoPara.list_num <= _gEmListOperaInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListOperaInfoPara.list_num + 1 - _gEmListOperaInfoPara.list_display_num;
    }
    
    if(_gEmListOperaInfoPara.list_sel > _gEmListOperaInfoPara.list_num - 1)
    {
        _gEmListOperaInfoPara.list_sel = 0;
    }
    if(_gEmListOperaInfoPara.list_page > max_page - 1)
    {
        _gEmListOperaInfoPara.list_page = 0;
    }
    _gEmListOperaInfoPara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gEmListOperaInfoPara.sub_region[0].x_pos = 20;
    _gEmListOperaInfoPara.sub_region[0].y_pos = 38;
    _gEmListOperaInfoPara.sub_region[0].x_size = 420;
    _gEmListOperaInfoPara.sub_region[0].y_size = 34;
    _gEmListOperaInfoPara.sub_region[1].x_pos = 20;
    _gEmListOperaInfoPara.sub_region[1].y_pos = 80;
    _gEmListOperaInfoPara.sub_region[1].x_size = 420;
    _gEmListOperaInfoPara.sub_region[1].y_size = 34;
    _gEmListOperaInfoPara.sub_region[2].x_pos = 20;
    _gEmListOperaInfoPara.sub_region[2].y_pos = 122;
    _gEmListOperaInfoPara.sub_region[2].x_size = 420;
    _gEmListOperaInfoPara.sub_region[2].y_size = 34;
    _gEmListOperaInfoPara.sub_region[3].x_pos = 20;
    _gEmListOperaInfoPara.sub_region[3].y_pos = 164;
    _gEmListOperaInfoPara.sub_region[3].x_size = 420;
    _gEmListOperaInfoPara.sub_region[3].y_size = 34;
    _gEmListOperaInfoPara.sub_region[4].x_pos = 20;
    _gEmListOperaInfoPara.sub_region[4].y_pos = 206;
    _gEmListOperaInfoPara.sub_region[4].x_size = 420;
    _gEmListOperaInfoPara.sub_region[4].y_size = 34;
    _gEmListOperaInfoPara.sub_region[5].x_pos = 20;
    _gEmListOperaInfoPara.sub_region[5].y_pos = 248;
    _gEmListOperaInfoPara.sub_region[5].x_size = 420;
    _gEmListOperaInfoPara.sub_region[5].y_size = 34;
    /*List控件*/
    _gListOperaInfoRightMem = GUI_ALLOC_AllocZero(EM_LIST_OPERA_INFO_ICON_ARROW_SIZE);
    _pListOperaInfoRightBuf = GUI_ALLOC_h2p(_gListOperaInfoRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListOperaInfoRightBuf, EM_LIST_OPERA_INFO_ICON_ARROW_SIZE);
    //第一行左边
    _gEmListOperaInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOperaInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListOperaInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListOperaInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListOperaInfoLeftTextBuf[0];
    _gEmListOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Product Model");
    //第一行右边
    _gEmListOperaInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOperaInfoPara.sub_widget_para[1][0].x_relative_pos = 393;
    _gEmListOperaInfoPara.sub_widget_para[1][0].y_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[1][0].x_size = 7;
    _gEmListOperaInfoPara.sub_widget_para[1][0].y_size = 12;
    _gEmListOperaInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gListOperaInfoRightMem;
    _gEmListOperaInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _pListOperaInfoRightBuf;
    _gEmListOperaInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
    //第二行左边
    _gEmListOperaInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOperaInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListOperaInfoPara.sub_widget_para[0][1].x_size = 158;
    _gEmListOperaInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListOperaInfoLeftTextBuf[1];
    _gEmListOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Battery Setting");
    //第二行右边
    _gEmListOperaInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOperaInfoPara.sub_widget_para[1][1].x_relative_pos = 393;
    _gEmListOperaInfoPara.sub_widget_para[1][1].y_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[1][1].x_size = 7;
    _gEmListOperaInfoPara.sub_widget_para[1][1].y_size = 12;
    _gEmListOperaInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListOperaInfoRightMem;
    _gEmListOperaInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListOperaInfoRightBuf;
    _gEmListOperaInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
    //第三行左边
    _gEmListOperaInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOperaInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListOperaInfoPara.sub_widget_para[0][2].x_size = 158;
    _gEmListOperaInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListOperaInfoLeftTextBuf[2];
    _gEmListOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Grid STD");
    //第三行右边
    _gEmListOperaInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOperaInfoPara.sub_widget_para[1][2].x_relative_pos = 299;
    _gEmListOperaInfoPara.sub_widget_para[1][2].y_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[1][2].x_size = 97;
    _gEmListOperaInfoPara.sub_widget_para[1][2].y_size = 17;
    _gEmListOperaInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListOperaInfoRightText3Buf;
    _gEmListOperaInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOperaInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_6;
    _gEmListOperaInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 200;//测试数据，实际赋值系统数据
    sprintf(_gEmListOperaInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content, "%dV", (uint16_t)_gEmListOperaInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value);
    //第四行左边
    _gEmListOperaInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOperaInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListOperaInfoPara.sub_widget_para[0][3].x_size = 158;
    _gEmListOperaInfoPara.sub_widget_para[0][3].y_size = 17;
    _gEmListOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListOperaInfoLeftTextBuf[3];
    _gEmListOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Run Setting");
    //第四行右边
    _gEmListOperaInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOperaInfoPara.sub_widget_para[1][3].x_relative_pos = 393;
    _gEmListOperaInfoPara.sub_widget_para[1][3].y_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[1][3].x_size = 7;
    _gEmListOperaInfoPara.sub_widget_para[1][3].y_size = 12;
    _gEmListOperaInfoPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gListOperaInfoRightMem;
    _gEmListOperaInfoPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _pListOperaInfoRightBuf;
    _gEmListOperaInfoPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
    //第五行左边
    _gEmListOperaInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOperaInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListOperaInfoPara.sub_widget_para[0][4].x_size = 158;
    _gEmListOperaInfoPara.sub_widget_para[0][4].y_size = 17;
    _gEmListOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListOperaInfoLeftTextBuf[4];
    _gEmListOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "485 Address");
    //第五行右边
    _gEmListOperaInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOperaInfoPara.sub_widget_para[1][4].x_relative_pos = 393;
    _gEmListOperaInfoPara.sub_widget_para[1][4].y_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[1][4].x_size = 7;
    _gEmListOperaInfoPara.sub_widget_para[1][4].y_size = 12;
    _gEmListOperaInfoPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gListOperaInfoRightMem;
    _gEmListOperaInfoPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _pListOperaInfoRightBuf;
    _gEmListOperaInfoPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
    //第六行左边
    _gEmListOperaInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOperaInfoPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gEmListOperaInfoPara.sub_widget_para[0][5].x_size = 158;
    _gEmListOperaInfoPara.sub_widget_para[0][5].y_size = 17;
    _gEmListOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListOperaInfoLeftTextBuf[5];
    _gEmListOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "485 Address");
    //第六行右边
    _gEmListOperaInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOperaInfoPara.sub_widget_para[1][5].x_relative_pos = 393;
    _gEmListOperaInfoPara.sub_widget_para[1][5].y_relative_pos = 11;
    _gEmListOperaInfoPara.sub_widget_para[1][5].x_size = 7;
    _gEmListOperaInfoPara.sub_widget_para[1][5].y_size = 12;
    _gEmListOperaInfoPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _gListOperaInfoRightMem;
    _gEmListOperaInfoPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _pListOperaInfoRightBuf;
    _gEmListOperaInfoPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
    
    /*List选中框*/
    _gEmListOperaInfoPara.selected_unvalid_color = 0xFF000000;
    _gEmListOperaInfoPara.selected_valid_color = 0x00D6E400;
    _gEmListOperaInfoPara.selected_size = 3;
    _gEmListOperaInfoPara.selected_r = 10;
    _gEmListOperaInfoPara.selected_x_start_offset = -2;
    _gEmListOperaInfoPara.selected_y_start_offset = -2;
    _gEmListOperaInfoPara.selected_x_end_offset = 2;
    _gEmListOperaInfoPara.selected_y_end_offset = 2;
//    _gEmListOperaInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gEmListOperaInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
    /*滚动条*/
    _gEmListOperaInfoPara.scrollbar_enable = true;
    _gEmListOperaInfoPara.scrollbar_color = 0x00D6E400;
    _gEmListOperaInfoPara.scrollbar_bk_color = 0x00565656;
    _gEmListOperaInfoPara.scrollbar_x_pos = 454;
    _gEmListOperaInfoPara.scrollbar_y_pos = 39;
    _gEmListOperaInfoPara.scrollbar_x_size = 6;
    _gEmListOperaInfoPara.scrollbar_y_size = 241;
    
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gListOperaInfoRightMem);
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
    static uint8_t pop_up_stat = 0;
    static uint32_t pop_timecount = 0;
    if(gEmWinListBasicInfo.win_handle == 0)
    {
        return;
    }
    if(GetSystick() - timecount > 30)
    {
        timecount = GetSystick();
        if(_gEmListOperaInfoPara.list_sel >= _gEmListOperaInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListOperaInfoPara.list_sel == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListOperaInfoPara, gEmWinListBasicInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListOperaInfoPara, gEmWinListBasicInfo.win_handle);
        }
    }
    
    if(pop_up_stat == 0)
    {
        if(GetSystick() - pop_timecount > 1000)
        {
            pop_up_stat = 1;
            pop_timecount = GetSystick();
            gPopUpWinError.create_win();
        }
    }
    else
    {
        if(GetSystick() - pop_timecount > 1000)
        {
            pop_up_stat = 0;
            pop_timecount = GetSystick();
            gPopUpWinError.delete_win();
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


