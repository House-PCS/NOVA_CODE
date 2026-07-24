/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        ADBMS181x.cpp
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_adbms6830.h"

#define LOG_TAG     "adbms6830"
#define WAKEUP_DELAY 1                          /* BMS ic wakeup delay  */
/**
 *******************************************************************************
 * Function: Delay_ms
 * @brief Delay mili second
 *
 * @details This function insert delay in ms.
 *
 * Parameters:
 * @param [in]  delay   Delay_ms
 *
 * @return None
 *
 *******************************************************************************
*/
static void Delay_ms(uint32_t delay)
{
    soft_delayms(delay);
}

/**
 *******************************************************************************
 * Function: adBmsCsLow
 * @brief Select chip select low
 *
 * @details This function does spi chip select low.
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsCsLow()
{
    GpioBitReset((&Cs_adbms6830_Handle));
}

/**
 *******************************************************************************
 * Function: adBmsCsHigh
 * @brief Select chip select High
 *
 * @details This function does spi chip select high.
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsCsHigh()
{
    GpioBitSet((&Cs_adbms6830_Handle));
}

/**
 *******************************************************************************
 * Function: spiWriteBytes
 * @brief Writes an array of bytes out of the SPI port.
 *
 * @details This function wakeup bms ic in IsoSpi mode send dumy byte data in spi line..
 *
 * @param [in]  size            Numberof bytes to be send on the SPI line
 *
 * @param [in]  *tx_Data    Tx data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
static void spiWriteBytes
(
    uint16_t size,                     /*Option: Number of bytes to be written on the SPI port*/
    uint8_t *tx_Data                       /*Array of bytes to be written on the SPI port*/
)
{
    Spi_adbms6830_Handle.pTxBuf = tx_Data;
    Spi_adbms6830_Handle.tx_size = size;
    SpiWrite8(&Spi_adbms6830_Handle);
}

/**
 *******************************************************************************
 * Function: spiWriteReadBytes
 * @brief Writes and read a set number of bytes using the SPI port.
 *
 * @details This function writes and read a set number of bytes using the SPI port.
 *
 * @param [in]  *tx_data    Tx data pointer
 *
 * @param [in]  *rx_data    Rx data pointer
 *
 * @param [in]  size            Data size
 *
 * @return None
 *
 *******************************************************************************
*/
static void spiWriteReadBytes
(
    uint8_t *tx_data,                   /*array of data to be written on SPI port*/
    uint8_t *rx_data,                   /*Input: array that will store the data read by the SPI port*/
    uint16_t size                           /*Option: number of bytes*/
)
{
    Spi_adbms6830_Handle.pTxBuf  = tx_data;
    Spi_adbms6830_Handle.tx_size = 4;
    Spi_adbms6830_Handle.pRxBuf  = rx_data;
    Spi_adbms6830_Handle.rx_size = size;

    SpiWrite8(&Spi_adbms6830_Handle);
    SpiRead8(&Spi_adbms6830_Handle);
}

/**
 *******************************************************************************
 * Function: spiReadBytes
 * @brief Read number of bytes using the SPI port.
 *
 * @details This function Read a set number of bytes using the SPI port.
 *
 * @param [in]  size            Data size
 *
 * @param [in]  *rx_data    Rx data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void spiReadBytes(uint16_t size, uint8_t *rx_data)
{
    Spi_adbms6830_Handle.pRxBuf  = rx_data;
    Spi_adbms6830_Handle.rx_size = size;

    SpiRead8(&Spi_adbms6830_Handle);
}

/**
 *******************************************************************************
 * Function: adBmsWakeupIc
 * @brief Wakeup bms ic using chip select
 *
 * @details This function wakeup thr bms ic using chip select.
 *
 * @param [in]  total_ic    Total_ic
 *
 * @return None
 *
 *******************************************************************************
*/
void adBmsWakeupIc(uint8_t total_ic)
{
    for (uint8_t ic = 0; ic < total_ic; ic++)
    {
        adBmsCsLow();
//        Delay_ms(WAKEUP_DELAY);
        soft_delayus(10);
        adBmsCsHigh();
//        Delay_ms(WAKEUP_DELAY);
        soft_delayus(10);
    }
}

/**
 *******************************************************************************
 * Function: SetOverVoltageThreshold
 * @brief Set Over Voltage Threshold.
 *
 * @details This function Set Over Voltage Threshold.
 *
 * Parameters:
 *
 * @param [in]  voltage      Over voltage
 *
 * @return OverVoltage_value
 *
 *******************************************************************************
*/
uint16_t SetOverVoltageThreshold(float voltage)
{
  uint16_t vov_value;
  uint8_t rbits = 12;
  voltage = (voltage - 1.5f);
  voltage = voltage / (16 * 0.000150);
  vov_value = (uint16_t )(voltage + 2 * (1 << (rbits - 1)));
  vov_value &= 0xFFF;
  return vov_value;
}

/**
 *******************************************************************************
 * Function: SetUnderVoltageThreshold
 * @brief Set Under Voltage Threshold.
 *
 * @details This function Set Under Voltage Threshold.
 *
 * Parameters:
 *
 * @param [in]  voltage      Under voltage
 *
 * @return UnderVoltage_value
 *
 *******************************************************************************
*/
uint16_t SetUnderVoltageThreshold(float voltage)
{
  uint16_t vuv_value;
  uint8_t rbits = 12;
  voltage = (voltage - 1.5f);
  voltage = voltage / (16 * 0.000150);
  vuv_value = (uint16_t )(voltage + 2 * (1 << (rbits - 1)));
  vuv_value &= 0xFFF;
  return vuv_value;
}

/**
 *******************************************************************************
 * Function: ConfigA_Flag
 * @brief Config A Flag bit.
 *
 * @details This function Set configuration A flag bit.
 *
 * Parameters:
 *
 * @param [in]  flag_d      Enum type flag bit
 *
 * @param [in]  flag       Enum type set or clr flag
 *
 * @return Flagbit_Value
 *
 *******************************************************************************
*/
uint8_t ConfigA_Flag(FLAG_D flag_d, CFGA_FLAG flag)
{
  uint8_t flag_value;
  if(flag == FLAG_SET)
  {
    flag_value = (1 << flag_d);
  }
  else
  {
    flag_value = (0 << flag_d);
  }
  return(flag_value);
}

/**
 *******************************************************************************
 * Function: ConfigA_Gpo
 * @brief Config A Gpio Pull High/Low.
 *
 * @details This function Set configuration A gpio as pull high/Low.
 *
 * Parameters:
 *
 * @param [in]  gpo      Enum type GPO Pin
 *
 * @param [in]  stat     Enum type CFGAGPO set Low or High
 *
 * @return Gpio_value
 *
 *******************************************************************************
*/
uint16_t ConfigA_Gpo(GPO gpo, CFGA_GPO stat)
{
  uint16_t gpovalue;
  if(stat == GPO_SET)
  {
    gpovalue = (1 << gpo);
  }
  else
  {
    gpovalue = (0 << gpo);
  }
  return(gpovalue);
}

/**
 *******************************************************************************
 * Function: ConfigB_DccBit
 * @brief Config B DCC bit.
 *
 * @details This function configure config B DCC bit.
 *
 * Parameters:
 *
 * @param [in]  dcc      Enum type DCC bit
 *
 * @param [in]  dccbit   Enum type DCC bit set or clr.
 *
 * @return DccBit_value
 *
 *******************************************************************************
*/
uint16_t ConfigB_DccBit(DCC dcc, DCC_BIT dccbit)
{
  uint16_t dccvalue;
  if(dccbit == DCC_BIT_SET)
  {
    dccvalue = (1 << dcc);
  }
  else
  {
    dccvalue = (0 << dcc);
  }
  return(dccvalue);
}

/**
 *******************************************************************************
 * Function: SetConfig_B_DischargeTimeOutValue
 * @brief Set Config B Discharge Time Out Value.
 *
 * @details This function Set configuration B Discharge Time Out Value with short or long range.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                      cell_asic ic structure pointer
 *
 * @param [in]  timer_rang              Enum type time range
 *
 * @param [in]  timeout_value           Enum type time out value
 *
 * @return None
 *
 *******************************************************************************
*/
void SetConfigB_DischargeTimeOutValue(uint8_t tIC, cell_asic *ic, DTRNG timer_rang, DCTO timeout_value)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].tx_cfgb.dtrng = timer_rang;
    if(timer_rang == RANG_0_TO_63_MIN)
    {
      ic[curr_ic].tx_cfgb.dcto = timeout_value;
    }
    else if(timer_rang == RANG_0_TO_16_8_HR)
    {
      ic[curr_ic].tx_cfgb.dcto = timeout_value;
    }
  }
}

/**
 *******************************************************************************
 * Function: SetPwmDutyCycle
 * @brief Set PWM discharges duty cycle.
 *
 * @details This function Set PWM discharges duty cycle.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  duty_cycle              Enum type PWM duty cycle value
 *
 * @return None
 *
 *******************************************************************************
*/
void SetPwmDutyCycle(uint8_t tIC, cell_asic *ic, PWM_DUTY duty_cycle)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    for(uint8_t pwmc = 0; pwmc < PWMA; pwmc++)
    {
     ic[curr_ic].PwmA.pwma[pwmc] = duty_cycle;
    }

    for(uint8_t pwmc = 0; pwmc < PWMB; pwmc++)
    {
     ic[curr_ic].PwmB.pwmb[pwmc] = duty_cycle;
    }
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseConfiga
 * @brief Parse the recived Configuration register A data
 *
 * @details This function Parse the recived Configuration register A data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                      cell_asic ic structure pointer
 *
 * @param [in]  *data                    Data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseConfiga(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].configa.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));

    ic[curr_ic].rx_cfga.cth = (ic[curr_ic].configa.rx_data[0] & 0x07);
    ic[curr_ic].rx_cfga.refon   = (ic[curr_ic].configa.rx_data[0] & 0x80) >> 7;

    ic[curr_ic].rx_cfga.flag_d  = (ic[curr_ic].configa.rx_data[1] & 0xFF);

    ic[curr_ic].rx_cfga.soakon   = (ic[curr_ic].configa.rx_data[2] & 0x80) >> 7;
    ic[curr_ic].rx_cfga.owrng    = (((ic[curr_ic].configa.rx_data[2] & 0x40) >> 6));
    ic[curr_ic].rx_cfga.owa    = ( (ic[curr_ic].configa.rx_data[2] & 0x38) >> 3);

    ic[curr_ic].rx_cfga.gpo        = ( (ic[curr_ic].configa.rx_data[3] & 0xFF)| ((ic[curr_ic].configa.rx_data[4] & 0x03) << 8) );

    ic[curr_ic].rx_cfga.snap   = ((ic[curr_ic].configa.rx_data[5] & 0x20) >> 5);
    ic[curr_ic].rx_cfga.mute_st   = ((ic[curr_ic].configa.rx_data[5] & 0x10) >> 4);
    ic[curr_ic].rx_cfga.comm_bk   = ((ic[curr_ic].configa.rx_data[5] & 0x08) >> 3);
    ic[curr_ic].rx_cfga.fc   = ((ic[curr_ic].configa.rx_data[5] & 0x07) >> 0);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseConfigb
 * @brief Parse the recived Configuration register B data
 *
 * @details This function Parse the recived Configuration register B data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   Data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseConfigb(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].configb.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));

    ic[curr_ic].rx_cfgb.vuv = ((ic[curr_ic].configb.rx_data[0])  | ((ic[curr_ic].configb.rx_data[1] & 0x0F) << 8));
    ic[curr_ic].rx_cfgb.vov  = (ic[curr_ic].configb.rx_data[2]<<4)+((ic[curr_ic].configb.rx_data[1] &0xF0)>>4)  ;
    ic[curr_ic].rx_cfgb.dtmen = (((ic[curr_ic].configb.rx_data[3] & 0x80) >> 7));
    ic[curr_ic].rx_cfgb.dtrng= ((ic[curr_ic].configb.rx_data[3] & 0x40) >> 6);
    ic[curr_ic].rx_cfgb.dcto   = ((ic[curr_ic].configb.rx_data[3] & 0x3F));
    ic[curr_ic].rx_cfgb.dcc = ((ic[curr_ic].configb.rx_data[4]) | ((ic[curr_ic].configb.rx_data[5] & 0xFF) << 8));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseConfig
 * @brief Parse the recived Configuration register A & B data
 *
 * @details This function Parse the recived Configuration register A & B data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *data                   Data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseConfig(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *data)
{
  switch (grp)
  {
  case A:
    adBms6830ParseConfiga(tIC, &ic[0], &data[0]);
    break;

  case B:
    adBms6830ParseConfigb(tIC, &ic[0], &data[0]);
    break;

  default:
    break;
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseCell
 * @brief Parse cell voltages
 *
 * @details This function Parse the recived cell register A,B,C,D,E and F data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *cv_data                Cell voltage data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
/* Parse cell voltages */
void adBms6830ParseCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *cv_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = RDCVALL_SIZE;}
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    LOG_ERROR(LOG_TAG, " Failed to allocate parse cell memory \r\n");
    return;
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &cv_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* Cell Register group A */
      ic[curr_ic].cell.c_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* Cell Register group B */
      ic[curr_ic].cell.c_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* Cell Register group C */
      ic[curr_ic].cell.c_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* Cell Register group D */
      ic[curr_ic].cell.c_codes[9] =  (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[10] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[11] = (data[4] + (data[5] << 8));
      break;

    case E: /* Cell Register group E */
      ic[curr_ic].cell.c_codes[12] = (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[13] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[14] = (data[4] + (data[5] << 8));
      break;

    case F: /* Cell Register group F */
#if(CELL==16)
      ic[curr_ic].cell.c_codes[15] = (data[0] + (data[1] << 8));
#endif
      break;

    case ALL_GRP: /* Cell Register group ALL */
      ic[curr_ic].cell.c_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].cell.c_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].cell.c_codes[2] = (data[4] + (data[5] << 8));
      ic[curr_ic].cell.c_codes[3] = (data[6] + (data[7] << 8));
      ic[curr_ic].cell.c_codes[4] = (data[8] + (data[9] << 8));
      ic[curr_ic].cell.c_codes[5] = (data[10] + (data[11] << 8));
      ic[curr_ic].cell.c_codes[6] = (data[12] + (data[13] << 8));
      ic[curr_ic].cell.c_codes[7] = (data[14] + (data[15] << 8));
      ic[curr_ic].cell.c_codes[8] = (data[16] + (data[17] << 8));
      ic[curr_ic].cell.c_codes[9] =  (data[18] + (data[19] << 8));
      ic[curr_ic].cell.c_codes[10] = (data[20] + (data[21] << 8));
      ic[curr_ic].cell.c_codes[11] = (data[22] + (data[23] << 8));
      ic[curr_ic].cell.c_codes[12] = (data[24] + (data[25] << 8));
      ic[curr_ic].cell.c_codes[13] = (data[26] + (data[27] << 8));
      ic[curr_ic].cell.c_codes[14] = (data[28] + (data[29] << 8));
#if(CELL==16)
      ic[curr_ic].cell.c_codes[15] = (data[30] + (data[31] << 8));
#endif
      break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseAverageCell
 * @brief Parse average cell voltages
 *
 * @details This function Parse the recived average cell register A,B,C,D,E and F data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *acv_data               Average cell voltage data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseAverageCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *acv_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = RDACALL_SIZE;}
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    LOG_ERROR(LOG_TAG, " Failed to allocate parse avg cell memory \r\n");
    return;
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &acv_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* Cell Register group A */
      ic[curr_ic].acell.ac_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* Cell Register group B */
      ic[curr_ic].acell.ac_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* Cell Register group C */
      ic[curr_ic].acell.ac_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* Cell Register group D */
      ic[curr_ic].acell.ac_codes[9] =  (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[10] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[11] = (data[4] + (data[5] << 8));
      break;

    case E: /* Cell Register group E */
      ic[curr_ic].acell.ac_codes[12] = (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[13] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[14] = (data[4] + (data[5] << 8));
      break;

    case F: /* Cell Register group F */

#if(CELL==16)
      ic[curr_ic].acell.ac_codes[15] = (data[0] + (data[1] << 8));
#endif
      break;

    case ALL_GRP: /* Cell Register group ALL */
      ic[curr_ic].acell.ac_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].acell.ac_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].acell.ac_codes[2] = (data[4] + (data[5] << 8));
      ic[curr_ic].acell.ac_codes[3] = (data[6] + (data[7] << 8));
      ic[curr_ic].acell.ac_codes[4] = (data[8] + (data[9] << 8));
      ic[curr_ic].acell.ac_codes[5] = (data[10] + (data[11] << 8));
      ic[curr_ic].acell.ac_codes[6] = (data[12] + (data[13] << 8));
      ic[curr_ic].acell.ac_codes[7] = (data[14] + (data[15] << 8));
      ic[curr_ic].acell.ac_codes[8] = (data[16] + (data[17] << 8));
      ic[curr_ic].acell.ac_codes[9] =  (data[18] + (data[19] << 8));
      ic[curr_ic].acell.ac_codes[10] = (data[20] + (data[21] << 8));
      ic[curr_ic].acell.ac_codes[11] = (data[22] + (data[23] << 8));
      ic[curr_ic].acell.ac_codes[12] = (data[24] + (data[25] << 8));
      ic[curr_ic].acell.ac_codes[13] = (data[26] + (data[27] << 8));
      ic[curr_ic].acell.ac_codes[14] = (data[28] + (data[29] << 8));
