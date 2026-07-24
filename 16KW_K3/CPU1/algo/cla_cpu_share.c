/*
 * cla_cpu_share.c
 *
 *  Created on: 2025-03-25
 *      Author: Nova007456
 */
#include "cla_cpu_share.h"

volatile float32 garrDebugDataCla[16];
#pragma DATA_SECTION(garrDebugDataCla,"Cla1ToCpuMsgRAM");

#pragma DATA_SECTION(gstCpuTransforClaData,"CpuToCla1MsgRAM");
volatile CpuTransforClaData_t   gstCpuTransforClaData;
