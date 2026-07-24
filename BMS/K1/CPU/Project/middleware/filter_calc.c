/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        filter_calc.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "filter_calc.h"



/** 
* @brief    函数用于比较两个整数
* @param    要比较的两个数的指针
* @return   none
* @note: 
*/
static int _int_cmp(const void *a, const void *b)  
{
    return (*(uint16_t*)a - *(uint16_t*)b);
}


/** 
* @brief    冒泡排序-升序
* @param    p_data:数据起始指针，count:数组元素个数，num:待排序数组单元素大小
* @return   none
* @note: 
*/
static void _bubbleAscending(void *p_data, uint16_t count, uint16_t num)
{
    int8_t *p_start_addr_8 = (int8_t *)p_data;
    int16_t *p_start_addr_16 = (int16_t *)p_data;
    int32_t *p_start_addr_32 = (int32_t *)p_data;
    
    for(uint16_t i = 0; i < count - 1; i++)
    {
        for(uint16_t j = 0; j < count - 1 - i; j++)
        {
            if(num == 1)
            {
                int8_t temp;
                if(p_start_addr_8[j] > p_start_addr_8[j + 1])
                {
                    temp = p_start_addr_8[j + 1];
                    p_start_addr_8[j + 1] = p_start_addr_8[j];
                    p_start_addr_8[j] = temp;
                }
            }
            else if(num == 2)
            {
                int16_t temp;
                if(p_start_addr_16[j] > p_start_addr_16[j + 1])
                {
                    temp = p_start_addr_16[j + 1];
                    p_start_addr_16[j + 1] = p_start_addr_16[j];
                    p_start_addr_16[j] = temp;
                }
            }
            else if(num == 4)
            {
                int32_t temp;
                if(p_start_addr_32[j] > p_start_addr_32[j + 1])
                {
                    temp = p_start_addr_32[j + 1];
                    p_start_addr_32[j + 1] = p_start_addr_32[j];
                    p_start_addr_32[j] = temp;
                }
            }
        }
    }
}



/** 
* @brief    冒泡排序-升序
* @param    p_data:数据起始指针，count:数组元素个数，num:待排序数组单元素大小
* @return   none
* @note: 
*/
static void _bubbleAscendingu(void *p_data, uint16_t count, uint16_t num)
{
    uint8_t *p_start_addr_8 = (uint8_t *)p_data;
    uint16_t *p_start_addr_16 = (uint16_t *)p_data;
    uint32_t *p_start_addr_32 = (uint32_t *)p_data;
    
    for(uint16_t i = 0; i < count - 1; i++)
    {
        for(uint16_t j = 0; j < count - 1 - i; j++)
        {
            if(num == 1)
            {
                int8_t temp;
                if(p_start_addr_8[j] > p_start_addr_8[j + 1])
                {
                    temp = p_start_addr_8[j + 1];
                    p_start_addr_8[j + 1] = p_start_addr_8[j];
                    p_start_addr_8[j] = temp;
                }
            }
            else if(num == 2)
            {
                int16_t temp;
                if(p_start_addr_16[j] > p_start_addr_16[j + 1])
                {
                    temp = p_start_addr_16[j + 1];
                    p_start_addr_16[j + 1] = p_start_addr_16[j];
                    p_start_addr_16[j] = temp;
                }
            }
            else if(num == 4)
            {
                int32_t temp;
                if(p_start_addr_32[j] > p_start_addr_32[j + 1])
                {
                    temp = p_start_addr_32[j + 1];
                    p_start_addr_32[j + 1] = p_start_addr_32[j];
                    p_start_addr_32[j] = temp;
                }
            }
        }
    }
}


/** 
* @brief    冒泡排序-升序
* @param    p_data:数据起始指针，count:数组元素个数，num:待排序数组单元素大小
* @return   none
* @note: 
*/
static void _bubbleAscending32f(void *p_data, uint16_t count)
{
    float *p_start_addr = (float *)p_data;
    
    for(uint16_t i = 0; i < count - 1; i++)
    {
        for(uint16_t j = 0; j < count - 1 - i; j++)
        {
            float temp;
            if(p_start_addr[j] > p_start_addr[j + 1])
            {
                temp = p_start_addr[j + 1];
                p_start_addr[j + 1] = p_start_addr[j];
                p_start_addr[j] = temp;
            }
        }
    }
}


/** 
* @brief    计算数组中元素的中值
* @param    p_data:对应数组，count：数组元素个数
* @return   滤波结果
* @note: 
*/
uint16_t FilterCalcMedian16(uint16_t *p_data, uint16_t count)
{
//    qsort(p_data, count, sizeof(uint16_t), _int_cmp);
    _bubbleAscending(p_data, count, sizeof(uint16_t));
    if(count % 2 != 0)
    {
        return p_data[count / 2];
    }
    else
    {
        return ((p_data[count / 2 - 1] + p_data[count / 2]) / 2);
    }
}



/** 
* @brief    滑动平均滤波
* @param    in:最新值输入，p_avg_sum:滑动滤波累加值指针，p_out:滑动滤波之后的值指针，N:滑动窗口大小,accu:累加次数是否达到滑动窗口大小
* @return   none
* @note: 
*/
void FilterCalcSlide32f(float in, float *p_avg_sum, float *p_out, uint16_t N, bool accu)
{
    if(accu == true)
    {
        *p_avg_sum -= *p_avg_sum / N;
        *p_avg_sum += in;
        *p_out = *p_avg_sum / N;
    }
    else
    {
        *p_avg_sum += in;
        *p_out = in;
    }
}

