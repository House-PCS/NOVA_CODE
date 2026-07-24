/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_led.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "dev_led.h"
#define  LED_TIMES              (100)
#define  LED_UPGRADE_TIMES      (200)
static LedInfo_t   _gLedInfo[LED_SRC_NUM];

static Gpio_HandleType          *pGpioHandle[LED_SRC_NUM];
static Timer_HandleType         *pTimerHandle[LED_SRC_NUM];

LedCartoonOverCallBack       LedCrrtoonOver;

static uint32_t led_times = LED_TIMES;


const   uint8_t SosTimeBuf[]={0x01,0x82,0x02,0x82,0x02,0x82,0x05,
                                   0x84,0x02,0x84,0x02,0x84,0x05,
                                   0x82,0x02,0x82,0x02,0x82,0x0C,
                                   0XFF};

/**
 * @brief  : MCU_LED，用于指示MCU运行状态
 * @para   : time_ms 间隔时间
 * @note   : for mb_output_card
 * @return : None
 */
void MCULedProcess(void)
{
    static uint32_t sn_time = 0;
    if(tim_difftime_from_now(sn_time) > led_times)
    {     
        sn_time = tim_get_systime();
        GpioBitToogle((&McuLedHandle));
//        PackVotageGet();
    }   
}


/**
 * @brief  : MCU_LED，用于指示MCU运行状态
 * @para   : time_ms 间隔时间
 * @note   : for mb_output_card
 * @return : None
 */
void LedTimeSetFunc(EN_LED_STATUS status)
{
    switch(status)
    {
        case LED_STATUS_IDLE    :
        {
            led_times = LED_TIMES;
            break;
        }  
        case LED_STATUS_NORMAL  :
        {
            led_times = LED_TIMES;
            break;
        }
        case LED_STATUS_UPGRADE :
        {
            led_times = LED_UPGRADE_TIMES;
            break;
        }    
    }
}


static void _LedOFF(EN_LED_SRC index,uint32_t data)
{
    uint8_t i=0;
    uint8_t j=0;

    if(_gLedInfo[index].led_dirver_mode == LED_GPIO)
    {
        if(_gLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            GpioBitReset(_gLedInfo[index].pGpio_Handle);
        }
        else
        {
            GpioBitSet(_gLedInfo[index].pGpio_Handle);
        }
    }
//    else if(_gLedInfo[index].led_dirver_mode == LED_HW_PWM)
//    {
//      
//    }else if(_gLedInfo[index].led_dirver_mode == LED_DRIVE_IC)
//    {
//        for(i =0;i<SK68XX_GROUP_NUM;i++)
//        {
//            for(j=0;j<SK68XX_UINT_LED_NUM;j++)
//            {
//                LedColorDataArr[i][j] = 0;
//            }
//        }
//        SetSk68xxLedClour((uint8_t *)&LedColorDataArr[0][0],(uint16_t)sizeof(LedColorDataArr));  
////        SetSk68xxLedClour(data);
//    }
}

static void _LedON(EN_LED_SRC index,uint32_t data)
{
    uint8_t i=0;
    uint8_t j=0;
    if(_gLedInfo[index].led_dirver_mode == LED_GPIO)
    {
        if(_gLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            GpioBitSet(_gLedInfo[index].pGpio_Handle);
        }
        else
        {
            GpioBitReset(_gLedInfo[index].pGpio_Handle);
        }
    }
//    else if(_gLedInfo[index].led_dirver_mode == LED_HW_PWM)
//    {
//  
//    }else if(_gLedInfo[index].led_dirver_mode == LED_DRIVE_IC)
//    {

//        for(i =0;i<SK68XX_GROUP_NUM;i++)
//        {
//            for(j=0;j<SK68XX_UINT_LED_NUM;j++)
//            {
//                LedColorDataArr[i][j] = data;
//            }
//        }
//        SetSk68xxLedClour((uint8_t *)&LedColorDataArr[0][0],(uint16_t)sizeof(LedColorDataArr));  
//    }
}

