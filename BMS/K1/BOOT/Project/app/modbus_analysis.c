
#include "modbus_analysis.h"
#include "systick.h"
#include "sys_info.h"
#include "rtc.h"

#define CAPACITY_STD                            (100000)

#define LOG_TAG     "ModbusAnalysis"

static ModbusReadRemainAndInput_t _gModbusRead  ;

static  uint8_t      _ModbusTxBuff[300]   ;
static  uint8_t      _ModbusSendTxBuff[300]   ;
static  uint16_t     _ModbusTxSize        ;
static  uint8_t      _ModbusAckData[300]        ;
static  uint16_t     _ModbusAckSize             ;

EN_MODBUS_SEND_STATUS EnDownSendCmd;

ModbusOnlyReadReg_t      garrFirstPackModbusOnlyReadReg[BMS_PACK_MAX_NUM]  = {0};           //一号Pack向监控卡汇报时使用的只读寄存器
ModbusOnlyReadReg_t      garrModbusOnlyReadReg[BMS_PACK_MAX_NUM]  = {0};
ModbusWriteReadReg_t     garrModbusWriteReadReg[BMS_PACK_MAX_NUM] = {0};
ModbusToolTestWriteReadModeReg_t  gstModbusToolTestWriteReadReg;                            //测试模式写读寄存器
ModbusToolTestOnlyReadModeReg_t   gstModbusToolTestOnlyReadReg;                             //测试模式只读寄存器

/*modbus数据包转发*/
void _ModbusPackTransmit(EN_MODBUS_HOST_SLAVE_TYPE en_modbus_host_or_slave, ModbusRTURevData_t *p_pack);
/*bms地址对应的数据结构体选择*/
uint8_t _ModbusRegStructSelect(uint8_t n_bms_addr);



/**
 * @brief  : 读输入寄存器（功能码：0x04）
 * @para   : modbus_type    ：modbus类型
             reg_start_addr ：寄存器起始地址
             reg_num        ：读取寄存器数量
             slave_addr     ：从机地址
 * @note   : None
 * @return : None
 */
