/*
 * authentication_fun.c
 *
 *  Created on: 2025年3月21日
 *      Author: Nova006352
 */



/*
 * authentication_function.c
 *
 *  Created on: 2024年9月30日
 *      Author: Nova006352
 */

#define QV_REGIONDEFAULT_VALUEV_NUM        (5)
#define PV_DSG_REGIONDEFAULT_VALUEV_NUM    (2)
#define PV_CHG_REGIONDEFAULT_VALUEV_NUM    (2)
#define PF_REGIONDEFAULT_VALUEF_NUM        (4)

#include "authentication_func.h"


//InverterPwr_t _gInvPwr;
//
//InvReactiveCoefficient_t gstInvReactiveCoefficient;
//ApprovalStdInfo_t gApprovalStdInfo;
//EN_NATIONAL_STANDARDS e_national_standards = EN_AUSTRALIA_A;
//
//EN_REATIVE_PWR_CTRL_MODE e_reative_pwr_mode = EN_POWER_FACTOR_MODE;
///*QV模式不同国家标准的电压区间段*/
//const uint16_t garrQvRegiondefaultValueV[EN_NATIONAL_STANDARDS_NUM][QV_REGIONDEFAULT_VALUEV_NUM] =
//{
//    {0, 207, 220, 240, 258},
//    {0, 205, 220, 235, 255},
//    {0, 215, 230, 240, 255},
//    {0, 207, 220, 235, 244},
//};
//
///*QV模式不同国家标准的k值*/
//const float garrQvCoefficientK[EN_NATIONAL_STANDARDS_NUM][QV_REGIONDEFAULT_VALUEV_NUM] =
//{
//    {0, -0.033846, 0, -0.033333, 0},//第3原为1
//    {0, -0.02,     0, -0.02,     0},
//    {0, -0.029333, 0, -0.04,     0},
//    {0, -0.046154, 0, -0.066667, 0},
//};
//
///*QV模式不同国家标准的b值*/
//const float garrQvCoefficientB[EN_NATIONAL_STANDARDS_NUM][QV_REGIONDEFAULT_VALUEV_NUM] =
//{
//    {0.44, 7.446124, 0, 8, -0.60},
//    {0.30, 4.4,      0, 4.7, -0.40},
//    {0.44, 6.746667, 0, 9.6, -0.60},
//    {0.60, 10.153846,0, 15.666667, -0.60},
//};
//
///*PV模式下不同国家标准下、放电情况下、电压判断区间段*/
//const uint16_t garrPvDsgRegiondefaultValueV[EN_NATIONAL_STANDARDS_NUM][PV_DSG_REGIONDEFAULT_VALUEV_NUM] =
//{
//    {253, 260},
//    {250, 260},
//    {253, 260},
//    {242, 250},
//};
//
///*PV模式下不同国家标准下、充电情况下、电压判断区间段*/
//const uint16_t garrPvChgRegiondefaultValueV[EN_NATIONAL_STANDARDS_NUM][PV_CHG_REGIONDEFAULT_VALUEV_NUM] =
//{
//    {207, 215},
//    {195, 215},
//    {207, 215},
//    {216, 224},
//};
//
///*PF模式下不同国家标准下、频率判断区间段，待赋值*/
//float garrPfRegionValueF[EN_NATIONAL_STANDARDS_NUM][PF_REGIONDEFAULT_VALUEF_NUM] =
//{
//    {47, 49.75, 50.25, 52},
//    {47, 49.85, 50.15, 52},
//    {45, 49.5, 50.5, 55},
//    {45, 49.8, 50.2, 55},
//};
//
///*PF模式下不同国家标准下、默认频率判断区间段（选择降频/升频曲线）*/
//const float garrPfRegionDefaultValueF[EN_NATIONAL_STANDARDS_NUM][PF_REGIONDEFAULT_VALUEF_NUM] =
//{
//    {47, 49.75, 50.25, 52},
//    {47, 49.85, 50.15, 52},
//    {45, 49.5, 50.5, 55},
//    {45, 49.8, 50.2, 55},
//};
//
///*PF模式下不同国家标准下、频率判断区间段（选择降频/升频曲线）+滞环*/
//const float garrPfRegionDelayValueF[EN_NATIONAL_STANDARDS_NUM][PF_REGIONDEFAULT_VALUEF_NUM] =
//{
//    {47, 49.85, 50.15, 52},
//    {47, 49.95, 50.05, 52},
//    {45, 49.55, 50.45, 55},
//    {45, 49.9, 50.1, 55},
//};
//
///*PF模式下相对应的参数*/
//const float garrPfParaValue[EN_NATIONAL_STANDARDS_NUM][PF_REGIONDEFAULT_VALUEF_NUM] =
//{
//    {48, 49, 50.75, 52},
//    {48, 49, 50.75, 52},
//    {47, 48.25, 51.75, 53},
//    {48, 49, 51, 52},
//};
//
//static uint16_t VoltIntervalJudgment(EN_NATIONAL_STANDARDS e_national_standards,uint16_t dsg_chg_mode,float grid_volt);
//static float PvModePwrCtrlPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_volt,float inv_act_pwr,float apparent_pwr);
//static float QvModePwrCtrlPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_volt,float apparent_pwr);
//static float PfModeUnderFreqPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_freq,float inv_act_pwr,float apparent_pwr);
//
///**
// * @brief
// * @param 1.国家标准;2.充放电模式;3.实际采集电压;
// * @param PV电压区间判断函数
// * @return 电压区间段
// */
//uint16_t VoltIntervalJudgment(EN_NATIONAL_STANDARDS e_national_standards,uint16_t dsg_chg_mode,float grid_volt)
//{
//    uint16_t i = 0;
//    if(dsg_chg_mode == 0)//充电
//    {
//        for(i = 0;i < PV_CHG_REGIONDEFAULT_VALUEV_NUM; i++)
//        {
//            if(grid_volt < garrPvChgRegiondefaultValueV[e_national_standards][i])
//            {
//                break;
//            }
//        }
//    }
//    if(dsg_chg_mode == 1)//放电
//    {
//        for(i = 0;i < PV_DSG_REGIONDEFAULT_VALUEV_NUM; i++)
//        {
//            if(grid_volt < garrPvDsgRegiondefaultValueV[e_national_standards][i])
//            {
//                break;
//            }
//        }
//    }
//    return i ;//确认i还是i-1
//}
//
///**
// * @brief PV功率模式控制
// * @param 1.国家标准;2.实际采集电压;3.逆变输入功率;4.视在功率;
// * @return 最大允许充放电有功功率
// */
//float PvModePwrCtrlPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_volt,float inv_act_pwr,float apparent_pwr)
//{
//    float max_chg_pwr = apparent_pwr;
//    float max_dsg_pwr = -1 * apparent_pwr;
//    float coe = 0;
//    float max_active_pwr = 0;
//    uint16_t dsg_chg_mode = 0;
//    uint16_t volt_range = 0;
//
//    if(inv_act_pwr >= 0)
//    {
//        dsg_chg_mode = 0;//充电
//    }
//    else if(inv_act_pwr < 0)
//    {
//        dsg_chg_mode = 1;//放电
//    }
//
//    volt_range = VoltIntervalJudgment(e_national_standards,dsg_chg_mode,grid_volt);//电压区间判断
//
//    if(dsg_chg_mode == 0)//充电
//    {
//        switch(volt_range)
//        {
//            case EN_VOLT_SECTION_1 :
//            {
//                coe = 0.2;
//                break;
//            }
//            case EN_VOLT_SECTION_2 :
//            {
//                coe = (grid_volt - garrPvChgRegiondefaultValueV[e_national_standards][0]) * 0.8/
//                (garrPvChgRegiondefaultValueV[e_national_standards][1] - garrPvChgRegiondefaultValueV[e_national_standards][0]) + 0.2;
//                break;
//            }
//            case EN_VOLT_SECTION_3 :
//            {
//                coe = 1;
//                break;
//            }
//            default:
//                break;
//        }
//        max_active_pwr = max_chg_pwr * coe;
//    }
//    if(dsg_chg_mode == 1)//卖电
//    {
//        switch(volt_range)
//        {
//            case EN_VOLT_SECTION_1 :
//            {
//                coe = 1;
//                break;
//            }
//            case EN_VOLT_SECTION_2 :
//            {
//                coe = (garrPvDsgRegiondefaultValueV[e_national_standards][1] - grid_volt) * 0.8/
//                (garrPvDsgRegiondefaultValueV[e_national_standards][1] - garrPvDsgRegiondefaultValueV[e_national_standards][0]) + 0.2;
//                break;
//            }
//            case EN_VOLT_SECTION_3 :
//            {
//                coe = 0.2;
//                break;
//            }
//            default:
//                break;
//        }
//        max_active_pwr = max_dsg_pwr * coe;
//    }
//    return max_active_pwr;
//}
//
///**
// * @brief QV功率模式控制
// * @param 1.国家标准;2.实际电网电压;3.最大视在功率;
// * @return 无功功率
// */
//float QvModePwrCtrlPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_volt,float apparent_pwr)//搞清输出有功、无功
//{
//    uint16_t i = 0;
//    float coe = 0;
//    float reactive_pwr = 0;
//    for(i = 0; i < QV_REGIONDEFAULT_VALUEV_NUM; i++)
//    {
//        if(grid_volt < garrQvRegiondefaultValueV[e_national_standards][i])
//        {
//            break;
//        }
//    }
//    coe = garrQvCoefficientK[e_national_standards][i-1] * grid_volt + garrQvCoefficientB[e_national_standards][i-1];
//    reactive_pwr = apparent_pwr * coe;//视在功率怎么给？
//    return reactive_pwr;
//}
//
//
///**
// * @brief PF功率模式控制
// * @param 1.国家标准;2.实际电网电压;3.逆变输入有功功率;4.最大视在功率;
// * @return 有功功率
// */
//float PfModeUnderFreqPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_freq,float inv_act_pwr,float apparent_pwr)
//{
//    static float coe = 0; //PF升降频系数
//    static float latch_value = 0;//扰动前功率锁存值
//    static float last_freq = 0;//锁存上一次频率值
//    static float active_pwr = 0; //返回有功功率
//    float max_dsg_pwr = -1 * apparent_pwr;//允许最大放电功率
//    float max_chg_pwr = apparent_pwr;//允许最大充电功率
//    uint16_t i = 0;
//    for(i = 0; i < PF_REGIONDEFAULT_VALUEF_NUM; i++)//频率区间判断
//    {
//         if(grid_freq <= garrPfRegionValueF[e_national_standards][i])
//         {
//             if(grid_freq <= garrPfRegionValueF[e_national_standards][1])
//             {
//                 garrPfRegionValueF[e_national_standards][1] = garrPfRegionDelayValueF[e_national_standards][1];//fLLCO + 0.1/0.1/0.05/0.1 滞环频率
//             }
//             if(grid_freq > garrPfRegionValueF[e_national_standards][2])
//             {
//                 garrPfRegionValueF[e_national_standards][2] = garrPfRegionDelayValueF[e_national_standards][2];//fULCO + 0.1/0.1/0.05/0.1 滞环频率
//             }
//             if(grid_freq > garrPfRegionValueF[e_national_standards][1])
//             {
//                 garrPfRegionValueF[e_national_standards][1] = garrPfRegionDefaultValueF[e_national_standards][1];
//             }
//             if(grid_freq <= garrPfRegionValueF[e_national_standards][2])
//             {
//                 garrPfRegionValueF[e_national_standards][2] = garrPfRegionDefaultValueF[e_national_standards][2];
//             }
//             break;
//         }
//    }
//    if((grid_freq > garrPfRegionValueF[e_national_standards][1]) && (grid_freq < garrPfRegionValueF[e_national_standards][2]))
//    {
//        latch_value = inv_act_pwr;
//    }
//    switch(i)
//    {
//        case EN_UNDER_FREQ_SECTION://欠频区间
//        {
//            coe = 0;
//            active_pwr = latch_value * coe;
//            break;
//        }
//        case EN_LOWER_FREQ_SECTION://降频区间
//        {
//            if(grid_freq >= garrPfParaValue[e_national_standards][1])
//            {
//                if(inv_act_pwr <= 0)//卖电
//                {
//                    active_pwr = latch_value;
//                }
//                else//充电
//                {
//                    if(grid_freq <= last_freq)//在升频区间发生频率回退（降频），则保持系数不变；当继续升频，重新计算系数
//                    {
//                        coe = (garrPfRegionDefaultValueF[e_national_standards][1] - grid_freq)/(garrPfRegionDefaultValueF[e_national_standards][1] - garrPfParaValue[e_national_standards][1]);
//                        last_freq = grid_freq;//更新锁存值
//                        active_pwr = latch_value * (1 - coe);
//                    }
//
//                }
//            }
//            else
//            {
//                if(grid_freq <= last_freq)//在降频区间发生频率回退（升频），则保持系数不变；当继续降频，重新计算系数
//                {
//                    coe = (garrPfParaValue[e_national_standards][1] - grid_freq)/(garrPfParaValue[e_national_standards][1] - garrPfParaValue[e_national_standards][0]);
//                    last_freq = grid_freq;//更新锁存值
//                }
//                if(inv_act_pwr >= 0)//电网往下拉/买电
//                {
//                    active_pwr = max_dsg_pwr * coe;
//                }
//                else//卖电
//                {
//                    active_pwr = latch_value + (max_dsg_pwr - latch_value) * coe;
//                }
//            }
//            break;
//        }
//        case EN_NORMAL_FREQ_SECTION ://正常区间
//        {
//            coe = 1;
//            last_freq = grid_freq;
//            active_pwr = latch_value * coe;
//            break;
//        }
//        case EN_UPPER_FREQ_SECTION ://升频区间
//        {
//            if(grid_freq <= garrPfParaValue[e_national_standards][2])
//            {
//                if(inv_act_pwr >= 0)//买电
//                {
//                    active_pwr = latch_value;
//                }
//                else//放电
//                {
//                    if(grid_freq >= last_freq)//在升频区间发生频率回退（降频），则保持系数不变；当继续升频，重新计算系数
//                    {
//                        coe = (grid_freq - garrPfRegionDefaultValueF[e_national_standards][2])/(garrPfParaValue[e_national_standards][2] - garrPfRegionDefaultValueF[e_national_standards][2]);
//                        last_freq = grid_freq;//更新锁存值
//                        active_pwr = latch_value * (1 - coe);
//                    }
//
//                }
//            }
//            else
//            {
//                if(grid_freq >= last_freq)//在升频区间发生频率回退（降频），则保持系数不变；当继续升频，重新计算系数
//                {
//                    coe = (grid_freq - garrPfParaValue[e_national_standards][2])/(garrPfParaValue[e_national_standards][3] - garrPfParaValue[e_national_standards][2]);
//                    last_freq = grid_freq;//更新锁存值
//                }
//                if(inv_act_pwr >= 0)//电网往下拉/买电
//                {
//                    active_pwr = latch_value + (max_chg_pwr - latch_value) * coe;
//                }
//                else//卖电
//                {
//                    active_pwr = max_chg_pwr * coe;
//                }
//            }
//            break;
//        }
//        case EN_OVER_FREQ_SECTION ://过频区间
//        {
//            coe = 0;
//            active_pwr = latch_value * coe;
//            break;
//        }
//        default:
//            break;
//    }
//    if(active_pwr < -5000)
//    {
//        active_pwr = -5000;
//    }
//    if(active_pwr > 5000)
//    {
//        active_pwr = 5000;
//    }
//    return active_pwr;
//
//}
//
//
///**
// * @brief 功率模式选择
// * @param 1.国家标准;2.逆变输入有功功率;3.输出逆变有功功率;4.输出逆变无功功率;
// */
///**/
//void PowerCtrlModeHandle(EN_NATIONAL_STANDARDS e_national_standards,float inv_active_pwr,InverterPwr_t* InvPwr)
//{
//    float pf_active_pwr = 0;
//    float pv_max_active_pwr = 0;
//    float qv_reactive_pwr = 0;
//    float reactive_factor = 0;
//    float allow_active_pwr = 0;
//    float actual_active_pwr = 0;
//    float allow_reactive_pwr = 0;
//    float apparent_pwr = 5000;//视在功率计算方式，额定逆变功率
//    float grid_freq = _gState_algo.grid_freq;//频率
//    float grid_volt = _gState_algo.grid_vol_rms;//电压
//
////    float grid_volt = (float)gSysSetInfo.uv_derating_enable;
//////    float grid_freq = (float)gSysSetInfo.uf_derating_enable;
////    float grid_freq = gSysSetInfo.freq_given;
//    uint16_t index = 0;
//
//    /*pf有功功率计算*/
//    // pf_active_pwr = PfModeUnderFreqPrcess(e_national_standards,grid_freq,inv_active_pwr,apparent_pwr);
//    pf_active_pwr = FrequencyResponse(grid_freq,inv_active_pwr,gSysSetInfo.pf_switch); //替换为欧标认证标准
//
//    /*pV最大有功功率计算*/
//    pv_max_active_pwr = PUCurveResponse(grid_volt);
//
//    ReactivepwrMode();//无功模式选择
//    /*无功功率计算*/
//    switch(e_reative_pwr_mode)
//    {
//        case EN_POWER_FACTOR_MODE://功因
//        {
//            float apparent_pwr_factor = 0;
//            reactive_factor = (float)gSysSetInfo.power_factor_setting_value / 100;//有正负号,且下发值需要缩小100倍
//            apparent_pwr_factor = inv_active_pwr / reactive_factor;//求出视在功率,不考虑正负
//            if(fabs(apparent_pwr_factor) > 5000)
//            {
//                apparent_pwr_factor = 5000;//不考虑视在正负
//            }
//            qv_reactive_pwr = sqrtf(apparent_pwr_factor * apparent_pwr_factor - inv_active_pwr * inv_active_pwr);//求出无功
//            qv_reactive_pwr = reactive_factor >= 0 ? qv_reactive_pwr : (-1 * qv_reactive_pwr);//根据功因正负判断无功功率正负
//            break;
//        }
//        case EN_REATIVE_PWR_MODE://无功功率控制,直接给定无功功率
//        {
//            qv_reactive_pwr = (float)gSysSetInfo.reactive_power_setting_value;
//            break;
//        }
//        case EN_QV_PWR_MODE://QV模式
//        {
////            qv_reactive_pwr = QvModePwrCtrlPrcess(e_national_standards,grid_volt,apparent_pwr);
//            qv_reactive_pwr = EUQvModePwrCtrlPrcess(grid_volt, inv_active_pwr);
//            break;
//        }
//        case EN_QP_PWR_MODE:
//        {
//            qv_reactive_pwr = QpCurveResponse(inv_active_pwr, grid_volt, 1);
//            //QP模式暂不考虑
//            break;
//        }
//        default:
//            break;
//    }
//
//    /*功率模式选择控制*/
//    index = gSysSetInfo.pf_switch << 2 | gSysSetInfo.pv_switch << 1 | 1;
//    switch(index)
//    {
//        case 0://不存在
//        {
//            break;
//        }
//        case 1://只有QV（也分几种：1.功因；2.无功控制；3.QV控制；4.QP模式）优先无功
//        {
//
//            allow_active_pwr = sqrtf(apparent_pwr * apparent_pwr - qv_reactive_pwr * qv_reactive_pwr);//有功限值
//            if(fabs(inv_active_pwr) > allow_active_pwr)
//            {
//                InvPwr->inv_output_act_pwr = inv_active_pwr >= 0 ? allow_active_pwr : (-1 * allow_active_pwr);
//            }
//            else
//            {
//                InvPwr->inv_output_act_pwr = inv_active_pwr;
//            }
//            InvPwr->inv_output_reactive_pwr = qv_reactive_pwr;
//            break;
//        }
//        case 2://PV模式
//        {
//            //无功模式一直在
//            break;
//        }
//        case 3://PV、QV模式,优先无功
//        {
//
//            allow_active_pwr = sqrtf(apparent_pwr * apparent_pwr - qv_reactive_pwr * qv_reactive_pwr);
//            if(pv_max_active_pwr <= inv_active_pwr)
//            {
//                actual_active_pwr = inv_active_pwr;
//            }
//            else
//            {
//                actual_active_pwr = pv_max_active_pwr;
//            }
//
//            if(fabs(actual_active_pwr) > fabs(allow_active_pwr))
//            {
//                InvPwr->inv_output_act_pwr = actual_active_pwr >= 0 ? allow_active_pwr : (-1 * allow_active_pwr);
//            }
//            else
//            {
//                InvPwr->inv_output_act_pwr = actual_active_pwr;
//            }
//
//            InvPwr->inv_output_reactive_pwr = qv_reactive_pwr;
//            break;
//        }
//        case 4://PF模式
//        {
//            //无功模式一直在
//            break;
//        }
//        case 5://PF、QV模式，优先有功
//        {
//            allow_reactive_pwr = sqrtf(apparent_pwr * apparent_pwr - pf_active_pwr * pf_active_pwr);
//
//            if(fabs(qv_reactive_pwr) > allow_reactive_pwr)
//            {
//                InvPwr->inv_output_reactive_pwr = qv_reactive_pwr >= 0 ? allow_reactive_pwr : (-1 * allow_reactive_pwr);
//            }
//            else
//            {
//                InvPwr->inv_output_reactive_pwr = qv_reactive_pwr;
//            }
//
//            InvPwr->inv_output_act_pwr = pf_active_pwr;
//            break;
//        }
//        case 6://PF、PV模式
//        {
//            //无功模式一直在
//            break;
//        }
//        case 7://全部存在（QV、PV、PF）
//        {
//            allow_reactive_pwr = sqrtf(apparent_pwr * apparent_pwr - pf_active_pwr * pf_active_pwr);
//
//            if(fabs(qv_reactive_pwr) > allow_reactive_pwr)
//            {
//                InvPwr->inv_output_reactive_pwr = qv_reactive_pwr >= 0 ? allow_reactive_pwr : (-1 * allow_reactive_pwr);
//            }
//            else
//            {
//                InvPwr->inv_output_reactive_pwr = qv_reactive_pwr;
//            }
//
//            InvPwr->inv_output_act_pwr = pf_active_pwr;
//            break;
//        }
//        default:
//            break;
//    }
//}
//
//
//void ApprovalStdInfoGet(void)
//{
//    if(gSysSetInfo.grid_standards == 0x02)
//    {
//        e_national_standards = EN_AUSTRALIA_A;
//    }
//    else if(gSysSetInfo.grid_standards == 0x03)
//    {
//        e_national_standards = EN_AUSTRALIA_B;
//    }
//    else if(gSysSetInfo.grid_standards == 0x04)
//    {
//        e_national_standards = EN_AUSTRALIA_C;
//    }
//    else if(gSysSetInfo.grid_standards == 0x05)
//    {
//        e_national_standards = EN_NEW_ZEALAND;
//    }
//    else
//    {
//        e_national_standards = EN_AUSTRALIA_A;
//    }
//    switch(e_national_standards)
//    {
//        case EN_AUSTRALIA_A:
//        {
//            gApprovalStdInfo.over_vol_level1 = 275;
//            gApprovalStdInfo.over_vol_level2 = 265;
//            gApprovalStdInfo.under_vol_level1 = 180;
//            gApprovalStdInfo.under_vol_level2 = 70;
//            gApprovalStdInfo.vol_nom_max = 258;
//            gApprovalStdInfo.over_vol_stop_out_activepwr = 260;
//            gApprovalStdInfo.under_vol_stop_out_activepwr = 180;
//            gApprovalStdInfo.recover_over_vol_stop_out_activepwr = 253;
//            gApprovalStdInfo.recover_under_vol_stop_out_activepwr = 205;
//            gApprovalStdInfo.over_freq = 52;
//            gApprovalStdInfo.under_freq = 47;
//            gApprovalStdInfo.recover_over_freq = 50.15;
//            gApprovalStdInfo.recover_under_freq = 47.7;
//            gApprovalStdInfo.pf_low_freq = 49.75;
//            gApprovalStdInfo.pf_high_freq = 50.25;
//            gApprovalStdInfo.pf_stop_under_freq = 47;
//            gApprovalStdInfo.pf_stop_upper_freq = 52;
//            gApprovalStdInfo.pf_freq_pwr_max = 48;
//            gApprovalStdInfo.pf_stop_chg_freq = 49;
//            gApprovalStdInfo.pf_stop_dsg_freq = 50.75;
//            gApprovalStdInfo.pf_freq_hyst = 0.1;
//            gApprovalStdInfo.qv_ful_reac_out_Volth = 207;
//            gApprovalStdInfo.qv_start_out_reac_Volth = 205;
//            gApprovalStdInfo.qv_start_abs_reac_Volth = 215;
//            gApprovalStdInfo.qv_ful_reac_abs_Volth = 207;
//            gApprovalStdInfo.qv_max_prov_Preac = 0.44;
//            gApprovalStdInfo.qv_max_abs_Preac = 0.6;
//            gApprovalStdInfo.pv_dsg_start_derate_Volth = 253;
//            gApprovalStdInfo.pv_dsg_min_out_Volth = 260;
//            gApprovalStdInfo.pv_chg_start_derate_Volth = 207;
//            gApprovalStdInfo.pv_chg_min_out_Volth = 215;
//            gApprovalStdInfo.pv_chg_Pmin = 0.2;
//            break;
//        }
//        case EN_AUSTRALIA_B:
//        {
//            gApprovalStdInfo.over_vol_level1 = 275;
//            gApprovalStdInfo.over_vol_level2 = 265;
//            gApprovalStdInfo.under_vol_level1 = 180;
//            gApprovalStdInfo.under_vol_level2 = 70;
//            gApprovalStdInfo.vol_nom_max = 258;
//            gApprovalStdInfo.over_vol_stop_out_activepwr = 260;
//            gApprovalStdInfo.under_vol_stop_out_activepwr = 180;
//            gApprovalStdInfo.recover_over_vol_stop_out_activepwr = 253;
//            gApprovalStdInfo.recover_under_vol_stop_out_activepwr = 205;
//            gApprovalStdInfo.over_freq = 52;
//            gApprovalStdInfo.under_freq = 47;
//            gApprovalStdInfo.recover_over_freq = 50.15;
//            gApprovalStdInfo.recover_under_freq = 47.7;
//            gApprovalStdInfo.pf_low_freq = 49.85;
//            gApprovalStdInfo.pf_high_freq = 50.15;
//            gApprovalStdInfo.pf_stop_under_freq = 47;
//            gApprovalStdInfo.pf_stop_upper_freq = 52;
//            gApprovalStdInfo.pf_freq_pwr_max = 48;
//            gApprovalStdInfo.pf_stop_chg_freq = 49;
//            gApprovalStdInfo.pf_stop_dsg_freq = 50.75;
//            gApprovalStdInfo.pf_freq_hyst = 0.1;
//            gApprovalStdInfo.qv_ful_reac_out_Volth = 220;
//            gApprovalStdInfo.qv_start_out_reac_Volth = 220;
//            gApprovalStdInfo.qv_start_abs_reac_Volth = 230;
//            gApprovalStdInfo.qv_ful_reac_abs_Volth = 220;
//            gApprovalStdInfo.qv_max_prov_Preac = 0.3;
//            gApprovalStdInfo.qv_max_abs_Preac = 0.4;
//            gApprovalStdInfo.pv_dsg_start_derate_Volth = 250;
//            gApprovalStdInfo.pv_dsg_min_out_Volth = 260;
//            gApprovalStdInfo.pv_chg_start_derate_Volth = 195;
//            gApprovalStdInfo.pv_chg_min_out_Volth = 215;
//            gApprovalStdInfo.pv_chg_Pmin = 0;
//            break;
//        }
//        case EN_AUSTRALIA_C:
//        {
//            gApprovalStdInfo.over_vol_level1 = 275;
//            gApprovalStdInfo.over_vol_level2 = 265;
//            gApprovalStdInfo.under_vol_level1 = 180;
//            gApprovalStdInfo.under_vol_level2 = 70;
//            gApprovalStdInfo.vol_nom_max = 258;
//            gApprovalStdInfo.over_vol_stop_out_activepwr = 260;
//            gApprovalStdInfo.under_vol_stop_out_activepwr = 180;
//            gApprovalStdInfo.recover_over_vol_stop_out_activepwr = 253;
//            gApprovalStdInfo.recover_under_vol_stop_out_activepwr = 205;
//            gApprovalStdInfo.over_freq = 55;
//            gApprovalStdInfo.under_freq = 45;
//            gApprovalStdInfo.recover_over_freq = 50.15;
//            gApprovalStdInfo.recover_under_freq = 47.7;
//            gApprovalStdInfo.pf_low_freq = 49.5;
//            gApprovalStdInfo.pf_high_freq = 50.5;
//            gApprovalStdInfo.pf_stop_under_freq = 45;
//            gApprovalStdInfo.pf_stop_upper_freq = 55;
//            gApprovalStdInfo.pf_freq_pwr_max = 47;
//            gApprovalStdInfo.pf_stop_chg_freq = 48.25;
//            gApprovalStdInfo.pf_stop_dsg_freq = 51.75;
//            gApprovalStdInfo.pf_freq_hyst = 0.05;
//            gApprovalStdInfo.qv_ful_reac_out_Volth = 240;
//            gApprovalStdInfo.qv_start_out_reac_Volth = 235;
//            gApprovalStdInfo.qv_start_abs_reac_Volth = 240;
//            gApprovalStdInfo.qv_ful_reac_abs_Volth = 235;
//            gApprovalStdInfo.qv_max_prov_Preac = 0.44;
//            gApprovalStdInfo.qv_max_abs_Preac = 0.6;
//            gApprovalStdInfo.pv_dsg_start_derate_Volth = 253;
//            gApprovalStdInfo.pv_dsg_min_out_Volth = 260;
//            gApprovalStdInfo.pv_chg_start_derate_Volth = 207;
//            gApprovalStdInfo.pv_chg_min_out_Volth = 215;
//            gApprovalStdInfo.pv_chg_Pmin = 0.2;
//            break;
//        }
//        case EN_NEW_ZEALAND:
//        {
//            gApprovalStdInfo.over_vol_level1 = 275;
//            gApprovalStdInfo.over_vol_level2 = 265;
//            gApprovalStdInfo.under_vol_level1 = 180;
//            gApprovalStdInfo.under_vol_level2 = 70;
//            gApprovalStdInfo.vol_nom_max = 249;
//            gApprovalStdInfo.over_vol_stop_out_activepwr = 260;
//            gApprovalStdInfo.under_vol_stop_out_activepwr = 180;
//            gApprovalStdInfo.recover_over_vol_stop_out_activepwr = 253;
//            gApprovalStdInfo.recover_under_vol_stop_out_activepwr = 205;
//            gApprovalStdInfo.over_freq = 55;
//            gApprovalStdInfo.under_freq = 45;
//            gApprovalStdInfo.recover_over_freq = 50.15;
//            gApprovalStdInfo.recover_under_freq = 47.7;
//            gApprovalStdInfo.pf_low_freq = 49.8;
//            gApprovalStdInfo.pf_high_freq = 50.2;
//            gApprovalStdInfo.pf_stop_under_freq = 45;
//            gApprovalStdInfo.pf_stop_upper_freq = 55;
//            gApprovalStdInfo.pf_freq_pwr_max = 48;
//            gApprovalStdInfo.pf_stop_chg_freq = 49;
//            gApprovalStdInfo.pf_stop_dsg_freq = 51;
//            gApprovalStdInfo.pf_freq_hyst = 0.1;
//            gApprovalStdInfo.qv_ful_reac_out_Volth = 258;
//            gApprovalStdInfo.qv_start_out_reac_Volth = 255;
//            gApprovalStdInfo.qv_start_abs_reac_Volth = 255;
//            gApprovalStdInfo.qv_ful_reac_abs_Volth = 244;
//            gApprovalStdInfo.qv_max_prov_Preac = 0.6;
//            gApprovalStdInfo.qv_max_abs_Preac = 0.6;
//            gApprovalStdInfo.pv_dsg_start_derate_Volth = 242;
//            gApprovalStdInfo.pv_dsg_min_out_Volth = 216;
//            gApprovalStdInfo.pv_chg_min_out_Volth = 224;
//            gApprovalStdInfo.pv_chg_Pmin = 0.2;
//            break;
//        }
//        default:
//            break;
//    }
//}
//
//void ReactivepwrMode(void)
//{
//    if(gSysSetInfo.reactive_power_control_method == 0)
//    {
//        e_reative_pwr_mode = EN_POWER_FACTOR_MODE;
//    }
//    else if(gSysSetInfo.reactive_power_control_method == 1)
//    {
//        e_reative_pwr_mode = EN_REATIVE_PWR_MODE;
//    }
//    else if(gSysSetInfo.reactive_power_control_method == 2)
//    {
//        e_reative_pwr_mode = EN_QV_PWR_MODE;
//    }
//    else if(gSysSetInfo.reactive_power_control_method == 3)
//    {
//        e_reative_pwr_mode = EN_QP_PWR_MODE;
//    }
//}



