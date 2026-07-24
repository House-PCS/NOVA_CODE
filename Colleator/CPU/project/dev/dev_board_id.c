
#include "dev_board_id.h"



void DevBoardIDInit()
{
    BoardIDBspInit();
}


EN_COLLECTOR_SUB_BOARD_TYPE SubBoardTypeGet()
{
    if(GpioBitGet(&BoardIDGpioHandle) == SET)
    {
        return EN_COLLECTOR_SUB_BOARD_TYPE_WIFI;
    }
    else
    {
        return EN_COLLECTOR_SUB_BOARD_TYPE_4G;
    }
}







