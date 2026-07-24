/*
 * This file is part of the CmBacktrace Library.
 *
 * Copyright (c) 2016, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is the configure head file for this library.
 * Created on: 2016-12-15
 */

#ifndef _CMB_CFG_H_
#define _CMB_CFG_H_

#include "debug.h"
/* print line, must config by user */
#define cmb_println(...)               LOG_STRING("",__VA_ARGS__);LOG_STRING("","\r\n")

/* enable bare metal(no OS) platform */
#define CMB_USING_BARE_METAL_PLATFORM

#if defined(STM32H750xx)
/* cpu platform type, must config by user */
#define CMB_CPU_PLATFORM_TYPE          CMB_CPU_ARM_CORTEX_M7

#elif defined(STM32F2XX)
/* cpu platform type, must config by user */
#define CMB_CPU_PLATFORM_TYPE          CMB_CPU_ARM_CORTEX_M4 //CMB_CPU_ARM_CORTEX_M3  //实际 STM32F207 为M3，但实际多为GD32F450，为M4

#elif defined(GD32F450)
/* cpu platform type, must config by user */
#define CMB_CPU_PLATFORM_TYPE          CMB_CPU_ARM_CORTEX_M4

#elif defined(GD32F470)
/* cpu platform type, must config by user */
#define CMB_CPU_PLATFORM_TYPE          CMB_CPU_ARM_CORTEX_M4

#elif defined(GD32E10x)
/* cpu platform type, must config by user */
#define CMB_CPU_PLATFORM_TYPE          CMB_CPU_ARM_CORTEX_M4

#elif defined(GD32F30X_HD)
/* cpu platform type, must config by user */
#define CMB_CPU_PLATFORM_TYPE          CMB_CPU_ARM_CORTEX_M4

#elif defined(GD32E50X_HD)
/* cpu platform type, must config by user */
#define CMB_CPU_PLATFORM_TYPE           CMB_CPU_ARM_CORTEX_M33    //实际 GD32E503 为M33，但实际多为GD32F303，为M4

#else
    #error "Missing define:CMB_CPU_PLATFORM_TYPE in cmb_cfg.h"
#endif


/* enable dump stack information */
#define CMB_USING_DUMP_STACK_INFO
/* language of print information */
#define CMB_PRINT_LANGUAGE             CMB_PRINT_LANUUAGE_ENGLISH
#endif /* _CMB_CFG_H_ */
