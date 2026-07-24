#include "slave_collect.h"
#include "dev_modbus.h"
#include "dev_modbus_rtu.h"
#include "sys_info_storage.h"
#define   COLLECT_WRITE_SIZE                265
#define   COLLECT_WRITE_QUEUE_DEPTH         5

typedef struct _pcs_type_info_t
{
	uint16_t model_id;
	char model[32];
} PcsTypeInfo_t;

PcsTypeInfo_t gPcsTypeInfo[] = {
	{ 0x00,		"Unknown" },
	{ 0x01,		"Roca5" }
};

const char* GetPcsModelByModelId(uint16_t model_id)
{
	for (int32_t i = 0; i < sizeof(gPcsTypeInfo)/sizeof(gPcsTypeInfo[0]); ++i)
	{
		if (gPcsTypeInfo[i].model_id == model_id)
		{
			return gPcsTypeInfo[i].model;
		}
	}

	return NULL;
}


void ModbusReadParse(T_ModbusRtuFrame *ptFrame);
void ModbusWriteParse(T_ModbusRtuFrame *ptFrame);
typedef  struct
{
	uint8_t  len;
	uint8_t  *pData;//[256];
}tsCollectTxData;
EN_COLLECT_POLLING_STATUS   _CollectSlavePollingStatus;

uint8_t                     _gComCollectTxDataBuf[COLLECT_WRITE_SIZE];
static  tsCollectTxData     _gComCollectTxData;                                                            //此为通过485发送缓冲

static  tsCollectTxData	    _gCollectWriteData[COLLECT_WRITE_QUEUE_DEPTH];
static  uint8_t             _CollectWriteDataBuf[COLLECT_WRITE_QUEUE_DEPTH][COLLECT_WRITE_SIZE];
static  uint8_t             _CollectWriteDataBufIndex = 0;
Queue_t	                    _gCollectWriteQueue;                                //此队列为屏幕通过接口来写采集器数据。


 
tsCollectPara_t             gCollectParaData;
tsCollectCtrlInfo           _gCollectCtrlInfo;
typedef struct _collect_ctrl
{
    uint8_t     correct_rtc_time_en;
}tsCollectCtrl;


static  tsCollectCtrl   CollectCtrl;

#define COLLECT_SLAVE_ADDR                          0X501E
#define COLLECT_SALVE_ADDR_NUM                      1
#define COLLECT_UART_BAUD_ADDR                      0X501F
#define COLLECT_UART_BAUD_NUM                       1


static  const tsModbusDataPoint NomalUserDataPoint[] = 
{
	{COLLECT_MODE_ID,                   MODE_ID_NUM,                    DATA_U16,   ATTR_RO,    &gCollectParaData.collect_data.mode_id                            },              //0   
    {COLLECT_EQUIPEMENT_ID,             COLLECT_EQUIPEMENT_ID_NUM,      DATA_U16,   ATTR_RO,    &gCollectParaData.collect_data.equipement_id[0]                   },              //1   
    {COLLECT_SLAVE_ADDR,                COLLECT_SALVE_ADDR_NUM,         DATA_U16,   ATTR_RW,    &gCollectParaData.collect_data.slave_comm_addr                    },              //2   
    {COLLECT_UART_BAUD_ADDR,            COLLECT_UART_BAUD_NUM,          DATA_U16,   ATTR_RW,    &gCollectParaData.collect_data.monitor_485_baud                   },              //3  
};

#define MORMAL_USER_NUM     (sizeof(NomalUserDataPoint)/sizeof(tsModbusDataPoint))

T_ModbusRtuFrame gCollectModbusData;
const tsModbusDataPoint *ModbusUserDataArr[MAX_USER_NUM] = {NomalUserDataPoint};
const uint16_t          ModbusUserDataPointNumArr[MAX_USER_NUM] = {MORMAL_USER_NUM};

bool CommCollectInit(void)
{
    bool ret;
    uint8_t  times = 0; 
    _gComCollectTxData.pData = _gComCollectTxDataBuf;       

	QueueInit(&_gCollectWriteQueue, COLLECT_WRITE_QUEUE_DEPTH, sizeof(tsCollectTxData));
	for(uint8_t i = 0; i < COLLECT_WRITE_QUEUE_DEPTH; i++)
	{
		_gCollectWriteData[i].pData = _CollectWriteDataBuf[i];
	}
    return true;
}

