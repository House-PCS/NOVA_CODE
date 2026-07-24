

//#include "fft_transform.h"
//#include "main.h"
//#include "math.h"
//#include "arm_math.h"
//#include <stdio.h>






//static float32_t testOutput_f32[TEST_LENGTH_SAMPLES*2]; 
//static float32_t testOutputMag_f32[TEST_LENGTH_SAMPLES*2]; 
//static float32_t testInput_f32[TEST_LENGTH_SAMPLES*2];
//static float32_t Phase_f32[TEST_LENGTH_SAMPLES*2]; /* ?λ*/


///*
//*********************************************************************************************************
//*    函 数 名: arm_rfft_f32_app
//*    功能说明: 调用函数arm_rfft_fast_f32计算幅频和相频
//*    形    参：无
//*    返 回 值: 无
//*********************************************************************************************************
//*/
//void arm_rfft_f32_app(void)
//{
//    uint16_t i;
//    arm_rfft_fast_instance_f32 S;
//    
//    
//    /* 正变换 */
//    ifftFlag = 0; 
//    
//    /* 初始化结构体S中的参数 */
//     arm_rfft_fast_init_f32(&S, TEST_LENGTH_SAMPLES);
//    
//    for(i=0; i<1024; i++)
//    {
//        /* 波形是由直流分量，50Hz正弦波组成，波形采样率1024，初始相位60° */
//        testInput_f32[i] = 1 + cos(2*3.1415926f*50*i/1024 + 3.1415926f/3);
//    }
//    /* 1024点实序列快速FFT */ 
//    arm_rfft_fast_f32(&S, testInput_f32, testOutput_f32, ifftFlag);
//    
//    /* 为了方便跟函数arm_cfft_f32计算的结果做对比，这里求解了1024组模值，实际函数arm_rfft_fast_f32
//       只求解出了512组  
//    */ 
//     arm_cmplx_mag_f32(testOutput_f32, testOutputMag_f32, TEST_LENGTH_SAMPLES);
//    
//    
////    printf("=========================================\r\n");
//    
//    /* 求相频 */
//    PowerPhaseRadians_f32(testOutput_f32, Phase_f32, TEST_LENGTH_SAMPLES, 0.5f);
//    /* 串口打印求解的幅频和相频 */
////    for(i=0; i<TEST_LENGTH_SAMPLES; i++)
////    {
////        printf("%f, %f\r\n", testOutputMag_f32[i], Phase_f32[i]);
////    }
//}





///*
//*********************************************************************************************************
//*	函 数 名: PowerPhaseRadians_f32
//*	功能说明: 求相位
//*	形    参：_ptr  复位地址，含实部和虚部
//*             _phase 求出相位，单位角度制，范围(-180, 180]
//*             _usFFTPoints  复数个数，每个复数是两个float32_t数值
//*             _uiCmpValue  比较值，需要求出相位的数值
//*	返 回 值: 无
//*********************************************************************************************************
//*/
//void PowerPhaseRadians_f32(float32_t *_ptr, float32_t *_phase, uint16_t _usFFTPoints, float32_t _uiCmpValue)		
//{
//	float32_t lX, lY;
//	uint16_t i;
//	float32_t phase;
//	float32_t mag;
//	
//	
//	for (i=0; i <_usFFTPoints; i++)
//	{
//		lX= _ptr[2*i];  	  /* 实部 */
//		lY= _ptr[2*i + 1];    /* 虚部 */
//		
// 		phase = atan2f(lY, lX);    		  				 /* atan2求解的结果范围是(-pi, pi], 弧度制 */
//		arm_sqrt_f32((float32_t)(lX*lX+ lY*lY), &mag);   /* 求模 */
//		
//		if(_uiCmpValue > mag)
//		{
//			Phase_f32[i] = 0;			
//		}
//		else
//		{
//			Phase_f32[i] = phase* 180.0f/3.1415926f;   /* 将求解的结果由弧度转换为角度 */
//		}
//	}
//}



