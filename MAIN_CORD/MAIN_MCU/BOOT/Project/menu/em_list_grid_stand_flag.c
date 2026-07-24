/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-01-17
  * @brief       高级设置-电网标准设置
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-17    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define  EM_LIST_NATION_INFO_ICON_ARROW_SIZE                (4096)

#define  EM_LIST_NATIONAL_FLAG_NUMS							(6)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListGridStandFlag = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

extern FlagIdx flag_index;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;

static char _gTitleBuf[] = " ";


/*List控件*/
static emListPara_t  _gEmListGridStandFlagPara;

static emWidgetTextPara_t  _gGridStandTextPara;
static emWidgetImgPara_t   _gNationalFlagImgPara[6];


static char _gListNationalFlagInfoLeftTextBuf[16][20];
static char _gListNationalFlagInfoRightText3Buf[8];


#define WIN_LIST_GRID_STAND_FLAG_ID								 (GUI_ID_USER + 0x10)
#define WIN_LIST_GRID_STAND_FLAG_TITLE_ID                        (GUI_ID_USER + 0x11)


static const GUI_WIDGET_CREATE_INFO _aListNationalFlagDialogCreate[] = 
{
	{ WINDOW_CreateIndirect,  " ",   			 WIN_LIST_GRID_STAND_FLAG_ID,    	 	 			309,    81,     149,    220,    0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Title",        	 WIN_LIST_GRID_STAND_FLAG_TITLE_ID,      			309,    81,     15,     15,     0,   0x0,  0 },
};

/**
  * @brief ListNationalFlagInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListNationalFlagInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_FLAG_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
            /*LIST*/
            EmListWidgetCreate(&_gEmListGridStandFlagPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListGridStandFlagPara, _gEmListGridStandFlagPara.list_pre_sel);

			GUI_RECT Rect;
			WM_GetClientRect(&Rect);
			GUI_SetColor(0x00424242);
			GUI_FillRoundedRect(Rect.x0, Rect.y0, Rect.x1, Rect.y1, 10);
			GUI_SetColor(0x00BDC909);
			GUI_DrawRoundedFrame(Rect.x0 + 1, Rect.y0 + 1, Rect.x1 - 1, Rect.y1 - 1,10,3);
			
			#if 0 
            GUI_SetColor(0x00565656);//上色
			
            for(int i = 0; i < _gEmListGridStandFlagPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListGridStandFlagPara.sub_region[i].x_pos, 
                                    _gEmListGridStandFlagPara.sub_region[i].y_pos, 
                                    _gEmListGridStandFlagPara.sub_region[i].x_pos + _gEmListGridStandFlagPara.sub_region[i].x_size - 1, 
                                    _gEmListGridStandFlagPara.sub_region[i].y_pos + _gEmListGridStandFlagPara.sub_region[i].y_size - 1,
                                    _gEmListGridStandFlagPara.selected_r);
            }
			#endif   /*此段用于调节位置时使用*/
			
            SelectedDrawInWMPaint(&_gEmListGridStandFlagPara, _gEmListGridStandFlagPara.list_sel);
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
	g_ui_ctx.key_init = 0;
    _Init();
    GUI_EnableAlpha(1);
	#if 1
    _gEmListGridStandFlagPara.list_x_offset = _aListNationalFlagDialogCreate[0].x0;
    _gEmListGridStandFlagPara.list_y_offset = _aListNationalFlagDialogCreate[0].y0;
