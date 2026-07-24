#ifndef SLAVE_DSP_HHH
#define SLAVE_DSP_HHH



#include "dev_udp_comm.h"
#include "udp.h"
#include <string.h>


//#pragma pack(2)

////typedef struct _dsp_para_stru_t
////{
////    

////}DspParaData_t;





//#pargma pack()

void DspCommProc(void);
void DspUdpParaInit(void);
void SendDspUdpData(void);



#endif