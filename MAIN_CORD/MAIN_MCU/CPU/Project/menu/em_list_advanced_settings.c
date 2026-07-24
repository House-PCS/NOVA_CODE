/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-15
  * @brief       高级设置
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-15    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"


#define  EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE                      (4096)
#define  EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE                  (4096)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinListAdvancedSettings = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    uint16_t grid_stand;
} UiCtx_t;

static UiCtx_t _gstUiCtx;
FlagIdx_t gstFlagIndex = {2};
uint16_t gSwitchIconStates = 0;
uint32_t gWaitTime = 0;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetImgPara_t  _gNationalFlagImgPara;

static char _garrTitleBuf[] = "Settings < More";


/*List控件*/
static emListPara_t  _gEmListAdvacnedSettingsPara;

static char _gListAdvacnedSettingsLeftTextBuf[11][22];
static char _gListAdvacnedSettingsRightTextBuf[2][20];

static GUI_HMEM _gListAdvancedSettingsRightMem;
static uint8_t *_pListAdvancedSettingsRightBuf;



#define WIN_LIST_ADVANCED_SETTINGS_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_ADVANCED_SETTINGS_TITLE_ID                         (GUI_ID_USER + 0x13)


static __SDRAM GUI_WIDGET_CREATE_INFO _aListGeneratorInputDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListAdvacnedSettingsWin",    WIN_LIST_ADVANCED_SETTINGS_ID,          0,   0,   480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_ADVANCED_SETTINGS_TITLE_ID,                31,  20,  400,  14,   0,   0x0,  0 },
};

static int32_t _SetMoreSettingFlag(int32_t index)
{
    if(index < EN_NATIONALLIST_CN || index >= EN_NATIONALLIST_MAX)
    {
        return -1;
    }

    if(_gEmListAdvacnedSettingsPara.list_page == 0)
    {
        snprintf(_gListAdvacnedSettingsRightTextBuf[0], sizeof(_gListAdvacnedSettingsRightTextBuf[0]), "%s", _garrNationalIdx[index].name);
        SpiFlashReadData(_garrNationalIdx[index].img_addr, _gNationalFlagImgPara.p_img_buf, _gNationalFlagImgPara.img_size);
        TEXT_SetText(_gEmListAdvacnedSettingsPara.handle[2][2],
                     (const char *)_gEmListAdvacnedSettingsPara.sub_widget_para[2][2].sub_widget.text_para.p_content);
        IMAGE_SetDTA(_gEmListAdvacnedSettingsPara.handle[1][2], _gNationalFlagImgPara.p_img_buf, _gNationalFlagImgPara.img_size);
    }

    return 0;
}



