
#ifndef __DEV_BOARD_ID_H_
#define __DEV_BOARD_ID_H_

#include "bsp.h"
#include "queue.h"
#include "systick.h"


typedef enum
{
    EN_COLLECTOR_SUB_BOARD_TYPE_WIFI,
    EN_COLLECTOR_SUB_BOARD_TYPE_4G
}EN_COLLECTOR_SUB_BOARD_TYPE;


void DevBoardIDInit();
EN_COLLECTOR_SUB_BOARD_TYPE SubBoardTypeGet();

#endif













