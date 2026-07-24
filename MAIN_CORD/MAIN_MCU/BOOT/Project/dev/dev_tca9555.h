/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_tca9555.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-03-21
  * @brief       GD32F470
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-03-21    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __DEV_TCA9555_H_
#define __DEV_TCA9555_H_

#include "gpio.h"
#include "bsp.h"
#include "i2c.h"

#define TCA9555_ADDR				(0x40)


#define USE_TCA9555_A				(1)

#define	TCA9555_A_PIN_MODE			(0xFF)
#define	TCA9555_A_OUTPUT_DATA		(0xFF)
#define	TCA9555_A_INPUT_DATA		(0xFF)


#define PORT0_INPUT_REG             (0x00)
#define PORT1_INPUT_REG             (0x01)
#define PORT0_OUTPUT_REG            (0x02)
#define PORT1_OUTPUT_REG            (0x03)
#define PORT0_POL_REG               (0x04)
#define PORT1_POL_REG               (0x05)
#define PORT0_MODE_CFG              (0x06)
#define PORT1_MODE_CFG              (0x07)


typedef enum
{
#if USE_TCA9555_A
		EN_TCA9555_A_INDEX,
#endif
#if USE_TCA9555_B
	EN_TCA9555_B_INDEX,
#endif
	
	EN_TCA9555_INDEX_NUM,
}en_tca9555_index_t;


typedef enum
{
	EN_TCA9555_CH_00 = 0x00,
    EN_TCA9555_CH_01,
    EN_TCA9555_CH_02,
    EN_TCA9555_CH_03,
    EN_TCA9555_CH_04,
    EN_TCA9555_CH_05,
    EN_TCA9555_CH_06,
    EN_TCA9555_CH_07,
    EN_TCA9555_CH_10,
    EN_TCA9555_CH_11,
    EN_TCA9555_CH_12,
    EN_TCA9555_CH_13,
    EN_TCA9555_CH_14,
    EN_TCA9555_CH_15,
    EN_TCA9555_CH_16,
    EN_TCA9555_CH_17,
    EN_TCA9555_CH_NUM,
    EN_TCA9555_CH_INVALID = 0xFF,
} EN_TCA9555_CH;


typedef struct _tca_9555_config_t
{
	uint32_t 	n_dev_id;
	uint8_t 	n_pin_mode;  
	uint8_t   	n_output_data;
	uint8_t  	n_input_data;
	
} DevTca9555Config_t;


int32_t DevTca9555DevConfig(void);

int32_t DevTca9555SetPort0Mode(uint8_t n_mode);
int32_t DevTca9555GetPort0Status(void);
int32_t DevTca9555SetPort0Status(uint8_t c_value);
int32_t DevTca9555SetPort0SinglePinMode(EN_TCA9555_CH ch, uint8_t c_mode);
int32_t DevTca9555GetPort0SinglePinStatus(EN_TCA9555_CH ch);
int32_t DevTca9555SetPort0SinglePinStatus(EN_TCA9555_CH ch, uint8_t c_value);

int32_t DevTca9555SetPort1Mode(uint8_t c_mode);
int32_t DevTca9555GetPort1Status(void);
int32_t DevTca9555SetPort1Status(uint8_t c_value);
int32_t DevTca9555SetPort1SinglePinMode(EN_TCA9555_CH ch, uint8_t c_mode);
int32_t DevTca9555GetPort1SinglePinStatus(EN_TCA9555_CH ch);
int32_t DevTca9555SetPort1SinglePinStatus(EN_TCA9555_CH ch, uint8_t c_value);
#endif