#if(CELL==16)
      ic[curr_ic].acell.ac_codes[15] = (data[30] + (data[31] << 8));
#endif
      break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseSCell
 * @brief Parse S cell voltages
 *
 * @details This function Parse the recived S cell register A,B,C,D,E and F data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *scv_data               S cell voltage data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
/* Parse S cell voltages */
void adBms6830ParseSCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *scv_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = RDSALL_SIZE;}
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    LOG_ERROR(LOG_TAG, " Failed to allocate parse scv memory \r\n");
    return;
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &scv_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* Cell Register group A */
      ic[curr_ic].scell.sc_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* Cell Register group B */
      ic[curr_ic].scell.sc_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* Cell Register group C */
      ic[curr_ic].scell.sc_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* Cell Register group D */
      ic[curr_ic].scell.sc_codes[9] =  (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[10] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[11] = (data[4] + (data[5] << 8));
      break;

    case E: /* Cell Register group E */
      ic[curr_ic].scell.sc_codes[12] = (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[13] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[14] = (data[4] + (data[5] << 8));
      break;

    case F: /* Cell Register group F */

#if(CELL==16)
      ic[curr_ic].scell.sc_codes[15] = (data[0] + (data[1] << 8));
#endif
      break;

    case ALL_GRP: /* Cell Register group ALL */
      ic[curr_ic].scell.sc_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].scell.sc_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].scell.sc_codes[2] = (data[4] + (data[5] << 8));
      ic[curr_ic].scell.sc_codes[3] = (data[6] + (data[7] << 8));
      ic[curr_ic].scell.sc_codes[4] = (data[8] + (data[9] << 8));
      ic[curr_ic].scell.sc_codes[5] = (data[10] + (data[11] << 8));
      ic[curr_ic].scell.sc_codes[6] = (data[12] + (data[13] << 8));
      ic[curr_ic].scell.sc_codes[7] = (data[14] + (data[15] << 8));
      ic[curr_ic].scell.sc_codes[8] = (data[16] + (data[17] << 8));
      ic[curr_ic].scell.sc_codes[9] = (data[18] + (data[19] << 8));
      ic[curr_ic].scell.sc_codes[10] = (data[20] + (data[21] << 8));
      ic[curr_ic].scell.sc_codes[11] = (data[22] + (data[23] << 8));
      ic[curr_ic].scell.sc_codes[12] = (data[24] + (data[25] << 8));
      ic[curr_ic].scell.sc_codes[13] = (data[26] + (data[27] << 8));
      ic[curr_ic].scell.sc_codes[14] = (data[28] + (data[29] << 8));
#if(CELL==16)
      ic[curr_ic].scell.sc_codes[15] = (data[30] + (data[31] << 8));
#endif
      break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseFCell
 * @brief Parse Filtered cell voltages
 *
 * @details This function Parse the recived filtered cell register A,B,C,D,E and F data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *fcv_data               Filtered cell voltage data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseFCell(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *fcv_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = RDFCALL_SIZE;}
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    LOG_ERROR(LOG_TAG, " Failed to allocate parse fcell memory \r\n");
    return;
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &fcv_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* Cell Register group A */
      ic[curr_ic].fcell.fc_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* Cell Register group B */
      ic[curr_ic].fcell.fc_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* Cell Register group C */
      ic[curr_ic].fcell.fc_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* Cell Register group D */
      ic[curr_ic].fcell.fc_codes[9] =  (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[10] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[11] = (data[4] + (data[5] << 8));
      break;

    case E: /* Cell Register group E */
      ic[curr_ic].fcell.fc_codes[12] = (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[13] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[14] = (data[4] + (data[5] << 8));
      break;

    case F: /* Cell Register group F */
#if(CELL==16)
      ic[curr_ic].fcell.fc_codes[15] = (data[0] + (data[1] << 8));
#endif
      break;

    case ALL_GRP: /* Cell Register group ALL */
      ic[curr_ic].fcell.fc_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].fcell.fc_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].fcell.fc_codes[2] = (data[4] + (data[5] << 8));
      ic[curr_ic].fcell.fc_codes[3] = (data[6] + (data[7] << 8));
      ic[curr_ic].fcell.fc_codes[4] = (data[8] + (data[9] << 8));
      ic[curr_ic].fcell.fc_codes[5] = (data[10] + (data[11] << 8));
      ic[curr_ic].fcell.fc_codes[6] = (data[12] + (data[13] << 8));
      ic[curr_ic].fcell.fc_codes[7] = (data[14] + (data[15] << 8));
      ic[curr_ic].fcell.fc_codes[8] = (data[16] + (data[17] << 8));
      ic[curr_ic].fcell.fc_codes[9] =  (data[18] + (data[19] << 8));
      ic[curr_ic].fcell.fc_codes[10] = (data[20] + (data[21] << 8));
      ic[curr_ic].fcell.fc_codes[11] = (data[22] + (data[23] << 8));
      ic[curr_ic].fcell.fc_codes[12] = (data[24] + (data[25] << 8));
      ic[curr_ic].fcell.fc_codes[13] = (data[26] + (data[27] << 8));
      ic[curr_ic].fcell.fc_codes[14] = (data[28] + (data[29] << 8));
#if(CELL==16)
      ic[curr_ic].fcell.fc_codes[15] = (data[30] + (data[31] << 8));
#endif
      break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseAux
 * @brief Parse aux voltages
 *
 * @details This function Parse the recived aux voltages register A,B,C and D data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *aux_data               Aux voltages data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseAux(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *aux_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = (RDASALL_SIZE-44);}  /* RDASALL_SIZE 68 byte - (RAUX 20 byte + STATUS 24 byte) */
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {
    LOG_ERROR(LOG_TAG, " Failed to allocate parse aux memory \r\n");
    return;
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &aux_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* Aux Register group A */
      ic[curr_ic].aux.a_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].aux.a_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].aux.a_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* Aux Register group B */
      ic[curr_ic].aux.a_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].aux.a_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].aux.a_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* Aux Register group C */
      ic[curr_ic].aux.a_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].aux.a_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].aux.a_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* Aux Register group D */
      ic[curr_ic].aux.a_codes[9] =  (data[0] + (data[1] << 8));
      ic[curr_ic].aux.a_codes[10] =  (data[2] + (data[3] << 8));
      ic[curr_ic].aux.a_codes[11] =  (data[4] + (data[5] << 8));
      break;

   case ALL_GRP: /* Aux Register group ALL */
      ic[curr_ic].aux.a_codes[0]  = (data[0] + (data[1] << 8));
      ic[curr_ic].aux.a_codes[1]  = (data[2] + (data[3] << 8));
      ic[curr_ic].aux.a_codes[2]  = (data[4] + (data[5] << 8));
      ic[curr_ic].aux.a_codes[3]  = (data[6] + (data[7] << 8));
      ic[curr_ic].aux.a_codes[4]  = (data[8] + (data[9] << 8));
      ic[curr_ic].aux.a_codes[5]  = (data[10] + (data[11] << 8));
      ic[curr_ic].aux.a_codes[6]  = (data[12] + (data[13] << 8));
      ic[curr_ic].aux.a_codes[7]  = (data[14] + (data[15] << 8));
      ic[curr_ic].aux.a_codes[8]  = (data[16] + (data[17] << 8));
      ic[curr_ic].aux.a_codes[9]  = (data[18] + (data[19] << 8));
      ic[curr_ic].aux.a_codes[10] = (data[20] + (data[21] << 8));
      ic[curr_ic].aux.a_codes[11] = (data[22] + (data[23] << 8));
     break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseRAux
 * @brief Parse raux voltages
 *
 * @details This function Parse the recived raux voltages register A,B,C and D data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  grp                     Enum type register group
 *
 * @param [in]  *raux_data              Raux voltages data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseRAux(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *raux_data)
{
  uint8_t *data, data_size, address = 0;
  if(grp == ALL_GRP){data_size = (RDASALL_SIZE-48);}  /* RDASALL_SIZE 68 byte - (AUX 24 byte + STATUS 24 byte) */
  else {data_size = RX_DATA;}
  data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
  if(data == NULL)
  {

    LOG_ERROR(LOG_TAG, " Failed to allocate parse raux memory \r\n");
    return;
  }
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&data[0], &raux_data[address], data_size); /* dst , src , size */
    address = ((curr_ic+1) * (data_size));
    switch (grp)
    {
    case A: /* RAux Register group A */
      ic[curr_ic].raux.ra_codes[0] = (data[0] + (data[1] << 8));
      ic[curr_ic].raux.ra_codes[1] = (data[2] + (data[3] << 8));
      ic[curr_ic].raux.ra_codes[2] = (data[4] + (data[5] << 8));
      break;

    case B: /* RAux Register group B */
      ic[curr_ic].raux.ra_codes[3] = (data[0] + (data[1] << 8));
      ic[curr_ic].raux.ra_codes[4] = (data[2] + (data[3] << 8));
      ic[curr_ic].raux.ra_codes[5] = (data[4] + (data[5] << 8));
      break;

    case C: /* RAux Register group C */
      ic[curr_ic].raux.ra_codes[6] = (data[0] + (data[1] << 8));
      ic[curr_ic].raux.ra_codes[7] = (data[2] + (data[3] << 8));
      ic[curr_ic].raux.ra_codes[8] = (data[4] + (data[5] << 8));
      break;

    case D: /* RAux Register group D */
      ic[curr_ic].raux.ra_codes[9] =  (data[0] + (data[1] << 8));
      break;

    case ALL_GRP: /* RAux Register group ALL */
      ic[curr_ic].raux.ra_codes[0]  = (data[0] + (data[1] << 8));
      ic[curr_ic].raux.ra_codes[1]  = (data[2] + (data[3] << 8));
      ic[curr_ic].raux.ra_codes[2]  = (data[4] + (data[5] << 8));
      ic[curr_ic].raux.ra_codes[3]  = (data[6] + (data[7] << 8));
      ic[curr_ic].raux.ra_codes[4]  = (data[8] + (data[9] << 8));
      ic[curr_ic].raux.ra_codes[5]  = (data[10] + (data[11] << 8));
      ic[curr_ic].raux.ra_codes[6]  = (data[12] + (data[13] << 8));
      ic[curr_ic].raux.ra_codes[7]  = (data[14] + (data[15] << 8));
      ic[curr_ic].raux.ra_codes[8]  = (data[16] + (data[17] << 8));
      ic[curr_ic].raux.ra_codes[9]  = (data[18] + (data[19] << 8));
     break;

    default:
      break;
    }
  }
  free(data);
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatusA
 * @brief Parse status A register data
 *
 * @details This function Parse the recived status A register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatusA(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].stat.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].stata.vref2   = (ic[curr_ic].stat.rx_data[0] | (ic[curr_ic].stat.rx_data[1] << 8));
    ic[curr_ic].stata.itmp = (ic[curr_ic].stat.rx_data[2] | (ic[curr_ic].stat.rx_data[3] << 8));
    ic[curr_ic].stata.vref3   = (ic[curr_ic].stat.rx_data[4] | (ic[curr_ic].stat.rx_data[5] << 8));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatusB
 * @brief Parse status B register data
 *
 * @details This function Parse the recived status B register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatusB(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].stat.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].statb.vd   = (ic[curr_ic].stat.rx_data[0] + (ic[curr_ic].stat.rx_data[1] << 8));
    ic[curr_ic].statb.va = (ic[curr_ic].stat.rx_data[2] + (ic[curr_ic].stat.rx_data[3] << 8));
    ic[curr_ic].statb.vr4k   = (ic[curr_ic].stat.rx_data[4] + (ic[curr_ic].stat.rx_data[5] << 8));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatusC
 * @brief Parse status C register data
 *
 * @details This function Parse the recived status C register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatusC(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].stat.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].statc.cs_flt   = (ic[curr_ic].stat.rx_data[0] + (ic[curr_ic].stat.rx_data[1] << 8));
    ic[curr_ic].statc.otp2_med = (ic[curr_ic].stat.rx_data[4] & 0x01);
    ic[curr_ic].statc.otp2_ed = ((ic[curr_ic].stat.rx_data[4] & 0x02) >> 1);
    ic[curr_ic].statc.otp1_med = ((ic[curr_ic].stat.rx_data[4] & 0x04) >> 2);
    ic[curr_ic].statc.otp1_ed = ((ic[curr_ic].stat.rx_data[4] & 0x08) >> 3);
    ic[curr_ic].statc.vd_uv  = ((ic[curr_ic].stat.rx_data[4] & 0x10) >> 4);
    ic[curr_ic].statc.vd_ov = ((ic[curr_ic].stat.rx_data[4] & 0x20) >> 5);
    ic[curr_ic].statc.va_uv = ((ic[curr_ic].stat.rx_data[4] & 0x40) >> 6);
    ic[curr_ic].statc.va_ov = ((ic[curr_ic].stat.rx_data[4] & 0x80) >> 7);
    ic[curr_ic].statc.oscchk = (ic[curr_ic].stat.rx_data[5] & 0x01);
    ic[curr_ic].statc.tmodchk = ((ic[curr_ic].stat.rx_data[5] & 0x02) >> 1);
    ic[curr_ic].statc.thsd = ((ic[curr_ic].stat.rx_data[5] & 0x04) >> 2);
    ic[curr_ic].statc.sleep = ((ic[curr_ic].stat.rx_data[5] & 0x08) >> 3);
    ic[curr_ic].statc.spiflt  = ((ic[curr_ic].stat.rx_data[5] & 0x10) >> 4);
    ic[curr_ic].statc.comp = ((ic[curr_ic].stat.rx_data[5] & 0x20) >> 5);
    ic[curr_ic].statc.vdel = ((ic[curr_ic].stat.rx_data[5] & 0x40) >> 6);
    ic[curr_ic].statc.vde = ((ic[curr_ic].stat.rx_data[5] & 0x80) >> 7);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatusD
 * @brief Parse status D register data
 *
 * @details This function Parse the recived status D register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatusD(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].stat.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    /* uv, ov bits 1 to 4 status bits */
    ic[curr_ic].statd.c_uv[0] = (ic[curr_ic].stat.rx_data[0] & 0x01);
    ic[curr_ic].statd.c_ov[0] = ((ic[curr_ic].stat.rx_data[0] & 0x02) >> 1);
    ic[curr_ic].statd.c_uv[1] = ((ic[curr_ic].stat.rx_data[0] & 0x04) >> 2);
    ic[curr_ic].statd.c_ov[1] = ((ic[curr_ic].stat.rx_data[0] & 0x08) >> 3);
    ic[curr_ic].statd.c_uv[2] = ((ic[curr_ic].stat.rx_data[0] & 0x10) >> 4);
    ic[curr_ic].statd.c_ov[2] = ((ic[curr_ic].stat.rx_data[0] & 0x20) >> 5);
    ic[curr_ic].statd.c_uv[3] = ((ic[curr_ic].stat.rx_data[0] & 0x40) >> 6);
    ic[curr_ic].statd.c_ov[3] = ((ic[curr_ic].stat.rx_data[0] & 0x80) >> 7);
    /* uv, ov bits 5 to 8 status bits */
    ic[curr_ic].statd.c_uv[4] = (ic[curr_ic].stat.rx_data[1] & 0x01);
    ic[curr_ic].statd.c_ov[4] = ((ic[curr_ic].stat.rx_data[1] & 0x02) >> 1);
    ic[curr_ic].statd.c_uv[5] = ((ic[curr_ic].stat.rx_data[1] & 0x04) >> 2);
    ic[curr_ic].statd.c_ov[5] = ((ic[curr_ic].stat.rx_data[1] & 0x08) >> 3);
    ic[curr_ic].statd.c_uv[6] = ((ic[curr_ic].stat.rx_data[1] & 0x10) >> 4);
    ic[curr_ic].statd.c_ov[6] = ((ic[curr_ic].stat.rx_data[1] & 0x20) >> 5);
    ic[curr_ic].statd.c_uv[7] = ((ic[curr_ic].stat.rx_data[1] & 0x40) >> 6);
    ic[curr_ic].statd.c_ov[7] = ((ic[curr_ic].stat.rx_data[1] & 0x80) >> 7);
    /* uv, ov bits 9 to 12 status bits */
    ic[curr_ic].statd.c_uv[8] = (ic[curr_ic].stat.rx_data[2] & 0x01);
    ic[curr_ic].statd.c_ov[8] = ((ic[curr_ic].stat.rx_data[2] & 0x02) >> 1);
    ic[curr_ic].statd.c_uv[9] = ((ic[curr_ic].stat.rx_data[2] & 0x04) >> 2);
    ic[curr_ic].statd.c_ov[9] = ((ic[curr_ic].stat.rx_data[2] & 0x08) >> 3);
    ic[curr_ic].statd.c_uv[10] = ((ic[curr_ic].stat.rx_data[2] & 0x10) >> 4);
    ic[curr_ic].statd.c_ov[10] = ((ic[curr_ic].stat.rx_data[2] & 0x20) >> 5);
    ic[curr_ic].statd.c_uv[11] = ((ic[curr_ic].stat.rx_data[2] & 0x40) >> 6);
    ic[curr_ic].statd.c_ov[11] = ((ic[curr_ic].stat.rx_data[2] & 0x80) >> 7);
    /* uv, ov bits 13 to 16 status bits */
    ic[curr_ic].statd.c_uv[12] = (ic[curr_ic].stat.rx_data[3] & 0x01);
    ic[curr_ic].statd.c_ov[12] = ((ic[curr_ic].stat.rx_data[3] & 0x02) >> 1);
    ic[curr_ic].statd.c_uv[13] = ((ic[curr_ic].stat.rx_data[3] & 0x04) >> 2);
    ic[curr_ic].statd.c_ov[13] = ((ic[curr_ic].stat.rx_data[3] & 0x08) >> 3);
    ic[curr_ic].statd.c_uv[14] = ((ic[curr_ic].stat.rx_data[3] & 0x10) >> 4);
    ic[curr_ic].statd.c_ov[14] = ((ic[curr_ic].stat.rx_data[3] & 0x20) >> 5);
