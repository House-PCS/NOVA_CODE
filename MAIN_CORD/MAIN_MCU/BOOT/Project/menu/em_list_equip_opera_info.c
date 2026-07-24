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


emWin_t  gEmWinListEquipOperaInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
	
	uint16_t bus_voltage;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;

static char _gTitleBuf[] = "Operational State Info";

/*List控件*/
static emListPara_t  _gEmListEquipOperaInfoPara;

static char _gListEquipOperaInfoLeftTextBuf[11][20];
static char _gListEquipOperaInfoRightText3Buf[8];
static GUI_HMEM _gListEquipOperaInfoRightMem;
static uint8_t *_pListEquipOperaInfoRightBuf;

#define WIN_LIST_EQUIP_OPERA_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_EQUIP_OPERA_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static const GUI_WIDGET_CREATE_INFO _aListEquipOperaInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListEquipOperaInfoWin",    WIN_LIST_EQUIP_OPERA_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_EQUIP_OPERA_INFO_TITLE_ID,     31,  20, 200,  20,   0,   0x0,  0 },  //144, 14
};


static int32_t _SetBusVoltage(uint16_t value)
{
	
	snprintf(_gListEquipOperaInfoRightText3Buf, sizeof(_gListEquipOperaInfoRightText3Buf), "%.1fV", (float)value);
	
//	value *= 100; 
//	
//	_gEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.digit_value = value / 100;
//	sprintf(_gEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.p_content, "%.1fV", \
//		    _gEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.digit_value);
//	
	if(_gEmListEquipOperaInfoPara.list_page == 5)
	{
		TEXT_SetText(_gEmListEquipOperaInfoPara.handle[1][5],(const char *)_gEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.p_content);
	}

	
	return 0;
}



