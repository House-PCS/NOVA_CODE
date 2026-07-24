

#ifndef __DEV_LED_H_
#define __DEV_LED_H_

#include "bsp.h"
#include "systick.h"
//#include "queue.h"
#include "gpio.h"
#include "timer.h"


#define LED_BLINK_TIME_NORMAL               (1000)  /* 正常模式led闪烁周期 （单次闪烁）*/
#define LED_BLINK_TIME_ENDURANCE_ERROR      (4000)  /* 耐久度测试故障led闪烁周期 （连闪两次）*/
#define LED_BLINK_TIME_AGING                (2000)  /* 老化模式led闪烁周期 （连闪两次）*/
#define LED_BLINK_TIME_TOOL                 (2000)  /* 工装模式led闪烁周期 （连闪三次）*/
#define LED_BLINK_TIME_UPGRAD               (500)   /* 升级中led闪烁周期   （单次闪烁）*/
#define LED_BLINK_TIME_CLOSE                (50)    /* 关机led闪烁周期     （单次闪烁）*/

typedef enum
{
    LED_SRC_SYS,
    LED_SRC_NUM
} EN_LED_SRC;

typedef enum
{
    LED_HW_GPIO,
    LED_HW_PWM
} EN_LED_HW_MODE;

typedef enum
{
    LED_HW_ACTIVE_HIGH,
    LED_HW_ACTIVE_LOW
} EN_LED_HW_ACTIVE_LEVEL;

typedef enum
{
    LED_STATUS_IDLE,
    LED_STATUS_OFF,
    LED_STATUS_ON,
    LED_STATUS_BLINK,
    LED_STATUS_BREATHING,
    LED_STATUS_STANDBY,
    LED_STATUS_SUCCEED,
    LED_STATUS_FAILED,
    LED_STATUS_AGING,
    LED_STATUS_TOOL,
    LED_STATUS_BLINK_FOUR,
    LED_STATUS_SOS
} EN_LED_STATUS;



typedef struct _led_info_t
{
    EN_LED_SRC              led_src;
    EN_LED_HW_MODE          led_hw_mode;
    EN_LED_HW_ACTIVE_LEVEL  led_active_level;
    EN_LED_STATUS           led_status;
    uint32_t                led_status_period;//主要针对呼吸，闪烁等持续变化状态，表示一个循环需要多长时间
    uint8_t                 led_brightness;//最大100

    Gpio_HandleType         *p_gpio_Handle;
    Timer_HandleType        *p_timer_Handle;
    uint8_t                  tim_ch;//针对PWM控制的LED，对应定时器哪个通道，GPIO控制的LED该参数不关心
} LedInfo_t;


void DevLedInit(void);
void DevLedProcess(void);


void LedCtlOn(EN_LED_SRC src);
void LedCtlOff(EN_LED_SRC src);
void LedCtlBlink(EN_LED_SRC src, uint32_t period);
void LedCtlStandby(EN_LED_SRC src);
void LedCtlSucceed(EN_LED_SRC src);
void LedCtlFailed(EN_LED_SRC src);
void LedCtlTool(EN_LED_SRC src);
void LedCtlAging(EN_LED_SRC src);
void LedBlinkSet(EN_LED_SRC src, uint8_t blink_count ,uint16_t period);
void LedCtlEnduranceTest(EN_LED_SRC src);
//void Alarm();

#endif














