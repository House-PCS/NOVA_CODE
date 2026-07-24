#ifndef _DEV_FC41D_COMMON_H
#define _DEV_FC41D_COMMON_H

#include "config.h"
#include "dev_fc41d.h"
#include "xlist.h"
#include "ringbuffer.h"
#include "proto_nova_uniform.h"
#include "collector.h"
#include "systick.h"
#include "bsp.h"
#include "dev_led.h"
#include "sys_info.h"
#include "dev_fc41d_aliyun.h"
#include "dev_fc41d_http_mqtt_info.h"

#define WIRELESS_DATA_RX_SIZE           1024*7
#define IRQ_READ_DATA_RX_SIZE           1024*8
#define FC41D_CTX_BASE_SIZE				1024*15 // 临时buf 4K, 任务队列400 b,  upgrade 2K, write 4K, busy 1K
#define FC41D_CTX_SIZE					(FC41D_CTX_BASE_SIZE+IRQ_READ_DATA_RX_SIZE)

#define AT_CMD_TIMEOUT				10 * 1000
#define AT_CMD_W_WIFI_TIMEOUT		410

#define FC41D_TASK_MAX_SIZE			16

#define WIFI_CONN_STAT_DISCONN		0
#define WIFI_CONN_STAT_CONN			1

#ifndef FC41D_WRITE_TIMEOU	
#define FC41D_WRITE_TIMEOU			1*1000 // 毫秒
#endif /*FC41D_WRITE_TIMEOU*/

#ifndef FC41D_WIFI_AUTO_CONN
#define FC41D_WIFI_AUTO_CONN		1
#endif /*FC41D_WIFI_AUTO_CONN*/

#ifndef FC41D_WIFI_AUTO_CONN_TIMEOU	
#define FC41D_WIFI_AUTO_CONN_TIMEOU 5*1000 // 毫秒
#endif /*FC41D_WIFI_AUTO_CONN_TIMEOU*/

#ifndef FC41D_MQTT_SUB_TEST_SIZE
#define FC41D_MQTT_SUB_TEST_SIZE	6
#endif /*FC41D_MQTT_SUB_TEST_SIZE*/

#ifndef FC41D_MQTT_AUTO_CONN
#define FC41D_MQTT_AUTO_CONN		1
#endif /*FC41D_MQTT_AUTO_CONN*/

#ifndef FC41D_MQTT_AUTO_CONN_TIMEOU	
#define FC41D_MQTT_AUTO_CONN_TIMEOU 10*1000 // 毫秒
#endif /*FC41D_MQTT_AUTO_CONN_TIMEOU*/

#define NOVA_MSG_BUSY		-100


#define NOVA_UPGRADE_COLLECTOR_PKG_MAX_SIZE (IRQ_READ_DATA_RX_SIZE - 2048)

#define NOVA_UPGRADE_COLLECTOR_TIMEOUT		3*60*1000 // 3min
#define NOVA_UPGRADE_PCS_TIMEOUT			15*60*1000 // 3min

#ifndef NOVA_UPGRADE_COLLECTOR_RETRY_MAX	
#define NOVA_UPGRADE_COLLECTOR_RETRY_MAX	5 // 升级尝试次数
#endif /*NOVA_UPGRADE_COLLECTOR_RETRY_MAX*/

#define NOVA_UPGRADE_COLLECTOR_FLASH_BASE_ADDR		0x001000
#define NOVA_UPGRADE_COLLECTOR_FLASH_CTX_ADDR		0x3E8000  // 采集器固件ctx

// AT status
#define AT_READY					"ready\r\n"
#define AT_OK						"OK\r\n"
#define AT_ERR						"ERROR\r\n"

// AT common
#define AT_FC41D_INIT				"AT\r\n"
#define AT_FC41D_RESET				"AT+QRST\r\n"
#define AT_FC41D_VER				"AT+QVERSION\r\n"
#define AT_FC41D_DISABLE_WEB_CFG	"AT+QWEBCFG=0\r\n"
#define AT_FC41D_URC_OFF			"AT+QURCCFG=0\r\n"
#define AT_FC41D_URC_ON				"AT+QURCCFG=1\r\n"

