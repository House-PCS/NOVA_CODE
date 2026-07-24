/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __PRIVATE_ANALYSIS_H_
#define __PRIVATE_ANALYSIS_H_
#include "systick.h"
#include "modbus_analysis.h"
#include "comm_analysis.h"
#include "fpga_com.h"
#include "firmware_upgrate.h"
#include "mid_comservice.h"
#include "firmware_upgrate.h"
#include "dev_ddr.h"

#define REV_QUEUE_DEPTH   2
#define COM_RX_SIZE     1100

#define SLAVE_WRITE_QUEUE_DEPTH         2

#define BMS_CMD_UPGRATE_Start           0x80F0
#define BMS_CMD_UPGRATE_BUSY            0x80F1
#define BMS_CMD_UPGRATE_END             0x80FF
#define READ_BMS_INFO_CMD               0x0001
#define WRITE_BMS_INFO_CMD              0x8002
#define READ_ALLBMS                     0x45
typedef enum
{
    EN_READ_WRITE_BMS_INFO,    
    EN_READ_DCDC_INFO,
    EN_READ_EVENT_NUM,
} EN_BMS_READ_WRITE_STATE;

typedef struct _slave_write_stru_t
{
    uint8_t             slave_addr;
    uint16_t            reg_addr;
    uint8_t             reg_num;
    uint16_t            data[20];
} SlaveWrite_t;

typedef struct _udp_rev_data_stru_t
{
    uint16_t                    device_type;
    uint8_t                     reserve;
    uint8_t                     opcmd;
    uint32_t                    addr;
    uint16_t                    length;
    uint8_t                    *pData;
    uint16_t                    data_size;    
} UdpRevData_t;

typedef struct _fpga_udp_rev_data_stru_t
{
    uint16_t                    data_size;
    uint8_t                        *pData;
} FpgaUdpRevData_t;

typedef enum
{
    COM_TYPE_UART_DATA,
} EN_COM_REV_DATA_TYPE;

typedef struct _com_rev_data_stru_t
{
    uint16_t                    data_size;
    uint8_t                     *pData;
} ComRevData_t;


typedef struct _com_send_data_stru_t
{
    EN_COM_REV_DATA_TYPE        data_type;
    uint16_t                    data_size;
    uint8_t                     *pData;
} ComSendData_t;

extern Queue_t gBMSModebusRevQueue;
extern Queue_t gBmsSendQueue;
extern Queue_t gBmsModbusSlaveCommSendQueue;
extern Queue_t gUdpSlaveCommSendQueue;
extern UdpRevData_t gUdpPara[REV_QUEUE_DEPTH];
void SlaveCommInit(void);
bool ProtoUnpackJudge(ComRevData_t *p);
bool UdpProtopackJudge(FpgaUdpRevData_t *p);
void ReadBMSReg(void);
void ReadDCDCReg(void);
void WriteBmsData(uint32_t reg_start_addr, uint16_t data_len, uint8_t *pReg_value);
void WriteDCDCData(uint32_t reg_start_addr, uint16_t data_len, uint8_t *pReg_value);
uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode);
bool BMSWriteDataSeed(uint32_t reg_start_addr, uint16_t len, uint8_t *data);
uint8_t RevBMSUpdataAnalysis(uint8_t *p, uint16_t size);
void testprocess(void);
#endif
