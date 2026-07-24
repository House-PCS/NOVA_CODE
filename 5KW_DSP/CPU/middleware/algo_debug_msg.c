#include "algo_debug_msg.h"

#include <string.h>

#include "algorithm_interface.h"
#include "algo_info.h"
#include "inverter_loop_cntl.h"
#include "llc_dcdc_cntl.h"
#include "mppt_process.h"


uint16_t c1_r_array[256];   // mapped to GS0 of shared RAM owned by CPU02
uint16_t c1_r_w_array[512]; // mapped to GS1 of shared RAM owned by CPU01
#pragma DATA_SECTION(c1_r_array,"CPU2TOCPU1DATA");
#pragma DATA_SECTION(c1_r_w_array,"CPU1TOCPU2DATA");

static float32*** _gWavePtr;

float testf = 0;
#define DEBUG_ADDSIG(type,index,sig) \
    *(const float**)(c1_r_w_array+type*32+index*2+2)=sig;\
    if(type_count[type]<index+1)type_count[type]=index+1;

void DebugWageInit()
{
    uint16_t type_count[8] = { 0 };
    _gWavePtr = (float32***)(c1_r_w_array + 4);
    const float* cache = InverterGetStateWave();
    //*(float**)(c1_r_w_array + 4) = &_gInvInfo->sign_sin;
    //*(float**)(c1_r_w_array + 6) = &testf;
    DEBUG_ADDSIG(0, 0, cache + 0);
    DEBUG_ADDSIG(0, 1, cache + 1);
    DEBUG_ADDSIG(0, 2, cache + 2);
    DEBUG_ADDSIG(0, 3, cache + 3);
    DEBUG_ADDSIG(0, 4, cache + 4);
    DEBUG_ADDSIG(0, 5, cache + 5);
    DEBUG_ADDSIG(0, 6, cache + 6);
    DEBUG_ADDSIG(0, 7, cache + 7);
    DEBUG_ADDSIG(0, 8, cache + 8);
    DEBUG_ADDSIG(0, 9, cache + 9);
    DEBUG_ADDSIG(0, 10, cache + 10);
    DEBUG_ADDSIG(0, 11, cache + 11);
    DEBUG_ADDSIG(0, 12, cache + 12);

    cache=LlcGetStateWave();
    DEBUG_ADDSIG(1, 0, cache + 0);
    DEBUG_ADDSIG(1, 1, cache + 1);
    DEBUG_ADDSIG(1, 2, cache + 2);
    DEBUG_ADDSIG(1, 3, cache + 3);
    DEBUG_ADDSIG(1, 4, cache + 4);
    DEBUG_ADDSIG(1, 5, cache + 5);
    DEBUG_ADDSIG(1, 6, cache + 6);
    DEBUG_ADDSIG(1, 7, cache + 7);

    cache= AlgoGetStateWave();
    DEBUG_ADDSIG(2, 0, cache + 0);
    DEBUG_ADDSIG(2, 1, cache + 1);
    DEBUG_ADDSIG(2, 2, cache + 2);
    DEBUG_ADDSIG(2, 3, cache + 3);
    DEBUG_ADDSIG(2, 4, cache + 4);
    DEBUG_ADDSIG(2, 5, cache + 5);
    DEBUG_ADDSIG(2, 6, cache + 6);
    DEBUG_ADDSIG(2, 7, cache + 7);
    DEBUG_ADDSIG(2, 8, cache + 8);
    DEBUG_ADDSIG(2, 9, cache + 9);
    DEBUG_ADDSIG(2, 10, cache + 10);
    DEBUG_ADDSIG(2, 11, cache + 11);
    DEBUG_ADDSIG(2, 12, cache + 12);

    cache= PvGetStateWave();
    DEBUG_ADDSIG(3, 0, cache + 0);
    DEBUG_ADDSIG(3, 1, cache + 1);
    DEBUG_ADDSIG(3, 2, cache + 2);
    DEBUG_ADDSIG(3, 3, cache + 3);
    DEBUG_ADDSIG(3, 4, cache + 4);
    DEBUG_ADDSIG(3, 5, cache + 5);
    DEBUG_ADDSIG(3, 6, cache + 6);
    DEBUG_ADDSIG(3, 7, cache + 7);
    DEBUG_ADDSIG(3, 8, cache + 8);
    DEBUG_ADDSIG(3, 9, cache + 9);
    DEBUG_ADDSIG(3, 10, cache + 10);
    DEBUG_ADDSIG(3, 11, cache + 11);
    DEBUG_ADDSIG(3, 12, cache + 12);
    DEBUG_ADDSIG(3, 13, cache + 13);
    DEBUG_ADDSIG(3, 14, cache + 14);

    for(int i = 0; i < 8; ++i)
    {
        c1_r_w_array[i * 32] = type_count[i];
    }
}

void DebugTest()
{
    //c1_r_w_array[0]++;
    testf += 0.1;
    if(testf > 1)testf -= 1;

}

void DebugReadMsg()
{
    static uint8_t msg_id = 0;
    if(c1_r_array[0] != 0xBB || c1_r_array[1] != 0xAA)
        return;
    if(c1_r_array[2] == msg_id)
        return;
    msg_id = c1_r_array[2];
    if(c1_r_array[3] == 0xCC)
    {
        void InverterTest(uint8_t data);
        InverterTest(c1_r_array[4]);
    }
}