#define UPGRADE_DATA_STRUCT_CHECK(data, size) \
	(((buf[0] == 0x5A && buf[1] == 0xA5 && buf[2] == 0x3C && buf[3] == 0xC3) \
	  && (buf[256] == 0x0B && buf[257] == 0x0B && buf[258] == 0x0B && buf[259] == 0x00) \
	  && size == *((uint32_t*)&buf[4])))


typedef enum 
{
	EN_FILE_PKG_TYPE_NOVA_UNKNOWN = 0x0000,
	EN_FILE_PKG_TYPE_NOVA_KSTAR_5KW = 0xC200,
	EN_FILE_PKG_TYPE_NOVA_KSTAR_10KW = 0xC201,
	EN_FILE_PKG_TYPE_NOVA_COLLECTOR = 0xC100,
	EN_FILE_PKG_TYPE_NOVA_PCS_5KW = 0xC280,
    EN_FILE_PKG_TYPE_NOVA_PCS_16KW = 0xC290,
} EN_DOWNLOAD_FILE_PKG_TYPE;

/**
 * @brief FC41D 任务状态
 */
typedef enum 
{
	DEV_FC41D_TASK_STAT_NONE = 0,
	DEV_FC41D_TASK_STAT_WRITE,			// 1
	DEV_FC41D_TASK_STAT_READ,			// 2
	DEV_FC41D_TASK_STAT_TIMEOUT,		// 3
} EN_DEV_FC41D_TASK_STAT;

/**
 * @brief FC41D Send 类型
 */
typedef enum 
{
	DEV_FC41D_WRITE_NONE = 0,
	DEV_FC41D_WRITE_NOVA,			// 1
	DEV_FC41D_WRITE_WIFI,			// 2
	DEV_FC41D_WRITE_MQTT,			// 3
	DEV_FC41D_WRITE_HTTP,			// 4
	DEV_FC41D_WRITE_RELAY,			// 5
	DEV_FC41D_WRITE_SOCKET,			// 6
} EN_DEV_FC41D_WRITE_TYPE;

////////////////////// UPGRADE ////////////////////
typedef enum 
{
	EN_UPGRADE_TYPE_NONE = 0,
	EN_UPGRADE_TYPE_REMOTE,				//1. 采集器升级开始(ACK)
	EN_UPGRADE_TYPE_LOCAL,				//2. 采集器升级文件下载中
} EN_UPGRADE_TYPE;

////////////////////// REMOTE UPGRADE ////////////////////
typedef enum 
{
	HTTP_SUB_CMD_NONE = 0,
	HTTP_SUB_CMD_UPGRADE_COLLECTOR_1,			// 采集器升级开始(ACK)
	HTTP_SUB_CMD_UPGRADE_COLLECTOR_2,			// 采集器升级文件下载中
	HTTP_SUB_CMD_UPGRADE_COLLECTOR_3,			// 采集器升级文件下载完成(ACK)
	HTTP_SUB_CMD_UPGRADE_COLLECTOR_4,			// 采集器升级中(reboot)
	HTTP_SUB_CMD_UPGRADE_COLLECTOR_5,			// 采集器升级Fail
	HTTP_SUB_CMD_UPGRADE_COLLECTOR_6,			// 下位机升级文件下载完成(ACK)
} EN_HTTP_SUB_CMD;

