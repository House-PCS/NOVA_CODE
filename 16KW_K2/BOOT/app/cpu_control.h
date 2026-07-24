/*
 * cpu_control.h
 *
 *  Created on: 2024Äê11ÔÂ22ÈÕ
 *      Author: Nova007456
 */

#ifndef APP_CPU_CONTROL_H_
#define APP_CPU_CONTROL_H_

#include "F28x_Project.h"
#include "mid_comservice.h"
#include "system.h"

extern uint8_t SciDataTemp;

typedef struct _cpu_interrupt_t
{
    EN_CPU_INTERRUPT_TIME time;
    uint32_t    cpu_int_count;
} CpuInterrupt_t;

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

#endif /* APP_CPU_CONTROL_H_ */
