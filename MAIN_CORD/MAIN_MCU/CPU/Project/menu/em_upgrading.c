/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_upgrading.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-12-12
  * @brief       升级进度界面
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-12-12    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"
#include "firmware_upgrate.h"

#define DEBUG_TEST                              0
#define	USING_TIMER								1

#define EM_UPGRADING_ICON_BACKGROUND_SIZE        (452 * 1024)

#define EN_UPGRADING_PROGRESS_BAR_XPOS           (139)
#define EN_UPGRADING_PROGRESS_BAR_YPOS           (200)
#define EN_UPGRADING_PROGRESS_BAR_LENGTH         (198)
#define EN_UPGRADING_PROGRESS_BAR_WIDTH          (5)

#define GUI_COLOR_PROGRESS_BAR_1            GUI_MAKE_COLOR(0x00434802)
#define GUI_COLOR_PROGRESS_BAR_2            GUI_MAKE_COLOR(0x00D6E400)

#define WIN_UPGRADING_TIPS_TEXT_ID          (GUI_ID_USER + 0x10)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

typedef struct _progress_rate_stru_t
{
    uint16_t cur_type;  // 当前进度类型
    uint8_t get_rate; // 下载进度
    uint8_t set_rate; // 分发进度
} ProgressRate_t;

typedef struct _ui_ctx_stru_t
{
    int8_t init; // 是否已经初始化(防止多实例)
    int8_t exit; // 是否退出升级
    uint32_t time; // 总耗时
    ProgressRate_t rate; // 总进度
} UiCtx_t;

emWin_t gstEmWinUpgrading = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

/* 存储设备相关变量 */
static GUI_HMEM _gMemBK;
static uint8_t *_pBKBuf;

static UiCtx_t _gstUiCtx;
static uint8_t  _gProgressBarRate = 0;
static TEXT_Handle  _gTextUpgradingHandle;// 升级状态提示句柄
static emWidgetTextPara_t  _gstUpgradingTextPara; // 升级状态提示Text属性
static char _garrUpgradingTextBuf[64] = {0}; // 升级状态提示



static int32_t SetProgressBarRate(int32_t ui_update)
{
#if DEBUG_TEST

#else
    uint32_t updating_status = 0;
    uint16_t upgrading_type = 0;
    updating_status = GetCurUpdataStatus();
    upgrading_type = GetCurUpdataType();

    _gstUiCtx.rate.get_rate = gstFirmwareUpgrateInfo.updata_get_persent;
    _gstUiCtx.rate.set_rate = gstFirmwareUpgrateInfo.updata_send_persent;
#endif /* DEBUG_TEST */
    if (_gstUiCtx.exit == 0)
    {
#if DEBUG_TEST // debug 
        if (_gstUiCtx.rate.set_rate == 0)
        {
            ++_gstUiCtx.rate.get_rate;
        }
        if (_gstUiCtx.rate.get_rate > 98)
        {
            ++_gstUiCtx.rate.set_rate;
        }
#endif
        if (_gstUiCtx.rate.set_rate > 99)
        {
            _gstUiCtx.rate.set_rate = 0;
            _gstUiCtx.rate.get_rate = 0;
            _gstUiCtx.exit = 1;
        }
    }

    _gstUiCtx.rate.cur_type = 0;
    if (_gstUiCtx.rate.get_rate > 98)
    {
        _gstUiCtx.rate.cur_type = 1;
    }

    // 设置显示文字
    if (_gstUiCtx.rate.cur_type == 0)
    {
        snprintf(_garrUpgradingTextBuf, sizeof(_garrUpgradingTextBuf),
                 "%s, %d%%", "download from collector",
                 _gstUiCtx.rate.get_rate);
        _gProgressBarRate = _gstUiCtx.rate.get_rate;
    }
    else
    {
        char updating_type_name[64] = {"unknown"};
#if DEBUG_TEST

#else
        snprintf(updating_type_name, sizeof(updating_type_name), "updating_status :%d, type : %d", updating_status, upgrading_type);
        switch (updating_status)
        {
            case EN_FIRMWARE_UPGRATE_STATUS_IDLE:
            {
                snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                         " idle...");
            }
            break;
            case EN_FIRMWARE_UPGRATE_STATUS_CHECK:
            {
                snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                         " check ...");
            }
            break;
            case EN_FIRMWARE_UPGRATE_STATUS_DEZIP:
            {
                snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                         " decompress ...");
            }
            break;
            case EN_FIRMWARE_UPGRATE_STATUS_TO_DO:
            {
                snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                         " to do ... ");
            }
            break;
            case EN_FIRMWARE_UPGRATE_STATUS_DEZIP_IMG_FONT:
            {
                snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                         " decompress monitor resource");
            }
            break;
            case EN_FIRMWARE_UPGRATE_STATUS_ERROR:
            {
                snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                         " error!");
            }
            break;
            case EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE:
            case EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_PREPARE_ACK:
            case EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_DATA:
            case EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_DATA_ACK:
            case EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_END:
            case EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_END_ACK:
            {
                switch (upgrading_type)
                {
                    case UPDATA_MAIN_TYPE:
                    {
                        snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                                 "monitor");
                    }
                    break;
                    case UPDATA_PCS_TYPE:
                    {
                        snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                                 "pcs");
                    }
                    break;
                    case UPDATA_ARC_TYPE:
                    {
                        snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                                 "arc");
                    }
                    break;
                    case UPDATA_BMS_TYPE:
                    {
                        snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                                 "bms");
                    }
                    break;
                    case UPDATA_FPGA_TYPE:
                    {
                        snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                                 "fpga");
                    }
                    break;
                    case UPDATA_MAIN_COP_TYPE:
                    {
                        snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                                 "monitor cop");
                    }
                    break;
                    case UPDATA_UI_TYPE:
                    {
                        snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                                 "monitor UI");
                    }
                    break;
                    case UPDATA_UI_TYPE1:
                    {
                        snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                                 "monitor Font");
                    }
                    break;
                    default:
                    {
                        snprintf(updating_type_name, sizeof(updating_type_name), "%s",
                                 "unknown");
                    }
                    break;
                }
            }
            break;
            default:
                break;
        }

