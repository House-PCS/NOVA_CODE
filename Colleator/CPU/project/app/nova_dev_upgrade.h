#ifndef _NOVA_DEV_UPGRADE_H_
#define _NOVA_DEV_UPGRADE_H_

#include <stdint.h>

#define NOVA_DEV_UPGRADE_CMD_FIRMWARE_START		0xF080
#define NOVA_DEV_UPGRADE_CMD_FIRMWARE_SEND		0xF180
#define NOVA_DEV_UPGRADE_CMD_FIRMWARE_END		0xFF80

typedef enum 
{
	NOVA_DEV_UPGRADE_STAT_NONE = 0,
	NOVA_DEV_UPGRADE_STAT_PREPARE,
	NOVA_DEV_UPGRADE_STAT_PREPARE2,
	NOVA_DEV_UPGRADE_STAT_START,
	NOVA_DEV_UPGRADE_STAT_SEND,
	NOVA_DEV_UPGRADE_STAT_END,
	NOVA_DEV_UPGRADE_STAT_COMPLETE,
} EN_NOVA_DEV_UPGRADE_STAT;

typedef enum
{
	NOVA_DEV_UPGRADE_TYPE_NONE = 0,
	NOVA_DEV_UPGRADE_TYPE_FIRMWARE,
	NOVA_DEV_UPGRADE_TYPE_RES,
} EN_NOVA_DEV_UPGRADE_TYPE;


#pragma pack(1)
typedef struct _NovaDevUpgradeDataAttr_t
{
	int16_t seq;
	uint8_t data[0];
} NovaDevUpgradeDataAttr_t;

typedef struct _NovaDevUpgradePkg_t
{
	uint8_t head;

	uint8_t addr_src;
	uint8_t addr_dest;

	uint16_t cmd;
	uint16_t data_len;

	uint8_t data[0];

} NovaDevUpgradePkg_t;

typedef struct _NovaDevUpgradeAck_t
{
	uint8_t head;

	uint8_t addr_src;
	uint8_t addr_dest;

	uint16_t cmd;
	uint8_t errcode;

	uint16_t check_sum;
} NovaDevUpgradeAck_t;

typedef struct _NovaDevUpgradeCtx_t
{
	int32_t flag;				// 标记当前CTX是否可用
	int32_t stat;				// 升级状态
	int32_t type;				// 升级类型
	uint32_t total_size;		// 当前升级包总字节
	uint32_t download_size;		// 当前升级包下载的总字节
	uint32_t send_size;			// 当前升级包预发送字节
	uint32_t pre_send_size;		// 当前升级包已发送字节
	uint8_t md5[16];			// 当前升级包MD5
	uint8_t data[0];			// 当前升级包指针
} NovaDevUpgradeCtx_t;

#pragma pack()


void NovaDevUpgradeCtxReset();

int32_t NovaDevUpgradeStatSet(EN_NOVA_DEV_UPGRADE_STAT stat);
EN_NOVA_DEV_UPGRADE_STAT NovaDevUpgradeStatGet();
int32_t NovaDevUpgradeTypeSet(EN_NOVA_DEV_UPGRADE_TYPE type);
EN_NOVA_DEV_UPGRADE_TYPE NovaDevUpgradeTypeGet();


int32_t NovaDevUpgradeFirmwareSetSize(uint32_t size);
uint32_t NovaDevUpgradeFirmwareGetSize();

int32_t NovaDevUpgradeFirmwareSendData(uint8_t *data, uint16_t len);
int32_t NovaDevUpgradeFirmwareSetBaud(uint32_t idx_baud);
int32_t NovaDevUpgradeFirmwareStart();
int32_t NovaDevUpgradeFirmwareSendDebug();
int32_t NovaDevUpgradeGetSpiFlashMd5(uint8_t *md5, uint32_t total_size);
int32_t NovaDevUpgradeFirmwareSend(int32_t resend);
int32_t NovaDevUpgradeFirmwareEnd();
int32_t NovaDevUpgradeIsComplete();


uint32_t NovaDevUpgradeSpiFlashAddr();

int32_t NovaDevCleanUpgradeFlashSector();

int32_t NovaDevUpgradeSpiFlashSaveCtxDebug(uint32_t len);
int32_t NovaDevUpgradeSpiFlashSaveCtx(uint32_t len, uint8_t md5[16]);
int32_t NovaDevUpgradeSpiFlashWrite(uint8_t *data, uint32_t len, int32_t back_size);
uint32_t NovaDevUpgradeSpiFlashRead(uint8_t *data, uint32_t len, uint32_t idx);

int32_t NovaDevUpgradeGetErrorCode(uint8_t *data, int32_t len);
int32_t NovaDevUpgradeGetCmd(uint8_t *data, int32_t len);

#endif /* _NOVA_DEV_UPGRADE_H_ */
