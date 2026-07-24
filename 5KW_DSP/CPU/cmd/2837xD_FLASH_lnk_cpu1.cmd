
MEMORY
{
PAGE 0 :  /* Program Memory */
          /* Memory (RAM/FLASH) blocks can be moved to PAGE1 for data allocation */
          /* BEGIN is used for the "boot to Flash" bootloader mode   */

   //BEGIN            : origin = 0x080000, length = 0x000002
   BEGIN            : origin = 0x098000, length = 0x000010
   RAMM0            : origin = 0x000122, length = 0x0002DE
   RAMD0            : origin = 0x00B000, length = 0x000800
   RAMLS0           : origin = 0x008000, length = 0x000800
   RAMLS1           : origin = 0x008800, length = 0x000800
   RAMLS2           : origin = 0x009000, length = 0x000800
   RAMLS3           : origin = 0x009800, length = 0x000800
   RAMLS4_5         : origin = 0x00A000, length = 0x001000
   RAMGS14          : origin = 0x01A000, length = 0x001000     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   RAMGS15          : origin = 0x01B000, length = 0x001000     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
   RESET            : origin = 0x3FFFC0, length = 0x000002
   IQTABLES         : origin = 0x3FE000, length = 0x000b50
   IQTABLES2        : origin = 0x3FEB50, length = 0x00008c
   FPUTABLES        : origin = 0x3FEBDC, length = 0x0006A0
   BOOTROM          : origin = 0x3FF27C, length = 0x000D44


   /* Flash sectors */
//   FLASHA           : origin = 0x080002, length = 0x001FFE  /* on-chip Flash */
//   FLASHB           : origin = 0x082000, length = 0x002000  /* on-chip Flash */
//   FLASHC           : origin = 0x084000, length = 0x002000  /* on-chip Flash */
//   FLASHD           : origin = 0x086000, length = 0x002000  /* on-chip Flash */
//   FLASHE           : origin = 0x088000, length = 0x008000  /* on-chip Flash */

   PROGRAM_SECTOR   : origin = 0x098010, length = 0x01FFF0
   //FLASH_CLA		: origin = 0x0B8000, length = 0x002000    /*  CLA1   Flash */
   //FLASHF           : origin = 0x090110, length = 0x007EF0  /* on-chip Flash */
   //FLASHG           : origin = 0x098000, length = 0x008000  /* on-chip Flash */
   //FLASHH           : origin = 0x0A0000, length = 0x008000  /* on-chip Flash */
   //FLASHI           : origin = 0x0A8000, length = 0x008000  /* on-chip Flash */
   //FLASHJ           : origin = 0x0B0000, length = 0x008000  /* on-chip Flash */
   //FLASHK           : origin = 0x0B8000, length = 0x002000  /* on-chip Flash */
   //FLASHL           : origin = 0x0BA000, length = 0x002000  /* on-chip Flash */
   //FLASHM           : origin = 0x0BC000, length = 0x002000  /* on-chip Flash */
   //FLASHN           : origin = 0x0BE000, length = 0x002000  /* on-chip Flash */

PAGE 1 : /* Data Memory */
         /* Memory (RAM/FLASH) blocks can be moved to PAGE0 for program allocation */

   BOOT_RSVD       : origin = 0x000002, length = 0x000120     /* Part of M0, BOOT rom will use this for stack */
   RAMM1           : origin = 0x000400, length = 0x000400     /* on-chip RAM block M1 */
   RAMD1           : origin = 0x00B800, length = 0x000800

   //RAMLS5      : origin = 0x00A800, length = 0x000800

   RAMGS0      : origin = 0x00C000, length = 0x001000
   RAMGS1      : origin = 0x00D000, length = 0x001000
   RAMGS2      : origin = 0x00E000, length = 0x001000
   RAMGS3      : origin = 0x00F000, length = 0x001000
   RAMGS4      : origin = 0x010000, length = 0x001000
   RAMGS5      : origin = 0x011000, length = 0x001000
   RAMGS6_13      : origin = 0x012000, length = 0x008000
//   RAMGS6      : origin = 0x012000, length = 0x001000
//   RAMGS7      : origin = 0x013000, length = 0x001000
//   RAMGS8      : origin = 0x014000, length = 0x001000
//   RAMGS9      : origin = 0x015000, length = 0x001000
//   RAMGS10     : origin = 0x016000, length = 0x001000
//   RAMGS11     : origin = 0x017000, length = 0x001000
//   RAMGS12     : origin = 0x018000, length = 0x001000     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */
//   RAMGS13     : origin = 0x019000, length = 0x001000     /* Only Available on F28379D, F28377D, F28375D devices. Remove line on other devices. */

   CPU2TOCPU1RAM   : origin = 0x03F800, length = 0x000400
   CPU1TOCPU2RAM   : origin = 0x03FC00, length = 0x000400

   CLA1_MSGRAMLOW   : origin = 0x001480,   length = 0x000080
   CLA1_MSGRAMHIGH  : origin = 0x001500,   length = 0x000080


}

