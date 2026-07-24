/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        gpio.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-07-20
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * <tr><td>2023-06-15    <td>V1.0.1    <td>           <td>增加同一串口可使用多种中断并存的机制
  * </table>
  ******************************************************************************
  */
#include "mdio.h"

void SmiRstLow()
{
	*((uint32_t *)0x40000020) = 0;
}

void SmiRstHigh()
{
	*((uint32_t *)0x40000020) = 1;
}

static void _MdcLow()
{
	*((uint32_t *)0x40000024) = 0;
}


static void _MdcHigh()
{
	*((uint32_t *)0x40000024) = 1;
}


static void _MdioLow()
{
	*((uint32_t *)0x40000028) = 0;
}


static void _MdioHigh()
{
	*((uint32_t *)0x40000028) = 1;
}


static uint8_t _MdioInputGet()
{
	if(*((uint32_t *)0x40000028) == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


//可不用
void MdioSet2Input()
{
	*((uint32_t *)0x4000002C) = 0;
}

void MdioSet2Output()
{
	*((uint32_t *)0x4000002C) = 1;
}


static void _SmiDelay(void)
{
    uint16_t  TimeIsOver = 20;

    while(TimeIsOver)
    {
        TimeIsOver--;
    }
}


void SmiWriteBits(uint8_t *tx_data, uint16_t tx_bit_size)
{
    uint16_t i = 0;
    uint8_t writeData = 0;
    
    for (i = 0; i < tx_bit_size; i++)
    {
        writeData = ((uint8_t*)tx_data)[i / 8];
        _MdcLow();
        if ((writeData >> (7 - i % 8) & 0x01) == 0x01)
        {
            _MdioHigh();
        }
        else
        {
            _MdioLow();
        }
        _SmiDelay();
        _MdcHigh();
        _SmiDelay();
    }
    _MdioHigh();
}


void SmiReadBits(uint8_t *rx_data, uint16_t rx_bit_size)
{
    uint16_t i = 0;
    uint16_t value = 0;
    
//    MdioSet2Input();
    
    for(i = 0; i < rx_bit_size; i++)
    {
        value = value << 1;
        _MdcHigh();
        _SmiDelay();
        _MdcLow();
        _SmiDelay();
        if(_MdioInputGet() == 1)
        {
            value = value | 0x0001;
        }
    }

    ((uint8_t*)rx_data)[0] = value & 0x00FF;
    ((uint8_t*)rx_data)[1] = (value >> 8) & 0x00FF;;
//    MdioSet2Output();
}


bool SmiSendTa()
{
//    MdioSet2Input();
    _MdcHigh();
    _SmiDelay();
    _MdcLow();
    _SmiDelay();
    _MdcHigh();
    _SmiDelay();
    _MdcLow();
    _SmiDelay();
    if(_MdioInputGet() == 0)
    {
        return true;
    }
    else
    {
        _MdcHigh();
        _SmiDelay();
        _MdcLow();
        _SmiDelay();
        if(_MdioInputGet() == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }	      
}

void SmiIdleState()
{
    _MdioHigh();
    _SmiDelay();
    _MdcHigh();
    _SmiDelay();
    _MdcLow();
    _SmiDelay();   
}




