/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_battery.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-17
  * @brief       高级设置->电池设置信息
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-17    <td>V1.0.0    <td>           <td>InitialVersion
  * <tr><td>2024-02-23    <td>V1.0.1    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "em_menu.h"
 
#define	 TEST_DYNAMIC						0

#define  EM_LIST_ICON_ARROW_SIZE                 (4096) 
#define  EM_LIST_ICON_SWITCH_SIZE                (4096)
  
static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t  gEmWinListAdvancedBattery = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_t
{
	int32_t select;
	int32_t key_init;
	int32_t waken_time;
	
	uint32_t switcher[2];
	uint16_t batt_waken_enable;
} UiCtx_t;

static UiCtx_t g_ui_ctx = {0};


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static IMAGE_Handle  _gImgSwithHandle;//开关图标句柄 

static emWidgetTextPara_t  _gTitleTextPara;
static char _gTitleBuf[] = "Settings < More Settings < Battery";

/*List控件*/
static emListPara_t  _gEmListAdvancedBatteryPara;

static char _gListAdvancedBatteryLeftTextBuf[11][26];
static char _gListAdvancedBatteryMidTextBuf[20];
static GUI_HMEM _gListAdvancedBatteryRightMem;
static uint8_t *_pListAdvancedBatteryRightBuf;

/*开关图片参数*/
static emWidgetImgPara_t  _gSwtichImgPara[2];  /*0:关	1:开*/

/*开关枚举和函数声明*/
typedef enum _SWITCH_STAT_E
{
	SWITCH_STAT_OFF = 0,
	SWITCH_STAT_ON,
} SWITCH_STAT_E;

int32_t _BatterySetSwitchStat(int32_t row,SWITCH_STAT_E stat);

#define WIN_LIST_ADVANCED_BATTERY_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_ADVANCED_BATTERY_TITLE_ID                         (GUI_ID_USER + 0x11)


static const GUI_WIDGET_CREATE_INFO _aListAdvancedBatteryDialogCreate[] =
{
	{ WINDOW_CreateIndirect,  "ListAdvancedBatteryWin",    WIN_LIST_ADVANCED_BATTERY_ID,    	 0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_ADVANCED_BATTERY_TITLE_ID,     		31,  20,  400,  14,   0,   0x0,  0 },
};

static int32_t _SetBatteryWakenStatus(uint16_t waken_enable)
{
	emWidgetImgPara_t  *tmp_switch_img_para = &_gSwtichImgPara[0];
	switch (waken_enable)
	{
		case SWITCH_STAT_OFF:
		{
			tmp_switch_img_para = &_gSwtichImgPara[0];
			break;
		}
		case SWITCH_STAT_ON:
		{
			tmp_switch_img_para = &_gSwtichImgPara[1];
			break;
		}
		default:
			break;
	}

	_gEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = tmp_switch_img_para->mem_img;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = tmp_switch_img_para->p_img_buf;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
	IMAGE_SetDTA(_gEmListAdvancedBatteryPara.handle[1][4], tmp_switch_img_para->p_img_buf, 
				 tmp_switch_img_para->img_size); 
	return 0;
}


/*函数声明*/
int32_t SetBatteryWakeTime(int32_t value);
extern int32_t GetWakeTimeValue(void);

/**
  * @brief ListAdvancedBatteryWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbAdvancedBatteryWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin, hWinOld, hItem;
	static int     switch_type = 0,row =0;
#if TEST_DYNAMIC
	static WM_HTIMER wm_timer_handle_switch;
#endif 
    switch (pMsg->MsgId) 
    {
        case WM_INIT_DIALOG:
        {
#if TEST_DYNAMIC
			wm_timer_handle_switch = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                             0, 	     /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                             1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                             0);		 /* 留待将来使用，应为0 */ 