#if(CELL==16)

    ic[curr_ic].statd.c_uv[15] = ((ic[curr_ic].stat.rx_data[3] & 0x40) >> 6);
    ic[curr_ic].statd.c_ov[15] = ((ic[curr_ic].stat.rx_data[3] & 0x80) >> 7);
#endif
    /* ct and cts */
    ic[curr_ic].statd.cts = (ic[curr_ic].stat.rx_data[4] & 0x03);
    ic[curr_ic].statd.ct = ((ic[curr_ic].stat.rx_data[4] & 0xFC) >> 2);
    /* oc_cntr */
    ic[curr_ic].statd.oc_cntr = (ic[curr_ic].stat.rx_data[5] & 0xFF);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatusE
 * @brief Parse status E register data
 *
 * @details This function Parse the recived status E register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatusE(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].stat.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].state.gpi   = ((ic[curr_ic].stat.rx_data[4] + ((ic[curr_ic].stat.rx_data[5] & 0x03) << 8)));
    ic[curr_ic].state.rev = ((ic[curr_ic].stat.rx_data[5] & 0xF0) >> 4);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseStatus
 * @brief Parse status register data
 *
 * @details This function Parse the recived status register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseStatus(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *data)
{
  uint8_t statc[RX_DATA], state[RX_DATA];
  switch (grp)
  {
    case A: /* Status Register group A */
      adBms6830ParseStatusA(tIC, &ic[0], &data[0]);
      break;

    case B: /* Status Register group B */
      adBms6830ParseStatusB(tIC, &ic[0], &data[0]);
      break;

    case C: /* Status Register group C */
      adBms6830ParseStatusC(tIC, &ic[0], &data[0]);
      break;

    case D: /* Status Register group D */
      adBms6830ParseStatusD(tIC, &ic[0], &data[0]);
      break;

    case E: /* Status Register group E */
      adBms6830ParseStatusE(tIC, &ic[0], &data[0]);
      break;

    case ALL_GRP: /* Status Register group ALL */
      /* Status A base address data[0] index */
      adBms6830ParseStatusA(tIC, &ic[0], &data[0]);
      /* Status B base address data[6] index */
      adBms6830ParseStatusB(tIC, &ic[0], &data[6]);
      /* Status C base address data[12] index */
      statc[0] = data[12];
      statc[1] = data[13];
      statc[4] = data[14];
      statc[5] = data[15];
      adBms6830ParseStatusC(tIC, &ic[0], &statc[0]);
      /* Status D base address data[16] index */
      adBms6830ParseStatusD(tIC, &ic[0], &data[16]);
      /* Status E base address data[22] index */
      state[4] = data[22];
      state[5] = data[23];
      adBms6830ParseStatusE(tIC, &ic[0], &state[0]);
     break;

    default:
      break;
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseComm
 * @brief Parse comm register
 *
 * @details This function Parse the recived comm register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseComm(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].com.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].comm.icomm[0] = ((ic[curr_ic].com.rx_data[0] & 0xF0) >> 4);
    ic[curr_ic].comm.fcomm[0] = (ic[curr_ic].com.rx_data[0] & 0x0F);
    ic[curr_ic].comm.data[0] = (ic[curr_ic].com.rx_data[1]);
    ic[curr_ic].comm.icomm[1] = ((ic[curr_ic].com.rx_data[2] & 0xF0) >> 4);
    ic[curr_ic].comm.data[1] = (ic[curr_ic].com.rx_data[3]);
    ic[curr_ic].comm.fcomm[1] = (ic[curr_ic].com.rx_data[2] & 0x0F);
    ic[curr_ic].comm.icomm[2] = ((ic[curr_ic].com.rx_data[4] & 0xF0) >> 4);
    ic[curr_ic].comm.data[2] = (ic[curr_ic].com.rx_data[5]);
    ic[curr_ic].comm.fcomm[2] = (ic[curr_ic].com.rx_data[4] & 0x0F);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParseSID
 * @brief Parse SID register
 *
 * @details This function Parse the recived sid register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseSID(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].rsid.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].sid.sid[0] = ic[curr_ic].rsid.rx_data[0];
    ic[curr_ic].sid.sid[1] = ic[curr_ic].rsid.rx_data[1];
    ic[curr_ic].sid.sid[2] = ic[curr_ic].rsid.rx_data[2];
    ic[curr_ic].sid.sid[3] = ic[curr_ic].rsid.rx_data[3];
    ic[curr_ic].sid.sid[4] = ic[curr_ic].rsid.rx_data[4];
    ic[curr_ic].sid.sid[5] = ic[curr_ic].rsid.rx_data[5];
  }
}


/**
 *******************************************************************************
 * Function: adBms6830ParseRR
 * @brief Parse SID register
 *
 * @details This function Parse the recived sid register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParseRR(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].retention.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    //ic[curr_ic].RR.rr[0] = ic[curr_ic].retention.rx_data[0];
    //ic[curr_ic].RR.rr[1] = ic[curr_ic].retention.rx_data[1];
    //ic[curr_ic].RR.rr[2] = ic[curr_ic].retention.rx_data[2];
    //ic[curr_ic].RR.rr[3] = ic[curr_ic].retention.rx_data[3];
    //ic[curr_ic].RR.rr[4] = ic[curr_ic].retention.rx_data[4];
    //ic[curr_ic].RR.rr[5] = ic[curr_ic].retention.rx_data[5];
  }
}



/**
 *******************************************************************************
 * Function: adBms6830ParsePwma
 * @brief Parse PWMA register
 *
 * @details This function Parse the recived pwma register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParsePwma(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].pwma.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].PwmA.pwma[0] = (ic[curr_ic].pwma.rx_data[0] & 0x0F);
    ic[curr_ic].PwmA.pwma[1] = ((ic[curr_ic].pwma.rx_data[0] & 0xF0) >> 4);
    ic[curr_ic].PwmA.pwma[2] = (ic[curr_ic].pwma.rx_data[1] & 0x0F);
    ic[curr_ic].PwmA.pwma[3] = ((ic[curr_ic].pwma.rx_data[1] & 0xF0) >> 4);
    ic[curr_ic].PwmA.pwma[4] = (ic[curr_ic].pwma.rx_data[2] & 0x0F);
    ic[curr_ic].PwmA.pwma[5] = ((ic[curr_ic].pwma.rx_data[2] & 0xF0) >> 4);
    ic[curr_ic].PwmA.pwma[6] = (ic[curr_ic].pwma.rx_data[3] & 0x0F);
    ic[curr_ic].PwmA.pwma[7] = ((ic[curr_ic].pwma.rx_data[3] & 0xF0) >> 4);
    ic[curr_ic].PwmA.pwma[8] = (ic[curr_ic].pwma.rx_data[4] & 0x0F);
    ic[curr_ic].PwmA.pwma[9] = ((ic[curr_ic].pwma.rx_data[4] & 0xF0) >> 4);
    ic[curr_ic].PwmA.pwma[10] = (ic[curr_ic].pwma.rx_data[5] & 0x0F);
    ic[curr_ic].PwmA.pwma[11] = ((ic[curr_ic].pwma.rx_data[5] & 0xF0) >> 4);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParsePwmb
 * @brief Parse PWMB register
 *
 * @details This function Parse the recived pwmb register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParsePwmb(uint8_t tIC, cell_asic *ic, uint8_t *data)
{
  uint8_t address = 0;
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    memcpy(&ic[curr_ic].pwmb.rx_data[0], &data[address], RX_DATA); /* dst , src , size */
    address = ((curr_ic+1) * (RX_DATA));
    ic[curr_ic].PwmB.pwmb[0] = (ic[curr_ic].pwmb.rx_data[0] & 0x0F);
    ic[curr_ic].PwmB.pwmb[1] = ((ic[curr_ic].pwmb.rx_data[0] & 0xF0) >> 4);
    ic[curr_ic].PwmB.pwmb[2] = (ic[curr_ic].pwmb.rx_data[1] & 0x0F);
    ic[curr_ic].PwmB.pwmb[3] = ((ic[curr_ic].pwmb.rx_data[1] & 0xF0) >> 4);
  }
}

/**
 *******************************************************************************
 * Function: adBms6830ParsePwm
 * @brief Parse PWM register
 *
 * @details This function Parse the recived pwm register data.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @param [in]  *data                   data pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830ParsePwm(uint8_t tIC, cell_asic *ic, GRP grp, uint8_t *data)
{
  switch (grp)
  {
    case A:
      adBms6830ParsePwma(tIC, &ic[0], &data[0]);
      break;

    case B:
      adBms6830ParsePwmb(tIC, &ic[0], &data[0]);
      break;

    default:
      break;
  }
}

/**
 *******************************************************************************
 * Function: adBms6830CreateConfiga
 * @brief Create the configation A write buffer
 *
 * @details This function create the configation A write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreateConfiga(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].configa.tx_data[0] = (((ic[curr_ic].tx_cfga.refon & 0x01) << 7) | (ic[curr_ic].tx_cfga.cth & 0x07));
    ic[curr_ic].configa.tx_data[1] = (ic[curr_ic].tx_cfga.flag_d & 0xFF);
    ic[curr_ic].configa.tx_data[2] = (((ic[curr_ic].tx_cfga.soakon & 0x01) << 7) | ((ic[curr_ic].tx_cfga.owrng & 0x01) << 6) | ((ic[curr_ic].tx_cfga.owa & 0x07) << 3));
    ic[curr_ic].configa.tx_data[3] = ((ic[curr_ic].tx_cfga.gpo & 0x00FF));
    ic[curr_ic].configa.tx_data[4] = ((ic[curr_ic].tx_cfga.gpo & 0x0300)>>8);
    ic[curr_ic].configa.tx_data[5] = (((ic[curr_ic].tx_cfga.snap & 0x01) << 5) | ((ic[curr_ic].tx_cfga.mute_st & 0x01) << 4) | ((ic[curr_ic].tx_cfga.comm_bk & 0x01) << 3) | (ic[curr_ic].tx_cfga.fc & 0x07));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830CreateConfigb
 * @brief Create the configation B write buffer
 *
 * @details This function create the configation B write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreateConfigb(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].configb.tx_data[0] = ((ic[curr_ic].tx_cfgb.vuv ));
    ic[curr_ic].configb.tx_data[1] = (((ic[curr_ic].tx_cfgb.vov & 0x000F) << 4) | ((ic[curr_ic].tx_cfgb.vuv ) >> 8));
    ic[curr_ic].configb.tx_data[2] = ((ic[curr_ic].tx_cfgb.vov >>4)&0x0FF);
    ic[curr_ic].configb.tx_data[3] = (((ic[curr_ic].tx_cfgb.dtmen & 0x01) << 7) | ((ic[curr_ic].tx_cfgb.dtrng & 0x01) << 6) | ((ic[curr_ic].tx_cfgb.dcto & 0x3F) << 0));
    ic[curr_ic].configb.tx_data[4] = ((ic[curr_ic].tx_cfgb.dcc & 0xFF));
    ic[curr_ic].configb.tx_data[5] = ((ic[curr_ic].tx_cfgb.dcc >>8 ));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830CreateClrflagData
 * @brief Create the clear flag write buffer
 *
 * @details This function create the clear flag write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreateClrflagData(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].clrflag.tx_data[0] = (ic[curr_ic].clflag.cl_csflt & 0x00FF);
    ic[curr_ic].clrflag.tx_data[1] = ((ic[curr_ic].clflag.cl_csflt & 0xFF00) >> 8);
    ic[curr_ic].clrflag.tx_data[2] = 0x00;
    ic[curr_ic].clrflag.tx_data[3] = 0x00;
    ic[curr_ic].clrflag.tx_data[4] = ((ic[curr_ic].clflag.cl_vaov << 7) | (ic[curr_ic].clflag.cl_vauv << 6) | (ic[curr_ic].clflag.cl_vdov << 5) | (ic[curr_ic].clflag.cl_vduv << 4)
                                      |(ic[curr_ic].clflag.cl_ced << 3)| (ic[curr_ic].clflag.cl_cmed << 2) | (ic[curr_ic].clflag.cl_sed << 1) | (ic[curr_ic].clflag.cl_smed));
    ic[curr_ic].clrflag.tx_data[5] = ((ic[curr_ic].clflag.cl_vde << 7) | (ic[curr_ic].clflag.cl_vdel << 6) | (ic[curr_ic].clflag.cl_spiflt << 4) |(ic[curr_ic].clflag.cl_sleep << 3)
                                      | (ic[curr_ic].clflag.cl_thsd << 2) | (ic[curr_ic].clflag.cl_tmode << 1) | (ic[curr_ic].clflag.cl_oscchk));
  }
}

/**
 *******************************************************************************
 * Function: adBms6830CreateComm
 * @brief Create the configation comm write buffer
 *
 * @details This function create the configation comm write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreateComm(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].com.tx_data[0] = ((ic[curr_ic].comm.icomm[0] & 0x0F)  << 4  | (ic[curr_ic].comm.fcomm[0]   & 0x0F));
    ic[curr_ic].com.tx_data[1] = ((ic[curr_ic].comm.data[0] ));
    ic[curr_ic].com.tx_data[2] = ((ic[curr_ic].comm.icomm[1] & 0x0F)  << 4 ) | (ic[curr_ic].comm.fcomm[1]   & 0x0F);
    ic[curr_ic].com.tx_data[3] = ((ic[curr_ic].comm.data[1]));
    ic[curr_ic].com.tx_data[4] = ((ic[curr_ic].comm.icomm[2] & 0x0F)  << 4  | (ic[curr_ic].comm.fcomm[2]   & 0x0F));
    ic[curr_ic].com.tx_data[5] = ((ic[curr_ic].comm.data[2]));
  }
}


/**
 *******************************************************************************
 * Function: adBms6830CreateRR
 * @brief Create the configation comm write buffer
 *
 * @details This function create the configation comm write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreateRR(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].retention.tx_data[0] = ic[curr_ic].RR.rr[0];
    ic[curr_ic].retention.tx_data[1] = ic[curr_ic].RR.rr[1];
    ic[curr_ic].retention.tx_data[2] = ic[curr_ic].RR.rr[2];
    ic[curr_ic].retention.tx_data[3] = ic[curr_ic].RR.rr[3];
    ic[curr_ic].retention.tx_data[4] = ic[curr_ic].RR.rr[4];
    ic[curr_ic].retention.tx_data[5] = ic[curr_ic].RR.rr[5];
  }
}


/**
 *******************************************************************************
 * Function: adBms6830CreatePwma
 * @brief Create the configation pwma write buffer
 *
 * @details This function create the configation pwma write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreatePwma(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].pwma.tx_data[0] = ((ic[curr_ic].PwmA.pwma[1] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[0] & 0x0F));
    ic[curr_ic].pwma.tx_data[1] = ((ic[curr_ic].PwmA.pwma[3] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[2] & 0x0F));
    ic[curr_ic].pwma.tx_data[2] = ((ic[curr_ic].PwmA.pwma[5] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[4] & 0x0F));
    ic[curr_ic].pwma.tx_data[3] = ((ic[curr_ic].PwmA.pwma[7] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[6] & 0x0F));
    ic[curr_ic].pwma.tx_data[4] = ((ic[curr_ic].PwmA.pwma[9] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[8] & 0x0F));
    ic[curr_ic].pwma.tx_data[5] = ((ic[curr_ic].PwmA.pwma[11] & 0x0F) << 4 | (ic[curr_ic].PwmA.pwma[10] & 0x0F));
  }
}
/**
 *******************************************************************************
 * Function: adBms6830CreatePwmb
 * @brief Create the configation pwmb write buffer
 *
 * @details This function create the configation pwmb write buffer.
 *
 * Parameters:
 *
 * @param [in]  tIC                     Total IC
 *
 * @param [in]  *ic                     cell_asic ic structure pointer
 *
 * @return None
 *
 *******************************************************************************
*/
void adBms6830CreatePwmb(uint8_t tIC, cell_asic *ic)
{
  for(uint8_t curr_ic = 0; curr_ic < tIC; curr_ic++)
  {
    ic[curr_ic].pwmb.tx_data[0] = ((ic[curr_ic].PwmB.pwmb[1] & 0x0F) << 4 | (ic[curr_ic].PwmB.pwmb[0] & 0x0F));
    ic[curr_ic].pwmb.tx_data[1] = ((ic[curr_ic].PwmB.pwmb[3] & 0x0F) << 4 | (ic[curr_ic].PwmB.pwmb[2] & 0x0F));
  }
}
/**************************************** BMS Driver APIs definitions ********************************************/
/* Precomputed CRC15 Table */
const uint16_t Crc15Table[256] =
{
    0x0000, 0xc599, 0xceab, 0xb32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,
    0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1,
    0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
    0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
    0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd,
    0x2544, 0x2be, 0xc727, 0xcc15, 0x98c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c,
    0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b, 0xc969, 0xcf0, 0xdf0d,
    0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
    0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
    0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
    0x4a88, 0x8f11, 0x57c, 0xc0e5, 0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b,
    0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
    0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070,
    0x85e9, 0xf84, 0xca1d, 0xc12f, 0x4b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528,
    0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59,
    0x2ac0, 0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01,
    0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9,
    0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a,
    0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25,
    0x2fbc, 0x846, 0xcddf, 0xc6ed, 0x374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453,
    0x1ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b,
    0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3,
    0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
};

