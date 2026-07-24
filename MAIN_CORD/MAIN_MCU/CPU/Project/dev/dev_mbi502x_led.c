/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_mbi502x_led.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-3-18
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-3-18     <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_mbi502x_led.h"

#define MBI5024_LED_MAX_CH_NUM 24

//static uint32_t  _gArrayUnitsDigit[] = {0x3D80,0x2100,0x1B80,0x3380,0x2700,0x3680,0x3E80,0x2180,0x3F80,0x3780};
////static uint32_t  _gArraytensDigit[] = {0x3F0000,0X60000,0X5B0000,0X4F0000,0X660000,0X6D0000,0X7D0000,0X70000,0X7F0000,0X6F0000};
//static uint32_t  _gArraytensDigit[] = {0x003F,0X6,0X5B,0X4F,0X66,0X6D,0X7D,0X7,0X7F,0X6F};
////static uint32_t  _gArrayUnitsLetter[] = {0xC0000003, 0x00000008, 0x00000004};        // 1+%+闪电
//static uint32_t  _gArrayUnitsLetter[] = {0x0003C000, 0x00080000, 0x00040000};

static uint32_t  _gArrayUnitsDigit[] = {0xFF40FF, 0xFFF9FF, 0xFF24FF, 0xFFB0FF, 0xFF99FF, 0xFF92FF, 0xFF82FF, 0xFFF8FF, 0xFF80FF, 0xFF90FF};
static uint32_t  _gArraytensDigit[] = {0xFFFF40, 0xFFFFF9, 0xFFFF24, 0xFFFFB0, 0xFFFF99, 0xFFFF92, 0xFFFF82, 0xFFFFF8, 0xFFFF80, 0xFFFF90};
static uint32_t  _gArrayUnitsLetter[] = {0XFFFFFF, 0xF0FFFF, 0xDFFFFF, 0xEFFFFF};

static uint32_t  _gLedBuf = 0x00000000;
static uint32_t  _gLedBuf1 = 0xDF90FF;
static uint32_t  _gLedBuf2 = 0X6F;

/**
  * @brief   MBI502xLed初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevMBI502xLedInit(void)
{
    MBI502xLedBspInit();
    _gLedBuf = 0;


}


/**
  * @brief   移位寄存器信号延时
  * @param   None
  * @return  None
  * @note    None
  */
static void _nop()
{
    uint8_t time = 0xf;
    while(time--);
}



/**
  * @brief   数据信号拉高
  * @param   None
  * @return  None
  * @note    None
  */
static void  _MB15024DataHigh(void)
{
    GpioBitSet(&MBI502xLedDataHandle);
}

/**
  * @brief   数据信号拉低
  * @param   None
  * @return  None
  * @note    None
  */
static void  _MB15024DataLow(void)
{
    GpioBitReset(&MBI502xLedDataHandle);
}

/**
  * @brief   时钟信号拉高
  * @param   None
  * @return  None
  * @note    None
  */
static void  _MB15024ClkHigh(void)
{
    GpioBitSet(&MBI502xLedClkHandle);
}

/**
  * @brief   时钟信号拉低
  * @param   None
  * @return  None
  * @note    None
  */
static void  _MB15024ClkLow(void)
{
    GpioBitReset(&MBI502xLedClkHandle);
}

/**
  * @brief   LATCH信号拉高
  * @param   None
  * @return  None
  * @note    None
  */
static void  _MB15024latchHigh(void)
{
    GpioBitSet(&MBI502xLedLatchHandle);
}

/**
  * @brief   LATCH信号拉低
  * @param   None
  * @return  None
  * @note    None
  */
static void  _MB15024latchLow(void)
{
    GpioBitReset(&MBI502xLedLatchHandle);
}

/**
  * @brief   OE信号拉高
  * @param   None
  * @return  None
  * @note    None
  */
static void  _MB15024OeHigh(void)
{
    GpioBitSet(&MBI502xLedOEHandle);
}

/**
  * @brief   OE信号拉低
  * @param   None
  * @return  None
  * @note    None
  */
static void  _MB15024OeLow(void)
{
    GpioBitReset(&MBI502xLedOEHandle);
}
void SwitchLedOn(void)
{

#if(SWITCH_LED_HIGH_ENABLE)
    GpioBitSet(&SwitchLedHandle); 
#else
    GpioBitReset(&SwitchLedHandle);
#endif  
}
void SwitchLedOff(void)
{

#if(SWITCH_LED_HIGH_ENABLE)
    GpioBitReset(&SwitchLedHandle);
#else
    GpioBitSet(&SwitchLedHandle);   
#endif  
       
}
void    DevSwitchLedInit(void)
{
        BspSwitchLedInit();
}

//static void  _MB15024Dig1(void)
//{
//    GpioBitSet(&NixieTubedig1FHandle);
//    GpioBitReset(&NixieTubedig2FHandle);
//    GpioBitReset(&NixieTubedig3GHandle);
//}

//static void  _MB15024Dig2(void)
//{
//    GpioBitReset(&NixieTubedig1FHandle);
//    GpioBitSet(&NixieTubedig2FHandle);
//    GpioBitReset(&NixieTubedig3GHandle);
//}
//static void  _MB15024Dig3(void)
//{
//    GpioBitReset(&NixieTubedig1FHandle);
//    GpioBitReset(&NixieTubedig2FHandle);
//    GpioBitSet(&NixieTubedig3GHandle);
//}

