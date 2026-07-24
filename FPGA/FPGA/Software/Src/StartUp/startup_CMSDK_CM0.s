;/**************************************************************************//**
; * @file     startup_CMSDK_CM0.s
; * @brief    CMSIS Cortex-M0 Core Device Startup File for
; *           Device CMSDK_CM0
; * @version  V3.01
; * @date     06. March 2012
; *
; * @note
; * Copyright (C) 2012 ARM Limited. All rights reserved.
; *
; * @par
; * ARM Limited (ARM) is supplying this software for use with Cortex-M
; * processor based microcontrollers.  This file can be freely distributed
; * within development tools that are supporting such ARM based processors.
; *
; * @par
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; ******************************************************************************/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000200

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000100

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler
                DCD     INT00_Handler             ; UART 0 RX Handler
                DCD     INT01_Handler             ; UART 0 TX Handler
                DCD     INT02_Handler             ; UART 1 RX Handler
                DCD     INT03_Handler             ; UART 1 TX Handler
                DCD     INT04_Handler             ; UART 2 RX Handler
                DCD     INT05_Handler             ; UART 2 TX Handler
                DCD     INT06_Handler             ; GPIO Port 0 Combined Handler
                DCD     INT07_Handler             ; GPIO Port 1 Combined Handler
                DCD     INT08_Handler             ; TIMER 0 handler
                DCD     INT09_Handler             ; TIMER 1 handler
                DCD     INT10_Handler             ; Dual timer handler
                DCD     INT11_Handler             ; Combines SPI Handler
                DCD     INT12_Handler             ; UART Combined Overflow Handler
                DCD     INT13_Handler             ; Ethernet
                DCD     INT14_Handler             ; Audio I2S
                DCD     INT15_Handler             ; DMA handler
                DCD     INT16_Handler             ; GPIO Port 0 pin 0 Handler
                DCD     INT17_Handler             ; GPIO Port 0 pin 1 Handler
                DCD     INT18_Handler             ; GPIO Port 0 pin 2 Handler
                DCD     INT19_Handler             ; GPIO Port 0 pin 3 Handler
                DCD     INT20_Handler             ; GPIO Port 0 pin 4 Handler
                DCD     INT21_Handler             ; GPIO Port 0 pin 5 Handler
                DCD     INT22_Handler             ; GPIO Port 0 pin 6 Handler
                DCD     INT23_Handler             ; GPIO Port 0 pin 7 Handler
                DCD     INT24_Handler             ; GPIO Port 0 pin 8 Handler
                DCD     INT25_Handler             ; GPIO Port 0 pin 9 Handler
                DCD     INT26_Handler             ; GPIO Port 0 pin 10 Handler
                DCD     INT27_Handler             ; GPIO Port 0 pin 11 Handler
                DCD     INT28_Handler             ; GPIO Port 0 pin 12 Handler
                DCD     INT29_Handler             ; GPIO Port 0 pin 13 Handler
                DCD     INT30_Handler             ; GPIO Port 0 pin 14 Handler
                DCD     INT31_Handler             ; GPIO Port 0 pin 15 Handler
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R0, =SystemInit
                ; Initialise at least r8, r9 to avoid X in tests
                ; Only important for simulation where X can cause
                ; unexpected core behaviour
                MOV     R8, R0
                MOV     R9, R8
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP


; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
               EXPORT  SysTick_Handler            [WEAK]
               B       .
               ENDP
Default_Handler PROC
                EXPORT INT00_Handler         [WEAK]
                EXPORT INT01_Handler         [WEAK]
                EXPORT INT02_Handler         [WEAK]
                EXPORT INT03_Handler         [WEAK]
                EXPORT INT04_Handler         [WEAK]
                EXPORT INT05_Handler         [WEAK]
                EXPORT INT06_Handler         [WEAK]
                EXPORT INT07_Handler         [WEAK]
                EXPORT INT08_Handler         [WEAK]
                EXPORT INT09_Handler         [WEAK]
                EXPORT INT10_Handler         [WEAK]
                EXPORT INT11_Handler         [WEAK]
                EXPORT INT12_Handler         [WEAK]
                EXPORT INT13_Handler         [WEAK]
                EXPORT INT14_Handler         [WEAK]
                EXPORT INT15_Handler         [WEAK]
                EXPORT INT16_Handler         [WEAK]
                EXPORT INT17_Handler         [WEAK]
                EXPORT INT18_Handler         [WEAK]
                EXPORT INT19_Handler         [WEAK]
                EXPORT INT20_Handler         [WEAK]
                EXPORT INT21_Handler         [WEAK]
                EXPORT INT22_Handler         [WEAK]
                EXPORT INT23_Handler         [WEAK]
                EXPORT INT24_Handler         [WEAK]
                EXPORT INT25_Handler         [WEAK]
                EXPORT INT26_Handler         [WEAK]
                EXPORT INT27_Handler         [WEAK]
                EXPORT INT28_Handler         [WEAK]
                EXPORT INT29_Handler         [WEAK]
                EXPORT INT30_Handler         [WEAK]
                EXPORT INT31_Handler         [WEAK]
INT00_Handler
INT01_Handler
INT02_Handler
INT03_Handler
INT04_Handler
INT05_Handler
INT06_Handler
INT07_Handler
INT08_Handler
INT09_Handler
INT10_Handler
INT11_Handler
INT12_Handler
INT13_Handler
INT14_Handler
INT15_Handler
INT16_Handler
INT17_Handler
INT18_Handler
INT19_Handler
INT20_Handler
INT21_Handler
INT22_Handler
INT23_Handler
INT24_Handler
INT25_Handler
INT26_Handler
INT27_Handler
INT28_Handler
INT29_Handler
INT30_Handler
INT31_Handler
                B       .
                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                END
