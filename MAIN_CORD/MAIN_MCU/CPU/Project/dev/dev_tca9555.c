/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_tca9555.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-03-21
  * @brief       GD32F470
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-03-21    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_tca9555.h"

#define USE_SIMI2C      1


#if 1
static DevTca9555Config_t DevTca9555List[EN_TCA9555_INDEX_NUM] = {0};


/* Private constants ---------------------------------------------------------*/
static DevTca9555Config_t DevTca9555Config[EN_TCA9555_INDEX_NUM] =
{
#if USE_TCA9555_A
    {0, TCA9555_A_PIN_MODE, TCA9555_A_OUTPUT_DATA, TCA9555_A_INPUT_DATA},
#endif
#if USE_TCA9555_B
    {1, TCA9555_B_PIN_MODE, TCA9555_B_OUTPUT_DATA, TCA9555_B_INPUT_DATA},
#endif
};

#endif



static void _Tca9555I2cRegWrite(uint8_t reg_addr, uint8_t value)
{
#if  (USE_SIMI2C == 0)
    /*标准I2C*/
    TCA9555I2CHandle.pTxBuf = &value;
    TCA9555I2CHandle.tx_size = 1;
    TCA9555I2CHandle.reg_addr = reg_addr;

    I2CRegWrite(&TCA9555I2CHandle);

#else
    /*模拟I2C*/
    TCA9555I2CHandle.pTxBuf = &value;
    TCA9555I2CHandle.tx_size = 1;
    TCA9555I2CHandle.reg_addr = reg_addr;
    TCA9555I2CHandle.slave_addr0 = TCA9555_ADDR;

    SimI2CRegWrite(&TCA9555I2CHandle);
#endif

}

static uint8_t _Tca9555I2cRegRead(uint8_t reg_addr)
{
    uint8_t ret = 0;
#if  (USE_SIMI2C == 0)
    /*标准I2C*/
    TCA9555I2CHandle.pRxBuf = &ret;
    TCA9555I2CHandle.rx_size = 1;
    TCA9555I2CHandle.reg_addr = reg_addr;

    I2CRegRead(&RtcHandle);
#else
    /*模拟I2C*/
    TCA9555I2CHandle.pRxBuf = &ret;
    TCA9555I2CHandle.rx_size = 1;
    TCA9555I2CHandle.reg_addr = reg_addr;
    TCA9555I2CHandle.slave_addr0 = TCA9555_ADDR;

    SimI2CRegRead(&TCA9555I2CHandle);
#endif
    return ret;
}

int32_t DevTca9555DevConfig(void)
{
    TCA9555BspInit();

    /****默认输出都为输入****/
    uint8_t i = 0;
    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        DevTca9555SetPort0Mode(DevTca9555List[i].n_pin_mode);
        DevTca9555SetPort0Status(DevTca9555List[i].n_output_data);
        DevTca9555SetPort1Mode(DevTca9555List[i].n_pin_mode);
        DevTca9555SetPort1Status(DevTca9555List[i].n_output_data);
    }

    return 0;
}


/**********************************port0***********************************/
/**
  * @brief   Set port0 tca9555 GPIO mode
  * @param   dev: pointer to the tca9555 device
  * @param   mode_value: mode data   0-output,1-input
  * @return
  * - true
  * - false
  * @note    None
  *
  */
int32_t DevTca9555SetPort0Mode(uint8_t c_mode)
{
    uint32_t n_dev_id = 0;
    uint8_t i = 0;
    uint8_t c_mode_cfg = PORT0_MODE_CFG;

    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        n_dev_id = DevTca9555Config[i].n_dev_id;
    }

    DevTca9555List[n_dev_id].n_pin_mode = c_mode;

    _Tca9555I2cRegWrite(c_mode_cfg, c_mode);

    return 0;
}
/**
  * @brief   get port0 tca9555 GPIO status
  * @param   dev: pointer to the tca9555 device
  * @param   status_value: pointer to status_value
  * @return
  * - true
  * - false
  * @note    None
  *
  */
int32_t DevTca9555GetPort0Status(void)
{
    uint32_t n_dev_id = 0;
    uint8_t i = 0;
    uint8_t input_buf = 0;
    uint8_t c_input_cfg = PORT0_INPUT_REG;

    for(i = 0; i < EN_TCA9555_CH_NUM; ++i)
    {
        n_dev_id = DevTca9555Config[i].n_dev_id;
    }

    input_buf = _Tca9555I2cRegRead(c_input_cfg);

    DevTca9555Config[n_dev_id].n_input_data = input_buf;

    return 0;
}

/**
  * @brief   set port0 tca9555 GPIO output
  * @param   dev: pointer to the tca9555 device
  * @param   out_value: output level,0:LOW,1:HIGH
  * @return
  * - true
  * - false
  * @note    None
  *
  */
