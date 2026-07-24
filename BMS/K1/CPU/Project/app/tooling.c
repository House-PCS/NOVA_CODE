
#include "tooling.h"
#include "debug.h"
#include "dev_led.h"
#include "dev_mos.h"
#include "modbus_analysis.h"
#include "bms_protocol.h"
#include "dev_spiflash.h"
#include "rtc.h"

#define LOG_TAG   "TOOL"


typedef enum
{
    EN_TOOLING_STEPS_IDLE,              //空闲
    EN_TOOLING_STEPS_GET_P_VOLTAGE,     //获取P端电压
} EN_TOOLING_STEPS;



/* 进入工装模式后的处理 */
void _ToolingModeInterProcess(void);
/* flash测试 */
void _FlashTest(void);
/* eeprom测试 */
void _EepromTest(void);


/**
 * @brief  : 工装初始化
 * @para   : None
 * @note   : None
 * @return : None
 */
void ToolingInit()
{
    //该函数只会执行一次，进入过工装模式后不会再次执行
    static bool is_first_enter_tooling_flag = false;
    static uint8_t remember_flag = 0;
    
    if(is_first_enter_tooling_flag == true)
    {
        //后续进入该函数时强制标志置位
        gstModbusToolTestWriteReadReg.test_mode_enter_ctrl = remember_flag;        //测试模式标志位
        return;
    }
    
	//更改通讯超时时间
    switch(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl)
    {
        case EN_TEST_MODE_REG_NORMAL:
        {
            gUartPackInfo.com_out_time = 200000;
            break;
        }
        case EN_TEST_MODE_REG_TOOL:
        {
            gUartPackInfo.com_out_time = 20000;
			LedCtlTool(LED_SRC_SYS);
            break;
        }
        case EN_TEST_MODE_REG_AGING_TEST:
        {
            gUartPackInfo.com_out_time = 20000;
			LedCtlAging(LED_SRC_SYS);
            break;
        }
        default:
            break;
    }
	
    //判断测试模式标志是否置位，若未置位则不进行后续步骤
    if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl == 0)
    {
        return;
    }
//    memset((uint8_t*)&gstModbusToolTestWriteReadReg, 0, sizeof(ModbusToolTestWriteReadModeReg_t));   //工装测试初始化清零
    memset((uint8_t*)&gstModbusToolTestOnlyReadReg, 0, sizeof(ModbusToolTestOnlyReadModeReg_t));
     
    remember_flag = gstModbusToolTestWriteReadReg.test_mode_enter_ctrl;             //记录测试模式标志
    _ToolingModeInterProcess();                                                     //进入测试模式初步处理
    is_first_enter_tooling_flag = true;
}    

/**
 * @brief  : 进入测试模式时的动作
 * @para   : None
 * @note   : None
 * @return : None
 */
void _ToolingModeInterProcess()
{
    //关闭mos、关闭限流
    CloseDisChargeMos();
    CloseChargeMos();
    ClosePreChargeMos();
    CCEnOff();
    
    //打开OCV使能
    OVCEnOpen();
    
    //Flash及Eeprom测试
    _FlashTest();
    _EepromTest();
    
}

/**
 * @brief  : Flash测试
 * @para   : None
 * @note   : None
 * @return : None
 */
void _FlashTest()
{
    uint8_t arr_write_test_data[10] = {4, 2, 5, 2, 6, 0, 8, 9, 0, 2};
    uint8_t arr_rec_test_buff[10];
    memset(arr_rec_test_buff, 0, 10);
    
    //flash读写测试
    spi_flash_write_bytes_with_erase(EXTER_FLASH_TEST_ADDR,arr_write_test_data, 10);
    spi_flash_read_bytes(EXTER_FLASH_TEST_ADDR, arr_rec_test_buff, 10);
    
    if(memcmp(arr_rec_test_buff, arr_write_test_data, 10) != 0)
    {
        gstModbusToolTestOnlyReadReg.flash_error_flag = 1;
    }
    else
    {
        gstModbusToolTestOnlyReadReg.flash_error_flag = 0;
    }
}

/**
 * @brief  : Eeprom测试
 * @para   : None
 * @note   : None
 * @return : None
 */
