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

#define  TEST_DYNAMIC                       0

#define  EM_LIST_ICON_ARROW_SIZE                 (4096)
#define  EM_LIST_ICON_SWITCH_SIZE                (4096)

#define  EM_REMOVE_FIVE_LINE_INFO                (0)


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListAdvancedBattery = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    int32_t waken_time;
    uint32_t switcher[2];
    uint16_t batt_waken_enable;
} UiCtx_t;

static UiCtx_t _gstUiCtx;


/*标题控件*/
static TEXT_Handle _gTitleHandle;//标题句柄
static IMAGE_Handle _gImgSwithHandle;//开关图标句柄

static emWidgetTextPara_t _gstTitleTextPara;
static char _garrTitleBuf[] = "Settings < More < Battery";

/*List控件*/
static emListPara_t _gstEmListAdvancedBatteryPara;

static char _garrListAdvancedBatteryLeftTextBuf[5][30];
static char _garrListAdvancedBatteryMidTextBuf[20];
static GUI_HMEM _gListAdvancedBatteryRightMem;
static uint8_t *_pListAdvancedBatteryRightBuf;

/*开关图片参数*/
static emWidgetImgPara_t _garrSwtichImgPara[2];  /*0:关   1:开*/

/*开关枚举和函数声明*/
typedef enum
{
    EM_SWITCH_STAT_OFF = 0,
    EN_SWITCH_STAT_ON,
} EN_SWITCH_STAT_E;


#define WIN_LIST_ADVANCED_BATTERY_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_ADVANCED_BATTERY_TITLE_ID                         (GUI_ID_USER + 0x11)


static __SDRAM GUI_WIDGET_CREATE_INFO _garrListAdvancedBatteryDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListAdvancedBatteryWin",    WIN_LIST_ADVANCED_BATTERY_ID,         0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",         WIN_LIST_ADVANCED_BATTERY_TITLE_ID,          31,  20,  400,  14,   0,   0x0,  0 },
};

#if 0
//目前暂未实现此功能
/*函数声明*/
extern int32_t GetWakeTimeValue(void);
#endif

static int32_t _SetBatteryWakenStatus(uint16_t waken_enable)
{
    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];
    switch (waken_enable)
    {
        case EM_SWITCH_STAT_OFF:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];
            break;
        }
        case EN_SWITCH_STAT_ON:
        {
            p_st_tmp_switch_img_para = &_garrSwtichImgPara[1];
            break;
        }
        default:
            break;
    }

    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    IMAGE_SetDTA(_gstEmListAdvancedBatteryPara.handle[1][4], p_st_tmp_switch_img_para->p_img_buf,
                 p_st_tmp_switch_img_para->img_size);
    return 0;
}


static int32_t _SetBatteryWakeTime(int32_t value)
{
    if(!_gstUiCtx.waken_time)
    {
        IMAGE_SetDTA(_gstEmListAdvancedBatteryPara.handle[1][4], _garrSwtichImgPara[0].p_img_buf,
                     _garrSwtichImgPara[0].img_size);
    }
    else
    {
        IMAGE_SetDTA(_gstEmListAdvancedBatteryPara.handle[1][4], _garrSwtichImgPara[1].p_img_buf,
                     _garrSwtichImgPara[1].img_size);

        _gstEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.digit_value = value;
        sprintf(_gstEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.p_content, "%dMin",
                (uint16_t)_gstEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.digit_value);

        TEXT_SetText(_gstEmListAdvancedBatteryPara.handle[2][4], (const char *)_gstEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.p_content);
    }
    return 0;
}


/**
  * @brief ListAdvancedBatteryWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbAdvancedBatteryWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    static int32_t switch_type = 0;
    static int32_t row = 0;
#if TEST_DYNAMIC
    static WM_HTIMER wm_timer_handle_switch;
#endif
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
#if TEST_DYNAMIC
            wm_timer_handle_switch = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                                    0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                                    1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                                    0);         /* 留待将来使用，应为0 */
