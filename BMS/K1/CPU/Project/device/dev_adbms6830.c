/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_adbms6830.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-8-20     <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_adbms6830.h" 
#include "systick.h"

#include "adBms6830CmdList.h"
#include "adBms6830GenericType.h"
#include "adBms6830Data.h"
#include "adBms6830ParseCreate.h"
#include "mcuWrapper.h"

#define LOG_TAG     "dev_adbms6830"


cell_asic IC[TOTAL_IC];


/* Set Under Voltage and Over Voltage Thresholds */
const float OV_THRESHOLD = 4.2;                 /* Volt */
const float UV_THRESHOLD = 2;                 /* Volt */
const uint16_t OWC_THRESHOLD  = 1500;        /*断线检测判断阈值电压，单位mV*/
const float OW_THRESHOLD_MAX = 0.95f;
const float OW_THRESHOLD_MIN = 0.60f;
    

static void _DevSpiAdbms6830CsLow(void);
static void _DevSpiAdbms6830CsHigh(void);
static void _DevSpiAdbms6830Read(uint16_t size, uint8_t *p_buf);
static void _DevSpiAdbms6830Write(uint16_t size, uint8_t *p_buf);
static void _DevSpiAdbms6830WriteRead(uint8_t *p_tx_buf, uint8_t *p_rx_buf, uint16_t size);




/** 
 * @brief    Adbms6830外设初始化
 * @param    NONE
 * @return   NULL
 * @note:
 */
void DevAdbms6830Init()
{
	AFEInComingApi_t api;
	
	api.p_delay_us = soft_delayus;
	api.p_delay_ms = soft_delayms;
	api.p_spi_cs_low = _DevSpiAdbms6830CsLow;
	api.p_spi_cs_high = _DevSpiAdbms6830CsHigh;
	api.p_spi_write_read = _DevSpiAdbms6830WriteRead;
	api.p_spi_read = _DevSpiAdbms6830Read;
	api.p_spi_write = _DevSpiAdbms6830Write;
	
	SpiAdbms6830BspInit();
	ADBMS6830LibInit(&api);
	
	for(uint8_t cic = 0; cic < TOTAL_IC; cic++)
	{
		/* Init config A */
        IC[cic].tx_cfga.refon = PWR_UP;
        IC[cic].tx_cfga.gpo = 0X3FF; /* All GPIO pull down off */
        IC[cic].tx_cfgb.dtmen = DTMEN_ON;
        /* Init config B */
        IC[cic].tx_cfgb.vov = SetOverVoltageThreshold(OV_THRESHOLD);
        IC[cic].tx_cfgb.vuv = SetUnderVoltageThreshold(UV_THRESHOLD);
//        IC[cic].tx_cfgb.dcc = ConfigB_DccBit(DCC16, DCC_BIT_SET);
        SetConfigB_DischargeTimeOutValue(TOTAL_IC, &IC[cic], RANG_0_TO_63_MIN, TIME_2MIN_OR_0_53HR);
	}
	adBmsWakeupIc(TOTAL_IC);
    spiSendCmd((uint8_t*)SRST);
//    LOG_INFO(LOG_TAG, "AFE Soft Reset done ok!");
    adBmsWakeupIc(TOTAL_IC);
    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGA, Config, A);
    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGB, Config, B);
    

}


/** 
 * @brief    Adbms6830 CS拉低
 * @param    NONE
 * @return   NULL
 * @note:
 */
static void _DevSpiAdbms6830CsLow()
{
	GpioBitReset((&gCsAdbms6830Handle));
}


/** 
 * @brief    Adbms6830 CS拉高
 * @param    NONE
 * @return   NULL
 * @note:
 */
static void _DevSpiAdbms6830CsHigh()
{
	GpioBitSet((&gCsAdbms6830Handle));
}



/** 
 * @brief    Adbms6830 SPI读
 * @param    size:要回读的数据长度
 * @param    p_buf:要回读的数据指针
 * @return   NULL
 * @note:
 */
static void _DevSpiAdbms6830Read(uint16_t size, uint8_t *p_buf)
{
	gSpiAdbms6830Handle.pRxBuf  = p_buf;
    gSpiAdbms6830Handle.rx_size = size;
    SpiRead8(&gSpiAdbms6830Handle);
}


/** 
 * @brief    Adbms6830 SPI写
 * @param    size:要写入的数据长度
 * @param    p_buf:要写入的数据指针
 * @return   NULL
 * @note:
 */
static void _DevSpiAdbms6830Write(uint16_t size, uint8_t *p_buf)
{
	gSpiAdbms6830Handle.pTxBuf = p_buf;
    gSpiAdbms6830Handle.tx_size = size;
    SpiWrite8(&gSpiAdbms6830Handle);
}


/** 
 * @brief    Adbms6830 SPI读写
 * @param    p_tx_buf:要写入的数据指针(命令)
 * @param    p_rx_buf:要回读的数据指针(数据)
 * @param    size:要回读的数据长度(写入长度固定为4)
 * @return   NULL
 * @note:
 */
