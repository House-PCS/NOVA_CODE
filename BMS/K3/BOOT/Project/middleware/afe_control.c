

/* Includes ------------------------------------------------------------------*/
#include "afe_control.h"
#include "bsp.h"

#define LOG_TAG     "afe_control"
#define TOTAL_IC 1
#define TSENSOR_NUM 4
#define INA240_REF  2500      //电流采样0点参考电压
#define IDLE_CURRENT  500      //BMS空闲状态浮动电流±500mA

cell_asic IC[TOTAL_IC];
Cell_Info gCell_Info;
//en_afe_state en_afe = EN_AFE_GET_V;
uint8_t gFirst_AFE_Coved_Flag = 0;
en_afe_state en_afe = EN_AFE_IDLE;
/* ADC Command Configurations */
RD      REDUNDANT_MEASUREMENT           = RD_OFF;
CH      AUX_CH_TO_CONVERT               = AUX_ALL;
CONT    CONTINUOUS_MEASUREMENT          = SINGLE;
OW_C_S  CELL_OPEN_WIRE_DETECTION        = OW_OFF_ALL_CH;
OW_AUX  AUX_OPEN_WIRE_DETECTION         = AUX_OW_OFF;
PUP     OPEN_WIRE_CURRENT_SOURCE        = PUP_DOWN;
DCP     DISCHARGE_PERMITTED             = DCP_OFF;
RSTF    RESET_FILTER                    = RSTF_OFF;
ERR     INJECT_ERR_SPI_READ             = WITHOUT_ERR;

/* Set Under Voltage and Over Voltage Thresholds */
const float OV_THRESHOLD = 4.2;                 /* Volt */
const float UV_THRESHOLD = 2;                 /* Volt */

/* configuration registers commands */
uint8_t WRCFGA[2]        = { 0x00, 0x01 };
uint8_t WRCFGB[2]        = { 0x00, 0x24 };
uint8_t WRCMCFG[2]       = { 0x00, 0x58 };
uint8_t RDCFGA[2]        = { 0x00, 0x02 };
uint8_t RDCFGB[2]        = { 0x00, 0x26 };
uint8_t RDCMCFG[2]       = { 0x00, 0x59 };

/* Read cell voltage result registers commands */
uint8_t RDCVA[2]         = { 0x00, 0x04 };
uint8_t RDCVB[2]         = { 0x00, 0x06 };
uint8_t RDCVC[2]         = { 0x00, 0x08 };
uint8_t RDCVD[2]         = { 0x00, 0x0A };
uint8_t RDCVE[2]         = { 0x00, 0x09 };
uint8_t RDCVF[2]         = { 0x00, 0x0B };
uint8_t RDCVALL[2]       = { 0x00, 0x0C };

/* Read average cell voltage result registers commands commands */
uint8_t RDACA[2]         = { 0x00, 0x44 };
uint8_t RDACB[2]         = { 0x00, 0x46 };
uint8_t RDACC[2]         = { 0x00, 0x48 };
uint8_t RDACD[2]         = { 0x00, 0x4A };
uint8_t RDACE[2]         = { 0x00, 0x49 };
uint8_t RDACF[2]         = { 0x00, 0x4B };
uint8_t RDACALL[2]       = { 0x00, 0x4C };

/* Read s voltage result registers commands */
uint8_t RDSVA[2]         = { 0x00, 0x03 };
uint8_t RDSVB[2]         = { 0x00, 0x05 };
uint8_t RDSVC[2]         = { 0x00, 0x07 };
uint8_t RDSVD[2]         = { 0x00, 0x0D };
uint8_t RDSVE[2]         = { 0x00, 0x0E };
uint8_t RDSVF[2]         = { 0x00, 0x0F };
uint8_t RDSALL[2]        = { 0x00, 0x10 };

/* Read c and s results */
uint8_t RDCSALL[2]       = { 0x00, 0x11 };
uint8_t RDACSALL[2]      = { 0x00, 0x51 };

