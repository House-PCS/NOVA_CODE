 /** 
 ****************************************************************************** 
 * @copyright (C) COPYRIGHT  Nova 
 * @file        dev_ext_wdg.c 
 * @author      Xi'an NovaStar Tech Co.,LTD. 
 * @version     V1.0.0 
 * @date       2026-6-1
 * @brief      
 ****************************************************************************** 
 * @attention 
 * @relation 
 * @par : 
 * <table> 
 * <tr><th>Date          <th>Version   <th>Author      <th>Description 
 * <tr><td>2024-12-25    <td>V1.0.0    <td>   <td>InitialVersion 
 * </table> 
 ****************************************************************************** 
 */ 

#include "dev_ext_wdg.h"

void DevFeedExtWdg(void)
{
    GpioBitToogle((&ExtWdgInHandle));
}