static void _ModbusReadInput(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num, uint8_t slave_addr)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint8_t slave_reg_serial = 0;

    uint32_t target_base_reg_addr = 0;      //目标寄存器起始地址
    uint32_t target_end_reg_addr = 0;       //目标寄存器终止地址
    uint8_t* p_target_reg_start_addr = 0;     //目标寄存器首地址
    
    if(modbus_type == EN_MODBUS_RTU)
    {
        offset = 0;
    }
    else
    {
        offset = 6;
    }

    if(slave_addr >= MODBUS_ADDR_PACK0 && slave_addr <= MODBUS_ADDR_PACK_MAX_ADDR)
    {
        slave_reg_serial = slave_addr - MODBUS_ADDR_PACK0;
    }
    
    //目标寄存器地址设置
    if((reg_start_addr >= MODBUS_ONLY_READ_REG_BASE_ADDR) && (reg_start_addr <= MODBUS_ONLY_READ_REG_END_ADDR))
    {
        //输入寄存器
        target_base_reg_addr = MODBUS_ONLY_READ_REG_BASE_ADDR;
        target_end_reg_addr  = MODBUS_ONLY_READ_REG_END_ADDR;
        p_target_reg_start_addr = (uint8_t*)&garrFirstPackModbusOnlyReadReg[slave_reg_serial];
    }
    else if((reg_start_addr >= MODBUS_TEST_ONLY_READ_REG_BASE_ADDR) && (reg_start_addr <= MODBUS_TEST_ONLY_READ_REG_END_ADDR))
    {
        //测试模式输入寄存器
        target_base_reg_addr = MODBUS_TEST_ONLY_READ_REG_BASE_ADDR;
        target_end_reg_addr  = MODBUS_TEST_ONLY_READ_REG_END_ADDR;
        p_target_reg_start_addr = (uint8_t*)&gstModbusToolTestOnlyReadReg;
    }

    /*做数据正确性判断*/
    /*
     *  error判断，根据业务添加代码
     */
    //寄存器数据更新
    UpdateModbusReg();
    
    /*寄存器地址判断*/
    if((reg_start_addr > target_end_reg_addr) || \
       ((reg_start_addr + reg_num - 1) > target_end_reg_addr)
      )
    {
        error = EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR;              /* 寄存器地址错误 */
    }
    
    /*地址*/
    _ModbusAckData[offset] = slave_addr;

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

        memcpy(&_ModbusAckData[offset + 3], p_target_reg_start_addr + ((reg_start_addr - target_base_reg_addr) * 2), reg_num * 2);
        for(int i = 0; i < reg_num; i ++)
        {
            TabLsbToMsb(&_ModbusAckData[offset + 3] + i * 2, 2);
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
 * @brief  : 读保持寄存器（功能码：0x03）
 * @para   : modbus_type    ：modbus类型
             reg_start_addr ：寄存器起始地址
             reg_num        ：读取寄存器数量
             slave_addr     ：从机地址
             error_code     ：错误码
 * @note   : None
 * @return : None
 */
static void _ModbusReadRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num, uint8_t slave_addr, uint8_t error_code)
{
    uint8_t error = 0;
    uint8_t offset = 0;
    uint8_t slave_reg_serial = 0;
    
    uint32_t target_base_reg_addr;              //目标寄存器起始地址
    uint32_t target_end_reg_addr;               //目标寄存器终止地址
    uint8_t* p_target_reg_start_addr = 0;       //目标寄存器首地址

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
    /* 计算第几个寄存器 */
    if(slave_addr >= MODBUS_ADDR_PACK0 && slave_addr <= MODBUS_ADDR_PACK_MAX_ADDR)
    {
        slave_reg_serial = slave_addr - MODBUS_ADDR_PACK0;
    }
    else
    {
        error = EN_MODBUS_ERROR_CODE_INVALID_SLAVE_ADDR;              /* 从机地址错误 */
    }
    //目标寄存器地址设置
    if((reg_start_addr >= MODBUS_WRITE_READ_REG_BASE_ADDR) && (reg_start_addr <= MODBUS_WRITE_READ_REG_END_ADDR))
    {
        //保持寄存器
        target_base_reg_addr = MODBUS_WRITE_READ_REG_BASE_ADDR;
        target_end_reg_addr  = MODBUS_WRITE_READ_REG_END_ADDR;
        p_target_reg_start_addr = (uint8_t*)&garrModbusWriteReadReg[slave_reg_serial];
        
    }
    else if((reg_start_addr >= MODBUS_TEST_WRITE_READ_REG_BASE_ADDR) && (reg_start_addr <= MODBUS_TEST_WRITE_READ_REG_END_ADDR))
    {
        //测试模式保持寄存器
        target_base_reg_addr = MODBUS_TEST_WRITE_READ_REG_BASE_ADDR;
        target_end_reg_addr  = MODBUS_TEST_WRITE_READ_REG_END_ADDR;
        p_target_reg_start_addr = (uint8_t*)&gstModbusToolTestWriteReadReg;
    }
        
    //寄存器数据更新
    UpdateModbusReg();
    
    /*寄存器地址判断*/
    if((reg_start_addr > target_end_reg_addr) || \
       ((reg_start_addr + reg_num - 1) > target_end_reg_addr)
      ) /* 读出寄存器值放入reg_value */
    {
        error = EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR;              /* 寄存器地址错误 */
    }

    if(error == 0)
    {

        /*功能码*/
        _ModbusAckData[offset + 1] = 0x03;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
        //保持寄存器

        memcpy(&_ModbusAckData[offset + 3], p_target_reg_start_addr + ((reg_start_addr - target_base_reg_addr) * 2), reg_num * 2);
        
        for(int i = 0; i < reg_num; i ++)
        {
            TabLsbToMsb(&_ModbusAckData[offset + 3] + i * 2, 2);
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
 * @brief  : 写单个寄存器（功能码：0x06）
* @para   : modbus_type     ：modbus类型
             reg_start_addr ：寄存器起始地址
             reg_num        ：读取寄存器数量
             slave_addr     ：从机地址
 * @note   : None
 * @return : None
 */
static void _ModbusWriteSingleRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_data, uint8_t slave_addr)
{
    uint8_t error = 0;
    uint8_t offset = 0;

    uint32_t target_base_reg_addr    = 0;       //目标寄存器起始地址
    uint32_t target_end_reg_addr     = 0;       //目标寄存器终止地址
    uint8_t* p_target_reg_start_addr = 0;       //目标寄存器首地址
    
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
    
    //目标寄存器地址设置
    if((reg_start_addr >= MODBUS_WRITE_READ_REG_BASE_ADDR) && (reg_start_addr <= MODBUS_WRITE_READ_REG_END_ADDR))
    {
        //保持寄存器
        target_base_reg_addr = MODBUS_WRITE_READ_REG_BASE_ADDR;
        target_end_reg_addr  = MODBUS_WRITE_READ_REG_END_ADDR;
        p_target_reg_start_addr = (uint8_t*)&garrModbusWriteReadReg[0];
    }
    else if((reg_start_addr >= MODBUS_TEST_WRITE_READ_REG_BASE_ADDR) && (reg_start_addr <= MODBUS_TEST_WRITE_READ_REG_END_ADDR))
    {
        //测试模式保持寄存器
        target_base_reg_addr = MODBUS_TEST_WRITE_READ_REG_BASE_ADDR;
        target_end_reg_addr  = MODBUS_TEST_WRITE_READ_REG_END_ADDR;
        p_target_reg_start_addr = (uint8_t*)&gstModbusToolTestWriteReadReg;
    }
    
    //寄存器数据更新
    UpdateModbusReg();
    
    /*寄存器地址判断*/
    if(reg_start_addr > target_end_reg_addr)
    {
        error = EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR;              /* 寄存器地址错误 */
    }

    if(error == 0)
    {
        /* 读出寄存器值放入reg_value */
        //***********数据写入************
        ((uint16_t*)p_target_reg_start_addr)[reg_start_addr - target_base_reg_addr] = reg_data;
        //根据写读寄存器进行相应操作
//        ModbusWriteReadRegProcess(reg_start_addr);
        
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x06;
        /*寄存器地址*/
        _ModbusAckData[offset + 2] = (reg_start_addr >> 8) & 0xFF;
        _ModbusAckData[offset + 3] = reg_start_addr & 0xFF;
        _ModbusAckData[offset + 4] = (((uint16_t*)p_target_reg_start_addr)[reg_start_addr - target_base_reg_addr] >> 8) & 0xFF;
        _ModbusAckData[offset + 5] = ((uint16_t*)p_target_reg_start_addr)[reg_start_addr - target_base_reg_addr] & 0xFF;
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
* @brief : 写多个寄存器（功能码：0x10）
* @para   : modbus_type     ：modbus类型
             reg_start_addr ：寄存器起始地址
             p_reg_data     ：数据指针
             reg_num        ：读取寄存器数量
             slave_addr     ：从机地址
 * @note   : None
 * @return : None
 */
static void _ModbusWriteMultiRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint8_t *p_reg_data, uint16_t reg_num, uint8_t slave_addr, uint8_t error_code)
{
    uint8_t  error              = 0;
    uint8_t  offset             = 0;
    uint16_t current_reg_value  = 0;
    uint8_t  slave_reg_serial   = 0;

    uint32_t target_base_reg_addr    = 0;       //目标寄存器起始地址
    uint32_t target_end_reg_addr     = 0;       //目标寄存器终止地址
    uint8_t* p_target_reg_start_addr = 0;       //目标寄存器首地址
    
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
    /* 判断是第几个电池包 */
    if(slave_addr >= MODBUS_ADDR_PACK0 && slave_addr <= MODBUS_ADDR_PACK_MAX_ADDR)
    {
        slave_reg_serial = slave_addr - MODBUS_ADDR_PACK0;
    }
    else
    {
         error = EN_MODBUS_ERROR_CODE_INVALID_SLAVE_ADDR;              /* 从机地址地址错误 */
    }
    
    //目标寄存器地址设置
    if((reg_start_addr >= MODBUS_WRITE_READ_REG_BASE_ADDR) && (reg_start_addr <= MODBUS_WRITE_READ_REG_END_ADDR))
    {   
        //保持寄存器
        target_base_reg_addr = MODBUS_WRITE_READ_REG_BASE_ADDR;
        target_end_reg_addr  = MODBUS_WRITE_READ_REG_END_ADDR;
        p_target_reg_start_addr = (uint8_t*)&garrModbusWriteReadReg[0];
    }
    else if((reg_start_addr >= MODBUS_TEST_WRITE_READ_REG_BASE_ADDR) && (reg_start_addr <= MODBUS_TEST_WRITE_READ_REG_END_ADDR))
    {
        //测试模式保持寄存器
        target_base_reg_addr = MODBUS_TEST_WRITE_READ_REG_BASE_ADDR;
        target_end_reg_addr  = MODBUS_TEST_WRITE_READ_REG_END_ADDR;
        p_target_reg_start_addr = (uint8_t*)&gstModbusToolTestWriteReadReg;
    }
    
    //寄存器数据更新
    UpdateModbusReg();
    
    /*寄存器地址判断*/
    if((reg_start_addr >= target_end_reg_addr) || ((reg_start_addr + reg_num - 1) > target_end_reg_addr))
    {
        error = EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR;              /* 寄存器地址错误 */
    }


    
    if(error == 0)
    {
        /* 写寄存器操作 */
        for(int i = 0; i < reg_num; i++)
        {
            /* 大端转小端 */
            current_reg_value = BEBufToUint16(&p_reg_data[2 * i]);    /* 寄存器值 */

            ((uint16_t*)&garrModbusWriteReadReg[slave_reg_serial])[reg_start_addr + i - MODBUS_WRITE_READ_REG_BASE_ADDR] = current_reg_value;

            //***********数据写入************
            ((uint16_t*)p_target_reg_start_addr)[reg_start_addr + i - target_base_reg_addr] = current_reg_value;

        }
//        //根据写读寄存器进行相应操作
//        for(int i = 0; i < reg_num; i++)
//        {
//            ModbusWriteReadRegProcess(reg_start_addr + i);
//        }


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
/**
 * @brief  : modbus命令处理
 * @para   : None
 * @note   : None
 * @return : None
 */
void ModbusCmdProcess()
{
    /*Modbus运行状态标志*/
    static EN_MODBUS_REV_STATUS         modbus_run_sta = EN_MODBUS_REV_STATUS_IDLE; //运行状态           
    static ModbusRTURevData_t          *p_rtu_cmd;
    static ModbusRTURevData_t          *p_host_rev_queue;                   //modbus主机接收队列指针
    static uint8_t                      func_code;
    static uint32_t                     ack_time = 0;
    static uint32_t                     uart_wait_time = 0;                 //串口等待时间
    static EN_MODBUS_TYPE               modbus_type = EN_MODBUS_RTU;
    static uint8_t                      slave_addr = 0;
    static EN_MODBUS_HOST_SLAVE_TYPE    en_modbus_host_or_slave = EN_MODBUS_HOST_SLAVE_TYPE_HOST;
    static uint8_t                      resend_count   = 0;             //重发次数          
    uint8_t                             error_code = 0;
    

    DevModbusDirCtlProcess(EN_MODBUS_HOST_SLAVE_TYPE_HOST);
    DevModbusDirCtlProcess(EN_MODBUS_HOST_SLAVE_TYPE_SLAVE);

    switch(modbus_run_sta)
    {
        case EN_MODBUS_REV_STATUS_IDLE:
        {
            if(QueueGet(&gModbusRTURevQueue, (void *)&p_rtu_cmd) == true)
            {
                //通讯成功标志设置
                gUartPackInfo.com_success_flag = 1;
                
                func_code = p_rtu_cmd->pData[1];
                modbus_type = EN_MODBUS_RTU;

                slave_addr = p_rtu_cmd->slave_addr;
                en_modbus_host_or_slave = p_rtu_cmd->en_modbus_host_or_slave;
                
                modbus_run_sta = EN_MODBUS_REV_STATUS_ANALYSIS;

            }
            
            //数据更新
            UpdateModbusReg();

            break;
        }

        /*****************************数据包解析*************************************/
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
                    //反馈应答
                    modbus_run_sta = EN_MODBUS_REV_STATUS_ACK;
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
                    //反馈应答
                    modbus_run_sta = EN_MODBUS_REV_STATUS_ACK;
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

                    //数据包地址是自己的，进行写操作
                    if(slave_addr == gUartPackInfo.ModbusAddr)
                    {
                        _ModbusWriteSingleRemain(modbus_type, reg_start_addr, reg_data, slave_addr);
                        //反馈应答
                        modbus_run_sta = EN_MODBUS_REV_STATUS_ACK;
                    }
                    else
                    {
                        //不是自己的
//                        if(gUartPackInfo.is_final_bms != true)
//                        {
//                            //记录要下发的数据
//                            memcpy(_ModbusTxBuff, p_rtu_cmd->pData, p_rtu_cmd->data_size);
//                            _ModbusTxSize = p_rtu_cmd->data_size;
//                            //不是最后一个Pack，转发
//                            modbus_run_sta = EN_MODBUS_REV_STATUS_TRANSMIT_WAIT_UART;
//                        }
//                        else
//                        {
                            //是最后一个Pack，ACK错误
                            _ModbusAckData[0] = gUartPackInfo.ModbusAddr;
                            /*功能码*/
                            _ModbusAckData[1] = 0x80 + EN_MODBUS_FUNC_CODE_WRITE_SINGLE;
                            /*错误码*/
                            _ModbusAckData[2] = EN_MODBUS_ERROR_CODE_INVALID_SLAVE_ADDR;
                            _ModbusAckSize = 3;
                            modbus_run_sta = EN_MODBUS_REV_STATUS_ACK;
//                        }
                        
                    }
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

                    /*数据包地址是自己的，进行写操作*/
                    if(slave_addr == gUartPackInfo.ModbusAddr)
                    {
                        _ModbusWriteMultiRemain(modbus_type, reg_start_addr, p_reg_data, reg_num, slave_addr, error_code);
                        //反馈应答
                        modbus_run_sta = EN_MODBUS_REV_STATUS_ACK;
                    }
                    else
                    {
                        //不是自己的
                        if(gUartPackInfo.is_final_bms != true)
                        {
                            //记录要下发的数据
                            memcpy(_ModbusTxBuff, p_rtu_cmd->pData, p_rtu_cmd->data_size);
                            _ModbusTxSize = p_rtu_cmd->data_size;
                            //不是最后一个Pack，转发
                            modbus_run_sta = EN_MODBUS_REV_STATUS_TRANSMIT_WAIT_UART;
                        }
                        else
                        {
                            //是最后一个Pack，ACK错误
                            _ModbusAckData[0] = gUartPackInfo.ModbusAddr;
                            /*功能码*/
                            _ModbusAckData[1] = 0x80 + EN_MODBUS_FUNC_CODE_WRITE_MULTI;
                            /*错误码*/
                            _ModbusAckData[2] = EN_MODBUS_ERROR_CODE_INVALID_SLAVE_ADDR;
                            _ModbusAckSize = 3;
                            modbus_run_sta = EN_MODBUS_REV_STATUS_ACK;
                        }
                    }
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
                    modbus_run_sta = EN_MODBUS_REV_STATUS_ACK;
                    break;
                }
            }
            ack_time = tim_get_systime();
            break;
        }
        
        case EN_MODBUS_REV_STATUS_ACK:
        {
            if(tim_difftime_from_now(ack_time) < 100)
            {
                break;
            }
            
            if(modbus_type == EN_MODBUS_RTU)
            {
                ModbusRTUSend(_ModbusAckData, _ModbusAckSize, EN_MODBUS_HOST_SLAVE_TYPE_SLAVE, 1);
                modbus_run_sta = EN_MODBUS_REV_STATUS_WAIT_ACK_COMPLETE;
            }
            else
            {
                /*后期增加MODBUS_TCP*/
            }
            break;
        }
        /*****************************数据包转发及应答*************************************/
        /* 转发等待串口空闲 */
        case EN_MODBUS_REV_STATUS_TRANSMIT_WAIT_UART:
        {
            //没有进行其它指令(modbus与私有协议全部空闲时才向下进行)，执行当前指令
            if(gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] == false)
            {
                return;
            }
            
            modbus_run_sta = EN_MODBUS_REV_STATUS_PACK_DOWN_TRANSMIT;
            gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = false;
            ack_time = tim_get_systime();
            break;
        }
        /*数据包转发*/
        case EN_MODBUS_REV_STATUS_PACK_DOWN_TRANSMIT:
        {
            if(tim_difftime_from_now(ack_time) < 10)
            {
                return;
            }
            //数据包地址不是自己的，转发（从机接收，主机转发）
            ModbusRTUSend(p_rtu_cmd->pData, p_rtu_cmd->data_size, EN_MODBUS_HOST_SLAVE_TYPE_HOST, 0);
            modbus_run_sta = EN_MODBUS_REV_STATUS_ACK_UP_TRANSMIT;
            ack_time = tim_get_systime();
        }
        /*数据包ACK应答转发*/
        case EN_MODBUS_REV_STATUS_ACK_UP_TRANSMIT:
        {
            if(tim_difftime_from_now(ack_time) > SetMaxWaitTime(gUartPackInfo.ModbusAddr))
            {
                //应答超时，反馈应答超时
                _ModbusAckData[0] = gUartPackInfo.ModbusAddr + 1;
                /*功能码*/
                _ModbusAckData[1] = 0x80 + EN_MODBUS_FUNC_CODE_WRITE_MULTI;
                /*错误码*/
                _ModbusAckData[2] = EN_MODBUS_ERROR_CODE_INVALID_SLAVE_ADDR;
                _ModbusAckSize = 3;
                gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
                modbus_run_sta = EN_MODBUS_REV_STATUS_ACK;
                break;
            }
            
            if(QueueGet(&gModbusHostQueue, (void *)&p_rtu_cmd) == false)
            {
                return;
            }
            //主机接收，从机转发
            ModbusRTUSend(p_rtu_cmd->pData, p_rtu_cmd->data_size, EN_MODBUS_HOST_SLAVE_TYPE_SLAVE, 0);
            gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
            modbus_run_sta = EN_MODBUS_REV_STATUS_WAIT_ACK_COMPLETE;
        }
        /*****************************等待完成*************************************/
        case EN_MODBUS_REV_STATUS_WAIT_ACK_COMPLETE:
        {
            uint16_t reg_start_addr = 0;
            if(modbus_type == EN_MODBUS_RTU)
            {
                if(DevModbusSendComplete(en_modbus_host_or_slave) == true)
                {
                    //接收寄存器地址
                    reg_start_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
                    
                    //应答完成后若不是最后一个数据包，且是特定寄存器，则同步下发
                    if((gUartPackInfo.is_final_bms != true) && (reg_start_addr == MODBUS_WRITE_READ_REG_DSP_WAKEUP_STATUS))
                    {
                        uart_wait_time = tim_get_systime();
                        modbus_run_sta = EN_MODBUS_REV_STATUS_PACK_DOWN_SEND_WAIT_UART;

                    }
                    else
                    {
                        modbus_run_sta = EN_MODBUS_REV_STATUS_IDLE;
                    }
                }
            }
            else
            {
                /*后期增加MODBUS_TCP,可不用等待直接跳入IDLE*/
            }
            break;
        }
        
        /*****************************数据包下发及应答*************************************/
        /*数据包下发等待串口空闲*/
        case EN_MODBUS_REV_STATUS_PACK_DOWN_SEND_WAIT_UART:
        {
             //没有进行其它指令(modbus与私有协议全部空闲时才向下进行)，执行当前指令
            if(gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] == false)
            {
                if(tim_difftime_from_now(uart_wait_time) > 1500)
                {
                    gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
                    modbus_run_sta = EN_MODBUS_REV_STATUS_IDLE;
                }
                return;
            }
            
            resend_count   = 0;                                                 //重发清零
            gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = false;
            ack_time = tim_get_systime();
            modbus_run_sta = EN_MODBUS_REV_STATUS_PACK_DOWN_SEND;
            break;
        }
        /*数据包下发*/
        case EN_MODBUS_REV_STATUS_PACK_DOWN_SEND:
        {
            //地址+1
            memcpy(_ModbusTxBuff, p_rtu_cmd->pData, p_rtu_cmd->data_size);
            _ModbusTxBuff[0] += 1; 
            //和校验重新计算下发
            ModbusRTUSend(_ModbusTxBuff, (p_rtu_cmd->data_size - 2), EN_MODBUS_HOST_SLAVE_TYPE_HOST, 1);
            ack_time = tim_get_systime();
            
            modbus_run_sta = EN_MODBUS_REV_STATUS_PACK_DOWN_SEND_WAIT_ACK;
            break;
        }
        
        /*数据包下发等待应答*/
        case EN_MODBUS_REV_STATUS_PACK_DOWN_SEND_WAIT_ACK:
        {
            if(tim_difftime_from_now(ack_time) > SetMaxWaitTime(gUartPackInfo.ModbusAddr))
            {
                if(resend_count < 3)
                {
                    modbus_run_sta = EN_MODBUS_REV_STATUS_PACK_DOWN_SEND;
                }
                else
                {
                    gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
                    modbus_run_sta = EN_MODBUS_REV_STATUS_IDLE;
                }
                resend_count++;
                break;
            }
            
            if(QueueGet(&gModbusHostQueue, (void *)&p_host_rev_queue) == false)
            {
                return;
            }
            
            uint16_t reg_addr = 0;
            uint16_t reg_data = 0;
            reg_addr = (p_rtu_cmd->pData[2] << 8) | p_rtu_cmd->pData[3];
            reg_data = (p_rtu_cmd->pData[4] << 8) | p_rtu_cmd->pData[5];
            
            //接收与查询的地址和内容不一致，判定写失败
            if(memcmp((uint8_t*)(&p_rtu_cmd->pData[2]), (uint8_t*)(&p_host_rev_queue->pData[2]), 4) != true)
            {
                //写失败重发三次
                if(resend_count < 3)
                {
                    modbus_run_sta = EN_MODBUS_REV_STATUS_PACK_DOWN_SEND;
                }
                else
                {
                    gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
                    modbus_run_sta = EN_MODBUS_REV_STATUS_IDLE;
                }
                resend_count++;
                break;
            }
            
            gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
            modbus_run_sta = EN_MODBUS_REV_STATUS_IDLE;
            break;
        }
        
        default:
            break;
    }

}

