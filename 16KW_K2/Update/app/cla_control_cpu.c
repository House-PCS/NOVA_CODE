#include "cla_control_cpu.h"

#include "F28x_Project.h"
#include "cla_control.h"
#include "cla_cpu_share.h"
#include "controler.h"
#include "pwr_compute.h"
#include "product_specification.h"

#define CPU_INV_CONTROL_TS        INV_CONTROL_TS
#define CPU_PWR_COMPUTE_FREQ      (1.0f / CPU_INV_CONTROL_TS)
#define CPU_PWR_LOWEST_AC_FREQ    INV_LOWEST_AC_FREQ
#define CPU_PWR_HIGHEST_AC_FREQ   INV_HIGHEST_AC_FREQ
#define CPU_PWR_COMPUTE_AC_PERIOD 5.0f
#define DISCONNECTCOUNTTRIGG      400.0f

#if defined(__TI_COMPILER_VERSION__)
#define CLA_NOBLOCKED __attribute__((noblocked))
#else
#define CLA_NOBLOCKED
#endif

extern EN_INV_STATE geInvControlStateCla;
extern CLA_NOBLOCKED InvLoop_t gstInvLoop;
extern CLA_NOBLOCKED ForWardPi_t gstVoltageLoop;
extern CLA_NOBLOCKED ForWardPi_t gstCurrentLoop;
extern CLA_NOBLOCKED ClaSinCosType_t gstSinCosSignal;

extern CLA_NOBLOCKED ClaAlternatingType_t gstGrid;
extern CLA_NOBLOCKED ClaAlternatingType_t gstInv;
extern CLA_NOBLOCKED ClaAlternatingType_t gstGenerator;
extern CLA_NOBLOCKED ClaAlternatingType_t gstBackUp;
extern CLA_NOBLOCKED ClaAlternatingType_t gstCT;
extern CLA_NOBLOCKED ClaAlternatingType_t gstNormalLoad;

extern CLA_NOBLOCKED PwrClaCompute_t gstInvData;
extern CLA_NOBLOCKED PwrClaCompute_t gstBackUpData;
extern CLA_NOBLOCKED PwrClaCompute_t gstGeneratorData;
extern CLA_NOBLOCKED PwrClaCompute_t gstGridData;
extern CLA_NOBLOCKED PwrClaCompute_t gstCTData;
extern CLA_NOBLOCKED PwrClaCompute_t gstNormalLoadData;

extern CLA_NOBLOCKED float32 gClaInitFinish;
extern CLA_NOBLOCKED float32 gInvCurrentRefAmp;
extern CLA_NOBLOCKED float32 gInvCurrentPhase;
extern CLA_NOBLOCKED float32 gInvCurrAmpPI;
extern CLA_NOBLOCKED float32 gInvCurrThetaPI;
extern CLA_NOBLOCKED float32 gInvIdPI;
extern CLA_NOBLOCKED float32 gInvIqPI;
extern CLA_NOBLOCKED float32 gDuty_err;
extern CLA_NOBLOCKED float32 gInvDisconnectCount;

static void SinglePhasePLLInitCpu(ClaType_t *value)
{
    value->pll.k1 = 1.0f;
    value->pll.k2 = 0.00001f;
    value->pll.kp = 1.43f;
    value->pll.ki = 200.0f;
    value->pll.cos0 = 1.0f;
    value->pll.sin0 = 0.0f;
    value->pll.cos1 = 1.0f;
    value->pll.sin1 = 0.0f;
    value->pll.theta = 0.0f;
    value->pll.theta1 = 0.0f;
}

static void InitForwardPiLoopCpu(ForWardPi_t *loop, float32 kp, float32 ki, float32 delta_max, float32 umax)
{
    loop->Ref = 0.0f;
    loop->Fbk = 0.0f;
    loop->Out_pi = 0.0f;
    loop->Out_Ki = 0.0f;
    loop->Out = 0.0f;
    loop->Kp = kp;
    loop->Ki = ki;
    loop->Kd = 0.0f;
    loop->delta_max = delta_max;
    loop->delta = 0.0f;
    loop->Umax = umax;
    loop->Umin = -umax;
    loop->erro = 0.0f;
    loop->erro_up = 0.0f;
}