#endif 
    gEmWinListGridStandFlag.win_handle = GUI_CreateDialogBox(_aListNationalFlagDialogCreate, GUI_COUNTOF(_aListNationalFlagDialogCreate), _cbListNationalFlagInfoWin, 
	pCurrentEmWin->win_handle, 0, 0);
    //WINDOW_SetBkColor(gEmWinListGridStandFlag.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListGridStandFlag.win_handle, 0x00000000);
    WM_SetTransState(gEmWinListGridStandFlag.win_handle, WM_CF_HASTRANS);
    SelectedDraw(&_gEmListGridStandFlagPara, gEmWinListGridStandFlag.win_handle);
    GUI_Exec();
    return gEmWinListGridStandFlag.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListGridStandFlag.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListGridStandFlag.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListGridStandFlag.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	EmKeyDefaultInfoSet();
	uint8_t max_page;
	
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_1;

	
	for(uint8_t i =0; i < EM_LIST_NATIONAL_FLAG_NUMS; i++)
	{
		_gNationalFlagImgPara[i].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
		_gNationalFlagImgPara[i].p_img_buf = GUI_ALLOC_h2p(_gNationalFlagImgPara[i].mem_img);
		_gNationalFlagImgPara[i].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
		SpiFlashReadData(EM_ICON_GERMAN_FLAG_ADDR + i * 4096, _gNationalFlagImgPara[i].p_img_buf, _gNationalFlagImgPara[i].img_size);
	}
	
	_gEmListGridStandFlagPara.list_start_id = WIN_LIST_GRID_STAND_FLAG_ID + 0x30;
    _gEmListGridStandFlagPara.list_num = 6;
    _gEmListGridStandFlagPara.list_display_num = 6;
    if(_gEmListGridStandFlagPara.list_num <= _gEmListGridStandFlagPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListGridStandFlagPara.list_num + 1 - _gEmListGridStandFlagPara.list_display_num;
    }
    
    if(_gEmListGridStandFlagPara.list_sel > _gEmListGridStandFlagPara.list_num - 1)
    {
        _gEmListGridStandFlagPara.list_sel = 0;
    }
    if(_gEmListGridStandFlagPara.list_page > max_page - 1)
    {
        _gEmListGridStandFlagPara.list_page = 0;
    }
	
	
    //_gEmListGridStandFlagPara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gEmListGridStandFlagPara.sub_region[0].x_pos = 6;
    _gEmListGridStandFlagPara.sub_region[0].y_pos = 10;
    _gEmListGridStandFlagPara.sub_region[0].x_size = 137;
    _gEmListGridStandFlagPara.sub_region[0].y_size = 30;
    _gEmListGridStandFlagPara.sub_region[1].x_pos = 6;
    _gEmListGridStandFlagPara.sub_region[1].y_pos = 44;
    _gEmListGridStandFlagPara.sub_region[1].x_size = 137;
    _gEmListGridStandFlagPara.sub_region[1].y_size = 30;
    _gEmListGridStandFlagPara.sub_region[2].x_pos = 6;
    _gEmListGridStandFlagPara.sub_region[2].y_pos = 77;
    _gEmListGridStandFlagPara.sub_region[2].x_size = 137;
    _gEmListGridStandFlagPara.sub_region[2].y_size = 30;
    _gEmListGridStandFlagPara.sub_region[3].x_pos = 6;
    _gEmListGridStandFlagPara.sub_region[3].y_pos = 110;
    _gEmListGridStandFlagPara.sub_region[3].x_size = 137;
    _gEmListGridStandFlagPara.sub_region[3].y_size = 30;
    _gEmListGridStandFlagPara.sub_region[4].x_pos = 6;
    _gEmListGridStandFlagPara.sub_region[4].y_pos = 145;
    _gEmListGridStandFlagPara.sub_region[4].x_size = 137;
    _gEmListGridStandFlagPara.sub_region[4].y_size = 30;
    _gEmListGridStandFlagPara.sub_region[5].x_pos = 6;
    _gEmListGridStandFlagPara.sub_region[5].y_pos = 179;
    _gEmListGridStandFlagPara.sub_region[5].x_size = 137;
    _gEmListGridStandFlagPara.sub_region[5].y_size = 30;
    /*List控件*/

    //第1行左边
    _gEmListGridStandFlagPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListGridStandFlagPara.sub_widget_para[0][0].x_relative_pos = 4;
    _gEmListGridStandFlagPara.sub_widget_para[0][0].y_relative_pos = 1;
    _gEmListGridStandFlagPara.sub_widget_para[0][0].x_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][0].y_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][0].sub_widget.img_para.mem_img = _gNationalFlagImgPara[0].mem_img;
    _gEmListGridStandFlagPara.sub_widget_para[0][0].sub_widget.img_para.p_img_buf = _gNationalFlagImgPara[0].p_img_buf;
    _gEmListGridStandFlagPara.sub_widget_para[0][0].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第1行右边
    _gEmListGridStandFlagPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridStandFlagPara.sub_widget_para[1][0].x_relative_pos = 50;
    _gEmListGridStandFlagPara.sub_widget_para[1][0].y_relative_pos = 5;
    _gEmListGridStandFlagPara.sub_widget_para[1][0].x_size = 90;
    _gEmListGridStandFlagPara.sub_widget_para[1][0].y_size = 25;
	_gEmListGridStandFlagPara.sub_widget_para[1][0].align = EM_TEXT_USER_ALIGN;
    _gEmListGridStandFlagPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListNationalFlagInfoLeftTextBuf[0];
    _gEmListGridStandFlagPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridStandFlagPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListGridStandFlagPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "Germary");
	
    //第2行左边
    _gEmListGridStandFlagPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListGridStandFlagPara.sub_widget_para[0][1].x_relative_pos = 4;
    _gEmListGridStandFlagPara.sub_widget_para[0][1].y_relative_pos = 1;
    _gEmListGridStandFlagPara.sub_widget_para[0][1].x_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][1].y_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][1].sub_widget.img_para.mem_img = _gNationalFlagImgPara[1].mem_img;
    _gEmListGridStandFlagPara.sub_widget_para[0][1].sub_widget.img_para.p_img_buf = _gNationalFlagImgPara[1].p_img_buf;
    _gEmListGridStandFlagPara.sub_widget_para[0][1].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第2行右边
    _gEmListGridStandFlagPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridStandFlagPara.sub_widget_para[1][1].x_relative_pos = 50;
    _gEmListGridStandFlagPara.sub_widget_para[1][1].y_relative_pos = 5;
    _gEmListGridStandFlagPara.sub_widget_para[1][1].x_size = 90;
    _gEmListGridStandFlagPara.sub_widget_para[1][1].y_size = 25;
	_gEmListGridStandFlagPara.sub_widget_para[1][1].align = EM_TEXT_USER_ALIGN;
    _gEmListGridStandFlagPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListNationalFlagInfoLeftTextBuf[1];
    _gEmListGridStandFlagPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridStandFlagPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListGridStandFlagPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "China");

    //第3行左边
    _gEmListGridStandFlagPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListGridStandFlagPara.sub_widget_para[0][2].x_relative_pos = 4;
    _gEmListGridStandFlagPara.sub_widget_para[0][2].y_relative_pos = 1;
    _gEmListGridStandFlagPara.sub_widget_para[0][2].x_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][2].y_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][2].sub_widget.img_para.mem_img = _gNationalFlagImgPara[2].mem_img;
    _gEmListGridStandFlagPara.sub_widget_para[0][2].sub_widget.img_para.p_img_buf = _gNationalFlagImgPara[2].p_img_buf;
    _gEmListGridStandFlagPara.sub_widget_para[0][2].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第3行右边
    _gEmListGridStandFlagPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridStandFlagPara.sub_widget_para[1][2].x_relative_pos = 50;
    _gEmListGridStandFlagPara.sub_widget_para[1][2].y_relative_pos = 5;
    _gEmListGridStandFlagPara.sub_widget_para[1][2].x_size = 90;
    _gEmListGridStandFlagPara.sub_widget_para[1][2].y_size = 25;
	_gEmListGridStandFlagPara.sub_widget_para[1][2].align = EM_TEXT_USER_ALIGN;
    _gEmListGridStandFlagPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListNationalFlagInfoLeftTextBuf[2];
    _gEmListGridStandFlagPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridStandFlagPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListGridStandFlagPara.sub_widget_para[1][2].sub_widget.text_para.p_content, "Australia");
    //第4行左边
    _gEmListGridStandFlagPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListGridStandFlagPara.sub_widget_para[0][3].x_relative_pos = 4;
    _gEmListGridStandFlagPara.sub_widget_para[0][3].y_relative_pos = 1;
    _gEmListGridStandFlagPara.sub_widget_para[0][3].x_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][3].y_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][3].sub_widget.img_para.mem_img = _gNationalFlagImgPara[3].mem_img;
    _gEmListGridStandFlagPara.sub_widget_para[0][3].sub_widget.img_para.p_img_buf = _gNationalFlagImgPara[3].p_img_buf;
    _gEmListGridStandFlagPara.sub_widget_para[0][3].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第4行右边
    _gEmListGridStandFlagPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridStandFlagPara.sub_widget_para[1][3].x_relative_pos = 50;
    _gEmListGridStandFlagPara.sub_widget_para[1][3].y_relative_pos = 5;
    _gEmListGridStandFlagPara.sub_widget_para[1][3].x_size = 90;
    _gEmListGridStandFlagPara.sub_widget_para[1][3].y_size = 25;
	_gEmListGridStandFlagPara.sub_widget_para[1][3].align = EM_TEXT_USER_ALIGN;
    _gEmListGridStandFlagPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _gListNationalFlagInfoLeftTextBuf[3];
    _gEmListGridStandFlagPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridStandFlagPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListGridStandFlagPara.sub_widget_para[1][3].sub_widget.text_para.p_content, "Italy");
    //第5行左边
    _gEmListGridStandFlagPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListGridStandFlagPara.sub_widget_para[0][4].x_relative_pos = 4;
    _gEmListGridStandFlagPara.sub_widget_para[0][4].y_relative_pos = 1;
    _gEmListGridStandFlagPara.sub_widget_para[0][4].x_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][4].y_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][4].sub_widget.img_para.mem_img = _gNationalFlagImgPara[4].mem_img;
    _gEmListGridStandFlagPara.sub_widget_para[0][4].sub_widget.img_para.p_img_buf = _gNationalFlagImgPara[4].p_img_buf;
    _gEmListGridStandFlagPara.sub_widget_para[0][4].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第5行右边
    _gEmListGridStandFlagPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridStandFlagPara.sub_widget_para[1][4].x_relative_pos = 50;
    _gEmListGridStandFlagPara.sub_widget_para[1][4].y_relative_pos = 5;
    _gEmListGridStandFlagPara.sub_widget_para[1][4].x_size = 90;
    _gEmListGridStandFlagPara.sub_widget_para[1][4].y_size = 25;
	_gEmListGridStandFlagPara.sub_widget_para[1][4].align = EM_TEXT_USER_ALIGN;
    _gEmListGridStandFlagPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _gListNationalFlagInfoLeftTextBuf[4];
    _gEmListGridStandFlagPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridStandFlagPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListGridStandFlagPara.sub_widget_para[1][4].sub_widget.text_para.p_content, "Spanish");
    //第6行左边
    _gEmListGridStandFlagPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListGridStandFlagPara.sub_widget_para[0][5].x_relative_pos = 4;
    _gEmListGridStandFlagPara.sub_widget_para[0][5].y_relative_pos = 1;
    _gEmListGridStandFlagPara.sub_widget_para[0][5].x_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][5].y_size = 30;
    _gEmListGridStandFlagPara.sub_widget_para[0][5].sub_widget.img_para.mem_img = _gNationalFlagImgPara[5].mem_img;
    _gEmListGridStandFlagPara.sub_widget_para[0][5].sub_widget.img_para.p_img_buf = _gNationalFlagImgPara[5].p_img_buf;
    _gEmListGridStandFlagPara.sub_widget_para[0][5].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第6行右边
    _gEmListGridStandFlagPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListGridStandFlagPara.sub_widget_para[1][5].x_relative_pos = 50;
    _gEmListGridStandFlagPara.sub_widget_para[1][5].y_relative_pos = 5;
    _gEmListGridStandFlagPara.sub_widget_para[1][5].x_size = 90;
    _gEmListGridStandFlagPara.sub_widget_para[1][5].y_size = 25;
	_gEmListGridStandFlagPara.sub_widget_para[1][5].align = EM_TEXT_USER_ALIGN;
    _gEmListGridStandFlagPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _gListNationalFlagInfoLeftTextBuf[5];
    _gEmListGridStandFlagPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListGridStandFlagPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListGridStandFlagPara.sub_widget_para[1][5].sub_widget.text_para.p_content, "France");

    /*List选中框*/
    _gEmListGridStandFlagPara.selected_unvalid_color = 0xFF000000;
    _gEmListGridStandFlagPara.selected_valid_color = 0x00D6E400;
    _gEmListGridStandFlagPara.selected_size = 3;
    _gEmListGridStandFlagPara.selected_r = 5;
    _gEmListGridStandFlagPara.selected_x_start_offset = 42;
    _gEmListGridStandFlagPara.selected_y_start_offset = -2;
    _gEmListGridStandFlagPara.selected_x_end_offset = -2;
    _gEmListGridStandFlagPara.selected_y_end_offset = 2;
    _gEmListGridStandFlagPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    //_gEmListGridStandFlagPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
