#include "arc_detect_process.h"
#include "FFT_transform.h"
#include <stdio.h>
#include "main.h"
#include "bsp.h"
#include "dev_cur_detect.h"
#include "dev_modbus_s.h"
#include "math.h"
#include "arm_math.h"
#include "uart.h"
#include "dma.h"
#include "modbus_analysis.h"
#include "dev_chan_sw.h"
#include "arc_detect_threshold.h"
#include "arc_detect_info.h"

Process_handle_t DetectProcessHandle;
gThreshold_t    DetectThreshold;
uint8_t alarm_flag;

static float32_t fft_Output_f32[TEST_LENGTH_SAMPLES];
static float32_t fft_Input_f32[TEST_LENGTH_SAMPLES];
static int16_t remove_dc_data[TEST_LENGTH_SAMPLES];
static EN_ARC_DETECT_PROCESS sta = EN_ARC_DET_SAMPLE;

/**************汉宁窗***************/
uint16_t array_hannning[1024] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9,
    10, 10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 19, 19, 20, 21, 22, 23, 24, 25, 26,
    27, 28, 29, 30, 31, 32, 33, 34, 36, 37, 38, 39, 40, 41, 43, 44, 45, 47, 48, 49, 51,
    52, 53, 55, 56, 58, 59, 61, 62, 64, 65, 67, 68, 70, 71, 73, 75, 76, 78, 80, 81, 83,
    85, 86, 88, 90, 92, 94, 95, 97, 99, 101, 103, 105, 107, 109, 111, 112, 114, 116, 118, 120,
    122, 125, 127, 129, 131, 133, 135, 137, 139, 141, 144, 146, 148, 150, 152, 155,
    157, 159, 162, 164, 166, 168, 171, 173, 176, 178, 180, 183, 185, 188, 190, 192,
    195, 197, 200, 202, 205, 207, 210, 212, 215, 218, 220, 223, 225, 228, 231, 233,
    236, 239, 241, 244, 247, 249, 252, 255, 257, 260, 263, 266, 268, 271, 274, 277,
    279, 282, 285, 288, 291, 294, 296, 299, 302, 305, 308, 311, 314, 317, 320, 322,
    325, 328, 331, 334, 337, 340, 343, 346, 349, 352, 355, 358, 361, 364, 367, 370,
    373, 376, 379, 382, 385, 388, 391, 394, 397, 400, 404, 407, 410, 413, 416, 419,
    422, 425, 428, 431, 434, 438, 441, 444, 447, 450, 453, 456, 459, 463, 466, 469,
    472, 475, 478, 481, 484, 488, 491, 494, 497, 500, 503, 506, 510, 513, 516, 519,
    522, 525, 529, 532, 535, 538, 541, 544, 547, 550, 554, 557, 560, 563, 566, 569,
    572, 576, 579, 582, 585, 588, 591, 594, 597, 600, 603, 607, 610, 613, 616, 619,
    622, 625, 628, 631, 634, 637, 640, 643, 646, 649, 652, 655, 659, 662, 665, 668,
    671, 674, 676, 679, 682, 685, 688, 691, 694, 697, 700, 703, 706, 709, 712, 715,
    718, 720, 723, 726, 729, 732, 735, 737, 740, 743, 746, 749, 751, 754, 757, 760,
    763, 765, 768, 771, 773, 776, 779, 781, 784, 787, 789, 792, 795, 797, 800, 803,
    805, 808, 810, 813, 815, 818, 820, 823, 825, 828, 830, 833, 835, 838, 840, 843,
    845, 847, 850, 852, 854, 857, 859, 861, 864, 866, 868, 870, 873, 875, 877, 879,
    881, 884, 886, 888, 890, 892, 894, 896, 898, 900, 903, 905, 907, 909, 911, 913,
    914, 916, 918, 920, 922, 924, 926, 928, 930, 931, 933, 935, 937, 938, 940, 942,
    944, 945, 947, 949, 950, 952, 953, 955, 957, 958, 960, 961, 963, 964, 966, 967,
    969, 970, 971, 973, 974, 975, 977, 978, 979, 981, 982, 983, 984, 986, 987, 988,
    989, 990, 991, 992, 993, 995, 996, 997, 998, 999, 1000, 1001, 1001, 1002, 1003,
    1004, 1005, 1006, 1007, 1007, 1008, 1009, 1010, 1010, 1011, 1012, 1013, 1013, 1014,
    1014, 1015, 1016, 1016, 1017, 1017, 1018, 1018, 1019, 1019, 1020, 1020, 1020, 1021,
    1021, 1021, 1022, 1022, 1022, 1022, 1023, 1023, 1023, 1023, 1023, 1024, 1024, 1024,
    1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1023, 1023, 1023,
    1023, 1023, 1022, 1022, 1022, 1022, 1021, 1021, 1021, 1020, 1020, 1020, 1019, 1019,
    1018, 1018, 1017, 1017, 1016, 1016, 1015, 1014, 1014, 1013, 1013, 1012, 1011, 1010,
    1010, 1009, 1008, 1007, 1007, 1006, 1005, 1004, 1003, 1002, 1001, 1001, 1000, 999,
    998, 997, 996, 995, 993, 992, 991, 990, 989, 988, 987, 986, 984, 983, 982, 981,
    979, 978, 977, 975, 974, 973, 971, 970, 969, 967, 966, 964, 963, 961, 960, 958,
    957, 955, 953, 952, 950, 949, 947, 945, 944, 942, 940, 938, 937, 935, 933, 931,
    930, 928, 926, 924, 922, 920, 918, 916, 914, 913, 911, 909, 907, 905, 903, 900,
    898, 896, 894, 892, 890, 888, 886, 884, 881, 879, 877, 875, 873, 870, 868, 866,
    864, 861, 859, 857, 854, 852, 850, 847, 845, 843, 840, 838, 835, 833, 830, 828,
    825, 823, 820, 818, 815, 813, 810, 808, 805, 803, 800, 797, 795, 792, 789, 787,
    784, 781, 779, 776, 773, 771, 768, 765, 763, 760, 757, 754, 751, 749, 746, 743,
    740, 737, 735, 732, 729, 726, 723, 720, 718, 715, 712, 709, 706, 703, 700, 697,
    694, 691, 688, 685, 682, 679, 676, 674, 671, 668, 665, 662, 659, 655, 652, 649, 646,
    643, 640, 637, 634, 631, 628, 625, 622, 619, 616, 613, 610, 607, 603, 600, 597, 594, 591,
    588, 585, 582, 579, 576, 572, 569, 566, 563, 560, 557, 554, 550, 547, 544, 541, 538, 535,
    532, 529, 525, 522, 519, 516, 513, 510, 506, 503, 500, 497, 494, 491, 488, 484, 481, 478,
    475, 472, 469, 466, 463, 459, 456, 453, 450, 447, 444, 441, 438, 434, 431, 428, 425, 422,
    419, 416, 413, 410, 407, 404, 400, 397, 394, 391, 388, 385, 382, 379, 376, 373, 370, 367,
    364, 361, 358, 355, 352, 349, 346, 343, 340, 337, 334, 331, 328, 325, 322, 320, 317, 314,
    311, 308, 305, 302, 299, 296, 294, 291, 288, 285, 282, 279, 277, 274, 271, 268, 266, 263,
    260, 257, 255, 252, 249, 247, 244, 241, 239, 236, 233, 231, 228, 225, 223, 220, 218, 215,
    212, 210, 207, 205, 202, 200, 197, 195, 192, 190, 188, 185, 183, 180, 178, 176, 173, 171,
    168, 166, 164, 162, 159, 157, 155, 152, 150, 148, 146, 144, 141, 139, 137, 135, 133, 131,
    129, 127, 125, 122, 120, 118, 116, 114, 112, 111, 109, 107, 105, 103, 101, 99, 97, 95, 94,
    92, 90, 88, 86, 85, 83, 81, 80, 78, 76, 75, 73, 71, 70, 68, 67, 65, 64, 62, 61, 59, 58, 56, 55,
    53, 52, 51, 49, 48, 47, 45, 44, 43, 41, 40, 39, 38, 37, 36, 34, 33, 32, 31, 30, 29, 28, 27, 26,
    25, 24, 23, 22, 21, 20, 19, 19, 18, 17, 16, 15, 15, 14, 13, 12, 12, 11, 10, 10, 9, 9, 8, 8, 7,
    7, 6, 6, 5, 5, 4, 4, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0
};
/***********  FFT傅立叶变换 **************/
void arm_rfft_f32_(float32_t *fft_Input_f32, float32_t *fft_OutputMag_f32)
{
	
    uint8_t ifftFlag = 0;
    uint32_t fftSize = 0;
    arm_rfft_fast_instance_f32 S;
    /* dsp库*/

    arm_rfft_fast_init_f32(&S, TEST_LENGTH_SAMPLES);                 /* 初始化结构体S中的参数 */
    arm_rfft_fast_f32(&S, fft_Input_f32, fft_Output_f32, ifftFlag);    /* 1024点实序列快速FFT */

    /* 为了方便跟函数arm_cfft_f32计算的结果做对比，这里求解了1024组模值，实际函数arm_rfft_fast_f32
       只求解出了512组  */
    arm_cmplx_mag_f32(fft_Output_f32, fft_OutputMag_f32, TEST_LENGTH_SAMPLES);
}

