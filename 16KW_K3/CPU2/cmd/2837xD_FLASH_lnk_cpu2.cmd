
MEMORY
{
PAGE 0 :  /* Program memory */
          /* RAM/FLASH blocks may also be allocated on PAGE 1 for data */
          /* BEGIN is used for "boot to Flash" bootloader mode */

   RAMLS0_4			: origin = 0x008000, length = 0x002800
 /*RAMLS0           : origin = 0x008000, length = 0x000800
   RAMLS1           : origin = 0x008800, length = 0x000800
   RAMLS2           : origin = 0x009000, length = 0x000800
   RAMLS3           : origin = 0x009800, length = 0x000800
   RAMLS4           : origin = 0x00A000, length = 0x000800*/
   RAMLS5			: origin = 0x00A800, length = 0x00800
   RAMD1           : origin = 0x00B800, length = 0x000800     /* D1: reserved (PAGE 0) */

   RESET            : origin = 0x3FFFC0, length = 0x000002     /* Reset vector */
   IQTABLES         : origin = 0x3FE000, length = 0x000b50     /* IQ math lookup tables */
   IQTABLES2        : origin = 0x3FEB50, length = 0x00008c
   FPUTABLES        : origin = 0x3FEBDC, length = 0x0006A0     /* FPU math lookup tables */
   BOOTROM          : origin = 0x3FF27C, length = 0x000D44     /* Boot ROM */

   /* Flash sectors Flash0-FLASH4 reserved for boot */
   //FLASH 0          : origin = 0x080000, length = 0x002000  /* on-chip Flash */
   //FLASH 1          : origin = 0x082000, length = 0x002000  /* on-chip Flash */
   //FLASH 2          : origin = 0x084000, length = 0x002000  /* on-chip Flash */
   //FLASH 3          : origin = 0x086000, length = 0x002000  /* on-chip Flash */
   //FLASH 4          : origin = 0x088000, length = 0x008000  /* on-chip Flash */

//   BEGIN              : origin = 0x098000, length = 0x000002
   BEGIN           	  : origin = 0x080000, length = 0x000002       /* Code entry point */
   PROGRAM_SECTOR     : origin = 0x098010, length = 0x01FFF0  /* CPU application Flash */
   //FLASH 5          : origin = 0x090000, length = 0x008000  /* on-chip Flash */
   //FLASH 6          : origin = 0x098000, length = 0x008000  /* on-chip Flash */
   //FLASH 7          : origin = 0x0A0000, length = 0x008000  /* on-chip Flash */
   //FLASH 8          : origin = 0x0A8000, length = 0x008000  /* on-chip Flash */
   //FLASH 9          : origin = 0x0B0000, length = 0x008000  /* on-chip Flash */
   //FLASH 10         : origin = 0x0B8000, length = 0x002000  /* on-chip Flash */
   //FLASH 11         : origin = 0x0BA000, length = 0x002000  /* on-chip Flash */
   //FLASH 12         : origin = 0x0BC000, length = 0x002000  /* on-chip Flash */
   //FLASH 13         : origin = 0x0BE000, length = 0x002000  /* on-chip Flash */

PAGE 1 : /* Data memory */
         /* RAM/FLASH blocks may also be allocated on PAGE 0 for program code */

   BOOT_RSVD       : origin = 0x000002, length = 0x00007E     /* M0 low region: Boot ROM stack */
   RAMM0           : origin = 0x000122, length = 0x0002DE     /* M0: .cio, Filter_RegsFile (PAGE 1) */
   RAMM1           : origin = 0x000400, length = 0x000400     /* M1: heap (.esysmem) */
   RAMD0           : origin = 0x00B000, length = 0x000800     /* D0: stack only (PAGE 1) */

//   RAMGS0_3    : origin = 0x00C000, length = 0x004000
   RAMGS0      : origin = 0x00C000, length = 0x001000
   RAMGS1      : origin = 0x00D000, length = 0x001000
   RAMGS2      : origin = 0x00E000, length = 0x001000
   RAMGS3      : origin = 0x00F000, length = 0x001000
   RAMGS4      : origin = 0x010000, length = 0x001000
   RAMGS5_15   : origin = 0x011000, length = 0x00B000   /* GS5-GS15, 11 x 4KB */
//   RAMGS5      : origin = 0x011000, length = 0x001000
//   RAMGS6      : origin = 0x012000, length = 0x001000
//   RAMGS7      : origin = 0x013000, length = 0x001000
//   RAMGS8      : origin = 0x014000, length = 0x001000
//   RAMGS9      : origin = 0x015000, length = 0x001000
//   RAMGS10     : origin = 0x016000, length = 0x001000
//   RAMGS11     : origin = 0x017000, length = 0x001000
//   RAMGS12     : origin = 0x018000, length = 0x001000
//   RAMGS13     : origin = 0x019000, length = 0x001000
//   RAMGS14     : origin = 0x01A000, length = 0x001000
//   RAMGS15     : origin = 0x01B000, length = 0x001000

   CPU2TOCPU1RAM   : origin = 0x03F800, length = 0x000400       /* CPU2 to CPU1 IPC message RAM */
   CPU1TOCPU2RAM   : origin = 0x03FC00, length = 0x000400

   CLA1_MSGRAMLOW   : origin = 0x001480,   length = 0x000080    /* CLA to CPU message RAM */
   CLA1_MSGRAMHIGH  : origin = 0x001500,   length = 0x000080    /* CPU to CLA message RAM */
}

