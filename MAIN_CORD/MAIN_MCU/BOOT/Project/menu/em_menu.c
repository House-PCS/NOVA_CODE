/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        em_menu.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-22
  * @brief       基于emWin的菜单配置
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "em_menu.h"
#include "key_backlight_led.h"


uint32_t gRamFreeByte;//实时监控emWin分配的内存使用剩余情况


emWin_t *pCurrentEmWin;
emWin_t *pPrevEmWin;
emWin_t *pCurrentPopUpSubWin;

EmKeyInfo_t gEmKeyInfo[EN_KEY_SRC_NUM];

//GUI_LOGPALETTE      Palette565;/*565*/
//GUI_LOGPALETTE      Palette888;/*888*/
//GUI_BITMAP          Bitmap;

#define TTF_ITC_FONT_SIZE       (32 * 1024)

GUI_TTF_DATA  gCSITCAvantBoldData;
GUI_HMEM  hMemITCAvantBold;
uint8_t *pBufITCAvantBold;
GUI_TTF_CS  gCSITCAvantBold16;
GUI_FONT gFontITCAvantBold16;
GUI_TTF_CS  gCSITCAvantBold13;
GUI_FONT gFontITCAvantBold13;

GUI_TTF_DATA  gCSITCAvantDemiData;
GUI_HMEM  hMemITCAvantDemi;
uint8_t *pBufITCAvantDemi;
GUI_TTF_CS  gCSITCAvantDemi16;
GUI_FONT gFontITCAvantDemi16;

GUI_TTF_DATA  gCSITCAvantMdData;
GUI_HMEM  hMemITCAvantMd;
uint8_t *pBufITCAvantMd;
GUI_TTF_CS  gCSITCAvantMd16;
GUI_FONT gFontITCAvantMd16;


/**
  * @brief   字库导入到SDRAM
  * @param   None
  * @param   None
  * @return  None
  * @note    None
  */
static void _LoadFont()
{
#if 1
    /*字库读取*/
    hMemITCAvantBold = GUI_ALLOC_AllocZero(TTF_ITC_FONT_SIZE);
    pBufITCAvantBold = GUI_ALLOC_h2p(hMemITCAvantBold);
    SpiFlashReadData(EM_FONT_ITCAVANT_BOLD_ADDR, pBufITCAvantBold, TTF_ITC_FONT_SIZE);
    
    hMemITCAvantDemi = GUI_ALLOC_AllocZero(TTF_ITC_FONT_SIZE);
    pBufITCAvantDemi = GUI_ALLOC_h2p(hMemITCAvantDemi);
    SpiFlashReadData(EM_FONT_ITCAVANT_DEMI_ADDR, pBufITCAvantDemi, TTF_ITC_FONT_SIZE);
    
    hMemITCAvantMd = GUI_ALLOC_AllocZero(TTF_ITC_FONT_SIZE);
    pBufITCAvantMd = GUI_ALLOC_h2p(hMemITCAvantMd);
    SpiFlashReadData(EM_FONT_ITCAVANT_MD_ADDR, pBufITCAvantMd, TTF_ITC_FONT_SIZE);
    
    gCSITCAvantBoldData.pData = pBufITCAvantBold;
    gCSITCAvantBoldData.NumBytes = TTF_ITC_FONT_SIZE;
    
    gCSITCAvantDemiData.pData = pBufITCAvantDemi;
    gCSITCAvantDemiData.NumBytes = TTF_ITC_FONT_SIZE;
    
    gCSITCAvantMdData.pData = pBufITCAvantMd;
    gCSITCAvantMdData.NumBytes = TTF_ITC_FONT_SIZE;
    
    /*不同字号字体配置*/
    gCSITCAvantBold16.pTTF = &gCSITCAvantBoldData;
    gCSITCAvantBold16.PixelHeight = 16;
    gCSITCAvantBold16.FaceIndex = 0;
//    GUI_TTF_CreateFont(&gFontITCAvantBold16, &gCSITCAvantBold16);
    GUI_TTF_CreateFontAA(&gFontITCAvantBold16, &gCSITCAvantBold16);//抗锯齿，字体更好看，但更加消耗CPU性能
    
    gCSITCAvantBold13.pTTF = &gCSITCAvantBoldData;
    gCSITCAvantBold13.PixelHeight = 13;
    gCSITCAvantBold13.FaceIndex = 0;
    GUI_TTF_CreateFontAA(&gFontITCAvantBold13, &gCSITCAvantBold13);//抗锯齿，字体更好看，但更加消耗CPU性能
#endif 
}

