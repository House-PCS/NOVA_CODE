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
 
#define  EM_BACK_ON_ICON_ARROW_SIZE                		(4096)   
  
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListPVInputInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	uint16_t pv_ch1_input_curr; // PV1输入电流
	uint16_t pv_ch1_input_volt; // PV1输入电压
    uint16_t pv_ch1_input_power;// PV1输入功率

    uint16_t pv_ch2_input_curr; // PV2输入电流
    uint16_t pv_ch2_input_volt; // PV2输入电压
    uint16_t pv_ch2_input_power;// PV2输入功率

} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

static emWidgetTextPara_t  _gTitleTextPara;
//static emWidgetTextPara_t  _gPressTextPara;
//static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 

static char _gTitleBuf[] = "Operational State Info < PV Input";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";

/*List控件*/
static emListPara_t  _gEmListPVInputInfoPara;

static char _gListPVInputInfoLeftTextBuf[6][20];
static char _gListPVInputInfoRightTextBuf[6][20];

static char _gListPVInputInfoHeadTextBuf[2][20];

/*PV Input中的线条信息*/
#define WIN_LIST_PV_INPUT_LINE_SIZE					(3)
#define WIN_LIST_PV_INPUT_LINE_LEN					(66)

#define WIN_LIST_PV1_INPUT_LINE_X0POS				(79)
#define WIN_LIST_PV1_INPUT_LINE_Y0POS				(50)
#define WIN_LIST_PV1_INPUT_LINE_X1POS				(79) 
#define WIN_LIST_PV1_INPUT_LINE_Y1POS				(50 + 66 -1)

#define WIN_LIST_PV2_INPUT_LINE_X0POS				(79)
#define WIN_LIST_PV2_INPUT_LINE_Y0POS				(146)
#define WIN_LIST_PV2_INPUT_LINE_X1POS				(79)
#define WIN_LIST_PV2_INPUT_LINE_Y1POS				(146 + 66 -1)


#define WIN_LIST_PV_INPUT_INFO_ID                              	 (GUI_ID_USER + 0x12)
#define WIN_LIST_PV_INPUT_INFO_TITLE_ID                        	 (GUI_ID_USER + 0x13)
#define WIN_LIST_PV_INPUT_INFO_PRESS_ID					      	 (GUI_ID_USER + 0x14)
#define WIN_LIST_PV_INPUT_INFO_BACK_ID					      	 (GUI_ID_USER + 0x15)
#define WIN_LIST_PV_INPUT_INFO_BACK_IMG_ID					     (GUI_ID_USER + 0x16)

static const GUI_WIDGET_CREATE_INFO _aListPVInputInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListPVInputInfoWin",    WIN_LIST_PV_INPUT_INFO_ID,     	0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_PV_INPUT_INFO_TITLE_ID,     	31,  20, 415,  14,   0,   0x0,  0 },
//	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_PV_INPUT_INFO_PRESS_ID,       	133, 273, 33,   14,   0,   0x0,  0 },
//	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_PV_INPUT_INFO_BACK_ID,    		171-5, 275, 15,   10,   0,   0x0,  0 },
//	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_PV_INPUT_INFO_BACK_IMG_ID,     188, 273, 150,  14,   0,   0x0,  0 },
};


