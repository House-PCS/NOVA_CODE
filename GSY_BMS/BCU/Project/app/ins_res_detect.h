/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        ins_res_detect.h
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
#ifndef __INS_RES_DETECT_H_
#define __INS_RES_DETECT_H_

#include "dev_ht7017.h"
#include "dev_ioctl.h"
#include "common.h"

typedef enum
{
    EN_INS_RES_DETECT_STAT_NONE = 0,//未开始
    EN_INS_RES_DETECT_STAT_ING,//进行中
    EN_INS_RES_DETECT_STAT_COMPLETE,//已完成
    EN_INS_RES_DETECT_STAT_ERROR,//错误
}EN_INS_RES_DETECT_STAT;


typedef enum
{
    EN_ELEC_METER_OP_STAT_IDLE,
    EN_ELEC_METER_OP_STAT_BUSY,
}EN_ELEC_METER_OP_STAT;




//用于非阻塞式初始化
typedef enum
{
    EN_INS_RES_EXEC_STEP_WPCFG,
    EN_INS_RES_EXEC_STEP_ANAEN,
    EN_INS_RES_EXEC_STEP_ANAEN_READ,
    EN_INS_RES_EXEC_STEP_MODULE_EN,
    EN_INS_RES_EXEC_STEP_MODULE_EN_READ,
}EN_INS_RES_INIT_STEP;


typedef enum
{
    EN_INS_RES_EXEC_STEP_IDLE,
    EN_INS_RES_EXEC_STEP_ELEC_METER_INIT,//电量计初始化
    EN_INS_RES_EXEC_STEP_S1_S3_CLOSE,//闭合U24继电器，导通Q1光耦, 断开Q3光耦
    EN_INS_RES_EXEC_STEP_WAIT1,//等待U24继电器和Q1光耦完成闭合
    EN_INS_RES_EXEC_STEP_GET_VP1,//采集_vp1
    EN_INS_RES_EXEC_STEP_GET_VN1,//采集_vn1
    EN_INS_RES_EXEC_STEP_S2_CLOSE,//导通Q3光耦, 断开Q1光耦
    EN_INS_RES_EXEC_STEP_WAIT2,//等待Q3光耦完成闭合
    EN_INS_RES_EXEC_STEP_GET_VP2,//采集_vp2
    EN_INS_RES_EXEC_STEP_GET_VN2,//采集_vn2
    EN_INS_RES_EXEC_STEP_CALC,//计算Rp,Rn,Vbat，并断开U24继电器，Q1，Q3光耦
    EN_INS_RES_EXEC_STEP_COMPLETE,//标记完成
    EN_INS_RES_EXEC_STEP_GAP,//多次循环检测间隔时间等待
    EN_INS_RES_EXEC_STEP_FAIL,//过程测试失败
}EN_INS_RES_EXEC_STEP;


typedef struct _ins_res_info_stru_t
{
    EN_INS_RES_DETECT_STAT      detect_stat;//绝缘检测状态
    uint32_t                    bp_res;//总正阻抗，单位：kΩ
    uint32_t                    bn_res;//总负阻抗，单位：kΩ
    uint32_t                    bat_volt;//通过绝缘检测电路测量计算出的电池电压，单位：V
}InsResInfo_t;


extern InsResInfo_t gInsResInfo;


void InsResDetectStart(void);
void InsResDetectStop(void);
void InsResDetectProcess(void);

#endif



