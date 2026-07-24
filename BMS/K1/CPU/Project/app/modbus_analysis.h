
#ifndef __MODBUS_ANALYSIS_H_
#define __MODBUS_ANALYSIS_H_


#include "dev_communication.h"
#include "sox.h"

#define MODBUSREG_FLASH_ADDR                   (0x00105000)
#define MODBUSREG_BACKUP_FLASH_ADDR            (0x00107000)
#define CURRENT_CALIBRATION_FACTOR_FLASH_ADDR  (0x00109000)

/**************************** 寄存器地址 *********************************/
/* 各寄存器起始地址与终止地址 */
#define MODBUS_ONLY_READ_REG_BASE_ADDR         (0X7000)
#define MODBUS_ONLY_READ_REG_END_ADDR          (0X70BF)
#define MODBUS_WRITE_READ_REG_BASE_ADDR        (0X70C0)
#define MODBUS_WRITE_READ_REG_END_ADDR         (0X7106)

#define MODBUS_TEST_WRITE_READ_REG_BASE_ADDR   (0X8000)
#define MODBUS_TEST_WRITE_READ_REG_END_ADDR    (0X800F)
#define MODBUS_TEST_ONLY_READ_REG_BASE_ADDR    (0X8010)
#define MODBUS_TEST_ONLY_READ_REG_END_ADDR     (0X8015)

/* 测试模式寄存器 */
#define MODBUS_TEST_MODE_WRITE_READ_REG_ENTER_TEST_MODE         (0X8000)    /* 进入测试模式 */

#define MODBUS_TEST_MODE_ONLY_READ_REG_KEY_STATUS               (0X8010)    /* 按键状态 */
#define MODBUS_TEST_MODE_ONLY_READ_REG_PACK_VOLTAG              (0X8011)    /* ADC电池包电压 */
#define MODBUS_TEST_MODE_ONLY_READ_REG_HEAT_STATUS              (0X8012)    /* 加热膜 */
#define MODBUS_TEST_MODE_ONLY_READ_REG_CHARGER_VOLTAGE          (0X8013)    /* 充电器电压 */
#define MODBUS_TEST_MODE_ONLY_READ_REG_FLASH_TEST_RESULT        (0X8014)    /* flash测试结果 */
#define MODBUS_TEST_MODE_ONLY_READ_REG_EEPROM_RESULT            (0X8015)    /* eeprom测试结果 */

/* 写读寄存器 */
#define MODBUS_WRITE_READ_REG_DSG_MOS_CTRL                      (0X70C0)  /* 充电MOS */
#define MODBUS_WRITE_READ_REG_CHG_MOS_CTRL                      (0X70C1)  /* 放电MOS */
#define MODBUS_WRITE_READ_REG_PRE_MOS_CTRL                      (0X70C2)  /* 预充MOS */
#define MODBUS_TEST_MODE_WRITE_READ_REG_CC_CTRL                 (0X70C3)  /* 限流验证 */
#define MODBUS_TEST_MODE_WRITE_READ_REG_SHUTDOWN_CTRL           (0X70C4)  /* 开关机 */
#define MODBUS_WRITE_READ_REG_RTC_TIMES_HIGH                    (0X70C5)  /* RTC */
#define MODBUS_WRITE_READ_REG_HEAT_CTRL                         (0X70C7)  /* 加热膜控制 */
#define MODBUS_WRITE_READ_REG_UPGRADE_STATUS                    (0X70C8)  /* 升级状态 */
#define MODBUS_WRITE_READ_REG_I_CALIBRATION                     (0X70F7)  /* 电流校准 */
#define MODBUS_WRITE_READ_REG_DSP_WAKEUP_STATUS                 (0X70FA)  /* DSP状态 */

