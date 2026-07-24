/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_advanced_communication.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-22
  * @brief       高级设置->通信参数
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"

#define  TEST_DYNAMIC                            0

#define  EM_LIST_ICON_ARROW_RIGHT_SIZE                          (4096)
#define  EM_LIST_ICON_ARROW_DOWN_SIZE                           (4096)
#define  EM_LIST_ADVANCED_COMMUNICATION_ICON_BACK_SIZE          (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);
void SetCommunBuadRateArrowDirection(void);

emWin_t gstEmWinListAdvanceCommunication = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t baudrate_list_show;
    int32_t select;
    int32_t key_init;
    uint16_t buadidx;
} UiCtx_t;

static UiCtx_t _gstUiCtx;
/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄
static IMAGE_Handle  _gImgArrowHandle;//右、下箭头句柄

/*List控件*/
static emListPara_t  _gstEmListAdvanceCommunicationPara;
static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;
static emWidgetImgPara_t  _garrArrowImgPara[2];  //0：右  1：下

static char _garrTitleBuf[] = "Setting < More < Communication";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";

static char _garrListAdvanceCommunicationLeftTextBuf[2][20];
static char _garrListAdvanceCommunicationMidTextBuf[2][20];
static GUI_HMEM _gListAdvanceCommunicationRightMem;
static uint8_t *_pListAdvanceCommunicationRightBuf;

typedef struct _buad_rate_idx_t
{
    uint16_t idx;
    const char rate[16];
} BuadRate_t;

static BuadRate_t _gBaudRateList[] =
{
    {0, "9600bps"},
    {1, "19200bps"},
    {2, "38400bps"},
    {3, "57600bps"},
    {4, "115200bps"},
//  {5, "256000bps"},
//  {6, "460800bps"},
};


#define WIN_LIST_ADVANCED_COMMUNICATION_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_ADVANCED_COMMUNICATION_TITLE_ID                         (GUI_ID_USER + 0x11)
#define WIN_LIST_ADVANCED_COMMUNICATION_PRESS_ID                         (GUI_ID_USER + 0x12)
#define WIN_LIST_ADVANCED_COMMUNICATION_BACK_IMG_ID                      (GUI_ID_USER + 0x13)
#define WIN_LIST_ADVANCED_COMMUNICATION_BACK_ID                          (GUI_ID_USER + 0x14)
#define WIN_LIST_ADVANCED_COMMUNICATION_ARROW_IMG_ID                     (GUI_ID_USER + 0x15)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListAdvancedCommunicationDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListAdvancedCommunicationWin",    WIN_LIST_ADVANCED_COMMUNICATION_ID,             0,      0,     480,    320,    0,   0x0,   0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_ADVANCED_COMMUNICATION_TITLE_ID,                       31,     20,     400,    14,     0,   0x0,   0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_ADVANCED_COMMUNICATION_PRESS_ID,                       133,    273,    33,     14,     0,   0x0,   0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_ADVANCED_COMMUNICATION_BACK_IMG_ID,                    171 - 5,  275,    15,     10,     0,   0x0,   0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_ADVANCED_COMMUNICATION_BACK_ID,                        188,    273,    150,    14,     0,   0x0,   0 },
    { IMAGE_CreateIndirect,   "arrow",          WIN_LIST_ADVANCED_COMMUNICATION_ARROW_IMG_ID,                   437,    91,     15,     15,     0,   0x0,   0 },
};


static int32_t _Set485BaudRate(int32_t ui_update, int32_t baudradeidx)
{
    snprintf(_garrListAdvanceCommunicationMidTextBuf[0], sizeof(_garrListAdvanceCommunicationMidTextBuf[0]), "%s", _gBaudRateList[baudradeidx].rate);
    if(ui_update)
    {
        TEXT_SetText(_gstEmListAdvanceCommunicationPara.handle[2][1], (const char *)_gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].sub_widget.text_para.p_content);
    }
    return 0;
}