#endif 
			
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_BATTERY_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gTitleTextPara.text_color); 
            
            /*LIST*/
            EmListWidgetCreate(&_gEmListAdvancedBatteryPara, pMsg->hWin);
            break;
        }
		case WM_DELETE:
		{
#if TEST_DYNAMIC
            WM_DeleteTimer(wm_timer_handle_switch);//；删除窗口，会自动删除对应内部的定时器
#endif 
			break;
		}
		case WM_TIMER:
        {
#if TEST_DYNAMIC
			++switch_type;
			_BatterySetSwitchStat(7,(SWITCH_STAT_E)(switch_type%2));

            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定1000mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif 
			break;
		}
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListAdvancedBatteryPara, _gEmListAdvancedBatteryPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(int i = 0; i < _gEmListAdvancedBatteryPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListAdvancedBatteryPara.sub_region[i].x_pos, 
                                    _gEmListAdvancedBatteryPara.sub_region[i].y_pos, 
                                    _gEmListAdvancedBatteryPara.sub_region[i].x_pos + _gEmListAdvancedBatteryPara.sub_region[i].x_size - 1, 
                                    _gEmListAdvancedBatteryPara.sub_region[i].y_pos + _gEmListAdvancedBatteryPara.sub_region[i].y_size - 1,
                                    _gEmListAdvancedBatteryPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListAdvancedBatteryPara, _gEmListAdvancedBatteryPara.list_sel);
	
			//获取从子界面得到的电池休眠时间
			if(g_ui_ctx.select == 4)
			{
				g_ui_ctx.waken_time = GetWakeTimeValue();
				SetBatteryWakeTime(g_ui_ctx.waken_time);
			}
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
	pPrevEmWin = &gEmWinListAdvancedSettings;
    _Init();
//    GUI_EnableAlpha(1);
    gEmWinListAdvancedBattery.win_handle = GUI_CreateDialogBox(_aListAdvancedBatteryDialogCreate, GUI_COUNTOF(_aListAdvancedBatteryDialogCreate), _cbAdvancedBatteryWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gEmWinListAdvancedBattery.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gEmWinListAdvancedBattery.win_handle, 0x00000000);
