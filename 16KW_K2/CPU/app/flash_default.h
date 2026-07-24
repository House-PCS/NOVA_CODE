/*
 * flash_default.h
 *
 *  Created on: 2026年1月30日
 *      Author: Nova007457
 */

 #ifndef FLASH_DEFAULT_H

#define FLASH_DEFAULT_H

#include "F28x_Project.h"



void Init_System_Parameter(void);
//bool Save_System_Parameter(UdpDspHoldPara_t* para);
void ShutDown_SaveFlash(void);
void Restore_Factory_Settings(void);
void UdpDspHoldPara_AutoSaveIfNeeded(void);



#endif 