/**
 * @brief  : 重启命令设置
 * @para   : None
 * @note   : None
 * @return : None
 */
void ResetCmdSet()
{
    EnDownSendCmd = EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND_WAIT_UART;
}

/**
 * @brief  : 重启命令下发
 * @para   : None
 * @note   : None
 * @return : None
 */
void _ResetCmdSend(uint8_t addr)
{
    _ModbusSendTxBuff[0] = addr;
    _ModbusSendTxBuff[1] = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;
    _ModbusSendTxBuff[2] = (uint8_t)(MODBUS_WRITE_READ_REG_UPGRADE_STATUS >> 8);
    _ModbusSendTxBuff[3] = (uint8_t)MODBUS_WRITE_READ_REG_UPGRADE_STATUS;
    _ModbusSendTxBuff[4] = 0X00;
    _ModbusSendTxBuff[5] = 0X02;
    //和校验计算下发
    ModbusRTUSend(_ModbusSendTxBuff, 6, EN_MODBUS_HOST_SLAVE_TYPE_HOST, 1);
}

/**
 * @brief  : 重启检测命令下发
 * @para   : None
 * @note   : None
 * @return : None
 */
void _ResetCheckCmdSend(uint8_t addr)
{
    _ModbusSendTxBuff[0] = addr;
    _ModbusSendTxBuff[1] = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;
    _ModbusSendTxBuff[2] = (uint8_t)(MODBUS_WRITE_READ_REG_UPGRADE_STATUS >> 8);
    _ModbusSendTxBuff[3] = (uint8_t)MODBUS_WRITE_READ_REG_UPGRADE_STATUS;
    _ModbusSendTxBuff[4] = 0X00;
    _ModbusSendTxBuff[5] = 0X04;
    //和校验计算下发
    ModbusRTUSend(_ModbusSendTxBuff, 6, EN_MODBUS_HOST_SLAVE_TYPE_HOST, 1);
}

