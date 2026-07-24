
#ifndef __FIRMWARE_UPGRATE_H_
#define __FIRMWARE_UPGRATE_H_

#include "dev_spiflash.h"
//#include "slave_bms.h"


#define FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR           0x000000

//#define FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR       0x001000  //0x137000  //311 SECTOR
//#define FIRMWARE_EXTERNAL_FLASH_ADDR                0x0C8000  //201 SECTOR0x001000

//#define FIRMWARE_EXTERNAL_FLASH_IMAG_ADDR           1200ul*4096ul    //0x4b0000 -5AFFFF
//#define FIRMWARE_EXTERNAL_FLASH_FONT_ADDR           1456ul*4096ul    //0x5b0000 -6AFFFF


#define FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR       0x4B0000ul  //压缩数据地址
#define FIRMWARE_EXTERNAL_FLASH_ADDR                0x6B0000ul  //解压数据地址   //
#define DEZIP_DATA_MAX_SECTOR_SIZE                  1024ul//4K*1024 =4M   
#define DEZIP_DATA_SIZE_MAX                         (4ul*1024ul*1024ul)   //解压之后的数据大小小于4M 
#define UPDATA_ZIP_RESOTRE_MAX_ADDR                 (FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR + 2024ul*1024ul)




#define UPGRATE_RESTART_TIME_OUT            6000  
#define WAIT_FIRMWARE_UPDATA_DELAY          120000UL                      
#define UPGRATE_TIME_OUT                    3000
#define UPGRATE_TIME_RECV_OUT               10000
#define SLAVE_UPGRATE_DATA_SIZE_256         256ul
#define SLAVE_UPGRATE_DATA_SIZE_2048        2048ul
#define SLAVE_UPGRATE_DATA_SIZE_1024        1024ul
#define SLAVE_UPGRATE_DATA_SIZE_768         768ul
#define SLAVE_UPGRATE_DATA_SIZE             256
#define ADDRESSING_TIME_OUT                 3000
#define UPDATE_DATA_START_ADDR              (FIRMWARE_EXTERNAL_FLASH_ADDR)
#define BUFFER_SIZE                         (1024)
#define ICON_NUM                            (100)


//固件寄存器地址
#define UPDATA_FPGA_DATA_BASE_ADDR          (0x24000000ul)
#define UPDATA_BMS_DATA_BASE_ADDR           (0x24400000ul)
#define UPDATA_SW_DATA_BASE_ADDR            (0x24500000ul)
#define UPDATA_DSP_DATA_BASE_ADDR           (0x24600000ul)
#define UDPDTA_ARC_DATA_BASE_ADDR           (0x24700000ul)
#define UPDATA_MAIN_COP_DATA_BASE_ADDR      (0x0ul)          //没有用，只是为了符合函数参数写的   
#define UDPDTA_PCS_COP_DATA_BASE_ADDR       (0x24800000ul)

#define CMD_SAME_FIARMWARE                  2

#define CMD_UPGRATE_PREPARE                 (0x80F0)
#define CMD_UPGRATE_DATA                    (0x80F1) 
#define CMD_UPGRATE_COMPLETE                (0x80FF) 
#define BMS_CMD_ADDRESSING                  (0X000D)
#define ROMOTE_SHUT_DOWN                    (0X000E)  



#define UPDATA_MAIN_TYPE                    0X00UL
#define UPDATA_PCS_TYPE                     0X01UL
#define UPDATA_ARC_TYPE                     0X02UL
#define UPDATA_BMS_TYPE                     0X03UL
#define UPDATA_FPGA_TYPE                    0X04UL
#define UPDATA_SW_TYPE                      0X05UL
#define UPDATA_MAIN_COP_TYPE                0X80UL   //监控卡协处理器
#define UPDATA_PCS_COP_TYPE                 0X81UL   //PCS协处理器
#define UPDATA_UI_TYPE                      0XF0UL   //图库
#define UPDATA_UI_TYPE1                     0XF1UL   //字库
#define UPDATA_NO_TYPE                      0XFFUL


#define UPGRATING                           1
#define NOUPGRATE                           0