//    WM_SetTransState(gEmWinListAdvancedBattery.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListAdvancedBatteryPara, gEmWinListAdvancedBattery.win_handle);
    return gEmWinListAdvancedBattery.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gEmWinListAdvancedBattery.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gEmWinListAdvancedBattery.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gEmWinListAdvancedBattery.win_handle = 0;
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
    _gTitleTextPara.p_text_font = EM_TEXT_FONT_7;
	
	/*开关图标*/
	_gSwtichImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_SWITCH_SIZE);
    _gSwtichImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gSwtichImgPara[0].mem_img);
	_gSwtichImgPara[0].img_size = EM_LIST_ICON_SWITCH_SIZE;
	SpiFlashReadData(EM_ICON_SET_CLOSE_ADDR, _gSwtichImgPara[0].p_img_buf, _gSwtichImgPara[0].img_size);
	
	_gSwtichImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_SWITCH_SIZE);
    _gSwtichImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gSwtichImgPara[1].mem_img);
	_gSwtichImgPara[1].img_size = EM_LIST_ICON_SWITCH_SIZE;
	SpiFlashReadData(EM_ICON_SET_OPEN_ADDR, _gSwtichImgPara[1].p_img_buf, _gSwtichImgPara[1].img_size);
	
	/*右箭头图标*/	
	_gListAdvancedBatteryRightMem = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_SIZE);
    _pListAdvancedBatteryRightBuf = GUI_ALLOC_h2p(_gListAdvancedBatteryRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListAdvancedBatteryRightBuf, EM_LIST_ICON_ARROW_SIZE);
    
    _gEmListAdvancedBatteryPara.list_start_id = WIN_LIST_ADVANCED_BATTERY_ID + 0x30;
    _gEmListAdvancedBatteryPara.list_num = 5;
    _gEmListAdvancedBatteryPara.list_display_num = 5;
    if(_gEmListAdvancedBatteryPara.list_num <= _gEmListAdvancedBatteryPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListAdvancedBatteryPara.list_num + 1 - _gEmListAdvancedBatteryPara.list_display_num;
    }
    
    if(_gEmListAdvancedBatteryPara.list_sel > _gEmListAdvancedBatteryPara.list_num - 1)
    {
        _gEmListAdvancedBatteryPara.list_sel = 0;
    }
    if(_gEmListAdvancedBatteryPara.list_page > max_page - 1)
    {
        _gEmListAdvancedBatteryPara.list_page = 0;
    }
    _gEmListAdvancedBatteryPara.list_bk_color = 0x00787878;

	/*List选中框*/
    /*5个子列表*/
    _gEmListAdvancedBatteryPara.sub_region[0].x_pos = 20;
    _gEmListAdvancedBatteryPara.sub_region[0].y_pos = 38;
    _gEmListAdvancedBatteryPara.sub_region[0].x_size = 440;
    _gEmListAdvancedBatteryPara.sub_region[0].y_size = 34;
    _gEmListAdvancedBatteryPara.sub_region[1].x_pos = 20;
    _gEmListAdvancedBatteryPara.sub_region[1].y_pos = 80;
    _gEmListAdvancedBatteryPara.sub_region[1].x_size = 440;
    _gEmListAdvancedBatteryPara.sub_region[1].y_size = 34;
    _gEmListAdvancedBatteryPara.sub_region[2].x_pos = 20;
    _gEmListAdvancedBatteryPara.sub_region[2].y_pos = 122;
    _gEmListAdvancedBatteryPara.sub_region[2].x_size = 440;
    _gEmListAdvancedBatteryPara.sub_region[2].y_size = 34;
    _gEmListAdvancedBatteryPara.sub_region[3].x_pos = 20;
    _gEmListAdvancedBatteryPara.sub_region[3].y_pos = 164;
    _gEmListAdvancedBatteryPara.sub_region[3].x_size = 440;
    _gEmListAdvancedBatteryPara.sub_region[3].y_size = 34;
    _gEmListAdvancedBatteryPara.sub_region[4].x_pos = 20;
    _gEmListAdvancedBatteryPara.sub_region[4].y_pos = 206;
    _gEmListAdvancedBatteryPara.sub_region[4].x_size = 440;
    _gEmListAdvancedBatteryPara.sub_region[4].y_size = 34;
    /*List控件*/

    //第一行左边
    _gEmListAdvancedBatteryPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][0].x_size = 210;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][0].y_size = 17;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListAdvancedBatteryLeftTextBuf[0];
    _gEmListAdvancedBatteryPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvancedBatteryPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Discharge Of Depth");
    //第一行右边
    _gEmListAdvancedBatteryPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][0].x_relative_pos = 415;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][0].x_size = 5;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][0].y_size = 10;
	_gEmListAdvancedBatteryPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gListAdvancedBatteryRightMem;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _pListAdvancedBatteryRightBuf;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_SIZE;
	//第二行左边
    _gEmListAdvancedBatteryPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][1].x_size = 260;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][1].y_size = 17;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListAdvancedBatteryLeftTextBuf[1];
    _gEmListAdvancedBatteryPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvancedBatteryPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Charge/Discharge Current");
    //第二行右边
    _gEmListAdvancedBatteryPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][1].x_relative_pos = 415;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][1].x_size = 5;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][1].y_size = 10;
	_gEmListAdvancedBatteryPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListAdvancedBatteryRightMem;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListAdvancedBatteryRightBuf;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_SIZE;
    //第三行左边
    _gEmListAdvancedBatteryPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][2].x_size = 260;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][2].y_size = 17;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListAdvancedBatteryLeftTextBuf[2];
    _gEmListAdvancedBatteryPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvancedBatteryPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Charge/Discharge Power");
    //第三行右边
    _gEmListAdvancedBatteryPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][2].x_relative_pos = 415;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][2].y_relative_pos = 12;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][2].x_size = 5;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][2].y_size = 10;
	_gEmListAdvancedBatteryPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gListAdvancedBatteryRightMem;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _pListAdvancedBatteryRightBuf;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_SIZE;
    //第四行左边
    _gEmListAdvancedBatteryPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][3].x_size = 260;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][3].y_size = 17;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListAdvancedBatteryLeftTextBuf[3];
    _gEmListAdvancedBatteryPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvancedBatteryPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Charge/Discharge Voltage");
    //第四行右边
    _gEmListAdvancedBatteryPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][3].x_relative_pos = 415;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][3].y_relative_pos = 12;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][3].x_size = 5;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][3].y_size = 10;
	_gEmListAdvancedBatteryPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gListAdvancedBatteryRightMem;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _pListAdvancedBatteryRightBuf;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_SIZE;
    //第五行左边
    _gEmListAdvancedBatteryPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][4].x_size = 210;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][4].y_size = 17;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListAdvancedBatteryLeftTextBuf[4];
    _gEmListAdvancedBatteryPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvancedBatteryPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvancedBatteryPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Battery Waken");
    //第五行右边
    _gEmListAdvancedBatteryPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][4].x_relative_pos = 386;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][4].y_relative_pos = 9;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][4].x_size = 40;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][4].y_size = 31;
	_gEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gSwtichImgPara[0].mem_img;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _gSwtichImgPara[0].p_img_buf;
    _gEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;	
	//第五行中间   
    _gEmListAdvancedBatteryPara.sub_widget_para[2][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvancedBatteryPara.sub_widget_para[2][4].x_relative_pos = 172;
    _gEmListAdvancedBatteryPara.sub_widget_para[2][4].y_relative_pos = 10;
    _gEmListAdvancedBatteryPara.sub_widget_para[2][4].x_size = 208;
    _gEmListAdvancedBatteryPara.sub_widget_para[2][4].y_size = 17;
    _gEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.p_content = _gListAdvancedBatteryMidTextBuf;
    _gEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.p_content, " ");
	
	  /*List选中框*/
    _gEmListAdvancedBatteryPara.selected_unvalid_color = 0xFF000000;
    _gEmListAdvancedBatteryPara.selected_valid_color = 0x00D6E400;
    _gEmListAdvancedBatteryPara.selected_size = 3;
    _gEmListAdvancedBatteryPara.selected_r = 10;
    _gEmListAdvancedBatteryPara.selected_x_start_offset = -2;
    _gEmListAdvancedBatteryPara.selected_y_start_offset = -2;
    _gEmListAdvancedBatteryPara.selected_x_end_offset = 2;
    _gEmListAdvancedBatteryPara.selected_y_end_offset = 2;
//    _gEmListAdvancedBatteryPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gEmListAdvancedBatteryPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;
	
#if 0
	    /*滚动条*/
    _gEmListAdvancedBatteryPara.scrollbar_enable = true;
    _gEmListAdvancedBatteryPara.scrollbar_color = 0x00D6E400;
    _gEmListAdvancedBatteryPara.scrollbar_bk_color = 0x00565656;
    _gEmListAdvancedBatteryPara.scrollbar_x_pos = 454;
    _gEmListAdvancedBatteryPara.scrollbar_y_pos = 39;
    _gEmListAdvancedBatteryPara.scrollbar_x_size = 6;
    _gEmListAdvancedBatteryPara.scrollbar_y_size = 241;
#endif
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
	GUI_ALLOC_Free(_gListAdvancedBatteryRightMem);
	GUI_ALLOC_Free(_gSwtichImgPara[0].mem_img);
	GUI_ALLOC_Free(_gSwtichImgPara[1].mem_img);
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
    if(gEmWinListAdvancedBattery.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 100)    //30
    {
        timecount = GetSystick();
        if(_gEmListAdvancedBatteryPara.list_sel + _gEmListAdvancedBatteryPara.list_page >= _gEmListAdvancedBatteryPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListAdvancedBatteryPara.list_sel + _gEmListAdvancedBatteryPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListAdvancedBatteryPara, gEmWinListAdvancedBattery.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListAdvancedBatteryPara, gEmWinListAdvancedBattery.win_handle);
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

		EmListUpward(&_gEmListAdvancedBatteryPara, gEmWinListAdvancedBattery.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_RIGHT)
	{
		++g_ui_ctx.select;
		if (g_ui_ctx.select >= _gEmListAdvancedBatteryPara.list_num)
		{
			--g_ui_ctx.select;
			return;
		}

		EmListDownward(&_gEmListAdvancedBatteryPara, gEmWinListAdvancedBattery.win_handle);
		return;
	}
	else if (event == EN_KEY_SRC_CONFIRM)
	{
		emWin_t *pnext = NULL;
		switch(g_ui_ctx.select)
		{
			case 0:
				pnext = &gEmWinListAdvanSettingBatDOD;
				break;
			case 1:
				pnext = &gEmWinListAdvanSettingBatCurrent;
				break;
			case 2:
				pnext = &gEmWinListAdvanSettingBatPower;
				break;
			case 3:
				pnext = &gEmWinListAdvanSettingBatVoltage;
				break;
			case 4:
				pnext = &gEmWinListAdvanSettingBatWakTime;
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
			pPrevEmWin = pCurrentEmWin;
			pCurrentEmWin->delete_win();
			pCurrentEmWin = pnext;
			pCurrentEmWin->create_win(); 
		}
		return;
	}
}



int32_t SetBatteryWakeTime(int32_t value)
{
	if(!g_ui_ctx.waken_time)
	{
		IMAGE_SetDTA(_gEmListAdvancedBatteryPara.handle[1][4], _gSwtichImgPara[0].p_img_buf, 
				 _gSwtichImgPara[0].img_size); 
	}
	else
	{
		IMAGE_SetDTA(_gEmListAdvancedBatteryPara.handle[1][4], _gSwtichImgPara[1].p_img_buf, 
				 _gSwtichImgPara[1].img_size); 

		_gEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.digit_value = value;
		sprintf(_gEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.p_content, "%dMin", 
						(uint16_t)_gEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.digit_value);
		
		TEXT_SetText(_gEmListAdvancedBatteryPara.handle[2][4], (const char *)_gEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.p_content);
	}
	return 0;
}
