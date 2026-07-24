

/* Includes ------------------------------------------------------------------*/
#include "afe_control.h"
#include "bsp.h"

#define LOG_TAG     "afe_control"
#define TOTAL_IC 1 
#define TSENSOR_NUM 4
#define INA240_REF  2500      //电流采样0点参考电压
#define IDLE_CURRENT  500      //BMS空闲状态浮动电流±500mA
static uint16_t gINA240Ref = 0;
cell_asic IC[TOTAL_IC];
Cell_Info gCell_Info;
//en_afe_state en_afe = EN_AFE_GET_V;
uint8_t gFirst_AFE_Coved_Flag = 0;
en_afe_state en_afe = EN_AFE_IDLE;
static en_check_state _eCheckStatus = EN_CHECK_IDLE;
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
const uint8_t WRCFGA[2]        = { 0x00, 0x01 };
const uint8_t WRCFGB[2]        = { 0x00, 0x24 };
const uint8_t WRCMCFG[2]       = { 0x00, 0x58 };
const uint8_t RDCFGA[2]        = { 0x00, 0x02 };
const uint8_t RDCFGB[2]        = { 0x00, 0x26 };
const uint8_t RDCMCFG[2]       = { 0x00, 0x59 };

/* Read cell voltage result registers commands */
const uint8_t RDCVA[2]         = { 0x00, 0x04 };
const uint8_t RDCVB[2]         = { 0x00, 0x06 };
const uint8_t RDCVC[2]         = { 0x00, 0x08 };
const uint8_t RDCVD[2]         = { 0x00, 0x0A };
const uint8_t RDCVE[2]         = { 0x00, 0x09 };
const uint8_t RDCVF[2]         = { 0x00, 0x0B };
const uint8_t RDCVALL[2]       = { 0x00, 0x0C };

/* Read average cell voltage result registers commands commands */
const uint8_t RDACA[2]         = { 0x00, 0x44 };
const uint8_t RDACB[2]         = { 0x00, 0x46 };
const uint8_t RDACC[2]         = { 0x00, 0x48 };
const uint8_t RDACD[2]         = { 0x00, 0x4A };
const uint8_t RDACE[2]         = { 0x00, 0x49 };
const uint8_t RDACF[2]         = { 0x00, 0x4B };
const uint8_t RDACALL[2]       = { 0x00, 0x4C };

/* Read s voltage result registers commands */
const uint8_t RDSVA[2]         = { 0x00, 0x03 };
const uint8_t RDSVB[2]         = { 0x00, 0x05 };
const uint8_t RDSVC[2]         = { 0x00, 0x07 };
const uint8_t RDSVD[2]         = { 0x00, 0x0D };
const uint8_t RDSVE[2]         = { 0x00, 0x0E };
const uint8_t RDSVF[2]         = { 0x00, 0x0F };
const uint8_t RDSALL[2]        = { 0x00, 0x10 };

/* Read c and s results */
const uint8_t RDCSALL[2]       = { 0x00, 0x11 };
const uint8_t RDACSALL[2]      = { 0x00, 0x51 };

/* Read all AUX and all Status Registers */
const uint8_t RDASALL[2]       = { 0x00, 0x35 };

/* Read filtered cell voltage result registers*/
const uint8_t RDFCA[2]         = { 0x00, 0x12 };
const uint8_t RDFCB[2]         = { 0x00, 0x13 };
const uint8_t RDFCC[2]         = { 0x00, 0x14 };
const uint8_t RDFCD[2]         = { 0x00, 0x15 };
const uint8_t RDFCE[2]         = { 0x00, 0x16 };
const uint8_t RDFCF[2]         = { 0x00, 0x17 };
const uint8_t RDFCALL[2]       = { 0x00, 0x18 };

/* Read aux results */
const uint8_t RDAUXA[2]        = { 0x00, 0x19 };
const uint8_t RDAUXB[2]        = { 0x00, 0x1A };
const uint8_t RDAUXC[2]        = { 0x00, 0x1B };
const uint8_t RDAUXD[2]        = { 0x00, 0x1F };

/* Read redundant aux results */
const uint8_t RDRAXA[2]        = { 0x00, 0x1C };
const uint8_t RDRAXB[2]        = { 0x00, 0x1D };
const uint8_t RDRAXC[2]        = { 0x00, 0x1E };
const uint8_t RDRAXD[2]        = { 0x00, 0x25 };

