/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        slave_ledctl.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-10-22
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-10-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __SLAVE_LEDCTL_H_
#define __SLAVE_LEDCTL_H_

#include "queue.h"
#include "dev_modbus_h.h"
#include "dev_modbus_rtu.h"

#define REQUEST_HEAD                            (0x5A)
#define RESPONSE_HEAD                           (0xA5)
#define SEND_ADDR                               (0x00)
#define RECV_ADDR                               (0x01)

#define RECV_BUFF_SIZE                          (512)

#define LEDCTL_START_MODE_ADDR                  (0xA000)
#define LEDCTL_SYS_MODE_ADDR                    (0xA001)
#define LEDCTL_COLLECTOR_MODE_ADDR              (0xA002)
#define LEDCTL_PCS_MODE_ADDR                    (0xA003)
#define LEDCTL_CONFIRM_KEY_MODE_ADDR            (0xA004)
#define LEDCTL_ESC_KEY_MODE_ADDR                (0xA005)
#define LEDCTL_UP_KEY_MODE_ADDR                 (0xA006)
#define LEDCTL_DOWN_KEY_MODE_ADDR               (0xA007)
#define LEDCTL_RERSVER_ADDR                     (0xA008)


#define LEDCTL_START_MODE_NUM                   (28)

//#define LEDCTL_START_MODE_NUM                   (1)
//#define LEDCTL_SYS_MODE_NUM                     (1)
//#define LEDCTL_COLLECTOR_MODE_NUM               (1)
//#define LEDCTL_PCS_MODE_NUM                     (1)
//#define LEDCTL_CONFIRM_KEY_MODE_NUM             (1)
//#define LEDCTL_ESC_KEY_MODE_NUM                 (1)
//#define LEDCTL_UP_KEY_MODE_NUM                  (1)
//#define LEDCTL_DOWN_KEY_MODE_NUM                (1)
//#define LEDCTL_RERSVER_NUM                      (20)


#define COPCTRL_MODE_ID_ADDR                    0x5800

#define COPCTRL_MODE_ID_NUM                     80


typedef enum
{
    EN_ATTR_INVALID = 0,
    EN_ATTR_RO,
    EN_ATTR_WO,
    EN_ATTR_RW,
} EN_DATA_ATTR_RW;

typedef enum
{
    EN_DATA_INVALID = 0,
    EN_DATA_U8 = 1,
    EN_DATA_S8 = 2,
    EN_DATA_U16 = 3,
    EN_DATA_S16 = 4,
    EN_DATA_U32 = 5,
    EN_DATA_S32 = 6,
    EN_DATA_STRING = 7,
} EN_DATA_TYPE;
typedef struct
{
    uint16_t base_addr;
    uint16_t num;
    EN_DATA_TYPE data_type;
    uint16_t attr_rw;
    void *pdata;
} ModbusDataPoint_t;

typedef enum
{
    EN_MAINCARD_POLLING_STATUS_IDLE = 0,
    EN_MAINCARD_POLLING_STATUS_REV,
    EN_MAINCARD_POLLING_STATUS_REV_OVERTIME,
    EN_MAINCARD_POLLING_STATUS_WAITING_TX,
    EN_MAINCARD_POLLING_STATUS_INTERVAL_WAIT,
} EN_MAINCARD_POLLING_STATUS;

typedef enum
{
    EN_ATMOS_LED_START_STA = 0,
    EN_ATMOS_LED_SYS_STA,
    EN_ATMOS_LED_COLLECT_STA,
    EN_ATMOS_LED_PCS_STA,
    EN_ATMOS_LED_CONFIRM_KEY_STA,
    EN_ATMOS_LED_ESC_KEY_STA,
    EN_ATMOS_LED_UP_KEY_STA,
    EN_ATMOS_LED_DOWN_KEY_STA,
} EN_ATMOS_LED_STATUS;

typedef enum
{
    EN_ATMOS_LED_CONFIRM_STA = 0,
    EN_ATMOS_LED_ESC_STA,
    EN_ATMOS_LED_UP_STA,
    EN_ATMOS_LED_DOWN_STA,
} EN_KEY_ATMOS_LED_STATUS;

typedef struct _led_ctl_tx_strut_t
{
    uint8_t  len;
    uint8_t  *pData;//[256];
} LedCtlTxData_t;

typedef struct _led_ctl_reg_data_stru_t
{
    uint16_t    start_led_mode;
    uint16_t    sys_led_mode;
    uint16_t    collector_led_mode;
    uint16_t    pcs_led_mode;
    uint16_t    confirm_led_mode;
    uint16_t    esc_led_mode;
    uint16_t    up_led_mode;
    uint16_t    down_led_mode;
    
    uint16_t    led_r_value_and_status;
    uint16_t    led_b_value_and_g_value;
    uint16_t    led_mode;
    uint16_t    led_period;
    uint16_t    reserved[16];
} LedCtrlPara_t;