//    /*滚动条*/
//    _gEmListGridStandFlagPara.scrollbar_enable = true;
//    _gEmListGridStandFlagPara.scrollbar_color = 0x00D6E400;
//    _gEmListGridStandFlagPara.scrollbar_bk_color = 0x00565656;
//    _gEmListGridStandFlagPara.scrollbar_x_pos = 454;
//    _gEmListGridStandFlagPara.scrollbar_y_pos = 39;
//    _gEmListGridStandFlagPara.scrollbar_x_size = 6;
//    _gEmListGridStandFlagPara.scrollbar_y_size = 241;


}


/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
	for(uint8_t i =0; i < EM_LIST_NATIONAL_FLAG_NUMS; i++)
	{
		 GUI_ALLOC_Free(_gNationalFlagImgPara[i].mem_img);
	}

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
    if(gEmWinListGridStandFlag.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gEmListGridStandFlagPara.list_sel + _gEmListGridStandFlagPara.list_page >= _gEmListGridStandFlagPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListGridStandFlagPara.list_sel + _gEmListGridStandFlagPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListGridStandFlagPara, gEmWinListGridStandFlag.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListGridStandFlagPara, gEmWinListGridStandFlag.win_handle);
        }
    }
    
//    if(pop_up_stat == 0)
//    {
//        if(GetSystick() - pop_timecount > 1000)
//        {
//            pop_up_stat = 1;
//            pop_timecount = GetSystick();
//            gPopUpWinError.create_win();
//        }
//    }
//    else
//    {
//        if(GetSystick() - pop_timecount > 1000)
//        {
//            pop_up_stat = 0;
//            pop_timecount = GetSystick();
//            gPopUpWinError.delete_win();
//        }
//    }
    
