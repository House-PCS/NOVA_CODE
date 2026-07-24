/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-10
  * @brief		 depend on dev_modbus_s
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-06-10    <td>V1.0.0    <td>           <td>InitialVersion 
  * </table>
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "modbus_analysis.h"
//#include "sys_fault.h"
//#include "data_conversion.h"
#include "systick.h"
#include "proto_uniform.h"
ModbusReadReg_t Modbus_ReadReg;
ModbusReg_t Modbus_writeReg;

/* Private Variable -------------------------------------------------------------*/


//ModbusReadReg_t _ModbusReadReg;
//ModbusReg_t _ModbusReg;
uint8_t gUpgradeRestartFlag;
static ModbusReadRemainAndInput_t _ModbusRead;
//BmsSampleResType gBmsSampleRes;
static uint8_t _ModbusAckData[256];
static uint8_t _ModbusAckSize;
/* Private function  -----------------------------------------------*/
//static uint16_t _ModBusRegUpdate(uint16_t addr,int16_t reg_value);
static void _ModbusReadInput(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num);
static void _ModbusReadRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num);
static void _ModbusWriteSingleRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_data);
static void _ModbusWriteMultiRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint8_t *p_reg_data, int16_t reg_num);

static uint32_t  mod_data_send_wait_time = 0;
uint16_t gArcState();


/* Private function prototypes -----------------------------------------------*/
/**
  * @brief   MODBUS Read input and Answer (04 function code)
  * @param   modbus_type: protocol type
  * @param   reg_start_addr: Register origin address
  * @param   reg_num: Register number
  * @return  None
  * @note    None
  *
*/
void ModbusRegInit(void)
{
    //软件版本号就是0
    memset(&Modbus_ReadReg.mode_id,0,sizeof(ModbusReadReg_t));//初始化，全部赋0
    memset(&Modbus_writeReg.reserve21_22,0,sizeof(ModbusReg_t));//初始化，全部赋0
}



