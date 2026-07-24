#ifndef DEV_FC41D_H
#define DEV_FC41D_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "bsp.h"

#define LED1		LED_SRC_SYS
#define LED2		LED_SRC_CONN
#define LED3		LED_SRC_PCS

#define FC41D_BLE_CONN			1
#define FC41D_BLE_DISCONN		0
#define FC41D_BLE_MTU			480
#define FC41D_MQTT_MTU			960

/**
 * @brief FC41D USART 串口状态机
 */
typedef enum
{
	FC41D_COMM_STATUS_INIT = 0,
	FC41D_COMM_STATUS_IDLE,
    FC41D_COMM_STATUS_WRITE,
	FC41D_COMM_STATUS_READ,
} EN_FC41D_COMM_STATUS;

/**
 * @brief FC41D 业务状态机
 */
typedef enum 
{
	FC41D_PROCESS_STATUS_INIT = 0,
	FC41D_PROCESS_STATUS_VER,				// 1
	FC41D_PROCESS_STATUS_DISABLE_WEB_CFG,	// 2
	FC41D_PROCESS_STATUS_URC,				// 3
	FC41D_PROCESS_STATUS_BLE,				// 4
	FC41D_PROCESS_STATUS_WIFI,				// 5
	FC41D_PROCESS_STATUS_MQTT,				// 6
	FC41D_PROCESS_STATUS_HTTP,				// 7
	FC41D_PROCESS_STATUS_SOCKET,			// 8
	FC41D_PROCESS_STATUS_NOVA_CMD,			// 9
	FC41D_PROCESS_STATUS_RELAY_MULTI_CMD,	// 10
} EN_FC41D_PROCESS_STATUS;

/**
 * @brief FC41D URC 上报状态
 */
typedef enum 
{
	FC41D_PROCESS_URC_STATUS_OFF = 0,
	FC41D_PROCESS_URC_STATUS_ON,
} EN_FC41D_PROCESS_URC_STATUS;

/**
 * @brief FC41D BLE状态机
 */
typedef enum 
{
	FC41D_PROCESS_BLE_STATUS_INIT = 0,	// 模块为外围设备进行 BLE 初始化
	FC41D_PROCESS_BLE_STATUS_ADDR,		// 1. 查询获取BLE设备地址
	FC41D_PROCESS_BLE_STATUS_STAT,		// 2. 查询BLE状态
	FC41D_PROCESS_BLE_STATUS_ADVDATA,	// 3. 配置ADVDATA广播数据，0x09定义蓝牙名称, 0xFF厂商数据
	FC41D_PROCESS_BLE_STATUS_GATTSSRV,	// 4. 创建BLE服务
	FC41D_PROCESS_BLE_STATUS_GATTSCHAR, // 5. 创建BLE特征值
	FC41D_PROCESS_BLE_STATUS_GATTSCHAR1, //5. 创建BLE特征值ff3
	FC41D_PROCESS_BLE_STATUS_ADVPARAM,	// 6. 设置BLE广播参数
	FC41D_PROCESS_BLE_STATUS_ADVSTART,	// 7. 开启广播
	FC41D_PROCESS_BLE_STATUS_ADVSTOP,	// 8. 关闭广播

	FC41D_PROCESS_BLE_STATUS_SEND_LEN,	// 9. 设置待发送数据长度
	FC41D_PROCESS_BLE_STATUS_SEND_DATA, // 10. 发送数据
	FC41D_PROCESS_BLE_STATUS_MTU,		// 11. 配置MTU
} EN_FC41D_PROCESS_BLE_STATUS;

/**
 * @brief FC41D WIFI状态机
 */
typedef enum 
{
	FC41D_PROCESS_WIFI_STATUS_INIT = 0,			// 模块为外围设备进行 WIFI 初始化
	FC41D_PROCESS_WIFI_STATION_WSCAN,			// 搜索WSCAN
	FC41D_PROCESS_WIFI_STATION_CONN,			// 连接AP
	FC41D_PROCESS_WIFI_STATION_CONN_STAT,		// 查询WIFI连接状态
	FC41D_PROCESS_WIFI_STATION_STAT,			// 查询连接的AP信息
	FC41D_PROCESS_WIFI_STATION_STOP,			// 禁用STA
	FC41D_PROCESS_WIFI_STATION_STOP_DEF,		// 禁用STA
	FC41D_PROCESS_WIFI_STATION_STAT_UPDATE,		// 更新连接的AP信息
	FC41D_PROCESS_WIFI_STATION_GET_IP,			// DEBUG查询FC41D所有信息
	FC41D_PROCESS_WIFI_STATION_GET_WIFI_INFO,	// DEBUG查询WIFI信息
	FC41D_PROCESS_WIFI_STATION_GET_MQTT_INFO,	// DEBUG查询MQTT信息
} EN_FC41D_PROCESS_WIFI_STATUS;