static void _DevSpiAdbms6830WriteRead(uint8_t *p_tx_buf, uint8_t *p_rx_buf, uint16_t size)
{
	gSpiAdbms6830Handle.pTxBuf  = p_tx_buf;
    gSpiAdbms6830Handle.tx_size = 4;
    gSpiAdbms6830Handle.pRxBuf  = p_rx_buf;
    gSpiAdbms6830Handle.rx_size = size;

    SpiWrite8(&gSpiAdbms6830Handle);
    SpiRead8(&gSpiAdbms6830Handle);
}



/**
 *******************************************************************************
 * Function: getVoltage
 * @brief Get voltages with multiplication factor.
 *
 * @details This function calculate the voltage.
 *
 * Parameters:
 * @param [in]  data    voltages(uint16_t)
 *
 * @return voltage(float)
 *
 *******************************************************************************
*/

/** 
 * @brief    电压换算
 * @param    data:AFE读到的值
 * @return   将AFE读到的值转换成实际电压值，单位mV
 * @note:
 */
static uint16_t _GetMVoltage(int16_t data)
{
    float voltage_float; //voltage in Volts

    //Raw value above 0x7FFF till 0XFFF will be negative so 2's complement.
    if((uint16_t)data > 0x7FFF)
    {
        data = 1 + (~data); //Taking one's complement and then adding 1 to get two's complement
        voltage_float = ((10000.0f - data) * 0.150f);
    }
    else
    {
        voltage_float = ((data + 10000) * 0.150f);
    }
    return (uint16_t)voltage_float;
}

/** 
 * @brief    电压换算
 * @param    data:AFE读到的值
 * @return   将AFE读到的值转换成实际电压值，单位V
 * @note:
 */
static float _GetVoltage(int16_t data)
{
    float voltage_float; //voltage in Volts

    //Raw value above 0x7FFF till 0XFFF will be negative so 2's complement.
    if(data > 0x7FFF)
    {
        data = 1 + (~data); //Taking one's complement and then adding 1 to get two's complement
        voltage_float = ((10000.0 - data) * 0.000150);
    }
    else
    {
        voltage_float = ((data + 10000) * 0.000150);
    }
    return voltage_float;
}


/** 
 * @brief    Cell open ow_even and ow_odd volatge collect
 * @param    tIC:AFE数量
 * @param    ic:全局数据结构体
 * @param    type:断线检测方式
 * @param    ow_c_s:奇偶通道标识
 * @return   NONE
 * @note:
 */
static void _ADBMS6830CellOWVoltageCollect(uint8_t tIC, cell_asic *ic, TYPE type, OW_C_S ow_c_s)
{
    if(type == Cell)
    {
        if(ow_c_s == OW_OFF_ALL_CH)
        {
            for (uint8_t cic = 0; cic < tIC; cic++)
            {
                for (uint8_t cell = 0; cell < CELL; cell++)
                {
                    ic[cic].owcell.cell_ow_ref[cell] = ic[cic].cell.c_codes[cell];
                }
            }
        }
        else if(ow_c_s == OW_ON_EVEN_CH)
        {
            for (uint8_t cic = 0; cic < tIC; cic++)
            {
                for (uint8_t cell = 0; cell < CELL; cell++)
                {
                    ic[cic].owcell.cell_ow_even[cell] = ic[cic].cell.c_codes[cell];
                }
            }
        }
        else if(ow_c_s == OW_ON_EVEN_CH)
        {
            for (uint8_t cic = 0; cic < tIC; cic++)
            {
                for (uint8_t cell = 0; cell < CELL ; cell++)
                {
                    ic[cic].owcell.cell_ow_odd[cell] = ic[cic].cell.c_codes[cell];
                }
            }
        }
    }
    else if(type == S_volt)
    {
        if(ow_c_s == OW_OFF_ALL_CH)
        {
            for (uint8_t cic = 0; cic < tIC; cic++)
            {
                for (uint8_t svolt = 0; svolt < CELL; svolt++)
                {
                    ic[cic].owcell.cell_ow_ref[svolt] = ic[cic].scell.sc_codes[svolt];
                }
            }
        }
        else if(ow_c_s == OW_ON_EVEN_CH)
        {
            for (uint8_t cic = 0; cic < tIC; cic++)
            {
                for (uint8_t svolt = 0; svolt < CELL; svolt++)
                {
                    ic[cic].owcell.cell_ow_even[svolt] = ic[cic].scell.sc_codes[svolt];
                }
            }
        }
        else
        {
            for (uint8_t cic = 0; cic < tIC; cic++)
            {
                for (uint8_t svolt = 0; svolt < CELL; svolt++)
                {
                    ic[cic].owcell.cell_ow_odd[svolt] = ic[cic].scell.sc_codes[svolt];
                }
            }
        }
    }
}


/** 
 * @brief    Open wire detection condtion check
 * @param    tIC:AFE数量
 * @param    ic:全局数据结构体
 * @param    type:断线检测方式
 * @return   NONE
 * @note:
 */
