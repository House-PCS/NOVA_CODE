
#ifndef __DEV_FLASH_EEPROM_ADDR_H_
#define __DEV_FLASH_EEPROM_ADDR_H_


/******************************Flash Addr***********************************/
#define SOX_FLASH_INFO_ADDR                  (0x00102000)
#define SOX_FLASH_INFO_FLAG_ADDR             (0x00104000)
#define SOX_FLASH_BACKUP_INFO_ADDR           (0x00106000)
#define SOX_FLASH_BACKUP_INFO_FLAG_ADDR      (0x00108000)
                                             
#define SOX_FLASH_INFO_ADDR_IN_UPGRADE       (0x0010A000)
                                             
#define ERROR_INFO_1_FLASH_ADDR              (0x0010B000)

#define ENDURANCE_TEST_ERROR_INFO_FLASH_ADDR (0x00110000)   //ƒÕæ√∂»≤‚ ‘π ’œ¥Ê¥¢µÿ÷∑

/******************************Eeprom Addr******************************/
#define ERROR1_SAVE_ADDR                    (0x00000000)
#define ERROR2_SAVE_ADDR                    (0x00000008)
#define ERROR3_SAVE_ADDR                    (0x00000010)
#define ERROR4_SAVE_ADDR                    (0x00000018)
#define ERROR5_SAVE_ADDR                    (0x00000020)
#define ERROR6_SAVE_ADDR                    (0x00000028)
#define ERROR7_SAVE_ADDR                    (0x00000030)
#define ERROR8_SAVE_ADDR                    (0x00000038)
#define ERROR9_SAVE_ADDR                    (0x00000040)
#define ERROR10_SAVE_ADDR                   (0x00000048)
#define ERROR11_SAVE_ADDR                   (0x00000050)
#define ERROR12_SAVE_ADDR                   (0x00000058)
#define ERROR13_SAVE_ADDR                   (0x00000060)
#define MAX_ERROR_SAVE_ADDR_NUM             (13)

#define SOX_FIRST_FIX_ADDR                  (0x00000100)

#define SOX_EEPROM_INFO_FLAG_ADDR           (0x00000200)
#define SOX_EEPROM_INFO_ADDR                (0x00000204)
#define SOX_EEPROM_BACKUP_INFO_FLAG_ADDR    (0x00000400)
#define SOX_EEPROM_BACKUP_INFO_ADDR         (0x00000404)

#endif

















