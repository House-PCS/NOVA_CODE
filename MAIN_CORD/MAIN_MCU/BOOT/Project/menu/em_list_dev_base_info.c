/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-08
  * @brief       设备基本信息ListView界面
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-08    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define  EM_BACK_ON_ICON_ARROW_SIZE							(4096)  
#define  EM_LIST_OPERA_INFO_ICON_ARROW_SIZE					(4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListDevBaseInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;

	uint16_t model;
	uint16_t sn[10];
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 

static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

static char _gTitleBuf[] = "Basic Info";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";

/*List控件*/
static emListPara_t  _gEmListDevBaseInfoPara;

static char _gListDevBaseInfoLeftTextBuf[6][32];
static char _gListDevBaseInfoRightText3Buf[2][32];
static GUI_HMEM _gListDevBaseInfoRightMem;
static uint8_t *_pListDevBaseInfoRightBuf;

#define WIN_LIST_OPERA_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_OPERA_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)
#define WIN_LIST_PRESS_ID									 (GUI_ID_USER + 0x12)
#define WIN_LIST_BACK_IMG_ID								 (GUI_ID_USER + 0x13)
#define WIN_LIST_BACK_ID									 (GUI_ID_USER + 0x14)

static const GUI_WIDGET_CREATE_INFO _aListDevBaseInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListDevBaseInfoWin",    WIN_LIST_OPERA_INFO_ID,     
		0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Basic Info",			   WIN_LIST_OPERA_INFO_TITLE_ID,     
		31,  20, 144,  14,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",					WIN_LIST_PRESS_ID,
		133, 	273, 	33,   14,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "BackImg",				WIN_LIST_BACK_IMG_ID,
		171-5, 	275, 	15,   10,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Back",				   WIN_LIST_BACK_ID, 
		188, 	273, 	150,  14,   0,   0x0,  0 },
};

