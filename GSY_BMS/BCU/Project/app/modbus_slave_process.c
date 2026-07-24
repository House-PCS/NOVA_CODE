/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_process.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-18
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-18     <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "modbus_slave_process.h"
#include "crc.h"
#include "modbus_upstream_analysis.h"

#define MODBUS_SLAVE_ADDR       0x01


static uint8_t _gModbusAckData[280];
static uint8_t _gModbusAckSize;


static uint16_t _gRegData[128];


/** 
 * @brief    Modbus读输入解析及回包数据处理
 * @param    modbus_type:RTU还是TCP
 * @param    reg_start_addr:寄存器起始地址
 * @param    reg_num:寄存器数量
 * @return   none
 * @note:
 */
static void _ModbusSlaveReadInput(EN_MODBUS_SLAVE_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    EN_MODBUS_UPSTREAM_OP_STAT op_res;
    
    if(modbus_type == EN_MODBUS_SLAVE_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _gModbusAckData[offset] = MODBUS_SLAVE_ADDR;
    
    /*做数据正确性判断*/
    /*
     *  error判断，根据业务添加代码
     */
    if(reg_num >= 128)//超过MODBUS最大承载量
    {
        error = EN_MODBUS_SLAVE_ERROR_CODE_INVLID_LEN;
    }
    else
    {
        op_res = ModbusUpstreamReadJudgeAndAnalysis(reg_start_addr, reg_num, _gRegData);
        if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_CORRECT;
        }
        else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_ADDR_ERROR)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_INVALID_REG_ADDR;
        }
        else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_LEN_ERROR)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_INVLID_LEN;
        }
        else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_BUSY)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_SLAVE_BUSY;
        }
    }
    
    if(error == EN_MODBUS_SLAVE_ERROR_CODE_CORRECT)
    {
        /*功能码*/
        _gModbusAckData[offset + 1] = 0x04;
        /*字节数*/
        _gModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
        memcpy((uint8_t *)&_gModbusAckData[offset + 3], (uint8_t *)_gRegData, reg_num * 2);
        
        _gModbusAckSize = reg_num * 2 + 3 + offset + 2;
    }
    else
    {
        /*功能码*/
        _gModbusAckData[offset + 1] = 0x84;
        /*错误码*/
        _gModbusAckData[offset + 2] = error;
        _gModbusAckSize = 3 + offset + 2;
    }
    
}


/** 
 * @brief    Modbus读保持解析及回包数据处理
 * @param    modbus_type:RTU还是TCP
 * @param    reg_start_addr:寄存器起始地址
 * @param    reg_num:寄存器数量
 * @return   none
 * @note:
 */
static void _ModbusSlaveReadRemain(EN_MODBUS_SLAVE_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    EN_MODBUS_UPSTREAM_OP_STAT op_res;
    
    if(modbus_type == EN_MODBUS_SLAVE_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _gModbusAckData[offset] = MODBUS_SLAVE_ADDR;
    
    /*做数据正确性判断*/
    /*
     *  error判断，根据业务添加代码
     */
    if(reg_num >= 128)//超过MODBUS最大承载量
    {
        error = EN_MODBUS_SLAVE_ERROR_CODE_INVLID_LEN;
    }
    else
    {
        op_res = ModbusUpstreamReadJudgeAndAnalysis(reg_start_addr, reg_num, _gRegData);
        if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_CORRECT;
        }
        else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_ADDR_ERROR)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_INVALID_REG_ADDR;
        }
        else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_LEN_ERROR)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_INVLID_LEN;
        }
        else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_BUSY)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_SLAVE_BUSY;
        }
    }
    
    if(error == EN_MODBUS_SLAVE_ERROR_CODE_CORRECT)
    {
        /*功能码*/
        _gModbusAckData[offset + 1] = 0x03;
        /*字节数*/
        _gModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
        memcpy((uint8_t *)&_gModbusAckData[offset + 3], (uint8_t *)_gRegData, reg_num * 2);
        
        _gModbusAckSize = reg_num * 2 + 3 + offset + 2;
    }
    else
    {
        /*功能码*/
        _gModbusAckData[offset + 1] = 0x83;
        /*错误码*/
        _gModbusAckData[offset + 2] = error;
        _gModbusAckSize = 3 + offset + 2;
    }
    
}


/** 
 * @brief    Modbus写单个字节解析及回包数据处理
 * @param    modbus_type:RTU还是TCP
 * @param    reg_start_addr:寄存器起始地址
 * @param    reg_data:寄存器值
 * @return   none
 * @note:
 */
