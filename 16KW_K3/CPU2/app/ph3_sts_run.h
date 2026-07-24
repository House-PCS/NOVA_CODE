/*
 * ph3_sts_run.h
 *
 * CPU2 three-phase status runtime.
 */
#ifndef APP_PH3_STS_RUN_H_
#define APP_PH3_STS_RUN_H_

#include <stdint.h>
#include <stdbool.h>
#include "F28x_Project.h"
#include "AlgoFw/package/cntl/sts/sts_interface.h"
#include "AlgoFw/package/cntl/sts/sts_adjust.h"



/* STS algorithm framework interfaces (external AlgoFw) */

typedef struct sts_output_status_stru_t
{
    Sts_Output_t sts_output;
    Sts_Err_Un_t sts_err;
    Sts_Warn_Un_t sts_warn;
    EN_STS_STATE  sts_state;
    uint8_t  sts_is_disconnectd;
    uint8_t  sts_is_disconnecting;
    uint8_t  sts_is_connecting;
    uint8_t  sts_is_connected;
    uint8_t  sts_is_gen_mode;
} StsOutputStatus_t;





void Ph3_Sts_Init(void);
void Ph3_Sts_Run(void);
StsOutputStatus_t *StsOutputUpdate_Get(void);
void Ph3_Sts_FlagUpdate(void);
void Ph3_Sts_DataSlowUpdate(void);
#endif /* APP_PH3_STS_RUN_H_ */

