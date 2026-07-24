/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-01-17
  * @brief       高级设置-电网标准设置
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-17    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define  EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE          (4096)
#define  EM_LIST_GRID_STAND_ICON_DOWN_ARROW_SIZE        (4096)
#define  EM_LIST_GRID_STAND_ICON_BACK_SIZE              (4096)
#define  EM_LIST_ICON_ARROW_SIZE                        (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListGridStand = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t grid_list_show;
    int32_t select;
    int32_t key_init;
    uint16_t gird_stand;
    uint32_t time_out;
} UiCtx_t;

static UiCtx_t _gstUiCtx;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

static char _garrTitleBuf[] = "Settings < Grid Standard";
static char _gGridStandBuf[] = "Grid Standard";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";
static char _garrCountryBuf[64] = {"China"};


static TEXT_Handle  _gTextGridStandHandle;//显示GridStandard句柄
static TEXT_Handle  _gTextCountryHandle;//显示Conutry句柄
static IMAGE_Handle  _gImgNationaHandle;//国旗图标句柄
static IMAGE_Handle  _gImgArrowHandle;//箭头图标句柄


/*List控件*/
static emListPara_t  _gstEmListOperaInfoPara;

static emWidgetTextPara_t  _gstGridStandTextPara;
static emWidgetTextPara_t  _gstCountryTextPara;
static emWidgetImgPara_t  _gstFlagImgPara;
static emWidgetImgPara_t  _garrArrowImgPara[2];   //0：右箭头   1：左箭头
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;

#define WIN_LIST_GRID_STAND_ID                                   (GUI_ID_USER + 0x10)
#define WIN_LIST_GRID_STAND_TITLE_ID                             (GUI_ID_USER + 0x11)
#define WIN_LIST_GRID_STAND_TEXT_ID                              (GUI_ID_USER + 0x12)
#define WIN_LIST_ARROW_IMG_ID                                    (GUI_ID_USER + 0x13)
#define WIN_LIST_GRID_STAND_FLAG_PRESS_ID                        (GUI_ID_USER + 0x14)
#define WIN_LIST_GRID_STAND_FLAG_BACK_IMG_ID                     (GUI_ID_USER + 0x15)
#define WIN_LIST_GRID_STAND_FLAG_BACK_ID                         (GUI_ID_USER + 0x16)

#define WIN_LIST_GRID_STAND_FLAG_IMG_ID                          (GUI_ID_USER + 0x17)
#define WIN_LIST_GRID_STAND_COUNTRY_TEXT_ID                      (GUI_ID_USER + 0x18)
#define WIN_LIST_GRID_STAND_ARROW_IMG_ID                         (GUI_ID_USER + 0x19)


static __SDRAM GUI_WIDGET_CREATE_INFO _garrListOperaInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListOperaInfoWin",    WIN_LIST_GRID_STAND_ID,                        0,      0,      480,    320,    0,   0x0,   0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_GRID_STAND_TITLE_ID,                       31,     20,     415,    14,     0,   0x0,   0 },
    { TEXT_CreateIndirect,    "Grid Standard",  WIN_LIST_GRID_STAND_TEXT_ID,                        31,     48,     152,    17,     0,   0x0,   0 },
    { IMAGE_CreateIndirect,   "Imgfalg",        WIN_LIST_GRID_STAND_FLAG_IMG_ID,                    295,    40,     30,     30,     0,   0x0,   0 },
    {TEXT_CreateIndirect,     "COUNTRY",        WIN_LIST_GRID_STAND_COUNTRY_TEXT_ID,                328,    48,     113,     17,     0,   0x0,   0 },
    {IMAGE_CreateIndirect,    "ARROW",          WIN_LIST_GRID_STAND_ARROW_IMG_ID,                   437,    50,     15,     15,     0,   0x0,   0 },
//    { IMAGE_CreateIndirect,     "Img",            WIN_LIST_ARROW_IMG_ID,                              434,    52,     20,     20,     0,   0x0,   0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_GRID_STAND_FLAG_PRESS_ID,                  133,     273,    33,     14,     0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_GRID_STAND_FLAG_BACK_IMG_ID,               171 - 5, 275,    15,     10,     0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_GRID_STAND_FLAG_BACK_ID,                   188,     273,    150,    14,     0,   0x0,  0 },
};

/*函数声明*/
void SetGridStandArrowDirection(void);
int32_t SetCurGridFlag(int32_t idx);