/**
  * @brief ListAdvacnedSettingsWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListAdvacnedSettingsWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin, hWinOld, hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_ADVANCED_SETTINGS_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gEmListAdvacnedSettingsPara, pMsg->hWin);

//            _SetMoreSettingFlag(_gstUiCtx.grid_stand);
//            _SetMoreSettingFlag(gstFlagIndex.flag_idx);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gEmListAdvacnedSettingsPara, _gEmListAdvacnedSettingsPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gEmListAdvacnedSettingsPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gEmListAdvacnedSettingsPara.sub_region[i].x_pos,
                                    _gEmListAdvacnedSettingsPara.sub_region[i].y_pos,
                                    _gEmListAdvacnedSettingsPara.sub_region[i].x_pos + _gEmListAdvacnedSettingsPara.sub_region[i].x_size - 1,
                                    _gEmListAdvacnedSettingsPara.sub_region[i].y_pos + _gEmListAdvacnedSettingsPara.sub_region[i].y_size - 1,
                                    _gEmListAdvacnedSettingsPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gEmListAdvacnedSettingsPara, _gEmListAdvacnedSettingsPara.list_sel);
#if 0
            int i = 0;
            for(i = 0; i < _gEmListAdvacnedSettingsPara.list_display_num; i++)
            {
                if (i == _gEmListAdvacnedSettingsPara.list_display_num - 1
                        && (_gEmListAdvacnedSettingsPara.list_page
                            == _gEmListAdvacnedSettingsPara.list_num
                            - _gEmListAdvacnedSettingsPara.list_display_num))
                {
                    if (_gstUiCtx.select == _gEmListAdvacnedSettingsPara.list_num - 1)
                    {
                        GUI_SetColor(0x00D6E400);//上色
                    }
                    GUI_FillRoundedRect(_gEmListAdvacnedSettingsPara.sub_region[i].x_pos + 250,
                                        _gEmListAdvacnedSettingsPara.sub_region[i].y_pos,
                                        _gEmListAdvacnedSettingsPara.sub_region[i].x_pos + _gEmListAdvacnedSettingsPara.sub_region[i].x_size - 1,
                                        _gEmListAdvacnedSettingsPara.sub_region[i].y_pos + _gEmListAdvacnedSettingsPara.sub_region[i].y_size - 1,
                                        _gEmListAdvacnedSettingsPara.selected_r);

                    continue;
                }
                GUI_FillRoundedRect(_gEmListAdvacnedSettingsPara.sub_region[i].x_pos,
                                    _gEmListAdvacnedSettingsPara.sub_region[i].y_pos,
                                    _gEmListAdvacnedSettingsPara.sub_region[i].x_pos + _gEmListAdvacnedSettingsPara.sub_region[i].x_size - 1,
                                    _gEmListAdvacnedSettingsPara.sub_region[i].y_pos + _gEmListAdvacnedSettingsPara.sub_region[i].y_size - 1,
                                    _gEmListAdvacnedSettingsPara.selected_r);
            }

            if (_gstUiCtx.select == _gEmListAdvacnedSettingsPara.list_num - 1)
            {

                /*
                uint16_t x = _gEmListAdvacnedSettingsPara.sub_region[11].x_pos;
                EN_LIST_SELECTED_TYPE type =  _gEmListAdvacnedSettingsPara.selected_type;
                _gEmListAdvacnedSettingsPara.selected_type == EN_LIST_SELECTED_TYPE_FILLED;
                _gEmListAdvacnedSettingsPara.sub_region[11].x_pos += 180;
                SelectedDrawInWMPaint(&_gEmListAdvacnedSettingsPara,
                                      _gEmListAdvacnedSettingsPara.list_sel);
                _gEmListAdvacnedSettingsPara.selected_type = type;
                _gEmListAdvacnedSettingsPara.sub_region[11].x_pos = x;
                */
            }
            else
            {

                SelectedDrawInWMPaint(&_gEmListAdvacnedSettingsPara,
                                      _gEmListAdvacnedSettingsPara.list_sel);
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
    pPrevEmWin = &gstEmWinListSettingsInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListAdvancedSettings.win_handle = GUI_CreateDialogBox(_aListGeneratorInputDialogCreate, GUI_COUNTOF(_aListGeneratorInputDialogCreate), _cbListAdvacnedSettingsWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListAdvancedSettings.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListAdvancedSettings.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListAdvancedSettings.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gEmListAdvacnedSettingsPara, gstEmWinListAdvancedSettings.win_handle);
    return gstEmWinListAdvancedSettings.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListAdvancedSettings.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListAdvancedSettings.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListAdvancedSettings.win_handle = 0;
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
    
//    _gstUiCtx.grid_stand = gstCollectInfo.power_data.power_set_info.grid_standard;
    if(!gSwitchIconStates)
    {
        gstFlagIndex.flag_idx = gstCollectInfo.power_data.power_set_info.grid_standard;
    }
    

    /*国旗图标的参数*/
    _gNationalFlagImgPara.mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE);
    _gNationalFlagImgPara.p_img_buf = GUI_ALLOC_h2p(_gNationalFlagImgPara.mem_img);
    _gNationalFlagImgPara.img_size = EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE;