#endif 
}

/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{
	extern void SetGridListShow(int32_t show);
	int32_t event = EmGetKeyEvent(p_msg);
	if (event < 0 || event > EN_KEY_SRC_NUM)
	{
		return;
	}
	if (event == EN_KEY_SRC_LEFT)
	{
		--g_ui_ctx.select;
		if (g_ui_ctx.select < 0)
		{
			++g_ui_ctx.select;
			return;
		}
		EmListUpward(&_gEmListGridStandFlagPara, gEmWinListGridStandFlag.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		++g_ui_ctx.select;
		if (g_ui_ctx.select >= _gEmListGridStandFlagPara.list_num)
		{
			--g_ui_ctx.select;
			return;
		}
		EmListDownward(&_gEmListGridStandFlagPara, gEmWinListGridStandFlag.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		extern int32_t SetCurGridFlag(int32_t idx);
		flag_index.flag_idx = g_ui_ctx.select;
		SetCurGridFlag(flag_index.flag_idx);
//		SetCurGridFlag(g_ui_ctx.select);
		SetGridListShow(0);
		
		return;
	}
	else if (event == EN_KEY_SRC_ESC)
	{
		SetGridListShow(0);
		
		extern void SetGridStandArrowDirection(void);
		SetGridStandArrowDirection();
		return;
	}
}