static void ADBMS6830OpenWireDetectionConditionCheck(uint8_t tIC, cell_asic *ic, TYPE type)
{
    if(type == Cell)
    {
        for (uint8_t cic = 0; cic < tIC; cic++)
        {
            for (uint8_t cell = 0; cell < CELL; cell++)
            {
                if((ic[cic].owcell.cell_ow_even[cell] - ic[cic].owcell.cell_ow_odd[cell]) > OWC_THRESHOLD)
                {
                    ic[cic].diag_result.cell_ow[cell] = 1; /* Cell open */
                }
                else
                {
                    ic[cic].diag_result.cell_ow[cell] = 0; /* Cell close */
                }
            }
        }
    }
    else if(type == S_volt)
    {
        for (uint8_t cic = 0; cic < tIC; cic++)
        {
            for (uint8_t cell = 0; cell < CELL; cell++)
            {
                if((ic[cic].owcell.cell_ow_even[cell] - ic[cic].owcell.cell_ow_odd[cell]) > 2000)
                {
                    ic[cic].diag_result.cellred_ow[cell] = 1; /* Cell open */
                }
                else
                {
                    ic[cic].diag_result.cellred_ow[cell] = 0; /* Cell close */
                }
            }
        }
    }
    else if(type == Aux)
    {
        for (uint8_t cic = 0; cic < tIC; cic++)
        {
            for (uint8_t gpio = 0; gpio < (AUX - 2); gpio++)
            {
                if((ic[cic].gpio.aux_pup_up[gpio] - ic[cic].gpio.aux_pup_down[gpio]) > 50000)
                {
                    ic[cic].diag_result.aux_ow[gpio] = 1; /* GPIO open */
                }
                else
                {
                    ic[cic].diag_result.aux_ow[gpio] = 0; /* GPIO close */
                }
            }
        }
    }
    else
    {
//        LOG_ERROR(LOG_TAG, "Wrong Resistor Type Selected\n");
    }
}

/** 
 * @brief    Open wire detection condtion check优化后的
 * @param    tIC:AFE数量
 * @param    ic:全局数据结构体
 * @param    type:断线检测方式
 * @return   NONE
 * @note:
 */
static void ADBMS6830OpenWireDetectionConditionCheck2(uint8_t tIC, cell_asic *ic, TYPE type)
{
    if(type == Cell)
    {
        for (uint8_t cic = 0; cic < tIC; cic++)
        {
            for (uint8_t cell = 0; cell < CELL - 3; cell += 2)
            {
                //偶数通道
                if(abs(ic[cic].owcell.cell_ow_even[cell + 3] - ic[cic].owcell.cell_ow_even[cell + 1]) > OWC_THRESHOLD)
                {
                    ic[cic].diag_result.cell_ow[cell + 3] = 1; /* Cell open */
                }
                else
                {
                    ic[cic].diag_result.cell_ow[cell + 3] = 0; /* Cell close */
                }
                //奇数通道
                if(abs(ic[cic].owcell.cell_ow_odd[cell + 2] - ic[cic].owcell.cell_ow_odd[cell]) > OWC_THRESHOLD)
                {
                    ic[cic].diag_result.cell_ow[cell + 2] = 1; /* Cell open */
                }
                else
                {
                    ic[cic].diag_result.cell_ow[cell + 2] = 0; /* Cell close */
                }
            }
        }
    }
    else if(type == S_volt)
    {
        for (uint8_t cic = 0; cic < tIC; cic++)
        {
            for (uint8_t cell = 0; cell < CELL; cell++)
            {
                if((ic[cic].owcell.cell_ow_even[cell] - ic[cic].owcell.cell_ow_odd[cell]) > 2000)
                {
                    ic[cic].diag_result.cellred_ow[cell] = 1; /* Cell open */
                }
                else
                {
                    ic[cic].diag_result.cellred_ow[cell] = 0; /* Cell close */
                }
            }
        }
    }
    else if(type == Aux)
    {
        for (uint8_t cic = 0; cic < tIC; cic++)
        {
            for (uint8_t gpio = 0; gpio < (AUX - 2); gpio++)
            {
                if((ic[cic].gpio.aux_pup_up[gpio] - ic[cic].gpio.aux_pup_down[gpio]) > 50000)
                {
                    ic[cic].diag_result.aux_ow[gpio] = 1; /* GPIO open */
                }
                else
                {
                    ic[cic].diag_result.aux_ow[gpio] = 0; /* GPIO close */
                }
            }
        }
    }
    else
    {
//        LOG_ERROR(LOG_TAG, "Wrong Resistor Type Selected\n");
    }
}


/** 
 * @brief    获取断线检测的数值
 * @param    NONE
 * @return   NONE
 * @note:
 */
float gOwValArr[CELL];
void GetOwVal(uint16_t *p_temp)
{
    for(int i=0; i<CELL; i++)
    {
        p_temp[i] = (uint16_t)(gOwValArr[i] * 100);
    }
}


