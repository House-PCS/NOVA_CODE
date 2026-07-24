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



#include "res_detect_process.h"
#include "systick.h"
#include "bsp.h"
#include "dev_ht7017.h"
#include "string.h"
#include "dev_chan_sw.h"
#include "modbus_analysis.h"
#include "math.h"

#define RES_SWITCH_DELAY    1000        //单位1ms     5000
//#define RES_STABLE_DELAY    3000


BmsSampleResType gBmsSampleRes[4] = {0};
//const BmsSampleResType *const IR_BmsSampleRes = &gBmsSampleRes;
EN_SAMPLE_RES SampleResState = RES_READY;

static int Ht7017_Init()                                     //计量IC HT7017初始化
{
    int ret = 0;
    static uint8_t state = 0;
    static uint8_t cmd[6] = {0};
    uint8_t sum = 0;
    static uint8_t sum_err = 0;
    static uint32_t com_time = 0;
    switch(state)
    {
        case 0:
        {
            Ht7017UartSend(cmd, HT_Write(HT_WPCFG, WRITE_EN_2, cmd)); //计量IC写保护寄存器
            state++;
            com_time = tim_get_systime();
            break;
        }
        case 1:
        {
            if(Ht7017RxTxState.ReceiveFlg)                         //验证返回包
            {
                if(Ht7017UartHandle.pRxBuf[0] == ACK_CORRECT)
                {
                    sum_err = 0;
                    state ++;
                }
                else
                {
                    if(sum_err++ >= 2)
                    {
                        sum_err = 0;
                        ret = -1;
                        state = 0;
                    }
                }
                Ht7017RxTxState.ReceiveNum = 0;
                Ht7017RxTxState.ReceiveFlg = false;
            }
            if(tim_get_systime() - com_time >= HT_COM_TIMEOUT )   //判断是否超时
            {
                state = 0;
                if(sum_err++ >= 2)
                {
                    sum_err = 0;
                    ret = -1;
                }
            }
            break;
        }
        case 2:
        {
            Ht7017UartSend(cmd, HT_Write(HT_ANAEN, 0x07, cmd)); //ADC开关寄存器 0x07打开电流通道I1、I2，电压通道U的ADC开关信号
            state++;
            com_time = tim_get_systime();
            break;
        }
        case 3:
        {
            if(Ht7017RxTxState.ReceiveFlg)
            {
                if(Ht7017UartHandle.pRxBuf[0] == ACK_CORRECT)    //验证返回包
                {
                    sum_err = 0;
                    state ++;
                }
                else
                {
                    if(sum_err++ >= 2)
                    {
                        sum_err = 0;
                        ret = -1;
                        state = 0;
                    }
                }
                Ht7017RxTxState.ReceiveNum = 0;
                Ht7017RxTxState.ReceiveFlg = false;
            }
            if(tim_get_systime() - com_time >= HT_COM_TIMEOUT )   //判断是否超时
            {
                state --;
                if(sum_err++ >= 2)
                {
                    sum_err = 0;
                    ret = -1;
                }
            }
            break;
        }
        case 4:
        {
            Ht7017UartSend(cmd, HT_Read(HT_ANAEN, cmd));         //读取ADC开关寄存器验证
            state++;
            com_time = tim_get_systime();
            break;
        }
        case 5:
        {
            if(Ht7017RxTxState.ReceiveFlg)
            {
                memcpy(&cmd[2], Ht7017UartHandle.pRxBuf, 4);
                sum = cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4];
                sum = ~sum;
                if(sum == cmd[5])
                {
                    if(0x07 == (uint32_t)(cmd[2] << 16 | cmd[3] << 8 | cmd[4]))
                    {
//                        ret = 1;
                        sum_err = 0;
//                        state = 0;
                        state ++;
                    }
                    else
                    {
                        sum_err = 0;
                        ret = -1;
                        state = 0;
                    }
                }
                else
                {
                    if(sum_err++ >= 2)
                    {
                        sum_err = 0;
                        ret = -1;
                        state = 0;
                    }
                }
                Ht7017RxTxState.ReceiveNum = 0;
                Ht7017RxTxState.ReceiveFlg = false;
            }
            if(tim_get_systime() - com_time >= HT_COM_TIMEOUT )
            {
                state --;
                if(sum_err++ >= 2)
                {
                    sum_err = 0;
                    ret = -1;
                }
            }
            break;
        }
        case 6:
        {
            Ht7017UartSend(cmd, HT_Write(HT_ModuleEn, ModuleEn_Data, cmd)); //开启模式控制寄存器  全部写0为关闭高通滤波器（直流）
            state++;
            com_time = tim_get_systime();
            break;
        }
        case 7:
        {
            if(Ht7017RxTxState.ReceiveFlg)                                //返回验证
            {
                if(Ht7017UartHandle.pRxBuf[0] == ACK_CORRECT)
                {
                    sum_err = 0;
                    state ++;
                }
                else
                {
                    if(sum_err++ >= 2)
                    {
                        sum_err = 0;
                        ret = -1;
                        state = 0;
                    }
                }
                Ht7017RxTxState.ReceiveNum = 0;
                Ht7017RxTxState.ReceiveFlg = false;
            }
            if(tim_get_systime() - com_time >= HT_COM_TIMEOUT )         //超时判断
            {
                state --;
                if(sum_err++ >= 2)
                {
                    sum_err = 0;
                    ret = -1;
                }
            }
            break;
        }
        case 8:
        {
            Ht7017UartSend(cmd, HT_Read(HT_ModuleEn, cmd));            //读取状态控制寄存器信息
            state++;
            com_time = tim_get_systime();
            break;
        }
        case 9:
        {
            if(Ht7017RxTxState.ReceiveFlg)
            {
                memcpy(&cmd[2], Ht7017UartHandle.pRxBuf, 4);
                sum = cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4];
                sum = ~sum;
                if(sum == cmd[5])
                {
                    if(ModuleEn_Data == (uint32_t)(cmd[2] << 16 | cmd[3] << 8 | cmd[4]))
                    {
                        ret = 1;
                        sum_err = 0;
                        state = 0;
                    }
                    else
                    {
                        sum_err = 0;
                        ret = -1;
                        state = 0;
                    }
                }
                else
                {
                    if(sum_err++ >= 2)
                    {
                        sum_err = 0;
                        ret = -1;
                        state = 0;
                    }
                }
                Ht7017RxTxState.ReceiveNum = 0;
                Ht7017RxTxState.ReceiveFlg = false;
            }
            if(tim_get_systime() - com_time >= HT_COM_TIMEOUT)         //超时判断
            {
                state --;
                if(sum_err++ >= 2)
                {
                    sum_err = 0;
                    ret = -1;
                }
            }
        }
        default :
            break;
    }
    return ret;
}