/* Read all AUX and all Status Registers */
uint8_t RDASALL[2]       = { 0x00, 0x35 };

/* Read filtered cell voltage result registers*/
uint8_t RDFCA[2]         = { 0x00, 0x12 };
uint8_t RDFCB[2]         = { 0x00, 0x13 };
uint8_t RDFCC[2]         = { 0x00, 0x14 };
uint8_t RDFCD[2]         = { 0x00, 0x15 };
uint8_t RDFCE[2]         = { 0x00, 0x16 };
uint8_t RDFCF[2]         = { 0x00, 0x17 };
uint8_t RDFCALL[2]       = { 0x00, 0x18 };

/* Read aux results */
uint8_t RDAUXA[2]        = { 0x00, 0x19 };
uint8_t RDAUXB[2]        = { 0x00, 0x1A };
uint8_t RDAUXC[2]        = { 0x00, 0x1B };
uint8_t RDAUXD[2]        = { 0x00, 0x1F };

/* Read redundant aux results */
uint8_t RDRAXA[2]        = { 0x00, 0x1C };
uint8_t RDRAXB[2]        = { 0x00, 0x1D };
uint8_t RDRAXC[2]        = { 0x00, 0x1E };
uint8_t RDRAXD[2]        = { 0x00, 0x25 };

/* Read status registers */
uint8_t RDSTATA[2]       = { 0x00, 0x30 };
uint8_t RDSTATB[2]       = { 0x00, 0x31 };
uint8_t RDSTATC[2]       = { 0x00, 0x32 };
uint8_t RDSTATCERR[2]    = { 0x00, 0x72 };              /* ERR */
uint8_t RDSTATD[2]       = { 0x00, 0x33 };
uint8_t RDSTATE[2]       = { 0x00, 0x34 };

/* Pwm registers commands */
uint8_t WRPWM1[2]        = { 0x00, 0x20 };
uint8_t RDPWM1[2]        = { 0x00, 0x22 };

uint8_t WRPWM2[2]        = { 0x00, 0x21 };
uint8_t RDPWM2[2]        = { 0x00, 0x23 };

/* retention register */
uint8_t ULRR[2]        = { 0x00, 0x38 };
uint8_t WRRR[2]        = { 0x00, 0x39 };
uint8_t RDRR[2]        = { 0x00, 0x3A };

/* Clear commands */
uint8_t CLRCELL[2]       = { 0x07, 0x11 };
uint8_t CLRAUX [2]       = { 0x07, 0x12 };
uint8_t CLRSPIN[2]       = { 0x07, 0x16 };
uint8_t CLRFLAG[2]       = { 0x07, 0x17 };
uint8_t CLRFC[2]         = { 0x07, 0x14 };
uint8_t CLOVUV[2]        = { 0x07, 0x15 };

/* Poll adc command */
uint8_t PLADC[2]         = { 0x07, 0x18 };
uint8_t PLAUT[2]         = { 0x07, 0x19 };
uint8_t PLCADC[2]        = { 0x07, 0x1C };
uint8_t PLSADC[2]        = { 0x07, 0x1D };
uint8_t PLAUX1[2]        = { 0x07, 0x1E };
uint8_t PLAUX2[2]        = { 0x07, 0x1F };

/* Diagn command */
uint8_t DIAGN[2]         = {0x07 , 0x15};

/* GPIOs Comm commands */
uint8_t WRCOMM[2]        = { 0x07, 0x21 };
uint8_t RDCOMM[2]        = { 0x07, 0x22 };
uint8_t STCOMM[13]       = { 0x07, 0x23, 0xB9, 0xE4 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00};

/* Mute and Unmute commands */
uint8_t MUTE[2]            = { 0x00, 0x28 };
uint8_t UNMUTE[2]        = { 0x00, 0x29 };

uint8_t RSTCC[2]         = { 0x00, 0x2E };
uint8_t SNAP[2]          = { 0x00, 0x2D };
uint8_t UNSNAP[2]        = { 0x00, 0x2F };
uint8_t SRST[2]          = { 0x00, 0x27 };

/* Read SID command */
uint8_t RDSID[2]         = { 0x00, 0x2C };
static void _irq_process(void);
/**
*******************************************************************************
* @brief Set configuration register A. Refer to the data sheet
*        Set configuration register B. Refer to the data sheet
*******************************************************************************
*/
void AFEInit(void)
{
    AFESampleTimeHandle.irq_process = _irq_process;
    AFETimerInit();
    SpiAdbms6830BspInit();
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
    spiSendCmd(SRST);
    LOG_INFO(LOG_TAG, "AFE Soft Reset done ok!");
    adBmsWakeupIc(TOTAL_IC);
    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGA, Config, A);
    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGB, Config, B); 
}

bool SetPassiveBalance(uint8_t cell_serial)
{
    if(cell_serial >= CELL)
    {
        LOG_ERROR(LOG_TAG, "Balance cell serial out of range!");
        return false;
    }
    
    /* Init config B */
    en_afe = EN_AFE_SET_B;
//    IC[0].tx_cfgb.dcc |= (1 << cell_serial);    
////    adBmsWakeupIc(TOTAL_IC);
//    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGB, Config, B);
//    adBmsReadData(TOTAL_IC, &IC[0], RDCFGB, Config, B);
//    printWriteConfig(TOTAL_IC, &IC[0], Config, B);
//    printReadConfig(TOTAL_IC, &IC[0], Config, B);
    return true;
}

bool ClearPassiveBalance(uint8_t cell_serial)
{
    if(cell_serial >= CELL)
    {
        LOG_ERROR(LOG_TAG, "Balance cell serial out of range!");
        return false;
    }
    en_afe = EN_AFE_CLEAR_B;
    /* Init config B */
//    IC[0].tx_cfgb.dcc &= (~(1 << cell_serial));        
//    adBmsWakeupIc(TOTAL_IC);
//    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGB, Config, B);
//    adBmsReadData(TOTAL_IC, &IC[0], RDCFGB, Config, B);
//    printWriteConfig(TOTAL_IC, &IC[0], Config, B);
//    printReadConfig(TOTAL_IC, &IC[0], Config, B);
    return true;
}

/*******************************************************************************
  * @FunctionName: SetBalanceCurrent
  * @Author:       xingxiao
  * @DateTime:     2023年1月13日T18:19:06+0800
  * @Purpose:      
  * @param:        duty: 0(0%), 1(6.6%), 2(13.2%), 3(19.8%), 4(26.4%), 5(33%), 6(39.6%), 7(46.2%), 
                         8(52.8%), 9(59.4%), 10(66%), 11(72.6%), 12(79.2%), 13(85.8%), 14(92.4%), 15(100%), 
  * @return:
*******************************************************************************/
bool SetBalanceCurrent(PWM_DUTY e_duty)
{   
//    adBmsWakeupIc(TOTAL_IC);
    SetPwmDutyCycle(TOTAL_IC, IC, e_duty);
    adBmsWriteData(TOTAL_IC, &IC[0], WRPWM1, Pwm, A);
    adBmsWriteData(TOTAL_IC, &IC[0], WRPWM2, Pwm, B);
    adBmsReadData(TOTAL_IC, &IC[0], RDPWM1, Pwm, A);
    adBmsReadData(TOTAL_IC, &IC[0], RDPWM2, Pwm, B);
    return true;
}

