//------------------------------------------------------------------------------
/*File Name         :   Gpio.c
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   Gpio固件库函数

*/
//------------------------------------------------------------------------------
#include "Gpio.h"

//------------------------------------------------------------------------------
/*Function Name     :   GpioInit 
**Description       :   Timer0初始化
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void GpioInit()
{
    ArmLed      = 0x00  ;
    PhyRstN     = 0x01  ;
}