static int32_t _SetGridStandards(int32_t ui_update, uint16_t grididx)
{
    snprintf(_garrCountryBuf, sizeof(_garrCountryBuf), "%s", _garrNationalIdx[grididx].name);
    SpiFlashReadData(_garrNationalIdx[grididx].img_addr, _gstFlagImgPara.p_img_buf, _gstFlagImgPara.img_size);
    
    if(ui_update)
    {
        TEXT_SetText(_gTextCountryHandle, (const char *)_gstCountryTextPara.p_content);
        IMAGE_SetDTA(_gImgNationaHandle, _gstFlagImgPara.p_img_buf, _gstFlagImgPara.img_size);
    }
    return 0;
}

/**
  * @brief ListOperaInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListOperaInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            _gTextGridStandHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_TEXT_ID);
            TEXT_SetFont(_gTextGridStandHandle, _gstGridStandTextPara.p_text_font);
            TEXT_SetText(_gTextGridStandHandle, (const char *)_gstGridStandTextPara.p_content);
            TEXT_SetTextAlign(_gTextGridStandHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextGridStandHandle, _gstGridStandTextPara.text_color);

            _gTextCountryHandle = WM_GetDialogItem(pMsg->hWin,
                                                   WIN_LIST_GRID_STAND_COUNTRY_TEXT_ID);
            TEXT_SetFont(_gTextCountryHandle, _gstCountryTextPara.p_text_font);
            TEXT_SetText(_gTextCountryHandle, (const char *)_gstCountryTextPara.p_content);
            TEXT_SetTextAlign(_gTextCountryHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTextCountryHandle, _gstCountryTextPara.text_color);

            _gImgNationaHandle = WM_GetDialogItem(pMsg->hWin,
                                                  WIN_LIST_GRID_STAND_FLAG_IMG_ID);
            IMAGE_SetDTA(_gImgNationaHandle, _gstFlagImgPara.p_img_buf, _gstFlagImgPara.img_size);
            _gImgArrowHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_ARROW_IMG_ID);
            IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);


            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_FLAG_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_FLAG_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_FLAG_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            SetCurGridFlag(gstFlagIndex.flag_idx);
//            SetCurGridFlag(_gstUiCtx.gird_stand);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListOperaInfoPara, _gstEmListOperaInfoPara.list_pre_sel);

            GUI_SetColor(0x00424242);//上色
            GUI_FillRoundedRect(20, 38,  20 + 440 - 1,  38 + 34 - 1, 10);
            GUI_SetColor(0x00BDC909);
            GUI_DrawRoundedFrame(20, 38,  20 + 440 - 1,  38 + 34 - 1, 10, 3);

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
    _gstUiCtx.grid_list_show = 0;
    _gstUiCtx.key_init = 0;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListGridStand.win_handle = GUI_CreateDialogBox(_garrListOperaInfoDialogCreate, GUI_COUNTOF(_garrListOperaInfoDialogCreate), _cbListOperaInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListGridStand.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListGridStand.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListGridStand.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    //SelectedDraw(&_gstEmListOperaInfoPara, gstEmWinListGridStand.win_handle);
    //gstEmWinListGridStandFlag.create_win();
    return gstEmWinListGridStand.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListGridStand.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListGridStand.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListGridStand.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    EmKeyDefaultInfoSet();
//    _gstUiCtx.gird_stand = gstCollectParaData.pcs_data.udp_pcs_hold_para.grid_standards;
    
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;


    _gstGridStandTextPara.p_content = _gGridStandBuf;
    _gstGridStandTextPara.text_color = 0x00FFFFFF;
    _gstGridStandTextPara.p_text_font = EM_TEXT_FONT_5;


    /*country*/
    _gstCountryTextPara.p_text_font = EM_TEXT_FONT_5;
    _gstCountryTextPara.text_color = 0x00FFFFFF;
    _gstCountryTextPara.p_content = _garrCountryBuf;

    /*国旗*/
    _gstFlagImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE);
    _gstFlagImgPara.p_img_buf = GUI_ALLOC_h2p(_gstFlagImgPara.mem_img);
    _gstFlagImgPara.img_size = EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE;
