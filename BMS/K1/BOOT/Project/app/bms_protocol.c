
#include "bms_protocol.h"
#include "fwdgt.h"
#include "flash.h"
#include <stdio.h>
#include <string.h>

#define LOG_TAG     "bms_protocol"

BMSSysInfo_t   _gBMSSysInfo;
FwdgtInfo_t     gFwdgtInfo;

/**
* @brief  : 重启标志设置
* @para    : None
 * @note   : None
 * @return : None
 */
void ResetFlagSet()
{
    //设置重启标志及重启时间
    _gBMSSysInfo.bms_reset_flag = true;
    _gBMSSysInfo.sys_reset_delay_times = tim_get_systime();
}



/**
 * @brief  : 自动重启处理
 * @para   :
 * @note   : None
 * @return : None
 */
void SysResetProcess()
{
    static bool process_flag = 1;       //重启处理标志
    
    //第一个电池包只能cmd重启，其余电池包在这里重启
    if((gUartPackInfo.ModbusAddr == MODBUS_ADDR_PACK0) || (gUartPackInfo.ModbusAddr == MODBUS_ADDR_DEFAULT_VALUE))
    {
        return;
    }
    
    //重启标志未置起，返回
    if(_gBMSSysInfo.bms_reset_flag == false)
    {
        return;
    } 
    
    if(tim_difftime_from_now(_gBMSSysInfo.sys_reset_delay_times) > 5000)
    {
        __set_PRIMASK(1);   // 关闭总中断
        NVIC_SystemReset();
    }

}

/**
* @brief CheckSumFun
* @param *pdata: data
* @param len: len
* @param mode: 0-checksum16     1-checksum32
* @return
* - checksum
* @note
*/
static uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode)
{
    uint16_t checksum16 = 0;
    uint32_t checksum32 = 0;
    uint16_t i = 0;

    if(mode)
    {
        for(i = 0; i < len; i ++)
        {
            checksum32 += pdata[i];
        }

        return checksum32;
    }
    else
    {
        for(i = 0; i < len; i ++)
        {
            checksum16 += pdata[i];
        }

        return checksum16;
    }
}



/**
 * @brief  : 自动重启处理
 * @para   :
 * @note   : None
 * @return : None
 */
uint8_t FwdgtFlagProcess()
{
    FwdgtInfo_t info;
    uint8_t sum = 0;
    //读取
    spi_flash_read_bytes(FWDGT_SPI_FLASH_ADDR, (uint8_t*)&info, sizeof(FwdgtInfo_t));
    //和校验
    sum = CheckSumFun((uint8_t*)&info, sizeof(info)-1, 0);
    
    //正确处理
    if((info.header == FWDGT_CNT_HEADER) && (sum == info.sumcheck))
    {
        memcpy((uint8_t*)&gFwdgtInfo, (uint8_t*)&info, sizeof(FwdgtInfo_t));
    }
    else
    {   
        //错误处理
        gFwdgtInfo.header = FWDGT_CNT_HEADER;
        gFwdgtInfo.fwdgt_cnt = 0;
        sum = CheckSumFun((uint8_t*)&gFwdgtInfo, sizeof(FwdgtInfo_t)-1, 0);
        gFwdgtInfo.sumcheck = sum;
        spi_flash_write_bytes_with_erase(FWDGT_SPI_FLASH_ADDR, (uint8_t*)&gFwdgtInfo, sizeof(FwdgtInfo_t));
    }
    
    if(gFwdgtRstFlag == 1)
    {
        if(gFwdgtInfo.fwdgt_cnt >= 3)
        {
            gFwdgtInfo.header = FWDGT_CNT_HEADER;
            gFwdgtInfo.fwdgt_cnt = 0;
            sum = CheckSumFun((uint8_t*)&gFwdgtInfo, sizeof(FwdgtInfo_t)-1, 0);
            gFwdgtInfo.sumcheck = sum;
            spi_flash_write_bytes_with_erase(FWDGT_SPI_FLASH_ADDR, (uint8_t*)&gFwdgtInfo, sizeof(FwdgtInfo_t));

            return 0;
        }
        else
        {
            gFwdgtInfo.header = FWDGT_CNT_HEADER;
            gFwdgtInfo.fwdgt_cnt++;
            sum = CheckSumFun((uint8_t*)&gFwdgtInfo, sizeof(FwdgtInfo_t)-1, 0);
            gFwdgtInfo.sumcheck = sum;
            spi_flash_write_bytes_with_erase(FWDGT_SPI_FLASH_ADDR, (uint8_t*)&gFwdgtInfo, sizeof(FwdgtInfo_t));
            return 1;
        }
    }
    
    return 1;
}

void ClearFwdgtCnt()
{
    gFwdgtInfo.fwdgt_cnt = 0;
    spi_flash_write_bytes_with_erase(FWDGT_SPI_FLASH_ADDR, (uint8_t*)&gFwdgtInfo, sizeof(FwdgtInfo_t));
}
