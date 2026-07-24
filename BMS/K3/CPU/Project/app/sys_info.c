/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sys_info.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-6-1      <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "sys_info.h"

static uint8_t fw_version[3] = {0, 1, 20};
SysInfo_t gSysInfo;
/*初始化调用*/
void SysInfoInit()
{
	GetFirmwareVersion();
	GetHardwareVersion();    
	GetExtendedVersion();
}

void GetExtendedVersion()
{
    uint8_t ex_version[12]; 
    FlashRead(12, ex_version, 0x7F00 + 11);   
    memcpy(&gSysInfo.ex_version[0], &ex_version, 12);    
}

void GetFirmwareVersion()
{
//    uint8_t fw_version[3] ; 
//    FlashRead(3,fw_version,0x7F00 + 4);   
    gSysInfo.sw_version = (fw_version[0] << 16) | (fw_version[1] << 8) | fw_version[2];	
}

void GetHardwareVersion()
{
    gSysInfo.hw_version = 0x0002;//A2 = 0000 00000 0000 0010
}