/**
 * @brief  : DMOS检测命令下发
 * @para   : None
 * @note   : None
 * @return : None
 */
void DMosCheckCmdSend(uint8_t addr)
{
    _ModbusSendTxBuff[0] = addr;
    _ModbusSendTxBuff[1] = EN_MODBUS_FUNC_CODE_READ_REMAIN;
    _ModbusSendTxBuff[2] = (uint8_t)(MODBUS_WRITE_READ_REG_DSG_MOS_CTRL >> 8);
    _ModbusSendTxBuff[3] = (uint8_t)MODBUS_WRITE_READ_REG_DSG_MOS_CTRL;
    _ModbusSendTxBuff[4] = 0X00;
    _ModbusSendTxBuff[5] = 0X04;
    //和校验计算下发
    ModbusRTUSend(_ModbusSendTxBuff, 6, EN_MODBUS_HOST_SLAVE_TYPE_HOST, 1);
}


/**
 * @brief  : modbus命令下发
 * @para   : None
 * @note   : None
 * @return : None
 */
void ModbusCmdDownSendProcess()
{
    static ModbusRTURevData_t  *p_rev_queue;        //接收队列指针
    static uint32_t             cmd_send_interval;  //指令发送间隔时间
    static uint32_t             ack_time;           //应答时间
    static uint32_t             reset_check_time;   //重启检测时间
    static uint8_t              send_count = 0;     //重发次数
    static uint8_t              target_addr;        //目标地址
    static uint32_t             reset_max_wait_time;//重启等待时间
    
    
    switch(EnDownSendCmd)
    {
        case EN_MODBUS_SEND_STATUS_IDLE:
        {
            break;
        }
        /**************** 重启指令下发 ****************/
        /* 等待串口 */
        case EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND_WAIT_UART:
        {
            //没有进行其它指令(modbus与私有协议全部空闲时才向下进行)，执行当前指令
            if(gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] == false)
            {
                return;
            }
            target_addr = gUartPackInfo.ModbusAddr + 1;
            gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = false;
            send_count = 0;
            cmd_send_interval = tim_get_systime();
            
            EnDownSendCmd = EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND;
            break;
        }
        /* 重启指令发送 */
        case EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND:
        {
            if(tim_difftime_from_now(cmd_send_interval) < 50)
            {
                break;
            }
            
            //下发重启指令
            _ResetCmdSend(target_addr);
            
            ack_time = tim_get_systime();
            EnDownSendCmd = EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND_WAIT_ACK;
            break;
        }
        /* 等待应答 */
        case EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND_WAIT_ACK:
        {
            if(tim_difftime_from_now(ack_time) > SetMaxWaitTime(gUartPackInfo.ModbusAddr))
            {
                send_count++;
                if(send_count < 3)
                {
                    EnDownSendCmd = EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND;
                }
                else
                {
                    gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
                    EnDownSendCmd = EN_MODBUS_SEND_STATUS_IDLE;
                }
                
                break;
            }
            
            //接收到回包
            if(QueueGet(&gModbusHostQueue, (void *)&p_rev_queue) == false)
            {
                return;
            }
            
            //回包正确，进入空闲，回包错误继续重发。
            if(p_rev_queue->pData[0] == target_addr && p_rev_queue->pData[1] == EN_MODBUS_FUNC_CODE_WRITE_SINGLE &&\
               p_rev_queue->pData[2] == (uint8_t)(MODBUS_WRITE_READ_REG_UPGRADE_STATUS >> 8) && p_rev_queue->pData[3] == (uint8_t)MODBUS_WRITE_READ_REG_UPGRADE_STATUS &&\
               p_rev_queue->pData[4] == 0X00 && p_rev_queue->pData[5] == 0X02)
            {
                
                //所有后续Pack发送完毕，检测是否重启成功
                cmd_send_interval = tim_get_systime();
                //设置重启检测时间
                reset_check_time = tim_get_systime();
                EnDownSendCmd = EN_MODBUS_SEND_STATUS_RESET_CHECK;
                
            }
            else
            {
                //ACK错误三次，进入空闲
                send_count++;
                if(send_count < 3)
                {
                    EnDownSendCmd = EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND;
                }
                else
                {
                    gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
                    EnDownSendCmd = EN_MODBUS_SEND_STATUS_IDLE;
                }
            }
            break;
        }
        /* 后续Pack重启检测 */
        case EN_MODBUS_SEND_STATUS_RESET_CHECK:
        {
            if(tim_difftime_from_now(cmd_send_interval) < 500)
            {
                break;
            }
            //只检测第二个Pack，
            //除第一个Pack，第二个Pack是最后重启的
            //此时Pack2已经复位，地址是默认地址
            _ResetCheckCmdSend(MODBUS_ADDR_DEFAULT_VALUE);
            cmd_send_interval = tim_get_systime();
            ack_time = tim_get_systime();
            reset_max_wait_time = (1 + (MODBUS_ADDR_PACK_MAX_ADDR - gUartPackInfo.ModbusAddr)) * 40000;
            EnDownSendCmd = EN_MODBUS_SEND_STATUS_RESET_CHECK_WAIT_ACK;
            break;
        }
        case EN_MODBUS_SEND_STATUS_RESET_CHECK_WAIT_ACK:
        {
            //重发120S，120S之后没有回应则不重启（每个Pack30S）
            if(tim_difftime_from_now(reset_check_time) > reset_max_wait_time)
            {
                gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
                EnDownSendCmd = EN_MODBUS_SEND_STATUS_IDLE;
                break;
            }
            //应答超时
            if(tim_difftime_from_now(ack_time) > 3000)
            {
                EnDownSendCmd = EN_MODBUS_SEND_STATUS_RESET_CHECK;
                break;
            }
            
            //接收到回包
            if(QueueGet(&gModbusHostQueue, (void *)&p_rev_queue) == false)
            {
                return;
            }
            
            //回包正确，进入下一项检测，回包错误继续重发。
            if(p_rev_queue->pData[0] == MODBUS_ADDR_DEFAULT_VALUE && p_rev_queue->pData[1] == EN_MODBUS_FUNC_CODE_WRITE_SINGLE &&\
               p_rev_queue->pData[2] == (uint8_t)(MODBUS_WRITE_READ_REG_UPGRADE_STATUS >> 8) && p_rev_queue->pData[3] == (uint8_t)MODBUS_WRITE_READ_REG_UPGRADE_STATUS &&\
               NOVA_GET_BIT(p_rev_queue->pData[5], 2) == 1)
            {
                send_count = 0;
                EnDownSendCmd = EN_MODBUS_SEND_STATUS_DMOS_CHECK;
            }
            else
            {
                EnDownSendCmd = EN_MODBUS_SEND_STATUS_RESET_CHECK;

            }
            break;

            
        }
        /* 放电MOS状态检测 */
        case EN_MODBUS_SEND_STATUS_DMOS_CHECK:
        {
            if(tim_difftime_from_now(cmd_send_interval) < 500)
            {
                break;
            }
            //只检测第二个Pack，
            //除第一个Pack，第二个Pack是最后重启的
            //此时Pack2已经复位，地址是默认地址
            DMosCheckCmdSend(MODBUS_ADDR_DEFAULT_VALUE);
            cmd_send_interval = tim_get_systime();
            ack_time = tim_get_systime();
            EnDownSendCmd = EN_MODBUS_SEND_STATUS_DMOS_CHECK_WAIT_ACK;
            break;
        }
        case EN_MODBUS_SEND_STATUS_DMOS_CHECK_WAIT_ACK:
        {
            //重发5S，5S之后没有回应则不重启
            //应答超时
            if(tim_difftime_from_now(ack_time) > 1000)
            {
                send_count++;
                if(send_count < 20)
                {
                    EnDownSendCmd = EN_MODBUS_SEND_STATUS_DMOS_CHECK;
                }
                else
                {
                    gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
                    EnDownSendCmd = EN_MODBUS_SEND_STATUS_IDLE;
                }
                break;
            }
            
            //接收到回包
            if(QueueGet(&gModbusHostQueue, (void *)&p_rev_queue) == false)
            {
                return;
            }
            
            //回包正确，且DMOS已打开。
            if(p_rev_queue->pData[0] == MODBUS_ADDR_DEFAULT_VALUE && p_rev_queue->pData[1] == EN_MODBUS_FUNC_CODE_READ_REMAIN &&\
               p_rev_queue->pData[4] != 0)
            {
                ResetFlagSet();
                gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
                EnDownSendCmd = EN_MODBUS_SEND_STATUS_IDLE;
            }
            else
            {
                send_count++;
                if(send_count < 30)
                {
                    EnDownSendCmd = EN_MODBUS_SEND_STATUS_DMOS_CHECK;
                }
                else
                {
                    gUartPackInfo.send_and_ack_complete_flag[EN_MODBUS_HOST_SLAVE_TYPE_HOST] = true;
                    EnDownSendCmd = EN_MODBUS_SEND_STATUS_IDLE;
                }
            }
            break;
        }
        default:
            break;
    }
    
}




