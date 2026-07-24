
#ifndef __FIRMWARE_UPGRATE_H_
#define __FIRMWARE_UPGRATE_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "CMSDK_CM0.h"
#include "core_cm0.h"
#include "dev_spiflash.h"
#include "private_analysis.h"
#include "mid_comservice.h"
#include "ddr_process.h"
#define UPGRADE_START           0x80F0
#define UPGRADE_BUSY            0x80F1
#define UPGRADE_END             0x80FF

#define FPGA_FIRMWARE_ADDR              0x00261000
#define FPGA_FIRMWARE_VERSION			0x00C00000
#define UPGRADE_FLAG_ADDR   0x00BFF000
#define UPGRADE_FLAG   0xA0

typedef enum
{
    EN_FIRMWARE_UPGRATE_STATUS_IDLE,//¿ÕÏÐÄ£Ê½   
    EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE,//Ô¤±¸×´Ì¬
    EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE_ACK,    
    EN_FIRMWARE_UPGRATE_STATUS_WAIT_PREPARE_ACK,//Ô¤±¸µÈ´ý¼ì²é×´Ì¬
    EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY,//·¢ËÍÊý¾Ý
    EN_FIRMWARE_UPGRATE_STATUS_WAIT_DATA_BUSY_ACK,//·¢ËÍÖÐµÈ´ý¼ì²é
    EN_FIRMWARE_UPGRATE_STATUS_SEND_END,//·¢ËÍ½ØÖ¹
    EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK,//·¢ËÍ½ØÖ¹¼ì²é
    EN_FIRMWARE_UPGRATE_STATUS_ERROR,//×´Ì¬´íÎó
    EN_FIRMWARE_UPGRATE_STATUS_OK,//OK
} EN_BMS_FIRMWARE_UPGRATE_STATUS;

typedef enum
{
    EN_FIRMWARE_ANALYSIS_STATUS_IDLE,//¿ÕÏÐÄ£Ê½
    EN_FIRMWARE_ANALYSIS_STATUS_START,    
    EN_FIRMWARE_ANALYSIS_STATUS_BUSY,
    EN_FIRMWARE_ANALYSIS_STATUS_END,//×´Ì¬´íÎó
    EN_FIRMWARE_ANALYSIS_STATUS_OK,//OK
} EN_FIRMWARE_ANALYSIS_STATUS;

typedef struct _bms_firmware_upgrate_info_stru_t
{
    uint8_t         bms_upgrate_en;
    uint32_t        bms_offset;
    uint32_t        bms_len;
    uint32_t        bms_checksum;
    
    uint32_t        len;
    uint32_t        offset;
    
    uint32_t        bms_seek;
    uint8_t         bms_ack;
    uint8_t         bms_end_ack;
    uint8_t         upgrate_success_flag;
    uint8_t         upgrate_finsh_flag;	
    uint8_t         upgrate_fail_flag;    
    uint8_t         bms_upgrating;

} BMSFirmwareUpgrateInfo_t;

typedef struct _fpgafirmware_upgrate_info_stru_t
{
    uint8_t         fpga_upgrate_en;
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
    uint8_t         upgrate_finsh_flag;	
    uint8_t         upgrate_fail_flag;    
    uint8_t         fpga_upgrating;

} FPGAFirmwareUpgrateInfo_t;

void BMSUpgrateFirmwareProcess(void);
void FPGAFirmwareAnalysis(void);
void UpdataStatusAnalysis(void);
void FirmwareUpgrate(void);
extern uint8_t gSoftwareFirmwareType[4];
extern FPGAFirmwareUpgrateInfo_t    gFPGAFirmwareUpgrateInfo;
extern BMSFirmwareUpgrateInfo_t     gBMSFirmwareUpgrateInfo;
#endif

