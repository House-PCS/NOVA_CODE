/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_list_opera_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-22
  * @brief       削峰填谷时间设置-保存成功
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-01-18    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"
#include <time.h>

#define  TEST_DYNAMIC                                           0
#define  EM_SAVAED_SET_TIME_ICON_TICK_SIZE                      (4096)

#define REC_SAVE_XPOS               (370)
#define REC_SAVE_YPOS               (285)
#define REC_SAVE_XSIZE              (90)
#define REC_SAVE_YSIZE              (25)
#define PEAK_DIGIT_NUMS             (32)

#define NUMS_LINE_LEN               (12)
#define NUMS_LIN_X_OFFSET           (16)
#define NUMS_LIN_Y_OFFFST           (27)


static GUI_HWIN _CreateWin(void);
static void _EndWin(void);
static void _Init(void);
static void _Del(void);
static void _Polling(void);
static void _keyHandler(KeyNewMsg_t *p_msg);


emWin_t gstEmWinListSavedSetTimeInfo = {0, NULL, _CreateWin, _EndWin, _keyHandler, _Polling};

typedef struct _ui_ctx_stru_t
{
    int32_t select;
    int32_t key_init;
    int8_t num[PEAK_DIGIT_NUMS];
    uint32_t    chg_start_time1;
    uint32_t    chg_end_time1;
    uint32_t    dsg_start_time1;
    uint32_t    dsg_end_time1;
    uint32_t    chg_start_time2;
    uint32_t    chg_end_time2;
    uint32_t    dsg_start_time2;
    uint32_t    dsg_end_time2;

} UiCtx_t;

typedef struct  _peak_load_time_stru_t
{
    uint32_t    chg_start_time1;
    uint32_t    chg_end_time1;
    uint32_t    dsg_start_time1;
    uint32_t    dsg_end_time1;

    uint32_t    chg_start_time2;
    uint32_t    chg_end_time2;
    uint32_t    dsg_start_time2;
    uint32_t    dsg_end_time2;
} PeakLoadTimeData_t;

static PeakLoadTimeData_t _gstPeakLoadTime;
static PeakLoadTimeData_t _gstPeakLoadTimeTemp;
static UiCtx_t _gstUiCtx;


/*标题控件*/
static TEXT_Handle  _gTitleHandle;//标题句柄
static TEXT_Handle  _gPressHandle;//底部文本句柄
static TEXT_Handle  _gSaveHandle;//save句柄
static TEXT_Handle  _garrTextNumHandle[PEAK_DIGIT_NUMS];
static IMAGE_Handle _gImgTickHandle;//底部对勾图像句柄

static emWidgetTextPara_t  _gstTitleTextPara;
static emWidgetTextPara_t  _gstPressTextPara;
static emWidgetTextPara_t  _gstSaveTextPara;
static emWidgetTextPara_t _garrNumTextPara[PEAK_DIGIT_NUMS];
static emWidgetImgPara_t  _gstImgTickPara;


static char _garrTitleBuf[] = "Settings < System < Working Mode < Set Time";
static char _garrPressBuf[] = "You have saved sucessfully.";
static char _garrSaveBuf[]  = "Save";
static char _garrSavedSetTimeNumTextBuf[PEAK_DIGIT_NUMS][20] = {0};

/*List控件*/
static emListPara_t  _gstEmListSavedSetTimeInfoPara;
static char _garrListSavedSetTimeInfoLeftTextBuf[8][25];
static char _garrListSavedSetTimeInfoRightTextBuf[8][20];
static char _garrListSavedSetTimeInfoHeadTextBuf[2][20];
static char _garrListSavedSetTimeInfoSetTimeErrTextBuf[80];

/*PV Input中的线条信息*/
#define WIN_LIST_SAVED_SET_TIME_LINE_SIZE               (3)
#define DIALOG_PARA_OFFSET                              (4)

#define WIN_LIST_SAVED_SET_TIME1_LINE_X0POS             (79)
#define WIN_LIST_SAVED_SET_TIME1_LINE_Y0POS             (50)
#define WIN_LIST_SAVED_SET_TIME1_LINE_X1POS             (79)
#define WIN_LIST_SAVED_SET_TIME1_LINE_Y1POS             (50 + 91 -1)

#define WIN_LIST_SAVED_SET_TIME2_LINE_X0POS             (79)
#define WIN_LIST_SAVED_SET_TIME2_LINE_Y0POS             (175)
#define WIN_LIST_SAVED_SET_TIME2_LINE_X1POS             (79)
#define WIN_LIST_SAVED_SET_TIME2_LINE_Y1POS             (175 + 91 -1)


#define WIN_LIST_SAVED_SET_TIME_INFO_ID                                  (GUI_ID_USER + 0x12)
#define WIN_LIST_SAVED_SET_TIME_INFO_TITLE_ID                            (GUI_ID_USER + 0x13)
#define WIN_LIST_SAVED_SET_TIME_INFO_PRESS_ID                            (GUI_ID_USER + 0x14)
#define WIN_LIST_SAVED_SET_TIME_INFO_SAVE_ID                             (GUI_ID_USER + 0x15)
#define WIN_LIST_SAVED_SET_TIME_INFO_TICK_ID                             (GUI_ID_USER + 0x16)

#define WIN_SAVED_SET_TIME_NUMBER1_ID                                    (GUI_ID_USER + 0x20)
#define WIN_SAVED_SET_TIME_NUMBER2_ID                                    (GUI_ID_USER + 0x21)
#define WIN_SAVED_SET_TIME_NUMBER3_ID                                    (GUI_ID_USER + 0x22)
#define WIN_SAVED_SET_TIME_NUMBER4_ID                                    (GUI_ID_USER + 0x23)
#define WIN_SAVED_SET_TIME_NUMBER5_ID                                    (GUI_ID_USER + 0x24)
#define WIN_SAVED_SET_TIME_NUMBER6_ID                                    (GUI_ID_USER + 0x25)
#define WIN_SAVED_SET_TIME_NUMBER7_ID                                    (GUI_ID_USER + 0x26)
#define WIN_SAVED_SET_TIME_NUMBER8_ID                                    (GUI_ID_USER + 0x27)
#define WIN_SAVED_SET_TIME_NUMBER9_ID                                    (GUI_ID_USER + 0x28)
#define WIN_SAVED_SET_TIME_NUMBER10_ID                                   (GUI_ID_USER + 0x29)
#define WIN_SAVED_SET_TIME_NUMBER11_ID                                   (GUI_ID_USER + 0x2A)
#define WIN_SAVED_SET_TIME_NUMBER12_ID                                   (GUI_ID_USER + 0x2B)
#define WIN_SAVED_SET_TIME_NUMBER13_ID                                   (GUI_ID_USER + 0x2C)
#define WIN_SAVED_SET_TIME_NUMBER14_ID                                   (GUI_ID_USER + 0x2D)
#define WIN_SAVED_SET_TIME_NUMBER15_ID                                   (GUI_ID_USER + 0x2E)
#define WIN_SAVED_SET_TIME_NUMBER16_ID                                   (GUI_ID_USER + 0x2F)
#define WIN_SAVED_SET_TIME_NUMBER17_ID                                   (GUI_ID_USER + 0x30)
#define WIN_SAVED_SET_TIME_NUMBER18_ID                                   (GUI_ID_USER + 0x31)
#define WIN_SAVED_SET_TIME_NUMBER19_ID                                   (GUI_ID_USER + 0x32)
#define WIN_SAVED_SET_TIME_NUMBER20_ID                                   (GUI_ID_USER + 0x33)
#define WIN_SAVED_SET_TIME_NUMBER21_ID                                   (GUI_ID_USER + 0x34)
#define WIN_SAVED_SET_TIME_NUMBER22_ID                                   (GUI_ID_USER + 0x35)
#define WIN_SAVED_SET_TIME_NUMBER23_ID                                   (GUI_ID_USER + 0x36)
#define WIN_SAVED_SET_TIME_NUMBER24_ID                                   (GUI_ID_USER + 0x37)
#define WIN_SAVED_SET_TIME_NUMBER25_ID                                   (GUI_ID_USER + 0x38)
#define WIN_SAVED_SET_TIME_NUMBER26_ID                                   (GUI_ID_USER + 0x39)
#define WIN_SAVED_SET_TIME_NUMBER27_ID                                   (GUI_ID_USER + 0x3A)
#define WIN_SAVED_SET_TIME_NUMBER28_ID                                   (GUI_ID_USER + 0x3B)
#define WIN_SAVED_SET_TIME_NUMBER29_ID                                   (GUI_ID_USER + 0x3C)
#define WIN_SAVED_SET_TIME_NUMBER30_ID                                   (GUI_ID_USER + 0x3D)
#define WIN_SAVED_SET_TIME_NUMBER31_ID                                   (GUI_ID_USER + 0x3E)
#define WIN_SAVED_SET_TIME_NUMBER32_ID                                   (GUI_ID_USER + 0x3F)