typedef union
{
    uint16_t value;
    struct
    {
        uint16_t upgrade_state                : 1;          /*    0:未升级，1：升级中  */
        uint16_t restate_state                : 1;          /*    0:未复位，1：复位  */
        uint16_t mintor_test_restate          : 1;          /*    监控卡测试复位  */
        uint16_t mintor_restate_state         : 1;          /*    0：监控卡未复位，1：监控卡复位中  */
        uint16_t bit4                         : 1;          
        uint16_t bit5                         : 1;          
        uint16_t bit6                         : 1;          
        uint16_t bit7                         : 1;          
        uint16_t bit8                         : 1;          
        uint16_t bit9                         : 1;          
        uint16_t bit10                        : 1;          
        uint16_t bit11                        : 1;          
        uint16_t bit12                        : 1;          
        uint16_t upgrade_res1                 : 1;        //bit 13 14 共同决定升级是否成功 0：默认 1：成功 2：失败
        uint16_t upgrade_res2                 : 1;        //     
        uint16_t bit15                        : 1;
    } bits;
} SysUpgradeState_t;



#pragma pack(2)
/*********************监控卡协处理器信息************************/
/*----监控卡协处理器基础信息---*/
typedef struct
{
    uint16_t    model_id;               //5800
    uint16_t    sn[10];                 //5801~580A
    uint16_t    hw_ver;                 //580B
    uint16_t    sw_ver[2];              //580C~580D
    uint16_t    sw_ver_ext[12];         //580E~5819
    uint16_t    reserve[6];             //581A~581F
} MainCardCopBaseInfo_t;

/*----监控卡协处理器状态信息---*/
typedef struct
{
    uint16_t    run_status;             //5820
    uint16_t    reserve[15];            //5821~582F
} MainCardCopStatusInfo_t;

/*----监控卡协处理器控制信息---*/
typedef struct
{
    SysUpgradeState_t    upgrade_status;     //5830
    uint16_t    start_led_mode;              //5831
    uint16_t    sys_led_mode;                //5832
    uint16_t    collector_led_mode;          //5833
    uint16_t    pcs_led_mode;                //5834
    uint16_t    confirm_led_mode;            //5835
    uint16_t    esc_led_mode;                //5836
    uint16_t    up_led_mode;                 //5837
    uint16_t    down_led_mode;               //5838
    uint16_t    led_dir_r;                   //5839
    uint16_t    led_g_b;                     //583A
    uint16_t    led_mode;                    //583B
    uint16_t    led_period;                  //583C
    uint16_t    reserve[23];                 //583D~584F
} MainCardCopCtrlInfo_t;

typedef struct _main_cop_ctrl_stru_t
{
    MainCardCopBaseInfo_t       base_info;
    MainCardCopStatusInfo_t     status_info;
    MainCardCopCtrlInfo_t       ctrl_info; 
} MainCopCtrlPara_t;
#pragma pack()


#pragma pack(1)
typedef struct _up_main_card_recv_stru_t
{
    uint8_t  packet_head ;      /* 固定为0x5A */
    uint8_t  send_addr;         /* 发送方的地址 */
    uint8_t  recv_addr;         /* 接收方的地址 */
    uint16_t cmd;               /* 不同cmd对应的数据格式不同 */
    uint16_t data_field_len;    /* 读写传输的数据长度，字节为单位 */
    uint8_t  *data_field;       /* data段 */
    uint16_t check_sum;         /* 校验和() */
} ReceiveDataPack_t;

typedef struct _up_main_card_response_stru_t
{
    uint8_t  packet_head ;      /* 固定为0xA5 */
    uint8_t  send_addr;         /* 发送方的地址 */
    uint8_t  recv_addr;         /* 接收方的地址 */
    uint16_t cmd;               /* 不同cmd对应的数据格式不同 */
    uint8_t  error_code;        /* error  */
    uint16_t check_sum;         /* 校验和() */
} ResponseDataPack_t;
#pragma pack()



extern MainCopCtrlPara_t  gstMainCopParaData;

// extern LedCtrlPara_t    gstLedCtrlParaData;

void WordExchange(uint16_t *pword);
void ModbusReadParse(ModbusRtuFrame_t *ptFrame);
bool ModbusWriteParse(ModbusRtuFrame_t *ptFrame);

uint16_t GetAtmosLedStatus(EN_ATMOS_LED_STATUS index);
uint16_t GetKeyAtmosLedStatus(EN_KEY_ATMOS_LED_STATUS sta);

void SlaveLedCtlModbusCommProcess(void);
void SlaveLedCommInit(void);
#endif
