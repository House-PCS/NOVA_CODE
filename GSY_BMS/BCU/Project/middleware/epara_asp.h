/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.h
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
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __EPARA_ASP_H_
#define __EPARA_ASP_H_

#include <stdint.h>

typedef enum
{
    EPARA_ASP_RESET_CNT=0,

    EPARA_ASP_SOC,
//    EPARA_ASP_SOH,
    EPARA_ASP_Q_MAX,
    EPARA_ASP_Q_REAL,
    EPARA_ASP_Q_RESIDUAL,
    EPARA_ASP_ACCUM_Q_CHARGE,
    EPARA_ASP_ACCUM_Q_DISCHARGE,
    
    EPARA_ASP_ONCE_ACCUM_KWH_CHARGE,
    EPARA_ASP_ONCE_ACCUM_KWH_DISCHARGE,
    EPARA_ASP_DAY_ACCUM_KWH_CHARGE,
    EPARA_ASP_DAY_ACCUM_KWH_DISCHARGE,
    EPARA_ASP_TOTAL_ACCUM_KWH_CHARGE,
    EPARA_ASP_TOTAL_ACCUM_KWH_DISCHARGE,
    
    EPARA_ASP_NUM
}EN_EPARA_ASP;

extern const uint32_t *const IR_EParaData;
extern uint32_t *const IR_EParaDataWrite;

/////////////////////////////读数据接口/////////////////////////////////////////////////////
#define IR_EParaSoc                      IR_EParaData[EPARA_ASP_SOC]
#define IR_EParaQmax                     IR_EParaData[EPARA_ASP_Q_MAX]
#define IR_EParaQreal                    IR_EParaData[EPARA_ASP_Q_REAL]
#define IR_EParaQresidual                IR_EParaData[EPARA_ASP_Q_RESIDUAL]
#define IR_EParaAccumQcharge             IR_EParaData[EPARA_ASP_ACCUM_Q_CHARGE]
#define IR_EParaAccumQdischarge          IR_EParaData[EPARA_ASP_ACCUM_Q_DISCHARGE]

#define IR_EParaOnceAccumKwhCharge       IR_EParaData[EPARA_ASP_ONCE_ACCUM_KWH_CHARGE]
#define IR_EParaOnceAccumKwhDisCharge    IR_EParaData[EPARA_ASP_ONCE_ACCUM_KWH_DISCHARGE]
#define IR_EParaDayAccumKwhCharge        IR_EParaData[EPARA_ASP_DAY_ACCUM_KWH_CHARGE]
#define IR_EParaDayAccumKwhDisCharge     IR_EParaData[EPARA_ASP_DAY_ACCUM_KWH_DISCHARGE]
#define IR_EParaTotalAccumKwhCharge      IR_EParaData[EPARA_ASP_TOTAL_ACCUM_KWH_CHARGE]
#define IR_EParaTotalAccumKwhDisCharge   IR_EParaData[EPARA_ASP_TOTAL_ACCUM_KWH_DISCHARGE]

/////////////////////////////写数据接口/////////////////////////////////////////////////////
//#define IR_EParaWriteSoc(x)                     IR_EParaDataWrite[EPARA_ASP_SOC] = x
//#define IR_EParaWriteQmax(x)                    IR_EParaDataWrite[EPARA_ASP_Q_MAX] = x
//#define IR_EParaWriteQreal(x)                   IR_EParaDataWrite[EPARA_ASP_Q_REAL] = x
//#define IR_EParaWriteQresidual(x)               IR_EParaDataWrite[EPARA_ASP_Q_RESIDUAL] = x
//#define IR_EParaWriteAccumQcharge(x)            IR_EParaDataWrite[EPARA_ASP_ACCUM_Q_CHARGE] = x
//#define IR_EParaWriteAccumQdischarge(x)         IR_EParaDataWrite[EPARA_ASP_ACCUM_Q_DISCHARGE] = x

//#define IR_EParaWriteOnceAccumKwhCharge         IR_EParaDataWrite[EPARA_ASP_ONCE_ACCUM_KWH_CHARGE]
//#define IR_EParaWriteOnceAccumKwhDisCharge      IR_EParaDataWrite[EPARA_ASP_ONCE_ACCUM_KWH_DISCHARGE]
//#define IR_EParaWriteDayAccumKwhCharge          IR_EParaDataWrite[EPARA_ASP_DAY_ACCUM_KWH_CHARGE]
//#define IR_EParaWriteDayAccumKwhDisCharge       IR_EParaDataWrite[EPARA_ASP_DAY_ACCUM_KWH_DISCHARGE]
//#define IR_EParaWriteTotalAccumKwhCharge        IR_EParaDataWrite[EPARA_ASP_TOTAL_ACCUM_KWH_CHARGE]
//#define IR_EParaWriteTotalAccumKwhDisCharge     IR_EParaDataWrite[EPARA_ASP_TOTAL_ACCUM_KWH_DISCHARGE]


#define EEPROM_BLOCK_NUM    32
#define EEPROM_BLOCK_BYTES    32
#define EEPROM_BYTES    4096



void EParaWrite(EN_EPARA_ASP num,uint32_t data);
void EParaAspInit(void);

#endif