typedef enum 
{
	COLLECTOR_UPGRADE_ERR_NONE = 0,
	COLLECTOR_UPGRADE_ERR_INNER,				//1. 内部错误
	COLLECTOR_UPGRADE_ERR_BAD_URL,				//2. 连接无法访问
	COLLECTOR_UPGRADE_ERR_FILE_NOT_FOUND,		//3. 找不到文件
	COLLECTOR_UPGRADE_ERR_DOWLOAD_FAILD,		//4. 下载文件出错
	COLLECTOR_UPGRADE_ERR_FILE_PARSE_FAILD,		//5. 文件解析出错
	COLLECTOR_UPGRADE_ERR_FILE_MD5_DOWNLOAD,	//6. 下载文件md5校验出错
	COLLECTOR_UPGRADE_ERR_FILE_MD5_CMP,			//7. 下载完成文件md5比较出错
	COLLECTOR_UPGRADE_ERR_FILE_STRUCT,			//8. 文件结构校验出错
	COLLECTOR_UPGRADE_ERR_TIMEOUT,				//9. 内部超时错误
	COLLECTOR_UPGRADE_ERR_DEBUG,				//10. 内部错误
} EN_COLLECTOR_UPGRADE_ERR;

////////////////////// LOCAL UPGRADE ////////////////////

typedef enum 
{
	COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NONE = 0,
	COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_COLLECTOR = 0x01,	// 采集器
	COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_KSTAR_5K = 0x10, // kstar 5kw
	COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_KSTAR_10K = 0x011, // kstar 10kw
	COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_MEGAREVO_5K = 0x20, // megarevo 5kw
	COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_MEGAREVO_10K = 0x21, // megarevo 10kw
	COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_SURO_8K = 0x30, // suro 8kw
	COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_SURO_10K = 0x31, // suro 10kw
	COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NOVA_5K = 0xA0, // nova 5kw
} EN_COLLECTOR_LOCAL_UPGRADE_FILE_TYPE;

typedef enum 
{
	COLLECTOR_LOCAL_UPGRADE_ERR_NONE = 0,
	COLLECTOR_LOCAL_UPGRADE_ERR_INNER,				//1. 内部错误
	COLLECTOR_LOCAL_UPGRADE_ERR_DEBUG,				//2. 内部调试错误
	COLLECTOR_LOCAL_UPGRADE_ERR_BAD_PROTO_LEN1,		//3. 协议长度超长错误
	COLLECTOR_LOCAL_UPGRADE_ERR_BAD_PROTO_LEN2,		//4. 协议长度超短错误
	COLLECTOR_LOCAL_UPGRADE_ERR_BAD_FILE_TYPE,		//5. 文件类型错误
	COLLECTOR_LOCAL_UPGRADE_ERR_FILE_PARSE_FAILD,	//6. 文件解析出错
	COLLECTOR_LOCAL_UPGRADE_ERR_FILE_SIZE,			//7. 文件大小错误
	COLLECTOR_LOCAL_UPGRADE_ERR_FILE_SIZE2,			//8. 下载完成Flash文件大小和实际文件大小不一致
	COLLECTOR_LOCAL_UPGRADE_ERR_FILE_STRUCT,		//9. 文件构校验出错
	COLLECTOR_LOCAL_UPGRADE_ERR_FILE_CHECK_SUM,		//10. 文件checksum校验出错
	COLLECTOR_LOCAL_UPGRADE_ERR_FILE_LOCAL_MD5,		//11. 文件checksum校验出错

	COLLECTOR_LOCAL_UPGRADE_ERR_UPGRADING,			//12. 正在升级中

	COLLECTOR_LOCAL_UPGRADE_ERR_ALREADY_DOWNLOAD = 0xCDEF,	//0xCDEF 固件已下载
} EN_COLLECTOR_LOCAL_UPGRADE_ERR;