/**
  * @brief ListEquipOperaInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListEquipOperaInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_EQUIP_OPERA_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListEquipOperaInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListEquipOperaInfoPara, _gEmListEquipOperaInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
		
			for(int i = 0; i < _gEmListEquipOperaInfoPara.list_display_num ; i++)
			{
				GUI_FillRoundedRect(_gEmListEquipOperaInfoPara.sub_region[i].x_pos, 
								_gEmListEquipOperaInfoPara.sub_region[i].y_pos, 
								_gEmListEquipOperaInfoPara.sub_region[i].x_pos + _gEmListEquipOperaInfoPara.sub_region[i].x_size - 1, 
								_gEmListEquipOperaInfoPara.sub_region[i].y_pos + _gEmListEquipOperaInfoPara.sub_region[i].y_size - 1,
								_gEmListEquipOperaInfoPara.selected_r);
			}
			SelectedDrawInWMPaint(&_gEmListEquipOperaInfoPara, _gEmListEquipOperaInfoPara.list_sel);
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
//    GUI_EnableAlpha(1);
    gEmWinListEquipOperaInfo.win_handle = GUI_CreateDialogBox(_aListEquipOperaInfoDialogCreate, GUI_COUNTOF(_aListEquipOperaInfoDialogCreate), _cbListEquipOperaInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListEquipOperaInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListEquipOperaInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListEquipOperaInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListEquipOperaInfoPara, gEmWinListEquipOperaInfo.win_handle);
    return gEmWinListEquipOperaInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListEquipOperaInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListEquipOperaInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListEquipOperaInfo.win_handle = 0;
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
	uint8_t i,j;
    
	g_ui_ctx.bus_voltage = gCollectParaData.pcs_input_front_data.bus_voltage;
	
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00EEEEEE;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListEquipOperaInfoPara.list_start_id = WIN_LIST_EQUIP_OPERA_INFO_ID + 0x30;
    _gEmListEquipOperaInfoPara.list_num = 11;
    _gEmListEquipOperaInfoPara.list_display_num = 6;
    if(_gEmListEquipOperaInfoPara.list_num <= _gEmListEquipOperaInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListEquipOperaInfoPara.list_num + 1 - _gEmListEquipOperaInfoPara.list_display_num;
    }


    if(_gEmListEquipOperaInfoPara.list_sel > _gEmListEquipOperaInfoPara.list_num - 1)
    {
        _gEmListEquipOperaInfoPara.list_sel = 0;
    }
    if(_gEmListEquipOperaInfoPara.list_page > max_page - 1)
    {
        _gEmListEquipOperaInfoPara.list_page = 0;
    }
    _gEmListEquipOperaInfoPara.list_bk_color = 0x00787878;
    /*6个子列表*/
	_gEmListEquipOperaInfoPara.sub_region[0].x_pos = 20;
    _gEmListEquipOperaInfoPara.sub_region[0].y_pos = 38;
    _gEmListEquipOperaInfoPara.sub_region[0].x_size = 420;
    _gEmListEquipOperaInfoPara.sub_region[0].y_size = 34;
    _gEmListEquipOperaInfoPara.sub_region[1].x_pos = 20;
    _gEmListEquipOperaInfoPara.sub_region[1].y_pos = 80;
    _gEmListEquipOperaInfoPara.sub_region[1].x_size = 420;
    _gEmListEquipOperaInfoPara.sub_region[1].y_size = 34;
    _gEmListEquipOperaInfoPara.sub_region[2].x_pos = 20;
    _gEmListEquipOperaInfoPara.sub_region[2].y_pos = 122;
    _gEmListEquipOperaInfoPara.sub_region[2].x_size = 420;
    _gEmListEquipOperaInfoPara.sub_region[2].y_size = 34;
    _gEmListEquipOperaInfoPara.sub_region[3].x_pos = 20;
    _gEmListEquipOperaInfoPara.sub_region[3].y_pos = 164;
    _gEmListEquipOperaInfoPara.sub_region[3].x_size = 420;
    _gEmListEquipOperaInfoPara.sub_region[3].y_size = 34;
    _gEmListEquipOperaInfoPara.sub_region[4].x_pos = 20;
    _gEmListEquipOperaInfoPara.sub_region[4].y_pos = 206;
    _gEmListEquipOperaInfoPara.sub_region[4].x_size = 420;
    _gEmListEquipOperaInfoPara.sub_region[4].y_size = 34;
    _gEmListEquipOperaInfoPara.sub_region[5].x_pos = 20;
    _gEmListEquipOperaInfoPara.sub_region[5].y_pos = 248;
    _gEmListEquipOperaInfoPara.sub_region[5].x_size = 420;
    _gEmListEquipOperaInfoPara.sub_region[5].y_size = 34;
	
    /*List控件*/
    _gListEquipOperaInfoRightMem = GUI_ALLOC_AllocZero(EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE);
    _pListEquipOperaInfoRightBuf = GUI_ALLOC_h2p(_gListEquipOperaInfoRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListEquipOperaInfoRightBuf, EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE);
	
	
    //第一行左边
    _gEmListEquipOperaInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][0].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][0].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[0];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "PV Input");
    //第一行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][0].x_relative_pos = 395;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][0].x_size = 5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][0].y_size = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第二行左边
    _gEmListEquipOperaInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][1].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][1].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[1];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Grid Info");
    //第二行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][1].x_relative_pos = 395;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][1].x_size = 5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][1].y_size = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第三行左边
    _gEmListEquipOperaInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][2].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][2].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[2];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Generator Input");
    //第三行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][2].x_relative_pos = 395;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][2].y_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][2].x_size = 5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][2].y_size = 10;
	_gEmListEquipOperaInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第四行左边
    _gEmListEquipOperaInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][3].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][3].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][3].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[3];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Back-up Load Output");
    //第四行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][3].x_relative_pos = 395;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][3].y_relative_pos = 13;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][3].x_size = 5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][3].y_size = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第五行左边
    _gEmListEquipOperaInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][4].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][4].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][4].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[4];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Normal Load Output");
    //第五行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][4].x_relative_pos = 395;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][4].y_relative_pos = 13;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][4].x_size = 5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][4].y_size = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第六行左边
    _gEmListEquipOperaInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][5].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][5].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][5].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[5];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Battery");
    //第六行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][5].x_relative_pos = 395;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][5].y_relative_pos = 13;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][5].x_size = 5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][5].y_size = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
	//第七行左边
	_gEmListEquipOperaInfoPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][6].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][6].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][6].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][6].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[6];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Inverter");
    //第七行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][6].x_relative_pos = 395;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][6].y_relative_pos = 13;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][6].x_size = 5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][6].y_size = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
	//第八行左边
	_gEmListEquipOperaInfoPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][7].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][7].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][7].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][7].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[7];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Temperature");
    //第八行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][7].x_relative_pos = 395;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][7].y_relative_pos = 13;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][7].x_size = 5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][7].y_size = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
    //第九行左边
	_gEmListEquipOperaInfoPara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][8].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][8].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][8].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][8].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[8];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][8].sub_widget.text_para.p_content, "Status Info");
    //第九行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][8].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][8].x_relative_pos = 395;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][8].y_relative_pos = 13;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][8].x_size = 5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][8].y_size = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][8].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
	//第十行左边
	_gEmListEquipOperaInfoPara.sub_widget_para[0][9].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][9].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][9].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][9].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][9].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][9].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[9];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][9].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][9].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][9].sub_widget.text_para.p_content, "System Settings");
    //第十行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][9].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][9].x_relative_pos = 395;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][9].y_relative_pos = 13;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][9].x_size = 5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][9].y_size = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][9].sub_widget.img_para.mem_img = _gListEquipOperaInfoRightMem;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][9].sub_widget.img_para.p_img_buf = _pListEquipOperaInfoRightBuf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][9].sub_widget.img_para.img_size = EM_LIST_EQUIP_OPERA_STA_INFO_ICON_ARROW_SIZE;
	//第十一行左边
	_gEmListEquipOperaInfoPara.sub_widget_para[0][10].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][10].x_relative_pos = 12;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][10].y_relative_pos = 10;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][10].x_size = 200;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][10].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][10].sub_widget.text_para.p_content = _gListEquipOperaInfoLeftTextBuf[10];
    _gEmListEquipOperaInfoPara.sub_widget_para[0][10].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListEquipOperaInfoPara.sub_widget_para[0][10].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListEquipOperaInfoPara.sub_widget_para[0][10].sub_widget.text_para.p_content, "Bus Voltage");
    //第十一行右边
    _gEmListEquipOperaInfoPara.sub_widget_para[1][10].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][10].x_relative_pos = 203;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][10].y_relative_pos = 11;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][10].x_size = 197;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][10].y_size = 17;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.p_content = _gListEquipOperaInfoRightText3Buf;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.text_color = 0x00E5E5E5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListEquipOperaInfoPara.sub_widget_para[1][10].sub_widget.text_para.digit_value = 200;//测试数据，实际赋值系统数据
	snprintf(_gListEquipOperaInfoRightText3Buf,sizeof(_gListEquipOperaInfoRightText3Buf),"%.1fV",(float)g_ui_ctx.bus_voltage);
	
    /*List选中框*/
    _gEmListEquipOperaInfoPara.selected_unvalid_color = 0xFF000000;
    _gEmListEquipOperaInfoPara.selected_valid_color = 0x00D6E400;
    _gEmListEquipOperaInfoPara.selected_size = 3;
    _gEmListEquipOperaInfoPara.selected_r = 10;
    _gEmListEquipOperaInfoPara.selected_x_start_offset = -2;
    _gEmListEquipOperaInfoPara.selected_y_start_offset = -2;
    _gEmListEquipOperaInfoPara.selected_x_end_offset = 2;
    _gEmListEquipOperaInfoPara.selected_y_end_offset = 2;
