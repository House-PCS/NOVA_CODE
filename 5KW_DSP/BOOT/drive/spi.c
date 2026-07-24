/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        spi.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


//
// Included Files
//
#include "spi.h"

static void _SpiWriteByte(uint16_t a);
static uint16_t _SpiReadByte(void);

static uint16_t _gRdataTemp = 0;

/**
* @brief: _SpiWriteByte
* @param: a                           
* @return: 
* @note: 
*
*/
static void _SpiWriteByte(uint16_t a)
{
	SpiaRegs.SPITXBUF = (a << 8);
    while(SpiaRegs.SPIFFRX.bit.RXFFST !=1) { }
    _gRdataTemp = SpiaRegs.SPIRXBUF;
}

/**
* @brief: _SpiReadByte
* @param: void                        
* @return: 
* @note: 
*
*/
static uint16_t _SpiReadByte(void)
{
	SpiaRegs.SPITXBUF = (_gRdataTemp << 8);
	while(SpiaRegs.SPIFFRX.bit.RXFFST !=1) { }
	_gRdataTemp = SpiaRegs.SPIRXBUF;

	return _gRdataTemp;
}

/**
* @brief: SpiFifoInit
* @param: void                        
* @note: 
*
*/
void SpiFifoInit(void)
{
	InitSpiaGpio();
    //
    // Initialize SPI FIFO registers
    //
    SpiaRegs.SPIFFTX.all = 0xE040;
    SpiaRegs.SPIFFRX.all = 0x2044;
    SpiaRegs.SPIFFCT.all = 0x0;

    //
    // Initialize core SPI registers
    //
    InitSpi();

}

/**
* @brief: SpiWriteData
* @param: len                         
* @param: data                        
* @note: 
*
*/
void SpiWriteData(uint16_t len, uint16_t* data)
{
	uint16_t i = 0;
	for(i = 0; i < len; i ++)
	{
		_SpiWriteByte(data[i]);
	}
}

/**
* @brief: SpiReadData
* @param: len                         
* @param: data                        
* @note: 
*
*/
void SpiReadData(uint16_t len, uint16_t* data)
{
	uint16_t i = 0;
	for(i = 0; i < len; i ++)
	{
		data[i] =  _SpiReadByte();
	}
}