int32_t DevTca9555SetPort0Status(uint8_t c_value)
{
    uint8_t i = 0;
    uint32_t n_dev_id = 0;
    uint8_t c_output_cfg = PORT0_OUTPUT_REG;
    uint8_t c_output_data = 0;

    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        n_dev_id = DevTca9555List[i].n_dev_id;
    }

    DevTca9555List[n_dev_id].n_output_data = c_value;

    c_output_data = DevTca9555List[n_dev_id].n_output_data;

    _Tca9555I2cRegWrite(c_output_cfg, c_output_data);

    return 0;
}
/**
  * @brief   Set port0 single tca9555 GPIO mode
  * @param   dev: pointer to the tca9555 device
  * @param   ch:  channel number
  * @param   mode_value: mode data  0-output,1-input
  * @return
  * - true
  * - false
  * @note    None
  *
  */
int32_t DevTca9555SetPort0SinglePinMode(EN_TCA9555_CH ch, uint8_t c_mode)
{
    if(ch > 15)
    {
        return -1;
    }

    uint8_t i = 0;
    uint32_t n_dev_id;
    uint8_t c_mode_cfg = PORT0_MODE_CFG;
    uint8_t mode_data = 0 ;

    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        n_dev_id = DevTca9555List[i].n_dev_id;
    }

    //从tca9555中读取到的模式
    mode_data = _Tca9555I2cRegRead(c_mode_cfg);
    DevTca9555List[n_dev_id].n_pin_mode = c_mode;

    DevTca9555List[n_dev_id].n_pin_mode &= ~(0x01 << ch);
    DevTca9555List[n_dev_id].n_pin_mode |= c_mode << ch;

    mode_data = DevTca9555List[n_dev_id].n_pin_mode;

    _Tca9555I2cRegWrite(c_mode_cfg, mode_data);
    return 0;
}

/**
  * @brief   get port0 single tca9555 GPIO status
  * @param   dev: pointer to the tca9555 device
  * @param   status_value: pointer to status_value
  * @return
  * - true
  * - false
  * @note    None
  *
  */

int32_t DevTca9555GetPort0SinglePinStatus(EN_TCA9555_CH ch)
{
    if(ch > 15)
    {
        return -1;
    }

    uint32_t n_dev_id = 0;
    uint32_t n_ret = 0;
    uint8_t i = 0;
    uint8_t input_buf = 0;
    uint8_t c_input_cfg = PORT0_INPUT_REG;

    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        n_dev_id = DevTca9555List[i].n_dev_id;
    }

    input_buf = _Tca9555I2cRegRead(c_input_cfg);
    DevTca9555List[n_dev_id].n_input_data = input_buf;

    n_ret = (DevTca9555List[n_dev_id].n_input_data >> ch) & 0x01;

    return n_ret;;
}
/**
  * @brief   set port0 single tca9555 GPIO output
  * @param   dev: pointer to the tca9555 device
  * @param   ch:  channel number
  * @param   out_value: output level,0:LOW,1:HIGH
  * @return
  * - true
  * - false BITREVERSE
  * @note    None
  *
  */
int32_t DevTca9555SetPort0SinglePinStatus(EN_TCA9555_CH ch, uint8_t c_value)
{
    if(ch > 15)
    {
        return -1;
    }

    uint32_t n_dev_id = 0;
    uint8_t i = 0;
    uint8_t output_data = 0;
    uint8_t output_cfg = PORT0_OUTPUT_REG;

    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        n_dev_id = DevTca9555List[i].n_dev_id;
    }

    output_data = _Tca9555I2cRegRead(output_cfg);
    DevTca9555List[n_dev_id].n_output_data = output_data;

    DevTca9555List[n_dev_id].n_output_data &= ~(0x01 << ch);
    DevTca9555List[n_dev_id].n_output_data |= (c_value << ch);

    output_data = DevTca9555List[n_dev_id].n_output_data;

    _Tca9555I2cRegWrite(output_cfg, output_data);

    return 0;
}



/**********************************port1***********************************/
/**
  * @brief   Set port1 tca9555 GPIO mode
  * @param   dev: pointer to the tca9555 device
  * @param   mode_value: mode data   0-output,1-input
  * @return
  * - true
  * - false
  * @note    None
  *
  */
int32_t DevTca9555SetPort1Mode(uint8_t c_mode)
{
    uint32_t n_dev_id = 0;
    uint8_t i = 0;
    uint8_t c_mode_cfg = PORT1_MODE_CFG;

    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        n_dev_id = DevTca9555Config[i].n_dev_id;
    }

    DevTca9555List[n_dev_id].n_pin_mode = c_mode;

    _Tca9555I2cRegWrite(c_mode_cfg, c_mode);

    return 0;
}
/**
  * @brief   get port1 tca9555 GPIO status
  * @param   dev: pointer to the tca9555 device
  * @param   status_value: pointer to status_value
  * @return
  * - true
  * - false
  * @note    None
  *
  */
