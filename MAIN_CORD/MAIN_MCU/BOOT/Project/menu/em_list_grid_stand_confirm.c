/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-18
  * @brief       高级设置->电网标准确认
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-18    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
  
#define		EM_BACK_ON_ICON_ARROW_SIZE                		(4096)  
#define 	EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE			(4096)
#define 	EM_LIST_ICON_ARROW_SIZE							(4096)

#define 	REC_XPOS			(20)
#define 	REC_YPOS			(38)
 
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gEmWinListGridStandConfirm = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄 
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄 

/*GridStandard列表*/
static TEXT_Handle  _gTextGridStandardHandle;//显示GridStandard句柄
static TEXT_Handle  _gTextCountryHandle;//显示GridStandard句柄
static IMAGE_Handle  _gImgNationaHandle;//国旗图标句柄 
static IMAGE_Handle  _gImgArrowHandle;//箭头图标句柄 


static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetTextPara_t  _gPressTextPara;
static emWidgetTextPara_t  _gBackTextPara;
static emWidgetImgPara_t  _gReturnImgPara; 

static emWidgetTextPara_t  _gGridStandardTextPara;
static emWidgetTextPara_t  _gCountryTextPara;
static emWidgetImgPara_t  _gFlagImgPara;
static emWidgetImgPara_t  _gArrowImgPara; 


static char _gTitleBuf[] = "Setting < Grid Standard";
static char _gPressBuf[] = "Press";
static char _gBackBuf[] = "return back to last page.";
static char _gPasswordBuf[] = "Password";
static char _gGridStandardBuf[] = "Grid Standard";
static char _gCountryBuf[] = "China";


#define WIN_LIST_GRID_STAND_CONFIRM_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_GRID_STAND_CONFIRM_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_GRID_STAND_CONFIRM_PRESS_ID					     (GUI_ID_USER + 0x14)
#define WIN_LIST_GRID_STAND_CONFIRM_BACK_ID					         (GUI_ID_USER + 0x15)
#define WIN_LIST_GRID_STAND_CONFIRM_BACK_IMG_ID					     (GUI_ID_USER + 0x16)

#define WIN_LIST_GRID_STAND_CONFIRM_TEXT_ID							 (GUI_ID_USER + 0x20)
#define WIN_LIST_GRID_STAND_CONGIRM_FLAG_IMG_ID					     (GUI_ID_USER + 0x21)
#define WIN_LIST_GRID_STAND_CONFIRM_COUNTRY_TEXT_ID					 (GUI_ID_USER + 0x22)
#define WIN_LIST_GRID_STAND_CONFIRM_ARROW_IMG_ID					 (GUI_ID_USER + 0x23)


/*资源列表*/
static const GUI_WIDGET_CREATE_INFO _aListGridStandConfirmDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListGridStandConfirmWin",    WIN_LIST_GRID_STAND_CONFIRM_ID,     				0,   	0,   	480,	320,  	0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         				WIN_LIST_GRID_STAND_CONFIRM_TITLE_ID,     			31,  	20,  	415,  	14,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Press",         				WIN_LIST_GRID_STAND_CONFIRM_PRESS_ID,       		133, 	273, 	33,   	14,   	0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "BackImg",   					WIN_LIST_GRID_STAND_CONFIRM_BACK_IMG_ID,    		171-5, 	275, 	15,  	10,   	0,   0x0,  0 },
	{ TEXT_CreateIndirect,    "Back",          				WIN_LIST_GRID_STAND_CONFIRM_BACK_ID,       			188, 	273, 	150, 	14,  	0,   0x0,  0 },
	{ TEXT_CreateIndirect,	  "GRID",						WIN_LIST_GRID_STAND_CONFIRM_TEXT_ID,				31,		48,		152,	17,		0,	 0x0,  0 },
	{ IMAGE_CreateIndirect,   "Imgfalg",					WIN_LIST_GRID_STAND_CONGIRM_FLAG_IMG_ID,			338,	41,		30,		30,		0,	 0x0,  0 },
	{ TEXT_CreateIndirect,    "COUNTRY",					WIN_LIST_GRID_STAND_CONFIRM_COUNTRY_TEXT_ID,        374,	48,		50,		17,		0,	 0x0,  0 },
	{ IMAGE_CreateIndirect,	  "ARROW",						WIN_LIST_GRID_STAND_CONFIRM_ARROW_IMG_ID,			437,	50,		15,		15,		0,	 0x0,  0 }
};



