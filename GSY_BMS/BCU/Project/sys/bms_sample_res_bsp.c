/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-11-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */



#include "bms_sample_res_bsp.h"
#include "systick.h"
#include "mcu_bsp.h"
#include "ht7017.h"
#include "string.h"


#define RES_COMPUTE_COEFF   0.8
#define RES_SWITCH_DELAY    3000        //单位1ms     5000

BmsSampleResType gBmsSampleRes = {0};
const BmsSampleResType *const IR_BmsSampleRes = &gBmsSampleRes;
EN_SAMPLE_RES SampleResState = RES_INIT;
/** 
* @brief  HT7017初始化
* @param  none
* @return  none
* @note:
*
*/
static int Ht7017_Init()
{
    int ret = 0;
    static uint8_t state = 0;
    static uint8_t cmd[10] = {0};
    uint8_t sum = 0;
    static uint8_t sum_err = 0;
    static uint32_t com_time = 0;
    

    return ret;
}

/** 
* @brief  电压采样读取函数
* @param  reg：寄存器地址
* @param  data：数据缓存
* @return  int：1，成功；0，等待；-1，失败
* @note:
*
*/
static int SampleVoltage(uint8_t reg,int16_t *data)
{
    int ret = 0;
    static uint8_t state = 0;
    static uint8_t cmd[10] = {0};
    uint8_t sum = 0;
    static uint8_t sum_err = 0;
    static uint32_t com_time = 0;
    uint32_t temp_u32 = 0;
    
    
    return ret;
}

/** 
* @brief  启动绝缘检测
* @param  none
* @return  none
* @note:
*
*/
void SampleResStart()
{
    gBmsSampleRes.sample_en = true;
}
/** 
* @brief  停止绝缘检测
* @param  none
* @return  none
* @note:
*
*/
void SampleResStop()
{
    gBmsSampleRes.sample_en = false;
    SampleResState = RES_IDLE;
    RES_KG_SET(false);
    RES_KP_SET(false);
    RES_KN_SET(false);
}
/** 
* @brief  绝缘采样模块初始化
* @param  none
* @return  none
* @note:
*
*/
void SampleRes_Init()
{
    Ht7017_Init();
}
/** 
* @brief  绝缘检测模块主函数
* @param  none
* @return  none
* @note:
*
*/
void SampleResMain()
{
    static uint32_t sample_res_time = 0;
    int8_t temp = 0;
    int32_t temp32 = 0;
    
    
}


