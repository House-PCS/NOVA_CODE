/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-11
  * @brief       状态信息
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

#define  EM_BACK_ON_ICON_ARROW_SIZE                (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListStatusInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t system_status;
    uint16_t inv_status;
    uint16_t pv_status;
    uint16_t dcdc_status;
} UiCtx_t;



typedef struct _inverter_status_stru_t
{
    uint16_t status_id;
    char status[32];
} InverterStatus_t;

typedef struct _sys_status_stru_t
{
    uint16_t status_id;
    char status[32];

} SysStatus_t;

typedef struct _dcdc_status_stru_t
{
    uint16_t status_id;
    char status[32];
} DCDCStatus_t;

typedef struct _pv_status_stru_t
{
    uint16_t status_id;
    char status[32];
} PVStatus_t;

static SysStatus_t _garrSysStatus[] =
{
    {0x00,      "Stand-by"},
    {0x01,      "Self-test"},
    {0x02,      "Hybrid grid connection"},
    {0x03,      "Off-Grid"},
    {0x04,      "Bypass"},
    {0x05,      "Shutdown"},
    {0xFF,      "N/A"},
};

static InverterStatus_t _garrInvStatus[] =
{
    {0x00,      "Stand-by"},
    {0x01,      "Off-Grid"},
    {0x02,      "On-Grid"},
    {0x03,      "Off-Grid to On-Grid"},
    {0x04,      "On-Grid to Off-Grid"},
    {0xFF,      "N/A"},
};

static PVStatus_t _garrPVStatus[] =
{
    {0x00,      "Stand-by"},
    {0x01,      "Soft-start"},
    {0x02,      "Running"},
    {0x03,      "Fault shutdown"},
    {0xFF,      "N/A"},
};

static DCDCStatus_t _garrDCDCStatus[] =
{
    {0x00,      "Stand-by"},
    {0x01,      "Fault shutdown"},
    {0x02,      "Charging"},
    {0x03,      "Discharging"},
    {0xFF,      "N/A"},
};


static UiCtx_t _gstUiCtx;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;

static char _garrTitleBuf[] = "Operational State Info < Status Info";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";


/*List控件*/
static emListPara_t  _gstEmListStatusPara;

static char _garrListStatusLeftTextBuf[6][20];
static char _garrListStatusRightTextBuf[6][20];


#define WIN_LIST_STATUS_INFO_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_STATUS_INFO_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_STATUS_INFO_PRESS_ID                         (GUI_ID_USER + 0x14)
#define WIN_LIST_STATUS_INFO_BACK_ID                          (GUI_ID_USER + 0x15)
#define WIN_LIST_STATUS_INFO_BACK_IMG_ID                      (GUI_ID_USER + 0x16)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListGeneratorInputDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListStatusInfoWin",    WIN_LIST_STATUS_INFO_ID,          0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_STATUS_INFO_TITLE_ID,              31,  20,  415,  14,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Press",          WIN_LIST_STATUS_INFO_PRESS_ID,              133, 273, 33,   14,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "BackImg",        WIN_LIST_STATUS_INFO_BACK_IMG_ID,           171 - 5, 275, 15,   10,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Back",           WIN_LIST_STATUS_INFO_BACK_ID,               188, 273, 150,  14,   0,   0x0,  0 },
};


static uint32_t _SetInverterStatus(int32_t ui_update, uint16_t inv_stat)
{
    if(inv_stat > 0xFF)
    {
        inv_stat = 0;
    }
    
    snprintf(_garrListStatusRightTextBuf[0], sizeof(_garrListStatusRightTextBuf[0]), "%s", _garrInvStatus[inv_stat].status);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListStatusPara.handle[1][0], (const char *)_gstEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}