static void _ModbusSlaveWriteSingleRemain(EN_MODBUS_SLAVE_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_data)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    EN_MODBUS_UPSTREAM_OP_STAT op_res;
    
    if(modbus_type == EN_MODBUS_SLAVE_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    
    /*地址*/
    _gModbusAckData[offset] = MODBUS_SLAVE_ADDR;
    /*做数据正确性判断*/
    /*
     *  error判断，根据业务添加代码
     */
    /*如Error为0执行写操作的任务*/
    op_res = ModbusUpstreamWriteJudgeAndAnalysis(reg_start_addr, 1, &reg_data);
    if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS)
    {
        error = EN_MODBUS_SLAVE_ERROR_CODE_CORRECT;
    }
    else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_ADDR_ERROR)
    {
        error = EN_MODBUS_SLAVE_ERROR_CODE_INVALID_REG_ADDR;
    }
    else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_LEN_ERROR)
    {
        error = EN_MODBUS_SLAVE_ERROR_CODE_INVLID_LEN;
    }
    else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_BUSY)
    {
        error = EN_MODBUS_SLAVE_ERROR_CODE_SLAVE_BUSY;
    }
    else if(op_res == EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA)
    {
        error = EN_MODBUS_SLAVE_ERROR_CODE_INVALID_DATA;
    }
    
    if(error == 0)
    {
        /*功能码*/
        _gModbusAckData[offset + 1] = 0x06;
        /*寄存器地址*/
        _gModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;
        _gModbusAckData[offset + 3] = reg_start_addr & 0xFF;
        _gModbusAckData[offset + 4] = (reg_data >> 8) & 0xFF;
        _gModbusAckData[offset + 5] = reg_data & 0xFF;
        _gModbusAckSize = 4 + 2 + offset + 2;
    }
    else
    {
        /*功能码*/
        _gModbusAckData[1] = 0x86;
        /*错误码*/
        _gModbusAckData[2] = error;
        _gModbusAckSize = 3 + offset + 2;
    }
}


/** 
 * @brief    Modbus写多个字节解析及回包数据处理
 * @param    modbus_type:RTU还是TCP
 * @param    reg_start_addr:寄存器起始地址
 * @param    p_reg_data:寄存器数据起始指针
 * @param    reg_num:寄存器数量
 * @return   none
 * @note:
 */
static void _ModbusSlaveWriteMultiRemain(EN_MODBUS_SLAVE_TYPE modbus_type, uint16_t reg_start_addr, uint8_t *p_reg_data, uint16_t reg_num)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    EN_MODBUS_UPSTREAM_OP_STAT op_res;
    
    if(modbus_type == EN_MODBUS_SLAVE_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _gModbusAckData[offset] = MODBUS_SLAVE_ADDR;
    /*做数据正确性判断*/
    /*
     *  error判断，根据业务添加代码
     */
    /*如Error为0执行写操作的任务*/
    if(reg_num >= 128)//超过MODBUS最大承载量
    {
        error = EN_MODBUS_SLAVE_ERROR_CODE_INVLID_LEN;
    }
    else
    {
        memcpy((uint8_t *)_gRegData, p_reg_data, reg_num * 2);
        op_res = ModbusUpstreamWriteJudgeAndAnalysis(reg_start_addr, reg_num, _gRegData);
        if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_SUCCESS)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_CORRECT;
        }
        else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_ADDR_ERROR)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_INVALID_REG_ADDR;
        }
        else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_LEN_ERROR)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_INVLID_LEN;
        }
        else if(op_res == EN_MODBUS_UPSTREAM_OP_STAT_BUSY)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_SLAVE_BUSY;
        }
        else if(op_res == EN_MODBUS_UPSTREAM_OP_WRITE_ILLEGAL_DATA)
        {
            error = EN_MODBUS_SLAVE_ERROR_CODE_INVALID_DATA;
        }
    }
    
    
    if(error == 0)
    {
        /*功能码*/
        _gModbusAckData[offset + 1] = 0x10;
        /*寄存器地址*/
        _gModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;
        _gModbusAckData[offset + 3] = reg_start_addr & 0xFF;
        _gModbusAckData[offset + 4] = (reg_num >> 8) & 0xFF;
        _gModbusAckData[offset + 5] = reg_num & 0xFF;
        _gModbusAckSize = 4 + 2 + offset + 2;
    }
    else
    {
        /*功能码*/
        _gModbusAckData[offset + 1] = 0x90;
        /*错误码*/
        _gModbusAckData[offset + 2] = error;
        _gModbusAckSize = 3 + offset + 2;
    }
}



/** 
 * @brief    Modbus Slave数据接收处理机制
 * @param    none
 * @return   none
 * @note:
 */
