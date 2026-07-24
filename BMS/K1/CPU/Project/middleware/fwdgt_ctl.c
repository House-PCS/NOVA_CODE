

/* Includes ------------------------------------------------------------------*/
#include "fwdgt_ctl.h"
#include "dev_spiflash.h"
#define LOG_TAG     "fwdgt_control"


void ClearFwdgtErrCnt()
{
    static uint32_t wait_time = 0;        //开机等待时间
    static uint8_t first_run_flag = 0;    //首次运行标志
    FwdgtInfo_t gFwdgtInfo;               //看门狗信息
    uint8_t sum = 0;                      //和校验
    
    //300s以内，或者已经运行过一次，不会运行
    if((first_run_flag == 1) || (tim_difftime_from_now(wait_time) < 300000))
    {
        return;
    }
    //置起运行标志
    first_run_flag = 1;
    //组包
    gFwdgtInfo.header = FWDGT_CNT_HEADER;
    gFwdgtInfo.fwdgt_cnt = 0;
    sum = CheckSumFun((uint8_t*)&gFwdgtInfo, sizeof(FwdgtInfo_t)-1, 0);
    gFwdgtInfo.sumcheck = sum;
    //写入flash
    spi_flash_write_bytes_with_erase(FWDGT_SPI_FLASH_ADDR, (uint8_t*)&gFwdgtInfo, sizeof(FwdgtInfo_t));
}


