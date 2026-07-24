/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-08
  * @brief       设备基本信息ListView界面
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-08    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "em_menu.h"

#define  EM_BACK_ON_ICON_ARROW_SIZE                         (4096)
#define  EM_LIST_OPERA_INFO_ICON_ARROW_SIZE                 (4096)

#define  EM_REMOVE_FIVE_INFO                                (1)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListDevBaseInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t pcs_model;
    uint32_t inv_version;
    uint16_t sn[10];
} UiCtx_t;

static UiCtx_t _gstUiCtx;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstBackTextPara;
static emWidgetImgPara_t  _gstReturnImgPara;

static TEXT_Handle  _gPressHandle;//Press句柄
static TEXT_Handle  _gBackHandle;//return back句柄
static IMAGE_Handle  _gImgReturnHandle;//图标句柄

static char _garrTitleBuf[] = "Basic Info";
static char _garrPressBuf[] = "Press";
static char _garrBackBuf[] = "return back to previous page.";

/*List控件*/
static emListPara_t  _gstEmListDevBaseInfoPara;

static char _garrListDevBaseInfoLeftTextBuf[6][32];
static char _garrListDevBaseInfoRightText3Buf[3][32];
static GUI_HMEM _gListDevBaseInfoRightMem;
static uint8_t *_pListDevBaseInfoRightBuf;

#define WIN_LIST_OPERA_INFO_ID                               (GUI_ID_USER + 0x10)
#define WIN_LIST_OPERA_INFO_TITLE_ID                         (GUI_ID_USER + 0x11)
#define WIN_LIST_PRESS_ID                                    (GUI_ID_USER + 0x12)
#define WIN_LIST_BACK_IMG_ID                                 (GUI_ID_USER + 0x13)
#define WIN_LIST_BACK_ID                                     (GUI_ID_USER + 0x14)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListDevBaseInfoDialogCreate[] =
{
    {WINDOW_CreateIndirect,  "ListDevBaseInfoWin",      WIN_LIST_OPERA_INFO_ID,             0,        0,    480,  320,  0,   0x0,  0},
    {TEXT_CreateIndirect,    "Basic Info",              WIN_LIST_OPERA_INFO_TITLE_ID,       31,       20,   144,  14,   0,   0x0,  0},
    {TEXT_CreateIndirect,    "Press",                   WIN_LIST_PRESS_ID,                  133,      273,  33,   14,   0,   0x0,  0},
    {IMAGE_CreateIndirect,   "BackImg",                 WIN_LIST_BACK_IMG_ID,               171 - 5,  275,  15,   10,   0,   0x0,  0},
    {TEXT_CreateIndirect,    "Back",                    WIN_LIST_BACK_ID,                   188,      273,  150,  14,   0,   0x0,  0},
};

