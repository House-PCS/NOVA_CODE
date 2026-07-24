


#include "tools_program.h"
#include "slave_collect.h"
#include "dev_spiflash.h"
#include "sys_info_storage.h"
#include "systick.h"
#include "dev_tca9555.h"
#include "dev_it7c4337.h"
#include "storage_cap.h"
ControlPara_t  gControlParaData;
GpioStatus_t getgpiovalue, pre_getgpiovalue;

uint16_t gToolsGetTimeFlag;

#define TOOLS_TEST_FLASH_ADDR           0x900000

static uint8_t _garrDevComUartTestData[4] = {0x56, 0x67, 0x78, 0x89};
static uint8_t _garrDevComUartTestErrData[2] = {0xbb, 0x55};


/**
  * @brief   获取BMS版本号
  * @param   None
  * @return  None
  * @note    None
  */

static bool _GetBMSVersion(uint16_t hw, uint16_t sw)
{
    uint16_t hw_ver, sw_ver;
    hw_ver = 0x0000;
    sw_ver = 0x1000;

    if((hw == hw_ver) & (sw == sw_ver))
    {
        return true;
    }
    else
    {
        return false;
    }
}


/**
  * @brief   获取BMS版本号
  * @param   None
  * @return  None
  * @note    None
  */

static bool _GetPcsVersion(uint16_t hw, uint16_t sw)
{

    uint16_t hw_ver, sw_ver;

    if((hw == hw_ver) & (sw == sw_ver))
    {
        return true;
    }
    else
    {
        return false;
    }


}

/**
  * @brief   检测协处理器的串口是否OK
  * @param   None
  * @return  None
  * @note    None
  */
