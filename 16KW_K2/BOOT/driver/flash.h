//###########################################################################
//
// FILE:    F2837xD_cla.h
//
// TITLE:   F2837xD Device CLA Register Definitions.
//
//###########################################################################
// $TI Release: F2837xD Support Library v210 $
// $Release Date: Tue Nov  1 14:46:15 CDT 2016 $
// $Copyright: Copyright (C) 2013-2016 Texas Instruments Incorporated -
//             http://www.ti.com/ ALL RIGHTS RESERVED $
//###########################################################################

#ifndef __FLASH_H__
#define __FLASH_H__

#include "F28x_Project.h"
#include <string.h>
#include "flash_programming_dcsm_c28.h"
#include "F021_F2837xD_C28x.h"
#ifdef __cplusplus
extern "C" {
#endif

bool InFlashInit(void);
bool InFlashErase(void);
bool InFlashWrite(uint32_t write_addr, uint16_t* data, uint16_t len);
bool InFlashRead(uint32_t read_addr, uint16_t* data, uint16_t len);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