static int SampleVoltage1(uint8_t reg, double *data)  //该采样函数可以获得ADC采样原始值和经过转换后得到的值
{
    int ret = 0;
    static uint8_t state = 0;
    static uint8_t cmd[10] = {0};
    uint8_t sum = 0;
    static uint8_t sum_err = 0;
    static uint32_t com_time = 0;
    int32_t temp = 0;
    uint32_t temp1 = 0;
    uint32_t temp2 = 0;
    switch(state)
    {
        case 0:
        {
            HT_Read(reg, cmd);
            Ht7017UartSend(cmd, 2);
            state++;
            com_time = tim_get_systime();
            break;
        }
        case 1:
        {
            if(Ht7017RxTxState.ReceiveFlg)
            {
                memcpy(&cmd[2], Ht7017UartHandle.pRxBuf, 4);
                sum = cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4];
                sum = ~sum;
                if(sum == cmd[5])
                {
                    temp = (uint32_t)cmd[2] << 16 | (uint32_t)cmd[3] << 8 | (uint32_t)cmd[4];

                    /*************************/
                    /****    ADC采样转化  ****/
                    if((temp & (1 << 21)) != 0) //负数
                    {
                        temp |= (0xFF << 24);
                    }
//                  temp1 = ((~temp)&0x3FFFF)+0x01;
//                  temp2 = temp & 0x3FFFF;                           //0bit-17bit 为数据位
//                  if((temp & 0x40000) == 0x40000)                   //18bit-23bit 为符号位
//                  {
//                      *temp_u32 = 0 - (temp1);
//                      *data = 0 - ((float)temp1 * 1200)/262144; //单位（mv）  典型电压1.2V
//                  }
//                  else
//                  {
//                      *temp_u32 = temp2;
//                      *data = ((float)temp * 1200)/262144 ; //单位（mv）
//                  }

                    *data = (double)(temp * 900) / 262144 ; //单位（mv）


                    /*****   ADC采样采样计算END      *****/
                    /*************************************/

                    /*************************************/
                    /*       有效值线性关系       */
//                  *temp_u32 = temp;
//                  *data = *temp_u32 * 0.000108 - 2.7835;
                    /*  有效值计算  END  */
//                  temp1 = ((~temp)&0x3FFFF)+0x01;
//                  if((temp & 0x40000) == 0x40000)
//                  {
//                      *temp_u32 = 0 - (temp1);
////                        *data = 0 - ((float)temp1 * 1)/1; //单位（mv）
//                  }
//                  else
//                  {
//                      *temp_u32 = temp;
////                        *data = ((float)temp * 1)/1 ; //单位（mv）
//                  }
//                  *data = (temp_u32 * 1200)/262144 ; //单位（mv）

//                  *temp_u32 = temp2>>13;
//                    if(reg == HT_RMS_I1)
//                    {
//                        gBmsSampleRes.volt_ch1  = temp_u32;
//                    }
//                    else if(reg == HT_RMS_I2)
//                    {
//                        gBmsSampleRes.volt_ch2  = temp_u32;
//                    }

//                    {
//                        *data = temp * 0.000108 - 2.7835;             //所拟合的线性系数
//                    }
                    ret = 1;
                    sum_err = 0;
                    state = 0;
                }
                else
                {
                    if(sum_err++ >= 2)
                    {
                        sum_err = 0;
                        ret = -1;
                        state = 0;
                    }
                }
                temp2 = tim_get_systime() - com_time;
                Ht7017RxTxState.ReceiveNum = 0;
                Ht7017RxTxState.ReceiveFlg = false;
            }

            if(tim_get_systime() - com_time >= HT_COM_TIMEOUT )
            {

                state = 0;
                if(sum_err++ >= 2)
                {
                    sum_err = 0;
                    ret = -1;
                }
            }
        }
        default :
            break;
    }
    return ret;
}
static int SampleVoltage(uint8_t reg, uint32_t *temp_u32)   //只获取有效值
{
    int ret = 0;
    static uint8_t state = 0;
    static uint8_t cmd[10] = {0};
    uint8_t sum = 0;
    static uint8_t sum_err = 0;
    static uint32_t com_time = 0;
    uint32_t temp = 0;
    switch(state)
    {
        case 0:
        {
            HT_Read(reg, cmd);
            Ht7017UartSend(cmd, 2);
            state++;
            com_time = tim_get_systime();
            break;
        }
        case 1:
        {
            if(Ht7017RxTxState.ReceiveFlg)
            {
                memcpy(&cmd[2], Ht7017UartHandle.pRxBuf, 4);
                sum = cmd[0] + cmd[1] + cmd[2] + cmd[3] + cmd[4];
                sum = ~sum;
                if(sum == cmd[5])
                {
                    *temp_u32 = (uint32_t)cmd[2] << 16 | (uint32_t)cmd[3] << 8 | (uint32_t)cmd[4];

                    ret = 1;
                    sum_err = 0;
                    state = 0;
                }
                else
                {
                    if(sum_err++ >= 2)
                    {
                        sum_err = 0;
                        ret = -1;
                        state = 0;
                    }
                }
                temp = tim_get_systime() - com_time;
                Ht7017RxTxState.ReceiveNum = 0;
                Ht7017RxTxState.ReceiveFlg = false;
            }
            if(tim_get_systime() - com_time >= HT_COM_TIMEOUT )
            {

                state = 0;
                if(sum_err++ >= 2)
                {
                    sum_err = 0;
                    ret = -1;
                }
            }
        }
        default :
            break;
    }
    return ret;
}
void SampleResStart(EN_CHANNAL channal_switch)                                       //开始采样
{
    gBmsSampleRes[channal_switch].sample_en = true;
}
void SampleResStop(EN_CHANNAL channal_switch)                                        //停止采样
{
    gBmsSampleRes[channal_switch].sample_en = false;
    SampleResState = RES_IDLE;
    RES_KG_SET(false);
    RES_KP_SET(false);
    RES_KN_SET(false);
}
void SampleRes_Init()                                       //计量IC HT7017初始化
{
    Ht7017_Init();
}


