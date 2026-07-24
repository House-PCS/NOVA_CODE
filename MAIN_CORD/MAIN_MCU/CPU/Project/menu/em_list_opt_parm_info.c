/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opt_parm_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-04
  * @brief       Setting < More < Opt parm
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-02-04    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"


#define  EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE                 (4096)
#define  EM_LIST_OPT_PARM_ICON_ARROW_UP_SIZE                    (4096)


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);

emWin_t gstEmWinListOptPARMInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t list_show;
    int32_t select;
    int32_t key_init;
    uint16_t method_idx;
} UiCtx_t;

typedef struct _method_idx_t
{
    uint16_t idx;
    const char method[16];
} MethodIdx_t;

static UiCtx_t _gstUiCtx;
static MethodIdx_t _garrMethodIdx[] =
{
    {0, "Seamless switch"},
    {1, "H-L penetration"},
    {2, "Anti-island" },
};

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static IMAGE_Handle _gImgArrowHandle;//最后一行需要变换的箭头句柄
static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetImgPara_t  _garrArrowImgPara[2];
static char _garrTitleBuf[] = "Settings < More < Opt PARM";

/*List控件*/
static emListPara_t  _gstEmListOptPARMPara;
static char _garrListOptPARMLeftTextBuf[13][40];
static char _garrListOptPARMMidTextBuf[2][20];


#define WIN_LIST_OPT_PARM_ID                               (GUI_ID_USER + 0x12)
#define WIN_LIST_OPT_PARM_TITLE_ID                         (GUI_ID_USER + 0x13)
#define WIN_LIST_OPT_PARM_ARROW_ID                         (GUI_ID_USER + 0x14)

static __SDRAM GUI_WIDGET_CREATE_INFO _garrListOptPARMDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListOptPARMWin",    WIN_LIST_OPT_PARM_ID,                    0,      0,      480,  320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_OPT_PARM_TITLE_ID,                 31,     20,     400,  14,   0,   0x0,  0 },
//    { IMAGE_CreateIndirect,   "Arrow",          WIN_LIST_OPT_PARM_ARROW_ID,                 414,    260,    15,   15,   0,   0x0,  0 },
};

/*函数声明*/
void SetGridDropSwitchArrowDirection(void);


static int32_t _SetGridDropSwitchMethod(int32_t ui_update, int32_t idx)
{
    snprintf(_garrListOptPARMMidTextBuf[0], sizeof(_garrListOptPARMMidTextBuf[0]), "%s", _garrMethodIdx[idx].method);

    if(_gstEmListOptPARMPara.list_page == 6)
    {
        if(ui_update)
        {
            TEXT_SetText(_gstEmListOptPARMPara.handle[2][5], (const char *)_gstEmListOptPARMPara.sub_widget_para[2][11].sub_widget.text_para.p_content);
        }
    }
    return 0;
}

/**
  * @brief ListOptPARMWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListOptPARMWin(WM_MESSAGE * pMsg)
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
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_OPT_PARM_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*右箭头*/
            _gImgArrowHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_OPT_PARM_ARROW_ID);
            IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListOptPARMPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListOptPARMPara, _gstEmListOptPARMPara.list_pre_sel);
            GUI_SetColor(0x00565656);//上色
            for(i = 0; i < _gstEmListOptPARMPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListOptPARMPara.sub_region[i].x_pos,
                                    _gstEmListOptPARMPara.sub_region[i].y_pos,
                                    _gstEmListOptPARMPara.sub_region[i].x_pos + _gstEmListOptPARMPara.sub_region[i].x_size - 1,
                                    _gstEmListOptPARMPara.sub_region[i].y_pos + _gstEmListOptPARMPara.sub_region[i].y_size - 1,
                                    _gstEmListOptPARMPara.selected_r);
            }
            SelectedDrawInWMPaint(&_gstEmListOptPARMPara, _gstEmListOptPARMPara.list_sel);
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
    pPrevEmWin = &gstEmWinListEquipOperaInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListOptPARMInfo.win_handle = GUI_CreateDialogBox(_garrListOptPARMDialogCreate, GUI_COUNTOF(_garrListOptPARMDialogCreate), _cbListOptPARMWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListOptPARMInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListOptPARMInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListOptPARMInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListOptPARMPara, gstEmWinListOptPARMInfo.win_handle);
    return gstEmWinListOptPARMInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListOptPARMInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListOptPARMInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListOptPARMInfo.win_handle = 0;
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

    /*标题*/
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;
    //右箭头
    _garrArrowImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE);
    _garrArrowImgPara[0].p_img_buf = GUI_ALLOC_h2p(_garrArrowImgPara[0].mem_img);
    _garrArrowImgPara[0].img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    SpiFlashReadData(EM_ICON_RIGHT_ARROW_ADDR, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);