/**
  * @brief ListAdvancedCommunicationWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbAdvancedCommunicationWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_COMMUNICATION_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_COMMUNICATION_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_COMMUNICATION_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_COMMUNICATION_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);
            /*初始化为右箭头*/
            _gImgArrowHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_COMMUNICATION_ARROW_IMG_ID);
            IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);
            /*LIST*/
            EmListWidgetCreate(&_gstEmListAdvanceCommunicationPara, pMsg->hWin);
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
            _AdvancedSysSetSwitchStat(0, (SWITCH_STAT_E)(switch_type % 2));

            WM_RestartTimer(pMsg->Data.v, 300);/*暂定300mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListAdvanceCommunicationPara, _gstEmListAdvanceCommunicationPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListAdvanceCommunicationPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListAdvanceCommunicationPara.sub_region[i].x_pos,
                                    _gstEmListAdvanceCommunicationPara.sub_region[i].y_pos,
                                    _gstEmListAdvanceCommunicationPara.sub_region[i].x_pos + _gstEmListAdvanceCommunicationPara.sub_region[i].x_size - 1,
                                    _gstEmListAdvanceCommunicationPara.sub_region[i].y_pos + _gstEmListAdvanceCommunicationPara.sub_region[i].y_size - 1,
                                    _gstEmListAdvanceCommunicationPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListAdvanceCommunicationPara, _gstEmListAdvanceCommunicationPara.list_sel);
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
    gstEmWinListAdvanceCommunication.win_handle = GUI_CreateDialogBox(_garrListAdvancedCommunicationDialogCreate, GUI_COUNTOF(_garrListAdvancedCommunicationDialogCreate), _cbAdvancedCommunicationWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListAdvanceCommunication.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvanceCommunication.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListAdvanceCommunication.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListAdvanceCommunicationPara, gstEmWinListAdvanceCommunication.win_handle);
    return gstEmWinListAdvanceCommunication.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvanceCommunication.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvanceCommunication.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvanceCommunication.win_handle = 0;
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

    _gListAdvanceCommunicationRightMem = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_RIGHT_SIZE);
    _pListAdvanceCommunicationRightBuf = GUI_ALLOC_h2p(_gListAdvanceCommunicationRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListAdvanceCommunicationRightBuf, EM_LIST_ICON_ARROW_RIGHT_SIZE);

    /*右箭头*/
    _garrArrowImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_RIGHT_SIZE);
    _garrArrowImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrArrowImgPara[0].mem_img);
    _garrArrowImgPara[0].img_size = EM_LIST_ICON_ARROW_RIGHT_SIZE;
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);
    /*下箭头*/
    _garrArrowImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_DOWN_SIZE);
    _garrArrowImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrArrowImgPara[1].mem_img);
    _garrArrowImgPara[1].img_size = EM_LIST_ICON_ARROW_DOWN_SIZE;
    SpiFlashReadData(EM_ICON_DOWN_ARROW_ADDR, _garrArrowImgPara[1].p_img_buf, _garrArrowImgPara[1].img_size);

    _gstPressTextPara.p_content = _garrPressBuf;
    _gstPressTextPara.text_color = 0x00EEEEEE;
    _gstPressTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstBackTextPara.p_content = _garrBackBuf;
    _gstBackTextPara.text_color = 0x00EEEEEE;
    _gstBackTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_ADVANCED_COMMUNICATION_ICON_BACK_SIZE);
    _gstReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gstReturnImgPara.mem_img);
    _gstReturnImgPara.img_size = EM_LIST_ADVANCED_COMMUNICATION_ICON_BACK_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

    _gstEmListAdvanceCommunicationPara.list_start_id = WIN_LIST_ADVANCED_COMMUNICATION_ID + 0x30;
    _gstEmListAdvanceCommunicationPara.list_num = 2;
    _gstEmListAdvanceCommunicationPara.list_display_num = 2;
    if(_gstEmListAdvanceCommunicationPara.list_num <= _gstEmListAdvanceCommunicationPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListAdvanceCommunicationPara.list_num + 1 - _gstEmListAdvanceCommunicationPara.list_display_num;
    }

    if(_gstEmListAdvanceCommunicationPara.list_sel > _gstEmListAdvanceCommunicationPara.list_num - 1)
    {
        _gstEmListAdvanceCommunicationPara.list_sel = 0;
    }
    if(_gstEmListAdvanceCommunicationPara.list_page > max_page - 1)
    {
        _gstEmListAdvanceCommunicationPara.list_page = 0;
    }
    _gstEmListAdvanceCommunicationPara.list_bk_color = 0x00787878;

    /*List选中框*/
    /*2个子列表*/
    _gstEmListAdvanceCommunicationPara.sub_region[0].x_pos = 20;
    _gstEmListAdvanceCommunicationPara.sub_region[0].y_pos = 38;
    _gstEmListAdvanceCommunicationPara.sub_region[0].x_size = 440;
    _gstEmListAdvanceCommunicationPara.sub_region[0].y_size = 34;
    _gstEmListAdvanceCommunicationPara.sub_region[1].x_pos = 20;
    _gstEmListAdvanceCommunicationPara.sub_region[1].y_pos = 80;
    _gstEmListAdvanceCommunicationPara.sub_region[1].x_size = 440;
    _gstEmListAdvanceCommunicationPara.sub_region[1].y_size = 34;

    /*List控件*/
    //第一行左边
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][0].x_size = 158;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListAdvanceCommunicationLeftTextBuf[0];
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceCommunicationPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "485 Address");
    //第一行右边
    _gstEmListAdvanceCommunicationPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[1][0].x_relative_pos = 417;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[1][0].x_size = 5;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[1][0].y_size = 10;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gListAdvanceCommunicationRightMem;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _pListAdvanceCommunicationRightBuf;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_ICON_ARROW_RIGHT_SIZE;
    //第二行左边
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][1].x_size = 200;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListAdvanceCommunicationLeftTextBuf[1];
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListAdvanceCommunicationPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "485 Baud Rate");
    //第二行中间文字
    _gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].x_relative_pos = 308;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].y_relative_pos = 8;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].x_size = 100;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].y_size = 17;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].sub_widget.text_para.p_content = _garrListAdvanceCommunicationMidTextBuf[0];
    _gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    strcpy(_gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].sub_widget.text_para.p_content, "9600bps");
    snprintf(_garrListAdvanceCommunicationMidTextBuf[0], sizeof(_garrListAdvanceCommunicationMidTextBuf[0]), "%s", _gBaudRateList[_gstUiCtx.buadidx].rate);

    /*List选中框*/
    _gstEmListAdvanceCommunicationPara.selected_unvalid_color = 0xFF000000;
    _gstEmListAdvanceCommunicationPara.selected_valid_color = 0x00D6E400;
    _gstEmListAdvanceCommunicationPara.selected_size = 3;
    _gstEmListAdvanceCommunicationPara.selected_r = 10;
    _gstEmListAdvanceCommunicationPara.selected_x_start_offset = -2;
    _gstEmListAdvanceCommunicationPara.selected_y_start_offset = -2;
    _gstEmListAdvanceCommunicationPara.selected_x_end_offset = 2;
    _gstEmListAdvanceCommunicationPara.selected_y_end_offset = 2;
//    _gstEmListAdvanceCommunicationPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListAdvanceCommunicationPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    _gstUiCtx.buadidx = gstCollectInfo.maincard_data.base_info.baudrate;
    _Set485BaudRate(0, _gstUiCtx.buadidx);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gListAdvanceCommunicationRightMem);
    GUI_ALLOC_Free(_garrArrowImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrArrowImgPara[1].mem_img);
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
    if(_gstUiCtx.buadidx != gstCollectInfo.maincard_data.base_info.baudrate)
    {
        _gstUiCtx.buadidx = gstCollectInfo.maincard_data.base_info.baudrate;
        _Set485BaudRate(1, _gstUiCtx.buadidx);
    }

    if(gstEmWinListAdvanceCommunication.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 100)    //30
    {
        timecount = GetSystick();
        if(_gstEmListAdvanceCommunicationPara.list_sel + _gstEmListAdvanceCommunicationPara.list_page >= _gstEmListAdvanceCommunicationPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListAdvanceCommunicationPara.list_sel + _gstEmListAdvanceCommunicationPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListAdvanceCommunicationPara, gstEmWinListAdvanceCommunication.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListAdvanceCommunicationPara, gstEmWinListAdvanceCommunication.win_handle);
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
    if(_gstUiCtx.baudrate_list_show)
    {
        gstEmWinListAdvanSettingCommunicateBaudRate.key_handler(p_msg);

        if(!_gstUiCtx.baudrate_list_show)
        {
            gstEmWinListAdvanSettingCommunicateBaudRate.delete_win();
        }
        return;
    }

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
        EmListUpward(&_gstEmListAdvanceCommunicationPara, gstEmWinListAdvanceCommunication.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if(_gstUiCtx.select >= _gstEmListAdvanceCommunicationPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListAdvanceCommunicationPara, gstEmWinListAdvancedSettings.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        emWin_t *pnext = NULL;
        switch(_gstUiCtx.select)
        {
            case 0:
            {
                pnext = &gstEmWinListAdvanSettingCommunicateAddr;
            }
            break;
            case 1:
            {
                if(!_gstUiCtx.baudrate_list_show)
                {
                    gstEmWinListAdvanSettingCommunicateBaudRate.create_win();
                    _gstUiCtx.baudrate_list_show = 1;
                }
                SetCommunBuadRateArrowDirection();
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
#if 0
        uint8_t data_buff[256 + 8] = {0};
        uint16_t tmp = 0;

        tmp = BigLittleSwap16(_gstUiCtx.buadidx);
        ModbusWriteSingleRemain(data_buff, 0x01, COLLECT_BAUD_ADDR, tmp);
#endif
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
    }
}


void SetCommunBuadRateArrowDirection(void)
{
    if(_gstUiCtx.baudrate_list_show)
    {
        //箭头向下
        IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[1].p_img_buf, _garrArrowImgPara[1].img_size);
    }
    else
    {
        //箭头向右
        IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);
    }
}

void SetCommuBaudRateListShow(int32_t show)
{
    _gstUiCtx.baudrate_list_show = show;
}

int32_t SetCurCommuBaudRate(int32_t index)
{
    _gstUiCtx.buadidx = index;
    snprintf(_gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].sub_widget.text_para.p_content,
             sizeof(_garrListAdvanceCommunicationMidTextBuf[0]), "%s", _gBaudRateList[index].rate);
    TEXT_SetText(_gstEmListAdvanceCommunicationPara.handle[2][1], (const char *)_gstEmListAdvanceCommunicationPara.sub_widget_para[2][1].sub_widget.text_para.p_content);
    return 0;
}

