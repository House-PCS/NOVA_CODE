/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bmu_ctl.h
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
  * <tr><td>2024-09-09    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __BMU_CTL_H_
#define __BMU_CTL_H_


#include "bms_data_manage.h"
#include "common.h"




//地址分配+掩码发送
typedef enum
{
    EN_BMU_CTL_CFG_STAT_START,//配置启动，如从控已分配了地址，则清除并重新等待地址分配
    EN_BMU_CTL_CFG_STAT_START_WAIT,//配置复位等待
    EN_BMU_CTL_CFG_STAT_ADDR_DISTR,//各个从控要分配的地址下发
    EN_BMU_CTL_CFG_STAT_ADDR_DISTR_WAIT,//各个从控要分配的地址下发后的等待
    EN_BMU_CTL_CFG_STAT_ADDR_LATCH,//地址锁存命令，要求各个从控存储分配的地址
    EN_BMU_CTL_CFG_STAT_ADDR_LATCH_WAIT,//地址锁存响应等待
    EN_BMU_CTL_CFG_STAT_MASK_SEND,
    EN_BMU_CTL_CFG_STAT_MASK_SEND_WAIT,
    EN_BMU_CTL_CFG_STAT_OK,//配置成功
    EN_BMU_CTL_CFG_STAT_ERROR,//配置错误
}EN_BMU_CTL_CFG_STAT;




typedef struct _bmu_cfg_info_stru_t
{
    uint8_t         cur_addr_offset;//从0xF4开始偏移的第几个设备地址
    bool            start_flag;//启动地址分配+掩码发送，从控收到该指令后，先清除之前已分配的地址，并将双向IO恢复至初始状态
    bool            distr_flag;//地址分配下发flag
    bool            distr_ack;//地址分配响应
    bool            latch_flag;//地址锁存下发flag
    uint8_t         latch_ack[PACK_NUM_MAX];//地址锁存响应,0表示未响应，latch是广播，所以需要接受响应n次(n为从控数量)
    bool            mask_flag;//mask下发flag
    uint8_t         mask_ack[PACK_NUM_MAX];//mask下发响应,0表示未响应，mask是广播，所以需要接受响应n次(n为从控数量)
}BMUCfgInfo_t;



typedef struct _bmu_ctl_info_stru_t
{
    bool            ctl_flag[PACK_NUM_MAX];//控制报文flag
    bool            passive_balance_flag[PACK_NUM_MAX];//被动均衡报文flag
}BMUCtlInfo_t;

extern BMUCfgInfo_t gBMUCfgInfo;
extern BMUCtlInfo_t gBMUCtlInfo;

EN_OP_RES BMUCfgPolling(void);
void BMUCtlProcess(void);

#endif



