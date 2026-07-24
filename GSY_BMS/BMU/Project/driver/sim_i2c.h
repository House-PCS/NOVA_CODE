
#ifndef __SIM_I2C_H_
#define __SIM_I2C_H_

#include "gpio.h"
#include "systick.h"




typedef struct _simi2c_config_stru_t
{
    uint32_t                scl_gpiox;
    uint32_t                scl_gpio_pinx;
    
    uint32_t                sda_gpiox;
    uint32_t                sda_gpio_pinx;
    
}SimI2C_Config;




typedef struct _simi2c_handle_stru_t
{
    SimI2C_Config           *simi2c_cfg;
    uint32_t                slave_addr0;/*暂不支持双地址从机*/
    uint32_t                reg_addr;
    void                    *pTxBuf;
    uint32_t                tx_size;
    void                    *pRxBuf;
    uint32_t                rx_size;
}SimI2C_HandleType;


void SimI2CInit(SimI2C_HandleType *pHandle);

bool SimI2CRegWrite(SimI2C_HandleType *pHandle);
bool SimI2CRegRead(SimI2C_HandleType *pHandle);






#endif






