static void _LedGray(EN_LED_SRC index, uint8_t level)
{
    if(_gLedInfo[index].led_dirver_mode == LED_GPIO)
    {
        /*暂时不支持，后续通过定时翻转模拟PWM*/
    }
    else if(_gLedInfo[index].led_dirver_mode == LED_HW_PWM)
    {
        //暂时不支持
    }else if(_gLedInfo[index].led_dirver_mode == LED_DRIVE_IC)
    {
    }
}

static void _LedBreath(EN_LED_SRC index, uint8_t first_flag)
{
    static uint8_t level;/*0~100*/
    static uint32_t timecount = 0;
    static uint8_t up_down; /*0-UP,1-DOWN*/
    uint32_t period = _gLedInfo[index].led_status_period;
    uint32_t one_level_time = period / 2 / 100;
    
    if(first_flag == 1)/*计数初始化*/
    {
        up_down = 0;
        level = 0;
        timecount = tim_get_systime();
        _LedGray(index, level);
        return;
    }
    
    if((uint32_t)(tim_get_systime() - timecount) >= one_level_time)
    {
        timecount = tim_get_systime();
        _LedGray(index, level);
        if(up_down == 0)
        {
            level++;
            if(level >= 101)
            {
                level = 100;/*最亮时长占用两个时间单位*/
                up_down = 1;
            }
        }
        else if(up_down == 1)
        {
            level--;
            if(level == 0)
            {
                up_down = 0;
            }
        }
    }
}

void AtmosLampSet(EN_LED_STATUS status,  uint8_t brightness)
{
    _gLedInfo[LED_SRC_ATMOS].led_status = status;
    _gLedInfo[LED_SRC_ATMOS].led_brightness = brightness;
}

void AtRunLedLampSet(EN_LED_STATUS status,  uint8_t brightness)
{
    _gLedInfo[LED_RUN_LED].led_status = status;
    _gLedInfo[LED_RUN_LED].led_brightness = brightness;        
}

void AtLed1LampSet(EN_LED_STATUS status,  uint8_t brightness)
{
    _gLedInfo[LED_Run1].led_status = status;
    _gLedInfo[LED_Run1].led_brightness = brightness;        
}

void AtLed2LampSet(EN_LED_STATUS status,  uint8_t brightness)
{
    _gLedInfo[LED_Run2].led_status = status;
    _gLedInfo[LED_Run2].led_brightness = brightness;        
}

void AtLed3LampSet(EN_LED_STATUS status,  uint8_t brightness)
{
    _gLedInfo[LED_Run3].led_status = status;
    _gLedInfo[LED_Run3].led_brightness = brightness;        
}

void AtLed4LampSet(EN_LED_STATUS status,  uint8_t brightness)
{
    _gLedInfo[LED_Run4].led_status = status;
    _gLedInfo[LED_Run4].led_brightness = brightness;        
}

void AtLed5LampSet(EN_LED_STATUS status,  uint8_t brightness)
{
    _gLedInfo[LED_Run5].led_status = status;
    _gLedInfo[LED_Run5].led_brightness = brightness;        
}

void AtLed6LampSet(EN_LED_STATUS status,  uint8_t brightness)
{
    _gLedInfo[LED_Run6].led_status = status;
    _gLedInfo[LED_Run6].led_brightness = brightness;        
}

