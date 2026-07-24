
#include "slave_bms.h"
#include "dev_modbus.h"
#include "public_proto.h"
#include "dev_udp_comm.h"
PackSlavePara_t            gstPackSlavePara;

#if (TEST_MODE == 2)
    static uint8_t _gSystemTestMpde = 2;            //测试模式
#else
    static uint8_t _gSystemTestMpde = 0;            //正常模式
#endif 


SlaveUdpWrite_t gstDspBmsSlaveWrite;
//PackPara_t      PackInfo[MAX_PACK_NUM];//
//PackInfo_t      PackNowInfo[MAX_PACK_NUM];
uint8_t BmsMoodLedData[30]; //颜色下发数据缓存
    
typedef enum
{
    BMS_ID_STATUS = 0,
    BMS_SEND_STATUS,
    BMS_SEND_DELAY_STATUS,
} EN_BMS_UDP_READ_EVENT;

typedef enum 
{
    EN_ADDRING_INIT,
    EN_ADDRING_INIT_DELAY,
    EN_ADDRING_SET_IP,
    EN_ADDRING_SET_READ_MODEIP_CMD,
    EN_ADDRING_READ_MODEIP_CHECK, 
    EN_ADDRING_SET_DOWN_SWITCH,
    EN_ADDRING_READ_SWITCH_CMD,
    EN_ADDRING_SWITCH_STATUS_CHECK,
    EN_ADDRING_OVER,
    EN_ADDRING_EXIT
} EN_BMS_ADDRING_STATUS;




EN_BMS_UDP_READ_EVENT eBmsReadEvent;
#define         SLAVE_BMS_UDP_WRITE_QUEUE_DEPTH         (5)
Queue_t         gBmsUdpSlaveCommSendQueue;
uint8_t BmsUdpSendBuf[1036];

//static __SDRAM uint8_t BmsUdpSendBuf[1036];

void SetBmsAddr(SlaveIpInfo slave_ip_port);
extern  SlaveUdpCommPara_t gstBmsUdpCommPara;
extern  SlaveIpInfo  gstBmsIpInfo[BMS_MAX_NUM];
extern  SlaveIpInfo  gstBmsDefaultIpInfo;
extern  SlaveIpInfo  gstBmsBroadcastIpInfo;
BmsCtrlInfo_t  BmsCtrlInfo;




void SetSysEnterTestMode(uint8_t test_mode)
{
    _gSystemTestMpde = test_mode;
}

typedef struct
{
	uint32_t base_addr;
	uint16_t num;
	void *pdata;
} UdpBmsDcdcDataPoint_t;


typedef enum
{
	EN_ATTR_INVALID = 0,
	EN_ATTR_RO,
	EN_ATTR_WO,
	EN_ATTR_RW,
} EN_TRANF_DATA_ATTR_RW;

typedef enum
{
	EN_DATA_INVALID = 0,
	EN_DATA_U8,
	EN_DATA_S8,
	EN_DATA_U16,
	EN_DATA_S16,
	EN_DATA_U32,
	EN_DATA_S32,
	EN_DATA_STRING,
} EN_TRANF_DATA_TYPE;


static const UdpBmsDcdcDataPoint_t UdpBmsDcdcDataPoint[] =
{
    {UDP_READ_BMS_BASE_ADDR,    BMS_DATA_NUM,   &gstPackSlavePara.pack_para[0].bms_info  },  //0         //?????长度要改
    {UDP_READ_DCDC_BASE_ADDR,   DCDC_DATA_NUM,  &gstPackSlavePara.pack_para[0].dcdc_info },  //1
};
static const UdpBmsDcdcDataPoint_t UdpBmsDcdcDataPoint_1[] =
{
    {UDP_READ_BMS_BASE_ADDR,    BMS_DATA_NUM,   &gstPackSlavePara.pack_para[1].bms_info  },  //0
    {UDP_READ_DCDC_BASE_ADDR,   DCDC_DATA_NUM,  &gstPackSlavePara.pack_para[1].dcdc_info },  //1
};

static const UdpBmsDcdcDataPoint_t UdpBmsDcdcDataPoint_2[] =
{
    {UDP_READ_BMS_BASE_ADDR,    BMS_DATA_NUM,   &gstPackSlavePara.pack_para[2].bms_info  },  //0
    {UDP_READ_DCDC_BASE_ADDR,   DCDC_DATA_NUM,  &gstPackSlavePara.pack_para[2].dcdc_info },  //1
};
static const UdpBmsDcdcDataPoint_t UdpBmsDcdcDataPoint_3[] =
{
    {UDP_READ_BMS_BASE_ADDR,    BMS_DATA_NUM,   &gstPackSlavePara.pack_para[3].bms_info  },  //0
    {UDP_READ_DCDC_BASE_ADDR,   DCDC_DATA_NUM,  &gstPackSlavePara.pack_para[3].dcdc_info },  //1
};
static const UdpBmsDcdcDataPoint_t UdpBmsDcdcDataPoint_4[] =
{
    {UDP_READ_BMS_BASE_ADDR,    BMS_DATA_NUM,   &gstPackSlavePara.pack_para[4].bms_info  },  //0
    {UDP_READ_DCDC_BASE_ADDR,   DCDC_DATA_NUM,  &gstPackSlavePara.pack_para[4].dcdc_info },  //1
};
static const UdpBmsDcdcDataPoint_t UdpBmsDcdcDataPoint_5[] =
{
    {UDP_READ_BMS_BASE_ADDR,    BMS_DATA_NUM,   &gstPackSlavePara.pack_para[5].bms_info  },  //0
    {UDP_READ_DCDC_BASE_ADDR,   DCDC_DATA_NUM,  &gstPackSlavePara.pack_para[5].dcdc_info },  //1
};