void GetPcsBmsInfoProc(void)
{//BMSPara_
}


void    SlaveCollectModbusCommProcess(void)
{
    static ModbusRTURevData_t *p;
    uint16_t mbrtu_state;
    uint16_t collect_recv_len;
    static uint32_t collect_comm_time_delay_cnt=0;
    switch(_CollectSlavePollingStatus)
    {
        case EN_COLLECT_POLLING_STATUS_IDLE:
        {
            //从串口队列中获取数据
            if(QueueGet(&gDevModbusCollector.rev_queue, (void *)&p) == true)
            {//对数据进行解析
                mbrtu_state=MBRTU_ParseUp(p->p_data,p->data_size,&gCollectModbusData,&collect_recv_len); 
               
                if(MBRTU_LEN_ERR==mbrtu_state)
                {//数据长度	
                    break;
                }
                if(MBRTU_OK!=mbrtu_state)
                {//需要返回异常
                    gCollectModbusData.uFuc |= 0x80;
                    if(MBRTU_FC_ERR == mbrtu_state)
                    {
                        gCollectModbusData.tDataspace.uData[0] = INVALID_FUNC;
                    }
                    else if(MBRTU_ADR_ERR == mbrtu_state)
                    {
                        gCollectModbusData.tDataspace.uData[0] = INVALID_ADDR;
                    }
                    else
                    {
                        gCollectModbusData.tDataspace.uData[0] = INVALID_DATA;
                    }
                    _gComCollectTxData.len = MBRTU_PackUp(_gComCollectTxData.pData, &gCollectModbusData);
                    _CollectSlavePollingStatus = EN_COLLECT_POLLING_STATUS_WAITING_TX;	
                    collect_comm_time_delay_cnt = tim_get_systime();
                                
                }else
                {
                    if((MBRTU_FC_READ_STANDREG == gCollectModbusData.uFuc)|| (MBRTU_FC_READ_ORREG == gCollectModbusData.uFuc) )
                    {//读取数据
                        ModbusReadParse(&gCollectModbusData);
                        _gComCollectTxData.len = MBRTU_PackUp(_gComCollectTxData.pData, &gCollectModbusData);
                        _CollectSlavePollingStatus = EN_COLLECT_POLLING_STATUS_WAITING_TX;	
                        collect_comm_time_delay_cnt = tim_get_systime(); 											
                        
                    }
                    else if( (MBRTU_FC_SWRITE_STANDREG == gCollectModbusData.uFuc)|| (MBRTU_FC_WRITE_STANDREG == gCollectModbusData.uFuc) )
                    {  //写数据
                        ModbusWriteParse(&gCollectModbusData);
                        _gComCollectTxData.len = MBRTU_PackUp(_gComCollectTxData.pData, &gCollectModbusData);
                        _CollectSlavePollingStatus = EN_COLLECT_POLLING_STATUS_WAITING_TX;	
                        collect_comm_time_delay_cnt = tim_get_systime(); 											
                    }									
                }
            }
            break;
        }
        case EN_COLLECT_POLLING_STATUS_REV:
        {
            break;
        }
        case EN_COLLECT_POLLING_STATUS_REV_OVERTIME:
        {
            break;
        }
        case EN_COLLECT_POLLING_STATUS_WAITING_TX:
        {
            if((uint32_t)(tim_get_systime() - collect_comm_time_delay_cnt) > 100)
            {
                collect_comm_time_delay_cnt = tim_get_systime();
                _CollectSlavePollingStatus = EN_COLLECT_POLLING_STATUS_INTERVAL_WAIT;	 
                ModbusCollectorRTUSend(_gComCollectTxData.pData, _gComCollectTxData.len);
            }
            break;
        }
        case EN_COLLECT_POLLING_STATUS_INTERVAL_WAIT:
        {
             if((uint32_t)(tim_get_systime() - collect_comm_time_delay_cnt) > 200)
            {
                 _CollectSlavePollingStatus = EN_COLLECT_POLLING_STATUS_IDLE;	 
            }
            break;
        }
        default:
            break;
    }
//    GetPcsBmsInfoProc();
}