#define UPGRADE_SEND_PCS_ERR                (1ul<<0)    //0X01
#define UPGRADE_SEND_ARC_ERR                (1ul<<1)    //0X02
#define UPGRADE_SEND_BMS_ERR                (1ul<<2)    //0X03
#define UPGRADE_SEND_MAIN_COP_ERR           (1ul<<3)    //0X04
#define UPGRADE_SEND_PCS_COP_ERR            (1ul<<4)    //0X05
#define UPGRADE_SEND_ADD_BMS_ERR            (1ul<<5)    //0X06
#define UPGRADE_MAIN_CHECK_ERR              (1ul<<6)    //0X07
#define UPGRADE_ZIP_ERR                     (1ul<<7)    //0X08
#define UPGRADE_CHECK_ERR                   (1ul<<8)    //0X09
#define UPGRADE_BMS_FLAG_ERR                (1ul<<9)    //0X0A
#define UPGRADE_PCS_FLAG_ERR                (1ul<<10)   //0X0B
#define UPGRADE_ARC_FLAG_ERR                (1ul<<11)   //0X0C
#define UPGRADE_MAIN_COP_FLAG_ERR           (1ul<<12)   //0X0D
#define UPGRADE_PCS_COP_FLAG_ERR            (1ul<<13)   //0X0E
#define UPGRADE_ADRESS_ERR                  (1ul<<14)   //0X0F 



/**************************升级标志********************************/
#define REST_SYS_MASK_BIT                   (1UL<<0)  
#define REST_EN_MASK_BIT                    (1UL<<1) 
#define REST_GET_STATUS_MASK_BIT            (1UL<<2)
#define REST_TEST_STATUS_MASK_BIT           (1UL<<3)
#define REST_EN_FIRST_PACK_BIT_MASK         (1UL<<4)

#define MAIN_COP_FIRMWARE_BIT_MASK          (1UL<<5)
#define PCS_COP_FIRMWARE_BIT_MASK           (1UL<<6)
#define ARC_FIRMWARE_BIT_MASK               (1UL<<7)
#define BMS_FIRMWARE_BIT_MASK               (1UL<<8)
#define FPGA_FIRMWARE_BIT_MASK              (1UL<<10)
#define PCS_FIRMWARE_BIT_MISK               (1UL<<11)
#define FIRMWARE_SEND_OVER_BIT_MASK         (1UL<<12)  //下发完成


#define GET_FIMWARE_STATUS_BIT_MASK         ((1UL<<13)|(1UL<<14))


typedef enum
{
   EN_FIRMWARE_UPGRATE_STATUS_IDLE = 0,
   EN_FIRMWARE_UPGRATE_STATUS_CHECK,
   EN_FIRMWARE_UPGRATE_STATUS_DEZIP,
   EN_FIRMWARE_UPGRATE_STATUS_TO_DO,
   EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE,
   EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_PREPARE_ACK,
   EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_DATA,
   EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_DATA_ACK,
   EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_END,
   EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_END_ACK,
   EN_FIRMWARE_UPGRATE_STATUS_MAINCTL,
   EN_FIRMWARE_UPGRATE_STATUS_ERROR,
   EN_FIRMWARE_UPGRATE_STATUS_OK,
   EN_FIRMWARE_UPGRATE_STATUS_DEZIP_IMG_FONT,
   EN_FIRMWARE_UPGRATE_STATUS_WAIT_RESTART,
   EN_FIRMWARE_UPGRATE_STATUS_RESET,
//   EN_FIRMWARE_ADDRESSING_PREPARE,
//   EN_FIRMWARE_ADDRESSING,
//   EN_FIRMWARE_ADDRESSING_ACK,
//   EN_FIRMWARE_ADDRESSING_ERROR,
} EN_FIRMWARE_UPGRATE_STATUS;


typedef enum
{
   EN_FIRMWARE_RESET_START = 0,
   EN_CHECK_SEND_OVER_BIT,
   EN_GET_FIRMWARE_RESET_STATUS, 
   EN_SEND_BMS_PCS_RESET_CMD,
   EN_WAIT_PCS_RESET_DELAY,
   EN_SEND_FIRST_BMS_RESET,
   EN_WAIT_FIRST_BMS_RESET,
   EN_WAIT_MAIN_COP_RESET,
   EN_MOINTOR_RESET,
   EN_MOINTOR_RESET_OVER,

} EN_FIRMWARE_RESET_STATUS;

