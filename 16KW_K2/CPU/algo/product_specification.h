
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        product_specification.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-1-22
  * @brief       用于产品性能参数定义
  ******************************************************************************
  * @attention 
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-3-19    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __PRODUCT_SPECIFICATION_H__
#define __PRODUCT_SPECIFICATION_H__

#ifdef __cplusplus
extern "C" {
#endif
//PV参数
#define PV_CUR_MAX 16.0f //PV的最大工作电流
#define PV_CUR_PROTECT 25.0f //PV触发关管电流
#define PV_MPPT_VOL_NORMAL 320.0f //PV额定MPPT电压
#define PV_MPPT_VOL_MIN 75.0f //PV最小运行MPPT电压
#define PV_MPPT_VOL_MAX 500.0f //PV最大运行MPPT电压
#define PV_OC_VOL_MAX 50.0f //PV最大开口电压
#define PVACTIVAT_BUS_THRESHOLD DC_BUS_VOL_NORMAL //PV激活Boost模式母线预充结束阈值
//母线电压范围
#define DC_BUS_VOL_MAX 500
#define DC_BUS_VOL_NORMAL 450  //额定直流母线电压
#define DC_BUS_VOL_MIN 400

#define TEST_MODE_DUTY_INIT (1.0f - PV_MPPT_VOL_NORMAL / DC_BUS_VOL_NORMAL) //支流源模式下的启动占空比，对应320V下的占空比
#define TEST_MODE_DC_VOLTAGE (PV_MPPT_VOL_NORMAL - 20.0f) //直流源模式下PV启动阈值300V，对应320V的电源

#define MUTICHANNAL_PV_MAX_POWER 24000 //多路PV功率最大值
#define SINGLE_PV_MAX_POWER 6300 //单路PV功率最大值
#define PV_CHANNAL 4 //PV通道数量
//逆变器参数定义
#define INV_PWR_NORMAL 16000.0f //逆变器额定功率
#define INV_CUR_MAX 120.0f
#define INV_VOL_MAX 390.0f
#define INV_RADIAN_FREQ 314.159265f //交流额定角频率
#define INV_VOL_NORMAL 311.0f //逆变器额定工作电压幅值
#define INV_Y_CAP 0.01 //导纳
//逆变器运行参数定影
#define INV_CONTROL_FREQ 20000.0f //逆变器控制频率
#define INV_CONTROL_TS 0.00005f //逆变器控制周期 = 1/INV_CONTROL_FREQ
#define INV_LOWEST_AC_FREQ 40.0f    //逆变器最低运行频率 
#define INV_HIGHEST_AC_FREQ 60.0f    //逆变器最高运行频率 
/***********************************/
//保护阈值设置
#define ERRO_INV_VOLT_OVER 400.0f   //逆变过压阈值
#define ERRO_INV_CUR_OVER 140.0f   //逆变过流阈值
#define ERRO_INV_BUS_OVER 550.0f   //母线电压过压阈值
#define WARN_INV_BUS_OVER 520.0f   //母线电压过压警告阈值
/***********************************/

//todo：逆变参数定义，后续需要更改
#define INV_POWER_NORMAL 16000.0f  //逆变器额定功率
#define INV_QMAX_NORMAL 9600.0f  //逆变器最大无功功率 :视在功率*0.6
//#define INV_VOL_NORMAL 230.0f   //逆变器额定电压
#define INV_CUR_NORMAL 23.7f   //逆变器额定电流
#define INV_OVER_LOAD_K 1.0f    //逆变器过载倍数
#define FREQ_NORMAL 50.0f      //逆变器额定频率
#define INV_REACTIVE_POWER_MAX_PU 0.6f  //额定最大无功功率

//功率变化率斜率范围
#define MIN_SLOPE 0.05f //最小变化率
#define MAX_SLOPE 36.0f  //最大变化率

//认证相关参数
#define AUS_APPROVE_EN  1
#define EU_APPROVE_EN   0




/***********************************/
//常用常量定义
#define PI                          (3.1415926f)
#define DOUBLE_PI                   (6.2831853f)
#define HALF_PI                     (1.5707963f)
/***********************************/

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif
