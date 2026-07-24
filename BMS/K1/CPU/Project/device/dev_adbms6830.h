/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_adbms6830.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-8-20     <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __dev_adbms6830_H_
#define __dev_adbms6830_H_


#include "bsp.h"



#define TOTAL_IC    (1)

typedef enum
{
	EN_AFE_COMM_STATUS_BUSY,//进行中
	EN_AFE_COMM_STATUS_COMPLETE,//通信完成
	EN_AFE_COMM_STATUS_ERROR,//通信错误
}EN_AFE_COMM_STATUS;

typedef enum
{
    EN_AFE_GPIO_ALL = 0x3FF,
    EN_AFE_GPIO_1 = (1 << 0),
    EN_AFE_GPIO_2 = (1 << 1),
    EN_AFE_GPIO_3 = (1 << 2),
    EN_AFE_GPIO_4 = (1 << 3),
    EN_AFE_GPIO_5 = (1 << 4),
    EN_AFE_GPIO_6 = (1 << 5),
    EN_AFE_GPIO_7 = (1 << 6),
    EN_AFE_GPIO_8 = (1 << 7),
    EN_AFE_GPIO_9 = (1 << 8),
    EN_AFE_GPIO_10 = (1 << 9),
}EN_AFE_GPIO;

//初始化
void DevAdbms6830Init(void);
//电压采样
void DevAFEVoltageSampleCmd(void);
EN_AFE_COMM_STATUS DevAFEVoltageSampleStat(uint16_t *p_voltage);
//GPIO输出控制
void DevAFEGpioIOValueCfg(uint8_t cic, EN_AFE_GPIO gpiox, uint8_t value);
bool DevAFEGpioIOCtl(void);
//GPIO采样，例如温度采样
void DevAFEAuxVoltageSampleCmd(EN_AFE_GPIO gpiox);
EN_AFE_COMM_STATUS DevAFEAuxVoltageSampleStat(uint16_t *p_voltage, EN_AFE_GPIO gpiox);
//均衡控制
bool DevAFEBalanceStart(uint16_t *p_dcc, uint16_t *p_rd_dcc);
bool DevAFEBalanceStop(uint16_t *p_rd_dcc);
//断线检测
bool DevAFEWireCheckPrep(void);
void DevAFEWireCheckCmd(uint8_t ch, uint8_t type);
EN_AFE_COMM_STATUS DevAFEWireCheckStat(uint8_t ch, uint8_t type);
void DevAFEWireCheckResultGet(uint16_t *p_res, uint8_t type);

//可变时长唤醒
void DevAFEWakeUpUs(uint32_t us);
void DevAFEWakeUpMs(uint32_t ms);

//AFE复位
void DevAFEReset(void);
//获取断线检测的最小值
void GetOwVal(uint16_t *p_temp);
    
#endif