/**
 * @brief FC41D WIFI模式
 */
typedef enum 
{
	DEV_FC41D_WIFI_TYPE_NONE = 0,
	DEV_FC41D_WIFI_TYPE_AP,
	DEV_FC41D_WIFI_TYPE_STATION,
} EN_DEV_FC41D_WIFI_TYPE;

/**
 * @brief FC41D WIFI station ErrCode连接状态
 */
typedef enum 
{
	FC41D_WIFI_STATION_ERR_NONE = 0,
	FC41D_WIFI_STATION_ERR_CONN,
	FC41D_WIFI_STATION_ERR_GOT_IP,
	FC41D_WIFI_STATION_ERR_SOCK,
	FC41D_WIFI_STATION_ERR_SAME_WIFI_CONN,
} EN_FC41D_WIFI_STATION_ERR;

/**
 * @brief FC41D TCP Server状态机
 */
typedef enum 
{
	FC41D_PROCESS_SOCKET_STATUS_CLOSE = 0,				// 0. tcp/udp 关闭
	FC41D_PROCESS_SOCKET_STATUS_OPEN,					// 1. tcp/udp 关闭
	FC41D_PROCESS_SOCKET_STATUS_CFG_ACCEPT,				// 2. 配置accept类型
	FC41D_PROCESS_SOCKET_STATUS_CFG_PORT_RELTYPE,		// 3. 配置端口释放类型
	FC41D_PROCESS_SOCKET_STATUS_CFG_DATA_TYPE,			// 4. 配置发送数据类型类型
	FC41D_PROCESS_SOCKET_STATUS_SEND_LEN,				// 5. 设置待发送数据长度
	FC41D_PROCESS_SOCKET_STATUS_SEND_DATA,				// 6. 发送数据
	FC41D_PROCESS_SOCKET_STATUS_SEND_NORMAL_DATA,		// 7. 发送数据
} EN_FC41D_PROCESS_TCP_S_STATUS;

/**
 * @brief FC41D MQTT 状态机
 */
typedef enum 
{
	FC41D_PROCESS_MQTT_CLOSE = 0,
	FC41D_PROCESS_MQTT_STAT,					// 1
	FC41D_PROCESS_MQTT_SET_CFG_VER,				// 2
	FC41D_PROCESS_MQTT_SET_CFG_DATATYPE,		// 3
	FC41D_PROCESS_MQTT_SET_CFG_SSL,				// 4
	FC41D_PROCESS_MQTT_SET_CFG_KEEPALIVE,		// 5
	FC41D_PROCESS_MQTT_SET_CFG_TIMEOUT,			// 6
	FC41D_PROCESS_MQTT_SET_CFG_SESSION,			// 7
	FC41D_PROCESS_MQTT_SET_CFG_WILL,			// 8
	FC41D_PROCESS_MQTT_OPEN,					// 9
	FC41D_PROCESS_MQTT_CONN,					// 10
	FC41D_PROCESS_MQTT_DISCONN,					// 11
	FC41D_PROCESS_MQTT_INIT_TOPIC,				// 12
	FC41D_PROCESS_MQTT_PUB_SEND_LEN,			// 13 设置待发送数据长度
	FC41D_PROCESS_MQTT_PUB_SEND_DATA,			// 14 发送数据
	FC41D_PROCESS_MQTT_CFG_COMPLETE,			// 15 仅配置MQTT,不做response
	FC41D_PROCESS_MQTT_PUB_NORMAL_DATA,			// 16 发送数据
} EN_FC41D_MQTT_STATUS;

/**
 * @brief FC41D MQTT ERRORCODE
 */
