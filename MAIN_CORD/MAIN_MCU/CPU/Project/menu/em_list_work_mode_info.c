/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_work_mode_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-01-15
  * @brief       Settings界面
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-15    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define  TEST_DYNAMIC                                         0
#define  EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE                (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListWorkingModeInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _work_mode_button_stru_t
{
    uint32_t tmp_switch[3];
    uint32_t key_stat[3];
    uint32_t key_time[3];
    uint32_t tmp_mode;
    uint32_t select;
} WorkModeButton_t;

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t work_mode;
    uint32_t arr_switcher[3];
    WorkModeButton_t mode_button;
} UiCtx_t;

typedef enum
{
    EN_SWITCH_STAT_OFF = 0,
    EN_SWITCH_STAT_ON,
} EN_SWITCH_STAT_E;

static UiCtx_t _gstUiCtx;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;
static IMAGE_Handle  _gImgSwithHandle;//开关图标句柄

static char _garrTitleBuf[] = "Settings < Working Mode";

/*List控件*/
static emListPara_t  _gstEmListWorkingModeInfoPara;

static char _garrListWorkingModeInfoLeftTextBuf[16][20];

/*开关图片参数*/
static emWidgetImgPara_t  _garrSwtichImgPara[2];  /*0:关   1:开*/


#define WIN_LIST_WORKMODE_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_WORKMODE_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)
#define WIN_LIST_SWITCH_ON_IMG_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_SWITCH_OFF_IMG_ID                              (GUI_ID_USER + 0x13)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListWorkingModeInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListWorkingModeInfoWin",    WIN_LIST_WORKMODE_INFO_ID,           0,   0,  480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",              WIN_LIST_WORKMODE_INFO_TITLE_ID,            31,  20, 400,  14,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "SWITON",             WIN_LIST_SWITCH_ON_IMG_ID,                  400, 38,  40,  40,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "SWITCHOFF",          WIN_LIST_SWITCH_OFF_IMG_ID,                 400, 38,  40,  40,   0,   0x0,  0 }
};

static int32_t _SetSwitchStat(uint8_t row, EN_SWITCH_STAT_E e_stat)
{

    emWidgetImgPara_t  *p_st_tmp_switch_img_para = &_garrSwtichImgPara[0];
    switch (e_stat)
    {
        case EN_SWITCH_STAT_OFF:
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

    _gstEmListWorkingModeInfoPara.sub_widget_para[1][row].sub_widget.img_para.mem_img = p_st_tmp_switch_img_para->mem_img;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][row].sub_widget.img_para.p_img_buf = p_st_tmp_switch_img_para->p_img_buf;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][row].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
    IMAGE_SetDTA(_gstEmListWorkingModeInfoPara.handle[1][row], p_st_tmp_switch_img_para->p_img_buf,
                 p_st_tmp_switch_img_para->img_size);
    return 0;
}

static int32_t _UpdateSwitch(int32_t select)
{
    if (select < 0 || select > sizeof(_gstUiCtx.arr_switcher) / sizeof(_gstUiCtx.arr_switcher[0]))
    {
        return -1;
    }

//    _gstUiCtx.select = select;
    for (int32_t i = 0; i < sizeof(_gstUiCtx.arr_switcher) / sizeof(_gstUiCtx.arr_switcher[0]); ++i)
    {
        _gstUiCtx.arr_switcher[i] = 0;
        _gstUiCtx.mode_button.tmp_switch[i] = 0;
        _gstUiCtx.mode_button.key_stat[i] = 0;
//        if (_gstUiCtx.select == i && i != 3)
        if (select == i && i != 3)
        {
            _gstUiCtx.arr_switcher[i] = (++_gstUiCtx.arr_switcher[i]) % 2;
            _gstUiCtx.mode_button.tmp_switch[i] = _gstUiCtx.arr_switcher[i];
            _gstUiCtx.mode_button.key_stat[i] = 1;
            _gstUiCtx.mode_button.key_time[i] = tim_get_systime();
            _gstUiCtx.mode_button.tmp_mode = i;
        }
        _SetSwitchStat((uint8_t)i, (EN_SWITCH_STAT_E)_gstUiCtx.arr_switcher[i]);
    }

    return 0;
}


