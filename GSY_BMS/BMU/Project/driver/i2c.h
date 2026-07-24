
#ifndef __I2C_H_
#define __I2C_H_

#include "gd32f30x_i2c.h"
#include "gd32f30x_gpio.h"
#include "stdbool.h"
#include "drv_opt.h"


typedef enum
{
    I2C0_PORT,
    I2C1_PORT,
    I2C2_PORT,
    I2C_NUM
}EN_I2Cx_PORT;


typedef struct _i2c_config_stru_t
{
    uint32_t                scl_gpiox;
    uint32_t                scl_gpio_pinx;
    uint32_t                scl_alt_num;
    
    uint32_t                sda_gpiox;
    uint32_t                sda_gpio_pinx;
    uint32_t                sda_alt_num;
    
    rcu_periph_enum         rcu_i2c;
    uint32_t                i2c_peri;
    uint32_t                speed;
    
    
    bool                    en_irq;
    uint32_t                nvic_irq;
    uint8_t                 irq_prio;
    uint8_t                 irq_sub_prio;
    usart_interrupt_enum    irq_type;
}I2C_Config;



typedef struct _i2c_handle_stru_t
{
    EN_I2Cx_PORT            i2cx;
    I2C_Config              *i2c_cfg;
    
    uint32_t                slave_addr0;/*暂不支持双地址从机*/
    uint32_t                reg_addr;
    void                    *pTxBuf;
    uint32_t                tx_size;
    void                    *pRxBuf;
    uint32_t                rx_size;
}I2C_HandleType;




void I2CInit(I2C_HandleType *pHandle);
bool I2CRegWrite(I2C_HandleType *pHandle);
bool I2CRegRead(I2C_HandleType *pHandle);

#endif





















