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
#include "bsp.h"
#include "ht7017.h"
#include "string.h"

#define RES_SWITCH_DELAY    3000        //µ¥Î»1ms     5000

BmsSampleResType gBmsSampleRes = {0};
const BmsSampleResType *const IR_BmsSampleRes = &gBmsSampleRes;
EN_SAMPLE_RES SampleResState = RES_INIT;

static int Ht7017_Init()
{
    int ret = 0;
    static uint8_t state = 0;
    static uint8_t cmd[10] = {0};
    uint8_t sum = 0;
    static uint8_t sum_err = 0;
    static uint32_t com_time = 0;
    
    switch(state)
    {
        case 0:
        {
            Ht7017UartSend(cmd,HT_Write(HT_WPCFG,WRITE_EN_2,cmd));
            state++;
            com_time = tim_get_systime();
            break;
        }
        case 1:
        {
            if(Ht7017RxTxState.ReceiveFlg)
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
            if(tim_get_systime() - com_time >= HT_COM_TIMEOUT )
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
            Ht7017UartSend(cmd,HT_Write(HT_ANAEN,0x07,cmd));
            state++;
            com_time = tim_get_systime();
            break;
        }
        case 3:
        {
            if(Ht7017RxTxState.ReceiveFlg)
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
        case 4:
        {
            Ht7017UartSend(cmd,HT_Read(HT_ANAEN,cmd));
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
                    if(0x07 == (uint32_t)(cmd[2]<<16 | cmd[3]<<8 | cmd[4]))
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
            Ht7017UartSend(cmd,HT_Write(HT_ModuleEn,ModuleEn_Data,cmd));
            state++;
            com_time = tim_get_systime();
            break;
        }
        case 7:
        {
            if(Ht7017RxTxState.ReceiveFlg)
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
        case 8:
        {
            Ht7017UartSend(cmd,HT_Read(HT_ModuleEn,cmd));
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
                    if(ModuleEn_Data == (uint32_t)(cmd[2]<<16 | cmd[3]<<8 | cmd[4]))
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
        default : break;
    }
    return ret;
}

static int SampleVoltage(uint8_t reg,int16_t *data)
{
    int ret = 0;
    static uint8_t state = 0;
    static uint8_t cmd[10] = {0};
    uint8_t sum = 0;
    static uint8_t sum_err = 0;
    static uint32_t com_time = 0;
    uint32_t temp_u32 = 0;
    
    switch(state)
    {
        case 0:
        {
            HT_Read(reg,cmd);
            Ht7017UartSend(cmd,2);
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
                    temp_u32 = cmd[2]<<16 | cmd[3]<<8 | cmd[4];
//                    if(reg == HT_RMS_I1)
//                    {
//                        gBmsSampleRes.volt_ch1  = temp_u32;
//                    }
//                    else if(reg == HT_RMS_I2)
//                    {
//                        gBmsSampleRes.volt_ch2  = temp_u32;
//                    }
                    
                    {
                        *data = temp_u32 * 0.000108 - 2.7835;
                    }
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
                temp_u32 = tim_get_systime() - com_time;
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
        default : break;
    }
    return ret;
}

void SampleResStart()
{
    gBmsSampleRes.sample_en = true;
}
void SampleResStop()
{
    gBmsSampleRes.sample_en = false;
    SampleResState = RES_IDLE;
    RES_KG_SET(false);
    RES_KP_SET(false);
    RES_KN_SET(false);
}
void SampleRes_Init()
{
    Ht7017_Init();
}
void SampleRes_Main()
{
    static uint32_t sample_res_time = 0;
    int8_t temp = 0;
    int32_t temp32 = 0;
    
    switch((uint8_t)SampleResState)
    {
        
        case RES_INIT:
        {
            gBmsSampleRes.res_main = 0xFFFF;
            if(Ht7017_Init() == 1)
            {
                SampleResState = RES_IDLE;
            }
            break;
        }
        case RES_IDLE:
        {
            if(gBmsSampleRes.sample_en)
            {
                SampleResState = RES_KP_ON;
            }
            break;
        }
        case RES_KP_ON:
        {
//            GpioBitWrite((&DO_Handle[RES_KG]),SET);
//            GpioBitWrite((&DO_Handle[RES_KP]),SET);
            RES_KG_SET(SET);
            RES_KP_SET(SET);
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
            temp = SampleVoltage(HT_RMS_I2,&gBmsSampleRes.volt_kp1);
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
            temp = SampleVoltage(HT_RMS_I1,&gBmsSampleRes.volt_kn1);
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
//            GpioBitWrite((&DO_Handle[RES_KP]),RESET);
//            GpioBitWrite((&DO_Handle[RES_KN]),SET);
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
            temp = SampleVoltage(HT_RMS_I2,&gBmsSampleRes.volt_kp2);
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
            temp = SampleVoltage(HT_RMS_I1,&gBmsSampleRes.volt_kn2);
            if(temp == 1)
            {
                SampleResState = RES_COMPUTE;
            }
            else if(temp == -1)
            {
                SampleResState = RES_SAMPLE_ERR;
            }
            break;
        }
        case RES_COMPUTE:
        {
//            GpioBitWrite((&DO_Handle[RES_KG]),RESET);
//            GpioBitWrite((&DO_Handle[RES_KN]),RESET);
//            RES_KG_SET(RESET);
            RES_KN_SET(RESET);
            gBmsSampleRes.volt_kn1 = -gBmsSampleRes.volt_kn1;
            gBmsSampleRes.volt_kn2 = -gBmsSampleRes.volt_kn2;

//            gBmsSampleRes.volt_kp1 = 164;
//            gBmsSampleRes.volt_kn1 = -331;
//            gBmsSampleRes.volt_kp2 = 383;
//            gBmsSampleRes.volt_kn2 = -112;
            
            
            gBmsSampleRes.res_total_voltage = (gBmsSampleRes.volt_kp1 - gBmsSampleRes.volt_kn1)*(R3*R10/(R3+R10) + R1)/(R3*R10/(R3+R10)) /100;
            temp32 = R5/((R6/(R5+R6)*(gBmsSampleRes.volt_kp2-gBmsSampleRes.volt_kn2)*gBmsSampleRes.volt_kn1/       \
                (gBmsSampleRes.volt_kp2*gBmsSampleRes.volt_kn1-gBmsSampleRes.volt_kn2*gBmsSampleRes.volt_kp1)) - R5/(R3*R9/(R3+R9)+R1)-1);
//            temp32 = R5/((R6/(R5+R6)*((double)gBmsSampleRes.volt_kp2-gBmsSampleRes.volt_kn2)*gBmsSampleRes.volt_kn1/       \
//                ((double)gBmsSampleRes.volt_kp2*gBmsSampleRes.volt_kn1-gBmsSampleRes.volt_kn2*gBmsSampleRes.volt_kp1)) - R5/(R3*R9/((double)R3+R9)+R1)-1);

            gBmsSampleRes.res_p = ((temp32 < 0) ? (-temp32) : temp32);
           temp32 = (R5+R6)/((R6/R5*(gBmsSampleRes.volt_kp2-gBmsSampleRes.volt_kn2)*gBmsSampleRes.volt_kp1 /      \
                (gBmsSampleRes.volt_kn2*gBmsSampleRes.volt_kp1-gBmsSampleRes.volt_kp2*gBmsSampleRes.volt_kn1))-(R5+R6)/(R3*R10/(R3+R10)+R1)-1);
            gBmsSampleRes.res_n = ((temp32 < 0) ? (-temp32) : temp32);
            //            if(gBmsSampleRes.sample_over)
//            {
////                if(gBmsSampleRes.res_main > gBmsSampleRes.res_p)
//                {
//                    gBmsSampleRes.res_main = gBmsSampleRes.res_p;
//                }
//                if(gBmsSampleRes.res_main > gBmsSampleRes.res_n)
//                {
//                    gBmsSampleRes.res_main = gBmsSampleRes.res_n;
//                }
//            }
            if((gBmsSampleRes.res_p == 0)&&(gBmsSampleRes.res_n == 0))
            {
                gBmsSampleRes.res_p = 0xFFFF;
                gBmsSampleRes.res_n = 0xffff;
                gBmsSampleRes.res_main = 0xffff;
                gBmsSampleRes.error_cnt++;
            }
            else
            {
                gBmsSampleRes.res_main = ((gBmsSampleRes.res_p < gBmsSampleRes.res_n) ? gBmsSampleRes.res_p : gBmsSampleRes.res_n);
            
                gBmsSampleRes.valid_cnt++;
                gBmsSampleRes.sample_over = true;

            }
            SampleResState = RES_KP_ON;

            break;
        }
        case RES_SAMPLE_ERR:
        {
            gBmsSampleRes.error_cnt++;
            SampleResState = RES_KP_ON;
            break;
        }
        default : break;
    }
}


