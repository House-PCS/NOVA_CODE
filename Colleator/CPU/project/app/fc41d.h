#ifndef _FC41d_H
#define _FC41d_H

#include <stdint.h>
#include "dev_fc41d.h"

DevFc41d_t* Fc41dGetPtr();
void Fc41dReset();
void Fc41dProcess();
void Fc41dDeinit();
int32_t Fc41dInit();

int32_t Fc41dGetStat();

int32_t Fc41dCheckUpgrade();

int32_t Fc41dCheckNetSetting();

int32_t Fc41dProcessing();

int32_t Fc41dSetMqttServerInfo(int8_t type, const char *ip, const char *port);

int8_t Fc41dWifiStatGet();

int8_t Fc41dMqttStatGet();

int32_t Fc41dMqttPubMeta(int32_t pub_idx, const uint8_t *data, int32_t len);

int32_t Fc41dMqttPub(int32_t pub_idx, int32_t cmd, uint16_t err, 
					 const uint8_t *data, int32_t len);

int32_t Fc41dGetBleMac(uint8_t *mac, int32_t len);
int32_t Fc41dGetWifiSsid(uint8_t *ssid, int32_t len);
int32_t Fc41dGetWifiPwd(uint8_t *pwd, int32_t len);
int32_t Fc41dGetWifiRssi();
int32_t Fc41dWifiSet(const char *ssid, const char *pwd);

int32_t Fc41dGetMqttServerIp(uint8_t *ip, int32_t len);
int32_t Fc41dGetMqttServerPort(uint8_t *port, int32_t len);
int8_t Fc41dGetMqttServerType();
int32_t Fc41dMqttSet(int8_t type, const char *ip, const char *port);

int32_t Fc41dUpdateWifiInfo();

int32_t Fc41dCheckMqttSubTopicStatus();

int32_t Fc41dIsBusy();

int32_t Fc41dGetTaskSize();

char* Fc41dGetFc41dVer();

int32_t GetBleConnStat();

#endif /*_FC41d_H*/
