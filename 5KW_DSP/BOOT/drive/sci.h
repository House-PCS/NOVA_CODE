/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sci.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __SCI_H__
#define __SCI_H__

#ifdef __cplusplus
extern "C" {
#endif

void SciaFifoInit(void);
void SciaSend(uint8_t* data, uint16_t len);
void SciaMsg(char * msg);
uint16_t GetSciaRecvFlag(void);
void ClearSciaRecvFlag(void);
void ClearScia(void);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
