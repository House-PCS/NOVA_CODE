/*
 * debug_analysis.h
 *
 *  Created on: Jan 7, 2025
 *      Author: Nova007456
 */

#ifndef APP_DEBUG_ANALYSIS_H_
#define APP_DEBUG_ANALYSIS_H_
#include "F28x_Project.h"

/* CPU debug channel extend: 0 = 16 channels, 1 = 80 channels (16 + 64) */
#define DEBUG_CPU_CHANNEL_EXTEND    1
#if (DEBUG_CPU_CHANNEL_EXTEND)
#define DEBUG_CPU_CHANNEL_NUM       96
#else
#define DEBUG_CPU_CHANNEL_NUM       16
#endif

extern uint16_t gDebugValue[10];

void Debug_Send(void);
void ScidSendMessage(void);
void ScidReadMessage(void);
void SetCpuDebugData(uint16_t buff_num,float32 value);
void DebugReveiveAnalysis(uint16_t buff[],uint16_t len);


#endif /* APP_DEBUG_ANALYSIS_H_ */
