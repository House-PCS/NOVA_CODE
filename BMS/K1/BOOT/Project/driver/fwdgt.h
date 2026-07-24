/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        fwdgt.h
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


#ifndef __FWDGT_H_
#define __FWDGT_H_

#include "gd32f30x_fwdgt.h"
#include "drv_opt.h"
#include "bsp.h"

typedef enum
{
	EN_WDG_IDLE = 0,    /*¿ÕÏÐ*/
	EN_OPEN_WDG,  		/*¿ªÆô¿´ÃÅ¹·*/
	EN_STOP_FEED, 		/*Í£Ö¹Î¹¹·*/
	EN_WAIT_FEED, 		/*³ÖÐøÎ¹¹·*/
    
}en_wdg_state;

extern uint8_t gFwdgtRstFlag;

bool FwdgtInit(void);
void FwdgtProcess(void);
void FeedDog(void);
void FwdgtOpenCmd(void);

#endif















