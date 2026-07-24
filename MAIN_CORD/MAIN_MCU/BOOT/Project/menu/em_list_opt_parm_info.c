/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opt_parm_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-04
  * @brief       Setting < More Settings < Opt parm
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-04    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
  

#define  EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE               	(4096)
#define  EM_LIST_OPT_PARM_ICON_ARROW_UP_SIZE					(4096)

  
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t  gEmWinListOptPARMInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t list_show;
	int32_t select;
	int32_t key_init;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};

typedef struct _method_idx_t
{
	const char method[16];
} MethodIdx_t;

static MethodIdx_t method_idx[] = 
{
	{"Seamless switch"  },
	{"H-L penetration"	},
	{"Anti-island" },
};



/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static IMAGE_Handle _gImgArrowHandle;//最后一行需要变换的箭头句柄

static emWidgetTextPara_t  _gTitleTextPara;
static emWidgetImgPara_t  _gArrowImgPara[2];
static char _gTitleBuf[] = "Settings < More Settings < Opt PARM";


/*List控件*/
static emListPara_t  _gEmListOptPARMPara;

static char _gListOptPARMLeftTextBuf[12][40];
static char _gListOptPARMMidTextBuf[2][20];


#define WIN_LIST_OPT_PARM_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_OPT_PARM_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_OPT_PARM_ARROW_ID						   (GUI_ID_USER + 0x14)

static const GUI_WIDGET_CREATE_INFO _aListOptPARMDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListOptPARMWin",    WIN_LIST_OPT_PARM_ID,     				0,   	0,   	480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         	WIN_LIST_OPT_PARM_TITLE_ID,     			31,  	20,  	400,  14,   0,   0x0,  0 },
	{ IMAGE_CreateIndirect,   "Arrow",         	WIN_LIST_OPT_PARM_ARROW_ID,     			414,  	260,  	15,   15,   0,   0x0,  0 },
};

/*函数声明*/
void SetGridDropSwitchArrowDirection(void);


