
#ifndef __DEV_MODBUS_S_H_
#define __DEV_MODBUS_S_H_

#include "uart.h"
#include "queue.h"
#include "bsp.h"

/**************************************普通协议包宏定义****************************************/
#define COMMON_PACK_REQUST_PACKET_HEAD      (0x5A)
#define COMMON_PACK_RESPONSE_PACKET_HEAD    (0xA5)

#define MONITORING_CARD_ID                  (0x00)       /* 监控卡地址 */
#define BMS_ID                              (0X03)       /* BMS地址 */

#define COMMON_PACK_FIRST_ADDR              (0X10)
#define COMMON_PACK_LAST_ADDR               (0X15)

/****************************************MODBUS宏定义******************************************/
#define MODBUS_DEVICE_ADDR	                (0x04)
#define MODBUS_MPPT_ADDR	                (0x05)

#define MODBUS_ADDR_DEFAULT_VALUE           (0X30)      /* modbus默认地址 */

#define	MODBUS_ADDR_PACK0	                (0x30)
#define	MODBUS_ADDR_PACK1	                (0x31)
#define MODBUS_ADDR_PACK2	                (0x32)
#define MODBUS_ADDR_PACK3	                (0x33)
#define MODBUS_ADDR_PACK_MAX_ADDR	        (MODBUS_ADDR_PACK3)
#define BMS_PACK_MAX_NUM        	        (4)
#define UART_NUM        	                (2)

typedef enum
{
    EN_MODBUS_HOST_SLAVE_TYPE_SLAVE         ,
	EN_MODBUS_HOST_SLAVE_TYPE_HOST          ,
    EN_MODBUS_HOST_SLAVE_TYPE_NUM           ,
} EN_MODBUS_HOST_SLAVE_TYPE;

typedef enum
{
	TYPE_MODBUS_DATA                        ,
	TYPE_COM_DATA                           ,
    TYPE_MPPT_DATA                          ,
}EN_UART_DATA_TYPE;

/****************************************MODBUS寄存器******************************************/
typedef struct _modbus_read_reg_stru_t
{
    uint16_t model_id                       ;
    uint16_t sn[10]                         ;
    uint16_t hardware_version               ;
    uint16_t software_version               ;
    uint16_t reserved[17]                   ;
    uint16_t cell_vdc[20]                   ;
    uint16_t cell_max_voltage               ;
    uint16_t cell_min_voltage               ;
    uint16_t cell_average_voltage           ;
    uint16_t cell_all_vdc_high              ;
    uint16_t cell_all_vdc_low               ;
    uint16_t cell_current_high              ;
    uint16_t cell_current_low               ;
    uint16_t current_power                  ;
    uint16_t abs_cell_current_high          ;
    uint16_t abs_cell_current_low           ;
    uint16_t reserved1[15]                  ;
    uint16_t soc_real                       ;
    uint16_t soc_act                        ;
    uint16_t soc_rpt                        ;
    uint16_t capacity_std_ah                ;
    uint16_t capacity_std_wh                ;
    uint16_t current_max_capa_high          ;
    uint16_t current_max_capa_low           ;
    uint16_t pack_health                    ;
    uint16_t pack_cycles                    ;
    uint16_t charging_capacity_high         ;
    uint16_t charging_capacity_low          ;
    uint16_t discharge_capacity_high        ;
    uint16_t discharge_capacity_low         ;
    uint16_t current_max_chg_current_high   ;
    uint16_t current_max_chg_current_low    ;
    uint16_t current_max_dsg_current_10_high;
    uint16_t current_max_dsg_current_10_low ;
    uint16_t current_max_dsg_current_30_high;
    uint16_t current_max_dsg_current_30_low ;
    uint16_t current_max_dsg_current_60_high;
    uint16_t current_max_dsg_current_60_low ;
    uint16_t pack_sleep_time_high           ;
    uint16_t pack_sleep_time_low            ;
    uint16_t charging_capacity2_high        ;
    uint16_t charging_capacity2_low         ;
    uint16_t discharge_capacity2_high       ;
    uint16_t discharge_capacity2_low        ;
    uint16_t pack_status                    ;
    uint16_t recharge_time_high             ;
    uint16_t recharge_time_low              ;
    uint16_t mos_status                     ;
    uint16_t bms_num                        ;// 电池包数量
    uint16_t up_finish_flag                 ;
    uint16_t ocv_flag                       ;//ocv校准方式标志
    uint16_t reserved2[15]                  ;
    uint16_t cell_temp[20]                  ;
    uint16_t cell_max_temp                  ;
    uint16_t cell_min_temp                  ;
    uint16_t cell_ave_temp                  ;
    uint16_t afe_temp                       ;
    uint16_t mos_temp                       ;
    uint16_t ambient_temp                   ;
    uint16_t balance_status                 ;
    uint16_t reserved3[15]                  ;
    uint16_t ems_error_code                 ;//系统级BMS故障（只有第一个BMS会对该故障赋值）
    uint16_t bms_run_status                 ;//BMS运行状态
    uint16_t mos_error_code                 ;//mos故障
    uint16_t voltage1_error_code            ;//电压故障1
    uint16_t voltage2_error_code            ;//电压故障2 预留
    uint16_t curr_error_code                ;//电流故障
    uint16_t temp_error_code                ;//温度故障
    uint16_t state_error_code               ;//状态故障
    uint16_t other1_error_code              ;//其他故障
    uint16_t bms_pre_chg_status             ;//BMS预充是否完成
    uint16_t once_run_error_code_1_high     ;//单次运行故障1
    uint16_t once_run_error_code_1_low      ;//单次运行故障1
    uint16_t once_run_error_code_2_high     ;//单次运行故障2
    uint16_t once_run_error_code_2_low      ;//单次运行故障3
    uint16_t reserved4[12]                  ;//预留
} ModbusOnlyReadReg_t; //只读寄存器