#define UDP_BMS_DCDC_ARR_NUM     (sizeof(UdpBmsDcdcDataPoint)/sizeof(UdpBmsDcdcDataPoint_t))
const UdpBmsDcdcDataPoint_t *UpdBmsDcdDataArr[BMS_MAX_NUM] = {UdpBmsDcdcDataPoint,UdpBmsDcdcDataPoint_1, UdpBmsDcdcDataPoint_2,UdpBmsDcdcDataPoint_3,UdpBmsDcdcDataPoint_4,UdpBmsDcdcDataPoint_5};
const uint16_t  UpdBmsDcdcDataPointNumArr[BMS_MAX_NUM] = {UDP_BMS_DCDC_ARR_NUM, UDP_BMS_DCDC_ARR_NUM, UDP_BMS_DCDC_ARR_NUM, UDP_BMS_DCDC_ARR_NUM, UDP_BMS_DCDC_ARR_NUM, UDP_BMS_DCDC_ARR_NUM};

/**
  * @brief  PACK数据读取
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void UdpBmsReadParse(uint8_t *p_recv_data,uint16_t len,uint8_t num_id,uint32_t data_start_addr)
{
    uint32_t start_addr;
    uint16_t reg_num;
    uint8_t index;
    uint16_t offset;
    uint16_t i;
    uint32_t reg_addr;
    uint16_t w_buffer;

    uint16_t udp_bms_dcdc_point_num = 0;
    const UdpBmsDcdcDataPoint_t *udp_data_point;
    udp_data_point = UpdBmsDcdDataArr[num_id];
    udp_bms_dcdc_point_num = UpdBmsDcdcDataPointNumArr[num_id];
    start_addr =data_start_addr;
    reg_num = len;

    for(i = 0; i < reg_num; i++)
    {
        reg_addr = start_addr + i;
        for(index = 0; index < udp_bms_dcdc_point_num; index++)
        {
            if(NULL != udp_data_point[index].pdata)
            {
                if((reg_addr >= udp_data_point[index].base_addr) && (reg_addr < (udp_data_point[index].base_addr + udp_data_point[index].num)))
                {
//                    offset = reg_addr - upd_data_point[index].base_addr;
//                    w_buffer = p_recv_data[i];
//                    *((uint8_t *)upd_data_point[index].pdata + offset) = (uint8_t)w_buffer;
                    offset = reg_addr - udp_data_point[index].base_addr;
                    w_buffer = DataToUnshort(p_recv_data + i * 2, USING_MODE_BIG);
                    *((uint16_t *)udp_data_point[index].pdata + offset) = w_buffer;
                    break;
                }
            }
        }
    }
}

uint8_t TestTranfDta[20]= {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0X10,
                           0X11,0X12,0X13,0X14,0X15,0X16,0X17,0X18,0X19,0X20
};

////len 寄存器的个数
////此函数仅仅是将协议中的32bit的数据 高16bit和低16bit进行了交换
////以后可以进行16bit批量处理。
//void UdpBmsTranfDataParse(uint8_t *p_recv_data,uint16_t len,uint32_t data_start_addr)
//{
//    uint32_t start_addr;
//    uint16_t reg_num;
//    uint8_t index;
//    uint16_t i;
//    uint32_t reg_addr;
//    uint16_t w_buffer;
//    uint16_t w_buffer1 =0;
//    uint16_t test_w_buffer =0;
//    uint16_t test_w_buffer1 = 0;

//    start_addr =data_start_addr;
//    reg_num = len;

//    for(i = 0; i < reg_num; i++)
//    {
//        reg_addr = start_addr + i;
//        for(index = 0; index < TRANF_TABLE_NUM; index++)
//        {
//            if(NULL != p_recv_data)
//            {
//                if((reg_addr >= TranfDataPoint[index].base_addr) && (reg_addr < (TranfDataPoint[index].base_addr + TranfDataPoint[index].num)))
//                {
//                    if( (EN_DATA_U32 == TranfDataPoint[index].data_type) || (EN_DATA_S32 == TranfDataPoint[index].data_type) )
//                    {
////                        offset = reg_addr - TranfDataPoint[index].base_addr;
//                        w_buffer  = DataToUnshort(p_recv_data + i * 2, USING_MODE_SMALL);
//                        w_buffer1 = DataToUnshort(p_recv_data + (i+1) * 2, USING_MODE_SMALL);
////                        *((uint16_t *)TranfDataPoint[index].pdata + offset ) = w_buffer;  //将32bit数据已经按照高16bit 和 低 16bit反转了。
////                        test_w_buffer = *((uint16_t *)p_recv_data + i);
////                        test_w_buffer1 = *((uint16_t *)p_recv_data + i + 1);
//                        *((uint16_t *)p_recv_data + i) = w_buffer1;
//                        *((uint16_t *)p_recv_data + i + 1) = w_buffer;
////                        test_w_buffer = *((uint16_t *)p_recv_data + i);
////                        test_w_buffer1 = *((uint16_t *)p_recv_data + i + 1);

//                        i+=1;

//                    }
//                    break;
//                }
//              
//            }
//        }
//    }  
//}

uint8_t test_data[128]={0x03,0x00,0x00,0x43,0x00,0x00,0x00,0x00,0x0A,0x00,
                        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x10,
                        0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x20};
static void test_recv_fun_call_back(uint8_t *pdata, uint16_t len, uint8_t num_id)
{//.
    uint8_t cnt = 0;
    uint16_t data = 0;
    volatile uint32_t data_addr = 0;
    uint16_t data_num = 0;
    uint16_t i = 0;
    uint8_t *p_recv_data = NULL;
    uint8_t *p_bms_data = NULL;
    uint8_t cmd = 0;
    pdata = test_data;
    len = 10;
    num_id = 0;
    
    if(num_id > BMS_MAX_NUM)
    {
        num_id = 0; 
    }
//    if(num_id == 0)
//    {//num_id 是广播数据
//        return ;
//    }
    if(len < 8)
    {
        return;
    }
    data = *(uint16_t *)&pdata[cnt];

    if(data != UDP_SLAVE_TYPE_BMS)
    {
        return;
    }
    cnt += 2;
    data = pdata[cnt++];
    cmd = pdata[cnt++];
    data_addr = *(uint32_t *)&pdata[cnt];


    if(cmd != 0X43)
    {
        //读取操作。
        return;
    }
    cnt += 4;
    data_num = *((uint16_t *)&pdata[cnt]);
    cnt += 2;
    p_recv_data = &pdata[cnt];
    data_num<<=1;  //字节数 按照8bit解析
    UdpBmsReadParse(p_recv_data,data_num,0,0X30010000);
    return;       
}



uint32_t recv_cnt=0;
static void recv_fun_call_back(uint8_t *pdata, uint16_t len, uint8_t num_id)
{

    uint8_t cnt = 0;
    uint16_t data = 0;
    volatile uint32_t data_addr = 0;
    uint16_t data_num = 0;
    uint16_t i = 0;
    uint8_t *p_recv_data = NULL;
    uint8_t *p_bms_data = NULL;
    uint8_t cmd = 0;
  
    if(num_id >= BMS_MAX_NUM)
    {
        num_id = 0; 
    }
    if(len < 8)
    {
        return;
    }
    data = *(uint16_t *)&pdata[cnt];

    if(data != UDP_SLAVE_TYPE_BMS)
    {
        return;
    }
    cnt += 2;
    data = pdata[cnt++];
    cmd = pdata[cnt++];
    data_addr = *(uint32_t *)&pdata[cnt];
    if(cmd != 0X43)
    {
        return;
    }

    recv_cnt++;
    cnt += 4;
    data_num = *((uint16_t *)&pdata[cnt]);
    cnt += 2;
    p_recv_data = &pdata[cnt];

    UdpBmsReadParse(p_recv_data,data_num,num_id,data_addr);  //进行批量处理即可。
//根据上位机设置，将第n个电池包数据更新采集器数据
//否则只是更新上位机需要展示的数据。
    BmsCtrlInfo.bms_ctrl_info_value.bits.recv_flag =1;
    //回包后重置该Pack的接收计数
    BmsCtrlInfo.pack_offline_count[num_id] = 0;
    
    return;    
        
}

void BmsUdpParaInit(void)
{
    InitalBmsUdpPara(recv_fun_call_back);
    QueueInit(&gBmsUdpSlaveCommSendQueue, SLAVE_BMS_UDP_WRITE_QUEUE_DEPTH, sizeof(gstDspBmsSlaveWrite));
}

//编码过程  162.168.1.0  -->ADDR  DATA
//读取写的值 是不是1
//电芯num+1
//162.168.1.1（远程地址++）  //读取写的值
//返回地址

void TestSendBmsUdpData(void)
{ 

    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = 0xff;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = 0x45;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = 0x1000;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 4;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = 0;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[2] = 2;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[3] = 3;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[4] = 4;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[5] = 5;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[6] = 6;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[7] ++;
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsUdpCommPara.slave_ip_info.remot_ipaddr,sizeof(gstBmsUdpCommPara.slave_ip_info));  //广播
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        ;
    }
}
/**
  * @brief  写Byte数据
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
//data_addr 数据地址    data_len 数据字节长度
void BroadCastByteDataToBms(uint32_t data_addr,uint16_t data_len,uint8_t *pdata) 
{ 
    uint16_t len = 0;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = BYTE_MODE; //字节操作
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = data_addr;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = data_len; //字节数
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    memcpy(gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data, pdata, data_len);
    len = (gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length + 10);
    memcpy(BmsUdpSendBuf, gstDspBmsSlaveWrite.slave_udp_write_data.data, len);
//    Lwip_udp_sendto_data(gstDspBmsSlaveWrite.slave_udp_pcb,BmsUdpSendBuf,len,&gstBmsDefaultIpInfo.remot_ipaddr, gstBmsDefaultIpInfo.remot_port);
    Lwip_udp_sendto_data(gstDspBmsSlaveWrite.slave_udp_pcb,BmsUdpSendBuf,len,&gstBmsBroadcastIpInfo.remot_ipaddr, gstBmsBroadcastIpInfo.remot_port);//广播
}

/**
  * @brief  写Word数据
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
//data_addr 数据地址    data_len 寄存器长度
void BroadCastWordDataToBms(uint32_t data_addr,uint16_t data) 
{ 
    uint8_t len = 0;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE; //寄存器操作
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = data_addr;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1; 
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    len = ((gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length <<1) + 10);
    memcpy(BmsUdpSendBuf, gstDspBmsSlaveWrite.slave_udp_write_data.data, len);
    Lwip_udp_sendto_data(gstDspBmsSlaveWrite.slave_udp_pcb,BmsUdpSendBuf,len,&gstBmsBroadcastIpInfo.remot_ipaddr, gstBmsBroadcastIpInfo.remot_port);
}

//data_addr 数据地址    data_len 寄存器长度
void _TestBroadCastWordDataToBms(uint32_t data_addr,uint16_t* data,uint16_t len) 
{ 
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE; //寄存器操作
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = data_addr;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = len; 
    memcpy(gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data, data, 2*len);
//    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status);
//    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status>>8);
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;

    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,\
            (uint8_t *)&gstBmsBroadcastIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //广播
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        ;
    }
}


/**
  * @brief  设置BMS地址
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void SetBmsAddr(SlaveIpInfo slave_ip_port)
{
    uint8_t len = 0;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UDP_DCDC_IP_ADDR_ADDR;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 2;

    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(slave_ip_port.remot_ipaddr.addr);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)((slave_ip_port.remot_ipaddr.addr)>>8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[2] = (uint8_t)((slave_ip_port.remot_ipaddr.addr)>>16);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[3] = (uint8_t)((slave_ip_port.remot_ipaddr.addr)>>24); 

    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;

    len = ( gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length<<1) +10;
    memcpy(BmsUdpSendBuf, gstDspBmsSlaveWrite.slave_udp_write_data.data, len);
    Lwip_udp_sendto_data(gstDspBmsSlaveWrite.slave_udp_pcb,BmsUdpSendBuf,len,&gstBmsDefaultIpInfo.remot_ipaddr, gstBmsDefaultIpInfo.remot_port);
}

/**
  * @brief  告知DCDC当前PCS类型
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void SetDcdcPcsType(uint16_t pcs_type)
{
    
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UDP_DCDC_PCS_TYPE_ADDR;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(pcs_type >> 8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(pcs_type);
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsBroadcastIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //广播IP
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        ;
    }
}

/**
  * @brief  设置下行端口
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void SetBmsDownSwitch(SlaveIpInfo slave_ip_port,uint16_t status)
{

    uint16_t len =0; 
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UDP_DOWN_SWITCH_ADDR;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status>>8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status);
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    len = ( (gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length<<1)  +10);
    memcpy(BmsUdpSendBuf, gstDspBmsSlaveWrite.slave_udp_write_data.data, len);
    Lwip_udp_sendto_data(gstDspBmsSlaveWrite.slave_udp_pcb,BmsUdpSendBuf,len,&slave_ip_port.remot_ipaddr, slave_ip_port.remot_port); 
}




void ReadDcdcUdpData(SlaveIpInfo slave_ip_port)
{ 
    uint16_t len =0; 
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_READ_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UDP_READ_DCDC_BASE_ADDR;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = DCDC_DATA_NUM;
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    len  = 10;
    memcpy(BmsUdpSendBuf, gstDspBmsSlaveWrite.slave_udp_write_data.data, len);
    Lwip_udp_sendto_data(gstDspBmsSlaveWrite.slave_udp_pcb,BmsUdpSendBuf,len,&slave_ip_port.remot_ipaddr, slave_ip_port.remot_port);
                
}

/**
  * @brief  通知BMS，所有Pack电压的平均值，用于计算DCDC的电压补偿（包间均衡微调使用）
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
bool SetBmsAllPackAveVolt(uint16_t total_ave_volt)
{    
    uint16_t data = 0;
    data = total_ave_volt;
    
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = BMS_REG_ADDR_ALL_PACK_AVE_VOLT_STATUS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(data >> 8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(data);
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsBroadcastIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //广播IP
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        ;
    }
    return true;
}

/**
  * @brief  充电均衡指令组包
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
bool SetBmsChgBlanceCtrl(uint8_t min_id)
{    
    if(min_id == 0xff)
    {
        return true;
    }
    //包头
    int16_t chg_banlance_status = 1;        //对应min_id
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UPD_DCDC_BANLANCE_EN;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    
    //设置SOC最低的Pack为充电
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(chg_banlance_status >> 8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(chg_banlance_status);
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsIpInfo[min_id].remot_ipaddr,sizeof(gstBmsDefaultIpInfo));  //
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/**
  * @brief  充电均衡清除组包
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
bool ResetBmsChgBlanceCtrl(uint8_t min_id)
{    
    //第一次没有旧ID，不用停止
    if(min_id == 0xff)
    {
        return true;
    }
    //停止之前充电的Pack
    //包头
    int16_t chg_banlance_status = 1;        //对应min_id
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UPD_DCDC_BANLANCE_EN;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = 0;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = 0;
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsIpInfo[min_id].remot_ipaddr,sizeof(gstBmsDefaultIpInfo));  //
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        return false;
    }
    return true;
}

/**
  * @brief  放电均衡指令组包
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
bool SetBmsDsgBlanceCtrl(uint8_t max_id)
{    
    //第一次没有旧ID，不用停止
    if(max_id == 0xff)
    {
        return true;
    }
    //设置SOC最高的Pack放电
    //包头
    int16_t dsg_banlance_status = -1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UPD_DCDC_BANLANCE_EN;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(dsg_banlance_status >> 8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(dsg_banlance_status);
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsIpInfo[max_id].remot_ipaddr,sizeof(gstBmsDefaultIpInfo));  //
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        return false;
    }
    
    return true;
}

/**
  * @brief  放电均衡指令清除组包
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
bool ResetBmsDsgBlanceCtrl(uint8_t max_id)
{    
    //第一次没有旧ID，不用停止
    if(max_id == 0xff)
    {
        return true;
    }
    //停止之前放电的Pack
    //包头
    int16_t dsg_banlance_status = -1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UPD_DCDC_BANLANCE_EN;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = 0;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = 0;
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsIpInfo[max_id].remot_ipaddr,sizeof(gstBmsDefaultIpInfo));  //
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        return false;
    }
    
    return true;
}

bool SetBmsBlanceDisable(void)
{
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UPD_DCDC_BANLANCE_EN;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = 0;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = 0;
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsBroadcastIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //广播开始升级
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        ;
    }
    return true;    
}





uint32_t ExchangeWord(uint32_t data)
{
    
    uint32_t  dec_data=0;
    uint8_t   byte;

    dec_data = (uint8_t)data;
    byte = (uint8_t)(data>>8);
    dec_data = (dec_data<<8)+byte;
    byte = (uint8_t)(data>>16); 
    dec_data = (dec_data<<8)+byte;   
    byte = (uint8_t)(data>>24);
    dec_data = (dec_data<<8)+byte;  
    return dec_data;
    
}

/**
  * @brief  BMS地址设置函数
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void    BmsAddrFun(void)
{
    static EN_BMS_ADDRING_STATUS  e_bms_addring_status = EN_ADDRING_INIT;
    static uint32_t bms_addring_time_delay =0;
    static uint8_t  err_cnt=0;
    uint32_t ipaddr=0;
    uint32_t read_ipaddr =0;
    
#if (TEST_MODE == 2)
    //测试模式下，不设置BMS地址
        e_bms_addring_status = EN_ADDRING_OVER;
#endif

    switch(e_bms_addring_status)
    {
        case EN_ADDRING_INIT:
            BmsCtrlInfo.batt_num = 0;
            BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_over = 0;
            BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_ok = 0;
            e_bms_addring_status = EN_ADDRING_INIT_DELAY;
            bms_addring_time_delay = tim_get_systime();  
            err_cnt = 0;
            break;
        case EN_ADDRING_INIT_DELAY:
            if((uint32_t)(tim_get_systime() - bms_addring_time_delay) > 6000)//5S
            {
                bms_addring_time_delay = tim_get_systime(); 
                e_bms_addring_status = EN_ADDRING_SET_IP;
            }
            break;
        case EN_ADDRING_SET_IP:
            SetBmsAddr(gstBmsIpInfo[BmsCtrlInfo.batt_num]); 
            bms_addring_time_delay = tim_get_systime();    
            e_bms_addring_status =EN_ADDRING_SET_READ_MODEIP_CMD;
            break;
        case EN_ADDRING_SET_READ_MODEIP_CMD: 
            if((uint32_t)(tim_get_systime() - bms_addring_time_delay) > 500)//500ms
            {
                 bms_addring_time_delay = tim_get_systime(); 
                 e_bms_addring_status = EN_ADDRING_READ_MODEIP_CHECK;
                 ReadDcdcUdpData(gstBmsIpInfo[BmsCtrlInfo.batt_num]);  //使用batt——num ID进行读取
            }
            break;
        case EN_ADDRING_READ_MODEIP_CHECK:

            if((uint32_t)(tim_get_systime() - bms_addring_time_delay) > 1000)//
            {//超时
                 bms_addring_time_delay = tim_get_systime(); 
                 //超时，连续10次进行   
                 err_cnt ++;
                 if(err_cnt > 10)
                 { 
                    err_cnt = 0;
                    e_bms_addring_status = EN_ADDRING_OVER;
                 }
                 else
                 {
                    e_bms_addring_status = EN_ADDRING_SET_IP;
                 }
            }
            ipaddr = *(uint32_t *)&gstBmsIpInfo[BmsCtrlInfo.batt_num].remot_ipaddr; 
            read_ipaddr = gstPackSlavePara.pack_para[BmsCtrlInfo.batt_num].dcdc_info.ctrl_info.ip_addr[0] << 16 |
                          gstPackSlavePara.pack_para[BmsCtrlInfo.batt_num].dcdc_info.ctrl_info.ip_addr[1]           ;
//            read_ipaddr = DataToUnshort((uint8_t *)&read_ipaddr, USING_MODE_BIG);
//            read_ipaddr = PackInfo[BmsCtrlInfo.batt_num].dcdc_info.ip_addr[0];
//            read_ipaddr <<=16;
//            read_ipaddr += PackInfo[BmsCtrlInfo.batt_num].dcdc_info.ip_addr[1];  
            read_ipaddr =  ExchangeWord(read_ipaddr);
           
            if(read_ipaddr == ipaddr)
            {
                e_bms_addring_status = EN_ADDRING_SET_DOWN_SWITCH;
            }

            break;


        case EN_ADDRING_SET_DOWN_SWITCH:
//设置打开下行开关。
            {
                SetBmsDownSwitch(gstBmsIpInfo[BmsCtrlInfo.batt_num],DCDC_DOWN_SWITCH_ON);
                e_bms_addring_status = EN_ADDRING_READ_SWITCH_CMD;
                
            }
            break;

        case EN_ADDRING_READ_SWITCH_CMD:
            {//回读下行开关状态
                if((uint32_t)(tim_get_systime() - bms_addring_time_delay) > 500)//
                {//超时
                    bms_addring_time_delay = tim_get_systime();  
                    ReadDcdcUdpData(gstBmsIpInfo[BmsCtrlInfo.batt_num]); 
                    e_bms_addring_status = EN_ADDRING_SWITCH_STATUS_CHECK;
                        
                }
            }
            break;
        case EN_ADDRING_SWITCH_STATUS_CHECK:
            if((uint32_t)(tim_get_systime() - bms_addring_time_delay) > 1000)//
            {//超时
                 bms_addring_time_delay = tim_get_systime(); 
                 //超时，连续3次进行   
                 err_cnt ++;
                 if(err_cnt > 6)
                 { 
                    err_cnt = 0;
                    e_bms_addring_status =EN_ADDRING_OVER;
                 }
                 else
                 {
                    e_bms_addring_status = EN_ADDRING_SET_DOWN_SWITCH;
                 }
            }


            if(gstPackSlavePara.pack_para[BmsCtrlInfo.batt_num].dcdc_info.ctrl_info.forward_status == DCDC_DOWN_SWITCH_ON)
            {
                //记录BMS数量
                BmsCtrlInfo.batt_num ++;
                if(BmsCtrlInfo.batt_num > BMS_MAX_NUM)
                {
                    //退出编址
                    e_bms_addring_status = EN_ADDRING_OVER;
                }
                else
                {//成功
                    err_cnt = 0;
                    e_bms_addring_status = EN_ADDRING_SET_IP;
                }
            }

            break;
        case EN_ADDRING_OVER:
            e_bms_addring_status = EN_ADDRING_EXIT;
            BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_over = 1; 
            if((BmsCtrlInfo.batt_num == 0)||(BmsCtrlInfo.batt_num == BMS_MAX_NUM))
            {//编址失败
                BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_ok = 0;

            }
            else
            {//编址OK
                BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_ok = 1;
            }
            break;
        case EN_ADDRING_EXIT:
            {
            }
            break;
        default:
            break;
    }
    
}


void BmsCommProc(void)
{
    SlaveUdpWrite_t *pBmsUdpSlaveWrite;
    static uint8_t read_dptr = 0;
    uint8_t i = 0;
    uint16_t len =0;
    struct udp_pcb *upcb1 = NULL;
    static uint8_t read_enent_dptr = 0;
    static uint32_t bms_send_time_delay = 0; 
    uint16_t data_len=0;

//    test_recv_fun_call_back(test_data,21,0);

    if(BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_over == 0)
    {
        return ;
    }

    
    //测试模式使用下方测试代码
    if(2 == _gSystemTestMpde)
    {
        
        gstPackSlavePara.pack_para[0].bms_info.base_info.model_id = 0x1234;
        gstPackSlavePara.pack_para[0].bms_info.base_info.hw_ver = 0;
        gstPackSlavePara.pack_para[0].bms_info.base_info.sw_ver[1] = 1;
        gstPackSlavePara.pack_para[0].bms_info.status_info.total_vol = 0x1388;
        gstPackSlavePara.pack_para[0].bms_info.status_info.soc_disp = 900;
        gstPackSlavePara.pack_para[0].bms_info.status_info.soh = 1000;
        gstPackSlavePara.pack_para[0].bms_info.status_info.max_chg_allow_curr = 0xC80;//
        gstPackSlavePara.pack_para[0].bms_info.status_info.max_dsg_allow_curr = 0xC80;
//        gstBmsSlavePara.pack_para[0].bms_info.base_info.exp_num = 0x01;                   ?????

        gstPackSlavePara.pack_para[0].bms_info.status_info.cell_ave_temp = 0x0100;
        gstPackSlavePara.pack_para[0].bms_info.error_info.mos_error   = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.temp_error  = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.curr_error  = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.vol_error   = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.sox_error   = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.other_error = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.mos_alarm   = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.temp_alarm  = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.curr_alarm  = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.vol_alarm   = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.sox_alarm   = 0;
        gstPackSlavePara.pack_para[0].bms_info.error_info.other_alarm = 0;
        
//        gstBmsSlavePara.pack_para[0].bms_info.base_info.stop_pre_dsg = 1;         ?????
        gstPackSlavePara.bms_com_time_out[0] = 0;
        gstPackSlavePara.bms_comm_err_cnt[0] = 0;
        gstPackSlavePara.bms_com_time_out[1] = 0;
        gstPackSlavePara.bms_comm_err_cnt[1] = 0;
        
//        PackInfo[0].bms_info.bms_pre_chg_status = 1;      ?????
        
        return;
    }

    switch(eBmsReadEvent)
    {
        case BMS_ID_STATUS:
        {
            if(BmsCtrlInfo.bms_ctrl_info_value.bits.mood_led_ctrl == 1)
            {
                BmsCtrlInfo.bms_ctrl_info_value.bits.mood_led_ctrl = 0;
                len = 18;
                memcpy(BmsUdpSendBuf,BmsMoodLedData,len);
//                Lwip_udp_sendto_data(gstBmsUdpCommPara.slave_udp_pcb,BmsUdpSendBuf,len,\
//                                            &gstBmsBroadcastIpInfo.remot_ipaddr, gstBmsBroadcastIpInfo.remot_port);
                eBmsReadEvent = BMS_SEND_DELAY_STATUS;


            }
            else if(QueueGet(&gBmsUdpSlaveCommSendQueue, (void *)&pBmsUdpSlaveWrite) == true)/*如果有主动写操作*/
            {
                uint16_t len = (pBmsUdpSlaveWrite->slave_udp_write_data.write_data_t.length<<1)+10;
                memcpy(BmsUdpSendBuf, pBmsUdpSlaveWrite->slave_udp_write_data.data, len);
                Lwip_udp_sendto_data(pBmsUdpSlaveWrite->slave_udp_pcb,BmsUdpSendBuf,len, \
                                    &pBmsUdpSlaveWrite->slave_ip_port.remot_ipaddr, pBmsUdpSlaveWrite->slave_ip_port.remot_port);
                eBmsReadEvent = BMS_SEND_DELAY_STATUS;

            }
            else
            {
//从第一个包开始 每次全部读取//主动查询 
                switch(read_enent_dptr)
                {
                    case 0:
//读取BMS数据
                        read_enent_dptr = 1;
                        gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
                        gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
                        gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_READ_CMD;
                        gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr = UDP_READ_BMS_BASE_ADDR;  
                        data_len = (sizeof(gstPackSlavePara.pack_para[0].bms_info));
                        break;
                    case 1:

                        read_enent_dptr = 0;
                        gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
                        gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
                        gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_READ_CMD;
                        gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr = UDP_READ_DCDC_BASE_ADDR;         
                        data_len = sizeof(gstPackSlavePara.pack_para[0].dcdc_info);
                        if(BmsCtrlInfo.batt_num == 0)
                        {
                            read_dptr = 0;
                        }
                        else
                        {
                            read_dptr++;
                            read_dptr %= (BmsCtrlInfo.batt_num);                            
                        }

                        break;
                    default:
                        break;
                }

                gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = (data_len>>1);
                len = 10;
                memcpy(BmsUdpSendBuf, gstDspBmsSlaveWrite.slave_udp_write_data.data, len);
                if(BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_ok == 0)
                {
                    Lwip_udp_sendto_data(gstBmsUdpCommPara.slave_udp_pcb,BmsUdpSendBuf,len,\
                                            &gstBmsDefaultIpInfo.remot_ipaddr, gstBmsDefaultIpInfo.remot_port);                        
                }
                else
                {
                    Lwip_udp_sendto_data(gstBmsUdpCommPara.slave_udp_pcb,BmsUdpSendBuf,len,\
                                            &gstBmsIpInfo[read_dptr].remot_ipaddr, gstBmsIpInfo[read_dptr].remot_port);
                } 
                
                //每个Pack的接收计数，接收到回报后重置该计数，计数超过10判断Pack离线
                if(BmsCtrlInfo.pack_offline_count[read_dptr] <= 15)
                {
                    BmsCtrlInfo.pack_offline_count[read_dptr]++;
                }
                //每个Pack的离线标志
                if(BmsCtrlInfo.pack_offline_count[read_dptr] > 10)
                {
                    BmsCtrlInfo.pack_offline[read_dptr] = 1;
                }
                else
                {
                    BmsCtrlInfo.pack_offline[read_dptr] = 0;
                }
                
                eBmsReadEvent = BMS_SEND_STATUS;
            }
            bms_send_time_delay = tim_get_systime();
            break;
        }
        case BMS_SEND_STATUS:
        {

            if(BmsCtrlInfo.bms_ctrl_info_value.bits.recv_flag == 1)
            {
                BmsCtrlInfo.bms_ctrl_info_value.bits.recv_flag = 0;
                eBmsReadEvent = BMS_ID_STATUS;  
                gstPackSlavePara.bms_comm_err_cnt[0] = 0;
                gstPackSlavePara.bms_com_time_out[0] = 0;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - bms_send_time_delay) > 300)
                {
                    bms_send_time_delay = tim_get_systime();
                    gstPackSlavePara.bms_comm_err_cnt[0] ++;
                    if(gstPackSlavePara.bms_comm_err_cnt[0] >= 10)
                    {
                        gstPackSlavePara.bms_comm_err_cnt[0] = 10;
                        gstPackSlavePara.bms_com_time_out[0] = 1;
                    }
                    eBmsReadEvent = BMS_ID_STATUS;
                }
            }
            //有一个Pack离线，报离线故障
            if(BmsCtrlInfo.pack_offline[0] == 1 ||
               BmsCtrlInfo.pack_offline[1] == 1 ||
               BmsCtrlInfo.pack_offline[2] == 1 ||
               BmsCtrlInfo.pack_offline[3] == 1 ||
               BmsCtrlInfo.pack_offline[4] == 1 ||
               BmsCtrlInfo.pack_offline[5] == 1
              )
            {
                gstPackSlavePara.bms_com_time_out[0] = 1;
            }




