/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_LIST_ARM_VERSION_INFO.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-25
  * @brief       ARM DSP版本
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-25    <td>V1.0.0    <td>           <td>InitialVersion
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


emWin_t  gEmWinListArmVersionInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;

	uint16_t arm_hw_ver;
	uint16_t arm_sw_ver;
	
	uint16_t dsp_hw_ver;
	uint16_t dsp_sw_ver;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gTitleTextPara;

static char _gTitleBuf[] = "Basic Info < ARM/DSP";

/*List控件*/
static emListPara_t  _gEmListArmVersionInfoPara;

static char _gListArmVersionInfoLeftTextBuf[2][20];
static char _gListArmVersionInfoRightTextBuf[2][20];


#define WIN_LIST_ARM_VERSION_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_ARM_VERSION_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)

static const GUI_WIDGET_CREATE_INFO _aListArmVersionInfoDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListArmVersionInfoWin",    WIN_LIST_ARM_VERSION_INFO_ID,     0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_ARM_VERSION_INFO_TITLE_ID,     31,  20, 415,  17,   0,   0x0,  0 },
};

static int32_t _SetArmVer(uint16_t hw, uint16_t sw)
{
    snprintf(_gListArmVersionInfoRightTextBuf[0], sizeof(_gListArmVersionInfoRightTextBuf[0]), 
			"%4X | %4X", hw, sw);
	TEXT_SetText(_gEmListArmVersionInfoPara.handle[1][0], 
				(const char *)_gEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
	return 0;
}

static int32_t _SetDspVer(uint16_t hw, uint16_t sw)
{
    snprintf(_gListArmVersionInfoRightTextBuf[1], sizeof(_gListArmVersionInfoRightTextBuf[1]), 
			"%4X | %4X", hw, sw);
	TEXT_SetText(_gEmListArmVersionInfoPara.handle[1][1], 
				(const char *)_gEmListArmVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
	return 0;
}



/**
  * @brief ListArmVersionInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListArmVersionInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ARM_VERSION_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListArmVersionInfoPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListArmVersionInfoPara, _gEmListArmVersionInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(int i = 0; i < _gEmListArmVersionInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListArmVersionInfoPara.sub_region[i].x_pos, 
                                    _gEmListArmVersionInfoPara.sub_region[i].y_pos, 
                                    _gEmListArmVersionInfoPara.sub_region[i].x_pos + _gEmListArmVersionInfoPara.sub_region[i].x_size - 1, 
                                    _gEmListArmVersionInfoPara.sub_region[i].y_pos + _gEmListArmVersionInfoPara.sub_region[i].y_size - 1,
                                    _gEmListArmVersionInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListArmVersionInfoPara, _gEmListArmVersionInfoPara.list_sel);
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
	pPrevEmWin = &gEmWinListDevBaseInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListArmVersionInfo.win_handle = GUI_CreateDialogBox(_aListArmVersionInfoDialogCreate, GUI_COUNTOF(_aListArmVersionInfoDialogCreate), _cbListArmVersionInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListArmVersionInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListArmVersionInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListArmVersionInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListArmVersionInfoPara, gEmWinListArmVersionInfo.win_handle);
    return gEmWinListArmVersionInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListArmVersionInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListArmVersionInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListArmVersionInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;

	g_ui_ctx.arm_hw_ver = gCollectParaData.collect_data.hw_ver;
	g_ui_ctx.arm_sw_ver = gCollectParaData.collect_data.software_ver;

	g_ui_ctx.dsp_hw_ver = gCollectParaData.pcs_input_front_data.hw_version;
	g_ui_ctx.dsp_sw_ver = gCollectParaData.pcs_input_front_data.software_ver;
    
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    
    _gEmListArmVersionInfoPara.list_start_id = WIN_LIST_ARM_VERSION_INFO_ID + 0x30;
    _gEmListArmVersionInfoPara.list_num = 2;
    _gEmListArmVersionInfoPara.list_display_num = 6;
    if(_gEmListArmVersionInfoPara.list_num <= _gEmListArmVersionInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListArmVersionInfoPara.list_num + 1 - _gEmListArmVersionInfoPara.list_display_num;
    }
    
    if(_gEmListArmVersionInfoPara.list_sel > _gEmListArmVersionInfoPara.list_num - 1)
    {
        _gEmListArmVersionInfoPara.list_sel = 0;
    }
    if(_gEmListArmVersionInfoPara.list_page > max_page - 1)
    {
        _gEmListArmVersionInfoPara.list_page = 0;
    }
    _gEmListArmVersionInfoPara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gEmListArmVersionInfoPara.sub_region[0].x_pos = 20;
    _gEmListArmVersionInfoPara.sub_region[0].y_pos = 38;
    _gEmListArmVersionInfoPara.sub_region[0].x_size = 440;
    _gEmListArmVersionInfoPara.sub_region[0].y_size = 34;
    _gEmListArmVersionInfoPara.sub_region[1].x_pos = 20;
    _gEmListArmVersionInfoPara.sub_region[1].y_pos = 80;
    _gEmListArmVersionInfoPara.sub_region[1].x_size = 440;
    _gEmListArmVersionInfoPara.sub_region[1].y_size = 34;

    /*List控件*/

    //第一行左边
    _gEmListArmVersionInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListArmVersionInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListArmVersionInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListArmVersionInfoPara.sub_widget_para[0][0].x_size = 158;
    _gEmListArmVersionInfoPara.sub_widget_para[0][0].y_size = 17;
    _gEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListArmVersionInfoLeftTextBuf[0];
    _gEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListArmVersionInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "ARM Version");
    //第一行右边
    _gEmListArmVersionInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListArmVersionInfoPara.sub_widget_para[1][0].x_relative_pos = 218;
    _gEmListArmVersionInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gEmListArmVersionInfoPara.sub_widget_para[1][0].x_size = 210;
    _gEmListArmVersionInfoPara.sub_widget_para[1][0].y_size = 17;
    _gEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _gListArmVersionInfoRightTextBuf[0];
    _gEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListArmVersionInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    snprintf(_gListArmVersionInfoRightTextBuf[0], sizeof(_gListArmVersionInfoRightTextBuf[0]), 
			"%4X | %4X", g_ui_ctx.arm_hw_ver, g_ui_ctx.arm_sw_ver);
    //第二行左边
    _gEmListArmVersionInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListArmVersionInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListArmVersionInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListArmVersionInfoPara.sub_widget_para[0][1].x_size = 158;
    _gEmListArmVersionInfoPara.sub_widget_para[0][1].y_size = 17;
    _gEmListArmVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListArmVersionInfoLeftTextBuf[1];
    _gEmListArmVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListArmVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListArmVersionInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "DSP Version");
    //第二行右边
    _gEmListArmVersionInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListArmVersionInfoPara.sub_widget_para[1][1].x_relative_pos = 218;
    _gEmListArmVersionInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gEmListArmVersionInfoPara.sub_widget_para[1][1].x_size = 210;
    _gEmListArmVersionInfoPara.sub_widget_para[1][1].y_size = 17;
    _gEmListArmVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _gListArmVersionInfoRightTextBuf[1];
    _gEmListArmVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListArmVersionInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    snprintf(_gListArmVersionInfoRightTextBuf[1], sizeof(_gListArmVersionInfoRightTextBuf[1]), 
			"%4X | %4X", g_ui_ctx.dsp_hw_ver, g_ui_ctx.dsp_sw_ver);
	
	_gEmListArmVersionInfoPara.selected_r = 10;
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
    if (g_ui_ctx.arm_hw_ver != gCollectParaData.collect_data.hw_ver
		|| g_ui_ctx.arm_sw_ver != gCollectParaData.collect_data.software_ver)
	{

		g_ui_ctx.arm_hw_ver = gCollectParaData.collect_data.hw_ver;
		g_ui_ctx.arm_sw_ver = gCollectParaData.collect_data.software_ver;
		_SetArmVer(g_ui_ctx.arm_hw_ver, g_ui_ctx.arm_sw_ver);
	}

    if (g_ui_ctx.dsp_hw_ver != gCollectParaData.pcs_input_front_data.hw_version
		|| g_ui_ctx.dsp_sw_ver != gCollectParaData.pcs_input_front_data.software_ver)
	{
		g_ui_ctx.dsp_hw_ver = gCollectParaData.pcs_input_front_data.hw_version;
		g_ui_ctx.dsp_sw_ver = gCollectParaData.pcs_input_front_data.software_ver;
		_SetDspVer(g_ui_ctx.dsp_hw_ver, g_ui_ctx.dsp_sw_ver);
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
	if(event < 0 || event > EN_KEY_SRC_NUM)
	{
		return;
	}
	
	if(event == EN_KEY_SRC_ESC)
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
				pnext = &gEmWinListDevBaseInfo;
			}
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win(); 
		}
		return;
	}
}