SECTIONS
{
   /* Program sections */
   .cinit              : > PROGRAM_SECTOR      PAGE = 0, ALIGN(8)    /* C initialization tables */
   .pinit              : > PROGRAM_SECTOR,     PAGE = 0, ALIGN(8)    /* C++ constructor tables */
   .text               : >> PROGRAM_SECTOR      PAGE = 0, ALIGN(8)   /* Program code */
   codestart           : > BEGIN       PAGE = 0, ALIGN(8)            /* Code startup */

#ifdef __TI_COMPILER_VERSION__
    #if __TI_COMPILER_VERSION__ >= 15009000
    .TI.ramfunc : {} LOAD = PROGRAM_SECTOR,
                         RUN = RAMD1,                         /* Copy from Flash, run in RAM */
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         ALIGN(8)
    #else
   ramfuncs            : LOAD = PROGRAM_SECTOR,
                         RUN = RAMD1,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         ALIGN(8)
    #endif
#endif

   /* Uninitialized data sections */
   .stack              : > RAMD0        PAGE = 1        /* Stack in D0 (2KB) */
   .ebss               : >> RAMGS0 | RAMGS1 | RAMGS2   PAGE = 1    /* Global/static vars in GS0-GS2 */
   .cio				   : > RAMM0 	PAGE = 1        /* C I/O buffers */
   .esysmem            : > RAMM1        PAGE = 1        /* Heap in M1 (1KB) */

   /* Initialized constants in Flash (copied to RAM at startup if needed) */
   .econst             : >> PROGRAM_SECTOR     PAGE = 0, ALIGN(8)   /* const data */
   .switch             : > PROGRAM_SECTOR      PAGE = 0, ALIGN(8)   /* switch jump tables */

   .reset              : > RESET,     	PAGE = 0, TYPE = DSECT /* Placeholder only, not loaded */

   Filter_RegsFile     : > RAMM0,    PAGE = 1

/* Dual-core GS shared sections: GS3/GS4 are reserved for CPU1/CPU2 shared globals */
   SHARERAMGS0      : > RAMGS3,       PAGE = 1
   SHARERAMGS1      : > RAMGS4,       PAGE = 1
   ramgs0           : > RAMGS3,       PAGE = 1
   ramgs1           : > RAMGS4,       PAGE = 1

   /* Required for IPC API drivers */
    GROUP : > CPU1TOCPU2RAM, PAGE = 1
    {
        GETBUFFER :    TYPE = DSECT
        GETWRITEIDX :  TYPE = DSECT
        PUTREADIDX :   TYPE = DSECT
    }

    GROUP : > CPU2TOCPU1RAM, PAGE = 1
    {
        PUTBUFFER
        PUTWRITEIDX
        GETREADIDX
    }

   /* SDFM example sections (disabled) */
//   Filter1_RegsFile : > RAMGS15, PAGE = 1, fill=0x1111
//   Filter2_RegsFile : > RAMGS15, PAGE = 1, fill=0x2222
//   Filter3_RegsFile : > RAMGS15, PAGE = 1, fill=0x3333
//   Filter4_RegsFile : > RAMGS15, PAGE = 1, fill=0x4444
//   Difference_RegsFile : >RAMGS15,   PAGE = 1, fill=0x3333

//   SFRA_F_Data      : > RAMGS15, ALIGN = 64, PAGE = 1      /* SFRA float data */

//   SFRA_Data       : > RAMGS15, ALIGN = 64, PAGE = 1

   FPUmathTables   : > FPUTABLES, PAGE = 0, TYPE = NOLOAD   /* On-chip ROM, do not load */

   /* CLA sections */

   .scratchpad      : > RAMLS5,           PAGE = 0    /* CLA scratchpad */
   .bss_cla         : > RAMLS5,           PAGE = 0    /* CLA uninitialized data */

   Cla1DataRam      : > RAMLS5,           PAGE = 0    /* CLA data RAM */
   cla_shared       : > RAMLS5,           PAGE = 0    /* CPU/CLA shared data */
   CLADataLS1       : > RAMLS5,           PAGE = 0    /* CLA LS1 data */
   CLAFuncs			: > RAMLS5,			PAGE = 0      /* CLA functions */

   Cla1ToCpuMsgRAM  : > CLA1_MSGRAMLOW,   	PAGE = 1   /* CLA to CPU message RAM */
   CpuToCla1MsgRAM  : > CLA1_MSGRAMHIGH,  	PAGE = 1   /* CPU to CLA message RAM */


   .const_cla      :  LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS5,
                      RUN_START(_Cla1ConstRunStart),
                      LOAD_START(_Cla1ConstLoadStart),
                      LOAD_SIZE(_Cla1ConstLoadSize)

	Cla1Prog       :  LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS0_4,
                      LOAD_START(_Cla1funcsLoadStart),
                      LOAD_END(_Cla1funcsLoadEnd),
                      RUN_START(_Cla1funcsRunStart),
                      LOAD_SIZE(_Cla1funcsLoadSize)

   CLA1mathTables   : LOAD = PROGRAM_SECTOR,
                      RUN = RAMLS5,
                      RUN_START(_CLA1mathTablesRunStart),
                      LOAD_START(_CLA1mathTablesLoadStart),
                      LOAD_SIZE(_CLA1mathTablesLoadSize)
}
/*
//===========================================================================
// End of file.
//===========================================================================
*/
