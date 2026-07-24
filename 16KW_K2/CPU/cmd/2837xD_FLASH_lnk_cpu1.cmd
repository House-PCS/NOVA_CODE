
MEMORY
{
PAGE 0 :  /* Program Memory */
          /* Memory (RAM/FLASH) blocks can be moved to PAGE1 for data allocation */
          /* BEGIN is used for the "boot to Flash" bootloader mode   */

   BEGIN            : origin = 0x080000, length = 0x000002
//   BEGIN            : origin = 0x098000, length = 0x000002
//   BEGIN            : origin = 0x098000, length = 0x000010
   RAMM0            : origin = 0x000122, length = 0x0002DE
   RAMD0            : origin = 0x00B000, length = 0x000800
   RAMLS0_3         : origin = 0x008000, length = 0x002000   /* CLA程序区: LS0+LS1+LS2+LS3+LS4 共5块 */
 /*RAMLS0           : origin = 0x008000, length = 0x000800
   RAMLS1           : origin = 0x008800, length = 0x000800
   RAMLS2           : origin = 0x009000, length = 0x000800
   RAMLS3           : origin = 0x009800, length = 0x000800
   RAMLS4           : origin = 0x00A000, length = 0x000800*/

   RAMLS4_5         : origin = 0x00A000, length = 0x001000   /* CLA数据区: 仅LS5 (.bss_cla+CLA1mathTables) */
 /*RAMLS5           : origin = 0x00A800, length = 0x000800*/

   RESET            : origin = 0x3FFFC0, length = 0x000002
   IQTABLES         : origin = 0x3FE000, length = 0x000b50
   IQTABLES2        : origin = 0x3FEB50, length = 0x00008c
   FPUTABLES        : origin = 0x3FEBDC, length = 0x0006A0
   BOOTROM          : origin = 0x3FF27C, length = 0x000D44


   /* Flash sectors FlashA-FLASHE 给boot预留*/
//   FLASHA           : origin = 0x080002, length = 0x001FFE  /* on-chip Flash */
//   FLASHB           : origin = 0x082000, length = 0x002000  /* on-chip Flash */
//   FLASHC           : origin = 0x084000, length = 0x002000  /* on-chip Flash */
//   FLASHD           : origin = 0x086000, length = 0x002000  /* on-chip Flash */
//   FLASHE           : origin = 0x088000, length = 0x008000  /* on-chip Flash */

   PROGRAM_SECTOR   : origin = 0x098010, length = 0x01FFF0    /*  CPU    Flash */
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

   RAMGS0_3    : origin = 0x00C000, length = 0x004000
/* RAMGS0      : origin = 0x00C000, length = 0x001000
   RAMGS1      : origin = 0x00D000, length = 0x001000
   RAMGS2      : origin = 0x00E000, length = 0x001000
   RAMGS3      : origin = 0x00F000, length = 0x001000*/
   RAMGS4_7    : origin = 0x010000, length = 0x002000
/* RAMGS4      : origin = 0x010000, length = 0x001000
   RAMGS5      : origin = 0x011000, length = 0x001000
   RAMGS6      : origin = 0x012000, length = 0x001000
   RAMGS7      : origin = 0x013000, length = 0x001000*/
   RAMGS8_14   : origin = 0x012000, length = 0x009000
//   RAMGS9      : origin = 0x015000, length = 0x001000
//   RAMGS10     : origin = 0x016000, length = 0x001000
//   RAMGS11     : origin = 0x017000, length = 0x001000
//   RAMGS12     : origin = 0x018000, length = 0x001000
//   RAMGS13     : origin = 0x019000, length = 0x001000
//   RAMGS14     : origin = 0x01A000, length = 0x001000
   RAMGS15     : origin = 0x01B000, length = 0x001000

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

#ifdef __TI_COMPILER_VERSION__
    #if __TI_COMPILER_VERSION__ >= 15009000
    .TI.ramfunc : {} LOAD = PROGRAM_SECTOR,
                         RUN = RAMGS0_3,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         ALIGN(8)
    #else
   ramfuncs            : LOAD = PROGRAM_SECTOR,
                         RUN = RAMGS0_3,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         ALIGN(8)
    #endif
#endif

   /* Allocate uninitalized data sections: */
   .stack              : > RAMM1        PAGE = 1
   .ebss               : > RAMGS8_14	PAGE = 1		//全局变量 存放数据 分配至RAM中
   .cio				   : > RAMGS4_7 	PAGE = 1
   .esysmem            : > RAMGS4_7     PAGE = 1

   /* Initalized sections go in Flash */
   .econst             : >> PROGRAM_SECTOR     PAGE = 0, ALIGN(8)
   .switch             : > PROGRAM_SECTOR      PAGE = 0, ALIGN(8)

   .reset              : > RESET,     	PAGE = 0, TYPE = DSECT /* not used, */

   Filter_RegsFile     : > RAMGS4_7,    PAGE = 1

   SHARERAMGS0      : > RAMGS4_7,     	PAGE = 1
   SHARERAMGS1      : > RAMGS4_7,    	PAGE = 1
   ramgs0           : > RAMGS4_7,     	PAGE = 1
   ramgs1           : > RAMGS4_7,     	PAGE = 1

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
   Filter1_RegsFile : > RAMGS15, PAGE = 1, fill=0x1111
   Filter2_RegsFile : > RAMGS15, PAGE = 1, fill=0x2222
   Filter3_RegsFile : > RAMGS15, PAGE = 1, fill=0x3333
   Filter4_RegsFile : > RAMGS15, PAGE = 1, fill=0x4444
   Difference_RegsFile : >RAMGS15,   PAGE = 1, fill=0x3333

   SFRA_F_Data      : > RAMGS15, ALIGN = 64, PAGE = 1

   SFRA_Data       : > RAMGS15, ALIGN = 64, PAGE = 1

   FPUmathTables   : > FPUTABLES, PAGE = 0, TYPE = NOLOAD

   /* CLA specific sections
    * Memory layout (ALL CLA direct-accessed data MUST be <= 0xFFFF):
    *   RAMLS0_4 (0x8000-0xA7FF): CLA program (5x LS-RAM)
    *   RAMLS5   (0xA800-0xAFFF): .bss_cla only (CLA global vars)
    *   RAMLS5 (0xA800-0xAFFF): Cla1DataRam + CLA1mathTables
    *     (CLAsin/cos/asin assembly uses R_ABS16_OC 16-bit direct addressing
    *      to access table symbols - must be <= 0xFFFF, RAMGS4_7 at 0x10000+ is ILLEGAL)
    */

   .scratchpad      : > RAMLS4_5,             PAGE = 0
   .bss_cla         : > RAMLS4_5,             PAGE = 0

   Cla1DataRam      : > RAMLS4_5,             PAGE = 0   /* 必须在 0xFFFF 以内，CLA 16-bit 直接寻址 */
   cla_shared       : > RAMLS4_5,             PAGE = 0
   CLADataLS1       : > RAMLS4_5,             PAGE = 0
   CLAFuncs         : > RAMLS4_5,             PAGE = 0

   Cla1ToCpuMsgRAM  : > CLA1_MSGRAMLOW,   	PAGE = 1
   CpuToCla1MsgRAM  : > CLA1_MSGRAMHIGH,  	PAGE = 1


   .const_cla      :  LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS4_5,
                      RUN_START(_Cla1ConstRunStart),
                      LOAD_START(_Cla1ConstLoadStart),
                      LOAD_SIZE(_Cla1ConstLoadSize)

   Cla1Prog        :  LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS0_3,
                      LOAD_START(_Cla1funcsLoadStart),
                      LOAD_END(_Cla1funcsLoadEnd),
                      RUN_START(_Cla1funcsRunStart),
                      LOAD_SIZE(_Cla1funcsLoadSize)

   CLA1mathTables   : LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS4_5,
                      RUN_START(_CLA1mathTablesRunStart),
                      LOAD_START(_CLA1mathTablesLoadStart),
                      LOAD_SIZE(_CLA1mathTablesLoadSize)
}
/*
//===========================================================================
// End of file.
//===========================================================================
*/