int32_t DevTca9555GetPort1Status(void)
{
    uint32_t n_dev_id = 0;
    uint8_t i = 0;
    uint8_t input_buf = 0;
    uint8_t c_input_cfg = PORT1_INPUT_REG;

    for(i = 0; i < EN_TCA9555_CH_NUM; ++i)
    {
        n_dev_id = DevTca9555Config[i].n_dev_id;
    }

    input_buf = _Tca9555I2cRegRead(c_input_cfg);

    DevTca9555Config[n_dev_id].n_input_data = input_buf;

    return 0;
}

/**
  * @brief   set port1 tca9555 GPIO output
  * @param   dev: pointer to the tca9555 device
  * @param   out_value: output level,0:LOW,1:HIGH
  * @return
  * - true
  * - false
  * @note    None
  *
  */
int32_t DevTca9555SetPort1Status(uint8_t c_value)
{
    uint8_t i = 0;
    uint32_t n_dev_id = 0;
    uint8_t c_output_cfg = PORT1_OUTPUT_REG;
    uint8_t c_output_data = 0;

    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        n_dev_id = DevTca9555List[i].n_dev_id;
    }

    DevTca9555List[n_dev_id].n_output_data = c_value;

    c_output_data = DevTca9555List[n_dev_id].n_output_data;

    _Tca9555I2cRegWrite(c_output_cfg, c_output_data);

    return 0;
}
/**
  * @brief   Set port1 single tca9555 GPIO mode
  * @param   dev: pointer to the tca9555 device
  * @param   ch:  channel number
  * @param   mode_value: mode data  0-output,1-input
  * @return
  * - true
  * - false
  * @note    None
  *
  */
int32_t DevTca9555SetPort1SinglePinMode(EN_TCA9555_CH ch, uint8_t c_mode)
{
    if(ch > 15)
    {
        return -1;
    }

    uint8_t i = 0;
    uint32_t n_dev_id;
    uint8_t c_mode_cfg = PORT1_MODE_CFG;
    uint8_t mode_data = 0 ;

    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        n_dev_id = DevTca9555List[i].n_dev_id;
    }

    //从tca9555中读取到的模式
    mode_data = _Tca9555I2cRegRead(c_mode_cfg);
    DevTca9555List[n_dev_id].n_pin_mode = c_mode;

    DevTca9555List[n_dev_id].n_pin_mode &= ~(0x01 << ch);
    DevTca9555List[n_dev_id].n_pin_mode |= c_mode << ch;

    mode_data = DevTca9555List[n_dev_id].n_pin_mode;

    _Tca9555I2cRegWrite(c_mode_cfg, mode_data);
    return 0;
}

/**
  * @brief   get port single tca9555 GPIO status
  * @param   dev: pointer to the tca9555 device
  * @param   status_value: pointer to status_value
  * @return
  * - true
  * - false
  * @note    None
  *
  */

int32_t DevTca9555GetPort1SinglePinStatus(EN_TCA9555_CH ch)
{
    if(ch > 15)
    {
        return -1;
    }

    uint32_t n_dev_id = 0;
    uint32_t n_ret = 0;
    uint8_t i = 0;
    uint8_t input_buf = 0;
    uint8_t c_input_cfg = PORT1_INPUT_REG;

    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        n_dev_id = DevTca9555List[i].n_dev_id;
    }

    input_buf = _Tca9555I2cRegRead(c_input_cfg);
    DevTca9555List[n_dev_id].n_input_data = input_buf;

    n_ret = (DevTca9555List[n_dev_id].n_input_data >> ch) & 0x01;

    return n_ret;
}

/**
  * @brief   set port1 single tca9555 GPIO output
  * @param   dev: pointer to the tca9555 device
  * @param   ch:  channel number
  * @param   out_value: output level,0:LOW,1:HIGH
  * @return
  * - true
  * - false BITREVERSE
  * @note    None
  *
  */
int32_t DevTca9555SetPort1SinglePinStatus(EN_TCA9555_CH ch, uint8_t c_value)
{
    if(ch > 15)
    {
        return -1;
    }

    uint32_t n_dev_id = 0;
    uint8_t i = 0;
    uint8_t output_data = 0;
    uint8_t output_cfg = PORT1_OUTPUT_REG;

    for(i = 0; i < EN_TCA9555_INDEX_NUM; ++i)
    {
        n_dev_id = DevTca9555List[i].n_dev_id;
    }

    output_data = _Tca9555I2cRegRead(output_cfg);
    DevTca9555List[n_dev_id].n_output_data = output_data;

    DevTca9555List[n_dev_id].n_output_data &= ~(0x01 << ch);
    DevTca9555List[n_dev_id].n_output_data |= (c_value << ch);

    output_data = DevTca9555List[n_dev_id].n_output_data;

    _Tca9555I2cRegWrite(output_cfg, output_data);

    return 0;
}
