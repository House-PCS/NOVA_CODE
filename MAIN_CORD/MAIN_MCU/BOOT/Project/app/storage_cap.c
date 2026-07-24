#include "storage_cap.h"

#define     INIT_STORAGE_YEAR  2024
#define     CAP_STORAGE_SIZE        (sizeof(tsCapStorageInfo))

#define     MIN(A,B)  ((A)<(B)?(A):(B))

tsCapStorageInfo    CapStorgeInfo;
DevRtc_t    RtcTime;
static  DevRtc_t    OldRtcTime;
tsCollectCapInfo    FlashOldCapData;
uint8_t  WriteCapFlag = 0;

//DATA +=DATA1
void  FlashCapDataAddTranf(uint16_t data[],uint32_t data1)
{
    uint32_t    temp_data =0;
    temp_data =   data[1]<<16;
    temp_data +=  data[0];
    temp_data +=  (data1/10);
    data[1] = temp_data>>16;
    data[0] = (uint16_t)temp_data;        
}

//DATA1 + DATA2
uint32_t  ArrDataCalcFun(uint16_t data[],uint16_t data1[])
{
    uint32_t    temp_data  =0;
    uint32_t    temp_data1 =0;
    temp_data = data[1]<<16;
    temp_data +=data[0];

    temp_data1 = data1[1]<<16;
    temp_data1 +=data1[0];

    if(temp_data  > temp_data1)
    {
        return (temp_data - temp_data1);
    }
    else
    {
        return (temp_data1 - temp_data);        
    } 
}

uint32_t ArrDataCalcFun1(uint16_t data[],uint32_t data1)
{
    uint32_t  temp_data;
    temp_data = data[1]<<16;
    temp_data += data[0];
    temp_data += data1;
    return temp_data;
}



void  SetWirteCapEN(void)
{
    WriteCapFlag = 1;
}

//uint8_t  cap_test_data[256];

bool    GetDayCapFlashAddr(uint32_t *paddr,DevRtc_t rtc_time)
{
    uint32_t        CapBaseAddr= CAP_STORAGE_BASE_ADDR;
    bool            get_status;
    uint32_t        cap_cal_temp= 0;    
    if((rtc_time.year < INIT_STORAGE_YEAR)||(paddr ==NULL))
    {//时间小于初始时间
        get_status = false;//return ;
    }else
    {
        cap_cal_temp = (rtc_time.year - INIT_STORAGE_YEAR);
        cap_cal_temp <<=13;
        cap_cal_temp *=12;
        *paddr = (cap_cal_temp + CapBaseAddr); 


        cap_cal_temp = (rtc_time.month - 1);
        cap_cal_temp <<=13;
        *paddr += cap_cal_temp;

        cap_cal_temp = (rtc_time.day - 1);
        cap_cal_temp <<=8;
        *paddr += cap_cal_temp;
        get_status = true;
    }
    return  get_status;
}

