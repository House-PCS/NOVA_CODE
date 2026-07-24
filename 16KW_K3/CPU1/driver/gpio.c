/*
 * gpio.c
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */
#include "F28x_Project.h"
#include "system.h"
#include "gpio.h"

/**
* @brief: LedGpioToggle
* @param: void                        
* @note: 
*
*/
void LedGpioToggle(void)
{
    GPIO_WritePin(LED_GPIO, GPIO_TOGGLE);
}

/**
* @brief: SetLedGpioState
* @param: void                        
* @note: 
*
*/
void SetLedGpioState(uint8_t value)
{
    if(value)
    {
        GPIO_WritePin(LED_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(LED_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetLedGpioState
* @param: void
* @note:
*
*/
void SetCpuTestGpioState(uint8_t value)
{
    if(value)
    {
        GPIO_WritePin(CPU_TEST_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(CPU_TEST_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetFlashCsGpioState
* @param: void
* @note:
*
*/
void SetFlashCsGpioState(uint8_t value)
{
    if(value)
    {
        GPIO_WritePin(FLASH_CS_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(FLASH_CS_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetEXCRlyState
* @param: void
* @note: 设置继电器启机EXC状态
*/
void SetEXCRlyState(uint16_t value)
{
    if(value == 1)//继电器闭合
    {
        GPIO_WritePin(RY_EXC_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_EXC_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetLoadRlyState
* @param: void
* @note: 设置负载继电器状态
*/
void SetLoadRlyState(uint16_t value)
{
    if(value == 1)//继电器闭合
    {
        GPIO_WritePin(RY_LOAD_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_LOAD_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetInvMainRlyState
* @param: void
* @note: 设置逆变主继电器状态
*/
void SetInvMainRlyState(uint16_t value)
{
    if(value == 1)//继电器闭合
    {
        GPIO_WritePin(RY_INV_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_INV_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetGridRlyState
* @param: void
* @note: 设置市电并网继电器状态
*/
void SetGridRlyState(uint16_t value)
{
    if(value == 1)//继电器闭合
    {
        GPIO_WritePin(RY1_Grid_GPIO, GPIO_SET);
        GPIO_WritePin(RY2_Grid_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY1_Grid_GPIO, GPIO_CLEAR);
        GPIO_WritePin(RY2_Grid_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetGenRlyState
* @param: void
* @note: 设置发电机继电器状态
*/
void SetGenRlyState(uint16_t value)
{
    if(value == 1)//继电器闭合
    {
        GPIO_WritePin(RY_GEN_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_GEN_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetPERlyState
* @param: void
* @note: 设置N线与PE连接继电器
*/
void SetPERlyState(uint16_t value)
{
    if(value == 1)//继电器闭合
    {
        GPIO_WritePin(RY_N_PE_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_N_PE_GPIO, GPIO_CLEAR);
    }
}

/**
* @brief: SetGenStarRlyState
* @param: void
* @note: 设置GEN自检状态，默认开机自检
*/
void SetGenStarRlyState(uint16_t value)
{
    if(value == 1)
    {
        GPIO_WritePin(RY_GEN_START_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RY_GEN_START_GPIO, GPIO_CLEAR);
    }
}
/**
* @brief: SetRcmuChkState
* @param: void
* @note: 设置RCMU自检状态，默认开机自检
*/
void SetRcmuChkState(uint16_t value)
{
    if(value == 1)
    {
        GPIO_WritePin(RCMU_CHK_GPIO, GPIO_SET);
    }
    else
    {
        GPIO_WritePin(RCMU_CHK_GPIO, GPIO_CLEAR);
    }
}

/**
* @brief: ARC_IO_Read
* @param: void
* @note: 设置N线与PE连接继电器
*/
uint16_t ArcIOStateRead(void)
{
    volatile uint16_t gpio_state;
    gpio_state = GPIO_ReadPin(ARC_STATE_GPIO);
    return gpio_state;
}
/**
* @brief: PoBootStateRead
* @param: void
* @note: 功率板BOOT状态读取
*/

uint16_t PoBootStateRead(void)
{
    volatile uint16_t boot_id_state[6];
    boot_id_state[0] = GPIO_ReadPin(BOOT0_STATE_GPIO);
    boot_id_state[1] = GPIO_ReadPin(BOOT1_STATE_GPIO);
    boot_id_state[2] = GPIO_ReadPin(BOOT2_STATE_GPIO);
    boot_id_state[3] = GPIO_ReadPin(BOOT3_STATE_GPIO);
    boot_id_state[4] =  ((boot_id_state[3] & 0X01) << 3) | ((boot_id_state[2] & 0X01) << 2) | ((boot_id_state[1] & 0X01) << 1) | (boot_id_state[0] && 0X01);
    return boot_id_state[4];
}
/**
* @brief: _AdcGpioSwitchInit
* @param: void
* @note:ADC的GPIO切换初始化
*/
static void _AdcGpioSwitchInit(void)
{
    EALLOW;
    /*      ADC   PV的GPIO控制*/
    GpioCtrlRegs.GPBPUD.bit.GPIO34    = 1;//禁止内部上拉
    GpioCtrlRegs.GPBMUX1.bit.GPIO34   = 0;//配置为GPIO
    GpioCtrlRegs.GPBCSEL1.bit.GPIO34  = 0;//分配给CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO34    = 1;//配置为输出引脚
    GpioDataRegs.GPBCLEAR.bit.GPIO34  = 1;//配置为低电平

    GpioCtrlRegs.GPBPUD.bit.GPIO35    = 1;//禁止内部上拉
    GpioCtrlRegs.GPBMUX1.bit.GPIO35   = 0;//配置为GPIO
    GpioCtrlRegs.GPBCSEL1.bit.GPIO35  = 0;//分配给CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO35    = 1;//配置为输出引脚
    GpioDataRegs.GPBCLEAR.bit.GPIO35  = 1;//配置为低电平
    /*      ADC  S0和S1的采样通道GPIO   AC_BUS  */
    GpioCtrlRegs.GPBPUD.bit.GPIO40    = 1;//禁止内部上拉
    GpioCtrlRegs.GPBMUX1.bit.GPIO40   = 0;//配置为GPIO
    GpioCtrlRegs.GPBCSEL2.bit.GPIO40  = 0;//分配给CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO40    = 1;//配置为输出引脚
    GpioDataRegs.GPBCLEAR.bit.GPIO40  = 1;//配置为低电平

    GpioCtrlRegs.GPBPUD.bit.GPIO41    = 1;//禁止内部上拉
    GpioCtrlRegs.GPBMUX1.bit.GPIO41   = 0;//配置为GPIO
    GpioCtrlRegs.GPBCSEL2.bit.GPIO41  = 0;//分配给CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO41    = 1;//配置为输出引脚
    GpioDataRegs.GPBCLEAR.bit.GPIO41  = 1;//配置为低电平
    /*  温度  TempS0和TempS1和TempS2的采样通道GPIO   */
    GpioCtrlRegs.GPAPUD.bit.GPIO25    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX2.bit.GPIO25   = 0;//配置为GPIO
    GpioCtrlRegs.GPACSEL4.bit.GPIO25  = 0;//分配给CPU1
    GpioCtrlRegs.GPADIR.bit.GPIO25    = 1;//配置为输出引脚
    GpioDataRegs.GPACLEAR.bit.GPIO25  = 1;//配置为低电平

    GpioCtrlRegs.GPAPUD.bit.GPIO26    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX2.bit.GPIO26   = 0;//配置为GPIO
    GpioCtrlRegs.GPACSEL4.bit.GPIO26  = 0;//分配给CPU1
    GpioCtrlRegs.GPADIR.bit.GPIO26    = 1;//配置为输出引脚
    GpioDataRegs.GPACLEAR.bit.GPIO26  = 1;//配置为低电平

    GpioCtrlRegs.GPAPUD.bit.GPIO27    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX2.bit.GPIO27   = 0;//配置为GPIO
    GpioCtrlRegs.GPACSEL4.bit.GPIO27  = 0;//分配给CPU1
    GpioCtrlRegs.GPADIR.bit.GPIO27    = 1;//配置为输出引脚
    GpioDataRegs.GPACLEAR.bit.GPIO27  = 1;//配置为低电平
    EDIS;
}
/**
* @brief: _InvGpioInit
* @param: void
* @return:Nothing
* @note:逆变器GPIO配置
*/
static void _InvGpioInit(void)
{
    EALLOW;
    /*      逆变 Q1管 Q3管    */
    GpioCtrlRegs.GPAPUD.bit.GPIO0   = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO0  = 1;//配置为EPWM1A

    GpioCtrlRegs.GPAPUD.bit.GPIO1   = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO1  = 1;//配置为EPWM1B

    /*      逆变 Q2管 Q4管    */
    GpioCtrlRegs.GPAPUD.bit.GPIO2    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO2   = 1;//配置为EPWM2A

    GpioCtrlRegs.GPAPUD.bit.GPIO3    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO3   = 1;//配置为EPWM2B

    /*      逆变 Q5管 Q7管    */
    GpioCtrlRegs.GPAPUD.bit.GPIO4    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO4   = 1;//配置为EPWM3A

    GpioCtrlRegs.GPAPUD.bit.GPIO5    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO5   = 1;//配置为EPWM3B

    /*      逆变 Q6管 Q8管    */
    GpioCtrlRegs.GPAPUD.bit.GPIO6    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO6   = 1;//配置为EPWM4A

    GpioCtrlRegs.GPAPUD.bit.GPIO7    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO7   = 1;//配置为EPWM4B

    /*      逆变 Q9管 Q11管    */
    GpioCtrlRegs.GPAPUD.bit.GPIO8    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO8   = 1;//配置为EPWM5A

    GpioCtrlRegs.GPAPUD.bit.GPIO9    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO9   = 1;//配置为EPWM5B

    /*      逆变 Q10管 Q12管    */
    GpioCtrlRegs.GPAPUD.bit.GPIO10   = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO10  = 1;//配置为EPWM6A

    GpioCtrlRegs.GPAPUD.bit.GPIO11   = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO11  = 1;//配置为EPWM6B

    EDIS;
}
/**
* @brief: _EpwmTZGpioInit
* @param: void
* @return:Nothing
* @note:EPWM的TZ的GPIO配置
*/
static void _EpwmTZGpioInit(void)
{
    EALLOW;
    /*      逆变过流TZ封波    */
    GpioCtrlRegs.GPCPUD.bit.GPIO82 = 1;  //禁止内部上拉
    GpioCtrlRegs.GPCQSEL2.bit.GPIO82 = 3;//与系统时钟不同步
    InputXbarRegs.INPUT1SELECT = 82;     //配置TZ1为GPIO82
    /*      母线过流TZ封波    */
    GpioCtrlRegs.GPCPUD.bit.GPIO83 = 1;  //禁止内部上拉
    GpioCtrlRegs.GPCQSEL2.bit.GPIO83 = 3;//与系统时钟不同步
    InputXbarRegs.INPUT2SELECT = 83;     //配置TZ2为GPIO83
    EDIS;
}
/**
* @brief: _DebugSciGpioInit
* @param: void
* @return:Nothing
* @note:调试的SCID的GPIO初始化配置
*/
static void _DebugSciGpioInit(void)
{
    EALLOW;
    /*      SciD的GPIO配置    */
    GpioCtrlRegs.GPCPUD.bit.GPIO76    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCGMUX1.bit.GPIO76  = 1;//配置为SCID TX引脚
    GpioCtrlRegs.GPCMUX1.bit.GPIO76   = 2;//配置为SCID TX引脚
    GpioCtrlRegs.GPCCSEL2.bit.GPIO76  = 0;//分配给CPU1

    GpioCtrlRegs.GPCPUD.bit.GPIO77    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCGMUX1.bit.GPIO77  = 1;//配置为SCID RX引脚
    GpioCtrlRegs.GPCMUX1.bit.GPIO77   = 2;//配置为SCID RX引脚
    GpioCtrlRegs.GPCCSEL2.bit.GPIO77  = 0;//分配给CPU1
    EDIS;
}
/**
* @brief: _LEDGpioInit
* @param: void
* @return:Nothing
* @note:运行灯GPIO配置
*/
static void _LEDGpioInit(void)
{
    GPIO_SetupPinMux(LED_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(LED_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

}
/**
* @brief: _ArcGpioInit
* @param: void
* @return:Nothing
* @note:拉弧检测GPIO配置
*/
static void _ArcGpioInit(void)
{
    GPIO_SetupPinMux(ARC_STATE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(ARC_STATE_GPIO, GPIO_INPUT, 0);
}
/**
* @brief: _RelayGpioInit
* @param: void
* @return:Nothing
* @note:继电器GPIO配置
*/
static void _RelayGpioInit(void)
{
    EALLOW;
    /*Relay_Total的GPIO配置*/
    GPIO_SetupPinMux(RY_INV_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_INV_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_INV_GPIO, GPIO_CLEAR);
    /*Relay_BackUP的GPIO配置*/
    GPIO_SetupPinMux(RY_LOAD_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_LOAD_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_LOAD_GPIO, GPIO_CLEAR);
    /*Relay_GRID的GPIO配置*/
    GPIO_SetupPinMux(RY1_Grid_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY1_Grid_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinMux(RY2_Grid_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY2_Grid_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY1_Grid_GPIO, GPIO_CLEAR);
    GPIO_WritePin(RY2_Grid_GPIO, GPIO_CLEAR);
    /*Relay_GEN的GPIO配置*/
    GPIO_SetupPinMux(RY_GEN_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_GEN_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_GEN_GPIO, GPIO_CLEAR);
    /*RY_N_PE_GPIO的GPIO配置*/
    GPIO_SetupPinMux(RY_N_PE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_N_PE_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_N_PE_GPIO, GPIO_CLEAR);
    /*RY_EXC_CON的GPIO配置*/
    GPIO_SetupPinMux(RY_EXC_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_EXC_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_EXC_GPIO, GPIO_SET);
    /*Relay_GEN_Start的GPIO配置*/
    GPIO_SetupPinMux(RY_GEN_START_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RY_GEN_START_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RY_GEN_START_GPIO, GPIO_SET);
    EDIS;
}
/**
* @brief: _PvGpioInit
* @param: void
* @return:Nothing
* @note:光伏GPIO配置
*/
static void _PvGpioInit(void)
{
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO14   = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO14  = 1;//配置为EPWM8A

    GpioCtrlRegs.GPAPUD.bit.GPIO15   = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX1.bit.GPIO15  = 1;//配置为EPWM8B

    GpioCtrlRegs.GPAPUD.bit.GPIO16    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX2.bit.GPIO16   = 1;//配置为EPWM9A
    GpioCtrlRegs.GPAGMUX2.bit.GPIO16  = 1;//配置为EPWM9A

    GpioCtrlRegs.GPAPUD.bit.GPIO17    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX2.bit.GPIO17   = 1;//配置为EPWM9B
    GpioCtrlRegs.GPAGMUX2.bit.GPIO17  = 1;//配置为EPWM9B
    EDIS;
}
/**
* @brief: _CH390GpioInit
* @param: void
* @return:Nothing
* @note:CH390的GPIO配置
*/
static void _CH390GpioInit(void)
{
    EALLOW;
    /*      SPIB的GPIO配置    */
    GpioCtrlRegs.GPBPUD.bit.GPIO63 = 1;//使能内部上拉
    GpioCtrlRegs.GPCPUD.bit.GPIO64 = 1;//使能内部上拉
    GpioCtrlRegs.GPCPUD.bit.GPIO65 = 1;//使能内部上拉

    GpioCtrlRegs.GPBQSEL2.bit.GPIO63 = 3;//配置为外设时钟，与系统时钟不同步
    GpioCtrlRegs.GPCQSEL1.bit.GPIO64 = 3;//配置为外设时钟，与系统时钟不同步
    GpioCtrlRegs.GPCQSEL1.bit.GPIO65 = 3;//配置为外设时钟，与系统时钟不同步

    GpioCtrlRegs.GPBGMUX2.bit.GPIO63 = 3; //配置为SIMO
    GpioCtrlRegs.GPCGMUX1.bit.GPIO64 = 3; //配置为SOMI
    GpioCtrlRegs.GPCGMUX1.bit.GPIO65 = 3; //配置为SPI的CLK

    GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 3; //配置为SIMO
    GpioCtrlRegs.GPCMUX1.bit.GPIO64 = 3; //配置为SOMI
    GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 3; //配置为SPI的CLK

    /*      CH390_CS的GPIO66配置    */
    GpioCtrlRegs.GPCPUD.bit.GPIO66    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCMUX1.bit.GPIO66   = 0;//配置为GPIO
    GpioCtrlRegs.GPCCSEL1.bit.GPIO66  = 0;//分配给CPU1
    GpioCtrlRegs.GPCDIR.bit.GPIO66    = 1;//配置为输出引脚
    GpioDataRegs.GPCSET.bit.GPIO66 = 1;

    /*      CH390_RST的GPIO67配置    */
    GpioCtrlRegs.GPCPUD.bit.GPIO67    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCMUX1.bit.GPIO67   = 0;//配置为GPIO
    GpioCtrlRegs.GPCCSEL1.bit.GPIO67  = 0;//分配给CPU1
    GpioCtrlRegs.GPCDIR.bit.GPIO67    = 1;//配置为输出引脚
    GpioDataRegs.GPCSET.bit.GPIO67 = 1;

    /*      CH390_INT的GPIO68配置    */
    GpioCtrlRegs.GPCPUD.bit.GPIO68    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCMUX1.bit.GPIO68   = 0;//配置为GPIO
    GpioCtrlRegs.GPCCSEL1.bit.GPIO68  = 0;//分配给CPU1
    GpioCtrlRegs.GPCDIR.bit.GPIO68    = 0;//配置为输入引脚
    EDIS;
}

/**
* @brief: _FlashGpioInit
* @param: void
* @return:Nothing
* @note:Flash的GPIO配置
*/
static void _FlashGpioInit(void)
{
    EALLOW;
    /*      SPIA的GPIO配置    */
    GpioCtrlRegs.GPBPUD.bit.GPIO58 = 1;//使能内部上拉
    GpioCtrlRegs.GPBPUD.bit.GPIO59 = 1;//使能内部上拉
    GpioCtrlRegs.GPBPUD.bit.GPIO60 = 1;//使能内部上拉

    GpioCtrlRegs.GPBQSEL2.bit.GPIO58 = 3;//配置为外设时钟，与系统时钟不同步
    GpioCtrlRegs.GPBQSEL2.bit.GPIO59 = 3;//配置为外设时钟，与系统时钟不同步
    GpioCtrlRegs.GPBQSEL2.bit.GPIO60 = 3;//配置为外设时钟，与系统时钟不同步

    GpioCtrlRegs.GPBGMUX2.bit.GPIO58 = 3; //配置为SIMO
    GpioCtrlRegs.GPBGMUX2.bit.GPIO59 = 3; //配置为SOMI
    GpioCtrlRegs.GPBGMUX2.bit.GPIO60 = 3; //配置为SPI的CLK

    GpioCtrlRegs.GPBMUX2.bit.GPIO58 = 3; //配置为SIMO
    GpioCtrlRegs.GPBMUX2.bit.GPIO59 = 3; //配置为SOMI
    GpioCtrlRegs.GPBMUX2.bit.GPIO60 = 3; //配置为SPI的CLK

    /*      Flash_CS的GPIO61配置    */
    GpioCtrlRegs.GPBPUD.bit.GPIO61    = 1;//禁止内部上拉
    GpioCtrlRegs.GPBMUX2.bit.GPIO61   = 0;//配置为GPIO
    GpioCtrlRegs.GPBCSEL4.bit.GPIO61  = 0;//分配给CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO61    = 1;//配置为输出引脚
    GpioDataRegs.GPBSET.bit.GPIO61 = 1;
    EDIS;
}

/**
* @brief: _HC245GpioInit
* @param: void
* @return:Nothing
* @note:HC245的GPIO初始化配置
*/
static void _HC245GpioInit(void)
{
    EALLOW;
    /*      HC245的ADDC  GPIO45配置    */
    GpioCtrlRegs.GPBPUD.bit.GPIO45    = 1;//禁止内部上拉
    GpioCtrlRegs.GPBMUX1.bit.GPIO45   = 0;//配置为GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO45    = 1;//配置为输出引脚
    GpioDataRegs.GPBSET.bit.GPIO45    = 0;//置低
    GpioCtrlRegs.GPBCSEL2.bit.GPIO45  = 1;//分配给CLA
    /*      HC245的PV_FAN  GPIO46配置    */
    GpioCtrlRegs.GPBPUD.bit.GPIO46    = 1;//禁止内部上拉
    GpioCtrlRegs.GPBMUX1.bit.GPIO46   = 0;//配置为GPIO
    GpioCtrlRegs.GPBCSEL2.bit.GPIO46  = 0;//分配给CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO46    = 1;//配置为输出引脚
    GpioDataRegs.GPBSET.bit.GPIO46    = 1;//置高
    /*      HC245的Relay  GPIO47配置    */
    GpioCtrlRegs.GPBPUD.bit.GPIO47    = 1;//禁止内部上拉
    GpioCtrlRegs.GPBMUX1.bit.GPIO47   = 0;//配置为GPIO
    GpioCtrlRegs.GPBCSEL2.bit.GPIO47  = 0;//分配给CPU1
    GpioCtrlRegs.GPBDIR.bit.GPIO47    = 1;//配置为输出引脚
    GpioDataRegs.GPBSET.bit.GPIO47    = 1;//置高
    EDIS;
}
/**
* @brief: _PwBootGpioInit
* @param: void
* @return:Nothing
* @note:功率板BOOT_IO初始化配置
*/
static void _PwBootGpioInit(void)
{
    EALLOW;
    /*      BOOT_ID初始化配置    */
    GPIO_SetupPinMux(BOOT0_STATE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BOOT0_STATE_GPIO, GPIO_INPUT, 0);

    GPIO_SetupPinMux(BOOT1_STATE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BOOT1_STATE_GPIO, GPIO_INPUT, 0);

    GPIO_SetupPinMux(BOOT2_STATE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BOOT2_STATE_GPIO, GPIO_INPUT, 0);

    GPIO_SetupPinMux(BOOT3_STATE_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BOOT3_STATE_GPIO, GPIO_INPUT, 0);
    EDIS;
}
/**
* @brief: _TestGpio_Init
* @param: void
* @return:Nothing
* @note:测试GPIO初始化配置
*/
static void _TestGpio_Init(void)
{
    EALLOW;
    /*      软件调试预留GPIO 可用来辅助调试      */
//    GPIO_SetupPinMux(CPU_TEST_GPIO, GPIO_MUX_CPU1, 0);
//    GPIO_SetupPinOptions(CPU_TEST_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

    GPIO_SetupPinMux(CLA_TEST_GPIO, GPIO_MUX_CPU1CLA, 0);
    GPIO_SetupPinOptions(CLA_TEST_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

    EDIS;
}
/**
* @brief: _RcmuChkGpioInit
* @param: void
* @return:Nothing
* @note:RCMU自检配置
*/
static void _RcmuChkGpioInit(void)
{
    EALLOW;

    GPIO_SetupPinMux(RCMU_CHK_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RCMU_CHK_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    EDIS;
}

/**
* @brief: _FanGpioInit
* @param: void
* @return:Nothing
* @note:风扇GPIO配置
*/
static void _FanGpioInit(void)
{
    EALLOW;

    /*风扇输出GPIO配置*/
    /*      EPWM10    */
    GPIO_SetupPinMux(FAN1_OUT_GPIO, GPIO_MUX_CPU1, 5); // 假设外设选项1对应EPWM10A
    GPIO_SetupPinOptions(FAN1_OUT_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

//    GPIO_SetupPinMux(FAN2_OUT_GPIO, GPIO_MUX_CPU1, 5); // 假设外设选项1对应EPWM10A
//    GPIO_SetupPinOptions(FAN2_OUT_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
//
//    /*      EPWM11    */
//    GPIO_SetupPinMux(FAN3_OUT_GPIO, GPIO_MUX_CPU1, 5); // 假设外设选项1对应EPWM10A
//    GPIO_SetupPinOptions(FAN3_OUT_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
//
//    GPIO_SetupPinMux(FAN4_OUT_GPIO, GPIO_MUX_CPU1, 5); // 假设外设选项1对应EPWM10A
//    GPIO_SetupPinOptions(FAN4_OUT_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

    /*风扇故障检测GPIO设置*/
    GPIO_SetupPinMux(FAN1_CAP_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(FAN1_CAP_GPIO, GPIO_INPUT, 0);

//    GPIO_SetupPinMux(FAN2_CAP_GPIO, GPIO_MUX_CPU1, 0);
//    GPIO_SetupPinOptions(FAN2_CAP_GPIO, GPIO_INPUT, 0);
//
//    GPIO_SetupPinMux(FAN3_CAP_GPIO, GPIO_MUX_CPU1, 0);
//    GPIO_SetupPinOptions(FAN3_CAP_GPIO, GPIO_INPUT, 0);
//
//    GPIO_SetupPinMux(FAN4_CAP_GPIO, GPIO_MUX_CPU1, 0);
//    GPIO_SetupPinOptions(FAN4_CAP_GPIO, GPIO_INPUT, 0);
    EDIS;
}
/**
* @brief: Gpio_Init
* @param: void
* @return:Nothing
* @note:GPIO配置总函数(包括所有外设)
*/
void Gpio_Init(void)
{
    EALLOW;
    _InvGpioInit();         //逆变GPIO初始化
    _DebugSciGpioInit();    //调试串口GPIO初始化
    _LEDGpioInit();         //指示灯GPIO配置
    _RelayGpioInit();       //继电器GPIO配置
    _HC245GpioInit();      //HC245的GPIO配置 EPWM发波和继电器的驱动
    _PvGpioInit();          //光伏GPIO配置
    _CH390GpioInit();      //CH390的GPIO配置
    _AdcGpioSwitchInit();    //ADC采样切换GPIO配置
    _EpwmTZGpioInit();    //EPWM的TZ保护封波配置
    _FlashGpioInit();      //Flash的Gpio配置
    _TestGpio_Init();        //测试的GPIO配置
    _ArcGpioInit();   /*拉弧检测故障IO配置*/
    _RcmuChkGpioInit(); /*RCMU自检GPIO配置*/
    _PwBootGpioInit();
    _FanGpioInit();
    EDIS;
}
