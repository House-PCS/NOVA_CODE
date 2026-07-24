/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-06-10
  * @brief       depend on dev_modbus_s
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-06-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MODBUS_ANALYSIS_H_
#define __MODBUS_ANALYSIS_H_
#ifdef __cplusplus
extern "C" {
#endif
/* Inculdes ------------------------------------------------------------------*/
#include "systick.h"
#ifdef __cplusplus
}
#endif

/* Exported macro -----------------------------------------------------------*/
/* MODBUS ARC寄存器地址 */

#define Channel1_ENABLE  0
#define Channel2_ENABLE  0
#define Channel3_ENABLE  0
#define Channel4_ENABLE  0
#define Channel5_ENABLE  0
#define Channel6_ENABLE  0
#define Channel7_ENABLE  0
#define Channel8_ENABLE  0

#define MODEBUS_MODE_ID                      0x0000
#define MODEBUS_SN_19_18                     0x0001
#define MODEBUS_SN_17_16                     0x0002
#define MODEBUS_SN_15_14                     0x0003
#define MODEBUS_SN_13_12                     0x0004
#define MODEBUS_SN_11_10                     0x0005
#define MODEBUS_SN_09_08                     0x0006
#define MODEBUS_SN_07_06                     0x0007
#define MODEBUS_SN_05_04                     0x0008
#define MODEBUS_SN_03_02                     0x0009
#define MODEBUS_SN_01_00                     0x000A

#define MODEBUS_HARDWARE_VERSION             0x000B
#define MODEBUS_SOFTWARE_VERSION             0x000C
#define MODEBUS_RESERVE1                     0x000D
#define MODEBUS_RESERVE2                     0x000E
#define MODEBUS_ChannelOfNum                 0x000F

#define MODEBUS_Channel1CurrentValue         0x0010
#define MODEBUS_Channel2CurrentValue         0x0011
#define MODEBUS_Channel3CurrentValue         0x0012
#define MODEBUS_Channel4CurrentValue         0x0013
#define MODEBUS_Channel5CurrentValue         0x0014
#define MODEBUS_Channel6CurrentValue         0x0015
#define MODEBUS_Channel7CurrentValue         0x0016
#define MODEBUS_Channel8CurrentValue         0x0017
#define MODEBUS_RealTimeArcAlarmInformation  0x0018

#define ArcAlarmInformation                  0x0019
#define Channel1ArcStrength                  0x001A
#define Channel2ArcStrength                  0x001B
#define Channel3ArcStrength                  0x001C
#define Channel4ArcStrength                  0x001D
#define Channel5ArcStrength                  0x001E
#define Channel6ArcStrength                  0x001F
#define Channel7ArcStrength                  0x0020
#define Channel8ArcStrength                  0x0021

#define Channel1ArcIntensityHistoryValue     0x0022
#define Channel2ArcIntensityHistoryValue     0x0023
#define Channel3ArcIntensityHistoryValue     0x0024
#define Channel4ArcIntensityHistoryValue     0x0025
#define Channel5ArcIntensityHistoryValue     0x0026
#define Channel6ArcIntensityHistoryValue     0x0027
#define Channel7ArcIntensityHistoryValue     0x0028
#define Channel8ArcIntensityHistoryValue     0x0029
#define ArcSelfTestResult                    0x002A


#define MODEBUS_RESERVE3                     0x002B
#define MODEBUS_RESERVE4                     0x002C
#define MODEBUS_RESERVE5                     0x002D
#define MODEBUS_RESERVE6                     0x002E
#define MODEBUS_RESERVE7                     0x002F

#define Channel1InputVoltage                 0x0030
#define Channel2InputVoltage                 0x0031
#define Channel3InputVoltage                 0x0032
#define Channel4InputVoltage                 0x0033
#define Channel5InputVoltage                 0x0034
#define Channel6InputVoltage                 0x0035
#define Channel7InputVoltage                 0x0036
#define Channel8InputVoltage                 0x0037
#define Channel1Rx                           0x0038
#define Channel2Rx                           0x0039
#define Channel3Rx                           0x003A
#define Channel4Rx                           0x003B
#define Channel5Rx                           0x003C
#define Channel6Rx                           0x003D
#define Channel7Rx                           0x003E
#define Channel8Rx                           0x003F