//            if((uint32_t)(tim_get_systime() - bms_send_time_delay) > 100)// 100ms
//            {
//                bms_send_time_delay = tim_get_systime();
//                eBmsReadEvent = BMS_ID_STATUS;    
//            }
            break;
        }
        case BMS_SEND_DELAY_STATUS:
        {
            if((uint32_t)(tim_get_systime() - bms_send_time_delay) > 30)
            {
                bms_send_time_delay = tim_get_systime();  
                eBmsReadEvent = BMS_ID_STATUS;
                    
            }
            break;

            break;
        }
        default:
            break;
    }
}

/**
* @brief   BMS Led灯板控制数据帧组包
* @param   None
* @return  None
* @note    50ms轮询一次
*/

bool BmsMoodLedCtrl(uint32_t clour,uint16_t mode,uint16_t period)
{
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UDP_MOOD_LED_CLOUR_ADDR ;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 4;

    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)((clour) >> 8); //状态
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)((clour) >> 0); //R
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[2] = (uint8_t)((clour) >> 24); //B
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[3] = (uint8_t)((clour) >> 16); //G
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[4] = (uint8_t)(mode >> 8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[5] = (uint8_t)(mode);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[6] = (uint8_t)(period >> 8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[7] = (uint8_t)(period);


    memcpy(BmsMoodLedData, &gstDspBmsSlaveWrite.slave_udp_write_data.data,18);
    Lwip_udp_sendto_data(gstBmsUdpCommPara.slave_udp_pcb,BmsMoodLedData,18,\
                                            &gstBmsBroadcastIpInfo.remot_ipaddr, gstBmsBroadcastIpInfo.remot_port);
    
//    BmsCtrlInfo.bms_ctrl_info_value.bits.mood_led_ctrl = 1;
    return true;
//    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
//    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsBroadcastIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //默认编制IP
//    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
//    {
//        return false;
//    }
//    return true;
}

//void BroadCastWordDataToBms(uint32_t data_addr,uint16_t data) 
//{ 
//    uint8_t len = 0;
//    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
//    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE; //寄存器操作
//    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
//    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = data_addr;
//    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1; 
//    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
//    len = ((gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length <<1) + 10);
//    memcpy(BmsUdpSendBuf, gstDspBmsSlaveWrite.slave_udp_write_data.data, len);
//    Lwip_udp_sendto_data(gstDspBmsSlaveWrite.slave_udp_pcb,BmsUdpSendBuf,len,&gstBmsBroadcastIpInfo.remot_ipaddr, gstBmsBroadcastIpInfo.remot_port);
//}


/**
 * @brief  BMS关机控制发送
  * @param   None  
  * @return  None
  * @note    None
  */
bool BMSShutDownSend()
{
    uint16_t data = 0;
    data = gstPackSlavePara.pack_para[0].bms_info.ctrl_info.shutdown | 0X08;
    
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = UDP_BMS_SHUTDOWN_ADDR;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(data >> 8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(data);
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsBroadcastIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //广播IP
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        ;
    }
    return true;
}

bool SetBmsSysUpgradeStatus(uint16_t status)
{
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = BMS_REG_ADDR_SYS_UPGRADE_STATUS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status >> 8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status);
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsBroadcastIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //广播开始升级
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        ;
    }
    return true;
}