void ClaControlCpuInitOnce(void)
{
    if (gClaInitFinish != 0.0f)
    {
        return;
    }

    gstInvLoop.disconnect_volt.ref_amplitude = INV_VOL_NORMAL;

    gstInvLoop.disconnect_curr.kp = 0.1f;
    gstInvLoop.disconnect_curr.ki = 500.0f;
    gstInvLoop.disconnect_curr.a1 = -1.0f;
    gstInvLoop.disconnect_curr.b0 = (2.0f * gstInvLoop.disconnect_curr.kp
            + gstInvLoop.disconnect_curr.ki * CPU_INV_CONTROL_TS) * 0.5f;
    gstInvLoop.disconnect_curr.b1 = -(2.0f * gstInvLoop.disconnect_curr.kp
            - gstInvLoop.disconnect_curr.ki * CPU_INV_CONTROL_TS) * 0.5f;

    gstInvLoop.disconnect_volt.kp = 0.10f;
    gstInvLoop.disconnect_volt.kr = 45.0f;
    gstInvLoop.disconnect_volt.wc = 3.14f;
    gstInvLoop.disconnect_volt.w0 = DOUBLE_PI * 50.0f;
    gstInvLoop.disconnect_volt.a0 = (4.0f * gstInvLoop.disconnect_volt.kp)
            / (CPU_INV_CONTROL_TS * CPU_INV_CONTROL_TS)
            + (4.0f * gstInvLoop.disconnect_volt.wc
                    * (gstInvLoop.disconnect_volt.kp + gstInvLoop.disconnect_volt.kr))
                    / CPU_INV_CONTROL_TS
            + gstInvLoop.disconnect_volt.kp * gstInvLoop.disconnect_volt.w0
                    * gstInvLoop.disconnect_volt.w0;
    gstInvLoop.disconnect_volt.a1 = (-8.0f * gstInvLoop.disconnect_volt.kp)
            / (CPU_INV_CONTROL_TS * CPU_INV_CONTROL_TS)
            + 2.0f * gstInvLoop.disconnect_volt.kp * gstInvLoop.disconnect_volt.w0
                    * gstInvLoop.disconnect_volt.w0;
    gstInvLoop.disconnect_volt.a2 = (4.0f * gstInvLoop.disconnect_volt.kp)
            / (CPU_INV_CONTROL_TS * CPU_INV_CONTROL_TS)
            - (4.0f * gstInvLoop.disconnect_volt.wc
                    * (gstInvLoop.disconnect_volt.kp + gstInvLoop.disconnect_volt.kr))
                    / CPU_INV_CONTROL_TS
            + gstInvLoop.disconnect_volt.kp * gstInvLoop.disconnect_volt.w0
                    * gstInvLoop.disconnect_volt.w0;
    gstInvLoop.disconnect_volt.b0 = 4.0f / (CPU_INV_CONTROL_TS * CPU_INV_CONTROL_TS)
            + (4.0f * gstInvLoop.disconnect_volt.wc) / CPU_INV_CONTROL_TS
            + gstInvLoop.disconnect_volt.w0 * gstInvLoop.disconnect_volt.w0;
    gstInvLoop.disconnect_volt.b1 = -8.0f / (CPU_INV_CONTROL_TS * CPU_INV_CONTROL_TS)
            + 2.0f * gstInvLoop.disconnect_volt.w0 * gstInvLoop.disconnect_volt.w0;
    gstInvLoop.disconnect_volt.b2 = 4.0f / (CPU_INV_CONTROL_TS * CPU_INV_CONTROL_TS)
            - (4.0f * gstInvLoop.disconnect_volt.wc) / CPU_INV_CONTROL_TS
            + gstInvLoop.disconnect_volt.w0 * gstInvLoop.disconnect_volt.w0;
    gstInvLoop.disconnect_volt.a0 = gstInvLoop.disconnect_volt.a0 / gstInvLoop.disconnect_volt.b0;
    gstInvLoop.disconnect_volt.a1 = gstInvLoop.disconnect_volt.a1 / gstInvLoop.disconnect_volt.b0;
    gstInvLoop.disconnect_volt.a2 = gstInvLoop.disconnect_volt.a2 / gstInvLoop.disconnect_volt.b0;
    gstInvLoop.disconnect_volt.b1 = gstInvLoop.disconnect_volt.b1 / gstInvLoop.disconnect_volt.b0;
    gstInvLoop.disconnect_volt.b2 = gstInvLoop.disconnect_volt.b2 / gstInvLoop.disconnect_volt.b0;
    gstInvLoop.disconnect_volt.Umax = INV_CUR_MAX;
    gstInvLoop.disconnect_volt.Umin = -1.0f * INV_CUR_MAX;
    gstInvLoop.en_inv = 0;

    SinglePhasePLLInitCpu(&gstGrid.volt);
    SinglePhasePLLInitCpu(&gstGrid.curr);
    SinglePhasePLLInitCpu(&gstInv.volt);
    SinglePhasePLLInitCpu(&gstInv.curr);
    SinglePhasePLLInitCpu(&gstGenerator.volt);
    SinglePhasePLLInitCpu(&gstGenerator.curr);
    SinglePhasePLLInitCpu(&gstCT.curr);
    SinglePhasePLLInitCpu(&gstNormalLoad.curr);
    SinglePhasePLLInitCpu(&gstBackUp.curr);

    InitForwardPiLoopCpu(&gstVoltageLoop, 0.02f, 0.005f, 20.0f, INV_CUR_MAX);
    InitForwardPiLoopCpu(&gstCurrentLoop, 0.5f, 0.01f, 10.0f, INV_VOL_MAX);
    geInvControlState = INV_STATE_SELFTEST;
    gstInv.volt.value.instant = 0.0f;
    gstInv.curr.value.instant = 0.0f;

    EPwm1Regs.TZCLR.bit.OST = 1;
    EPwm1Regs.TZOSTCLR.bit.OST1 = 1;

    gstSinCosSignal.sin_ext[0] = 0.0f;
    gstSinCosSignal.sin_ext[1] = 0.0f;
    gstSinCosSignal.cos_ext[0] = 1.0f;
    gstSinCosSignal.cos_ext[1] = 1.0f;
    gstSinCosSignal.theta_ext = 0.0f;
    gstSinCosSignal.sin_eff[0] = 0.0f;
    gstSinCosSignal.sin_eff[1] = 0.0f;
    gstSinCosSignal.cos_eff[0] = 1.0f;
    gstSinCosSignal.cos_eff[1] = 1.0f;
    gstSinCosSignal.theta_eff = 0.0f;
    gstSinCosSignal.wn_eff = INV_RADIAN_FREQ;
    gstSinCosSignal.wn_disconnect = INV_RADIAN_FREQ;

    gInvCurrentPhase = 0.0f;
    gInvCurrentRefAmp = 0.0f;
    gInvCurrAmpPI = 0.0f;
    gInvCurrThetaPI = 0.0f;
    gInvIdPI = 0.0f;
    gInvIqPI = 0.0f;
    gDuty_err = 0.0f;

    RmsPwrCpuInit(&gstInvData, CPU_PWR_COMPUTE_FREQ, CPU_PWR_LOWEST_AC_FREQ, CPU_PWR_HIGHEST_AC_FREQ,
            CPU_PWR_COMPUTE_AC_PERIOD);
    RmsPwrCpuInit(&gstBackUpData, CPU_PWR_COMPUTE_FREQ, CPU_PWR_LOWEST_AC_FREQ, CPU_PWR_HIGHEST_AC_FREQ,
            CPU_PWR_COMPUTE_AC_PERIOD);
    RmsPwrCpuInit(&gstGeneratorData, CPU_PWR_COMPUTE_FREQ, CPU_PWR_LOWEST_AC_FREQ, CPU_PWR_HIGHEST_AC_FREQ,
            CPU_PWR_COMPUTE_AC_PERIOD);
    RmsPwrCpuInit(&gstGridData, CPU_PWR_COMPUTE_FREQ, CPU_PWR_LOWEST_AC_FREQ, CPU_PWR_HIGHEST_AC_FREQ,
            CPU_PWR_COMPUTE_AC_PERIOD);
    RmsPwrCpuInit(&gstCTData, CPU_PWR_COMPUTE_FREQ, CPU_PWR_LOWEST_AC_FREQ, CPU_PWR_HIGHEST_AC_FREQ,
            CPU_PWR_COMPUTE_AC_PERIOD);
    RmsPwrCpuInit(&gstNormalLoadData, CPU_PWR_COMPUTE_FREQ, CPU_PWR_LOWEST_AC_FREQ, CPU_PWR_HIGHEST_AC_FREQ,
            CPU_PWR_COMPUTE_AC_PERIOD);

    gClaInitFinish = 1.0f;
}