/* 只读寄存器 */
#define MODBUS_ONLY_READ_REG_VERSION_READ                       (0X7000)  /* 版本读取 */
#define MODBUS_ONLY_READ_REG_CELL_VOLTAGE                       (0X701E)  /* 单节电芯电压 */
#define MODBUS_ONLY_READ_REG_PACK_VOLTAGE_AFE                   (0X7035)  /* AFE电池包电压 */
#define MODBUS_ONLY_READ_REG_I_HIGH                             (0X7037)  /* 电流 */
#define MODBUS_ONLY_READ_REG_CELL_TEMP                          (0X707C)  /* 温度 */


#define MODBUS_CELL_UV_LVL1                    (0X70D9)
#define MODBUS_CELL_UV_LVL2                    (0X70DA)
#define MODBUS_CELL_UV_LVL3                    (0X70DB)
#define MODBUS_CELL_OV_LVL1                    (0X70DC)
#define MODBUS_CELL_OV_LVL2                    (0X70DD)
#define MODBUS_CELL_OV_LVL3                    (0X70DE)
#define MODBUS_PACK_OV_LVL_LOW                 (0X70DF)
#define MODBUS_PACK_OV_LVL_HIGH                (0X70E0)
#define MODBUS_PACK_UV_LVL_LOW                 (0X70E1)
#define MODBUS_PACK_UV_LVL_HIGH                (0X70E2)
#define MODBUS_CELL_OT_LVL1                    (0X70E3)
#define MODBUS_CELL_OT_LVL2                    (0X70E4)
#define MODBUS_CELL_OT_LVL3                    (0X70E5)
#define MODBUS_CELL_UT_LVL                     (0X70E6)
#define MODBUS_MOS_OT_LVL1                     (0X70E7)
#define MODBUS_MOS_OT_LVL2                     (0X70E8)
#define MODBUS_MOS_OT_LVL3                     (0X70E9)
#define MODBUS_DSG_OC_LVL1                     (0X70EA)
#define MODBUS_DSG_OC_LVL2                     (0X70EB)
#define MODBUS_DSG_OC_LVL3                     (0X70EC)
#define MODBUS_CHG_OC_LVL1                     (0X70ED)
#define MODBUS_CHG_OC_LVL2                     (0X70EE)
#define MODBUS_CHG_OC_LVL3                     (0X70EF)
#define MODBUS_CELL_TEMP_DIFF                  (0X70F0)
#define MODBUS_CELL_VOLTAGE_DIFF               (0X70F1)
#define MODBUS_SOH_UNDER1                      (0X70F2)
#define MODBUS_SOH_UNDER2                      (0X70F3)
#define MODBUS_SOH_UNDER3                      (0X70F4)
#define MODBUS_ERROR_CLEAR                     (0X70F5)
#define MODBUS_FORCE_OCV                       (0X70F6)

typedef enum
{
    EN_MODBUS_RTU,
    EN_MODBUS_TCP
} EN_MODBUS_TYPE;

typedef enum
{
    EN_MODBUS_REV_STATUS_IDLE                       ,
    EN_MODBUS_REV_STATUS_ANALYSIS                   ,
    EN_MODBUS_REV_STATUS_ACK                        ,
    EN_MODBUS_REV_STATUS_WAIT_ACK_COMPLETE          ,   /* 等待发送完成 */
    EN_MODBUS_REV_STATUS_TRANSMIT_WAIT_UART         ,   /* 转发等待串口空闲 */
    EN_MODBUS_REV_STATUS_PACK_DOWN_SEND_WAIT_UART   ,   /* 请求包下发等待串口空闲 */
    EN_MODBUS_REV_STATUS_PACK_DOWN_SEND             ,   /* 请求包下发（地址+1下发） */
    EN_MODBUS_REV_STATUS_PACK_DOWN_SEND_WAIT_ACK    ,   /* 请求包下发等待应答 */
    EN_MODBUS_REV_STATUS_PACK_DOWN_TRANSMIT         ,   /* 转发请求包（同步转发，请求包的内容不更改） */
    EN_MODBUS_REV_STATUS_ACK_UP_TRANSMIT            ,   /* 转发ack应答 */
} EN_MODBUS_REV_STATUS;