/**
  * @brief ListGridStandConfirmWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListGridStandConfirmWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;

    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
			
			/*中间的列表*/
			_gTextGridStandardHandle = 	WM_GetDialogItem(pMsg->hWin,WIN_LIST_GRID_STAND_CONFIRM_TEXT_ID);
			TEXT_SetFont(_gTextGridStandardHandle, _gGridStandardTextPara.p_text_font);
            TEXT_SetText(_gTextGridStandardHandle, (const char *)_gGridStandardTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridStandardHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridStandardHandle, _gGridStandardTextPara.text_color); 
			
			_gTextCountryHandle = 	WM_GetDialogItem(pMsg->hWin,WIN_LIST_GRID_STAND_CONFIRM_COUNTRY_TEXT_ID);
			TEXT_SetFont(_gTextCountryHandle, _gCountryTextPara.p_text_font);
            TEXT_SetText(_gTextCountryHandle, (const char *)_gCountryTextPara.p_content);
            TEXT_SetTextAlign(_gTextCountryHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextCountryHandle, _gCountryTextPara.text_color); 
			
			_gImgNationaHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONGIRM_FLAG_IMG_ID);
            IMAGE_SetDTA(_gImgNationaHandle, _gFlagImgPara.p_img_buf, _gFlagImgPara.img_size); 			
			
			_gImgArrowHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_ARROW_IMG_ID);
            IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara.p_img_buf, _gArrowImgPara.img_size); 
		
			/*底部return back*/
			_gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_BACK_ID);
			TEXT_SetFont(_gBackHandle, _gBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gBackTextPara.text_color); 
			/*底部Press*/
			_gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_PRESS_ID);
			TEXT_SetFont(_gPressHandle, _gPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gPressTextPara.text_color); 
			/*底部图标*/
			_gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_CONFIRM_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gReturnImgPara.p_img_buf, _gReturnImgPara.img_size); 
			
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListGridStandConfirmPara, _gEmListGridStandConfirmPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
			GUI_FillRoundedRect(REC_XPOS, REC_YPOS,  REC_XPOS + 440 - 1,  REC_YPOS + 34 - 1, 10);
			GUI_SetColor(0x00BDC909);
			GUI_DrawRoundedFrame(REC_XPOS, REC_YPOS,  REC_XPOS + 440 - 1,  REC_YPOS + 34 - 1, 10,3);
			
//            SelectedDrawInWMPaint(&_gEmListGridStandConfirmPara, _gEmListGridStandConfirmPara.list_sel);
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
//    GUI_EnableAlpha(1);
    gEmWinListGridStandConfirm.win_handle = GUI_CreateDialogBox(_aListGridStandConfirmDialogCreate, GUI_COUNTOF(_aListGridStandConfirmDialogCreate), _cbListGridStandConfirmWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListGridStandConfirm.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListGridStandConfirm.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListGridStandConfirm.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
//    SelectedDraw(&_gEmListGridStandConfirmPara, gEmWinListGridStandConfirm.win_handle);
    return gEmWinListGridStandConfirm.win_handle;
	
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListGridStandConfirm.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListGridStandConfirm.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListGridStandConfirm.win_handle = 0;
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
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
	
	/*gridstand*/
	_gGridStandardTextPara.p_text_font = EM_TEXT_FONT_5;
	_gGridStandardTextPara.text_color = 0x00FFFFFF;
	_gGridStandardTextPara.p_content = _gGridStandardBuf;
	
	/*country*/
	_gCountryTextPara.p_text_font = EM_TEXT_FONT_5;
	_gCountryTextPara.text_color = 0x00FFFFFF;
	_gCountryTextPara.p_content = _gCountryBuf;
	
	/*国旗*/
	_gFlagImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE);
	_gFlagImgPara.p_img_buf = GUI_ALLOC_h2p(_gFlagImgPara.mem_img);
	_gFlagImgPara.img_size = EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE;
	SpiFlashReadData(EM_ICON_CHINESE_FLAG_ADDR,_gFlagImgPara.p_img_buf,_gFlagImgPara.img_size);
	/*箭头*/
	_gArrowImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_SIZE);
	_gArrowImgPara.p_img_buf = GUI_ALLOC_h2p(_gArrowImgPara.mem_img);
	_gArrowImgPara.img_size = EM_LIST_ICON_ARROW_SIZE;
	SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR,_gArrowImgPara.p_img_buf,_gArrowImgPara.img_size);

	/*标题*/
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00EEEEEE;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gReturnImgPara.mem_img);
	GUI_ALLOC_Free(_gArrowImgPara.mem_img);
	GUI_ALLOC_Free(_gFlagImgPara.mem_img);
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
    
}