/** 
 * @brief    Open wire detection condtion check-采用参考电压法
 * @param    tIC:AFE数量
 * @param    ic:全局数据结构体
 * @param    type:断线检测方式
 * @return   NONE
 * @note:
 */
static void ADBMS6830OpenWireDetectionConditionCheck3(uint8_t tIC, cell_asic *ic, TYPE type)
{
    memset(gOwValArr,0,CELL);
    int16_t ref_voltage[CELL];
    for (uint8_t cic = 0; cic < tIC; cic++)
    {
        for (uint8_t cell = 0; cell < CELL - 1; cell += 2)
        {
            //奇数通道
            ref_voltage[cell] = _GetMVoltage(ic[cic].owcell.cell_ow_ref[cell]);
            gOwValArr[cell] = ((float)_GetMVoltage(ic[cic].owcell.cell_ow_odd[cell]))/_GetMVoltage(ic[cic].owcell.cell_ow_even[cell]);
            if(gOwValArr[cell] < OW_THRESHOLD_MIN || gOwValArr[cell] > OW_THRESHOLD_MAX)
            {
                ic[cic].diag_result.cell_ow[cell] = 1; /* Cell open */
            }
            else if(ref_voltage[cell] < 1000)
            {
                ic[cic].diag_result.cell_ow[cell + 1] = 1; /* Cell open */
            }
            else
            {
                ic[cic].diag_result.cell_ow[cell] = 0; /* Cell close */
            }
            //偶数通道
            ref_voltage[cell + 1] = _GetMVoltage(ic[cic].owcell.cell_ow_ref[cell + 1]);
            gOwValArr[cell + 1] = ((float)_GetMVoltage(ic[cic].owcell.cell_ow_even[cell + 1]))/_GetMVoltage(ic[cic].owcell.cell_ow_ref[cell + 1]);
            if(gOwValArr[cell + 1] < OW_THRESHOLD_MIN || gOwValArr[cell + 1] > OW_THRESHOLD_MAX)
            {
                ic[cic].diag_result.cell_ow[cell + 1] = 1; /* Cell open */
            }
            else if(ref_voltage[cell + 1] < 1000)
            {
                ic[cic].diag_result.cell_ow[cell + 1] = 1; /* Cell open */
            }
            else
            {
                ic[cic].diag_result.cell_ow[cell + 1] = 0; /* Cell close */
            }
        }
    }
}









/** 
 * @brief    AFE电压采样命令
 * @param    NONE
 * @return   NULL
 * @note:
 */
void DevAFEVoltageSampleCmd()
{
	adBmsWakeupIc(TOTAL_IC);
    adBms6830_Adcv(RD_OFF, SINGLE, DCP_OFF, RSTF_OFF, OW_OFF_ALL_CH);
	adBmsPollAdc((uint8_t*)PLADC);
}



/** 
 * @brief    AFE电压采样状态获取(所有AFE都采样)
 * @param    p_voltage：各通道采样到的电压，单位mV，数量要按照AFE拥有的通道数*AFE数量定义，如果实际使用少于最大通道数，则部分数据不使用即可
 * @return   EN_AFE_COMM_STATUS:采样状态
 * @note:    必须在执行完DevAFEVoltageSampleCmd后执行
 */
EN_AFE_COMM_STATUS DevAFEVoltageSampleStat(uint16_t *p_voltage)
{
    uint8_t read_data = 0x00;
    
	_DevSpiAdbms6830Read(1, &read_data);
	if(read_data == 0xFF)
	{
		uint8_t i = 0;
		uint8_t j=0;
		
		adBmsCsHigh();
		
		adBmsWakeupIc(TOTAL_IC);
		adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVA, Cell, A);
		adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVB, Cell, B);
		adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVC, Cell, C);
		adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVD, Cell, D);
		adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVE, Cell, E);
		adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVF, Cell, F);
		
		for(i = 0; i < TOTAL_IC; i++)
		{
			if(IC[i].cccrc.cell_pec)
			{
				return EN_AFE_COMM_STATUS_ERROR;
			}
		}
		
		/*向形参赋值采样到的结果数据*/
		for(i = 0;i < TOTAL_IC; i++)
		{
			for(j = 0; j < CELL; j ++)
			{
				p_voltage[i * CELL + j] = _GetMVoltage(IC[i].cell.c_codes[j]);
			}
		}
		
		return EN_AFE_COMM_STATUS_COMPLETE;
	}
	else
	{
		return EN_AFE_COMM_STATUS_BUSY;
	}
}


/** 
 * @brief    AFE控制GPIO设定值修改,不下发给AFE，通过DevAFEGpioIOCtl下发生效
 * @param    cic:AFE芯片序号
 * @param    gpiox:要控制的GPIO序号，可以同时控制多个
 * @param    value:0表示置低，1表示置高
 * @return   NULL
 * @note:
 */
