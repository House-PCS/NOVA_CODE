//------------------------------------------------------------------------------
/*File Name         :   system_CMSDK_CM0.c
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   system_CMSDK_CM0库函数

*/
//------------------------------------------------------------------------------
#include "system_CMSDK_CM0.h"

//------------------------------------------------------------------------------
/*Description       :   全局变量
*/
//------------------------------------------------------------------------------
uint32_t SystemCoreClock;     

//------------------------------------------------------------------------------
/*Function Name     :   Delay 
**Description       :   延迟参数
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void Delay(uint32_t count) 
{  
    while (count--) 
    {  
        
    };
}

//------------------------------------------------------------------------------
/*Function Name     :   SystemCoreClockUpdate 
**Description       :   系统时钟频率
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void SystemCoreClockUpdate (void)            /* Get Core Clock Frequency      */
{
  SystemCoreClock = XTAL;
}

//------------------------------------------------------------------------------
/*Function Name     :   SystemInit 
**Description       :   系统复位
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------
void SystemInit (void)
{
  SystemCoreClock = XTAL;
}