typedef union
{
    struct
    {
        uint8_t  bms_type:1;
        uint8_t  sw_type:1;
        uint8_t  fpga_type:1;
        uint8_t  pcs_type:1;
        uint8_t  arc_type:1;
        uint8_t  main_cop_type:1;
        uint8_t  pcs_cop_type:1;
    }bits;
    uint8_t packet_type;
    
}tuUpgradePacketType;

			
typedef struct _firmware_upgrate_info_stru_t
{

    uint8_t         enter_updata_buad_mode;
    uint8_t         rev_start_flag; /*接收到升级提示命令后，置1*/
    uint8_t         rev_end_flag;/*接收到升级包发完命令后，置1*/
    uint32_t        write_addr;
    
    uint32_t        file_size;          //已经获取到的升级文件大小。
    uint32_t        file_check_sum;
    
    uint32_t        file_size_read;
    uint32_t        package_count;
    
    uint8_t         bms_data_existed;
    uint32_t        bms_data_offset;//包含文件头
    uint32_t        bms_data_len;//包含文件头
    uint32_t        bms_data_checksum;


    uint8_t         fpga_data_existed;
    uint32_t        fpga_data_offset;
    uint32_t        fpga_data_len;//包含文件头
    uint32_t        fpga_data_checksum;


    uint8_t         slave_prepared_ack;//1表示准备好，2表示当前不允许升级
    uint8_t         slave_data_ack;//1表示数据接收OK，2表示数据有误
    uint8_t         slave_end_ack;//1表示数据包整体无误，2表示数据有误
    uint8_t         bms_addressing_ack;



    uint32_t        slave_data_offset;
    uint32_t        slave_len;
    uint32_t        slave_seek;
    uint32_t        slave_checksum; 

    uint8_t         pcs_existed;
    uint32_t        pcs_data_offset;
    uint32_t        pcs_len;
    uint32_t        pcs_checksum;
    
    uint8_t         main_cop_existed;       //监控卡协处理器
    uint32_t        main_cop_data_offset;
    uint32_t        main_cop_len;
    uint32_t        main_cop_checksum;
    
    uint8_t         pcs_cop_existed;       //PCS协处理器
    uint32_t        pcs_cop_data_offset;
    uint32_t        pcs_cop_len;
    uint32_t        pcs_cop_checksum;

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
 

    uint8_t         updata_slave_type;        //当前正在下发固件类型

    uint8_t         main_ctl_data_existed;
    uint32_t        main_ctl_data_offset;//包含文件头
    uint32_t        main_ctl_data_len;//包含文件头
    uint32_t        main_ctl_data_checksum;

    uint8_t         upgrate_end_flag;           /*1:升级成功标志，2:升级失败标志*/
    uint32_t        updata_get_total_size;   //从采集器获取到的文件大小。
    uint8_t         updata_get_persent;      //获取到的升级文件百分比。
    uint32_t        updata_send_total_size;  //监控卡下发给固件总文件大小。
    uint32_t        updata_send_size;        //已经下发文件大小
    uint8_t         updata_send_persent;     //已经下发文件百分比
    uint32_t        updata_dezip_file_size;  //解压之后文件大小。
    uint8_t         re_send_cnt;
    uint16_t        firmware_updata_pack_num;
    uint32_t        size_per_transfer;
    uint8_t         updata_protocl_space_num;
    uint8_t         upgrading_status;
    uint32_t        upgrad_err_code;
    uint8_t         upgrad_resting;


    uint32_t        updata_send_base_addr;           //数据地址
    EN_FIRMWARE_RESET_STATUS    updata_reset_status;
    tuUpgradePacketType sys_firmware_packet_type;
}FirmwareUpgrateInfo_t;


extern FirmwareUpgrateInfo_t gstFirmwareUpgrateInfo;
void FirmwareUpgrateInit(void);
uint8_t GetAdressStatus(void);
uint16_t UpgratePrepare(void);
uint16_t UpgrateDataDeal(uint8_t *pSrc, uint16_t rev_len);
uint16_t UpgrateRevComplete(void);
void BmsReadHistoryCap(uint8_t *p_data,uint16_t size);
void UpgrateFirmwareProcess(void);
uint8_t GetUpdataBaudStatusInfo(void);
bool SetUpdataFirmwareType(uint16_t firmware_type,uint8_t confirm_status);
uint16_t GetCurUpdataType(void);
EN_FIRMWARE_UPGRATE_STATUS GetCurUpdataStatus(void);
int32_t UpdateIcon(void);
uint8_t  GetUpdataStatus(void);
uint8_t GetPcsRomoteAck(void);
void CleanPcsRmoteAck(void);

uint8_t GetUpgradeType(void);
#endif