typedef enum 
{
	FC41D_MQTT_ERR_NONE = 0,

	FC41D_MQTT_ERR_UNKNOWN_OPEN,	//1 AT open cmd failed
	FC41D_MQTT_ERR_UNKNOWN_CONN,	//2 AT conn cmd failed
	FC41D_MQTT_ERR_AT_OPEN_TIMEOUT,	//3 AT conn cmd failed
	FC41D_MQTT_ERR_AT_CONN_TIMEOUT,	//4 AT conn cmd failed
	FC41D_MQTT_ERR_CLIENT_ID,		//5 client id 未知
	FC41D_MQTT_ERR_SOCK,			//6 Socket连接失败
	FC41D_MQTT_ERR_PARAM,			//7 参数错误
	FC41D_MQTT_ERR_OCCUPY,			//8 标识符被占用
	FC41D_MQTT_ERR_MEM,				//9 内存不足
	FC41D_MQTT_ERR_DNS,				//10 域名解析失败
	FC41D_MQTT_ERR_SOCK_DUMP,		//11 Socket异常关闭
	FC41D_MQTT_ERR_CONNACK,			//12 Conn未得到ACK回应
	FC41D_MQTT_ERR_CONN,			//13 Conn失败
	FC41D_MQTT_ERR_PROTO_VER,		//14 拒绝连接：协议版本不兼容
	FC41D_MQTT_ERR_ID,				//15 拒绝连接：标识符异常
	FC41D_MQTT_ERR_SRV,				//16 服务不可用
	FC41D_MQTT_ERR_AUTH,			//17 用户名，密码错误
	FC41D_MQTT_ERR_UNAUTH,			//18 未授权
} EN_FC41D_MQTT_ERRCODE;

/**
 * @brief FC41D HTTP(s) 请求状态
 */
typedef enum 
{
	FC41D_PROCESS_HTTP_STATUS_OFF = 0,
	FC41D_PROCESS_HTTP_STATUS_ON,
} EN_FC41D_PROCESS_HTTP_STATUS;

/**
 * @brief FC41D HTTP 状态机
 */
typedef enum 
{
	FC41D_PROCESS_HTTP_SET_USE_URC = 0,
	FC41D_PROCESS_HTTP_SET_URC_SKIP_HEADER,		// 1
	FC41D_PROCESS_HTTP_SET_URL,					// 2
	FC41D_PROCESS_HTTP_SET_H_USER_AGENT,		// 3
	FC41D_PROCESS_HTTP_SET_H_DATE,				// 4 
	FC41D_PROCESS_HTTP_SET_H_AUTH,				// 5
	FC41D_PROCESS_HTTP_SET_H_RANGE,				// 6
	FC41D_PROCESS_HTTP_TEST,					// 7
	FC41D_PROCESS_HTTP_GET,						// 8
	FC41D_PROCESS_HTTP_DEV_UPGRADE_START,		// 9 
	FC41D_PROCESS_HTTP_DEV_UPGRADE_DOWN,		// 10
	FC41D_PROCESS_HTTP_DEV_UPGRADE_FAIL,		// 11
	FC41D_PROCESS_HTTP_DEV_UPGRADE_RETRY,		// 12
	FC41D_PROCESS_HTTP_OTA_FC41D,				// 13
} EN_FC41D_HTTP_STATUS;

/**
 * @brief FC41D 指令转发状态机
 */
typedef enum 
{
	FC41D_PROCESS_NOVA_CMD_NONE = 0,	
	FC41D_PROCESS_NOVA_CMD_INIT,										// 1
	FC41D_PROCESS_RELAY_MULTI_CMD_KSTAR,								// 2 
	FC41D_PROCESS_RELAY_MULTI_CMD_MEGAREVO,								// 3 
	FC41D_PROCESS_RELAY_MULTI_CMD_NOVA_GET_MONITOR_HARDFAULT,			// 4
} EN_FC41D_PROCESS_RELAY_MULTI_CMD_STATUS;


/**
 * @brief FC41D TCP/UDP 工作模式
 */
typedef enum 
{
	DEV_FC41D_SOCKET_NONE = 0,
	DEV_FC41D_SOCKET_SERVER_TCP,	// 1 
	DEV_FC41D_SOCKET_CLIENT_TCP,	// 2
	DEV_FC41D_SOCKET_SERVER_UDP,	// 3
	DEV_FC41D_SOCKET_CLIENT_UDP,	// 4
	DEV_FC41D_SOCKET_CLOSING,		// 5
} EN_DEV_FC41D_SOCKET_TYPE;


struct _dev_fc41d_ctx_t; // fc41d 模组上下文
typedef struct _dev_fc41d_t DevFc41d_t; // fc41d 模组对外函数指针列表

// 外部USART串口发送函数
typedef int32_t (*fc41d_fn_write)(DevFc41d_t *p, const uint8_t *data, int32_t len);

