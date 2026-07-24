
#ifndef _TOOLS_PROGRAM_H_
#define _TOOLS_PROGRAM_H_


#include "bsp.h"


extern uint16_t gToolsGetTimeFlag;


#define CONTROL_ENTER_TEST_REG_ADDR     0x8000
#define CONTROL_IO_CTR_REG_ADDR         0x8001
#define CONTROL_GET_RTC_REG_ADDR        0x8002


#define CONTROL_TEST_FLASH_REG_ADDR     0x8010
#define CONTROL_GET_IO_REG_ADDR         0x8011



#define TEST_REG_NUM                    1
#define IO_CTR_REG_NUM                  1
#define GET_RTC_REG_NUM                 2
#define TEST_FLASH_REG_NUM              1
#define GET_IO_REG_NUM                  1



//new
#define TOOLLING_ENTER_TEST_REG_ADDR     0x5850
#define TOOLLING_GET_IO_REG_ADDR         0x5851
#define TOOLLING_GET_RTC_REG_ADDR        0x5852
#define TOOLLING_IO_CTR_REG_ADDR         0x5854

#define TOOLLING_TEST_FLASH_REG_ADDR     0x5860
#define TOOLLING_IO_REG_RESULT_ADDR      0x5861
#define TOOLLING_RTC_REULT_REG_ADDR      0x5862
#define TOOLLING_COP_UART_REG_ADDR       0x5863
#define TOOLLING_SW_REG_ADDR             0x5864



#define TOOLLING_ENTER_TEST_REG_NUM     1
#define TOOLLING_GET_IO_REG_NUM         1
#define TOOLLING_GET_RTC_REG_NUM        2
#define TOOLLING_IO_CTR_REG_NUM         1

#define TOOLLING_TEST_FLASH_REG_NUM     1
#define TOOLLING_IO_REG_RESULT_NUM      1
#define TOOLLING_RTC_REULT_REG_NUM      1
#define TOOLLING_COP_UART_REG_NUM       1
#define TOOLLING_SW_REG_NUM             2




typedef struct _control_para_stru_t
{
    uint16_t    enter_test_mode;             //0x8000
    uint16_t    io_ctr;
    uint16_t    rtc_time[2];
    uint16_t    reserved[12];

    uint16_t    flash_test;                 //0x8010
    uint16_t    io_get;
} ControlPara_t;

extern ControlPara_t  gControlParaData;


typedef enum
{
    EN_TOOLING_TEST_IDLE,
    EN_TOOLS_ENTER_TEST_MODE,
    EN_TOOLING_TEST_DRIVE_IO,
    EN_TOOLING_TEST_RTC,
    EN_TOOLING_TEST_FLASH,
    EN_TOOLING_TEST_GET_IO,
    EN_TOOLING_TEST_COP_UART,
} EN_TOOLING_STATUS;



typedef enum
{
    EN_CONTROL_STEP_IO_CTR_IDLE ,
    EN_CONTROL_STEP_IO_CTR_HIGH ,
    EN_CONTROL_STEP_IO_CTR_LOW ,
    EN_CONTROL_STEP_IO_CTR_HIGH_ACK ,
    EN_CONTROL_STEP_IO_CTR_LOW_ACK ,
    EN_CONTROL_STEP_IO_CTR_END ,

} EN_CONTROL_IO_CTR_STEP;



typedef union _get_gpio_status
{
    struct
    {
        uint8_t bit0: 1;
        uint8_t bit1: 1;
        uint8_t bit2: 1;
        uint8_t bit3: 1;
        uint8_t bit4: 1;
        uint8_t bit5: 1;
        uint8_t bit6: 1;
        uint8_t bit7: 1;
    } bits;
    uint8_t byte_value;
} GpioStatus_t;

/*----工装控制信息---*/
typedef struct
{
    uint16_t enter_tool_mode;   //5850
    uint16_t drm_enable     ;   //5851     GET IO
    uint16_t rtc[2]         ;   //5852~5853     RTC时间
    uint16_t relay_io       ;   //5854
    uint16_t reserve[11]    ;   //5855~585F     预留
    
} ToolCtrlInfo_t;

/*----工装状态信息---*/
typedef struct
{
    uint16_t flash_result       ;   //5860 
    uint16_t drm_status         ;   //5861
    uint16_t rtc_result         ;   //5862  RTC时间结果
    uint16_t cop_uart_result    ;   //5863
    uint16_t main_version[2]    ;   //5864
    uint16_t reserve[10]        ;   //5866~586F
} ToolstatusInfo_t;

/*----工装信息---*/
typedef struct
{
    ToolCtrlInfo_t     ctrl_info;
    ToolstatusInfo_t   status_info;
} ToolInfo_t;




bool CheckUartIsNormal(void);
uint16_t TestFlashIsOk(void);
void TestToolsProcess(void);

void GetStableGpioBitData(void);

extern ControlPara_t  gControlParaData;


void ToolingTestRs485Init(void);

#endif

