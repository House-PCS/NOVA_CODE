/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        exmc.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-21
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-21    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __EXMC_H_
#define __EXMC_H_

#include "drv_opt.h"



/****SDRAM****/
#define SDRAM_TIMEOUT                            ((uint32_t)0x0000FFFF)
/* Burst Length */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0003)

/* Burst Type */
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)

/* CAS Latency */
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)

/* Write Mode */
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)



typedef enum
{
    EN_EXMC_MODE_MASK_SRAM      = 0x01,
    EN_EXMC_MODE_MASK_PSRAM     = 0x02,
    EN_EXMC_MODE_MASK_ROM       = 0x04,
    EN_EXMC_MODE_MASK_NOR       = 0x08,
    EN_EXMC_MODE_MASK_NAND      = 0x10,
    EN_EXMC_MODE_MASK_PC_CARD   = 0x20,
    EN_EXMC_MODE_MASK_SDRAM     = 0x40,
}EN_EXMC_MODE_MASK;



typedef struct _exmc_config_stru_t
{
    uint32_t            gpio_a_pinx; /*multi pins*/
    uint32_t            gpio_b_pinx;
    uint32_t            gpio_c_pinx; 
    uint32_t            gpio_d_pinx;
    uint32_t            gpio_e_pinx;
    uint32_t            gpio_f_pinx;
    uint32_t            gpio_g_pinx;
    uint32_t            gpio_h_pinx;
    uint32_t            gpio_i_pinx;
    
    bool                sdram_en;
    uint32_t            sdram_device_id;/*根据硬件电路管脚选择决定：暂仅支持单路 
                                         EXMC_SDRAM_DEVICE0(0xC0000000~0xCFFFFFFF) EXMC_SDRAM_DEVICE1(0xD0000000~0xDFFFFFFF)*/
//    exmc_sdram_timing_parameter_struct      sdram_timing; //默认在驱动代码里写死
//    exmc_sdram_command_parameter_struct     sdram_cmd; //默认在驱动代码里写死
//    exmc_sdram_parameter_struct             sdram_para;//默认在驱动代码里写死
    
    bool                sram_en;
    uint32_t            sram_region_id;/*根据硬件电路管脚选择决定：暂仅支持单路
                                         EXMC_BANK0_NORSRAM_REGION0(0x60000000~0x63FFFFFF) EXMC_BANK0_NORSRAM_REGION2(0x64000000~0x67FFFFFF)
                                         EXMC_BANK0_NORSRAM_REGION2(0x68000000~0x6BFFFFFF) EXMC_BANK0_NORSRAM_REGION2(0x6C000000~0x6FFFFFFF)*/
//    exmc_norsram_timing_parameter_struct    sram_timing; //默认在驱动代码里写死 EXMC_BANK0_NORSRAM_REGION2
//    exmc_norsram_parameter_struct           sram_para; //默认在驱动代码里写死
    
    
}ExmcConfig_t;


typedef struct _exmc_handle_stru_t
{
    ExmcConfig_t        *exmc_config;
    uint32_t            sram_start_addr;
}Exmc_HandleType;


void ExmcInit(Exmc_HandleType *pHandle, EN_EXMC_MODE_MASK mask);
void ExmcSdramInit(Exmc_HandleType *pHandle);
void ExmcSramInit(Exmc_HandleType *pHandle);


#endif















