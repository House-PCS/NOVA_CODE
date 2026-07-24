/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        pll.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-4-10
  * @brief       PLL封装文件
  ******************************************************************************
  * @attention   暂未使用
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-4-10    <td>V1.0.0    <td>dengjiahui    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#include "F2837xD_device.h"
#include "Solar_F.h"
#include "pll.h"

#include "SPLL_1ph_F.h"

/**
 * @brief PLL初始化
 * @param pll 初始化的pll地址
*/
void PllInit(Pll_t* pll)
{
    /*SPLL_1ph_F_init(50, 0.00005, pll);
    SPLL_1ph_F_notch_coeff_update(0.00005, 200 * 3.1415, 0.00001, 0.1, pll);*/
    NOTCH_FLTR_F_VARS_init(&pll->notch_vars);
    NOTCH_FLTR_F_COEFF_Update(0.00005, 200 * 3.1415, 0.00001, 0.1, &pll->notch_coeff);
    CNTL_PI_F_init(&pll->cntl_pi);
    pll->cntl_pi.Kp = 50;
    pll->cntl_pi.Ki = 0.002;
    pll->cntl_pi.Umax = 62.8;
    pll->cntl_pi.Umin = -62.8;
   //pll->cntl_pi.Umax = 6.28;
   //pll->cntl_pi.Umin = -6.28;
    pll->cntl_pi.Ref = 0;
    pll->delta_wn = 0;
}

/**
 * @brief Pll运算程序
 * @param pll pll地址
 * @param ref 给定
 * @param fbk 反馈
 * @return 频率偏差
*/
float PllHandler(Pll_t* pll, float ref, float fbk)
{
    pll->ref = ref;
    pll->fbk = fbk;

    pll->notch_vars.In = ref * fbk;
    NOTCH_FLTR_F_run(&pll->notch_vars, &pll->notch_coeff);
    float phase_err = pll->notch_vars.Out;

    pll->cntl_pi.Fbk = phase_err;
    CNTL_PI_F_FUNC(&pll->cntl_pi);
    pll->delta_wn = pll->cntl_pi.Out;
    return pll->delta_wn;
}

float PllOffsetHandler(Pll_t* pll, float ref, float offset, uint8_t en_inject)
{
    static float deg = 0;
    pll->ref = ref;
    pll->fbk = -cosf(deg + offset);

    pll->notch_vars.In = ref * pll->fbk;
    NOTCH_FLTR_F_run(&pll->notch_vars, &pll->notch_coeff);
    float phase_err = pll->notch_vars.Out;

    pll->cntl_pi.Fbk = phase_err;
    CNTL_PI_F_FUNC(&pll->cntl_pi);
    pll->delta_wn = pll->cntl_pi.Out + en_inject * 3.14;
    deg += (pll->delta_wn + 314.15f) / 20000.0f;
    if(deg > 6.283)
    {
        deg = deg - 6.283;
    }
    return sinf(deg);
}

/**
 * @brief 获取两个信号相位偏差
 * @param sign1 信号1
 * @param sign2 信号2
 * @param notch_vars 陷波器参数
 * @param notch_coeff 陷波器配置
 * @return 偏差
 */
float PllGetSignPhaseErr(float sign1, float sign2, NOTCH_VARS_F* notch_vars, NOTCH_COEFF_F* notch_coeff)
{
    notch_vars->In = sign1 * sign2;
    NOTCH_FLTR_F_run(notch_vars, notch_coeff);
    return notch_vars->Out;
}