bool SetFirstPackRest(uint16_t status)
{
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = BMS_REG_ADDR_SYS_UPGRADE_STATUS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status >> 8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status);
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsIpInfo[0].remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //只给第一个包发送数据
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        ;
    }
    return true;    
}

bool SetDspWakeUpStatusToBms(uint8_t status)
{
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = BMS_REG_ADDR_DSP_WAKEUP_STATUS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = 1;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[0] = (uint8_t)(status >> 8);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[1] = (uint8_t)(status);
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
//    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,(uint8_t *)&gstBmsDefaultIpInfo.remot_ipaddr,sizeof(gstBmsDefaultIpInfo));  //默认编制IP
//    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
//    {
//        ;
//    }
    return true;
}

bool CollectWriteDcdcFun(uint16_t reg_addr, uint16_t reg_num, uint8_t *pdata, uint16_t bms_index, uint8_t using_mode)
{
    if(bms_index > 4)
    {
        return false;
    }

    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = (reg_addr + UDP_READ_DCDC_BASE_ADDR);//减去modbus偏移地址
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = reg_num;

 
    for(uint8_t i = 0; i < reg_num; i++)
    {
        *(uint16_t *)&gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[i*2] = DataToUnshort(pdata + i * 2, using_mode);

    }
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;
//    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,\
//                (uint8_t *)&gstBmsDefaultIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //默认IP

    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,\
                (uint8_t *)&gstBmsBroadcastIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //广播
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        ;
    }
    return true;
}