void AFEProcess(void)
{
    static uint32_t conv_time = 0;
    static uint32_t conv_time1 = 0;
    uint8_t read_data = 0x00;
    static uint8_t temp = 0;
    switch(en_afe)
	{
        case EN_AFE_IDLE:
        {                      
            break;
        }
        case EN_AFE_GET_V:
        {
            adBmsWakeupIc(TOTAL_IC);
            adBms6830_Adcv(REDUNDANT_MEASUREMENT, CONTINUOUS_MEASUREMENT, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
//            conv_time = tim_get_systime();
            adBmsPollAdc(PLADC);
            en_afe =  EN_AFE_BUSY_V;
        }
        case EN_AFE_BUSY_V:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {                
                uint8_t i = 0;
                adBmsCsHigh();
//                conv_time = tim_get_systime() - conv_time;
//                LOG_DEBUG(LOG_TAG, "conv_votage_time:%dms\r\n", conv_time);
                
                /* 读取电压 */ 
                adBmsWakeupIc(TOTAL_IC);
                adBmsReadData(TOTAL_IC, &IC[0], RDCVA, Cell, A);
                adBmsReadData(TOTAL_IC, &IC[0], RDCVB, Cell, B);
                adBmsReadData(TOTAL_IC, &IC[0], RDCVC, Cell, C);
                adBmsReadData(TOTAL_IC, &IC[0], RDCVD, Cell, D);
                adBmsReadData(TOTAL_IC, &IC[0], RDCVE, Cell, E);
                adBmsReadData(TOTAL_IC, &IC[0], RDCVF, Cell, F);

                /* debug */
//                printVoltages(TOTAL_IC, &IC[0], Cell);    
                
                gCell_Info.total_cell_voltage = 0;
                
                gCell_Info.cell_max_voltage = getMVoltage(IC[0].cell.c_codes[0]);
                gCell_Info.cell_min_voltage = getMVoltage(IC[0].cell.c_codes[0]);
                for(i = 0; i < CELL; i ++)
                {
                    gCell_Info.total_cell_voltage += getMVoltage(IC[0].cell.c_codes[i]);
                    gCell_Info.cell_voltage[i] = getMVoltage(IC[0].cell.c_codes[i]);
                    
                    if(getMVoltage(IC[0].cell.c_codes[i]) > gCell_Info.cell_max_voltage)
                    {
                        gCell_Info.cell_max_voltage = getMVoltage(IC[0].cell.c_codes[i]);
                    }
                    
                    if(getMVoltage(IC[0].cell.c_codes[i]) < gCell_Info.cell_min_voltage)
                    {
                        gCell_Info.cell_min_voltage = getMVoltage(IC[0].cell.c_codes[i]);
                    }
                } 
                gCell_Info.cell_ave_voltage = gCell_Info.total_cell_voltage / CELL;
                en_afe = EN_AFE_GET_C;  
            }            
            break;
        }
        case EN_AFE_GET_C:
        {
            /* 电流转换 */
            /* Init config A */
            IC[0].tx_cfga.refon = PWR_UP;
            IC[0].tx_cfga.gpo = 0X3FF; /* All GPIO pull down off */
            adBmsWakeupIc(TOTAL_IC);                  
            adBmsWriteData(TOTAL_IC, &IC[0], WRCFGA, Config, A);
            AUX_CH_TO_CONVERT = GPIO1;             
            adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, AUX_CH_TO_CONVERT);
//            conv_time = tim_get_systime();       
            adBmsPollAdc(PLADC);
            
            en_afe =  EN_AFE_BUSY_C;
        }
        case EN_AFE_BUSY_C:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {
                int32_t temp;
                adBmsCsHigh();
//                conv_time = tim_get_systime() - conv_time;
//                LOG_DEBUG(LOG_TAG, "conv_current_time:%dms\r\n", conv_time);
                /* 读取电流 */ 
//                adBmsWakeupIc(TOTAL_IC);                
                adBmsReadData(TOTAL_IC, &IC[0], RDAUXA, Aux, A);
                temp = getMVoltage(IC[0].aux.a_codes[0]);
                gCell_Info.cell_current = (temp - INA240_REF)  * 80;   // I(mA) = U(mV) / 0.000625(ohm) /20(增益)
                
//                if(abs(gCell_Info.cell_current) < IDLE_CURRENT)
//                {
//                    gCell_Info.current_status = 0;
//                }
//                else if(gCell_Info.cell_current > IDLE_CURRENT)
//                {
//                    gCell_Info.current_status = 2;
//                }
//                else
//                {
//                    gCell_Info.current_status = 1;
//                }
                /* debug */
//                printVoltages(TOTAL_IC, &IC[0], Aux);  
//                LOG_DEBUG(LOG_TAG, "Pack Current:%dAm\r\n", gCell_Info.cell_current);                
                
//                en_afe = EN_AFE_GET_T0;
                en_afe = EN_AFE_IDLE;
            }            
            break;
        }
        case EN_AFE_GET_T0:
        {
            /* 温度转换 */
            /* Init config A */
//            IC[0].tx_cfga.refon = PWR_UP;
//            IC[0].tx_cfga.gpo = 0X3FF; /* All GPIO pull down off */
            adBmsWakeupIc(TOTAL_IC);
//            adBmsWriteData(TOTAL_IC, &IC[0], WRCFGA, Config, A);
            AUX_CH_TO_CONVERT = GPIO2;            
            adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, AUX_CH_TO_CONVERT);
