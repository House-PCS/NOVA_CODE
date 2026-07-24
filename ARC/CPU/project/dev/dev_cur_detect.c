
#include "dev_cur_detect.h"



uint16_t gArcCurData1[SAMPLE_NUM];
uint16_t gArcCurData2[SAMPLE_NUM * 3];


DevCurDetect_t gDevCurDetect;

static void _DMAAdc1IrqProcess();
static void _DMAAdc2IrqProcess();

static void _StartSample(EN_ARC_CUR_ID id);
static void _StartAllSample();

void ArcHandleClearInit()
{
    memset(gArcCurData1, 0, sizeof(gArcCurData1));
    memset(gArcCurData2, 0, sizeof(gArcCurData2));
}

void DevAdcCurDetectInit()
{
    gDevCurDetect.start_all_sample = _StartAllSample;

    Adc1ArcCurHandle.continuous_num = SAMPLE_NUM;
    Adc1ArcCurHandle.pRData = gArcCurData1;
    Adc1ArcCurHandle.dma_complete_process = _DMAAdc1IrqProcess;
    Adc1ArcCurBspInit();


    Adc2ArcCurHandle.continuous_num = SAMPLE_NUM;
    Adc2ArcCurHandle.pRData = gArcCurData2;
    Adc2ArcCurHandle.dma_complete_process = _DMAAdc2IrqProcess;
    Adc2ArcCurBspInit();
    DetectIdBspInit();
}



static void _DMAAdc1IrqProcess()
{
    AdcStopConv(&Adc1ArcCurHandle);
    /*第二路数据拷贝*/
//    uint16_t gDevCurDetect_cur_data3[1024];

//    memcpy((uint16_t*)gDevCurDetect.arc_cur_data_2, (uint16_t*)Adc1ArcCurHandle.pRData, SAMPLE_NUM * 2);
    memcpy(gDevCurDetect.arc_cur_data_2, Adc1ArcCurHandle.pRData, sizeof(gDevCurDetect.arc_cur_data_2));
    gDevCurDetect.arc_cur_complete_2 = true;
}


static void _DMAAdc2IrqProcess()
{
    uint16_t i = 0, j = 0;
    AdcStopConv(&Adc2ArcCurHandle);
    /*第一、三、四路数据拷贝*/
    for(i = 0; i < SAMPLE_NUM; i++)
    {
        gDevCurDetect.arc_cur_data_4[i] = Adc2ArcCurHandle.pRData[j];
        gDevCurDetect.arc_cur_data_3[i] = Adc2ArcCurHandle.pRData[j + 1];
        gDevCurDetect.arc_cur_data_1[i] = Adc2ArcCurHandle.pRData[j + 2];
        j += 3;
    }
    gDevCurDetect.arc_cur_complete_1 = true;
    gDevCurDetect.arc_cur_complete_3 = true;
    gDevCurDetect.arc_cur_complete_4 = true;
}

static void _StartSample(EN_ARC_CUR_ID id)
{
    switch(id)
    {
        case EN_ARC_CUR_ID_134:
        {
            gDevCurDetect.arc_cur_complete_1 = false;
            gDevCurDetect.arc_cur_complete_3 = false;
            gDevCurDetect.arc_cur_complete_4 = false;
            AdcStartConv(&Adc2ArcCurHandle);
            break;
        }
        case EN_ARC_CUR_ID_2:
        {
            gDevCurDetect.arc_cur_complete_2 = false;
            AdcStartConv(&Adc1ArcCurHandle);
            break;
        }
        default:
            break;
    }
}


static void _StartAllSample()
{
    gDevCurDetect.arc_cur_complete_1 = false;
    gDevCurDetect.arc_cur_complete_2 = false;
    gDevCurDetect.arc_cur_complete_3 = false;
    gDevCurDetect.arc_cur_complete_4 = false;

    AdcStartConv(&Adc2ArcCurHandle);
    AdcStartConv(&Adc1ArcCurHandle);
}