bool CollectWriteBmsFun(uint16_t reg_addr, uint16_t reg_num, uint8_t *pdata, uint16_t bms_index, uint8_t using_mode)
{
    if(bms_index > BMS_MAX_NUM)
    {
        return false;
    }
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.device_type = UDP_SLAVE_TYPE_BMS;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.reserved = WORD_MODE;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.opcmd  = UDP_WRITE_CMD;
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.addr  = (reg_addr + UDP_READ_BMS_BASE_ADDR);
    gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.length = reg_num;

    for(uint8_t i = 0; i < reg_num; i++)
    {
        *(uint16_t *)&gstDspBmsSlaveWrite.slave_udp_write_data.write_data_t.data[i*2] = DataToUnshort(pdata + i * 2, using_mode);
    
    }
    gstDspBmsSlaveWrite.slave_udp_pcb = gstBmsUdpCommPara.slave_udp_pcb;


//    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,\
//                (uint8_t *)&gstBmsDefaultIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //默认IP

    memcpy((uint8_t *)&gstDspBmsSlaveWrite.slave_ip_port.remot_ipaddr,\
                (uint8_t *)&gstBmsBroadcastIpInfo.remot_ipaddr,sizeof(gstBmsBroadcastIpInfo));  //广播
    if(QueuePut(&gBmsUdpSlaveCommSendQueue, &gstDspBmsSlaveWrite) == false)
    {
        ;
    }
    return true;
}

/*BMS 设置RTC time*/
bool BMS_RtcTimeSet(uint32_t rtc_time)
{
    bool status;
    uint32_t  tranf_time = 0;

    tranf_time = rtc_time << 16;
    tranf_time += (rtc_time >> 16);
    status = CollectWriteBmsFun(BMS_REG_ADDR_RTC_TIME, sizeof(rtc_time) / 2 , (uint8_t *)&tranf_time, 0, 0); //
    return status;

}
uint8_t GetBmsNum(void)
{
    //测试模式固定1个Pack
    if(2 == _gSystemTestMpde)
        return 1;
    else if(BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_ok == 0)
        return 0;
    else
        return (uint8_t)BmsCtrlInfo.batt_num;
}

