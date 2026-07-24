/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-11
  * @brief       电池信息
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-12    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
  
#define  EM_BACK_ON_ICON_ARROW_SIZE                (4096)  
  
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListBatteryInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
	
    uint16_t    batt_volt;
    uint16_t    batt_current;
    uint16_t    rated_capacity;
    uint16_t    batt_soh;
    uint16_t    batt_soc;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara;

static char _gTitleBuf[] = "Operational State Info < Battery";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";


/*List控件*/
static emListPara_t  _gEmListBatteryInfoPara;

static char _gListBatteryInfoLeftTextBuf[6][20];
static char _gListBatteryInfoRightTextBuf[6][20];


#define WIN_LIST_BATTERY_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_BATTERY_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_BATTERY_INFO_PRESS_ID					       (GUI_ID_USER + 0x14)
#define WIN_LIST_BATTERY_INFO_BACK_ID					       (GUI_ID_USER + 0x15)
#define WIN_LIST_BATTERY_INFO_BACK_IMG_ID					   (GUI_ID_USER + 0x16)

static const GUI_WIDGET_CREATE_INFO _aListGeneratorInputDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListBatteryInfoWin",    WIN_LIST_BATTERY_INFO_ID,     		0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_BATTERY_INFO_TITLE_ID,     		31,  20,  415,  14,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_BATTERY_INFO_PRESS_ID,       		133, 273, 33,   14,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_BATTERY_INFO_BACK_IMG_ID,    		171-5, 275, 15,   10,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_BATTERY_INFO_BACK_ID,       		188, 273, 150,  14,   0,   0x0,  0 },
};

