/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can.c
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
  * <tr><td>2022-11-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#ifndef _PACK_PARA_ASP__
#define _PACK_PARA_ASP__

#include <stdint.h>
#include "AppCfg.h"




#ifdef  CMC_64T68
#define CELL_SAMPLE_MAX             64
#define CELL_TEMP_SAMPLE_MAX        68

#elif   (defined  CMC_32T34)
#define CELL_SAMPLE_MAX             32
#define CELL_TEMP_SAMPLE_MAX        34
#else
#define CELL_SAMPLE_MAX             1
#define CELL_TEMP_SAMPLE_MAX        1
#endif

//#define PACK_PARA_NUM   200
#pragma pack(1) 
typedef struct _pack_para_stru_t
{
    uint8_t CellValid[(CELL_SAMPLE_MAX+7)/8];     //电池路数使能
    uint8_t CellTempValid[(CELL_TEMP_SAMPLE_MAX+7)/8];    //温度路数使能
    uint8_t PoleTempValid[(CELL_TEMP_SAMPLE_MAX+7)/8];    //极柱温度使能
    uint8_t OtherTempValid[(CELL_TEMP_SAMPLE_MAX+7)/8];    //其他温度使能
    uint8_t TempErrAllowNum;    //温度采样错误冗余路数
}PackParaType;
#pragma pack()

#define PACK_PARA_NUM   (sizeof(PackParaType))

extern const PackParaType *const IR_PackPara;


#endif
