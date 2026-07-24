/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        llc_dcdc_cntl.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-4-10
  * @brief       LLC-DCDC???????
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-4-15    <td>V1.0.0    <td>dengjiahui    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "dev_sensor.h"
#include "Solar_F.h"
#include "CNTL_PI_F.h"
#include "sci.h"
#include "voltagesourceinvlcfltr.h"
#include "llc_dcdc_cntl.h"

#include "algo_err_check.h"
#include "algo_info.h"
#include "inverter_loop_cntl.h"
#include "pwm_cntl.h"
#include "activation.h"
#include "generator.h"

#define SIGN_EXP_STARTUP      (0.005)        //内部Exp信号
#define SIGN_EXP_SPEED        (0.5)         //Exp信号的速率
#define SIGN_STAMP_STARTUP    (0.05)        
#define SIGN_STAMP_SPEED      (0.1)         
//exp信号（差分形式）
static float _gSignE[2] = { SIGN_EXP_STARTUP,SIGN_EXP_STARTUP };
static float _gSignStamp = SIGN_STAMP_STARTUP;

//float WATCH_geLlcFsm = 0;
//float WATCH_gSignE_0 = 0;

//传感器数据结构体
typedef struct sensor_data_stru_t
{
    float dc_bus_vol;   //DC bus电压
    float dc_bat_vol;   //DC bat电压
    float dc_trans_cur; //DC 变压器电流
}SensorData_t;

static SensorData_t _gSensorData;           //传感器数据
static float _garrSciBuf[8];                //Sci发送缓冲
static float _gPwm[2], _gDeg;                //LLC开关管PWM占空比
uint8_t gIsLlcStartupOk = 0;                //LLC缓起完成标志
static float _gMaxPwm = LLC_MAX_PWM;
uint8_t gLlcErr = 0;
static const AlgoInfo_t* _gAlgoInfo;
EN_LLC_STATE_DEF geLlcFsm = EN_LLC_STATE_DEF_READY;
static uint16_t llcrunstate_flag = 0;
static void _LlcFsmRun();
static void _LlcSoftStart();
/**
 * @brief 获取LLC相关的传感器数据
 */
void LlcSensorUpdate()
{
    _gSensorData.dc_bus_vol = gstSensorData.bus_voltage;
    _gSensorData.dc_bat_vol = gstSensorData.bat_voltage;
    _gSensorData.dc_trans_cur = gstSensorData.transformer_current;
}

void LlcCntlInit()
{
    _gAlgoInfo = AlgoStateGet();
    _gPwm[0] = 0;
    _gPwm[1] = 0;
    _gSignE[0] = SIGN_EXP_STARTUP;
    _gSignE[1] = SIGN_EXP_STARTUP;
}

/**
 * @brief LLC控制
 */
void LlcLoopCntl()
{
    LlcSensorUpdate();
    _LlcFsmRun();
    PwmSetLlcDuty(_gPwm[0], _gPwm[1]);
    PwmSetLlcDegDuty(_gDeg);
//float duty = _GvLoopCntl(500, _gSensorData.dc_bus_vol);
//LLCSetPwm(duty + 0.24);
}

void* LlcGetStateWave()
{
    extern float gGridPowerSet;
    extern uint8_t gRunstate;
    _garrSciBuf[0] = _gSensorData.dc_bus_vol;
    _garrSciBuf[1] = gAlgoErrDef.byte[0] | ((uint32_t)gAlgoErrDef.byte[1] << 16);
    _garrSciBuf[2] = _gSensorData.dc_bat_vol;
    _garrSciBuf[3] = gstSensorData.bat_current;
    _garrSciBuf[4] = _gPwm[0];
    _garrSciBuf[5] = _gSignE[0];
    _garrSciBuf[6] = EPwm6Regs.CMPA.bit.CMPA;
    _garrSciBuf[7] = EPwm7Regs.TBPHS.bit.TBPHS;
    return _garrSciBuf;
}

void LlcSendStateWave()
{
    LlcGetStateWave();
}

void LlcTest(uint8_t data)
{
    gEnLlcPwm = data == 1;
    gEnInvPwm = data == 1;
}

void LlcSetMaxPwm(float limit)
{
    if(limit >= 0.02 && limit <= 0.48)
    {
        _gMaxPwm = limit;
    }
}

float LlcGetMaxPwm()
{
    return _gMaxPwm;
}

