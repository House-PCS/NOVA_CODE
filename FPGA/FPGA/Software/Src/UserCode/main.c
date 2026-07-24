//------------------------------------------------------------------------------
/*Description       :   头文件
*/
//------------------------------------------------------------------------------
#include "Gpio.h"
#include "Timer.h"
#include "Uart.h"
#include "Spi.h"
#include "BmsCache.h"
#include "DdrCache.h"
#include "EthUdp.h"
#include "dev_phy.h"

//------------------------------------------------------------------------------
/*Description       :   Main函数
*/
//------------------------------------------------------------------------------

int main(void)
{
    UartInit()      ;
    printf("Uart Init Done!\n");
    
    GpioInit()      ;
    printf("Gpio Init Done!\n");
    
    DevPhy8201Init();
    printf("Phy Init Done!\n");

    SpiInit();
    printf("Spi Init Done!\n");

    TimerInit()     ;
    printf("Timer Init Done!\n");

    BmsCacheInit()  ;
    printf("BmsCache Init Done!\n");

    DdrCacheInit()  ;
    printf("DdrCache Init Done!\n");

    EthUdpInit()    ;
    printf("EthUdp Init Done!\n");


    if(BmsCacheTableTest())
    {
        printf("BmsCache test ok!\n");
    }
    else
    {
        printf("BmsCache test error!\n");
    }

    if(DdrCacheTest())
    {
        printf("DdrCache test ok!\n");
    }
    else
    {
        printf("DdrCache test error!\n");
    }

    
    if(UartTxTest() == 20)
    {
        printf("UartTxTest is done!\n");
    }

//    if(SpiWrTest())
//    {
//        printf("SpiWrTest is done!\n");
//    }


    while(1)
    {

    };
    
}