bool CapacityStorageFun(tsCapStorageInfo    cap_diff)
{
//根据时间来确认flash存储地址   base 地址-->0x258000
//两次时间超过4小时需要存储一次   //或者按键触发  总共最多存储8次
    bool                storage_status;
    uint32_t            CapBaseAddr= CAP_STORAGE_BASE_ADDR;
    uint8_t             i = 0;
    tsCapStorageInfo    cap_storge_info;
    uint8_t             read_flash_buf[32];
    uint8_t             write_flash_buf[32];
    uint32_t            CurCapStorageAddr = 0;
    cap_storge_info =   cap_diff;
//RtcTime

    if(GetDayCapFlashAddr(&CurCapStorageAddr,RtcTime) == false)
    {   
        storage_status = false;
        return  storage_status;
    }
    for(i = 0;i<8;i++)
    {//每天最多存储8个位置    
        //读取地址是否为0A 0A 0X0A 
        SpiFlashReadData((CurCapStorageAddr+i*CAP_STORAGE_SIZE),read_flash_buf, 32);    
        if((read_flash_buf[1] == 0X0A) &&(read_flash_buf[2] == 0X0A)&&(read_flash_buf[3] == 0X0A))  
        {
            
        }
        else
       {
            if((read_flash_buf[1] == 0XFF) &&(read_flash_buf[2] == 0XFF)&&(read_flash_buf[3] == 0XFF)&&(read_flash_buf[4] == 0XFF))  
            {
                break;
            }
        }
    }
    
    if( i == 7)
    {//表示已经存储满了
     //
         storage_status = false;
    }
    else
    {//根据 i决定存储位置
        cap_storge_info.load_consum_cap_diff    = MIN(cap_storge_info.load_consum_cap_diff,CAP_MAX_DAY);  //取最小值
        cap_storge_info.purchase_cap_diff       = MIN(cap_storge_info.purchase_cap_diff,CAP_MAX_DAY);
        cap_storge_info.selling_cap_diff        = MIN(cap_storge_info.selling_cap_diff,CAP_MAX_DAY);

        memcpy(write_flash_buf,(uint8_t *)(&cap_storge_info.cap_storge_id),CAP_STORAGE_SIZE);
        SpiFlashWriteBytesWithoutErase((CurCapStorageAddr + i*CAP_STORAGE_SIZE + 4),write_flash_buf+4,CAP_STORAGE_SIZE-4);
        SpiFlashWriteBytesWithoutErase((CurCapStorageAddr + i*CAP_STORAGE_SIZE ),write_flash_buf,4);
         storage_status = true; 
    }    
//    SpiFlashReadData(CurCapStorageAddr,cap_test_data, 256); 

    return storage_status;
}

