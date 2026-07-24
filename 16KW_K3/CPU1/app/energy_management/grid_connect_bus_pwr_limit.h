/**
 * @file grid_connect_bus_pwr_limit.h
 * @brief 并网母线电压有功功率限制
 */

#ifndef GRID_CONNECT_BUS_PWR_LIMIT_H
#define GRID_CONNECT_BUS_PWR_LIMIT_H

#include "energy_management_def.h"

typedef enum
{
    EN_GRID_BUS_PWR_CTRL_NORMAL = 0,  /**< 理论带：直通功率设定 */
    EN_GRID_BUS_PWR_CTRL_HOLD,        /**< 过渡带：保持 / 收紧功率 */
    EN_GRID_BUS_PWR_CTRL_PI_CHG,      /**< 过压保护：PI 限制正向功率 */
    EN_GRID_BUS_PWR_CTRL_PI_DSG,      /**< 欠压保护：PI 限制负向功率 */
} EN_GRID_BUS_PWR_CTRL;


typedef struct grid_bus_pwr_limit_output_stru_t
{
    float32 inv_pwr_limited;          /**< 限幅后并网有功功率 (W) */
} GridBusPwrLimit_Output_t;

extern GridBusPwrLimit_Output_t gstGridBusPwrLimitOut;

void GridBusPwrLimit_Init(void);
void GridBusPwrLimit_Update(float32 bus_pwr_derate_in);
float32 GridBusPwrLimit_GridPwr(float32 inv_pwr, float32 load_active_pwr);
#endif // GRID_CONNECT_BUS_PWR_LIMIT_H