//  SpiFlashReadData(EM_ICON_CHINESE_FLAG_ADDR,_gstFlagImgPara.p_img_buf,_gstFlagImgPara.img_size);
//    SpiFlashReadData(_garrNationalIdx[_gstUiCtx.gird_stand].img_addr, _gstFlagImgPara.p_img_buf, _gstFlagImgPara.img_size);

    /*右箭头*/
    _garrArrowImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_ICON_ARROW_SIZE);
    _garrArrowImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrArrowImgPara[0].mem_img);
    _garrArrowImgPara[0].img_size = EM_LIST_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);
    /*下箭头*/
    _garrArrowImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_GRID_STAND_ICON_DOWN_ARROW_SIZE);
    _garrArrowImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrArrowImgPara[1].mem_img);
    _garrArrowImgPara[1].img_size = EM_LIST_GRID_STAND_ICON_DOWN_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_DOWN_ARROW_ADDR, _garrArrowImgPara[1].p_img_buf, _garrArrowImgPara[1].img_size);


    _gstPressTextPara.p_content = _garrPressBuf;
    _gstPressTextPara.text_color = 0x00EEEEEE;
    _gstPressTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstBackTextPara.p_content = _garrBackBuf;
    _gstBackTextPara.text_color = 0x00EEEEEE;
    _gstBackTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_GRID_STAND_ICON_BACK_SIZE);
    _gstReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gstReturnImgPara.mem_img);
    _gstReturnImgPara.img_size = EM_LIST_GRID_STAND_ICON_BACK_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

    _SetGridStandards(0, gstFlagIndex.flag_idx);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_garrArrowImgPara[0].mem_img);
    GUI_ALLOC_Free(_garrArrowImgPara[1].mem_img);
    GUI_ALLOC_Free(_gstReturnImgPara.mem_img);
    GUI_ALLOC_Free(_gstFlagImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    if(gSwitchIconStates)
    {
        if(tim_get_systime() - gWaitTime < 1200)
        {
            gWaitTime = tim_get_systime();
            _SetGridStandards(1, gstFlagIndex.flag_idx); 
        }
        else
        {
            if(_gstUiCtx.gird_stand != gstCollectInfo.power_data.power_set_info.grid_standard)
            {
                _gstUiCtx.gird_stand = gstCollectInfo.power_data.power_set_info.grid_standard;
                _SetGridStandards(1, _gstUiCtx.gird_stand);
            }
            gSwitchIconStates = 0;
        }
    }
    
    if(gstFlagIndex.flag_idx > EN_NATIONALLIST_THA && gstFlagIndex.flag_idx < EN_NATIONALLIST_MAX)
    {
        WM_HideWindow(_gImgNationaHandle);
    }
    else
    {
        WM_ShowWindow(_gImgNationaHandle);
    }
    
#if 0
    if(_gstUiCtx.gird_stand != gstCollectInfo.power_data.power_set_info.grid_standard)
    {
        _gstUiCtx.gird_stand = gstCollectInfo.power_data.power_set_info.grid_standard;
        _SetGridStandards(1, _gstUiCtx.gird_stand);
    }
#endif
    if (_gstUiCtx.grid_list_show)
    {
        gstEmWinListGridStandFlag.polling();
    }
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

    if (_gstUiCtx.grid_list_show)
    {
        gstEmWinListGridStandFlag.key_handler(p_msg);

        if (!_gstUiCtx.grid_list_show)
        {
            gstEmWinListGridStandFlag.delete_win();
        }
        return;
    }

    if (event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }
    if (event == EN_KEY_SRC_CONFIRM)
    {
        if (!_gstUiCtx.grid_list_show)
        {
            gstEmWinListGridStandFlag.create_win();
            _gstUiCtx.grid_list_show = 1;
        }
        SetGridStandArrowDirection();
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
    {   
        if(!_gstUiCtx.grid_list_show && pCurrentEmWin->win_handle != 0)
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

void SetGridStandArrowDirection(void)
{
    if(_gstUiCtx.grid_list_show)
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


void SetGridListShow(int32_t show)
{
    _gstUiCtx.grid_list_show = show;
}

int32_t SetCurGridFlag(int32_t idx)
{
    if (idx < EN_NATIONALLIST_CN || idx >= EN_NATIONALLIST_MAX)
    {
        return -1;
    }

    snprintf(_garrCountryBuf, sizeof(_garrCountryBuf), "%s", _garrNationalIdx[idx].name);
    SpiFlashReadData(_garrNationalIdx[idx].img_addr, _gstFlagImgPara.p_img_buf,
                     _gstFlagImgPara.img_size);
    TEXT_SetText(_gTextCountryHandle, (const char *)_gstCountryTextPara.p_content);
    IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);
    IMAGE_SetDTA(_gImgNationaHandle, _gstFlagImgPara.p_img_buf, _gstFlagImgPara.img_size);
    return 0;
}

