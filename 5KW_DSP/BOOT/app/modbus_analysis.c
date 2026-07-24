/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
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

#include "modbus_analysis.h"


#define MOD_FUN_ERROR       (1)
#define MOD_REG_ADDR_ERROR  (2)
#define MOD_REG_DATA_ERROR  (3)
#define MOD_COMM_ERROR      (4)

const uint16_t ModelID = 0x1234;
const uint16_t Hardware_Ver[2] = {1, 0};
const uint16_t Software_Ver[3] = {8, 8, 9};
//static ModbusReadRemainAndInput_t _gModbusRead;
static uint8_t _ModbusAckData[300];
static uint16_t _ModbusAckSize;
ModbusReadReg_t ModbusReadReg;
ModbusReg_t     ModbusReg;

uint16_t gUpgraderRestartFlag = 0;

/**
* @brief: ModbusRegTest
* @param: void                        
* @note: 
*
*/
void ModbusRegInit(void)
{
    uint16_t i = 0;
    uint16_t len = 0;
    uint16_t sw_version = 0;
    uint16_t hw_version = 0;

    hw_version = (Hardware_Ver[0] << 8) | Hardware_Ver[1];
    sw_version = (Software_Ver[0] << 12) | (Software_Ver[1] << 6) | Software_Ver[2];

    /* 测试数据 */
    len = sizeof(ModbusReadReg_t);
    for(i = 0; i < len; i ++)
    {
        ((uint16_t*)&ModbusReadReg)[i] = 0;
    }

    len = sizeof(ModbusReg_t);
    for(i = 0; i < len; i ++)
    {
        ((uint16_t*)&ModbusReg)[i] = 0;
    }

    ModbusReadReg.model_id = ModelID;
    ModbusReadReg.hardware_version = hw_version;
    ModbusReadReg.software_version = sw_version;
    memset(ModbusReadReg.sn, 0x60, 10);


}
static void ModBusReadRegUpdate()
{

}

/**
* @brief: ModbusRTUSend
* @param: p                           
* @param: size                        
* @note: 
*
*/
void ModbusRTUSend(uint8_t *p, uint16_t size)
{
    uint16_t crc;

    crc = CrcModbus(p, size);
    _ModbusAckData[size] = crc & 0xFF;
    _ModbusAckData[size + 1] = (crc >> 8) & 0xFF;

//    _gModbusSendMaxTime = 1000 * (8 + 4) * Uart1_ModS_Handle.tx_size / Uart1_ModS_Handle.uart_cfg->baud_rate + 2;//单位ms,8表示数据位时长，4表示起始+停止+GAP时长, +2表示增加一些冗余
    SciaSend(_ModbusAckData, _ModbusAckSize + 2);
//    UartSend(&Uart1_ModS_Handle);
//    _gModbusSendTimeout = tim_get_systime();
}

static void _ModbusReadInput(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num, uint8_t slave_addr)
{
    uint8_t error = 0;
    uint8_t offset = 0;

    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = slave_addr;

    /*做数据正确性判断*/
    if(reg_start_addr < MODBUS_READ_REG_START || reg_start_addr >= MODBUS_REG_START)
    {
        error = MOD_REG_ADDR_ERROR;
    }
    /*
     *  error判断，根据业务添加代码
     */
    ModBusReadRegUpdate();
    if(error == 0)
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x04;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
        MemcpyU16toU8SwapByte(&_ModbusAckData[offset + 3], (uint16_t*)&ModbusReadReg + (reg_start_addr - MODBUS_READ_REG_START), reg_num * 2);

        _ModbusAckSize = reg_num * 2 + 3 + offset;
    }
    else
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x84;
        /*错误码*/
        _ModbusAckData[offset + 2] = error;
        _ModbusAckSize = 3 + offset;
    }

}

/**
* @brief: _ModbusReadRemain
* @param: modbus_type                 
* @param: reg_start_addr              
* @param: reg_num                     
* @param: slave_addr                  
* @param: error_code                  
* @return: 
* @note: 
*
*/
static void _ModbusReadRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num, uint8_t slave_addr, uint8_t error_code)
{
    uint8_t error = 0;
    uint8_t offset = 0;

    error = error_code;
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = slave_addr;


    /*做数据正确性判断*/
    if(reg_start_addr < MODBUS_REG_START || reg_start_addr > MODBUS_REG_END)
    {
        error = MOD_REG_ADDR_ERROR;
    }
    /*
     *  error判断，根据业务添加代码
     */


    if(error == 0)
    {

        /*功能码*/
        _ModbusAckData[offset + 1] = 0x03;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
        MemcpyU16toU8SwapByte(&_ModbusAckData[offset + 3], (uint16_t*)&ModbusReg + (reg_start_addr - MODBUS_REG_START), reg_num * 2);

        _ModbusAckSize = reg_num * 2 + 3 + offset;
    }
    else
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x83;
        /*错误码*/
        _ModbusAckData[offset + 2] = error;
        _ModbusAckSize = 3 + offset;
    }

}