#pragma pack(1)
typedef struct _dev_fc41d_write_ctx_t
{
	int8_t debug;
	int8_t meta_send;		// 是否发送原数据
	int8_t finish;
	int8_t to_self;
	int8_t not_clean;		// 发送完数据是否清空buf(多TLV使用)
	int8_t uniform_type;	// 0:一个TLV, 1:多TLV组合
	int32_t wait_write_idx;	// 待处理的请求起始索引
	int32_t try_times;		// 每包数据尝试次数
	int32_t trim_send_len;	// 发送多包数据时分包已发送的数据长度
	int32_t mqtt_pub_idx;
	int32_t cmd_proity;
	int32_t socket_id;		// UDP/TCP 指定的
	
	uint8_t *uniform_body;
	int32_t next_cmd;
	int32_t upload_type;
	int8_t upgrade_reset;	// 升级是否立即重启

	int32_t split_idx[2];	// 一个复杂任务轮询处理时的flag, idx[0] 标识子包一轮是否能处理完成
							// idx[1] 标识子包一轮处理的进度, 主要用于过滤掉不完整的多余数据

	int32_t cmd;
	int8_t req_type;
	int32_t send_len;

	uint8_t data[0];
} DevFc41dWriteCtx_t;

typedef struct _dev_fc41d_nova_send_t 
{
	int32_t len;
	uint8_t data[0];
} DevFc41dNovaSend_t;

typedef struct _dev_fc41d_task_t 
{
	int8_t use;

	int32_t process_status;			// 主任务类型[BLE, WIFI, MQTT, TCP]
	int32_t sub_status;				// 子任务类型
	int32_t socket_id;				// 只有TCP/UDP有效

	uint32_t timeout;				// 任务超时时间
	int8_t stat;					// 任务当前进度
} DevFc41dTask_t;

typedef struct _dev_fc41d_tasklist_t 
{
	DevFc41dTask_t t;
	xlist_head_t l;
} DevFc41dTaskList_t;

typedef struct _dev_fc41d_ap_t 
{
	uint8_t status;
} DevFc41dAp_t;

typedef struct _dev_fc41d_station_t 
{
	uint8_t status;
	int8_t net_setting_flag;
	uint8_t conn_stat;				// 0 disconn, 1 conn
	uint8_t got_ip_stat;			// 0 disconn, 1 conn
	uint8_t ssid[32];
	uint8_t pwd[64];				// 至少8个字节
	uint8_t bssid[32];				// WIFI mac
	int32_t rssi;					// WIFI 强度
	int32_t channel;				// WIFI 信道
	uint8_t cipher_type[32];		// 加密类型
	uint8_t ip[16];					// ip addr
	uint8_t gate[16];				// gw addr
	uint8_t mask[16];				// mask addr
	uint8_t dns[16];				// dns addr
} DevFc41dStation_t;

typedef union _union_DevFc41dWifi_t 
{
	DevFc41dAp_t ap;
	DevFc41dStation_t station;
} UnionDevFc41dWifi_t;

typedef struct _dev_fc41d_wifi_t 
{
	uint8_t type;
	UnionDevFc41dWifi_t info;
} DevFc41dWifi_t;

typedef struct _dev_fc41d_ble_t 
{
	uint8_t status;
	uint8_t conn;
	uint16_t mtu;
	uint8_t name[21];
    uint8_t mac[6];

	int8_t send_flag;

	int8_t local_only;		// 本地模式(不进行mqtt上报，仅蓝牙连接)
} DevFc41dBle_t;

typedef struct _dev_fc41d_com_t 
{
	uint8_t ver[64];
} DevFc41dCom_t;

typedef struct _dev_fc41d_socket_info_t
{
	uint8_t status;			// 当前socket状态
	int8_t init_type;		// 在初始化未完成时socket类型
	int8_t type;			// 0: none, 1: tcp server, 2: tcp, 3: udp server, 4:udp
	int8_t access_mode;		// 0:缓存, 1:直吐, 2:透传

	uint32_t transpktsize;  // 透传模式下数据发送的分包长度[1,1460],default:1024
	uint32_t transwaittm;	// 透传模式下串口接收用户数据的等待时间 [0,20]*100ms default:2
	uint32_t accept_mode;	// "TCP Incoming"连接的接入方式 0:自动接入,1手动接入
	uint32_t tcp_tw_cycle;	// 占用端口的释放方式 0:立即释放,1延迟释放
	uint32_t datatype;		// 发送数据格式 0:string, 1:hex, 2:特殊定制

	char remote_ip[16];
	uint16_t remote_port;
	uint16_t local_port;

} DevFc41dSocketInfo_t;

