
#include "para_process.h"
#include "String.h"

#define SN_HEAD_ADDR             0x200000
#define SN_HEAD_BACKUP_ADDR      0x202000
#define SN_ADDR                  (SN_HEAD_ADDR + 0x200)
#define SN_BACKUP_ADDR           (SN_HEAD_BACKUP_ADDR + 0x200)

void SNRead(char *p)
{
    uint8_t buf[256];
    SpiFlashReadData(SN_HEAD_ADDR, buf, 4);
    if(buf[0] != 0x0A || buf[1] != 0x0A || buf[2] != 0x0A || buf[3] != 0x0A)
    {
        SpiFlashReadData(SN_HEAD_BACKUP_ADDR, buf, 4);
        if(buf[0] != 0x0A || buf[1] != 0x0A || buf[2] != 0x0A || buf[3] != 0x0A)
        {
            uint8_t i;
            /*默认SN*/
            for(i = 0; i < 20; i++)
            {
                p[i] = 'A';
            }
            p[i] = 0x00;
        }
        else
        {
            SpiFlashFastReadData(SN_BACKUP_ADDR, buf, 256);
            /*暂定不超过20个字符，不包含\0*/
            if(strlen((char *)buf) > 20)
            {
                memcpy(p, buf, 20);
                p[20] = 0x00;
            }
            else
            {
                strcpy((char *)p, (char *)buf);
            }
        }
    }
    else
    {
        SpiFlashReadData(SN_ADDR, buf, 256);
        /*暂定不超过20个字符，不包含\0*/
        if(strlen((char *)buf) > 20)
        {
            memcpy(p, buf, 20);
            p[20] = 0x00;
        }
        else
        {
            strcpy((char *)p, (char *)buf);
        }
    }
}

void SNWrite(uint8_t *p)
{
    uint8_t buf[256];
    uint8_t valid_flag[4];
    uint8_t i = 0;
    
    memset(buf, 0xFF, 256);
    for(i = 0; i < 4; i++)
    {
        valid_flag[i] = 0x0A;
    }
    
    if(strlen((char *)p) > 20)
    {
        memcpy(buf, p, 20);
        buf[20] = 0x00;
    }
    else
    {
        strcpy((char *)buf, (char *)p);
    }
    /*数据区域*/
    SpiFlashEraseSector(SN_HEAD_ADDR);
    SpiFlashWriteBytesWithoutErasenoAligned(SN_ADDR, buf, 256);
    SpiFlashWriteBytesWithoutErasenoAligned(SN_HEAD_ADDR, valid_flag, 4);
    
    /*备份数据区域*/
    SpiFlashEraseSector(SN_HEAD_BACKUP_ADDR);
    SpiFlashWriteBytesWithoutErasenoAligned(SN_ADDR, buf, 256);
    SpiFlashWriteBytesWithoutErasenoAligned(SN_HEAD_BACKUP_ADDR, valid_flag, 4);
    
    
}











