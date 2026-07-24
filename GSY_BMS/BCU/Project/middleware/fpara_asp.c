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
  
  
#include "fpara_asp.h"
#include "string.h"
#include "systick.h"


FparaInfoType BmsPara={0};
const FparaInfoType* const IR_BmsPara=&BmsPara;
FparaInfoType* const IR_Write_BmsPara=&BmsPara;
const FparaInfoType BmsPara_Cfg={
    0x0009,     //版本号
    {
        280000,
        1400,560,
        1400,560,
        
        3550,
        3600,
        1000,
        2900,
        2850,
        0,
        
        100,
        3100,
        
        25,
        24,
        
        50,     //预充压差
        100,     //采集与累加压差
        
        true,      //上电指令有效使能
    },
    {   
        3650,
        3650,
        3650,        
        300,
        
        2700,
        2700,
        2700,        
        100,
        
        200,
        400,
        600,        
        100,
        
        45,50,55,3,     //charge_temp_high
        10,0,-10,3,        //charge_temp_low
        45,50,55,3,     //discharge_temp_high
        0,-10,-20,3,        //discharge_temp_low
        10,15,20,3,     //cell_temp_diff
        
        8520,8640,8700,720,   //total_vol_high
        6720,6600,6480,240,         //total_vol_low
        
        1450,1500,1650,50,      //charge_curr_high
        1450,1500,1650,50,      //discharge_curr_high
        
        500,200,100,10,          //res_low
        55,60,65,5,             //pole_temp_high
    },
    {
        4,
        30,
        75,
        20, 
    },
    {
        {2,0x0a,0x0f,0x0e,0x0d,6},
        false,          //DHCP_EN
        {10,50,3,12},
        {255,255,255,0},
        {10,50,3,1},
        251,
    },
};
//////////////////////BOOT_INFO////////////////////////////
const BootInfoType BootInfoCfg = {
    4,
    0x1001,
    {"SN123456789123456789"},
    {"CMC_32T34",}
};
BootInfoType BootInfo;
const BootInfoType* const IR_BootInfo = &BootInfo;
BootInfoType* const IR_Write_BootInfo = &BootInfo;
///////////////////////////////////////////////////////////////////////////////////////////
/*
运行参数存储于内部FLASH，使用两个区域乒乓模式进行存储，区域最尾端最后写入有效标识，防止写失败。
循环策略：区域最后一个字节存入有效标志，有效标志每写一次++，有效标识大者有效。

*/
FparaInfoType BmsPara_Flash;   //备份写入FLASH的参数
/** 
* @brief  系统参数初始化
* @param  buff：Flash数据
* @return  none
* @note:
*
*/
bool FParaDataInit(uint8_t* buff)
{
    memcpy(&BmsPara,buff,sizeof(FparaInfoType));
    if(BmsPara.versions!=BmsPara_Cfg.versions)
    {
        memcpy(&BmsPara,&BmsPara_Cfg,sizeof(FparaInfoType));
        return true;
    }
    return false;
}
EN_FPARA_WRITE_TYPE FParaWriteState = FPARA_WRITE_IDLE;