/**
  * @brief ListWorkingModeInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListWorkingModeInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_WORKMODE_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListWorkingModeInfoPara, pMsg->hWin);
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
            ++row;
            _SetSwitchStat(row % 3, (SWITCH_STAT_E)(switch_type % 2));

            WM_RestartTimer(pMsg->Data.v, 300);/*暂定300mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListWorkingModeInfoPara, _gstEmListWorkingModeInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(i = 0; i < _gstEmListWorkingModeInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListWorkingModeInfoPara.sub_region[i].x_pos,
                                    _gstEmListWorkingModeInfoPara.sub_region[i].y_pos,
                                    _gstEmListWorkingModeInfoPara.sub_region[i].x_pos + _gstEmListWorkingModeInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListWorkingModeInfoPara.sub_region[i].y_pos + _gstEmListWorkingModeInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListWorkingModeInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListWorkingModeInfoPara, _gstEmListWorkingModeInfoPara.list_sel);

            for (i = 0; i < sizeof(_gstUiCtx.arr_switcher) / sizeof(_gstUiCtx.arr_switcher[0]); ++i)
            {
                _SetSwitchStat((uint8_t)i, (EN_SWITCH_STAT_E)_gstUiCtx.arr_switcher[i]);
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
    _gstUiCtx.key_init = 0;
    pPrevEmWin = &gstEmWinListSettingsInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListWorkingModeInfo.win_handle = GUI_CreateDialogBox(_garrListWorkingModeInfoDialogCreate, GUI_COUNTOF(_garrListWorkingModeInfoDialogCreate), _cbListWorkingModeInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListWorkingModeInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListWorkingModeInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListWorkingModeInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListWorkingModeInfoPara, gstEmWinListWorkingModeInfo.win_handle);
    return gstEmWinListWorkingModeInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListWorkingModeInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListWorkingModeInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListWorkingModeInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;
    int32_t i = 0;
    int32_t j = sizeof(_gstUiCtx.arr_switcher) / sizeof(_gstUiCtx.arr_switcher[0]);
    EmKeyDefaultInfoSet();
    //每次设置前将图标清零
    memset(_gstUiCtx.arr_switcher, 0, sizeof(_gstUiCtx.arr_switcher));

    _gstUiCtx.work_mode = _gstUiCtx.mode_button.tmp_mode;
//    _gstUiCtx.work_mode = gstCollectParaData.pcs_hold_data.work_mode;
//  _gstUiCtx.work_mode = _gstUiCtx.select;

    _gstUiCtx.arr_switcher[_gstUiCtx.work_mode] = 1;



    for (i = 0; i < j; ++i)
    {
        if (_gstUiCtx.arr_switcher[i])
        {
            break;
        }
    }
    if (i == j)
    {
        _gstUiCtx.arr_switcher[0] = 1;
    }

    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _garrSwtichImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE);
    _garrSwtichImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrSwtichImgPara[0].mem_img);
    _garrSwtichImgPara[0].img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_SET_CLOSE_ADDR, _garrSwtichImgPara[0].p_img_buf, _garrSwtichImgPara[0].img_size);

    _garrSwtichImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE);
    _garrSwtichImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrSwtichImgPara[1].mem_img);
    _garrSwtichImgPara[1].img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_SET_OPEN_ADDR, _garrSwtichImgPara[1].p_img_buf, _garrSwtichImgPara[1].img_size);


    _gstEmListWorkingModeInfoPara.list_start_id = WIN_LIST_WORKMODE_INFO_ID + 0x30;
    _gstEmListWorkingModeInfoPara.list_num = 3;
    _gstEmListWorkingModeInfoPara.list_display_num = 6;
    if(_gstEmListWorkingModeInfoPara.list_num <= _gstEmListWorkingModeInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListWorkingModeInfoPara.list_num + 1 - _gstEmListWorkingModeInfoPara.list_display_num;
    }

    if(_gstEmListWorkingModeInfoPara.list_sel > _gstEmListWorkingModeInfoPara.list_num - 1)
    {
        _gstEmListWorkingModeInfoPara.list_sel = 0;
    }
    if(_gstEmListWorkingModeInfoPara.list_page > max_page - 1)
    {
        _gstEmListWorkingModeInfoPara.list_page = 0;
    }
    _gstEmListWorkingModeInfoPara.list_bk_color = 0x00787878;
    /*2个子列表*/
    _gstEmListWorkingModeInfoPara.sub_region[0].x_pos = 20;
    _gstEmListWorkingModeInfoPara.sub_region[0].y_pos = 38;
    _gstEmListWorkingModeInfoPara.sub_region[0].x_size = 440;
    _gstEmListWorkingModeInfoPara.sub_region[0].y_size = 34;
    _gstEmListWorkingModeInfoPara.sub_region[1].x_pos = 20;
    _gstEmListWorkingModeInfoPara.sub_region[1].y_pos = 80;
    _gstEmListWorkingModeInfoPara.sub_region[1].x_size = 440;
    _gstEmListWorkingModeInfoPara.sub_region[1].y_size = 34;
    _gstEmListWorkingModeInfoPara.sub_region[2].x_pos = 20;
    _gstEmListWorkingModeInfoPara.sub_region[2].y_pos = 122;
    _gstEmListWorkingModeInfoPara.sub_region[2].x_size = 440;
    _gstEmListWorkingModeInfoPara.sub_region[2].y_size = 34;

    /*List控件*/
    //第一行左边
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][0].x_size = 200;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListWorkingModeInfoLeftTextBuf[0];
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListWorkingModeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Self Consumption");
    //第一行右边
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][0].x_relative_pos = 380;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][0].y_relative_pos = 9;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][0].x_size = 40;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][0].y_size = 31;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
    //第二行左边
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][1].x_size = 200;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListWorkingModeInfoLeftTextBuf[1];
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListWorkingModeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Battery Priority");
    //第二行右边
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][1].x_relative_pos = 380;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][1].y_relative_pos = 9;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][1].x_size = 40;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][1].y_size = 31;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _garrSwtichImgPara[1].mem_img;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[1].p_img_buf;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;
    //第三行左边
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][2].x_size = 200;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListWorkingModeInfoLeftTextBuf[2];
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListWorkingModeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Peak Shifting");
    //第三行右边
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][2].x_relative_pos = 380;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][2].y_relative_pos = 9;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][2].x_size = 40;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][2].y_size = 31;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _garrSwtichImgPara[0].mem_img;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _garrSwtichImgPara[0].p_img_buf;
    _gstEmListWorkingModeInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_WORKMODE_INFO_ICON_ARROW_SIZE;


    /*List选中框*/
    _gstEmListWorkingModeInfoPara.selected_unvalid_color = 0xFF000000;
    _gstEmListWorkingModeInfoPara.selected_valid_color = 0x00D6E400;
    _gstEmListWorkingModeInfoPara.selected_size = 3;
    _gstEmListWorkingModeInfoPara.selected_r = 10;
    _gstEmListWorkingModeInfoPara.selected_x_start_offset = -2;
    _gstEmListWorkingModeInfoPara.selected_y_start_offset = -2;
    _gstEmListWorkingModeInfoPara.selected_x_end_offset = 2;
    _gstEmListWorkingModeInfoPara.selected_y_end_offset = 2;
