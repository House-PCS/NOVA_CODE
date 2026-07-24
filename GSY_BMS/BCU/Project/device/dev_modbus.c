/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  
#include "dev_modbus.h"
#include "data_process.h"



/** 
* @brief  MODBUS协议从机消息处理
* @param  ModbusType：协议类型
* @param  ModbusSlaveInfo：指向单个协议集
* @param  RxBuff：接收数据缓存
* @param  len：接收到的字节数
* @param  TxBuff：发送数据缓存 
* @return  uint16_t：待发送字节数
* @note:
*
*/
uint16_t ModbusSlaveProtProce(MODBUS_MESSAGE_TYPE ModbusType,const MODBUS_SLAVE_CFG *const ModbusSlaveInfo,uint8_t *const RxBuff,uint16_t len,uint8_t *const TxBuff){
	uint16_t MessageStart=0,SendNum=0,CRC_CalculateVal=0,CRC_DataVal=0,RegadStart=0,DataNum=0,RegadEnd=0;
	uint8_t i=0,j=0,DataStart=0;
	uint16_t TempU16=0,RelativeRegadStart=0;
	bool FuncErr=true;
	if(ModbusType==MODBUS_TCP){
		if(len<(MODBUS_DATA_NUM_MIN+MODBUS_TCP_FH)){		//短报文非MODBUS报文
			return 0;
		}
		MessageStart=MODBUS_TCP_FH;
		memcpy(TxBuff,RxBuff,MODBUS_TCP_FH);
		SendNum=MODBUS_TCP_FH;
	}else {
		if(len<MODBUS_DATA_NUM_MIN){		//短报文非MODBUS报文
			return 0;
		}
	}
	
	if(RxBuff[MessageStart]== ModbusSlaveInfo->DeviceAddr){		//校验设备地址
		if(ModbusType==MODBUS_RTU){			//CRC
            CRC_CalculateVal=Crc16_Modbus(RxBuff,len-2);
			CRC_DataVal=((uint16_t)RxBuff[len-1]<<8)+RxBuff[len-2];
			if(CRC_CalculateVal!=CRC_DataVal){
				return 0;
			}
		}
		memcpy(&TxBuff[SendNum],&RxBuff[MessageStart],2);
		SendNum+=2;
		if(RxBuff[MessageStart+1]==0x01 ||RxBuff[MessageStart+1]==0x02 ||RxBuff[MessageStart+1]==0x03 ||RxBuff[MessageStart+1]==0x04){
//			if(ModbusSlaveInfo->ReadMessageNum >0){
				for(i=0;i<ModbusSlaveInfo->ReadMessageNum;i++){
					if(RxBuff[MessageStart+1]==ModbusSlaveInfo->ReadCfgInfo[i].Func){
						FuncErr=false;
						RegadStart=((uint16_t)RxBuff[MessageStart+2]<<8)+RxBuff[MessageStart+3];
						DataNum=((uint16_t)RxBuff[MessageStart+4]<<8)+RxBuff[MessageStart+5];
						RegadEnd=DataNum+RegadStart;
						if(ModbusSlaveInfo->ReadCfgInfo[i].RegisterOffset!=0){
							TempU16=ModbusSlaveInfo->ReadCfgInfo[i].OffsetCount-1;
						}else {
							TempU16=0;
						}
						if((RegadStart>=ModbusSlaveInfo->ReadCfgInfo[i].RegadStart)&&(RegadEnd<=(ModbusSlaveInfo->ReadCfgInfo[i].RegadStart+	\
									ModbusSlaveInfo->ReadCfgInfo[i].RegisterOffset*TempU16+ModbusSlaveInfo->ReadCfgInfo[i].DataNum))){
							if(ModbusSlaveInfo->ReadCfgInfo[i].RegisterOffset>0){		//需要进行地址偏移
								TempU16=(RegadStart-ModbusSlaveInfo->ReadCfgInfo[i].RegadStart)/ModbusSlaveInfo->ReadCfgInfo[i].RegisterOffset;
								RelativeRegadStart=(RegadStart-ModbusSlaveInfo->ReadCfgInfo[i].RegadStart)%ModbusSlaveInfo->ReadCfgInfo[i].RegisterOffset;
								if(RegadEnd>(ModbusSlaveInfo->ReadCfgInfo[i].RegadStart+TempU16*ModbusSlaveInfo->ReadCfgInfo[i].RegisterOffset+ModbusSlaveInfo->ReadCfgInfo[i].DataNum)){
									TxBuff[SendNum-1]|=0x80;
									TxBuff[SendNum++]=02;			//数据地址错误		txbuff[2]=02;
									break;
								}
							}else {		//不需要地址偏移
								RelativeRegadStart=RegadStart-ModbusSlaveInfo->ReadCfgInfo[i].RegadStart;
								TempU16=0;
							}
							if(ModbusSlaveInfo->ReadCfgInfo[i].ReadDataProcess!=0){
								ModbusSlaveInfo->ReadCfgInfo[i].ReadDataProcess(TempU16);		
							}
							TxBuff[SendNum++]=DataNum<<1;
							for(j=0;j<DataNum;j++){
								TempU16=ModbusSlaveInfo->ReadCfgInfo[i].TxBuff[RelativeRegadStart+j];
								TxBuff[SendNum++]=(uint8_t)(TempU16>>8);
								TxBuff[SendNum++]=(uint8_t)(TempU16>>0);
							}
							break;
						}
						
					}
				}
				if(i>=ModbusSlaveInfo->ReadMessageNum){			////功能码错误或寄存器地址错误
					if(FuncErr){	//功能码错误
						TxBuff[SendNum-1]|=0x80;
						TxBuff[SendNum++]=01;
					}else {	//寄存器地址错误
						TxBuff[SendNum-1]|=0x80;
						TxBuff[SendNum++]=02;
					}
				}
//			}
//			else {		//功能码错误		txbuff[2]=01;
//				TxBuff[SendNum-1]|=0x80;
//				TxBuff[SendNum++]=01;
//			}
		}
		else if(RxBuff[MessageStart+1]==0x05 ||RxBuff[MessageStart+1]==0x06 ||RxBuff[MessageStart+1]==0x0F||RxBuff[MessageStart+1]==0x10){
            for(i=0;i<ModbusSlaveInfo->WriteMessageNum;i++){
                if(RxBuff[MessageStart+1]==ModbusSlaveInfo->WriteCfgInfo[i].Func){
                    FuncErr=false;
                    RegadStart=((uint16_t)RxBuff[MessageStart+2]<<8)+RxBuff[MessageStart+3];
                    if(RxBuff[MessageStart+1]==0x05 ||RxBuff[MessageStart+1]==0x06)
                    {
                        DataNum=1;
                        DataStart=MessageStart+4;
                    }
                    else
                    {
                        DataNum=((uint16_t)RxBuff[MessageStart+4]<<8)+RxBuff[MessageStart+5];  
                        DataStart=MessageStart+7;
                    }
                    RegadEnd=DataNum+RegadStart;
                    if(ModbusSlaveInfo->WriteCfgInfo[i].OffsetCount!=0){
                        TempU16=ModbusSlaveInfo->WriteCfgInfo[i].OffsetCount-1;
                    }else {
                        TempU16=0;
                    }
                    if((RegadStart>=ModbusSlaveInfo->WriteCfgInfo[i].RegadStart)&&(RegadEnd<=(ModbusSlaveInfo->WriteCfgInfo[i].RegadStart+	\
                                ModbusSlaveInfo->WriteCfgInfo[i].RegisterOffset*TempU16+ModbusSlaveInfo->WriteCfgInfo[i].DataNum))){
                        if(ModbusSlaveInfo->WriteCfgInfo[i].RegisterOffset>0){		//需要进行地址偏移
                            TempU16=(RegadStart-ModbusSlaveInfo->WriteCfgInfo[i].RegadStart)/ModbusSlaveInfo->WriteCfgInfo[i].RegisterOffset;
                            RelativeRegadStart=(RegadStart-ModbusSlaveInfo->WriteCfgInfo[i].RegadStart)%ModbusSlaveInfo->WriteCfgInfo[i].RegisterOffset;
                            if(RegadEnd>(ModbusSlaveInfo->WriteCfgInfo[i].RegadStart+TempU16*ModbusSlaveInfo->WriteCfgInfo[i].RegisterOffset+ModbusSlaveInfo->WriteCfgInfo[i].DataNum)){
                                TxBuff[SendNum-1]|=0x80;
                                TxBuff[SendNum++]=02;			//数据地址错误		txbuff[2]=02;
                                break;
                            }
                        }else {		//不需要地址偏移
                            RelativeRegadStart=RegadStart-ModbusSlaveInfo->WriteCfgInfo[i].RegadStart;
                            TempU16=0;
                        }
                        for(j=0;j<DataNum;j++){
                            TempU16=(RxBuff[DataStart++]<<8);
                            TempU16|=(RxBuff[DataStart++]);
                            ModbusSlaveInfo->WriteCfgInfo[i].RxBuff[j]=TempU16;
                        }
                        
                        if(ModbusSlaveInfo->WriteCfgInfo[i].WriteDataProcess!=0){
                            ModbusSlaveInfo->WriteCfgInfo[i].WriteDataProcess(TempU16,RelativeRegadStart,DataNum);		
                        }
                        memcpy(&TxBuff[SendNum],&RxBuff[MessageStart+2],4);
                        SendNum+=4;
                        break;
                    }
						
                }
            }
            if(i>=ModbusSlaveInfo->WriteMessageNum){			////功能码错误或寄存器地址错误
                if(FuncErr){	//功能码错误
                    TxBuff[SendNum-1]|=0x80;
                    TxBuff[SendNum++]=01;
                }else {	//寄存器地址错误
                    TxBuff[SendNum-1]|=0x80;
                    TxBuff[SendNum++]=02;
                }
            }
		}
		else	{		//功能码错误	
			TxBuff[SendNum-1]|=0x80;
			TxBuff[SendNum++]=01;
		}
		if(ModbusType==MODBUS_RTU){			//CRC
            TempU16=Crc16_Modbus(TxBuff,SendNum);
			TxBuff[SendNum++]=(uint8_t)(TempU16>>0);
			TxBuff[SendNum++]=(uint8_t)(TempU16>>8);
		}
		return SendNum;
	}
	return 0;
}

