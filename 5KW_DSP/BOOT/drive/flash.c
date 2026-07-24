//###########################################################################
//
// FILE:   Example_2837xDSpi_FFDLB.c
//
// TITLE:  SPI Digital Loop Back program.
//
//! \addtogroup cpu01_example_list
//! <h1>SPI Digital Loop Back (spi_loopback)</h1>
//!
//!  This program uses the internal loop back test mode of the peripheral.
//!  Other then boot mode pin configuration, no other hardware configuration
//!  is required. Interrupts are not used.
//!
//!  A stream of data is sent and then compared to the received stream.
//!  The sent data looks like this: \n
//!  0000 0001 0002 0003 0004 0005 0006 0007 .... FFFE FFFF \n
//!  This pattern is repeated forever.
//!
//!  \b Watch \b Variables \n
//!  - \b sdata - sent data
//!  - \b rdata - received data
//!
//
//###########################################################################
//
// $Release Date: $
// $Copyright:
// Copyright (C) 2013-2023 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the
//   distribution.
//
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//###########################################################################

//
// Included Files
//
#include "flash.h"

bool InFlashInit(void)
{
    Fapi_StatusType oReturnCheck;

    EALLOW;
    DcsmCommonRegs.FLSEM.all = 0xA501;
    EDIS;

    EALLOW;
    Flash0EccRegs.ECC_ENABLE.bit.ENABLE = 0x0;

    oReturnCheck = Fapi_initializeAPI(F021_CPU0_BASE_ADDRESS, 200);

    if(oReturnCheck != Fapi_Status_Success)
    {
        //
        // Check Flash API documentation for possible errors.
        //
        return false;
    }

    oReturnCheck = Fapi_setActiveFlashBank(Fapi_FlashBank0);

    if(oReturnCheck != Fapi_Status_Success)
    {
        //
        // Check Flash API documentation for possible errors.
        //
        return false;
    }
    EDIS;
    return true;
}

bool InFlashErase(void)
{
    Fapi_StatusType oReturnCheck;
    Fapi_FlashStatusWordType oFlashStatusWord;
    //
    // Erase Sector F.
    //
    EALLOW;
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                   (uint32 *)Bzero_SectorF_start);
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    oReturnCheck = Fapi_doBlankCheck((uint32 *)Bzero_SectorF_start,
                                     Bzero_16KSector_u32length,
                                     &oFlashStatusWord);
    if(oReturnCheck != Fapi_Status_Success)
    {
        return false;
    }
    //
    // Erase Sector G.
    //
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                   (uint32 *)Bzero_SectorG_start);
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    oReturnCheck = Fapi_doBlankCheck((uint32 *)Bzero_SectorG_start,
                                     Bzero_16KSector_u32length,
                                     &oFlashStatusWord);
    if(oReturnCheck != Fapi_Status_Success)
    {
        return false;
    }
    //
    // Erase Sector H.
    //
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                   (uint32 *)Bzero_SectorH_start);
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    oReturnCheck = Fapi_doBlankCheck((uint32 *)Bzero_SectorH_start,
                                     Bzero_16KSector_u32length,
                                     &oFlashStatusWord);
    if(oReturnCheck != Fapi_Status_Success)
    {
        return false;
    }
    //
    // Erase Sector I.
    //
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                   (uint32 *)Bzero_SectorI_start);
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    oReturnCheck = Fapi_doBlankCheck((uint32 *)Bzero_SectorI_start,
                                     Bzero_16KSector_u32length,
                                     &oFlashStatusWord);
    if(oReturnCheck != Fapi_Status_Success)
    {
        return false;
    }
    //
    // Erase Sector J.
    //
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                   (uint32 *)Bzero_SectorJ_start);
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    oReturnCheck = Fapi_doBlankCheck((uint32 *)Bzero_SectorJ_start,
                                     Bzero_16KSector_u32length,
                                     &oFlashStatusWord);
    if(oReturnCheck != Fapi_Status_Success)
    {
        return false;
    }

    //
    // Erase Sector K.
    //
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                   (uint32 *)Bzero_SectorK_start);
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    oReturnCheck = Fapi_doBlankCheck((uint32 *)Bzero_SectorK_start,
                                     Bzero_16KSector_u32length,
                                     &oFlashStatusWord);
    if(oReturnCheck != Fapi_Status_Success)
    {
        return false;
    }

    //
    // Erase Sector L.
    //
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                   (uint32 *)Bzero_SectorL_start);
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    oReturnCheck = Fapi_doBlankCheck((uint32 *)Bzero_SectorL_start,
                                     Bzero_16KSector_u32length,
                                     &oFlashStatusWord);
    if(oReturnCheck != Fapi_Status_Success)
    {
        return false;
    }

    //
    // Erase Sector M.
    //
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                   (uint32 *)Bzero_SectorM_start);
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    oReturnCheck = Fapi_doBlankCheck((uint32 *)Bzero_SectorM_start,
                                     Bzero_16KSector_u32length,
                                     &oFlashStatusWord);
    if(oReturnCheck != Fapi_Status_Success)
    {
        return false;
    }

    //
    // Erase Sector N.
    //
    oReturnCheck = Fapi_issueAsyncCommandWithAddress(Fapi_EraseSector,
                   (uint32 *)Bzero_SectorN_start);
    while (Fapi_checkFsmForReady() != Fapi_Status_FsmReady)
    {
    }

    oReturnCheck = Fapi_doBlankCheck((uint32 *)Bzero_SectorN_start,
                                     Bzero_16KSector_u32length,
                                     &oFlashStatusWord);
    if(oReturnCheck != Fapi_Status_Success)
    {
        return false;
    }

    EDIS;

    return true;
}

bool InFlashWrite(uint32_t write_addr, uint16_t* data, uint16_t len)
{
    uint32 addr_index = 0UL;
    uint16 i = 0U;
    Fapi_StatusType oReturnCheck = Fapi_Status_Success;

    EALLOW;
    if((write_addr % 8 != 0) || (len % 8 != 0))
    {
        return false;
    }

    for(i = 0, addr_index = write_addr;
            (addr_index < (write_addr + len)) && (oReturnCheck == Fapi_Status_Success); i += 8, addr_index += 8)
    {
        oReturnCheck = Fapi_issueProgrammingCommand((uint32 *)addr_index, data + i,
                       8,
                       0,
                       0,
                       Fapi_AutoEccGeneration);

        //
        // Wait until FSM is done with program operation.
        //
        while(Fapi_checkFsmForReady() == Fapi_Status_FsmBusy)
        {
        }

        if(oReturnCheck != Fapi_Status_Success)
        {
            return false;
        }
    }
    EDIS;

    return true;
}

bool InFlashRead(uint32_t read_addr, uint16_t* data, uint16_t len)
{
    if((read_addr < Bzero_SectorA_start) || (read_addr > Bzero_SectorN_End))
    {
        return false;
    }

    memcpy(data, (uint16_t*)read_addr, len);

    return true;
}