/**
* @brief: _ModbusWriteSingleRemain
* @param: modbus_type                 
* @param: reg_start_addr              
* @param: reg_data                    
* @param: slave_addr                  
* @return: 
* @note: 
*
*/
static void _ModbusWriteSingleRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_data, uint8_t slave_addr)
{
    uint8_t error = 0;
    uint8_t offset = 0;

    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }

    /*地址*/
    _ModbusAckData[offset] = slave_addr;
    /*做数据正确性判断*/
    if(reg_start_addr < MODBUS_REG_START || reg_start_addr > MODBUS_REG_END)
    {
        error = MOD_REG_ADDR_ERROR;
    }
    /*
     *  error判断，根据业务添加代码
     */
    /*如Error为0执行写操作的任务*/
    if(error == 0)
    {
        ((uint16_t*)&ModbusReg)[reg_start_addr - MODBUS_REG_START] = reg_data;


        /*功能码*/
        _ModbusAckData[offset + 1] = 0x06;
        /*寄存器地址*/
        _ModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;
        _ModbusAckData[offset + 3] = reg_start_addr & 0xFF;
        _ModbusAckData[offset + 4] = (reg_data >> 8) & 0xFF;
        _ModbusAckData[offset + 5] = reg_data & 0xFF;
        _ModbusAckSize = 4 + 2 + offset;
    }
    else
    {
        /*功能码*/
        _ModbusAckData[1] = 0x86;
        /*错误码*/
        _ModbusAckData[2] = error;
        _ModbusAckSize = 3 + offset;
    }
}

/**
* @brief: _ModbusWriteMultiRemain
* @param: modbus_type                 
* @param: reg_start_addr              
* @param: p_reg_data                  
* @param: reg_num                     
* @param: slave_addr                  
* @param: error_code                  
* @return: 
* @note: 
*
*/
static void _ModbusWriteMultiRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint8_t *p_reg_data, uint16_t reg_num, uint8_t slave_addr, uint8_t error_code)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint8_t i = 0;
    uint16_t value = 0;

    error = error_code;
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = slave_addr;

    /*做数据正确性判断*/
    if(reg_start_addr < MODBUS_REG_START || reg_start_addr > MODBUS_REG_END)
    {
        error = MOD_REG_ADDR_ERROR;
    }
    /*
     *  error判断，根据业务添加代码
     */
    /*如Error为0执行写操作的任务*/

    if(error == 0)
    {
        /* 写寄存器操作 */
        for (i = 0; i < reg_num; i++)
        {
            value = p_reg_data[i * 2 + 1] + (p_reg_data[i * 2] << 8);
            ((uint16_t*)&ModbusReg)[reg_start_addr + i - MODBUS_REG_START] = value;
            if(reg_start_addr + i == 0x622f)
            {
                if(ModbusReg.sysupgradestatus.b.restate_state == 1)
                {
                    gUpgraderRestartFlag = 1;
                }
                if(gUpgraderRestartFlag == 1)
                {
                    ModbusReg.sysupgradestatus.b.mintor_test_restate = 0;
                }
            }
        }
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x10;
        /*寄存器地址*/
        _ModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;
        _ModbusAckData[offset + 3] = reg_start_addr & 0xFF;
        _ModbusAckData[offset + 4] = (reg_num >> 8) & 0xFF;
        _ModbusAckData[offset + 5] = reg_num & 0xFF;
        _ModbusAckSize = 4 + 2 + offset;

    }
    else
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x90;
        /*错误码*/
        _ModbusAckData[offset + 2] = error;
        _ModbusAckSize = 3 + offset;
    }
}

static EN_MODBUS_REV_STATUS sta = EN_MODBUS_REV_STATUS_IDLE;