/* Read status registers */
const uint8_t RDSTATA[2]       = { 0x00, 0x30 };
const uint8_t RDSTATB[2]       = { 0x00, 0x31 };
const uint8_t RDSTATC[2]       = { 0x00, 0x32 };
const uint8_t RDSTATCERR[2]    = { 0x00, 0x72 };              /* ERR */
const uint8_t RDSTATD[2]       = { 0x00, 0x33 };
const uint8_t RDSTATE[2]       = { 0x00, 0x34 };

/* Pwm registers commands */
const uint8_t WRPWM1[2]        = { 0x00, 0x20 };
const uint8_t RDPWM1[2]        = { 0x00, 0x22 };

const uint8_t WRPWM2[2]        = { 0x00, 0x21 };
const uint8_t RDPWM2[2]        = { 0x00, 0x23 };

/* retention register */
const uint8_t ULRR[2]        = { 0x00, 0x38 };
const uint8_t WRRR[2]        = { 0x00, 0x39 };
const uint8_t RDRR[2]        = { 0x00, 0x3A };

/* Clear commands */
const uint8_t CLRCELL[2]       = { 0x07, 0x11 };
const uint8_t CLRAUX [2]       = { 0x07, 0x12 };
const uint8_t CLRSPIN[2]       = { 0x07, 0x16 };
const uint8_t CLRFLAG[2]       = { 0x07, 0x17 };
const uint8_t CLRFC[2]         = { 0x07, 0x14 };
const uint8_t CLOVUV[2]        = { 0x07, 0x15 };

/* Poll adc command */
const uint8_t PLADC[2]         = { 0x07, 0x18 };
const uint8_t PLAUT[2]         = { 0x07, 0x19 };
const uint8_t PLCADC[2]        = { 0x07, 0x1C };
const uint8_t PLSADC[2]        = { 0x07, 0x1D };
const uint8_t PLAUX1[2]        = { 0x07, 0x1E };
const uint8_t PLAUX2[2]        = { 0x07, 0x1F };

/* Diagn command */
const uint8_t DIAGN[2]         = {0x07 , 0x15};

/* GPIOs Comm commands */
const uint8_t WRCOMM[2]        = { 0x07, 0x21 };
const uint8_t RDCOMM[2]        = { 0x07, 0x22 };
const uint8_t STCOMM[13]       = { 0x07, 0x23, 0xB9, 0xE4 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00};

/* Mute and Unmute commands */
const uint8_t MUTE[2]            = { 0x00, 0x28 };
const uint8_t UNMUTE[2]        = { 0x00, 0x29 };

const uint8_t RSTCC[2]         = { 0x00, 0x2E };
const uint8_t SNAP[2]          = { 0x00, 0x2D };
const uint8_t UNSNAP[2]        = { 0x00, 0x2F };
const uint8_t SRST[2]          = { 0x00, 0x27 };

/* Read SID command */
const uint8_t RDSID[2]         = { 0x00, 0x2C };
static void _irq_process(void);
static void _AFEOpenWireCheck(void);
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
//        IC[cic].tx_cfgb.vuv = SetUnderVoltageThreshold(UV_THRESHOLD);
//        IC[cic].tx_cfgb.dcc = ConfigB_DccBit(DCC16, DCC_BIT_SET);               
        SetConfigB_DischargeTimeOutValue(TOTAL_IC, &IC[cic], RANG_0_TO_63_MIN, TIME_2MIN_OR_0_53HR);
    }
    adBmsWakeupIc(TOTAL_IC);
    spiSendCmd((uint8_t*)SRST);
    LOG_INFO(LOG_TAG, "AFE Soft Reset done ok!");
    adBmsWakeupIc(TOTAL_IC);
    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGA, Config, A);
    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGB, Config, B); 
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
    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRPWM1, Pwm, A);
    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRPWM2, Pwm, B);
    adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDPWM1, Pwm, A);
    adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDPWM2, Pwm, B);
    return true;
}

void AFEProcess(void)
{
    static uint32_t conv_time = 0;
    static uint32_t conv_time1 = 0;
    uint8_t read_data = 0x00;
    switch(en_afe)
	{
        case EN_AFE_IDLE:
        {                      
            break;
        }
        case EN_AFE_GET_V:
        {
//            conv_time = tim_get_systime();
            adBmsWakeupIc(TOTAL_IC);
            adBms6830_Adcv(REDUNDANT_MEASUREMENT, CONTINUOUS_MEASUREMENT, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
            
            adBmsPollAdc((uint8_t*)PLADC);
            en_afe =  EN_AFE_BUSY_V;
        }
        case EN_AFE_BUSY_V:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {                
                uint8_t i = 0;
                adBmsCsHigh();                
                
                /* 读取电压 */ 
                adBmsWakeupIc(TOTAL_IC);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVA, Cell, A);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVB, Cell, B);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVC, Cell, C);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVD, Cell, D);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVE, Cell, E);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVF, Cell, F);
                adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGA, Config, A);
                /* debug */
