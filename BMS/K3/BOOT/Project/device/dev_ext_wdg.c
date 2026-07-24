 /** 
 ****************************************************************************** 
 * @copyright (C) COPYRIGHT  Nova 
 * @file        dev_ext_wdg.c 
 * @author      Xi'an NovaStar Tech Co.,LTD. 
 * @version     V1.0.0 
 * @date       2024-12-25
 * @brief      
 ****************************************************************************** 
 * @attention 
 * @relation 
 * @par : 
 * <table> 
 * <tr><th>Date          <th>Version   <th>Author      <th>Description 
 * <tr><td>2024-12-25    <td>V1.0.0    <td>Xingwen Xu  <td>InitialVersion 
 * </table> 
 ****************************************************************************** 
 */ 

#include "dev_ext_wdg.h"

 /** 
 ******************************************************************************* 
 * Function: DevFeedExtWdg 
 * @brief:  feed external watchdog, DIO706S.
 * 
 * @details: feeddog by toogle pin of WDI.
 * 
 * Parameters: 
 * @param [in]  
 * 
 * @return: None 
 * 
 ******************************************************************************* 
 */ 
void DevFeedExtWdg(void)
{
    GpioBitToogle((&ExtWdgInHandle));
}

 /** 
 ******************************************************************************* 
 * Function: DevGetExtWdg 
 * @brief:  get external watchdog ststus. 
 * 
 * @details: get status by pin of WDO. 
 * 
 * Parameters: 
 * @param [in]  
 * 
 * @return: status of WDO. 
 * 
 ******************************************************************************* 
 */ 
uint8_t DevGetExtWdg(void)
{
    uint8_t rtn_value = 0;

    rtn_value = GpioBitGet((&ExtWdgInHandle));
    return rtn_value;
}


/************** END OF FILE ************/



