#ifndef SYS_INFO_STORAGE_HHH
#define SYS_INFO_STORAGE_HHH
#include    "dev_spiflash.h"
#include    "slave_collect.h"

#define FLASH_PAGE_SIZE         256     

#define APP_FIRMWARE_HEAD_INTERNAL_ADDR                                0xFF00//0xDF00
#define MAIN_CTL_FACTORY_INFO_FLASH_ADDR                               0x200000
#define MAIN_CTL_FACTORY_INFO_FLASH_BACKUP_ADDR                        0x201000
                          
#define MAIN_CTL_PARA_INFO_FLASH_ADDR                                  0x202000
#define MAIN_CTL_PARA_INFO_FLASH_BACKUP_ADDR                           0x203000


#pragma pack(2)

typedef union _sofeware_ver
{

    struct  
    {
        uint32_t ver_low:8;
        uint32_t ver_mid:8;
        uint32_t ver_hi:8;
    }bits;
    uint32_t    word_value;
} SoftwareVer_t;



#pragma pack()


void SysInfoInit(void);
void DeviceBaseInfoWrite(void);
void DeviceCtlWrite(void);
void DeviceCtlRead(void);
void McuVersionTypeInit(void);
void WriteInfoToStorgeData(CollectStorageInfo_t *pdst,CollectInfo_t *psrc);
void ReadDataToStorageData(CollectInfo_t *pdst,CollectStorageInfo_t *psrc);
void StorageDataRest(uint8_t user_type);
void StorgDezipFileSize(uint32_t size);
uint32_t GetDezipFileSize(void);

#endif