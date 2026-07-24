
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        hard_cmpss.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-7-29
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-7-29    <td>V1.0.0    <td>zhangqi    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "hard_cmpss.h"
#include "algo_err_check.h"
#define IND_CUR_OVER_HIGH  (uint16_t)(50 * 15.51515)//初始化按照50A阈值计算
/* 按照50A的阈值计算，(KCUR*ALGOERR_THRESHOLD_INV_CUR_OVER) *4096/3.3;
上式中：KCUR为电流传感器的转移阻抗，在这里为0.0125V/A      
*/
#define IND_CUR_OVER_LOW   (uint16_t)(50 * 15.51515)// 初始化按照-50A的阈值计算
/* 按照-50A的阈值计算，(-KCUR*ALGOERR_THRESHOLD_INV_CUR_OVER)*4096/3.3 ;
上式中：KCUR为电流传感器的转移阻抗，在这里为0.0125V/A      
*/
#define REF_1_5V 1862 //1.5/3.3*4096
#define CMPSS_STATUS ( EPwm2Regs.TZFLG.bit.DCAEVT1 || EPwm3Regs.TZFLG.bit.DCAEVT1 )

/**
* @brief:Cmpss_Init
* @return: 
* @note:初始化电感电流采样的硬件比较器保护功能 
*
*/
void Cmpss_Init()
{
    //Cmpss2对应Inductor current
    //比较电压设置
    EALLOW;
    Cmpss2Regs.COMPDACCTL.all = 0; 
    Cmpss2Regs.COMPDACCTL.bit.SELREF = 0; //配置DAC的参考源位模拟输入为VCCA-3.3V
    Cmpss2Regs.DACHVALS.bit.DACVAL = IND_CUR_OVER_HIGH + REF_1_5V; //配置H比较器参考电压阈值
    Cmpss2Regs.DACLVALS.bit.DACVAL = IND_CUR_OVER_LOW + REF_1_5V; //配置H比较器参考电压阈值
    Cmpss2Regs.COMPCTL.bit.COMPDACE = 1;
    //
    Cmpss2Regs.COMPDACCTL.bit.SWLOADSEL = 0; //0：系统时钟触发D触发器实现影子寄存器DACHVALA的更新;1:同步信号EOWMSYNCPER触发D触发器实现影子寄存器的更新
    Cmpss2Regs.COMPDACCTL.bit.DACSOURCE = 0; //选用DACHVALA作为DACHVALA影子寄存器的输入

    Cmpss2Regs.COMPCTL.bit.COMPHSOURCE = 0; //选用DACHVALA作为比较器的负输入
    Cmpss2Regs.COMPCTL.bit.COMPLSOURCE = 0; //选用DACLVALA作为比较器的负输入

    Cmpss2Regs.COMPCTL.bit.COMPHINV = 0; //高位比较器输出不反相
    Cmpss2Regs.COMPCTL.bit.COMPLINV = 1; //低位比较器输出反相
    /*******数字滤波器配置**********/
    /*NOTE: 由于采用了数字滤波器，其响应略微滞后(4+1)/SYSCLK_FREQUENCE，100M时钟为50ns*/
    Cmpss2Regs.CTRIPHFILCTL.bit.SAMPWIN = 4; //采样窗口长度4+1
    Cmpss2Regs.CTRIPHFILCTL.bit.THRESH = 2;  //次数大于2+1认为过流
    Cmpss2Regs.CTRIPHFILCLKCTL.bit.CLKPRESCALE = 0;
    Cmpss2Regs.CTRIPLFILCTL.bit.SAMPWIN = 4;
    Cmpss2Regs.CTRIPLFILCTL.bit.THRESH = 2;
    Cmpss2Regs.CTRIPLFILCLKCTL.bit.CLKPRESCALE = 0;
	  //Reset filter logic & start filtering
    Cmpss2Regs.CTRIPHFILCTL.bit.FILINIT = 1;
    Cmpss2Regs.CTRIPLFILCTL.bit.FILINIT = 1;
    /****************************/
                        
    Cmpss2Regs.COMPCTL.bit.ASYNCHEN = 1; //直通高位比较器数字滤波器；TODO:后续如误触发严重需配置数字滤波器
    Cmpss2Regs.COMPCTL.bit.ASYNCLEN = 1; //直通低位比较器数字滤波器；TODO:后续如误触发严重需配置数字滤波器


    Cmpss2Regs.COMPCTL.bit.CTRIPHSEL = 2; //选择高位比较器数字滤波器输出至EPWM_XBAR 
    Cmpss2Regs.COMPCTL.bit.CTRIPLSEL = 2; //选择低位比较器数字滤波器输出至EPWM_XBAR 


    EPwmXbarRegs.TRIP4MUX0TO15CFG.all = 0x00; //选择CMPSS2,CTRIPH和CTRIPL作为XBAR.G2和G3的输入
    EPwmXbarRegs.TRIP4MUXENABLE.all = 0x0C; // 使能EPWM,G2和G3

    //配置EPWM2和EPWM3的DC_module
    EPwm2Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 3; //选择TRIP4作为EPWM2的DCAH
    EPwm3Regs.DCTRIPSEL.bit.DCAHCOMPSEL = 3; //选择TRIP4作为EPWM3的DCAH
    //配置EPWM2和EPWM3的TZDCSEL-对信号DCAH和DCAL
    EPwm2Regs.TZDCSEL.all = 0x02; //选择DCAH-高电平触发OUTPUT EVENT1
    EPwm3Regs.TZDCSEL.all = 0x02; //选择DCAH-高电平触发OUTPUT EVENT1

    EPwm2Regs.DCACTL.bit.EVT1SRCSEL = 0; //选择DCAEVT1作为触发源
    EPwm3Regs.DCACTL.bit.EVT1SRCSEL = 0; //选择DCAEVT1作为触发源

    EPwm2Regs.DCACTL.bit.EVT1FRCSYNCSEL= 1; //异步触发
    EPwm3Regs.DCACTL.bit.EVT1FRCSYNCSEL= 1; //异步触发

    //TODO:如果采样信号受功率开关器件开关时刻的影响很大，易误触发则需配置Blankwindow-消隐时间
    //配置TZ触发
    EPwm2Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; //异步触发
    EPwm3Regs.TZCTL.bit.DCAEVT1 = TZ_FORCE_LO; //异步触发
    EPwm2Regs.TZSEL.bit.DCAEVT1 = 0; //不使能DCAEVT1作为One-shot-trip源
    EPwm3Regs.TZSEL.bit.DCAEVT1 = 0; //不使能DCAEVT1作为One-shot-trip源
    Cmpss2Regs.COMPSTSCLR.bit.HLATCHCLR = 1; //复位高位比较器数字滤波器输出 
    Cmpss2Regs.COMPSTSCLR.bit.LLATCHCLR = 1; //复位低位比较器数字滤波器输出 
    EDIS;

} 
/**
* @brief:Clear_cmpss_ost
* @return: 
* @note:清除硬件cmpss过流
*/
void Clear_cmpss_ost()
{
    EALLOW;
    Cmpss2Regs.COMPSTSCLR.bit.HLATCHCLR = 0;  
    Cmpss2Regs.COMPSTSCLR.bit.LLATCHCLR = 0;  
    EPwm2Regs.TZCLR.bit.DCAEVT1 = 1;
    EPwm3Regs.TZCLR.bit.DCAEVT1 = 1;
    EDIS;

}
/**
* @brief:Read_cmpss_status
* @return: 返回是否触发硬件过流 
* @note:
*/
uint16_t Read_cmpss_status()
{
  return CMPSS_STATUS; 
}
/**
* @brief:Updata_cmpss_dac
* @return: 返回是否配置成功 
* @note:
*/
uint16_t Updata_cmpss_dac(float adcref_reg)
{
  uint16_t cmpss_1_5V_ref = (uint16_t)(adcref_reg * 3 / 3.3);
  
  if(adcref_reg > 2000 && adcref_reg < 2100)
  {
    Cmpss2Regs.DACHVALS.bit.DACVAL = cmpss_1_5V_ref + IND_CUR_OVER_HIGH  ; //配置H比较器参考电压阈值
    Cmpss2Regs.DACLVALS.bit.DACVAL = cmpss_1_5V_ref - IND_CUR_OVER_LOW  ; //配置H比较器参考电压阈值
    return 1;
  }
    return 0;
}
/**
* @brief:Calibrate_cmpss_dac
* @return: 返回是否配置成功 
* @note:校正cmpss的DAC输出
*/
uint16_t Calibrate_cmpss_dac(float adcref_reg)
{
  uint16_t cmpss_1_5V_ref = (uint16_t)(adcref_reg * 3 / 3.3);
  int16_t dac_Loffset = -50;
  int16_t dac_Hoffset = 50;
  uint16_t ref_count = 0;
  uint16_t ref_delay = 0;

  
  if(adcref_reg > 2000 && adcref_reg < 2100)
  {
    while(ref_count < 101)
    {
      ref_count++;
      Cmpss2Regs.DACHVALS.bit.DACVAL = cmpss_1_5V_ref + dac_Loffset; //配置H比较器参考电压阈值
      while(ref_delay < 100)
        ref_delay++;
      ref_delay = 0;
      if (Read_cmpss_status() )
      {
        dac_Loffset ++;
        Clear_cmpss_ost();
      }
      else
        break;
    }
    Clear_cmpss_ost();

    ref_count = 0;
    ref_delay = 0;
    while(ref_count < 101)
    {
      ref_count++;
      Cmpss2Regs.DACHVALS.bit.DACVAL = cmpss_1_5V_ref + dac_Hoffset ; //配置H比较器参考电压阈值
      while(ref_delay < 100)
        ref_delay++;
      ref_delay = 0;
      if (Read_cmpss_status() == 0 )
      {
        dac_Hoffset --;
        Clear_cmpss_ost();
      }
      else
        break;
    }
  }
  int16_t dac_offset = (dac_Hoffset + dac_Loffset) / 2;
  if (abs(dac_Hoffset - dac_Loffset)  < 10)
  {
    Cmpss2Regs.DACHVALS.bit.DACVAL = cmpss_1_5V_ref + IND_CUR_OVER_HIGH + dac_offset; //配置H比较器参考电压阈值
    Cmpss2Regs.DACLVALS.bit.DACVAL = cmpss_1_5V_ref - IND_CUR_OVER_LOW  + dac_offset; //配置H比较器参考电压阈值

    EALLOW;
    EPwm2Regs.TZSEL.bit.DCAEVT1 = 1; //使能DCAEVT1作为One-shot-trip源
    EPwm3Regs.TZSEL.bit.DCAEVT1 = 1; //使能DCAEVT1作为One-shot-trip源
    EDIS;
    return 1;
  }
  else
  {
    Cmpss2Regs.DACHVALS.bit.DACVAL = cmpss_1_5V_ref + IND_CUR_OVER_HIGH  ; //配置H比较器参考电压阈值
    Cmpss2Regs.DACLVALS.bit.DACVAL = cmpss_1_5V_ref - IND_CUR_OVER_LOW   ; //配置H比较器参考电压阈值
    Clear_cmpss_ost();
    return 0;

  }
}