void Res_Detect_PrepareRetry(void)
{
    uint8_t i;

    SampleResState = RES_READY;           
    channal_switch = EN_CHANNAL1;         
    for(i = 0; i < 4; i++)
    {
        gBmsSampleRes[i].sample_over = false; 
        gBmsSampleRes[i].sample_en = false;   
    }
}

void SampleRes_Main(EN_CHANNAL channal_switch)              //绝缘阻抗采值、计算过程
{
    static uint32_t sample_res_time = 0;
    static uint32_t switch_stable_time = 0;
    int8_t temp = 0;
    float temp32 = 0;
    if(Modbus_writeReg.resdetectenable == 1)
    {
        SampleResState = RES_READY;
        gBmsSampleRes[0].sample_over = false;
        gBmsSampleRes[1].sample_over = false;
        gBmsSampleRes[0].sample_en = false;
        gBmsSampleRes[1].sample_en = false;

    }
    switch((uint8_t)SampleResState)
    {
        case RES_READY:
        {
            GpioBitReset(&DetectChannelSwitchHandle1);
            GpioBitReset(&DetectChannelSwitchHandle2);
            RES_KG_SET(RESET);
            RES_KP_SET(RESET);
            RES_KN_SET(RESET);
            SampleResState = RES_START;
            break;
        }
        case RES_START:
        {
            dev_chan_sw(channal_switch);                    //通道选择
            SampleResState = RES_WAIT;
            switch_stable_time = tim_get_systime();
            break;
        }
        case RES_WAIT:
        {
            if(tim_get_systime() - sample_res_time >= RES_SWITCH_DELAY)
            {
                SampleResStart(channal_switch);              //检测开始置位
                SampleResState = RES_INIT;
            }
            break;
        }
        case RES_INIT:
        {
            gBmsSampleRes[channal_switch].res_main = 0xFFFF;
            if(Ht7017_Init() == 1)
            {
                ModbusTest_ReadReg.ht7017_init_result = 1;
                SampleResState = RES_IDLE;
            }
            else
            {
                SampleResState = RES_INIT;
            }
            break;
        }
        case RES_IDLE:
        {
            if(gBmsSampleRes[channal_switch].sample_en)
            {
                SampleResState = RES_KP_ON;
            }
            break;
        }
        case RES_KP_ON:
        {
            RES_KG_SET(SET);
            RES_KP_SET(SET);
            RES_KN_SET(RESET);
            SampleResState = RES_WAIT1;
            sample_res_time = tim_get_systime();
            break;
        }
        case RES_WAIT1:
        {
            if(tim_get_systime() - sample_res_time >= RES_SWITCH_DELAY)
            {
                SampleResState = RES_SAMPLE_VP1;
            }
            break;
        }
        case RES_SAMPLE_VP1:
        {
//            temp = SampleVoltage(HT_RMS_I2,&gBmsSampleRes[channal_switch].adc_kp1);
            temp = SampleVoltage1(HT_SQL_I2, &gBmsSampleRes[channal_switch].volt_kp1);
            if(temp == 1)
            {
                SampleResState = RES_SAMPLE_VN1;

            }
            else if(temp == -1)
            {
                SampleResState = RES_SAMPLE_ERR;
            }
            break;
        }
        case RES_SAMPLE_VN1:
        {
//            temp = SampleVoltage(HT_RMS_I1,&gBmsSampleRes[channal_switch].adc_kn1);
            temp = SampleVoltage1(HT_SQL_I1, &gBmsSampleRes[channal_switch].volt_kn1);
            if(temp == 1)
            {
                SampleResState = RES_KN_ON;
            }
            else if(temp == -1)
            {
                SampleResState = RES_SAMPLE_ERR;
            }
            break;
        }
        case RES_KN_ON:
        {
            RES_KG_SET(SET);
            RES_KP_SET(RESET);
            RES_KN_SET(SET);
            SampleResState = RES_WAIT2;
            sample_res_time = tim_get_systime();
            break;
        }
        case RES_WAIT2:
        {
            if(tim_get_systime() - sample_res_time >= RES_SWITCH_DELAY)
            {
                SampleResState = RES_SAMPLE_VP2;
                sample_res_time = tim_get_systime();
            }
            break;
        }
        case RES_SAMPLE_VP2:
        {

//            temp = SampleVoltage(HT_RMS_I2,&gBmsSampleRes[channal_switch].adc_kp2);
            temp = SampleVoltage1(HT_SQL_I2, &gBmsSampleRes[channal_switch].volt_kp2);
            if(temp == 1)
            {
                SampleResState = RES_SAMPLE_VN2;
            }
            else if(temp == -1)
            {
                SampleResState = RES_SAMPLE_ERR;
            }
            break;
        }
        case RES_SAMPLE_VN2:
        {

//            temp = SampleVoltage(HT_RMS_I1,&gBmsSampleRes[channal_switch].adc_kn2);
            temp = SampleVoltage1(HT_SQL_I1, &gBmsSampleRes[channal_switch].volt_kn2);
            if(temp == 1)
            {
                SampleResState = RES_CALIBRATION;
            }
            else if(temp == -1)
            {
                SampleResState = RES_SAMPLE_ERR;
            }
            break;
        }
        case RES_CALIBRATION:
        {
//            /*打开KG，打开KP，断开kn时，上通道（V1S1）第1通道采集值 */
//            gBmsSampleRes[channal_switch].volt_kp1 = (double)gBmsSampleRes[channal_switch].adc_kp1 * 0.00010613 + 0.1936; //后面为拟合系数
//            /*打开KG，打开KP，断开kn时，下通道（V2S1）第1通道采集值 */
//            gBmsSampleRes[channal_switch].volt_kn1 = (double)gBmsSampleRes[channal_switch].adc_kn1 * 0.00011238 +0.1576;
//            /*打开KG，断开KP，打开kn时，上通道（V1S2）第1通道采集值 */
//            gBmsSampleRes[channal_switch].volt_kp2 = (double)gBmsSampleRes[channal_switch].adc_kp2 * 0.00010794 + 0.6439;
//            /*打开KG，断开KP，打开kn时，上通道（V2S2）第1通道采集值 */
//            gBmsSampleRes[channal_switch].volt_kn2 = (double)gBmsSampleRes[channal_switch].adc_kn2 * 0.00011259 - 0.4945;
////            gBmsSampleRes[channal_switch].volt_kn2 = (double)gBmsSampleRes[channal_switch].adc_kn2 * 0.00011259 - 0.945;
//            //一断一短和一短一电阻
//            if(gBmsSampleRes[channal_switch].adc_kp1 < 130000)
//             {
//                 gBmsSampleRes[channal_switch].volt_kp1 = 0;
//             }
//             if(gBmsSampleRes[channal_switch].adc_kn1 < 130000)
//             {
//                gBmsSampleRes[channal_switch].volt_kn1 = 0;
//             }
//             if(gBmsSampleRes[channal_switch].adc_kp2 < 130000)
//             {
//                 gBmsSampleRes[channal_switch].volt_kp2 = 0;
//             }
//             if(gBmsSampleRes[channal_switch].adc_kn2 < 130000)
//             {
//                 gBmsSampleRes[channal_switch].volt_kn2 = 0;
//             }
            gBmsSampleRes[channal_switch].volt_kn1 = - gBmsSampleRes[channal_switch].volt_kn1;
            gBmsSampleRes[channal_switch].volt_kn2 = - (gBmsSampleRes[channal_switch].volt_kn2);
            SampleResState = RES_COMPUTE;
            break;
        }
        case RES_COMPUTE:
        {
            RES_KG_SET(RESET);
            RES_KN_SET(RESET);
            double temp1 = 0, temp2 = 0, temp3 = 0;
            temp1 = (R3 * R9 / (R3 + R9));

            temp2 = ((gBmsSampleRes[channal_switch].volt_kp2 - gBmsSampleRes[channal_switch].volt_kn2) * gBmsSampleRes[channal_switch].volt_kp1) /
                    (gBmsSampleRes[channal_switch].volt_kn2 * gBmsSampleRes[channal_switch].volt_kp1 - gBmsSampleRes[channal_switch].volt_kp2 * gBmsSampleRes[channal_switch].volt_kn1);

            temp3 = ((gBmsSampleRes[channal_switch].volt_kp2 - gBmsSampleRes[channal_switch].volt_kn2) * gBmsSampleRes[channal_switch].volt_kn1) /
                    (gBmsSampleRes[channal_switch].volt_kp2 * gBmsSampleRes[channal_switch].volt_kn1 - gBmsSampleRes[channal_switch].volt_kn2 * gBmsSampleRes[channal_switch].volt_kp1);

            //总电压
            gBmsSampleRes[channal_switch].res_total_voltage = ((gBmsSampleRes[channal_switch].volt_kp1 - gBmsSampleRes[channal_switch].volt_kn1) * (temp1 + R1)) / temp1 / 1000;
            //rx
            gBmsSampleRes[channal_switch].res_p1 = (int32_t)R5 / ((R6 / (R5 + R6) * temp3 - R5 / (temp1 + R1) - 1));
//            gBmsSampleRes[channal_switch].res_p1 = temp32;
//            gBmsSampleRes[channal_switch].res_p1 = ((temp32 < 0) ? (-temp32) : temp32);
            //ry
            gBmsSampleRes[channal_switch].res_n1 = (int32_t)(R5 + R6) / (R6 / R5 * temp2 - (R5 + R6) / (temp1 + R1) - 1);
//            gBmsSampleRes[channal_switch].res_n1 = ((temp32 < 0) ? (-temp32) : temp32);
//            gBmsSampleRes[channal_switch].res_n1 = temp32;
            /*无光伏时，阻抗统一为最大值*/
            if(gBmsSampleRes[channal_switch].res_total_voltage < 200)/*未接入PV，一律认为最大阻值*/
            {
                gBmsSampleRes[channal_switch].res_p1 = 0xffff;
                gBmsSampleRes[channal_switch].res_n1 = 0xffff;
//                gBmsSampleRes[channal_switch].res_main = 0xffff;
            }
            if(gBmsSampleRes[channal_switch].res_p1 <= -100 || gBmsSampleRes[channal_switch].res_p1 > 10000)
            {
                gBmsSampleRes[channal_switch].res_p1 = 65535;
            }
            if(gBmsSampleRes[channal_switch].res_n1 <= -100 || gBmsSampleRes[channal_switch].res_n1 > 10000)
            {
                gBmsSampleRes[channal_switch].res_n1 = 65535;
            }

//            if(fabs(gBmsSampleRes[channal_switch].res_p1) < 100) //<100k默认为0了
//            {
//                gBmsSampleRes[channal_switch].res_p1 = 0;
//            }
//            if(fabs(gBmsSampleRes[channal_switch].res_n1) < 100)
//            {
//                gBmsSampleRes[channal_switch].res_n1 = 0;
//            }

            //取最小值
            gBmsSampleRes[channal_switch].res_main = gBmsSampleRes[channal_switch].res_p1 >= gBmsSampleRes[channal_switch].res_n1 ? gBmsSampleRes[channal_switch].res_n1 : gBmsSampleRes[channal_switch].res_p1;

            gBmsSampleRes[channal_switch].sample_over = true;
            gBmsSampleRes[channal_switch].sample_en = false;
            SampleResState = RES_START;
            break;
        }


        case RES_SAMPLE_ERR:
        {
            gBmsSampleRes[channal_switch].error_cnt++;
            SampleResState = RES_KP_ON;
            break;
        }
        default :
            break;
    }

}