void _EepromTest()
{
    uint8_t arr_write_test_data[10] = {4, 2, 5, 2, 6, 0, 8, 9, 0, 2};
    uint8_t arr_rec_test_buff[10];
    memset(arr_rec_test_buff, 0, 10);
    
    //eeprom读写测试
    EepromWrite(arr_write_test_data, EXTER_EEPROM_TEST_ADDR, 10);
    //写与读之间要有2ms的延时，否则读不出数据
    soft_delayms(5);
    EepromRead(arr_rec_test_buff, EXTER_EEPROM_TEST_ADDR, 10);
    
    if(memcmp(arr_rec_test_buff, arr_write_test_data, 10) != 0)
    {
        gstModbusToolTestOnlyReadReg.eeprom_error_flag = 1;
    }
    else
    {
        gstModbusToolTestOnlyReadReg.eeprom_error_flag = 0;
    }
}


/**
 * @brief  : 工装测试步骤处理
 * @para   : None
 * @note   : None
 * @return : None
 */
void ToolingProcess()
{
    static EN_TOOLING_STEPS tool_steps  = EN_TOOLING_STEPS_IDLE;
    static uint32_t steps_wait_time     = 0;                        //每两步之间的间隔时间
    
    //非工装模式，该函数不运行
    if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl != 1)
    {
        return;
    }
        
    switch(tool_steps)
    {
        case EN_TOOLING_STEPS_IDLE:
        {
            tool_steps = EN_TOOLING_STEPS_GET_P_VOLTAGE;
            break;
        }
        case EN_TOOLING_STEPS_GET_P_VOLTAGE:
        {
            gstModbusToolTestOnlyReadReg.pack_adc_val       = gCell_Info.total_cell_voltage / 100;  /* 单位0.1V */
            gstModbusToolTestOnlyReadReg.charger_voltage    = OVCVoltageGet() / 100;                /* 单位0.1V */
            tool_steps = EN_TOOLING_STEPS_IDLE;
            break;
        }
        default:
            break;
    }
}


/************版本及ID读取*************/
typedef enum
{
    EN_ENDURANCE_TEST_MOS_READ_STATUS_READ              ,           /* 版本读取 */       
    EN_ENDURANCE_TEST_MOS_READ_STATUS_WAIT_ACK          ,           /* 等待应答 */
    EN_ENDURANCE_TEST_MOS_READ_STATUS_INTERVAL          ,           /* 等待下一轮读取（间隔） */
} EN_ENDURANCE_TEST_MOS_READ_STATUS;



/**
 * @brief  : 通讯长期运行耐久度测试
 * @para   : 
 * @note   : None
 * @return : 0:测试中，1:测试成功，2:测试失败
 */
uint8_t _ComEnduranceTestFunc()
{
    static ModbusRTURevData_t    *p_rtu_cmd;
    static EN_ENDURANCE_TEST_MOS_READ_STATUS   e_status      = EN_ENDURANCE_TEST_MOS_READ_STATUS_READ;
    static uint32_t              wait_ack_time  = 0;
    static uint32_t              interval_time  = 0;                 //间隔时间
    static uint8_t               error_flag     = 0;                 //错误标志
    static uint8_t               out_time_count = 0;                 //超时计数
    static uint8_t               rev_error_count= 0;                 //接收错误计数
    uint8_t                      return_flag    = 0;                 //返回标志
      
    switch(e_status)
    {
        /*********** MOS状态读取 *********/
        case EN_ENDURANCE_TEST_MOS_READ_STATUS_READ:           
        {
            //读取MOS状态
            DMosCheckCmdSend(MODBUS_ADDR_DEFAULT_VALUE);
            
            wait_ack_time = tim_get_systime();
            e_status = EN_ENDURANCE_TEST_MOS_READ_STATUS_WAIT_ACK;
            break;           
        }
        /* 等待ack应答 */
        case EN_ENDURANCE_TEST_MOS_READ_STATUS_WAIT_ACK:           
        {
            if(tim_difftime_from_now(wait_ack_time) > 1000)
            {
                //应答超时
                out_time_count++;
                //超时计数限值
                out_time_count  = out_time_count > 15 ? 15 : out_time_count;
                
                //计算两帧的间隔时间
                interval_time = tim_get_systime();
                e_status = EN_ENDURANCE_TEST_MOS_READ_STATUS_INTERVAL;
                
                break;
            }

            //接收到回包
            if(QueueGet(&gModbusHostQueue, (void *)&p_rtu_cmd) == false)
            {
                return 0;
            }
            
            //回包正确。
            if((p_rtu_cmd->pData[0] == MODBUS_ADDR_DEFAULT_VALUE) && (p_rtu_cmd->pData[1] == EN_MODBUS_FUNC_CODE_READ_REMAIN))
            {
                //清除接收错误计数
                rev_error_count = 0;
            }
            else
            {
                rev_error_count++;
                //接收错误计数限值
                rev_error_count  = rev_error_count > 15 ? 15 : rev_error_count;
            }
            //清除接收超时计数
            out_time_count = 0;
            
            //计算两帧的间隔时间
            interval_time = tim_get_systime();
            e_status = EN_ENDURANCE_TEST_MOS_READ_STATUS_INTERVAL;
            
            break;           
        }
        /* 结束 */
        case EN_ENDURANCE_TEST_MOS_READ_STATUS_INTERVAL:           
        {
            //返回值设置
            if((rev_error_count > 10) || (out_time_count > 10))
            {   //错误
                return_flag = 2;
            }
            else
            {   //正确
                return_flag = 1;
            }
            
            //间隔1S，进行下一次数据读取
            if(tim_difftime_from_now(interval_time) > 1000)
            {
                e_status = EN_ENDURANCE_TEST_MOS_READ_STATUS_READ;
            }
           
            break;
        }
        default:
            break;
    }
    return return_flag;    
}

