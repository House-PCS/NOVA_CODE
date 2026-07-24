/*
 * data_process.c
 *
 *  Created on: 2025年10月24日
 *      Author: Nova007457
 */


#include "data_process.h"
#include "adc.h"
#include "stdlib.h"


/**
* @brief: sliding_window_init
* @param: void
* @return:Nothing
* @note:方差计算滑动窗口初始化
*/
void sliding_window_init(sliding_window_t *window, float *buffer, uint16_t size) {
    window->buffer = buffer;
    window->size = size;
    window->index = 0;
    window->count = 0;
    window->is_full = false;
    window->sum = 0.0f;
    window->sum_squares = 0.0f;
    window->recalc_counter = 0;
    window->variance = 0.0f;
    
    // 初始化缓冲区为0
    for(uint16_t i = 0; i < size; i++) {
        window->buffer[i] = 0.0f;
    }
}


/**
* @brief: sliding_window_calculate_stats
* @param: void
* @return:Nothing
* @note:计算统计量（方差）
*/

void sliding_window_calculate_stats(sliding_window_t *data) {
    if(data->count < 2) {
        // 至少需要2个数据点才能计算方差
        data->variance = 0.0f;
        return;
    }
        
    // 计算方差: Var = (sum(x²) - (sum(x))²/n) / (n-1)
    float n = (float)data->count;
    data->variance = (data->sum_squares - (data->sum * data->sum) / n) / (n - 1);
    
    // 避免浮点误差导致的负方差
    if(data->variance < 0.0f) {
        data->variance = 0.0f;
    }
    
}


/**
* @brief: sliding_window_recalc_stats
* @param: void
* @return:Nothing
* @note: from buffer full recalc sum and sum_squares, prevent floating-point error accumulation
*/
static void sliding_window_recalc_stats(sliding_window_t *data) {
    uint16_t n = data->is_full ? data->size : data->count;
    float32 new_sum = 0.0f;
    float32 new_sum_squares = 0.0f;
    
    for (uint16_t i = 0; i < n; i++) {
        float32 val = data->buffer[i];
        new_sum += val;
        new_sum_squares += val * val;
    }
    data->sum = new_sum;
    data->sum_squares = new_sum_squares;
    data->recalc_counter = 0;
}

/**
* @brief: sliding_window_add_data
* @param: void
* @return:Nothing
* @note:方差计算滑动窗口数据更新
*/

void sliding_window_add_data(sliding_window_t *data, float32 new_data) {
    // 如果窗口已满，先移除最老的数据
    if(data->is_full) {
        float old_data = data->buffer[data->index];
        data->sum -= old_data;
        data->sum_squares -= old_data * old_data;
    }
    
    // 添加新数据
    data->buffer[data->index] = new_data;
    data->sum += new_data;
    data->sum_squares += new_data * new_data;
    
    // 更新索引和计数
    data->index = (data->index + 1) % data->size;   
    
    if(!data->is_full) {
        data->count++;
        if(data->count >= data->size) {
            data->is_full = true;
            data->count = data->size;
        }
    }
    // 计算统计量
    sliding_window_calculate_stats(data);

    if(data->is_full) {
        data->recalc_counter++;
    }

    // sum_squares < 0 or periodic full recalculation from buffer
    if (data->sum_squares < 0.0f || data->recalc_counter >= (data->size *10000)) {
        sliding_window_reset(data);
        // sliding_window_calculate_stats(data);
    }
}


// 重置滑动窗口
void sliding_window_reset(sliding_window_t *data) {
    data->index = 0;
    data->count = 0;
    data->is_full = false;
    data->sum = 0.0f;
    data->sum_squares = 0.0f;
    data->variance = 0.0f;
    data->recalc_counter = 0;
    
    // 清空缓冲区
    for(uint16_t i = 0; i < data->size; i++) {
        data->buffer[i] = 0.0f;
    }
}


/*********************************平均滤波函数******************************/
/**
* @brief: data_filter_init
* @param: static inline void
* @return:Nothing
* @note:初始化数据滤波器
*/ 
void data_filter_init(DataFilter* filter , uint8_t filter_window_size) 
{

    filter->data_buffer = (float32*)malloc(filter_window_size * sizeof(float32));
    
    if (filter->data_buffer == NULL) {
        // 处理内存分配失败的情况
        return;
    }
    filter->index = 0;
    filter->is_full = 0;
    filter->sum = 0.0f;
    filter->window_size = filter_window_size;
}

/**
* @brief: data_filter_update
* @param: static inline void
* @return:Nothing
* @note:添加新数据值并返回滤波后的值
*/ 
float32 data_filter_update(DataFilter* filter, float new_data) 
{
    // 减去即将被替换的值
    if(filter->is_full) {
        filter->sum -= filter->data_buffer[filter->index];
    }
    
    // 添加新值
    filter->sum += new_data;
    filter->data_buffer[filter->index] = new_data;
    
    // 更新索引
    filter->index++;
    if(filter->index >= filter->window_size) {
        filter->index = 0;
        filter->is_full = 1;
    }
    
    // 计算平均值
    uint16_t  count = filter->is_full ? filter->window_size : filter->index;
    return filter->sum / count;
}



/**
* @brief: LowPass_1order_butterfilter
* @param: void
* @return:Nothing
* @note:1阶巴特沃斯滤波器
*/
float32 LowPass_1order_butterfilter(LowPassfilter_1order_t* Lowfilter_1order,float32 W0 , float32 T , float32 xn)//yn=a0*x(n)+a1*x(n-1)-b1*y(n-1)
{
    Lowfilter_1order->a0 = W0*T/(W0*T+2);
    Lowfilter_1order->a1 = Lowfilter_1order->a0;
    Lowfilter_1order->b1 = (W0*T-2)/(W0*T+2);

    Lowfilter_1order->yn = xn*Lowfilter_1order->a0 + Lowfilter_1order->a1*Lowfilter_1order->xn_last1 - Lowfilter_1order->b1 * Lowfilter_1order->yn_last1;

    Lowfilter_1order->xn_last1 = xn;
    Lowfilter_1order->yn_last1 = Lowfilter_1order->yn;

    return Lowfilter_1order->yn;
}