/**
*******************************************************************************
* Function: Pec15_Calc
* @brief CRC15 Pec Calculation Function
*
* @details This function calculates and return the CRC15 value
*
* Parameters:
* @param [in]   Len Data length
*
* @param [in] *data    Data pointer
*
* @return CRC15_Value
*
*******************************************************************************
*/
uint16_t Pec15_Calc
(
    uint8_t len, /* Number of bytes that will be used to calculate a PEC */
    uint8_t *data /* Array of data that will be used to calculate  a PEC */
)
{
    uint16_t remainder, addr;
    remainder = 16; /* initialize the PEC */
    for (uint8_t i = 0; i < len; i++) /* loops for each byte in data array */
    {
        addr = (((remainder >> 7)^data[i]) & 0xff); /* calculate PEC table address */
        remainder = ((remainder << 8)^Crc15Table[addr]);
    }
    return(remainder * 2); /* The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2 */
}

uint16_t pec10_calc(bool rx_cmd, int len, uint8_t *data)
{
    uint16_t remainder = 16; /* PEC_SEED;   0000010000 */
    uint16_t polynom = 0x8F; /* x10 + x7 + x3 + x2 + x + 1 <- the CRC15 polynomial         100 1000 1111   48F */

    /* Perform modulo-2 division, a byte at a time. */
    for (uint8_t pbyte = 0; pbyte < len; ++pbyte)
    {
        /* Bring the next byte into the remainder. */
        remainder ^= (uint16_t)(data[pbyte] << 2);
        /* Perform modulo-2 division, a bit at a time.*/
        for (uint8_t bit_ = 8; bit_ > 0; --bit_)
        {
            /* Try to divide the current data bit. */
            if ((remainder & 0x200) > 0)//equivalent to remainder & 2^14 simply check for MSB
            {
                remainder = (uint16_t)((remainder << 1));
                remainder = (uint16_t)(remainder ^ polynom);
            }
            else
            {
                remainder = (uint16_t)(remainder << 1);
            }
        }
    }
    if (rx_cmd == true)
    {
        remainder ^= (uint16_t)((data[len] & 0xFC) << 2);
        /* Perform modulo-2 division, a bit at a time */
        for (uint8_t bit_ = 6; bit_ > 0; --bit_)
        {
            /* Try to divide the current data bit */
            if ((remainder & 0x200) > 0)//equivalent to remainder & 2^14 simply check for MSB
            {
                remainder = (uint16_t)((remainder << 1));
                remainder = (uint16_t)(remainder ^ polynom);
            }
            else
            {
                remainder = (uint16_t)((remainder << 1));
            }
        }
    }
    return ((uint16_t)(remainder & 0x3FF));
}

/**
*******************************************************************************
* Function: spiSendCmd
* @brief Send command in spi line
*
* @details This function send bms command in spi line
*
* Parameters:
* @param [in]   tx_cmd  Tx command bytes
*
* @return None
*
*******************************************************************************
*/
void spiSendCmd(uint8_t tx_cmd[2])
{
    uint8_t cmd[4];
    uint16_t cmd_pec;
    cmd[0] = tx_cmd[0];
    cmd[1] =  tx_cmd[1];
    cmd_pec = Pec15_Calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);
    //uint8_t dummay_data[2];
    //  dummay_data[0]=0xff;
    //  for (uint8_t i=0; i<tIC ; i++)
    //  {
    //      adBmsCsLow();
    //      spiWriteBytes(1,&dummay_data[0]);
    //      adBmsCsHigh();
    //  }
    adBmsCsLow();
    spiWriteBytes(4, &cmd[0]);
    adBmsCsHigh();
}
/**
*******************************************************************************
* Function: spiReadData
* @brief Spi Read Bms Data
*
* @details This function send bms command in spi line and read command corrospond data byte.
*
* Parameters:
* @param [in]   tIC     Total IC
*
* @param [in]  tx_cmd   Tx command bytes
*
* @param [in]  *rx_data Rx data pointer
*
* @param [in]  *pec_error Pec error pointer
*
* @param [in]  *cmd_cntr command counter pointer
*
* @return None
*
*******************************************************************************
*/
void spiReadData
(
    uint8_t tIC,
    uint8_t tx_cmd[2],
    uint8_t *rx_data,
    uint8_t *pec_error,
    uint8_t *cmd_cntr,
    uint8_t regData_size
)
{
    uint8_t *data, *copyArray, src_address = 0;
    uint16_t cmd_pec, received_pec, calculated_pec;
    uint8_t BYTES_IN_REG = regData_size;
    uint8_t RX_BUFFER = (regData_size * tIC);

    data = (uint8_t *)calloc(RX_BUFFER, sizeof(uint8_t));
    copyArray = (uint8_t *)calloc(BYTES_IN_REG, sizeof(uint8_t));
    if((data == NULL) || (copyArray == NULL))
    {
        LOG_ERROR(LOG_TAG, " Failed to allocate spi read data memory \r\n");
        return;
    }
    else
    {
        uint8_t cmd[4];
        cmd[0] = tx_cmd[0];
        cmd[1] = tx_cmd[1];
        cmd_pec = Pec15_Calc(2, cmd);
        cmd[2] = (uint8_t)(cmd_pec >> 8);
        cmd[3] = (uint8_t)(cmd_pec);
        uint8_t dummay_data[2];
        dummay_data[0] = 0xff;
        for (uint8_t i = 0; i < tIC ; i++)
        {
            adBmsCsLow();
            spiWriteBytes(1, &dummay_data[0]);
            adBmsCsHigh();
        }
        adBmsCsLow();
        spiWriteReadBytes(&cmd[0], &data[0], RX_BUFFER);                 /* Read the configuration data of all ICs on the daisy chain into readdata array */
        adBmsCsHigh();
        for (uint8_t current_ic = 0; current_ic < tIC; current_ic++)     /* executes for each ic in the daisy chain and packs the data */
        {
            /* Into the r_comm array as well as check the received data for any bit errors */
            for (uint8_t current_byte = 0; current_byte < (BYTES_IN_REG - 2); current_byte++)
            {
                rx_data[(current_ic * BYTES_IN_REG) + current_byte] = data[current_byte + (current_ic * BYTES_IN_REG)];
            }
            /* Get command counter value */
            cmd_cntr[current_ic] = (data[(current_ic * BYTES_IN_REG) + (BYTES_IN_REG - 2)] >> 2);
            /* Get received pec value from ic*/
            received_pec = (uint16_t)(((data[(current_ic * BYTES_IN_REG) + (BYTES_IN_REG - 2)] & 0x03) << 8) | data[(current_ic * BYTES_IN_REG) + (BYTES_IN_REG - 1)]);
            /* Copy each ic correspond data + pec value for calculate data pec */
            memcpy(&copyArray[0], &data[src_address], BYTES_IN_REG);
            src_address = ((current_ic + 1) * (regData_size));
            /* Calculate data pec */
            calculated_pec = (uint16_t)pec10_calc(true, (BYTES_IN_REG - 2), &copyArray[0]);
            /* Match received pec with calculated pec */
            if (received_pec == calculated_pec)
            {
                pec_error[current_ic] = 0;    /* If no error is there value set to 0 */
            }
            else
            {
                pec_error[current_ic] = 1;    /* If error is there value set to 1 */
            }
        }
    }
    free(data);
    free(copyArray);
}

/**
*******************************************************************************
* Function: spiWriteData
* @brief Spi Write Bms Data
*
* @details This function write the data into bms ic.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  tx_cmd   Tx command bytes
*
* @param [in]  *data   Data pointer
*
* @return None
*
*******************************************************************************
*/
void spiWriteData
(
    uint8_t tIC,
    uint8_t tx_cmd[2],
    uint8_t *data
)
{
    uint8_t BYTES_IN_REG = TX_DATA;
    uint8_t CMD_LEN = 4 + (RX_DATA * tIC);
    uint16_t data_pec, cmd_pec;
    uint8_t *cmd, copyArray[TX_DATA], src_address = 0;
    uint8_t cmd_index;
    cmd = (uint8_t *)calloc(CMD_LEN, sizeof(uint8_t));
    if(cmd == NULL)
    {
        LOG_ERROR(LOG_TAG, " Failed to allocate cmd array memory \r\n");
        return;
    }
    else
    {
        cmd[0] = tx_cmd[0];
        cmd[1] = tx_cmd[1];
        cmd_pec = Pec15_Calc(2, cmd);
        cmd[2] = (uint8_t)(cmd_pec >> 8);
        cmd[3] = (uint8_t)(cmd_pec);
        cmd_index = 4;
        /* executes for each LTC68xx, this loops starts with the last IC on the stack */
        for (uint8_t current_ic = tIC; current_ic > 0; current_ic--)
        {
            src_address = ((current_ic - 1) * TX_DATA);
            /* The first configuration written is received by the last IC in the daisy chain */
            for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)
            {
                cmd[cmd_index] = data[((current_ic - 1) * 6) + current_byte];
                cmd_index = cmd_index + 1;
            }
            /* Copy each ic correspond data + pec value for calculate data pec */
            memcpy(&copyArray[0], &data[src_address], TX_DATA); /* dst, src, size */
            /* calculating the PEC for each Ics configuration register data */
            data_pec = (uint16_t)pec10_calc(true, BYTES_IN_REG, &copyArray[0]);
            cmd[cmd_index] = (uint8_t)(data_pec >> 8);
            // pc.LOG_DEBUG(LOG_TAG, "DPEC0: 0x%x, ", (uint8_t)(data_pec >> 8)); //print DPEC0
            cmd_index = cmd_index + 1;
            cmd[cmd_index] = (uint8_t)data_pec;
            // pc.LOG_DEBUG(LOG_TAG, "DPEC1: 0x%x,\r\n\r\n", (uint8_t)(data_pec));  // print DPEC1
            cmd_index = cmd_index + 1;
        }
        uint8_t dummay_data[2];
        dummay_data[0] = 0xff;
        for (uint8_t i = 0; i < tIC ; i++)
        {
            adBmsCsLow();
            spiWriteBytes(1, &dummay_data[0]);
            adBmsCsHigh();
        }
        adBmsCsLow();
        spiWriteBytes(CMD_LEN, &cmd[0]);
        adBmsCsHigh();
    }
    free(cmd);
}

