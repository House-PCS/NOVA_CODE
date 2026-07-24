/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_sgm58031.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-09
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09     <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __DEV_SGM58031_H_
#define __DEV_SGM58031_H_


#include "mcu_bsp.h"


//SGM58031内部寄存器地址
#define CONVERSION_REG_ADDR                  0x00   //AD值转换寄存器，16bit数据，默认值0x0000，只读
#define CONFIG_REG_ADDR                      0x01   //配置寄存器,默认0x8583，可读可写
#define LO_THRESH_REG_ADDR                   0x02   //比较器阈值下限，默认0x8000
#define HI_THRESH_REG_ADDR                   0x03   //比较器阈值上限，默认0x7FFF
#define CONFIG1_REG_ADDR                     0x04   //扩展配置寄存器，默认0x0000
#define CHIP_ID_REG_ADDR                     0x05   //芯片ID，默认0x0080
#define GN_TRIM1_REG_ADDR                    0x06   //增益修正，默认0x03FA	


typedef enum 
{
    SGM_ADDR_GND = 0x90,
    SGM_ADDR_VDD = 0x92,
    SGM_ADDR_SDA = 0x94,
    SGM_ADDR_SCL = 0x96,
}SGM_ADDR;


/*config寄存器*/
typedef union 
{
	uint16_t  reg_value;
	struct
    {
        uint16_t    comp_que:2;
		uint16_t    comp_lat:1;
		uint16_t    comp_pol:1;
		uint16_t    comp_mode:1;
		uint16_t    dr:3;
		uint16_t    mode:1;
		uint16_t    pga:3;
		uint16_t    mux:3;
		uint16_t    os:1;
	}reg;
}Sgm58031ConfigReg_t;

/*config1寄存器,预留*/
typedef union
{
    uint16_t  reg_value;
    struct
    {
        uint16_t    reserved1:3;
        uint16_t    ext_ref:1;
        uint16_t    bus_flex:1;
        uint16_t    reserved2:1;
        uint16_t    burnout:1;
        uint16_t    dr_sel:1;
        uint16_t    pd:1;
        uint16_t    reserved3:7;
    }reg;
}Sgm58031Config1Reg_t;


typedef enum 
{
    EN_SGM58031_COMP_QUE_AFT_ONE,//一次转换后生效
    EN_SGM58031_COMP_QUE_AFT_TOW,//两次转换后生效
    EN_SGM58031_COMP_QUE_AFT_FOUR,//四次转换后生效
    EN_SGM58031_COMP_QUE_DISABLE,//禁用比较器
}EN_SGM58031_COMP_QUE;


//比较器锁存
//该位控制ALERT / RDY引脚在置位后是锁存还是在转换后的阈值上限和下限范围内清零
typedef enum 
{
    EN_SGM58031_COMP_LAT_NON,//非锁存比较器
    EN_SGM58031_COMP_LAT_ON,//锁存比较器
}EN_SGM58031_COMP_LAT;

//比较器极性
//该位控制ALERT / RDY引脚的极性
typedef enum 
{
    EN_SGM58031_COMP_POL_LOW,//低电平有效
    EN_SGM58031_COMP_POL_HIGH,//高电平有效
}EN_SGM58031_COMP_POL;

//比较器模式
//该位配置比较器工作模式
typedef enum 
{
    EN_SGM58031_COMP_MODE_TRAD,//传统比较器
    EN_SGM58031_COMP_MODE_WINDOW,//窗口比较器
}EN_SGM58031_COMP_MODE;

typedef enum 
{
    EN_SGM58031_DR_6P25_7P5,//采样频率，单位Hz
    EN_SGM58031_DR_12P5_15,
    EN_SGM58031_DR_25_30,
    EN_SGM58031_DR_50_60,
    EN_SGM58031_DR_100_120,
    EN_SGM58031_DR_200_240,
    EN_SGM58031_DR_400_480,
    EN_SGM58031_DR_800_960,
}EN_SGM58031_DR;


typedef enum 
{
    EN_SGM58031_MODE_CONTINUOUS_MODE,
    EN_SGM58031_MODE_SINGLE_MODE
}EN_SGM58031_MODE;

typedef enum 
{
    EN_SGM58031_PGA_6144,
    EN_SGM58031_PGA_4096,
    EN_SGM58031_PGA_2048,
    EN_SGM58031_PGA_1024,
    EN_SGM58031_PGA_512,
    EN_SGM58031_PGA_256
}EN_SGM58031_PGA;

typedef enum 
{
	EN_SGM58031_MUX_AIN0_AIN1= 0,	//MUX选通的连接到PGA的引脚为AIN0和AIN1(差分输入)，默认项
	EN_SGM58031_MUX_AIN0_AIN3,
    EN_SGM58031_MUX_AIN1_AIN3,
    EN_SGM58031_MUX_AIN2_AIN3,
    EN_SGM58031_MUX_AIN0_GND,
    EN_SGM58031_MUX_AIN1_GND,
    EN_SGM58031_MUX_AIN2_GND,
    EN_SGM58031_MUX_AIN3_GND,
}EN_SGM58031_MUX;


typedef enum 
{
	EN_SGM58031_OS_NO_EFFECT = 0,			    //无效
	EN_SGM58031_OS_BEGIN_A_SINGLE_CONVERSION	//开启一次转换
}EN_SGM58031_OS;







typedef enum
{
    EN_DEV_TEMP_DETECT_1,//总正继电器温度
    EN_DEV_TEMP_DETECT_2,//总负继电器温度
    EN_DEV_TEMP_DETECT_3,//预充继电器温度
    EN_DEV_TEMP_DETECT_4,//高压箱箱体温度
    EN_DEV_TEMP_NUM
}EN_DEV_TEMP_DETECT_IDX;


typedef enum
{
	EN_DEV_SGM_OP_STATUS_BUSY,//进行中
	EN_DEV_SGM_OP_STATUS_COMPLETE,//通信完成
	EN_DEV_SGM_OP_STATUS_ERROR,//通信错误
}EN_DEV_SGM_OP_STATUS;



void DevSgm58031Init(void);
bool DevSgm58031VoltGetCmd(EN_DEV_TEMP_DETECT_IDX ch);
EN_DEV_SGM_OP_STATUS DevSgm58031VoltGetStat(uint16_t *p_value);



#endif