static int32_t _SetProductModel(uint8_t ui_update, uint16_t pcs_model)
{
    char arr_model[32] = {0};
    const char* p_mode = GetPcsModelByModelId(_gstUiCtx.pcs_model);
    _gstUiCtx.pcs_model = pcs_model;

    if(p_mode == NULL)
    {
        snprintf(_garrListDevBaseInfoRightText3Buf[0],
                 sizeof(_garrListDevBaseInfoRightText3Buf[0]), "%s", "Unknown");
        if(ui_update)
        {
            TEXT_SetText(_gstEmListDevBaseInfoPara.handle[1][0],
                (const char *)_gstEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
        }
        return 0;
    }
    
    snprintf(arr_model, sizeof(arr_model), "%s", p_mode);
    
    if (gDeviceVersionType == DEVICE_TYPE_LCD)
    {
        snprintf(_garrListDevBaseInfoRightText3Buf[0],
                 sizeof(_garrListDevBaseInfoRightText3Buf[0]), "%s-C-SP", arr_model);
    }
    else if (gDeviceVersionType == DEVICE_TYPE_LED)
    {
        snprintf(_garrListDevBaseInfoRightText3Buf[0],
                 sizeof(_garrListDevBaseInfoRightText3Buf[0]), "%s-E-SP", arr_model);
    }

    if(ui_update)
    {
        TEXT_SetText(_gstEmListDevBaseInfoPara.handle[1][0],
            (const char *)_gstEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content);
    }
    return 0;
}

static int32_t _SetProductSn(const char *p_model)
{
    snprintf(_garrListDevBaseInfoRightText3Buf[1], sizeof(_garrListDevBaseInfoRightText3Buf[1]),
             "%s", p_model);

    TEXT_SetText(_gstEmListDevBaseInfoPara.handle[1][1],
                 (const char *)_gstEmListDevBaseInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content);
    return 0;
}

static int32_t _SetInverterVer(uint8_t ui_update, uint32_t sw)
{
    uint8_t sw_highbit8 = sw >> 16;
    uint8_t sw_midbit8  = sw >> 8;
    uint8_t sw_lowbit8  = sw >> 0;

    snprintf(_garrListDevBaseInfoRightText3Buf[2], sizeof(_garrListDevBaseInfoRightText3Buf[2]),
             "V%d.%d.%d", sw_highbit8, sw_midbit8, sw_lowbit8);
    if(ui_update)
    {
        TEXT_SetText(_gstEmListDevBaseInfoPara.handle[1][2],
                     (const char *)_gstEmListDevBaseInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content);
    }
    return 0;
}

/**
  * @brief ListDevBaseInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListDevBaseInfoWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_OPERA_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListDevBaseInfoPara, pMsg->hWin);

            _SetProductModel(1, _gstUiCtx.pcs_model);

            /*底部return back*/
            _gBackHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BACK_ID);
            TEXT_SetFont(_gBackHandle, _gstBackTextPara.p_text_font);
            TEXT_SetText(_gBackHandle, (const char *)_gstBackTextPara.p_content);
            TEXT_SetTextAlign(_gBackHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gBackHandle, _gstBackTextPara.text_color);
            /*底部Press*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);
            /*底部图标*/
            _gImgReturnHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_BACK_IMG_ID);
            IMAGE_SetDTA(_gImgReturnHandle, _gstReturnImgPara.p_img_buf, _gstReturnImgPara.img_size);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListDevBaseInfoPara, _gstEmListDevBaseInfoPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(i = 0; i < _gstEmListDevBaseInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListDevBaseInfoPara.sub_region[i].x_pos,
                                    _gstEmListDevBaseInfoPara.sub_region[i].y_pos,
                                    _gstEmListDevBaseInfoPara.sub_region[i].x_pos + _gstEmListDevBaseInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListDevBaseInfoPara.sub_region[i].y_pos + _gstEmListDevBaseInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListDevBaseInfoPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListDevBaseInfoPara, _gstEmListDevBaseInfoPara.list_sel);
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
    pPrevEmWin =  &gstEmWinMain3DNav;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListDevBaseInfo.win_handle = GUI_CreateDialogBox(_garrListDevBaseInfoDialogCreate, GUI_COUNTOF(_garrListDevBaseInfoDialogCreate), _cbListDevBaseInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListDevBaseInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListDevBaseInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListDevBaseInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListDevBaseInfoPara, gstEmWinListDevBaseInfo.win_handle);
    return gstEmWinListDevBaseInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListDevBaseInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListDevBaseInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListDevBaseInfo.win_handle = 0;
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
    
    _gstUiCtx.pcs_model = gstCollectInfo.power_data.pcs_base_info.model_id;
    _gstUiCtx.inv_version = gstCollectInfo.maincard_data.base_info.sw_ver;
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

    _gstEmListDevBaseInfoPara.list_start_id = WIN_LIST_OPERA_INFO_ID + 0x30;
    _gstEmListDevBaseInfoPara.list_num = 5;//5;
    _gstEmListDevBaseInfoPara.list_display_num = 5; //5;
    if(_gstEmListDevBaseInfoPara.list_num <= _gstEmListDevBaseInfoPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListDevBaseInfoPara.list_num + 1 - _gstEmListDevBaseInfoPara.list_display_num;
    }


    if(_gstEmListDevBaseInfoPara.list_sel > _gstEmListDevBaseInfoPara.list_num - 1)
    {
        _gstEmListDevBaseInfoPara.list_sel = 0;
    }
    if(_gstEmListDevBaseInfoPara.list_page > max_page - 1)
    {
        _gstEmListDevBaseInfoPara.list_page = 0;
    }
    _gstEmListDevBaseInfoPara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gstEmListDevBaseInfoPara.sub_region[0].x_pos = 20;
    _gstEmListDevBaseInfoPara.sub_region[0].y_pos = 38;
    _gstEmListDevBaseInfoPara.sub_region[0].x_size = 420;
    _gstEmListDevBaseInfoPara.sub_region[0].y_size = 34;
    _gstEmListDevBaseInfoPara.sub_region[1].x_pos = 20;
    _gstEmListDevBaseInfoPara.sub_region[1].y_pos = 80;
    _gstEmListDevBaseInfoPara.sub_region[1].x_size = 420;
    _gstEmListDevBaseInfoPara.sub_region[1].y_size = 34;
    _gstEmListDevBaseInfoPara.sub_region[2].x_pos = 20;
    _gstEmListDevBaseInfoPara.sub_region[2].y_pos = 122;
    _gstEmListDevBaseInfoPara.sub_region[2].x_size = 420;
    _gstEmListDevBaseInfoPara.sub_region[2].y_size = 34;
    _gstEmListDevBaseInfoPara.sub_region[3].x_pos = 20;
    _gstEmListDevBaseInfoPara.sub_region[3].y_pos = 164;
    _gstEmListDevBaseInfoPara.sub_region[3].x_size = 420;
    _gstEmListDevBaseInfoPara.sub_region[3].y_size = 34;
    _gstEmListDevBaseInfoPara.sub_region[4].x_pos = 20;
    _gstEmListDevBaseInfoPara.sub_region[4].y_pos = 206;
    _gstEmListDevBaseInfoPara.sub_region[4].x_size = 420;
    _gstEmListDevBaseInfoPara.sub_region[4].y_size = 34;
