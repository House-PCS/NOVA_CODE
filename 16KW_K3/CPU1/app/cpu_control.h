/*
 * cpu_control.h
 *
 *  Created on: Nov 22, 2024
 *      Author: Nova007456
 */

#ifndef APP_CPU_CONTROL_H_
#define APP_CPU_CONTROL_H_

#include "F28x_Project.h"
#include "mid_comservice.h"
#include "system.h"

extern uint8_t SciDataTemp;

inline void SciaRxDataRecv(void)
{
    if(SciaRegs.SCIFFRX.bit.RXFFST != 0)
    {
        SciDataTemp = SciaRegs.SCIRXBUF.all;
        mid_comservice_data_in(EN_COMSERVICE_COM_INDEX2, &SciDataTemp, 1);
    }
}

inline void ScibRxDataRecv(void)
{
    if(ScibRegs.SCIFFRX.bit.RXFFST != 0)
    {
        SciDataTemp = ScibRegs.SCIRXBUF.all;
        mid_comservice_data_in(EN_COMSERVICE_COM_INDEX0, &SciDataTemp, 1);
    }
}

inline void ScicRxDataRecv(void)
{
    if(ScicRegs.SCIFFRX.bit.RXFFST != 0)
    {
        SciDataTemp = ScicRegs.SCIRXBUF.all;
        mid_comservice_data_in(EN_COMSERVICE_COM_INDEX1, &SciDataTemp, 1);
    }
}
void CpuIntFunInit(void);
uint32_t GetCpuInterruptRunCont(void);

/* Updated every 80kHz ISR: slot passed to GetAdcResult (0~4) */
extern volatile EN_CPU_INTERRUPT_TIME gCpuAdcSampleTimeSlot;
/* Per-slot hit count since last reset (for 1ms debug polling) */
extern volatile uint32_t gCpuTimeSlotHitCnt[5];

#endif /* APP_CPU_CONTROL_H_ */
