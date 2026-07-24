

#ifndef __DEV_LED_H_
#define __DEV_LED_H_

#include "bsp.h"
#include "systick.h"
//#include "queue.h"
#include "gpio.h"
#include "timer.h"


typedef enum
{
    LED_SRC_SYS,
    LED_SRC_PCS,
    LED_SRC_CONN,
    LED_SRC_NUM
}EN_LED_SRC;

typedef enum
{
    LED_HW_GPIO,
    LED_HW_PWM
}EN_LED_HW_MODE;

typedef enum
{
    LED_HW_ACTIVE_HIGH,
    LED_HW_ACTIVE_LOW
}EN_LED_HW_ACTIVE_LEVEL;

//typedef enum
//{
//    POWER_LED_MODE_OFF,
//    POWER_LED_MODE_ON,
//    POWER_LED_MODE_BLINK
//}EN_POWER_LED_MODE;


typedef enum
{
    LED_STATUS_IDLE,
    LED_STATUS_OFF,
    LED_STATUS_ON,
    LED_STATUS_BLINK,
    LED_STATUS_BREATHING,
    LED_STATUS_SOS
}EN_LED_STATUS;



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
}LedInfo_t;



void DevLedInit();
void DevLedProcess();


void LedCtlOn(EN_LED_SRC src);
void LedCtlOff(EN_LED_SRC src);
void LedCtlBlink(EN_LED_SRC src, uint32_t period);

void LedDirectOn(EN_LED_SRC src);
void LedDirectOff(EN_LED_SRC src);

#endif