#endif

            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_BATTERY_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListAdvancedBatteryPara, pMsg->hWin);
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

            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定1000mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListAdvancedBatteryPara, _gstEmListAdvancedBatteryPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListAdvancedBatteryPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListAdvancedBatteryPara.sub_region[i].x_pos,
                                    _gstEmListAdvancedBatteryPara.sub_region[i].y_pos,
                                    _gstEmListAdvancedBatteryPara.sub_region[i].x_pos + _gstEmListAdvancedBatteryPara.sub_region[i].x_size - 1,
                                    _gstEmListAdvancedBatteryPara.sub_region[i].y_pos + _gstEmListAdvancedBatteryPara.sub_region[i].y_size - 1,
                                    _gstEmListAdvancedBatteryPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListAdvancedBatteryPara, _gstEmListAdvancedBatteryPara.list_sel);
#if 0
            //目前暂未实现此功能
            //获取从子界面得到的电池休眠时间
            if(_gstUiCtx.select == 4)
            {
                _gstUiCtx.waken_time = GetWakeTimeValue();
                _SetBatteryWakeTime(_gstUiCtx.waken_time);
            }
#endif
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
    _gstUiCtx.key_init = 0;
    pPrevEmWin = &gstEmWinListAdvancedSettings;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListAdvancedBattery.win_handle = GUI_CreateDialogBox(_garrListAdvancedBatteryDialogCreate, GUI_COUNTOF(_garrListAdvancedBatteryDialogCreate), _cbAdvancedBatteryWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListAdvancedBattery.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvancedBattery.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListAdvancedBattery.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListAdvancedBatteryPara, gstEmWinListAdvancedBattery.win_handle);
    return gstEmWinListAdvancedBattery.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvancedBattery.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvancedBattery.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvancedBattery.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;
    EmKeyDefaultInfoSet();

    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

#if EM_REMOVE_FIVE_LINE_INFO   
    /*开关图标*/
    _garrSwtichImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_SWITCH_SIZE);
    _garrSwtichImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrSwtichImgPara[0].mem_img);
    _garrSwtichImgPara[0].img_size = EM_LIST_ICON_SWITCH_SIZE;
    SpiFlashReadData(EM_ICON_SET_CLOSE_ADDR, _garrSwtichImgPara[0].p_img_buf, _garrSwtichImgPara[0].img_size);

    _garrSwtichImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_SWITCH_SIZE);
    _garrSwtichImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrSwtichImgPara[1].mem_img);
    _garrSwtichImgPara[1].img_size = EM_LIST_ICON_SWITCH_SIZE;
    SpiFlashReadData(EM_ICON_SET_OPEN_ADDR, _garrSwtichImgPara[1].p_img_buf, _garrSwtichImgPara[1].img_size);
#endif
    /*右箭头图标*/
    _gListAdvancedBatteryRightMem = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_SIZE);
    _pListAdvancedBatteryRightBuf = GUI_ALLOC_h2p(_gListAdvancedBatteryRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListAdvancedBatteryRightBuf, EM_LIST_ICON_ARROW_SIZE);

    _gstEmListAdvancedBatteryPara.list_start_id = WIN_LIST_ADVANCED_BATTERY_ID + 0x30;
    _gstEmListAdvancedBatteryPara.list_num = 3;
    _gstEmListAdvancedBatteryPara.list_display_num = 5;
    if(_gstEmListAdvancedBatteryPara.list_num <= _gstEmListAdvancedBatteryPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListAdvancedBatteryPara.list_num + 1 - _gstEmListAdvancedBatteryPara.list_display_num;
    }

    if(_gstEmListAdvancedBatteryPara.list_sel > _gstEmListAdvancedBatteryPara.list_num - 1)
    {
        _gstEmListAdvancedBatteryPara.list_sel = 0;
    }
    if(_gstEmListAdvancedBatteryPara.list_page > max_page - 1)
    {
        _gstEmListAdvancedBatteryPara.list_page = 0;
    }
    _gstEmListAdvancedBatteryPara.list_bk_color = 0x00787878;

    /*List选中框*/
    /*5个子列表*/
    _gstEmListAdvancedBatteryPara.sub_region[0].x_pos = 20;
    _gstEmListAdvancedBatteryPara.sub_region[0].y_pos = 38;
    _gstEmListAdvancedBatteryPara.sub_region[0].x_size = 440;
    _gstEmListAdvancedBatteryPara.sub_region[0].y_size = 34;
    _gstEmListAdvancedBatteryPara.sub_region[1].x_pos = 20;
    _gstEmListAdvancedBatteryPara.sub_region[1].y_pos = 80;
    _gstEmListAdvancedBatteryPara.sub_region[1].x_size = 440;
    _gstEmListAdvancedBatteryPara.sub_region[1].y_size = 34;
    _gstEmListAdvancedBatteryPara.sub_region[2].x_pos = 20;
    _gstEmListAdvancedBatteryPara.sub_region[2].y_pos = 122;
    _gstEmListAdvancedBatteryPara.sub_region[2].x_size = 440;
    _gstEmListAdvancedBatteryPara.sub_region[2].y_size = 34;