static bool _CheckCopUartIsNormal(uint32_t sw)
{
    if(sw != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/**
  * @brief   写flash
  * @param   None
  * @return  None
  * @note    None
  */
static void _WriteFlashAddr(uint16_t value)
{
    uint8_t buf[256];

    SpiFlashAutoEraseSectors(TOOLS_TEST_FLASH_ADDR, 256);
    memcpy(buf, (uint8_t *)&value, 2);
    SpiFlashWriteBytesWithoutErase(TOOLS_TEST_FLASH_ADDR, buf, 2);//写的数据
}

/**
  * @brief   读flash
  * @param   None
  * @return  None
  * @note    None
  */
static bool _ReadFlashAddr(void)
{
    bool result;
    uint8_t buf[256];

    SpiFlashReadData(TOOLS_TEST_FLASH_ADDR, buf, 2);
    if(buf[0] == 0xBB && buf[1] == 0x55)
    {
        result = true;
    }
    else
    {
        result = false;
    }
    return result;
}



/**
  * @brief   检测flash是否正常
  * @param   None
  * @return  None
  * @note    1:写成功 0:写失败
  */
uint16_t TestFlashIsOk(void)
{
    uint16_t data = 0x55BB;
    uint16_t res;

    static uint8_t flash_flag = 0;
    if(!flash_flag)
    {
        _WriteFlashAddr(data);
        flash_flag = 1;
    }

    if(_ReadFlashAddr())
    {
        res = 1;
    }
    else
    {
        res = 0;
    }

    return res;
}




/**
  * @brief   获取寄存器中的rtc时间
  * @param   None
  * @return  None
  * @note    None
  */
uint32_t GetRegRtcTime(void)
{
    uint32_t time = 0;

    time = gControlParaData.rtc_time[0] << 16 | gControlParaData.rtc_time[1];

    return time;
}


/**
  * @brief   获取GPIO高低电平
  * @param   Gpio_HandleType
  * @return  0--低 1--高
  * @note    None
  */
void GetStableGpioBitData(void)
{

    uint8_t GpioStatus[8];
    uint8_t i = 0;

    for(i = 0; i < 8; i++)
    {
        GpioStatus[i] = GpioBitGet(&(ConnectGPIOHandle[i]));
//         if(GpioStatus[i] == 1)
//        {
//            getgpiovalue.byte_value |= (1 << i);
//        }
//        else
//        {
//            getgpiovalue.byte_value |= (0 << i);
//        }
    }
#if 1
    //0
    if(GpioStatus[0] == 1)
    {
        getgpiovalue.bits.bit0 = 1;
    }
    else
    {
        getgpiovalue.bits.bit0 = 0;
    }

    //1
    if(GpioStatus[1] == 1)
    {
        getgpiovalue.bits.bit1 = 1;
    }
    else
    {
        getgpiovalue.bits.bit1 = 0;
    }

    //2
    if(GpioStatus[2] == 1)
    {
        getgpiovalue.bits.bit2 = 1;
    }
    else
    {
        getgpiovalue.bits.bit2 = 0;
    }

    //3
    if(GpioStatus[3] == 1)
    {
        getgpiovalue.bits.bit3 = 1;
    }
    else
    {
        getgpiovalue.bits.bit3 = 0;
    }

    //4
    if(GpioStatus[4] == 1)
    {
        getgpiovalue.bits.bit4 = 1;
    }
    else
    {
        getgpiovalue.bits.bit4 = 0;
    }

    //5
    if(GpioStatus[5] == 1)
    {
        getgpiovalue.bits.bit5 = 1;
    }
    else
    {
        getgpiovalue.bits.bit5 = 0;
    }

    //6
    if(GpioStatus[6] == 1)
    {
        getgpiovalue.bits.bit6 = 1;
    }
    else
    {
        getgpiovalue.bits.bit6 = 0;
    }

    //7
    if(GpioStatus[7] == 1)
    {
        getgpiovalue.bits.bit7 = 1;
    }
    else
    {
        getgpiovalue.bits.bit7 = 0;
    }
#endif
}


/**
  * @brief   获取所有IO的稳定电平
  * @param   None
  * @return  None
  * @note    None
  */
uint8_t GetStableGpioLevel(void)
{
    uint8_t cnt = 0;
    uint8_t value;
    while(1)
    {
        if(pre_getgpiovalue.byte_value == getgpiovalue.byte_value)
        {
            cnt++;
            if(cnt > 10)
            {
                value = getgpiovalue.byte_value;
                return value;
            }
        }
        else
        {
            cnt = 0;
            pre_getgpiovalue.byte_value = getgpiovalue.byte_value;
            break;
        }
    }
    return value;
}

/**
  * @brief   拉高CTRL_L/CTRL_R电平信号
  * @param   None
  * @return  None
  * @note    None
  */
static void _SetGpioLevel(void)
{
    //拉高左侧灯板
    GpioBitSet((&OutputGPIOHandle[0]));
    //拉高右侧灯板
    GpioBitSet((&OutputGPIOHandle[1]));
}

/**
  * @brief   拉低CTRL_L/CTRL_R电平信号
  * @param   None
  * @return  None
  * @note    None
  */
static void _ResetGpioLevel(void)
{
    //拉高左侧灯板
    GpioBitReset((&OutputGPIOHandle[0]));
    //拉高右侧灯板
    GpioBitReset((&OutputGPIOHandle[1]));
}
/**
  * @brief   拉高继电器1 继电器2电平信号
  * @param   None
  * @return  None
  * @note    None
  */
void SetDelayLevel(void)
{
//    DevTca9555SetPort1SinglePinStatus((EN_TCA9555_CH)5, 1);     //继电器2
//    DevTca9555SetPort1SinglePinStatus((EN_TCA9555_CH)4, 1);     //继电器1
}


/**
  * @brief   拉低继电器1 继电器2电平信号
  * @param   None
  * @return  None
  * @note    None
  */
void ResetDelayLevel(void)
{
//    DevTca9555SetPort1SinglePinStatus((EN_TCA9555_CH)5, 0);     //继电器2
//    DevTca9555SetPort1SinglePinStatus((EN_TCA9555_CH)4, 0);     //继电器1
}

/**
  * @brief   校验时间标志
  * @param   None
  * @return  None
  * @note    None
  */
static uint16_t GetTestToolsCorrectRtcStatus()
{
    return gToolsGetTimeFlag;
}

/**
  * @brief   清除校验时间标志
  * @param   None
  * @return  None
  * @note    None
  */
static void CleanGetTestToolsCorrectRtcStatus()
{
    gToolsGetTimeFlag = 0;
}


static uint8_t _RecRs485DataAnalysis(DevModbus_t *p_dev)
{
    if(p_dev->mode == EN_MODBUS_MODE_HOST_VOLTMETER_ARC)
    {
        if(p_dev->rxbuf[0] == 0x12 && p_dev->rxbuf[1] == 0x23 && p_dev->rxbuf[2] == 0x34 && p_dev->rxbuf[3] == 0x45)
        {
            ModbusVoltmeterRTUSend(&_garrDevComUartTestData[0], 4);
        }
        else
        {
            ModbusVoltmeterRTUSend(&_garrDevComUartTestErrData[0], 2);
        }
    }
    else if(p_dev->mode == EN_MODBUS_MODE_HOST_PCS)
    {
        if(p_dev->rxbuf[0] == 0x12 && p_dev->rxbuf[1] == 0x23 && p_dev->rxbuf[2] == 0x34 && p_dev->rxbuf[3] == 0x45)
        {
            ModbusPCSRTUSend(&_garrDevComUartTestData[0], 4);
        }
        else
        {
            ModbusPCSRTUSend(&_garrDevComUartTestErrData[0], 2);
        }
    }
    // else if(p_dev->mode == EN_MODBUS_MODE_HOST_COPROCOESSOR)
    // {
    //     if(p_dev->rxbuf[0] == 0x12 && p_dev->rxbuf[1] == 0x23 && p_dev->rxbuf[2] == 0x34 && p_dev->rxbuf[3] == 0x45)
    //     {
    //         ModbusCoprocessorRTUSend(&_garrDevComUartTestData[0], 4);
    //     }
    //     else
    //     {
    //         ModbusCoprocessorRTUSend(&_garrDevComUartTestErrData[0], 2);
    //     }
    // }

    return true;
}


/**
  * @brief   测试Rs485回调函数的初始化
  * @param   None
  * @return  None
  * @note    None
  */
void ToolingTestRs485Init(void)
{
    RegistTestRs485FunPoint(_RecRs485DataAnalysis);
}


/**
  * @brief   轮询检测
  * @param   None
  * @return  None
  * @note    None
  */
void TestToolsProcess(void)
{
    static uint32_t wait_time;
    static uint32_t wait_time1;
    static uint32_t rtc_wait_time;
    static uint16_t io_get;
    static uint32_t time;
    static uint32_t first_rtc_time;
    static uint32_t next_rtc_time;
    static uint8_t  enter_flag = 0;
    static uint16_t tmp_flash_res = 0xffff;

    static EN_TOOLING_STATUS en_tooling_step;

    switch(en_tooling_step)
    {
        case EN_TOOLING_TEST_IDLE:
        {
            if(!enter_flag)
            {
                SetEnterTestModeFlag(gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode);
                enter_flag = 1;
                first_rtc_time = GetRtcTimeStamp();
            }
            en_tooling_step = EN_TOOLING_TEST_DRIVE_IO;
            break;
        }
        case EN_TOOLING_TEST_DRIVE_IO:
        {
            if(gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.relay_io == 0x01)
            {
                _SetGpioLevel();     //继电器
                if(tim_difftime_from_now(wait_time) > 10)
                {
                    wait_time = tim_get_systime();
                    en_tooling_step = EN_TOOLING_TEST_RTC;
                }
            }

            if(gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.relay_io == 0x00)
            {
                _ResetGpioLevel();     //继电器
                if(tim_difftime_from_now(wait_time1) > 10)
                {
                    wait_time1 = tim_get_systime();
                    en_tooling_step = EN_TOOLING_TEST_RTC;
                }
            }
            rtc_wait_time = tim_get_systime();
            break;
        }
        case EN_TOOLING_TEST_RTC:
        {
            if((uint32_t)(tim_get_systime() - rtc_wait_time) > 500)
            {
                next_rtc_time = GetRtcTimeStamp();

                gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.rtc[0] = next_rtc_time >> 16;
                gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.rtc[1] = next_rtc_time;

                if(next_rtc_time > first_rtc_time)
                {
                    gstCollectInfo.maincard_data.inside_info.tool_info.status_info.rtc_result = 1;  //成功
                }
                else
                {
                    gstCollectInfo.maincard_data.inside_info.tool_info.status_info.rtc_result = 2;  //失败
                }

                en_tooling_step = EN_TOOLING_TEST_GET_IO;
                rtc_wait_time = tim_get_systime();
            }
            break;
        }
        case EN_TOOLING_TEST_GET_IO:
        {
            GetStableGpioBitData();
            io_get = GetStableGpioLevel();

            gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.drm_enable = io_get;
            en_tooling_step = EN_TOOLING_TEST_FLASH;

            break;
        }
        case EN_TOOLING_TEST_FLASH:
        {
            tmp_flash_res = TestFlashIsOk();
            if(tmp_flash_res == 1)
            {
                gstCollectInfo.maincard_data.inside_info.tool_info.status_info.flash_result = 1;  //成功
                en_tooling_step = EN_TOOLING_TEST_COP_UART;
            }
            else
            {
                gstCollectInfo.maincard_data.inside_info.tool_info.status_info.flash_result = 2;  //失败
                en_tooling_step = EN_TOOLING_TEST_COP_UART;
            }
            break;
        }
        case EN_TOOLING_TEST_COP_UART:
        {
            if(_CheckCopUartIsNormal(gstCollectInfo.maincard_data.inside_info.main_cop_info.base_info.sw_ver[0] << 16 |
                                     (gstCollectInfo.maincard_data.inside_info.main_cop_info.base_info.sw_ver[1])))
            {
                gstCollectInfo.maincard_data.inside_info.tool_info.status_info.cop_uart_result = 1;  //成功
                en_tooling_step = EN_TOOLING_TEST_IDLE;
            }
            else
            {
                gstCollectInfo.maincard_data.inside_info.tool_info.status_info.cop_uart_result = 2;  //失败
                en_tooling_step = EN_TOOLING_TEST_IDLE;
            }
        }
        default:
            break;
    }
}