//    SpiFlashReadData(_garrNationalIdx[_gstUiCtx.grid_stand].img_addr, _gNationalFlagImgPara.p_img_buf, _gNationalFlagImgPara.img_size);
    SpiFlashReadData(_garrNationalIdx[gstFlagIndex.flag_idx].img_addr, _gNationalFlagImgPara.p_img_buf, _gNationalFlagImgPara.img_size);
    
    /*标题*/
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;


    _gListAdvancedSettingsRightMem = GUI_ALLOC_AllocZero(EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE);
    _pListAdvancedSettingsRightBuf = GUI_ALLOC_h2p(_gListAdvancedSettingsRightMem);
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _pListAdvancedSettingsRightBuf, EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE);

    _gEmListAdvacnedSettingsPara.list_start_id = WIN_LIST_ADVANCED_SETTINGS_ID + 0x30;
    _gEmListAdvacnedSettingsPara.list_num = 6;//8;//10;  //11;  //12;
    _gEmListAdvacnedSettingsPara.list_display_num = 6;
    if(_gEmListAdvacnedSettingsPara.list_num <= _gEmListAdvacnedSettingsPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gEmListAdvacnedSettingsPara.list_num + 1 - _gEmListAdvacnedSettingsPara.list_display_num;
    }

    if(_gEmListAdvacnedSettingsPara.list_sel > _gEmListAdvacnedSettingsPara.list_num - 1)
    {
        _gEmListAdvacnedSettingsPara.list_sel = 0;
    }
    if(_gEmListAdvacnedSettingsPara.list_page > max_page - 1)
    {
        _gEmListAdvacnedSettingsPara.list_page = 0;
    }

    /*List选中框*/
    /*4个子列表*/
    _gEmListAdvacnedSettingsPara.sub_region[0].x_pos = 20;
    _gEmListAdvacnedSettingsPara.sub_region[0].y_pos = 38;
    _gEmListAdvacnedSettingsPara.sub_region[0].x_size = 420;
    _gEmListAdvacnedSettingsPara.sub_region[0].y_size = 34;
    _gEmListAdvacnedSettingsPara.sub_region[1].x_pos = 20;
    _gEmListAdvacnedSettingsPara.sub_region[1].y_pos = 80;
    _gEmListAdvacnedSettingsPara.sub_region[1].x_size = 420;
    _gEmListAdvacnedSettingsPara.sub_region[1].y_size = 34;
    _gEmListAdvacnedSettingsPara.sub_region[2].x_pos = 20;
    _gEmListAdvacnedSettingsPara.sub_region[2].y_pos = 122;
    _gEmListAdvacnedSettingsPara.sub_region[2].x_size = 420;
    _gEmListAdvacnedSettingsPara.sub_region[2].y_size = 34;
    _gEmListAdvacnedSettingsPara.sub_region[3].x_pos = 20;
    _gEmListAdvacnedSettingsPara.sub_region[3].y_pos = 164;
    _gEmListAdvacnedSettingsPara.sub_region[3].x_size = 420;
    _gEmListAdvacnedSettingsPara.sub_region[3].y_size = 34;
    _gEmListAdvacnedSettingsPara.sub_region[4].x_pos = 20;
    _gEmListAdvacnedSettingsPara.sub_region[4].y_pos = 206;
    _gEmListAdvacnedSettingsPara.sub_region[4].x_size = 420;
    _gEmListAdvacnedSettingsPara.sub_region[4].y_size = 34;
    _gEmListAdvacnedSettingsPara.sub_region[5].x_pos = 20;
    _gEmListAdvacnedSettingsPara.sub_region[5].y_pos = 248;
    _gEmListAdvacnedSettingsPara.sub_region[5].x_size = 420;
    _gEmListAdvacnedSettingsPara.sub_region[5].y_size = 34;


    /*List控件*/
    //第一行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][0].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][0].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[0];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "System");
    //第一行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][0].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][0].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][0].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
    //第二行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][1].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][1].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[1];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Battery");
    //第二行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][1].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][1].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][1].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
    //第三行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][2].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][2].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[2];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Grid Standard");
    //第三行中间图标
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][2].x_relative_pos = 258;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][2].y_relative_pos = 3;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][2].x_size = 30;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][2].y_size = 30;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _gNationalFlagImgPara.mem_img;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _gNationalFlagImgPara.p_img_buf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_NATIONAL_FLAG_ICON_ARROW_SIZE;
    //第三行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[2][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[2][2].x_relative_pos = 191;
    _gEmListAdvacnedSettingsPara.sub_widget_para[2][2].y_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[2][2].x_size = 208;
    _gEmListAdvacnedSettingsPara.sub_widget_para[2][2].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[2][2].sub_widget.text_para.p_content = _gListAdvacnedSettingsRightTextBuf[0];
    _gEmListAdvacnedSettingsPara.sub_widget_para[2][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[2][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
//    snprintf(_gListAdvacnedSettingsRightTextBuf[0], sizeof( _gListAdvacnedSettingsRightTextBuf[0]), "%s", 
//             _garrNationalIdx[_gstUiCtx.grid_stand].name);
    snprintf(_gListAdvacnedSettingsRightTextBuf[0], sizeof( _gListAdvacnedSettingsRightTextBuf[0]), "%s", 
             _garrNationalIdx[gstFlagIndex.flag_idx].name);
    //第四行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][3].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][3].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[3];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Extend Meter Brand");
    //第四行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][3].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][3].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][3].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][3].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
    //第5行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[4];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Operational");
    //第5行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
#if 0    
    //第五行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[4];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Time");
    //第五行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
  
    //第六行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[5];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Language");
    //第六行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
    //第七行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][6].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][6].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][6].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][6].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[6];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Communication");
    //第七行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][6].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][6].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][6].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][6].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
#endif 
    //第八行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[5];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "LCD Backlight Time");
   
    //第八行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
#if 0    
    //第九行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][7].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][7].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][7].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][7].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[7];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Clear Fault History");
    //第九行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][7].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][7].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][7].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][7].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
#if 0
    //第十行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[9];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][9].sub_widget.text_para.p_content, "Factory Restory");
    //第十行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][9].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][9].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][9].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][9].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][9].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][9].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][9].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][9].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
#endif
    //第十一行左边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][8].x_relative_pos = 11;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][8].y_relative_pos = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][8].x_size = 230;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][8].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][8].sub_widget.text_para.p_content = _gListAdvacnedSettingsLeftTextBuf[8];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][8].sub_widget.text_para.p_content, "Set Password");
    //第十一行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][8].widget_type = EM_WIDGET_TYPE_IMG;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][8].x_relative_pos = 394;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][8].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][8].x_size = 10;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][8].y_size = 5;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][8].sub_widget.img_para.mem_img = _gListAdvancedSettingsRightMem;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][8].sub_widget.img_para.p_img_buf = _pListAdvancedSettingsRightBuf;
    _gEmListAdvacnedSettingsPara.sub_widget_para[1][8].sub_widget.img_para.img_size = EM_LIST_ADVANCED_SETTINGS_ICON_ARROW_SIZE;
    //第十二行右边
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].widget_type = EM_WIDGET_TYPE_TEXT;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].x_relative_pos = 269;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].y_relative_pos = 12;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].x_size = 132;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].y_size = 17;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].sub_widget.text_para.p_content = _gListAdvacnedSettingsRightTextBuf[1];
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gEmListAdvacnedSettingsPara.sub_widget_para[0][9].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gEmListAdvacnedSettingsPara.sub_widget_para[0][9].sub_widget.text_para.p_content, "Save As Default");
#endif
    /*List选中框*/
    _gEmListAdvacnedSettingsPara.selected_unvalid_color = 0xFF000000;
    _gEmListAdvacnedSettingsPara.selected_valid_color = 0x00D6E400;
    _gEmListAdvacnedSettingsPara.selected_size = 3;
    _gEmListAdvacnedSettingsPara.selected_r = 10;
    _gEmListAdvacnedSettingsPara.selected_x_start_offset = -2;
    _gEmListAdvacnedSettingsPara.selected_y_start_offset = -2;
    _gEmListAdvacnedSettingsPara.selected_x_end_offset = 2;
    _gEmListAdvacnedSettingsPara.selected_y_end_offset = 2;
//    _gEmListAdvacnedSettingsPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gEmListAdvacnedSettingsPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    /*滚动条*/
    _gEmListAdvacnedSettingsPara.scrollbar_enable = true;
    _gEmListAdvacnedSettingsPara.scrollbar_color = 0x00D6E400;
    _gEmListAdvacnedSettingsPara.scrollbar_bk_color = 0x00565656;
    _gEmListAdvacnedSettingsPara.scrollbar_x_pos = 454;
    _gEmListAdvacnedSettingsPara.scrollbar_y_pos = 39;
    _gEmListAdvacnedSettingsPara.scrollbar_x_size = 6;
    _gEmListAdvacnedSettingsPara.scrollbar_y_size = 241;

}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gNationalFlagImgPara.mem_img);
    GUI_ALLOC_Free(_gListAdvancedSettingsRightMem);
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
    if(gstEmWinListAdvancedSettings.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gEmListAdvacnedSettingsPara.list_sel + _gEmListAdvacnedSettingsPara.list_page >= _gEmListAdvacnedSettingsPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gEmListAdvacnedSettingsPara.list_sel + _gEmListAdvacnedSettingsPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gEmListAdvacnedSettingsPara, gstEmWinListAdvancedSettings.win_handle);
        }
        else
        {
            EmListUpward(&_gEmListAdvacnedSettingsPara, gstEmWinListAdvancedSettings.win_handle);
        }
    }
