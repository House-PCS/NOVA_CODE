#include "stdio.h"
#include "arc_detect_threshold.h"
#include "math.h"


#define N 512
#define P 4
#define ALPHA 1
#define LEN 512

#define constant_value 5

float signal[N];
float noise[N];
float threshold[N];
int targets[N];
int k = 0;

/***********类平均值算法确定阈值************/
//void CA_CFAR(float *signal,int len,float *threshold)
//{
////    int num = 0,j = 0;
////    for(int i = P/2;i < len - P/2;i++)          //从第5个开始，到第507结束
////    {
////        float sum1 = 0,sum2 = 0,sum = 0;
////        for(int j = 0; j < i - P/2;j++)         //检测值前半部分求和
////        {
////            sum1 += signal[j];
////        }
////        for(int m = i + P/2;m < 512;m++)        //检测值后半部分求和
////        {
////            sum2 += signal[m];
////        }
////        sum = sum1 + sum2;                      //整体求和
////        threshold[i] = (sum / (len-P))*ALPHA;   //当前阈值
////     }
//    float sum = 0;
//  for(int i = 0;i<512; i++)
//  {
//      sum += signal[i];
//  }
//  for(int i = P/2; i<len - P/2;i++)
//  {
//        float  sum1 = 0;
//      for(int j = (i-P/2);j<(i+P/2);j++)
//      {
//          sum1 += signal[j];
//      }
//      threshold[i] = ((sum-sum1)/(len-P))*ALPHA;
////        threshold[i] = (sum1/P)*ALPHA;
//  }
//
//}
/************************END*********************/


/************类平均值算法确定阈值************/
void CA_CFAR(float *signal, float *threshold)
{
//  int num = 0,j = 0;
//  for(int i = P/2;i < len - P/2;i++)          //从第5个开始，到第507结束
//  {
//      float sum1 = 0,sum2 = 0,sum = 0;
//      for(int j = 0; j < i - P/2;j++)         //检测值前半部分求和
//      {
//          sum1 += signal[j];
//      }
//      for(int m = i + P/2;m < 512;m++)        //检测值后半部分求和
//      {
//          sum2 += signal[m];
//      }
//      sum = sum1 + sum2;                      //整体求和
//      threshold[i] = (sum / (len-P))*ALPHA;   //当前阈值
//   }
    if(signal == NULL || threshold == NULL) return; //非空校验
    
    /*类平均值*/
    float sum = 0, sum1 = 0;
    for(int i = 0; i < LEN; i++)
    {
        sum += signal[i];
    }
    for(int i = P / 2; i < LEN - P / 2; i++)
    {
        sum1 = 0;
        for(int j = (i - P / 2); j < (i + P / 2); j++)
        {
            sum1 += signal[j];
        }
        threshold[i] = ((sum - sum1) / (LEN - P)) * ALPHA;
    }

}
/*快速排序*/
static int quick_sort(float *a, int low, int high)
{
    int i  = low; //第一位
    int j  = high;//最后一位
    int key = a[i];//将第一个数作为基数值 -- 先找到一个基准值

    while(i < j)
    {
        while(i < j && a[j] >= key)
        {
            j--;
        }
        a[i] = a[j];
        while(i < j && a[i] <= key)
        {
            i++;
        }
        a[j] = a[i];
    }
    a[i] = key;
    if(i - 1 > low)
    {
        quick_sort(a, low, i - 1);
    }
    if(i + 1 < high)
    {
        quick_sort(a, i + 1, high);
    }

    return 0;
}

/*中位数+绝对值和*/
float Absolute_difference(float32_t *signal)
{
    float32_t median = 0;
    float32_t sum = 0;
//    float array[10]={1,3,1,7,9,2,4,6,8,0};
    /*FFT信号排序*/
    quick_sort(signal, 0, 511);
    /*取中位数*/
    median = (signal[(N / 2) - 1] + signal[N / 2]) / 2;
    /*绝对值和*/
    for(int i = 0; i < N; i++)
    {
        sum += fabs(signal[i] - median);
    }
    return sum;

}

/************************END*********************/



/****************恒定值确定阈值*******************/

void Constant_Value(float *signal, float threshold)
{
    int k = 0;
    threshold = signal[0] / constant_value;    //恒定值--倍除
}

