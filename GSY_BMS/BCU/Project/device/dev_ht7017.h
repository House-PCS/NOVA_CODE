/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_ht7017.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-09
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09     <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __DEV_HT7017_H_
#define __DEV_HT7017_H_

#include "mcu_bsp.h"

#define HT_HEAD                 0x6A

#define HT_ACK_OK               0x54
#define HT_ACK_ERROR            0x63

#define HT_CMD_SPL_I1           0x00
#define HT_CMD_SPL_I2           0x01
#define HT_CMD_SPL_U            0x02
#define HT_CMD_RMS_I1           0x06
#define HT_CMD_RMS_I2           0x07
#define HT_CMD_RMS_U            0x08

#define HT_CMD_WPCFG            0x32
#define HT_CMD_MODULE_EN        0x42
#define HT_CMD_ANAEN            0x43




#define HT_VOLT_COVERT         (900 / pow(2, 18))

typedef struct _ht_uart_rev_msg_stru_t
{
    uint8_t     *p_buf;
    uint16_t    buf_depth;
    uint16_t    valid_num;
    uint16_t    buf_in_addr;
    uint16_t    buf_out_addr;
}HTUartRevMsg_t;


typedef enum
{
    EN_DEV_HT7017_STAT_IDLE,
    EN_DEV_HT7017_STAT_WAIT_WRITE_ACK,
    EN_DEV_HT7017_STAT_WAIT_READ_ACK,
    EN_DEV_HT7017_STAT_ACKED,
    EN_DEV_HT7017_STAT_NO_ACK
}EN_DEV_HT7017_STAT;


typedef enum
{
    EN_DEV_HT7017_ACK_RES_NONE,//尚未回复
    EN_DEV_HT7017_ACK_RES_INVALID_CHECKSUM,//校验不对
    EN_DEV_HT7017_ACK_RES_ABNORMAL,//芯片回复异常，如字节数不匹配等
    EN_DEV_HT7017_ACK_RES_TIMEOUT,//回复超时(即不回复)
    EN_DEV_HT7017_ACK_RES_OK
}EN_DEV_HT7017_ACK_RES;


void DevHT7017Init(void);
void DevHT7017WriteCmd(uint8_t cmd, uint8_t *p_data);
void DevHT7017ReadCmd(uint8_t cmd, uint8_t *p_data);
EN_DEV_HT7017_ACK_RES DevHT7017WriteAck(void);
EN_DEV_HT7017_ACK_RES DevHT7017ReadAck(uint8_t *p_data);

void DevHT7017Process(void);








#endif

