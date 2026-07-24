#include "fc41d.h"

DevFc41d_t *gFc41d = NULL;

DevFc41d_t* Fc41dGetPtr()
{
	return gFc41d;
}

void Fc41dReset()
{
	DevFc41dReset(gFc41d, 1);
}

void Fc41dProcess()
{
	gFc41d->process(gFc41d);
}

void Fc41dDeinit()
{
	gFc41d->del(gFc41d);
	gFc41d = NULL;
}

extern int32_t test_md5_upgrade();
extern int32_t _DevFc41dAliyunGetFlashDataMd5(void *fc41d, uint8_t md5[16], uint32_t len);

int32_t Fc41dInit()
{
	if (DevFc41dInit(&gFc41d) != 0) 
	{
		return -1;
	}

	test_md5_upgrade();
	return 0;
}

int32_t Fc41dGetStat()
{
	return (int32_t)DevFc41dGetStat(gFc41d);
}

int32_t Fc41dCheckUpgrade()
{
	return DevFc41dCheckUpgrade(gFc41d);
}

int32_t Fc41dCheckNetSetting()
{
	return DevFc41dCheckNetSetting(gFc41d);
}

int32_t Fc41dProcessing()
{
	return DevFc41dProcessing(gFc41d);
}

int32_t Fc41dSetMqttServerInfo(int8_t type, const char *ip, const char *port)
{
	return DevFc41dSetMqttServerInfo(gFc41d, type, ip, port);
}

int8_t Fc41dWifiStatGet()
{
	return DevFc41dGetWifiStat(gFc41d);
}

int8_t Fc41dMqttStatGet()
{
	return DevFc41dGetMqttStat(gFc41d);
}

int32_t Fc41dMqttPubMeta(int32_t pub_idx, const uint8_t *data, int32_t len)
{
	return gFc41d->mqtt_pub_meta(gFc41d, pub_idx, data, len);
}

int32_t Fc41dMqttPub(int32_t pub_idx, int32_t cmd, uint16_t err, 
					 const uint8_t *data, int32_t len)
{
	return gFc41d->mqtt_pub(gFc41d, pub_idx, cmd, err, data, len);
}

int32_t Fc41dGetBleMac(uint8_t *mac, int32_t len)
{
	return DevFc41dGetBleMac(gFc41d, mac, len);
}

int32_t Fc41dGetWifiSsid(uint8_t *ssid, int32_t len)
{
	return DevFc41dGetWifiSsid(gFc41d, ssid, len);
}

int32_t Fc41dGetWifiPwd(uint8_t *pwd, int32_t len)
{
	return DevFc41dGetWifiPwd(gFc41d, pwd, len);
}

int32_t Fc41dGetWifiRssi()
{
	return DevFc41dGetWifiRssi(gFc41d);
}

int32_t Fc41dWifiSet(const char *ssid, const char *pwd)
{
	return DevFc41dWifiSet(gFc41d, ssid, pwd);
}

int32_t Fc41dGetMqttServerIp(uint8_t *ip, int32_t len)
{
	return DevFc41dGetMqttServerIp(gFc41d, ip, len);
}

int32_t Fc41dGetMqttServerPort(uint8_t *port, int32_t len)
{
	return DevFc41dGetMqttServerPort(gFc41d, port, len);
}

int8_t Fc41dGetMqttServerType()
{
	return DevFc41dGetMqttServerType(gFc41d);
}

int32_t Fc41dMqttSet(int8_t type, const char *ip, const char *port)
{
	return DevFc41dMqttSet(gFc41d, type, ip, port);
}

int32_t Fc41dUpdateWifiInfo()
{
	return DevFc41dUpdateWifiInfo(gFc41d);
}

int32_t Fc41dCheckMqttSubTopicStatus()
{
	return DevFc41dCheckMqttSubTopicStatus(gFc41d);
}

int32_t Fc41dIsBusy()
{
	return DevFc41dIsBusy(gFc41d);
}

int32_t Fc41dGetTaskSize()
{
	return DevFc41dGetTaskSize(gFc41d);
}

char* Fc41dGetFc41dVer()
{
	return DevFc41dGetFc41dVer(gFc41d);
}

int32_t GetBleConnStat()
{
	return DevFc41dGetBleConnStat(gFc41d);
}

int32_t test_md5_upgrade()
{

	uint8_t md5[16] = {0};
	_DevFc41dAliyunGetFlashDataMd5(gFc41d, md5, 0x1d49c);
	return 0;
}