struct _dev_fc41d_t 
{
	int32_t (*del)(DevFc41d_t*);
	int32_t (*read)(DevFc41d_t*, const uint8_t*, int32_t);
	int32_t (*write)(DevFc41d_t*, const uint8_t*, int32_t);

	int32_t (*mqtt_pub_resp)(DevFc41d_t*, void *, int32_t, int32_t, uint16_t, 
							 const uint8_t*, int32_t);
	int32_t (*mqtt_pub_meta)(DevFc41d_t*, int32_t, const uint8_t*, int32_t);
	int32_t (*mqtt_pub)(DevFc41d_t*, int32_t, int32_t, uint16_t, const uint8_t*, int32_t);
	int32_t (*http_req)(DevFc41d_t*, void *, int32_t, uint16_t, const uint8_t*, int32_t);

	int32_t (*process)(DevFc41d_t*);

	int32_t (*task_head_del)(DevFc41d_t*);
	int32_t (*task_push_inhead)(DevFc41d_t*, int32_t, int32_t, int8_t, int32_t);
	int32_t (*task_push_append)(DevFc41d_t*, int32_t, int32_t, int8_t, int32_t);

	struct _dev_fc41d_ctx_t *priv_data;
};

/**
 * @brief 设置MQTT服务器地址、端口
 *
 * @param fc41d
 * @param type 0:ip, 1:domain
 * @param ip
 * @param 
 *
 * @return 0 sucess, otherwise failed
 */
int32_t DevFc41dSetMqttServerInfo(DevFc41d_t *fc41d, int8_t type, const char *ip, 
								  const char *port);

/**
 * @brief device fc41d wifi 连接状态
 *
 * @param fc41d
 *
 * @return 1 连接， 0未连接
 */
int8_t DevFc41dGetWifiStat(DevFc41d_t *fc41d);

/**
 * @brief device fc41d mqtt 连接状态
 *
 * @return 1 已联网, 0 掉线
 */
int8_t DevFc41dGetMqttStat(DevFc41d_t *fc41d);

int32_t DevFc41dCheckUpgrade(DevFc41d_t *fc41d);

int32_t DevFc41dCheckNetSetting(DevFc41d_t *fc41d);

/**
 * @brief 判断当前设备是否正在处理请求
 *
 * @param 
 *
 * @return 0 工作状态, otherwise IDLE状态
 */
int32_t DevFc41dProcessing(DevFc41d_t *fc41d);

int32_t DevFc41dReset(DevFc41d_t *fc41d, int32_t clean_data_flag);

int32_t DevFc41dGetBleName(DevFc41d_t *fc41d, uint8_t *ble, int32_t len);

int32_t DevFc41dGetBleMac(DevFc41d_t *fc41d, uint8_t *mac, int32_t len);
int32_t DevFc41dGetWifiSsid(DevFc41d_t *fc41d, uint8_t *ssid, int32_t len);
int32_t DevFc41dGetWifiPwd(DevFc41d_t *fc41d, uint8_t *pwd, int32_t len);
int32_t DevFc41dGetWifiRssi(DevFc41d_t *fc41d);
int32_t DevFc41dWifiSet(DevFc41d_t *fc41d, const char *ssid, const char *pwd);

int32_t DevFc41dGetMqttServerIp(DevFc41d_t *fc41d, uint8_t *ip, int32_t len);
int32_t DevFc41dGetMqttServerPort(DevFc41d_t *fc41d, uint8_t *port, int32_t len);
int8_t DevFc41dGetMqttServerType(DevFc41d_t *fc41d);
int32_t DevFc41dMqttSet(DevFc41d_t *fc41d, int8_t type, const char *ip, const char *port);
int32_t DevFc41dUpdateWifiInfo(DevFc41d_t *fc41d);
int32_t DevFc41dCheckMqttSubTopicStatus(DevFc41d_t *fc41d);

int32_t DevFc41dIsBusy(DevFc41d_t *fc41d);

int32_t DevFc41dGetTaskSize(DevFc41d_t *fc41d);

int8_t DevFc41dGetStat(DevFc41d_t *fc41d);

char* DevFc41dGetFc41dVer(DevFc41d_t *fc41d);

int32_t DevFc41dGetBleConnStat(DevFc41d_t *fc41d);

/**
 * @brief device fc41d 外设初始化
 *
 * @param fc41d
 *
 * @return 0 is success, otherwise failed
 */
int32_t DevFc41dInit(DevFc41d_t **fc41d);

#endif /* DEV_FC41D_H */
