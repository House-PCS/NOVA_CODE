/**
 * @file relay_ctrl.c
 * @brief Relay control and self-test
 */

#include "relay_ctrl.h"
#include "gpio.h"
#include <string.h>
#include "fault.h"
#include "debug_analysis.h"

static RelayInfo_t _gRelayCtrl[EN_RELAT_CTRL_TYPE_COUNT];
static RelaySelfTestInValue_t _gRelayTestInValue;
static RelaySelfTest_t _gRelaySelfTest;

/**
 * @brief Set relay hardware state and update open flag
 * @param relay_type Relay channel
 * @param state RELAY_ON or RELAY_OFF
 */
void RelayControl(EN_RELAT_CTRL_TYPE relay_type, uint8_t state)
{
    switch(relay_type)
    {
        case EN_RELAT_CTRL_TYPE_LOAD:
            if(state == RELAY_ON)
            {
                SetLoadRlyState(1);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_LOAD].open_flag = true;
            }
            else
            {
                SetLoadRlyState(0);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_LOAD].open_flag = false;
            }
            break;

        case EN_RELAT_CTRL_TYPE_GRID:
            if(state == RELAY_ON)
            {
                SetGridRlyState(1);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_GRID].open_flag = true;
            }
            else
            {
                SetGridRlyState(0);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_GRID].open_flag = false;
            }
            break;

        case EN_RELAT_CTRL_TYPE_INV:
            if(state == RELAY_ON)
            {
                SetInvMainRlyState(1);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_INV].open_flag = true;
            }
            else
            {
                SetInvMainRlyState(0);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_INV].open_flag = false;
            }
            break;

        case EN_RELAT_CTRL_TYPE_GEN:
            if(state == RELAY_ON)
            {
                SetGenRlyState(1);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag = true;
            }
            else
            {
                SetGenRlyState(0);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag = false;
            }
            break;

        case EN_RELAT_CTRL_TYPE_N_PE:
            if(state == RELAY_ON)
            {
                SetPERlyState(1);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_N_PE].open_flag = true;
            }
            else
            {
                SetPERlyState(0);
                _gRelayCtrl[EN_RELAT_CTRL_TYPE_N_PE].open_flag = false;
            }
            break;

        case EN_RELAT_CTRL_TYPE_GEN_START:
            if(state == RELAY_ON)
            {
                SetGenStarRlyState(1);
            }
            else
            {
                SetGenStarRlyState(0);
            }
            break;

        case EN_RELAT_CTRL_TYPE_EXC:
            if(state == RELAY_ON)
            {
                SetEXCRlyState(1);
            }
            else
            {
                SetEXCRlyState(0);
            }
            break;

        default:
            break;
    }
}

static void _RelaySelfTestInit(void)
{
    _gRelaySelfTest.retest_requested = false;
    _gRelaySelfTest.test_complete = 0;
    _gRelaySelfTest.selfTestState = STATE_IDLE;
}

/** Initialize all relays to OFF and reset self-test state */
void RelayControlInit(void)
{
    memset(_gRelayCtrl, 0, sizeof(_gRelayCtrl));
    for(uint8_t i = 0; i < EN_RELAT_CTRL_TYPE_COUNT; i++)
    {
        RelayControl((EN_RELAT_CTRL_TYPE)i, RELAY_OFF);
    }
//    RelayControl(EN_RELAT_CTRL_TYPE_GEN_START, RELAY_OFF);
//    RelayControl(EN_RELAT_CTRL_TYPE_EXC, RELAY_OFF);

    _RelaySelfTestInit();
}

RelaySelfTestInValue_t* GetRelaySelfInputValue(void)
{
    return &_gRelayTestInValue;
}

/**
 * @brief Relay stick / wiring self-test state machine
 * @note Call periodically from main loop
 */
