/**
 * @file drm_pwr_sched.h
 * @brief DRM 功率调度
 */

#ifndef DRM_PWR_SCHED_H
#define DRM_PWR_SCHED_H

#include "energy_management_def.h"

typedef enum
{
    EN_DRM_DRM0 = 0,
    EN_DRM_DRM1,
    EN_DRM_DRM2,
    EN_DRM_DRM3,
    EN_DRM_DRM4,
    EN_DRM_DRM5,
    EN_DRM_DRM6,
    EN_DRM_DRM7,
    EN_DRM_DRM8,
} DrmPwrSched_State_t;

typedef struct drm_pwr_sched_output_stru_t
{
    float32 input_rated_apparent_pwr;   /**< 电网输入额定视在功率 (VA) */
    float32 output_rated_apparent_pwr;  /**< 电网输出额定视在功率 (VA) */
    uint16_t drm_relay_control;         /**< DRM 并网使能：1 使能，0 禁止 */
} DrmPwrSched_Output_t;

extern DrmPwrSched_Output_t gstDrmPwrSchedOut;

void DrmPwrSched_Init(void);
void DrmPwrSched_Update(void);

#endif // DRM_PWR_SCHED_H