SECTIONS
{
   /* Allocate program areas: */
   .cinit              : > PROGRAM_SECTOR      PAGE = 0, ALIGN(8)
   .pinit              : > PROGRAM_SECTOR,     PAGE = 0, ALIGN(8)
   .text               : >> PROGRAM_SECTOR      PAGE = 0, ALIGN(8)
   codestart           : > BEGIN       PAGE = 0, ALIGN(8)

       /* CLA specific sections */
#if defined(__TI_EABI__)
	Cla1Prog    : LOAD = PROGRAM_SECTOR,
                  RUN = RAMLS4_5,
                  LOAD_START(Cla1funcsLoadStart),
                  LOAD_END(Cla1funcsLoadEnd),
                  RUN_START(Cla1funcsRunStart),
                  LOAD_SIZE(Cla1funcsLoadSize),
                  PAGE = 0, ALIGN(4)
#else
  	Cla1Prog    : LOAD = PROGRAM_SECTOR,
                  RUN = RAMLS4_5,
                  LOAD_START(_Cla1funcsLoadStart),
                  LOAD_END(_Cla1funcsLoadEnd),
                  RUN_START(_Cla1funcsRunStart),
                  LOAD_SIZE(_Cla1funcsLoadSize),
                  PAGE = 0, ALIGN(4)
#endif

#ifdef __TI_COMPILER_VERSION__
    #if __TI_COMPILER_VERSION__ >= 15009000
    .TI.ramfunc : {} LOAD = PROGRAM_SECTOR,
                         RUN = RAMLS0 | RAMLS1,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         PAGE = 0, ALIGN(8)
    #else
   ramfuncs            : LOAD = PROGRAM_SECTOR,
                         RUN = RAMLS0 | RAMLS1,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         PAGE = 0, ALIGN(8)
    #endif
#endif

   /* Allocate uninitalized data sections: */
   .stack              : > RAMM1       PAGE = 1
   .ebss               : > RAMGS6_13   PAGE = 1
   .esysmem            : > RAMGS6_13   PAGE = 1

   /* Initalized sections go in Flash */
   .econst             : >> PROGRAM_SECTOR      PAGE = 0, ALIGN(8)
   .switch             : > PROGRAM_SECTOR      PAGE = 0, ALIGN(8)

   .reset              : > RESET,     PAGE = 0, TYPE = DSECT /* not used, */

   Filter_RegsFile     : > RAMGS0,     PAGE = 1

   SHARERAMGS0      : > RAMGS0,     PAGE = 1
   SHARERAMGS1      : > RAMGS1,     PAGE = 1
   ramgs0           : > RAMGS0,     PAGE = 1
   ramgs1           : > RAMGS1,     PAGE = 1

   CPU1TOCPU2DATA	: > CPU1TOCPU2RAM, PAGE = 1,ALIGN(8)
   CPU2TOCPU1DATA	: > CPU2TOCPU1RAM, PAGE = 1,ALIGN(8)

   /* The following section definitions are required when using the IPC API Drivers */
    GROUP : > CPU1TOCPU2RAM, PAGE = 1
    {
        PUTBUFFER
        PUTWRITEIDX
        GETREADIDX
    }

    GROUP : > CPU2TOCPU1RAM, PAGE = 1
    {
        GETBUFFER :    TYPE = DSECT
        GETWRITEIDX :  TYPE = DSECT
        PUTREADIDX :   TYPE = DSECT
    }

   /* The following section definition are for SDFM examples */
   Filter1_RegsFile : > RAMGS1, PAGE = 1, fill=0x1111
   Filter2_RegsFile : > RAMGS2, PAGE = 1, fill=0x2222
   Filter3_RegsFile : > RAMGS3, PAGE = 1, fill=0x3333
   Filter4_RegsFile : > RAMGS4, PAGE = 1, fill=0x4444
   Difference_RegsFile : >RAMGS5,   PAGE = 1, fill=0x3333

   SFRA_F_Data      : > RAMGS2, ALIGN = 64, PAGE = 1

   SFRA_Data       : > RAMGS2, ALIGN = 64, PAGE = 1

   FPUmathTables   : > FPUTABLES, PAGE = 0, TYPE = NOLOAD

      /* CLA specific sections */

   .scratchpad      : > RAMLS3,           PAGE = 0
   .bss_cla         : > RAMLS2,           PAGE = 0

   Cla1DataRam      : > RAMLS3,           PAGE = 0
   cla_shared       : > RAMLS2,           PAGE = 0
   CLADataLS1       : > RAMLS2,           PAGE = 0
   CLAFuncs			: > RAMLS2,			  PAGE = 0

   Cla1ToCpuMsgRAM  : > CLA1_MSGRAMLOW,   PAGE = 1
   CpuToCla1MsgRAM  : > CLA1_MSGRAMHIGH,  PAGE = 1

   /* CLA C compiler sections */
    #if defined(__TI_EABI__)
   .const_cla      : LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS2,
                      RUN_START(Cla1ConstRunStart),
                      LOAD_START(Cla1ConstLoadStart),
                      LOAD_SIZE(Cla1ConstLoadSize),
                      PAGE = 0,
                      ALIGN(4)
  #else
   .const_cla      : LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS2,
                      RUN_START(_Cla1ConstRunStart),
                      LOAD_START(_Cla1ConstLoadStart),
                      LOAD_SIZE(_Cla1ConstLoadSize),
                      PAGE = 0,
                      ALIGN(4)
  #endif
}

/*
//===========================================================================
// End of file.
//===========================================================================
*/
