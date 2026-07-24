/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_modbus.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-27
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-27    <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __DEV_MODBUS_H_
#define __DEV_MODBUS_H_


#include "bsp.h"
#include "queue.h"
#include "systick.h"

#define MODBUS_RTU_REV_QUEUE_DEPTH   3
#define MODBUS_RTU_RX_SIZE           2086//550//512


#define MBRTU_OK                     0x5000   /* OK */
#define MBRTU_FC_ERR                 0x5001   /* 功能码错误 */
#define MBRTU_ADR_ERR                0x5002   /* 起始地址错误 */
#define MBRTU_LEN_ERR                0x5003   /* 输出长度错误 */
#define MBRTU_OPER_ERR               0x5004   /* 数据读或写失败 */
#define MBRTU_CRC_ERR                0x5005   /* 校验错误 */
#define MBRTU_PARA_INVALID           0x5006   /* 输入参数无效 */


#define MBRTU_FC_READ_STANDREG         0x03         /* 读保持寄存器功能码     */
#define MBRTU_FC_READ_ORREG            0x04         /* 读只读寄存器功能码     */
#define MBRTU_FC_SWRITE_STANDREG       0x06         /* 写单个寄存器功能码     */
#define MBRTU_FC_WRITE_STANDREG        0x10         /* 写保持寄存器功能码     */
#define MBRTU_FC_READ_STANDREG_ERR     0x83         /* 读保持寄存器异常功能码 */
#define MBRTU_FC_READ_ORREG_ERR        0x84         /* 读只读寄存器异常功能码 */
#define MBRTU_FC_SWRITE_STANDREG_ERR   0x86     /* 写单个寄存器异常功能码 */
#define MBRTU_FC_WRITE_STANDREG_ERR    0x90         /* 写保持寄存器异常功能码 */



typedef struct _comm_host_rev_data_stru_t
{
    uint16_t                    data_size;
    uint8_t                     *p_data;
} ModbusRTURevData_t;

typedef enum
{
    EN_MODBUS_MODE_SLAVE_COLLECTOR,
    EN_MODBUS_MODE_HOST_VOLTMETER_ARC,
    EN_MODBUS_MODE_HOST_PCS,
    EN_MODBUS_MODE_HOST_COPROCOESSOR,
} EN_MODBUS_MODE;

typedef struct _dev_modbus_host_stru_t
{
    EN_MODBUS_MODE              mode;
    bool                        dir_manual;//485方向是否手动控制
    uint8_t                     rxbuf[MODBUS_RTU_RX_SIZE];
    uint8_t                     rx_queue_buf[MODBUS_RTU_REV_QUEUE_DEPTH][MODBUS_RTU_RX_SIZE];
    uint16_t                    rev_index;
    uint16_t                    rev_len;

    Queue_t                     rev_queue;
    bool                        flag_send_complete;//只在dir_manual=1时有用
    uint32_t                    send_time_out;
    uint32_t                    send_max_time;
    void                        (*send)(Uart_HandleType *);
} DevModbus_t;



typedef struct _slave_read_stru_t
{
    uint8_t             slave_addr;
    uint8_t             func_code;
    uint16_t            reg_addr;
    uint8_t             reg_num;
    uint8_t             data[30];
} SlaveRead_t;

typedef struct _slave_write_stru_t
{
    uint8_t             slave_addr;
    uint8_t             func_code;
    uint16_t            reg_addr;
    uint8_t             reg_num;
    uint8_t             data_num;
    uint16_t            data[1500];
} SlaveWrite_t;


extern DevModbus_t gstDevModbusCollector;
extern DevModbus_t gstDevModbusVoltmeterArc;
extern DevModbus_t gstDevModbusCoprocessor;


void ModbusHostRead(SlaveRead_t *p_slave_read, void (*send)(uint8_t *, uint16_t ));
void ModbusHostWrite(SlaveWrite_t *p_slave_write, void (*send)(uint8_t *, uint16_t ), uint8_t * psend_data);
uint16_t Modebus_ParseUp(uint8_t *p_recv_buf, uint16_t rec_len, SlaveRead_t *ptFrameInfo, uint8_t *Pdata);
void ModbusBMSRTUSend(uint8_t *p_data, uint16_t len);
void ModbusPCSRTUSend(uint8_t *p_data, uint16_t len);
void ModbusCollectorRTUSend(uint8_t *p_data, uint16_t len);
void ModbusVoltmeterRTUSend(uint8_t *p_data, uint16_t len);
void ModbusCoprocessorRTUSend(uint8_t *p_data, uint16_t len);

void DevModbusRTUInit(void);
void DevModbusDirCtlProcess(void);

typedef uint8_t (*ProgramSnCallBackFun)(uint8_t *p, uint16_t data_size);
void RegisterProgramSnFun(ProgramSnCallBackFun prgram_sn_call_back_fun);

typedef uint8_t    (*SlaveUpdataCallFun)(uint8_t *p, uint16_t data_size);
void RegisterSlaveUpdataFunPoint(SlaveUpdataCallFun call_back_fun);

typedef uint8_t     (*ToolsTestRs485CallFun)(DevModbus_t *p);
void RegistTestRs485FunPoint(ToolsTestRs485CallFun call_back_fun);
void SetEnterTestModeFlag(uint16_t test_flag);
#endif