static __SDRAM GUI_WIDGET_CREATE_INFO _garrListSavedSetTimeInfoDialogCreate[] =
{
    { WINDOW_CreateIndirect,  "ListSavedSetTimeInfoWin",    WIN_LIST_SAVED_SET_TIME_INFO_ID,            0,      0,      480,    320,  0,   0x0,  0 },
    { TEXT_CreateIndirect,    "Title",          WIN_LIST_SAVED_SET_TIME_INFO_TITLE_ID,                  31,     20,     415,    14,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "    ",           WIN_LIST_SAVED_SET_TIME_INFO_PRESS_ID,                  122,    292,    210,    14,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "    ",           WIN_LIST_SAVED_SET_TIME_INFO_SAVE_ID,                   397,    291,    45,     15,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM1",           WIN_SAVED_SET_TIME_NUMBER1_ID,                          381,    46,     9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM2",           WIN_SAVED_SET_TIME_NUMBER2_ID,                          397,    46,     9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM3",           WIN_SAVED_SET_TIME_NUMBER3_ID,                          420,    46,     9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM4",           WIN_SAVED_SET_TIME_NUMBER4_ID,                          436,    46,     9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM5",           WIN_SAVED_SET_TIME_NUMBER5_ID,                          381,    74,     9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM6",           WIN_SAVED_SET_TIME_NUMBER6_ID,                          397,    74,     9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM7",           WIN_SAVED_SET_TIME_NUMBER7_ID,                          420,    74,     9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM8",           WIN_SAVED_SET_TIME_NUMBER8_ID,                          436,    74,     9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM9",           WIN_SAVED_SET_TIME_NUMBER9_ID,                          381,    100,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM10",          WIN_SAVED_SET_TIME_NUMBER10_ID,                         397,    100,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM11",          WIN_SAVED_SET_TIME_NUMBER11_ID,                         420,    100,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM12",          WIN_SAVED_SET_TIME_NUMBER12_ID,                         436,    100,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM13",          WIN_SAVED_SET_TIME_NUMBER13_ID,                         381,    128,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM14",          WIN_SAVED_SET_TIME_NUMBER14_ID,                         397,    128,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM15",          WIN_SAVED_SET_TIME_NUMBER15_ID,                         420,    128,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM16",          WIN_SAVED_SET_TIME_NUMBER16_ID,                         436,    128,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM17",          WIN_SAVED_SET_TIME_NUMBER17_ID,                         381,    171,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM18",          WIN_SAVED_SET_TIME_NUMBER18_ID,                         397,    171,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM19",          WIN_SAVED_SET_TIME_NUMBER19_ID,                         420,    171,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM20",          WIN_SAVED_SET_TIME_NUMBER20_ID,                         436,    171,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM21",          WIN_SAVED_SET_TIME_NUMBER21_ID,                         381,    199,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM22",          WIN_SAVED_SET_TIME_NUMBER22_ID,                         397,    199,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM23",          WIN_SAVED_SET_TIME_NUMBER23_ID,                         420,    199,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM24",          WIN_SAVED_SET_TIME_NUMBER24_ID,                         436,    199,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM25",          WIN_SAVED_SET_TIME_NUMBER25_ID,                         381,    225,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM26",          WIN_SAVED_SET_TIME_NUMBER26_ID,                         397,    225,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM27",          WIN_SAVED_SET_TIME_NUMBER27_ID,                         420,    225,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM28",          WIN_SAVED_SET_TIME_NUMBER28_ID,                         436,    225,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM29",          WIN_SAVED_SET_TIME_NUMBER29_ID,                         381,    253,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM30",          WIN_SAVED_SET_TIME_NUMBER30_ID,                         397,    253,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM31",          WIN_SAVED_SET_TIME_NUMBER31_ID,                         420,    253,    9,      17,   0,   0x0,  0 },
    { TEXT_CreateIndirect,    "NUM32",          WIN_SAVED_SET_TIME_NUMBER32_ID,                         436,    253,    9,      17,   0,   0x0,  0 },
    { IMAGE_CreateIndirect,   "TICK",           WIN_LIST_SAVED_SET_TIME_INFO_TICK_ID,                   100,    288,    20,     20,   0,   0x0,  0 }
};

static int32_t _SetPeakTime1ChrgStartTimeNumber(int32_t ui_update, uint32_t chrg_start_time)
{
    uint8_t i = 0;
    struct tm* p_st_timeinfo;
    char arr_tmp_data[8] = {0};

    p_st_timeinfo = localtime(&chrg_start_time); // 根据时间戳生成tm结构体对象
    //以十进制数写时,分钟， 24 小时制
    strftime(arr_tmp_data, sizeof(arr_tmp_data), "%H%M", p_st_timeinfo);

    for(i = 0; i < 4; ++i)
    {
        _garrSavedSetTimeNumTextBuf[i][0] = arr_tmp_data[i];
        _gstUiCtx.num[i] = atoi((const char *)& _garrSavedSetTimeNumTextBuf[i][0]);
    }

    if(ui_update)
    {
        for(i = 0; i < 4; ++i)
        {
            TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
        }
    }
    return 0;
}
static int32_t _SetPeakTime1ChrgEndTimeNumber(int32_t ui_update, uint32_t chrg_end_time)
{
    uint8_t i = 0;
    struct tm* p_st_timeinfo;
    char arr_tmp_data[8] = {0};

    p_st_timeinfo = localtime(&chrg_end_time); // 根据时间戳生成tm结构体对象
    //以十进制数写时,分钟， 24 小时制
    strftime(arr_tmp_data, sizeof(arr_tmp_data), "%H%M", p_st_timeinfo);

    for(i = 4 ; i < 8; ++i)
    {
        _garrSavedSetTimeNumTextBuf[i][0] = arr_tmp_data[i - 4];
        _gstUiCtx.num[i] = atoi((const char *)& _garrSavedSetTimeNumTextBuf[i][0]);
    }

    if(ui_update)
    {
        for(i = 4; i < 8; ++i)
        {
            TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
        }
    }
    return 0;
}
static int32_t _SetPeakTime1DischrgStartTimeNumber(int32_t ui_update, uint32_t dischrg_start_time)
{
    uint8_t i = 0;
    struct tm* p_st_timeinfo;
    char arr_tmp_data[8] = {0};

    p_st_timeinfo = localtime(&dischrg_start_time); // 根据时间戳生成tm结构体对象
    //以十进制数写时,分钟， 24 小时制
    strftime(arr_tmp_data, sizeof(arr_tmp_data), "%H%M", p_st_timeinfo);

    for(i = 8 ; i < 12; ++i)
    {
        _garrSavedSetTimeNumTextBuf[i][0] = arr_tmp_data[i - 8];
        _gstUiCtx.num[i] = atoi((const char *)& _garrSavedSetTimeNumTextBuf[i][0]);
    }

    if(ui_update)
    {
        for(i = 8; i < 12; ++i)
        {
            TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
        }
    }
    return 0;
}

static int32_t _SetPeakTime1DischrgEndTimeNumber(int32_t ui_update, uint32_t dischrg_end_time)
{
    uint8_t i = 0;
    struct tm* p_st_timeinfo;
    char arr_tmp_data[8] = {0};

    p_st_timeinfo = localtime(&dischrg_end_time); // 根据时间戳生成tm结构体对象
    //以十进制数写时,分钟， 24 小时制
    strftime(arr_tmp_data, sizeof(arr_tmp_data), "%H%M", p_st_timeinfo);

    for(i = 12 ; i < 16; ++i)
    {
        _garrSavedSetTimeNumTextBuf[i][0] = arr_tmp_data[i - 12];
        _gstUiCtx.num[i] = atoi((const char *)& _garrSavedSetTimeNumTextBuf[i][0]);
    }

    if(ui_update)
    {
        for(i = 12; i < 16; ++i)
        {
            TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
        }
    }
    return 0;
}

static int32_t _SetPeakTime2ChrgStartTimeNumber(int32_t ui_update, uint32_t chrg_start_time)
{
    uint8_t i = 0;
    struct tm* p_st_timeinfo;
    char arr_tmp_data[8] = {0};

    p_st_timeinfo = localtime(&chrg_start_time); // 根据时间戳生成tm结构体对象
    //以十进制数写时,分钟， 24 小时制
    strftime(arr_tmp_data, sizeof(arr_tmp_data), "%H%M", p_st_timeinfo);

    for(i = 16; i < 20; ++i)
    {
        _garrSavedSetTimeNumTextBuf[i][0] = arr_tmp_data[i - 16];
        _gstUiCtx.num[i] = atoi((const char *)& _garrSavedSetTimeNumTextBuf[i][0]);
    }

    if(ui_update)
    {
        for(i = 16; i < 20; ++i)
        {
            TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
        }
    }
    return 0;
}
static int32_t _SetPeakTime2ChrgEndTimeNumber(int32_t ui_update, uint32_t chrg_end_time)
{
    uint8_t i = 0;
    struct tm* p_st_timeinfo;
    char arr_tmp_data[8] = {0};

    p_st_timeinfo = localtime(&chrg_end_time); // 根据时间戳生成tm结构体对象
    //以十进制数写时,分钟， 24 小时制
    strftime(arr_tmp_data, sizeof(arr_tmp_data), "%H%M", p_st_timeinfo);

    for(i = 20 ; i < 24; ++i)
    {
        _garrSavedSetTimeNumTextBuf[i][0] = arr_tmp_data[i - 20];
        _gstUiCtx.num[i] = atoi((const char *)& _garrSavedSetTimeNumTextBuf[i][0]);
    }

    if(ui_update)
    {
        for(i = 20; i < 24; ++i)
        {
            TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
        }
    }
    return 0;
}
static int32_t _SetPeakTime2DischrgStartTimeNumber(int32_t ui_update, uint32_t dischrg_start_time)
{
    uint8_t i = 0;
    struct tm* p_st_timeinfo;
    char arr_tmp_data[8] = {0};

    p_st_timeinfo = localtime(&dischrg_start_time); // 根据时间戳生成tm结构体对象
    //以十进制数写时,分钟， 24 小时制
    strftime(arr_tmp_data, sizeof(arr_tmp_data), "%H%M", p_st_timeinfo);

    for(i = 24 ; i < 28; ++i)
    {
        _garrSavedSetTimeNumTextBuf[i][0] = arr_tmp_data[i - 24];
        _gstUiCtx.num[i] = atoi((const char *)& _garrSavedSetTimeNumTextBuf[i][0]);
    }

    if(ui_update)
    {
        for(i = 24; i < 28; ++i)
        {
            TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
        }
    }
    return 0;
}

static int32_t _SetPeakTime2DischrgEndTimeNumber(int32_t ui_update, uint32_t dischrg_end_time)
{
    uint8_t i = 0;
    struct tm* p_st_timeinfo;
    char arr_tmp_data[8] = {0};

    p_st_timeinfo = localtime(&dischrg_end_time); // 根据时间戳生成tm结构体对象
    //以十进制数写时,分钟， 24 小时制
    strftime(arr_tmp_data, sizeof(arr_tmp_data), "%H%M", p_st_timeinfo);

    for(i = 28 ; i < 32; ++i)
    {
        _garrSavedSetTimeNumTextBuf[i][0] = arr_tmp_data[i - 28];
        _gstUiCtx.num[i] = atoi((const char *)& _garrSavedSetTimeNumTextBuf[i][0]);
    }

    if(ui_update)
    {
        for(i = 28; i < 32; ++i)
        {
            TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
        }
    }
    return 0;
}

/**
  * @brief  获取设定完成后的消峰填谷时间（秒数）
  * @param  void
  * @retval NULL
  */

static PeakLoadTimeData_t _GetPeakLoadTimeSec(void)
{
    PeakLoadTimeData_t st_peak_time = {0};

    int32_t hour  = 0;
    int32_t minute = 0;
    int32_t second = 0;

    hour    = (_gstUiCtx.num[0] * 10 + _gstUiCtx.num[1]) * 3600;
    minute  = (_gstUiCtx.num[2] * 10 + _gstUiCtx.num[3]) * 60;
    st_peak_time.chg_start_time1 = hour + minute;

    hour    = (_gstUiCtx.num[4] * 10 + _gstUiCtx.num[5]) * 3600;
    minute  = (_gstUiCtx.num[6] * 10 + _gstUiCtx.num[7]) * 60;
    st_peak_time.chg_end_time1 = hour + minute;

    hour    = (_gstUiCtx.num[8] * 10 + _gstUiCtx.num[9]) * 3600;
    minute  = (_gstUiCtx.num[10] * 10 + _gstUiCtx.num[11]) * 60;
    st_peak_time.dsg_start_time1  = hour + minute;

    hour    = (_gstUiCtx.num[12] * 10 + _gstUiCtx.num[13]) * 3600;
    minute  = (_gstUiCtx.num[14] * 10 + _gstUiCtx.num[15]) * 60;
    st_peak_time.dsg_end_time1  = hour + minute;

    hour    = (_gstUiCtx.num[16] * 10 + _gstUiCtx.num[17]) * 3600;
    minute  = (_gstUiCtx.num[18] * 10 + _gstUiCtx.num[19]) * 60;
    st_peak_time.chg_start_time2  = hour + minute;

    hour    = (_gstUiCtx.num[20] * 10 + _gstUiCtx.num[21]) * 3600;
    minute  = (_gstUiCtx.num[22] * 10 + _gstUiCtx.num[23]) * 60;
    st_peak_time.chg_end_time2  = hour + minute;

    hour    = (_gstUiCtx.num[24] * 10 + _gstUiCtx.num[25]) * 3600;
    minute  = (_gstUiCtx.num[26] * 10 + _gstUiCtx.num[27]) * 60;
    st_peak_time.dsg_start_time2  = hour + minute;

    hour    = (_gstUiCtx.num[28] * 10 + _gstUiCtx.num[29]) * 3600;
    minute  = (_gstUiCtx.num[30] * 10 + _gstUiCtx.num[31]) * 60;
    st_peak_time.dsg_end_time2  = hour + minute;

    return st_peak_time;
}

/**
  * @brief  获取设定完成后的充电开始时间1（秒数）
  * @param  void
  * @retval NULL
  */

static uint32_t _GetChrgStartTime1Sec(void)
{
    uint32_t start_time;
    struct tm st_timeinfo;

    st_timeinfo.tm_year = 1970 - 1900;
    st_timeinfo.tm_mon  = 01 - 1;
    st_timeinfo.tm_mday = 01;
    st_timeinfo.tm_hour = (int32_t)(_gstUiCtx.num[0] * 10 + _gstUiCtx.num[1]);
    st_timeinfo.tm_min  = (int32_t)(_gstUiCtx.num[2] * 10 + _gstUiCtx.num[3]);
    st_timeinfo.tm_sec  = 0;
    st_timeinfo.tm_isdst = 0;

    start_time = mktime(&st_timeinfo);

    return start_time;
}

/**
  * @brief  获取设定完成后的充电结束时间1（秒数）
  * @param  void
  * @retval NULL
  */

static uint32_t _GetChrgEndTime1Sec(void)
{
    uint32_t end_time;
    struct tm st_timeinfo;

    st_timeinfo.tm_year = 1970 - 1900;
    st_timeinfo.tm_mon  = 01 - 1;
    st_timeinfo.tm_mday = 01;
    st_timeinfo.tm_hour = (int32_t)(_gstUiCtx.num[4] * 10 + _gstUiCtx.num[5]);
    st_timeinfo.tm_min  = (int32_t)(_gstUiCtx.num[6] * 10 + _gstUiCtx.num[7]);
    st_timeinfo.tm_sec  = 0;
    st_timeinfo.tm_isdst = 0; //不采用夏令时

    end_time = mktime(&st_timeinfo);

    return end_time;
}

/**
  * @brief  获取设定完成后的放电开始时间1（秒数）
  * @param  void
  * @retval NULL
  */

static uint32_t _GetDischrgStartTime1Sec(void)
{
    uint32_t start_time;
    struct tm st_timeinfo;

    st_timeinfo.tm_year = 1970 - 1900;
    st_timeinfo.tm_mon  = 01 - 1;
    st_timeinfo.tm_mday = 01;
    st_timeinfo.tm_hour = (int32_t)(_gstUiCtx.num[8] * 10 + _gstUiCtx.num[9]);
    st_timeinfo.tm_min  = (int32_t)(_gstUiCtx.num[10] * 10 + _gstUiCtx.num[11]);
    st_timeinfo.tm_sec  = 0;
    st_timeinfo.tm_isdst = 0;

    start_time = mktime(&st_timeinfo);

    return start_time;
}

/**
  * @brief  获取设定完成后的放电结束时间1（秒数）
  * @param  void
  * @retval NULL
  */

static uint32_t _GetDischrgEndTime1Sec(void)
{
    uint32_t end_time;
    struct tm st_timeinfo;

    st_timeinfo.tm_year = 1970 - 1900;
    st_timeinfo.tm_mon  = 01 - 1;
    st_timeinfo.tm_mday = 01;
    st_timeinfo.tm_hour = (int32_t)(_gstUiCtx.num[12] * 10 + _gstUiCtx.num[13]);
    st_timeinfo.tm_min  = (int32_t)(_gstUiCtx.num[14] * 10 + _gstUiCtx.num[15]);
    st_timeinfo.tm_sec  = 0;
    st_timeinfo.tm_isdst = 0;

    end_time = mktime(&st_timeinfo);

    return end_time;
}

/**
  * @brief  获取设定完成后的充电开始时间2（秒数）
  * @param  void
  * @retval NULL
  */

static uint32_t _GetChrgStartTime2Sec(void)
{
    uint32_t start_time;
    struct tm st_timeinfo;

    st_timeinfo.tm_year = 1970 - 1900;
    st_timeinfo.tm_mon  = 01 - 1;
    st_timeinfo.tm_mday = 01;
    st_timeinfo.tm_hour = (int32_t)(_gstUiCtx.num[16] * 10 + _gstUiCtx.num[17]);
    st_timeinfo.tm_min  = (int32_t)(_gstUiCtx.num[18] * 10 + _gstUiCtx.num[19]);
    st_timeinfo.tm_sec  = 0;
    st_timeinfo.tm_isdst = 0;

    start_time = mktime(&st_timeinfo);

    return start_time;
}

/**
  * @brief  获取设定完成后的充电结束时间2（秒数）
  * @param  void
  * @retval NULL
  */

static uint32_t _GetChrgEndTime2Sec(void)
{
    uint32_t end_time;
    struct tm st_timeinfo;

    st_timeinfo.tm_year = 1970 - 1900;
    st_timeinfo.tm_mon  = 01 - 1;
    st_timeinfo.tm_mday = 01;
    st_timeinfo.tm_hour = (int32_t)(_gstUiCtx.num[20] * 10 + _gstUiCtx.num[21]);
    st_timeinfo.tm_min  = (int32_t)(_gstUiCtx.num[22] * 10 + _gstUiCtx.num[23]);
    st_timeinfo.tm_sec  = 0;
    st_timeinfo.tm_isdst = 0;

    end_time = mktime(&st_timeinfo);

    return end_time;
}

/**
  * @brief  获取设定完成后的放电开始时间2（秒数）
  * @param  void
  * @retval NULL
  */

static uint32_t _GetDischrgStartTime2Sec(void)
{
    uint32_t start_time;
    struct tm st_timeinfo;

    st_timeinfo.tm_year = 1970 - 1900;
    st_timeinfo.tm_mon  = 01 - 1;
    st_timeinfo.tm_mday = 01;
    st_timeinfo.tm_hour = (int32_t)(_gstUiCtx.num[24] * 10 + _gstUiCtx.num[25]);
    st_timeinfo.tm_min  = (int32_t)(_gstUiCtx.num[26] * 10 + _gstUiCtx.num[27]);
    st_timeinfo.tm_sec  = 0;
    st_timeinfo.tm_isdst = 0;

    start_time = mktime(&st_timeinfo);

    return start_time;
}

/**
  * @brief  获取设定完成后的放电结束时间2（秒数）
  * @param  void
  * @retval NULL
  */

static uint32_t _GetDischrgEndTime2Sec(void)
{
    uint32_t end_time;
    struct tm st_timeinfo;

    st_timeinfo.tm_year = 1970 - 1900;
    st_timeinfo.tm_mon  = 01 - 1;
    st_timeinfo.tm_mday = 01;
    st_timeinfo.tm_hour = (int32_t)(_gstUiCtx.num[28] * 10 + _gstUiCtx.num[29]);
    st_timeinfo.tm_min  = (int32_t)(_gstUiCtx.num[30] * 10 + _gstUiCtx.num[31]);
    st_timeinfo.tm_sec  = 0;
    st_timeinfo.tm_isdst = 0;

    end_time = mktime(&st_timeinfo);

    return end_time;
}

/**
  * @brief  获取当前错误的提示字符串
  * @param  void
  * @retval 提示字符串
  */
char *GetPeakShiftingErr(void)
{
    return _garrListSavedSetTimeInfoSetTimeErrTextBuf;
}

/**
  * @brief  检验哪个时间段被设置
  * @param  void
  * @retval time_flag：bit0-充1 bit1-充2 bit2-放1 bit3-放2
  */
static uint8_t _JudgeWhichTimeIsSet(void)
{
    uint8_t time_flag = 0;

    _gstPeakLoadTimeTemp.chg_start_time1 = _GetChrgStartTime1Sec();
    _gstPeakLoadTimeTemp.chg_end_time1   = _GetChrgEndTime1Sec();
    _gstPeakLoadTimeTemp.dsg_start_time1 = _GetDischrgStartTime1Sec();
    _gstPeakLoadTimeTemp.dsg_end_time1   = _GetDischrgEndTime1Sec();
    _gstPeakLoadTimeTemp.chg_start_time2 = _GetChrgStartTime2Sec();
    _gstPeakLoadTimeTemp.chg_end_time2   = _GetChrgEndTime2Sec();
    _gstPeakLoadTimeTemp.dsg_start_time2 = _GetDischrgStartTime2Sec();
    _gstPeakLoadTimeTemp.dsg_end_time2   = _GetDischrgEndTime2Sec();


    //检测标志位 查看是哪一个时间被设置
    if(_gstPeakLoadTimeTemp.chg_start_time1 > 0 || _gstPeakLoadTimeTemp.chg_end_time1 > 0)
    {
        time_flag |= (0x01 << 0);
    }
    if(_gstPeakLoadTimeTemp.chg_start_time2 > 0 || _gstPeakLoadTimeTemp.chg_end_time2 > 0)
    {
        time_flag |= (0x01 << 1);
    }
    if(_gstPeakLoadTimeTemp.dsg_start_time1 > 0 || _gstPeakLoadTimeTemp.dsg_end_time1 > 0)
    {
        time_flag |= (0x01 << 2);
    }
    if(_gstPeakLoadTimeTemp.dsg_start_time2 > 0 || _gstPeakLoadTimeTemp.dsg_end_time2 > 0)
    {
        time_flag |= (0x01 << 3);
    }

    return time_flag;
}

/**
  * @brief  判断时间段的充电开始时间是否大于结束时间
  * @param  void
  * @retval 0--时间合理 1--时间不合理（开始时间大于结束时间）
  */
static int32_t _JudgeTimeStartAndEndTime(uint32_t time1, uint32_t time2)
{
    int32_t n_ret = 0;
    if(time1 >= time2)
    {
        n_ret = 1;;
    }
    else
    {
        n_ret = 0;
    }

    return n_ret;
}

/**
  * @brief  判断时间段的充电时间和放电时间是否有重合
  * @param  void
  * @retval 0-没有重叠 1-有重叠
  * @note   若(start1 < end2 && start2 < end1)  两段时间有重叠
  */
static int32_t _JudgeChgAndDchgTimeIsCorss(uint32_t start1_time, uint32_t end1_time, uint32_t start2_time, uint32_t end2_time, uint32_t start3_time, uint32_t end3_time)
{
    int32_t n_ret = 0;
    if((start1_time < end2_time && start2_time < end1_time) || (start1_time < end3_time && start3_time < end1_time))
    {
        n_ret = 1;
    }
    else
    {
        n_ret = 0;
    }
    return n_ret;
}

/**
  * @brief  检验设置的时间是否合理
  * @param  void
  * @retval 0--时间合理 1--时间不合理（开始时间大于结束时间）
  */
static int32_t _CheckTimeIsCorrect(void)
{
    int32_t n_ret = 0;
    uint8_t cross_res1 = 0;
    uint8_t cross_res2 = 0;
    uint8_t time_flag = 0;  //bit0：C1  bit1: C2  bit3：F1  bit4:F2


    //检测标志位 查看是哪一个时间被设置
    time_flag = _JudgeWhichTimeIsSet();
    //时间都为0,未设置 报错 返回1
    if(time_flag == 0)
    {
        n_ret = 1;

        strcpy(_garrListSavedSetTimeInfoSetTimeErrTextBuf, "At least set a complete \ndischarging and\ncharging time period !"); // 复制字符串内容
        return n_ret;
    }

    //至少设置了一个充放电时间
    if(!((((time_flag & 0x01) || (time_flag & 0x02)) == 1) && (((time_flag & 0x04) || (time_flag & 0x08)) == 1)))
    {
        n_ret = 1;

        strcpy(_garrListSavedSetTimeInfoSetTimeErrTextBuf, "At least set a complete \ndischarging and\ncharging time period !"); // 复制字符串内容
        return n_ret;
    }

    //判断开始和结束时间
    if((time_flag & 0x01) >> 0)  //充1
    {
        //存在 判断

        n_ret = _JudgeTimeStartAndEndTime(_gstPeakLoadTimeTemp.chg_start_time1, _gstPeakLoadTimeTemp.chg_end_time1);
        if(n_ret)
        {
            strcpy(_garrListSavedSetTimeInfoSetTimeErrTextBuf, "The start time cannot be\nlater than the end time !"); // 复制字符串内容
            return n_ret;
        }
    }
    if((time_flag & 0x02) >> 1) //充2
    {
        //存在 判断
        n_ret = _JudgeTimeStartAndEndTime(_gstPeakLoadTimeTemp.chg_start_time2, _gstPeakLoadTimeTemp.chg_end_time2);
        if(n_ret)
        {
            strcpy(_garrListSavedSetTimeInfoSetTimeErrTextBuf, "The start time cannot be\nlater than the end time !"); // 复制字符串内容
            return n_ret;
        }
    }
    if((time_flag & 0x04) >> 2) //放1
    {
        //存在 判断
        n_ret = _JudgeTimeStartAndEndTime(_gstPeakLoadTimeTemp.dsg_start_time1, _gstPeakLoadTimeTemp.dsg_end_time1);
        if(n_ret)
        {
            strcpy(_garrListSavedSetTimeInfoSetTimeErrTextBuf, "The start time cannot be\nlater than the end time !"); // 复制字符串内容
            return n_ret;
        }
    }
    if((time_flag & 0x08) >> 3) //放2
    {
        //存在 判断
        n_ret = _JudgeTimeStartAndEndTime(_gstPeakLoadTimeTemp.dsg_start_time2, _gstPeakLoadTimeTemp.dsg_end_time2);
        if(n_ret)
        {
            strcpy(_garrListSavedSetTimeInfoSetTimeErrTextBuf, "The start time cannot be\nlater than the end time !"); // 复制字符串内容
            return n_ret;
        }
    }

    if(!n_ret)   //时间完整
    {
        if(((time_flag & 0x01) >> 0) && ((time_flag & 0x04) >> 2 || (time_flag & 0x08) >> 3)) //判断是否有重合
        {
            cross_res1 = _JudgeChgAndDchgTimeIsCorss(_gstPeakLoadTimeTemp.chg_start_time1, _gstPeakLoadTimeTemp.chg_end_time1, _gstPeakLoadTimeTemp.dsg_start_time1,
                         _gstPeakLoadTimeTemp.dsg_end_time1, _gstPeakLoadTimeTemp.dsg_start_time2, _gstPeakLoadTimeTemp.dsg_end_time2);
        }
        if(((time_flag & 0x02) >> 1) && ((time_flag & 0x04) >> 2 || (time_flag & 0x08) >> 3))
        {
            cross_res2 = _JudgeChgAndDchgTimeIsCorss(_gstPeakLoadTimeTemp.chg_start_time2, _gstPeakLoadTimeTemp.chg_end_time2, _gstPeakLoadTimeTemp.dsg_start_time1,
                         _gstPeakLoadTimeTemp.dsg_end_time1, _gstPeakLoadTimeTemp.dsg_start_time2, _gstPeakLoadTimeTemp.dsg_end_time2);
        }
        n_ret = cross_res1 | cross_res2;

        if(n_ret)
        {
            strcpy(_garrListSavedSetTimeInfoSetTimeErrTextBuf, "The discharge time period\nand charging time period\ncannot overlap !"); // 复制字符串内容
        }
    }

    return n_ret;
}


/**
  * @brief  绘制选中框
  * @param  NULL
  * @retval NULL
  */
static void _DrawSelSaveRectPaint(int32_t sel)
{
    if(sel == 0)
    {
        GUI_SetColor(0x00BDC909);
        GUI_DrawRoundedFrame(20, 38, 20 + 440, 38 + 117 - 1, 10, 3);
    }
    else if(sel == 1)
    {
        GUI_SetColor(0x00BDC909);
        GUI_DrawRoundedFrame(20, 163, 20 + 440, 163 + 117 - 1, 10, 3);
    }
    else
    {
        GUI_SetColor(0x00BDC909);
        GUI_DrawRoundedFrame(REC_SAVE_XPOS, REC_SAVE_YPOS,  REC_SAVE_XPOS + REC_SAVE_XSIZE - 1,  REC_SAVE_YPOS + REC_SAVE_YSIZE - 1, 10, 3);
    }
}

/**
  * @brief  设置选中数字文本的颜色
  * @param  NULL
  * @retval NULL
  */
static void _SetSavedPeakTimeNumColor(int32_t index)
{
    static volatile uint8_t arr_nums_sel_falg[PEAK_DIGIT_NUMS] = {0};
    uint8_t i = 0;
    if (index < 0 || index >= PEAK_DIGIT_NUMS)
    {
        return;
    }
    for(i = 0; i < PEAK_DIGIT_NUMS; i++)
    {
        arr_nums_sel_falg[i] = 0;
        TEXT_SetTextColor(_garrTextNumHandle[i], 0x00FFFFFF);
    }
    arr_nums_sel_falg[index] = 1;
    TEXT_SetTextColor(_garrTextNumHandle[index], 0x00CDF100);
}

/**
  * @brief  设置选中数字变化
  * @param  NULL
  * @retval NULL
  */
static void _SetSavedPeakTimeNumAdd(int32_t index, int32_t v)
{
    _gstUiCtx.num[index] = v;
    snprintf(_garrSavedSetTimeNumTextBuf[index], sizeof(_garrSavedSetTimeNumTextBuf[index]), "%d", _gstUiCtx.num[index]);

    _garrNumTextPara[index].digit_value = v;
    sprintf(_garrNumTextPara[index].p_content, "%1d", (int)_garrNumTextPara[index].digit_value);
    if(9 < _garrNumTextPara[index].digit_value)
    {
        _garrNumTextPara[index].digit_value = 0;
    }
    TEXT_SetText(_garrTextNumHandle[index], _garrSavedSetTimeNumTextBuf[index]);
}


/**
  * @brief  设置选中数字的下横线颜色变化
  * @param  NULL
  * @retval NULL
  */
static void _DrawSavedPeakTimeNumUnderLine(int32_t index)
{
    GUI_SetColor(0x00CDF100);
    switch(index)
    {
        case 0:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 1:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 2:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 3:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 4:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 5:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 6:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 7:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 8:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 9:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 10:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 11:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 12:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 13:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 14:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 15:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 16:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 17:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 18:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 19:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 20:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 21:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 22:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 23:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 24:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 25:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 26:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 27:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 28:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 29:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 30:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        case 31:
        {
            GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18, \
                         _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[index + DIALOG_PARA_OFFSET].y0 + 18);
        }
        break;
        default:
            break;
    }
}