/** 
* @brief  Flash参数初始化
* @param  none
* @return  none
* @note:
*
*/
void FParaAspInit()
{
    uint8_t addr_flag_a = 0;
    uint8_t addr_flag_b = 0;
    uint8_t temp_u8 = 0;
    uint32_t flag_addr = 0;
    uint8_t DataBuff[FPARA_MAX];

    while(FPARA_MAX > FMC_PAGE_SIZE);
    InternalFlashByteRead(FPARA_A_FLAG_ADD,&addr_flag_a,1);
    InternalFlashByteRead(FPARA_B_FLAG_ADD,&addr_flag_b,1);
    
    if(addr_flag_a==0xFF)
    {
        if(addr_flag_b==0xFE)
        {
            flag_addr=FPARA_A_ADDR;
        }
        else
        {
            flag_addr=FPARA_B_ADDR;  //考虑标识写入失败
        }
    }
    else if(addr_flag_b==0xFF)
    {
        if(addr_flag_a == 0xFE)
        {
            flag_addr=FPARA_B_ADDR;
        }
        else
        {
            flag_addr=FPARA_A_ADDR;
        }
    }
    else
    {
        if(addr_flag_b > addr_flag_a)
        {
            flag_addr=FPARA_B_ADDR;
        }
        else//
        {
            flag_addr=FPARA_A_ADDR;
        }
    }    
    InternalFlashByteRead(flag_addr,DataBuff,FPARA_MAX);     //参数读取
    if(FParaDataInit(DataBuff))     //需要重新写入
    {
        if(flag_addr == FPARA_A_ADDR)
        {
            temp_u8=addr_flag_a+1;
//            InternalFlashByteWrite(FPARA_B_ADDR,(uint8_t*)&BmsPara,sizeof(FparaInfoType));
            InternalFlashErasePage(FPARA_B_ADDR);
            InternalFlashByteWriteWithoutErase(FPARA_B_ADDR,(uint8_t*)&BmsPara,sizeof(FparaInfoType));
            InternalFlashByteWriteWithoutErase(FPARA_B_FLAG_ADD,&temp_u8,1);
        }
        else
        {
            temp_u8=addr_flag_b+1;
//            InternalFlashByteWrite(FPARA_A_ADDR,(uint8_t*)&BmsPara,sizeof(FparaInfoType));
            InternalFlashErasePage(FPARA_A_ADDR);
            InternalFlashByteWriteWithoutErase(FPARA_A_ADDR,(uint8_t*)&BmsPara,sizeof(FparaInfoType));
            InternalFlashByteWriteWithoutErase(FPARA_A_FLAG_ADD,&temp_u8,1);
        }
        memcpy(&BmsPara_Flash,&BmsPara,sizeof(FparaInfoType));
    }
    
    InternalFlashByteRead(BOOT_INFO_ADDR,(uint8_t*)&BootInfo,sizeof(BootInfoType));     //参数读取 
    if(BootInfo.DeviceType == 0xff)
    {
        InternalFlashByteWriteWithoutErase(BOOT_INFO_ADDR,(uint8_t*)&BootInfoCfg,sizeof(BootInfoType));
        BootInfo = BootInfoCfg;
    }
}
/** 
* @brief  系统参数FLASH写入
* @param  none
* @return  none
* @note:
*
*/
void FParaWriteFun()
{
    uint8_t addr_flag_a = 0;
    uint8_t addr_flag_b = 0;
    uint8_t temp_u8 = 0;
    uint32_t flag_addr = 0;
    static uint32_t write_time=0;

    if(FParaWriteState == FPARA_WRITE_IDLE)
    {
        return;
    }

    if(tim_get_systime() - write_time < 15000)  //
    {
        return;
    }
    if(memcmp(&BmsPara_Flash,&BmsPara,sizeof(FparaInfoType))==0)
    {
        return;
    }
    InternalFlashByteRead(FPARA_A_FLAG_ADD,&addr_flag_a,1);
    InternalFlashByteRead(FPARA_B_FLAG_ADD,&addr_flag_b,1);
    if(addr_flag_a==0xFF)
    {
        if(addr_flag_b==0xFE)
        {
            flag_addr=FPARA_A_ADDR;
        }
        else
        {
            flag_addr=FPARA_B_ADDR;  //考虑标识写入失败
        }
    }
    else if(addr_flag_b==0xFF)
    {
        if(addr_flag_a==0xFE)
        {
            flag_addr=FPARA_B_ADDR;
        }
        else
        {
            flag_addr=FPARA_A_ADDR;
        }
    }
    else
    {
        if(addr_flag_b > addr_flag_a)
        {
            flag_addr=FPARA_B_ADDR;
        }
        else//
        {
            flag_addr=FPARA_A_ADDR;
        }
    }    
    if(flag_addr == FPARA_A_ADDR)
    {
        temp_u8= addr_flag_a + 1;

        InternalFlashErasePage(FPARA_B_ADDR);
        InternalFlashByteWriteWithoutErase(FPARA_B_ADDR,(uint8_t*)&BmsPara,sizeof(FparaInfoType));
        InternalFlashByteWriteWithoutErase(FPARA_B_FLAG_ADD,&temp_u8,1);
    }
    else
    {
        temp_u8=addr_flag_b +1 ;
//        InternalFlashByteWrite(FPARA_A_ADDR,(uint8_t*)&BmsPara,sizeof(FparaInfoType));
        InternalFlashErasePage(FPARA_A_ADDR);
        InternalFlashByteWriteWithoutErase(FPARA_A_ADDR,(uint8_t*)&BmsPara,sizeof(FparaInfoType));
        InternalFlashByteWriteWithoutErase(FPARA_A_FLAG_ADD,&temp_u8,1);
    }
    memcpy(&BmsPara_Flash,&BmsPara,sizeof(FparaInfoType));
    FParaWriteState = FPARA_WRITE_IDLE;
    write_time = tim_get_systime();
}
EN_FPARA_WRITE_TYPE BootInfoWriteState = FPARA_WRITE_IDLE;
/** 
* @brief  BOOT数据FLASH写入
* @param  none
* @return  none
* @note:
*
*/
static void BootInfoWriteFun()
{
    static uint32_t write_time=0;

    if(BootInfoWriteState == FPARA_WRITE_IDLE)
    {
        return;
    }

    if(tim_get_systime() - write_time < 10000)  //
    {
        return;
    }
    write_time = tim_get_systime();
    InternalFlashWrite(BOOT_INFO_ADDR,(uint32_t*)&BootInfo,sizeof(BootInfoType));
    BootInfoWriteState = FPARA_WRITE_IDLE;
    
}
/** 
* @brief  Flash数据写入主函数
* @param  none
* @return  none
* @note:
*
*/
void FParaWriteMain()
{
    FParaWriteFun();
    BootInfoWriteFun();
}
/** 
* @brief  系统参数Flash写入设置
* @param  type：参数写入方式
* @return  none
* @note:
*
*/
void FParaSaveSet(EN_FPARA_WRITE_TYPE type)
{
    FParaWriteState = type;
}
/** 
* @brief  BOOT数据Flash写入设置
* @param  type：参数写入方式
* @return  none
* @note:
*
*/
void BootInfoSaveSet(EN_FPARA_WRITE_TYPE type)
{
    BootInfoWriteState = type;
}

