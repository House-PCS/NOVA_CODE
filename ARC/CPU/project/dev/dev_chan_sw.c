#include "dev_chan_sw.h"
#include "bsp.h"
#include "main.h"
#include "modbus_analysis.h"

EN_CHANNAL channal_switch = EN_CHANNAL1;
void dev_chan_sw(EN_CHANNAL channal_switch)
{
    switch(channal_switch)
    {
        case EN_CHANNAL1:   
        {
//            if(3 == channal_num)
//            {
//                GpioBitReset(&DetectChannelSwitchHandle1);
//                GpioBitReset(&DetectChannelSwitchHandle2);
//            }
//            if(1 == channal_num)
//            {
//                GpioBitReset(&DetectChannelSwitchHandle2);
//            }
            GpioBitReset(&DetectChannelSwitchHandle1);
            GpioBitSet(&DetectChannelSwitchHandle2);
             break;
        }
        case EN_CHANNAL2:
        {
//            if(3 == channal_num)
//            {
//                GpioBitSet(&DetectChannelSwitchHandle1);
//                GpioBitReset(&DetectChannelSwitchHandle2); 
//            }
//            if(1 == channal_num)
//            {
//                GpioBitSet(&DetectChannelSwitchHandle2);
//            }
            GpioBitSet(&DetectChannelSwitchHandle1);
            GpioBitReset(&DetectChannelSwitchHandle2);        
            break;
        }
        case EN_CHANNAL3:
        {
            GpioBitReset(&DetectChannelSwitchHandle1);
            GpioBitSet(&DetectChannelSwitchHandle2);
            break;
        }
        case EN_CHANNAL4:
        {
            GpioBitSet(&DetectChannelSwitchHandle1);
            GpioBitSet(&DetectChannelSwitchHandle2);
            break;
        }
        default :
            break;        
     }
}
int dev_chan_detect()
{
    uint8_t Sw_Flag = 0;
    Sw_Flag = (GpioBitGet(&DetectIdHandle1)<<1)|GpioBitGet(&DetectIdHandle2);
    Modbus_ReadReg.ChannelOfNum = Sw_Flag + 1;
//  Sw_Flag = 3;
    return Modbus_ReadReg.ChannelOfNum;
}