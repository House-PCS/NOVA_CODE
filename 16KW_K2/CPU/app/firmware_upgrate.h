
#ifndef __FIRMWARE_UPGRATE_H_
#define __FIRMWARE_UPGRATE_H_
#include "dev_flash.h"
#include "private_analysis.h"
#include "ethernet_analysis.h"
#include "modbus_analysis.h"
#include "system.h"


#define UPGRADE_START           0x80F0
#define UPGRADE_BUSY            0x80F1
#define UPGRADE_END             0x80FF

#define COPRO_FIRMWARE_FRAME_ADDR       0x00100000
#define COPRO_FIRMWARE_ADDR             0x00101000
#define ARC_FIRMWARE_FRAME_ADDR         0x00080000
#define ARC_FIRMWARE_ADDR               0x00081000
#define DSP_FIRMWARE_FRAME_ADDR         0x00000000
#define DSP_FIRMWARE_ADDR               0x00001000

#define UPGRADE_FLAG   0xA0

typedef enum
{
    EN_ROCA3_PCS_16KHS = 0x43,
    EN_ROCA3_PCS_15KHT = 0xC0,
    EN_PCS_TYPE_NUM = 0xFF,
} EN_PCS_TYPE;

typedef enum
{
    EN_FIRMWARE_UPGRATE_STATUS_IDLE,//空闲模式
    EN_FIRMWARE_UPGRATE_STATUS_START,//升级开始
    EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE,//预备状态
    EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE_ACK,
    EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY,//发送数据
    EN_FIRMWARE_UPGRATE_STATUS_WAIT_DATA_BUSY_ACK,//发送中等待检查
    EN_FIRMWARE_UPGRATE_STATUS_SEND_END,//发送截止
    EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK,//发送截止检查
    EN_FIRMWARE_UPGRATE_STATUS_ERROR,//状态错误
    EN_FIRMWARE_UPGRATE_STATUS_OK,//OK
} EN_BMS_FIRMWARE_UPGRATE_STATUS;

typedef struct _firmware_upgrate_info_stru_t
{
    // uint8_t         arc_upgrate_en;
    // uint8_t         arc_existed;
    // uint32_t        arc_offset;
    // uint32_t        arc_firmware_len;
    // uint32_t        arc_checksum;
    // uint32_t        arc_seek;
    // uint8_t         arc_ack;
    // uint8_t         arc_end_ack;
    // uint8_t         arc_upg_state;  // 0:无效 1：存储flash成功 2：存储flash失败 3：下发子模块成功 4：下发子模块失败

    uint8_t         ack_upgrate_en;
    uint8_t         ack_existed;
    uint32_t        ack_offset;
    uint32_t        ack_firmware_len;
    uint32_t        ack_checksum;
    uint32_t        ack_seek;
    uint8_t         ack_ack;
    uint8_t         ack_end_ack;
    uint8_t         ack_upg_state;  // 0:无效 1：存储
    uint16_t        ack_pack_num;

} FirmwareUpgrateInfo_t;

typedef struct _firmware_recv_info_stru_t
{
    uint16_t        firmware_type[4];       //升级固件类型
    uint16_t        firmware_version[3];    //升级的版本号
    uint32_t        firmware_len;           //固件长度
    uint32_t        firmware_len_temp;      //固件长度临时值
    uint32_t        checksum;                   //32位校验加和
    uint32_t        checksumtemp;               //32位校验加和临时值
    uint16_t        upg_state;                     //监控升级完成状态 0：无效 1：升级中 2：升级成功 3：升级失败 4:转发成功 5：转发失败
} FirmwareRecvInfo_t;

typedef struct _firmware_recv_infos_stru_t
{
    FirmwareRecvInfo_t  pcs_firmware_info;
    FirmwareRecvInfo_t  arc_firmware_info;
    FirmwareRecvInfo_t  copro_firmware_info; // MCU协处理器固件信息 , coprocessor
} FirmwareRecvInfoS_t;

typedef struct _version_info_stru_t
{
    uint16_t        model_id;
    uint16_t        soft_version;       //软件版本
    uint16_t        extern_version[6];  //扩展版本
    uint16_t        device_type;        //设备类型 0x00-0x7F单相逆变器   0x80-0xFF三相逆变器
} VersionInfo_t;

typedef struct _version_infos_stru_t
{
    VersionInfo_t  pcs_version_info;
    VersionInfo_t  arc_version_info;
} VersionInfoS_t;


extern FirmwareRecvInfoS_t  gstFirmwareInfo;
extern VersionInfoS_t gstVersionInfo;
extern uint16_t garrDspFirmwareType[4];

void ArcUpgrateFirmwareProcess(void);
void CoproUpgrateFirmwareProcess(void);
void ArcUpgrateEnable(void);
bool EthernentUpgradeRecv(MonitorUdp_t *p_monitor_frame, uint16_t *p_data);
void FirmwareParamInit(void);
bool ArcFirmwareSending(void);
bool CoproFirmwareSending(void);
bool FirmwareUpgrateIsBusy(void);

#endif