void  GetHistoryCapInfoFromFlash(DevRtc_t rtc_temp)
{
//    gCollectParaData.collect_data.collect_cap_info.load_cost_year
//当天的数据，需要先根据时钟信息获取存储地址  开机读取一次 之后就是每次存储需要增加

    uint32_t    cur_year_addr = 0;
    uint32_t    cur_month_addr = 0;
    uint32_t    cur_day_cap_addr =0;
    uint32_t    temp_addr = 0; 
    uint8_t     read_flash_buf[32];
    DevRtc_t    rtc_time_temp;
    rtc_time_temp = rtc_temp;//RtcTime;
    rtc_time_temp.month = 1;
    rtc_time_temp.day = 1;
    rtc_time_temp.hour = 0;
    rtc_time_temp.minute = 0;
    rtc_time_temp.second = 0;
    GetDayCapFlashAddr(&cur_year_addr,rtc_time_temp);//获取当年1月1号的地址

    uint8_t     i =0;
    uint8_t     j =0;
    uint8_t     k =0;
    uint16_t    y =0;


    uint32_t  temp_data;
    tsCapStorageInfo    cap_storge_info;

//当天的地址
//    cur_day_cap_addr += (cur_year_addr + (RtcTime.month-1)*0x2000+(RtcTime.day-1)*256);
    cur_day_cap_addr += (cur_year_addr + ((RtcTime.month-1)<<13)+((RtcTime.day-1)<<8));
///RtcTime.day 当天的    
    for(k=0;k<8;k++)
    {
        temp_addr = cur_day_cap_addr+(k<<5);
        SpiFlashReadData(temp_addr,read_flash_buf, CAP_STORAGE_SIZE);
        if((read_flash_buf[0]==0x0A)&&(read_flash_buf[1]==0x0A)&&(read_flash_buf[0]==0x0A)&&(read_flash_buf[1]==0x0A))
        {
            memcpy((uint8_t *)(&cap_storge_info.cap_storge_id),read_flash_buf,CAP_STORAGE_SIZE);
            gCollectParaData.collect_data.collect_cap_info.load_cost_day += cap_storge_info.load_consum_cap_diff;
            gCollectParaData.collect_data.collect_cap_info.electic_purchase_day += cap_storge_info.purchase_cap_diff;
            gCollectParaData.collect_data.collect_cap_info.electic_sale_day   +=  cap_storge_info.selling_cap_diff;
        }
    }
//RtcTime.month 先算当月地址
//    cur_month_addr = (cur_year_addr + (RtcTime.month-1)*0x2000);
    for(j=1;j<=RtcTime.day;j++)
    {

        for(k =0;k<8;k++)
        {
            temp_addr = cur_month_addr+((j-1)<<8)+(k<<5);
            SpiFlashReadData(temp_addr,read_flash_buf, CAP_STORAGE_SIZE);
            if((read_flash_buf[0]==0x0A)&&(read_flash_buf[1]==0x0A)&&(read_flash_buf[0]==0x0A)&&(read_flash_buf[1]==0x0A))
            {
                memcpy((uint8_t *)(&cap_storge_info.cap_storge_id),read_flash_buf,CAP_STORAGE_SIZE);
                FlashCapDataAddTranf(gCollectParaData.collect_data.collect_cap_info.load_cost_month,cap_storge_info.load_consum_cap_diff);
                FlashCapDataAddTranf(gCollectParaData.collect_data.collect_cap_info.electic_purchase_month,cap_storge_info.purchase_cap_diff);
                FlashCapDataAddTranf(gCollectParaData.collect_data.collect_cap_info.electic_sale_month,cap_storge_info.selling_cap_diff);
            }                
        }
    }
//先算当年地址
    for(i=1;i<=RtcTime.month;i++)
    {
        for(j=1;j<=RtcTime.day;j++)
        {
            for(k=0;k<8;k++)
            {
//                temp_addr = (cur_year_addr+i*32*256+j*256)+(k<<5);

                temp_addr = cur_year_addr +((i-1)<<13) + ((j-1)<<8) +(k<<5);
                SpiFlashReadData(temp_addr,read_flash_buf, 32);
                if((read_flash_buf[0]==0x0A)&&(read_flash_buf[1]==0x0A)&&(read_flash_buf[0]==0x0A)&&(read_flash_buf[1]==0x0A))
                {
                    //开始计算当年
                    //需要进行判断 是否有异常值。
                    memcpy((uint8_t *)(&cap_storge_info.cap_storge_id),read_flash_buf,CAP_STORAGE_SIZE);                
                    FlashCapDataAddTranf(gCollectParaData.collect_data.collect_cap_info.load_cost_year,cap_storge_info.load_consum_cap_diff);
                    FlashCapDataAddTranf(gCollectParaData.collect_data.collect_cap_info.electic_purchase_year,cap_storge_info.purchase_cap_diff);
                    FlashCapDataAddTranf(gCollectParaData.collect_data.collect_cap_info.electic_sale_year,cap_storge_info.selling_cap_diff);

                }
            }
            
        }
    }

//以上已经算出来当年的用电量//  后面需要把每一年的用电量都累加出来 就能算出当年用电量。
//需要读取之前所有年度的用电量
//基础地址开始计算
    gCollectParaData.collect_data.collect_cap_info.electic_purchase_total[0] =  gCollectParaData.collect_data.collect_cap_info.electic_purchase_year[0];
    gCollectParaData.collect_data.collect_cap_info.electic_purchase_total[1] =  gCollectParaData.collect_data.collect_cap_info.electic_purchase_year[1];  
    gCollectParaData.collect_data.collect_cap_info.electic_sale_total[0] =  gCollectParaData.collect_data.collect_cap_info.electic_sale_year[0];
    gCollectParaData.collect_data.collect_cap_info.electic_sale_total[1] =  gCollectParaData.collect_data.collect_cap_info.electic_sale_year[1];  
    gCollectParaData.collect_data.collect_cap_info.load_cost_total[0] =  gCollectParaData.collect_data.collect_cap_info.load_cost_year[0];
    gCollectParaData.collect_data.collect_cap_info.load_cost_total[1] =  gCollectParaData.collect_data.collect_cap_info.load_cost_year[1];


    for(y=1;y<=(RtcTime.year-2024);y++)   //RtcTime.year 从2024年 到 RtcTime.year前一年所有数据  + RtcTime.year当年的数据
    {
        for(i=1;i<=RtcTime.month;i++)
        {
            for(j=1;j<=RtcTime.day;j++)
            {
                for(k=0;k<8;k++)
                {
                    temp_addr = CAP_STORAGE_BASE_ADDR+(y<<14)*6 +((i-1)<<13) + ((j-1)<<8) +(k<<5);
                    SpiFlashReadData(temp_addr,read_flash_buf, 32);
                    if((read_flash_buf[0]==0x0A)&&(read_flash_buf[1]==0x0A)&&(read_flash_buf[0]==0x0A)&&(read_flash_buf[1]==0x0A))
                    {
                        //开始计算当年
                        //需要进行判断 是否有异常值。
                        memcpy((uint8_t *)(&CapStorgeInfo.cap_storge_id),read_flash_buf,CAP_STORAGE_SIZE);                      
                        FlashCapDataAddTranf(gCollectParaData.collect_data.collect_cap_info.load_cost_total,cap_storge_info.load_consum_cap_diff);
                        FlashCapDataAddTranf(gCollectParaData.collect_data.collect_cap_info.electic_purchase_total,cap_storge_info.purchase_cap_diff);
                        FlashCapDataAddTranf(gCollectParaData.collect_data.collect_cap_info.electic_sale_total,cap_storge_info.selling_cap_diff);

                    }
                }
                
            }
        }

    }
    memcpy((uint8_t *)&FlashOldCapData,(uint8_t *)&gCollectParaData.collect_data.collect_cap_info,sizeof(tsCollectCapInfo));
}