void WordExchange(uint16_t *pword)
{
	uint16_t buffer;
	
	buffer = *pword;
	*pword = (buffer>>8)|(buffer<<8);
}
/*  
将数据从modbus_point 对应位置读取到 ptFrame-> tDataspace
以便 modeBus再进行转换为串口发送数组中
*/
void ModbusReadParse(T_ModbusRtuFrame *ptFrame)
{
    uint16_t start_addr;
    uint16_t reg_num;
    uint8_t buffer[256];
    uint8_t index;
    uint8_t offset;
    uint16_t i;
    uint16_t reg_addr;
    uint16_t w_buffer;

    const tsModbusDataPoint   *modbus_point;
    uint16_t modbus_point_num;

 
    if(gCollectParaData.collect_data.user_type > MAX_USER_NUM)
    {
        gCollectParaData.collect_data.user_type = NORMAL_USER;
    }
    modbus_point = ModbusUserDataArr[gCollectParaData.collect_data.user_type];
    modbus_point_num = ModbusUserDataPointNumArr[gCollectParaData.collect_data.user_type];



    start_addr = ptFrame->tDataspace.uStartAddr;
    reg_num = ptFrame->tDataspace.uRegLen;
    memset(buffer, 0, 256);

    for(i=0; i<reg_num; i++)
    {
      reg_addr = start_addr+ i;
      for(index = 0; index < modbus_point_num; index++)
      {
          if((NULL != modbus_point[index].pdata)&& ((ATTR_RO == modbus_point[index].attr_rw)||(ATTR_RW == modbus_point[index].attr_rw)))
          {
              if((reg_addr >= modbus_point[index].base_addr)&&(reg_addr < (modbus_point[index].base_addr + modbus_point[index].num)))
              {
                      offset = reg_addr - modbus_point[index].base_addr;
                      if( (DATA_U8 == modbus_point[index].data_type)|| (DATA_S8 == modbus_point[index].data_type) )
                      {
                           w_buffer = *((uint8_t *)modbus_point[index].pdata + offset);
                      }
                      else if( (DATA_U32 == modbus_point[index].data_type)|| (DATA_S32 == modbus_point[index].data_type) )
                      {
                            if((offset%2) == 0)
                            {
                                    w_buffer = *((uint16_t *)modbus_point[index].pdata + offset + 1);
                            }
                            else
                            {
                                    w_buffer = *((uint16_t *)modbus_point[index].pdata + offset - 1);
                            }
                      }
                      else if(DATA_STRING == modbus_point[index].data_type)
                      {
                            w_buffer = *((uint16_t *)modbus_point[index].pdata + offset);
                            WordExchange(&w_buffer);
                      }
                      else
                      {
                            w_buffer = *((uint16_t *)modbus_point[index].pdata + offset);
                      }
                      memmove(buffer + i*2, &w_buffer, 2);
                      break;
              }
          }
      }
    }
    DataModeChange(ptFrame->tDataspace.uData, buffer, USING_MODE_BIG, reg_num * 2);
}