void DevAFEGpioIOValueCfg(uint8_t cic, EN_AFE_GPIO gpiox, uint8_t value)
{
    if(cic >= TOTAL_IC)
    {
        return;
    }
    
    IC[cic].tx_cfga.refon = PWR_UP;
    if(value == 0)
    {
        IC[cic].tx_cfga.gpo &= (~gpiox);
    }
    else
    {
        IC[cic].tx_cfga.gpo |= gpiox;
    }
}

/** 
 * @brief    AFE控制GPIO输出,配合DevAFEGpioIOValueCfg使用
 * @param    gpiox:要控制的GPIO序号，可以同时控制多个
 * @param    value:0表示置低，1表示置高
 * @return   true表示下发生效，false表示未生效
 * @note:   可多次调用配合DevAFEGpioIOValueCfg使用配置多个IC多个IO的不同状态，通过DevAFEGpioIOCtl一次下发
 */
bool DevAFEGpioIOCtl()
{
    uint8_t i = 0;
    
    adBmsWakeupIc(TOTAL_IC);
    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGA, Config, A);
    adBmsReadData(TOTAL_IC, &IC[0], RDCFGA, Config, A);
    
    for(i = 0; i < TOTAL_IC; i++)
    {
        if(IC[i].tx_cfga.gpo != IC[i].rx_cfga.gpo)
        {
            break;
        }
    }
    if(i == TOTAL_IC)
    {
        return true;
    }
    else
    {
        return false;
    }
}



/** 
 * @brief    AFE AUX电压采样命令
 * @param    gpiox:要采样的GPIO
 * @return   NULL
 * @note:    一次只能采一个通道，或者全采
 */
void DevAFEAuxVoltageSampleCmd(EN_AFE_GPIO gpiox)
{
    for(uint8_t cic = 0; cic < TOTAL_IC; cic++)
    {
        IC[cic].tx_cfga.refon = PWR_UP;
    }
    adBmsWakeupIc(TOTAL_IC);
    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGA, Config, A);
    
    if(gpiox == EN_AFE_GPIO_ALL)
    {
        adBms6830_Adax(AUX_OW_OFF, PUP_DOWN, AUX_ALL);
    }
    else
    {
        uint8_t cnt = 0;
        while(((gpiox >> cnt) & 0x01) != 1)
        {
            cnt++;
            if(cnt >= 10)
            {
                return;
            }
        }
        adBms6830_Adax(AUX_OW_OFF, PUP_DOWN, (CH)(cnt + 1));
        adBmsPollAdc(PLADC);
//        adBmsPollAdc(PLAUX1);
    }
}



/** 
 * @brief    AFE AUX电压采样状态获取(所有AFE同时获取)
 * @param    p_voltage：对应通道采样到的电压，单位mV，如果采单个通道，则该指针指向单个数据(多IC的话是多个数据)，如果全采，该指针指向一个包含10个元素的数组(多IC的话就是多个数组)
 * @return   EN_AFE_COMM_STATUS:采样状态
 * @note:    必须在执行完DevAFEAuxVoltageSampleCmd后执行
 */