//    _gstEmListWorkingModeInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListWorkingModeInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    /*滚动条*/
//    _gstEmListWorkingModeInfoPara.scrollbar_enable = true;
//    _gstEmListWorkingModeInfoPara.scrollbar_color = 0x00D6E400;
//    _gstEmListWorkingModeInfoPara.scrollbar_bk_color = 0x00565656;
//    _gstEmListWorkingModeInfoPara.scrollbar_x_pos = 454;
//    _gstEmListWorkingModeInfoPara.scrollbar_y_pos = 39;
//    _gstEmListWorkingModeInfoPara.scrollbar_x_size = 6;
//    _gstEmListWorkingModeInfoPara.scrollbar_y_size = 241;

}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_garrSwtichImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrSwtichImgPara[1].mem_img);
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
    if(gstEmWinListWorkingModeInfo.win_handle == 0)
    {
        return;
    }
    
    if(_gstUiCtx.mode_button.key_stat[_gstUiCtx.mode_button.select] == 1)
    {
//        if(_gstUiCtx.arr_switcher[_gstUiCtx.mode_button.select] != _gstUiCtx.mode_button.tmp_switch[_gstUiCtx.mode_button.select])
        if(_gstUiCtx.work_mode != _gstUiCtx.mode_button.tmp_mode)
        {
            _gstUiCtx.work_mode = _gstUiCtx.mode_button.tmp_mode;
            _UpdateSwitch(_gstUiCtx.work_mode);
        }
        if(tim_get_systime() - _gstUiCtx.mode_button.key_time[_gstUiCtx.mode_button.select] > 5000)
        {
            _gstUiCtx.mode_button.key_stat[_gstUiCtx.mode_button.select] = 0;
            _gstUiCtx.mode_button.key_time[_gstUiCtx.mode_button.select] = tim_get_systime();
        }
    }
    else
    {
        if(_gstUiCtx.work_mode != gstCollectInfo.power_data.power_set_info.work_mode)
        {
            _gstUiCtx.work_mode = gstCollectInfo.power_data.power_set_info.work_mode;
            _UpdateSwitch(_gstUiCtx.work_mode);
        }
    }
    

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gstEmListWorkingModeInfoPara.list_sel + _gstEmListWorkingModeInfoPara.list_page >= _gstEmListWorkingModeInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListWorkingModeInfoPara.list_sel + _gstEmListWorkingModeInfoPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListWorkingModeInfoPara, gstEmWinListWorkingModeInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListWorkingModeInfoPara, gstEmWinListWorkingModeInfo.win_handle);
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
        EmListUpward(&_gstEmListWorkingModeInfoPara, gstEmWinListWorkingModeInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListWorkingModeInfoPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListWorkingModeInfoPara, gstEmWinListWorkingModeInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        _gstUiCtx.mode_button.select = _gstUiCtx.select;
        _UpdateSwitch(_gstUiCtx.select);

        if(_gstUiCtx.select == 0 || _gstUiCtx.select == 1)
        {
            uint8_t data_buff[256 + 8] = {0};
            uint16_t tmp = 0;
            tmp = BigLittleSwap16((uint16_t)_gstUiCtx.select);
            ModbusWriteSingleRemain(data_buff, 0x01, COLLECT_POWER_WORK_MODE_SECTION_ADDR, tmp);
        }

        if (_gstUiCtx.select == 2)
        {
            // 肖峰填谷
            if(pCurrentEmWin->win_handle != 0)
            {
                pPrevEmWin = pCurrentEmWin;
                pCurrentEmWin->delete_win();
                pCurrentEmWin = &gstEmWinListSavedSetTimeInfo;
                pCurrentEmWin->create_win();
            }
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
                pnext = &gstEmWinListSettingsInfo;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }

}

int32_t SetPeakLoadBalacne(uint32_t on)
{
    int32_t i = 0;
    for (i = 0; i < sizeof(_gstUiCtx.arr_switcher) / sizeof(_gstUiCtx.arr_switcher[0]); ++i)
    {
        _gstUiCtx.arr_switcher[i] = 0;
    }
    _gstUiCtx.arr_switcher[2] = on;
    return 0;
}