uint16_t Get_Cfg_Slave(uint8_t index)
{
    uint16_t state = 0;
    
    if(index >= 20)
    {
        state = 1;
    }
    else
    {
        if(index >=1)
        {
            if(index == 9)
            {
                CollectCtrl.correct_rtc_time_en =1;
            }else
            {
                DeviceCtlWrite();
            }
            
        }
        else
        {//0 1 需要写入ID SN号
            DeviceBaseInfoWrite();
        }

           
    }


    return state;
}
/*
此函数的作用  时将MODEBUS中的数据转换到实际的对应寄存器中。
将ptFrame 中的数据转化到modbus_point 所对应的映射地址中


将从上位机接收到的数据通过解析完成之后 在写入MCU
*/
void ModbusWriteParse(T_ModbusRtuFrame *ptFrame)
{
    uint16_t start_addr;
    uint16_t reg_num;
    uint8_t index;
    uint8_t offset;
    uint16_t i;
    uint16_t reg_addr;
    uint16_t w_buffer;
    uint16_t cfg_slave = 0;
    uint8_t  write_en =0;
    start_addr = ptFrame->tDataspace.uStartAddr;
    reg_num = ptFrame->tDataspace.uRegLen;

    const tsModbusDataPoint   *modbus_point;
    uint16_t modbus_point_num;

    if(gCollectParaData.collect_data.user_type > MAX_USER_NUM)
    {
        gCollectParaData.collect_data.user_type = NORMAL_USER;
    }
    modbus_point = ModbusUserDataArr[gCollectParaData.collect_data.user_type];
    modbus_point_num = ModbusUserDataPointNumArr[gCollectParaData.collect_data.user_type];



    for(i=0; i<reg_num; i++)
    {
        reg_addr = start_addr+ i;
        for(index = 0; index < modbus_point_num; index++)
        {
            if((NULL != modbus_point[index].pdata)&&((ATTR_WO == modbus_point[index].attr_rw)||(ATTR_RW == modbus_point[index].attr_rw)))
            {
                    if((reg_addr >= modbus_point[index].base_addr)&&(reg_addr < (modbus_point[index].base_addr + modbus_point[index].num)) )
                    {
                          offset = reg_addr - modbus_point[index].base_addr;
                          w_buffer = DataToUnshort(ptFrame->tDataspace.uData + i*2, USING_MODE_BIG);
                          
                          if( (DATA_U8 == modbus_point[index].data_type)|| (DATA_S8 == modbus_point[index].data_type) )
                          {
                              *((uint8_t *)modbus_point[index].pdata + offset) = (uint8_t)w_buffer;
                          }
                          else if( (DATA_U32 == modbus_point[index].data_type)|| (DATA_S32 == modbus_point[index].data_type) )
                          {
                                  if((offset%2) == 0)
                                  {
                                          *((uint16_t *)modbus_point[index].pdata + offset + 1) = w_buffer;
                                  }
                                  else
                                  {
                                          *((uint16_t *)modbus_point[index].pdata + offset - 1) = w_buffer;
                                  }
                          }
                          else if(DATA_STRING == modbus_point[index].data_type)
                          {
                                  WordExchange(&w_buffer);
                                  *((uint16_t *)modbus_point[index].pdata + offset) = w_buffer;
                          }
                          else
                          {
                                  *((uint16_t *)modbus_point[index].pdata + offset) = w_buffer;
                          }
                          break;
                  }
            }
        }
    }
}

bool GetCollectData(uint16_t data_start_addr,uint16_t reg_len,uint8_t *p_data)
{   //读取数据地址
    //获取相对地址。
    uint16_t    start_addr = COLLECT_MODE_ID;
    uint16_t    offset =    (data_start_addr - COLLECT_MODE_ID);
    uint8_t     *p_source_data;
    if((data_start_addr < COLLECT_MODE_ID)||(p_data == NULL))
    {
        return false;
    }
    p_source_data = (uint8_t *)&gCollectParaData.collect_data.mode_id;
    p_source_data += (offset<<1);
    memcpy(p_data,p_source_data,sizeof(reg_len<<1));
    return true;
}
//pdata
bool  SetCollectData(uint8_t *pdata,uint16_t len)
{
   //256+ 9
    if((pdata == NULL)|| (len>COLLECT_WRITE_SIZE))
    {
        return false;
    }
    _gCollectWriteData[_CollectWriteDataBufIndex].len = len;
    _gCollectWriteData[_CollectWriteDataBufIndex].pData = pdata;   
    memcpy(_gCollectWriteData[_CollectWriteDataBufIndex].pData,pdata,len);
    if(QueuePut(&_gCollectWriteQueue, &_gCollectWriteData[_CollectWriteDataBufIndex]) == false) 
    {
        return false;
    }
    _CollectWriteDataBufIndex++;
    _CollectWriteDataBufIndex %= COLLECT_WRITE_QUEUE_DEPTH;
    return true;
}
uint8_t GetCorrectRtcStatus(void)
{
    return  CollectCtrl.correct_rtc_time_en;
}
void CleanCorrectRtcFlag(void)
{
    CollectCtrl.correct_rtc_time_en = 0;
}
uint16_t    GetCollectBaud()
{
    return  gCollectParaData.collect_data.monitor_485_baud;
}


