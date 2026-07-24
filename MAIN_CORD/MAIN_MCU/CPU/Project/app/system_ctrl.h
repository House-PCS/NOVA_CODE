#ifndef SYSTEM_CTRL_HHH
#define SYSTEM_CTRL_HHH
#include "slave_collect.h"


#define     CAP_STORAGE_BASE_ADDR           0x258000
#define     MAX_CHG_VOLT_VALUE              576//54.0V
#define     MIN_DSG_VOLT_VALUE              432
#define     MIN_SOH                         700ul //70%
#define     CUR_DIRECT_MIN_THRESHOLD        (-10)
#define     CUR_DIRECT_MAX_THRESHOLD        (10)
#define     CUR_DIRECT_STANDBY              0
#define     CUR_DIRECT_POSITIVE             1
#define     CUR_DIRECT_NEGATIVE             2
#define     UART_BAUD_MAX                   7


#define     BMS_NORMAL                     0X00
#define     BMS_TRUN_OFF                   0X01
#define     BMS_TURN_ON                    0X02

#pragma pack(2)
typedef  enum
{
    EN_SYS_BMS_ADRESS_STATUS,
    EN_SYS_INIT_STATUS,
    EN_SYS_WAKEUP_STATUS,
    EN_SYS_IDLE_STATUS,
    EN_SYS_WORKING_STATUS,
    EN_SYS_ERROR_STATUS,
} EN_SYS_STATUS;


typedef struct _sys_ctrl_info_stru_t
{
    union
    {
        struct  
        {
            uint16_t peak_shift_chg_en:1;
            uint16_t peak_shift_dsg_en:1;
            uint16_t pcs_chg_ctrl_en:2;     //00--买电异常  1--买电正常 2--买电关闭
            uint16_t pcs_dsg_ctrl_en:2;     //00--卖电异常  1--卖电正常 2--卖电关闭
            
            uint16_t flag_bat_depth:2;          //放电深度到达标志
            uint16_t flag_bat_min_soc_depth:2;  //单包最小soc标志
            uint16_t soc_retern_diff:2;         //发电机回充标志
            uint16_t chg_end_retern_diff:2;     //充电末端充电回差标志
        }bits;
        uint16_t sys_ctrl_flag;
    }sys_ctrl_status;
    
} SysCtrlInfo_t;



#pragma pack()

void SystemCtrlInit(void);
void SystemCtrlProc(void);
void CleanBackLightTimeCnt(void);
void SystemTurnOffCtrl(void);

//void JudgeDRM0ConnectStatus(void);
uint32_t GetOnGridTime(void);
uint8_t GetTurnOffStatus(void);
void GetOnGridNumber(void);
void LcdBackLightCtrl(void);

EN_SYS_STATUS GetSysRunStatus(void);
#endif