#if EM_REMOVE_FIVE_LINE_INFO
    _gstEmListAdvancedBatteryPara.sub_region[3].x_pos = 20;
    _gstEmListAdvancedBatteryPara.sub_region[3].y_pos = 164;
    _gstEmListAdvancedBatteryPara.sub_region[3].x_size = 440;
    _gstEmListAdvancedBatteryPara.sub_region[3].y_size = 34;
    _gstEmListAdvancedBatteryPara.sub_region[4].x_pos = 20;
    _gstEmListAdvancedBatteryPara.sub_region[4].y_pos = 206;
    _gstEmListAdvancedBatteryPara.sub_region[4].x_size = 440;
    _gstEmListAdvancedBatteryPara.sub_region[4].y_size = 34;
    /*List控件*/
#endif
    //第一行左边
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][0].x_size = 210;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListAdvancedBatteryLeftTextBuf[0];
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvancedBatteryPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Depth of Discharge");
    //第一行右边
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][0].x_relative_pos = 415;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][0].x_size = 5;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][0].y_size = 10;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gListAdvancedBatteryRightMem;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _pListAdvancedBatteryRightBuf;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_SIZE;
    //第二行左边
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][1].x_size = 320;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListAdvancedBatteryLeftTextBuf[1];
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvancedBatteryPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Max Charge/Discharge Current");
    //第二行右边
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][1].x_relative_pos = 415;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][1].x_size = 5;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][1].y_size = 10;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListAdvancedBatteryRightMem;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListAdvancedBatteryRightBuf;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_SIZE;
    //第三行左边
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][2].x_size = 320;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListAdvancedBatteryLeftTextBuf[2];
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvancedBatteryPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Max Charge/Discharge Power");
    //第三行右边
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][2].x_relative_pos = 415;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][2].y_relative_pos = 12;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][2].x_size = 5;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][2].y_size = 10;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gListAdvancedBatteryRightMem;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _pListAdvancedBatteryRightBuf;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_SIZE;
#if EM_REMOVE_FIVE_LINE_INFO  
    //第四行左边
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][3].x_size = 290;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListAdvancedBatteryLeftTextBuf[3];
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvancedBatteryPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Max Charge/Discharge Voltage");
    //第四行右边
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][3].x_relative_pos = 415;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][3].y_relative_pos = 12;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][3].x_size = 5;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][3].y_size = 10;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gListAdvancedBatteryRightMem;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _pListAdvancedBatteryRightBuf;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_SIZE;
    
    //第五行左边
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][4].x_size = 210;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListAdvancedBatteryLeftTextBuf[4];
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvancedBatteryPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvancedBatteryPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Battery Waken");
    //第五行右边
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].x_relative_pos = 386;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].y_relative_pos = 9;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].x_size = 40;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].y_size = 31;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListAdvancedBatteryPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_ICON_SWITCH_SIZE;
    //第五行中间
    _gstEmListAdvancedBatteryPara.sub_widget_para[2][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvancedBatteryPara.sub_widget_para[2][4].x_relative_pos = 172;
    _gstEmListAdvancedBatteryPara.sub_widget_para[2][4].y_relative_pos = 10;
    _gstEmListAdvancedBatteryPara.sub_widget_para[2][4].x_size = 208;
    _gstEmListAdvancedBatteryPara.sub_widget_para[2][4].y_size = 17;
    _gstEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.p_content = _garrListAdvancedBatteryMidTextBuf;
    _gstEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvancedBatteryPara.sub_widget_para[2][4].sub_widget.text_para.p_content, " ");
