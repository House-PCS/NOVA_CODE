/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_led.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-22
  * @brief       GD32F450
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __DEV_LED_H_
#define __DEV_LED_H_

#include "mcu_bsp.h"
#include "systick.h"



typedef enum
{
    LED_SRC_DEBUG,
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


#endif




