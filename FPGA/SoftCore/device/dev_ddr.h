
#ifndef __DEV_DDR_H
#define __DEV_DDR_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "CMSDK_CM0.h"
#include "core_cm0.h"
#include "string.h"
#include "systick.h"
#include "private_analysis.h"
#define DDR           ((CMSDK_DDR_TypeDef   *) 0x20000000   )
#define FPGA_DDR_FIRMWARE_ADDR              0x24000000
#define BMS_DDR_FIRMWARE_ADDR               0x24400000
typedef struct
{
    __IO   uint32_t  DATA1;          /*!< Offset: 0x000 Data Register    (R/W) */
    __IO   uint32_t  DATA2;         /*!< Offset: 0x004 Status Register  (R/W) */
    __IO   uint32_t  DATA3;          /*!< Offset: 0x008 Control Register (R/W) */
    __IO   uint32_t  DATA4;          /*!< Offset: 0x008 Control Register (R/W) */    

} CMSDK_DDR_TypeDef;


typedef struct _ddr_firmware_info_stru_t
{
    uint32_t    ddr_bms_check_sum_com;
    uint32_t    ddr_bms_check_sum_rece;
    uint16_t    ddr_bms_pack_count;    
    uint8_t     ddr_bms_firmware_success;
    uint32_t    ddr_bms_len;    
    
    uint32_t    ddr_fpga_check_sum_com;
    uint32_t    ddr_fpga_check_sum_rece;
    uint16_t    ddr_fpga_pack_count;
    uint8_t     ddr_fpga_firmware_success;
    uint32_t    ddr_fpga_len;    
    
} DDR_Firmware_Info_t;

void WriteDDRSend(uint32_t start_addr, uint8_t *p, uint32_t len);
void DDRSendByte(char dat);
extern DDR_Firmware_Info_t gDDRFirmwareInfo;
extern uint8_t bms_ddr_temp[256];
extern uint8_t fpga_ddr_temp[256];
#endif 