typedef enum
{
    EN_MODBUS_SEND_STATUS_IDLE                       ,
    EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND_WAIT_UART  ,  /* 重启指令下发等待串口空闲 */
    EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND            ,  /* 重启指令下发 */  
    EN_MODBUS_SEND_STATUS_RESET_DOWN_SEND_WAIT_ACK   ,  /* 重启指令下发等待应答 */   
    EN_MODBUS_SEND_STATUS_RESET_CHECK                ,  /* 检测是否重启成功 */  
    EN_MODBUS_SEND_STATUS_UART_DIR_SET               ,  /* 串口方向校准 */  
    EN_MODBUS_SEND_STATUS_RESET_CHECK_WAIT_ACK       ,  /* 等待检测成功ACK */ 
    EN_MODBUS_SEND_STATUS_DMOS_CHECK                 ,  /* 检测DMOS是否打开 */  
    EN_MODBUS_SEND_STATUS_DMOS_CHECK_WAIT_ACK        ,  /* 等待DMOS成功ACK */ 
} EN_MODBUS_SEND_STATUS;


typedef enum
{
    EN_MODBUS_FUNC_CODE_READ_INPUT          = 0x04,
    EN_MODBUS_FUNC_CODE_READ_REMAIN         = 0x03,
    EN_MODBUS_FUNC_CODE_WRITE_SINGLE        = 0x06,
    EN_MODBUS_FUNC_CODE_WRITE_MULTI         = 0x10
} EN_MODBUS_FUNC_CODE;

typedef enum
{
    EN_MODBUS_ERROR_CODE_INVALID_CMD        = 0x01,
    EN_MODBUS_ERROR_CODE_INVALID_SLAVE_ADDR = 0x02,
    EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR   = 0x03,
    EN_MODBUS_ERROR_CODE_INVALID_DATA       = 0x04,
} EN_MODBUS_ERROR_CODE;

typedef enum
{
    EN_EXP_NORMAL,
    EN_CHARGING_SLAVE_FIND,
    EN_CHARGING_SWITCH,
    EN_DISCHARGING_SLAVE_FIND,
    EN_DISCHARGING_SWITCH,
    EN_SWITCH_READY_GET,
    EN_SWITCH_READY_TIMEOUT,
    EN_SWITCH_END,

} EN_EXP_SWITCH_STATUS;

typedef enum
{
    EN_TEST_MODE_REG_NORMAL,        //正常模式
    EN_TEST_MODE_REG_TOOL,          //工装模式
    EN_TEST_MODE_REG_AGING_TEST,    //老化模式
} EN_TEST_MODE_REG;

#pragma pack(1)
typedef struct _modbus_read_stru_t
{
    uint8_t        dev_addr;
    uint8_t        func_code;
    uint8_t        byte_num;
    uint16_t       *pData;/*包含校验，实际有效长度由byte_num决定*/
} ModbusReadRemainAndInput_t;
#pragma pack()



void ModbusCmdProcess(void);
void ModbusCmdDownSendProcess(void);
bool UpdateModbusReg(void);
void ModbusRegInit(void);
void ModbusRegSave(void);
void CurrentCalibrationFactorSave(void);
void CurrentCalibrationFactorRead(void);
void ResetCmdSet(void);
void DMosCheckCmdSend(uint8_t addr);
void ShutdownRegSet(uint16_t status);
uint8_t ShutdownRegGetSelf(void);
uint8_t ShutdownRegGetMonitor(void);
void RstClearShutDownReg(void);

extern ModbusOnlyReadReg_t      garrFirstPackModbusOnlyReadReg[BMS_PACK_MAX_NUM];
extern ModbusOnlyReadReg_t      garrModbusOnlyReadReg[BMS_PACK_MAX_NUM];
extern ModbusWriteReadReg_t     garrModbusWriteReadReg[BMS_PACK_MAX_NUM];
extern ModbusToolTestWriteReadModeReg_t  gstModbusToolTestWriteReadReg;                            //测试模式写读寄存器
extern ModbusToolTestOnlyReadModeReg_t   gstModbusToolTestOnlyReadReg;                             //测试模式只读寄存器
#endif