/**
  * @brief ListOptPARMWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListOptPARMWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_OPT_PARM_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
			/*右箭头*/
			_gImgArrowHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_OPT_PARM_ARROW_ID);
			IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara[0].p_img_buf, _gArrowImgPara[0].img_size);
			
            /*LIST*/
            EmListWidgetCreate(&_gEmListOptPARMPara, pMsg->hWin);
			
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListOptPARMPara, _gEmListOptPARMPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
			int i = 0;
            for(i = 0; i < _gEmListOptPARMPara.list_display_num; i++)
            {
				GUI_FillRoundedRect(_gEmListOptPARMPara.sub_region[i].x_pos, 
								_gEmListOptPARMPara.sub_region[i].y_pos, 
								_gEmListOptPARMPara.sub_region[i].x_pos + _gEmListOptPARMPara.sub_region[i].x_size - 1, 
								_gEmListOptPARMPara.sub_region[i].y_pos + _gEmListOptPARMPara.sub_region[i].y_size - 1,
								_gEmListOptPARMPara.selected_r);
            }		

			SelectedDrawInWMPaint(&_gEmListOptPARMPara, _gEmListOptPARMPara.list_sel);
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
	pPrevEmWin = &gEmWinListEquipOperaInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListOptPARMInfo.win_handle = GUI_CreateDialogBox(_aListOptPARMDialogCreate, GUI_COUNTOF(_aListOptPARMDialogCreate), _cbListOptPARMWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListOptPARMInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListOptPARMInfo.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListOptPARMInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListOptPARMPara, gEmWinListOptPARMInfo.win_handle);
    return gEmWinListOptPARMInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListOptPARMInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListOptPARMInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListOptPARMInfo.win_handle = 0;
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
 
	
	/*标题*/
    _gTitleTextPara.p_content = _gTitleBuf;
    _gTitleTextPara.text_color = 0x00FFFFFF;
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    //右箭头
	_gArrowImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE);
	_gArrowImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gArrowImgPara[0].mem_img);
	_gArrowImgPara[0].img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _gArrowImgPara[0].p_img_buf, _gArrowImgPara[0].img_size);
	//下箭头
	_gArrowImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_OPT_PARM_ICON_ARROW_UP_SIZE);
    _gArrowImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gArrowImgPara[1].mem_img);
	_gArrowImgPara[1].img_size = EM_LIST_OPT_PARM_ICON_ARROW_UP_SIZE;
	SpiFlashReadData(EM_ICON_UP_ARROW_ADDR, _gArrowImgPara[1].p_img_buf, _gArrowImgPara[1].img_size);
	
    _gEmListOptPARMPara.list_start_id = WIN_LIST_OPT_PARM_ID + 0x30;
    _gEmListOptPARMPara.list_num = 12;
    _gEmListOptPARMPara.list_display_num = 6;
    if(_gEmListOptPARMPara.list_num <= _gEmListOptPARMPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListOptPARMPara.list_num + 1 - _gEmListOptPARMPara.list_display_num;
    }
    
    if(_gEmListOptPARMPara.list_sel > _gEmListOptPARMPara.list_num - 1)
    {
        _gEmListOptPARMPara.list_sel = 0;
    }
    if(_gEmListOptPARMPara.list_page > max_page - 1)
    {
        _gEmListOptPARMPara.list_page = 0;
    }
	
	/*List选中框*/
    /*6个子列表*/
    _gEmListOptPARMPara.sub_region[0].x_pos = 20;
    _gEmListOptPARMPara.sub_region[0].y_pos = 38;
    _gEmListOptPARMPara.sub_region[0].x_size = 420;
    _gEmListOptPARMPara.sub_region[0].y_size = 34;
    _gEmListOptPARMPara.sub_region[1].x_pos = 20;
    _gEmListOptPARMPara.sub_region[1].y_pos = 80;
    _gEmListOptPARMPara.sub_region[1].x_size = 420;
    _gEmListOptPARMPara.sub_region[1].y_size = 34;
	_gEmListOptPARMPara.sub_region[2].x_pos = 20;
    _gEmListOptPARMPara.sub_region[2].y_pos = 122;
    _gEmListOptPARMPara.sub_region[2].x_size = 420;
    _gEmListOptPARMPara.sub_region[2].y_size = 34;
    _gEmListOptPARMPara.sub_region[3].x_pos = 20;
    _gEmListOptPARMPara.sub_region[3].y_pos = 164;
    _gEmListOptPARMPara.sub_region[3].x_size = 420;
    _gEmListOptPARMPara.sub_region[3].y_size = 34;
	_gEmListOptPARMPara.sub_region[4].x_pos = 20;
    _gEmListOptPARMPara.sub_region[4].y_pos = 206;
    _gEmListOptPARMPara.sub_region[4].x_size = 420;
    _gEmListOptPARMPara.sub_region[4].y_size = 34;
    _gEmListOptPARMPara.sub_region[5].x_pos = 20;
    _gEmListOptPARMPara.sub_region[5].y_pos = 248;
    _gEmListOptPARMPara.sub_region[5].x_size = 420;
    _gEmListOptPARMPara.sub_region[5].y_size = 34;


    /*List控件*/
    //第一行左边
    _gEmListOptPARMPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][0].x_size = 230;
    _gEmListOptPARMPara.sub_widget_para[0][0].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[0];
    _gEmListOptPARMPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "On-Gird Power");
    //第一行右边
    _gEmListOptPARMPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][0].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][0].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][0].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;//_gListOptPARMRightMem;
    _gEmListOptPARMPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;//_pListOptPARMRightBuf;
    _gEmListOptPARMPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	 //第二行左边
    _gEmListOptPARMPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][1].x_size = 230;
    _gEmListOptPARMPara.sub_widget_para[0][1].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[1];
    _gEmListOptPARMPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "React Mode");
	//第二行右边
	_gEmListOptPARMPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][1].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][1].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][1].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    //第三行左边
    _gEmListOptPARMPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][2].x_size = 330;
    _gEmListOptPARMPara.sub_widget_para[0][2].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[2];
    _gEmListOptPARMPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Grid Overvoltage protection point");
	//第三行右边
	_gEmListOptPARMPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][2].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][2].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][2].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][2].y_size = 5;
    _gEmListOptPARMPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	//第四行左边
    _gEmListOptPARMPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][3].x_size = 330;
    _gEmListOptPARMPara.sub_widget_para[0][3].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[3];
    _gEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Inverter Undervoltage protection point");
	//第四行右边
	_gEmListOptPARMPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][3].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][3].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][3].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][3].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	//第五行左边
    _gEmListOptPARMPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][4].x_size = 330;
    _gEmListOptPARMPara.sub_widget_para[0][4].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[4];
    _gEmListOptPARMPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Overfrequency protection point");
	//第五行右边
	_gEmListOptPARMPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][4].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][4].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][4].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][4].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	//第六行左边
    _gEmListOptPARMPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][5].x_size = 280;
    _gEmListOptPARMPara.sub_widget_para[0][5].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[5];
    _gEmListOptPARMPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Underfrequency protection point");
	//第六行右边
	_gEmListOptPARMPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][5].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][5].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][5].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][5].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	//第七行左边
    _gEmListOptPARMPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][6].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][6].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][6].x_size = 260;
    _gEmListOptPARMPara.sub_widget_para[0][6].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[6];
    _gEmListOptPARMPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Overvolatge Derating Point");
	//第七行右边
	_gEmListOptPARMPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][6].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][6].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][6].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][6].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	//第八行左边
    _gEmListOptPARMPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][7].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][7].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][7].x_size = 300;
    _gEmListOptPARMPara.sub_widget_para[0][7].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[7];
    _gEmListOptPARMPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Undervoltage Derating Point");
	//第八行右边
	_gEmListOptPARMPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][7].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][7].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][7].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][7].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	//第九行左边
    _gEmListOptPARMPara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][8].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][8].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][8].x_size = 300;
    _gEmListOptPARMPara.sub_widget_para[0][8].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][8].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[8];
    _gEmListOptPARMPara.sub_widget_para[0][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][8].sub_widget.text_para.p_content, "Overfrequency Derating Point");
	//第九行右边
	_gEmListOptPARMPara.sub_widget_para[1][8].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][8].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][8].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][8].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][8].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][8].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][8].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][8].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	//第十行左边
    _gEmListOptPARMPara.sub_widget_para[0][9].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][9].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][9].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][9].x_size = 300;
    _gEmListOptPARMPara.sub_widget_para[0][9].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][9].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[9];
    _gEmListOptPARMPara.sub_widget_para[0][9].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][9].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][9].sub_widget.text_para.p_content, "Underfrequency Derating Point");
	//第十行右边
	_gEmListOptPARMPara.sub_widget_para[1][9].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][9].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][9].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][9].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][9].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][9].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][9].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][9].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	//第十一行左边
    _gEmListOptPARMPara.sub_widget_para[0][10].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][10].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][10].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][10].x_size = 300;
    _gEmListOptPARMPara.sub_widget_para[0][10].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][10].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[10];
    _gEmListOptPARMPara.sub_widget_para[0][10].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][10].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][10].sub_widget.text_para.p_content, "Reactive Response Time");
	//第十一行右边
	_gEmListOptPARMPara.sub_widget_para[1][10].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][10].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][10].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][10].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][10].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][10].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][10].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][10].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
	//第十二行左边
    _gEmListOptPARMPara.sub_widget_para[0][11].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[0][11].x_relative_pos = 11;
    _gEmListOptPARMPara.sub_widget_para[0][11].y_relative_pos = 10;
    _gEmListOptPARMPara.sub_widget_para[0][11].x_size = 300;
    _gEmListOptPARMPara.sub_widget_para[0][11].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[0][11].sub_widget.text_para.p_content = _gListOptPARMLeftTextBuf[11];
    _gEmListOptPARMPara.sub_widget_para[0][11].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[0][11].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[0][11].sub_widget.text_para.p_content, "Grid Drop Switching Method");