void RelaySelfTest(void)
{
    static uint32_t timeout;
    static uint16_t cont = 0;
    static bool first_run_flag = true;

    if(first_run_flag)
    {
        timeout = TimGetSystime();
        first_run_flag = false;
    }

    if(_gRelaySelfTest.retest_requested && _gRelaySelfTest.selfTestState == RELAY_TEST_FAIL)
    {
        _RelaySelfTestInit();
        first_run_flag = true;
        return;
    }

    if(_gRelaySelfTest.test_complete != 0)
    {
        return;
    }

    switch(_gRelaySelfTest.selfTestState)
    {
        case STATE_IDLE:
        {
            if(TimDifftimeFromNow(timeout) < 200)
            {
                break;
            }

            if(_gRelayCtrl[EN_RELAT_CTRL_TYPE_GRID].open_flag)
            {
                RelayControl(EN_RELAT_CTRL_TYPE_GRID, RELAY_OFF);
            }
            if(_gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag)
            {
                RelayControl(EN_RELAT_CTRL_TYPE_GEN, RELAY_OFF);
            }
            RelayControl(EN_RELAT_CTRL_TYPE_LOAD, RELAY_OFF);

            timeout = TimGetSystime();
            _gRelaySelfTest.selfTestState = STATE_CHECK_GRID;
            cont = 2;
            break;
        }

        case STATE_CHECK_GRID:
        {
            if(TimDifftimeFromNow(timeout) < 1000)
            {
                break;
            }

            if(!_gRelayCtrl[EN_RELAT_CTRL_TYPE_GRID].open_flag && !_gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag)
            {
                if((VOLTAGE_IN_RANGE(_gRelayTestInValue.grid_a_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
                   && (VOLTAGE_IN_RANGE(_gRelayTestInValue.grid_b_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
                   && (VOLTAGE_IN_RANGE(_gRelayTestInValue.grid_c_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH)))
                {
                    /* Grid present: verify other nodes are de-energized before closing grid relay */
                    if(_gRelayTestInValue.mid_a_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.mid_b_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.mid_c_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Load_a_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Load_b_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Load_c_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Inv_a_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Inv_b_vol_rms <= GRID_VOLT_RMS_LOW
                       && _gRelayTestInValue.Inv_c_vol_rms <= GRID_VOLT_RMS_LOW)
                    {
                        cont++;
                    }
                    else
                    {
                        if(cont > 0)
                        {
                            cont--;
                        }
                    }

                    if(cont >= 5)
                    {
                        RelayControl(EN_RELAT_CTRL_TYPE_GRID, RELAY_ON);
                        _gRelaySelfTest.selfTestState = STATE_RELAY_CLOSE_TEST;
                        timeout = TimGetSystime();
                        cont = 2;
                    }
                    else if(cont == 0)
                    {
                        _gRelaySelfTest.test_complete = -1;
                        _gRelaySelfTest.selfTestState = RELAY_TEST_FAIL;
                    }
                }
                else
                {
                    _gRelaySelfTest.test_complete = 1;
                    _gRelaySelfTest.selfTestState = STATE_IDLE;
                }
            }
            else if(_gRelayCtrl[EN_RELAT_CTRL_TYPE_GRID].open_flag || _gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag)
            {
                _gRelaySelfTest.test_complete = 1;
                _gRelaySelfTest.selfTestState = STATE_IDLE;
                break;
            }
            break;
        }

        case STATE_RELAY_CLOSE_TEST:
        {
            if(_gRelayCtrl[EN_RELAT_CTRL_TYPE_GEN].open_flag)
            {
                _gRelaySelfTest.test_complete = 1;
                _gRelaySelfTest.selfTestState = STATE_IDLE;
                RelayControl(EN_RELAT_CTRL_TYPE_GRID, RELAY_OFF);
                break;
            }

            if(TimDifftimeFromNow(timeout) < 1000)
            {
                break;
            }

            /* Mid-point instant voltage should track grid; other ports must stay de-energized */
            if((fabsf(_gRelayTestInValue.mid_a_vol_instant - _gRelayTestInValue.grid_a_vol_instant) < 20.0f)
               && (fabsf(_gRelayTestInValue.mid_b_vol_instant - _gRelayTestInValue.grid_b_vol_instant) < 20.0f)
               && (fabsf(_gRelayTestInValue.mid_c_vol_instant - _gRelayTestInValue.grid_c_vol_instant) < 20.0f)
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Gen_a_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Gen_b_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Gen_c_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Load_a_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Load_b_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Load_c_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Inv_a_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Inv_b_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH))
               && (!VOLTAGE_IN_RANGE(_gRelayTestInValue.Inv_c_vol_rms, GRID_VOLT_RMS_LOW, GRID_VOLT_RMS_HIGH)))
            {
                cont++;
            }
            else
            {
                if(cont > 0)
                {
                    cont--;
                }
            }

            if(cont >= 5)
            {
                _gRelaySelfTest.test_complete = 1;
                RelayControl(EN_RELAT_CTRL_TYPE_GRID, RELAY_OFF);
                _gRelaySelfTest.selfTestState = STATE_IDLE;
                cont = 0;
            }
            else if(cont == 0)
            {
                _gRelaySelfTest.test_complete = -1;
                _gRelaySelfTest.selfTestState = RELAY_TEST_FAIL;
            }
            break;
        }

        default:
            _gRelaySelfTest.selfTestState = STATE_IDLE;
            break;
    }
}

/** @return Self-test result: 0 = in progress, 1 = pass, -1 = fail */
int16 GetRelaySelfTestResult(void)
{
    return _gRelaySelfTest.test_complete;
}

int16 GetRelayState(EN_RELAT_CTRL_TYPE relay_type)
{
    if(relay_type >= EN_RELAT_CTRL_TYPE_COUNT)
    {
        return 0;
    }
    return _gRelayCtrl[relay_type].open_flag;
}

void RelayRetestRequestSet(bool value)
{
    _gRelaySelfTest.retest_requested = value;
}