typedef struct _modbus_reg_stru_t
{
    uint16_t    dsg_mos_ctl                 ;
    uint16_t    chg_mos_ctl                 ;
    uint16_t    pre_mos_ctl                 ;
    uint16_t    limit_curr_ctl              ;
    uint16_t    bms_shutdown                ;
    uint16_t    rtc_times_high              ;
    uint16_t    rtc_times_low               ;
    uint16_t    heat_ctl                    ;
    uint16_t    sys_up_status               ;   //系统升级状态
    uint16_t    reserved[16]                ;
    uint16_t    cell_uv_lvl1                ;
    uint16_t    cell_uv_lvl2                ;
    uint16_t    cell_uv_lvl3                ;
    uint16_t    cell_ov_lvl1                ;
    uint16_t    cell_ov_lvl2                ;
    uint16_t    cell_ov_lvl3                ;
    uint16_t    pack_uv_lvl_high            ;
    uint16_t    pack_uv_lvl_low             ;
    uint16_t    pack_ov_lvl_high            ;
    uint16_t    pack_ov_lvl_low             ;
    uint16_t    cell_ot_lvl1                ;
    uint16_t    cell_ot_lvl2                ;
    uint16_t    cell_ot_lvl3                ;
    uint16_t    cell_ut_lvl                 ;
    uint16_t    mos_ot_lvl1                 ;
    uint16_t    mos_ot_lvl2                 ;
    uint16_t    mos_ot_lvl3                 ;
    uint16_t    dsg_oc_lvl1                 ;
    uint16_t    dsg_oc_lvl2                 ;
    uint16_t    dsg_oc_lvl3                 ;
    uint16_t    chg_oc_lvl1                 ;
    uint16_t    chg_oc_lvl2                 ;
    uint16_t    chg_oc_lvl3                 ;
    uint16_t    cell_temp_diff              ;
    uint16_t    cell_voltage_diff           ;
    uint16_t    soh_under_lvl1              ;
    uint16_t    soh_under_lvl2              ;
    uint16_t    soh_under_lvl3              ;
    uint16_t    error_clear                 ;
    uint16_t    force_ocv                   ;   //强制校准soc
    
    uint16_t    current_calibrat_high       ;   //校准电流（仅进行电流校准时使用）
    uint16_t    current_calibrat_low        ;
    
    uint16_t    pre_dsg_end                 ;   //预充结束
    uint16_t    dsp_wakeup_status           ;   //DSP唤醒状态
    uint16_t    reserved2[14]               ;
    

} ModbusWriteReadReg_t; //读写寄存器


