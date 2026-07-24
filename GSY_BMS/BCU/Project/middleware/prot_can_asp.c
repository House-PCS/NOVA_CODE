/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  
#include "prot_can_asp.h"
#include "prot_can_pack.h"
//#include "prot_can_pcs.h"
#include "prot_can_pcs_hs.h"
#include "prot_can_addr_bms.h"
#include "prot_can_pack_para.h"
#include "prot_can_lem.h"
#include "prot_can_test.h"
#include "prot_can_debug.h"
#include "debug_can_asp.h"
#include "prot_can_factory_test.h"



#define CAN0_PROT_NUM    2
PROT_INIT_FUN ProtInin_CAN0[CAN0_PROT_NUM] = {
//    CanProtInit_Addr, CanProtInit_Pack, CanProtInit_PackPara, 
    CanProtInit_Pcs, 
//    CanProtInit_Test, 
    CanProtInit_FuncTest,
};
#define CAN1_PROT_NUM    5
PROT_INIT_FUN ProtInin_CAN1[CAN1_PROT_NUM] = {
    //CanProtInit_Pcs,
    CanProtInit_Addr, CanProtInit_Pack, CanProtInit_PackPara,   
    CanProtInit_Test,CanProtInitDebug,    // CanProtInit_Lem,
    
};
//////////////////////PORT_CFG//////////////////////////
#define PORT_CAN_ASP_NUM   CAN_PORT_NUM          // CAN_PORT_NUM
PROT_INIT_FUN* CanProtAspCfgInfo[PORT_CAN_ASP_NUM] = {
    ProtInin_CAN0,
    ProtInin_CAN1,
};
CanProtInfoType   Can0ProtInfo[CAN0_PROT_NUM];
CanProtInfoType   Can1ProtInfo[CAN1_PROT_NUM];
CanPortProtGatherType CanPortProtGather[PORT_CAN_ASP_NUM]={
    {CAN0_PROT_NUM, Can0ProtInfo, &Queue_CanRx[CAN_0], &Queue_CanTx[CAN_0], 0},
    {CAN1_PROT_NUM, Can1ProtInfo, &Queue_CanRx[CAN_1], &Queue_CanTx[CAN_1], 0},
};
///////////////////Init////////////////////////////
/** 
* @brief  CAN协议ASP层初始化
* @param  none
* @return  none
* @note:
*
*/
void ProtCanAspInit()
{
    uint8_t i=0,j=0;
	
	DebugAspCfgInit();  //需放在for循环之前。
    for(i=0;i<PORT_CAN_ASP_NUM;i++)
    {
        for(j=0;j<CanPortProtGather[i].ProtNum;j++)
        {
            if(CanProtAspCfgInfo[i][j]!=NULL)
            {
                CanProtAspCfgInfo[i][j](&CanPortProtGather[i].CanProtCfgGather[j]);            
            }
        }
        
    }
//    for(i=0;i<CAN_PORT_NUM;i++)
//    {
//        for(j=0;j<CanPortProtGather[i].ProtNum;j++)
//        {
//            CanPortProtGather[i].CanProtCfgGather[j].;
//        }
//        
//    }
    CanProtGather.PortNum=CAN_PORT_NUM;
    CanProtGather.CanPortProtCfgGather =CanPortProtGather;
}