//            conv_time = tim_get_systime();
            adBmsPollAdc(PLADC);
            en_afe =  EN_AFE_BUSY_T0;            
        }
        case EN_AFE_BUSY_T0:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {
                adBmsCsHigh();
                en_afe =  EN_AFE_IDLE;    
//                conv_time = tim_get_systime() - conv_time;
//                LOG_DEBUG(LOG_TAG, "EN_AFE_BUSY_T0_time:%dms\r\n", conv_time);                
            }            
            break;
        }
        case EN_AFE_GET_T1:
        {
            /* 温度转换 */
            adBmsWakeupIc(TOTAL_IC);
            AUX_CH_TO_CONVERT = GPIO3;            
            adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, AUX_CH_TO_CONVERT);
            adBmsPollAdc(PLADC);
//            conv_time = tim_get_systime();
            en_afe =  EN_AFE_BUSY_T1;            
        }
        case EN_AFE_BUSY_T1:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {
                adBmsCsHigh();
                en_afe =  EN_AFE_IDLE; 
//                conv_time = tim_get_systime() - conv_time;
//                LOG_DEBUG(LOG_TAG, "EN_AFE_BUSY_T1_time:%dms\r\n", conv_time);                 
            }            
            break;
        }
        case EN_AFE_GET_T2:
        {
            /* 温度转换 */
            adBmsWakeupIc(TOTAL_IC);
            AUX_CH_TO_CONVERT = GPIO4;            
            adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, AUX_CH_TO_CONVERT);
            adBmsPollAdc(PLADC);
//            conv_time = tim_get_systime();
            en_afe =  EN_AFE_BUSY_T2;            
        }
        case EN_AFE_BUSY_T2:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {
                adBmsCsHigh();
                en_afe =  EN_AFE_IDLE;
//                conv_time = tim_get_systime() - conv_time;
//                LOG_DEBUG(LOG_TAG, "EN_AFE_BUSY_T2_time:%dms\r\n", conv_time);         
            }            
            break;
        }
        case EN_AFE_GET_T3:
        {
            /* 温度转换 */
            adBmsWakeupIc(TOTAL_IC);
            AUX_CH_TO_CONVERT = GPIO5;            
            adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, AUX_CH_TO_CONVERT);
            adBmsPollAdc(PLADC);
//            conv_time = tim_get_systime();
            en_afe =  EN_AFE_BUSY_T3;            
        }
        case EN_AFE_BUSY_T3:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {
                adBmsCsHigh();
                en_afe =  EN_AFE_IDLE;
//                conv_time = tim_get_systime() - conv_time;
//                LOG_DEBUG(LOG_TAG, "EN_AFE_BUSY_T3_time:%dms\r\n", conv_time);                         
            }            
            break;
        }
        case EN_AFE_GET_T_AFE:
        {
            /* 温度转换 */
            adBmsWakeupIc(TOTAL_IC);
            AUX_CH_TO_CONVERT = GPIO8;            
            adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, AUX_CH_TO_CONVERT);
            adBmsPollAdc(PLADC);
