/*
 * cpu2_upgrade_share.h
 *
 * CPU1/CPU2 升级共享 RAM 布局（GS3 控制 / GS4 数据），两核共用头文件。
 */
#ifndef APP_CPU2_UPGRADE_SHARE_H_
#define APP_CPU2_UPGRADE_SHARE_H_

#include <stdint.h>

#define CPU2_UPG_MAGIC              0x43503255U  /* "CPU2U" */
#define CPU2_UPG_ACK_INVALID        0xFFU
#define CPU2_UPG_ACK_OK             0U
#define CPU2_UPG_ACK_ERR            1U

#define CPU2_UPG_GS3_ADDR           0x0000F000UL
#define CPU2_UPG_GS4_ADDR           0x00010000UL
#define CPU2_UPG_DATA_BUF_WORDS     (2048U + 16U)

typedef enum
{
    CPU2_UPG_CMD_NONE = 0,
    CPU2_UPG_CMD_START,
    CPU2_UPG_CMD_BUSY,
    CPU2_UPG_CMD_END,
} Cpu2UpgCmd_t;

typedef enum
{
    CPU2_UPG_ST_IDLE = 0,
    CPU2_UPG_ST_ERASE,
    CPU2_UPG_ST_PROG,
    CPU2_UPG_ST_VERIFY,
    CPU2_UPG_ST_OK,
    CPU2_UPG_ST_ERR,
} Cpu2UpgStatus_t;

typedef struct
{
    volatile uint32_t magic;
    volatile uint32_t cmd;
    volatile uint32_t seq;
    volatile uint32_t payload_len;
    volatile uint32_t upgrade_flag;
    volatile uint32_t status;
    volatile uint32_t ack;
    volatile uint32_t reserved[8];
} Cpu2UpgradeCtrl_t;

typedef struct
{
    volatile uint16_t data[CPU2_UPG_DATA_BUF_WORDS];
} Cpu2UpgradeData_t;

#endif /* APP_CPU2_UPGRADE_SHARE_H_ */