void ModbusSlaveRxProcess()
{
    static EN_MODBUS_SLAVE_REV_STATUS sta = EN_MODBUS_SLAVE_REV_IDLE;
    static DevUartRevData_t *p_rtu_cmd;
    static void *p_cmd;
    static uint8_t func_code;
    static uint32_t wait_time_count = 0;
    static uint32_t wait_max_time = 0;
    static EN_MODBUS_SLAVE_TYPE   modbus_type = EN_MODBUS_SLAVE_RTU;
    static EN_DEV_UART_TYPE uart_index = EN_DEV_UART_TYPE_485_1;
    
    //读数据整合
    ModbusUpstreamReadInfoProcess();
    
    switch(sta)
    {
        case EN_MODBUS_SLAVE_REV_IDLE:
        {
            //当前两路485均为从机
            if(QueueGet(&gDevUartRevQueue[EN_DEV_UART_TYPE_485_1], (void *)&p_rtu_cmd) == true)
            {
                /*先进行数据校验*/
                if(p_rtu_cmd->pData[0] != MODBUS_SLAVE_ADDR)//地址不匹配
                {
                    break;
                }
                if(CrcJudge(p_rtu_cmd->pData, p_rtu_cmd->data_size) != true)//校验不匹配
                {
                    break;
                }
                
                func_code = p_rtu_cmd->pData[1];
                modbus_type = EN_MODBUS_SLAVE_RTU;
                sta = EN_MODBUS_SLAVE_REV_ANALYSIS;
                uart_index = EN_DEV_UART_TYPE_485_1;
            }
            else if(QueueGet(&gDevUartRevQueue[EN_DEV_UART_TYPE_485_2], (void *)&p_rtu_cmd) == true)
            {
                func_code = p_rtu_cmd->pData[1];
                modbus_type = EN_MODBUS_SLAVE_RTU;
                sta = EN_MODBUS_SLAVE_REV_ANALYSIS;
                uart_index = EN_DEV_UART_TYPE_485_2;
            }
            //后续增加MODBUS_TCP
            
            break;
        }
        case EN_MODBUS_SLAVE_REV_ANALYSIS:
        {
            switch(func_code)
            {
                case EN_MODBUS_SLAVE_FUNC_CODE_READ_INPUT:
                {
                    uint16_t reg_start_addr; 
                    uint16_t reg_num; 
                    if(modbus_type == EN_MODBUS_SLAVE_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusSlaveReadInput(modbus_type, reg_start_addr, reg_num);
                    break;
                }
                case EN_MODBUS_SLAVE_FUNC_CODE_READ_REMAIN:
                {
                    uint16_t reg_start_addr; 
                    uint16_t reg_num; 
                    if(modbus_type == EN_MODBUS_SLAVE_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusSlaveReadRemain(modbus_type, reg_start_addr, reg_num);
                    break;
                }
                case EN_MODBUS_SLAVE_FUNC_CODE_WRITE_SINGLE:
                {
                    uint16_t reg_start_addr; 
                    uint16_t reg_data; 
                    if(modbus_type == EN_MODBUS_SLAVE_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_data = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusSlaveWriteSingleRemain(modbus_type, reg_start_addr, reg_data);
                    break;
                }
                case EN_MODBUS_SLAVE_FUNC_CODE_WRITE_MULTI:
                {
                    uint16_t reg_start_addr; 
                    uint16_t reg_num;
                    uint8_t *p_reg_data; 
                    if(modbus_type == EN_MODBUS_SLAVE_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                        p_reg_data = (uint8_t *)&(p_rtu_cmd->pData[7]);
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusSlaveWriteMultiRemain(modbus_type, reg_start_addr, p_reg_data, reg_num);
                    break;
                }
                default:
                {
                    uint8_t offset;
                    if(modbus_type == EN_MODBUS_SLAVE_RTU)
                    {
                        offset = 0;
                    }
                    else
                    {
                        offset = 6;
                    }
                    _gModbusAckData[offset] = MODBUS_SLAVE_ADDR;
                    _gModbusAckData[offset + 1] = func_code | 0x80;
                    _gModbusAckData[offset + 2] = EN_MODBUS_SLAVE_ERROR_CODE_INVALID_CMD;
                    _gModbusAckSize = 3 + offset + 2;
                    break;
                }
            }
            sta = EN_MODBUS_SLAVE_REV_ACK;
            break;
        }
        case EN_MODBUS_SLAVE_REV_ACK:
        {
            uint16_t crc;
            if(modbus_type == EN_MODBUS_SLAVE_RTU)
            {
                /*算校验*/
                crc = CRC16(_gModbusAckData, _gModbusAckSize - 2);
                _gModbusAckData[_gModbusAckSize - 2] = crc & 0xFF;
                _gModbusAckData[_gModbusAckSize - 1] = (crc >> 8) & 0xFF;
                
                DevUartCommSend(uart_index, _gModbusAckData, _gModbusAckSize);
                sta = EN_MODBUS_SLAVE_REV_ACK_WAIT;
            }
            else
            {
                /*后期增加MODBUS_TCP*/
            }
            break;
        }
        case EN_MODBUS_SLAVE_REV_ACK_WAIT:
        {
            if(modbus_type == EN_MODBUS_SLAVE_RTU)
            {
                if(Dev485SendCompleteJudge(uart_index) == true)
                {
                    sta = EN_MODBUS_SLAVE_REV_IDLE;
                }
            }
            else
            {
                /*后期增加MODBUS_TCP,可不用等待直接跳入IDLE*/
            }
            break;
        }
        default:break;
    }
}