#if EM_REMOVE_FIVE_INFO
    _gstEmListDevBaseInfoPara.sub_region[5].x_pos = 20;
    _gstEmListDevBaseInfoPara.sub_region[5].y_pos = 248;
    _gstEmListDevBaseInfoPara.sub_region[5].x_size = 420;
    _gstEmListDevBaseInfoPara.sub_region[5].y_size = 34;
#endif
    /*List控件*/
    _gListDevBaseInfoRightMem = GUI_ALLOC_AllocZero(EM_LIST_OPERA_INFO_ICON_ARROW_SIZE);
    _pListDevBaseInfoRightBuf = GUI_ALLOC_h2p(_gListDevBaseInfoRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListDevBaseInfoRightBuf, EM_LIST_OPERA_INFO_ICON_ARROW_SIZE);
    //第一行左边
    _gstEmListDevBaseInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][0].x_size = 158;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListDevBaseInfoLeftTextBuf[0];
    _gstEmListDevBaseInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;

    strcpy(_gstEmListDevBaseInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Product Model");
    //第一行右边
    _gstEmListDevBaseInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][0].x_relative_pos = 180;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][0].x_size = 230;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][0].y_size = 16;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListDevBaseInfoRightText3Buf[0];
    _gstEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;

//    char arr_model[32] = {0};
//    _gstUiCtx.pcs_model = gstCollectInfo.power_data.pcs_base_info.model_id;
//    snprintf(arr_model, sizeof(arr_model), "%s", GetPcsModelByModelId(_gstUiCtx.pcs_model));
    _SetProductModel(0, _gstUiCtx.pcs_model);