/** 
* @brief    滑动平均滤波
* @param    in:最新值输入，p_avg_sum:滑动滤波累加值指针，p_out:滑动滤波之后的值指针，N:滑动窗口大小,accu:累加次数是否达到滑动窗口大小
* @return   none
* @note: 
*/
void FilterCalcSlide16u(uint16_t in, float *p_avg_sum, uint16_t *p_out, uint16_t N, bool accu)
{
    if(accu == true)
    {
        *p_avg_sum -= *p_avg_sum / N;
        *p_avg_sum += in;
        *p_out = *p_avg_sum / N;
    }
    else
    {
        *p_avg_sum += in;
        *p_out = in;
    }
}


/** 
* @brief    滑动最大值
* @param    p_in:最新值输入指针，in_step:输入历史值存储位置步进, p_out:滑动滤波之后的值指针，N:滑动窗口大小,accu:累加次数是否达到滑动窗口大小
* @return   none
* @note: 
*/
void FilterCalcSlideMaxValue16u(uint16_t *p_in, uint16_t step, uint16_t *p_out, uint16_t N, bool accu)
{
    if(accu == true)
    {
        uint16_t max = *p_in;
        for(uint16_t i = 0; i < N; i++)
        {
            if(max < *(p_in + i * step))
            {
                max = *(p_in + i * step);
            }
        }
        *p_out = max;
    }
    else
    {
        *p_out = *p_in;
    }
}


/** 
* @brief    滑动最小值
* @param    p_in:最新值输入指针，in_step:输入历史值存储位置步进, p_out:滑动滤波之后的值指针，N:滑动窗口大小,accu:累加次数是否达到滑动窗口大小
* @return   none
* @note: 
*/
void FilterCalcSlideMinValue16u(uint16_t *p_in, uint16_t step, uint16_t *p_out, uint16_t N, bool accu)
{
    if(accu == true)
    {
        uint16_t min = *p_in;
        for(uint16_t i = 0; i < N; i++)
        {
            if(min > *(p_in + i * step))
            {
                min = *(p_in + i * step);
            }
        }
        *p_out = min;
    }
    else
    {
        *p_out = *p_in;
    }
}


/** 
* @brief    滑动中位数
* @param    p_in:最新值输入指针，in_step:输入历史值存储位置步进, p_out:滑动滤波之后的值指针，N:滑动窗口大小,accu:累加次数是否达到滑动窗口大小
* @return   none
* @note: 
*/
void FilterCalcSlideMidianValue16u(uint16_t *p_in, uint16_t step, uint16_t *p_out, uint16_t N, bool accu)
{
    uint16_t *p_in_temp = (uint16_t *)malloc(N * sizeof(uint16_t));
    if(p_in_temp == NULL)
    {
        *p_out = *p_in;
        return;
    }
    
    if(accu == true)
    {
        for(int i = 0; i < N; i++)
        {
            p_in_temp[i] = *(p_in + i * step);
        }
        _bubbleAscendingu(p_in_temp, N, sizeof(uint16_t));//排序
        if(N % 2 != 0)
        {
            *p_out = p_in_temp[N / 2];
        }
        else
        {
            *p_out = ((p_in_temp[N / 2 - 1] + p_in_temp[N / 2]) / 2);
        }
    }
    else
    {
        *p_out = *p_in;
    }
    free(p_in_temp);
}



/** 
* @brief    滑动中位数绝对偏差的中位数
* @param    p_in:最新值输入指针，in_step:输入历史值存储位置步进, p_out:滑动滤波之后的值指针，N:滑动窗口大小,accu:累加次数是否达到滑动窗口大小
* @return   none
* @note: 
*/
void FilterCalcSlideMadMidianValue16u(uint16_t *p_in, uint16_t step, uint16_t *p_out, uint16_t N, bool accu)
{
    uint16_t *p_in_temp = (uint16_t *)malloc(N * sizeof(uint16_t));
    uint16_t *p_mad_temp = (uint16_t *)malloc(N * sizeof(uint16_t));
    uint16_t mad;
    
    if(p_in_temp == NULL && p_mad_temp == NULL)
    {
        *p_out = *p_in;
        return;
    }
    else if(p_in_temp == NULL)
    {
        *p_out = *p_in;
        free(p_mad_temp);
        return;
    }
    else if(p_mad_temp == NULL)
    {
        *p_out = *p_in;
        free(p_in_temp);
        return;
    }

    if(accu == true)
    {
        //计算中位数
        for(int i = 0; i < N; i++)
        {
            p_in_temp[i] = *(p_in + i * step);
        }
        _bubbleAscendingu(p_in_temp, N, sizeof(uint16_t));//排序
        if(N % 2 != 0)
        {
            mad = p_in_temp[N / 2];
        }
        else
        {
            mad = ((p_in_temp[N / 2 - 1] + p_in_temp[N / 2]) / 2);
        }
        
        //计算绝对偏差
        for(int i = 0; i < N; i++)
        {
            p_mad_temp[i] = (uint16_t)abs((int16_t)mad - (int16_t)p_in_temp[i]);
        }
        _bubbleAscendingu(p_mad_temp, N, sizeof(uint16_t));//绝对偏差排序
        if(N % 2 != 0)
        {
            *p_out = p_mad_temp[N / 2];
        }
        else
        {
            *p_out = ((p_mad_temp[N / 2 - 1] + p_mad_temp[N / 2]) / 2);
        }
    }
    else
    {
        *p_out = 0;
    }
    free(p_in_temp);
    free(p_mad_temp);
}