//    //下箭头
//    _garrArrowImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_OPT_PARM_ICON_ARROW_UP_SIZE);
//    _garrArrowImgPara[1].p_img_buf = GUI_ALLOC_h2p(_garrArrowImgPara[1].mem_img);
//    _garrArrowImgPara[1].img_size = EM_LIST_OPT_PARM_ICON_ARROW_UP_SIZE;
//    SpiFlashReadData(EM_ICON_UP_ARROW_ADDR, _garrArrowImgPara[1].p_img_buf, _garrArrowImgPara[1].img_size);

    _gstEmListOptPARMPara.list_start_id = WIN_LIST_OPT_PARM_ID + 0x30;
    _gstEmListOptPARMPara.list_num = 4;//13;
    _gstEmListOptPARMPara.list_display_num = 6;
    if(_gstEmListOptPARMPara.list_num <= _gstEmListOptPARMPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListOptPARMPara.list_num + 1 - _gstEmListOptPARMPara.list_display_num;
    }

    if(_gstEmListOptPARMPara.list_sel > _gstEmListOptPARMPara.list_num - 1)
    {
        _gstEmListOptPARMPara.list_sel = 0;
    }
    if(_gstEmListOptPARMPara.list_page > max_page - 1)
    {
        _gstEmListOptPARMPara.list_page = 0;
    }

    /*List选中框*/
    /*6个子列表*/
    _gstEmListOptPARMPara.sub_region[0].x_pos = 20;
    _gstEmListOptPARMPara.sub_region[0].y_pos = 38;
    _gstEmListOptPARMPara.sub_region[0].x_size = 420;
    _gstEmListOptPARMPara.sub_region[0].y_size = 34;
    _gstEmListOptPARMPara.sub_region[1].x_pos = 20;
    _gstEmListOptPARMPara.sub_region[1].y_pos = 80;
    _gstEmListOptPARMPara.sub_region[1].x_size = 420;
    _gstEmListOptPARMPara.sub_region[1].y_size = 34;
    _gstEmListOptPARMPara.sub_region[2].x_pos = 20;
    _gstEmListOptPARMPara.sub_region[2].y_pos = 122;
    _gstEmListOptPARMPara.sub_region[2].x_size = 420;
    _gstEmListOptPARMPara.sub_region[2].y_size = 34;
    _gstEmListOptPARMPara.sub_region[3].x_pos = 20;
    _gstEmListOptPARMPara.sub_region[3].y_pos = 164;
    _gstEmListOptPARMPara.sub_region[3].x_size = 420;
    _gstEmListOptPARMPara.sub_region[3].y_size = 34;
//    _gstEmListOptPARMPara.sub_region[4].x_pos = 20;
//    _gstEmListOptPARMPara.sub_region[4].y_pos = 206;
//    _gstEmListOptPARMPara.sub_region[4].x_size = 420;
//    _gstEmListOptPARMPara.sub_region[4].y_size = 34;
//    _gstEmListOptPARMPara.sub_region[5].x_pos = 20;
//    _gstEmListOptPARMPara.sub_region[5].y_pos = 248;
//    _gstEmListOptPARMPara.sub_region[5].x_size = 420;
//    _gstEmListOptPARMPara.sub_region[5].y_size = 34;


    /*List控件*/
    //第一行左边
    _gstEmListOptPARMPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][0].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][0].x_size = 230;
    _gstEmListOptPARMPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[0];
    _gstEmListOptPARMPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Export Limitation");
    //第一行右边
    _gstEmListOptPARMPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][0].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][0].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][0].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][0].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][0].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;//_gListOptPARMRightMem;
    _gstEmListOptPARMPara.sub_widget_para[1][0].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;//_pListOptPARMRightBuf;
    _gstEmListOptPARMPara.sub_widget_para[1][0].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    //第二行左边
    _gstEmListOptPARMPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][1].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][1].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][1].x_size = 230;
    _gstEmListOptPARMPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[1];
    _gstEmListOptPARMPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Reactive Power Mode");
    //第二行右边
    _gstEmListOptPARMPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][1].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][1].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][1].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][1].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][1].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][1].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][1].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    
    //第3行左边
    _gstEmListOptPARMPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][2].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][2].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][2].x_size = 230;
    _gstEmListOptPARMPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[2];
    _gstEmListOptPARMPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Import Limitation");
    //第3行右边
    _gstEmListOptPARMPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][2].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][2].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][2].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][2].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][2].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][2].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][2].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    
    //第4行左边
    _gstEmListOptPARMPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][3].x_size = 300;
    _gstEmListOptPARMPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[3];
    _gstEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Reactive Response Time");
    //第4行右边
    _gstEmListOptPARMPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][3].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][3].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][3].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][3].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
