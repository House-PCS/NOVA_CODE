/* include ------------------------------------------------------------------*/
#include <algo_interface.h>
#include "energy_management.h"
#include "cla_control.h"
#include "product_specification.h"
#include "AlgoFw/component/Comp_NoiseVar.h"


InvInterface_t gstInvInterface; //逆变器接口
Comp_NoiseVar_t gstPvPwrNoiseVar[4]; //PV功率噪声方差计算组件
Comp_NoiseVar_t gstPvPwrTotalNoiseVar;
#pragma DATA_SECTION(gstInvInterface, "CpuToCla1MsgRAM");
//
//PwrClaCompute_t gstBackupData;
//#pragma DATA_SECTION(gstBackupData, "Cla1DataRam");
//PwrClaCompute_t gstInvData;
//#pragma DATA_SECTION(gstInvData, "Cla1DataRam");

/* private define -----------------------------------------------------------*/



/* private variables --------------------------------------------------------*/



/* public function prototype ------------------------------------------------*/
/**
  * @brief  InterfaceCmd
  * @param  None
  * @retval None
  * @note   None loop_type 0:开环运行；非零为闭环运行；2：进入并网指令，3：允许并网
*/
void InterfaceCmd(uint16_t en_inv, uint16_t en_grid, uint16_t loop_type, uint16_t clear_fault, uint16_t self_test_finish)
{
    if(en_inv == 1 && self_test_finish == 1)
    {
        gstInvInterface.en_inv_cmd = 1;
        if(loop_type == 0)
        {
            gstInvInterface.openloop_inv_cmd = 1;
        }
        else
        {
            gstInvInterface.closeloop_inv_cmd = 1;
            if(loop_type == 2)   //DONE:增加进入并网中指令
            {
                gstInvInterface.en_oning_grid_cmd = 1;
            }
            else if(loop_type == 3)//DONE:增加进入并网指令
            {
                gstInvInterface.en_on_grid_cmd = 1;
            }
            if(loop_type < 2) //脱网
            {
                gstInvInterface.en_oning_grid_cmd = 0;
                gstInvInterface.en_on_grid_cmd = 0;
            }
        }
    }
    else
    {
        gstInvInterface.en_inv_cmd = 0;
        gstInvInterface.openloop_inv_cmd = 0;
        gstInvInterface.closeloop_inv_cmd = 0;
        gstInvInterface.en_oning_grid_cmd = 0;
        gstInvInterface.en_on_grid_cmd = 0;
    }
    if((clear_fault == 1) && (gstInvInterface.clear_fault_cmd == 0))
    {
        gstInvInterface.clear_fault_cmd = 1;
    }
    else
    {
        gstInvInterface.clear_fault_cmd = 0;
    }
    gstInvInterface.self_test_finish = self_test_finish;

}
/**
  * @brief  AlgoInterfaceInit
  * @param  None
  * @return None
  * @note   接口初始化
*/
void AlgoInterfaceInit(void)
{
        gstInvInterface.en_inv_cmd = 0;
        gstInvInterface.openloop_inv_cmd = 0;
        gstInvInterface.closeloop_inv_cmd = 0;
        gstInvInterface.en_on_grid_cmd = 0;
        gstInvInterface.active_pwr_ref = 0.0f;
        gstInvInterface.reactive_pwr_ref = 0.0f;
        gstInvInterface.reactive_pwr_compute = 0.0f;
        gstInvInterface.inv_cur_amp_max = 120.0f;

        Comp_NoiseVar_F32_Init(&gstPvPwrNoiseVar[0],40000,0.2f*6.28f/20000.0f,2);
        Comp_NoiseVar_F32_Init(&gstPvPwrNoiseVar[1],40000,0.2f*6.28f/20000.0f,2);
        Comp_NoiseVar_F32_Init(&gstPvPwrNoiseVar[2],40000,0.2f*6.28f/20000.0f,2);
        Comp_NoiseVar_F32_Init(&gstPvPwrNoiseVar[3],40000,0.2f*6.28f/20000.0f,2);
        Comp_NoiseVar_F32_Init(&gstPvPwrTotalNoiseVar,40000,0.2f*6.28f/20000.0f,2);
}
/**
  * @brief  SetPwrRef
  * @param  active_pwr_ref:目标有功功率值；reactive_pwr_ref:目标无功功率值;
  * @return None
  * @note   None
*/
void SetPwrRef(float32 active_pwr_ref, float32 reactive_pwr_ref)
{
    gstInvInterface.active_pwr_ref = active_pwr_ref;
    gstInvInterface.reactive_pwr_ref = reactive_pwr_ref;
}



/* private function prototype -----------------------------------------------*/