/**
* @brief  : modbus数据包转发
* @para    : None
 * @note   : None
 * @return : None
 */
void _ModbusPackTransmit(EN_MODBUS_HOST_SLAVE_TYPE en_modbus_host_or_slave, ModbusRTURevData_t *p_pack)
{
    /*因为是转发，所以要从另一个串口转出*/
    /*因为原本的数据中已带有crc校验位，所以不用重复添加*/
    if(en_modbus_host_or_slave == EN_MODBUS_HOST_SLAVE_TYPE_HOST)
    {
        ModbusRTUSend(p_pack->pData, p_pack->data_size, EN_MODBUS_HOST_SLAVE_TYPE_SLAVE, 0);
    }
    else
    {
        ModbusRTUSend(p_pack->pData, p_pack->data_size, EN_MODBUS_HOST_SLAVE_TYPE_HOST, 0);
    }

}


/**
 * @brief  : modbus读寄存器更新
 * @para   : pPackStatus：
 * @note   : None
 * @return : None
 */
static void _ModBusReadRegUpdate(PackStatus_t *pPackStatus)
{
    uint32_t rtc_times  = 0;
    uint32_t total_cell_vol = 0;
    int32_t pack_current = 0;
    int32_t abs_pack_current = 0;
    uint32_t current_power = 0;
    uint32_t cur_max_chg_current = 0;
    uint32_t cur_max_dsg_current_10s = 0;
    uint32_t cur_max_dsg_current_30s = 0;
    uint32_t cur_max_dsg_current_60s = 0;
    static uint32_t sleep_time = 0;

    uint8_t ver[3] = {1, 11, 11};
    garrModbusOnlyReadReg[0].hardware_version                   = (ver[0] << 12) | (ver[1] << 6) | ver[2]               ;
    garrModbusOnlyReadReg[0].software_version                   = (ver[0] << 12) | (ver[1] << 6) | ver[2]               ;
    garrModbusOnlyReadReg[0].model_id                           = 0XFFFF                                                ;
    
    for(int i=0; i<10; i++)
    {
        garrModbusOnlyReadReg[0].sn[i] = 1;
    }
//    memcpy(garrModbusOnlyReadReg[0].sn, gSysInfo.sn, 20);

    for(int i=0; i<20; i++)
    {
        garrModbusOnlyReadReg[0].cell_vdc[i] = 1;
    }
//    memcpy((uint8_t*)&garrModbusOnlyReadReg[0].cell_vdc, (uint8_t*)&gCell_Info.cell_voltage, 32)    ;

    garrModbusOnlyReadReg[0].cell_max_voltage                   = 1       ;
    garrModbusOnlyReadReg[0].cell_min_voltage                   = 1       ;
    garrModbusOnlyReadReg[0].cell_average_voltage               = 1       ;
    
    garrModbusOnlyReadReg[0].cell_all_vdc_high                  = 0       ;
    garrModbusOnlyReadReg[0].cell_all_vdc_low                   = 1       ;
    
    garrModbusOnlyReadReg[0].cell_current_high                  = 0       ;
    garrModbusOnlyReadReg[0].cell_current_low                   = 1       ;
    
    //电流绝对值
    abs_pack_current = (pack_current < 0) ? (-pack_current) : pack_current;
    
    garrModbusOnlyReadReg[0].abs_cell_current_high              = 0            ;
    garrModbusOnlyReadReg[0].abs_cell_current_low               = 1            ;
    
    garrModbusOnlyReadReg[0].current_power                      = 1        ;

    garrModbusOnlyReadReg[0].soc_real                           = 1           ;
    garrModbusOnlyReadReg[0].soc_act                            = 1           ;
    garrModbusOnlyReadReg[0].soc_rpt                            = 1           ;
    
    garrModbusOnlyReadReg[0].capacity_std_ah                    = 1           ;   //1mAh -> 0.1Ah
    garrModbusOnlyReadReg[0].capacity_std_wh                    = 1           ;   //1mWh -> 0.1h
    garrModbusOnlyReadReg[0].current_max_capa_high              = 1           ;
    garrModbusOnlyReadReg[0].current_max_capa_low               = 1           ;
    garrModbusOnlyReadReg[0].pack_health                        = 1           ;
    garrModbusOnlyReadReg[0].pack_cycles                        = 1           ;

    garrModbusOnlyReadReg[0].charging_capacity_high             = 0   ;
    garrModbusOnlyReadReg[0].charging_capacity_low              = 1   ;
    garrModbusOnlyReadReg[0].discharge_capacity_high            = 0   ;
    garrModbusOnlyReadReg[0].discharge_capacity_low             = 1   ;
    
    garrModbusOnlyReadReg[0].current_max_chg_current_high       = 0            ;
    garrModbusOnlyReadReg[0].current_max_chg_current_low        = 1            ;
    
    garrModbusOnlyReadReg[0].current_max_dsg_current_10_high    = 0       ;
    garrModbusOnlyReadReg[0].current_max_dsg_current_10_low     = 1       ;
    
    garrModbusOnlyReadReg[0].current_max_dsg_current_30_high    = 0        ;
    garrModbusOnlyReadReg[0].current_max_dsg_current_30_low     = 1        ;
    
    garrModbusOnlyReadReg[0].current_max_dsg_current_60_high    = 0        ;
    garrModbusOnlyReadReg[0].current_max_dsg_current_60_low     = 1        ;
    
//    garrModbusOnlyReadRegTemp[0].pack_sleep_time_high               = pPackStatus->ocv_time_count >> 16                     ;
//    garrModbusOnlyReadRegTemp[0].pack_sleep_time_low                = (uint16_t)pPackStatus->ocv_time_count                 ;
    ////////////////////////////////////////////////
    
    if(tim_difftime_from_now(sleep_time) > 800)
    {
        sleep_time = tim_get_systime();
        garrModbusOnlyReadReg[0].pack_sleep_time_high               = (uint16_t)sleep_time                     ;
        garrModbusOnlyReadReg[0].pack_sleep_time_low++                 ;
    }
        ///////////////////////////////////////////////////
    garrModbusOnlyReadReg[0].charging_capacity2_high            = 0   ;
    garrModbusOnlyReadReg[0].charging_capacity2_low             = 1   ;
    garrModbusOnlyReadReg[0].discharge_capacity2_high           = 0  ;
    garrModbusOnlyReadReg[0].discharge_capacity2_low            = 1  ;
    garrModbusOnlyReadReg[0].pack_status                        = 7   ;
    garrModbusOnlyReadReg[0].recharge_time_high                 = 0   ;
    garrModbusOnlyReadReg[0].recharge_time_low                  = 1     ;
    garrModbusOnlyReadReg[0].mos_status                         = 0    ;
                              
    //电池包数量更新
    garrModbusOnlyReadReg[0].bms_num                            = gUartPackInfo.bms_num;
    
    garrModbusOnlyReadReg[0].ocv_flag                           = 1;
    
    //BMS运行状态更新
    garrModbusOnlyReadReg[0].bms_run_status                     =  7;
    
    garrModbusOnlyReadReg[0].cell_max_temp                      = 1        ;
    garrModbusOnlyReadReg[0].cell_min_temp                      = 1        ;
    garrModbusOnlyReadReg[0].cell_ave_temp                      = 1        ;
    garrModbusOnlyReadReg[0].afe_temp                           = 1        ;
    garrModbusOnlyReadReg[0].mos_temp                           = 1        ;
    garrModbusOnlyReadReg[0].ambient_temp                       = 1        ;
    garrModbusOnlyReadReg[0].balance_status                     = 1        ;
    
    garrModbusOnlyReadReg[0].mos_error_code         = 0XFF       ;   //mos故障
    garrModbusOnlyReadReg[0].voltage1_error_code    = 0XFF       ;   //电压故障
    garrModbusOnlyReadReg[0].voltage2_error_code    = 0XFF       ;   //电压故障2  预留
    garrModbusOnlyReadReg[0].curr_error_code        = 0XFF       ;   //电流故障
    garrModbusOnlyReadReg[0].temp_error_code        = 0XFF       ;   //温度故障
    garrModbusOnlyReadReg[0].state_error_code       = 0XFF       ;   //状态故障
    garrModbusOnlyReadReg[0].other1_error_code      = 0XFF       ;   //其他故障
    
    garrModbusOnlyReadReg[0].ems_error_code         = 0XFF       ;   //ems故障
    
    
    /* RTC Update */
    rtc_times = RtcGetTime();
    garrModbusWriteReadReg[0].rtc_times_high            = 0;
    garrModbusWriteReadReg[0].rtc_times_low             = 1;
    
    //加热膜
    garrModbusWriteReadReg[0].heat_ctl                  = GpioBitGet((&HeatEnHandle));
    gstModbusToolTestOnlyReadReg.heat_status            = GpioBitGet((&HeatEnHandle));
    
    garrModbusWriteReadReg[0].pre_mos_ctl               = 0XFF   ;
    garrModbusWriteReadReg[0].dsg_mos_ctl               = 0XFF   ;
    garrModbusWriteReadReg[0].chg_mos_ctl               = 0XFF   ;
    garrModbusWriteReadReg[0].limit_curr_ctl            = 0XFF   ;

    //数据同步更新
    memcpy(garrFirstPackModbusOnlyReadReg, garrModbusOnlyReadReg, (sizeof(ModbusOnlyReadReg_t) * gUartPackInfo.bms_num));
    
    //第一个Pack会对某些数据进行处理，
    //只有第一个Pack进行以下数据更新
    if((gUartPackInfo.ModbusAddr != MODBUS_ADDR_PACK0) || (gUartPackInfo.com_success_flag == 0))
    {
        return;
    }
    
    for(int i = 0; i < gUartPackInfo.bms_num; i++)
    {
        garrFirstPackModbusOnlyReadReg[i].current_max_chg_current_high       = 0  ;
        garrFirstPackModbusOnlyReadReg[i].current_max_chg_current_low        = 1  ;
                                                                               
        garrFirstPackModbusOnlyReadReg[i].current_max_dsg_current_10_high    = 0  ;
        garrFirstPackModbusOnlyReadReg[i].current_max_dsg_current_10_low     = 1  ;
                                                                               
        garrFirstPackModbusOnlyReadReg[i].current_max_dsg_current_30_high    = 0  ;
        garrFirstPackModbusOnlyReadReg[i].current_max_dsg_current_30_low     = 1  ;
                                                                               
        garrFirstPackModbusOnlyReadReg[i].current_max_dsg_current_60_high    = 0  ;
        garrFirstPackModbusOnlyReadReg[i].current_max_dsg_current_60_low     = 1  ;
    }

}