//    if(pop_up_stat == 0)
//    {
//        if(GetSystick() - pop_timecount > 1000)
//        {
//            pop_up_stat = 1;
//            pop_timecount = GetSystick();
//            gstPopUpWinError.create_win();
//        }
//    }
//    else
//    {
//        if(GetSystick() - pop_timecount > 1000)
//        {
//            pop_up_stat = 0;
//            pop_timecount = GetSystick();
//            gstPopUpWinError.delete_win();
//        }
//    }
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

        EmListUpward(&_gEmListAdvacnedSettingsPara, gstEmWinListAdvancedSettings.win_handle);

        if (_gEmListAdvacnedSettingsPara.list_num - _gstUiCtx.select - 1
                == _gEmListAdvacnedSettingsPara.list_display_num)
        {
            WM_Paint(gstEmWinListAdvancedSettings.win_handle);
        }
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gEmListAdvacnedSettingsPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }

        EmListDownward(&_gEmListAdvacnedSettingsPara, gstEmWinListAdvancedSettings.win_handle);

        if (_gstUiCtx.select == _gEmListAdvacnedSettingsPara.list_num - 1)
        {
            WM_Paint(gstEmWinListAdvancedSettings.win_handle);
        }
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        emWin_t *pnext = NULL;
        switch(_gstUiCtx.select)
        {
            case 0:
                pnext = &gstEmWinListAdvanceSystem;
                break;
            case 1:
                pnext = &gstEmWinListAdvancedBattery;
                break;
            case 2:
                pnext = &gstEmWinListGridStand;
                break;
            case 3:
                pnext = &gstEmWinListMeterBrandInfo;
                break;
            case 4:
                pnext = &gstEmWinListOptPARMInfo;
                break;
#if 0
            case 4:
                pnext = &gstEmWinListSetDataInfo;
                break;

            case 5:
                pnext = &gstEmWinListLanguage;
                break;
                        
            case 6:
                pnext = &gstEmWinListAdvanceCommunication;
                break;
#endif
            case 5:
                pnext = &gstEmWinListAdvanSettingOptLCDBKLTime;
                break;
#if 0
            case 7:
                pnext = &gstEmWinListDelHistoryFault;
                break;
//            case 9:
//                pnext = &gstEmWinListAdvanSettingFactoryRestore;
//                break;
            case 8:
                pnext = &gstEmWinListADVSetPassword;
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