/**
  * @brief ListSavedSetTimeInfoWin callback
  * @param  NULL
  * @retval NULL
  */
static void _cbListSavedSetTimeInfoWin(WM_MESSAGE * pMsg)
{
    int32_t i = 0;
    WM_HWIN hWin;
    WM_HWIN hWinOld;
    WM_HWIN hItem;
    static int32_t num = -1;
    static int32_t index = -1;
    static int32_t selected = -1;
#if TEST_DYNAMIC
    static WM_HTIMER wm_timer_handel_nums;
#endif
    switch (pMsg->MsgId)
    {
        case WM_INIT_DIALOG:
        {
#if TEST_DYNAMIC
            wm_timer_handel_nums = WM_CreateTimer(pMsg->hWin, /* 接受信息的窗口的句柄 */
                                                  0,          /* 用户定义的Id。如果不对同一窗口使用多个定时器，此值可以设置为零。 */
                                                  1000,         /* 周期，此周期过后指定窗口应收到消息*/
                                                  0);         /* 留待将来使用，应为0 */
#endif

            /*标题*/
            _gTitleHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SAVED_SET_TIME_INFO_TITLE_ID);
            TEXT_SetFont(_gTitleHandle, _gstTitleTextPara.p_text_font);
            TEXT_SetText(_gTitleHandle, (const char *)_gstTitleTextPara.p_content);
            TEXT_SetTextAlign(_gTitleHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gTitleHandle, _gstTitleTextPara.text_color);
#if 0
            /*底部文本*/
            _gPressHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SAVED_SET_TIME_INFO_PRESS_ID);
            TEXT_SetFont(_gPressHandle, _gstPressTextPara.p_text_font);
            TEXT_SetText(_gPressHandle, (const char *)_gstPressTextPara.p_content);
            TEXT_SetTextAlign(_gPressHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gPressHandle, _gstPressTextPara.text_color);

            _gImgTickHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SAVED_SET_TIME_INFO_TICK_ID);
            IMAGE_SetDTA(_gImgTickHandle, _gstImgTickPara.p_img_buf, _gstImgTickPara.img_size);

            /*保存文本*/
            _gSaveHandle = WM_GetDialogItem(pMsg->hWin, WIN_LIST_SAVED_SET_TIME_INFO_SAVE_ID);
            TEXT_SetFont(_gSaveHandle, _gstSaveTextPara.p_text_font);
            TEXT_SetText(_gSaveHandle, (const char *)_gstSaveTextPara.p_content);
            TEXT_SetTextAlign(_gSaveHandle, TEXT_CF_LEFT | TEXT_CF_VCENTER);
            TEXT_SetTextColor(_gSaveHandle, _gstSaveTextPara.text_color);