bool UpdateModbusReg(void)
{
//    if(gSoxDebugParam.sox_debug_mode)
//    {
//        _ModBusReadRegUpdate(&gPackStatus_debug);
//    }
//    else
//    {
        _ModBusReadRegUpdate(NULL);
//    }

    return true;
}

MSH_CMD_EXPORT(UpdateModbusReg, -- -);



///**
// * @brief  : 备份电流校准系数到flash中
//* @para    : None
// * @note   : None
// * @return : None
// */
//void CurrentCalibrationFactorSave(void)
//{
////    uint8_t save_flg[4];
////    float calibration_factor[2];
////    calibration_factor[0] = gPackStatus.chg_pack_current_calibration_factor;
////    calibration_factor[1] = gPackStatus.dsg_pack_current_calibration_factor;
////    memset((uint8_t *)save_flg, 0x5A, 4);
////    spi_flash_write_bytes_with_erase(CURRENT_CALIBRATION_FACTOR_FLASH_ADDR, (uint8_t*)calibration_factor, 8);
////    spi_flash_write_bytes_with_erase(CURRENT_CALIBRATION_FACTOR_FLASH_ADDR + 0x1000, save_flg, 4);
//}

/**
 * @brief  : 读取Flash中所存的电流校准系数
* @para    : None
 * @note   : None
 * @return : None
 */
