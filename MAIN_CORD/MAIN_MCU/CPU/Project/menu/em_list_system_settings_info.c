/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-12
  * @brief       系统设定信息
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-12    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"

#define  EM_BACK_ON_ICON_ARROW_SIZE                             (4096)
#define  EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE                  (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinListSystemSettingsInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t work_mode;
    uint16_t grid_standards;
} UiCtx_t;

static UiCtx_t _gstUiCtx;


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄
static IMAGE_Handle  _gImgTempratureHandle;//温度图标句柄

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t   _gstReturnImgPara;
static emWidgetImgPara_t   _gstNationalFlagImgPara;


static char _garrTitleBuf[] = "Operational State Info < System Settings";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";


/*List控件*/
static emListPara_t  _gstEmListSystemSettingsInfoPara;

static char _garrListSystemSettingsInfoLeftTextBuf[6][20];
static char _garrListSystemSettingsInfoRightTextBuf[6][20];


#define WIN_LIST_TEMPERATURE_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_TEMPERATURE_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_TEMPERATURE_INFO_PRESS_ID                         (GUI_ID_USER + 0x14)
#define WIN_LIST_TEMPERATURE_INFO_BACK_ID                          (GUI_ID_USER + 0x15)
#define WIN_LIST_TEMPERATURE_INFO_BACK_IMG_ID                      (GUI_ID_USER + 0x16)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListGeneratorInputDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListSystemSettingsInfoWin",    WIN_LIST_TEMPERATURE_INFO_ID,             0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_TEMPERATURE_INFO_TITLE_ID,             31,  20,  415,  14,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_TEMPERATURE_INFO_PRESS_ID,             133, 273, 33,   14,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_TEMPERATURE_INFO_BACK_IMG_ID,          171 - 5, 275, 15,   10,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_TEMPERATURE_INFO_BACK_ID,              188, 273, 150,  14,   0,   0x0,  0 },
};

/**
  * @brief   设置工作模式
  * @param   ui_update
  * @param   work_mode
  * @return  int32_t
  * @note    None
  */