/**
* @brief: _LlcFsmRun
* @param: 
* @param: 
* @return:
* @note: Llc状态机运行
*
*/
static void _LlcFsmRun()
{
	/*************************/
	//状态切换
    switch (geLlcFsm)
	{
	    case EN_LLC_STATE_DEF_READY:
		{
			/***************************/
			//condition
			/***************************/
		    if((llcrunstate_flag == LLC_SOFTSTART) && (gAlgoErr == 0)) //切换至LLC软起逻辑
			{
				/***************************/
				//nextstate
			    geLlcFsm = EN_LLC_STATE_DEF_SOFTSTART_PV;
				/***************************/
				/***************************/
				//action
                llcrunstate_flag = 0;   //状态复位
				/***************************/
			}
            else 
            {
                if((_gAlgoInfo->sys_enable == 0) || (gAlgoErr == 1) || _gAlgoInfo->inv_release_ok == 0)
                {
				    geLlcFsm = EN_LLC_STATE_DEF_READY;

                }/* code */
                else
                {
				    geLlcFsm = EN_LLC_STATE_DEF_SOFTSTART_BAT;
                }
            }
            
		    break;
		}
        case EN_LLC_STATE_DEF_SOFTSTART_PV:
        {
			/***************************/
			//condition
			/***************************/
		    if(gAlgoErr == 1) //软起失败，切换至READY状态 
			{
				/***************************/
				//nextstate
			    geLlcFsm = EN_LLC_STATE_DEF_READY;
				/***************************/
				/***************************/
				//action
                llcrunstate_flag = 0;   //状态复位
				/***************************/
			}
			/***************************/
			//condition
			/***************************/
		    else if((_gSensorData.dc_bat_vol > 47) && (_gPwm[0] == _gMaxPwm) && (_gPwm[1] == _gMaxPwm)) //电池电压大于35V，PV激活，等待电池响应
			{
				/***************************/
				//nextstate
			    geLlcFsm = EN_LLC_STATE_DEF_SOFTSTARTOK_PV;
				/***************************/
				/***************************/
				//action
				/***************************/
			}

            break;
        }
        case EN_LLC_STATE_DEF_SOFTSTARTOK_PV:
        {
			/***************************/
			//condition
			/***************************/
		    if(gAlgoErr == 1 || llcrunstate_flag == LLC_READY) //软起失败或命令切换至READY状态，切换至READY状态 
			{
				/***************************/
				//nextstate
			    geLlcFsm = EN_LLC_STATE_DEF_READY;
				/***************************/
				/***************************/
				//action
                llcrunstate_flag = 0;   //状态复位
			    gInvReleaseOk = 1; //PV启机禁止母线放电 
				/***************************/
			}
			/***************************/
			//condition
			/***************************/
		    else if((_gAlgoInfo->sys_enable == 1) && (ActivationFsmStateRead() == EN_ACTIVATION_STATE_DEF_NORMAL))
			{
				/***************************/
				//nextstate
			    geLlcFsm = EN_LLC_STATE_DEF_SOFTSTART_BAT;
				/***************************/
				/***************************/
				//action
				/***************************/
			}
			/***************************/
			//condition
			/***************************/
		    else if((_gAlgoInfo->sys_enable == 0) && (ActivationFsmStateRead() == EN_ACTIVATION_STATE_DEF_NORMAL))
			{
				/***************************/
				//nextstate
			    geLlcFsm = EN_LLC_STATE_DEF_READY;
				/***************************/
				/***************************/
				//action
				/***************************/
			}

            break;
        }
	    case EN_LLC_STATE_DEF_SOFTSTART_BAT:
		{
			/***************************/
			//condition
			/***************************/
		    if((gAlgoErr == 1) || (llcrunstate_flag == LLC_READY) || (_gAlgoInfo->sys_enable == 0)) //软起失败，切换至READY状态 
			{
				/***************************/
				//nextstate
			    geLlcFsm = EN_LLC_STATE_DEF_READY;
				/***************************/
				/***************************/
				//action
                llcrunstate_flag = 0;   //状态复位
				/***************************/
			}
			/***************************/
			//condition
			/***************************/
		    else if(_gPwm[0] == _gMaxPwm && _gPwm[1] == _gMaxPwm && _gDeg == 1) //软起成功切换至正常运行模式
			{
				/***************************/
				//nextstate
			    geLlcFsm = EN_LLC_STATE_DEF_NORMAL;
				/***************************/
				/***************************/
				//action
				/***************************/
			}
		    break;
		}
	    case EN_LLC_STATE_DEF_NORMAL:
		{
		    if((gAlgoErr == 1) || (llcrunstate_flag == LLC_READY) || (_gAlgoInfo->sys_enable == 0)) //软起失败，切换至READY状态 
			{
		        // if((GetGridToGenFinish() == 1) && ((gAlgoErrDefGetInvVoltOver() == 1) || (gAlgoErrDefGetCmpssCurOver() == 1)))
		        // {
		        // 发电机模式下这个不切换状态
		        // }
		        // else
		        // {
	            /***************************/
	            //nextstate
	            geLlcFsm = EN_LLC_STATE_DEF_READY;
	            /***************************/
	            /***************************/
	            //action
	            llcrunstate_flag = 0;   //状态复位
	            /***************************/
		        // }
			}
		    break;
		}
	    default:
		{
		    geLlcFsm = EN_LLC_STATE_DEF_READY;
		    break;
		}
	}
	/************************/
	/************************/
	// 状态运行
    switch (geLlcFsm)
	{
	    case EN_LLC_STATE_DEF_READY:  //TODO:将电池启机与PV启机分开区别在于电池启机存在高压侧泄放
		{
            _gPwm[0] = 0;
            _gPwm[1] = 0;
            _gSignE[0] = SIGN_EXP_STARTUP;
            _gSignE[1] = SIGN_EXP_STARTUP;
            _gSignStamp = SIGN_STAMP_STARTUP;
            _gDeg = SIGN_EXP_STARTUP;
            gIsLlcStartupOk = 0;
		    break;
		}
	    case EN_LLC_STATE_DEF_SOFTSTART_PV:
		{
            _LlcSoftStart();
		    break;
		}
	    case EN_LLC_STATE_DEF_SOFTSTARTOK_PV:
		{
            // _gDeg == _gDeg;
            _gPwm[0] = _gMaxPwm;
            _gPwm[1] = _gMaxPwm;
		    break;
        }
	    case EN_LLC_STATE_DEF_SOFTSTART_BAT:
		{
            _LlcSoftStart();
		    break;
		}
	    case EN_LLC_STATE_DEF_NORMAL:
		{
            // _gDeg == 1;
            _gPwm[0] = _gMaxPwm;
            _gPwm[1] = _gMaxPwm;
            gIsLlcStartupOk = (_gDeg == 1);
		    break;
		}
	    default:
		{
            _gPwm[0] = 0;
            _gPwm[1] = 0;
            _gSignE[0] = SIGN_EXP_STARTUP;
            _gSignE[1] = SIGN_EXP_STARTUP;
            _gSignStamp = SIGN_STAMP_STARTUP;
            _gDeg = SIGN_EXP_STARTUP;
            gIsLlcStartupOk = 0;
		    break;
		}
	}
	/************************/
}
/**
* @brief: LlcRunStateSet
* @param: Llrunstate 
* @return:
* @note: LLC运行状态设置
*
*/
void LlcRunStateSet(uint16_t llcrunstate)
{
    if(llcrunstate == LLC_SOFTSTART)
	{
	    llcrunstate_flag = LLC_SOFTSTART;  // LLC软起模式
	}
    else if(llcrunstate == LLC_READY)  // LLC就绪模式
	{
	    llcrunstate_flag = LLC_READY;
	}
}

