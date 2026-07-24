
#ifndef __DEV_LED_H_
#define __DEV_LED_H_

#include "bsp.h"
#include "systick.h"
//#include "queue.h"
#include "gpio.h"
#include "timer.h"
#define     LED_SCAN_TIME   25UL//25MS
#define     LED_SOS_TIME_BASE       100UL/25
#define     LED_SOC_T       400UL/25
#define     LED_BLINK_T     1000UL/25
#define     LED_BREATH_T    2000UL/25
#define     LED_TEST_MODE_BASE  2000UL/25
#define     LED_TYPE_MAX    6
typedef enum
{
	LED_Run1,
	LED_Run2,
	LED_Run3,
	LED_Run4,
	LED_Run5,
	LED_Run6,
    LED_RUN_LED, 
	LED_SRC_ATMOS,
    LED_SRC_NUM
}EN_LED_SRC;

typedef enum
{
    LED_GPIO=0,//gpio驱动led的亮灭
    LED_HW_PWM,//pwm驱动灯的亮灭
    LED_DRIVE_IC,//驱动IC控制LED
}EN_LED_HW_MODE;

typedef enum
{
    LED_HW_ACTIVE_HIGH,
    LED_HW_ACTIVE_LOW
}EN_LED_HW_ACTIVE_LEVEL;//灯的亮度

//typedef enum
//{
//    POWER_LED_MODE_OFF,
//    POWER_LED_MODE_ON,
//    POWER_LED_MODE_BLINK
//}EN_POWER_LED_MODE;


//typedef enum
//{
//    LED_STATUS_IDLE,
//    LED_STATUS_OFF,
//    LED_STATUS_ON,
//    LED_STATUS_BLINK,
//    LED_STATUS_BREATHING_SINGLE,
//    LED_STATUS_SOS,
//    LED_STATUS_BREATHING_MULTI,
//    LED_STATUS_TURN_0N,
//    LED_STATUS_TEST_MODE,
//}EN_LED_STATUS;//LED的状态


typedef enum
{
    LED_STATUS_IDLE,
    LED_STATUS_NORMAL,
    LED_STATUS_UPGRADE
}EN_LED_STATUS;//LED的状态


typedef struct _led_info_t
{
    EN_LED_SRC              led_src;
    EN_LED_HW_MODE          led_dirver_mode;
    EN_LED_HW_ACTIVE_LEVEL  led_active_level;
    EN_LED_STATUS           led_status;
    uint32_t                led_status_period;//主要针对呼吸，闪烁等持续变化状态，表示一个循环需要多长时间
    uint32_t                led_time_count;
    uint8_t                 led_brightness;//最大100
	uint8_t                 sos_dptr;
    
    Gpio_HandleType         *pGpio_Handle;
    Timer_HandleType        *pTimer_Handle;
    uint8_t                  tim_ch;//针对PWM控制的LED，对应定时器哪个通道，GPIO控制的LED该参数不关心
	
}LedInfo_t;

typedef   void (*LedCartoonOverCallBack)(void);
void McuLedBspInit(void);
void LedProcess(void);
void LedInit(void);
void MCULedProcess(void);

void LedTimeSetFunc(EN_LED_STATUS status);
//void AtmosLampSet(EN_LED_STATUS status,  uint8_t brightness);
//void SetLedClour(uint8_t r_data,uint8_t g_data,uint8_t b_data,uint8_t set_led_level);

//void AtLed1LampSet(EN_LED_STATUS status,  uint8_t brightness);
//void AtLed2LampSet(EN_LED_STATUS status,  uint8_t brightness);
//void AtLed3LampSet(EN_LED_STATUS status,  uint8_t brightness);
//void AtLed4LampSet(EN_LED_STATUS status,  uint8_t brightness);
//void AtLed5LampSet(EN_LED_STATUS status,  uint8_t brightness);
//void AtLed6LampSet(EN_LED_STATUS status,  uint8_t brightness);
//void AtRunLedLampSet(EN_LED_STATUS status,  uint8_t brightness);

#endif