typedef struct
{
    uint16_t    test_mode_enter_ctrl        ;   //8000
    uint16_t    reserved[16]                ;   
} ModbusToolTestWriteReadModeReg_t; //测试模式只读寄存器

typedef struct
{
    uint16_t    start_key_status            ;   //8010
    uint16_t    pack_adc_val                ;
    uint16_t    heat_status                 ;
    uint16_t    charger_voltage             ;
    uint16_t    flash_error_flag            ;
    uint16_t    eeprom_error_flag           ;
} ModbusToolTestOnlyReadModeReg_t; //测试模式只读寄存器


/******************************************************************************************/

#pragma pack(1)
typedef struct _comm_host_rev_data_stru_t
{
    uint8_t                     slave_addr  ;
    Uart_HandleType*            p_uarthandle;
    EN_MODBUS_HOST_SLAVE_TYPE   en_modbus_host_or_slave;
	uint16_t			        data_size   ;
	uint8_t				        *pData      ;
}ModbusRTURevData_t; 
#pragma pack()

typedef struct
{
    uint8_t                 ModbusAddr                                  ;   /*本机MODBUS地址*/
    uint8_t                 final_bms_addr                              ;   /*目前最后一个电池包的地址*/
    uint8_t                 bms_num                                     ;   /*电池包的数量*/
    bool                    is_final_bms                                ;   /*判断是不是最后一个BMS*/
    
    uint8_t                 com_success_flag                            ;  /*监控卡通讯成功信号*/
    
    uint32_t                final_com_time                              ;   /* 最后的通讯时间点 */
    uint32_t                com_out_time                                ;   /* 通讯超时时间 */
    uint8_t                 com_out_flag                                ;   /* 通讯超时标志 */
    
    bool                    send_and_ack_complete_flag[UART_NUM]        ;   /*完成一次收发，允许再次发送*/
    
    ModbusOnlyReadReg_t*    p_modbus_only_read_reg[BMS_PACK_MAX_NUM]    ;   /*modbus只读寄存地址指针(处理后的数据)*/
    ModbusWriteReadReg_t*   p_modbus_write_read_reg[BMS_PACK_MAX_NUM]   ;   /*modbus写读寄存地址指针*/
    uint32_t                modbus_only_read_reg_size                   ;   /*modbus只读寄存器长度*/
    uint32_t                modbus_write_read_reg_size                  ;   /*modbus写读寄存器长度*/
}DevUartPackInfo_t;


/*数据队列*/
extern Queue_t	        gModbusRTURevQueue                     ;    /*MODBUS从机队列*/
extern Queue_t	        gModbusHostQueue                       ;    /*MODBUS主机队列*/
extern Queue_t	        gUpCommonPackQueue                     ;    /*普通协议包队列（与上一级相连）*/
extern Queue_t	        gDownCommonPackQueue                   ;    /*普通协议包队列（与下一级相连）*/
extern Queue_t	        gModbusRTUPackForwardQueue             ;    /*MODBUS数据包转发队列（不是自己的包，向下转发）*/
extern uint8_t          gBMSSleepFlag                          ;
//extern Uart_HandleType *gModbusUart[MODBUS_HOST_SLAVE_TYPE_NUM];    /*MODBUS主、从串口指针*/

extern DevUartPackInfo_t gUartPackInfo;                                    /*串口协议包及发送配置相关信息*/

/*函数*/
void DevModbusRTUInit(void);
void ModbusRTUSend(uint8_t *p, uint16_t size, EN_MODBUS_HOST_SLAVE_TYPE en_modbustype, uint8_t is_need_add_crc);
void CommonPackSend(uint8_t *p, uint16_t size, Uart_HandleType* p_uarthandle);
bool DevModbusSendComplete(EN_MODBUS_HOST_SLAVE_TYPE en_modbustype);
void DevModbusDirCtlProcess(EN_MODBUS_HOST_SLAVE_TYPE en_modbustype);
uint16_t BEBufToUint16(uint8_t *_pBuf);
void ModbusHostOrSlaveSet(Uart_HandleType* p_uart, EN_MODBUS_HOST_SLAVE_TYPE en_modbustype);
void TabLsbToMsb(uint8_t *str, uint16_t len);
void UartSendTest(Uart_HandleType *pUartHandle);
uint32_t SetMaxWaitTime(uint32_t addr);             //设置最大等待时间

#endif








