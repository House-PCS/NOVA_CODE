
#include "dev_dbg.h"

void DevDbgInit()
{
    DbgUartBspInit();
}



int fputc(int ch, FILE *f)
{
    DbgUartHandle.tx_size = 1;
    DbgUartHandle.pTxBuf = (uint8_t *)&ch;
    UartNormalSend(&DbgUartHandle);
    return(ch);
}