void CurrentCalibrationFactorRead(void)
{
//    uint8_t save_flg[4];
//    float calibration_factor[2];
//    
//    //读取标志位
//    spi_flash_read_bytes(CURRENT_CALIBRATION_FACTOR_FLASH_ADDR + 0x1000, save_flg, 4);
//    //若标志位正确，则读取影响因子，否则影响因子设为1
//    if((save_flg[0] == 0X5A) && (save_flg[1] == 0X5A) && (save_flg[2] == 0X5A) && (save_flg[3] == 0X5A))
//    {
//        spi_flash_read_bytes(CURRENT_CALIBRATION_FACTOR_FLASH_ADDR, (uint8_t*)calibration_factor, 8);//Save Flag
//    
//        gPackStatus.chg_pack_current_calibration_factor = calibration_factor[0];
//        gPackStatus.dsg_pack_current_calibration_factor = calibration_factor[1];
//    }
//    else
//    {
//        gPackStatus.dsg_pack_current_calibration_factor = 1.0f;
//        gPackStatus.chg_pack_current_calibration_factor = 1.0f;
//    }
//    
//    //上下限限制
//    if(gPackStatus.dsg_pack_current_calibration_factor < 0.5f ||  gPackStatus.dsg_pack_current_calibration_factor > 1.5f)
//    {
//        gPackStatus.dsg_pack_current_calibration_factor = 1;
//    }
//    if(gPackStatus.chg_pack_current_calibration_factor < 0.5f ||  gPackStatus.chg_pack_current_calibration_factor > 1.5f)
//    {
//        gPackStatus.chg_pack_current_calibration_factor = 1;
//    }
}


/**
 * @brief  : 读取flash中存储的modbus数据备份
* @para    : None
 * @note   : None
 * @return : None
 */
bool ModbusRegRead(void)
{
    ModbusWriteReadReg_t st_MODBUS_Reg = {0};
    uint8_t save_flg[4];

    spi_flash_read_bytes(MODBUSREG_FLASH_ADDR + 0x1000, (uint8_t*)save_flg, 4);
    if(save_flg[0] != 0x5A || save_flg[1] != 0x5A || save_flg[2] != 0x5A || save_flg[3] != 0x5A)
    {
        spi_flash_read_bytes(MODBUSREG_BACKUP_FLASH_ADDR + 0x1000, (uint8_t*)save_flg, 4);
        if(save_flg[0] != 0x5A || save_flg[1] != 0x5A || save_flg[2] != 0x5A || save_flg[3] != 0x5A)
        {
            return false;
        }
        else
        {
            spi_flash_read_bytes(MODBUSREG_BACKUP_FLASH_ADDR, (uint8_t*)&st_MODBUS_Reg, sizeof(ModbusWriteReadReg_t));
            memcpy((uint8_t*)&garrModbusWriteReadReg[0], (uint8_t*)&st_MODBUS_Reg, sizeof(ModbusWriteReadReg_t));
        }
    }
    else
    {
        spi_flash_read_bytes(MODBUSREG_FLASH_ADDR, (uint8_t*)&st_MODBUS_Reg, sizeof(ModbusWriteReadReg_t));
        memcpy((uint8_t*)&garrModbusWriteReadReg[0], (uint8_t*)&st_MODBUS_Reg, sizeof(ModbusWriteReadReg_t));
    }
    return true;
}