#endif /* DEBUG_TEST */
        snprintf(_garrUpgradingTextBuf, sizeof(_garrUpgradingTextBuf),
                 "%s %s, %d%%", "sending to", updating_type_name,
                 _gstUiCtx.rate.set_rate);
        _gProgressBarRate = _gstUiCtx.rate.set_rate
                            + _gstUiCtx.rate.get_rate;
    }

    if (_gstUiCtx.exit)
    {
        snprintf(_garrUpgradingTextBuf, sizeof(_garrUpgradingTextBuf),
                 "%s", "upgrade complete!");
    }

    if (ui_update && _gTextUpgradingHandle)
    {
        TEXT_SetText(_gTextUpgradingHandle, (const char *)_gstUpgradingTextPara.p_content);
    }
    return 0;
}

/**
  * @brief FuncListWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbUpgradingWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;

    switch (pMsg->MsgId)
    {
        case WM_CREATE:
        {
            ++_gstUiCtx.time;
            _gProgressBarRate = 0;
            _gTextUpgradingHandle = TEXT_Create(
                                        10, EN_UPGRADING_PROGRESS_BAR_YPOS + 50, EM_XSIZE - 20, 100,
                                        WIN_UPGRADING_TIPS_TEXT_ID,
                                        WM_CF_SHOW,
                                        "",
                                        TEXT_CF_HCENTER);
            TEXT_SetFont(_gTextUpgradingHandle, _gstUpgradingTextPara.p_text_font);
            TEXT_SetText(_gTextUpgradingHandle, (const char *)_gstUpgradingTextPara.p_content);
            TEXT_SetTextColor(_gTextUpgradingHandle, _gstUpgradingTextPara.text_color);
            break;
        }
        case WM_DELETE:
        {
            break;
        }
        case WM_TIMER:
        {
			WM_RestartTimer(pMsg->Data.v, 10);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
            if(_gProgressBarRate >= EN_UPGRADING_PROGRESS_BAR_LENGTH - 1)
            {
                _gProgressBarRate = EN_UPGRADING_PROGRESS_BAR_LENGTH - 1;
            }

            /*处理进度条，重新绘制*/

            hWinOld = WM_SelectWindow(pMsg->hWin);//先选择要在哪个窗口绘制
            GUI_SetColor(GUI_COLOR_PROGRESS_BAR_2);//上色
            GUI_SetPenSize(EN_UPGRADING_PROGRESS_BAR_WIDTH);
            GUI_DrawLine(EN_UPGRADING_PROGRESS_BAR_XPOS, EN_UPGRADING_PROGRESS_BAR_YPOS, EN_UPGRADING_PROGRESS_BAR_XPOS + _gProgressBarRate, EN_UPGRADING_PROGRESS_BAR_YPOS);

            WM_SelectWindow(hWinOld);//恢复绘制之前的窗口选择
            break;
        }
        case WM_PAINT:
        {
            GUI_Clear();
            GUI_DrawStreamedBitmapAuto(_pBKBuf, 0, 0);
            GUI_SetColor(GUI_COLOR_PROGRESS_BAR_1);//底色
            GUI_SetPenSize(EN_UPGRADING_PROGRESS_BAR_WIDTH);
            GUI_DrawLine(EN_UPGRADING_PROGRESS_BAR_XPOS, EN_UPGRADING_PROGRESS_BAR_YPOS,
                         EN_UPGRADING_PROGRESS_BAR_XPOS + EN_UPGRADING_PROGRESS_BAR_LENGTH - 1,
                         EN_UPGRADING_PROGRESS_BAR_YPOS);
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
    if (_gstUiCtx.init)
    {
        return NULL;
    }
    _Init();
    _gstUiCtx.init = 1;
    /*创建一个基于桌面窗口的子窗口*/
    gstEmWinUpgrading.win_handle = WM_CreateWindow(0, 0, EM_XSIZE, EM_YSIZE, WM_CF_SHOW, _cbUpgradingWin, 0);
    //pCurrentEmWin = &gstEmWinUpgrading;
    WINDOW_SetBkColor(gstEmWinUpgrading.win_handle, 0x006A7105);

#if USING_TIMER
            /* 创建定时器 */
	WM_CreateTimer(gstEmWinUpgrading.win_handle, /* 接受信息的窗口的句柄 */
							  0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
							  10,         /* 周期，此周期过后指定窗口应收到消息*/
							  0);         /* 留待将来使用，应为0 */
#endif 

    GUI_Exec();
//    _EndWin();
    return gstEmWinUpgrading.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if (_gstUiCtx.init)
    {
        _gstUiCtx.init = 0;
    }

    if(gstEmWinUpgrading.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinUpgrading.win_handle);
    _Del();
    GUI_Exec();
    //GUI_EnableAlpha(0);
    gstEmWinUpgrading.win_handle = 0;
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    /*为背景图申请内存*/
    /*
    _gMemBK = GUI_ALLOC_AllocZero(EM_UPGRADING_ICON_BACKGROUND_SIZE);
    _pBKBuf = GUI_ALLOC_h2p(_gMemBK);
    SpiFlashReadData(EM_ICON_START_ADDR, _pBKBuf, EM_UPGRADING_ICON_BACKGROUND_SIZE);
    */
    /*基本参数初始化*/
    _gProgressBarRate = 0;
    memset(&_gstUiCtx, 0, sizeof(_gstUiCtx));

    memset(_garrUpgradingTextBuf, 0, sizeof(_garrUpgradingTextBuf));
    _gstUpgradingTextPara.p_content = _garrUpgradingTextBuf;
    _gstUpgradingTextPara.text_color = 0X00B2BE00;
    _gstUpgradingTextPara.p_text_font = EM_TEXT_FONT_3;//EM_TEXT_FONT_7;
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    //GUI_ALLOC_Free(_gMemBK);
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
#if 0
    static uint32_t wait_time = 0;
    if(_gstUiCtx.exit == 1 && pCurrentEmWin->win_handle != 0)
    {
        if(tim_get_systime() - wait_time > 500)
        {
            _gstUiCtx.exit = 0;
            gstEmWinUpgrading.delete_win();
            pCurrentEmWin = &gstEmWinStartup;
            pCurrentEmWin->create_win();
            wait_time = tim_get_systime();
        }
    }
#else
    if(_gstUiCtx.exit > 1 && pCurrentEmWin->win_handle != 0)
    {
        ShowEmUpgrading(0);
        return;
    }
#endif
    
    
    SetProgressBarRate(1);

#if USING_TIMER

#else
	if (gstEmWinUpgrading.win_handle) 
	{
		if(_gProgressBarRate >= EN_UPGRADING_PROGRESS_BAR_LENGTH - 1)
		{
			_gProgressBarRate = EN_UPGRADING_PROGRESS_BAR_LENGTH - 1;
		}

		/*处理进度条，重新绘制*/

		WM_HWIN hWinOld;
		hWinOld = WM_SelectWindow(gstEmWinUpgrading.win_handle);//先选择要在哪个窗口绘制
		GUI_SetColor(GUI_COLOR_PROGRESS_BAR_2);//上色
		GUI_SetPenSize(EN_UPGRADING_PROGRESS_BAR_WIDTH);
		GUI_DrawLine(EN_UPGRADING_PROGRESS_BAR_XPOS, EN_UPGRADING_PROGRESS_BAR_YPOS, EN_UPGRADING_PROGRESS_BAR_XPOS + _gProgressBarRate, EN_UPGRADING_PROGRESS_BAR_YPOS);

		WM_SelectWindow(hWinOld);//恢复绘制之前的窗口选择
		GUI_Exec();
		//WM_Paint(gstEmWinUpgrading.win_handle);
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

    if (event == EN_KEY_SRC_ESC)
    {
        ShowEmUpgrading(0);
        return;
    }
}


int32_t ShowEmUpgrading(int32_t flag)
{
    static emWin_t *p_st_pre = NULL;
    if (flag)
    {
        if (!p_st_pre)
        {
            // create upgrading wind
            p_st_pre = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = &gstEmWinUpgrading;
            pCurrentEmWin->create_win();
        }

        pCurrentEmWin->polling();
    }
    else
    {
        if (p_st_pre)
        {
            p_st_pre->delete_win();
            p_st_pre = &gstEmWinHomePage;
            if (pCurrentEmWin)
            {
                pCurrentEmWin->delete_win();
                pCurrentEmWin = p_st_pre;
                pCurrentEmWin->create_win();
            }
            p_st_pre = NULL;
        }
        //  back last wind
    }
    return 0;
}