/************ 对采集到的ADC值进行去直流、加窗、FFT操作 **********/
void detect_handle_process(uint16_t *adc_sample, uint16_t *array_hanning_lib, float32_t *fft_OutputMag_f32)
{

//  float32_t array_Add_hanning[TEST_LENGTH_SAMPLES];
	
    uint16_t i = 0, j = 0, average = 0;
    uint32_t sum = 0;
    for(j = 0; j < TEST_LENGTH_SAMPLES; j++)                                   /*  求平均值来作为直流分量  */
    {
        sum += adc_sample[j];
    }
    average = sum >> 10;
    for(i = 0; i < TEST_LENGTH_SAMPLES; i++)
    {
//      remove_dc_data[i] =(int16_t)(adc_sample[i]) - average ;
        remove_dc_data[i] = (int16_t)(adc_sample[i]) - 2048 ;         /* 去直流*/ //或者average设定为2048
        fft_Input_f32[i] = (float) (((int32_t)(remove_dc_data[i]) * array_hanning_lib[i]) >> 10); /* 加窗 */
    }
    arm_rfft_f32_(fft_Input_f32, fft_OutputMag_f32);
}

void All_Channal_handle()
{
    detect_handle_process(gDevCurDetect.arc_cur_data_1, array_hannning, DetectProcessHandle.FFT1_OutputMag_f32);  /*数据处理过程：包括去直流、加窗、FFT变换 */
    detect_handle_process(gDevCurDetect.arc_cur_data_2, array_hannning, DetectProcessHandle.FFT2_OutputMag_f32);
    detect_handle_process(gDevCurDetect.arc_cur_data_3, array_hannning, DetectProcessHandle.FFT3_OutputMag_f32);
    detect_handle_process(gDevCurDetect.arc_cur_data_4, array_hannning, DetectProcessHandle.FFT4_OutputMag_f32);
}