/**
 * @brief  : 备份modbus数据到flash中
* @para    : None
 * @note   : None
 * @return : None
 */
void ModbusRegSave(void)
{
    uint8_t save_flg[4];

    memset((uint8_t *)save_flg, 0x5A, 4);
    spi_flash_write_bytes_with_erase(MODBUSREG_FLASH_ADDR, (uint8_t*)&garrModbusWriteReadReg[0], sizeof(ModbusWriteReadReg_t));
    spi_flash_write_bytes_with_erase(MODBUSREG_FLASH_ADDR + 0x1000, save_flg, 4);
    spi_flash_write_bytes_with_erase(MODBUSREG_BACKUP_FLASH_ADDR, (uint8_t*)&garrModbusWriteReadReg[0], sizeof(ModbusWriteReadReg_t));
    spi_flash_write_bytes_with_erase(MODBUSREG_BACKUP_FLASH_ADDR + 0x1000, save_flg, 4);
}


/**
 * @brief  : modbus数据初始化
* @para    : None
 * @note   : None
 * @return : None
 */
void ModbusRegInit(void)
{
    garrModbusWriteReadReg[0].mos_ot_lvl1           = 750               ;//mos过温1级故障
    garrModbusWriteReadReg[0].mos_ot_lvl2           = 850               ;//mos过温2级故障
    garrModbusWriteReadReg[0].mos_ot_lvl3           = 950               ;//mos过温3级故障
    
    garrModbusWriteReadReg[0].cell_uv_lvl1          = 2600              ;//电芯欠压1级故障 65℃≥最小温度>0℃
    garrModbusWriteReadReg[0].cell_uv_lvl2          = 2550              ;//电芯欠压2级故障 65℃≥最小温度>0℃
    garrModbusWriteReadReg[0].cell_uv_lvl3          = 2000              ;//电芯欠压3级故障 65℃≥最小温度>0℃
    garrModbusWriteReadReg[0].cell_ov_lvl1          = 3650              ;//电芯过压1级故障 65℃≥最小温度>0℃
    garrModbusWriteReadReg[0].cell_ov_lvl2          = 3700              ;//电芯过压2级故障 65℃≥最小温度>0℃
    garrModbusWriteReadReg[0].cell_ov_lvl3          = 3750              ;//电芯过压3级故障 65℃≥最小温度>0℃
    
    garrModbusWriteReadReg[0].pack_uv_lvl_low       = 32000             ;//32000 MSB 0x409C (2.0 * 电芯串数)
    garrModbusWriteReadReg[0].pack_ov_lvl_high      = 0                 ;//
    garrModbusWriteReadReg[0].pack_ov_lvl_low       = 58400             ;//58400 MSB 0xE420 （3.65*电芯串数）
    garrModbusWriteReadReg[0].pack_uv_lvl_high      = 0                 ;//
    garrModbusWriteReadReg[0].cell_voltage_diff     = 350               ;//电芯压差大350
    
    garrModbusWriteReadReg[0].dsg_oc_lvl1           = 120               ;//放电过流1级故障
    garrModbusWriteReadReg[0].dsg_oc_lvl2           = 130               ;//放电过流2级故障
    garrModbusWriteReadReg[0].dsg_oc_lvl3           = 140               ;//放电过流3级故障
    garrModbusWriteReadReg[0].chg_oc_lvl1           = 120               ;//充电过流1级故障
    garrModbusWriteReadReg[0].chg_oc_lvl2           = 130               ;//充电过流2级故障
    garrModbusWriteReadReg[0].chg_oc_lvl3           = 140               ;//充电过流3级故障
    
    garrModbusWriteReadReg[0].cell_ut_lvl           = (uint16_t)(-100)  ;
    garrModbusWriteReadReg[0].cell_ot_lvl1          = 550               ;
    garrModbusWriteReadReg[0].cell_ot_lvl2          = 600               ;
    garrModbusWriteReadReg[0].cell_ot_lvl3          = 650               ;
    garrModbusWriteReadReg[0].cell_temp_diff        = 100               ;
    
    garrModbusWriteReadReg[0].soh_under_lvl1        = 800               ;
    garrModbusWriteReadReg[0].soh_under_lvl2        = 750               ;
    garrModbusWriteReadReg[0].soh_under_lvl3        = 700               ;


    garrModbusWriteReadReg[0].dsg_mos_ctl           = 0;
    garrModbusWriteReadReg[0].chg_mos_ctl           = 0;
    garrModbusWriteReadReg[0].pre_mos_ctl           = 0;

    /*modbus只读寄存器、写读寄存器指针初始化*/
    for(int i = 0 ; i < BMS_PACK_MAX_NUM ; i++)
    {
        gUartPackInfo.p_modbus_only_read_reg[i]        = &garrModbusOnlyReadReg[i]     ;
        gUartPackInfo.p_modbus_write_read_reg[i]       = &garrModbusWriteReadReg[i]    ;
    }
    /*寄存器长度初始化*/
    gUartPackInfo.modbus_only_read_reg_size         = sizeof(ModbusOnlyReadReg_t)   ;
    gUartPackInfo.modbus_write_read_reg_size          = sizeof(ModbusWriteReadReg_t)  ;
    
    gUartPackInfo.ModbusAddr                        = MODBUS_ADDR_DEFAULT_VALUE     ;   //默认电池包的地址
    gUartPackInfo.is_final_bms                      = true                          ;   //是否为最后一个电池包
    gUartPackInfo.final_bms_addr                    = MODBUS_ADDR_PACK0             ;   //默认最后一个电池包的地址
    gUartPackInfo.bms_num                           = 1                             ;   //默认电池包数量1
    gUartPackInfo.com_out_time                      = 60000                        ;   //通讯超时时间(ms) 
    gUartPackInfo.final_com_time                    = tim_get_systime()             ;   //最后的通讯时间
    
    for(int i = 0; i < UART_NUM; i++)
    {
        gUartPackInfo.send_and_ack_complete_flag[i] = true                          ;   //完成一次收发，允许再次发送标志
    }
    
    for(int i = 0; i < BMS_PACK_MAX_NUM; i++)
    {
//        gstAllPackLimit.chg_ending_factor[i] = 1.0f;
//        gstAllPackLimit.chg_end_factor[i]    = 1.0f;
//        gstAllPackLimit.dsg_ending_factor[i] = 1.0f;
//        gstAllPackLimit.dsg_end_factor[i]    = 1.0f;

//        gstAllPackLimit.arr_chg_limit_final[i]         = 500;
//        gstAllPackLimit.arr_dsg_limit_10s_final[i]     = 500;
//        gstAllPackLimit.arr_dsg_limit_30s_final[i]     = 500;
//        gstAllPackLimit.arr_dsg_limit_60s_final[i]     = 500;
//        
//        gstAllPackLimit.arr_chg_current_limit[i] = 500;
//        gstAllPackLimit.arr_dsg_current_limit_10s[i] = 500;
//        gstAllPackLimit.arr_dsg_current_limit_30s[i] = 500;
//        gstAllPackLimit.arr_dsg_current_limit_60s[i] = 500;
    }
}


/**
 * @brief  : 选择bms地址对应的modbus数据结构体
 * @para   : n_bms_addr:待查询的bms地址
 * @note   : None
 * @return : 数据结构体编号
 */
uint8_t _ModbusRegStructSelect(uint8_t n_bms_addr)
{
    return MODBUS_ADDR_PACK_MAX_ADDR - n_bms_addr;
}