#if 0    
    
    //第4行左边
    _gstEmListOptPARMPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][3].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][3].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][3].x_size = 330;
    _gstEmListOptPARMPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[3];
    _gstEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Grid Overvoltage Protection Point");
    //第4行右边
    _gstEmListOptPARMPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][3].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][3].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][3].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][3].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][3].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][3].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][3].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    //第5行左边
    _gstEmListOptPARMPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][4].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][4].x_size = 330;
    _gstEmListOptPARMPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[4];
    _gstEmListOptPARMPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Inverter Undervoltage Protection Point");
    //第5行右边
    _gstEmListOptPARMPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][4].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][4].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][4].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][4].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][4].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][4].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][4].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    //第6行左边
    _gstEmListOptPARMPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][5].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][5].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][5].x_size = 330;
    _gstEmListOptPARMPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[5];
    _gstEmListOptPARMPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Overfrequency Protection Point");
    //第6行右边
    _gstEmListOptPARMPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][5].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][5].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][5].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][5].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][5].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][5].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][5].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    //第7行左边
    _gstEmListOptPARMPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][6].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][6].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][6].x_size = 280;
    _gstEmListOptPARMPara.sub_widget_para[0][6].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[6];
    _gstEmListOptPARMPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Underfrequency Protection Point");
    //第7行右边
    _gstEmListOptPARMPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][6].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][6].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][6].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][6].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][6].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][6].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][6].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    //第8行左边
    _gstEmListOptPARMPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][7].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][7].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][7].x_size = 260;
    _gstEmListOptPARMPara.sub_widget_para[0][7].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[7];
    _gstEmListOptPARMPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Overvoltage Derating Point");
    //第8行右边
    _gstEmListOptPARMPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][7].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][7].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][7].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][7].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][7].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][7].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][7].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    //第9行左边
    _gstEmListOptPARMPara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][8].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][8].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][8].x_size = 300;
    _gstEmListOptPARMPara.sub_widget_para[0][8].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][8].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[8];
    _gstEmListOptPARMPara.sub_widget_para[0][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][8].sub_widget.text_para.p_content, "Undervoltage Derating Point");
    //第9行右边
    _gstEmListOptPARMPara.sub_widget_para[1][8].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][8].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][8].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][8].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][8].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][8].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][8].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][8].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    //第10行左边
    _gstEmListOptPARMPara.sub_widget_para[0][9].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][9].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][9].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][9].x_size = 300;
    _gstEmListOptPARMPara.sub_widget_para[0][9].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][9].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[9];
    _gstEmListOptPARMPara.sub_widget_para[0][9].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][9].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][9].sub_widget.text_para.p_content, "Overfrequency Derating Point");
    //第10行右边
    _gstEmListOptPARMPara.sub_widget_para[1][9].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][9].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][9].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][9].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][9].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][9].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][9].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][9].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    //第11行左边
    _gstEmListOptPARMPara.sub_widget_para[0][10].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][10].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][10].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][10].x_size = 300;
    _gstEmListOptPARMPara.sub_widget_para[0][10].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][10].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[10];
    _gstEmListOptPARMPara.sub_widget_para[0][10].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][10].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][10].sub_widget.text_para.p_content, "Underfrequency Derating Point");
    //第11行右边
    _gstEmListOptPARMPara.sub_widget_para[1][10].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][10].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][10].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][10].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][10].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][10].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][10].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][10].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;
    //第12行左边
    _gstEmListOptPARMPara.sub_widget_para[0][11].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][11].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][11].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][11].x_size = 300;
    _gstEmListOptPARMPara.sub_widget_para[0][11].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][11].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[11];
    _gstEmListOptPARMPara.sub_widget_para[0][11].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][11].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][11].sub_widget.text_para.p_content, "Reactive Response Time");
    //第12行右边
    _gstEmListOptPARMPara.sub_widget_para[1][11].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][11].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][11].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][11].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][11].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][11].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][11].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][11].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;    
