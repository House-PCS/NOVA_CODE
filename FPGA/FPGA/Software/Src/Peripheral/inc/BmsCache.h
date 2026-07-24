//------------------------------------------------------------------------------
/*File Name         :   BmsCache.h
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   BmsCache库函数头文件

*/
//------------------------------------------------------------------------------

#ifndef _BMSCACHE_H
#define _BMSCACHE_H

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
}BMS_CACHE_TypeDef;

#define         BMS_CACHE           ((BMS_CACHE_TypeDef*) 0x30010000)

//------------------------------------------------------------------------------
/*Description       :   函数声明
*/
//------------------------------------------------------------------------------

void BmsCacheWr(int i, uint32_t Data);
uint32_t BmsCacheRd(int i);
void BmsCacheInit(void);
int BmsCacheTableTest(void);
#endif