#define Channel1Ry                           0x0040
#define Channel2Ry                           0x0041
#define Channel3Ry                           0x0042
#define Channel4Ry                           0x0043
#define Channel5Ry                           0x0044
#define Channel6Ry                           0x0045
#define Channel7Ry                           0x0046
#define Channel8Ry                           0x0047

#define ImpedanceAlarmInformation            0x0048
#define ImpedanceBreakdownInformation        0x0049

#define MODEBUS_RESERVE8                     0x004A
#define MODEBUS_RESERVE9                     0x004B
#define MODEBUS_RESERVE10                    0x004C
#define MODEBUS_RESERVE11                    0x004D
#define MODEBUS_RESERVE12                    0x004E
#define MODEBUS_RESERVE13                    0x004F

#define MODEBUS_RESERVE14                    0x0100
#define MODEBUS_RESERVE15                    0x0101


#define ArcSelfTestCommand                   0x0102
#define ArcAlarmThreshold1                   0x0103
#define ArcAlarmThreshold2                   0x0104
#define ArcAlarmThreshold3                   0x0105
#define ArcAlarmThreshold4                   0x0106
#define ArcAlarmThreshold5                   0x0107
#define ArcAlarmThreshold6                   0x0108
#define ArcAlarmThreshold7                   0x0109
#define ArcAlarmThreshold8                   0x010A

#define MODEBUS_RESERVE16                    0x010B
#define MODEBUS_RESERVE17                    0x010C
#define MODEBUS_RESERVE18                    0x010D
#define MODEBUS_RESERVE19                    0x010E
#define MODEBUS_RESERVE20                    0x010F

#define ImpedanceAlarmThreshold1Rx           0x0110
#define ImpedanceAlarmThreshold2Rx           0x0111
#define ImpedanceAlarmThreshold3Rx           0x0112
#define ImpedanceAlarmThreshold4Rx           0x0113
#define ImpedanceAlarmThreshold5Rx           0x0114
#define ImpedanceAlarmThreshold6Rx           0x0115
#define ImpedanceAlarmThreshold7Rx           0x0116
#define ImpedanceAlarmThreshold8Rx           0x0117

#define ImpedanceAlarmThreshold1Ry           0x0118
#define ImpedanceAlarmThreshold2Ry           0x0119
#define ImpedanceAlarmThreshold3Ry           0x011A
#define ImpedanceAlarmThreshold4Ry           0x011B
#define ImpedanceAlarmThreshold5Ry           0x011C
#define ImpedanceAlarmThreshold6Ry           0x011D
#define ImpedanceAlarmThreshold7Ry           0x011E
#define ImpedanceAlarmThreshold8Ry           0x011F

#define EnterTestMode                        0x8006
#define ARCDETECT                            0X8010



