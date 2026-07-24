//------------------------------------------------------------------------------
/*File Name         :   system_CMSDK_CM0.h
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   system_CMSDK_CM0库函数头文件

*/
//------------------------------------------------------------------------------

#ifndef SYSTEM_CMSDK_CM0_H
#define SYSTEM_CMSDK_CM0_H

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
/*Description       :   头文件
*/
//------------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "CMSDK_CM0.h"

//------------------------------------------------------------------------------
/*Description       :   宏定义
*/
//------------------------------------------------------------------------------
#define XTAL    (50000000UL)        /* Oscillator frequency               */

//------------------------------------------------------------------------------
/*Description       :   全局变量声明
*/
//------------------------------------------------------------------------------
extern uint32_t SystemCoreClock;     /*!< System Clock Frequency (Core Clock)  */

//------------------------------------------------------------------------------
/*Description       :   函数声明
*/
//------------------------------------------------------------------------------
extern void Delay(uint32_t count);
extern void SystemInit (void);
extern void SystemCoreClockUpdate (void);


#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_CMSDK_CM0_H */