//                printVoltages(TOTAL_IC, &IC[0], Cell);    
                
                gCell_Info.total_cell_voltage = 0;
                
                gCell_Info.cell_max_voltage = getMVoltage(IC[0].cell.c_codes[0]);
                gCell_Info.cell_min_voltage = getMVoltage(IC[0].cell.c_codes[0]);
                for(i = 0; i < CELL; i ++)
                {
                    gCell_Info.total_cell_voltage += getMVoltage(IC[0].cell.c_codes[i]);
                    if(gSoxDebugParam.sox_debug_mode)
                    {
                        gCell_Info.cell_voltage[i] = 3300;
                    }
                    else
                    {
                        gCell_Info.cell_voltage[i] = getMVoltage(IC[0].cell.c_codes[i]);
                    }
                                        
                    if(getMVoltage(IC[0].cell.c_codes[i]) > gCell_Info.cell_max_voltage)
                    {
                        gCell_Info.cell_max_voltage = getMVoltage(IC[0].cell.c_codes[i]);
                    }
                    
                    if(getMVoltage(IC[0].cell.c_codes[i]) < gCell_Info.cell_min_voltage)
                    {
                        gCell_Info.cell_min_voltage = getMVoltage(IC[0].cell.c_codes[i]);
                    }
                }
                gCell_Info.cell_diff_voltage = gCell_Info.cell_max_voltage - gCell_Info.cell_min_voltage;
                gCell_Info.cell_ave_voltage = gCell_Info.total_cell_voltage / CELL;
//                en_afe = EN_AFE_GET_C;
                if(gSoxDebugParam.sox_debug_mode)
                {
                    gCell_Info.cell_max_voltage = 3300;
                    gCell_Info.cell_min_voltage = 3300;
                    gCell_Info.cell_ave_voltage = 3300;
                    gCell_Info.cell_diff_voltage = 0;
                    gCell_Info.total_cell_voltage = 49500;
                }                
                if((gPackStatus.mode != EN_PACK_CURRENT_DISCHARGE_MODE) && (gCell_Info.cell_diff_voltage > 100) /*&& (gBmsErrorInfo.cell1_error_code.value ==0)*/)
                {
                    en_afe = EN_AFE_SET_B;
//                    en_afe = EN_AFE_IDLE;
                }
                else
                {
                    en_afe = EN_AFE_IDLE;
                }
//                conv_time = tim_get_systime() - conv_time;
//                LOG_DEBUG(LOG_TAG, "conv_votage_time:%dms\r\n", conv_time);
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
            adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGA, Config, A);
            AUX_CH_TO_CONVERT = GPIO1;             
            adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, AUX_CH_TO_CONVERT);
//            conv_time = tim_get_systime();       
            adBmsPollAdc((uint8_t*)PLADC);
            
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
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDAUXA, Aux, A);
                temp = getMVoltage(IC[0].aux.a_codes[0]);
                /* 校准REF */
                if(gFirst_AFE_Coved_Flag == 1)
                {
                    gCell_Info.cur_ref = temp;
                }
                gCell_Info.cell_current = (temp - gINA240Ref)  * 200;   // I(mA) = U(mV) / 0.0005(ohm) /20(增益)
                
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
            adBmsPollAdc((uint8_t*)PLADC);
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
            adBmsPollAdc((uint8_t*)PLADC);
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
            adBmsPollAdc((uint8_t*)PLADC);
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
            adBmsPollAdc((uint8_t*)PLADC);
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
            adBmsPollAdc((uint8_t*)PLADC);
