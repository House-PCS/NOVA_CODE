//------------------------------------------------------------------------------
/*File Name         :   DdrCache.c
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   TIMER固件库函数

*/
//------------------------------------------------------------------------------
#include "DdrCache.h"

//------------------------------------------------------------------------------
/*Function Name     :   DdrCacheWr 
**Description       :   DdrCache写操作
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------
void DdrCacheWr(int i, uint32_t Data)
{
    if(i == 0)
    {
        CACHE_DDR->DATA0 = Data;
    }
    else if(i == 1)
    {
        CACHE_DDR->DATA1 = Data;
    }
    else if(i == 2)
    {
        CACHE_DDR->DATA2 = Data;
    }
    else if(i == 3)
    {
        CACHE_DDR->DATA3 = Data;
    }
    else if(i == 4)
    {
        CACHE_DDR->DATA4 = Data;
    }
    else if(i == 5)
    {
        CACHE_DDR->DATA5 = Data;
    }
    else if(i == 6)
    {
        CACHE_DDR->DATA6 = Data;
    }
    else if(i == 7)
    {
        CACHE_DDR->DATA7 = Data;
    }
}

//------------------------------------------------------------------------------
/*Function Name     :   DdrCacheRd 
**Description       :   DdrCache读操作
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------
uint32_t DdrCacheRd(int i)
{
    uint32_t    Data ;

    if(i == 0)
    {
        Data = CACHE_DDR->DATA0;
    }
    else if(i == 1)
    {
        Data = CACHE_DDR->DATA1;
    }
    else if(i == 2)
    {
        Data = CACHE_DDR->DATA2;
    }

    else if(i == 3)
    {
        Data = CACHE_DDR->DATA3;
    }
    else if(i == 4)
    {
        Data = CACHE_DDR->DATA4;
    }
    else if(i == 5)
    {
        Data = CACHE_DDR->DATA5;
    }
    else if(i == 6)
    {
        Data = CACHE_DDR->DATA6;
    }
    else if(i == 7)
    {
        Data = CACHE_DDR->DATA7;
    }

    return Data;
}

//------------------------------------------------------------------------------
/*Function Name     :   DdrCacheInit 
**Description       :   DdrCache初始化
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------
void DdrCacheInit()
{
    DdrCacheWr(0, 0x04030201);
    DdrCacheWr(1, 0x08070605);
    DdrCacheWr(2, 0x0c0b0a09);
    DdrCacheWr(3, 0x100f0e0d);
    DdrCacheWr(4, 0x14131211);
    DdrCacheWr(5, 0x18171615);
    DdrCacheWr(6, 0x1c1b1a19);
    DdrCacheWr(7, 0x201f1e1d);
}

//------------------------------------------------------------------------------
/*Function Name     :   DdrCacheTest
**Description       :   DdrCache测试
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

int DdrCacheTest()
{
    if((DdrCacheRd(0) == 0x04030201) && (DdrCacheRd(1) == 0x08070605) && (DdrCacheRd(2) == 0x0c0b0a09) && (DdrCacheRd(3) == 0x100f0e0d)
        && (DdrCacheRd(4) == 0x14131211) && (DdrCacheRd(5) == 0x18171615) && (DdrCacheRd(6) == 0x1c1b1a19) && (DdrCacheRd(7) == 0x201f1e1d))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