//void    RigisterLedScanCallBackFun(LedScanCbfunc fun)
//{
//    LedCallBackFun = fun;
//}
void LedInit()
{
    McuLedBspInit();
    RunLedEnInit();
    //    PowerSwitchLedInit();
    if(geBMSType == EN_BMS_A0)
    {

    }
    else if(geBMSType ==  EN_BMS_A1)
    {
//    _gLedInfo[LED_Run1].led_src = LED_Run1;
//    _gLedInfo[LED_Run1].led_dirver_mode = LED_GPIO;
//    _gLedInfo[LED_Run1].led_status = LED_STATUS_BLINK;
//    _gLedInfo[LED_Run1].led_status_period = 1000ul/LED_SCAN_TIME;//默认值
//    _gLedInfo[LED_Run1].led_active_level = LED_HW_ACTIVE_LOW;
//    _gLedInfo[LED_Run1].tim_ch = 0;
//    //    _gLedInfo[LED_Run1].pGpio_Handle = &RunLed1Handle;

//    _gLedInfo[LED_Run2].led_src = LED_Run2;
//    _gLedInfo[LED_Run2].led_dirver_mode = LED_GPIO;
//    _gLedInfo[LED_Run2].led_status = LED_STATUS_BLINK;
//    _gLedInfo[LED_Run2].led_status_period = 1000ul/LED_SCAN_TIME;//默认值
//    _gLedInfo[LED_Run2].led_active_level = LED_HW_ACTIVE_LOW;
//    _gLedInfo[LED_Run2].tim_ch = 0;
//    //    _gLedInfo[LED_Run2].pGpio_Handle = &RunLed2Handle;


//    _gLedInfo[LED_Run3].led_src = LED_Run3;
//    _gLedInfo[LED_Run3].led_dirver_mode = LED_GPIO;
//    _gLedInfo[LED_Run3].led_status = LED_STATUS_BLINK;
//    _gLedInfo[LED_Run3].led_status_period = 1000ul/LED_SCAN_TIME;//默认值
//    _gLedInfo[LED_Run3].led_active_level = LED_HW_ACTIVE_LOW;
//    _gLedInfo[LED_Run3].tim_ch = 0;
//    //    _gLedInfo[LED_Run3].pGpio_Handle = &RunLed3Handle;


//    _gLedInfo[LED_Run4].led_src = LED_Run4;
//    _gLedInfo[LED_Run4].led_dirver_mode = LED_GPIO;
//    _gLedInfo[LED_Run4].led_status = LED_STATUS_BLINK;
//    _gLedInfo[LED_Run4].led_status_period = 1000ul/LED_SCAN_TIME;//默认值
//    _gLedInfo[LED_Run4].led_active_level = LED_HW_ACTIVE_LOW;
//    _gLedInfo[LED_Run4].tim_ch = 0;
//    //    _gLedInfo[LED_Run4].pGpio_Handle = &RunLed4Handle;

//    _gLedInfo[LED_Run5].led_src = LED_Run5;
//    _gLedInfo[LED_Run5].led_dirver_mode = LED_GPIO;
//    _gLedInfo[LED_Run5].led_status = LED_STATUS_BLINK;
//    _gLedInfo[LED_Run5].led_status_period = 1000ul/LED_SCAN_TIME;//默认值
//    _gLedInfo[LED_Run5].led_active_level = LED_HW_ACTIVE_LOW;
//    _gLedInfo[LED_Run5].tim_ch = 0;
//    //	_gLedInfo[LED_Run5].pGpio_Handle = &RunLed5Handle;

//    _gLedInfo[LED_Run6].led_src = LED_Run6;
//    _gLedInfo[LED_Run6].led_dirver_mode = LED_GPIO;
//    _gLedInfo[LED_Run6].led_status = LED_STATUS_BLINK;
//    _gLedInfo[LED_Run6].led_status_period = 1000ul/LED_SCAN_TIME;//默认值
//    _gLedInfo[LED_Run6].led_active_level = LED_HW_ACTIVE_LOW;
//    _gLedInfo[LED_Run6].tim_ch = 0;
//    //	_gLedInfo[LED_Run6].pGpio_Handle = &RunLed6Handle;

//    _gLedInfo[LED_RUN_LED].led_src = LED_RUN_LED;
//    _gLedInfo[LED_RUN_LED].led_dirver_mode = LED_GPIO;
//    _gLedInfo[LED_RUN_LED].led_status = LED_STATUS_BLINK;
//    _gLedInfo[LED_RUN_LED].led_status_period = 1000;//默认值
//    _gLedInfo[LED_RUN_LED].led_active_level = LED_HW_ACTIVE_LOW;
//    _gLedInfo[LED_RUN_LED].tim_ch = 0;
//    //    _gLedInfo[LED_RUN_LED].pGpio_Handle = &McuLedHandle;

//    _gLedInfo[LED_SRC_ATMOS].led_src = LED_SRC_ATMOS;
//    _gLedInfo[LED_SRC_ATMOS].led_dirver_mode = LED_DRIVE_IC;
//    _gLedInfo[LED_SRC_ATMOS].led_status = LED_STATUS_IDLE;
//    _gLedInfo[LED_SRC_ATMOS].led_status_period = 1000;//默认值
//    _gLedInfo[LED_SRC_ATMOS].led_active_level = LED_HW_ACTIVE_HIGH;
//    _gLedInfo[LED_SRC_ATMOS].tim_ch = 0;

//    //    Timer7BspInit();
//    _LedOFF(LED_Run1,0);
//    _LedOFF(LED_Run2,0);
//    _LedOFF(LED_Run3,0);
//    _LedOFF(LED_Run3,0);
//    _LedOFF(LED_Run4,0);
//    _LedOFF(LED_Run5,0);
//    _LedOFF(LED_Run6,0);	
//    _LedOFF(LED_SRC_ATMOS,0);
    //    Sk68xx_Inial(&LedColorDataArr[0][0]);
    }
    //    RigisterLedScanCallBackFun(LedProcess);
    //    _gLedInfo[LED_SRC_POWER_WHITE].led_status = LED_STATUS_BLINK;
}

