/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        main.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "dev_led.h"
#include "cpu_timers.h"
#include "gpio.h"
#include "interrupt.h"
#include "pwm.h"
#include "sci.h"
#include "spi.h"
#include "system.h"
#include "CH390_Interface.h"
#include "dev_flash.h"

#include "F28x_Project.h"
#include "ethernet_analysis.h"
#include "system.h"
#include "string.h"

#include "cpu_control.h"//TODO:测试用后续删除
#include "dev_flash.h"
#include "protocol_process.h"
#include "firmware_iap.h"

uint32_t count = 0  ;//TODO:测试用后续删除
uint16_t temp[4096] = {0};//TODO:测试用后续删除
uint8  flash_read_en = 0;//TODO:测试用后续删除

//TODO:CMD文件目前没适配boot程序,在线升级稳定后需要修改CMD中APP与BOOT的空间分配,以及注意编译出来的bin文件大小问题
void main(void)
{
    InitSysCtrl();

    DELAY_MS(500);
    DINT;//DINT关闭中断

//    InitFlash();
    IER = 0x0000;
    IFR = 0x0000;

    InitPieCtrl();
    InitPieVectTable();
    CpuIntFunInit();
    EPwm_Init();        //EPWM外设配置
    Gpio_Init();        //GPIO外设配置
    Sci_Init();         //SCI外设配置
    Spi_Init();         //Spi外设配置
//    Adc_Init();         //ADC配置初始化
    CpuTimer_Init();    //CPU定时器外设配置

    DevLedInit();       //Led外设初始化
    FlashInit();        //片外Flash初始化
    enterbootmodejudge();
    ProtocolInit();     //所有协议初始化
//    SpiFlashEraseChip();   //测试用，后续删除

    EINT;//EINT打开中断
    ERTM;

    for(;;)
    {
        ProtocolProcess();
        DevLedProcess();
        UpgradeProcesshandle();
        if (flash_read_en)
        {
            SpiFlashReadBytes(0x00000000 + count * 0x1000, (uint16_t*)&temp, sizeof(temp));
        }
        
    }
}
//
// End of file
//