#if 0  
    if (gDeviceVersionType == DEVICE_TYPE_LCD)
    {
        strcpy(_gstEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "Roca2-16kL-C");
    }
    else if (gDeviceVersionType == DEVICE_TYPE_LED)
    {
        strcpy(_gstEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "Roca2-16kL-E");
    }
    else
    {
        strcpy(_gstEmListDevBaseInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "Roca2-16kL");
    }
#endif
    //第二行左边
    _gstEmListDevBaseInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][1].x_size = 158;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListDevBaseInfoLeftTextBuf[1];
    _gstEmListDevBaseInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListDevBaseInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Product SN");
    //第二行右边
    _gstEmListDevBaseInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][1].x_relative_pos = 180;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][1].y_relative_pos = 10;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][1].x_size = 230;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][1].y_size = 16;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListDevBaseInfoRightText3Buf[1];
    _gstEmListDevBaseInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    strcpy(_gstEmListDevBaseInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "1234567890");

    //第三行左边
    _gstEmListDevBaseInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][2].x_size = 158;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListDevBaseInfoLeftTextBuf[2];
    _gstEmListDevBaseInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListDevBaseInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Major Version");
    //第三行右边
    _gstEmListDevBaseInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][2].x_relative_pos = 180;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][2].y_relative_pos = 10;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][2].x_size = 230;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][2].y_size = 16;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListDevBaseInfoRightText3Buf[2];
    _gstEmListDevBaseInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    _SetInverterVer(0, _gstUiCtx.inv_version);
    // _gstEmListDevBaseInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    // _gstEmListDevBaseInfoPara.sub_widget_para[1][2].x_relative_pos = 393;
    // _gstEmListDevBaseInfoPara.sub_widget_para[1][2].y_relative_pos = 11;
    // _gstEmListDevBaseInfoPara.sub_widget_para[1][2].x_size = 97;
    // _gstEmListDevBaseInfoPara.sub_widget_para[1][2].y_size = 17;
    // _gstEmListDevBaseInfoPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gListDevBaseInfoRightMem;
    // _gstEmListDevBaseInfoPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _pListDevBaseInfoRightBuf;
    // _gstEmListDevBaseInfoPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
    //第四行左边
    _gstEmListDevBaseInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][3].x_size = 200;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListDevBaseInfoLeftTextBuf[3];
    _gstEmListDevBaseInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListDevBaseInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Inverter Version");
    //第四行右边
    _gstEmListDevBaseInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][3].x_relative_pos = 393;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][3].y_relative_pos = 11;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][3].x_size = 7;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][3].y_size = 12;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gListDevBaseInfoRightMem;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _pListDevBaseInfoRightBuf;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
#if EM_REMOVE_FIVE_INFO    
    //第五行左边
    _gstEmListDevBaseInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][4].x_size = 200;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListDevBaseInfoLeftTextBuf[4];
    _gstEmListDevBaseInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListDevBaseInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListDevBaseInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Battery Pack Version");
    //第五行右边
    _gstEmListDevBaseInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][4].x_relative_pos = 393;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][4].y_relative_pos = 11;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][4].x_size = 7;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][4].y_size = 12;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gListDevBaseInfoRightMem;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _pListDevBaseInfoRightBuf;
    _gstEmListDevBaseInfoPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_OPERA_INFO_ICON_ARROW_SIZE;