//uint8_t    cal_fun(uint8_t data1,uint8_t data2,uint8_t data3)
//{
//    uint16_t    temp=0;
//    temp = data1;
//    temp *=  data2;
//    temp /= data3;
//    return  (uint8_t)temp;    
//}

////Sk68xx_tu display_data;
//uint32_t  LedBreathData[LED_TYPE_MAX]={0X00007F00,0X007F0000,0X007F7F00,0X7F000000,0X7F007F00,0X7F7F7F00};
//                                    //blue          red                    blue
//uint32_t  clour_test =0;
//static  uint8_t  LedBreathTypeDptr=0;

//void LedProcess(void)
//{
////25ms 回调

//    uint8_t i;
//    static  uint8_t breath_direct=0;
////    Sk68xx_tu display_breath_data; 
//    static  uint8_t sos_time_cnt=0;
//    static  uint8_t cartoon_time_cnt=0;
//    uint32_t  temp=0;
////    display_data.word_value = LedBreathData[LedBreathTypeDptr];
//    for(i = 0; i < LED_SRC_NUM; i++)
//    {
//        switch(_gLedInfo[i].led_status)
//        {
//            case LED_STATUS_IDLE:
//            {
//                _LedOFF((EN_LED_SRC)i,0);
//                _gLedInfo[i].sos_dptr = 0;  
//                if( i == LED_SRC_ATMOS)
//                {
////                    memset((uint8_t *)&LedColorDataArr[0][0],display_breath_data.word_value,sizeof(LedColorDataArr));
////                    SetSk68xxLedClour((uint8_t *)&LedColorDataArr[0][0],(uint16_t)sizeof(LedColorDataArr));                    
//                }

//                break;
//            }
//            case LED_STATUS_OFF:
//            {
//                _LedOFF((EN_LED_SRC)i,0);
//                _gLedInfo[i].sos_dptr = 0;
//                if( i == LED_SRC_ATMOS)
//                {
////                    memset((uint8_t *)&LedColorDataArr[0][0],display_breath_data.word_value,sizeof(LedColorDataArr));
////                    SetSk68xxLedClour((uint8_t *)&LedColorDataArr[0][0],(uint16_t)sizeof(LedColorDataArr));                    
//                }
//                break;
//            }
//            case LED_STATUS_ON:
//            {
////                _LedON(i,display_data.word_value);
////                _LedON(i,MultiSigleLedClour.word_value);

//                
//                _gLedInfo[i].sos_dptr = 0;
//                break;
//            }
//            case LED_STATUS_BLINK:
//            {//25ms
//                
//                 _gLedInfo[i].led_status_period = LED_BLINK_T;
//                 if(_gLedInfo[i].led_time_count ++ >= _gLedInfo[i].led_status_period)
//                 {
//                    _gLedInfo[i].led_time_count = 0;

