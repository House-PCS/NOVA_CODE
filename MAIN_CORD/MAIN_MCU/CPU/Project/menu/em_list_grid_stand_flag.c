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

#define  EM_LIST_NATION_INFO_ICON_ARROW_SIZE                (4096)

#define  EM_LIST_NATIONAL_FLAG_NUMS                         (6)

static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListGridStandFlag = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};


typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
} UiCtx_t;

static UiCtx_t _gstUiCtx;

/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static emWidgetTextPara_t  _gstTitleTextPara;

static char _garrTitleBuf[] = " ";


/*List控件*/
static emListPara_t  _gstEmListGridStandFlagPara;

static emWidgetTextPara_t  _gstGridStandTextPara;
static emWidgetImgPara_t   _gstNationalFlagImgPara[16];


static char _garrListNationalFlagInfoLeftTextBuf[16][20];
static char _garrListReserveFlagInfoLeftTextBuf[2][20];
static char _garrListNationalFlagInfoRightText3Buf[8];


#define WIN_LIST_GRID_STAND_FLAG_ID                              (GUI_ID_USER + 0x10)
#define WIN_LIST_GRID_STAND_FLAG_TITLE_ID                        (GUI_ID_USER + 0x11)


static __SDRAM GUI_WIDGET_CREATE_INFO _garrListNationalFlagDialogCreate[] =
{
    { WINDOW_CreateIndirect,  " ",               WIN_LIST_GRID_STAND_FLAG_ID,                       280,    81,     180,    220,    0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",           WIN_LIST_GRID_STAND_FLAG_TITLE_ID,                 280,    81,     15,     15,     0,   0x0,  0 },
};

/******函数声明******/
extern void SetGridListShow(int32_t show);
extern int32_t SetCurGridFlag(int32_t idx);
extern void SetGridStandArrowDirection(void);

/**
  * @brief ListNationalFlagInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListNationalFlagInfoWin(WM_MESSAGE * pMsg)
{
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_GRID_STAND_FLAG_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListGridStandFlagPara, pMsg->hWin);
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListGridStandFlagPara, _gstEmListGridStandFlagPara.list_pre_sel);

            GUI_RECT Rect;
            WM_GetClientRect(&Rect);
            GUI_SetColor(0x00424242);
            GUI_FillRoundedRect(Rect.x0, Rect.y0, Rect.x1, Rect.y1, 10);
            GUI_SetColor(0x00BDC909);
            GUI_DrawRoundedFrame(Rect.x0 + 1, Rect.y0 + 1, Rect.x1 - 1, Rect.y1 - 1, 10, 3);

#if 0
            GUI_SetColor(0x00565656);//上色

            for(int i = 0; i < _gstEmListGridStandFlagPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListGridStandFlagPara.sub_region[i].x_pos,
                                    _gstEmListGridStandFlagPara.sub_region[i].y_pos,
                                    _gstEmListGridStandFlagPara.sub_region[i].x_pos + _gstEmListGridStandFlagPara.sub_region[i].x_size - 1,
                                    _gstEmListGridStandFlagPara.sub_region[i].y_pos + _gstEmListGridStandFlagPara.sub_region[i].y_size - 1,
                                    _gstEmListGridStandFlagPara.selected_r);
            }
#endif   /*此段用于调节位置时使用*/

            SelectedDrawInWMPaint(&_gstEmListGridStandFlagPara, _gstEmListGridStandFlagPara.list_sel);
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
    _Init();
    GUI_EnableAlpha(1);
#if 1
    _gstEmListGridStandFlagPara.list_x_offset = _garrListNationalFlagDialogCreate[0].x0;
    _gstEmListGridStandFlagPara.list_y_offset = _garrListNationalFlagDialogCreate[0].y0;
#endif
    gstEmWinListGridStandFlag.win_handle = GUI_CreateDialogBox(_garrListNationalFlagDialogCreate, GUI_COUNTOF(_garrListNationalFlagDialogCreate), _cbListNationalFlagInfoWin,
                                           pCurrentEmWin->win_handle, 0, 0);
    //WINDOW_SetBkColor(gstEmWinListGridStandFlag.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListGridStandFlag.win_handle, 0x00000000);
    WM_SetTransState(gstEmWinListGridStandFlag.win_handle, WM_CF_HASTRANS);
    SelectedDraw(&_gstEmListGridStandFlagPara, gstEmWinListGridStandFlag.win_handle);
    GUI_Exec();
    return gstEmWinListGridStandFlag.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListGridStandFlag.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListGridStandFlag.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListGridStandFlag.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t max_page;
    uint8_t i = 0;
    EmKeyDefaultInfoSet();

    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00FFFFFF;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_1;

    _gstNationalFlagImgPara[0].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
    _gstNationalFlagImgPara[0].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[0].mem_img);
    _gstNationalFlagImgPara[0].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_CHINESE_FLAG_ADDR, _gstNationalFlagImgPara[0].p_img_buf, _gstNationalFlagImgPara[0].img_size);

    _gstNationalFlagImgPara[1].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
    _gstNationalFlagImgPara[1].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[1].mem_img);
    _gstNationalFlagImgPara[1].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_GERMAN_FLAG_ADDR, _gstNationalFlagImgPara[1].p_img_buf, _gstNationalFlagImgPara[1].img_size);


    for(i = 2; i < 5; i++)
    {
        _gstNationalFlagImgPara[i].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
        _gstNationalFlagImgPara[i].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[i].mem_img);
        _gstNationalFlagImgPara[i].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
        SpiFlashReadData(EM_ICON_AUSTRALIA_FLAG_ADDR + (i - 2) * 4096, _gstNationalFlagImgPara[i].p_img_buf, _gstNationalFlagImgPara[i].img_size);
    }

    _gstNationalFlagImgPara[5].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
    _gstNationalFlagImgPara[5].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[5].mem_img);
    _gstNationalFlagImgPara[5].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_ENGLAND_FLAG_ADDR, _gstNationalFlagImgPara[5].p_img_buf, _gstNationalFlagImgPara[5].img_size);

    
    _gstNationalFlagImgPara[6].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
    _gstNationalFlagImgPara[6].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[6].mem_img);
    _gstNationalFlagImgPara[6].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_HUNGARY_FLAG_ADDR, _gstNationalFlagImgPara[6].p_img_buf, _gstNationalFlagImgPara[6].img_size);
    
    _gstNationalFlagImgPara[7].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
    _gstNationalFlagImgPara[7].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[7].mem_img);
    _gstNationalFlagImgPara[7].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_BELGIUM_FLAG_ADDR, _gstNationalFlagImgPara[7].p_img_buf, _gstNationalFlagImgPara[7].img_size);
    
    _gstNationalFlagImgPara[8].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
    _gstNationalFlagImgPara[8].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[8].mem_img);
    _gstNationalFlagImgPara[8].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_GREECE_FLAG_ADDR, _gstNationalFlagImgPara[8].p_img_buf, _gstNationalFlagImgPara[8].img_size);
    
    _gstNationalFlagImgPara[9].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
    _gstNationalFlagImgPara[9].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[9].mem_img);
    _gstNationalFlagImgPara[9].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_FRANCE_FLAG_ADDR, _gstNationalFlagImgPara[9].p_img_buf, _gstNationalFlagImgPara[9].img_size);
    
    _gstNationalFlagImgPara[10].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
    _gstNationalFlagImgPara[10].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[10].mem_img);
    _gstNationalFlagImgPara[10].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_THAILAND_FLAG_ADDR, _gstNationalFlagImgPara[10].p_img_buf, _gstNationalFlagImgPara[10].img_size);
    
    for(i = 11; i < 13; i++)
    {
//        _gstNationalFlagImgPara[i].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
//        _gstNationalFlagImgPara[i].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[i].mem_img);
//        _gstNationalFlagImgPara[i].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
//        SpiFlashReadData(EM_ICON_RESERVE_FLAG_ADDR, _gstNationalFlagImgPara[i].p_img_buf, _gstNationalFlagImgPara[i].img_size);
    }  
    
    _gstNationalFlagImgPara[13].mem_img = GUI_ALLOC_AllocZero(EM_LIST_NATION_INFO_ICON_ARROW_SIZE);
    _gstNationalFlagImgPara[13].p_img_buf = GUI_ALLOC_h2p(_gstNationalFlagImgPara[13].mem_img);
    _gstNationalFlagImgPara[13].img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    SpiFlashReadData(EM_ICON_NEW_ZEALAND_FLAG_ADDR, _gstNationalFlagImgPara[13].p_img_buf, _gstNationalFlagImgPara[13].img_size);    

    _gstEmListGridStandFlagPara.list_start_id = WIN_LIST_GRID_STAND_FLAG_ID + 0x30;
    _gstEmListGridStandFlagPara.list_num = 16;
    _gstEmListGridStandFlagPara.list_display_num = 6;
    if(_gstEmListGridStandFlagPara.list_num <= _gstEmListGridStandFlagPara.list_display_num)
    {
        max_page = 1;
    }
    else
    {
        max_page = _gstEmListGridStandFlagPara.list_num + 1 - _gstEmListGridStandFlagPara.list_display_num;
    }

    if(_gstEmListGridStandFlagPara.list_sel > _gstEmListGridStandFlagPara.list_num - 1)
    {
        _gstEmListGridStandFlagPara.list_sel = 0;
    }
    if(_gstEmListGridStandFlagPara.list_page > max_page - 1)
    {
        _gstEmListGridStandFlagPara.list_page = 0;
    }


    //_gstEmListGridStandFlagPara.list_bk_color = 0x00787878;
    /*6个子列表*/
    _gstEmListGridStandFlagPara.sub_region[0].x_pos = 6;
    _gstEmListGridStandFlagPara.sub_region[0].y_pos = 10;
    _gstEmListGridStandFlagPara.sub_region[0].x_size = 168;
    _gstEmListGridStandFlagPara.sub_region[0].y_size = 30;
    _gstEmListGridStandFlagPara.sub_region[1].x_pos = 6;
    _gstEmListGridStandFlagPara.sub_region[1].y_pos = 44;
    _gstEmListGridStandFlagPara.sub_region[1].x_size = 168;
    _gstEmListGridStandFlagPara.sub_region[1].y_size = 30;
    _gstEmListGridStandFlagPara.sub_region[2].x_pos = 6;
    _gstEmListGridStandFlagPara.sub_region[2].y_pos = 77;
    _gstEmListGridStandFlagPara.sub_region[2].x_size = 168;
    _gstEmListGridStandFlagPara.sub_region[2].y_size = 30;
    _gstEmListGridStandFlagPara.sub_region[3].x_pos = 6;
    _gstEmListGridStandFlagPara.sub_region[3].y_pos = 110;
    _gstEmListGridStandFlagPara.sub_region[3].x_size = 168;
    _gstEmListGridStandFlagPara.sub_region[3].y_size = 30;
    _gstEmListGridStandFlagPara.sub_region[4].x_pos = 6;
    _gstEmListGridStandFlagPara.sub_region[4].y_pos = 145;
    _gstEmListGridStandFlagPara.sub_region[4].x_size = 168;
    _gstEmListGridStandFlagPara.sub_region[4].y_size = 30;
    _gstEmListGridStandFlagPara.sub_region[5].x_pos = 6;
    _gstEmListGridStandFlagPara.sub_region[5].y_pos = 179;
    _gstEmListGridStandFlagPara.sub_region[5].x_size = 168;
    _gstEmListGridStandFlagPara.sub_region[5].y_size = 30;
    /*List控件*/

    //第1行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][0].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][0].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][0].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][0].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][0].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[0].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][0].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[0].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][0].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第1行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][0].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][0].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][0].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][0].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][0].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[0];
    _gstEmListGridStandFlagPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][0].sub_widget.text_para.p_content, "China");

    //第2行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][1].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][1].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][1].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][1].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][1].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[1].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][1].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[1].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][1].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第2行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][1].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][1].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][1].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][1].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][1].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[1];
    _gstEmListGridStandFlagPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][1].sub_widget.text_para.p_content, "Germany");

    //第3行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][2].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][2].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][2].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][2].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][2].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[2].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][2].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[2].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][2].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第3行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][2].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][2].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][2].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][2].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][2].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[2];
    _gstEmListGridStandFlagPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][2].sub_widget.text_para.p_content, "Australia A");

    //第4行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][3].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][3].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][3].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][3].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][3].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[2].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][3].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[2].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][3].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第4行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][3].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][3].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][3].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][3].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][3].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[3];
    _gstEmListGridStandFlagPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][3].sub_widget.text_para.p_content, "Australia B");

    //第5行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][4].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][4].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][4].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][4].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][4].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[2].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][4].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[2].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][4].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第5行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][4].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][4].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][4].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][4].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][4].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[4];
    _gstEmListGridStandFlagPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][4].sub_widget.text_para.p_content, "Australia C");

    //第6行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][5].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][5].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][5].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][5].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][5].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[13].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][5].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[13].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][5].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第6行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][5].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][5].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][5].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][5].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][5].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[5];
    _gstEmListGridStandFlagPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][5].sub_widget.text_para.p_content, "New Zealand");

    //第7行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][6].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][6].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][6].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][6].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][6].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[3].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][6].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[3].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][6].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第7行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][6].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][6].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][6].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][6].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][6].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][6].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[6];
    _gstEmListGridStandFlagPara.sub_widget_para[1][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][6].sub_widget.text_para.p_content, "Italy");

    //第8行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][7].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][7].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][7].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][7].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][7].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[4].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][7].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[4].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][7].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第8行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][7].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][7].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][7].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][7].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][7].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][7].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[7];
    _gstEmListGridStandFlagPara.sub_widget_para[1][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][7].sub_widget.text_para.p_content, "Spain");

    //第9行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][8].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][8].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][8].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][8].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][8].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][8].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[5].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][8].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[5].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][8].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第9行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][8].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][8].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][8].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][8].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][8].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][8].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][8].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[8];
    _gstEmListGridStandFlagPara.sub_widget_para[1][8].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][8].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][8].sub_widget.text_para.p_content, "Britain");

    //第10行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][9].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][9].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][9].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][9].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][9].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][9].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[6].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][9].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[6].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][9].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第10行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][9].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][9].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][9].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][9].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][9].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][9].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][9].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[9];
    _gstEmListGridStandFlagPara.sub_widget_para[1][9].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][9].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][9].sub_widget.text_para.p_content, "Hungary");

    //第11行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][10].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][10].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][10].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][10].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][10].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][10].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[7].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][10].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[7].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][10].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第11行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][10].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][10].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][10].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][10].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][10].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][10].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][10].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[10];
    _gstEmListGridStandFlagPara.sub_widget_para[1][10].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][10].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][10].sub_widget.text_para.p_content, "Belgium");

    //第12行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][11].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][11].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][11].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][11].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][11].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][11].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[8].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][11].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[8].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][11].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第12行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][11].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][11].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][11].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][11].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][11].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][11].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][11].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[11];
    _gstEmListGridStandFlagPara.sub_widget_para[1][11].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][11].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][11].sub_widget.text_para.p_content, "Greece");

    //第13行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][12].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][12].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][12].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][12].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][12].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][12].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[9].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][12].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[9].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][12].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第13行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][12].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][12].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][12].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][12].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][12].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][12].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][12].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[12];
    _gstEmListGridStandFlagPara.sub_widget_para[1][12].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][12].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][12].sub_widget.text_para.p_content, "France");
    //第14行左边
    _gstEmListGridStandFlagPara.sub_widget_para[0][13].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][13].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][13].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][13].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][13].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][13].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[10].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][13].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[10].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][13].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
    //第14行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][13].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][13].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][13].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][13].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][13].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][13].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][13].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[13];
    _gstEmListGridStandFlagPara.sub_widget_para[1][13].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][13].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][13].sub_widget.text_para.p_content, "Thailand");
    //第15行左边
