#include "cell_process.h"
#include "fwdgt_ctl.h"
#include "bms_protocol.h"
#include "afe_control.h"

void AFEFunInit(void)
{
    static uint8_t sox_init_flag = 0;

    if(AFEFirstSample() == true)
    {
        if(!sox_init_flag)
        {
            sox_init_flag = 1;
            SoxInit(&gPackStatus);
        }

        FaultAnalysisProcess();     //故障处理
        BMSRunStateProcess();       //BMS运行状态处理
        SoxProcess(&gPackStatus);
    }
    ClearFwdgtErrCnt();     //看门狗故障计数清除
}