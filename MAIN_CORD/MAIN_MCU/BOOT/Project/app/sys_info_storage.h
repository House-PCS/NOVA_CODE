#ifndef SYS_INFO_STORAGE_HHH
#define SYS_INFO_STORAGE_HHH
#include    "dev_spiflash.h"
#include    "slave_collect.h"

#define FLASH_PAGE_SIZE         256     

#define MAIN_CTL_FACTORY_INFO_FLASH_ADDR                               0x200000
#define MAIN_CTL_FACTORY_INFO_FLASH_BACKUP_ADDR                        0x201000
                          
#define MAIN_CTL_PARA_INFO_FLASH_ADDR                                  0x202000
#define MAIN_CTL_PARA_INFO_FLASH_BACKUP_ADDR                           0x203000


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
}tuSoftwareVer;



#pragma pack()


void SysInfoInit(void);
void DeviceBaseInfoWrite(void);
void DeviceCtlWrite(void);
#endif