/**
  * @brief   UI初始化
  * @param   None
  * @param   None
  * @return  None
  * @note    None
  */
void EmMenuInit()
{
    GUI_Init();
    GUI_UC_SetEncodeUTF8();
    GUI_SetFont(GUI_FONT_8X16);
    WM_SetCreateFlags(WM_CF_MEMDEV); /*创建新窗口时使用默认从存储器中读取*/
    WM_MOTION_Enable(0); /* 窗口是否可移动 */
    WM_MULTIBUF_Enable(1); /*启动vm自动多缓存*/
    GUI_SetBkColor(GUI_BLACK);
    GUI_Clear();
    GUI_SetColor(GUI_WHITE);
    GUI_SetTextMode(GUI_TM_TRANS);
    
    GUI_DispStringAt("Starting...", 0, 0);

    WINDOW_SetDefaultBkColor(GUI_TRANSPARENT);
    
    /*加载启动界面*/
    if(pCurrentEmWin == NULL)
    {
        pCurrentEmWin = &gEmWinStartup;
    }
    pCurrentEmWin->create_win();
    
    /*加载字库*/
    _LoadFont();
}

/**
  * @brief   检测是否有其他按键按下
  * @param   current_key:当前已按下的按键
  * @return  true表示有其他按键按下，false表示没有其他按键按下
  * @note    None
  */
static bool _KeyOtherDetect(EN_KEY_SRC current_key)
{
    uint8_t i;
    
    for(i = 0; i < EN_KEY_SRC_NUM; i++)
    {
        if(i != current_key)
        {
            if(DevKeyPressed(i) == true)
            {
                break;
            }
        }
    }
    if(i == EN_KEY_SRC_NUM)
    {
        return false;
    }
    else
    {
        return true;
    }
}


/**
  * @brief   按键处理
  * @param   None
  * @param   None
  * @return  None
  * @note    None
  */
static void _KeyValueGet()
{
    KeyNewMsg_t *p_msg;
    
    if(pCurrentEmWin == NULL)
    {
        return;
    }
    
    if(QueueGet(&gKeyMsgQueue, (void *)&p_msg) == true)
    {
        if(_KeyOtherDetect(p_msg->key_src) == true)//有多个按键按下，菜单不响应
        {
            return;
        }
        pCurrentEmWin->key_handler(p_msg);
    }
	
}

uint32_t gRamFreeByte;/*用于查看当前给emWin分配的内存还剩多少*/
uint32_t gLeastFreeByte = 0xFFFFFFFF;/*用于查看emWin控件的最大内存使用量*/

/**
  * @brief   UI绘制大轮询处理
  * @param   None
  * @param   None
  * @return  None
  * @note    None
  */
void MenuPolling()
{
    gRamFreeByte = GUI_ALLOC_GetNumFreeBytes();
    if(gLeastFreeByte >= gRamFreeByte)
    {
        gLeastFreeByte = gRamFreeByte;
    }
    _KeyValueGet();
    
    
    if(pCurrentEmWin->win_handle != 0)
    {
        GUI_Exec();
        pCurrentEmWin->polling();
    }
}


/**
  * @brief   界面切换
  * @param   None
  * @param   None
  * @return  None
  * @note    None
  */
void MenuSwitch(emWin_t *p_next_win)
{
    pCurrentEmWin->delete_win();
    
    pCurrentEmWin = p_next_win;
    pCurrentEmWin->create_win(); 
}



/**
  * @brief   按键功能处理
  * @param   p_msg:从底层传上来的按键消息
  * @return  -1表示无效，不执行，正值表示按键键值
  * @note    None
  */