typedef struct _dev_fc41d_socket_t 
{
	uint8_t status;

	int32_t mtu;
	int8_t send_flag;			// AT指令是否正在发送数据

	int8_t s_id;			// 服务端listener soket
	int8_t c_id[12];		// 客户端socket

	DevFc41dSocketInfo_t sock[12]; // socket句柄
} DevFc41dSocket_t;

typedef struct _dev_fc41d_mqtt_t 
{
	uint8_t status;				// 当前MQTT任务状态

	int32_t mtu;				// 最大传输单元
	uint8_t conn_stat;			// MQTT连接状态
	uint8_t errcode;			// 最后一次记录的MQTT错误

	char version[16];			// MQTT版本
    int32_t sock_id;			// AT指令socket id(0~5)
    int32_t datatype;			// AT指令上报数据类型
    int32_t ssl;
    int32_t keepalive;			// AT指令设置MQTT保活时间

	int8_t server_type;			// 0 ip, 1 域名, 2 HTTP(S) token服务器地址
    char server_ip[128];		// ip/域名
    char server_port[6];		// 端口号，默认1883
	char *s_url;				// token获取地址

    char clientname[64];		// AT指令MQTT客户端名称
    char username[64];			// AT指令只支持MQTT用户名
    char passwd[64];			// AT指令MQTT密码

	int8_t send_flag;			// AT指令是否正在发送数据
	int8_t try_send_count;		// AT指令发送数据第一步重试次数
	int8_t test_sub_process;	// 自发自用确认MQTT连接正常
	int8_t test_sub;			// 自发自用MQTT保活主题
} DevFc41dMqtt_t;

/**
 * @brief FC41D http custom 类型
 */
typedef enum 
{
	DEV_FC41D_HTTP_CUSTOM_NONE = 0,
	DEV_FC41D_HTTP_CUSTOM_ALIYUNOSS,	// 1 阿里云OSS(upgrade使用)
	DEV_FC41D_HTTP_CUSTOM_MQTT_INFO,	// 2 MQTT 认证信息平台
} EN_DEV_FC41D_HTTP_CUSTOM_TYPE;

typedef union _union_DevFc41dHttp_t 
{
#ifdef	FC41D_HTTP_ALIYUN_OSS
	DevFc41dHttpAliyunOss_t ali_oss;
#endif

	DevFc41dHttpMqttInfo_t mqtt_info;

} UnionDevFc41dHttp_t;

typedef struct _dev_fc41d_http_t
{
	uint8_t status;
	int32_t cmd;
	int32_t sub_cmd;

	uint8_t resp_type;
	uint8_t resp_header;

	uint16_t timeout;

	uint8_t ota_fc41d;
	uint32_t ota_fc41d_reboot_time;
	int32_t url_len;
	char *url;

	int8_t first_pkg;
	int32_t _continue;

	int32_t custom_type;
	UnionDevFc41dHttp_t custom;
} DevFc41dHttp_t;

typedef struct _dev_fc41d_ringbuf_relay_multi_cmd_t 
{
	rbuffer_t ctx;
	uint8_t arr[1024];
} DevFc41dRingBufRelayMultiCmd_t;

typedef struct _dev_fc41d_nova_cmd_t
{
	uint8_t status;
	int32_t cmd;
	int32_t sub_cmd;

	DevFc41dRingBufRelayMultiCmd_t rb_data;
} DevFc41dNovaCmd_t;

typedef struct _dev_fc41d_ringbuf_read_t 
{
	rbuffer_t ctx;
	uint8_t arr[IRQ_READ_DATA_RX_SIZE];
} DevFc41dRingBufRead_t;