/* Exported types ------------------------------------------------------------*/
#pragma pack(1)
typedef struct _modbus_read_reg_stru_t
{
    uint16_t mode_id                  ;
    uint16_t sn[10]                   ;
    uint16_t hardware_version         ;
    uint16_t software_version         ;
    uint16_t reserve1_2[2]            ;
    uint16_t ChannelOfNum             ;
    int16_t channel1currentvalue      ;
    int16_t channel2currentvalue      ;
    int16_t channel3currentvalue      ;
    int16_t channel4currentvalue      ;
    int16_t channel5currentvalue      ;
    int16_t channel6currentvalue      ;
    int16_t channel7currentvalue      ;
    int16_t channel8currentvalue      ;

    uint16_t arcalarminformation          ;//历史状态
    uint16_t realtimearcalarminformation  ;//实时状态
    uint16_t channel1arcstrength          ;
    uint16_t channel2arcstrength          ;
    uint16_t channel3arcstrength          ;
    uint16_t channel4arcstrength          ;
    uint16_t channel5arcstrength          ;
    uint16_t channel6arcstrength          ;
    uint16_t channel7arcstrength          ;
    uint16_t channel8arcstrength          ;

    uint16_t channel1arcIntensityhistoryvalue   ;
    uint16_t channel2arcIntensityhistoryvalue   ;
    uint16_t channel3arcIntensityhistoryvalue   ;
    uint16_t channel4arcIntensityhistoryvalue   ;
    uint16_t channel5arcIntensityhistoryvalue   ;
    uint16_t channel6arcIntensityhistoryvalue   ;
    uint16_t channel7arcIntensityhistoryvalue   ;
    uint16_t channel8arcIntensityhistoryvalue   ;
    uint16_t arcselftestresult          ;

    uint16_t reserve3_7[5]              ;
    int16_t  channel1inputvoltage       ;
    int16_t  channel2inputvoltage       ;
    int16_t  channel3inputvoltage       ;
    int16_t  channel4inputvoltage       ;
    int16_t  channel5inputvoltage       ;
    int16_t  channel6inputvoltage       ;
    int16_t  channel7inputvoltage       ;
    int16_t  channel8inputvoltage       ;

    uint16_t channel1rx ;
    uint16_t channel2rx ;
    uint16_t channel3rx ;
    uint16_t channel4rx ;
    uint16_t channel5rx ;
    uint16_t channel6rx ;
    uint16_t channel7rx ;
    uint16_t channel8rx ;

    uint16_t channel1ry ;
    uint16_t channel2ry ;
    uint16_t channel3ry ;
    uint16_t channel4ry ;
    uint16_t channel5ry ;
    uint16_t channel6ry ;
    uint16_t channel7ry ;
    uint16_t channel8ry ;

    uint16_t impedancealarmInformation     ;
    uint16_t impedanceBreakdownInformation ;
    uint16_t reserve8_13[6]              ;
    uint16_t reserve14[0x00FF - 0x004F]    ;

    uint16_t reserve14_15[2];

    uint16_t arcselftestcommand        ;
    uint16_t arcalarmthreshold1        ;
    uint16_t arcalarmthreshold2        ;
    uint16_t arcalarmthreshold3        ;
    uint16_t arcalarmthreshold4        ;
    uint16_t arcalarmthreshold5        ;
    uint16_t arcalarmthreshold6        ;
    uint16_t arcalarmthreshold7        ;
    uint16_t arcalarmthreshold8        ;

    uint16_t reserve16_20[5];

    uint16_t impedanceAlarmThreshold1Rx;
    uint16_t impedanceAlarmThreshold2Rx;
    uint16_t impedanceAlarmThreshold3Rx;
    uint16_t impedanceAlarmThreshold4Rx;
    uint16_t impedanceAlarmThreshold5Rx;
    uint16_t impedanceAlarmThreshold6Rx;
    uint16_t impedanceAlarmThreshold7Rx;
    uint16_t impedanceAlarmThreshold8Rx;

    uint16_t impedanceAlarmThreshold1Ry;
    uint16_t impedanceAlarmThreshold2Ry;
    uint16_t impedanceAlarmThreshold3Ry;
    uint16_t impedanceAlarmThreshold4Ry;
    uint16_t impedanceAlarmThreshold5Ry;
    uint16_t impedanceAlarmThreshold6Ry;
    uint16_t impedanceAlarmThreshold7Ry;
    uint16_t impedanceAlarmThreshold8Ry;

} ModbusReadReg_t; //读寄存器

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t upgrade_state                : 1;          /*    0:未升级，1：升级中  */
        uint16_t restate_state                : 1;          /*    0:未复位，1：复位  */
        uint16_t mintor_test_restate          : 1;          /*    监控卡测试复位  */
        uint16_t mintor_restate_state         : 1;          /*    0：监控卡未复位，1：监控卡复位中  */
        uint16_t reserve1                     : 1;
        uint16_t reserve2                     : 1;
        uint16_t reserve3                     : 1;
        uint16_t reserve4                     : 1;
        uint16_t reserve5                     : 1;
        uint16_t reserve6                     : 1;
        uint16_t reserve7                     : 1;
        uint16_t reserve8                     : 1;
        uint16_t reserve9                     : 1;
        uint16_t reserve10                    : 1;
        uint16_t reserve11                    : 1;
        uint16_t reserve12                    : 1;
    } b;

} SysUpgradeState_t;