#endif
            for(i = 0; i < PEAK_DIGIT_NUMS; i++)
            {
                _garrTextNumHandle[i] = WM_GetDialogItem(pMsg->hWin, WIN_SAVED_SET_TIME_NUMBER1_ID + i);
                TEXT_SetFont(_garrTextNumHandle[i], _garrNumTextPara[i].p_text_font);
                TEXT_SetText(_garrTextNumHandle[i], (const char *)_garrNumTextPara[i].p_content);
                TEXT_SetTextAlign(_garrTextNumHandle[i], TEXT_CF_RIGHT | TEXT_CF_VCENTER);
                TEXT_SetTextColor(_garrTextNumHandle[i], _garrNumTextPara[i].text_color);
            }
            _SetSavedPeakTimeNumColor(_gstUiCtx.select);

            /*LIST*/
            EmListWidgetCreate(&_gstEmListSavedSetTimeInfoPara, pMsg->hWin);
            break;
        }
        case WM_DELETE:
        {
#if TEST_DYNAMIC
            WM_DeleteTimer(wm_timer_handel_nums); //删除窗口，会自动删除对应内部的定时器
#endif
            break;
        }
        case WM_TIMER:
        {
#if TEST_DYNAMIC
            ++num;
            ++selected;
            index = num % PEAK_DIGIT_NUMS;
            //_SetSavedPeakTimeNumColor(index);

            WM_Paint(pMsg->hWin);
            WM_RestartTimer(pMsg->Data.v, 1000);/*暂定100mS，时间可调，针对WM_TIMER，pMsg->Data.v表示到期定时器的句柄*/
#endif
            break;
        }
        case WM_PAINT:
        {
//            SelectedDel2(&_gstEmListSavedSetTimeInfoPara, _gstEmListSavedSetTimeInfoPara.list_pre_sel);
            GUI_SetColor(0x00424242);//上色
            for(i = 0; i < _gstEmListSavedSetTimeInfoPara.list_display_num; i++)
            {
                GUI_FillRoundedRect(_gstEmListSavedSetTimeInfoPara.sub_region[i].x_pos,
                                    _gstEmListSavedSetTimeInfoPara.sub_region[i].y_pos,
                                    _gstEmListSavedSetTimeInfoPara.sub_region[i].x_pos + _gstEmListSavedSetTimeInfoPara.sub_region[i].x_size - 1,
                                    _gstEmListSavedSetTimeInfoPara.sub_region[i].y_pos + _gstEmListSavedSetTimeInfoPara.sub_region[i].y_size - 1,
                                    _gstEmListSavedSetTimeInfoPara.selected_r);
            }

            /*右下角保存框的填充*/
//            GUI_SetColor(0x00424242);//上色
//            GUI_FillRoundedRect(REC_SAVE_XPOS, REC_SAVE_YPOS,  REC_SAVE_XPOS + REC_SAVE_XSIZE - 1,  REC_SAVE_YPOS + REC_SAVE_YSIZE - 1, 10);
//          GUI_SetColor(0xFF000000);
//          GUI_DrawRoundedFrame(REC_SAVE_XPOS, REC_SAVE_YPOS,  REC_SAVE_XPOS + REC_SAVE_XSIZE - 1,  REC_SAVE_YPOS + REC_SAVE_YSIZE - 1, 10,3);

            GUI_SetPenSize(WIN_LIST_SAVED_SET_TIME_LINE_SIZE);
            GUI_SetColor(0x00FFFFFF);
            for(i = 0; i < PEAK_DIGIT_NUMS; i++)
            {
                GUI_DrawLine(_garrListSavedSetTimeInfoDialogCreate[i + 4].x0 - 2, _garrListSavedSetTimeInfoDialogCreate[i + 4].y0 + 18, \
                             _garrListSavedSetTimeInfoDialogCreate[i + 4].x0 - 2 + 12, _garrListSavedSetTimeInfoDialogCreate[i + 4].y0 + 18);
            }


            GUI_SetPenSize(WIN_LIST_SAVED_SET_TIME_LINE_SIZE);
            GUI_SetColor(0x00A2A856);
            GUI_DrawLine(79, 50, 79, 50 + 91);
            GUI_DrawLine(79, 175, 79, 175 + 91);
//          GUI_DrawLine(WIN_LIST_SAVED_SET_TIME1_LINE_X0POS,WIN_LIST_SAVED_SET_TIME1_LINE_Y0POS,WIN_LIST_SAVED_SET_TIME1_LINE_X1POS,WIN_LIST_SAVED_SET_TIME1_LINE_Y1POS);
//          GUI_DrawLine(WIN_LIST_SAVED_SET_TIME2_LINE_X0POS,WIN_LIST_SAVED_SET_TIME2_LINE_Y0POS,WIN_LIST_SAVED_SET_TIME2_LINE_X1POS,WIN_LIST_SAVED_SET_TIME2_LINE_Y1POS);

            _DrawSavedPeakTimeNumUnderLine(_gstUiCtx.select);

#if TEST_DYNAMIC
            _DrawSelSaveRectPaint(selected % 3);

#else
            _DrawSelSaveRectPaint(_gstUiCtx.select / (PEAK_DIGIT_NUMS / 2));
#endif
            //SelectedDrawInWMPaint(&_gstEmListSavedSetTimeInfoPara, _gstEmListSavedSetTimeInfoPara.list_sel);
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
    pPrevEmWin = &gstEmWinListWorkingModeInfo;
    _Init();
//    GUI_EnableAlpha(1);
    gstEmWinListSavedSetTimeInfo.win_handle = GUI_CreateDialogBox(_garrListSavedSetTimeInfoDialogCreate, GUI_COUNTOF(_garrListSavedSetTimeInfoDialogCreate), _cbListSavedSetTimeInfoWin, WM_HBKWIN, 0, 0);
    WINDOW_SetBkColor(gstEmWinListSavedSetTimeInfo.win_handle, 0x00353535);
//    WINDOW_SetBkColor(gstEmWinListSavedSetTimeInfo.win_handle, 0x00000000);
//    WM_SetTransState(gstEmWinListSavedSetTimeInfo.win_handle, WM_CF_HASTRANS);
    GUI_Exec();
    SelectedDraw(&_gstEmListSavedSetTimeInfoPara, gstEmWinListSavedSetTimeInfo.win_handle);
    return gstEmWinListSavedSetTimeInfo.win_handle;
}

