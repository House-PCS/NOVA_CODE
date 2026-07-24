#ifndef ALGORITHM_INTERFACE_H
#define ALGORITHM_INTERFACE_H
#include "F2837xD_device.h"

typedef struct algorithm_power_stru_t
{
    float pwr_grid;
    float pwr_inv;
    float pwr_load;
    float pwr_bat;
    float pwr_pv1;
    float pwr_pv2;

} AlgPwr_t;
#define VRMS_VV2 220.0f
#define VRMS_VV3 240.0f
#define FREQ_ULCO 50.25f
#define FREQ_LLCO 49.75f
void AlgoSendStateWave();
void AlgoInit();
void AlgoCntl();
void AlgoSetGridPwr(float pwr);
void AlgoSetReactivePwr(float pwr);
int16_t  GridDirGet(void);
int16_t CtDirGet(void);
void* AlgoGetStateWave();

extern float gLoadPower;
extern float gBatpower;
extern float grid_power;
#endif
