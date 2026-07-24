/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-11
  * @brief       Normal负载输出信息
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


emWin_t  gEmWinListNormalLoadInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
	
	uint16_t load_volt;
	uint16_t load_curr;
	uint16_t load_frequency;
	uint16_t load_power;
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

static char _gTitleBuf[] = "Operational State Info < Normal Load Output";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";


/*List控件*/
static emListPara_t  _gEmListNormalLoadPara;

static char _gListNormalLoadLeftTextBuf[6][20];
static char _gListNormalLoadRightTextBuf[6][20];


#define WIN_LIST_NORMAL_LOAD_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_NORMAL_LOAD_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_NORMAL_LOAD_PRESS_ID					      (GUI_ID_USER + 0x14)
#define WIN_LIST_NORMAL_LOAD_BACK_ID					      (GUI_ID_USER + 0x15)
#define WIN_LIST_NORMAL_LOAD_BACK_IMG_ID					  (GUI_ID_USER + 0x16)

static const GUI_WIDGET_CREATE_INFO _aListGeneratorInputDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListNormalLoadInfoWin",    WIN_LIST_NORMAL_LOAD_ID,     		0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_NORMAL_LOAD_TITLE_ID,     			31,  20,  415,  14,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_NORMAL_LOAD_PRESS_ID,       		133, 273, 33,   14,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_NORMAL_LOAD_BACK_IMG_ID,    		171-5, 275, 15,   10,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_NORMAL_LOAD_BACK_ID,       		188, 273, 150,  14,   0,   0x0,  0 },
};

