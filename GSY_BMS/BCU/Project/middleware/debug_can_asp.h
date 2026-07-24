#ifndef __DEBUG_CAN_ASP_H_
#define __DEBUG_CAN_ASP_H_
    
#include <stdint.h>
#include <stdbool.h>
#include "prot_can_debug.h"
#include "mem_priv.h"
    
typedef struct
{
    uint8_t ch_num;
    uint16_t total_bytes;
    bool (*read_data_fun)(uint8_t cnt,uint8_t* buff);        //
    bool (*write_data_fun)(uint8_t len,uint8_t* buff);
    uint16_t data_num;      //数据字段数,写数据时使用
}DebugMessageAspType;


typedef struct _production_test_stru_t
{
    bool    test_mode;
    uint8_t do_ctrl;
    uint8_t di_state;
}ProductionTestType;

//extern ProductionTestType ProductionTest;
void DebugAspCfgInit(void);

#endif
