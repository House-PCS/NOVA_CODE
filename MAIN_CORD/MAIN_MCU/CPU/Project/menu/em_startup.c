/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_startup.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-12-12
  * @brief       启动界面
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

#define EM_STARTUP_ICON_BACKGROUND_SIZE        (452 * 1024)

#define EN_STARTUP_PROGRESS_BAR_XPOS           143
#define EN_STARTUP_PROGRESS_BAR_YPOS           200
#define EN_STARTUP_PROGRESS_BAR_LENGTH         194
#define EN_STARTUP_PROGRESS_BAR_WIDTH          5


#define GUI_COLOR_PROGRESS_BAR_1          GUI_MAKE_COLOR(0x00434802)
#define GUI_COLOR_PROGRESS_BAR_2          GUI_MAKE_COLOR(0x00D6E400)

//#define GUI_COLOR_PROGRESS_BAR_1          GUI_MAKE_COLOR(0x00000002)
//#define GUI_COLOR_PROGRESS_BAR_2          GUI_MAKE_COLOR(0x000000FF)

//#define GUI_COLOR_PROGRESS_BAR_1          GUI_MAKE_COLOR(0x00434802)
//#define GUI_COLOR_PROGRESS_BAR_2          GUI_MAKE_COLOR(0x00D6E400)



static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinStartup = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

/* 存储设备相关变量 */
static GUI_HMEM _gMemBK;
static uint8_t *_pBKBuf;
static uint8_t  _gProgressBarRate = 0;



/**
  * @brief FuncListWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbStartupWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;

    switch (pMsg->MsgId)
    {
        case WM_CREATE:
        {
            _gProgressBarRate = 0;
            /* 创建定时器 */
            WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                           0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                           10,         /* 周期，此周期过后指定窗口应收到消息*/
                           0);         /* 留待将来使用，应为0 */
            break;
        }
        case WM_DELETE:
        {
            break;
        }
        case WM_TIMER:
        {
            WM_RestartTimer(pMsg->Data.v, 10);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#if 0
            _gProgressBarRate += 30;
            if(_gProgressBarRate >= EN_STARTUP_PROGRESS_BAR_LENGTH - 1)
            {
                _gProgressBarRate = EN_STARTUP_PROGRESS_BAR_LENGTH - 1;
                //_gProgressBarRate = 0;
            }
            WM_Paint(pMsg->hWin);
#else
            _gProgressBarRate += 10;
            if(_gProgressBarRate >= EN_STARTUP_PROGRESS_BAR_LENGTH - 1)
            {
                _gProgressBarRate = EN_STARTUP_PROGRESS_BAR_LENGTH - 1;
            }

            /*处理进度条，重新绘制*/

            hWinOld = WM_SelectWindow(pMsg->hWin);//先选择要在哪个窗口绘制
            GUI_SetColor(GUI_COLOR_PROGRESS_BAR_2);//上色
            GUI_SetPenSize(EN_STARTUP_PROGRESS_BAR_WIDTH);
            GUI_DrawLine(EN_STARTUP_PROGRESS_BAR_XPOS, EN_STARTUP_PROGRESS_BAR_YPOS, EN_STARTUP_PROGRESS_BAR_XPOS + _gProgressBarRate, EN_STARTUP_PROGRESS_BAR_YPOS);

            WM_SelectWindow(hWinOld);//恢复绘制之前的窗口选择
#endif
            break;
        }
        case WM_PAINT:
        {
            GUI_DrawStreamedBitmapAuto(_pBKBuf, 0, 0);
            GUI_SetColor(GUI_COLOR_PROGRESS_BAR_1);//底色
            GUI_SetPenSize(EN_STARTUP_PROGRESS_BAR_WIDTH);
            GUI_DrawLine(EN_STARTUP_PROGRESS_BAR_XPOS, EN_STARTUP_PROGRESS_BAR_YPOS,
                         EN_STARTUP_PROGRESS_BAR_XPOS + EN_STARTUP_PROGRESS_BAR_LENGTH - 1,
                         EN_STARTUP_PROGRESS_BAR_YPOS);
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
    /*创建一个基于桌面窗口的子窗口*/
    gstEmWinStartup.win_handle = WM_CreateWindow(0, 0, EM_XSIZE, EM_YSIZE, WM_CF_SHOW, _cbStartupWin, 0);
//    gstEmWinStartup.win_handle = WM_CreateWindowAsChild(0, 0, EM_XSIZE, EM_YSIZE, WM_HBKWIN, WM_CF_SHOW, _cbStartupWin, 0);
    pCurrentEmWin = &gstEmWinStartup;
    GUI_Exec();
//    _EndWin();
    return gstEmWinStartup.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinStartup.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinStartup.win_handle);
//    GUI_EndDialog(gstEmWinStartup.win_handle, 0);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinStartup.win_handle = 0;
}

/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    /*创建背景图内存*/
//    _gMemBK = GUI_MEMDEV_CreateFixed(0,0, EM_XSIZE, EM_YSIZE, GUI_MEMDEV_HASTRANS, GUI_MEMDEV_APILIST_16, GUI_COLOR_CONV_565);

    /*将背景图绘制在存储设备_gMemBK*/
//    GUI_MEMDEV_Select(_gMemBK);
//    GUI_SetBkColor(GUI_TRANSPARENT);
//    GUI_Clear();
//    GUI_MEMDEV_Select(0);

    /*为背景图申请内存*/
    _gMemBK = GUI_ALLOC_AllocZero(EM_STARTUP_ICON_BACKGROUND_SIZE);
    _pBKBuf = GUI_ALLOC_h2p(_gMemBK);
    SpiFlashReadData(EM_ICON_START_ADDR, _pBKBuf, EM_STARTUP_ICON_BACKGROUND_SIZE);
//    SpiFlashReadData(EM_ICON_BACKGROUND_ADDR, _pBKBuf, EM_STARTUP_ICON_BACKGROUND_SIZE);
    /*基本参数初始化*/
    _gProgressBarRate = 0;
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gMemBK);
}

/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    if(_gProgressBarRate == EN_STARTUP_PROGRESS_BAR_LENGTH - 1)
    {
        if(pCurrentEmWin->win_handle != 0)
        {
            pCurrentEmWin->delete_win();
            pCurrentEmWin = &gstEmWinHomePage;           
//            pCurrentEmWin = &gstEmWinMain;
            pCurrentEmWin->create_win();
        }
    }
}

/**
  * @brief  当前界面按键解析
  * @param  NULL
  * @retval NULL
  */
static void _keyHandler(KeyNewMsg_t *p_msg)
{

}



