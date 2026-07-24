#ifndef __ARC_DETECT_H_
#define __ARC_DETECT_H_

#include "adc.h"
#include "arm_math.h"
#include "dev_cur_detect.h"
#include "math.h"

#define TEST_LENGTH_SAMPLES 1024 
#define LENGTH_THRESHOLD 512
//#define FFT_CONVERT_LENGTH 512    只有f=fs/2范围内的信号才是被采样到的有效信号。


extern uint8_t alarm_flag;
typedef struct process_hanlde_stru_t
{
    float32_t FFT1_OutputMag_f32[TEST_LENGTH_SAMPLES];
	float32_t FFT2_OutputMag_f32[TEST_LENGTH_SAMPLES];
	float32_t FFT3_OutputMag_f32[TEST_LENGTH_SAMPLES];
	float32_t FFT4_OutputMag_f32[TEST_LENGTH_SAMPLES];
}Process_handle_t;

typedef struct gthreshold_stru_t
{
    float32_t Threshold1[LENGTH_THRESHOLD];
	float32_t Threshold2[LENGTH_THRESHOLD];
	float32_t Threshold3[LENGTH_THRESHOLD];
	float32_t Threshold4[LENGTH_THRESHOLD];
    
}gThreshold_t;


typedef enum
{
    EN_ARC_DET_SAMPLE,
	EN_ARC_DET_SAMPLE_WAIT,
    EN_ARC_DET_CALCULATE,
//	EN_ARC_DET_THRESHOLD,
    EN_ARC_DET_JUDGE,
    EN_ARC_DET_SAMPLE_ERROR
}EN_ARC_DETECT_PROCESS;
typedef enum
{
    EN_SW_CHANNAL1,
	EN_SW_CHANNAL12,
    EN_SW_CHANNAL123,
	EN_SW_CHANNAL1234
}EN_SW_CHANNAL;


//extern DevCurDetect_t gDevCurDetect;
//extern ModbusReadReg_t Modbus_ReadReg;
//extern ModbusReg_t Modbus_writeReg;
extern Process_handle_t DetectProcessHandle;
extern gThreshold_t	DetectThreshold;

void detect_handle_process(uint16_t *adc_sample, uint16_t *array_hanning_lib,float32_t * fft_Input_f32);
void arm_rfft_f32_(float32_t *fft_Input_f32,float32_t *fft_OutputMag_f32);
//int Max_valuehistory(uint16_t value,uint16_t* addr);
void All_Channal_handle(void);
void ArcDectectProcess(void);
void ArcStateInit(void);

#endif