EN_AFE_COMM_STATUS DevAFEAuxVoltageSampleStat(uint16_t *p_voltage, EN_AFE_GPIO gpiox)
{
    uint8_t read_data = 0x00;
    
	_DevSpiAdbms6830Read(1, &read_data);
    if(read_data == 0xFF)
    {
        adBmsCsHigh();
        adBmsWakeupIc(TOTAL_IC);
        
        if(gpiox == EN_AFE_GPIO_ALL)
        {
            adBmsReadData(TOTAL_IC, &IC[0], RDAUXA, Aux, A);
            adBmsReadData(TOTAL_IC, &IC[0], RDAUXB, Aux, B);
            adBmsReadData(TOTAL_IC, &IC[0], RDAUXC, Aux, C);
            adBmsReadData(TOTAL_IC, &IC[0], RDAUXD, Aux, D);
            for(uint8_t i = 0; i < TOTAL_IC; i++)
            {
                for(uint8_t j = 0; j < 10; j++)
                {
                    p_voltage[i * 10 + j] = _GetMVoltage(IC[i].aux.a_codes[j]);
                }
            }
            return EN_AFE_COMM_STATUS_COMPLETE;
        }
        else if(gpiox == EN_AFE_GPIO_1 || gpiox == EN_AFE_GPIO_2 || gpiox == EN_AFE_GPIO_3)
        {
            adBmsReadData(TOTAL_IC, &IC[0], RDAUXA, Aux, A);
            for(uint8_t i = 0; i < TOTAL_IC; i++)
            {
                if(gpiox == EN_AFE_GPIO_1)
                {
                    p_voltage[i] = _GetMVoltage(IC[i].aux.a_codes[0]);
                }
                else if(gpiox == EN_AFE_GPIO_2)
                {
                    p_voltage[i] = _GetMVoltage(IC[i].aux.a_codes[1]);
                }
                else if(gpiox == EN_AFE_GPIO_3)
                {
                    p_voltage[i] = _GetMVoltage(IC[i].aux.a_codes[2]);
                }
            }
            return EN_AFE_COMM_STATUS_COMPLETE;
        }
        else if(gpiox == EN_AFE_GPIO_4 || gpiox == EN_AFE_GPIO_5 || gpiox == EN_AFE_GPIO_6)
        {
            adBmsReadData(TOTAL_IC, &IC[0], RDAUXB, Aux, B);
            for(uint8_t i = 0; i < TOTAL_IC; i++)
            {
                if(gpiox == EN_AFE_GPIO_4)
                {
                    p_voltage[i] = _GetMVoltage(IC[i].aux.a_codes[3]);
                }
                else if(gpiox == EN_AFE_GPIO_5)
                {
                    p_voltage[i] = _GetMVoltage(IC[i].aux.a_codes[4]);
                }
                else if(gpiox == EN_AFE_GPIO_6)
                {
                    p_voltage[i] = _GetMVoltage(IC[i].aux.a_codes[5]);
                }
            }
            return EN_AFE_COMM_STATUS_COMPLETE;
        }
        else if(gpiox == EN_AFE_GPIO_7 || gpiox == EN_AFE_GPIO_8 || gpiox == EN_AFE_GPIO_9)
        {
            adBmsReadData(TOTAL_IC, &IC[0], RDAUXC, Aux, C);
            for(uint8_t i = 0; i < TOTAL_IC; i++)
            {
                if(gpiox == EN_AFE_GPIO_7)
                {
                    p_voltage[i] = _GetMVoltage(IC[i].aux.a_codes[6]);
                }
                else if(gpiox == EN_AFE_GPIO_8)
                {
                    p_voltage[i] = _GetMVoltage(IC[i].aux.a_codes[7]);
                }
                else if(gpiox == EN_AFE_GPIO_9)
                {
                    p_voltage[i] = _GetMVoltage(IC[i].aux.a_codes[8]);
                }
            }
            return EN_AFE_COMM_STATUS_COMPLETE;
        }
        else if(gpiox == EN_AFE_GPIO_10)
        {
            adBmsReadData(TOTAL_IC, &IC[0], RDAUXD, Aux, D);
            for(uint8_t i = 0; i < TOTAL_IC; i++)
            {
                p_voltage[i] = _GetMVoltage(IC[i].aux.a_codes[10]);
            }
            return EN_AFE_COMM_STATUS_COMPLETE;
        }
        else
        {
            return EN_AFE_COMM_STATUS_ERROR;
        }
        
        //
    }
    else
	{
		return EN_AFE_COMM_STATUS_BUSY;
	}
}


/** 
 * @brief    AFE均衡控制开启
 * @param    p_dcc:指向表示均衡对应通道是否开启的数组,数组下标代表AFE数量,如果只有一个AFE，则该数组只有一个元素
 * @param    p_rd_dcc：AFE当前回读到的dcc状态
 * @return   true表示下发生效，false表示未生效
 * @note:
 */
bool DevAFEBalanceStart(uint16_t *p_dcc, uint16_t *p_rd_dcc)
{ 
    uint8_t i = 0;
    uint32_t duty_cycle = PWM_46_2_PCT;
//    uint8_t j = 0;
//    /*增加对于相邻通道是否同时开启的判断*/
//    uint8_t current_ch_flag = 0;//用于记录当前通道是否开启状态，下一个通道基于前一个通道判断
//    for(i = 0; i < TOTAL_IC; i++)
//    {
//        for(j = 0; j < CELL; j++)//按照芯片拥有的通道数，而不是实际使用的通道数
//        {
//            if(((p_dcc[i] >> j) & 0x01) == 0x01)
//            {
//                if(current_ch_flag == 1)
//                {
//                    return false;
//                }
//                current_ch_flag = 1;
//            }
//            else
//            {
//                current_ch_flag = 0;
//            }
//        }
//    }
    
    /*如无相邻通道同时开启情况，则正常执行*/
    for(i = 0; i < TOTAL_IC; i++)
    {
        IC[i].tx_cfgb.dcc = p_dcc[i];
    }    
    adBmsWakeupIc(TOTAL_IC);
    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGB, Config, B);
//    SetPwmDutyCycle(TOTAL_IC, &IC[0], duty_cycle);
//    adBmsWriteData(TOTAL_IC, &IC[0], WRPWM1, Pwm, A);
//    adBmsWriteData(TOTAL_IC, &IC[0], WRPWM2, Pwm, B);
//    adBmsReadData(TOTAL_IC, &IC[0], RDPWM1, Pwm, A);
//    adBmsReadData(TOTAL_IC, &IC[0], RDPWM2, Pwm, B);
    adBmsReadData(TOTAL_IC, &IC[0], RDCFGB, Config, B);
    
    for(i = 0; i < TOTAL_IC; i++)
    {
        p_rd_dcc[i] = IC[i].rx_cfgb.dcc;
    }
