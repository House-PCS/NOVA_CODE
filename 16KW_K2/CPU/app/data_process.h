/*
 * data_process.h
 *
 *  Created on: 2025年10月24日
 *      Author: Nova007457
 */
#ifndef DATA_PROCESS_H

#define DATA_PROCESS_H

#include "F28x_Project.h"

#define CT_ADCSIZE (300)
#define CT_currvar_limit (0.20f)

typedef struct {
    float32 *buffer;          // 数据缓冲区
    uint16_t size;          // 窗口大小
    uint16_t index;         // 当前写入索引
    uint16_t count;         // 当前数据数量
    bool is_full;           // 缓冲区是否已满
    
    // 统计变量
    volatile float32 sum;              // 数据和
    volatile float32 sum_squares;      // 数据平方和
    volatile uint32_t recalc_counter;  // periodic full recalculation counter
    volatile float32 variance;         // 当前方差
} sliding_window_t;

// 滤波结构体
typedef struct {
    uint16_t index;                            // 当前索引
    uint16_t is_full;                          // 缓冲区是否已满
    float32 sum;                                // 当前总和
    uint8_t window_size;                        //数据窗口大小，即多少个数据进行平均滤波
    float32* data_buffer;          // 数据缓冲区
} DataFilter;


typedef struct LowPassfilter_1order_stru_t
{
   float32 a0;
   float32 a1;
   float32 b1;

   float32 yn;
   float32 yn_last1;
   float32 xn_last1;

} LowPassfilter_1order_t;


void sliding_window_init(sliding_window_t *window, float *buffer, uint16_t size);
void sliding_window_calculate_stats(sliding_window_t *data);
void sliding_window_add_data(sliding_window_t *data, float new_data);
void sliding_window_reset(sliding_window_t *data);

void data_filter_init(DataFilter* filter , uint8_t filter_window_size);
float32 data_filter_update(DataFilter* filter, float new_data); 

float32 LowPass_1order_butterfilter(LowPassfilter_1order_t* Lowfilter_1order,float32 W0 , float32 T , float32 xn);

extern sliding_window_t CTcurr_detect;

#endif 

