#include "stdio.h"
#include "ca_cfar.h"


#define N 512
#define P 10
#define ALPHA 1

float signal[N];
float noise[N];
float threshold[N];
int targets[N];
int k = 0;


/*********类平均值算法确定阈值************/
void CA_CFAR(float *signal,int len,float *threshold)
{
//	int num = 0,j = 0;
//	for(int i = P/2;i < len - P/2;i++)          //从第5个开始，到第507结束
//	{
//		float sum1 = 0,sum2 = 0,sum = 0;
//		for(int j = 0; j < i - P/2;j++)         //检测值前半部分求和
//		{
//			sum1 += signal[j];
//		}
//		for(int m = i + P/2;m < 512;m++)        //检测值后半部分求和
//		{
//			sum2 += signal[m];
//		}
//		sum = sum1 + sum2;                      //整体求和
//		threshold[i] = (sum / (len-P))*ALPHA;   //当前阈值
//	 }
	float sum = 0,sum1 = 0;
	for(int i = 0;i<512; i++)
	{
		sum += signal[i];
	}
	for(int i = P/2; i<len - P/2;i++)
	{
		sum1 =0;
		for(int j = (i-P/2);j<(i+P/2);j++)
		{
			sum1 += signal[j];
		}
		threshold[i] = ((sum-sum1)/(len-P))*ALPHA;
	}

}
/************************END*********************/


/****************恒定值确定阈值*******************/
int Constant_Value(float *signal,int len)
{
	int k = 0;
	for(int i = 0;i < len;i++)
	{		
		if(signal[i]>(signal[0]/5))              
		{
			k++ ;
		}
	}
		return k;
}
/********************END**********************/


/**************类恒虚警算法 ************/


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
//	for(int i = p/2;i< len-p/2; i++)
//	{
//		float sum = 0.0;
//		for(int j = i - p/2;j++)
//		{
//			sum += signal[j];
//		}
//		float mean = sum/p;
//		float noise_estimate = ALPHY* noise[i- p/2]+(1-ALPHY)*mean;
//		float threshold_i = G*noise_estimate;
//		threshold[i] = thredshold_i;
//		if(signal[i] > threshold_i)
//		{
//			targets[i] = i;
//		}
//	}
//}
/**************************END************************/


/*****************电弧强度*********************/

int Arc_strength(float *signal,int len)
{
	int num = 0;
	CA_CFAR(signal,len,threshold);
	for(int i = 0;i< len;i++)
	{		
		if(signal[i]>threshold[i])              
		{
			num++;
		}
	}
	return num;
}

/************电弧判断+消抖******************/   //连续超过几个数之后，确定电弧发生。
int Arcjudgment(float *signal,int len,int Continuous_value)
{
	int k = 0;
	float threshold[512];
	CA_CFAR(signal,len,threshold);
	for(int i = 0;i < len;i++)
	{		
		if(signal[i]>threshold[i]) 
//		if(signal[i]>0) 			
		{
			k++ ;
		}
		else
		{
			k-- ;
		}
		if(i == 100)
		{
			if(k > Continuous_value)
			{
				return k;
			}
			else
			{
				return 0;
			}
		}
	}
	return 0;			
}






/*****************电弧判断*******************/

int ArcJudge(uint16_t value,uint16_t threshold)	
{
		if(value > threshold)
		{
			return 1;
		}
		else
		{
			return 0;
		}
}


/*****************消抖********************/

/*****************线性回归分析********************/
void Least_square_method(float *signal)
{
	float sum_X = 0;
	float sum_Y = 0;
	float sum_X_2 = 0;
	float sum_Y_2 = 0;
	float sum_XY = 0;
	float aver_X = 0;
	float aver_Y = 0;
	float Lxx = 0;
	float Lyy = 0;
	float Lxy = 0;
	int arr_x[512] ={0}; 
	/*********去除数据中的五个最大值，五个最小值*****************/
	for(int i=0;i < 512;i++)
	{
		arr_x[i] = i+1;
	}
	for(int j = 0;j < 5;j++)
	{
		int max_index = 0;
		for(int k = 1;k < (512-j);k++)
		{
			if(signal[j] > signal[max_index])
			{
				max_index = j;
			}
		}
		for(int m = max_index;m < (512 - j);m++)
		{
			arr_x[m] = arr_x[m+1];
			signal[m] =  signal[m+1];
		}
	}
	for(int n = 0; n < 5; n++)
	{
		int min_index = 0;
		for(int o = 1;o < (507-n);o++)
		{
			if(signal[o] < signal[min_index])
			{
				min_index = o;
			}
		}
		for(int p = min_index;p < (506 - n);p++)
		{
			arr_x[p] = arr_x[p+1];
			signal[p] =  signal[p+1];
		}
	}
	///回归分析公式计算//
	for(int q = 0;q < 502; q++)
	{
		sum_X +=arr_x[q];
		sum_Y +=signal[q];
		sum_X_2 += arr_x[q]*arr_x[q];
		sum_Y_2 += signal[q]*signal[q];
		sum_XY += arr_x[q]*signal[q];
	}
	aver_X = sum_X/502;
	aver_Y = sum_Y/502;
	Lxx = sum_X_2 - (sum_X*sum_X)/502;
	Lyy = sum_Y_2 - (sum_Y*sum_Y)/502;
	Lxy = sum_XY - (sum_X*sum_Y)/502;
	
	float z = Lxy/Lxx;   //斜率
	float B0 = aver_Y - z*aver_X; //截距
	float U = z*Lxy;            //回归平方和
	float Q = Lyy - U;			//残余平方和
	float F = (U*(N-2))/Q;      //统计量F ，F检验表 判断回归方程显著性
	 
}