static int32_t _SetPv1Cur(uint16_t cur)
{
	float _cur = (float)cur/10;
    snprintf(_gListPVInputInfoLeftTextBuf[0], sizeof(_gListPVInputInfoLeftTextBuf[0]), 
			"%.1fA", _cur);
	TEXT_SetText(_gEmListPVInputInfoPara.handle[0][0], 
				(const char *)_gEmListPVInputInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetPv1Vol(uint16_t vol)
{
	float _vol = (float)vol/10;
    snprintf(_gListPVInputInfoLeftTextBuf[1], sizeof(_gListPVInputInfoLeftTextBuf[1]), 
			"%.1fV", _vol);
	TEXT_SetText(_gEmListPVInputInfoPara.handle[0][1], 
				(const char *)_gEmListPVInputInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetPv1Power(uint16_t power)
{
    snprintf(_gListPVInputInfoLeftTextBuf[2], sizeof(_gListPVInputInfoLeftTextBuf[2]), 
			"%dW", power);
	TEXT_SetText(_gEmListPVInputInfoPara.handle[0][2], 
				(const char *)_gEmListPVInputInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetPv2Cur(uint16_t cur)
{
	float _cur = (float)cur/10;
    snprintf(_gListPVInputInfoLeftTextBuf[3], sizeof(_gListPVInputInfoLeftTextBuf[3]), 
			"%.1fA", _cur);
	TEXT_SetText(_gEmListPVInputInfoPara.handle[0][3], 
				(const char *)_gEmListPVInputInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetPv2Vol(uint16_t vol)
{
	float _vol = (float)vol/10;
    snprintf(_gListPVInputInfoLeftTextBuf[4], sizeof(_gListPVInputInfoLeftTextBuf[4]), 
			"%.1fV", _vol);
	TEXT_SetText(_gEmListPVInputInfoPara.handle[0][4], 
				(const char *)_gEmListPVInputInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetPv2Power(uint16_t power)
{
    snprintf(_gListPVInputInfoLeftTextBuf[5], sizeof(_gListPVInputInfoLeftTextBuf[5]), 
			"%d", power);
	TEXT_SetText(_gEmListPVInputInfoPara.handle[0][5], 
				(const char *)_gEmListPVInputInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content);
	return 0;
}


/**
  * @brief ListPVInputInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListPVInputInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;

    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_PV_INPUT_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListPVInputInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListPVInputInfoPara, _gEmListPVInputInfoPara.list_pre_sel);
			GUI_SetColor(0x00424242);//上色
			for(int i = 0; i < _gEmListPVInputInfoPara.list_display_num; i++)
			{
			   GUI_FillRoundedRect(_gEmListPVInputInfoPara.sub_region[i].x_pos, 
								   _gEmListPVInputInfoPara.sub_region[i].y_pos, 
								   _gEmListPVInputInfoPara.sub_region[i].x_pos + _gEmListPVInputInfoPara.sub_region[i].x_size - 1, 
								   _gEmListPVInputInfoPara.sub_region[i].y_pos + _gEmListPVInputInfoPara.sub_region[i].y_size - 1,
								   _gEmListPVInputInfoPara.selected_r);
			}
			// SelectedDrawInWMPaint(&_gEmListPVInputInfoPara, _gEmListPVInputInfoPara.list_sel);
			GUI_SetPenSize(WIN_LIST_PV_INPUT_LINE_SIZE);
			GUI_SetColor(0x00A2A856);
			GUI_DrawLine(WIN_LIST_PV1_INPUT_LINE_X0POS,WIN_LIST_PV1_INPUT_LINE_Y0POS,WIN_LIST_PV1_INPUT_LINE_X1POS,WIN_LIST_PV1_INPUT_LINE_Y1POS);
			GUI_DrawLine(WIN_LIST_PV2_INPUT_LINE_X0POS,WIN_LIST_PV2_INPUT_LINE_Y0POS,WIN_LIST_PV2_INPUT_LINE_X1POS,WIN_LIST_PV2_INPUT_LINE_Y1POS);
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
    gEmWinListPVInputInfo.win_handle = GUI_CreateDialogBox(_aListPVInputInfoDialogCreate, GUI_COUNTOF(_aListPVInputInfoDialogCreate), _cbListPVInputInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListPVInputInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListPVInputInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListPVInputInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListPVInputInfoPara, gEmWinListPVInputInfo.win_handle);
    return gEmWinListPVInputInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListPVInputInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListPVInputInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListPVInputInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
	g_ui_ctx.pv_ch1_input_curr = gCollectParaData.pcs_input_back_data.pv_ch1_input_curr;
	g_ui_ctx.pv_ch1_input_volt = gCollectParaData.pcs_input_back_data.pv_ch1_input_volt;
	g_ui_ctx.pv_ch1_input_power = gCollectParaData.pcs_input_back_data.pv_ch1_input_power;
	g_ui_ctx.pv_ch2_input_curr = gCollectParaData.pcs_input_back_data.pv_ch2_input_curr;
	g_ui_ctx.pv_ch2_input_volt = gCollectParaData.pcs_input_back_data.pv_ch1_input_volt;
	g_ui_ctx.pv_ch2_input_power = gCollectParaData.pcs_input_back_data.pv_ch2_input_power;
	EmKeyDefaultInfoSet();
	/*底部back的参数*/
	//_gPressTextPara.p_content = _gPressBuf;
    //_gPressTextPara.text_color = 0x00EEEEEE;
    //_gPressTextPara.p_text_font = EM_TEXT_FONT_1;
	
	//_gBackTextPara.p_content = _gBackBuf;
    //_gBackTextPara.text_color = 0x00EEEEEE;
    //_gBackTextPara.p_text_font = EM_TEXT_FONT_1;
	
	_gReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_BACK_ON_ICON_ARROW_SIZE);
    _gReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gReturnImgPara.mem_img);
    _gReturnImgPara.img_size = EM_BACK_ON_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size);
	
	/*标题相关参数*/
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00EEEEEE;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListPVInputInfoPara.list_start_id = WIN_LIST_PV_INPUT_INFO_ID + 0x30;
    _gEmListPVInputInfoPara.list_num = 6;
    _gEmListPVInputInfoPara.list_display_num = 6;
    
    if(_gEmListPVInputInfoPara.list_sel > _gEmListPVInputInfoPara.list_num - 1)
    {
        _gEmListPVInputInfoPara.list_sel = 0;
    }
    _gEmListPVInputInfoPara.list_bk_color = 0x00787878;
	/*List选中框*/
    _gEmListPVInputInfoPara.selected_r = 10;
	
    /*4个子列表*/
    _gEmListPVInputInfoPara.sub_region[0].x_pos = 20;
    _gEmListPVInputInfoPara.sub_region[0].y_pos = 38;
    _gEmListPVInputInfoPara.sub_region[0].x_size = 440;
    _gEmListPVInputInfoPara.sub_region[0].y_size = 88;
    _gEmListPVInputInfoPara.sub_region[1].x_pos = 20;
    _gEmListPVInputInfoPara.sub_region[1].y_pos = 38;
    _gEmListPVInputInfoPara.sub_region[1].x_size = 440;
    _gEmListPVInputInfoPara.sub_region[1].y_size = 88;
	_gEmListPVInputInfoPara.sub_region[2].x_pos = 20;
    _gEmListPVInputInfoPara.sub_region[2].y_pos = 38;
    _gEmListPVInputInfoPara.sub_region[2].x_size = 440;
    _gEmListPVInputInfoPara.sub_region[2].y_size = 88;
    _gEmListPVInputInfoPara.sub_region[3].x_pos = 20;
    _gEmListPVInputInfoPara.sub_region[3].y_pos = 134;
    _gEmListPVInputInfoPara.sub_region[3].x_size = 440;
    _gEmListPVInputInfoPara.sub_region[3].y_size = 88;
	_gEmListPVInputInfoPara.sub_region[4].x_pos = 20;
    _gEmListPVInputInfoPara.sub_region[4].y_pos = 134;
    _gEmListPVInputInfoPara.sub_region[4].x_size = 440;
    _gEmListPVInputInfoPara.sub_region[4].y_size = 88;
    _gEmListPVInputInfoPara.sub_region[5].x_pos = 20;
    _gEmListPVInputInfoPara.sub_region[5].y_pos = 134;
    _gEmListPVInputInfoPara.sub_region[5].x_size = 440;
    _gEmListPVInputInfoPara.sub_region[5].y_size = 88;


    /*List控件*/

    //第一行左边
    _gEmListPVInputInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[0][0].x_relative_pos = 73;
    _gEmListPVInputInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListPVInputInfoPara.sub_widget_para[0][0].x_size = 136;
    _gEmListPVInputInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListPVInputInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListPVInputInfoLeftTextBuf[0];
    _gEmListPVInputInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListPVInputInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Current");
	//第一行右边
    _gEmListPVInputInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[1][0].x_relative_pos = 224;
    _gEmListPVInputInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListPVInputInfoPara.sub_widget_para[1][0].x_size = 196;
    _gEmListPVInputInfoPara.sub_widget_para[1][0].y_size = 17;
	_gEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListPVInputInfoRightTextBuf[0];
    _gEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    snprintf(_gListPVInputInfoLeftTextBuf[0], sizeof(_gListPVInputInfoLeftTextBuf[0]), 
			"%.1fA", (float)(g_ui_ctx.pv_ch1_input_curr/10));
    sprintf(_gEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "%dA", (uint16_t)_gEmListPVInputInfoPara.sub_widget_para[1][0].sub_widget.text_para.digit_value);
	  
    //第二行左边
    _gEmListPVInputInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[0][1].x_relative_pos = 73;
    _gEmListPVInputInfoPara.sub_widget_para[0][1].y_relative_pos = 40;
    _gEmListPVInputInfoPara.sub_widget_para[0][1].x_size = 136;
    _gEmListPVInputInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListPVInputInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListPVInputInfoLeftTextBuf[1];
    _gEmListPVInputInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListPVInputInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Voltage");
    //第二行右边
    _gEmListPVInputInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[1][1].x_relative_pos = 224;
    _gEmListPVInputInfoPara.sub_widget_para[1][1].y_relative_pos = 40;
    _gEmListPVInputInfoPara.sub_widget_para[1][1].x_size = 196;
    _gEmListPVInputInfoPara.sub_widget_para[1][1].y_size = 17;
	_gEmListPVInputInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListPVInputInfoRightTextBuf[1];
    _gEmListPVInputInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListPVInputInfoPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    snprintf(_gListPVInputInfoLeftTextBuf[1], sizeof(_gListPVInputInfoLeftTextBuf[1]), 
			"%.1fV", (float)(g_ui_ctx.pv_ch1_input_volt/10));
	
	//第三行左边
    _gEmListPVInputInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[0][2].x_relative_pos = 73;
    _gEmListPVInputInfoPara.sub_widget_para[0][2].y_relative_pos = 65;
    _gEmListPVInputInfoPara.sub_widget_para[0][2].x_size = 136;
    _gEmListPVInputInfoPara.sub_widget_para[0][2].y_size = 17;
    _gEmListPVInputInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListPVInputInfoLeftTextBuf[2];
    _gEmListPVInputInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListPVInputInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Power");
    //第三行右边
    _gEmListPVInputInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[1][2].x_relative_pos = 224;
    _gEmListPVInputInfoPara.sub_widget_para[1][2].y_relative_pos = 65;
    _gEmListPVInputInfoPara.sub_widget_para[1][2].x_size = 196;
    _gEmListPVInputInfoPara.sub_widget_para[1][2].y_size = 17;
	_gEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListPVInputInfoRightTextBuf[2];
    _gEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListPVInputInfoPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 3000;//测试数据，实际赋值系统数据
    snprintf(_gListPVInputInfoLeftTextBuf[2], sizeof(_gListPVInputInfoLeftTextBuf[2]), 
			"%dW", g_ui_ctx.pv_ch1_input_power);
	
	
	//第四行左边
    _gEmListPVInputInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[0][3].x_relative_pos = 73;
    _gEmListPVInputInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListPVInputInfoPara.sub_widget_para[0][3].x_size = 136;
    _gEmListPVInputInfoPara.sub_widget_para[0][3].y_size = 17;
    _gEmListPVInputInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListPVInputInfoLeftTextBuf[3];
    _gEmListPVInputInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListPVInputInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Current");
	//第四行右边
    _gEmListPVInputInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[1][3].x_relative_pos = 224;
    _gEmListPVInputInfoPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gEmListPVInputInfoPara.sub_widget_para[1][3].x_size = 196;
    _gEmListPVInputInfoPara.sub_widget_para[1][3].y_size = 17;
	_gEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _gListPVInputInfoRightTextBuf[3];
    _gEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListPVInputInfoPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    snprintf(_gListPVInputInfoLeftTextBuf[3], sizeof(_gListPVInputInfoLeftTextBuf[3]), 
			"%.1fA", (float)(g_ui_ctx.pv_ch2_input_curr/10));
	  
    //第五行左边
    _gEmListPVInputInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[0][4].x_relative_pos = 73;
    _gEmListPVInputInfoPara.sub_widget_para[0][4].y_relative_pos = 40;
    _gEmListPVInputInfoPara.sub_widget_para[0][4].x_size = 136;
    _gEmListPVInputInfoPara.sub_widget_para[0][4].y_size = 17;
    _gEmListPVInputInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListPVInputInfoLeftTextBuf[4];
    _gEmListPVInputInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListPVInputInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Voltage");
    //第五行右边
    _gEmListPVInputInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[1][4].x_relative_pos = 224;
    _gEmListPVInputInfoPara.sub_widget_para[1][4].y_relative_pos = 40;
    _gEmListPVInputInfoPara.sub_widget_para[1][4].x_size = 196;
    _gEmListPVInputInfoPara.sub_widget_para[1][4].y_size = 17;
	_gEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _gListPVInputInfoRightTextBuf[4];
    _gEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListPVInputInfoPara.sub_widget_para[1][4].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
    snprintf(_gListPVInputInfoLeftTextBuf[4], sizeof(_gListPVInputInfoLeftTextBuf[4]), 
			"%.1fV", (float)(g_ui_ctx.pv_ch2_input_volt/10));
	
	//第六行左边
    _gEmListPVInputInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[0][5].x_relative_pos = 73;
    _gEmListPVInputInfoPara.sub_widget_para[0][5].y_relative_pos = 65;
    _gEmListPVInputInfoPara.sub_widget_para[0][5].x_size = 136;
    _gEmListPVInputInfoPara.sub_widget_para[0][5].y_size = 17;
    _gEmListPVInputInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListPVInputInfoLeftTextBuf[5];
    _gEmListPVInputInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListPVInputInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Power");
    //第六行右边
    _gEmListPVInputInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListPVInputInfoPara.sub_widget_para[1][5].x_relative_pos = 224;
    _gEmListPVInputInfoPara.sub_widget_para[1][5].y_relative_pos = 65;
    _gEmListPVInputInfoPara.sub_widget_para[1][5].x_size = 196;
    _gEmListPVInputInfoPara.sub_widget_para[1][5].y_size = 17;
	_gEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _gListPVInputInfoRightTextBuf[5];
    _gEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListPVInputInfoPara.sub_widget_para[1][5].sub_widget.text_para.digit_value = 3000;//测试数据，实际赋值系统数据
    snprintf(_gListPVInputInfoLeftTextBuf[5], sizeof(_gListPVInputInfoLeftTextBuf[5]), 
			"%d", g_ui_ctx.pv_ch2_input_power);


    //PV1标题
	_gEmListPVInputInfoPara.sub_widget_para[2][0].widget_type = EM_WIDGET_TYPE_TEXT;
	_gEmListPVInputInfoPara.sub_widget_para[2][0].x_relative_pos = 15;
    _gEmListPVInputInfoPara.sub_widget_para[2][0].y_relative_pos = 37;
    _gEmListPVInputInfoPara.sub_widget_para[2][0].x_size = 35;
    _gEmListPVInputInfoPara.sub_widget_para[2][0].y_size = 17;
    _gEmListPVInputInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content = _gListPVInputInfoHeadTextBuf[0];
    _gEmListPVInputInfoPara.sub_widget_para[2][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListPVInputInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content, "PV1");
	//PV2标题
	_gEmListPVInputInfoPara.sub_widget_para[2][3].widget_type = EM_WIDGET_TYPE_TEXT;
	_gEmListPVInputInfoPara.sub_widget_para[2][3].x_relative_pos = 15;
    _gEmListPVInputInfoPara.sub_widget_para[2][3].y_relative_pos = 37;
    _gEmListPVInputInfoPara.sub_widget_para[2][3].x_size = 35;
    _gEmListPVInputInfoPara.sub_widget_para[2][3].y_size = 17;
    _gEmListPVInputInfoPara.sub_widget_para[2][3].sub_widget.text_para.p_content = _gListPVInputInfoHeadTextBuf[1];
    _gEmListPVInputInfoPara.sub_widget_para[2][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListPVInputInfoPara.sub_widget_para[2][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListPVInputInfoPara.sub_widget_para[2][3].sub_widget.text_para.p_content, "PV2");
	
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
	if (g_ui_ctx.pv_ch1_input_curr != gCollectParaData.pcs_input_back_data.pv_ch1_input_curr)
	{
		g_ui_ctx.pv_ch1_input_curr = gCollectParaData.pcs_input_back_data.pv_ch1_input_curr;
		_SetPv1Cur(g_ui_ctx.pv_ch1_input_curr);
	}

	if (g_ui_ctx.pv_ch1_input_volt != gCollectParaData.pcs_input_back_data.pv_ch1_input_volt)
	{
		g_ui_ctx.pv_ch1_input_volt = gCollectParaData.pcs_input_back_data.pv_ch1_input_volt;
		_SetPv1Vol(g_ui_ctx.pv_ch1_input_volt);
	}

	if (g_ui_ctx.pv_ch1_input_power != gCollectParaData.pcs_input_back_data.pv_ch1_input_power)
	{
		g_ui_ctx.pv_ch1_input_power = gCollectParaData.pcs_input_back_data.pv_ch1_input_power;
		_SetPv1Power(g_ui_ctx.pv_ch1_input_power);
	}

	if (g_ui_ctx.pv_ch2_input_curr != gCollectParaData.pcs_input_back_data.pv_ch2_input_curr)
	{
		g_ui_ctx.pv_ch2_input_curr = gCollectParaData.pcs_input_back_data.pv_ch2_input_curr;
		_SetPv2Cur(g_ui_ctx.pv_ch2_input_curr);
	}

	if (g_ui_ctx.pv_ch2_input_volt != gCollectParaData.pcs_input_back_data.pv_ch1_input_volt)
	{
		g_ui_ctx.pv_ch2_input_volt = gCollectParaData.pcs_input_back_data.pv_ch1_input_volt;
		_SetPv2Vol(g_ui_ctx.pv_ch2_input_volt);
	}
	if (g_ui_ctx.pv_ch2_input_power != gCollectParaData.pcs_input_back_data.pv_ch2_input_power)
	{
		g_ui_ctx.pv_ch2_input_power = gCollectParaData.pcs_input_back_data.pv_ch2_input_power;
		_SetPv2Power(g_ui_ctx.pv_ch2_input_power);
	}

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

