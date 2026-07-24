
#include "dev_adc_test.h"
uint16_t gAdcTestBuf[1024 * 2];

static void _DMAIrqProcess();
static uint8_t _complete_flag = 0;

void DevAdcTestInit()
{
    AdcTestHandle.pRData = gAdcTestBuf;
    AdcTestHandle.dma_complete_process = _DMAIrqProcess;
    AdcTestBspInit();
    
    
}



static void _DMAIrqProcess()
{
    AdcStopConv(&AdcTestHandle);
    _complete_flag = 1;
}



void DevAdcTestProcess()
{
    static uint8_t sta = 0;
    static uint32_t timeout = 0;
    switch(sta)
    {
        case 0:
        {
            _complete_flag = 0;
            AdcStartConv(&AdcTestHandle);
            sta = 1;
            break;
        }
        case 1:
        {
            if(_complete_flag == 1)
            {
                sta = 2;
                timeout = tim_get_systime();
                
            }
            break;
        }
        case 2:
        {
            if((uint32_t)(tim_get_systime() - timeout) > 2000)
            {
                sta = 0;
            }
        }
        default:break;
    }
}