//            conv_time = tim_get_systime();
            en_afe =  EN_AFE_BUSY_T_AFE;            
        }
        case EN_AFE_BUSY_T_AFE:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {
                uint8_t i = 0;
//                uint8_t len = 0;
                int16_t cell_ave_temp = 0;
                
//                len = sizeof(gCell_Info.cell_temperature) / sizeof(int16_t);
                adBmsCsHigh();
//                conv_time = tim_get_systime() - conv_time;
//                LOG_DEBUG(LOG_TAG, "EN_AFE_BUSY_T_AFE:%dms\r\n", conv_time);
                
                /* 温度读取 */   
//                adBmsWakeupIc(TOTAL_IC);
                adBmsReadData(TOTAL_IC, &IC[0], RDAUXA, Aux, A);
                adBmsReadData(TOTAL_IC, &IC[0], RDAUXB, Aux, B);
                adBmsReadData(TOTAL_IC, &IC[0], RDAUXC, Aux, C);
                for(i = 0; i < TSENSOR_NUM; i ++)
                {
                    gCell_Info.cell_temperature[i] = (int16_t)(TemperatureDataChange(getVoltage(IC[0].aux.a_codes[i + 1]), 100) * 10);     //第一个GPIO采集电流，温度从第二个io开始  
                    cell_ave_temp += gCell_Info.cell_temperature[i];
                    
                    if(gCell_Info.cell_temperature[i] >= gCell_Info.cell_max_temperature)
                    {
                        gCell_Info.cell_max_temperature = gCell_Info.cell_temperature[i];
                    }
                    else if(gCell_Info.cell_temperature[i] <= gCell_Info.cell_min_temperature)
                    {
                        gCell_Info.cell_min_temperature = gCell_Info.cell_temperature[i];
                    }                    
                }
                
                gCell_Info.cell_ave_temperature = cell_ave_temp / TSENSOR_NUM;
                gCell_Info.afe_net_temperature = (int16_t)(TemperatureDataChange(getVoltage(IC[0].aux.a_codes[7]), 15) * 10); 
//                float_temperature = gCell_Info.afe_net_temperature;
//                u16_temperature = float_cov_uint16(float_temperature);
//                float_temperature = uint16_cov_float(u16_temperature);
                /* debug */
//                printVoltages(TOTAL_IC, &IC[0], Aux);
//                LOG_DEBUG(LOG_TAG, "Cell_NTC_Temperature0: %4.3f c\r\n", gCell_Info.cell_temperature[0] / 100.0);
//                LOG_DEBUG(LOG_TAG, "Cell_NTC_Temperature0: %4.3f c\r\n", gCell_Info.cell_temperature[1] / 100.0);
//                LOG_DEBUG(LOG_TAG, "Cell_NTC_Temperature0: %4.3f c\r\n", gCell_Info.cell_temperature[2] / 100.0);
//                LOG_DEBUG(LOG_TAG, "Cell_NTC_Temperature0: %4.3f c\r\n", gCell_Info.cell_temperature[3] / 100.0);
//                LOG_DEBUG(LOG_TAG, "AFE_NTC_Temperature: %4.3f c\r\n", gCell_Info.afe_net_temperature / 100.0);
                
                en_afe = EN_AFE_IDLE;           
                break;                
            }            
            break;
        }
        case EN_AFE_SET_B:
        {
            IC[0].tx_cfgb.dcc = 0xFFFF; 
//    adBmsWakeupIc(TOTAL_IC);
            adBmsWriteData(TOTAL_IC, &IC[0], WRCFGB, Config, B);
            adBmsReadData(TOTAL_IC, &IC[0], RDCFGB, Config, B);
            printWriteConfig(TOTAL_IC, &IC[0], Config, B);
            printReadConfig(TOTAL_IC, &IC[0], Config, B);
            
            en_afe = EN_AFE_IDLE;
            break;
        }
        case EN_AFE_CLEAR_B:
        {
            IC[0].tx_cfgb.dcc = 0; 
//    adBmsWakeupIc(TOTAL_IC);
            adBmsWriteData(TOTAL_IC, &IC[0], WRCFGB, Config, B);
            adBmsReadData(TOTAL_IC, &IC[0], RDCFGB, Config, B);
            printWriteConfig(TOTAL_IC, &IC[0], Config, B);
            printReadConfig(TOTAL_IC, &IC[0], Config, B);
            
            en_afe = EN_AFE_IDLE;
            break;
        }
        default:
            break;
    }
}