/**
  * @brief Close current win
  * @param  NULL
  * @retval NULL
  */
static void _EndWin(void)
{
    if(gstEmWinListSavedSetTimeInfo.win_handle == 0)
    {
        return;
    }
    WM_DeleteWindow(gstEmWinListSavedSetTimeInfo.win_handle);
    _Del();
    GUI_EnableAlpha(0);
    gstEmWinListSavedSetTimeInfo.win_handle = 0;
}


/**
  * @brief  当前界面参数初始化
  * @param  NULL
  * @retval NULL
  */
static void _Init()
{
    uint8_t i = 0 ;
    EmKeyDefaultInfoSet();
#if 0 
    /*底部文本的参数*/
    _gstPressTextPara.p_content = _garrPressBuf;
    _gstPressTextPara.text_color = 0x00CDF100;
    _gstPressTextPara.p_text_font = EM_TEXT_FONT_7;

    /*底部对勾的参数*/
    _gstImgTickPara.mem_img = GUI_ALLOC_AllocZero(EM_SAVAED_SET_TIME_ICON_TICK_SIZE);
    _gstImgTickPara.p_img_buf = GUI_ALLOC_h2p(_gstImgTickPara.mem_img);
    _gstImgTickPara.img_size = EM_SAVAED_SET_TIME_ICON_TICK_SIZE;
    SpiFlashReadData(EM_ICON_TICK_ADDR, _gstImgTickPara.p_img_buf, _gstImgTickPara.img_size);
    /*保存文本参数*/
    _gstSaveTextPara.p_content = _garrSaveBuf;
    _gstSaveTextPara.text_color = 0x00FFFFFF;
    _gstSaveTextPara.p_text_font = EM_TEXT_FONT_5;
#endif
    for(i = 0; i < PEAK_DIGIT_NUMS; i++)
    {
        _garrNumTextPara[i].p_text_font = EM_TEXT_FONT_5;
        _garrNumTextPara[i].text_color = 0x00FFFFFF;
        _garrNumTextPara[i].p_content = _garrSavedSetTimeNumTextBuf[i];
//      _garrNumTextPara[i].digit_value = 0;
//      sprintf(_garrNumTextPara[i].p_content, "%1d", (int)_garrNumTextPara[i].digit_value);
    }

//    gstCollectParaData.collect_data.peak_load_shift_time.chg_start_time1 = BigLittleSwap32(gstCollectParaData.collect_data.peak_load_shift_time.chg_start_time1);
//    gstCollectParaData.collect_data.peak_load_shift_time.chg_end_time1 = BigLittleSwap32(gstCollectParaData.collect_data.peak_load_shift_time.chg_end_time1);

    _SetPeakTime1ChrgStartTimeNumber(0,     gstCollectInfo.maincard_data.peak_time_info.chg_start_time1);
    _SetPeakTime1ChrgEndTimeNumber(0,       gstCollectInfo.maincard_data.peak_time_info.chg_end_time1);
    _SetPeakTime1DischrgStartTimeNumber(0,  gstCollectInfo.maincard_data.peak_time_info.dsg_start_time1);
    _SetPeakTime1DischrgEndTimeNumber(0,    gstCollectInfo.maincard_data.peak_time_info.dsg_end_time1);
    _SetPeakTime2ChrgStartTimeNumber(0,     gstCollectInfo.maincard_data.peak_time_info.chg_start_time2);
    _SetPeakTime2ChrgEndTimeNumber(0,       gstCollectInfo.maincard_data.peak_time_info.chg_end_time2);
    _SetPeakTime2DischrgStartTimeNumber(0,  gstCollectInfo.maincard_data.peak_time_info.dsg_start_time2);
    _SetPeakTime2DischrgEndTimeNumber(0,    gstCollectInfo.maincard_data.peak_time_info.dsg_end_time2);


    /*标题相关参数*/
    _gstTitleTextPara.p_content = _garrTitleBuf;
    _gstTitleTextPara.text_color = 0x00EEEEEE;
    _gstTitleTextPara.p_text_font = EM_TEXT_FONT_7;

    _gstEmListSavedSetTimeInfoPara.list_start_id = WIN_LIST_SAVED_SET_TIME_INFO_ID + 0x60;
    _gstEmListSavedSetTimeInfoPara.list_num = 8;
    _gstEmListSavedSetTimeInfoPara.list_display_num = 8;

    if(_gstEmListSavedSetTimeInfoPara.list_sel > _gstEmListSavedSetTimeInfoPara.list_num - 1)
    {
        _gstEmListSavedSetTimeInfoPara.list_sel = 0;
    }
    _gstEmListSavedSetTimeInfoPara.list_bk_color = 0x00787878;
    /*List选中框*/
    _gstEmListSavedSetTimeInfoPara.selected_r = 10;

    /*4个子列表*/
    _gstEmListSavedSetTimeInfoPara.sub_region[0].x_pos = 20;
    _gstEmListSavedSetTimeInfoPara.sub_region[0].y_pos = 38;
    _gstEmListSavedSetTimeInfoPara.sub_region[0].x_size = 440;
    _gstEmListSavedSetTimeInfoPara.sub_region[0].y_size = 117;
    _gstEmListSavedSetTimeInfoPara.sub_region[1].x_pos = 20;
    _gstEmListSavedSetTimeInfoPara.sub_region[1].y_pos = 38;
    _gstEmListSavedSetTimeInfoPara.sub_region[1].x_size = 440;
    _gstEmListSavedSetTimeInfoPara.sub_region[1].y_size = 117;
    _gstEmListSavedSetTimeInfoPara.sub_region[2].x_pos = 20;
    _gstEmListSavedSetTimeInfoPara.sub_region[2].y_pos = 38;
    _gstEmListSavedSetTimeInfoPara.sub_region[2].x_size = 440;
    _gstEmListSavedSetTimeInfoPara.sub_region[2].y_size = 117;
    _gstEmListSavedSetTimeInfoPara.sub_region[3].x_pos = 20;
    _gstEmListSavedSetTimeInfoPara.sub_region[3].y_pos = 38;
    _gstEmListSavedSetTimeInfoPara.sub_region[3].x_size = 440;
    _gstEmListSavedSetTimeInfoPara.sub_region[3].y_size = 117;
    _gstEmListSavedSetTimeInfoPara.sub_region[4].x_pos = 20;
    _gstEmListSavedSetTimeInfoPara.sub_region[4].y_pos = 163;
    _gstEmListSavedSetTimeInfoPara.sub_region[4].x_size = 440;
    _gstEmListSavedSetTimeInfoPara.sub_region[4].y_size = 117;
    _gstEmListSavedSetTimeInfoPara.sub_region[5].x_pos = 20;
    _gstEmListSavedSetTimeInfoPara.sub_region[5].y_pos = 163;
    _gstEmListSavedSetTimeInfoPara.sub_region[5].x_size = 440;
    _gstEmListSavedSetTimeInfoPara.sub_region[5].y_size = 117;
    _gstEmListSavedSetTimeInfoPara.sub_region[6].x_pos = 20;
    _gstEmListSavedSetTimeInfoPara.sub_region[6].y_pos = 163;
    _gstEmListSavedSetTimeInfoPara.sub_region[6].x_size = 440;
    _gstEmListSavedSetTimeInfoPara.sub_region[7].y_size = 117;
    _gstEmListSavedSetTimeInfoPara.sub_region[7].x_pos = 20;
    _gstEmListSavedSetTimeInfoPara.sub_region[7].y_pos = 163;
    _gstEmListSavedSetTimeInfoPara.sub_region[7].x_size = 440;
    _gstEmListSavedSetTimeInfoPara.sub_region[7].y_size = 117;


    /*List控件*/

    //第一行左边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][0].x_relative_pos = 73;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][0].y_relative_pos = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][0].x_size = 186;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][0].y_size = 17;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoLeftTextBuf[0];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[0][0].sub_widget.text_para.p_content, "Charge Start-time");
    //第一行右边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][0].x_relative_pos = 390;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][0].y_relative_pos = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][0].x_size = 5;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][0].y_size = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoRightTextBuf[0];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[1][0].sub_widget.text_para.p_content, ":");
    //第二行左边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][1].x_relative_pos = 73;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][1].y_relative_pos = 37;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][1].x_size = 186;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][1].y_size = 17;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoLeftTextBuf[1];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[0][1].sub_widget.text_para.p_content, "Charge End-time");
    //第二行右边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][1].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][1].x_relative_pos = 390;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][1].y_relative_pos = 38;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][1].x_size = 5;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][1].y_size = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoRightTextBuf[1];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][1].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[1][1].sub_widget.text_para.p_content, ":");
    //第三行左边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][2].x_relative_pos = 73;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][2].y_relative_pos = 67;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][2].x_size = 186;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][2].y_size = 17;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoLeftTextBuf[2];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[0][2].sub_widget.text_para.p_content, "Discharge Start-time");
    //第三行右边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][2].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][2].x_relative_pos = 390;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][2].y_relative_pos = 64;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][2].x_size = 5;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][2].y_size = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoRightTextBuf[2];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][2].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[1][2].sub_widget.text_para.p_content, ":");
    //第四行左边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][3].x_relative_pos = 73;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][3].y_relative_pos = 91;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][3].x_size = 186;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][3].y_size = 17;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoLeftTextBuf[3];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[0][3].sub_widget.text_para.p_content, "Discharge End-time");
    //第四行右边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][3].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][3].x_relative_pos = 390;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][3].y_relative_pos = 92;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][3].x_size = 5;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][3].y_size = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoRightTextBuf[3];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][3].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[1][3].sub_widget.text_para.p_content, ":");
    //第五行左边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][4].x_relative_pos = 73;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][4].y_relative_pos = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][4].x_size = 186;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][4].y_size = 17;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoLeftTextBuf[4];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[0][4].sub_widget.text_para.p_content, "Charge Start-time");
    //第五行右边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][4].x_relative_pos = 390;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][4].y_relative_pos = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][4].x_size = 5;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][4].y_size = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoRightTextBuf[4];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[1][4].sub_widget.text_para.p_content, ":");

    //第六行左边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][5].x_relative_pos = 73;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][5].y_relative_pos = 37;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][5].x_size = 186;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][5].y_size = 17;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoLeftTextBuf[5];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[0][5].sub_widget.text_para.p_content, "Charge End-time");
    //第六行右边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][5].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][5].x_relative_pos = 390;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][5].y_relative_pos = 38;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][5].x_size = 5;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][5].y_size = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoRightTextBuf[5];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][5].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[1][5].sub_widget.text_para.p_content, ":");
    //第七行左边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][6].x_relative_pos = 73;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][6].y_relative_pos = 64;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][6].x_size = 186;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][6].y_size = 17;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoLeftTextBuf[6];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[0][6].sub_widget.text_para.p_content, "Discharge Start-time");
    //第七行右边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][6].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][6].x_relative_pos = 390;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][6].y_relative_pos = 64;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][6].x_size = 5;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][6].y_size = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoRightTextBuf[6];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][6].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[1][6].sub_widget.text_para.p_content, ":");
    //第八行左边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][7].x_relative_pos = 73;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][7].y_relative_pos = 91;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][7].x_size = 186;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][7].y_size = 17;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoLeftTextBuf[7];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[0][7].sub_widget.text_para.p_content, "Discharge End-time");
    //第八行右边
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][7].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][7].x_relative_pos = 390;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][7].y_relative_pos = 90;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][7].x_size = 5;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][7].y_size = 10;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoRightTextBuf[7];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][7].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[1][7].sub_widget.text_para.p_content, ":");

    //Time1标题
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][0].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][0].x_relative_pos = 3;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][0].y_relative_pos = 35;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][0].x_size = 50;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][0].y_size = 31;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoHeadTextBuf[0];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][0].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[2][0].sub_widget.text_para.p_content, "Time\n1    ");
    //Time2标题
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][4].widget_type = EM_WIDGET_TYPE_TEXT;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][4].x_relative_pos = 3;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][4].y_relative_pos = 35;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][4].x_size = 50;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][4].y_size = 31;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][4].sub_widget.text_para.p_content = _garrListSavedSetTimeInfoHeadTextBuf[1];
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][4].sub_widget.text_para.text_color = 0x00FFFFFF;
    _gstEmListSavedSetTimeInfoPara.sub_widget_para[2][4].sub_widget.text_para.p_text_font = EM_TEXT_FONT_5;
    strcpy(_gstEmListSavedSetTimeInfoPara.sub_widget_para[2][4].sub_widget.text_para.p_content, "Time\n2    ");
}