void    GetDiffCapFun(void)
{
    uint32_t    temp_data;
    CapStorgeInfo.cap_storge_id = 0X0A0A0A0A;
    CapStorgeInfo.purchase_cap_diff  = ArrDataCalcFun(FlashOldCapData.electic_purchase_total,gCollectParaData.pcs_input_back_data.elec_purchase_consum_total);
    CapStorgeInfo.load_consum_cap_diff = ArrDataCalcFun(FlashOldCapData.load_cost_total,gCollectParaData.pcs_input_back_data.load_total_sonsum_power);
    CapStorgeInfo.selling_cap_diff = ArrDataCalcFun(FlashOldCapData.electic_sale_total,gCollectParaData.pcs_input_back_data.elec_sale_consum_total);
//获取到 PCS 和 监控卡存储的差值===

//限幅处理          
    CapStorgeInfo.load_consum_cap_diff    = MIN(CapStorgeInfo.load_consum_cap_diff,CAP_MAX_DAY);  //取最小值
    CapStorgeInfo.purchase_cap_diff       = MIN(CapStorgeInfo.purchase_cap_diff,CAP_MAX_DAY);
    CapStorgeInfo.selling_cap_diff        = MIN(CapStorgeInfo.selling_cap_diff,CAP_MAX_DAY);




    temp_data = ArrDataCalcFun1(FlashOldCapData.electic_purchase_month,CapStorgeInfo.purchase_cap_diff);
    gCollectParaData.collect_data.collect_cap_info.electic_purchase_month[0] = (uint16_t)temp_data;
    gCollectParaData.collect_data.collect_cap_info.electic_purchase_month[1] =(uint16_t)temp_data>>16; 
    temp_data = ArrDataCalcFun1(FlashOldCapData.electic_purchase_year,CapStorgeInfo.purchase_cap_diff);
    gCollectParaData.collect_data.collect_cap_info.electic_purchase_year[0] = (uint16_t)temp_data;
    gCollectParaData.collect_data.collect_cap_info.electic_purchase_year[1] =(uint16_t)temp_data>>16; 
    gCollectParaData.collect_data.collect_cap_info.electic_purchase_day  = FlashOldCapData.electic_purchase_day + CapStorgeInfo.purchase_cap_diff;



    temp_data = ArrDataCalcFun1(FlashOldCapData.load_cost_month,CapStorgeInfo.load_consum_cap_diff);
    gCollectParaData.collect_data.collect_cap_info.load_cost_month[0] = (uint16_t)temp_data;
    gCollectParaData.collect_data.collect_cap_info.load_cost_month[1] =(uint16_t)temp_data>>16; 
    temp_data = ArrDataCalcFun1(FlashOldCapData.load_cost_year,CapStorgeInfo.load_consum_cap_diff);
    gCollectParaData.collect_data.collect_cap_info.load_cost_year[0] = (uint16_t)temp_data;
    gCollectParaData.collect_data.collect_cap_info.load_cost_year[1] =(uint16_t)temp_data>>16; 
    gCollectParaData.collect_data.collect_cap_info.load_cost_day  = FlashOldCapData.load_cost_day + CapStorgeInfo.load_consum_cap_diff;



    temp_data = ArrDataCalcFun1(FlashOldCapData.electic_sale_month,CapStorgeInfo.selling_cap_diff);
    gCollectParaData.collect_data.collect_cap_info.electic_sale_month[0] = (uint16_t)temp_data;
    gCollectParaData.collect_data.collect_cap_info.electic_sale_month[1] =(uint16_t)temp_data>>16; 
    temp_data = ArrDataCalcFun1(FlashOldCapData.electic_sale_year,CapStorgeInfo.selling_cap_diff);
    gCollectParaData.collect_data.collect_cap_info.electic_sale_year[0] = (uint16_t)temp_data;
    gCollectParaData.collect_data.collect_cap_info.electic_sale_year[1] =(uint16_t)temp_data>>16; 
    gCollectParaData.collect_data.collect_cap_info.electic_sale_day  = FlashOldCapData.electic_sale_day + CapStorgeInfo.selling_cap_diff;    
}


