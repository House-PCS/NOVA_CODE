/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_sgm58031.c
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
  * <tr><td>2024-09-09      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_sgm58031.h"


/*config寄存器默认值*/
static Sgm58031ConfigReg_t _gSgm58031ConfigReg = 
{
    .reg.comp_que       = EN_SGM58031_COMP_QUE_DISABLE,
    .reg.comp_lat       = EN_SGM58031_COMP_LAT_NON,
    .reg.comp_pol       = EN_SGM58031_COMP_POL_LOW,
    .reg.comp_mode      = EN_SGM58031_COMP_MODE_TRAD,
    .reg.dr             = EN_SGM58031_DR_50_60,  
    .reg.mode           = EN_SGM58031_MODE_SINGLE_MODE,
    .reg.pga            = EN_SGM58031_PGA_4096,
    .reg.mux            = EN_SGM58031_MUX_AIN0_GND,
    .reg.os             = EN_SGM58031_OS_NO_EFFECT,
};



/**
  * @brief   SGM58031寄存器写操作
  * @param   reg_addr：寄存器地址
  * @param   reg_value：寄存器值
  * @return  NULL
  */
static bool _reg_write(uint8_t reg_addr, uint16_t reg_value)
{
    uint8_t write_data[2];
    
    write_data[0] = (reg_value >> 8) & 0xff;
	write_data[1] = reg_value & 0xff;
    
    gSGMI2CHandle.pTxBuf = write_data;
    gSGMI2CHandle.reg_addr = reg_addr;
    gSGMI2CHandle.tx_size = 2;
    return SimI2CRegWrite(&gSGMI2CHandle);
}


/**
  * @brief   SGM58031寄存器读操作
  * @param   reg_addr：寄存器地址
  * @param   p_reg_value：回读寄存器值传递的指针
  * @return  NULL
  */
static bool _reg_read(uint8_t reg_addr, uint16_t *p_reg_value)
{
    uint8_t read_data[2];
    
    gSGMI2CHandle.pRxBuf = p_reg_value;
    gSGMI2CHandle.reg_addr = reg_addr;
    gSGMI2CHandle.rx_size = 2;
    
    if(SimI2CRegRead(&gSGMI2CHandle) == false)
    {
        return false;
    }
    *p_reg_value = (uint16_t)((read_data[0] << 8) | read_data[1]);
    return true;
}




/**
  * @brief   SGM58031初始化
  * @param   none
  * @return  NULL
  */
void DevSgm58031Init()
{
    gEepromSimI2CHandle.slave_addr0 = SGM_ADDR_GND;
    gEepromSimI2CHandle.reg_addr_width = EN_SIMI2C_REG_ADDR_WIDTH_8;
    SGMI2CBspInit();
    
    /*config默认值写入*/
    _reg_write(CONFIG_REG_ADDR, _gSgm58031ConfigReg.reg_value);
}



/**
  * @brief   SGM58031对应通道电压获取命令下发
  * @param   none
  * @return  命令下发是否生效
  */
bool DevSgm58031VoltGetCmd(EN_DEV_TEMP_DETECT_IDX ch)
{
    EN_SGM58031_MUX mux;
    /*通道选择*/
    switch(ch)
    {
        case EN_DEV_TEMP_DETECT_1:
        {
            mux = EN_SGM58031_MUX_AIN0_GND;
            break;
        }
        case EN_DEV_TEMP_DETECT_2:
        {
            mux = EN_SGM58031_MUX_AIN1_GND;
            break;
        }
        case EN_DEV_TEMP_DETECT_3:
        {
            mux = EN_SGM58031_MUX_AIN2_GND;
            break;
        }
        case EN_DEV_TEMP_DETECT_4:
        {
            mux = EN_SGM58031_MUX_AIN3_GND;
            break;
        }
        default:
            return false;
    }

    _gSgm58031ConfigReg.reg.mux = mux;
    _gSgm58031ConfigReg.reg.os = EN_SGM58031_OS_BEGIN_A_SINGLE_CONVERSION;
    return _reg_write(CONFIG_REG_ADDR, _gSgm58031ConfigReg.reg_value);
}


static uint16_t _pga_map[] = {6144, 4096, 2048, 1024, 512, 256};

/**
  * @brief   SGM58031对应通道电压获取
  * @param   ch:要回读的通道，
  * @param   p_value：回读到的值，只有当返回值为EN_SGM_OP_STATUS_COMPLETE，该值才有效
  * @return  通信状态
  */
EN_DEV_SGM_OP_STATUS DevSgm58031VoltGetStat(uint16_t *p_value)
{
    static EN_DEV_SGM_OP_STATUS stat = EN_DEV_SGM_OP_STATUS_BUSY;
    uint16_t cfg_rd_value, covert_value;
    
    if(_reg_read(CONFIG_REG_ADDR, &cfg_rd_value) == true)
    {
        if((cfg_rd_value >> 15 & 0x1) != 0)//OS状态，1表示转换完成
        {
            if(_reg_read(CONVERSION_REG_ADDR, &covert_value) == true)
            {
                *p_value = (uint32_t)covert_value * _pga_map[EN_SGM58031_PGA_4096] / 32768; //按照2^15次方，最高位为1表示负压，本驱动不考虑
                stat = EN_DEV_SGM_OP_STATUS_COMPLETE;
            }
            else
            {
                stat = EN_DEV_SGM_OP_STATUS_ERROR;
            }
        }
        else//OS为0，尚未转换完成
        {
            stat = EN_DEV_SGM_OP_STATUS_BUSY;
        }
    }
    else
    {
        stat = EN_DEV_SGM_OP_STATUS_ERROR;
    }
    return stat;
}