#if 0
	//第十二行右边 
	_gEmListOptPARMPara.sub_widget_para[1][11].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListOptPARMPara.sub_widget_para[1][11].x_relative_pos = 394;
    _gEmListOptPARMPara.sub_widget_para[1][11].y_relative_pos = 12;
    _gEmListOptPARMPara.sub_widget_para[1][11].x_size = 10;
    _gEmListOptPARMPara.sub_widget_para[1][11].y_size = 5;
	_gEmListOptPARMPara.sub_widget_para[1][11].sub_widget.img_para.mem_img = _gArrowImgPara[0].mem_img;
    _gEmListOptPARMPara.sub_widget_para[1][11].sub_widget.img_para.p_img_buf = _gArrowImgPara[0].p_img_buf;
    _gEmListOptPARMPara.sub_widget_para[1][11].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
#endif
	//第十二行中间
    _gEmListOptPARMPara.sub_widget_para[2][11].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListOptPARMPara.sub_widget_para[2][11].x_relative_pos = 226;
    _gEmListOptPARMPara.sub_widget_para[2][11].y_relative_pos = 8;
    _gEmListOptPARMPara.sub_widget_para[2][11].x_size = 160;
    _gEmListOptPARMPara.sub_widget_para[2][11].y_size = 17;
    _gEmListOptPARMPara.sub_widget_para[2][11].sub_widget.text_para.p_content = _gListOptPARMMidTextBuf[0];
    _gEmListOptPARMPara.sub_widget_para[2][11].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListOptPARMPara.sub_widget_para[2][11].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListOptPARMPara.sub_widget_para[2][11].sub_widget.text_para.p_content, "Seamless switch");
										

	/*List选中框*/
    _gEmListOptPARMPara.selected_unvalid_color = 0xFF000000;
    _gEmListOptPARMPara.selected_valid_color = 0x00D6E400;
    _gEmListOptPARMPara.selected_size = 3;
    _gEmListOptPARMPara.selected_r = 10;
    _gEmListOptPARMPara.selected_x_start_offset = -2;
    _gEmListOptPARMPara.selected_y_start_offset = -2;
    _gEmListOptPARMPara.selected_x_end_offset = 2;
    _gEmListOptPARMPara.selected_y_end_offset = 2;