void ArcStateInit(void)
{
    ArcHandleClearInit();
    sta = EN_ARC_DET_SAMPLE;
}

void ArcDectectProcess(void)
{
    static EN_ARC_DETECT_PROCESS sta = EN_ARC_DET_SAMPLE;
    uint8_t Sw_Flag = 0;
    uint8_t value = 0;
    uint32_t timeout = 0xFFFFFFFF;
//  Sw_Flag = dev_chan_sw();
    switch(sta)
    {
        case EN_ARC_DET_SAMPLE:                    /*进入采样*/
        {
            gDevCurDetect.start_all_sample();
            sta = EN_ARC_DET_SAMPLE_WAIT;
            break;
        }
        case EN_ARC_DET_SAMPLE_WAIT:              /*等待ADC转换完成*/
        {
            while(((gDevCurDetect.arc_cur_complete_1 && gDevCurDetect.arc_cur_complete_2 &&  \
                    gDevCurDetect.arc_cur_complete_3 && gDevCurDetect.arc_cur_complete_4) != true) && (timeout-- > 0));
            if(timeout == 0)
            {
                sta = EN_ARC_DET_SAMPLE_ERROR;
            }
            else
            {
                sta = EN_ARC_DET_CALCULATE;
            }
            break;

        }

        case EN_ARC_DET_CALCULATE:               /*采样数据计算处理*/
        {
            All_Channal_handle();
            sta = EN_ARC_DET_JUDGE;
            break;
        }
//        case EN_ARC_DET_THRESHOLD:
//        {
//            CA_CFAR(DetectProcessHandle.FFT4_OutputMag_f32,DetectThreshold.Threshold4);
//            CA_CFAR(DetectProcessHandle.FFT3_OutputMag_f32,DetectThreshold.Threshold3);
//            CA_CFAR(DetectProcessHandle.FFT2_OutputMag_f32,DetectThreshold.Threshold2);
//            CA_CFAR(DetectProcessHandle.FFT1_OutputMag_f32,DetectThreshold.Threshold1);
//            sta = EN_ARC_DET_JUDGE;
//            break;
//        }
        case EN_ARC_DET_JUDGE:
        {
            alarm_flag = gArcRealState();
            sta = EN_ARC_DET_SAMPLE;
            break;
        }
        case EN_ARC_DET_SAMPLE_ERROR:
        {
            sta = EN_ARC_DET_SAMPLE;
            break;
        }
    }
}