//            conv_time = tim_get_systime();
            en_afe =  EN_AFE_BUSY_T_AFE;            
        }
        case EN_AFE_BUSY_T_AFE:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {                                
                adBmsCsHigh();                               
                en_afe = EN_AFE_IDLE;           
                break;                
            }            
            break;
        }
        case EN_AFE_T_READ:
        {
            uint8_t i = 0;
            int16_t cell_ave_temp = 0;
            /* 温度读取 */   
//                adBmsWakeupIc(TOTAL_IC);
            adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDAUXA, Aux, A);
            adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDAUXB, Aux, B);
            adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDAUXC, Aux, C);
            gCell_Info.cell_min_temperature = gCell_Info.cell_ave_temperature;
            gCell_Info.cell_max_temperature = gCell_Info.cell_ave_temperature;
            for(i = 0; i < TSENSOR_NUM; i ++)
            {
                if(gSoxDebugParam.sox_debug_mode)
                {
                    gCell_Info.cell_temperature[i] = 235;
                }
                else
                {
                    gCell_Info.cell_temperature[i] = (int16_t)(TemperatureDataChange4200(getVoltage(IC[0].aux.a_codes[i + 1]), 100) * 10);     //第一个GPIO采集电流，温度从第二个io开始  
                }
                
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
//            gCell_Info.afe_net_temperature = (int16_t)(TemperatureDataChange(getVoltage(IC[0].aux.a_codes[7]), 15) * 10); 
            en_afe = EN_AFE_IDLE;
            break;
        }
        case EN_AFE_SET_B:
        {
            uint16_t dcc_cell = 0;
//            conv_time = tim_get_systime();
            for(uint8_t i = 0; i < CELL; i ++)
            {
                if(gCell_Info.cell_voltage[i] - gCell_Info.cell_min_voltage > 50)
                {
                    dcc_cell |= (1 << i);
                }
            }
            
            IC[0].tx_cfgb.dcc = dcc_cell; 
            IC[0].tx_cfgb.vuv = gCell_Info.cell_min_voltage;                      
            adBmsWakeupIc(TOTAL_IC);
            SetConfigB_DischargeTimeOutValue(TOTAL_IC, &IC[0], RANG_0_TO_63_MIN, TIME_2MIN_OR_0_53HR);
//            adBms6830_AllDccStart();
            adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGB, Config, B);
//            adBmsReadData(TOTAL_IC, &IC[0], RDCFGB, Config, B);
//            printWriteConfig(TOTAL_IC, &IC[0], Config, B);
//            printReadConfig(TOTAL_IC, &IC[0], Config, B);
//            conv_time = tim_get_systime() - conv_time;
//            LOG_DEBUG(LOG_TAG, "conv_time:%dms\r\n", conv_time);
            gCell_Info.balance_open_status = 1;
            en_afe = EN_AFE_IDLE;
            break;
        }
        case EN_AFE_CLEAR_B:
        {
            if(gCell_Info.balance_open_status)
            {
                gCell_Info.balance_open_status = 0;
//                conv_time = tim_get_systime();
                IC[0].tx_cfgb.dcc = 0; 
                adBmsWakeupIc(TOTAL_IC);
//                adBms6830_AllDccStop();
                adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGB, Config, B);
//                adBmsReadData(TOTAL_IC, &IC[0], RDCFGB, Config, B);
//                printWriteConfig(TOTAL_IC, &IC[0], Config, B);
//                printReadConfig(TOTAL_IC, &IC[0], Config, B);
//                conv_time = tim_get_systime() - conv_time;
//                LOG_DEBUG(LOG_TAG, "conv_time:%dms\r\n", conv_time);
            }
            en_afe = EN_AFE_IDLE;
            break;
        }
        case EN_AFE_OPENWIRE_CHECK:
        {
            _AFEOpenWireCheck();
            if(_eCheckStatus == EN_CHECK_OVER)
            {
                en_afe = EN_AFE_IDLE;
                _eCheckStatus = EN_CHECK_IDLE;
            }
            break;
        }
        default:
            break;
    }
}

