/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        epwm.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#include "epwm.h"

// Variables used to indirectly address the peripheral
volatile struct EPWM_REGS *ePWM[] =
{
    &EPwm1Regs,         //intentional: (ePWM[0] not used)
    &EPwm1Regs, &EPwm2Regs, &EPwm3Regs, &EPwm4Regs, &EPwm5Regs, &EPwm6Regs,
    &EPwm7Regs, &EPwm8Regs, &EPwm9Regs, &EPwm10Regs, &EPwm11Regs, &EPwm12Regs
};

//TODO DisablePWMCLKCounting
/**
* @brief: DisablePWMCLKCounting
* @param: void                        
* @note: 
*
*/
void DisablePWMCLKCounting(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EDIS;
}

//TODO EnablePWMCLKCounting
/**
* @brief: EnablePWMCLKCounting
* @param: void                        
* @note: 
*
*/
void EnablePWMCLKCounting(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

/**
* @brief: SetupInverterPWM
* @param: inv_pwm_no                  
* @param: pwm_period_ticks            
* @param: pwm_dbred_ticks             
* @param: pwm_dbfed_ticks             
* @note: 
*
*/
void SetupInverterPWM(uint16_t inv_pwm_no, uint16_t pwm_period_ticks, uint16_t pwm_dbred_ticks, uint16_t pwm_dbfed_ticks)
{
    EALLOW;

    // PWM clock on F2837x is divided by 2
    // ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV=1
    // Deadband needs to be 0.5us => 10ns*50=500ns
    // Time Base SubModule Registers
    (*ePWM[inv_pwm_no]).TBCTL.bit.PRDLD = TB_SHADOW;
    (*ePWM[inv_pwm_no]).TBPRD = pwm_period_ticks >>1 ; // PWM frequency = 1 / period
    (*ePWM[inv_pwm_no]).TBPHS.bit.TBPHS = 0;
    (*ePWM[inv_pwm_no]).TBCTR = 0;
    (*ePWM[inv_pwm_no]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    (*ePWM[inv_pwm_no]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
    (*ePWM[inv_pwm_no]).TBCTL.bit.CLKDIV = TB_DIV1;

    (*ePWM[inv_pwm_no+1]).TBCTL.bit.PRDLD = TB_SHADOW;
    (*ePWM[inv_pwm_no+1]).TBPRD = pwm_period_ticks >>1 ; // PWM frequency = 1 / period
    (*ePWM[inv_pwm_no+1]).TBPHS.bit.TBPHS = 0;
    (*ePWM[inv_pwm_no+1]).TBCTR = 0;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.CLKDIV = TB_DIV1;

//  (*ePWM[inv_pwm_no+2]).TBCTL.bit.PRDLD = TB_SHADOW;
//  (*ePWM[inv_pwm_no+2]).TBPRD = pwm_period_ticks >>1 ; // PWM frequency = 1 / period
//  (*ePWM[inv_pwm_no+2]).TBPHS.bit.TBPHS = 0;
//  (*ePWM[inv_pwm_no+2]).TBCTR = 0;
//  (*ePWM[inv_pwm_no+2]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
//  (*ePWM[inv_pwm_no+2]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
//  (*ePWM[inv_pwm_no+2]).TBCTL.bit.CLKDIV = TB_DIV1;

    // Counter Compare Submodule Registers
    (*ePWM[inv_pwm_no]).CMPA.bit.CMPA = 0; // set duty 0% initially
    (*ePWM[inv_pwm_no]).CMPB.bit.CMPB = 0; // set duty 0% initially
    (*ePWM[inv_pwm_no]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    (*ePWM[inv_pwm_no+1]).CMPA.bit.CMPA = 0; // set duty 0% initially
    (*ePWM[inv_pwm_no+1]).CMPB.bit.CMPB = 0; // set duty 0% initially
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

//  (*ePWM[inv_pwm_no+2]).CMPA.bit.CMPA = 0; // set duty 0% initially
//  (*ePWM[inv_pwm_no+2]).CMPB.bit.CMPB = 0;
//  (*ePWM[inv_pwm_no+2]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
//  (*ePWM[inv_pwm_no+2]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
//  (*ePWM[inv_pwm_no+2]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
//  (*ePWM[inv_pwm_no+2]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    // Action Qualifier SubModule Registers
    // assuming positive half cycle comes first i.e. dhl>0
//  (*ePWM[inv_pwm_no]).AQCTLA.bit.CAU = AQ_SET; // CTR = CMPA@UP , set to 1
//  (*ePWM[inv_pwm_no]).AQCTLA.bit.CAD = AQ_CLEAR; // CTR = CMPA@Down , toggle

    // to start don't configure the PWM to do anything
    (*ePWM[inv_pwm_no]).AQCTLA.all = 0;
    (*ePWM[inv_pwm_no]).AQCTLB.all = 0;

    (*ePWM[inv_pwm_no+1]).AQCTLA.all = 0; // set to take no action
    (*ePWM[inv_pwm_no+1]).AQCTLB.all = 0;
//  (*ePWM[inv_pwm_no+1]).AQCTLA.bit.CAD = AQ_CLEAR; // CTR = CMPA@Down , clear

//  (*ePWM[inv_pwm_no+2]).AQCTLA.all = 0; // set to take no action
//  (*ePWM[inv_pwm_no+2]).AQCTLB.all = 0; // set to take no action
//  (*ePWM[inv_pwm_no+2]).AQCTLA.bit.CAD = AQ_SET;
//  (*ePWM[inv_pwm_no+2]).AQCTLA.bit.CAU = AQ_SET;
//  (*ePWM[inv_pwm_no+2]).AQCTLB.bit.CBD = AQ_SET;
//  (*ePWM[inv_pwm_no+2]).AQCTLB.bit.CBU = AQ_SET;
////    (*ePWM[inv_pwm_no+2]).AQCTLA.bit.CAD = AQ_CLEAR; // CTR = CMPA@Down , clear

    // Active high complementary PWMs - Set up the deadband
    (*ePWM[inv_pwm_no]).DBCTL.bit.IN_MODE = DBA_ALL;
    (*ePWM[inv_pwm_no]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    (*ePWM[inv_pwm_no]).DBCTL.bit.POLSEL = DB_ACTV_HIC;
    (*ePWM[inv_pwm_no]).DBRED.all =  pwm_dbred_ticks;
    (*ePWM[inv_pwm_no]).DBFED.all =  pwm_dbred_ticks;

    (*ePWM[inv_pwm_no+1]).DBCTL.bit.IN_MODE = DBA_ALL;
    (*ePWM[inv_pwm_no+1]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
    (*ePWM[inv_pwm_no+1]).DBCTL.bit.POLSEL = DB_ACTV_HIC;
    (*ePWM[inv_pwm_no+1]).DBRED.all =  pwm_dbred_ticks;
    (*ePWM[inv_pwm_no+1]).DBFED.all =  pwm_dbred_ticks;

//  (*ePWM[inv_pwm_no+2]).DBCTL.bit.IN_MODE = DBA_ALL;
//  (*ePWM[inv_pwm_no+2]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
//  (*ePWM[inv_pwm_no+2]).DBCTL.bit.POLSEL = DB_ACTV_HIC;
//  (*ePWM[inv_pwm_no+2]).DBRED.all =  pwm_dbred_ticks;
//  (*ePWM[inv_pwm_no+2]).DBFED.all =  pwm_dbred_ticks;

    (*ePWM[inv_pwm_no]).TBCTL.bit.PHSEN = TB_DISABLE;
    (*ePWM[inv_pwm_no]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // sync "down-stream"

    (*ePWM[inv_pwm_no+1]).TBCTL.bit.PHSEN = TB_DISABLE;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // sync "down-stream"
    // configure PWM 2 as slaves and let it pass the sync in pulse from PWM1
//  ePWM[inv_pwm_no+1]->TBCTL.bit.SYNCOSEL=TB_SYNC_IN;
//  ePWM[inv_pwm_no+1]->TBCTL.bit.PHSEN=TB_ENABLE;
//  ePWM[inv_pwm_no+1]->TBPHS.bit.TBPHS=2;
//  ePWM[inv_pwm_no+1]->TBCTL.bit.PHSDIR=TB_UP;

//  (*ePWM[inv_pwm_no+2]).TBCTL.bit.PHSEN = TB_DISABLE;
//  (*ePWM[inv_pwm_no+2]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // sync "down-stream"

    EDIS;
}

/**
* @brief: SetupPVPWM
* @param: inv_pwm_no                  
* @param: pwm_period_ticks            
* @param: pwm_dbred_ticks             
* @param: pwm_dbfed_ticks             
* @note: 
*
*/
void SetupPVPWM(uint16_t inv_pwm_no, uint16_t pwm_period_ticks, uint16_t pwm_dbred_ticks, uint16_t pwm_dbfed_ticks)
{
    EALLOW;

    // PWM clock on F2837x is divided by 2
    // ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV=1
    // Deadband needs to be 0.5us => 10ns*50=500ns
    // Time Base SubModule Registers
    (*ePWM[inv_pwm_no]).TBCTL.bit.PRDLD = TB_SHADOW;
    (*ePWM[inv_pwm_no]).TBPRD = pwm_period_ticks >> 1 ; // PWM frequency = 1 / period
    (*ePWM[inv_pwm_no]).TBPHS.bit.TBPHS = 0;
    (*ePWM[inv_pwm_no]).TBCTR = 0;
    (*ePWM[inv_pwm_no]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    (*ePWM[inv_pwm_no]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
    (*ePWM[inv_pwm_no]).TBCTL.bit.CLKDIV = TB_DIV1;

    // Counter Compare Submodule Registers
    (*ePWM[inv_pwm_no]).CMPA.bit.CMPA = (*ePWM[inv_pwm_no]).TBPRD >> 1; // set duty 0% initially
    (*ePWM[inv_pwm_no]).CMPB.bit.CMPB = (*ePWM[inv_pwm_no]).TBPRD >> 1; // set duty 0% initially; // set duty 0% initially
    (*ePWM[inv_pwm_no]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    // to start don't configure the PWM to do anything
    (*ePWM[inv_pwm_no]).AQCTLA.all = 0;
    (*ePWM[inv_pwm_no]).AQCTLB.all = 0;
    (*ePWM[inv_pwm_no]).AQCTLA.bit.CAU = AQ_SET; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no]).AQCTLA.bit.CAD = AQ_CLEAR; // CTR = CMPA@Down , toggle
    (*ePWM[inv_pwm_no]).AQCTLB.bit.CBU = AQ_SET; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no]).AQCTLB.bit.CBD = AQ_CLEAR; // CTR = CMPA@Down , toggle

    // Active high complementary PWMs - Set up the deadband
//  (*ePWM[inv_pwm_no]).DBCTL.bit.IN_MODE = DBA_ALL;
//  (*ePWM[inv_pwm_no]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
//  (*ePWM[inv_pwm_no]).DBCTL.bit.POLSEL = DB_ACTV_HIC;
//  (*ePWM[inv_pwm_no]).DBRED.all =  pwm_dbred_ticks;
//  (*ePWM[inv_pwm_no]).DBFED.all =  pwm_dbred_ticks;

    (*ePWM[inv_pwm_no]).TBCTL.bit.PHSEN = TB_DISABLE;
    (*ePWM[inv_pwm_no]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // sync "down-stream"



    EDIS;
}

/**
* @brief: SetupLLCPWM
* @param: inv_pwm_no                  
* @param: pwm_period_ticks            
* @param: pwm_dbred_ticks             
* @param: phase_move                  
* @note: 
*
*/
void SetupLLCPWM(uint16_t inv_pwm_no, uint16_t pwm_period_ticks, uint16_t pwm_dbred_ticks, uint16_t phase_move)
{
    EALLOW;

    // PWM clock on F2837x is divided by 2
    // ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV=1
    // Deadband needs to be 0.5us => 10ns*50=500ns
    // Time Base SubModule Registers
    (*ePWM[inv_pwm_no]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    (*ePWM[inv_pwm_no]).TBPRD = pwm_period_ticks >> 1; // PWM frequency = 1 / period
    (*ePWM[inv_pwm_no]).TBCTL.bit.PHSEN = TB_DISABLE;
    (*ePWM[inv_pwm_no]).TBCTL.bit.PRDLD = TB_SHADOW;
    (*ePWM[inv_pwm_no]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // sync "down-stream"
    (*ePWM[inv_pwm_no]).TBPHS.bit.TBPHS = 0;
    (*ePWM[inv_pwm_no]).TBCTR = 0;
    (*ePWM[inv_pwm_no]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
    (*ePWM[inv_pwm_no]).TBCTL.bit.CLKDIV = TB_DIV1;

    // Counter ComparSubmodule Registers
    (*ePWM[inv_pwm_no]).CMPA.bit.CMPA = 0;//(*ePWM[inv_pwm_no]).TBPRD >> 1; // set duty 0% initially
    (*ePWM[inv_pwm_no]).CMPB.bit.CMPB = 0;//(*ePWM[inv_pwm_no]).TBPRD >> 1; // set duty 0% initially; // set duty 0% initially
//  (*ePWM[inv_pwm_no]).CMPA.bit.CMPA = (*ePWM[inv_pwm_no]).TBPRD - (uint16_t)((float32)(*ePWM[inv_pwm_no]).TBPRD * 0.4);
//  (*ePWM[inv_pwm_no]).CMPB.bit.CMPB = (uint16_t)((float32)(*ePWM[inv_pwm_no]).TBPRD * 0.4);
    (*ePWM[inv_pwm_no]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    // to start don'tonfigure the PWM to do anything
    (*ePWM[inv_pwm_no]).AQCTLA.all = 0;
    (*ePWM[inv_pwm_no]).AQCTLB.all = 0;
    (*ePWM[inv_pwm_no]).AQCTLA.bit.CAU = AQ_SET; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no]).AQCTLA.bit.CAD = AQ_CLEAR; // CTR = CMPA@Down , toggle
    (*ePWM[inv_pwm_no]).AQCTLB.bit.CBU = AQ_CLEAR; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no]).AQCTLB.bit.CBD = AQ_SET; // CTR = CMPA@Down , toggle

    // Active high complementary PWMs - Set up the deadband
//  (*ePWM[inv_pwm_no]).DBCTL.bit.IN_MODE = DBA_ALL;
//  (*ePWM[inv_pwm_no]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
//  (*ePWM[inv_pwm_no]).DBCTL.bit.POLSEL = DB_ACTV_HIC;
//  (*ePWM[inv_pwm_no]).DBRED.all =  pwm_dbred_ticks;
//  (*ePWM[inv_pwm_no]).DBFED.all =  pwm_dbred_ticks;

    (*ePWM[inv_pwm_no+1]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    (*ePWM[inv_pwm_no+1]).TBPRD = pwm_period_ticks >> 1; // PWM frequency = 1 / period
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.PHSEN = TB_DISABLE;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.PRDLD = TB_SHADOW;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // sync "down-stream"
    (*ePWM[inv_pwm_no+1]).TBPHS.bit.TBPHS = 0;
    (*ePWM[inv_pwm_no+1]).TBCTR = 0;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.CLKDIV = TB_DIV1;

    // Counter ComparSubmodule Registers
    (*ePWM[inv_pwm_no+1]).CMPA.bit.CMPA = 0;//(*ePWM[inv_pwm_no]).TBPRD >> 1; // set duty 0% initially
    (*ePWM[inv_pwm_no+1]).CMPB.bit.CMPB = 0;//(*ePWM[inv_pwm_no]).TBPRD >> 1; // set duty 0% initially; // set duty 0% initially
//  (*ePWM[inv_pwm_no+1]).CMPA.bit.CMPA = (uint16_t)((float32)(*ePWM[inv_pwm_no]).TBPRD * 0.4);
//  (*ePWM[inv_pwm_no+1]).CMPB.bit.CMPB = (*ePWM[inv_pwm_no]).TBPRD - (uint16_t)((float32)(*ePWM[inv_pwm_no]).TBPRD * 0.4);
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    // to start don'tonfigure the PWM to do anything
    (*ePWM[inv_pwm_no+1]).AQCTLA.all = 0;
    (*ePWM[inv_pwm_no+1]).AQCTLB.all = 0;
    (*ePWM[inv_pwm_no+1]).AQCTLA.bit.CAU = AQ_CLEAR; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no+1]).AQCTLA.bit.CAD = AQ_SET; // CTR = CMPA@Down , toggle
    (*ePWM[inv_pwm_no+1]).AQCTLB.bit.CBU = AQ_SET; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no+1]).AQCTLB.bit.CBD = AQ_CLEAR; // CTR = CMPA@Down , toggle
    EDIS;
}

/**
* @brief: SetupBusPWM
* @param: inv_pwm_no                  
* @param: pwm_period_ticks            
* @param: pwm_dbred_ticks             
* @note: 
*
*/
void SetupBusPWM(uint16_t inv_pwm_no, uint16_t pwm_period_ticks, uint16_t pwm_dbred_ticks)
{
    EALLOW;

    // PWM clock on F2837x is divided by 2
    // ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV=1
    // Deadband needs to be 0.5us => 10ns*50=500ns
    // Time Base SubModule Registers
    (*ePWM[inv_pwm_no]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    (*ePWM[inv_pwm_no]).TBPRD = pwm_period_ticks >> 1; // PWM frequency = 1 / period
    (*ePWM[inv_pwm_no]).TBCTL.bit.PHSEN = TB_DISABLE;
    (*ePWM[inv_pwm_no]).TBCTL.bit.PRDLD = TB_SHADOW;
    (*ePWM[inv_pwm_no]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // sync "down-stream"
    (*ePWM[inv_pwm_no]).TBPHS.bit.TBPHS = 0;
    (*ePWM[inv_pwm_no]).TBCTR = 0;
    (*ePWM[inv_pwm_no]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
    (*ePWM[inv_pwm_no]).TBCTL.bit.CLKDIV = TB_DIV1;

    // Counter ComparSubmodule Registers
    (*ePWM[inv_pwm_no]).CMPA.bit.CMPA = 0;//(*ePWM[inv_pwm_no]).TBPRD >> 1; // set duty 0% initially
    (*ePWM[inv_pwm_no]).CMPB.bit.CMPB = 0;//(*ePWM[inv_pwm_no]).TBPRD >> 1; // set duty 0% initially; // set duty 0% initially
//  (*ePWM[inv_pwm_no]).CMPA.bit.CMPA = (*ePWM[inv_pwm_no]).TBPRD - (uint16_t)((float32)(*ePWM[inv_pwm_no]).TBPRD * 0.4);
//  (*ePWM[inv_pwm_no]).CMPB.bit.CMPB = (uint16_t)((float32)(*ePWM[inv_pwm_no]).TBPRD * 0.4);
    (*ePWM[inv_pwm_no]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    // to start don'tonfigure the PWM to do anything
    (*ePWM[inv_pwm_no]).AQCTLA.all = 0;
    (*ePWM[inv_pwm_no]).AQCTLB.all = 0;
    (*ePWM[inv_pwm_no]).AQCTLA.bit.CAU = AQ_SET; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no]).AQCTLA.bit.CAD = AQ_CLEAR; // CTR = CMPA@Down , toggle
    (*ePWM[inv_pwm_no]).AQCTLB.bit.CBU = AQ_CLEAR; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no]).AQCTLB.bit.CBD = AQ_SET; // CTR = CMPA@Down , toggle

    // Active high complementary PWMs - Set up the deadband
//  (*ePWM[inv_pwm_no]).DBCTL.bit.IN_MODE = DBA_ALL;
//  (*ePWM[inv_pwm_no]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
//  (*ePWM[inv_pwm_no]).DBCTL.bit.POLSEL = DB_ACTV_HIC;
//  (*ePWM[inv_pwm_no]).DBRED.all =  pwm_dbred_ticks;
//  (*ePWM[inv_pwm_no]).DBFED.all =  pwm_dbred_ticks;

    (*ePWM[inv_pwm_no+1]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    (*ePWM[inv_pwm_no+1]).TBPRD = pwm_period_ticks >> 1; // PWM frequency = 1 / period
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.PHSEN = TB_DISABLE;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.PRDLD = TB_SHADOW;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // sync "down-stream"
    (*ePWM[inv_pwm_no+1]).TBPHS.bit.TBPHS = 0;
    (*ePWM[inv_pwm_no+1]).TBCTR = 0;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.HSPCLKDIV = TB_DIV1;
    (*ePWM[inv_pwm_no+1]).TBCTL.bit.CLKDIV = TB_DIV1;

    // Counter ComparSubmodule Registers
    (*ePWM[inv_pwm_no+1]).CMPA.bit.CMPA = 0;//(*ePWM[inv_pwm_no]).TBPRD >> 1; // set duty 0% initially
    (*ePWM[inv_pwm_no+1]).CMPB.bit.CMPB = 0;//(*ePWM[inv_pwm_no]).TBPRD >> 1; // set duty 0% initially; // set duty 0% initially
//  (*ePWM[inv_pwm_no+1]).CMPA.bit.CMPA = (uint16_t)((float32)(*ePWM[inv_pwm_no]).TBPRD * 0.4);
//  (*ePWM[inv_pwm_no+1]).CMPB.bit.CMPB = (*ePWM[inv_pwm_no]).TBPRD - (uint16_t)((float32)(*ePWM[inv_pwm_no]).TBPRD * 0.4);
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    (*ePWM[inv_pwm_no+1]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;

    // to start don'tonfigure the PWM to do anything
    (*ePWM[inv_pwm_no+1]).AQCTLA.all = 0;
    (*ePWM[inv_pwm_no+1]).AQCTLB.all = 0;
    (*ePWM[inv_pwm_no+1]).AQCTLA.bit.CAU = AQ_CLEAR; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no+1]).AQCTLA.bit.CAD = AQ_SET; // CTR = CMPA@Down , toggle
    (*ePWM[inv_pwm_no+1]).AQCTLB.bit.CBU = AQ_SET; // CTR = CMPA@UP , set to 1
    (*ePWM[inv_pwm_no+1]).AQCTLB.bit.CBD = AQ_CLEAR; // CTR = CMPA@Down , toggle
    EDIS;
}
