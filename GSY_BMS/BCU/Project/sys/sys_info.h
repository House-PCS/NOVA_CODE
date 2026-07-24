
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sys_info.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-5-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-6-25      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#ifndef __SYS_INFO_H_
#define __SYS_INFO_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <String.h>



typedef struct _sys_info_stru_t
{
    /*主控*/
    bool        bcu_enter_test_mode_ack;
    bool        bcu_exit_test_mode_ack;
    bool        bcu_can_test_ack;
    bool        bcu_rs485_1_test_ack;
    bool        bcu_rs485_2_test_ack;
    bool        bcu_eth_test_ack;
    bool        bcu_daisy_chain_test_ack;
    bool        bcu_high_voltage_test_ack;
    uint32_t    bcu_high_voltage_value[3];
    bool        bcu_ins_test_ack;//绝缘检测
    uint32_t    bcu_ins_res_value[2];//绝缘阻抗值(90~110KΩ)
    bool        bcu_cur_div_test_ack;//分流器检测
    uint32_t    bcu_cur_div_value;//分流器电压值(1972~2012mV)
    bool        bcu_hall_test_ack;//霍尔检测
    uint32_t    bcu_hall_value[3];
    bool        bcu_temp_test_ack;//温度检测
    uint8_t     bcu_temp_value[4];//4路温度值
    bool        bcu_io_input_test_ack;
    uint16_t    bcu_io_input_value;//主控有8个输入IO,取低位
    bool        bcu_io_output_test_ack;
    bool        bcu_eeprom_test_ack;
    uint8_t     bcu_eeprom_test_result;//01表示成功，02表示失败
    bool        bcu_flash_test_ack;
    uint8_t     bcu_flash_test_result;//01表示成功，02表示失败
    bool        bcu_rtc_test_ack;
    uint8_t     bcu_rtc_test_result;//01表示成功，02表示失败    
    /*从控*/
    bool        bmu_enter_test_mode_ack;
    bool        bmu_exit_test_mode_ack;
    bool        bmu_voltage_test_ack;//电压采集
    uint16_t    bmu_voltage_value[64];
    bool        bmu_temp_test_ack;//温度采集
    uint8_t     bmu_temp_value[68];
    bool        bmu_btemp_test_ack;//均衡温度采集
    uint16_t    bmu_btemp_value[8];
    bool        bmu_io_inout_test_ack;//均衡温度采集
    uint8_t     bmu_io_inout_value;//两路双向IO
    bool        bmu_io_output_test_ack;
    bool        bmu_pwm_test_ack;//PWM输出测试
    bool        bmu_io_input_test_ack;
    uint8_t     bmu_io_input_value;//从控有?个输入IO,取低位
    bool        bmu_rs485_test_ack;
    bool        bmu_flash_test_ack;
    uint8_t     bmu_flash_test_result;//01表示成功，02表示失败
}SysInfo_t;

extern SysInfo_t gSysInfo;

#endif