//static void _AFEOpenWireCheck(void)
//{
//    uint8_t read_data = 0x00;
//    static uint32_t wait_time = 0;
//    switch(_eCheckStatus)
//    {
//        case EN_CHECK_IDLE:
//        {
//            wait_time = tim_get_systime();
//            _eCheckStatus = EN_CHECK_START;            
//            break;
//        }
//        case EN_CHECK_START:
//        {
//            if(tim_difftime_from_now(wait_time) > 5)
//            {
//                if(gCell_Info.balance_open_status)
//                {
//                    gCell_Info.balance_open_status = 0;
//                    IC[0].tx_cfgb.dcc = 0; 
//                    adBmsWakeupIc(TOTAL_IC);
//                    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGB, Config, B);
//                }
//                _eCheckStatus = EN_CHECK_COV1;
//                wait_time = tim_get_systime();
//            }
//            break;
//        }
//        case EN_CHECK_COV1:
//        {
//            if(tim_difftime_from_now(wait_time) > 10)
//            {
//                for(uint8_t cic = 0; cic < TOTAL_IC; cic++)
//                {
//                /* Init config A */
//                IC[cic].tx_cfga.refon = PWR_UP;
//                IC[cic].tx_cfga.soakon = SOAKON_CLR;   /* Set or clear if required */
//                IC[cic].tx_cfga.owrng = SHORT;         /* Set or clear if required */
//                }
//                adBmsWakeupIc(TOTAL_IC);
//                spiSendCmd((uint8_t*)SRST);
//                adBmsWakeupIc(TOTAL_IC);
//                adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGA, Config, A);
////                adBms6830_Adcv(RD_OFF, SINGLE, DCP_OFF, RSTF_OFF, OW_ON_EVEN_CH);
//                adBms6830_Adsv(SINGLE, DCP_OFF, OW_ON_EVEN_CH);
//                adBmsPollAdc((uint8_t*)PLADC);
//                _eCheckStatus = EN_CHECK_COV1_BUSY;
//            }
//            break;
//        }
//        case EN_CHECK_COV1_BUSY:
//        {
//            spiReadBytes(1, &read_data);
//            if(read_data == 0xFF)
//            {                
//                adBmsCsHigh();
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVA, S_volt, A);
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVB, S_volt, B);
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVC, S_volt, C);
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVD, S_volt, D);
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVE, S_volt, E);
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVF, S_volt, F);
//                adBms6830_cell_ow_volatge_collect(TOTAL_IC, &IC[0], S_volt, OW_ON_EVEN_CH);
//                
//                _eCheckStatus = EN_CHECK_COV2;
////                _eCheckStatus = EN_CHECK_OVER_WAIT;
//                wait_time = tim_get_systime();
//            }
//            break;
//        }
//        case EN_CHECK_COV2:
//        {
//            if(tim_difftime_from_now(wait_time) > 10)
//            {
////                adBms6830_Adcv(RD_OFF, SINGLE, DCP_OFF, RSTF_OFF, OW_ON_ODD_CH);
//                adBms6830_Adsv(SINGLE, DCP_OFF, OW_ON_ODD_CH);
//                adBmsPollAdc((uint8_t*)PLADC);
//                _eCheckStatus = EN_CHECK_COV2_BUSY;
//            }
//            break;
//        }
//        case EN_CHECK_COV2_BUSY:
//        {
//            spiReadBytes(1, &read_data);
//            if(read_data == 0xFF)
//            {                
//                adBmsCsHigh();
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVA, S_volt, A);
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVB, S_volt, B);
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVC, S_volt, C);
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVD, S_volt, D);
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVE, S_volt, E);
//                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDSVF, S_volt, F);
//                adBms6830_cell_ow_volatge_collect(TOTAL_IC, &IC[0], S_volt, OW_ON_ODD_CH);

//                adBms6830_open_wire_detection_condtion_check(TOTAL_IC, &IC[0], S_volt);
//                for(uint8_t ic = 0; ic < TOTAL_IC; ic++)
//                {
//                    for(uint8_t cell = 0; cell < CELL; cell++)
//                    {
//                        if(IC[ic].diag_result.cell_ow[cell])
//                        {
//                            gBmsErrorInfo.cellsample_error_code.b.cell_vol_sample_miss = 1;
//                        }
//                    }
//                }
//                _eCheckStatus = EN_CHECK_OVER_WAIT;
//                wait_time = tim_get_systime();
//            }
//            break;
//        }
//        case EN_CHECK_OVER_WAIT:
//        {
//            if(tim_difftime_from_now(wait_time) > 5)
//            {
//                _eCheckStatus = EN_CHECK_OVER;
//            }
//            break;
//        }
//        
//        default: break;
//    }
//}
static void _AFEOpenWireCheck(void)
{
    uint8_t read_data = 0x00;
    static uint32_t wait_time = 0;
    switch(_eCheckStatus)
    {
        case EN_CHECK_IDLE:
        {
            wait_time = tim_get_systime();
            _eCheckStatus = EN_CHECK_START;            
            break;
        }
        case EN_CHECK_START:
        {
            if(tim_difftime_from_now(wait_time) > 5)
            {
                if(gCell_Info.balance_open_status)
                {
                    gCell_Info.balance_open_status = 0;
                    IC[0].tx_cfgb.dcc = 0; 
                    adBmsWakeupIc(TOTAL_IC);
                    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGB, Config, B);
                }
                _eCheckStatus = EN_CHECK_COV1;
                wait_time = tim_get_systime();
            }
            break;
        }
        case EN_CHECK_COV1:
        {
            if(tim_difftime_from_now(wait_time) > 10)
            {
                for(uint8_t cic = 0; cic < TOTAL_IC; cic++)
                {
                /* Init config A */
                IC[cic].tx_cfga.refon = PWR_UP;
                IC[cic].tx_cfga.soakon = SOAKON_CLR;   /* Set or clear if required */
                IC[cic].tx_cfga.owrng = SHORT;         /* Set or clear if required */
                }
                adBmsWakeupIc(TOTAL_IC);
                spiSendCmd((uint8_t*)SRST);
                adBmsWakeupIc(TOTAL_IC);
                adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGA, Config, A);
                adBms6830_Adcv(RD_OFF, SINGLE, DCP_OFF, RSTF_OFF, OW_ON_EVEN_CH);
                adBmsPollAdc((uint8_t*)PLADC);
                _eCheckStatus = EN_CHECK_COV1_BUSY;
            }
            break;
        }
        case EN_CHECK_COV1_BUSY:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {                
                adBmsCsHigh();
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVA, Cell, A);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVB, Cell, B);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVC, Cell, C);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVD, Cell, D);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVE, Cell, E);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVF, Cell, F);
                adBms6830_cell_ow_volatge_collect(TOTAL_IC, &IC[0], Cell, OW_ON_EVEN_CH);
                
                _eCheckStatus = EN_CHECK_COV2;