static void _ModbusReadInput(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint8_t temp;
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = MODBUS_DEVICE_ADDR;
    
    /*做数据正确性判断*/
    /*
     *  error判断，根据业务添加代码
     */
    
    if(error == 0)
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x04;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
		memcpy(&_ModbusAckData[offset + 3], (uint8_t*)&Modbus_ReadReg + ((reg_start_addr - MODEBUS_MODE_ID) * 2), reg_num * 2);
		for (int i = 0; i < reg_num; i++)
        {
			 temp = _ModbusAckData[offset + 3+ (2*i)];
			_ModbusAckData[offset + 3+ 2*i] = _ModbusAckData[offset + 3+ (2*i+1)];
			_ModbusAckData[offset + 3+ (2*i+1)]	= temp;
		}
        _ModbusAckSize = reg_num * 2 + 3 + offset;
        
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
  * @brief   MODBUS Read Hold register input and response (03 function code)
  * @param   modbus_type: protocol type
  * @param   reg_start_addr: Register origin address
  * @param   reg_num: Register number
  * @return  None
  * @note    None
  *
*/
static void _ModbusReadRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint8_t temp;
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = MODBUS_DEVICE_ADDR;
    
    /*做数据正确性判断*/
    /*
     *  error判断，根据业务添加代码
     */
//    _ModBusReadRegUpdate(); /* 读出寄存器值放入reg_value */
    
    if(error == 0)
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x03;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
		memcpy(&_ModbusAckData[offset + 3], (uint8_t*)&Modbus_writeReg + ((reg_start_addr - MODEBUS_RESERVE14) * 2), reg_num * 2);
		for (int i = 0; i < reg_num; i++)
        {
			temp = _ModbusAckData[offset + 3+ (2*i)];
			_ModbusAckData[offset + 3+ 2*i] = _ModbusAckData[offset + 3+ (2*i+1)];
			_ModbusAckData[offset + 3+ (2*i+1)]	= temp;
		}
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
  * @brief   MODBUS write hold register input and response (06 function code) 
  * @param   modbus_type: protocol type
  * @param   reg_start_addr: Register origin address
  * @param   reg_num: Register number
  * @return  None
  * @note    None
  *
*/
static void _ModbusWriteSingleRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_data)
{
    uint8_t error = 0;
    uint8_t offset = 0;
	uint16_t value = 0;
    
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    
    /*地址*/
    _ModbusAckData[offset] = MODBUS_DEVICE_ADDR;
    /*做数据正确性判断*/
    /*
     *  error判断，根据业务添加代码
     */
    /*如Error为0执行写操作的任务*/
    
    if(error == 0)
    {
//         value = BEBufToUint16(&p_reg_data[2 * i]);
//         value = _ModBusRegUpdate(reg_start_addr,reg_data);

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
  * @brief   MODBUS writes multiple hold register inputs and replies (10 function codes)
  * @param   modbus_type: protocol type
  * @param   reg_start_addr: Register origin address
  * @param   reg_num: Register number
  * @return  None
  * @note    None
  *
*/
static void _ModbusWriteMultiRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint8_t *p_reg_data, int16_t reg_num)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint8_t i = 0;
    uint16_t value = 0;
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }
    /*地址*/
    _ModbusAckData[offset] = MODBUS_DEVICE_ADDR;
    /*做数据正确性判断*/
    /*
     *  error判断，根据业务添加代码
     */
    
    /*如Error为0执行写操作的任务*/
    if(error == 0)
    {
        /* 写寄存器操作 */
        for (i = 0; i < reg_num; i++)
        {
 
            value = BEBufToUint16(&p_reg_data[2 * i]);    /* 寄存器值 */
//            value = _ModBusRegUpdate(reg_start_addr+i,value);
            *((uint16_t*)&Modbus_writeReg + reg_start_addr + i - MODEBUS_RESERVE14) = value;  
        }
        if(reg_start_addr == 0x010D)
        {
            if(Modbus_writeReg.sysupgradestatus.b.restate_state == 1)
            {
                gUpgradeRestartFlag = 1;              
            }
            if(gUpgradeRestartFlag == 1)
            {
                Modbus_writeReg.sysupgradestatus.b.mintor_test_restate = 0;
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
 
/*轮询执行*/
void ModbusCmdProcess()
{
    static EN_MODBUS_REV_STATUS sta = EN_MODBUS_REV_STATUS_IDLE;
    static ModbusRTURevData_t *p_rtu_cmd;
//    static ModbusTCPRevData_t *p_tcp_cmd;
    static void *p_cmd;
    static uint8_t func_code;
    static uint32_t wait_time_count = 0;
    static uint32_t wait_max_time = 0;
    static EN_MODBUS_TYPE   modbus_type = EN_MODBUS_RTU;
    
    switch(sta)
    {
        case EN_MODBUS_REV_STATUS_IDLE:
        {
            if(QueueGet(&gModbusMTURevQueue, (void *)&p_rtu_cmd) == true)
            {
				if(p_rtu_cmd->data_type == UART_DATA_TYPE_UPGRADE)
				{
					com_revupgradedata.is_update_data = true;
					com_revupgradedata.com_revdata.data_size = p_rtu_cmd->data_size;
					com_revupgradedata.com_revdata.pData = p_rtu_cmd->pData;
					com_revupgradedata.overtime = tim_get_systime();
					sta = EN_MODBUS_REV_STATUS_IDLE;								
				}
				else
				{					
					func_code = p_rtu_cmd->pData[1];
					modbus_type = EN_MODBUS_RTU;
					sta = EN_MODBUS_REV_STATUS_ANALYSIS;
				}
			}
//            else if(QueueGet(&gModbusTCPRevQueue, (void *)&p_tcp_cmd) == true)
//            {
//                
//            }
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
                    _ModbusReadInput(modbus_type, reg_start_addr, reg_num);
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
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusReadRemain(modbus_type, reg_start_addr, reg_num);
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
                    _ModbusWriteSingleRemain(modbus_type, reg_start_addr, reg_data);
                    break;
                }
                case EN_MODBUS_FUNC_CODE_WRITE_MULTI:
                {
                    uint16_t reg_start_addr; 
                    uint16_t reg_num;
                    uint8_t *p_reg_data = NULL; 
                    if(modbus_type == EN_MODBUS_RTU)
                    {
                        reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                        reg_num = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
                        p_reg_data = &p_rtu_cmd->pData[7];
                    }
                    else
                    {
                        /*后期增加MODBUS_TCP*/
                    }
                    _ModbusWriteMultiRemain(modbus_type, reg_start_addr, p_reg_data, reg_num);
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
                    _ModbusAckData[offset] = MODBUS_DEVICE_ADDR;
                    _ModbusAckData[offset + 1] = func_code | 0x80;
                    _ModbusAckData[offset + 2] = EN_MODBUS_ERROR_CODE_INVALID_CMD;
                    _ModbusAckSize = 3 + offset;
                    break;
                }
            }
            mod_data_send_wait_time = tim_get_systime();
            sta = EN_MODBUS_REV_STATUS_ACK_SEND_WAIT;
//            sta = EN_MODBUS_REV_STATUS_ACK;
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK_SEND_WAIT:
        {
            if(tim_difftime_from_now(mod_data_send_wait_time)>150)
            {
                sta = EN_MODBUS_REV_STATUS_ACK;
            }
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK:
        {
			static uint32_t delay_time = 0;
			static uint8_t  cont = 0;
//			if(tim_difftime_from_now(delay_time) > 10)                   
//			{
//				delay_time = tim_get_systime();
//				cont ++;
//				if(cont >= 6)      
//				{
//					cont = 0;
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
                if(DevModbusSendComplete() == true)
                {
                    sta = EN_MODBUS_REV_STATUS_IDLE;
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






