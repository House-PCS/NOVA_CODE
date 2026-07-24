
#include "main.h"

int main(void)
{
    SystickInit();
    DevLedInit();
    Uart0Init();
    MidComserviceInit();
    SlaveCommInit();
    FpgaComInit();
    SpiInit();
    DevPhy8201Init();
    BmsRegInit();
    DCDCRegInit();
    while(1)
    {
        ut_test_process();
        DevLedProcess();
        MidComserviceHandle();
        SlaveCommProcess();
        UpdataStatusAnalysis();	
        FPGARecordPorcess();
    }
}