typedef struct _modbus_reg_stru_t
{
    uint16_t reserve21_22[2]           ;
    uint16_t arcselftestcommand        ;
    uint16_t arcalarmthreshold1        ;
    uint16_t arcalarmthreshold2        ;
    uint16_t arcalarmthreshold3        ;
    uint16_t arcalarmthreshold4        ;
    uint16_t arcalarmthreshold5        ;
    uint16_t arcalarmthreshold6        ;
    uint16_t arcalarmthreshold7        ;
    uint16_t arcalarmthreshold8        ;
    uint16_t arcdetectlevel            ;//拉弧灵敏度等级
    uint16_t resdetectenable           ;

    SysUpgradeState_t sysupgradestatus ;//升级状态

    uint16_t reserve22_26[2]           ;

    uint16_t impedanceAlarmThreshold1Rx;
    uint16_t impedanceAlarmThreshold2Rx;
    uint16_t impedanceAlarmThreshold3Rx;
    uint16_t impedanceAlarmThreshold4Rx;
    uint16_t impedanceAlarmThreshold5Rx;
    uint16_t impedanceAlarmThreshold6Rx;
    uint16_t impedanceAlarmThreshold7Rx;
    uint16_t impedanceAlarmThreshold8Rx;

    uint16_t impedanceAlarmThreshold1Ry;
    uint16_t impedanceAlarmThreshold2Ry;
    uint16_t impedanceAlarmThreshold3Ry;
    uint16_t impedanceAlarmThreshold4Ry;
    uint16_t impedanceAlarmThreshold5Ry;
    uint16_t impedanceAlarmThreshold6Ry;
    uint16_t impedanceAlarmThreshold7Ry;
    uint16_t impedanceAlarmThreshold8Ry;
} ModbusReg_t;  //写寄存器

/*工装测试可读寄存器*/

typedef struct _modbus_read_reg_test_stru_t
{
    uint16_t      enter_test_mode      ;
    uint16_t      arc_detect           ;
    uint16_t      insulation_detect    ;
    uint16_t      reserve[7]           ;

    uint16_t       channal1_arc_detect_result            ;
    uint16_t       channal2_arc_detect_result            ;

    uint16_t       ht7017_init_result                    ;

    uint16_t       channal1_insulation_detect_result_p1  ;
    uint16_t       channal1_insulation_detect_result_n1  ;
    uint16_t       channal2_insulation_detect_result_p1  ;
    uint16_t       channal2_insulation_detect_result_n1  ;


} ModbuseReadTestReg_t;

/*工装测试保持寄存器*/

typedef struct _modbus_reg_test_stru_t
{
    uint16_t      enter_test_mode      ;
    uint16_t      arc_detect           ;
    uint16_t      insulation_detect    ;

} ModbusTestReg_t;





#pragma pack()

typedef enum
{
    EN_MODBUS_RTU,
    EN_MODBUS_TCP
} EN_MODBUS_TYPE;

typedef enum
{
    EN_MODBUS_REV_STATUS_IDLE,
    EN_MODBUS_REV_STATUS_ANALYSIS,
    EN_MODBUS_REV_STATUS_ACK_SEND_WAIT,
    EN_MODBUS_REV_STATUS_ACK,
    EN_MODBUS_REV_STATUS_ACK_WAIT,
} EN_MODBUS_REV_STATUS;

typedef enum
{
    EN_MODBUS_FUNC_CODE_READ_INPUT = 0x04,
    EN_MODBUS_FUNC_CODE_READ_REMAIN = 0x03,
    EN_MODBUS_FUNC_CODE_WRITE_SINGLE = 0x06,
    EN_MODBUS_FUNC_CODE_WRITE_MULTI = 0x10
} EN_MODBUS_FUNC_CODE;

typedef enum
{
    EN_MODBUS_ERROR_CODE_INVALID_CMD = 0x01,
    EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR = 0x02,
    EN_MODBUS_ERROR_CODE_INVALID_DATA = 0x03,
} EN_MODBUS_ERROR_CODE;

#pragma pack(1)
typedef struct _modbus_read_stru_t
{
    uint8_t        dev_addr;
    uint8_t        func_code;
    uint8_t        byte_num;
    uint16_t       *pData;/*包含校验，实际有效长度由byte_num决定*/
} ModbusReadRemainAndInput_t;
#pragma pack()
/* Exported constants --------------------------------------------------------*/

extern ModbusReadReg_t Modbus_ReadReg;
extern ModbusReg_t Modbus_writeReg;
extern uint8_t gUpgradeRestartFlag;
extern ModbuseReadTestReg_t ModbusTest_ReadReg;
extern ModbusTestReg_t ModbusTest_writeReg;
/* Exported functions ------------------------------------------------------- */
void ModbusCmdProcess(void);
void ModbusRegInit(void);
void _ModBusReadRegUpdate(void);
void ModbusRegInit(void);
#endif
/************************ (C) COPYRIGHT Nova *************END OF FILE**********/












