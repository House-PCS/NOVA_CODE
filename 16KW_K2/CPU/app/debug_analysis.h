/*
 * debug_analysis.h
 *
 *  Created on: 2025Äê1ÔÂ7ÈÕ
 *      Author: Nova007456
 */

#ifndef APP_DEBUG_ANALYSIS_H_
#define APP_DEBUG_ANALYSIS_H_
#include "F28x_Project.h"


extern uint16_t gDebugValue[10];

void Debug_Send(void);
void ScidSendMessage(void);
void ScidReadMessage(void);
void SetCpuDebugData(uint16_t buff_num,float32 value);
void DebugReveiveAnalysis(uint16_t buff[],uint16_t len);


#endif /* APP_DEBUG_ANALYSIS_H_ */