#endif

#if 0   
   //第13行左边
    _gstEmListOptPARMPara.sub_widget_para[0][12].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[0][12].x_relative_pos = 11;
    _gstEmListOptPARMPara.sub_widget_para[0][12].y_relative_pos = 10;
    _gstEmListOptPARMPara.sub_widget_para[0][12].x_size = 300;
    _gstEmListOptPARMPara.sub_widget_para[0][12].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[0][12].sub_widget.text_para.p_content = _garrListOptPARMLeftTextBuf[12];
    _gstEmListOptPARMPara.sub_widget_para[0][12].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[0][12].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListOptPARMPara.sub_widget_para[0][12].sub_widget.text_para.p_content, "Grid-Off Switching");

    //第十二行右边
    _gstEmListOptPARMPara.sub_widget_para[1][12].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListOptPARMPara.sub_widget_para[1][12].x_relative_pos = 394;
    _gstEmListOptPARMPara.sub_widget_para[1][12].y_relative_pos = 12;
    _gstEmListOptPARMPara.sub_widget_para[1][12].x_size = 10;
    _gstEmListOptPARMPara.sub_widget_para[1][12].y_size = 5;
    _gstEmListOptPARMPara.sub_widget_para[1][12].sub_widget.img_para.mem_img = _garrArrowImgPara[0].mem_img;
    _gstEmListOptPARMPara.sub_widget_para[1][12].sub_widget.img_para.p_img_buf = _garrArrowImgPara[0].p_img_buf;
    _gstEmListOptPARMPara.sub_widget_para[1][12].sub_widget.img_para.img_size = EM_LIST_OPT_PARM_ICON_ARROW_RIGHT_SIZE;

    //第13行中间
    _gstEmListOptPARMPara.sub_widget_para[2][12].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListOptPARMPara.sub_widget_para[2][12].x_relative_pos = 226;
    _gstEmListOptPARMPara.sub_widget_para[2][12].y_relative_pos = 8;
    _gstEmListOptPARMPara.sub_widget_para[2][12].x_size = 160;
    _gstEmListOptPARMPara.sub_widget_para[2][12].y_size = 17;
    _gstEmListOptPARMPara.sub_widget_para[2][12].sub_widget.text_para.p_content = _garrListOptPARMMidTextBuf[0];
    _gstEmListOptPARMPara.sub_widget_para[2][12].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListOptPARMPara.sub_widget_para[2][12].sub_widget.text_para.p_text_font = EM_TEXT_FONT_7;
//    strcpy(_gstEmListOptPARMPara.sub_widget_para[2][12].sub_widget.text_para.p_content, "Seamless switch");
    snprintf(_garrListOptPARMMidTextBuf[0], sizeof(_garrListOptPARMMidTextBuf[0]), "%s", _garrMethodIdx[_gstUiCtx.method_idx].method);
#endif
    /*List选中框*/
    _gstEmListOptPARMPara.selected_unvalid_color = 0xFF000000;
    _gstEmListOptPARMPara.selected_valid_color = 0x00D6E400;
    _gstEmListOptPARMPara.selected_size = 3;
    _gstEmListOptPARMPara.selected_r = 10;
    _gstEmListOptPARMPara.selected_x_start_offset = -2;
    _gstEmListOptPARMPara.selected_y_start_offset = -2;
    _gstEmListOptPARMPara.selected_x_end_offset = 2;
    _gstEmListOptPARMPara.selected_y_end_offset = 2;
//    _gstEmListOptPARMPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    _gstEmListOptPARMPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

#if 0
    /*滚动条*/
    _gstEmListOptPARMPara.scrollbar_enable = true;
    _gstEmListOptPARMPara.scrollbar_color = 0x00D6E400;
    _gstEmListOptPARMPara.scrollbar_bk_color = 0x00565656;
    _gstEmListOptPARMPara.scrollbar_x_pos = 454;
    _gstEmListOptPARMPara.scrollbar_y_pos = 39;
    _gstEmListOptPARMPara.scrollbar_x_size = 6;
    _gstEmListOptPARMPara.scrollbar_y_size = 241;
#endif
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_garrArrowImgPara[0].mem_img);
//    GUI_ALLOC_Free(_garrArrowImgPara[1].mem_img);
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
    if(gstEmWinListOptPARMInfo.win_handle == 0)
    {
        return;
    }

