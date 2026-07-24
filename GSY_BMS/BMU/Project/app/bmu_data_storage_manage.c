/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bmu_data_storage_manage.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-18
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-18     <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "bmu_data_storage_manage.h"


#define FLASH_BMU_ADDR_MASK_INFO_VALID_FLAG_ADDR       (0x42000)
#define FLASH_BMU_ADDR_MASK_INFO_ADDR                  (FLASH_BMU_ADDR_MASK_INFO_VALID_FLAG_ADDR + 0x100)

//BMU采样掩码
static const BMUAddrMaskInfo_t _gBMUDefaultAddrMaskInfo = 
{
    0, //sa, 0代表尚未获取编址
    {0xFF, 0x7F, 0xFF, 0x7F}, //cell_volt_mask
    {0xFF, 0xE3, 0x7F, 0x00},//cell_temp_mask,包含极柱和正负极
    {0x00, 0x22, 0x00, 0x00},//pole_temp_mask
    {0x01, 0x00, 0x40, 0x00},//other_temp_mask
    {0x0F},//balance_temp_mask
};







/** 
 * @brief    判断FLASH中对应数据是否有效
 * @param    addr:FLASH对应扇区首地址
 * @return   是否有效，true代表有效，false代表无效
 * @note:
 */
static bool _FlashDataValidJudge(uint32_t addr)
{
    uint8_t temp[256];
    
    memset(temp, 0x00, 256);
    SpiFlashReadData(addr, temp, 256);
    for(uint16_t i = 0; i < 256; i++)
    {
        if(temp[i] != 0x0A)
        {
            return false;
        }
    }
    return true;
}



/** 
 * @brief    上电从存储器加载相关数据
 * @param    none
 * @return   none
 * @note:
 */
void DataStorageInfoInit()
{
    //bmu采样掩码信息
    if(_FlashDataValidJudge(FLASH_BMU_ADDR_MASK_INFO_VALID_FLAG_ADDR) == true)
    {
        SpiFlashReadData(FLASH_BMU_ADDR_MASK_INFO_ADDR, (uint8_t *)&gBMUDataInfo.bmu_addr_mask_info, sizeof(BMUAddrMaskInfo_t));
    }
    else
    {
        memcpy((uint8_t *)&gBMUDataInfo.bmu_addr_mask_info, &_gBMUDefaultAddrMaskInfo, sizeof(BMUAddrMaskInfo_t));
    }
}





/** 
 * @brief    BMU采样掩码信息烧写
 * @param    p_buf：数据内容
 * @param    size：数据长度
 * @return   none
 * @note:    none
 */ 
void BMUSampleInfoStorge(BMUAddrMaskInfo_t *p_addr_mask_info)
{
    uint8_t buf[256];
    //擦除
    SpiFlashEraseSector(FLASH_BMU_ADDR_MASK_INFO_VALID_FLAG_ADDR);
    //写入数据
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_BMU_ADDR_MASK_INFO_ADDR, (uint8_t *)p_addr_mask_info, sizeof(BMUAddrMaskInfo_t));
    //写入标记
    memset(buf, 0x0A, 256);
    SpiFlashWriteBytesWithoutErasenoAligned(FLASH_BMU_ADDR_MASK_INFO_VALID_FLAG_ADDR, buf, 256);
}