#if 0
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[11].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[11].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
#else    
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].sub_widget.text_para.p_content = _garrListReserveFlagInfoLeftTextBuf[1];
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[0][14].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[0][14].sub_widget.text_para.p_content, " ");
#endif
    //第15行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][14].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][14].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][14].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][14].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][14].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][14].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][14].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[14];
    _gstEmListGridStandFlagPara.sub_widget_para[1][14].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][14].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][14].sub_widget.text_para.p_content, "Local");
    //第16行左边
#if 0 
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].widget_type = EM_WIDGET_TYPE_IMG;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].sub_widget.img_para.mem_img = _gstNationalFlagImgPara[12].mem_img;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].sub_widget.img_para.p_img_buf = _gstNationalFlagImgPara[12].p_img_buf;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].sub_widget.img_para.img_size = EM_LIST_NATION_INFO_ICON_ARROW_SIZE;
#else
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].x_relative_pos = 4;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].y_relative_pos = 1;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].x_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].y_size = 30;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].sub_widget.text_para.p_content = _garrListReserveFlagInfoLeftTextBuf[0];
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[0][15].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[0][15].sub_widget.text_para.p_content, " ");
#endif
    //第16行右边
    _gstEmListGridStandFlagPara.sub_widget_para[1][15].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListGridStandFlagPara.sub_widget_para[1][15].x_relative_pos = 50;
    _gstEmListGridStandFlagPara.sub_widget_para[1][15].y_relative_pos = 5;
    _gstEmListGridStandFlagPara.sub_widget_para[1][15].x_size = 110;
    _gstEmListGridStandFlagPara.sub_widget_para[1][15].y_size = 25;
    _gstEmListGridStandFlagPara.sub_widget_para[1][15].align = EM_TEXT_USER_ALIGN;
    _gstEmListGridStandFlagPara.sub_widget_para[1][15].sub_widget.text_para.p_content = _garrListNationalFlagInfoLeftTextBuf[15];
    _gstEmListGridStandFlagPara.sub_widget_para[1][15].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListGridStandFlagPara.sub_widget_para[1][15].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListGridStandFlagPara.sub_widget_para[1][15].sub_widget.text_para.p_content, "60Hz");
    
    /*List选中框*/
    _gstEmListGridStandFlagPara.selected_unvalid_color = 0xFF000000;
    _gstEmListGridStandFlagPara.selected_valid_color = 0x00D6E400;
    _gstEmListGridStandFlagPara.selected_size = 3;
    _gstEmListGridStandFlagPara.selected_r = 5;
    _gstEmListGridStandFlagPara.selected_x_start_offset = 42;
    _gstEmListGridStandFlagPara.selected_y_start_offset = -2;
    _gstEmListGridStandFlagPara.selected_x_end_offset = -2;
    _gstEmListGridStandFlagPara.selected_y_end_offset = 2;
    _gstEmListGridStandFlagPara.selected_type = EN_LIST_SELECTED_TYPE_FILLED;
    //_gstEmListGridStandFlagPara.selected_type = EN_LIST_SELECTED_TYPE_FRAME;

    /*滚动条*/
    _gstEmListGridStandFlagPara.scrollbar_enable = true;
    _gstEmListGridStandFlagPara.scrollbar_color = 0x00D6E400;
    _gstEmListGridStandFlagPara.scrollbar_bk_color = 0x00565656;
    _gstEmListGridStandFlagPara.scrollbar_x_pos = 454;
    _gstEmListGridStandFlagPara.scrollbar_y_pos = 39;
    _gstEmListGridStandFlagPara.scrollbar_x_size = 6;
    _gstEmListGridStandFlagPara.scrollbar_y_size = 241;
}