/**
  * @brief  Send Modbus func04 Data.
  * @param  modbus format
  * @return NULL
  * @note   读输入寄存器
  */
void ModbusReadInput(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
	uint16_t crc = 0;
	
    p_packet[0] = slave_addr;
    p_packet[1] = MBRTU_FC_READ_ORREG;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;
    
	crc = CrcModbus(p_packet, 6);
	p_packet[6] = (crc >> 8) & 0xFF;
	p_packet[7] = crc & 0xFF;
	
    SetCollectData(p_packet, 8);
}


/**
  * @brief  Send Modbus func03 Data.
  * @param  modbus format
  * @return NULL
  * @note   读保持寄存器
  */
void ModbusReadRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
	uint16_t crc = 0;
	
    p_packet[0] = slave_addr;
    p_packet[1] = MBRTU_FC_READ_STANDREG;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;
    
   	crc = CrcModbus(p_packet, 6);
	p_packet[6] = (crc >> 8) & 0xFF;
	p_packet[7] = crc & 0xFF;
	
    SetCollectData(p_packet, 8);
}


/**
  * @brief  Send Modbus func10 Data.
  * @param  modbus format
  * @return NULL
  * @note   写多个寄存器
  */
void ModbusWriteMultiRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_value)
{
	uint16_t crc = 0;
	
    p_packet[0] = slave_addr;
    p_packet[1] = MBRTU_FC_WRITE_STANDREG;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;
    p_packet[6] = reg_num * 2;
	
	crc = CrcModbus(p_packet, 7);
	p_packet[7] = (crc >> 8) & 0xFF;
	p_packet[8] = crc & 0xFF;
	
    memcpy(&p_packet[9], p_reg_value, p_packet[6]);
    
    SetCollectData(p_packet, p_packet[6] + 9);
}


/**
  * @brief  Send Modbus func06 Data.
  * @param  modbus format
  * @return NULL
  * @note   写单个寄存器
  */
void ModbusWriteSingleRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t value)
{
	uint16_t crc = 0;
	
    p_packet[0] = slave_addr;
    p_packet[1] = MBRTU_FC_SWRITE_STANDREG;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (value >> 8) & 0xFF;
    p_packet[5] = value & 0xFF;
    
	crc = CrcModbus(p_packet, 6);
	p_packet[6] = (crc >> 8) & 0xFF;
	p_packet[7] = crc & 0xFF;
	
    SetCollectData(p_packet, 8);
}


//tsPvInputInfo   GetPvInputInfo(void)
//{
//    return  CollectDiplayInfo.pv_input_info;
//}

//tsGridOutputInfo    GetGridOutputInfo(void)
//{
//    return  CollectDiplayInfo.grid_output_info;    
//}
//tsOilInputInfo      GetOilInputInfo(void)
//{
//    return  CollectDiplayInfo.oil_input_info;    
//}
//tsTerminalLoadInfo  GetTerminalLoadInfo(void)
//{
//    return  CollectDiplayInfo.terminal_load_info;      
//}
//tsInverInfo         GetInverInfo(void)
//{
//    return  CollectDiplayInfo.inver_info;    
//}
//tsBattInfo          GetBattInfo(void)
//{
//    return  CollectDiplayInfo.batt_info;    
//}
//int16_t             GetBusVoltInfo(void)
//{
//    return  CollectDiplayInfo.dc_bus_volt_value;    
//}
//tsTempInfo          GetTempInfo(void)
//{
//    return  CollectDiplayInfo.temp_info;    
//}
//tsPcsStatusInfo     GetPcsStatusInfo(void)
//{
//    return  CollectDiplayInfo.pcs_status_info;    
//}
//tsSysSettingInfo    GetSysSettintInfo(void)
//{
//    return  CollectDiplayInfo.sys_setting_status_info;        
//}




