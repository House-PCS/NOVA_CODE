//------------------------------------------------------------------------------
/*File Name         :   Spi.c
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   Spi固件库函数

*/
//------------------------------------------------------------------------------
#include "Spi.h"

//------------------------------------------------------------------------------
/*Function Name     :   SpiInit 
**Description       :   Spi初始化
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void SpiInit()
{
    SPI->MODE = 3;
    SPI->FREQ_COE = 255;
}

//------------------------------------------------------------------------------
/*Function Name     :   SpiWrData 
**Description       :   Spi初始化
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void SpiWrData(uint8_t Data)
{
    SPI->WDATA = Data;
    while((SPI->TRAN_FLAG - 1));
}


//------------------------------------------------------------------------------
/*Function Name     :   SpiWrTest 
**Description       :   Spi初始化
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------
int SpiWrTest()
{
    SPI->CS = 0x01;
    SpiWrData(0x82);
    SPI->CS = 0x00;

    return 1;
}