void ClaControlCpuRunStateSync(void)
{
    switch (geInvControlStateCla)
    {
    case INV_STATE_STANDBY:
        if (geInvControlState == INV_STATE_SELFTEST)
        {
            geInvControlStateCla = INV_STATE_SELFTEST;
        }
        break;

    case INV_STATE_SELFTEST:
        if (geInvControlState == INV_STATE_OPENING)
        {
            gstInvLoop.en_inv = 1;
            geInvControlStateCla = INV_STATE_OPENING;
        }
        else if (geInvControlState == INV_STATE_DISCONNECTED)
        {
            gstInvLoop.en_inv = 1;
            gInvDisconnectCount = DISCONNECTCOUNTTRIGG;
            geInvControlStateCla = INV_STATE_DISCONNECTED;
        }
        else if (geInvControlState != INV_STATE_SELFTEST)
        {
            geInvControlStateCla = INV_STATE_STANDBY;
        }
        break;

    case INV_STATE_OPENING:
        if (geInvControlState != INV_STATE_OPENING)
        {
            geInvControlStateCla = INV_STATE_STANDBY;
        }
        break;

    case INV_STATE_SOFTSTARTING:
        break;

    case INV_STATE_DISCONNECTED:
        if (geInvControlState == INV_STATE_GRID_CONNECTING)
        {
            geInvControlStateCla = INV_STATE_GRID_CONNECTING;
        }
        else if (geInvControlState == INV_STATE_GEN_CONNECTING)
        {
            geInvControlStateCla = INV_STATE_GEN_CONNECTING;
        }
        else if (geInvControlState != INV_STATE_DISCONNECTED)
        {
            geInvControlStateCla = INV_STATE_STANDBY;
        }
        break;

    case INV_STATE_GRID_CONNECTING:
        if (geInvControlState == INV_STATE_GRID_CONNECTED)
        {
            gstCurrentLoop.Kp = 5.5f;
            // gstCurrentLoop.Ki = 0.0f;
            geInvControlStateCla = INV_STATE_GRID_CONNECTED;
        }
        else if (geInvControlState == INV_STATE_DISCONNECTED)
        {
            gInvDisconnectCount = DISCONNECTCOUNTTRIGG;
            geInvControlStateCla = INV_STATE_DISCONNECTED;
        }
        else if (geInvControlState != INV_STATE_GRID_CONNECTING)
        {
            geInvControlStateCla = INV_STATE_STANDBY;
        }
        break;

    case INV_STATE_GRID_CONNECTED:
        if (geInvControlState == INV_STATE_GRID_DISCONNECTING)
        {
            gInvCurrentPhase = 0.0f;
            gInvCurrAmpPI = 0.0f;
            gInvCurrThetaPI = 0.0f;
            gInvIdPI = 0.0f;
            gInvIqPI = 0.0f;
            geInvControlStateCla = INV_STATE_GRID_DISCONNECTING;
        }
        else if (geInvControlState != INV_STATE_GRID_CONNECTED)
        {
            geInvControlStateCla = INV_STATE_STANDBY;
        }
        break;

    case INV_STATE_GRID_DISCONNECTING:
        if (geInvControlState == INV_STATE_DISCONNECTED)
        {
            gstCurrentLoop.Kp = 0.5f;
            // gstCurrentLoop.Ki = 0.0f;
            gInvDisconnectCount = 0.0f;
            geInvControlStateCla = INV_STATE_DISCONNECTED;
        }
        else if (geInvControlState != INV_STATE_GRID_DISCONNECTING)
        {
            geInvControlStateCla = INV_STATE_STANDBY;
        }
        break;

    case INV_STATE_GEN_CONNECTING:
        if (geInvControlState == INV_STATE_GEN_CONNECTED)
        {
            gstCurrentLoop.Kp = 5.5f;
            // gstCurrentLoop.Ki = 0.0f;
            geInvControlStateCla = INV_STATE_GEN_CONNECTED;
        }
        else if (geInvControlState == INV_STATE_DISCONNECTED)
        {
            gInvDisconnectCount = DISCONNECTCOUNTTRIGG;
            geInvControlStateCla = INV_STATE_DISCONNECTED;
        }
        else if (geInvControlState != INV_STATE_GEN_CONNECTING)
        {
            geInvControlStateCla = INV_STATE_STANDBY;
        }
        break;

    case INV_STATE_GEN_CONNECTED:
        if (geInvControlState == INV_STATE_GEN_DISCONNECTING)
        {
            gInvCurrentPhase = 0.0f;
            gInvCurrAmpPI = 0.0f;
            gInvCurrThetaPI = 0.0f;
            gInvIdPI = 0.0f;
            gInvIqPI = 0.0f;
            geInvControlStateCla = INV_STATE_GEN_DISCONNECTING;
        }
        else if (geInvControlState != INV_STATE_GEN_CONNECTED)
        {
            geInvControlStateCla = INV_STATE_STANDBY;
        }
        break;

    case INV_STATE_GEN_DISCONNECTING:
        if (geInvControlState == INV_STATE_DISCONNECTED)
        {
            gstCurrentLoop.Kp = 0.5f;
            // gstCurrentLoop.Ki = 0.0f;
            gInvDisconnectCount = 0.0f;
            geInvControlStateCla = INV_STATE_DISCONNECTED;
        }
        else if (geInvControlState != INV_STATE_GEN_DISCONNECTING)
        {
            geInvControlStateCla = INV_STATE_STANDBY;
        }
        break;

    default:
        geInvControlStateCla = INV_STATE_STANDBY;
        break;
    }
}

