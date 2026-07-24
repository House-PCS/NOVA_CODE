/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        i2c.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       GD32F30x
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion 
  * </table>
  ******************************************************************************
  */
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
    
    uint32_t                sda_gpiox;
    uint32_t                sda_gpio_pinx;
    
    uint8_t                 alt_rmp_mode;/*1:Alternate1, 2:Alternate2, 3:Remap*/
    uint32_t                alt_rmp_function;/*alt_rmp_mode=1时表示afio_function， alt_rmp_mode=3时表示remap function, alt_rmp_mode为其他值，该字段无意义*/
    
    rcu_periph_enum         rcu_i2c;
    uint32_t                i2c_peri;
    uint32_t                speed;
    
    
    bool                    en_irq;
    uint32_t                nvic_irq;
    uint8_t                 irq_prio;
    uint8_t                 irq_sub_prio;
    usart_interrupt_enum    irq_type;
}I2C_Config;


typedef enum
{
    EN_I2C_REG_ADDR_WIDTH_8,
    EN_I2C_REG_ADDR_WIDTH_16
}EN_I2C_REG_ADDR_WIDTH;


typedef struct _i2c_handle_stru_t
{
    EN_I2Cx_PORT            i2cx;
    I2C_Config              *i2c_cfg;
    
    uint32_t                slave_addr0;/*暂不支持双地址从机*/
    uint32_t                reg_addr;
    EN_I2C_REG_ADDR_WIDTH   reg_addr_width;
    void                    *pTxBuf;
    uint32_t                tx_size;
    void                    *pRxBuf;
    uint32_t                rx_size;
}I2C_HandleType;

void I2CTimeoutHndle(I2C_HandleType *pHandle);
void I2CInit(I2C_HandleType *pHandle);
bool I2CRegWrite(I2C_HandleType *pHandle);
bool I2CRegRead(I2C_HandleType *pHandle);
void I2CDeviceWaitStandby(I2C_HandleType *pHandle);
#endif





















