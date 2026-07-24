/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-10
  * @brief       depend on dev_modbus_s
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
#include "dev_modbus_s.h"
#include "arc_detect_info.h"
#include "dev_cur_detect.h"
#include "alarm.h"
#include "arc_detect_threshold.h"
#include "arc_detect_process.h"
#include "dev_chan_sw.h"
#include "res_detect_process.h"
#include "proto_uniform.h"
#include "main.h"
#include "systick.h"
#include "dev_spiflash.h"
#include "dev_chan_sw.h"

ModbusReadReg_t Modbus_ReadReg;
ModbusReg_t Modbus_writeReg;
FlashSaveDateReg_t Flash_DateReg;

ModbuseReadTestReg_t ModbusTest_ReadReg;
ModbusTestReg_t ModbusTest_writeReg;

/* Private Variable -------------------------------------------------------------*/
extern DevCurDetect_t gDevCurDetect;
uint8_t gUpgradeRestartFlag;
static uint8_t sw_ver[3] = {0, 0, 20};
static uint8_t hd_ver[3] = {0, 1};
static ModbusReadRemainAndInput_t _ModbusRead;
//BmsSampleResType gBmsSampleRes;
static uint8_t _ModbusAckData[256];
static uint8_t _ModbusAckSize;
/* Private function  -----------------------------------------------*/
void _ModBusReadRegUpdate();
static uint16_t _ModBusRegUpdate(uint16_t addr, int16_t reg_value);
static void _ModbusReadInput(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num);
static void _ModbusReadRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_num);
static void _ModbusWriteSingleRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint16_t reg_data);
static void _ModbusWriteMultiRemain(EN_MODBUS_TYPE modbus_type, uint16_t reg_start_addr, uint8_t *p_reg_data, int16_t reg_num);
static uint32_t mod_data_send_wait_time = 0;
uint16_t gArcState();