/**
 * @brief  : 故障存储（长期耐久测试使用）
 * @para   : com_error_flag:通讯故障标志
 * @note   : None
 * @return : 
 */
void _ErrorSaveEnduranceTestFunc(uint8_t com_error_flag)
{
    uint8_t  save_buff[32];                 //数据存储buff
    uint8_t  read_buff[32];                 //读取buff
    uint8_t  error_level            = 0;    //故障等级
    static uint32_t first_rtc_time  = 0;    //首次上电时间
    uint32_t rtc_time               = 0;    //本次RTC时间
    static uint32_t last_rtc_time   = 0;    //RTC时间记录
    uint32_t delta_rtc_time         = 0;    //上电时长   = rtc_time - first_rtc_time;
    
    static uint32_t save_addr = ENDURANCE_TEST_ERROR_INFO_FLASH_ADDR;   //数据保存地址
    
    //获取系统首次上电的RTC时间
    if(first_rtc_time == 0)
    {
        first_rtc_time  = RtcGetTime();
        rtc_time        = first_rtc_time;
        last_rtc_time   = first_rtc_time;
    }
    
    error_level = GetSystemErrorLevel();
    rtc_time = RtcGetTime();
    
    //半个小时判断一次
    if(rtc_time - last_rtc_time <= 1800)
    {
        return;
    }
    
    //有故障出现
    if((error_level != 0) || (com_error_flag == 2))
    {
        //记录故障数据及时间
        //存储BUFF清空
        memset(save_buff, 0, 32);
        memset(read_buff, 0, 32);
        //数据头
        save_buff[0] = 0XAA;
        //记录故障
        save_buff[1] = com_error_flag;
        memcpy((uint8_t*)&save_buff[2], (uint8_t*)&gBmsErrorInfo, sizeof(gBmsErrorInfo));
        
        //记录时间
        delta_rtc_time = rtc_time - first_rtc_time;
        memcpy((uint8_t*)&save_buff[27], (uint8_t*)&delta_rtc_time, 4);
        
        //数据尾
        save_buff[31] = 0X55;
        
        //存储过后读取验证，若验证有误则再次写入，循环5次
        for(int i = 0; i < 5; i++)
        {
            //存储
            spi_flash_write_bytes_without_erase(save_addr, save_buff, 32); 
            soft_delayms(5);

            //读取保存的数据
            spi_flash_read_bytes(save_addr, read_buff, 32);
            
            //确认备份成功
            if((read_buff[0] == 0XAA) && (read_buff[31] == 0X55))
            {
                //地址偏移
                save_addr += 32;
                //停止重复备份
                break;
            }
            soft_delayms(5);
        }
    }
    
    //记录RTC时间
    last_rtc_time = rtc_time;
}

//uint8_t test_flag;
//uint8_t read_data_buff[1024];
//void FlashReadTest()
//{
//    
//    static uint32_t read_addr = ENDURANCE_TEST_ERROR_INFO_FLASH_ADDR;

//    if(test_flag == 1)
//    {
//        //清除
//        spi_flash_auto_erase_sector(ENDURANCE_TEST_ERROR_INFO_FLASH_ADDR, 1024);
//        test_flag = 0;
//    }
//    //读取保存的数据
//    spi_flash_read_bytes(read_addr, read_data_buff, 1024);
//}