/**
*******************************************************************************
* Function: adBmsReadData
* @brief Adbms Read Data From Bms ic.
*
* @details This function send bms command, read payload data parse into function and check pec error.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *ic      cell_asic stucture pointer
*
* @param [in]  cmd_arg   command bytes
*
* @param [in]  TYPE   Enum type of resistor
*
* @param [in]  GRP   Enum type of resistor group
*
* @return None
*
*******************************************************************************
*/
void adBmsReadData(uint8_t tIC, cell_asic *ic, uint8_t cmd_arg[2], TYPE type, GRP group)
{
    uint16_t rBuff_size;
    uint8_t regData_size;
    if(group == ALL_GRP)
    {
        if(type == Rdcvall)
        {
            rBuff_size = RDCVALL_SIZE;
            regData_size = RDCVALL_SIZE;
        }
        else if(type == Rdsall)
        {
            rBuff_size = RDSALL_SIZE;
            regData_size = RDSALL_SIZE;
        }
        else if(type == Rdacall)
        {
            rBuff_size = RDACALL_SIZE;
            regData_size = RDACALL_SIZE;
        }
        else if(type == Rdfcall)
        {
            rBuff_size = RDFCALL_SIZE;
            regData_size = RDFCALL_SIZE;
        }
        else if(type == Rdcsall)
        {
            rBuff_size = RDCSALL_SIZE;
            regData_size = RDCSALL_SIZE;
        }
        else if(type == Rdasall)
        {
            rBuff_size = RDASALL_SIZE;
            regData_size = RDASALL_SIZE;
        }
        else if(type == Rdacsall)
        {
            rBuff_size = RDACSALL_SIZE;
            regData_size = RDACSALL_SIZE;
        }
        else
        {
            LOG_ERROR(LOG_TAG, "Read All cmd wrong type select \r\n");
        }
    }
    else
    {
        rBuff_size = (tIC * RX_DATA);
        regData_size = RX_DATA;
    }
    uint8_t *read_buffer, *pec_error, *cmd_count;
    read_buffer = (uint8_t *)calloc(rBuff_size, sizeof(uint8_t));
    pec_error = (uint8_t *)calloc(tIC, sizeof(uint8_t));
    cmd_count = (uint8_t *)calloc(tIC, sizeof(uint8_t));
    if((pec_error == NULL) || (cmd_count == NULL) || (read_buffer == NULL))
    {
        LOG_ERROR(LOG_TAG, " Failed to allocate memory \r\n");
        return;
    }
    else
    {
        spiReadData(tIC, &cmd_arg[0], &read_buffer[0], &pec_error[0], &cmd_count[0], regData_size);
        switch (type)
        {
            case Config:
                adBms6830ParseConfig(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.cfgr_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Cell:
                adBms6830ParseCell(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.cell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case AvgCell:
                adBms6830ParseAverageCell(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.acell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case S_volt:
                adBms6830ParseSCell(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.scell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case F_volt:
                adBms6830ParseFCell(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.fcell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Aux:
                adBms6830ParseAux(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.aux_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case RAux:
                adBms6830ParseRAux(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.raux_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Status:
                adBms6830ParseStatus(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.stat_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Comm:
                adBms6830ParseComm(tIC, ic, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.comm_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Pwm:
                adBms6830ParsePwm(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.pwm_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case RR:
                adBms6830ParseRR(tIC, ic, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.rr_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Sid:
                adBms6830ParseSID(tIC, ic, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.sid_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Rdcvall:
                /* 32 byte cell data + 2 byte pec */
                adBms6830ParseCell(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.cell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Rdacall:
                /* 32 byte avg cell data + 2 byte pec */
                adBms6830ParseAverageCell(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.acell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Rdsall:
                /* 32 byte scell volt data + 2 byte pec */
                adBms6830ParseSCell(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.scell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Rdfcall:
                /* 32 byte fcell data + 2 byte pec */
                adBms6830ParseFCell(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.fcell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Rdcsall:
                /* 64 byte + 2 byte pec = 32 byte cell data + 32 byte scell volt data */
                adBms6830ParseCell(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.cell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                adBms6830ParseSCell(tIC, ic, group, &read_buffer[32]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.scell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Rdacsall:
                /* 64 byte + 2 byte pec = 32 byte avg cell data + 32 byte scell volt data */
                adBms6830ParseAverageCell(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.acell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                adBms6830ParseSCell(tIC, ic, group, &read_buffer[32]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.scell_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            case Rdasall:
                /* 68 byte + 2 byte pec:
                24 byte gpio data + 20 byte Redundant gpio data +
                24 byte status A(6 byte), B(6 byte), C(4 byte), D(6 byte) & E(2 byte)
                */
                adBms6830ParseAux(tIC, ic, group, &read_buffer[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.aux_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                adBms6830ParseRAux(tIC, ic, group, &read_buffer[24]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.raux_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                adBms6830ParseStatus(tIC, ic, group, &read_buffer[44]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    ic[cic].cccrc.stat_pec = pec_error[cic];
                    ic[cic].cccrc.cmd_cntr = cmd_count[cic];
                }
                break;

            default:
                break;
        }
    }
    free(read_buffer);
    free(pec_error);
    free(cmd_count);
}
/**
*******************************************************************************
* Function: adBmsWriteData
* @brief Adbms Write Data into Bms ic.
*
* @details This function write the data into bms ic.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *ic      cell_asic stucture pointer
*
* @param [in]  cmd_arg   command bytes
*
* @param [in]  TYPE   Enum type of resistor
*
* @param [in]  GRP   Enum type of resistor group
*
* @return None
*
*******************************************************************************
*/
void adBmsWriteData(uint8_t tIC, cell_asic *ic, uint8_t cmd_arg[2], TYPE type, GRP group)
{
    uint8_t data_len = TX_DATA, write_size = (TX_DATA * tIC);
    uint8_t *write_buffer = (uint8_t *)calloc(write_size, sizeof(uint8_t));
    if(write_buffer == NULL)
    {
        LOG_ERROR(LOG_TAG, " Failed to allocate write_buffer array memory \r\n");
        return;
    }
    else
    {
        switch (type)
        {
            case Config:
            {
                switch (group)
                {
                    case A:
                    {
                        adBms6830CreateConfiga(tIC, &ic[0]);
                        for (uint8_t cic = 0; cic < tIC; cic++)
                        {
                            for (uint8_t data = 0; data < data_len; data++)
                            {
                                write_buffer[(cic * data_len) + data] = ic[cic].configa.tx_data[data];
                            }
                        }
                        break;
                    }
                    case B:
                    {
                        adBms6830CreateConfigb(tIC, &ic[0]);
                        for (uint8_t cic = 0; cic < tIC; cic++)
                        {
                            for (uint8_t data = 0; data < data_len; data++)
                            {
                                write_buffer[(cic * data_len) + data] = ic[cic].configb.tx_data[data];
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case Cmcfg:
            {
                uint8_t txdata[6] = {0xE0, 0x0E, 0x00, 0x00, 0x00, 0x00};
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    for (uint8_t data = 0; data < data_len; data++)
                    {
                        write_buffer[(cic * data_len) + data] = txdata[data];
                    }
                }
                break;
            }
            case Comm:
            {
                adBms6830CreateComm(tIC, &ic[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    for (uint8_t data = 0; data < data_len; data++)
                    {
                        write_buffer[(cic * data_len) + data] = ic[cic].com.tx_data[data];
                    }
                }
                break;
            }

            case RR:
            {
                adBms6830CreateRR(tIC, &ic[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    for (uint8_t data = 0; data < data_len; data++)
                    {
                        write_buffer[(cic * data_len) + data] = ic[cic].retention.tx_data[data];
                    }
                }
                break;
            }

            case Pwm:
            {
                switch (group)
                {
                    case A:
                        adBms6830CreatePwma(tIC, &ic[0]);
                        for (uint8_t cic = 0; cic < tIC; cic++)
                        {
                            for (uint8_t data = 0; data < data_len; data++)
                            {
                                write_buffer[(cic * data_len) + data] = ic[cic].pwma.tx_data[data];
                            }
                        }
                        break;
                    case B:
                        adBms6830CreatePwmb(tIC, &ic[0]);
                        for (uint8_t cic = 0; cic < tIC; cic++)
                        {
                            for (uint8_t data = 0; data < data_len; data++)
                            {
                                write_buffer[(cic * data_len) + data] = ic[cic].pwmb.tx_data[data];
                            }
                        }
                        break;
                    default:
                        break;
                }
                break;
            }

            case Clrflag:
            {
                adBms6830CreateClrflagData(tIC, &ic[0]);
                for (uint8_t cic = 0; cic < tIC; cic++)
                {
                    for (uint8_t data = 0; data < data_len; data++)
                    {
                        write_buffer[(cic * data_len) + data] = ic[cic].clrflag.tx_data[data];
                    }
                }
                break;
            }
            default:
                break;                           
        }
    }
    spiWriteData(tIC, cmd_arg, &write_buffer[0]);
    free(write_buffer);
}

/**
*******************************************************************************
* Function: adBmsPollAdc
* @brief PLADC Command.
*
* @details Send poll adc command and retun adc conversion count.
*
* Parameters:
*
* @param [in]  tIC      Total IC
*
* @param [in]  tx_cmd   Tx command byte
*
* @return None
*
*******************************************************************************
*/
uint32_t adBmsPollAdc(uint8_t tx_cmd[2])
{
    uint32_t ret = 0;
    uint8_t cmd[4];
    uint16_t cmd_pec;
    uint8_t read_data = 0x00;
    uint8_t SDO_Line = 0xFF;
    cmd[0] = tx_cmd[0];
    cmd[1] = tx_cmd[1];
    cmd_pec = Pec15_Calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);
//    conv_count = tim_get_systime();
    adBmsCsLow();
    spiWriteBytes(4, &cmd[0]);
    
//    conv_count = tim_get_systime() - conv_count;
    return ret;
}
uint32_t adBmsPollAdcWait(uint8_t tx_cmd[2])
{
    uint32_t conv_count = 0;
    uint8_t cmd[4];
    uint16_t cmd_pec;
    uint8_t read_data = 0x00;
    uint8_t SDO_Line = 0xFF;
    cmd[0] = tx_cmd[0];
    cmd[1] = tx_cmd[1];
    cmd_pec = Pec15_Calc(2, cmd);
    cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);
    conv_count = tim_get_systime();
    adBmsCsLow();
    spiWriteBytes(4, &cmd[0]);
    do
    {
        spiReadBytes(1, &read_data);
    }
    while(!(read_data == SDO_Line));
    adBmsCsHigh();
    conv_count = tim_get_systime() - conv_count;
    return(conv_count);
}

/**
*******************************************************************************
* Function: adBms6830_Adcv
* @brief ADCV Command.
*
* @details Send adcv command to start cell voltage conversion.
*
* Parameters:
*
* @param [in]  RD      Enum type Read bit
*
* @param [in]  CONT   Enum type continuous measurement bit
*
* @param [in]  DCP   Enum type discharge bit
*
* @param [in]  RSTF   Enum type Reset filter
*
* @param [in]  OW_C_S   Enum type open wire c/s
*
* @return None
*
*******************************************************************************
*/
void adBms6830_Adcv
(
    RD rd,
    CONT cont,
    DCP dcp,
    RSTF rstf,
    OW_C_S owcs
)
{
    uint8_t cmd[2];
    cmd[0] = 0x02 + rd;
    cmd[1] = (cont << 7) + (dcp << 4) + (rstf << 2) + (owcs & 0x03) + 0x60;
    spiSendCmd(cmd);
}

/**
*******************************************************************************
* Function: adBms6830_Adsv
* @brief ADSV Command.
*
* @details Send s_adcv command to start cell voltage conversion.
*
* Parameters:
*
* @param [in]  cont    Enum type continuous measurement bit
*
* @param [in]  dcp    Enum type discharge bit
*
* @param [in]  owcs   Enum type open wire c/s
*
* @return None
*
*******************************************************************************
*/
void adBms6830_Adsv
(
    CONT cont,
    DCP dcp,
    OW_C_S owcs
)
{
    uint8_t cmd[2];
    cmd[0] = 0x01;
    cmd[1] = (cont << 7) + (dcp << 4) + (owcs & 0x03) + 0x68;
    spiSendCmd(cmd);
}

/**
*******************************************************************************
* Function: adBms6830_Adax
* @brief ADAX Command.
*
* @details Send Aux command to starts auxiliary conversion.
*
* Parameters:
*
* @param [in]  owcs    Enum type open wire c/s
*
* @param [in]  pup    Enum type Pull Down current during aux conversion
*
* @param [in]  ch    Enum type gpio Channel selection
*
* @return None
*
*******************************************************************************
*/
void adBms6830_Adax
(
    OW_AUX owaux,
    PUP pup,
    CH ch
)
{
    uint8_t cmd[2];
    cmd[0] = 0x04 + owaux;
    cmd[1] = (pup << 7) + (((ch >> 4) & 0x01) << 6) + (ch & 0x0F) + 0x10;
    spiSendCmd(cmd);
}
/**
*******************************************************************************
* Function: adBms6830_Adax2
* @brief ADAX2 Command.
*
* @details Send Aux2 command to starts auxiliary conversion.
*
* Parameters:
*
* @param [in]  ch    Enum type gpio Channel selection
*
* @return None
*
*******************************************************************************
*/
void adBms6830_Adax2
(
    CH ch
)
{
    uint8_t cmd[2];
    cmd[0] = 0x04;
    cmd[1] = (ch & 0x0F);
    spiSendCmd(cmd);
}

/**
 *******************************************************************************
 * Function: printWriteConfig
 * @brief Print write config A/B result.
 *
 * @details This function Print write config result into terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @param [in]  grp      Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printWriteConfig(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    LOG_DEBUG(LOG_TAG, "IC%d:\r\n",(ic+1));
    if(type == Config)
    {
      if(grp == A)
      {
        LOG_DEBUG1(LOG_TAG, "Write Config A:\r\n");
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configa.tx_data[0]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configa.tx_data[1]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configa.tx_data[2]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configa.tx_data[3]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configa.tx_data[4]);
        LOG_DEBUG1(LOG_TAG, "0x%X\r\n\r\n", IC[ic].configa.tx_data[5]);
      }
      else if(grp == B)
      {
        LOG_DEBUG1(LOG_TAG, "Write Config B:\r\n");
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configb.tx_data[0]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configb.tx_data[1]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configb.tx_data[2]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configb.tx_data[3]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configb.tx_data[4]);
        LOG_DEBUG1(LOG_TAG, "0x%X\r\n\r\n", IC[ic].configb.tx_data[5]);
      }
      else if(grp == ALL_GRP)
      {
        LOG_DEBUG1(LOG_TAG, "Write Config A:\r\n");
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configa.tx_data[0]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configa.tx_data[1]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configa.tx_data[2]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configa.tx_data[3]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configa.tx_data[4]);
        LOG_DEBUG1(LOG_TAG, "0x%X\r\n\r\n", IC[ic].configa.tx_data[5]);

        LOG_DEBUG1(LOG_TAG, "Write Config B:\r\n");
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configb.tx_data[0]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configb.tx_data[1]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configb.tx_data[2]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configb.tx_data[3]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].configb.tx_data[4]);
        LOG_DEBUG1(LOG_TAG, "0x%X\r\n\r\n", IC[ic].configb.tx_data[5]);
      }
      else{ LOG_ERROR(LOG_TAG, "Wrong Register Group Select\r\n"); }
    }
  }
}

/**
 *******************************************************************************
 * Function: printReadConfig
 * @brief Print read config result.
 *
 * @details This function Print read config result into terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @param [in]  grp      Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printReadConfig(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    LOG_DEBUG(LOG_TAG, "IC%d:\r\n",(ic+1));
    if(type == Config)
    {
      if(grp == A)
      {
        LOG_DEBUG1(LOG_TAG, "Read Config A:\r\n");
        LOG_DEBUG1(LOG_TAG, "REFON:0x%X, ", IC[ic].rx_cfga.refon);
        LOG_DEBUG1(LOG_TAG, "CTH:0x%X\r\n", IC[ic].rx_cfga.cth & 0x07);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[0]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x01));
        LOG_DEBUG1(LOG_TAG, "FLAG_D[1]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x02)>>1);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[2]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x04)>>2);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[3]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x08)>>3);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[4]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x10)>>4);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[5]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x20)>>5);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[6]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x40)>>6);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[7]:0x%X\r\n", (IC[ic].rx_cfga.flag_d & 0x80)>>7);
        LOG_DEBUG1(LOG_TAG, "OWA[2:0]:0x%X, ", (IC[ic].rx_cfga.owa));
        LOG_DEBUG1(LOG_TAG, "OWRNG:0x%X, ", (IC[ic].rx_cfga.owrng));
        LOG_DEBUG1(LOG_TAG, "SOAKON:0x%X, ", (IC[ic].rx_cfga.soakon));
        LOG_DEBUG1(LOG_TAG, "GPO:0x%X, ", (IC[ic].rx_cfga.gpo));
        LOG_DEBUG1(LOG_TAG, "FC:0x%X, ", (IC[ic].rx_cfga.fc));
        LOG_DEBUG1(LOG_TAG, "COMM_BK:0x%X, ", (IC[ic].rx_cfga.comm_bk));
        LOG_DEBUG1(LOG_TAG, "MUTE_ST:0x%X, ", (IC[ic].rx_cfga.mute_st));
        LOG_DEBUG1(LOG_TAG, "SNAP:0x%X\r\n\r\n", (IC[ic].rx_cfga.snap));
        LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
        LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n",IC[ic].cccrc.cfgr_pec);
      }
      else if(grp == B)
      {
        LOG_DEBUG1(LOG_TAG, "Read Config B:\r\n");
        LOG_DEBUG1(LOG_TAG, "VUV:0x%X, ", IC[ic].rx_cfgb.vuv);
        LOG_DEBUG1(LOG_TAG, "VOV:0x%X, ", IC[ic].rx_cfgb.vov);
        LOG_DEBUG1(LOG_TAG, "DCTO:0x%X, ", IC[ic].rx_cfgb.dcto);
        LOG_DEBUG1(LOG_TAG, "DTRNG:0x%X, ", IC[ic].rx_cfgb.dtrng);
        LOG_DEBUG1(LOG_TAG, "DTMEN:0x%X, ", IC[ic].rx_cfgb.dtmen);
        LOG_DEBUG1(LOG_TAG, "DCC:0x%X\r\n\r\n", IC[ic].rx_cfgb.dcc);
        LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
        LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n",IC[ic].cccrc.cfgr_pec);
      }
      else if(grp == ALL_GRP)
      {
        LOG_DEBUG1(LOG_TAG, "Read Config A:\r\n");
        LOG_DEBUG1(LOG_TAG, "REFON:0x%X, ", IC[ic].rx_cfga.refon);
        LOG_DEBUG1(LOG_TAG, "CTH:0x%X\r\n", IC[ic].rx_cfga.cth & 0x07);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[0]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x01));
        LOG_DEBUG1(LOG_TAG, "FLAG_D[1]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x02)>>1);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[2]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x04)>>2);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[3]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x08)>>3);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[4]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x10)>>4);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[5]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x20)>>5);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[6]:0x%X, ", (IC[ic].rx_cfga.flag_d & 0x40)>>6);
        LOG_DEBUG1(LOG_TAG, "FLAG_D[7]:0x%X\r\n", (IC[ic].rx_cfga.flag_d & 0x80)>>7);
        LOG_DEBUG1(LOG_TAG, "OWA[2:0]:0x%X, ", (IC[ic].rx_cfga.owa));
        LOG_DEBUG1(LOG_TAG, "OWRNG:0x%X, ", (IC[ic].rx_cfga.owrng));
        LOG_DEBUG1(LOG_TAG, "SOAKON:0x%X, ", (IC[ic].rx_cfga.soakon));
        LOG_DEBUG1(LOG_TAG, "GPO:0x%X, ", (IC[ic].rx_cfga.gpo));
        LOG_DEBUG1(LOG_TAG, "FC:0x%X, ", (IC[ic].rx_cfga.fc));
        LOG_DEBUG1(LOG_TAG, "COMM_BK:0x%X, ", (IC[ic].rx_cfga.comm_bk));
        LOG_DEBUG1(LOG_TAG, "MUTE_ST:0x%X, ", (IC[ic].rx_cfga.mute_st));
        LOG_DEBUG1(LOG_TAG, "SNAP:0x%X\r\n\r\n", (IC[ic].rx_cfga.snap));

        LOG_DEBUG1(LOG_TAG, "Read Config B:\r\n");
        LOG_DEBUG1(LOG_TAG, "VUV:0x%X, ", IC[ic].rx_cfgb.vuv);
        LOG_DEBUG1(LOG_TAG, "VOV:0x%X, ", IC[ic].rx_cfgb.vov);
        LOG_DEBUG1(LOG_TAG, "DCTO:0x%X, ", IC[ic].rx_cfgb.dcto);
        LOG_DEBUG1(LOG_TAG, "DTRNG:0x%X, ", IC[ic].rx_cfgb.dtrng);
        LOG_DEBUG1(LOG_TAG, "DTMEN:0x%X, ", IC[ic].rx_cfgb.dtmen);
        LOG_DEBUG1(LOG_TAG, "DCC:0x%X\r\n\r\n", IC[ic].rx_cfgb.dcc);
        LOG_DEBUG1(LOG_TAG, "CCount:%d,", IC[ic].cccrc.cmd_cntr);
        LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n", IC[ic].cccrc.cfgr_pec);
      }
      else{ LOG_ERROR(LOG_TAG, "Wrong Register Group Select\r\n"); }
    }
  }
}

/**
 *******************************************************************************
 * Function: printVoltages
 * @brief Print Voltages.
 *
 * @details This function Print Voltages into IAR I/O terminal.
 *
 * Parameters:
 * @param [in]  tIC    Total IC
 *
 * @param [in]  *IC    cell_asic stucture pointer
 *
 * @param [in]  type    Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printVoltages(uint8_t tIC, cell_asic *IC, TYPE type)
{
  float voltage;
  uint16_t temp;
  uint8_t channel;
  if((type == Cell) || (type == AvgCell) || (type == F_volt) || (type == S_volt))
  {
    channel = CELL;
  }
  else if (type == Aux){ channel = AUX;}
  else if (type == RAux){channel = RAUX;}
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    LOG_DEBUG(LOG_TAG, "IC%d:",(ic+1));
    for(uint8_t index = 0; index < channel; index++)
    {
      if(type == Cell){ temp = IC[ic].cell.c_codes[index]; }
      else if(type == AvgCell){ temp = IC[ic].acell.ac_codes[index]; }
      else if(type == F_volt){ temp = IC[ic].fcell.fc_codes[index]; }
      else if(type == S_volt){ temp = IC[ic].scell.sc_codes[index]; }
      else if(type == Aux){ temp = IC[ic].aux.a_codes[index]; }
      else if(type == RAux){ temp = IC[ic].raux.ra_codes[index]; }
      voltage = getVoltage(temp);
      if(type == Cell)
      {
        LOG_DEBUG1(LOG_TAG, "C%d=%fV,\t",(index+1), voltage);
        if(index == (channel-1))
        {
          LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
          LOG_DEBUG1(LOG_TAG, "PECError:%d",IC[ic].cccrc.cell_pec);
        }
      }
      else if(type == AvgCell)
      {
        LOG_DEBUG1(LOG_TAG, "AC%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
          LOG_DEBUG1(LOG_TAG, "PECError:%d",IC[ic].cccrc.acell_pec);
        }
      }
      else if(type == F_volt)
      {
        LOG_DEBUG1(LOG_TAG, "FC%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
          LOG_DEBUG1(LOG_TAG, "PECError:%d",IC[ic].cccrc.fcell_pec);
        }
      }
      else if(type == S_volt)
      {
        LOG_DEBUG1(LOG_TAG, "S%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
          LOG_DEBUG1(LOG_TAG, "PECError:%d",IC[ic].cccrc.scell_pec);
        }
      }
      else if(type == Aux)
      {
        if(index <= 9)
        {
          LOG_DEBUG1(LOG_TAG, "AUX%d=%fV,\t",(index+1), voltage);
        }
        else if(index == 10)
        {
          LOG_DEBUG1(LOG_TAG, "VMV:%fV,",(20 * voltage));
        }
        else if(index == 11)
        {
          LOG_DEBUG1(LOG_TAG, "V+:%fV,",(20 * voltage));
          LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
          LOG_DEBUG1(LOG_TAG, "PECError:%d",IC[ic].cccrc.aux_pec);
        }
      }
      else if(type == RAux)
      {
        LOG_DEBUG1(LOG_TAG, "RAUX%d=%fV,",(index+1), voltage);
        if(index == (channel-1))
        {
          LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
          LOG_DEBUG1(LOG_TAG, "PECError:%d",IC[ic].cccrc.raux_pec);
        }
      }
      else{ LOG_ERROR(LOG_TAG, "Wrong Register Group Select\r\n"); }
    }
    LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
  }
}

/**
 *******************************************************************************
 * Function: PrintStatus
 * @brief Print status reg. result.
 *
 * @details This function Print status result into IAR I/O terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @param [in]  grp      Enum type of resistor group
 *
 * @return None
 *
 *******************************************************************************
*/
void printStatus(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  float voltage;
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    LOG_DEBUG(LOG_TAG, "IC%d:\r\n",(ic+1));
    if(type == Status)
    {
      if(grp == A)
      {
        LOG_DEBUG(LOG_TAG, "Status A:\r\n");
        voltage = getVoltage(IC[ic].stata.vref2);
        LOG_DEBUG1(LOG_TAG, "VREF2:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.vref3);
        LOG_DEBUG1(LOG_TAG, "VREF3:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.itmp);
        LOG_DEBUG1(LOG_TAG, "ITMP:%fC\r\n", (voltage/0.0075f)-273);

        LOG_DEBUG1(LOG_TAG, "CCount:%d, ",IC[ic].cccrc.cmd_cntr);
        LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n",IC[ic].cccrc.stat_pec);
      }
      else if(grp == B)
      {
        LOG_DEBUG1(LOG_TAG, "Status B:\r\n");
        voltage = getVoltage(IC[ic].statb.va);
        LOG_DEBUG1(LOG_TAG, "VA:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vd);
        LOG_DEBUG1(LOG_TAG, "VD:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vr4k);
        LOG_DEBUG1(LOG_TAG, "VR4K:%fV\r\n", voltage);

        LOG_DEBUG1(LOG_TAG, "CCount:%d, ",IC[ic].cccrc.cmd_cntr);
        LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n",IC[ic].cccrc.stat_pec);
      }
      else if(grp == C)
      {
        LOG_DEBUG1(LOG_TAG, "Status C:\r\n");
        LOG_DEBUG1(LOG_TAG, "CSFLT:0x%X, ", IC[ic].statc.cs_flt);

        LOG_DEBUG1(LOG_TAG, "OTP2_MED:0x%X, ", IC[ic].statc.otp2_med);
        LOG_DEBUG1(LOG_TAG, "OTP2_ED:0x%X, ", IC[ic].statc.otp2_ed);
        LOG_DEBUG1(LOG_TAG, "OTP1_MED:0x%X ", IC[ic].statc.otp1_med);
        LOG_DEBUG1(LOG_TAG, "OTP1_ED:0x%X, ", IC[ic].statc.otp1_ed);
        LOG_DEBUG1(LOG_TAG, "VD_UV:0x%X, ", IC[ic].statc.vd_uv);
        LOG_DEBUG1(LOG_TAG, "VD_OV:0x%X, ", IC[ic].statc.vd_ov);
        LOG_DEBUG1(LOG_TAG, "VA_UV:0x%X, ", IC[ic].statc.va_uv);
        LOG_DEBUG1(LOG_TAG, "VA_OV:0x%X\r\n", IC[ic].statc.va_ov);

        LOG_DEBUG1(LOG_TAG, "OSCCHK:0x%X, ", IC[ic].statc.oscchk);
        LOG_DEBUG1(LOG_TAG, "TMODCHK:0x%X, ", IC[ic].statc.tmodchk);
        LOG_DEBUG1(LOG_TAG, "THSD:0x%X, ", IC[ic].statc.thsd);
        LOG_DEBUG1(LOG_TAG, "SLEEP:0x%X, ", IC[ic].statc.sleep);
        LOG_DEBUG1(LOG_TAG, "SPIFLT:0x%X, ", IC[ic].statc.spiflt);
        LOG_DEBUG1(LOG_TAG, "COMP:0x%X, ", IC[ic].statc.comp);
        LOG_DEBUG1(LOG_TAG, "VDEL:0x%X, ", IC[ic].statc.vdel);
        LOG_DEBUG1(LOG_TAG, "VDE:0x%X\r\n", IC[ic].statc.vde);

        LOG_DEBUG1(LOG_TAG, "CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n", IC[ic].cccrc.stat_pec);
      }
      else if(grp == D)
      {
        LOG_DEBUG1(LOG_TAG, "Status D:\r\n");
        LOG_DEBUG1(LOG_TAG, "C1UV:0x%X, ", IC[ic].statd.c_uv[0]);
        LOG_DEBUG1(LOG_TAG, "C2UV:0x%X, ", IC[ic].statd.c_uv[1]);
        LOG_DEBUG1(LOG_TAG, "C3UV:0x%X, ", IC[ic].statd.c_uv[2]);
        LOG_DEBUG1(LOG_TAG, "C4UV:0x%X, ", IC[ic].statd.c_uv[3]);
        LOG_DEBUG1(LOG_TAG, "C5UV:0x%X, ", IC[ic].statd.c_uv[4]);
        LOG_DEBUG1(LOG_TAG, "C6UV:0x%X, ", IC[ic].statd.c_uv[5]);
        LOG_DEBUG1(LOG_TAG, "C7UV:0x%X, ", IC[ic].statd.c_uv[6]);
        LOG_DEBUG1(LOG_TAG, "C8UV:0x%X, ", IC[ic].statd.c_uv[7]);
        LOG_DEBUG1(LOG_TAG, "C9UV:0x%X, ", IC[ic].statd.c_uv[8]);
        LOG_DEBUG1(LOG_TAG, "C10UV:0x%X, ", IC[ic].statd.c_uv[9]);
        LOG_DEBUG1(LOG_TAG, "C11UV:0x%X, ", IC[ic].statd.c_uv[10]);
        LOG_DEBUG1(LOG_TAG, "C12UV:0x%X, ", IC[ic].statd.c_uv[11]);
        LOG_DEBUG1(LOG_TAG, "C13UV:0x%X, ", IC[ic].statd.c_uv[12]);
        LOG_DEBUG1(LOG_TAG, "C14UV:0x%X, ", IC[ic].statd.c_uv[13]);
        LOG_DEBUG1(LOG_TAG, "C15UV:0x%X, ", IC[ic].statd.c_uv[14]);
#if(CELL==16)
        LOG_DEBUG1(LOG_TAG, "C16UV:0x%X\r\n", IC[ic].statd.c_uv[15]);
#endif
        LOG_DEBUG1(LOG_TAG, "C1OV:0x%X, ", IC[ic].statd.c_ov[0]);
        LOG_DEBUG1(LOG_TAG, "C2OV:0x%X, ", IC[ic].statd.c_ov[1]);
        LOG_DEBUG1(LOG_TAG, "C3OV:0x%X, ", IC[ic].statd.c_ov[2]);
        LOG_DEBUG1(LOG_TAG, "C4OV:0x%X, ", IC[ic].statd.c_ov[3]);
        LOG_DEBUG1(LOG_TAG, "C5OV:0x%X, ", IC[ic].statd.c_ov[4]);
        LOG_DEBUG1(LOG_TAG, "C6OV:0x%X, ", IC[ic].statd.c_ov[5]);
        LOG_DEBUG1(LOG_TAG, "C7OV:0x%X, ", IC[ic].statd.c_ov[6]);
        LOG_DEBUG1(LOG_TAG, "C8OV:0x%X, ", IC[ic].statd.c_ov[7]);
        LOG_DEBUG1(LOG_TAG, "C9OV:0x%X, ", IC[ic].statd.c_ov[8]);
        LOG_DEBUG1(LOG_TAG, "C10OV:0x%X, ", IC[ic].statd.c_ov[9]);
        LOG_DEBUG1(LOG_TAG, "C11OV:0x%X, ", IC[ic].statd.c_ov[10]);
        LOG_DEBUG1(LOG_TAG, "C12OV:0x%X, ", IC[ic].statd.c_ov[11]);
        LOG_DEBUG1(LOG_TAG, "C13OV:0x%X, ", IC[ic].statd.c_ov[12]);
        LOG_DEBUG1(LOG_TAG, "C14OV:0x%X, ", IC[ic].statd.c_ov[13]);
        LOG_DEBUG1(LOG_TAG, "C15OV:0x%X, ", IC[ic].statd.c_ov[14]);
#if(CELL==16)
        LOG_DEBUG1(LOG_TAG, "C16OV:0x%X\r\n", IC[ic].statd.c_ov[15]);
#endif
        LOG_DEBUG1(LOG_TAG, "CTS:0x%X, ", IC[ic].statd.cts);
        LOG_DEBUG1(LOG_TAG, "CT:0x%X, ", IC[ic].statd.ct);
        LOG_DEBUG1(LOG_TAG, "OC_CNTR:0x%X\r\n", IC[ic].statd.oc_cntr);

        LOG_DEBUG1(LOG_TAG, "CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n", IC[ic].cccrc.stat_pec);
      }
      else if(grp == E)
      {
        LOG_DEBUG1(LOG_TAG, "Status E:\r\n");
        LOG_DEBUG1(LOG_TAG, "GPI:0x%X, ", IC[ic].state.gpi);
        LOG_DEBUG1(LOG_TAG, "REV_ID:0x%X\r\n", IC[ic].state.rev);

        LOG_DEBUG1(LOG_TAG, "CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n", IC[ic].cccrc.stat_pec);
      }
      else if(grp == ALL_GRP)
      {
        LOG_DEBUG1(LOG_TAG, "Status A:\r\n");
        voltage = getVoltage(IC[ic].stata.vref2);
        LOG_DEBUG1(LOG_TAG, "VREF2:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.vref3);
        LOG_DEBUG1(LOG_TAG, "VREF3:%fV, ", voltage);
        voltage = getVoltage(IC[ic].stata.itmp);
        LOG_DEBUG1(LOG_TAG, "ITMP:%fC\r\n\r\n", (voltage/0.0075f)-273);

        LOG_DEBUG1(LOG_TAG, "Status B:\r\n");
        voltage = getVoltage(IC[ic].statb.va);
        LOG_DEBUG1(LOG_TAG, "VA:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vd);
        LOG_DEBUG1(LOG_TAG, "VD:%fV, ", voltage);
        voltage = getVoltage(IC[ic].statb.vr4k);
        LOG_DEBUG1(LOG_TAG, "VR4K:%fV\r\n\r\n", voltage);

        LOG_DEBUG1(LOG_TAG, "Status C:\r\n");
        LOG_DEBUG1(LOG_TAG, "CSFLT:0x%X, ", IC[ic].statc.cs_flt);

        LOG_DEBUG1(LOG_TAG, "OTP2_MED:0x%X, ", IC[ic].statc.otp2_med);
        LOG_DEBUG1(LOG_TAG, "OTP2_ED:0x%X, ", IC[ic].statc.otp2_ed);
        LOG_DEBUG1(LOG_TAG, "OTP1_MED:0x%X, ", IC[ic].statc.otp1_med);
        LOG_DEBUG1(LOG_TAG, "OTP1_ED:0x%X, ", IC[ic].statc.otp1_ed);
        LOG_DEBUG1(LOG_TAG, "VD_UV:0x%X, ", IC[ic].statc.vd_uv);
        LOG_DEBUG1(LOG_TAG, "VD_OV:0x%X, ", IC[ic].statc.vd_ov);
        LOG_DEBUG1(LOG_TAG, "VA_UV:0x%X, ", IC[ic].statc.va_uv);
        LOG_DEBUG1(LOG_TAG, "VA_OV:0x%X\r\n", IC[ic].statc.va_ov);

        LOG_DEBUG1(LOG_TAG, "OSCCHK:0x%X, ", IC[ic].statc.oscchk);
        LOG_DEBUG1(LOG_TAG, "TMODCHK:0x%X, ", IC[ic].statc.tmodchk);
        LOG_DEBUG1(LOG_TAG, "THSD:0x%X, ", IC[ic].statc.thsd);
        LOG_DEBUG1(LOG_TAG, "SLEEP:0x%X, ", IC[ic].statc.sleep);
        LOG_DEBUG1(LOG_TAG, "SPIFLT:0x%X, ", IC[ic].statc.spiflt);
        LOG_DEBUG1(LOG_TAG, "COMP:0x%X, ", IC[ic].statc.comp);
        LOG_DEBUG1(LOG_TAG, "VDEL:0x%X, ", IC[ic].statc.vdel);
        LOG_DEBUG1(LOG_TAG, "VDE:0x%X\r\n\r\n", IC[ic].statc.vde);

        LOG_DEBUG1(LOG_TAG, "Status D:\r\n");
        LOG_DEBUG1(LOG_TAG, "C1UV:0x%X, ", IC[ic].statd.c_uv[0]);
        LOG_DEBUG1(LOG_TAG, "C2UV:0x%X, ", IC[ic].statd.c_uv[1]);
        LOG_DEBUG1(LOG_TAG, "C3UV:0x%X, ", IC[ic].statd.c_uv[2]);
        LOG_DEBUG1(LOG_TAG, "C4UV:0x%X, ", IC[ic].statd.c_uv[3]);
        LOG_DEBUG1(LOG_TAG, "C5UV:0x%X, ", IC[ic].statd.c_uv[4]);
        LOG_DEBUG1(LOG_TAG, "C6UV:0x%X, ", IC[ic].statd.c_uv[5]);
        LOG_DEBUG1(LOG_TAG, "C7UV:0x%X, ", IC[ic].statd.c_uv[6]);
        LOG_DEBUG1(LOG_TAG, "C8UV:0x%X, ", IC[ic].statd.c_uv[7]);
        LOG_DEBUG1(LOG_TAG, "C9UV:0x%X, ", IC[ic].statd.c_uv[8]);
        LOG_DEBUG1(LOG_TAG, "C10UV:0x%X, ", IC[ic].statd.c_uv[9]);
        LOG_DEBUG1(LOG_TAG, "C11UV:0x%X, ", IC[ic].statd.c_uv[10]);
        LOG_DEBUG1(LOG_TAG, "C12UV:0x%X, ", IC[ic].statd.c_uv[11]);
        LOG_DEBUG1(LOG_TAG, "C13UV:0x%X, ", IC[ic].statd.c_uv[12]);
        LOG_DEBUG1(LOG_TAG, "C14UV:0x%X, ", IC[ic].statd.c_uv[13]);
        LOG_DEBUG1(LOG_TAG, "C15UV:0x%X, ", IC[ic].statd.c_uv[14]);
#if(CELL==16)
        LOG_DEBUG1(LOG_TAG, "C16UV:0x%X\r\n", IC[ic].statd.c_uv[15]);
#endif
        LOG_DEBUG1(LOG_TAG, "C1OV:0x%X, ", IC[ic].statd.c_ov[0]);
        LOG_DEBUG1(LOG_TAG, "C2OV:0x%X, ", IC[ic].statd.c_ov[1]);
        LOG_DEBUG1(LOG_TAG, "C3OV:0x%X, ", IC[ic].statd.c_ov[2]);
        LOG_DEBUG1(LOG_TAG, "C4OV:0x%X, ", IC[ic].statd.c_ov[3]);
        LOG_DEBUG1(LOG_TAG, "C5OV:0x%X, ", IC[ic].statd.c_ov[4]);
        LOG_DEBUG1(LOG_TAG, "C6OV:0x%X, ", IC[ic].statd.c_ov[5]);
        LOG_DEBUG1(LOG_TAG, "C7OV:0x%X, ", IC[ic].statd.c_ov[6]);
        LOG_DEBUG1(LOG_TAG, "C8OV:0x%X, ", IC[ic].statd.c_ov[7]);
        LOG_DEBUG1(LOG_TAG, "C9OV:0x%X, ", IC[ic].statd.c_ov[8]);
        LOG_DEBUG1(LOG_TAG, "C10OV:0x%X, ", IC[ic].statd.c_ov[9]);
        LOG_DEBUG1(LOG_TAG, "C11OV:0x%X, ", IC[ic].statd.c_ov[10]);
        LOG_DEBUG1(LOG_TAG, "C12OV:0x%X, ", IC[ic].statd.c_ov[11]);
        LOG_DEBUG1(LOG_TAG, "C13OV:0x%X, ", IC[ic].statd.c_ov[12]);
        LOG_DEBUG1(LOG_TAG, "C14OV:0x%X, ", IC[ic].statd.c_ov[13]);
        LOG_DEBUG1(LOG_TAG, "C15OV:0x%X, ", IC[ic].statd.c_ov[14]);
#if(CELL==16)
        LOG_DEBUG1(LOG_TAG, "C16OV:0x%X\r\n", IC[ic].statd.c_ov[15]);
#endif
        LOG_DEBUG1(LOG_TAG, "CTS:0x%X, ", IC[ic].statd.cts);
        LOG_DEBUG1(LOG_TAG, "CT:0x%X\r\n\r\n", IC[ic].statd.ct);

        LOG_DEBUG1(LOG_TAG, "Status E:\r\n");
        LOG_DEBUG1(LOG_TAG, "GPI:0x%X, ", IC[ic].state.gpi);
        LOG_DEBUG1(LOG_TAG, "REV_ID:0x%X\r\n\r\n", IC[ic].state.rev);

        LOG_DEBUG1(LOG_TAG, "CCount:%d, ", IC[ic].cccrc.cmd_cntr);
        LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n", IC[ic].cccrc.stat_pec);
      }
      else{ LOG_ERROR(LOG_TAG, "Wrong Register Group Select\r\n"); }
    }
  }
}

/**
 *******************************************************************************
 * Function: PrintDeviceSID
 * @brief Print Device SID.
 *
 * @details This function Print Device SID into IAR I/O terminal.
 *
 * Parameters:
 * @param [in]  tIC      Total IC
 *
 * @param [in]  *IC      cell_asic stucture pointer
 *
 * @param [in]  type     Enum type of resistor
 *
 * @return None
 *
 *******************************************************************************
*/
void printDeviceSID(uint8_t tIC, cell_asic *IC, TYPE type)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    LOG_DEBUG(LOG_TAG, "IC%d:\r\n",(ic+1));
    if(type == Sid)
    {
        LOG_DEBUG1(LOG_TAG, "Read Device SID:\r\n");
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].sid.sid[0]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].sid.sid[1]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].sid.sid[2]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].sid.sid[3]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].sid.sid[4]);
        LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].sid.sid[5]);
        LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
        LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n",IC[ic].cccrc.sid_pec);
     }
     else{ LOG_ERROR(LOG_TAG, "Wrong Register Type Select\r\n"); }
  }
}

/**
*******************************************************************************
* Function: printWritePwmDutyCycle
* @brief Print Write Pwm Duty Cycle.
*
* @details This function Print write pwm duty cycle value.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @param [in]  grp      Enum group of resistor
*
* @return None
*
*******************************************************************************
*/
void printWritePwmDutyCycle(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    LOG_DEBUG(LOG_TAG, "IC%d:\r\n",(ic+1));
    if(grp == A)
    {
      LOG_DEBUG1(LOG_TAG, "Write Pwma Duty Cycle:\r\n");
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwma.tx_data[0]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwma.tx_data[1]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwma.tx_data[2]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwma.tx_data[3]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwma.tx_data[4]);
      LOG_DEBUG1(LOG_TAG, "0x%X\r\n\r\n", IC[ic].pwma.tx_data[5]);
    }
    else if(grp == B)
    {
      LOG_DEBUG1(LOG_TAG, "Write Pwmb Duty Cycle:\r\n");
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwmb.tx_data[0]);
      LOG_DEBUG1(LOG_TAG, "0x%X\r\n\r\n", IC[ic].pwmb.tx_data[1]);
    }
    else if(grp == ALL_GRP)
    {
      LOG_DEBUG1(LOG_TAG, "Write Pwma Duty Cycle:\r\n");
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwma.tx_data[0]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwma.tx_data[1]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwma.tx_data[2]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwma.tx_data[3]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwma.tx_data[4]);
      LOG_DEBUG1(LOG_TAG, "0x%X\r\n", IC[ic].pwma.tx_data[5]);

      LOG_DEBUG1(LOG_TAG, "Write Pwmb Duty Cycle:\r\n");
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].pwmb.tx_data[0]);
      LOG_DEBUG1(LOG_TAG, "0x%X\r\n\r\n", IC[ic].pwmb.tx_data[1]);
    }
    else{ LOG_ERROR(LOG_TAG, "Wrong Register Group Select\r\n"); }
  }
}

/**
*******************************************************************************
* Function: printReadPwmDutyCycle
* @brief Print Read Pwm Duty Cycle.
*
* @details This function print read pwm duty cycle value.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @param [in]  grp      Enum group of resistor
*
* @return None
*
*******************************************************************************
*/
void printReadPwmDutyCycle(uint8_t tIC, cell_asic *IC, TYPE type, GRP grp)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    LOG_DEBUG(LOG_TAG, "IC%d:\r\n",(ic+1));
    if(grp == A)
    {
      LOG_DEBUG1(LOG_TAG, "Read PWMA Duty Cycle:\r\n");
      LOG_DEBUG1(LOG_TAG, "PWM1:0x%X, ", IC[ic].PwmA.pwma[0]);
      LOG_DEBUG1(LOG_TAG, "PWM2:0x%X, ", IC[ic].PwmA.pwma[1]);
      LOG_DEBUG1(LOG_TAG, "PWM3:0x%X, ", IC[ic].PwmA.pwma[2]);
      LOG_DEBUG1(LOG_TAG, "PWM4:0x%X, ", IC[ic].PwmA.pwma[3]);
      LOG_DEBUG1(LOG_TAG, "PWM5:0x%X, ", IC[ic].PwmA.pwma[4]);
      LOG_DEBUG1(LOG_TAG, "PWM6:0x%X, ", IC[ic].PwmA.pwma[5]);
      LOG_DEBUG1(LOG_TAG, "PWM7:0x%X, ", IC[ic].PwmA.pwma[6]);
      LOG_DEBUG1(LOG_TAG, "PWM8:0x%X, ", IC[ic].PwmA.pwma[7]);
      LOG_DEBUG1(LOG_TAG, "PWM9:0x%X, ", IC[ic].PwmA.pwma[8]);
      LOG_DEBUG1(LOG_TAG, "PWM10:0x%X, ", IC[ic].PwmA.pwma[9]);
      LOG_DEBUG1(LOG_TAG, "PWM11:0x%X, ", IC[ic].PwmA.pwma[10]);
      LOG_DEBUG1(LOG_TAG, "PWM12:0x%X, ", IC[ic].PwmA.pwma[11]);
      LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
      LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n",IC[ic].cccrc.pwm_pec);
    }
    else if(grp == B)
    {
      LOG_DEBUG1(LOG_TAG, "Read PWMB Duty Cycle:\r\n");
      LOG_DEBUG1(LOG_TAG, "PWM13:0x%X, ", IC[ic].PwmB.pwmb[0]);
      LOG_DEBUG1(LOG_TAG, "PWM14:0x%X, ", IC[ic].PwmB.pwmb[1]);
      LOG_DEBUG1(LOG_TAG, "PWM15:0x%X, ", IC[ic].PwmB.pwmb[2]);
      LOG_DEBUG1(LOG_TAG, "PWM16:0x%X, ", IC[ic].PwmB.pwmb[3]);
      LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
      LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n",IC[ic].cccrc.pwm_pec);
    }
    else if(grp == ALL_GRP)
    {
      LOG_DEBUG1(LOG_TAG, "Read PWMA Duty Cycle:\r\n");
      LOG_DEBUG1(LOG_TAG, "PWM1:0x%X, ", IC[ic].PwmA.pwma[0]);
      LOG_DEBUG1(LOG_TAG, "PWM2:0x%X, ", IC[ic].PwmA.pwma[1]);
      LOG_DEBUG1(LOG_TAG, "PWM3:0x%X, ", IC[ic].PwmA.pwma[2]);
      LOG_DEBUG1(LOG_TAG, "PWM4:0x%X, ", IC[ic].PwmA.pwma[3]);
      LOG_DEBUG1(LOG_TAG, "PWM5:0x%X, ", IC[ic].PwmA.pwma[4]);
      LOG_DEBUG1(LOG_TAG, "PWM6:0x%X, ", IC[ic].PwmA.pwma[5]);
      LOG_DEBUG1(LOG_TAG, "PWM7:0x%X, ", IC[ic].PwmA.pwma[6]);
      LOG_DEBUG1(LOG_TAG, "PWM8:0x%X, ", IC[ic].PwmA.pwma[7]);
      LOG_DEBUG1(LOG_TAG, "PWM9:0x%X, ", IC[ic].PwmA.pwma[8]);
      LOG_DEBUG1(LOG_TAG, "PWM10:0x%X, ", IC[ic].PwmA.pwma[9]);
      LOG_DEBUG1(LOG_TAG, "PWM11:0x%X, ", IC[ic].PwmA.pwma[10]);
      LOG_DEBUG1(LOG_TAG, "PWM12:0x%X\r\n", IC[ic].PwmA.pwma[11]);

      LOG_DEBUG1(LOG_TAG, "Read PWMB Duty Cycle:\r\n");
      LOG_DEBUG1(LOG_TAG, "PWM13:0x%X, ", IC[ic].PwmB.pwmb[0]);
      LOG_DEBUG1(LOG_TAG, "PWM14:0x%X, ", IC[ic].PwmB.pwmb[1]);
      LOG_DEBUG1(LOG_TAG, "PWM15:0x%X, ", IC[ic].PwmB.pwmb[2]);
      LOG_DEBUG1(LOG_TAG, "PWM16:0x%X, ", IC[ic].PwmB.pwmb[3]);
      LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
      LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n",IC[ic].cccrc.pwm_pec);
    }
    else{ LOG_ERROR(LOG_TAG, "Wrong Register Type Select\r\n"); }
  }
}

/**
*******************************************************************************
* Function: printWriteCommData
* @brief Print Write Comm data.
*
* @details This function Print write comm data.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @return None
*
*******************************************************************************
*/
void printWriteCommData(uint8_t tIC, cell_asic *IC, TYPE type)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    LOG_DEBUG(LOG_TAG, "IC%d:\r\n",(ic+1));
    if(type == Comm)
    {
      LOG_DEBUG1(LOG_TAG, "Write Comm Data:\r\n");
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].com.tx_data[0]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].com.tx_data[1]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].com.tx_data[2]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].com.tx_data[3]);
      LOG_DEBUG1(LOG_TAG, "0x%X, ", IC[ic].com.tx_data[4]);
      LOG_DEBUG1(LOG_TAG, "0x%X\r\n\r\n", IC[ic].com.tx_data[5]);
    }
    else{ LOG_ERROR(LOG_TAG, "Wrong Register Group Select\r\n"); }
  }
}

/**
*******************************************************************************
* Function: printReadCommData
* @brief Print Read Comm Data.
*
* @details This function print read comm data.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @return None
*
*******************************************************************************
*/
void printReadCommData(uint8_t tIC, cell_asic *IC, TYPE type)
{
  for(uint8_t ic = 0; ic < tIC; ic++)
  {
    LOG_DEBUG(LOG_TAG, "IC%d:\r\n",(ic+1));
    if(type == Comm)
    {
      LOG_DEBUG1(LOG_TAG, "Read Comm Data:\r\n");
      LOG_DEBUG1(LOG_TAG, "ICOM0:0x%X, ", IC[ic].comm.icomm[0]);
      LOG_DEBUG1(LOG_TAG, "ICOM1:0x%X, ", IC[ic].comm.icomm[1]);
      LOG_DEBUG1(LOG_TAG, "ICOM2:0x%X\r\n", IC[ic].comm.icomm[2]);
      LOG_DEBUG1(LOG_TAG, "FCOM0:0x%X, ", IC[ic].comm.fcomm[0]);
      LOG_DEBUG1(LOG_TAG, "FCOM1:0x%X, ", IC[ic].comm.fcomm[1]);
      LOG_DEBUG1(LOG_TAG, "FCOM2:0x%X\r\n", IC[ic].comm.fcomm[2]);
      LOG_DEBUG1(LOG_TAG, "DATA0:0x%X, ", IC[ic].comm.data[0]);
      LOG_DEBUG1(LOG_TAG, "DATA1:0x%X, ", IC[ic].comm.data[1]);
      LOG_DEBUG1(LOG_TAG, "DATA2:0x%X\r\n", IC[ic].comm.data[2]);
      LOG_DEBUG1(LOG_TAG, "CCount:%d,",IC[ic].cccrc.cmd_cntr);
      LOG_DEBUG1(LOG_TAG, "PECError:%d\r\n\r\n",IC[ic].cccrc.comm_pec);
    }
    else{ LOG_ERROR(LOG_TAG, "Wrong Register Type Select\r\n"); }
  }
}

/**
*******************************************************************************
* Function: printDiagnosticTestResult
* @brief Print diagnostic test result.
*
* @details This function Print diagnostic test result (PASS,FAIL) into console terminal.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  TEST     Enum type diagnostic test
*
* @return None
*
*******************************************************************************
*/
void printDiagnosticTestResult(uint8_t tIC, cell_asic *IC, DIAGNOSTIC_TYPE type)
{
  if(type == OSC_MISMATCH)
  {
    LOG_DEBUG(LOG_TAG, "OSC Diagnostic Test:\r\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      LOG_DEBUG1(LOG_TAG, "IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.osc_mismatch);
    }
    LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
  }

  else if(type == SUPPLY_ERROR)
  {
    LOG_DEBUG(LOG_TAG, "Force Supply Error Detection Test:\r\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      LOG_DEBUG1(LOG_TAG, "IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.supply_error);
    }
    LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
  }

  else if(type == THSD)
  {
    LOG_DEBUG(LOG_TAG, "Thsd Diagnostic Test:\r\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      LOG_DEBUG1(LOG_TAG, "IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.thsd);
    }
    LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
  }

  else if(type == FUSE_ED)
  {
    LOG_DEBUG(LOG_TAG, "Fuse_ed Diagnostic Test:\r\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      LOG_DEBUG1(LOG_TAG, "IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.fuse_ed);
    }
    LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
  }

  else if(type == FUSE_MED)
  {
    LOG_DEBUG(LOG_TAG, "Fuse_med Diagnostic Test:\r\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      LOG_DEBUG1(LOG_TAG, "IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.fuse_med);
    }
    LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
  }

  else if(type == TMODCHK)
  {
    LOG_DEBUG(LOG_TAG, "TMODCHK Diagnostic Test:\r\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      LOG_DEBUG1(LOG_TAG, "IC%d:",(ic+1));
      diagnosticTestResultPrint(IC[ic].diag_result.tmodchk);
    }
    LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
  }
  else{LOG_ERROR(LOG_TAG, "Wrong Diagnostic Selected\r\n");}
}

/**
*******************************************************************************
* Function: diagnosticResultPrint
* @brief Print diagnostic (PASS/FAIL) result.
*
* @details This function print diagnostic (PASS/FAIL) result into console.
*
* Parameters:
* @param [in]   result   Result byte
*
* @return None
*
*******************************************************************************
*/
void diagnosticTestResultPrint(uint8_t result)
{
  if(result == 1)
  {
    LOG_DEBUG(LOG_TAG, "PASS\r\n");
  }
  else
  {
    LOG_DEBUG(LOG_TAG, "FAIL\r\n");
  }
}

/**
*******************************************************************************
* Function: printOpenWireTestResult
* @brief Print open wire test result.
*
* @details This function print open wire test result.
*
* Parameters:
* @param [in]   tIC      Total IC
*
* @param [in]  *IC      cell_asic stucture pointer
*
* @param [in]  type     Enum type of resistor
*
* @return None
*
*******************************************************************************
*/
void printOpenWireTestResult(uint8_t tIC, cell_asic *IC, TYPE type)
{
  if(type == Cell)
  {
    LOG_DEBUG(LOG_TAG, "Cell Open Wire Test\r\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      LOG_DEBUG1(LOG_TAG, "IC%d:\r\n",(ic+1));
      for(uint8_t cell = 0; cell < CELL; cell++)
      {
        LOG_DEBUG1(LOG_TAG, "CELL%d:",(cell+1));
        openWireResultPrint(IC[ic].diag_result.cell_ow[cell]);
      }
      LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
    }
  }
  else if(type == S_volt)
  {
    LOG_DEBUG(LOG_TAG, "Cell redundant Open Wire Test\r\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      LOG_DEBUG1(LOG_TAG, "IC%d:\r\n",(ic+1));
      for(uint8_t cell = 0; cell < CELL; cell++)
      {
        LOG_DEBUG1(LOG_TAG, "CELL%d:",(cell+1));
        openWireResultPrint(IC[ic].diag_result.cellred_ow[cell]);
      }
      LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
    }
  }
  else if(type == Aux)
  {
    LOG_DEBUG(LOG_TAG, "Aux Open Wire Test\r\n");
    for(uint8_t ic = 0; ic < tIC; ic++)
    {
      LOG_DEBUG1(LOG_TAG, "IC%d:\r\n",(ic+1));
      for(uint8_t gpio = 0; gpio < (AUX-2); gpio++)
      {
        LOG_DEBUG1(LOG_TAG, "GPIO%d:",(gpio+1));
        openWireResultPrint(IC[ic].diag_result.aux_ow[gpio]);
      }
      LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
    }
  }
  else{LOG_ERROR(LOG_TAG, "Wrong Resistor Type Selected\r\n");}
}

/**
*******************************************************************************
* Function: openWireResultPrint
* @brief Print open wire (OPEN/CLOSE) result.
*
* @details This function print open wire result into console.
*
* Parameters:
* @param [in]   result   Result byte
*
* @return None
*
*******************************************************************************
*/
void openWireResultPrint(uint8_t result)
{
  if(result == 1)
  {
    LOG_DEBUG(LOG_TAG, " OPEN\r\n");
  }
  else
  {
    LOG_DEBUG(LOG_TAG, " CLOSE\r\n");
  }
}

/**
 *******************************************************************************
 * Function: printPollAdcConvTime
 * @brief Print Poll adc conversion Time.
 *
 * @details This function print poll adc conversion Time.
 *
 * @return None
 *
 *******************************************************************************
*/
void printPollAdcConvTime(int count)
{
  LOG_DEBUG(LOG_TAG, "Adc Conversion Time = %d ms\r\n", count);
}

/**
 *******************************************************************************
 * Function: printMenu
 * @brief Print Command Menu.
 *
 * @details This function print all command menu.
 *
 * @return None
 *
 *******************************************************************************
*/
void printMenu()
{
  LOG_DEBUG(LOG_TAG, "List of ADBMS6830 Command:\r\n");
  LOG_DEBUG1(LOG_TAG, "Write and Read Configuration: 1         |Clear All Flags: 31                          \r\n");
  LOG_DEBUG1(LOG_TAG, "Read Configuration: 2                   |Set DCC Discharge: 32                        \r\n");
  LOG_DEBUG1(LOG_TAG, "Start Cell Voltage Conversion: 3        |Clear DCC Discharge: 33                      \r\n");
  LOG_DEBUG1(LOG_TAG, "Read Cell Voltages: 4                   |Write and Read PWM Register: 34              \r\n");
  LOG_DEBUG1(LOG_TAG, "Start S-Voltage Conversion: 5           |GPIO SPI Communication: 35                   \r\n");
  LOG_DEBUG1(LOG_TAG, "Read S-Voltages: 6                      |GPIO I2C Communication Write to Slave: 36    \r\n");
  LOG_DEBUG1(LOG_TAG, "Start Avg Cell Voltage Conversion: 7    |GPIO I2C Communication Read from Slave: 37   \r\n");
  LOG_DEBUG1(LOG_TAG, "Read Avg Cell Voltages: 8               |Set PWM Discharge time out value: 38         \r\n");
  LOG_DEBUG1(LOG_TAG, "Start F-Cell Voltage Conversion: 9      |OSC Mismatch self test: 39                   \r\n");
  LOG_DEBUG1(LOG_TAG, "Read F-Cell Voltages: 10                |Run Supply Error detection self test: 40     \r\n");
  LOG_DEBUG1(LOG_TAG, "Start Aux Voltage Conversion: 11        |Run Thermal Shutdown self test: 41           \r\n");
  LOG_DEBUG1(LOG_TAG, "Read Aux Voltages: 12                   |Run Fuse_ed self test: 42                    \r\n");
  LOG_DEBUG1(LOG_TAG, "Start RAux Voltage Conversion: 13       |Run Fuse_med self test: 43                   \r\n");
  LOG_DEBUG1(LOG_TAG, "Read RAux Voltages: 14                  |Run Tmodchk self test: 44                    \r\n");
  LOG_DEBUG1(LOG_TAG, "Read Status Registers: 15               |Check CSFLT status bits: 45                  \r\n");
  LOG_DEBUG1(LOG_TAG, "Loop Measurements: 16                   |Check ERR bit functionality: 46              \r\n");
  LOG_DEBUG1(LOG_TAG, "Read Device SID: 17                     |Cell Open Wire Test: 47                      \r\n");
  LOG_DEBUG1(LOG_TAG, "Set or reset the gpio pins: 18          |Redundant Cell Open Wire Test: 48            \r\n");
  LOG_DEBUG1(LOG_TAG, "Enable MUTE: 19                         |Aux Open Wire Test: 49                       \r\n");
  LOG_DEBUG1(LOG_TAG, "Disable MUTE: 20                        |Read RDCVALL cmd Voltages: 50                \r\n");
  LOG_DEBUG1(LOG_TAG, "Soft Reset: 21                          |Read RDACALL cmd Voltages: 51                \r\n");
  LOG_DEBUG1(LOG_TAG, "Reset Command Counter: 22               |Read RDSALL cmd Voltages: 52                 \r\n");
  LOG_DEBUG1(LOG_TAG, "Reset PEC Error Flags: 23               |Read RDFCALL cmd Voltages: 53                \r\n");
  LOG_DEBUG1(LOG_TAG, "Snapshot: 24                            |Read RDCSALL cmd Voltages: 54                \r\n");
  LOG_DEBUG1(LOG_TAG, "Release Snapshot: 25                    |Read RDACSALL cmd Voltages: 55               \r\n");
  LOG_DEBUG1(LOG_TAG, "Clear Cell registers: 26                |Read RDASALL cmd Voltages: 56                \r\n");
  LOG_DEBUG1(LOG_TAG, "Clear Aux registers: 27                 |                                             \r\n");
  LOG_DEBUG1(LOG_TAG, "Clear Spin registers: 28                |                                             \r\n");
  LOG_DEBUG1(LOG_TAG, "Clear Fcell registers: 29               |                                             \r\n");
  LOG_DEBUG1(LOG_TAG, "Clear OVUV registers: 30                |                                             \r\n");

  LOG_DEBUG1(LOG_TAG, "\r\n");
  LOG_DEBUG1(LOG_TAG, "Print '0' for menu\r\n");
  LOG_DEBUG1(LOG_TAG, "Please enter command: \r\n");
  LOG_DEBUG1(LOG_TAG, "\r\n\r\n");
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
float getVoltage(int16_t data)
{
    float voltage_float; //voltage in Volts

    //Raw value above 0x7FFF till 0XFFF will be negative so 2's complement.
    if(data > 0x7FFF) {
        data = 1+ (~data); //Taking one's complement and then adding 1 to get two's complement
        voltage_float = ((10000.0 - data) * 0.000150);
    } else {
        voltage_float = ((data + 10000) * 0.000150);
    }
    return voltage_float;
}

uint16_t getMVoltage(int16_t data)
{
    float voltage_float; //voltage in Volts

    //Raw value above 0x7FFF till 0XFFF will be negative so 2's complement.
    if(data > 0x7FFF) {
        data = 1+ (~data); //Taking one's complement and then adding 1 to get two's complement
        voltage_float = ((10000.0 - data) * 0.150);
    } else {
        voltage_float = ((data + 10000) * 0.150);
    }
    return (uint16_t)voltage_float;
}

void adBms6830_AllDccStart(void)
{
    uint8_t cmd[] = {0x00, 0x24, 0xB1, 0x9E, 0x98, 0x5B, 0x46, 0x82, 0xFF, 0xFF, 0x01, 0x23};
    spiWriteBytes(4, &cmd[0]);
}

void adBms6830_AllDccStop(void)
{
    uint8_t cmd[] = {0x00, 0x24, 0xB1, 0x9E, 0x98, 0x5B, 0x46, 0x82, 0x00, 0x00, 0x01, 0x6E};
    spiWriteBytes(4, &cmd[0]);
}

/**
*******************************************************************************
* @brief Cell open ow_even and ow_odd volatge collect
*******************************************************************************
*/
void adBms6830_cell_ow_volatge_collect(uint8_t tIC, cell_asic *ic, TYPE type, OW_C_S ow_c_s)
{
  if(type == Cell)
  {
    if(ow_c_s == OW_ON_EVEN_CH)
    {
      for (uint8_t cic = 0; cic < tIC; cic++)
      {
        for (uint8_t cell = 0; cell < CELL; cell++)
        {
          ic[cic].owcell.cell_ow_even[cell] = ic[cic].cell.c_codes[cell];
        }
      }
    }
    else
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
    if(ow_c_s == OW_ON_EVEN_CH)
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
*******************************************************************************
* @brief Open wire detection condtion check
*******************************************************************************
*/
void adBms6830_open_wire_detection_condtion_check(uint8_t tIC, cell_asic *ic, TYPE type)
{
  if(type == Cell)
  {
    for (uint8_t cic = 0; cic < tIC; cic++)
    {
      for (uint8_t cell = 0; cell < CELL; cell++)
      {
        if((ic[cic].owcell.cell_ow_even[cell] - ic[cic].owcell.cell_ow_odd[cell]) > 1500)
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
      for (uint8_t gpio = 0; gpio < (AUX-2); gpio++)
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

    printf("Wrong Resistor Type Selected\n");

  }
}
