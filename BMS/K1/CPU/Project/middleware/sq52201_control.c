

/* Includes ------------------------------------------------------------------*/
#include "sq52201_control.h"
#include "dev_sq52201.h"
#include "bms_protocol.h"

#define LOG_TAG     "sq52201_control"

typedef enum
{
    EN_SQ52201_SAMPLE_STEPS_IDLE,
    EN_SQ52201_SAMPLE_STEPS_GET_V,     //电压获取
    EN_SQ52201_SAMPLE_STEPS_GET_C      //电流获取
} EN_SQ52201_SAMPLE_STEPS;


static EN_SQ52201_SAMPLE_STEPS  _gSq52201SampleSteps = EN_SQ52201_SAMPLE_STEPS_IDLE;

/* sq52201定时器中断 */
void _Sq52201TimerIrq(void);

/**
 * @brief  : 初始化
 * @para   : None
 * @note   : None
 * @return : None
 */
void Sq52201Init(void)
{
    DevSQ52201Init();
    SQ52201SampleTimeHandle.irq_process = _Sq52201TimerIrq;
}


/**
 * @brief  : sq52201定时器中断
 * @para   : None
 * @note   : None
 * @return : None
 */
void _Sq52201TimerIrq()
{
    _gSq52201SampleSteps = EN_SQ52201_SAMPLE_STEPS_GET_C;
}


/**
 * @brief  : sq52201定时器中断
 * @para   : None
 * @note   : None
 * @return : None
 */
void Sq52201Process()
{
    if(_gBMSSysInfo.bms_upgrade_ing_flag == true)
    {
        return;
    }
    
    switch(_gSq52201SampleSteps)
    {
        case EN_SQ52201_SAMPLE_STEPS_IDLE:
        {
            break;
        }
        case EN_SQ52201_SAMPLE_STEPS_GET_V:     //电压获取
        {
            gCell_Info.total_cell_voltage = SQ52201GetBusV();
            _gSq52201SampleSteps = EN_SQ52201_SAMPLE_STEPS_IDLE;
            break;
        }
        case EN_SQ52201_SAMPLE_STEPS_GET_C:      //电流获取
        {
            gPackStatus.pack_current_real = SQ52201GetCurrent();
            
            //电流过大检测标志
            if(gPackStatus.pack_current_real <= -150000 || gPackStatus.pack_current_real >= 150000)
            {
                NOVA_SET_BIT(gPackStatus.ocv_process_flag, 11);
            }
            
            gPackStatus.pack_current_real  = gPackStatus.pack_current_real > 200000 ? 200000 : gPackStatus.pack_current_real;
            gPackStatus.pack_current_real  = gPackStatus.pack_current_real < -200000 ? -200000 : gPackStatus.pack_current_real;
            if(gPackStatus.pack_current_real > PACK_CURRENT_MIN_THRESHOLD)
            {
                gPackStatus.pack_current_calibration = gPackStatus.dsg_pack_current_calibration_factor * gPackStatus.pack_current_real;
            }
            else
            {
              gPackStatus.pack_current_calibration = gPackStatus.chg_pack_current_calibration_factor * gPackStatus.pack_current_real;  
            }
//            SOCAmpereHourIntegral(200, &gPackStatus);
            _gSq52201SampleSteps = EN_SQ52201_SAMPLE_STEPS_IDLE;
            break;
        }
        default:
        {
            break;
        }
    }
}


