
#ifndef __SYS_INFO_H_
#define __SYS_INFO_H_


#include "config.h"
#include "dev_spiflash.h"
#include "pcs_op_kstar.h"

typedef enum
{
    EN_WIFI_MODULE_UNDETECTED,//WIFI模组检测不到
    EN_WIFI_MODULE_OFF_GRID,//WIFI模组未连接路由器
    EN_WIFI_MODULE_OFF_CLOUD,//WIFI模组未连接云平台
    EN_WIFI_MODULE_OK,//WIFI模组和云平台建立链接
}EN_MODULE_STATUS;

typedef enum
{
    EN_INV_COMM_FAIL,
    EN_INV_COMM_OK,
    EN_INV_COMM_ERROR_CODE
}EN_INV_COMM_STATUS;


typedef struct _collector_dev_info_stru_t
{
    uint16_t            read_pcs_period;//单位：S
    uint16_t            report_period;//单位：S
    uint8_t             sn[20];
    uint8_t             modelid[2];
    uint8_t             cloud_node_type;//0:IP， 1:网址
    uint8_t             cloud_node[128];
    
    uint32_t            pcs_info_updated;
    bool                kstar_sys_info_updated;
    uint8_t             kstar_pack_num;
    
    EN_MODULE_STATUS    wifi_module_flag;           
    EN_INV_COMM_STATUS  inv_comm_flag;
    
}CollectorDevInfo_t;

extern CollectorDevInfo_t gCollectorDevInfo;

extern uint8_t gPcsDevDebugModebusInfo[64];

extern uint8_t gCommonBaseInfo[2560];

void SysInfoInit();

#endif