static int32_t _SetProductModel(const char *model)
{
    snprintf(_gListDevBaseInfoRightText3Buf[0], sizeof(_gListDevBaseInfoRightText3Buf[0]), 
			"%s", model);
	TEXT_SetText(_gEmListDevBaseInfoPara.handle[1][0], 
				(const char *)_gEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetProductSn(const char *model)
{
    snprintf(_gListDevBaseInfoRightText3Buf[1], sizeof(_gListDevBaseInfoRightText3Buf[1]), 
			"%s", model);
	TEXT_SetText(_gEmListDevBaseInfoPara.handle[1][1], 
				(const char *)_gEmListDevBaseInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
	return 0;
}

/**
  * @brief ListDevBaseInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListDevBaseInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
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
            EmListWidgetCreate(&_gEmListDevBaseInfoPara, pMsg->hWin);

			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListDevBaseInfoPara, _gEmListDevBaseInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(int i = 0; i < _gEmListDevBaseInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListDevBaseInfoPara.sub_region[i].x_pos, 
                                    _gEmListDevBaseInfoPara.sub_region[i].y_pos, 
                                    _gEmListDevBaseInfoPara.sub_region[i].x_pos + _gEmListDevBaseInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListDevBaseInfoPara.sub_region[i].y_pos + _gEmListDevBaseInfoPara.sub_region[i].y_size - 1,
                                    _gEmListDevBaseInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListDevBaseInfoPara, _gEmListDevBaseInfoPara.list_sel);
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
	pPrevEmWin =  &gEmWinMain3DNav;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListDevBaseInfo.win_handle = GUI_CreateDialogBox(_aListDevBaseInfoDialogCreate, GUI_COUNTOF(_aListDevBaseInfoDialogCreate), _cbListDevBaseInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListDevBaseInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListDevBaseInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListDevBaseInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListDevBaseInfoPara, gEmWinListDevBaseInfo.win_handle);
    return gEmWinListDevBaseInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListDevBaseInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListDevBaseInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListDevBaseInfo.win_handle = 0;
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

	/*底部back的参数*/
	_gPressTextPara.p_content = _gPressBuf;
    _gPressTextPara.text_color = 0x00EEEEEE;
    _gPressTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gBackTextPara.p_content = _gBackBuf;
    _gBackTextPara.text_color = 0x00EEEEEE;
    _gBackTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_BACK_ON_ICON_ARROW_SIZE);
    _gReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gReturnImgPara.mem_img);
    _gReturnImgPara.img_size = EM_BACK_ON_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size);
    
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_1;
    
    _gEmListDevBaseInfoPara.list_start_id = WIN_LIST_OPERA_INFO_ID + 0x30;
    _gEmListDevBaseInfoPara.list_num = 5;
    _gEmListDevBaseInfoPara.list_display_num = 5;
    if(_gEmListDevBaseInfoPara.list_num <= _gEmListDevBaseInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListDevBaseInfoPara.list_num + 1 - _gEmListDevBaseInfoPara.list_display_num;
    }


    if(_gEmListDevBaseInfoPara.list_sel > _gEmListDevBaseInfoPara.list_num - 1)
    {
        _gEmListDevBaseInfoPara.list_sel = 0;
    }
    if(_gEmListDevBaseInfoPara.list_page > max_page - 1)
    {
        _gEmListDevBaseInfoPara.list_page = 0;
    }
    _gEmListDevBaseInfoPara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gEmListDevBaseInfoPara.sub_region[0].x_pos = 20;
    _gEmListDevBaseInfoPara.sub_region[0].y_pos = 38;
    _gEmListDevBaseInfoPara.sub_region[0].x_size = 420;
    _gEmListDevBaseInfoPara.sub_region[0].y_size = 34;
    _gEmListDevBaseInfoPara.sub_region[1].x_pos = 20;
    _gEmListDevBaseInfoPara.sub_region[1].y_pos = 80;
    _gEmListDevBaseInfoPara.sub_region[1].x_size = 420;
    _gEmListDevBaseInfoPara.sub_region[1].y_size = 34;
    _gEmListDevBaseInfoPara.sub_region[2].x_pos = 20;
    _gEmListDevBaseInfoPara.sub_region[2].y_pos = 122;
    _gEmListDevBaseInfoPara.sub_region[2].x_size = 420;
    _gEmListDevBaseInfoPara.sub_region[2].y_size = 34;
    _gEmListDevBaseInfoPara.sub_region[3].x_pos = 20;
    _gEmListDevBaseInfoPara.sub_region[3].y_pos = 164;
    _gEmListDevBaseInfoPara.sub_region[3].x_size = 420;
    _gEmListDevBaseInfoPara.sub_region[3].y_size = 34;
    _gEmListDevBaseInfoPara.sub_region[4].x_pos = 20;
    _gEmListDevBaseInfoPara.sub_region[4].y_pos = 206;
    _gEmListDevBaseInfoPara.sub_region[4].x_size = 420;
    _gEmListDevBaseInfoPara.sub_region[4].y_size = 34;
    _gEmListDevBaseInfoPara.sub_region[5].x_pos = 20;
    _gEmListDevBaseInfoPara.sub_region[5].y_pos = 248;
    _gEmListDevBaseInfoPara.sub_region[5].x_size = 420;
    _gEmListDevBaseInfoPara.sub_region[5].y_size = 34;
    /*List控件*/
    _gListDevBaseInfoRightMem = GUI_ALLOC_AllocZero(EM_LIST_OPERA_INFO_ICON_ARROW_SIZE);
    _pListDevBaseInfoRightBuf = GUI_ALLOC_h2p(_gListDevBaseInfoRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListDevBaseInfoRightBuf, EM_LIST_OPERA_INFO_ICON_ARROW_SIZE);
    //第一行左边
    _gEmListDevBaseInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListDevBaseInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListDevBaseInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListDevBaseInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListDevBaseInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListDevBaseInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListDevBaseInfoLeftTextBuf[0];
    _gEmListDevBaseInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListDevBaseInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListDevBaseInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Product Model");
    //第一行右边
    _gEmListDevBaseInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListDevBaseInfoPara.sub_widget_para[1][0].x_relative_pos = 180;
    _gEmListDevBaseInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListDevBaseInfoPara.sub_widget_para[1][0].x_size = 230;
    _gEmListDevBaseInfoPara.sub_widget_para[1][0].y_size = 16;
    _gEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListDevBaseInfoRightText3Buf[0];
    _gEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "Roca 5");
    //第二行左边
    _gEmListDevBaseInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListDevBaseInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListDevBaseInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListDevBaseInfoPara.sub_widget_para[0][1].x_size = 158;
    _gEmListDevBaseInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListDevBaseInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListDevBaseInfoLeftTextBuf[1];
    _gEmListDevBaseInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListDevBaseInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListDevBaseInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Product SN Code");
    //第二行右边
    _gEmListDevBaseInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListDevBaseInfoPara.sub_widget_para[1][1].x_relative_pos = 180;
    _gEmListDevBaseInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gEmListDevBaseInfoPara.sub_widget_para[1][1].x_size = 230;
    _gEmListDevBaseInfoPara.sub_widget_para[1][1].y_size = 16;
    _gEmListDevBaseInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListDevBaseInfoRightText3Buf[1];
    _gEmListDevBaseInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListDevBaseInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListDevBaseInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "1234567890");
    //第三行左边
    _gEmListDevBaseInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListDevBaseInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListDevBaseInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListDevBaseInfoPara.sub_widget_para[0][2].x_size = 158;
    _gEmListDevBaseInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListDevBaseInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListDevBaseInfoLeftTextBuf[2];
    _gEmListDevBaseInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListDevBaseInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListDevBaseInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "ARM/DSP Version");
    //第三行右边
    _gEmListDevBaseInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListDevBaseInfoPara.sub_widget_para[1][2].x_relative_pos = 393;
    _gEmListDevBaseInfoPara.sub_widget_para[1][2].y_relative_pos = 11;
    _gEmListDevBaseInfoPara.sub_widget_para[1][2].x_size = 97;
    _gEmListDevBaseInfoPara.sub_widget_para[1][2].y_size = 17;
    _gEmListDevBaseInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gListDevBaseInfoRightMem;
    _gEmListDevBaseInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _pListDevBaseInfoRightBuf;
    _gEmListDevBaseInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
    //第四行左边
    _gEmListDevBaseInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListDevBaseInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListDevBaseInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListDevBaseInfoPara.sub_widget_para[0][3].x_size = 200;
    _gEmListDevBaseInfoPara.sub_widget_para[0][3].y_size = 17;
    _gEmListDevBaseInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListDevBaseInfoLeftTextBuf[3];
    _gEmListDevBaseInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListDevBaseInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListDevBaseInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Battery Pack Version");
    //第四行右边
    _gEmListDevBaseInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListDevBaseInfoPara.sub_widget_para[1][3].x_relative_pos = 393;
    _gEmListDevBaseInfoPara.sub_widget_para[1][3].y_relative_pos = 11;
    _gEmListDevBaseInfoPara.sub_widget_para[1][3].x_size = 7;
    _gEmListDevBaseInfoPara.sub_widget_para[1][3].y_size = 12;
    _gEmListDevBaseInfoPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gListDevBaseInfoRightMem;
    _gEmListDevBaseInfoPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _pListDevBaseInfoRightBuf;
    _gEmListDevBaseInfoPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
    //第五行左边
    _gEmListDevBaseInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListDevBaseInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListDevBaseInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListDevBaseInfoPara.sub_widget_para[0][4].x_size = 200;
    _gEmListDevBaseInfoPara.sub_widget_para[0][4].y_size = 17;
    _gEmListDevBaseInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListDevBaseInfoLeftTextBuf[4];
    _gEmListDevBaseInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListDevBaseInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListDevBaseInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Operational Records");
    //第五行右边
    _gEmListDevBaseInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListDevBaseInfoPara.sub_widget_para[1][4].x_relative_pos = 393;
    _gEmListDevBaseInfoPara.sub_widget_para[1][4].y_relative_pos = 11;
    _gEmListDevBaseInfoPara.sub_widget_para[1][4].x_size = 7;
    _gEmListDevBaseInfoPara.sub_widget_para[1][4].y_size = 12;
    _gEmListDevBaseInfoPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gListDevBaseInfoRightMem;
    _gEmListDevBaseInfoPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _pListDevBaseInfoRightBuf;
    _gEmListDevBaseInfoPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
    
    /*List选中框*/
    _gEmListDevBaseInfoPara.selected_unvalid_color = 0xFF000000;
    _gEmListDevBaseInfoPara.selected_valid_color = 0x00D6E400;
    _gEmListDevBaseInfoPara.selected_size = 3;
    _gEmListDevBaseInfoPara.selected_r = 10;
    _gEmListDevBaseInfoPara.selected_x_start_offset = -2;
    _gEmListDevBaseInfoPara.selected_y_start_offset = -2;
    _gEmListDevBaseInfoPara.selected_x_end_offset = 2;
    _gEmListDevBaseInfoPara.selected_y_end_offset = 2;
    _gEmListDevBaseInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gEmListDevBaseInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
    /*滚动条*/
	if (max_page > 1) 
	{
		_gEmListDevBaseInfoPara.scrollbar_enable = true;
		_gEmListDevBaseInfoPara.scrollbar_color = 0x00D6E400;
		_gEmListDevBaseInfoPara.scrollbar_bk_color = 0x00565656;
		_gEmListDevBaseInfoPara.scrollbar_x_pos = 454;
		_gEmListDevBaseInfoPara.scrollbar_y_pos = 39;
		_gEmListDevBaseInfoPara.scrollbar_x_size = 6;
		_gEmListDevBaseInfoPara.scrollbar_y_size = 241;
	}
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gListDevBaseInfoRightMem);
    GUI_ALLOC_Free(_gReturnImgPara.mem_img);
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
    if(gEmWinListDevBaseInfo.win_handle == 0)
    {
        return;
    }

	if (g_ui_ctx.model != 0x01)
	{
		char model[32] = {0};
		g_ui_ctx.model = 0x01;
		snprintf(model, sizeof(model), "%s", GetPcsModelByModelId(0x01));
		_SetProductModel(model);
	}

	if (memcmp(g_ui_ctx.sn, gCollectParaData.collect_data.sn, sizeof(g_ui_ctx.sn)))
	{
		char sn[sizeof(g_ui_ctx.sn)+1] = {0};
		memcpy(g_ui_ctx.sn, gCollectParaData.collect_data.sn, sizeof(g_ui_ctx.sn));
		if (g_ui_ctx.sn[0] == 0)
		{
			snprintf(sn, sizeof(sn), "%s", "unknown");
		}
		else
		{
			snprintf(sn, sizeof(sn), "%s", (char *)g_ui_ctx.sn);
		}
		_SetProductSn(sn);
	}

