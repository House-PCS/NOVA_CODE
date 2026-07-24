/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
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
  
  
  
#include "epara_asp.h"
#include "dev_24c32.h"
#include "mcu_bsp.h"
#include "fpara_asp.h"

#define EEPROM_WRITE    2   //写数据方式，1:固定位置写入，2:优化循环写入，提升寿命
/******************
地址规划：
4K字节，循环存储,分为8块，寿命提升8倍

*///////////////////
uint32_t EParaData[EPARA_ASP_NUM]={0};
const uint32_t *const IR_EParaData = EParaData;
//uint32_t *const IR_EParaDataWrite = EParaData;

/** 
* @brief  EEPROM参数读函数
* @param  none
* @return  bool：读取参数是否有效
* @note:
*
*/
bool EParaRead()
{
    EepromRead((uint8_t*)EParaData,0,EPARA_ASP_NUM*4);
    if(EParaData[EPARA_ASP_RESET_CNT] == 0xFFFFFFFF)
    {
        return false;
    }
    return true;
    
}
#define EPARA_BASE_ADDR     0
//#define EPARA_END_ADDR     1024
#define EPARA_BACK_CNT  4   //单个数据存储寿命提升 EPARA_BACK_CNT 倍，需考虑存储数据量及总容量因素
typedef struct 
{
    uint8_t spot;   //为初始化方便，从1开始计数
    uint32_t cnt;
}
EParaSaveType;
EParaSaveType EParaSaveInfo[EPARA_ASP_NUM] = {0};
bool EParaRead_1()
{
    uint16_t i = 0, j = 0;
    uint32_t cnt[EPARA_BACK_CNT] = {0};
   
    EepromRead((uint8_t*)&cnt[0],EPARA_BASE_ADDR,4);
    if(cnt[0] == 0xFFFFFFFF)
    {
//        for(i = 0; i < EPARA_ASP_NUM; i++)
//        {
//            for(j = 0; j < 4; j++)
//            {
//                EepromWrite(&EEPROM_I2cHandle,(uint8_t*)&cnt[1],EPARA_BASE_ADDR + i*7*EPARA_BACK_CNT + j*7,3);
//            }
//        }
        return false;
    }
    for(i = 0; i < EPARA_ASP_NUM; i++)
    {
//        for(j = 0; j < 4; j++)
//        {
//            EepromRead(&EEPROM_I2cHandle,(uint8_t*)&cnt[j],EPARA_SAVE_ADDR+i*7,3);
//        }
        j = 0;
        EepromRead((uint8_t*)&cnt[j],EPARA_BASE_ADDR + i*8*EPARA_BACK_CNT + j*8,4);
        j++;
        do{
            EepromRead((uint8_t*)&cnt[j],EPARA_BASE_ADDR + i*8*EPARA_BACK_CNT + j*8,4);
            if(cnt[j-1] > cnt[j] || (cnt[j] == 0xFFFFFFFF))
            {
                break;
            }
            j++;
        }while(j < EPARA_BACK_CNT);
        
        EParaSaveInfo[i].spot = j;
        EParaSaveInfo[i].cnt = cnt[j-1];
        EepromRead((uint8_t*)&EParaData[i],EPARA_BASE_ADDR + i*8*EPARA_BACK_CNT + (j - 1)*8 + 4,4);
    }
    return true;
}
void EParaWrite_1(EN_EPARA_ASP num,uint32_t data)
{    
    EParaData[num] = data;
    if(EParaSaveInfo[num].spot == EPARA_BACK_CNT)
    {
        EParaSaveInfo[num].spot = 1;
    }
    else 
    {
        EParaSaveInfo[num].spot++;
    }
    EParaSaveInfo[num].cnt++;
    EepromWrite((uint8_t*)&EParaData[num],EPARA_BASE_ADDR + num*8*EPARA_BACK_CNT + (EParaSaveInfo[num].spot-1)*8 + 4,4);
    EepromWrite((uint8_t*)&EParaSaveInfo[num].cnt,EPARA_BASE_ADDR + num*8*EPARA_BACK_CNT + (EParaSaveInfo[num].spot-1)*8,4);
}
/** 
* @brief  EEPROM参数写函数
* @param  num：EEPROM参数序号
* @return  data：数据
* @note:
*
*/
#if (EEPROM_WRITE == 1)
void EParaWrite(EN_EPARA_ASP num,uint32_t data)
{    
    EParaData[num] = data;
    EepromWrite(&EEPROM_I2cHandle,(uint8_t*)&EParaData[num],num*4,4);
}
#elif (EEPROM_WRITE == 2)
void EParaWrite(EN_EPARA_ASP num,uint32_t data)
{    
    EepromWrite((uint8_t*)EParaData,0,EPARA_ASP_NUM*4);
}
#endif

/** 
* @brief  EEPROM参数初始化
* @param  none
* @return  none
* @note:
*
*/
void EParaAspInit()
{
#if (EEPROM_WRITE == 1)
    if(!EParaRead())     //读EPARA
#elif (EEPROM_WRITE == 2)
    if(!EParaRead_1())     //读EPARA
#endif    
    
    {
        EParaData[EPARA_ASP_RESET_CNT] = 0;
        EParaData[EPARA_ASP_SOC] = 0;
        EParaData[EPARA_ASP_Q_MAX] = IR_PARA_Qmax;
        EParaData[EPARA_ASP_Q_REAL] = IR_PARA_Qmax;
        EParaData[EPARA_ASP_Q_RESIDUAL] = 0;
        EParaData[EPARA_ASP_ACCUM_Q_CHARGE] = 0;
        EParaData[EPARA_ASP_ACCUM_Q_DISCHARGE] = 0;
        EParaData[EPARA_ASP_ONCE_ACCUM_KWH_CHARGE] = 0;
        EParaData[EPARA_ASP_ONCE_ACCUM_KWH_DISCHARGE] = 0;
        EParaData[EPARA_ASP_DAY_ACCUM_KWH_CHARGE] = 0;
        EParaData[EPARA_ASP_DAY_ACCUM_KWH_DISCHARGE] = 0;
        EParaData[EPARA_ASP_TOTAL_ACCUM_KWH_CHARGE] = 0;
        EParaData[EPARA_ASP_TOTAL_ACCUM_KWH_DISCHARGE] = 0;
        
#if (EEPROM_WRITE == 1)
        EepromWrite(&EEPROM_I2cHandle,(uint8_t*)EParaData,0,EPARA_ASP_NUM*4);
#elif (EEPROM_WRITE == 2)
        for(uint8_t i = 0; i < EPARA_ASP_NUM; i++)
        {
            EParaWrite_1(i,EParaData[i]);
        }
#endif
    }
    else
    {
        EParaData[EPARA_ASP_RESET_CNT]++;
        if(EParaData[EPARA_ASP_RESET_CNT]>= 0xfffffff)     //产品周期不为0xFFFFFFFF
        {
            EParaData[EPARA_ASP_RESET_CNT] = 0;
        }
#if (EEPROM_WRITE == 1)
        EParaWrite(EPARA_ASP_RESET_CNT,EParaData[EPARA_ASP_RESET_CNT]);
#elif (EEPROM_WRITE == 2)
        EParaWrite_1(EPARA_ASP_RESET_CNT,EParaData[EPARA_ASP_RESET_CNT]);
#endif
        
    }
    
}