static uint32_t _SetDCDCStatus(int32_t ui_update, uint16_t dcdc_stat)
{
    if(dcdc_stat > 0)
    {
        dcdc_stat = 0;
    }
    
    snprintf(_garrListStatusRightTextBuf[1], sizeof(_garrListStatusRightTextBuf[1]), "%s", _garrDCDCStatus[dcdc_stat].status);

    if (ui_update)
    {
        TEXT_SetText(_gstEmListStatusPara.handle[1][1], (const char *)_gstEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    }

    return 0;
}


/**
  * @brief ListStatusInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListStatusInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_STATUS_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListStatusPara, pMsg->hWin);

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_STATUS_INFO_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_STATUS_INFO_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_STATUS_INFO_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);

            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListStatusPara, _gstEmListStatusPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListStatusPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListStatusPara.sub_region[i].x_pos,
                                    _gstEmListStatusPara.sub_region[i].y_pos,
                                    _gstEmListStatusPara.sub_region[i].x_pos + _gstEmListStatusPara.sub_region[i].x_size - 1,
                                    _gstEmListStatusPara.sub_region[i].y_pos + _gstEmListStatusPara.sub_region[i].y_size - 1,
                                    _gstEmListStatusPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListStatusPara, _gstEmListStatusPara.list_sel);

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
    gstEmWinListStatusInfo.win_handle = GUI_CreateDialogBox(_garrListGeneratorInputDialogCreate, GUI_COUNTOF(_garrListGeneratorInputDialogCreate), _cbListStatusInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListStatusInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListStatusInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListStatusInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListStatusPara, gstEmWinListStatusInfo.win_handle);
    return gstEmWinListStatusInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListStatusInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListStatusInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListStatusInfo.win_handle = 0;
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

    _gstUiCtx.inv_status  = gstCollectInfo.power_data.model_info.on_grid_state;
    _gstUiCtx.dcdc_status = gstCollectInfo.power_data.model_info.dcdc_run_state;


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


    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListStatusPara.list_start_id = WIN_LIST_STATUS_INFO_ID + 0x30;
    _gstEmListStatusPara.list_num = 2;
    _gstEmListStatusPara.list_display_num = 6;
    if(_gstEmListStatusPara.list_num <= _gstEmListStatusPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListStatusPara.list_num + 1 - _gstEmListStatusPara.list_display_num;
    }

    if(_gstEmListStatusPara.list_sel > _gstEmListStatusPara.list_num - 1)
    {
        _gstEmListStatusPara.list_sel = 0;
    }
    if(_gstEmListStatusPara.list_page > max_page - 1)
    {
        _gstEmListStatusPara.list_page = 0;
    }
    _gstEmListStatusPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _gstEmListStatusPara.selected_r = 10;

    /*3个子列表*/
    _gstEmListStatusPara.sub_region[0].x_pos = 20;
    _gstEmListStatusPara.sub_region[0].y_pos = 38;
    _gstEmListStatusPara.sub_region[0].x_size = 440;
    _gstEmListStatusPara.sub_region[0].y_size = 34;
    _gstEmListStatusPara.sub_region[1].x_pos = 20;
    _gstEmListStatusPara.sub_region[1].y_pos = 80;
    _gstEmListStatusPara.sub_region[1].x_size = 440;
    _gstEmListStatusPara.sub_region[1].y_size = 34;


    /*List控件*/

    //第一行左边
    _gstEmListStatusPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListStatusPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListStatusPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListStatusPara.sub_widget_para[0][0].x_size = 158;
    _gstEmListStatusPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListStatusPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListStatusLeftTextBuf[0];
    _gstEmListStatusPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListStatusPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListStatusPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Inverter");
    //第一行右边
    _gstEmListStatusPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListStatusPara.sub_widget_para[1][0].x_relative_pos = 218;
    _gstEmListStatusPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListStatusPara.sub_widget_para[1][0].x_size = 208;
    _gstEmListStatusPara.sub_widget_para[1][0].y_size = 17;
    _gstEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListStatusRightTextBuf[0];
    _gstEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "Bypass");
//    _gstEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.digit_value = 220;//测试数据，实际赋值系统数据
//    sprintf(_gstEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "%dV", (uint16_t)_gstEmListStatusPara.sub_widget_para[1][0].sub_widget.text_para.digit_value);
    //第二行左边
    _gstEmListStatusPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListStatusPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListStatusPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListStatusPara.sub_widget_para[0][1].x_size = 158;
    _gstEmListStatusPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListStatusPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListStatusLeftTextBuf[1];
    _gstEmListStatusPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListStatusPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListStatusPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Battery");
    //第二行右边
    _gstEmListStatusPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListStatusPara.sub_widget_para[1][1].x_relative_pos = 218;
    _gstEmListStatusPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gstEmListStatusPara.sub_widget_para[1][1].x_size = 208;
    _gstEmListStatusPara.sub_widget_para[1][1].y_size = 17;
    _gstEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListStatusRightTextBuf[1];
    _gstEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "Discharging");
//    _gstEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.digit_value = 20;//测试数据，实际赋值系统数据
//    sprintf(_gstEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "%dA", (uint16_t)_gstEmListStatusPara.sub_widget_para[1][1].sub_widget.text_para.digit_value);
    _SetInverterStatus(0, _gstUiCtx.inv_status);
    _SetDCDCStatus(0, _gstUiCtx.dcdc_status);
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gstReturnImgPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    if(_gstUiCtx.inv_status != gstCollectInfo.power_data.model_info.on_grid_state)
    {
        _gstUiCtx.inv_status = gstCollectInfo.power_data.model_info.on_grid_state;
        _SetInverterStatus(1, _gstUiCtx.inv_status);
    }

    if(_gstUiCtx.dcdc_status != gstCollectInfo.power_data.model_info.dcdc_run_state)
    {
        _gstUiCtx.dcdc_status = gstCollectInfo.power_data.model_info.dcdc_run_state;
        _SetDCDCStatus(1, _gstUiCtx.dcdc_status);
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

