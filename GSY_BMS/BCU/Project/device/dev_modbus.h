#ifndef __MODBUS_H_
#define __MODBUS_H_

#include "stdbool.h"
#include "stdint.h"

#define MODBUS_DATA_NUM_MIN		5
#define MODBUS_TCP_FH		6


typedef enum{
	MODBUS_RTU=0,
	MODBUS_TCP,
}MODBUS_MESSAGE_TYPE;

typedef  void (*MODBUS_RXFUN)(const uint8_t *const,uint16_t);
typedef  void (*MODBUS_SLAVE_TXFUN)(uint16_t,uint8_t);

typedef struct {
	uint8_t Func;
	uint16_t RegadStart;
	uint16_t DataNum;
	uint16_t RegisterOffset;		//寄存器偏移寻址宽度，相对于RegadStart，不需要此功能时写0
	uint16_t OffsetCount;		//寄存器可偏移次数，RegisterOffset为0时无效
	const uint16_t *const TxBuff;
    void (*ReadDataProcess)(uint16_t);  //(偏移次数)
}MODBUS_SLAVE_READ_CFG;
typedef struct {
	uint8_t Func;
	uint16_t RegadStart;
	uint16_t DataNum;
	uint16_t RegisterOffset;		//寄存器偏移寻址宽度，相对于RegadStart，不需要此功能时写0
	uint16_t OffsetCount;		//寄存器可偏移次数，RegisterOffset为0时无效
	uint16_t *const RxBuff;
    void (*WriteDataProcess)(uint16_t,uint16_t,uint8_t);    //(偏移次数，寄存器地址，寄存器数量)
}MODBUS_SLAVE_WRITE_CFG;


typedef struct {
	uint8_t DeviceAddr;
    const MODBUS_SLAVE_READ_CFG *ReadCfgInfo;
	uint8_t ReadMessageNum;
	const MODBUS_SLAVE_WRITE_CFG *WriteCfgInfo;
	uint8_t WriteMessageNum;
}MODBUS_SLAVE_CFG;


uint16_t ModbusSlaveProtProce(MODBUS_MESSAGE_TYPE ModbusType,const MODBUS_SLAVE_CFG *const ModbusSlaveInfo,uint8_t *const RxBuff,uint16_t len,uint8_t *const TxBuff);

	
#endif