//                 }
//                if(_gLedInfo[i].led_time_count <=(_gLedInfo[i].led_status_period >>1))
//                {
//                    _LedOFF((EN_LED_SRC)i,0);
//                }else
//                {
////                    _LedON(i,display_data.word_value);
////                    _LedON(i,MultiSigleLedClour.word_value);
//                }
//                _gLedInfo[i].sos_dptr = 0;
//                break;
//            }

//            case LED_STATUS_BREATHING_SINGLE:
//            {

//                _gLedInfo[i].led_status_period = (LED_BREATH_T>>1);
//                if(breath_direct ==0)
//                {
//                    if(_gLedInfo[i].led_time_count++ >= _gLedInfo[i].led_status_period)
//                    {
//                        _gLedInfo[i].led_time_count = _gLedInfo[i].led_status_period;
//                        breath_direct =1;
//                    }
//                }else
//                {
//                    if(_gLedInfo[i].led_time_count>0)
//                    {
//                        _gLedInfo[i].led_time_count--;
//                    }
//                    if(_gLedInfo[i].led_time_count ==0)
//                    {
//                        breath_direct =0;
////                        LedBreathTypeDptr++;
////                        if(LedBreathTypeDptr >= LED_TYPE_MAX)
////                        {
////                            LedBreathTypeDptr = 0;
////                        }
//                    }
//                }
////                temp = display_data.byte_value.rgb_bdata;
//                temp *= _gLedInfo[i].led_time_count;
//                temp /= _gLedInfo[i].led_status_period;
////                display_data.word_value = MultiSigleLedClour.word_value;   //设置的颜色
////                display_breath_data.byte_value.rgb_rdata  = cal_fun(display_data.byte_value.rgb_rdata,_gLedInfo[i].led_time_count,_gLedInfo[i].led_status_period);
////                display_breath_data.byte_value.rgb_gdata  = cal_fun(display_data.byte_value.rgb_gdata,_gLedInfo[i].led_time_count,_gLedInfo[i].led_status_period);
////                display_breath_data.byte_value.rgb_bdata  = cal_fun(display_data.byte_value.rgb_bdata,_gLedInfo[i].led_time_count,_gLedInfo[i].led_status_period);
////                _LedON(i,display_breath_data.word_value);
//                _gLedInfo[i].sos_dptr = 0;
//                break;
//            }
//            case LED_STATUS_SOS:
//            {     //25mS回调  
//                if(SosTimeBuf[_gLedInfo[i].sos_dptr]==0xFF)
//                {
//                    _gLedInfo[i].sos_dptr =0;
//                }
//                if(SosTimeBuf[_gLedInfo[i].sos_dptr]&0x80)
//                {
//                    _LedON((EN_LED_SRC)i,0X7F7F7F00);    //0X7F7F7F00    _LedON(i,display_data.word_value); 
//                }else
//                {
//                    _LedOFF((EN_LED_SRC)i,0);
//                }
//                if(sos_time_cnt++ >= ((SosTimeBuf[_gLedInfo[i].sos_dptr]&0x7f)*LED_SOS_TIME_BASE))
//                {
//                    sos_time_cnt = 0;
//                    _gLedInfo[i].sos_dptr++;  
//                }  
//                break;
//            }
//            case LED_STATUS_BREATHING_MULTI:
//            {

