
#ifndef __REQUEST_BODY_PROCESS_H_
#define __REQUEST_BODY_PROCESS_H_

#include "common.h"
#include "stdbool.h"

/*READ*/
#define UPPER_CMD_EVENT_READ_WIFI_AP_INFO                           0x00FF0000
#define UPPER_CMD_EVENT_READ_CURRENT_CONNECTED_AP                   0x00FF0001
#define UPPER_CMD_EVENT_READ_CURRENT_DEVICE_INFO                    0x00FF0002
#define UPPER_CMD_EVENT_READ_CALIBRATION_TIME                       0x00FF0003
#define UPPER_CMD_EVENT_READ_DEVICE_NAME                            0x00FF0004
#define UPPER_CMD_EVENT_READ_DEVICE_PARA                            0x00FF0005
#define UPPER_CMD_EVENT_READ_DEVICE_INTERFACE                       0x00FF0006
#define UPPER_CMD_EVENT_READ_DEVICE_CELL_GROUP                      0x00FF0007
#define UPPER_CMD_EVENT_READ_DEVICE_CTL_INPUT_POWER                 0x00FF0008
#define UPPER_CMD_EVENT_READ_DEVICE_CTL_SCREEN_REST                 0x00FF0009
#define UPPER_CMD_EVENT_READ_DEVICE_CTL_INTERFACE_SWITCH            0x00FF000A
#define UPPER_CMD_EVENT_READ_DEVICE_CTL_CHARGE_MODE                 0x00FF000E
#define UPPER_CMD_EVENT_READ_DEVICE_CTL_CHARGE_MAX_CAPACITY         0x00FF000F

/*WRITE*/
#define UPPER_CMD_EVENT_WRITE_TO_CONNECT_AP                         0x80FF0001
#define UPPER_CMD_EVENT_WRITE_DEVICE_NAME                           0x80FF0004
#define UPPER_CMD_EVENT_WRITE_CALIBRATION_TIME                      0x80FF0003
#define UPPER_CMD_EVENT_WRITE_DEVICE_CTL_INPUT_POWER                0x80FF0008
#define UPPER_CMD_EVENT_WRITE_DEVICE_CTL_SCREEN_REST                0x80FF0009
#define UPPER_CMD_EVENT_WRITE_DEVICE_CTL_INTERFACE_SWITCH           0x80FF000A
#define UPPER_CMD_EVENT_WRITE_DEVICE_CTL_CHARGE_MODE                0x80FF000E
#define UPPER_CMD_EVENT_WRITE_DEVICE_CTL_CHARGE_MAX_CAPACITY        0x80FF000F









void CmdErrorLen(uint8_t *p, uint32_t cmd, uint16_t len);

uint16_t ScanAPPackaging(uint8_t *pDst,  uint8_t *pSrc, uint16_t src_size);
uint16_t ConnectedAPPackaging(uint8_t *pDst);
uint16_t DeviceParaPackaging(uint8_t *pDst, uint8_t ack_or_report);
uint16_t DeviceInterfaceInfoPackaging(uint8_t *pDst, uint8_t ack_or_report);
uint16_t DeviceCellGroupStatusPackaging(uint8_t *pDst, uint8_t ack_or_report, uint8_t cell_group_index);
uint16_t DeviceCtlReadInputPowerPackaging(uint8_t *pDst);
uint16_t DeviceCtlReadSrceenRestPackaging(uint8_t *pDst);
uint16_t DeviceCtlReadOutputInterfaceSwitchPackaging(uint8_t *pDst);
uint16_t DeviceCtlReadChargeModePackaging(uint8_t *pDst);
uint16_t DeviceCtlReadChargeMaxCapacityPackaging(uint8_t *pDst);
uint16_t DeviceReadNamePackaging(uint8_t *pDst);
uint16_t DeviceInfoPackaging(uint8_t *pDst, uint8_t ack_or_report);



bool DeviceCtlWriteInputPowerDataProcess(uint8_t *pSrc, uint16_t rev_len);
bool DeviceCtlWriteSrceenRestDataProcess(uint8_t *pSrc, uint16_t rev_len);
bool DeviceCtlWriteOutputInterfaceSwitchDataProcess(uint8_t *pSrc, uint16_t rev_len);
bool DeviceCtlWriteChargeModeDataProcess(uint8_t *pSrc, uint16_t rev_len);
bool DeviceCtlWriteChargeMaxCapacityDataProcess(uint8_t *pSrc, uint16_t rev_len);
bool DeviceWriteNameDataProcess(uint8_t *pSrc, uint16_t rev_len);

void APConnectStatusAck(uint8_t *pDst, uint16_t err);
#endif