//    _gEmListEquipOperaInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gEmListEquipOperaInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
    /*滚动条*/
    _gEmListEquipOperaInfoPara.scrollbar_enable = true;
    _gEmListEquipOperaInfoPara.scrollbar_color = 0x00CDF100;
    _gEmListEquipOperaInfoPara.scrollbar_bk_color = 0x00565656;
    _gEmListEquipOperaInfoPara.scrollbar_x_pos = 457;
    _gEmListEquipOperaInfoPara.scrollbar_y_pos = 39;
    _gEmListEquipOperaInfoPara.scrollbar_x_size = 6;
    _gEmListEquipOperaInfoPara.scrollbar_y_size = 240;
	
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
    if(gEmWinListEquipOperaInfo.win_handle == 0)
    {
        return;
    }
	
	if(g_ui_ctx.bus_voltage != gCollectParaData.pcs_input_front_data.bus_voltage)
	{
		g_ui_ctx.bus_voltage = gCollectParaData.pcs_input_front_data.bus_voltage;
		_SetBusVoltage(g_ui_ctx.bus_voltage);
	}
	

#if 0
    if(GetSystick() - timecount > 100)    //30
    {
        timecount = GetSystick();
        if(_gEmListEquipOperaInfoPara.list_sel + _gEmListEquipOperaInfoPara.list_page >= _gEmListEquipOperaInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListEquipOperaInfoPara.list_sel + _gEmListEquipOperaInfoPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListEquipOperaInfoPara, gEmWinListEquipOperaInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListEquipOperaInfoPara, gEmWinListEquipOperaInfo.win_handle);
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
		EmListUpward(&_gEmListEquipOperaInfoPara, gEmWinListEquipOperaInfo.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		++g_ui_ctx.select;
		if (g_ui_ctx.select >= _gEmListEquipOperaInfoPara.list_num)
		{
			--g_ui_ctx.select;
			return;
		}
		EmListDownward(&_gEmListEquipOperaInfoPara, gEmWinListEquipOperaInfo.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		emWin_t *pnext = NULL;
		switch(g_ui_ctx.select)
		{
			case 0:
				pnext = &gEmWinListPVInputInfo;
				break;
			case 1:
				pnext = &gEmWinListGridInfo;
				break;
			case 2:
				pnext = &gEmWinListGeneratorInput;
				break;
			case 3:
				pnext = &gEmWinListBackUpLoadInfo;
				break;
			case 4:
				pnext = &gEmWinListNormalLoadInfo;
				break;
			case 5:
				pnext = &gEmWinListBatteryInfo;
				break;
			case 6:
				pnext = &gEmWinListInverterInfo;
				break;
			case 7:
				pnext = &gEmWinListTemperatureInfo;
				break;
			case 8:
				pnext = &gEmWinListStatusInfo;
				break;
			case 9:
				pnext = &gEmWinListSystemSettingsInfo;
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