//                _gLedInfo[i].led_status_period = (LED_BREATH_T>>1);
//                if(breath_direct ==0)
//                {
//                    if(_gLedInfo[i].led_time_count++ >= _gLedInfo[i].led_status_period)
//                    {
//                        _gLedInfo[i].led_time_count = _gLedInfo[i].led_status_period;
//                        breath_direct =1;
//                    }
//                }else
//                {
//                    if(_gLedInfo[i].led_time_count>0)
//                    {
//                        _gLedInfo[i].led_time_count--;
//                    }
//                    if(_gLedInfo[i].led_time_count ==0)
//                    {
//                        breath_direct =0;
//                        LedBreathTypeDptr++;
//                        if(LedBreathTypeDptr >= LED_TYPE_MAX)
//                        {
//                            LedBreathTypeDptr = 0;
//                        }
//                    }
//                }
////                temp = display_data.byte_value.rgb_bdata;
//                temp *= _gLedInfo[i].led_time_count;
//                temp /= _gLedInfo[i].led_status_period;
////                display_breath_data.byte_value.rgb_rdata  = cal_fun(display_data.byte_value.rgb_rdata,_gLedInfo[i].led_time_count,_gLedInfo[i].led_status_period);
////                display_breath_data.byte_value.rgb_gdata  = cal_fun(display_data.byte_value.rgb_gdata,_gLedInfo[i].led_time_count,_gLedInfo[i].led_status_period);
////                display_breath_data.byte_value.rgb_bdata  = cal_fun(display_data.byte_value.rgb_bdata,_gLedInfo[i].led_time_count,_gLedInfo[i].led_status_period);
////                _LedON(i,display_breath_data.word_value);
//                _gLedInfo[i].sos_dptr = 0;
//                break;
//            }

////            case LED_STATUS_TURN_0N:
////            {
////25

////                static  uint8_t  cartoon_mode =0;
////                static  uint8_t  cartoon_dptr =0;
////                static  uint8_t  cartoon_cnt=0;
////               
////                uint8_t     i=0;
////                uint8_t     j=0;
////                switch(cartoon_mode)
////                {
////                    case 0:
////                    {
////                        if(cartoon_time_cnt++>=1)
////                        {
////                            cartoon_time_cnt = 0;
////                            memset((uint8_t *)&LedColorDataArr[0][0],0,sizeof(LedColorDataArr));
////                            
////                            cartoon_dptr++;
////                            if(cartoon_dptr > SK68XX_UINT_LED_NUM-3)
////                            {
////                                cartoon_dptr =0; 
////                                
////                                if(++cartoon_cnt>5)
////                                {
////                                    cartoon_cnt = 0;
////                                    cartoon_mode =1;
////                                }
////                            }
////                            for(i=0;i<SK68XX_GROUP_NUM;i++)
////                            {
////                                switch(i)
////                                {       
////                                    case 0:

////                                        LedColorDataArr[i][cartoon_dptr] = 0X7f000000;  //0X00007F00;
////                                        LedColorDataArr[i][cartoon_dptr+1] = 0X7f000000;//0X007f0000;
////                                        LedColorDataArr[i][cartoon_dptr+2] = 0X7f000000;//0X7f000000;
////                                        break;
////                                    case 1:
////                                        LedColorDataArr[i][cartoon_dptr] = 0X007f0000;  //0X00007F00;
////                                        LedColorDataArr[i][cartoon_dptr+1] = 0X007f0000;//0X007f0000;
////                                        LedColorDataArr[i][cartoon_dptr+2] = 0X007f0000;//0X7f000000;
////                                        break;
////                                    case 2:
////                                        LedColorDataArr[i][cartoon_dptr] = 0X00007F00;  //0X00007F00;
////                                        LedColorDataArr[i][cartoon_dptr+1] = 0X00007F00;//0X007f0000;
////                                        LedColorDataArr[i][cartoon_dptr+2] = 0X00007F00;//0X7f000000;
////                                        break;
////                                    case 3:
////                                        LedColorDataArr[i][cartoon_dptr] = 0X7f7f0000;  //0X00007F00;
////                                        LedColorDataArr[i][cartoon_dptr+1] = 0X7f7f0000;//0X007f0000;
////                                        LedColorDataArr[i][cartoon_dptr+2] = 0X7f7f0000;//0X7f000000;
////                                        break;
////                                    case 4:


////                                          for(j=0;j<SK68XX_UINT_LED_NUM;j++)
////                                          {
////                                            LedColorDataArr[i][j]= 0X7f000000;
////                                          }
//////                                        LedColorDataArr[i][cartoon_dptr] = 0X7f007F00;  //0X00007F00;
//////                                        LedColorDataArr[i][cartoon_dptr+1] = 0X7f007F00;//0X007f0000;
//////                                        LedColorDataArr[i][cartoon_dptr+2] = 0X7f007F00;//0X7f000000;
////                                        break;
////                                    default:
////                                        break;
////        
////                                }