/********************END**********************/

///*********************恒虚警*********************/
//void CA_CFAR1(float *signal,int len,float *threshold)
//{
////    float sum = 0,sum1 = 0;
////    for(int i = 0;i<512; i++)
////    {
////        sum += signal[i];
////    }
//  for(int i = P/2; i<len - P/2;i++)
//  {
//        float sum1 = 0;
//      for(int j = (i-P/2);j<=(i+P/2);j++)
//      {
//          sum1 += signal[j];
//      }
////        threshold[i] = ((sum-sum1)/(len-P))*ALPHA;
//        threshold[i] = (sum1/5)*ALPHA;
//  }

//}
///********************end*********************/

/*****************类恒虚警算法****************/


//#define N 1024
//#define P 64
//#define G 16
//#define ALPHA 0.5

//float siganl[N];
//float noise[N];
//float threshold[N];
//int targets[N];

//int CFAR(float *signal,int len)
//{
//  for(int i = p/2;i< len-p/2; i++)
//  {
//      float sum = 0.0;
//      for(int j = i - p/2;j++)
//      {
//          sum += signal[j];
//      }
//      float mean = sum/p;
//      float noise_estimate = ALPHY* noise[i- p/2]+(1-ALPHY)*mean;
//      float threshold_i = G*noise_estimate;
//      threshold[i] = thredshold_i;
//      if(signal[i] > threshold_i)
//      {
//          targets[i] = i;
//      }
//  }
//}
/************************END************************/

/*******************面积阈值法****************/
//利用微积分思想将曲线分为511个小长方形，以1为步进值（宽），阈值点为高，相乘即为小长方形面积

//int Areathershold(float *signal)   //电弧强度也可以用面积大小来表示
//{
//    float threshold[512];
//    int AreaValue = 0;
//    CA_CFAR(signal,threshold);
////    CA_CFAR(signal,len,threshold);
//    for(uint16_t i = 0;i <= 512;i++)
//    {
//        AreaValue +=threshold[i];
//    }
//    return AreaValue;
//}

//int CountThershold(float *signal)
//{
//    float threshold[512];
//    int CountValue = 0;
////    CA_CFAR(signal,threshold);   //
//    for(uint16_t i =0;i<=512;i++)
//    {
//        if(signal[i]>threshold[i])
//        {
//            CountValue++;
//        }
//    }
//    return CountValue;
//}



/**********************电弧强度***********************/

int Arc_strength(float *signal, float *threshold)
{
//  float threshold[512];
    int num = 0;
//    float sum = 0;
    CA_CFAR(signal, threshold); //类平均值

    //类恒虚警法

    for(int i = 0; i < LEN; i++)  //计数阈值
    {
        if(signal[i] > threshold[i])
        {
            num++;
        }
    }
    return num;

//        for(uint16_t i = 0;i <= 512;i++)  //面积阈值
//    {
//        AreaValue +=threshold[i];
//    }
//    return AreaValue;


}