static  int32_t _SetLoadVoltage(uint16_t load_vol)
{
	snprintf(_gListNormalLoadRightTextBuf[0],sizeof(_gListNormalLoadRightTextBuf[0]),"%dV",load_vol);
	TEXT_SetText(_gEmListNormalLoadPara.handle[1][0],(const char *)_gEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
	
	return 0;
}

static  int32_t _SetLoadCurrent(uint16_t load_curr)
{
	snprintf(_gListNormalLoadRightTextBuf[1],sizeof(_gListNormalLoadRightTextBuf[1]),"%dA",load_curr);
	TEXT_SetText(_gEmListNormalLoadPara.handle[1][1],(const char *)_gEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
	
	return 0;
}

static  int32_t _SetLoadFrequency(uint16_t load_freq)
{
	snprintf(_gListNormalLoadRightTextBuf[2],sizeof(_gListNormalLoadRightTextBuf[2]),"%dHz",load_freq);
	TEXT_SetText(_gEmListNormalLoadPara.handle[1][2],(const char *)_gEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
	
	return 0;
}

static  int32_t _SetLoadPower(uint16_t load_power)
{
	snprintf(_gListNormalLoadRightTextBuf[3],sizeof(_gListNormalLoadRightTextBuf[2]),"%dW",load_power);
	TEXT_SetText(_gEmListNormalLoadPara.handle[1][3],(const char *)_gEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.p_content);
	
	return 0;
}

/**
  * @brief ListNormalLoadInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListNormalLoadInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_NORMAL_LOAD_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListNormalLoadPara, pMsg->hWin);
			
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_NORMAL_LOAD_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_NORMAL_LOAD_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_NORMAL_LOAD_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
			
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListNormalLoadPara, _gEmListNormalLoadPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(int i = 0; i < _gEmListNormalLoadPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListNormalLoadPara.sub_region[i].x_pos, 
                                    _gEmListNormalLoadPara.sub_region[i].y_pos, 
                                    _gEmListNormalLoadPara.sub_region[i].x_pos + _gEmListNormalLoadPara.sub_region[i].x_size - 1, 
                                    _gEmListNormalLoadPara.sub_region[i].y_pos + _gEmListNormalLoadPara.sub_region[i].y_size - 1,
                                    _gEmListNormalLoadPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListNormalLoadPara, _gEmListNormalLoadPara.list_sel);
			
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
    gEmWinListNormalLoadInfo.win_handle = GUI_CreateDialogBox(_aListGeneratorInputDialogCreate, GUI_COUNTOF(_aListGeneratorInputDialogCreate), _cbListNormalLoadInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListNormalLoadInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListNormalLoadInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListNormalLoadInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListNormalLoadPara, gEmWinListNormalLoadInfo.win_handle);
    return gEmWinListNormalLoadInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListNormalLoadInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListNormalLoadInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListNormalLoadInfo.win_handle = 0;
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
#if 0
	g_ui_ctx.load_volt = gCollectParaData;
	g_ui_ctx.load_curr = gCollectParaData;
	g_ui_ctx.load_power = gCollectParaData;
	g_ui_ctx.load_frequency = gCollectParaData;
#endif
	
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
    
    _gEmListNormalLoadPara.list_start_id = WIN_LIST_NORMAL_LOAD_ID + 0x30;
    _gEmListNormalLoadPara.list_num = 4;
    _gEmListNormalLoadPara.list_display_num = 6;
    if(_gEmListNormalLoadPara.list_num <= _gEmListNormalLoadPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListNormalLoadPara.list_num + 1 - _gEmListNormalLoadPara.list_display_num;
    }
    
    if(_gEmListNormalLoadPara.list_sel > _gEmListNormalLoadPara.list_num - 1)
    {
        _gEmListNormalLoadPara.list_sel = 0;
    }
    if(_gEmListNormalLoadPara.list_page > max_page - 1)
    {
        _gEmListNormalLoadPara.list_page = 0;
    }
    _gEmListNormalLoadPara.list_bk_color = 0x00787878;
	/*List选中框*/
    _gEmListNormalLoadPara.selected_r = 10;
	
    /*4个子列表*/
    _gEmListNormalLoadPara.sub_region[0].x_pos = 20;
    _gEmListNormalLoadPara.sub_region[0].y_pos = 38;
    _gEmListNormalLoadPara.sub_region[0].x_size = 440;
    _gEmListNormalLoadPara.sub_region[0].y_size = 34;
    _gEmListNormalLoadPara.sub_region[1].x_pos = 20;
    _gEmListNormalLoadPara.sub_region[1].y_pos = 80;
    _gEmListNormalLoadPara.sub_region[1].x_size = 440;
    _gEmListNormalLoadPara.sub_region[1].y_size = 34;
    _gEmListNormalLoadPara.sub_region[2].x_pos = 20;
    _gEmListNormalLoadPara.sub_region[2].y_pos = 122;
    _gEmListNormalLoadPara.sub_region[2].x_size = 440;
    _gEmListNormalLoadPara.sub_region[2].y_size = 34;
    _gEmListNormalLoadPara.sub_region[3].x_pos = 20;
    _gEmListNormalLoadPara.sub_region[3].y_pos = 164;
    _gEmListNormalLoadPara.sub_region[3].x_size = 440;
    _gEmListNormalLoadPara.sub_region[3].y_size = 34;

    /*List控件*/

    //第一行左边
    _gEmListNormalLoadPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListNormalLoadPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListNormalLoadPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListNormalLoadPara.sub_widget_para[0][0].x_size = 158;
    _gEmListNormalLoadPara.sub_widget_para[0][0].y_size = 17;
    _gEmListNormalLoadPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListNormalLoadLeftTextBuf[0];
    _gEmListNormalLoadPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListNormalLoadPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListNormalLoadPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Normal Voltage");
    //第一行右边
    _gEmListNormalLoadPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListNormalLoadPara.sub_widget_para[1][0].x_relative_pos = 220;
    _gEmListNormalLoadPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListNormalLoadPara.sub_widget_para[1][0].x_size = 208;
    _gEmListNormalLoadPara.sub_widget_para[1][0].y_size = 17;
	_gEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListNormalLoadRightTextBuf[0];
    _gEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 220;//测试数据，实际赋值系统数据
//    sprintf(_gEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "%dV", (uint16_t)_gEmListNormalLoadPara.sub_widget_para[1][0].sub_widget.text_para.digit_value);
    snprintf(_gListNormalLoadRightTextBuf[0],sizeof(_gListNormalLoadRightTextBuf[0]),"%dV", g_ui_ctx.load_volt);
   //第二行左边
    _gEmListNormalLoadPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListNormalLoadPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListNormalLoadPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListNormalLoadPara.sub_widget_para[0][1].x_size = 158;
    _gEmListNormalLoadPara.sub_widget_para[0][1].y_size = 17;
    _gEmListNormalLoadPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListNormalLoadLeftTextBuf[1];
    _gEmListNormalLoadPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListNormalLoadPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListNormalLoadPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Normal Current");
    //第二行右边
    _gEmListNormalLoadPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListNormalLoadPara.sub_widget_para[1][1].x_relative_pos = 220;
    _gEmListNormalLoadPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gEmListNormalLoadPara.sub_widget_para[1][1].x_size = 208;
    _gEmListNormalLoadPara.sub_widget_para[1][1].y_size = 17;
	_gEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListNormalLoadRightTextBuf[1];
    _gEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
//    sprintf(_gEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "%dA", (uint16_t)_gEmListNormalLoadPara.sub_widget_para[1][1].sub_widget.text_para.digit_value);
     snprintf(_gListNormalLoadRightTextBuf[1],sizeof(_gListNormalLoadRightTextBuf[1]),"%dA", g_ui_ctx.load_curr);
	//第三行左边
    _gEmListNormalLoadPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListNormalLoadPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListNormalLoadPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListNormalLoadPara.sub_widget_para[0][2].x_size = 158;
    _gEmListNormalLoadPara.sub_widget_para[0][2].y_size = 17;
    _gEmListNormalLoadPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListNormalLoadLeftTextBuf[2];
    _gEmListNormalLoadPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListNormalLoadPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListNormalLoadPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Frequency");
    //第三行右边
    _gEmListNormalLoadPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListNormalLoadPara.sub_widget_para[1][2].x_relative_pos = 220;
    _gEmListNormalLoadPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gEmListNormalLoadPara.sub_widget_para[1][2].x_size = 208;
    _gEmListNormalLoadPara.sub_widget_para[1][2].y_size = 17;
	_gEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _gListNormalLoadRightTextBuf[2];
    _gEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.digit_value = 50;//测试数据，实际赋值系统数据
//    sprintf(_gEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.p_content, "%dHz", (uint16_t)_gEmListNormalLoadPara.sub_widget_para[1][2].sub_widget.text_para.digit_value);
     snprintf(_gListNormalLoadRightTextBuf[2],sizeof(_gListNormalLoadRightTextBuf[2]),"%dHz", g_ui_ctx.load_frequency);
	//第四行左边
    _gEmListNormalLoadPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListNormalLoadPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListNormalLoadPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListNormalLoadPara.sub_widget_para[0][3].x_size = 158;
    _gEmListNormalLoadPara.sub_widget_para[0][3].y_size = 17;
    _gEmListNormalLoadPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListNormalLoadLeftTextBuf[3];
    _gEmListNormalLoadPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListNormalLoadPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListNormalLoadPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Power");
    //第四行右边
    _gEmListNormalLoadPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListNormalLoadPara.sub_widget_para[1][3].x_relative_pos = 220;
    _gEmListNormalLoadPara.sub_widget_para[1][3].y_relative_pos = 10;
    _gEmListNormalLoadPara.sub_widget_para[1][3].x_size = 208;
    _gEmListNormalLoadPara.sub_widget_para[1][3].y_size = 17;
	_gEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _gListNormalLoadRightTextBuf[3];
    _gEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _gEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.digit_value = 5000;//测试数据，实际赋值系统数据
//    sprintf(_gEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.p_content, "%dW", (uint16_t)_gEmListNormalLoadPara.sub_widget_para[1][3].sub_widget.text_para.digit_value);
	snprintf(_gListNormalLoadRightTextBuf[3],sizeof(_gListNormalLoadRightTextBuf[3]),"%dW", g_ui_ctx.load_power);
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
#if 0
	if(g_ui_ctx.load_volt != gCollectParaData)
	{
		g_ui_ctx.load_volt = gCollectParaData;
		_SetLoadVoltage(g_ui_ctx.load_volt);
	}
	if(g_ui_ctx.load_curr != gCollectParaData)
	{
		g_ui_ctx.load_curr = gCollectParaData;
		_SetLoadCurrent(g_ui_ctx.load_curr);
	}
	if(g_ui_ctx.load_power != gCollectParaData)
	{
		g_ui_ctx.load_power = gCollectParaData;
		_SetLoadPower(g_ui_ctx.load_power);
	}

	if(g_ui_ctx.load_frequency != gCollectParaData)
	{
		g_ui_ctx.load_frequency = gCollectParaData;
		_SetLoadFrequency(g_ui_ctx.load_frequency);
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

