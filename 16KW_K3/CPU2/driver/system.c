/*
 * system.c
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */

#include "system.h"
#include "F28x_Project.h"

/**
* @brief: SysRestart
* @param: void
* @note: 系统复位函数
*/
void SysRestart(void)
{
    EALLOW;
    WdRegs.WDCR.all = 0x00af;
    WdRegs.WDCR.bit.WDCHK = 2;
    EDIS;
}