static int32_t _SetPcsWorkMode(uint8_t ui_update, uint16_t work_mode)
{
    switch(work_mode)
    {
        case 0:
        {
            snprintf(_garrListSystemSettingsInfoRightTextBuf[0], sizeof(_garrListSystemSettingsInfoRightTextBuf[0]),
                     "%s", "Self Consump");
            break;
        }
        case 1:
        {
            snprintf(_garrListSystemSettingsInfoRightTextBuf[0], sizeof(_garrListSystemSettingsInfoRightTextBuf[0]),
                     "%s", "BAT Priority");
            break;
        }
        case 2:
        {
            snprintf(_garrListSystemSettingsInfoRightTextBuf[0], sizeof(_garrListSystemSettingsInfoRightTextBuf[0]),
                     "%s", "Peak Shifting");
            break;
        }
        default:
            break;
    }

    if(ui_update)
    {
        TEXT_SetText( _gstEmListSystemSettingsInfoPara.handle[1][0], (const char *)_gstEmListSystemSettingsInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}
/**
  * @brief   设置电网标准
  * @param   ui_update
  * @param   work_mode
  * @return  int32_t
  * @note    None
  */
static int32_t _SetSysGridStandards(uint8_t ui_update, uint16_t grid_standards)
{
    snprintf(_garrListSystemSettingsInfoRightTextBuf[1], sizeof(_garrListSystemSettingsInfoRightTextBuf[1]), "%s", _garrNationalIdx[grid_standards].name);
    SpiFlashReadData(_garrNationalIdx[grid_standards].img_addr, _gstNationalFlagImgPara.p_img_buf, _gstNationalFlagImgPara.img_size);

    if(ui_update)
    {
        TEXT_SetText(_gstEmListSystemSettingsInfoPara.handle[2][1], (const char *)_gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content);
        IMAGE_SetDTA(_gstEmListSystemSettingsInfoPara.handle[1][1],  _gstNationalFlagImgPara.p_img_buf, _gstNationalFlagImgPara.img_size);
    }
    return 0;
}

/**
  * @brief ListSystemSettingsInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListSystemSettingsInfoWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListSystemSettingsInfoPara, pMsg->hWin);

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_TEMPERATURE_INFO_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListSystemSettingsInfoPara, _gstEmListSystemSettingsInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListSystemSettingsInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListSystemSettingsInfoPara.sub_region[i].x_pos,
                                    _gstEmListSystemSettingsInfoPara.sub_region[i].y_pos,
                                    _gstEmListSystemSettingsInfoPara.sub_region[i].x_pos + _gstEmListSystemSettingsInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListSystemSettingsInfoPara.sub_region[i].y_pos + _gstEmListSystemSettingsInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListSystemSettingsInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListSystemSettingsInfoPara, _gstEmListSystemSettingsInfoPara.list_sel);

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
    pPrevEmWin = &gstEmWinListEquipOperaInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListSystemSettingsInfo.win_handle = GUI_CreateDialogBox(_garrListGeneratorInputDialogCreate, GUI_COUNTOF(_garrListGeneratorInputDialogCreate), _cbListSystemSettingsInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListSystemSettingsInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListSystemSettingsInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListSystemSettingsInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListSystemSettingsInfoPara, gstEmWinListSystemSettingsInfo.win_handle);
    return gstEmWinListSystemSettingsInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListSystemSettingsInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListSystemSettingsInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListSystemSettingsInfo.win_handle = 0;
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

    _gstUiCtx.work_mode = gstCollectInfo.power_data.power_set_info.work_mode;
    _gstUiCtx.grid_standards = gstCollectInfo.power_data.power_set_info.grid_standard;

    /*底部back的参数*/
    _gstPressTextPara.p_content = _garrPressBuf;
    _gstPressTextPara.text_color = 0x00EEEEEE;
    _gstPressTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstBackTextPara.p_content = _garrBackBuf;
    _gstBackTextPara.text_color = 0x00EEEEEE;
    _gstBackTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstReturnImgPara.mem_img = GUI_ALLOC_AllocZero(EM_BACK_ON_ICON_ARROW_SIZE);
    _gstReturnImgPara.p_img_buf = GUI_ALLOC_h2p(_gstReturnImgPara.mem_img);
    _gstReturnImgPara.img_size = EM_BACK_ON_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_BACK_ON_ADDR, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);
    /*国旗图标的参数*/
    _gstNationalFlagImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE);
    _gstNationalFlagImgPara.p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara.mem_img);
    _gstNationalFlagImgPara.img_size = EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_GERMAN_FLAG_ADDR, _gstNationalFlagImgPara.p_img_buf, _gstNationalFlagImgPara.img_size);

    /*标题*/
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListSystemSettingsInfoPara.list_start_id = WIN_LIST_TEMPERATURE_INFO_ID + 0x30;
    _gstEmListSystemSettingsInfoPara.list_num = 2;
    _gstEmListSystemSettingsInfoPara.list_display_num = 6;
    if(_gstEmListSystemSettingsInfoPara.list_num <= _gstEmListSystemSettingsInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListSystemSettingsInfoPara.list_num + 1 - _gstEmListSystemSettingsInfoPara.list_display_num;
    }

    if(_gstEmListSystemSettingsInfoPara.list_sel > _gstEmListSystemSettingsInfoPara.list_num - 1)
    {
        _gstEmListSystemSettingsInfoPara.list_sel = 0;
    }
    if(_gstEmListSystemSettingsInfoPara.list_page > max_page - 1)
    {
        _gstEmListSystemSettingsInfoPara.list_page = 0;
    }
    _gstEmListSystemSettingsInfoPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _gstEmListSystemSettingsInfoPara.selected_r = 10;

    /*4个子列表*/
    _gstEmListSystemSettingsInfoPara.sub_region[0].x_pos = 20;
    _gstEmListSystemSettingsInfoPara.sub_region[0].y_pos = 38;
    _gstEmListSystemSettingsInfoPara.sub_region[0].x_size = 440;
    _gstEmListSystemSettingsInfoPara.sub_region[0].y_size = 34;
    _gstEmListSystemSettingsInfoPara.sub_region[1].x_pos = 20;
    _gstEmListSystemSettingsInfoPara.sub_region[1].y_pos = 80;
    _gstEmListSystemSettingsInfoPara.sub_region[1].x_size = 440;
    _gstEmListSystemSettingsInfoPara.sub_region[1].y_size = 34;

    /*List控件*/
    //第一行左边
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][0].x_size = 158;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListSystemSettingsInfoLeftTextBuf[0];
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSystemSettingsInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Working Mode");
    //第一行右边
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][0].x_relative_pos = 218;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][0].x_size = 208;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][0].y_size = 17;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListSystemSettingsInfoRightTextBuf[0];
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _SetPcsWorkMode(0, _gstUiCtx.work_mode);
//    strcpy(_gstEmListSystemSettingsInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "Self Consumpt");
//    _gstEmListSystemSettingsInfoPara.sub_widget_para[2][0].sub_widget.text_para.digit_value = 114;//测试数据，实际赋值系统数据
//    sprintf(_gstEmListSystemSettingsInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content, "%dF", (uint16_t)_gstEmListSystemSettingsInfoPara.sub_widget_para[2][0].sub_widget.text_para.digit_value);
    //第二行左边
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][1].x_size = 230;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListSystemSettingsInfoLeftTextBuf[1];
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSystemSettingsInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Grid-connected Standard");
    //第二行中间图标
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][1].x_relative_pos = 286;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][1].y_relative_pos = 2;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][1].x_size = 30;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][1].y_size = 30;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gstNationalFlagImgPara.mem_img;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara.p_img_buf;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE;
    //第二行右边
    _gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].x_relative_pos = 218;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].y_relative_pos = 10;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].x_size = 208;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].y_size = 17;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content = _garrListSystemSettingsInfoRightTextBuf[1];
    _gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _SetSysGridStandards(0, _gstUiCtx.grid_standards);
//    strcpy(_gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content, "German");
//    _gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.digit_value = 124;//测试数据，实际赋值系统数据
//    sprintf(_gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.p_content, "%dF", (uint16_t)_gstEmListSystemSettingsInfoPara.sub_widget_para[2][1].sub_widget.text_para.digit_value);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gstReturnImgPara.mem_img);
    GUI_ALLOC_Free(_gstNationalFlagImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    if(_gstUiCtx.work_mode != gstCollectInfo.power_data.power_set_info.work_mode)
    {
        _gstUiCtx.work_mode = gstCollectInfo.power_data.power_set_info.work_mode;
        _SetPcsWorkMode(1, _gstUiCtx.work_mode);
    }

    if(_gstUiCtx.grid_standards != gstCollectInfo.power_data.power_set_info.grid_standard)
    {
        _gstUiCtx.grid_standards = gstCollectInfo.power_data.power_set_info.grid_standard;
        _SetSysGridStandards(1, _gstUiCtx.grid_standards);
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
    if (event < 0 || event > EN_KEY_SRC_NUM)
    {
        return;
    }

    if (event == EN_KEY_SRC_ESC)
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
                pnext = &gstEmWinListEquipOperaInfo;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}

