#ifndef __ALARM_H_
#define __ALARM_H_

#include <stdint.h>
#include <stdbool.h>


typedef struct {
	uint8_t AlarmCode;		//故障位码，从1开始
	bool (*AlarmParseFunc)(void);		//故障条件判断函数
	uint8_t lastAlarmState;
	uint16_t EnabledTime;			//故障产生延时时间,单位 ms
	uint16_t DisenabledTime;		//故障消除延时时间
	uint32_t ValidTimer;		//故障信号有效计时开始
	uint32_t InvalidTimer;		//故障信号无效计时开始
}AlarmPartsType;

typedef struct {
	uint8_t AlarmNum;
	AlarmPartsType *const AlarmParts;
	uint8_t * alarmCode;		//实时状态故障码       
}AlarmPacketType;
typedef struct {
	uint8_t AlarmTypeNum;		
	AlarmPacketType *AlarmPacket;
}AlarmInfoType;


extern AlarmInfoType AlarmInfo;
extern const AlarmInfoType* const IR_AlarmInfo;

void BmsAlarmMain(void );

#endif
