/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-12
  * @brief       系统设定信息
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
  
#define  EM_BACK_ON_ICON_ARROW_SIZE                				(4096)  
#define  EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE				    (4096)
  
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gEmWinListSystemSettingsInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
} UiCtx_t;

static emWidgetImgPara_t  _gFlagImgPara[11] = {0}; /*test*/
typedef struct _em_national_flag_para_stru_t
{
	char 			national_name[20];	//国家名称
	int32_t			national_addr;		//国旗地址
	
	//国旗图片所用参数
//	emWidgetImgPara_t FlagImgPara;
////	GUI_HMEM        mem_img;
////    uint8_t         *p_img_buf;
////    uint32_t        img_size;
}_emNationalFalgPara_t;


typedef enum 
{
	EN_GERMANY = 0,
	EN_CHINESE,
	EN_AUSTRALIA,
	EN_ITALY,
	EN_SPAIN,
	EN_FRANCE,
	EN_GREECE,
	EN_HUNGARY,
	EN_BELGIUM,
	EN_THAILAND,
	EN_ENGLAND,
}_ennationaflag_t;


const _emNationalFalgPara_t _gNationalFlagList[11] = 
{
	{"Germany",		EM_ICON_GERMAN_FLAG_ADDR},  	//德国
	{"Chinese",		EM_ICON_CHINESE_FLAG_ADDR},		//中国
	{"Australia",	EM_ICON_AUSTRALIA_FLAG_ADDR},	//澳洲
	{"Italy",		EM_ICON_ITALY_FLAG_ADDR},		//意大利
	{"Spain",		EM_ICON_SPAIN_FLAG_ADDR},		//西班牙
	{"France",		EM_ICON_FRANCE_FLAG_ADDR},		//法国
	{"Greece",		EM_ICON_GREECE_FLAG_ADDR},		//希腊
	{"Hungary",		EM_ICON_HUNGARY_FLAG_ADDR},		//匈牙利
	{"Belgium",		EM_ICON_BELGIUM_FLAG_ADDR},		//比利时
	{"Thailand",	EM_ICON_THAILAND_FLAG_ADDR},	//泰国	
	{"England",		EM_ICON_ENGLAND_FLAG_ADDR},		//英国
};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 
static IMAGE_Handle  _gImgTempratureHandle;//温度图标句柄 

static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 
static emWidgetImgPara_t  _gNationalFlagImgPara;  


static char _gTitleBuf[] = "Operational State Info < System Settings";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";


/*List控件*/
static emListPara_t  _gEmListSystemSettingsInfoPara;

static char _gListSystemSettingsInfoLeftTextBuf[6][20];
static char _gListSystemSettingsInfoRightTextBuf[6][20];


#define WIN_LIST_TEMPERATURE_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_TEMPERATURE_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_TEMPERATURE_INFO_PRESS_ID					       (GUI_ID_USER + 0x14)
#define WIN_LIST_TEMPERATURE_INFO_BACK_ID					       (GUI_ID_USER + 0x15)
#define WIN_LIST_TEMPERATURE_INFO_BACK_IMG_ID					   (GUI_ID_USER + 0x16)

static const GUI_WIDGET_CREATE_INFO _aListGeneratorInputDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListSystemSettingsInfoWin",    WIN_LIST_TEMPERATURE_INFO_ID,     		0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_TEMPERATURE_INFO_TITLE_ID,     		31,  20,  415,  14,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",         	WIN_LIST_TEMPERATURE_INFO_PRESS_ID,       		133, 273, 33,   14,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "BackImg",   		WIN_LIST_TEMPERATURE_INFO_BACK_IMG_ID,    		171-5, 275, 15,   10,   0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Back",          	WIN_LIST_TEMPERATURE_INFO_BACK_ID,       		188, 273, 150,  14,   0,   0x0,  0 },
};