#endif
    /*List选中框*/
    _gstEmListAdvancedBatteryPara.selected_unvalid_color = 0xFF000000;
    _gstEmListAdvancedBatteryPara.selected_valid_color = 0x00D6E400;
    _gstEmListAdvancedBatteryPara.selected_size = 3;
    _gstEmListAdvancedBatteryPara.selected_r = 10;
    _gstEmListAdvancedBatteryPara.selected_x_start_offset = -2;
    _gstEmListAdvancedBatteryPara.selected_y_start_offset = -2;
    _gstEmListAdvancedBatteryPara.selected_x_end_offset = 2;
    _gstEmListAdvancedBatteryPara.selected_y_end_offset = 2;
//    _gstEmListAdvancedBatteryPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListAdvancedBatteryPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

#if 0
    /*滚动条*/
    _gstEmListAdvancedBatteryPara.scrollbar_enable = true;
    _gstEmListAdvancedBatteryPara.scrollbar_color = 0x00D6E400;
    _gstEmListAdvancedBatteryPara.scrollbar_bk_color = 0x00565656;
    _gstEmListAdvancedBatteryPara.scrollbar_x_pos = 454;
    _gstEmListAdvancedBatteryPara.scrollbar_y_pos = 39;
    _gstEmListAdvancedBatteryPara.scrollbar_x_size = 6;
    _gstEmListAdvancedBatteryPara.scrollbar_y_size = 241;
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
#if EM_REMOVE_FIVE_LINE_INFO
    GUI_ALLOC_Free(_garrSwtichImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrSwtichImgPara[1].mem_img);
#endif
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
    if(gstEmWinListAdvancedBattery.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 100)    //30
    {
        timecount = GetSystick();
        if(_gstEmListAdvancedBatteryPara.list_sel + _gstEmListAdvancedBatteryPara.list_page >= _gstEmListAdvancedBatteryPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListAdvancedBatteryPara.list_sel + _gstEmListAdvancedBatteryPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListAdvancedBatteryPara, gstEmWinListAdvancedBattery.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListAdvancedBatteryPara, gstEmWinListAdvancedBattery.win_handle);
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
    int32_t event = 0;
    event = EmGetKeyEvent(p_msg);
    if (event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }

    if (event == EN_KEY_SRC_LEFT)
    {
        --_gstUiCtx.select;
        if (_gstUiCtx.select < 0)
        {
            ++_gstUiCtx.select;
            return;
        }

        EmListUpward(&_gstEmListAdvancedBatteryPara, gstEmWinListAdvancedBattery.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListAdvancedBatteryPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }

        EmListDownward(&_gstEmListAdvancedBatteryPara, gstEmWinListAdvancedBattery.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        emWin_t *pnext = NULL;
        switch(_gstUiCtx.select)
        {
            case 0:
                pnext = &gstEmWinListAdvanSettingBatDOD;
                break;
            case 1:
                pnext = &gstEmWinListAdvanSettingBatCurrent;
                break;
            case 2:
                pnext = &gstEmWinListAdvanSettingBatPower;
                break;
#if EM_REMOVE_FIVE_LINE_INFO
            case 3:
                pnext = &gstEmWinListAdvanSettingBatVoltage;
                break;
            
            case 4:
                pnext = &gstEmWinListAdvanSettingBatWakTime;
                break;
#endif           
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
            if(p_msg->press_down_time >= gstEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
            {
                pnext = &gstEmWinHomePage;
            }
            else
            {
                pnext = &gstEmWinListAdvancedSettings;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}