//    return true;
    
    for(i = 0; i < TOTAL_IC; i++)
    {
        if(IC[i].tx_cfgb.dcc != IC[i].rx_cfgb.dcc)
        {
            break;
        }
    }
    if(i == TOTAL_IC)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/** 
* @brief    AFE均衡关闭
* @param    p_rd_dcc：AFE当前回读到的dcc状态
* @return   true表示下发生效，false表示未生效
* @note:
*/
bool DevAFEBalanceStop(uint16_t *p_rd_dcc)
{
//    uint8_t i = 0;
//    for(i = 0; i < TOTAL_IC; i++)
//    {
//        IC[i].tx_cfgb.dcc = 0;
//    } 
//    adBmsWakeupIc(TOTAL_IC);
//    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGB, Config, B);
//    adBmsReadData(TOTAL_IC, &IC[0], RDCFGB, Config, B);
//    
//    for(i = 0; i < TOTAL_IC; i++)
//    {
//        p_rd_dcc[i] = IC[i].rx_cfgb.dcc;
//    }
//    
//    for(i = 0; i < TOTAL_IC; i++)
//    {
//        if(IC[i].tx_cfgb.dcc != IC[i].rx_cfgb.dcc)
//        {
//            break;
//        }
//    }
//    if(i == TOTAL_IC)
//    {
//        return true;
//    }
//    else
//    {
//        return false;
//    }
    IC[0].tx_cfgb.dcc = 0;
    adBmsWakeupIc(TOTAL_IC);

    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGB, Config, B);
    
    SetPwmDutyCycle(TOTAL_IC, &IC[0], PWM_0_0_PCT);
    adBmsWriteData(TOTAL_IC, &IC[0], WRPWM1, Pwm, A);
    adBmsWriteData(TOTAL_IC, &IC[0], WRPWM2, Pwm, B);
    return true;
}


/** 
* @brief    AFE断线检测准备
* @param    NONE
* @return   true表示可以正常执行，false表示当前不能进行断线检测
* @note:
*/
bool DevAFEWireCheckPrep()
{
    adBmsWakeupIc(TOTAL_IC);
    adBmsReadData(TOTAL_IC, &IC[0], RDCFGB, Config, B);
	for(uint8_t cic = 0; cic < TOTAL_IC; cic++)
	{
		if(IC[cic].rx_cfgb.dcc != 0)
        {
            return false; //有均衡动作时不能进行断线检测
        }
	}
    for(uint8_t cic = 0; cic < TOTAL_IC; cic++)
    {
        /* Init config A */
        IC[cic].tx_cfga.refon = PWR_UP;
        IC[cic].tx_cfga.soakon = SOAKON_CLR;   /* Set or clear if required */
        IC[cic].tx_cfga.owrng = SHORT;         /* Set or clear if required */
    }
    spiSendCmd(SRST);
    adBmsWakeupIc(TOTAL_IC);
    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGA, Config, A);
	return true;
}



/** 
 * @brief    AFE断线检测命令
 * @param    ch:0:表示不选择通道，采样参考电压，1表示偶通道，2表示奇通道
 * @param    type:0-Cell模式，1-S-Volt模式 
 * @return   NULL
 * @note:    需要在调用该接口之前，先调用DevAFEWireCheckPrep，且确保返回true
 */
void DevAFEWireCheckCmd(uint8_t ch, uint8_t type)
{
    OW_C_S ow_ch;
    if(ch == 0)
    {
        ow_ch = OW_OFF_ALL_CH;
    }
    else if(ch == 1)
    {
        ow_ch = OW_ON_EVEN_CH;
    }
    else
    {
        ow_ch = OW_ON_ODD_CH;
    }
    
    if(type == 0)
    {
        adBms6830_Adcv(RD_OFF, SINGLE, DCP_OFF, RSTF_OFF, ow_ch);
    }
    else
    {
        adBms6830_Adsv(SINGLE, DCP_OFF, ow_ch);
    }
    adBmsPollAdc((uint8_t*)PLADC);
}


/** 
 * @brief    AFE断线检测状态获取(所有AFE同时获取)
 * @param    ch:0:表示不选择通道，采样参考电压，1表示偶通道，2表示奇通道
 * @param    type:0-Cell模式，1-S-Volt模式 
 * @return   EN_AFE_COMM_STATUS:检测状态
 * @note:    必须在执行完DevAFEWireCheckCmd后执行
 */
