
#ifndef __FAULT_RECORD_PROCESS_H
#define __FAULT_RECORD_PROCESS_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "CMSDK_CM0.h"
#include "core_cm0.h"
#include "systick.h"
#include "dev_led.h"
#include "uart.h"
#include "mid_comservice.h"
#include "comm_analysis.h"
#include "fpga_com.h"
#include "firmware_upgrate.h"
#include "dev_phy.h"
#include "spi.h"
#include "dev_spiflash.h"
#include "dev_ddr.h"
#endif 

typedef enum
{
    EN_FAULT_ANALYSIS_STATUS_IDLE,//¿ÕÏÐÄ£Ê½
    EN_FAULT_ANALYSIS_STATUS_START,    
    EN_FAULT_ANALYSIS_STATUS_BUSY,
    EN_FAULT_ANALYSIS_STATUS_END,//×´Ì¬´íÎó
    EN_FAULT_ANALYSIS_STATUS_OK,//OK
} EN_FAULT_ANALYSIS_STATUS;

typedef struct _fpgafault_info_stru_t
{
    uint8_t         fpga_en;
    uint32_t        fpga_offset;
    uint32_t        fpga_len;
    uint32_t        lenth;    
    uint32_t        fpga_checksum;
    
    uint32_t        len;
    uint32_t        offset;
    
    uint32_t        fpga_seek;
    uint32_t        seek;    
    uint8_t         fpga_ack;
    uint8_t         fpga_end_ack;
    uint8_t         upgrate_success_flag;
    uint8_t         fault_record_finsh_flag;	
    uint8_t         fault_record_fail_flag;    
    uint8_t         fpga_upgrating;

} FPGAFaultInfo_t;

void FPGAFaultAnalysis(void);
void FPGARecordPorcess(void);