/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    uint8_t i = 0;
    for(i = 0; i < EM_LIST_NATIONAL_FLAG_NUMS; i++)
    {
        GUI_ALLOC_Free(_gstNationalFlagImgPara[i].mem_img);
    }

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
    if(gstEmWinListGridStandFlag.win_handle == 0)
    {
        return;
    }

#if 0
    if(GetSystick() - timecount > 300)
    {
        timecount = GetSystick();
        if(_gstEmListGridStandFlagPara.list_sel + _gstEmListGridStandFlagPara.list_page >= _gstEmListGridStandFlagPara.list_num - 1)
        {
            dir = 1;
        }
        else if(_gstEmListGridStandFlagPara.list_sel + _gstEmListGridStandFlagPara.list_page == 0)
        {
            dir = 0;
        }
        if(dir == 0)
        {
            EmListDownward(&_gstEmListGridStandFlagPara, gstEmWinListGridStandFlag.win_handle);
        }
        else
        {
            EmListUpward(&_gstEmListGridStandFlagPara, gstEmWinListGridStandFlag.win_handle);
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
        EmListUpward(&_gstEmListGridStandFlagPara, gstEmWinListGridStandFlag.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_RIGHT)
    {
        ++_gstUiCtx.select;
        if (_gstUiCtx.select >= _gstEmListGridStandFlagPara.list_num)
        {
            --_gstUiCtx.select;
            return;
        }
        EmListDownward(&_gstEmListGridStandFlagPara, gstEmWinListGridStandFlag.win_handle);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        uint8_t data_buff[256 + 8] = {0};
        uint16_t tmp = 0;

        gstFlagIndex.flag_idx = _gstUiCtx.select;  
        tmp = BigLittleSwap16(_gstUiCtx.select);
        ModbusWriteSingleRemain(data_buff, 0x01, 0x633B, tmp);
        gWaitTime = tim_get_systime();
        gSwitchIconStates = 1;  
        SetGridListShow(0);
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
    {
        SetGridListShow(0);
        SetGridStandArrowDirection();
        return;
    }
}


