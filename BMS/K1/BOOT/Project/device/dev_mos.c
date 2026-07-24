/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.c
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

#include "dev_mos.h"
#include "fault_analysis.h"
#define LOG_TAG "dev_mos"



typedef struct
{
    bool n_cmos_is_open;
    bool n_dmos_is_open;
    bool n_pmos_is_open;
}_MosOpenStatus_t;

_MosOpenStatus_t _gMosOpenStatus;


/**
 * @brief  : open charge mos
 * @para   : None
 * @note   : None
 * @return : None
 */
void OpenChargeMos(void)
{
    if(true == ChargeMosIsOpen())
    {
        LOG_DEBUG(LOG_TAG, "ChargeMos Was Opened\r\n");
    }
    else
    {
        GpioBitSet((&CMosEnHandle));
        _gMosOpenStatus.n_cmos_is_open = true;
        LOG_DEBUG(LOG_TAG, "OpenChargeMos\r\n");
    }   
//    gFaultAnalysisFlag.chargemos_enable_flag = 1; 

}

/**
 * @brief  : close charge mos
 * @para   : None
 * @note   : None
 * @return : None
 */
void CloseChargeMos(void)
{
    if(true == ChargeMosIsOpen())
    {
        GpioBitReset((&CMosEnHandle));
        _gMosOpenStatus.n_cmos_is_open = false;
        LOG_DEBUG(LOG_TAG, "CloseChargeMos\r\n");
    }
    else
    {
        LOG_DEBUG(LOG_TAG, "ChargeMos Was Closed\r\n");
    } 
////    gFaultAnalysisFlag.chargemos_enable_flag = 0;     
}

/**
 * @brief  : get mos status
 * @para   : None
 * @note   : None
* @return : SET:true   RESET:false
 */
bool ChargeMosIsOpen(void)
{
    if(GpioOutputBitGet((&CMosEnHandle)) == SET)
    {
        return true;
    }
    else
    {
        return false;
    }
}
/**
 * @brief  : open Discharge mos
 * @para   : None
 * @note   : None
 * @return : None
 */
void OpenDisChargeMos(void)
{
    if(true == DisChargeMosIsOpen())
    {
        LOG_DEBUG(LOG_TAG, "DisChargeMos Was Opened\r\n");
    }
    else
    {
        GpioBitSet((&DMosEnHandle));
        _gMosOpenStatus.n_dmos_is_open = true;
        LOG_DEBUG(LOG_TAG, "OpenDisChargeMos\r\n");
    }  
}

/**
 * @brief  : close Discharge mos
 * @para   : None
 * @note   : None
 * @return : None
 */
void CloseDisChargeMos(void)
{
    if(true == DisChargeMosIsOpen())
    {
        GpioBitReset((&DMosEnHandle));
        _gMosOpenStatus.n_dmos_is_open = false;
        LOG_DEBUG(LOG_TAG, "CloseDisChargeMos\r\n");
    }
    else
    {
        LOG_DEBUG(LOG_TAG, "DisChargeMos Was Closed\r\n");
    }   
}

/**
 * @brief  : get mos status
 * @para   : None
 * @note   : None
* @return : SET:true   RESET:false
 */
bool DisChargeMosIsOpen(void)
{
    if(GpioOutputBitGet((&DMosEnHandle)) == SET)
    {
        return true;
    }
    else
    {
        return false;
    }
}
/**
 * @brief  : open Precharge mos
 * @para   : None
 * @note   : None
 * @return : None
 */
void OpenPreChargeMos(void)
{
    if(true == PreChargeMosIsOpen())
    {
        LOG_DEBUG(LOG_TAG, "PreChargeMos Was Opened\r\n");
    }
    else
    {
        GpioBitSet((&PMosEnHandle));
        _gMosOpenStatus.n_pmos_is_open = true;
        LOG_DEBUG(LOG_TAG, "OpenPreChargeMos\r\n");
    }
}

/**
 * @brief  : close Precharge mos
 * @para   : None
 * @note   : None
 * @return : None
 */
void ClosePreChargeMos(void)
{
    if(true == PreChargeMosIsOpen())
    {
        GpioBitReset((&PMosEnHandle));
        _gMosOpenStatus.n_pmos_is_open = false;
        LOG_DEBUG(LOG_TAG, "ClosePreChargeMos\r\n");
    }
    else
    {
        LOG_DEBUG(LOG_TAG, "PreChargeMos Was Closed\r\n");
    } 
}

/**
 * @brief  : get mos status
 * @para   : None
 * @note   : None
* @return : SET:true   RESET:false
 */
bool PreChargeMosIsOpen(void)
{
    if(GpioOutputBitGet((&PMosEnHandle)) == SET)
    {
        return true;
    }
    else
    {
        return false;
    }
}


void MosInit(void)
{
    /*MosBsp*/
    MosBspInit();
    
    /*×´Ì¬ÇåÁã*/
    _gMosOpenStatus.n_cmos_is_open = false;
    _gMosOpenStatus.n_dmos_is_open = false;
    _gMosOpenStatus.n_pmos_is_open = false;
    
}

