
#ifndef __FIRMWARE_UPGRATE_H_
#define __FIRMWARE_UPGRATE_H_

#include "dev_spiflash.h"



#define FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR           0x000000




//#define  FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR      0x001000  //0x137000  //311 SECTOR
//#define FIRMWARE_EXTERNAL_FLASH_ADDR                0x0C8000  //201 SECTOR0x001000


#define FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR       0x4B0000  //压缩数据地址
#define FIRMWARE_EXTERNAL_FLASH_ADDR                0x5B0000  //解压数据地址
#define DEZIP_DATA_MAX_SECTOR_SIZE                  1024//4K*1024 =4M     //解压之后，擦除的sector 最大需要擦除1024 个secotr 

#define DEZIP_DATA_SIZE_MAX                         (4ul*1024ul*1024ul)   //解压之后的数据大小小于4M     


           
#define UPDATA_ZIP_RESOTRE_MAX_ADDR                 (FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR + 1024*1024)

#define CMD_UPGRATE_PREPARE                 0x80F0
#define CMD_UPGRATE_DATA                    0x80F1 
#define CMD_UPGRATE_COMPLETE                0x80FF   



#define UPDATA_MAIN_TYPE                0X00
#define UPDATA_PCS_TYPE                 0X01
#define UPDATA_ARC_TYPE                 0X02
#define UPDATA_BMS_TYPE                 0X03
#define UPDATA_UI_TYPE                  0XF0
#define UPDATA_UI_TYPE1                 0XF1
#define UPDATA_NO_TYPE                  0XFF

typedef enum
{
    FIRMWARE_UPGRATE_STATUS_IDLE,
    FIRMWARE_UPGRATE_STATUS_CHECK,
    FIRMWARE_UPGRATE_STATUS_DEZIP,
    FIRMWARE_UPGRATE_STATUS_TO_DO,
    FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE,
    FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_PREPARE_ACK,
    FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_DATA,
    FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_DATA_ACK,
    FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_END,
    FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_END_ACK,
    FIRMWARE_UPGRATE_STATUS_MAINCTL,
    FIRMWARE_UPGRATE_STATUS_ERROR,
    FIRMWARE_UPGRATE_STATUS_OK,
    FIRMWARE_UPGRATE_STATUS_WAIT_RESTART,
}EN_FIRMWARE_UPGRATE_STATUS;

			
typedef struct _firmware_upgrate_info_stru_t
{
    uint8_t         enter_updata_buad_mode;
    uint8_t         rev_start_flag; /*接收到升级提示命令后，置1*/
    uint8_t         rev_end_flag;/*接收到升级包发完命令后，置1*/
    uint32_t        write_addr;
    
    uint32_t        file_size;
    uint32_t        file_check_sum;
    
    uint32_t        file_size_read;
    uint32_t        package_count;
    
    uint8_t         bms_data_existed;
    uint32_t        bms_data_offset;//包含文件头
    uint32_t        bms_data_len;//包含文件头
    uint32_t        bms_data_checksum;



    uint8_t         slave_prepared_ack;//1表示准备好，2表示当前不允许升级
    uint8_t         slave_data_ack;//1表示数据接收OK，2表示数据有误
    uint8_t         slave_end_ack;//1表示数据包整体无误，2表示数据有误
    
    uint32_t        slave_data_offset;
    uint32_t        slave_len;
    uint32_t        slave_seek;


    uint8_t         pcs_existed;
    uint32_t        pcs_data_offset;
    uint32_t        pcs_len;
    uint32_t        pcs_checksum;

    uint8_t         arc_existed;
    uint32_t        arc_data_offset;
    uint32_t        arc_len;
    uint32_t        arc_checksum;   


    uint8_t         ui_type_img_existed;
    uint32_t        ui_type_img_offset;
    uint32_t        ui_type_img_len;
    uint32_t        ui_type_img_checksum; 


    uint8_t         ui_type1_font_existed;
    uint32_t        ui_type1_font_offset;
    uint32_t        ui_type1_font_len;
    uint32_t        ui_type1_font_checksum; 

    uint8_t         updata_slave_type;

    uint8_t         main_ctl_data_existed;
    uint32_t        main_ctl_data_offset;//包含文件头
    uint32_t        main_ctl_data_len;//包含文件头
    uint32_t        main_ctl_data_checksum;

    uint8_t         upgrate_end_flag; /*1:升级成功标志，2:升级失败标志*/
    uint8_t         updata_working;

    uint32_t        updata_get_total_size;   //从采集器获取到的文件大小。
    uint8_t         updata_get_persent;
    uint32_t        updata_send_size;
    uint8_t         updata_send_persent;
}FirmwareUpgrateInfo_t;


extern FirmwareUpgrateInfo_t gFirmwareUpgrateInfo;
void        FirmwareUpgrateInit(void);
uint16_t    UpgratePrepare(void);
uint16_t    UpgrateDataDeal(uint8_t *pSrc, uint16_t rev_len);
uint16_t    UpgrateRevComplete(void);
void        BmsReadHistoryCap(uint8_t *p_data,uint16_t size);
void        UpgrateFirmwareProcess(void);
uint8_t     GetUpdataStuatus(void);
uint8_t     GetUpdataBaudStatusInfo(void);
#endif