/************电弧判断+消抖******************/   //连续设定阈值之后，确定电弧发生情况。
int Arcjudgment(float *signal, float *threshold, int set)
{
    float32_t k = 0;
    float32_t value[512] = {0};
    static uint8_t arc_ret = 0;

    /* 10秒窗口内每隔250ms采样一次做比较：k<set 计数+1 */
    static uint32_t window_start_ms = 0;
    static uint32_t last_sample_ms = 0;
    static int16_t  low_cnt = 0;
    static uint32_t last_leak_ms = 0;

    /* 维持“1分钟没出现则清零”的逻辑（以触发事件为准） */
    static uint32_t last_event_ms = 0;
    const uint32_t now_ms = tim_get_systime();
//    for(int i = 0;i<512;i++)
//    {
//        value[i] = signal[i];
//    }
//    k = Absolute_difference(value);
    k = Arc_strength(signal,threshold);

    /* 初始化窗口 */
    if(window_start_ms == 0)
    {
        window_start_ms = now_ms;
        last_sample_ms = now_ms;
        last_leak_ms = now_ms;
        low_cnt = 0;
    }

    /* 漏桶：每2秒 -1（不小于0） */
    if(last_leak_ms != 0U)
    {
        uint32_t leak_sec = tim_difftime(last_leak_ms, now_ms) / 2000U;
        if(leak_sec > 0U)
        {
            if(low_cnt > 0)
            {
                if(leak_sec >= (uint32_t)low_cnt)
                {
                    low_cnt = 0;
                }
                else
                {
                    low_cnt -= (int16_t)leak_sec;
                }
            }
            /* 保留余数，避免时间误差累积 */
            last_leak_ms += leak_sec * 2000U;
        }
    }

    /* 每隔250ms取一次“采样点”进行比较计数 */
    if(tim_difftime(last_sample_ms, now_ms) >= 250U)
    {
        last_sample_ms = now_ms;
        if(k > set)
        {
            if(low_cnt < 0x7FFF)
            {
                low_cnt++;
            }
        }
    }

    /* 10s窗口结束时做一次判定，并更新计数/窗口 */
    if(tim_difftime(window_start_ms, now_ms) >= 10000U)
    {
        if(low_cnt >= 13)
        {
            arc_ret = 1;
            last_event_ms = now_ms;
        }

        /* 更新窗口：重置计数，开始下一轮20s统计 */
        window_start_ms = now_ms;
        last_sample_ms = now_ms;
        last_leak_ms = now_ms;
        low_cnt = 0;
    }

    /* 重置返回值逻辑保持不变：1分钟没出现事件 -> 清零返回值 */
    if((arc_ret == 1) && (last_event_ms != 0U) && (tim_difftime(last_event_ms, now_ms) >= 60000U))
    {
        arc_ret = 0;
    }

    return arc_ret;
}


/*****************电弧判断*******************/

//int ArcJudge(uint16_t value,uint16_t threshold)
//{
//      if(value > threshold)
//      {
//          return 1;
//      }
//      else
//      {
//          return 0;
//      }
//}


/*****************消抖********************/

/*****************线性回归分析********************/
//void Least_square_method(float *signal)
//{
//  float sum_X = 0;
//  float sum_Y = 0;
//  float sum_X_2 = 0;
//  float sum_Y_2 = 0;
//  float sum_XY = 0;
//  float aver_X = 0;
//  float aver_Y = 0;
//  float Lxx = 0;
//  float Lyy = 0;
//  float Lxy = 0;
//  int arr_x[512] ={0};
//  /*********去除数据中的五个最大值，五个最小值*****************/
//  for(int i=0;i < 512;i++)
//  {
//      arr_x[i] = i+1;
//  }
//  for(int j = 0;j < 5;j++)
//  {
//      int max_index = 0;
//      for(int k = 1;k < (512-j);k++)
//      {
//          if(signal[j] > signal[max_index])
//          {
//              max_index = j;
//          }
//      }
//      for(int m = max_index;m < (512 - j);m++)
//      {
//          arr_x[m] = arr_x[m+1];
//          signal[m] =  signal[m+1];
//      }
//  }
//  for(int n = 0; n < 5; n++)
//  {
//      int min_index = 0;
//      for(int o = 1;o < (507-n);o++)
//      {
//          if(signal[o] < signal[min_index])
//          {
//              min_index = o;
//          }
//      }
//      for(int p = min_index;p < (506 - n);p++)
//      {
//          arr_x[p] = arr_x[p+1];
//          signal[p] =  signal[p+1];
//      }
//  }
//  ///回归分析公式计算//
//  for(int q = 0;q < 502; q++)
//  {
//      sum_X +=arr_x[q];
//      sum_Y +=signal[q];
//      sum_X_2 += arr_x[q]*arr_x[q];
//      sum_Y_2 += signal[q]*signal[q];
//      sum_XY += arr_x[q]*signal[q];
//  }
//  aver_X = sum_X/502;
//  aver_Y = sum_Y/502;
//  Lxx = sum_X_2 - (sum_X*sum_X)/502;
//  Lyy = sum_Y_2 - (sum_Y*sum_Y)/502;
//  Lxy = sum_XY - (sum_X*sum_Y)/502;
//
//  float z = Lxy/Lxx;   //斜率
//  float B0 = aver_Y - z*aver_X; //截距
//  float U = z*Lxy;            //回归平方和
//  float Q = Lyy - U;          //残余平方和
//  float F = (U*(N-2))/Q;      //统计量F ，F检验表 判断回归方程显著性
//
//}








