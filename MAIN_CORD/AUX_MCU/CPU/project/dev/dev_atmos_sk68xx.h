#ifndef _DEV_ATMOS_SK68XX_H_
#define _DEV_ATMOS_SK68XX_H_

#include "bsp.h"

typedef enum
{
    EN_ATMOS_CH_1,  
    EN_ATMOS_CH_2,	
    EN_ATMOS_CH_3,	
    EN_ATMOS_CH_4,	//按键背光
	EN_ATMOS_CH_5,  //侧灯板左
	EN_ATMOS_CH_6,	//侧灯板右
    EN_ATMOS_CH_NUM
} EN_ATMOS_CHx;


#define ATMOS_CH_1_NUM        			  (8)
#define ATMOS_CH_2_NUM       			  (8)
#define ATMOS_CH_3_NUM       			  (8)
#define ATMOS_CH_4_NUM       			  (4)
#define ATMOS_CH_5_NUM					  (21)
#define ATMOS_CH_6_NUM					  (21)

#define ATMOS_PER_CH_MAX_NUM              (28)  //(7)


#define ATMOS_MAX_COLOR_CHANGE_NUM        (11)

typedef enum
{
    EN_ATMOS_EFFECT_TYPE_OFF,
    EN_ATMOS_EFFECT_TYPE_ON,
    EN_ATMOS_EFFECT_TYPE_BLINK,
    EN_ATMOS_EFFECT_TYPE_BREATH,
    EN_ATMOS_EFFECT_TYPE_IDLE,//不处理模式，对于常亮或长灭，执行一次就不再重复执行，可跳入该模式下
    EN_ATMOS_EFFECT_TYPE_HALF_ON,
    EN_ATMOS_EFFECT_TYPE_SWITCH_ON,
    EN_ATMOS_EFFECT_TYPE_SWITCH_ON_RESTE,
    EN_ATMOS_EFFECT_TYPE_THIRD_BREATH,
    EN_ATMOS_EFFECT_TYPE_SINGLE_ON,
    EN_ATMOS_EFFECT_TYPE_START_BREATH,
    EN_ATMOS_EFFECT_TYPE_GRADIENT,//渐变
} EN_ATMOS_EFFECT_TYPE;



typedef struct _atmos_cfg_stru_t
{
    EN_ATMOS_EFFECT_TYPE    effect_type;
    uint8_t                 led_num;
    uint8_t                 color_change_num;//针对存在多种颜色切换的场景
    uint8_t                 color_index;//颜色切换计数
    uint8_t                 effect_brightness_duty;//亮度调节，默认是100%
    uint32_t                data[ATMOS_MAX_COLOR_CHANGE_NUM][ATMOS_PER_CH_MAX_NUM];
    uint32_t                effect_period;//单位ms,对于长亮，表示切换颜色的时间周期，对于闪烁时间周期，一亮一灭算一个周期，对于呼吸从最暗到最亮，再到最暗算一个周期，对于渐变，从起始亮度到结束亮度算一个周期
    uint32_t                effect_time_count;
    uint8_t                 effect_one_color_times;//针对闪烁和呼吸，每种颜色执行次数
    uint8_t                 effect_one_color_times_count;//针对闪烁和呼吸，每种颜色执行次数计数
    bool                    effect_single_mode;//特效是否循环展示，如果为true，则执行完停留在最后一个画面
    uint8_t                 effect_gradient_start_brightness_duty;//仅仅用于渐变特效
    uint8_t                 effect_gradient_end_brightness_duty;//仅仅用于渐变特效
    uint8_t                 effect_master_control_mode; //是否有上位机控制其整周期 如果为ture，则使用上位机的周期
    uint8_t                 effect_period_status;    //上位机周期上下沿变换的状态
} DevAtmosCfg_t;

extern DevAtmosCfg_t    garrDevAtmosCfg[];

void DevAtmosInit(void);
void DevAtmosLedProcess(void);



#endif