void    CapInitl(void)
{
    uint32_t  temp_data;
    uint32_t  temp_data1;
    DevRtcReadTime(&RtcTime);
    OldRtcTime = RtcTime;
    GetHistoryCapInfoFromFlash(RtcTime);   //上电从FLASH 里边获取容量信息
                                    //获取到总，当年 当月 当天的用电量
    GetDiffCapFun();                //确保能读取到数据才可以    
}
void    CapCtrl(void)
{
    static  uint32_t  old_rtc_stamp;
    RtcTime.time_stamp =GetRtcTimeStamp();
//获取到时间戳然后转为时间
    RtcSecConvert2Date(&RtcTime);
    if(old_rtc_stamp == RtcTime.time_stamp)
    {
        return;
    }
    old_rtc_stamp = RtcTime.time_stamp;
//每隔1S进来一次



    if((OldRtcTime.day != RtcTime.day)||(OldRtcTime.month!=RtcTime.month)||(OldRtcTime.year!=RtcTime.year))
    {//
        OldRtcTime = RtcTime;
//日期切换 置位 test_flag =1
    }
    else 
    {
        if((RtcTime.hour - OldRtcTime.hour)>4)
        {
//置位 test_flag =1            
        }
    }
    if(WriteCapFlag ==1)  
    {
        WriteCapFlag = 0;
        OldRtcTime = RtcTime;
        if(CapacityStorageFun(CapStorgeInfo) == true)
        {
            GetHistoryCapInfoFromFlash(RtcTime);   //读取 更新历史用电量信息
        }
        GetDiffCapFun();
    }
    else
    {//如果PCS的总的用电量没有改变则不需要更新
        GetDiffCapFun();         
    }    
}