//                _eCheckStatus = EN_CHECK_OVER_WAIT;
                wait_time = tim_get_systime();
            }
            break;
        }
        case EN_CHECK_COV2:
        {
            if(tim_difftime_from_now(wait_time) > 10)
            {
                adBms6830_Adcv(RD_OFF, SINGLE, DCP_OFF, RSTF_OFF, OW_ON_ODD_CH);
                adBmsPollAdc((uint8_t*)PLADC);
                _eCheckStatus = EN_CHECK_COV2_BUSY;
            }
            break;
        }
        case EN_CHECK_COV2_BUSY:
        {
            spiReadBytes(1, &read_data);
            if(read_data == 0xFF)
            {                
                adBmsCsHigh();
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVA, Cell, A);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVB, Cell, B);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVC, Cell, C);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVD, Cell, D);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVE, Cell, E);
                adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCVF, Cell, F);
                adBms6830_cell_ow_volatge_collect(TOTAL_IC, &IC[0], Cell, OW_ON_ODD_CH);

                adBms6830_open_wire_detection_condtion_check(TOTAL_IC, &IC[0], Cell);
//                for(uint8_t ic = 0; ic < TOTAL_IC; ic++)
//                {
//                    for(uint8_t cell = 0; cell < CELL; cell++)
//                    {
//                        if(IC[ic].diag_result.cell_ow[cell])
//                        {
//                            gBmsErrorInfo.cellsample_error_code.b.cell_vol_sample_miss = 1;
//                        }
//                    }
//                }
                if(gCell_Info.cell_diff_voltage > 1500)
                {
//                    gBmsErrorInfo.cellsample_error_code.b.cell_vol_sample_miss = 1;
                }
                _eCheckStatus = EN_CHECK_OVER_WAIT;
                wait_time = tim_get_systime();
            }
            break;
        }
        case EN_CHECK_OVER_WAIT:
        {
            if(tim_difftime_from_now(wait_time) > 5)
            {
                _eCheckStatus = EN_CHECK_OVER;
            }
            break;
        }
        
        default: break;
    }
}

bool VotageCurrentGet(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! VotageCurrentGet miss!!!\r\n");
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
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! CurrentGet miss!!!\r\n");
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
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! EN_AFE_GET_T0 miss!!!\r\n");
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
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! EN_AFE_GET_T1 miss!!!\r\n");
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
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! EN_AFE_GET_T2 miss!!!\r\n");
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
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! EN_AFE_GET_T3 miss!!!\r\n");
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
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! EN_AFE_GET_T_AFE miss!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_GET_T_AFE;    
    } 
    
    return true;
}

bool TemperatureRead(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! TemperatureRead miss!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_T_READ;    
    } 
    
    return true;
}

bool SetBalance(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! SetBalance miss!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_SET_B;    
    } 
    
    return true;
}

bool ClearBalance(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! ClearBalance miss!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_CLEAR_B;    
    } 
    
    return true;
}

bool OpenWireCheck(void)
{
    if(en_afe != EN_AFE_IDLE)
    {
        LOG_DEBUG(LOG_TAG, "AFE is Busy!!! OpenWireCheck miss!!!\r\n");
        return false;
    }
    else
    {
        en_afe = EN_AFE_OPENWIRE_CHECK;    
    } 
    
    return true;    
}
en_afe_state AFEStatusGet(void)
{    
    return en_afe;
}

static void read_cfg(void)
{
    adBmsReadData(TOTAL_IC, &IC[0], (uint8_t*)RDCFGB, Config, B);   
    printReadConfig(TOTAL_IC, &IC[0], Config, B);    
}

