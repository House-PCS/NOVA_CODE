/*
 * ph3_sts_ifc_update.h
 *
 * CPU1 read-side STS interface (data produced by CPU2, CPU2TOCPU1RAM shared).
 * NOTE: Declarations only; implementation intentionally left empty for now.
 */
#ifndef APP_PH3_STS_IFC_UPDATE_H_
#define APP_PH3_STS_IFC_UPDATE_H_

#include <stdint.h>
#include <stdbool.h>
#include "F28x_Project.h"
#include "AlgoFw/package/cntl/sts/sts_interface.h"
#include "AlgoFw/package/cntl/sts/sts_adjust.h"
#include "dev_dual_core_receive.h"

typedef struct sts_input_status_stru_t
{
    volatile uint16_t sts_run_en;
    volatile uint16_t force_gen_mode;
    volatile uint16_t en_connect;
    volatile uint16_t en_disconnect;
    
    volatile uint16_t warn_clear;
    volatile uint16_t err_clear;
} StsInputStatus_t;



volatile StsInputStatus_t *StsEnUpdate_Get(void);

void Ph3StsInfo_Init(void);
void SetStsEnRunEn(uint8_t sts_run_en);
void SetStsEnForceGenMode(uint8_t force_gen_mode);
void SetStsEnEnConnect(uint8_t en_connect);
void SetStsEnEnDisconnect(uint8_t en_disconnect);
void SetStsEnWarnClear(uint8_t warn_clear);
void SetStsEnErrClear(uint8_t err_clear);

void StsOutputUpdate(void);
uint8_t Get_StsOutput_RelagState(void);
uint8_t Get_StsOutput_IsDisconnectd(void);
uint8_t Get_StsOutput_IsDisconnecting(void);
uint8_t Get_StsOutput_IsConnecting(void);
uint8_t Get_StsOutput_IsConnected(void);
EN_STS_STATE Get_StsOutput_State(void);
uint8_t Get_StsOutput_IsWarn(void);
uint8_t Get_StsOutput_IsErr(void);
const Sts_Err_Un_t *Get_StsOutput_Err(void);
const Sts_Warn_Un_t *Get_StsOutput_Warn(void);

uint8_t  GetStsGenForceState(void);
float32 Get_StsOutput_VolRmsAdjust(void);
float32 Get_StsOutput_wAdjust(void);


uint8_t GetStsEnRunEn(void);
uint8_t GetStsEnForceGenMode(void);

#endif /* APP_PH3_STS_IFC_UPDATE_H_ */