/**
* @brief: ModbusCmdProcess
* @param:                             
* @note: 轮询执行
*
*/
void ModbusCmdProcess()
{
    static SciRevData_t *p_rtu_cmd;

    static uint8_t func_code;
    static EN_MODBUS_TYPE   modbus_type = EN_MODBUS_RTU;
    static uint8_t   slave_addr = 0;
    uint8_t error_code = 0;
    SciRevData_t com_revdata ;
    static uint32_t com_data_send_wait_time = 0;
    static uint32_t mod_data_send_wait_time = 0;

    memset((uint16_t *)&com_revdata,0,sizeof(SciRevData_t));
    MidComserviceHandle();

    switch(sta)
    {
        case EN_MODBUS_REV_STATUS_IDLE:
        {
            if(QueueGet(&gstSciRevQueue, (void *)&p_rtu_cmd) == true)
            {
                if(p_rtu_cmd->data_type == TYPE_MODBUS_DATA)
                {
                    func_code = p_rtu_cmd->pData[1];
                    modbus_type = EN_MODBUS_RTU;
                    slave_addr = p_rtu_cmd->slave_addr;
                    sta = EN_MODBUS_REV_STATUS_ANALYSIS;
                }
                else if(p_rtu_cmd->data_type == TYPE_COM_DATA)
                {
                    com_revdata.data_size = p_rtu_cmd->data_size;
                    com_revdata.pData = p_rtu_cmd->pData;
                    //TODO 升级协议解析
                    ProtoUnpackJudge(&com_revdata);
                    com_data_send_wait_time = TimGetSystime();
                    sta = EN_COM_REV_STATUS_ACK_WAIT;
                }
            }

            break;
        }
        case EN_MODBUS_REV_STATUS_ANALYSIS:
        {
            switch(func_code)
            {
                case EN_MODBUS_FUNC_CODE_READ_INPUT:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_num;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusReadInput(modbus_type, reg_start_addr, reg_num, slave_addr);
                    break;
                }
                case EN_MODBUS_FUNC_CODE_READ_REMAIN:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_num;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];

                        if((p_rtu_cmd->data_size != 8) || (reg_num > 128))
                        {
                            error_code = EN_MODBUS_ERROR_CODE_INVALID_CMD;
                        }
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusReadRemain(modbus_type, reg_start_addr, reg_num, slave_addr, error_code);
                    break;
                }
                case EN_MODBUS_FUNC_CODE_WRITE_SINGLE:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_data;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_data = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusWriteSingleRemain(modbus_type, reg_start_addr, reg_data, slave_addr);
                    break;
                }
                case EN_MODBUS_FUNC_CODE_WRITE_MULTI:
                {
                    uint16_t reg_start_addr;
                    uint16_t reg_num;
                    uint8_t *p_reg_data = NULL;
                    uint8_t byte_num;

                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                        p_reg_data = &p_rtu_cmd->pData[7];
                        byte_num = p_rtu_cmd->pData[6];
                        if((p_rtu_cmd->data_size < 11) || (byte_num != (2 * reg_num)))
                        {
                            error_code = EN_MODBUS_ERROR_CODE_INVALID_CMD;
                        }
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusWriteMultiRemain(modbus_type, reg_start_addr, p_reg_data, reg_num, slave_addr, error_code);
                    sta = EN_MODBUS_REV_STATUS_ACK;
                    break;
                }
                default:
                {
                    uint8_t offset;
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        offset = 0;
                    }
                    else
                    {
                        offset = 6;
                    }
                    _ModbusAckData[offset] = slave_addr;
                    _ModbusAckData[offset + 1] = func_code | 0x80;
                    _ModbusAckData[offset + 2] = EN_MODBUS_ERROR_CODE_INVALID_CMD;
                    _ModbusAckSize = 3 + offset;
                    break;
                }
            }
            mod_data_send_wait_time = TimGetSystime();
            sta = EN_MODBUS_REV_STATUS_ACK_SEND_WAIT;
//            sta = EN_MODBUS_REV_STATUS_ACK;
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK_SEND_WAIT:
        {
            if(TimDifftimeFromNow(mod_data_send_wait_time) > 100)
            {
                sta = EN_MODBUS_REV_STATUS_ACK;
            }
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK:
        {
            if(modbus_type == EN_MODBUS_RTU)
            {
                ModbusRTUSend(_ModbusAckData, _ModbusAckSize);
                sta = EN_MODBUS_REV_STATUS_ACK_WAIT;
            }
            else
            {
                /*后期增加MODBUS_TCP*/
            }
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK_WAIT:
        {
            if(modbus_type == EN_MODBUS_RTU)
            {
//                if(DevModbusSendComplete() == true)
//                {
                sta = EN_MODBUS_REV_STATUS_IDLE;
//                }
            }
            else
            {
                /*后期增加MODBUS_TCP,可不用等待直接跳入IDLE*/
            }
            break;
        }
        case EN_COM_REV_STATUS_ACK_WAIT:
        {
            if(TimDifftimeFromNow(com_data_send_wait_time) > 100)
            {
                sta = EN_COM_SEND;
            }
            break;
        }
        case EN_COM_SEND:
        {
            DataSendPolling(gReadResponsed);
            sta = EN_MODBUS_REV_STATUS_IDLE;
            break;
        }
        default:
            break;
    }

}

