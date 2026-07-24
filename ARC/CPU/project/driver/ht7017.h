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
#ifndef __HT7017_H_
#define __HT7017_H_



#define HT_HEAD         0x6A
#define ACK_CORRECT     0x54


#define HT_RO   0
#define HT_WR   1

//需要写入的寄存器值
#define WRITE_EN_2    0x00BC

#define ModuleEn_Data    0x0000

//寄存器地址
#define HT_SQL_I1   0x00
#define HT_SQL_I2   0x01

#define HT_RMS_I1   0x06
#define HT_RMS_I2   0x07
#define HT_RMS_U    0x08

#define HT_WPCFG        0x32
#define HT_ModuleEn     0x42
#define HT_ANAEN        0x43



#define HT_COM_TIMEOUT  2000     //单位 1ms

unsigned char HT_Read(unsigned char add,unsigned char *pdata);
unsigned char HT_Write(unsigned char add,unsigned short data,unsigned char *pdata);
    
#endif