#if 0
    if(GetSystick() - timecount > 30)
    {
        timecount = GetSystick();
        if(_gEmListDevBaseInfoPara.list_sel >= _gEmListDevBaseInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListDevBaseInfoPara.list_sel == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListDevBaseInfoPara, gEmWinListDevBaseInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListDevBaseInfoPara, gEmWinListDevBaseInfo.win_handle);
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
		EmListUpward(&_gEmListDevBaseInfoPara, gEmWinListDevBaseInfo.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		++g_ui_ctx.select;
		if (g_ui_ctx.select >= _gEmListDevBaseInfoPara.list_num)
		{
			--g_ui_ctx.select;
			return;
		}
		EmListDownward(&_gEmListDevBaseInfoPara, gEmWinListDevBaseInfo.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		emWin_t *pnext = NULL;
		switch(g_ui_ctx.select)
		{
			case 2:
				pnext = &gEmWinListArmVersionInfo;
				break;
			case 3:
				pnext = &gEmWinListBatteryVersionInfo;
				break;
			case 4:
				pnext = &gEmWinListFaultRecordsInfo;
				break;
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
			if(p_msg->press_down_time >= gEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
			{
				pnext = &gEmWinHomePage;
			}
			else
			{
				pnext = &gEmWinMain3DNav;
			}
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win(); 			
		}
		return;
	}
}