bool VotageCurrentGet(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! VotageCurrentGet Error!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_GET_V;    
    }   
    return true;
}
bool CurrentGet(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! CurrentGet Error!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_GET_C;    
    }  
    return true;
}
bool TemperatureGet_T0(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! EN_AFE_GET_T0 Error!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_GET_T0;    
    } 
    
    return true;
}

bool TemperatureGet_T1(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! EN_AFE_GET_T1 Error!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_GET_T1;    
    } 
    
    return true;
}

bool TemperatureGet_T2(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! EN_AFE_GET_T2 Error!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_GET_T2;    
    } 
    
    return true;
}

bool TemperatureGet_T3(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! EN_AFE_GET_T3 Error!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_GET_T3;    
    } 
    
    return true;
}

bool TemperatureGet_AFE(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! EN_AFE_GET_T_AFE Error!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_GET_T_AFE;    
    } 
    
    return true;
}

en_afe_state AFEStatusGet(void)
{    
    return en_afe;
}

static void read_cfg(void)
{
    adBmsReadData(TOTAL_IC, &IC[0], RDCFGB, Config, B);   
    printReadConfig(TOTAL_IC, &IC[0], Config, B);    
}

static void _irq_process(void)
{
    static uint16_t count = 0;
    static uint8_t blinkflag = 0;
    uint8_t temp = 0;
    if(gFirst_AFE_Coved_Flag == 7)
    {        
        count ++;
        if(count == 500)
        {
            count = 0;
        }
        
        if(count % 10 == 0)
        {
            SOCAmpereHourIntegral(10);
            VotageCurrentGet();
        }              
        else if(count == 96)
        {
            TemperatureGet_T0();
        }
        else if(count == 196)
        {
            TemperatureGet_T1();
        }
        else if(count == 296)
        {
            TemperatureGet_T2();
        }
        else if(count == 396)
        {
            TemperatureGet_T3();
        }
        else if(count == 496)
        {
            TemperatureGet_AFE();
        }
    }    
}

bool AFEFirstSample(void)
{
    /* 确保AFE第一次采集完数据后再进入故障解析 */
    if(gFirst_AFE_Coved_Flag == 7)
    {
        return true;
    }
    if(gFirst_AFE_Coved_Flag == 0)
    {
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 1;
            VotageCurrentGet();
        }    
    }
    
    if(gFirst_AFE_Coved_Flag == 1)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 2;
            TemperatureGet_T0();
        }   
    }
    
    if(gFirst_AFE_Coved_Flag == 2)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 3;
            TemperatureGet_T1();
        }   
    }
    
    if(gFirst_AFE_Coved_Flag == 3)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 4;
            TemperatureGet_T2();
        }   
    }
    
    if(gFirst_AFE_Coved_Flag == 4)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 5;
            TemperatureGet_T3();
        }   
    }
    
    if(gFirst_AFE_Coved_Flag == 5)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 6;
            TemperatureGet_AFE();
        }   
    }
    
    if(gFirst_AFE_Coved_Flag == 6)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 7;
            return true;
        }   
    }
    
    return false;
}

MSH_CMD_EXPORT(VotageCurrentGet, AFE);
MSH_CMD_EXPORT(CurrentGet, AFE);
//MSH_CMD_EXPORT(TemperatureGet, AFE);
MSH_CMD_EXPORT(read_cfg, AFE);
MSH_CMD_EXPORT(SetPassiveBalance, cell 0~15);
MSH_CMD_EXPORT(ClearPassiveBalance, cell 0~15);
FINSH_CMD_EXPORT(SetBalanceCurrent, cell 0~15);