static void _irq_process(void)
{
    static uint16_t count = 0;
    static uint8_t blinkflag = 0;
    static uint8_t soc_ampere_flag = 0;
    static int32_t current_sum_50_times = 0;
    static uint8_t sampcount = 0;
    static uint8_t openwireflag = 0;
    uint8_t temp = 0;
    
    if(gFirst_AFE_Coved_Flag == 8)
    {
        /* 每10次电流采样后进行断线检测 */
        if(sampcount == 5)
//        if(0)
        {
            if(count > 10)
            {
                OpenWireCheck();
                sampcount = 0;
                openwireflag = 1;
            }
            count ++;
            return;
        }
        else
        {
            count ++;
            /* 开始断线检测 */
            if(openwireflag == 1)
            {
                /* 一个周期时间到了，count清零，重新开始电流采样 */
                if(count == 900)
                {
                    count = 0;
                    openwireflag = 0;
                }
                /* 断线检测一个周期没到，返回停止采样 */
                else
                {
                    return;
                }                                
            }
        }            
             
        if(count % 30 == 0)
        {                    
            if(count == 900)
            {
                count = 0;
                soc_ampere_flag = 1;
            }
            CurrentGet();
            current_sum_50_times += gCell_Info.cell_current;            
        }
        else if(count == 10 || count == 100 || count == 220 || count == 310 || count == 400 || count == 490 || count == 610 || count == 700 || count == 790 || count == 880)
        {
            ClearBalance();
        }
        else if(count == 15 || count == 105 || count == 225 || count == 315 || count == 405 || count == 495 || count == 615 || count == 705 || count == 795 || count == 885)
        {
            VotageCurrentGet();
        }            
        else if(count == 160)
        {
            TemperatureGet_T0();
        }
        else if(count == 250)
        {
            TemperatureGet_T1();
        }
        else if(count == 460)
        {
            TemperatureGet_T2();
        }
        else if(count == 640)
        {
            TemperatureGet_T3();
        }
        else if(count == 820)
        {
            TemperatureRead();
        }
        
        if(soc_ampere_flag)
        {
            soc_ampere_flag = 0;
            if(gSoxDebugParam.sox_debug_mode == 0)
            {
                gPackStatus.pack_current = current_sum_50_times / 30;
                current_sum_50_times = 0;
                SOCAmpereHourIntegral(900, &gPackStatus);
            }
            else if(gSoxDebugParam.sox_debug_mode == 1)
            { 
                gPackStatus_debug.pack_current = gSoxDebugParam.cell_current_debug;
//                gPackStatus_debug.pack_current = -gCell_Info.cell_current;
                SOCAmpereHourIntegral(900, &gPackStatus_debug);
            }
            else if(gSoxDebugParam.sox_debug_mode == 2)
            {
                gPackStatus.pack_current = current_sum_50_times / 30;
                SOCAmpereHourIntegral(900, &gPackStatus);
                gPackStatus_debug.pack_current = -gSoxDebugParam.cell_current_debug;
                SOCAmpereHourIntegral(900, &gPackStatus_debug);
            }
            count = 0;
            sampcount ++;
        }
    }    
}

