//------------------------------------------------------------------------------
/*File Name         :   BmsCache.c
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   TIMER固件库函数

*/
//------------------------------------------------------------------------------
#include "BmsCache.h"

//------------------------------------------------------------------------------
/*Function Name     :   BmsCacheWr 
**Description       :   BmsCache写入参数
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void BmsCacheWr(int i, uint32_t Data)
{
    if(i == 0)
    {
        BMS_CACHE->DATA0 = Data;
    }
    else if(i == 1)
    {
        BMS_CACHE->DATA1 = Data;
    }
    else if(i == 2)
    {
        BMS_CACHE->DATA2 = Data;
    }
}

//------------------------------------------------------------------------------
/*Function Name     :   BmsCacheRd
**Description       :   BmsCache读出参数
**Input Param       :   int i:选择读取第几个寄存器
**Output Param      :
**Return value      :   return Data:返回读取的32bit数据
*/
//------------------------------------------------------------------------------

uint32_t BmsCacheRd(int i)
{
    uint32_t    Data ;

    if(i == 0)
    {
        Data = BMS_CACHE->DATA0;
    }
    else if(i == 1)
    {
        Data = BMS_CACHE->DATA1;
    }
    else if(i == 2)
    {
        Data = BMS_CACHE->DATA2;
    }

    return Data;
}

//------------------------------------------------------------------------------
/*Function Name     :   BmsCacheInit
**Description       :   BmsCache读出参数
**Input Param       :   int i:选择读取第几个寄存器
**Output Param      :
**Return value      :   return Data:返回读取的32bit数据
*/
//------------------------------------------------------------------------------
void BmsCacheInit()
{
    BmsCacheWr(0, 0x04030201);
    BmsCacheWr(1, 0x08070605);
    BmsCacheWr(2, 0x0c0b0a09);
}

//------------------------------------------------------------------------------
/*Function Name     :   BmsCacheTableTest
**Description       :   BmsCache写操作
**Input Param       :   
**Output Param      :
**Return value      :   
*/
//------------------------------------------------------------------------------

int BmsCacheTableTest()
{
    if((BmsCacheRd(0) == 0x04030201) && (BmsCacheRd(1) == 0x08070605) && (BmsCacheRd(2) == 0x0c0b0a09))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