//typedef void (*NixieTubeNumSet)(void);
//NixieTubeNumSet NixieTubeGroupSet[3]= {_MB15024Dig2,_MB15024Dig1,_MB15024Dig3};

/**
  * @brief   数据通过移位寄存器写入，先写高bit
  * @param   None
  * @return  None
  * @note    None
  */
static void _LedWrite(uint32_t data)
{
    _MB15024ClkLow();
//    _MB15024DataLow();
    _MB15024latchLow();
//    _MB15024latchHigh();
//    _MB15024OeHigh();
    _MB15024OeLow();
    for(uint8_t i = 0; i < MBI5024_LED_MAX_CH_NUM; i++)
    {
        if(data & 0x800000)
        {
            _MB15024DataHigh();
        }
        else
        {
            _MB15024DataLow();
        }
        _nop();
        _MB15024ClkHigh();
        _nop();
        _MB15024ClkLow();
        _nop();
        data <<= 1;

    }
    _nop();
    _MB15024latchHigh();

    _nop();
    _MB15024latchLow();
    _nop();
    _MB15024OeLow();
}

/**
  * @brief   测试代码，进行动态显示
  * @param   None
  * @return  None
  * @note    None
  */
void TestLedActiveDisPlay(void)
{
    static uint32_t wait_time = 0;
    static uint32_t timer_cnt = 0;

    if(tim_get_systime() - wait_time > 1000)
    {
        wait_time = tim_get_systime();
        if(timer_cnt == 101)
        {
            timer_cnt = 0;
        }
        DevDightLedCtl(timer_cnt, 1);
        timer_cnt++;
    }
}



/**
  * @brief   闪电符号灯控制
  * @param   level: 1表示亮，0表示灭
  * @return  None
  * @note    None
  */
//void DevLightningLedCtl(uint8_t level)
//{
//  if(level == 1)
//  {
//      _gLedBuf[2] |= (1 << 2);
//  }
//  else
//  {
//      _gLedBuf[2] &= ~(1 << 2);
//  }
//}


///**
//  * @brief   百分比符号灯控制
//  * @param   level: 1表示亮，0表示灭
//  * @return  None
//  * @note    None
//  */
//void DevPercentLedCtl(uint8_t level)
//{
//  if(level == 1)
//  {
//      _gLedBuf[2] |= (1 << 1);
//  }
//  else
//  {
//      _gLedBuf[2] &= ~(1 << 1);
//  }
//}



/**
  * @brief   数字灯控制
  * @param   value: 0~100
  * @return  None
  * @note    None
  */
void DevDightLedCtl(uint8_t value, bool charge)
{
    static uint8_t _oldvalue = 0;
    uint8_t tens_digit = 0;
    uint8_t units_digit = 0;
    if(_oldvalue == value)
    {
        if(value)
        {
            return ;
        }
    }

    if(value <= 9)
    {
        _gLedBuf = _gArrayUnitsDigit[value];
    }
    else if(value <= 99)
    {
        tens_digit  = value / 10;
        units_digit = value % 10;
        _gLedBuf = _gArraytensDigit[tens_digit] & _gArrayUnitsDigit[units_digit];
    }
    else
    {
        _gLedBuf  = _gArraytensDigit[0] & _gArrayUnitsDigit[0] & _gArrayUnitsLetter[1];
    }
    _gLedBuf &= _gArrayUnitsLetter[2];

#if 0
    if(charge == 1)
    {
        _gLedBuf &= _gArrayUnitsLetter[3];
    }
#endif
//    _LedWrite(_gLedBuf1);
////    _LedWrite(_gLedBuf2);
    _LedWrite(_gLedBuf);
    _oldvalue = value;
}

/**
  * @brief   是否充电灯控制
  * @param   None
  * @return  None
  * @note    None
  */
void DevDightChargeLedCtl(uint16_t charge)
{
    if(charge == 2) //充电
    {
        _gLedBuf &= _gArrayUnitsLetter[3];
        _LedWrite(_gLedBuf);
    }
}


/**
  * @brief   故障符号灯控制
  * @param   value: 0表示E0,...9表示E9
  * @return  None
  * @note    None
  */
//void DevFaultLedCtl(uint8_t value)
//{
//  if(value > 9)
//  {
//      return;
//  }
//  _gLedBuf[2] &= ~(0xF << 3);
//  _gLedBuf[1] = (_gLedBuf[1] & 0xFFF8) | _gArrayUnitsLetter[4];
//  _gLedBuf[0] = (_gLedBuf[0] & 0xfFF8) | _gArrayUnitsDigit[value];
//}






/**
  * @brief   数码管轮询控制,10ms刷一组灯(非精确机制)
  * @param   None
  * @return  None
  * @note    None
  */
//void DevMBI502xProcess()
//{
//  static uint32_t time_count = 0;
//  static uint8_t led_index = 0;
//
////    if(GetSystick() - time_count >= 8)
//  {
//      time_count = GetSystick();
//      _LedWrite(_gLedBuf[led_index]);
//        NixieTubeGroupSet[led_index]();
//      led_index++;
//      led_index %= 3;
//  }
//}