static int32_t _SetBatteryVoltage(uint16_t vol)
{
	snprintf(_gListBatteryInfoRightTextBuf[0], sizeof(_gListBatteryInfoRightTextBuf[0]), "%d", vol);
	TEXT_SetText(_gEmListBatteryInfoPara.handle[1][0], (const char *)_gEmListBatteryInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetBatteryCurrent(uint16_t curr)
{
	snprintf(_gListBatteryInfoRightTextBuf[1], sizeof(_gListBatteryInfoRightTextBuf[1]), "%d", curr);
	TEXT_SetText(_gEmListBatteryInfoPara.handle[1][1], (const char *)_gEmListBatteryInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetBatteryRatedCapacity(uint16_t capacity)
{
	snprintf(_gListBatteryInfoRightTextBuf[2], sizeof(_gListBatteryInfoRightTextBuf[2]), "%d", capacity);
	TEXT_SetText(_gEmListBatteryInfoPara.handle[1][2], (const char *)_gEmListBatteryInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetBatterySOH(uint16_t SOH)
{
	snprintf(_gListBatteryInfoRightTextBuf[3], sizeof(_gListBatteryInfoRightTextBuf[2]), "%d", SOH);
	TEXT_SetText(_gEmListBatteryInfoPara.handle[1][3], (const char *)_gEmListBatteryInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetBatterySOC(uint16_t SOC)
{
	snprintf(_gListBatteryInfoRightTextBuf[4], sizeof(_gListBatteryInfoRightTextBuf[2]), "%d", SOC);
	TEXT_SetText(_gEmListBatteryInfoPara.handle[1][4], (const char *)_gEmListBatteryInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content);
	return 0;
}
/**
  * @brief ListBatteryInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListBatteryInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BATTERY_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListBatteryInfoPara, pMsg->hWin);
			
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BATTERY_INFO_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BATTERY_INFO_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BATTERY_INFO_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
			
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListBatteryInfoPara, _gEmListBatteryInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(int i = 0; i < _gEmListBatteryInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListBatteryInfoPara.sub_region[i].x_pos, 
                                    _gEmListBatteryInfoPara.sub_region[i].y_pos, 
                                    _gEmListBatteryInfoPara.sub_region[i].x_pos + _gEmListBatteryInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListBatteryInfoPara.sub_region[i].y_pos + _gEmListBatteryInfoPara.sub_region[i].y_size - 1,
                                    _gEmListBatteryInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListBatteryInfoPara, _gEmListBatteryInfoPara.list_sel);
			
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
	pPrevEmWin = &gEmWinListEquipOperaInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListBatteryInfo.win_handle = GUI_CreateDialogBox(_aListGeneratorInputDialogCreate, GUI_COUNTOF(_aListGeneratorInputDialogCreate), _cbListBatteryInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListBatteryInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListBatteryInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListBatteryInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListBatteryInfoPara, gEmWinListBatteryInfo.win_handle);
    return gEmWinListBatteryInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListBatteryInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListBatteryInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListBatteryInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	g_ui_ctx.batt_volt = gCollectParaData.pcs_input_front_data.batt_volt;
	g_ui_ctx.batt_current = gCollectParaData.pcs_input_front_data.batt_current;

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
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListBatteryInfoPara.list_start_id = WIN_LIST_BATTERY_INFO_ID + 0x30;
    _gEmListBatteryInfoPara.list_num = 5;
    _gEmListBatteryInfoPara.list_display_num = 6;
    if(_gEmListBatteryInfoPara.list_num <= _gEmListBatteryInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListBatteryInfoPara.list_num + 1 - _gEmListBatteryInfoPara.list_display_num;
    }
    
    if(_gEmListBatteryInfoPara.list_sel > _gEmListBatteryInfoPara.list_num - 1)
    {
        _gEmListBatteryInfoPara.list_sel = 0;
    }
    if(_gEmListBatteryInfoPara.list_page > max_page - 1)
    {
        _gEmListBatteryInfoPara.list_page = 0;
    }
    _gEmListBatteryInfoPara.list_bk_color = 0x00787878;
	/*List选中框*/
    _gEmListBatteryInfoPara.selected_r = 10;
	
    /*4个子列表*/
    _gEmListBatteryInfoPara.sub_region[0].x_pos = 20;
    _gEmListBatteryInfoPara.sub_region[0].y_pos = 38;
    _gEmListBatteryInfoPara.sub_region[0].x_size = 440;
    _gEmListBatteryInfoPara.sub_region[0].y_size = 34;
    _gEmListBatteryInfoPara.sub_region[1].x_pos = 20;
    _gEmListBatteryInfoPara.sub_region[1].y_pos = 80;
    _gEmListBatteryInfoPara.sub_region[1].x_size = 440;
    _gEmListBatteryInfoPara.sub_region[1].y_size = 34;
    _gEmListBatteryInfoPara.sub_region[2].x_pos = 20;
    _gEmListBatteryInfoPara.sub_region[2].y_pos = 122;
    _gEmListBatteryInfoPara.sub_region[2].x_size = 440;
    _gEmListBatteryInfoPara.sub_region[2].y_size = 34;
    _gEmListBatteryInfoPara.sub_region[3].x_pos = 20;
    _gEmListBatteryInfoPara.sub_region[3].y_pos = 164;
    _gEmListBatteryInfoPara.sub_region[3].x_size = 440;
    _gEmListBatteryInfoPara.sub_region[3].y_size = 34;
	_gEmListBatteryInfoPara.sub_region[4].x_pos = 20;
    _gEmListBatteryInfoPara.sub_region[4].y_pos = 206;
    _gEmListBatteryInfoPara.sub_region[4].x_size = 440;
    _gEmListBatteryInfoPara.sub_region[4].y_size = 34;

    /*List控件*/

    //第一行左边
    _gEmListBatteryInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListBatteryInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListBatteryInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListBatteryInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListBatteryInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListBatteryInfoLeftTextBuf[0];
    _gEmListBatteryInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBatteryInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "BAT Voltage");
    //第一行右边
    _gEmListBatteryInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryInfoPara.sub_widget_para[1][0].x_relative_pos = 220;
    _gEmListBatteryInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListBatteryInfoPara.sub_widget_para[1][0].x_size = 208;
    _gEmListBatteryInfoPara.sub_widget_para[1][0].y_size = 17;
	_gEmListBatteryInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListBatteryInfoRightTextBuf[0];
    _gEmListBatteryInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBatteryInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 50;//测试数据，实际赋值系统数据
	snprintf(_gListBatteryInfoRightTextBuf[0], sizeof(_gListBatteryInfoRightTextBuf[0]), 
			 "%d", g_ui_ctx.batt_volt);
    //第二行左边
    _gEmListBatteryInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListBatteryInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListBatteryInfoPara.sub_widget_para[0][1].x_size = 158;
    _gEmListBatteryInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListBatteryInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListBatteryInfoLeftTextBuf[1];
    _gEmListBatteryInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBatteryInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "BAT Current");
    //第二行右边
    _gEmListBatteryInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryInfoPara.sub_widget_para[1][1].x_relative_pos = 220;
    _gEmListBatteryInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gEmListBatteryInfoPara.sub_widget_para[1][1].x_size = 208;
    _gEmListBatteryInfoPara.sub_widget_para[1][1].y_size = 17;
	_gEmListBatteryInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListBatteryInfoRightTextBuf[1];
    _gEmListBatteryInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBatteryInfoPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
	snprintf(_gListBatteryInfoRightTextBuf[1], sizeof(_gListBatteryInfoRightTextBuf[1]), 
			 "%d", g_ui_ctx.batt_current);
    //第三行左边
    _gEmListBatteryInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListBatteryInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListBatteryInfoPara.sub_widget_para[0][2].x_size = 158;
    _gEmListBatteryInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListBatteryInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListBatteryInfoLeftTextBuf[2];
    _gEmListBatteryInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBatteryInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Rated Capacity");
    //第三行右边
    _gEmListBatteryInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryInfoPara.sub_widget_para[1][2].x_relative_pos = 220;
    _gEmListBatteryInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gEmListBatteryInfoPara.sub_widget_para[1][2].x_size = 208;
    _gEmListBatteryInfoPara.sub_widget_para[1][2].y_size = 17;
	_gEmListBatteryInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListBatteryInfoRightTextBuf[2];
    _gEmListBatteryInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBatteryInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 100;//测试数据，实际赋值系统数据
    sprintf(_gEmListBatteryInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content, "%dAh", (uint16_t)_gEmListBatteryInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value);
    //第四行左边
    _gEmListBatteryInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListBatteryInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListBatteryInfoPara.sub_widget_para[0][3].x_size = 158;
    _gEmListBatteryInfoPara.sub_widget_para[0][3].y_size = 17;
    _gEmListBatteryInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListBatteryInfoLeftTextBuf[3];
    _gEmListBatteryInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBatteryInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "SOH");
    //第四行右边
    _gEmListBatteryInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryInfoPara.sub_widget_para[1][3].x_relative_pos = 220;
    _gEmListBatteryInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gEmListBatteryInfoPara.sub_widget_para[1][3].x_size = 208;
    _gEmListBatteryInfoPara.sub_widget_para[1][3].y_size = 17;
	_gEmListBatteryInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _gListBatteryInfoRightTextBuf[3];
    _gEmListBatteryInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBatteryInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 100;//测试数据，实际赋值系统数据
    sprintf(_gEmListBatteryInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content, "%d%%", (uint16_t)_gEmListBatteryInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value);
	//第五行左边
    _gEmListBatteryInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListBatteryInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListBatteryInfoPara.sub_widget_para[0][4].x_size = 158;
    _gEmListBatteryInfoPara.sub_widget_para[0][4].y_size = 17;
    _gEmListBatteryInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListBatteryInfoLeftTextBuf[4];
    _gEmListBatteryInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListBatteryInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "SOC");
    //第五行右边
    _gEmListBatteryInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListBatteryInfoPara.sub_widget_para[1][4].x_relative_pos = 220;
    _gEmListBatteryInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gEmListBatteryInfoPara.sub_widget_para[1][4].x_size = 208;
    _gEmListBatteryInfoPara.sub_widget_para[1][4].y_size = 17;
	_gEmListBatteryInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _gListBatteryInfoRightTextBuf[4];
    _gEmListBatteryInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListBatteryInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListBatteryInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 85;//测试数据，实际赋值系统数据
    sprintf(_gEmListBatteryInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content, "%d%%", (uint16_t)_gEmListBatteryInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
     GUI_ALLOC_Free(_gReturnImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
	if (g_ui_ctx.batt_volt != gCollectParaData.pcs_input_front_data.batt_volt)
	{
		g_ui_ctx.batt_volt = gCollectParaData.pcs_input_front_data.batt_volt;
		_SetBatteryVoltage(g_ui_ctx.batt_volt);
	}

	if (g_ui_ctx.batt_current != gCollectParaData.pcs_input_front_data.batt_current)
	{
		g_ui_ctx.batt_current = gCollectParaData.pcs_input_front_data.batt_current;
		_SetBatteryCurrent(g_ui_ctx.batt_current);
	}

#if 0
	if (g_ui_ctx.rated_capacity != gCollectParaData.collect_data.rated_capacity)
	{
		g_ui_ctx.rated_capacity = gCollectParaData.collect_data.rated_capacity;
		_SetBatteryRatedCapacity(g_ui_ctx.rated_capacity);
	}

	if (g_ui_ctx.batt_soh != gCollectParaData.collect_data.batt_soh)
	{
		g_ui_ctx.batt_soh = gCollectParaData.collect_data.batt_soh;
		_SetBatterySOH(g_ui_ctx.batt_soh);
	}

	if (g_ui_ctx.batt_soc != gCollectParaData.collect_data.batt_soc)
	{
		g_ui_ctx.batt_soh = gCollectParaData.collect_data.batt_soc;
		_SetBatterySOC(g_ui_ctx.batt_soc);
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

	if (event == EN_KEY_SRC_ESC)
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
				pnext = &gEmWinListEquipOperaInfo;
			}
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win(); 
		}
		return;
	}
}