//    _gEmListOptPARMPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gEmListOptPARMPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
    
    /*滚动条*/
    _gEmListOptPARMPara.scrollbar_enable = true;
    _gEmListOptPARMPara.scrollbar_color = 0x00D6E400;
    _gEmListOptPARMPara.scrollbar_bk_color = 0x00565656;
    _gEmListOptPARMPara.scrollbar_x_pos = 454;
    _gEmListOptPARMPara.scrollbar_y_pos = 39;
    _gEmListOptPARMPara.scrollbar_x_size = 6;
    _gEmListOptPARMPara.scrollbar_y_size = 241;

}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
	GUI_ALLOC_Free(_gArrowImgPara[0].mem_img);
	GUI_ALLOC_Free(_gArrowImgPara[1].mem_img);
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
    if(gEmWinListOptPARMInfo.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gEmListOptPARMPara.list_sel + _gEmListOptPARMPara.list_page >= _gEmListOptPARMPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListOptPARMPara.list_sel + _gEmListOptPARMPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListOptPARMPara, gEmWinListOptPARMInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListOptPARMPara, gEmWinListOptPARMInfo.win_handle);
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
	if(g_ui_ctx.list_show)
	{
		gEmWinListAdvaSettingOptGridDropSwitch.key_handler(p_msg);
		
		if(!g_ui_ctx.list_show)
		{
			gEmWinListAdvaSettingOptGridDropSwitch.delete_win();
		}
		return;
	}
	
	
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

		EmListUpward(&_gEmListOptPARMPara, gEmWinListOptPARMInfo.win_handle);

		if (_gEmListOptPARMPara.list_num - g_ui_ctx.select - 1
			== _gEmListOptPARMPara.list_display_num)
		{
			WM_Paint(gEmWinListOptPARMInfo.win_handle);
		}
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		++g_ui_ctx.select;
		if (g_ui_ctx.select >= _gEmListOptPARMPara.list_num)
		{
			--g_ui_ctx.select;
			return;
		}

		EmListDownward(&_gEmListOptPARMPara, gEmWinListOptPARMInfo.win_handle);

		if (g_ui_ctx.select == _gEmListOptPARMPara.list_num-1)
		{
			WM_Paint(gEmWinListOptPARMInfo.win_handle);
		}
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		emWin_t *pnext = NULL;
		switch(g_ui_ctx.select)
		{
			case 0:
				pnext = &gEmWinListAdvanSettingOptGridPower;
				break;
			case 1:
				pnext = &gEmWinListAdvanSettingOptReactMode;
				break;
			case 2:
				pnext = &gEmWinListAdvanSettingOptGridOVP;
				break;
			case 3: 
				pnext = &gEmWinListAdvanSettingOptInverterUVP;
				break;
			case 4:
				pnext = &gEmWinListAdvanSettingOptOFPPoint;
				break;
			case 5:
				pnext = &gEmWinListAdvanSettingOptUFPPoint;
				break;
			case 6:
				pnext = &gEmWinListAdvanSettingOptOVDPoint;
				break;
			case 7:
				pnext = &gEmWinListAdvanSettingOptUVDPoint;
				break;
			case 8:
				pnext = &gEmWinListAdvanSettingOptOFDPoint;
				break;
			case 9:
				pnext = &gEmWinListAdvanSettingOptUFDPoint;
				break;
			case 10:
				pnext = &gEmWinListAdvanSettingOptReactvRespTim;
				break;
			case 11:
				{
					if(!g_ui_ctx.list_show)
					{
						gEmWinListAdvaSettingOptGridDropSwitch.create_win();
						g_ui_ctx.list_show =1;
					}
					SetGridDropSwitchArrowDirection();
				}
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
				pnext = &gEmWinListAdvancedSettings;
			}
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win(); 
		}
		return;
	}
}
/**
  * @brief  设置箭头朝向
  * @param  NULL
  * @retval NULL
  */
void SetGridDropSwitchArrowDirection(void)
{
	if(g_ui_ctx.list_show)
	{		
		//箭头向上
		IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara[1].p_img_buf, _gArrowImgPara[1].img_size); 
	}
	else
	{
		//箭头向右
		IMAGE_SetDTA(_gImgArrowHandle, _gArrowImgPara[0].p_img_buf, _gArrowImgPara[0].img_size); 
	} 
}


void SetGridDropSwitchListShow(int32_t show)
{
	g_ui_ctx.list_show = show;
}

int32_t SetCurGridDropSwitchMethod(int32_t index)
{
	snprintf(_gEmListOptPARMPara.sub_widget_para[2][11].sub_widget.text_para.p_content, 
			sizeof(_gListOptPARMMidTextBuf[0]), "%s", method_idx[index].method);
	
	TEXT_SetText(_gEmListOptPARMPara.handle[2][5], (const char *)_gEmListOptPARMPara.sub_widget_para[2][11].sub_widget.text_para.p_content);
	return 0;
}