#if 0
    if(_gstUiCtx.method_idx != gstCollectParaData)
    {
        _gstUiCtx.method_idx != gstCollectParaData;
        _SetGridDropSwitchMethod(1, _gstUiCtx.method_idx);
    }
#endif

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gstEmListOptPARMPara.list_sel + _gstEmListOptPARMPara.list_page >= _gstEmListOptPARMPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListOptPARMPara.list_sel + _gstEmListOptPARMPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListOptPARMPara, gstEmWinListOptPARMInfo.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListOptPARMPara, gstEmWinListOptPARMInfo.win_handle);
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
    if(_gstUiCtx.list_show)
    {
        gstEmWinListAdvaSettingOptGridDropSwitch.key_handler(p_msg);

        if(!_gstUiCtx.list_show)
        {
            gstEmWinListAdvaSettingOptGridDropSwitch.delete_win();
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
        EmListUpward(&_gstEmListOptPARMPara, gstEmWinListOptPARMInfo.win_handle);

//        if (_gstEmListOptPARMPara.list_num - _gstUiCtx.select - 1
//                == _gstEmListOptPARMPara.list_display_num)
//        {
//            WM_Paint(gstEmWinListOptPARMInfo.win_handle);
//        }
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListOptPARMPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListOptPARMPara, gstEmWinListOptPARMInfo.win_handle);

//        if (_gstUiCtx.select == _gstEmListOptPARMPara.list_num - 1)
//        {
//            WM_Paint(gstEmWinListOptPARMInfo.win_handle);
//        }
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        emWin_t *pnext = NULL;
        switch(_gstUiCtx.select)
        {
            case 0:
                pnext = &gstEmWinListAdvanSettingOptGridPower;
                break;
            case 1:
                pnext = &gstEmWinListAdvanSettingOptReactMode;
                break;
            case 2:
                pnext = &gstEmWinListAdvanSettingHouseholdGridPower;//gstEmWinListAdvanSettingHouseholdLimitEnable;
                break;
            case 3:
                 pnext = &gstEmWinListAdvanSettingOptReactvRespTim;
                break;
            #if 0
            case 3:
                pnext = &gstEmWinListAdvanSettingOptGridOVP;
                break;
            case 4:
               pnext = &gstEmWinListAdvanSettingOptInverterUVP;
                break;
            case 5:
                pnext = &gstEmWinListAdvanSettingOptOFPPoint;
                break;
            case 6:
                pnext = &gstEmWinListAdvanSettingOptUFPPoint;
                break;
            case 7:
                pnext = &gstEmWinListAdvanSettingOptOVDPoint;
                break;
            case 8:
                pnext = &gstEmWinListAdvanSettingOptUVDPoint;
                break;
            case 9:
                pnext = &gstEmWinListAdvanSettingOptOFDPoint;
                break;
            case 10:
                pnext = &gstEmWinListAdvanSettingOptUFDPoint;
                break;
            case 11:
                pnext = &gstEmWinListAdvanSettingOptReactvRespTim;
                break;
            case 12:
            {
                if(!_gstUiCtx.list_show)
                {
                    gstEmWinListAdvaSettingOptGridDropSwitch.create_win();
                    _gstUiCtx.list_show = 1;
                }
                SetGridDropSwitchArrowDirection();
            }
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
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pnext;
            pCurrentEmWin->create_win();
        }
        return;
    }
}
/**
  * @brief  设置箭头朝向
  * @param  NULL
  * @retval NULL
  */
void SetGridDropSwitchArrowDirection(void)
{
    if(_gstUiCtx.list_show)
    {
        //箭头向上
        IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[1].p_img_buf, _garrArrowImgPara[1].img_size);
    }
    else
    {
        //箭头向右
        IMAGE_SetDTA(_gImgArrowHandle, _garrArrowImgPara[0].p_img_buf, _garrArrowImgPara[0].img_size);
    }
}

void SetGridDropSwitchListShow(int32_t show)
{
    _gstUiCtx.list_show = show;
}

int32_t SetCurGridDropSwitchMethod(int32_t index)
{
    snprintf(_gstEmListOptPARMPara.sub_widget_para[2][12].sub_widget.text_para.p_content,
             sizeof(_garrListOptPARMMidTextBuf[0]), "%s", _garrMethodIdx[index].method);

    TEXT_SetText(_gstEmListOptPARMPara.handle[2][5], (const char *)_gstEmListOptPARMPara.sub_widget_para[2][12].sub_widget.text_para.p_content);
    return 0;
}