bool AFEFirstSample(void)
{
    static uint8_t filter_count = 0;
    static uint32_t max_vol = 0;
    static uint32_t min_vol = 0;
    static uint32_t ave_vol = 0;
    static uint16_t max_cur_ref = 0;
    static uint16_t min_cur_ref = 0;
    static uint16_t ave_cur_ref = 0;
    
    /* 确保AFE第一次采集完数据后再进入故障解析 */
    if(gFirst_AFE_Coved_Flag > 7)
    {
        return true;
    }
    if(gFirst_AFE_Coved_Flag == 0)
    {
        if(AFEStatusGet() == EN_AFE_IDLE)
        {                
            VotageCurrentGet();                          
            if(filter_count == 0)
            {
                max_vol = gCell_Info.total_cell_voltage;
                min_vol = gCell_Info.total_cell_voltage;
            }
            
            if(gCell_Info.total_cell_voltage > max_vol)
            {
                max_vol = gCell_Info.total_cell_voltage;
            }
            
            if(gCell_Info.total_cell_voltage < min_vol)
            {
                min_vol = gCell_Info.total_cell_voltage;
            }
            ave_vol += gCell_Info.total_cell_voltage;
            filter_count ++;
        }    

        if(filter_count > 5)
        {
            gFirst_AFE_Coved_Flag = 1;
            filter_count = 0;
            gCell_Info.bat_init_vol = (ave_vol - min_vol - max_vol) / 4;    
        }                
    }

    if(gFirst_AFE_Coved_Flag == 1)
    {                         
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            CurrentGet();
            if(filter_count == 0)
            {
                max_cur_ref = gCell_Info.cur_ref;
                min_cur_ref = gCell_Info.cur_ref;
            }
            
            if(gCell_Info.cur_ref > max_cur_ref)
            {
                max_cur_ref = gCell_Info.cur_ref;
            }
            
            if(gCell_Info.cur_ref < min_cur_ref)
            {
                min_cur_ref = gCell_Info.cur_ref;
            }
            ave_cur_ref += gCell_Info.cur_ref;
            filter_count ++;
        }    

        if(filter_count > 5)
        {
            gFirst_AFE_Coved_Flag = 2;
            gINA240Ref = (ave_cur_ref - max_cur_ref - min_cur_ref) / 4;    
        }           
    }
    
    if(gFirst_AFE_Coved_Flag == 2)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 3;
            TemperatureGet_T0();
        }   
    }
    
    if(gFirst_AFE_Coved_Flag == 3)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 4;
            TemperatureGet_T1();
        }   
    }
    
    if(gFirst_AFE_Coved_Flag == 4)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 5;
            TemperatureGet_T2();
        }   
    }
    
    if(gFirst_AFE_Coved_Flag == 5)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 6;
            TemperatureGet_T3();
        }   
    }
    
    if(gFirst_AFE_Coved_Flag == 6)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
            gFirst_AFE_Coved_Flag = 7;
            TemperatureRead();
        }   
    }
    
    if(gFirst_AFE_Coved_Flag == 7)
    {                       
        if(AFEStatusGet() == EN_AFE_IDLE)
        {
//            gFirst_AFE_Coved_Flag = 8;
            return true;
        }   
    }
    
    return false;
}

void StopAFESample(void)
{
    gFirst_AFE_Coved_Flag = 9;
}
void StartAFESample(void)
{
    gFirst_AFE_Coved_Flag = 8;
}
static void cellv_debug(void)
{
    uint32_t pladc_count = 0;
    int32_t temp;
    
    pladc_count = tim_get_systime();
    adBmsWakeupIc(TOTAL_IC);
    adBms6830_Adcv(REDUNDANT_MEASUREMENT, CONTINUOUS_MEASUREMENT, DISCHARGE_PERMITTED, RESET_FILTER, CELL_OPEN_WIRE_DETECTION);
    adBmsPollAdcWait((uint8_t*)PLADC); 
//    LOG_DEBUG(LOG_TAG, "EN_AFE_BUSY_T1_time:%dms\r\n", pladc_count);     

//    IC[0].tx_cfgb.dcc = 0xFFFF; 
//    IC[0].tx_cfgb.vuv = gCell_Info.cell_min_voltage;                      
//    adBmsWakeupIc(TOTAL_IC);
//    SetConfigB_DischargeTimeOutValue(TOTAL_IC, &IC[0], RANG_0_TO_63_MIN, TIME_2MIN_OR_0_53HR);
////            adBms6830_AllDccStart();
//    adBmsWriteData(TOTAL_IC, &IC[0], WRCFGB, Config, B);    
    
//    adBmsWakeupIc(TOTAL_IC);                  
    adBmsWriteData(TOTAL_IC, &IC[0], (uint8_t*)WRCFGA, Config, A);
//    AUX_CH_TO_CONVERT = GPIO1;             
//    adBms6830_Adax(AUX_OPEN_WIRE_DETECTION, OPEN_WIRE_CURRENT_SOURCE, AUX_CH_TO_CONVERT);
       
//    adBmsPollAdcWait(PLADC); 
//    adBmsReadData(TOTAL_IC, &IC[0], RDAUXA, Aux, A);
//    temp = getMVoltage(IC[0].aux.a_codes[0]);
//    gCell_Info.cell_current = (temp - INA240_REF)  * 80;   // I(mA) = U(mV) / 0.000625(ohm) /20(增益)    
    
    pladc_count = tim_get_systime() - pladc_count;
    LOG_DEBUG(LOG_TAG, "conv_time:%dms\r\n", pladc_count);
}

MSH_CMD_EXPORT(cellv_debug, AFE);
MSH_CMD_EXPORT(VotageCurrentGet, AFE);
MSH_CMD_EXPORT(CurrentGet, AFE);
//MSH_CMD_EXPORT(TemperatureGet, AFE);
MSH_CMD_EXPORT(read_cfg, AFE);
MSH_CMD_EXPORT(SetPassiveBalance, cell 0~15);
MSH_CMD_EXPORT(ClearPassiveBalance, cell 0~15);
FINSH_CMD_EXPORT(SetBalanceCurrent, cell 0~15);