/**
* @brief: LlcRunStateRead
* @param:  
* @return: geLlcFsm
* @note: Llc运行状态读取
*
*/
EN_LLC_STATE_DEF LlcRunStateRead()
{
//    WATCH_geLlcFsm = geLlcFsm;
    return geLlcFsm;
}
/**
* @brief: _LlcSoftStart
* @param: 
* @return:
* @note: LLC软启动
*
*/
void _LlcSoftStart()
{
    float32 sign_exp_speed;
    if(_gSignStamp < 1)
    {
        _gSignStamp += 0.00005 * SIGN_STAMP_SPEED;
    }

    _gPwm[0] = _gSignStamp;
    _gPwm[1] = _gSignStamp;

    if(_gPwm[0] > _gMaxPwm)
    {
        _gPwm[0] = _gMaxPwm;
    }
    if(_gPwm[1] > _gMaxPwm)
    {
        _gPwm[1] = _gMaxPwm;
    }

    // if((_gPwm[0] == _gMaxPwm) && (_gPwm[1] == _gMaxPwm) && (llcrunstate_flag == LLC_SOFTSTART))
    if((_gPwm[0] == _gMaxPwm) && (_gPwm[1] == _gMaxPwm)) 
    {
        if(_gSignE[1] < 0.125f)
        {
            sign_exp_speed = 0.1f;
        }
        else if((_gSignE[1] >= 0.125f) && (_gSignE[1] < 0.25f))
        {
            sign_exp_speed = 0.2f;
        }
        else
        {
            sign_exp_speed = 0.5f;
        }
    //更新内部Exp信号
        _gSignE[0] = _gSignE[1] * (1 + 0.00005 * sign_exp_speed); //exp[n] = exp[n-1] + exp[n-1] * kT, k为速率
        _gSignE[1] = _gSignE[0];
//        WATCH_gSignE_0 = _gSignE[0];

    /*缓启动信号*/
	    if(LlcRunStateRead() == EN_LLC_STATE_DEF_SOFTSTART_PV)
		{
            if(_gSignE[0] > 0.05)
            {
                _gSignE[0] = 0.05;
                _gSignE[1] = 0.05;
            }
		}
	    else
		{
            if(_gSignE[0] > 1)
            {
                _gSignE[0] = 1;
                _gSignE[1] = 1;
            }
        }
        _gDeg = _gSignE[0];
    }
    else
    {
        _gDeg = 0;
    }
}