void _DebugInfoOutput()
{
    static uint32_t time = 0;
    
    if(tim_difftime_from_now(time) < 5000)
    {
        return;
    }
    
    time = tim_get_systime();
    
    LOG_DEBUG(LOG_TAG, "--MaxVol: %d --MinVol: %d  --\n\r", gCell_Info.cell_max_voltage, gCell_Info.cell_min_voltage);
    LOG_DEBUG(LOG_TAG, "--MosT: %d --AfeT: %d --AmbT: %d --\n\r", gCell_Info.mos_temperature,
                                                                  gCell_Info.afe_ntc_temperature, 
                                                                  gCell_Info.afe_ambient_temperature);
    LOG_DEBUG(LOG_TAG, "--CellT: %d %d %d %d --\n\r", gCell_Info.arr_cell_temperature[0], 
                                                      gCell_Info.arr_cell_temperature[1], 
                                                      gCell_Info.arr_cell_temperature[2], 
                                                      gCell_Info.arr_cell_temperature[3]);
    LOG_DEBUG(LOG_TAG, "--Error[VTC]: 0X%x 0X%x 0X%x --\n\r", gBmsErrorInfo.voltage1_error_code.value, 
                                                              gBmsErrorInfo.temper_error_code.value, 
                                                              gBmsErrorInfo.curr_error_code.value);
}

/**
 * @brief  : 长期运行耐久度测试
        LED闪烁方式如下：
                单故障：
                    通讯异常闪烁两下
                    电压温度异常闪三下
                    电流异常闪四下
                    单故障闪烁周期2S
                双故障：
                    通讯及电压异常，闪烁两下，周期3S
                    通讯及电流异常，闪烁两下，周期4S
                    电压及电流异常，闪烁三下，周期3S
                全故障：
                    闪烁两下，周期5S
 * @para   : 
 * @note   : None
 * @return : 0:测试中，1:测试成功，2:测试失败
 */
void EnduranceTestFunc()
{
    static  uint8_t  com_error_flag     = 0;            //通讯错误标志  
    static  uint8_t  error_change_flag  = 0;            //故障更新标志  
            uint8_t  blink_count        = 0;            //闪烁次数
            uint16_t blink_period       = 2000;         //闪烁周期
    static  uint8_t  error_flag_old[3]  = {0,0,0};      //闪烁周期（记录）
            uint8_t  error_flag[3]      = {0,0,0};      //闪烁周期      
    
    //只有耐久度测试模式允许运行
    if((_gBMSSysInfo.endurance_mode_flag != 1) || 
       (gstModbusToolTestWriteReadReg.test_mode_enter_ctrl == 2))
    {
        return;
    }
            
    //通讯测试
    com_error_flag  = _ComEnduranceTestFunc();
    _ErrorSaveEnduranceTestFunc(com_error_flag);
    _DebugInfoOutput();
            
    //设置故障标志
    if(com_error_flag == 2)
    {   //通讯故障
        error_flag[0] = 1;
    }
    if((gBmsErrorInfo.voltage1_error_code.value != 0) || (gBmsErrorInfo.temper_error_code.value != 0))
    {   //电压温度故障
        error_flag[1] = 1;
    }
    if(gBmsErrorInfo.curr_error_code.value != 0)
    {   //电流故障
        error_flag[2] = 1;
    }
    
    memcpy(error_flag_old, error_flag, 3);
            
    //有故障
    if((error_flag[0] != 0) || (error_flag[1] != 0) || (error_flag[2] != 0))
    {
        //通讯故障或出现板间故障，LED闪烁方式改变
        if(error_flag[0] == 1)
        {
            blink_count = 2;
        }
        else if(error_flag[1] == 1)
        {
            blink_count = 3;
        }
        else if(error_flag[2] == 1)
        {
            blink_count = 4;
        }
        
        //多故障，改变闪烁周期
        if((error_flag[0] == 1) && (error_flag[1] == 1) && (error_flag[2] == 1))
        {
            blink_period = 5000;
        }
        else if(((error_flag[0] == 1) && (error_flag[1] == 1)) ||\
           ((error_flag[1] == 1) && (error_flag[2] == 1)))
        {
            blink_period = 3000;
        }
        else if((error_flag[0] == 1) && (error_flag[2] == 1))
        {
            blink_period = 4000;
        }
        
        //故障有所更改，设置LED
        if((error_flag[0] != error_flag_old[0]) ||\
           (error_flag[1] != error_flag_old[1]) ||\
           (error_flag[2] != error_flag_old[2])
          )
        {
            error_change_flag = 1;
        }
        //设置LED(要更新LED必须更新LED的模式，否则会更新失败)
        if(error_change_flag == 1)
        {
            LedCtlOff(LED_SRC_SYS);
            error_change_flag = 0;
        }
        else
        {
            LedBlinkSet(LED_SRC_SYS, blink_count, blink_period);
        }
    }
}








