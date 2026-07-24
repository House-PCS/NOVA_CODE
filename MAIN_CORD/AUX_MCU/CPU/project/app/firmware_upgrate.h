#ifndef __FIRMWARE_UPGRATE_H_
#define __FIRMWARE_UPGRATE_H_


#include "flash.h"

#define  FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR       0x040000
#define  FIRMWARE_EXTERNAL_FLASH_ADDR            0x041000

#define CMD_UPGRATE_PREPARE                 0x80F0
#define CMD_UPGRATE_DATA                    0x80F1 
#define CMD_UPGRATE_COMPLETE                0x80FF


typedef struct _firmware_upgrate_info_stru_t
{
    uint8_t         rev_start_flag; /*接收到升级提示命令后，置1*/
    uint8_t         rev_end_flag;/*接收到升级包发完命令后，置1*/
    uint32_t        write_addr;
    
    uint32_t        file_size;          //已经获取到的升级文件大小。
    uint32_t        file_check_sum;     //接收到的check_sum
    uint32_t        file_size_read;
    uint32_t        package_count;
    uint16_t        space_data_size;
    uint32_t        file_all_calc_check_sum;
    uint32_t        file_one_pack_check_sum;
    uint32_t        old_pack_id;
    uint32_t        pack_id;

    uint32_t        slave_data_offset;
    uint32_t        slave_len;
    uint32_t        slave_seek;
    uint32_t        slave_checksum; 

    uint8_t         upgrate_end_flag;        /*1:升级成功标志，2:升级失败标志*/
    uint32_t        updata_get_total_size;   //从监控卡获取到的文件大小。
    uint8_t         updata_get_persent;      //获取到的升级文件百分比。
    uint32_t        updata_send_size;        //已经下发文件大小
    uint8_t         updata_send_persent;     //已经下发文件百分比

    uint8_t         re_send_cnt;
    uint16_t        firmware_updata_pack_num;
    uint32_t        size_per_transfer;
    uint8_t         updata_protocl_space_num;
    uint8_t         upgrading_status;
    uint32_t        upgrad_err_code;
    uint8_t         upgrad_resting;
    uint8_t         upgrade_rest_status;
}FirmwareUpgrateInfo_t;


#pragma pack(2)
typedef union _sofeware_ver
{
    struct  
    {
        uint16_t ver_low:6;
        uint16_t ver_mid:6;
        uint16_t ver_hi:4;
    }bits;
    uint16_t    word_value;
} SoftwareVer_t;

typedef struct _sofeware_extern_ver
{
    uint8_t soft_extern_ver[12];
} SoftwareExternVer_t;


#pragma pack()



extern FirmwareUpgrateInfo_t gstFirmwareUpgrateInfo;

void FirmwareUpgrateInit(void);
void UpgrateFirmwareProcess(void);
void SysInfoInit();
#endif