#endif
    /*List选中框*/
    _gstEmListDevBaseInfoPara.selected_unvalid_color = 0xFF000000;
    _gstEmListDevBaseInfoPara.selected_valid_color = 0x00D6E400;
    _gstEmListDevBaseInfoPara.selected_size = 3;
    _gstEmListDevBaseInfoPara.selected_r = 10;
    _gstEmListDevBaseInfoPara.selected_x_start_offset = -2;
    _gstEmListDevBaseInfoPara.selected_y_start_offset = -2;
    _gstEmListDevBaseInfoPara.selected_x_end_offset = 2;
    _gstEmListDevBaseInfoPara.selected_y_end_offset = 2;
    _gstEmListDevBaseInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListDevBaseInfoPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    /*滚动条*/
    if (max_page > 1)
    {
        _gstEmListDevBaseInfoPara.scrollbar_enable = true;
        _gstEmListDevBaseInfoPara.scrollbar_color = 0x00D6E400;
        _gstEmListDevBaseInfoPara.scrollbar_bk_color = 0x00565656;
        _gstEmListDevBaseInfoPara.scrollbar_x_pos = 454;
        _gstEmListDevBaseInfoPara.scrollbar_y_pos = 39;
        _gstEmListDevBaseInfoPara.scrollbar_x_size = 6;
        _gstEmListDevBaseInfoPara.scrollbar_y_size = 241;
    }
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gListDevBaseInfoRightMem);
    GUI_ALLOC_Free(_gstReturnImgPara.mem_img);
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
    uint8_t arr_model_buff[256 + 8] = {0};
    uint8_t arr_sn_buff[256 + 8] = {0};
    if(gstEmWinListDevBaseInfo.win_handle == 0)
    {
        return;
    }

    if (_gstUiCtx.pcs_model != gstCollectInfo.power_data.pcs_base_info.model_id)
    {
//        char arr_model[32] = {0};
        _gstUiCtx.pcs_model = gstCollectInfo.power_data.pcs_base_info.model_id;
//        snprintf(arr_model, sizeof(arr_model), "%s", GetPcsModelByModelId(_gstUiCtx.pcs_model));
        _SetProductModel(1, _gstUiCtx.pcs_model);

        // uint16_t tmp = atoi(arr_model);
        // uint16_t model_num = BigLittleSwap16(tmp);

        // ModbusWriteSingleRemain(arr_model_buff, 0x01, COLLECT_MODE_ID, model_num);
    }

    if (memcmp(_gstUiCtx.sn, gstCollectInfo.maincard_data.base_info.sn, sizeof(_gstUiCtx.sn)))
    {
        char sn[sizeof(_gstUiCtx.sn) + 1] = {0};
        memcpy(_gstUiCtx.sn, gstCollectInfo.maincard_data.base_info.sn, sizeof(_gstUiCtx.sn));
        if (_gstUiCtx.sn[0] == 0)
        {
            snprintf(sn, sizeof(sn), "%s", "unknown");
        }
        else
        {
            snprintf(sn, sizeof(sn), "%s", (char *)_gstUiCtx.sn);
            _SetProductSn((const char *)_gstUiCtx.sn);
        }
    }
    
    if (_gstUiCtx.inv_version != gstCollectInfo.maincard_data.base_info.sw_ver)
    {
        _gstUiCtx.inv_version = gstCollectInfo.maincard_data.base_info.sw_ver;
        _SetInverterVer(1, _gstUiCtx.inv_version);
    }

#if 0
    if(GetSystick() - timecount > 30)
    {
        timecount = GetSystick();
        if(_gstEmListDevBaseInfoPara.list_sel >= _gstEmListDevBaseInfoPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListDevBaseInfoPara.list_sel == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListDevBaseInfoPara, gstEmWinListDevBaseInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListDevBaseInfoPara, gstEmWinListDevBaseInfo.win_handle);
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
        EmListUpward(&_gstEmListDevBaseInfoPara, gstEmWinListDevBaseInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListDevBaseInfoPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListDevBaseInfoPara, gstEmWinListDevBaseInfo.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        emWin_t *pnext = NULL;
        switch(_gstUiCtx.select)
        {
            // case 2:
            // {
            //     pnext = &gstEmWinListArmVersionInfo;
            //     break;
            // }
            case 3:
            {
                pnext = &gstEmWinListArmVersionInfo;
                break;
            }
            #if EM_REMOVE_FIVE_INFO
            case 4:
            {
                pnext = &gstEmWinListBatteryVersionInfo;
                break;
            }
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
                pnext = &gstEmWinMain3DNav;
            }
            pPrevEmWin = pCurrentEmWin;
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}