/**
  * @brief ListSystemSettingsInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListSystemSettingsInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListSystemSettingsInfoPara, pMsg->hWin);
			
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
			
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListSystemSettingsInfoPara, _gEmListSystemSettingsInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(int i = 0; i < _gEmListSystemSettingsInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListSystemSettingsInfoPara.sub_region[i].x_pos, 
                                    _gEmListSystemSettingsInfoPara.sub_region[i].y_pos, 
                                    _gEmListSystemSettingsInfoPara.sub_region[i].x_pos + _gEmListSystemSettingsInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListSystemSettingsInfoPara.sub_region[i].y_pos + _gEmListSystemSettingsInfoPara.sub_region[i].y_size - 1,
                                    _gEmListSystemSettingsInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListSystemSettingsInfoPara, _gEmListSystemSettingsInfoPara.list_sel);
			
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
    gEmWinListSystemSettingsInfo.win_handle = GUI_CreateDialogBox(_aListGeneratorInputDialogCreate, GUI_COUNTOF(_aListGeneratorInputDialogCreate), _cbListSystemSettingsInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListSystemSettingsInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListSystemSettingsInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListSystemSettingsInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListSystemSettingsInfoPara, gEmWinListSystemSettingsInfo.win_handle);
    return gEmWinListSystemSettingsInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListSystemSettingsInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListSystemSettingsInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListSystemSettingsInfo.win_handle = 0;
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
	/*国旗图标的参数*/
	_gNationalFlagImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE);
    _gNationalFlagImgPara.p_img_buf = GUI_ALLOC_h2p(_gNationalFlagImgPara.mem_img);
    _gNationalFlagImgPara.img_size = EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_GERMAN_FLAG_ADDR, _gNationalFlagImgPara.p_img_buf, _gNationalFlagImgPara.img_size);
	
	/*标题*/
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListSystemSettingsInfoPara.list_start_id = WIN_LIST_TEMPERATURE_INFO_ID + 0x30;
    _gEmListSystemSettingsInfoPara.list_num = 2;
    _gEmListSystemSettingsInfoPara.list_display_num = 6;
    if(_gEmListSystemSettingsInfoPara.list_num <= _gEmListSystemSettingsInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListSystemSettingsInfoPara.list_num + 1 - _gEmListSystemSettingsInfoPara.list_display_num;
    }
    
    if(_gEmListSystemSettingsInfoPara.list_sel > _gEmListSystemSettingsInfoPara.list_num - 1)
    {
        _gEmListSystemSettingsInfoPara.list_sel = 0;
    }
    if(_gEmListSystemSettingsInfoPara.list_page > max_page - 1)
    {
        _gEmListSystemSettingsInfoPara.list_page = 0;
    }
    _gEmListSystemSettingsInfoPara.list_bk_color = 0x00787878;
	/*List选中框*/
    _gEmListSystemSettingsInfoPara.selected_r = 10;
	
    /*4个子列表*/
    _gEmListSystemSettingsInfoPara.sub_region[0].x_pos = 20;
    _gEmListSystemSettingsInfoPara.sub_region[0].y_pos = 38;
    _gEmListSystemSettingsInfoPara.sub_region[0].x_size = 440;
    _gEmListSystemSettingsInfoPara.sub_region[0].y_size = 34;
    _gEmListSystemSettingsInfoPara.sub_region[1].x_pos = 20;
    _gEmListSystemSettingsInfoPara.sub_region[1].y_pos = 80;
    _gEmListSystemSettingsInfoPara.sub_region[1].x_size = 440;
    _gEmListSystemSettingsInfoPara.sub_region[1].y_size = 34;

    /*List控件*/
    //第一行左边
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListSystemSettingsInfoLeftTextBuf[0];
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSystemSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Working Mode");
    //第一行右边
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][0].x_relative_pos = 218;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][0].x_size = 208;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][0].y_size = 17;
	_gEmListSystemSettingsInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListSystemSettingsInfoRightTextBuf[0];
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListSystemSettingsInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "Self Consumpt");
//    _gEmListSystemSettingsInfoPara.sub_widget_para[2][0].sub_widget.text_para.digit_value = 114;//测试数据，实际赋值系统数据
//    sprintf(_gEmListSystemSettingsInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content, "%dF", (uint16_t)_gEmListSystemSettingsInfoPara.sub_widget_para[2][0].sub_widget.text_para.digit_value);
    //第二行左边
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][1].x_size = 230;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListSystemSettingsInfoLeftTextBuf[1];
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSystemSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListSystemSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Grid-connected Standards");
	//第二行中间图标
	_gEmListSystemSettingsInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][1].x_relative_pos = 320;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][1].y_relative_pos = 2;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][1].x_size = 30;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][1].y_size = 30;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gNationalFlagImgPara.mem_img;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _gNationalFlagImgPara.p_img_buf;
    _gEmListSystemSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE;
    //第二行右边
    _gEmListSystemSettingsInfoPara.sub_widget_para[2][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListSystemSettingsInfoPara.sub_widget_para[2][1].x_relative_pos = 218;
    _gEmListSystemSettingsInfoPara.sub_widget_para[2][1].y_relative_pos = 10;
    _gEmListSystemSettingsInfoPara.sub_widget_para[2][1].x_size = 208;
    _gEmListSystemSettingsInfoPara.sub_widget_para[2][1].y_size = 17;
	_gEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content = _gListSystemSettingsInfoRightTextBuf[1];
    _gEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
	strcpy(_gEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content, "German");
//    _gEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.digit_value = 124;//测试数据，实际赋值系统数据
//    sprintf(_gEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content, "%dF", (uint16_t)_gEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.digit_value);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
	GUI_ALLOC_Free(_gReturnImgPara.mem_img);
    GUI_ALLOC_Free(_gNationalFlagImgPara.mem_img);
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