int32_t EmGetKeyEvent(KeyNewMsg_t *p_msg)
{
    EN_KEY_SRC key_idx = p_msg->key_src;
    
    if(p_msg->press_value == EN_KEY_SRC_PRESSED_DOWN)
    {
        gEmKeyInfo[key_idx].status = EN_EM_KEY_PRESSED_DOWN;
    }
    else
    {
        gEmKeyInfo[key_idx].status = EN_EM_KEY_PRESSED_UP;
    }
    
    if(gEmKeyInfo[key_idx].key_func_valid == false)
    {
        if(p_msg->press_down_time == 0)//包含按键按下和抬起
        {
            gEmKeyInfo[key_idx].key_func_valid = true;
        }
        else
        {
            return -1;
        }
    }
    
    switch(gEmKeyInfo[key_idx].type)
    {
        case EN_EM_KEY_TRIGGER_TYPE_ONLY_SHORT_PRESS:
        {
            if(p_msg->press_value == EN_KEY_SRC_PRESSED_DOWN)
            {
                gEmKeyInfo[key_idx].key_func_valid = false;
                return key_idx;
            }
            break;
        }
        case EN_EM_KEY_TRIGGER_TYPE_LONG_CONTINUOUS_PRESS:
        {
            static uint8_t continuous_status[EN_KEY_SRC_NUM];
            static uint32_t continuous_time_count[EN_KEY_SRC_NUM];
            if(p_msg->press_value == EN_KEY_SRC_PRESSED_DOWN)
            {
                if(p_msg->press_down_time == 0)
                {
                    continuous_status[key_idx] = 0;
                    return key_idx;
                }
                else if(p_msg->press_down_time >= gEmKeyInfo[key_idx].long_continuous_start_time)
                {
                    if(continuous_status[key_idx] == 0)
                    {
                        continuous_status[key_idx] = 1;
                        continuous_time_count[key_idx] = p_msg->press_down_time;
                        return key_idx;
                    }
                }
                if(continuous_status[key_idx] == 1)
                {
                    if(p_msg->press_down_time - continuous_time_count[key_idx] >= gEmKeyInfo[key_idx].long_continuous_period)
                    {
                        continuous_time_count[key_idx] = p_msg->press_down_time;
                        return key_idx;
                    }
                }
            }
            else
            {
                continuous_status[key_idx] = 0;
                continuous_time_count[key_idx] = 0;
            }
            break;
        }
        case EN_EM_KEY_TRIGGER_TYPE_LONG_SINGLE_PRESS:
        {
            if(p_msg->press_value == EN_KEY_SRC_PRESSED_DOWN)
            {
                if(p_msg->press_down_time >= gEmKeyInfo[key_idx].long_single_time)
                {
                    gEmKeyInfo[key_idx].key_func_valid = false;
                    return key_idx;
                }
            }
            else
            {
                if(p_msg->press_down_time < gEmKeyInfo[key_idx].long_single_time)
                {
                    return key_idx;
                }
            }
            break;
        }
        default:break;
    }
    return -1;
}


/**
  * @brief   按键的默认功能定义
  * @param   None
  * @param   None
  * @return  None
  * @note    None
  */
void EmKeyDefaultInfoSet()
{
    /*按键上*/
    gEmKeyInfo[EN_KEY_SRC_LEFT].type = EN_EM_KEY_TRIGGER_TYPE_LONG_CONTINUOUS_PRESS;
    gEmKeyInfo[EN_KEY_SRC_LEFT].long_continuous_start_time = 1000;
    gEmKeyInfo[EN_KEY_SRC_LEFT].long_continuous_period = 200;
    gEmKeyInfo[EN_KEY_SRC_LEFT].key_func_valid = false;
    /*按键下*/
    gEmKeyInfo[EN_KEY_SRC_RIGHT].type = EN_EM_KEY_TRIGGER_TYPE_LONG_CONTINUOUS_PRESS;
    gEmKeyInfo[EN_KEY_SRC_RIGHT].long_continuous_start_time = 1000;
    gEmKeyInfo[EN_KEY_SRC_RIGHT].long_continuous_period = 200;
    gEmKeyInfo[EN_KEY_SRC_RIGHT].key_func_valid = false;
    /*按键OK*/
    gEmKeyInfo[EN_KEY_SRC_CONFIRM].type = EN_EM_KEY_TRIGGER_TYPE_ONLY_SHORT_PRESS;
    gEmKeyInfo[EN_KEY_SRC_CONFIRM].key_func_valid = false;
    /*按键ESC*/
    gEmKeyInfo[EN_KEY_SRC_ESC].type = EN_EM_KEY_TRIGGER_TYPE_LONG_SINGLE_PRESS;
    gEmKeyInfo[EN_KEY_SRC_ESC].long_single_time = 2000;
    gEmKeyInfo[EN_KEY_SRC_ESC].key_func_valid = false;
}





