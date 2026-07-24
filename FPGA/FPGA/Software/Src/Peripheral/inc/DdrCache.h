//------------------------------------------------------------------------------
/*File Name         :   DdrCache.h
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   TIMER固件库函数头文件

*/
//------------------------------------------------------------------------------

#ifndef _DDRCACHE_H
#define _DDRCACHE_H

//------------------------------------------------------------------------------
/*Description       :   头文件
*/
//------------------------------------------------------------------------------
#include "system_CMSDK_CM0.h"

//------------------------------------------------------------------------------
/*Description       :   宏定义
*/
//------------------------------------------------------------------------------

typedef struct
{
  __IO   uint32_t  DATA0;
  __IO   uint32_t  DATA1;
  __IO   uint32_t  DATA2;
  __IO   uint32_t  DATA3;
  __IO   uint32_t  DATA4;
  __IO   uint32_t  DATA5;
  __IO   uint32_t  DATA6;
  __IO   uint32_t  DATA7;
}DDR_CACHE_TypeDef;

#define CACHE_DDR                   ((DDR_CACHE_TypeDef   *) 0x20000000)

//------------------------------------------------------------------------------
/*Description       :   函数声明
*/
//------------------------------------------------------------------------------
void DdrCacheWr(int i, uint32_t Data);
uint32_t DdrCacheRd(int i);
void DdrCacheInit(void);
int DdrCacheTest(void);

#endif