uint8_t enter_test = 0;
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
    _ModBusReadRegUpdate();
    if(error == 0)
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x04;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体
        if(reg_start_addr <= 0x011F)
        {
            memcpy(&_ModbusAckData[offset + 3], (uint8_t*)&Modbus_ReadReg + ((reg_start_addr - MODEBUS_MODE_ID) * 2), reg_num * 2);
        }
        else
        {
            memcpy(&_ModbusAckData[offset + 3], (uint8_t*)&ModbusTest_ReadReg + ((reg_start_addr - EnterTestMode) * 2), reg_num * 2);
        }
        for (int i = 0; i < reg_num; i++)
        {
            temp = _ModbusAckData[offset + 3 + (2 * i)];
            _ModbusAckData[offset + 3 + 2 * i] = _ModbusAckData[offset + 3 + (2 * i + 1)];
            _ModbusAckData[offset + 3 + (2 * i + 1)] = temp;
        }
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
    _ModBusReadRegUpdate(); /* 读出寄存器值放入reg_value */

    if(error == 0 && (reg_start_addr >= MODEBUS_RESERVE14))
    {
        /*功能码*/
        _ModbusAckData[offset + 1] = 0x03;
        /*字节数*/
        _ModbusAckData[offset + 2] = reg_num * 2;
        /*寄存器对应的值*/
        //根据业务添加代码，可通过memcpy的方式，需要按照寄存器定义顺序定义结构体  Modbus_ReadReg

        if(reg_start_addr < 0x8000)
        {
            memcpy(&_ModbusAckData[offset + 3], (uint8_t*)&Modbus_writeReg + ((reg_start_addr - MODEBUS_RESERVE14) * 2), reg_num * 2);
        }
        else
        {
            memcpy(&_ModbusAckData[offset + 3], (uint8_t*)&ModbusTest_writeReg + ((reg_start_addr - 0x8000) * 2), reg_num * 2);
        }

        for (int i = 0; i < reg_num; i++)
        {
            temp = _ModbusAckData[offset + 3 + (2 * i)];
            _ModbusAckData[offset + 3 + 2 * i] = _ModbusAckData[offset + 3 + (2 * i + 1)];
            _ModbusAckData[offset + 3 + (2 * i + 1)] = temp;
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
            if(reg_start_addr < 0x8000)
            {
                *((uint16_t*)&Modbus_writeReg + reg_start_addr + i - MODEBUS_RESERVE14) = value;
            }
            else
            {
                *((uint16_t*)&ModbusTest_writeReg + reg_start_addr + i - 0x8000) = value;
            }
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
        if(reg_start_addr == 0x8000)
        {
            if(value)
            {
                enter_test = 1;
            }
        }
        if(enter_test)
        {
//            if(reg_start_addr == 0x8001)
//            {
//                if(value)
//                {
//                    Modbus_writeReg.arcdetectlevel = 1;
//                }
//            }
            if(reg_start_addr == 0x8002)
            {
                if(value)
                {
                    Modbus_writeReg.resdetectenable = 1;
//                    Modbus_writeReg.arcdetectlevel = 0;
//                    Modbus_writeReg.arcselftestcommand = 0xFF;
//                    channal_switch = 0;
                    Modbus_ReadReg.arcselftestresult = 0;
                    gBmsSampleRes[0].sample_over = 0;
                    gBmsSampleRes[1].sample_over = 0;
                    gBmsSampleRes[2].sample_over = 0;
                    gBmsSampleRes[3].sample_over = 0;
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

    memcpy(&thershold_info.arc_threshold1, &Modbus_writeReg.arcalarmthreshold1, sizeof(ModbusReg_t) - 6); //减去前三个保持寄存器
    ThresholdInfoWriteToFlash();
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
            if(tim_difftime_from_now(mod_data_send_wait_time) > 150)
            {
                sta = EN_MODBUS_REV_STATUS_ACK;
            }
            break;
        }
        case EN_MODBUS_REV_STATUS_ACK:
        {
            static uint32_t delay_time = 0;
            static uint8_t  cont = 0;
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
        default:
            break;
    }
}
/**
  * @brief   read register updates data for MODBUS
  * @param   None
  * @return  None
  * @note    None
  *
*/
void _ModBusReadRegUpdate()  //读寄存器值更新
{
//
    static uint8_t _data_version[6];
//
//    memcpy(&Modbus_ReadReg.mode_id, &gSysInfo.model_id, sizeof(SysInfo_t));

    //版本扩展号，后续根据flash读取值进行修改
//  Modbus_ReadReg.VersionExtend[0] = 1;
//  Modbus_ReadReg.VersionExtend[1] = 2;
//  Modbus_ReadReg.VersionExtend[2] = 3;
//  Modbus_ReadReg.VersionExtend[3] = 4;
//  Modbus_ReadReg.VersionExtend[4] = 5;
//  Modbus_ReadReg.VersionExtend[5] = 6;

    switch(channal_num)
    {
        case 3:
        {
//            Modbus_ReadReg.arcselftestresult =0;
            Modbus_ReadReg.channel4currentvalue = 0 ;//暂选1024中第一个值
            Modbus_ReadReg.channel4arcstrength = Arc_strength(DetectProcessHandle.FFT4_OutputMag_f32, DetectThreshold.Threshold4);
            Modbus_ReadReg.channel4arcIntensityhistoryvalue = Max_valuehistory(Modbus_ReadReg.channel4arcstrength, &Modbus_ReadReg.channel4arcstrength);
            Modbus_ReadReg.channel4inputvoltage = gBmsSampleRes[3].res_total_voltage;
            Modbus_ReadReg.channel4rx = gBmsSampleRes[3].res_p1;
            Modbus_ReadReg.channel4ry = gBmsSampleRes[3].res_n1;

            Modbus_ReadReg.channel3currentvalue = 0 ;//暂选1024中第一个值
            Modbus_ReadReg.channel3arcstrength = Arc_strength(DetectProcessHandle.FFT3_OutputMag_f32, DetectThreshold.Threshold3);
            Modbus_ReadReg.channel3arcIntensityhistoryvalue = Max_valuehistory(Modbus_ReadReg.channel3arcstrength, &Modbus_ReadReg.channel3arcstrength);
            Modbus_ReadReg.channel3inputvoltage = gBmsSampleRes[2].res_total_voltage;
            Modbus_ReadReg.channel3rx = gBmsSampleRes[2].res_p1;
            Modbus_ReadReg.channel3ry = gBmsSampleRes[2].res_n1;
        }
        case 1:
        {
            Modbus_ReadReg.channel2currentvalue = 0 ;//暂选1024中第一个值
            Modbus_ReadReg.channel2arcstrength = Arc_strength(DetectProcessHandle.FFT2_OutputMag_f32, DetectThreshold.Threshold2);
            Modbus_ReadReg.channel2arcIntensityhistoryvalue = Max_valuehistory(Modbus_ReadReg.channel2arcstrength, &Modbus_ReadReg.channel2arcstrength);
            Modbus_ReadReg.channel2inputvoltage = gBmsSampleRes[1].res_total_voltage;
            Modbus_ReadReg.channel2rx = gBmsSampleRes[1].res_p1;
            Modbus_ReadReg.channel2ry = gBmsSampleRes[1].res_n1;


            Modbus_ReadReg.channel1currentvalue = 0 ;//暂选1024中第一个值
            Modbus_ReadReg.channel1arcstrength = Arc_strength(DetectProcessHandle.FFT1_OutputMag_f32, DetectThreshold.Threshold1);
            Modbus_ReadReg.channel1arcIntensityhistoryvalue = Max_valuehistory(Modbus_ReadReg.channel1arcstrength, &Modbus_ReadReg.channel1arcstrength);
            Modbus_ReadReg.channel1inputvoltage = gBmsSampleRes[0].res_total_voltage;  //这块应该就不用算了
            Modbus_ReadReg.channel1rx = gBmsSampleRes[0].res_p1;
            Modbus_ReadReg.channel1ry = gBmsSampleRes[0].res_n1;
        }
        break;
        default:
            break;
    }


    Modbus_ReadReg.realtimearcalarminformation = gArcRealState();
    Modbus_ReadReg.arcalarminformation = gArcState();
    Modbus_ReadReg.impedancealarmInformation = gResRiskState(1);
    Modbus_ReadReg.impedanceBreakdownInformation = gResRiskState(0.1f);
    
    //工装写寄存器值拷贝至读寄存器
    ModbusTest_ReadReg.channal1_insulation_detect_result_volt = gBmsSampleRes[0].res_total_voltage;
    ModbusTest_ReadReg.channal2_insulation_detect_result_volt = gBmsSampleRes[1].res_total_voltage;
    ModbusTest_ReadReg.channal2_insulation_detect_result_p1 = gBmsSampleRes[1].res_p1;//暂不删除
    ModbusTest_ReadReg.channal2_insulation_detect_result_n1 = gBmsSampleRes[1].res_n1;//暂不删除
    ModbusTest_ReadReg.channal1_arc_detect_result = gDevCurDetect.arc_cur_data_1[0];//只拿首值？
    ModbusTest_ReadReg.channal2_arc_detect_result = gDevCurDetect.arc_cur_data_2[0];

    /* 写寄存器拷贝到读寄存器 */
    memcpy((uint8_t*)&Modbus_ReadReg.reserve14_15, (uint8_t*)&Modbus_writeReg.reserve21_22, sizeof(sys_thershold_info_t));

}


/**
  * @brief   write register updates data for MODBUS
  * @param   None
  * @return  None
  * @note    None
  *
*/



/**
  * @brief   Example Initialize the MODBUS register
  * @param   None
  * @return  None
  * @note    None
  *
*/
void ModbusRegInit(void)
{
    static uint8_t version_from_flash[12];
    memset(&Modbus_ReadReg.mode_id, 0, sizeof(ModbusReadReg_t)); //初始化，全部赋0

    gSysInfo.model_id = 0XC283;
    memset(gSysInfo.sn, 0x50, 20);
    //读取扩展版本号
    InternalFlashRead(ARC_FIRMWARE_ADDR + 11, version_from_flash, 12);
    for(uint8_t i = 0; i < 6; i++ )
    {
        Modbus_ReadReg.VersionExtend[i] = version_from_flash[2 * i] << 8 | version_from_flash[2 * i + 1];
    }
    gSysInfo.hw_version = (hd_ver[0] << 8) | hd_ver[1] ;
    gSysInfo.sw_version = (sw_ver[0] << 12) | (sw_ver[1] << 6) | sw_ver[2];
    memcpy((uint8_t *)&Modbus_ReadReg.mode_id, (uint8_t *)&gSysInfo.model_id, sizeof(SysInfo_t));
    memset(&Modbus_ReadReg.reserve1_2, 0, sizeof(ModbusReadReg_t) - 26);      //初始化，全部赋0
    memcpy((uint8_t*)&Modbus_writeReg.arcalarmthreshold1, (uint8_t*)&thershold_info.arc_threshold1, sizeof(sys_thershold_info_t));
    Modbus_writeReg.arcdetectlevel = 0;

}