/**
  * @brief  当前界面参数反初始化
  * @param  NULL
  * @retval NULL
  */
static void _Del()
{
    GUI_ALLOC_Free(_gstImgTickPara.mem_img);
}


/**
  * @brief  当前界面轮询处理
  * @param  NULL
  * @retval NULL
  */
static void _Polling()
{
    if( _gstUiCtx.chg_start_time1 != gstCollectInfo.maincard_data.peak_time_info.chg_start_time1)
    {
        _gstUiCtx.chg_start_time1    = gstCollectInfo.maincard_data.peak_time_info.chg_start_time1;
        _SetPeakTime1ChrgStartTimeNumber(1, _gstUiCtx.chg_start_time1);
    }

    if(_gstUiCtx.chg_end_time1 != gstCollectInfo.maincard_data.peak_time_info.chg_end_time1 )
    {
        _gstUiCtx.chg_end_time1 = gstCollectInfo.maincard_data.peak_time_info.chg_end_time1;
        _SetPeakTime1ChrgEndTimeNumber(1, _gstUiCtx.chg_end_time1);

    }
    if(_gstUiCtx.dsg_start_time1 != gstCollectInfo.maincard_data.peak_time_info.dsg_start_time1 )
    {
        _gstUiCtx.dsg_start_time1 = gstCollectInfo.maincard_data.peak_time_info.dsg_start_time1;
        _SetPeakTime1DischrgStartTimeNumber(1, _gstUiCtx.dsg_start_time1);

    }
    if(_gstUiCtx.dsg_end_time1   != gstCollectInfo.maincard_data.peak_time_info.dsg_end_time1)
    {
        _gstUiCtx.dsg_end_time1 = gstCollectInfo.maincard_data.peak_time_info.dsg_end_time1;
        _SetPeakTime1DischrgEndTimeNumber(1, _gstUiCtx.dsg_end_time1);
    }


    if( _gstUiCtx.chg_start_time2 != gstCollectInfo.maincard_data.peak_time_info.chg_start_time2 )
    {
        _gstUiCtx.chg_start_time2 = gstCollectInfo.maincard_data.peak_time_info.chg_start_time2;
        _SetPeakTime2ChrgStartTimeNumber(1, _gstUiCtx.chg_start_time2 );
    }
    if(_gstUiCtx.chg_end_time2 != gstCollectInfo.maincard_data.peak_time_info.chg_end_time2 )
    {
        _gstUiCtx.chg_end_time2 = gstCollectInfo.maincard_data.peak_time_info.chg_end_time2;
        _SetPeakTime2ChrgEndTimeNumber(1, _gstUiCtx.chg_end_time2);
    }
    if(_gstUiCtx.dsg_start_time2 != gstCollectInfo.maincard_data.peak_time_info.dsg_start_time2 )
    {
        _gstUiCtx.dsg_start_time2 = gstCollectInfo.maincard_data.peak_time_info.dsg_start_time2;
        _SetPeakTime2DischrgStartTimeNumber(1, _gstUiCtx.dsg_start_time2);

    }
    if(_gstUiCtx.dsg_end_time2 != gstCollectInfo.maincard_data.peak_time_info.dsg_end_time2)
    {
        _gstUiCtx.dsg_end_time2 = gstCollectInfo.maincard_data.peak_time_info.dsg_end_time2;
        _SetPeakTime2DischrgEndTimeNumber(1, _gstUiCtx.dsg_end_time2);
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

    if (event == EN_KEY_SRC_RIGHT)
    {
        if (_gstUiCtx.select >= PEAK_DIGIT_NUMS)
        {
            return;
        }
//      _gstUiCtx.num[_gstUiCtx.select] = abs(10 + (--(_gstUiCtx.num[_gstUiCtx.select])))%10;

        //设置小时十位
        //设置充电开始1 小时十位
        if(_gstUiCtx.select == 0)
        {
            _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 2 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[1] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 1 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电结束1小时十位
        if(_gstUiCtx.select == 4)
        {
            _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 2 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[5] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 1 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电开始1 小时十位
        if(_gstUiCtx.select == 8)
        {
            _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 2 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[9] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 1 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电结束1 小时十位
        if(_gstUiCtx.select == 12)
        {
            _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 2 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[13] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 1 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电开始2 小时十位
        if(_gstUiCtx.select == 16)
        {
            _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 2 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[17] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 1 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电结束2 小时十位
        if(_gstUiCtx.select == 20)
        {
            _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 2 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[21] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 1 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电开始2 小时十位
        if(_gstUiCtx.select == 24)
        {
            _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 2 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[25] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 1 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电结束2 小时十位
        if(_gstUiCtx.select == 28)
        {
            _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 2 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[29] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 1 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }

        //设置充电开始1  小时个位
        if(_gstUiCtx.select == 1)
        {
            if(_gstUiCtx.num[0] == 0 || _gstUiCtx.num[0] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 9 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 3 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电结束1  小时个位
        if(_gstUiCtx.select == 5)
        {
            if(_gstUiCtx.num[4] == 0 || _gstUiCtx.num[4] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 9 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 3 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电开始1  小时个位
        if(_gstUiCtx.select == 9)
        {
            if(_gstUiCtx.num[8] == 0 || _gstUiCtx.num[8] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 9 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 3 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电结束1  小时个位
        if(_gstUiCtx.select == 13)
        {
            if(_gstUiCtx.num[12] == 0 || _gstUiCtx.num[12] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 9 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 3 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电开始2  小时个位
        if(_gstUiCtx.select == 17)
        {
            if(_gstUiCtx.num[16] == 0 || _gstUiCtx.num[16] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 9 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 3 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电结束2  小时个位
        if(_gstUiCtx.select == 21)
        {
            if(_gstUiCtx.num[20] == 0 || _gstUiCtx.num[20] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 9 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 3 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电开始2  小时个位
        if(_gstUiCtx.select == 25)
        {
            if(_gstUiCtx.num[24] == 0 || _gstUiCtx.num[24] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 9 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 3 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电结束2 小时个位
        if(_gstUiCtx.select == 29)
        {
            if(_gstUiCtx.num[28] == 0 || _gstUiCtx.num[28] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 9 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 3 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }

        //设置分钟的十位
        if(_gstUiCtx.select == 2 || _gstUiCtx.select == 6 || _gstUiCtx.select == 10 || _gstUiCtx.select == 14 ||
                _gstUiCtx.select == 18 || _gstUiCtx.select == 22 || _gstUiCtx.select == 26 || _gstUiCtx.select == 30)
        {
            _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 5 : _gstUiCtx.num[_gstUiCtx.select];
        }
        //设置分钟的个位
        else if(_gstUiCtx.select == 3 || _gstUiCtx.select == 7 || _gstUiCtx.select == 11 || _gstUiCtx.select == 15 ||
                _gstUiCtx.select == 19 || _gstUiCtx.select == 23 || _gstUiCtx.select == 27 || _gstUiCtx.select == 31)
        {
            _gstUiCtx.num[_gstUiCtx.select] = --_gstUiCtx.num[_gstUiCtx.select] < 0 ? 9 : _gstUiCtx.num[_gstUiCtx.select];
        }

        _SetSavedPeakTimeNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        return;
    }
    else if (event == EN_KEY_SRC_LEFT)
    {
        if (_gstUiCtx.select >= PEAK_DIGIT_NUMS)
        {
            return;
        }
//      _gstUiCtx.num[_gstUiCtx.select] = (++(_gstUiCtx.num[_gstUiCtx.select]))%10;
        //设置充电开始1 小时十位
        if(_gstUiCtx.select == 0)
        {
            _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 2 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[1] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 1 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电结束1小时十位
        if(_gstUiCtx.select == 4)
        {
            _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 2 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[5] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 1 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电开始1 小时十位
        if(_gstUiCtx.select == 8)
        {
            _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 2 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[9] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 1 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电结束1 小时十位
        if(_gstUiCtx.select == 12)
        {
            _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 2 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[13] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 1 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电开始2 小时十位
        if(_gstUiCtx.select == 16)
        {
            _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 2 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[17] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 1 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电结束2 小时十位
        if(_gstUiCtx.select == 20)
        {
            _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 2 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[21] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 1 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电开始2 小时十位
        if(_gstUiCtx.select == 24)
        {
            _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 2 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[25] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 1 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电结束2 小时十位
        if(_gstUiCtx.select == 28)
        {
            _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 2 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            if(_gstUiCtx.num[_gstUiCtx.select] == 2 && _gstUiCtx.num[29] > 3)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 1 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }

        //设置充电开始1  小时个位
        if(_gstUiCtx.select == 1)
        {
            if(_gstUiCtx.num[0] == 0 || _gstUiCtx.num[0] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 9 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 3 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电结束1  小时个位
        if(_gstUiCtx.select == 5)
        {
            if(_gstUiCtx.num[4] == 0 || _gstUiCtx.num[4] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 9 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 3 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电开始1  小时个位
        if(_gstUiCtx.select == 9)
        {
            if(_gstUiCtx.num[8] == 0 || _gstUiCtx.num[8] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 9 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 3 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电结束1  小时个位
        if(_gstUiCtx.select == 13)
        {
            if(_gstUiCtx.num[12] == 0 || _gstUiCtx.num[12] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 9 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 3 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电开始2  小时个位
        if(_gstUiCtx.select == 17)
        {
            if(_gstUiCtx.num[16] == 0 || _gstUiCtx.num[16] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 9 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 3 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置充电结束2  小时个位
        if(_gstUiCtx.select == 21)
        {
            if(_gstUiCtx.num[20] == 0 || _gstUiCtx.num[20] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 9 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 3 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电开始2  小时个位
        if(_gstUiCtx.select == 25)
        {
            if(_gstUiCtx.num[24] == 0 || _gstUiCtx.num[24] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 9 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 3 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }
        //设置放电结束2 小时个位
        if(_gstUiCtx.select == 29)
        {
            if(_gstUiCtx.num[28] == 0 || _gstUiCtx.num[28] == 1)
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 9 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
            else
            {
                _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 3 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
            }
        }

        //设置分钟的十位
        if(_gstUiCtx.select == 2 || _gstUiCtx.select == 6 || _gstUiCtx.select == 10 || _gstUiCtx.select == 14 ||
                _gstUiCtx.select == 18 || _gstUiCtx.select == 22 || _gstUiCtx.select == 26 || _gstUiCtx.select == 30)
        {
            _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 5 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
        }
        //设置分钟的个位
        else if(_gstUiCtx.select == 3 || _gstUiCtx.select == 7 || _gstUiCtx.select == 11 || _gstUiCtx.select == 15 ||
                _gstUiCtx.select == 19 || _gstUiCtx.select == 23 || _gstUiCtx.select == 27 || _gstUiCtx.select == 31)
        {
            _gstUiCtx.num[_gstUiCtx.select] = ++(_gstUiCtx.num[_gstUiCtx.select]) > 9 ? 0 : _gstUiCtx.num[_gstUiCtx.select];
        }

        _SetSavedPeakTimeNumAdd(_gstUiCtx.select, _gstUiCtx.num[_gstUiCtx.select]);
        return;
    }
    else if (event == EN_KEY_SRC_CONFIRM)
    {
        _gstUiCtx.select = (++_gstUiCtx.select) % (PEAK_DIGIT_NUMS + 2);
        if (_gstUiCtx.select >= PEAK_DIGIT_NUMS)
        {
            _gstUiCtx.select = 0;
        }

        _SetSavedPeakTimeNumColor(_gstUiCtx.select);
        WM_Paint(gstEmWinListSavedSetTimeInfo.win_handle);
        if ((_gstUiCtx.select % (PEAK_DIGIT_NUMS / 2) == 0)
                || (_gstUiCtx.select == PEAK_DIGIT_NUMS))
        {
            WM_Paint(gstEmWinListSavedSetTimeInfo.win_handle);
        }

#if 0
        emWin_t *pnext = NULL;
        if (_gstUiCtx.select == 32)
        {
            extern int32_t SetPeakLoadBalacne(uint32_t on);
            SetPeakLoadBalacne(1);
            pnext = &gstEmWinListEquipOperaInfo;
        }
        if(pnext != NULL && pCurrentEmWin->win_handle != 0)
        {
            pCurrentEmWin->delete_win();
            pCurrentEmWin = pPrevEmWin;
            pCurrentEmWin->create_win();
        }
#endif
        return;
    }
    else if (event == EN_KEY_SRC_ESC)
    {
        int32_t time_complete_flag = 0;
        time_complete_flag = _CheckTimeIsCorrect();

        if(time_complete_flag)  //时间设置异常
        {
            //根据具体错误提示
            gstPopUpPeakShiftingTimeErrorWin.create_win();
            pCurrentEmWin = &gstPopUpPeakShiftingTimeErrorWin;
        }

        //所有时间都完整合理 下发削峰填谷时间并将工作模式切换到削峰填谷模式
        if(!time_complete_flag)
        {
            uint8_t arr_data_buff[256 + 8] = {0};
            uint8_t arr_data_buff_mode[256 + 8] = {0};
            uint16_t tmp = 0;

            _gstPeakLoadTime.chg_start_time1    = BigLittleSwap32(_gstPeakLoadTimeTemp.chg_start_time1);
            _gstPeakLoadTime.chg_end_time1      = BigLittleSwap32(_gstPeakLoadTimeTemp.chg_end_time1);
            _gstPeakLoadTime.dsg_start_time1    = BigLittleSwap32(_gstPeakLoadTimeTemp.dsg_start_time1);
            _gstPeakLoadTime.dsg_end_time1      = BigLittleSwap32(_gstPeakLoadTimeTemp.dsg_end_time1);
            _gstPeakLoadTime.chg_start_time2    = BigLittleSwap32(_gstPeakLoadTimeTemp.chg_start_time2);
            _gstPeakLoadTime.chg_end_time2      = BigLittleSwap32(_gstPeakLoadTimeTemp.chg_end_time2);
            _gstPeakLoadTime.dsg_start_time2    = BigLittleSwap32(_gstPeakLoadTimeTemp.dsg_start_time2);
            _gstPeakLoadTime.dsg_end_time2      = BigLittleSwap32(_gstPeakLoadTimeTemp.dsg_end_time2);

            //下发设置完成的削峰填谷时间
            ModbusWriteMultiRemain(arr_data_buff, 0x01, COLLECT_LOAD_PEAK_TIME_ADDR, 16, (uint8_t *)(&_gstPeakLoadTime));

            //下发工作模式为削峰填谷
            tmp = BigLittleSwap16(0x02);
            ModbusWriteSingleRemain(arr_data_buff_mode, 0x01, COLLECT_POWER_WORK_MODE_SECTION_ADDR, tmp);
            if(pCurrentEmWin->win_handle != 0)
            {
                emWin_t *pnext = NULL;
                if(p_msg->press_down_time >= gstEmKeyInfo[EN_KEY_SRC_ESC].long_single_time)
                {
                    pnext = &gstEmWinHomePage;
                }
                else
                {
                    pnext = &gstEmWinListWorkingModeInfo;
                }
                pPrevEmWin = pCurrentEmWin;
                pCurrentEmWin->delete_win();
                pCurrentEmWin = pnext;
                pCurrentEmWin->create_win();
            }
        }
        return;
    }
}