////                            }

////                            SetSk68xxLedClour((uint8_t *)&LedColorDataArr[0][0],(uint16_t)sizeof(LedColorDataArr));
////                        }
////                        break;
////                    }
////                    case 1:
////                    {
////                        static uint8_t j=0;
////                        if(cartoon_time_cnt++>=1)
////                        {
////                            cartoon_time_cnt = 0;
////                            memset((uint8_t *)&LedColorDataArr[0][0],0,sizeof(LedColorDataArr));           
////                            cartoon_dptr++;
////                            if(cartoon_dptr > SK68XX_UINT_LED_NUM)
////                            {
////                                cartoon_dptr =0;  
////                                cartoon_mode =0;
////                                LedCrrtoonOver();
////                                break;

////                            }
////                            for(i=0;i<SK68XX_GROUP_NUM;i++)
////                            {
////                                for(j=0;j<cartoon_dptr;j++)
////                                {
////                                    LedColorDataArr[i][j] = 0X00007F00;
////                                }
////                            }
////                            SetSk68xxLedClour((uint8_t *)&LedColorDataArr[0][0],(uint16_t)sizeof(LedColorDataArr));
////                        }
////                        break;
////                    }
////                    case 2:
////                    {

////                        static uint8_t j=0;
////                        if(cartoon_time_cnt++>=1)
////                        {
////                            cartoon_time_cnt = 0;
////                            memset((uint8_t *)&LedColorDataArr[0][0],0,sizeof(LedColorDataArr));
////                            
////                            cartoon_dptr++;
////                            if(cartoon_dptr > SK68XX_UINT_LED_NUM)
////                            {
////                                cartoon_dptr =0; 

////                   
////                            }
////                            for(i=0;i<SK68XX_GROUP_NUM;i++)
////                            {
////                                for(j=0;j<cartoon_dptr;j++)
////                                {
////                                    LedColorDataArr[i][j] = 0X00007F00;
////                                }
////                            }
////                            SetSk68xxLedClour((uint8_t *)&LedColorDataArr[0][0],(uint16_t)sizeof(LedColorDataArr));
////                        }
////                        break;
////                    }
////                    default:
////                        break;
////                }
////                break;
////            }
//            case LED_STATUS_TEST_MODE:
//            {

//                static uint8_t test_clour_dptr=0;
//                if(_gLedInfo[i].led_time_count++> LED_TEST_MODE_BASE)
//                {
//                    _gLedInfo[i].led_time_count = 0;
//                    test_clour_dptr++;
//                    if(test_clour_dptr > 2)
//                    {
//                        test_clour_dptr = 0;
//                    }

//                    if(test_clour_dptr == 0)
//                    {
//                        _LedON((EN_LED_SRC)i,0XFF000000);
//                    }
//                    else if(test_clour_dptr == 1)
//                    {
//                        _LedON((EN_LED_SRC)i,0X00FF0000);
//                    }
//                    else if(test_clour_dptr == 2)
//                    {
//                        _LedON((EN_LED_SRC)i,0X0000FF00);                
//                    }
//  
//                }

//                break;
//            }

//            default:break;
//        }
//    }
//}

//void    RigisterLedCartoonCallBackFun(LedCartoonOverCallBack fun)
//{
//    if(fun!=NULL)
//    {
//        LedCrrtoonOver = fun;
//    }
//}
////void    SetLedClour(uint8_t r_data,uint8_t g_data,uint8_t b_data)
////{
////    MultiSigleLedClour.byte_value.rgb_rdata = r_data>>1;
////    MultiSigleLedClour.byte_value.rgb_gdata = g_data>>1;  
////    MultiSigleLedClour.byte_value.rgb_bdata = b_data>>1;
////}


////uint32_t LedColorDataArr[SK68XX_GROUP_NUM][SK68XX_UINT_LED_NUM]= {0};
////led_dirver_mode
























