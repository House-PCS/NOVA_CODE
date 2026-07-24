/*
 * gpio.c
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */

#include "F28x_Project.h"
#include "gpio.h"

#define LED_GPIO    99
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
* @brief: LedGpioSet
* @param: void                        
* @note: 
*
*/
void LedGpioSet(void)
{
    GPIO_WritePin(LED_GPIO, GPIO_SET);
}

/**
* @brief: LedGpioReset
* @param: void                        
* @note: 
*
*/
void LedGpioReset(void)
{
    GPIO_WritePin(LED_GPIO, GPIO_CLEAR);
}

/**
* @brief: TestCpuGpioToggle
* @param: void
* @note:
*
*/
void TestCpuGpioToggle(void)
{
    GpioDataRegs.GPCTOGGLE.bit.GPIO93 = 1;
}
/**
* @brief: TestCpuGpioSet
* @param: void
* @note:
*
*/
void TestCpuGpioSet(void)
{
    GpioDataRegs.GPCSET.bit.GPIO93 = 1;
}

/**
* @brief: TestCpuGpioReset
* @param: void
* @note:
*
*/
void TestCpuGpioReset(void)
{
    GpioDataRegs.GPCCLEAR.bit.GPIO93 = 1;
}

/**
* @brief: FlashCsGpioHigh
* @param: void
* @note: Flash的GPIO拉高
*/
void FlashCsGpioHigh(void)
{
    GpioDataRegs.GPBSET.bit.GPIO61 = 1;
}
/**
* @brief: FlashCsGpioLow
* @param: void
* @note: Flash的GPIO拉低
*/
void FlashCsGpioLow(void)
{
    GpioDataRegs.GPBCLEAR.bit.GPIO61 = 1;
}
/**
* @brief: AdcGpioSwitchInit
* @param: void
* @note:ADC的GPIO切换初始化
*/
void AdcGpioSwitchInit(void)
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
* @brief: _InvGpio_Init
* @param: void
* @return:Nothing
* @note:逆变器GPIO配置
*/
void InvGpio_Init(void)
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
* @brief: Epwm_TZ_Gpio_Init
* @param: void
* @return:Nothing
* @note:EPWM的TZ的GPIO配置
*/
void Epwm_TZ_Gpio_Init(void)
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
* @brief: DebugSciGpio_Init
* @param: void
* @return:Nothing
* @note:调试的SCID的GPIO初始化配置
*/
void DebugSciGpio_Init(void)
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
* @brief: LEDGpio_Init
* @param: void
* @return:Nothing
* @note:运行灯GPIO配置
*/
void LEDGpio_Init(void)
{
    GPIO_SetupPinMux(LED_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(LED_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

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
* @brief: RelayGpio_Init
* @param: void
* @return:Nothing
* @note:继电器GPIO配置
*/
void RelayGpio_Init(void)
{
    EALLOW;
    /*Relay_Total的GPIO配置*/
    GpioCtrlRegs.GPCPUD.bit.GPIO87    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCMUX2.bit.GPIO87   = 0;//配置为GPIO
    GpioCtrlRegs.GPCCSEL3.bit.GPIO87  = 0;//分配给CPU
    GpioCtrlRegs.GPCDIR.bit.GPIO87    = 1;//配置为输出引脚
    /*Relay_BackUP的GPIO配置*/
    GpioCtrlRegs.GPCPUD.bit.GPIO88    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCMUX2.bit.GPIO88   = 0;//配置为GPIO
    GpioCtrlRegs.GPCCSEL4.bit.GPIO88  = 0;//分配给CPU1
    GpioCtrlRegs.GPCDIR.bit.GPIO88    = 1;//配置为输出引脚
    /*Relay_GRID的GPIO配置*/
    GpioCtrlRegs.GPCPUD.bit.GPIO89    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCMUX2.bit.GPIO89   = 0;//配置为GPIO
    GpioCtrlRegs.GPCCSEL4.bit.GPIO89  = 0;//分配给CPU1
    GpioCtrlRegs.GPCDIR.bit.GPIO89    = 1;//配置为输出引脚
    /*Relay_GEN的GPIO配置*/
    GpioCtrlRegs.GPCPUD.bit.GPIO90    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCMUX2.bit.GPIO90   = 0;//配置为GPIO
    GpioCtrlRegs.GPCCSEL4.bit.GPIO90  = 0;//分配给CPU1
    GpioCtrlRegs.GPCDIR.bit.GPIO90    = 1;//配置为输出引脚
    /*RY_EXC_CON的GPIO配置*/
    GpioCtrlRegs.GPAPUD.bit.GPIO29    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX2.bit.GPIO29   = 0;//配置为GPIO
    GpioCtrlRegs.GPACSEL4.bit.GPIO29  = 0;//分配给CPU1
    GpioCtrlRegs.GPADIR.bit.GPIO29    = 1;//配置为输出引脚
    GpioDataRegs.GPASET.bit.GPIO29    = 1;//配置为输出高
    /*RY_GEN_START的GPIO配置*/
    GpioCtrlRegs.GPAPUD.bit.GPIO30    = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX2.bit.GPIO30   = 0;//配置为GPIO
    GpioCtrlRegs.GPACSEL4.bit.GPIO30  = 0;//分配给CPU1
    GpioCtrlRegs.GPADIR.bit.GPIO30    = 1;//配置为输出引脚
    GpioDataRegs.GPACLEAR.bit.GPIO30  = 1;//配置为输出低
    EDIS;
}
/**
* @brief: PvGpio_Init
* @param: void
* @return:Nothing
* @note:光伏GPIO配置
*/
void PvGpio_Init(void)
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
* @brief: CH390_Gpio_Init
* @param: void
* @return:Nothing
* @note:CH390的GPIO配置
*/
void CH390_Gpio_Init(void)
{
    EALLOW;
    /*      SPIB的GPIO配置    */
    GpioCtrlRegs.GPBPUD.bit.GPIO63 = 1;//使能内部上拉
    GpioCtrlRegs.GPCPUD.bit.GPIO64 = 1;//使能内部上拉
    GpioCtrlRegs.GPCPUD.bit.GPIO65 = 1;//使能内部上拉
//    GpioCtrlRegs.GPCPUD.bit.GPIO66 = 1;//使能内部上拉

    GpioCtrlRegs.GPBQSEL2.bit.GPIO63 = 3;//配置为外设时钟，与系统时钟不同步
    GpioCtrlRegs.GPCQSEL1.bit.GPIO64 = 3;//配置为外设时钟，与系统时钟不同步
    GpioCtrlRegs.GPCQSEL1.bit.GPIO65 = 3;//配置为外设时钟，与系统时钟不同步
//    GpioCtrlRegs.GPCQSEL1.bit.GPIO66 = 3;//配置为外设时钟，与系统时钟不同步

    GpioCtrlRegs.GPBGMUX2.bit.GPIO63 = 3; //配置为SIMO
    GpioCtrlRegs.GPCGMUX1.bit.GPIO64 = 3; //配置为SOMI
    GpioCtrlRegs.GPCGMUX1.bit.GPIO65 = 3; //配置为SPI的CLK
//    GpioCtrlRegs.GPCGMUX1.bit.GPIO66 = 3; //配置为SPI的片选

    GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 3; //配置为SIMO
    GpioCtrlRegs.GPCMUX1.bit.GPIO64 = 3; //配置为SOMI
    GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 3; //配置为SPI的CLK
//    GpioCtrlRegs.GPCMUX1.bit.GPIO66 = 3; //配置为SPI的片选

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
* @brief: Flash_Gpio_Init
* @param: void
* @return:Nothing
* @note:Flash的GPIO配置
*/
void Flash_Gpio_Init(void)
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
* @brief: HC245_Gpio_Init
* @param: void
* @return:Nothing
* @note:HC245的GPIO初始化配置
*/
void HC245_Gpio_Init(void)
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
* @brief: TestGpio_Init
* @param: void
* @return:Nothing
* @note:测试GPIO初始化配置
*/
void TestGpio_Init(void)
{
    EALLOW;
    /*      软件调试预留GPIO 可用来辅助调试      */
    GpioCtrlRegs.GPCPUD.bit.GPIO93    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCMUX2.bit.GPIO93   = 0;//配置为GPIO
    GpioCtrlRegs.GPCCSEL4.bit.GPIO93  = 0;//分配给CPU1
    GpioCtrlRegs.GPCDIR.bit.GPIO93    = 1;//配置为输出引脚

    GpioCtrlRegs.GPCPUD.bit.GPIO94    = 1;//禁止内部上拉
    GpioCtrlRegs.GPCMUX2.bit.GPIO94   = 0;//配置为GPIO
    GpioCtrlRegs.GPCCSEL4.bit.GPIO94  = 1;//分配给CLA1
    GpioCtrlRegs.GPCDIR.bit.GPIO94    = 1;//配置为输出引脚

    EDIS;
}

/**
* @brief: _FANGpio_Init
* @param: void
* @return:Nothing
* @note:FANGPIO配置
*/
void FANGpio_Init(void)
{
    EALLOW;
    /*     fan PWM    */

    GpioCtrlRegs.GPAPUD.bit.GPIO18   = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX2.bit.GPIO18  = 1;//配置为EPWM10A
    GpioCtrlRegs.GPAGMUX2.bit.GPIO18 = 1;

    GpioCtrlRegs.GPAPUD.bit.GPIO19   = 1;//禁止内部上拉
    GpioCtrlRegs.GPAMUX2.bit.GPIO19  = 1;//配置为EPWM10B
    GpioCtrlRegs.GPAGMUX2.bit.GPIO19 = 1;

    EDIS;
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
    InvGpio_Init();         //逆变GPIO初始化
    DebugSciGpio_Init();    //调试串口GPIO初始化
    LEDGpio_Init();         //指示灯GPIO配置
    RelayGpio_Init();       //继电器GPIO配置
    HC245_Gpio_Init();      //HC245的GPIO配置 EPWM发波和继电器的驱动
    PvGpio_Init();          //光伏GPIO配置
    CH390_Gpio_Init();      //CH390的GPIO配置
    AdcGpioSwitchInit();    //ADC采样切换GPIO配置
    Epwm_TZ_Gpio_Init();    //EPWM的TZ保护封波配置
    Flash_Gpio_Init();      //Flash的Gpio配置
    TestGpio_Init();        //测试的GPIO配置
    _FanGpioInit();
    _RcmuChkGpioInit();
    EDIS;
}
