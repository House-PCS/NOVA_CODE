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

#define MBI5024_LED_MAX_CH_NUM 16

static uint16_t  _gArrayUnitsDigit[] = {0x007E, 0x0030, 0x006D, 0x0079, 0x0033, 0x005B, 0x005F, 0x0070, 0x007F, 0x007B};
static uint16_t  _gArrayUnitsLetter[] = {0x0077, 0x007C, 0x0039, 0x005E, 0x004F, 0x0000};        // E

static uint16_t  _gLedBuf[3];


/**
  * @brief   MBI502xLed初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevMBI502xLedInit(void)
{
    MBI502xLedBspInit();
	
	/*配置好VCC PIN脚映射*/
	_gLedBuf[2] = 0x3 << 7; //0000 0001 1xxx xxxx
	_gLedBuf[1] = 0x6 << 7; //0000 0011 0xxx xxxx
	_gLedBuf[0] = 0x5 << 7; //0000 0010 1xxx xxxx
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



/**
  * @brief   数据通过移位寄存器写入，先写高bit
  * @param   None
  * @return  None
  * @note    None
  */
static void _LedWrite(uint16_t data)
{
	_MB15024ClkLow();
    _MB15024DataLow();
    _MB15024latchLow(); 
    _MB15024OeHigh();    
	for(uint8_t i = 0; i < MBI5024_LED_MAX_CH_NUM; i++)
	{
		if(data & 0x8000)
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
  * @brief   闪电符号灯控制
  * @param   level: 1表示亮，0表示灭
  * @return  None
  * @note    None
  */
void DevLightningLedCtl(uint8_t level)
{
	if(level == 1)
	{
		_gLedBuf[2] |= (1 << 2);
	}
	else
	{
		_gLedBuf[2] &= ~(1 << 2);
	}
} 


/**
  * @brief   百分比符号灯控制
  * @param   level: 1表示亮，0表示灭
  * @return  None
  * @note    None
  */
void DevPercentLedCtl(uint8_t level)
{
	if(level == 1)
	{
		_gLedBuf[2] |= (1 << 1);
	}
	else
	{
		_gLedBuf[2] &= ~(1 << 1);
	}
}

  

/**
  * @brief   数字灯控制
  * @param   value: 0~100
  * @return  None
  * @note    None
  */
void DevDightLedCtl(uint8_t value)
{
	if(value <= 9)	
	{
		_gLedBuf[2] &= ~(0xF << 3);
		_gLedBuf[1] &= 0xFFF8;
		_gLedBuf[0] = (_gLedBuf[0] & 0xFFF8) | _gArrayUnitsDigit[value];	
	}
	else if(value <= 99)
	{
		_gLedBuf[2] &= ~(0xF << 3);
		_gLedBuf[1] = (_gLedBuf[1] & 0xFFF8) | _gArrayUnitsDigit[value / 10];	
		_gLedBuf[0] = (_gLedBuf[0] & 0xFFF8) | _gArrayUnitsDigit[value % 10];	
	}
	else
	{
		_gLedBuf[2] |= (0xF << 3);
		_gLedBuf[1] = (_gLedBuf[1] & 0xFFF8) | _gArrayUnitsDigit[0];	
		_gLedBuf[0] = (_gLedBuf[0] & 0xFFF8) | _gArrayUnitsDigit[0];	
	}
}



/**
  * @brief   故障符号灯控制
  * @param   value: 0表示E0,...9表示E9
  * @return  None
  * @note    None
  */
void DevFaultLedCtl(uint8_t value)
{
	if(value > 9)
	{
		return;
	}
	_gLedBuf[2] &= ~(0xF << 3);	
	_gLedBuf[1] = (_gLedBuf[1] & 0xFFF8) | _gArrayUnitsLetter[4];	
	_gLedBuf[0] = (_gLedBuf[0] & 0xfFF8) | _gArrayUnitsDigit[value];	
}






/**
  * @brief   数码管轮询控制,10ms刷一组灯(非精确机制)
  * @param   None
  * @return  None
  * @note    None
  */
void DevMBI502xProcess(void)
{
	static uint32_t time_count = 0; 
	static uint8_t led_index = 0;
	
//	if(GetSystick() - time_count >= 8)
	{
		time_count = GetSystick();
		_LedWrite(_gLedBuf[led_index]);
		led_index++;
		led_index %= 3;
	}
}