EN_AFE_COMM_STATUS DevAFEWireCheckStat(uint8_t ch, uint8_t type)
{
    uint8_t read_data = 0x00;
    OW_C_S ow_ch;
    
    if(ch == 0)
    {
        ow_ch = OW_OFF_ALL_CH;
    }
    else if(ch == 1)
    {
        ow_ch = OW_ON_EVEN_CH;
    }
    else
    {
        ow_ch = OW_ON_ODD_CH;
    }
    
	_DevSpiAdbms6830Read(1, &read_data);
    if(read_data == 0xFF)
    {
        if(type == 0)
        {
            adBmsReadData(TOTAL_IC, &IC[0], RDCVA, Cell, A);
            adBmsReadData(TOTAL_IC, &IC[0], RDCVB, Cell, B);
            adBmsReadData(TOTAL_IC, &IC[0], RDCVC, Cell, C);
            adBmsReadData(TOTAL_IC, &IC[0], RDCVD, Cell, D);
            adBmsReadData(TOTAL_IC, &IC[0], RDCVE, Cell, E);
            adBmsReadData(TOTAL_IC, &IC[0], RDCVF, Cell, F);
            _ADBMS6830CellOWVoltageCollect(TOTAL_IC, &IC[0], Cell, ow_ch);
        }
        else
        {
            adBmsReadData(TOTAL_IC, &IC[0], RDSVA, S_volt, A);
            adBmsReadData(TOTAL_IC, &IC[0], RDSVB, S_volt, B);
            adBmsReadData(TOTAL_IC, &IC[0], RDSVC, S_volt, C);
            adBmsReadData(TOTAL_IC, &IC[0], RDSVD, S_volt, D);
            adBmsReadData(TOTAL_IC, &IC[0], RDSVE, S_volt, E);
            adBmsReadData(TOTAL_IC, &IC[0], RDSVF, S_volt, F);
            _ADBMS6830CellOWVoltageCollect(TOTAL_IC, &IC[0], S_volt, ow_ch);
        }
        return EN_AFE_COMM_STATUS_COMPLETE;
    }
    else
    {
        return EN_AFE_COMM_STATUS_BUSY;
    }
}


/** 
 * @brief    AFE断线结果获取(所有AFE同时获取)
 * @param    p_res：结果输出，需要传入一个下标为AFE数量的数组，每个成员代表一个AFE的检测结果，每个bit代表一个通道
 * @param    type:0-Cell模式，1-S-Volt模式 
 * @return   EN_AFE_COMM_STATUS:检测状态
 * @note:    必须在执行完DevAFEWireCheckCmd和DevAFEWireCheckStat后执行，当前仅支持了电压检测通道的断线检测
 */
void DevAFEWireCheckResultGet(uint16_t *p_res, uint8_t type)
{
//    if(type == 0)
//    {
//        ADBMS6830OpenWireDetectionConditionCheck2(TOTAL_IC, &IC[0], Cell);//采用Cell模式
//    }
//    else
//    {
//        ADBMS6830OpenWireDetectionConditionCheck2(TOTAL_IC, &IC[0], S_volt);//采用S_volt模式
//    }
    
    ADBMS6830OpenWireDetectionConditionCheck3(TOTAL_IC, &IC[0], S_volt);
    
    for(uint8_t i = 0; i < TOTAL_IC; i++)
    {
        p_res[i] = 0;
        for(uint8_t j = 0; j < CELL; j++)
        {
            if(IC[i].diag_result.cell_ow[j] == 1)
            {
                p_res[i] |= (1 << j);
            }
        }
    }
}

/** 
 * @brief    AFE可变时长唤醒
 * @param    val:唤醒时间，us
 * @return   NULL
 * @note:    
 */
void DevAFEWakeUpUs(uint32_t us)
{
    for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
    {
        adBmsCsLow();
        Adbms6830DelayUs(us >> 1);
        adBmsCsHigh();
        Adbms6830DelayUs(us >> 1);
    }
}

/** 
 * @brief    AFE可变时长唤醒
 * @param    val:唤醒时间，ms
 * @return   NULL
 * @note:    
 */
void DevAFEWakeUpMs(uint32_t ms)
{
    for (uint8_t ic = 0; ic < TOTAL_IC; ic++)
    {
        adBmsCsLow();
        delay_1ms(ms >> 1);
        adBmsCsHigh();
        delay_1ms(ms >> 1);
    }
}


/** 
 * @brief    AFE复位
 * @param    val:唤醒时间，ms
 * @return   NULL
 * @note:    
 */
void DevAFEReset()
{
    for(uint8_t cic = 0; cic < TOTAL_IC; cic++)
	{
		/* Init config A */
        IC[cic].tx_cfga.refon = PWR_UP;
        IC[cic].tx_cfga.gpo = 0X3FF; /* All GPIO pull down off */
        IC[cic].tx_cfgb.dtmen = DTMEN_ON;
        /* Init config B */
        IC[cic].tx_cfgb.vov = SetOverVoltageThreshold(OV_THRESHOLD);
        IC[cic].tx_cfgb.vuv = SetUnderVoltageThreshold(UV_THRESHOLD);
//        IC[cic].tx_cfgb.dcc = ConfigB_DccBit(DCC16, DCC_BIT_SET);
        SetConfigB_DischargeTimeOutValue(TOTAL_IC, &IC[cic], RANG_0_TO_63_MIN, TIME_2MIN_OR_0_53HR);
	}
	adBmsWakeupIc(TOTAL_IC);
    spiSendCmd((uint8_t*)SRST);
//    LOG_INFO(LOG_TAG, "AFE Soft Reset done ok!");
    adBmsWakeupIc(TOTAL_IC);
    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGA, Config, A);
    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGB, Config, B);
}