typedef struct _dev_fc41d_ringbuf_write_t 
{
	rbuffer_t ctx;
	uint8_t arr[1024*4];
} DevFc41dRingBufWrite_t;

typedef struct _dev_fc41d_ringbuf_busy_t 
{
	rbuffer_t ctx;
	uint8_t arr[512];
} DevFc41dRingBufBusy_t;

typedef struct _dev_fc41d_conf_t
{
	int8_t wifi_auto_conn; // MQTT自动连接(最大限度保持在线)
	uint32_t wifi_auto_conn_timeout; // MQTT自动连接超时时间
	uint32_t wifi_auto_conn_time;

	int8_t mqtt_auto_conn; // MQTT自动连接(最大限度保持在线)
	uint32_t mqtt_auto_conn_timeout; // MQTT自动连接超时时间
	uint32_t mqtt_auto_conn_time;

} DevFc41dConf_t;

typedef struct _dev_fc41d_fn_t
{
	int32_t (*fn_usart_stat_get)(DevFc41d_t *);
	int32_t (*fn_usart_stat_set)(DevFc41d_t *, int32_t);
	int32_t (*fn_process_stat_get)(DevFc41d_t *);

} DevFc41dFn_t;

typedef struct _dev_fc41d_write_stat_t
{
	int8_t status;			// 发送状态
	uint32_t timeout;		// 发送超时时间
	uint32_t try_count;		// 发送AT失败尝试次数
	uint32_t employ_time;	// 处理指令占用时间
	uint32_t relay_low_machine;	// 处理下位机指令占用时间
} DevFc41dWriteStat_t;

typedef struct _dev_fc41d_update_cache_t
{
	rbuffer_t ctx;
	uint32_t size;
	uint8_t arr[1024*2];
} DevFc41dUpgradeCache_t;

typedef struct _dev_fc41d_upgrade_t
{
	int8_t type;			// 升级类型[1 remote , 2 local]
	uint8_t pkg_type;		// 升级包类型
	int8_t flag;			// 升级中标志
	int8_t stat;			// 升级状态[0 失败, 非0成功]
	uint32_t time;			// 升级时长
	uint32_t timeout;		// 升级超时时长
	uint32_t retry_count;	// 重试次数

	DevFc41dUpgradeCache_t down; // 升级包cache
} DevFc41dUpgrade_t;

typedef struct _dev_fc41d_ctx_t 
{
	int8_t stat;
	uint32_t stat_timeout;

	// task list
	int32_t task_size;
	DevFc41dTaskList_t task;

	int32_t usart_status;
	int32_t process_status;
	int8_t urc_status;

	DevFc41dConf_t cfg;				// fc41d 默认配置

	DevFc41dWriteStat_t write_stat; // 发送状态

	DevFc41dRingBufRead_t rb_read;		// recv data
	DevFc41dRingBufWrite_t rb_write;	// write data
	DevFc41dRingBufBusy_t rb_busy;		// busy data wait input rb_read;

	DevFc41dUpgrade_t upgrade;

	DevFc41dCom_t com_info;						// common info
	DevFc41dBle_t ble;							// bluetooth ctx
	DevFc41dWifi_t wifi;						// wifi ctx
	DevFc41dSocket_t socket;					// socket ctx
	DevFc41dMqtt_t mqtt;						// mqtt ctx
	DevFc41dHttp_t http;						// http|https
	DevFc41dNovaCmd_t nova_cmd;	// bluetooth ctx

	DevFc41dFn_t fn;

	DevFc41dTaskList_t tl[FC41D_TASK_MAX_SIZE];
	uint32_t ack_seq;				// uniform ack seq
} DevFc41dCtx_t;

#pragma pack()

int32_t StrEncode(const char *src, char *dest, int32_t dest_size);
int32_t StrDecode(const char *src, char *dest, int32_t dest_size);

#endif /*_DEV_FC41D_COMMON_H*/
