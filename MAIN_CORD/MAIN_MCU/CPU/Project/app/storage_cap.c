#include "storage_cap.h"
#include "system_ctrl.h"
#include "firmware_upgrate.h"
#include "slave_pcs.h"
#include "modbus_crc.h"
#include "slave_bms.h"

#define  CAP_DAY_SIZE           0x100
#define  CAP_DEBUG_TEST         0
#define  HALF_AN_HOUR_CAP       800ul

#if CAP_DEBUG_TEST
uint32_t pv_sum = 0;
uint32_t buy_sum = 0;
uint32_t sell_sum = 0;
uint32_t eps_sum = 0;
uint32_t batt_chg_sum = 0;
uint32_t batt_dsg_sum = 0;
uint32_t remote_sum = 0;
uint32_t gen_elec_sum = 0;
uint32_t ev_chg_sum = 0;
uint32_t ev_dsg_sum = 0;
#endif

CapStorageInfo_t    gstCapStorgeInfo;
DevRtc_t            gstRtcTime;
static  DevRtc_t    _gstOldRtcTime;
uint8_t             gWriteCapFlag = 0;
uint8_t             gDataTimeSwitchFlag = 0;
uint8_t             gOldRtcUpdtaFlag = 0;
CapStorageInfo_t    gstStartTodayCaPStorgeInfo;
CapStorageInfo_t    gstStartMonthCaPStorgeInfo;
CapStorageInfo_t    gstStartYearCaPStorgeInfo;
CapStorageInfo_t    gstTodayCaPStorgeInfo;
CapStorageInfo_t    gstTmpCapStorgeInfo;
CapFromPcsValue_t   gstTmpCapFromPcsInfo;
CapFromPcsValue_t   gstCurrentCapPcsInfo;
uint8_t             erase_test = 0;
uint8_t             gChangeFlag = 0;
uint8_t             gTodayValueExisFlag = 0;
uint8_t             gCleanPcsCapFlag = 0;
uint8_t             gRefreshCapFlag = 0;
uint8_t             gArrReadBuf[4096];

void SetWirteCapEN(void)
{
    gWriteCapFlag = 1;
}
void UpdataFlashDataEn(void)
{
    gOldRtcUpdtaFlag = 1;
}


bool GetDayCapFlashAddr(uint32_t *paddr, DevRtc_t rtc_time)
{
    uint32_t        cap_base_addr = 0;
    bool            get_status;
    uint32_t        cap_cal_temp = 0;
    cap_base_addr = CAP_STORAGE_BASE_ADDR;
    CollectCapInfo_t ttt;

    if((rtc_time.year < INIT_STORAGE_YEAR) || (rtc_time.year >= END_STORAGE_YEAR) || (paddr == NULL))
    {
        //时间小于初始时间
        get_status = false;//return ;
    }
    else
    {
        cap_cal_temp = (rtc_time.year - INIT_STORAGE_YEAR);
        cap_cal_temp <<= 13;
        cap_cal_temp *= 12;
        *paddr = (cap_cal_temp + cap_base_addr);


        cap_cal_temp = (rtc_time.month - 1);
        cap_cal_temp <<= 13;
        *paddr += cap_cal_temp;

        cap_cal_temp = (rtc_time.day - 1);
        cap_cal_temp <<= 8;
        *paddr += cap_cal_temp;
        get_status = true;
    }

    return  get_status;
}

//清除pcs历史值
void CleanPcsCapValue(void)
{
    //存储结束后发送清空指令
    SetPcsCapInfoToZero(1);  //需确认下发的寄存器
}



//存储23：30~00:00产生的值
void CapacityStorageOldDayFun(CapStorageInfo_t cap_sum)
{
//    uint8_t  gArrReadBuf[4096];
    uint32_t old_cap_storage_addr = 0;
    uint32_t cur_sector_start_addr = 0;
    uint32_t cur_sector_shift_cnt = 0;
    CapStorageInfo_t    st_cap_storge_info;
    uint8_t arr_read_flash_buf[CAP_STORAGE_SIZE];
    uint8_t arr_write_flash_buf[CAP_STORAGE_SIZE];
    uint16_t crc_data = 0;

    memset(arr_read_flash_buf, 0, sizeof(arr_read_flash_buf));
    memset(arr_write_flash_buf, 0, sizeof(arr_write_flash_buf));
    memset(gArrReadBuf, 0, 4096);

    if(GetDayCapFlashAddr(&old_cap_storage_addr, _gstOldRtcTime) == false)
    {
        return;
    }
    st_cap_storge_info = cap_sum;

    SpiFlashReadData((old_cap_storage_addr), arr_read_flash_buf, CAP_STORAGE_SIZE);
    if((arr_read_flash_buf[0] == 0X0A) && (arr_read_flash_buf[1] == 0X0A) && (arr_read_flash_buf[2] == 0X0A) && (arr_read_flash_buf[3] == 0X0A))
    {
        memcpy(arr_write_flash_buf, (uint8_t *)(&st_cap_storge_info.cap_storge_id), CAP_STORAGE_SIZE);
        //进行CRC校验
        crc_data = CrcModbus((uint8_t*)(&st_cap_storge_info.cap_storge_id), CAP_STORAGE_SIZE);
        cur_sector_start_addr = old_cap_storage_addr & 0xfffff000;
        cur_sector_shift_cnt = old_cap_storage_addr - cur_sector_start_addr;
        SpiFlashReadData(cur_sector_start_addr, gArrReadBuf, 4096);
        SpiFlashAutoEraseSectors(cur_sector_start_addr, 0x1000); //擦除4K
        SpiFlashWriteBytesWithoutErase(cur_sector_start_addr, gArrReadBuf, cur_sector_shift_cnt); //将read_buf[0] ---read_buf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase(old_cap_storage_addr + 256, &gArrReadBuf[cur_sector_shift_cnt + 256], (0x1000 - cur_sector_shift_cnt - 256)); //将gArrReadBuf[0] ---gArrReadBuf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase((old_cap_storage_addr + 4), arr_write_flash_buf + 4, CAP_STORAGE_SIZE - 4);
        SpiFlashWriteBytesWithoutErase((old_cap_storage_addr), arr_write_flash_buf, 4);
    }

    /*读数据*/
    SpiFlashReadData(old_cap_storage_addr, arr_read_flash_buf, CAP_STORAGE_SIZE);

    /*进行crc数据校验*/
    if(crc_data == CrcModbus(arr_read_flash_buf, CAP_STORAGE_SIZE))
    {

    }
    else
    {
        memcpy(arr_write_flash_buf, (uint8_t *)(&st_cap_storge_info.cap_storge_id), CAP_STORAGE_SIZE);
        cur_sector_start_addr = old_cap_storage_addr & 0xfffff000;
        cur_sector_shift_cnt = old_cap_storage_addr - cur_sector_start_addr;
        SpiFlashReadData(cur_sector_start_addr, gArrReadBuf, 4096);
        SpiFlashAutoEraseSectors(cur_sector_start_addr, 0x1000); //擦除4K
        SpiFlashWriteBytesWithoutErase(cur_sector_start_addr, gArrReadBuf, cur_sector_shift_cnt); //将read_buf[0] ---read_buf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase(old_cap_storage_addr + 256, &gArrReadBuf[cur_sector_shift_cnt + 256], (0x1000 - cur_sector_shift_cnt - 256)); //将gArrReadBuf[0] ---gArrReadBuf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase((old_cap_storage_addr + 4), arr_write_flash_buf + 4, CAP_STORAGE_SIZE - 4);
        SpiFlashWriteBytesWithoutErase((old_cap_storage_addr), arr_write_flash_buf, 4);
    }
}


bool CapacityStorageFun(CapStorageInfo_t cap_sum)
{
//根据时间来确认flash存储地址   base 地址-->0x258000
//两次时间超过30min存储一次
//    uint8_t             gArrReadBuf[4096];
    uint32_t            flash_cap_cur_day_addr = 0;
    uint32_t            cur_sector_shift_cnt = 0;
    uint32_t            cur_sector_start_addr = 0;
    bool                storage_status = true;
    uint8_t             i = 0;
    CapStorageInfo_t    st_cap_storge_info;
    uint8_t             arr_read_flash_buf[CAP_STORAGE_SIZE];
    uint8_t             arr_write_flash_buf[CAP_STORAGE_SIZE];
    uint32_t            cur_cap_storage_addr = 0;
    st_cap_storge_info =   cap_sum;
    uint8_t today_first_write_flag = 0;
    uint16_t            crc_data = 0;

    memset(arr_read_flash_buf, 0, sizeof(arr_read_flash_buf));
    memset(arr_write_flash_buf, 0, sizeof(arr_write_flash_buf));
    memset(gArrReadBuf, 0, 4096);
//gstRtcTime
    if(GetDayCapFlashAddr(&cur_cap_storage_addr, gstRtcTime) == false)
    {
        storage_status = false;
        return  storage_status;
    }
    if(gDataTimeSwitchFlag == 1)
    {
        gDataTimeSwitchFlag = 0;
//需要判断此时是否是第一次（日期切换）存储，如果是第一次存储，需要读取当前SECTOR数据，将此sector 清空，将之前的数据再回写回去。

        GetDayCapFlashAddr(&flash_cap_cur_day_addr, gstRtcTime); //获取当天的地址
        cur_sector_start_addr = flash_cap_cur_day_addr & 0xfffff000;
        cur_sector_shift_cnt = flash_cap_cur_day_addr - cur_sector_start_addr;
        SpiFlashReadData(cur_sector_start_addr, gArrReadBuf, 4096);
        SpiFlashAutoEraseSectors(cur_sector_start_addr, 0x1000); //擦除4K
        SpiFlashWriteBytesWithoutErase(cur_sector_start_addr, gArrReadBuf, cur_sector_shift_cnt); //将read_buf[0] ---read_buf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase(flash_cap_cur_day_addr + 256, &gArrReadBuf[cur_sector_shift_cnt + 256], (0x1000 - cur_sector_shift_cnt - 256)); //将gArrReadBuf[0] ---gArrReadBuf[cur_sector_shift_addr]  //写入

//        SpiFlashReadData(cur_sector_start_addr,gArrReadBuf, 4096);  ///测试是否成功
    }

    //读取地址是否为0A 0A 0X0A
    SpiFlashReadData((cur_cap_storage_addr), arr_read_flash_buf, CAP_STORAGE_SIZE);
    if((arr_read_flash_buf[0] == 0X0A) && (arr_read_flash_buf[1] == 0X0A) && (arr_read_flash_buf[2] == 0X0A) && (arr_read_flash_buf[3] == 0X0A))
    {
        memcpy(arr_write_flash_buf, (uint8_t *)(&st_cap_storge_info.cap_storge_id), CAP_STORAGE_SIZE);
        //进行CRC校验
        crc_data = CrcModbus((uint8_t *)(&st_cap_storge_info.cap_storge_id), CAP_STORAGE_SIZE);
        //写数据
        GetDayCapFlashAddr(&flash_cap_cur_day_addr, gstRtcTime); //获取当天的地址
        cur_sector_start_addr = flash_cap_cur_day_addr & 0xfffff000;
        cur_sector_shift_cnt = flash_cap_cur_day_addr - cur_sector_start_addr;
        SpiFlashReadData(cur_sector_start_addr, gArrReadBuf, 4096);
        SpiFlashAutoEraseSectors(cur_sector_start_addr, 0x1000); //擦除4K
        SpiFlashWriteBytesWithoutErase(cur_sector_start_addr, gArrReadBuf, cur_sector_shift_cnt); //将read_buf[0] ---read_buf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase(flash_cap_cur_day_addr + 256, &gArrReadBuf[cur_sector_shift_cnt + 256], (0x1000 - cur_sector_shift_cnt - 256)); //将gArrReadBuf[0] ---gArrReadBuf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase((cur_cap_storage_addr + 4), arr_write_flash_buf + 4, CAP_STORAGE_SIZE - 4);
        SpiFlashWriteBytesWithoutErase((cur_cap_storage_addr), arr_write_flash_buf, 4);
//        SpiFlashReadData(cur_sector_start_addr, gArrReadBuf, 4096);
    }
    else if(((arr_read_flash_buf[0] == 0x00) && (arr_read_flash_buf[1] == 0x00) && (arr_read_flash_buf[2] == 0x00) && (arr_read_flash_buf[3] == 0x00)) || \
            ((arr_read_flash_buf[0] == 0xFF) && (arr_read_flash_buf[1] == 0xFF) && (arr_read_flash_buf[2] == 0xFF) && (arr_read_flash_buf[3] == 0xFF)))
    {
        //非连续天工作，重新上电后的第一次写数据
        memcpy(arr_write_flash_buf, (uint8_t *)(&st_cap_storge_info.cap_storge_id), CAP_STORAGE_SIZE);
        //进行CRC校验
        crc_data = CrcModbus((uint8_t *)(&st_cap_storge_info.cap_storge_id), CAP_STORAGE_SIZE);
        GetDayCapFlashAddr(&flash_cap_cur_day_addr, gstRtcTime); //获取当天的地址
        cur_sector_start_addr = flash_cap_cur_day_addr & 0xfffff000;
        cur_sector_shift_cnt = flash_cap_cur_day_addr - cur_sector_start_addr;
        SpiFlashReadData(cur_sector_start_addr, gArrReadBuf, 4096);
        SpiFlashAutoEraseSectors(cur_sector_start_addr, 0x1000); //擦除4K
        SpiFlashWriteBytesWithoutErase(cur_sector_start_addr, gArrReadBuf, cur_sector_shift_cnt); //将read_buf[0] ---read_buf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase(flash_cap_cur_day_addr + 256, &gArrReadBuf[cur_sector_shift_cnt + 256], (0x1000 - cur_sector_shift_cnt - 256)); //将gArrReadBuf[0] ---gArrReadBuf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase((cur_cap_storage_addr + 4), arr_write_flash_buf + 4, CAP_STORAGE_SIZE - 4);
        SpiFlashWriteBytesWithoutErase((cur_cap_storage_addr), arr_write_flash_buf, 4);
//        SpiFlashReadData(cur_sector_start_addr, gArrReadBuf, 4096);
    }

    /*读数据*/
    SpiFlashReadData(cur_cap_storage_addr, arr_read_flash_buf, CAP_STORAGE_SIZE);

    /*进行crc数据校验*/
    if(crc_data == CrcModbus(arr_read_flash_buf, CAP_STORAGE_SIZE))
    {

    }
    else
    {
        /*写数据*/
        GetDayCapFlashAddr(&flash_cap_cur_day_addr, gstRtcTime); //获取当天的地址
        cur_sector_start_addr = flash_cap_cur_day_addr & 0xfffff000;
        cur_sector_shift_cnt = flash_cap_cur_day_addr - cur_sector_start_addr;
        SpiFlashReadData(cur_sector_start_addr, gArrReadBuf, 4096);
        SpiFlashAutoEraseSectors(cur_sector_start_addr, 0x1000); //擦除4K
        SpiFlashWriteBytesWithoutErase(cur_sector_start_addr, gArrReadBuf, cur_sector_shift_cnt); //将read_buf[0] ---read_buf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase(flash_cap_cur_day_addr + 256, &gArrReadBuf[cur_sector_shift_cnt + 256], (0x1000 - cur_sector_shift_cnt - 256)); //将gArrReadBuf[0] ---gArrReadBuf[cur_sector_shift_addr]  //写入
        SpiFlashWriteBytesWithoutErase((cur_cap_storage_addr + 4), arr_write_flash_buf + 4, CAP_STORAGE_SIZE - 4);
        SpiFlashWriteBytesWithoutErase((cur_cap_storage_addr), arr_write_flash_buf, 4);
    }

    //校准存储值校准
    gstTmpCapStorgeInfo.pv_generation_cap_sum = gstCapStorgeInfo.pv_generation_cap_sum;
    gstTmpCapStorgeInfo.purchase_cap_sum = gstCapStorgeInfo.purchase_cap_sum;
    gstTmpCapStorgeInfo.load_consum_cap_sum = gstCapStorgeInfo.load_consum_cap_sum;
    gstTmpCapStorgeInfo.selling_cap_sum = gstCapStorgeInfo.selling_cap_sum;
    gstTmpCapStorgeInfo.batt_chg_cap_sum = gstCapStorgeInfo.batt_chg_cap_sum;
    gstTmpCapStorgeInfo.batt_dsg_cap_sum = gstCapStorgeInfo.batt_dsg_cap_sum;
    gstTmpCapStorgeInfo.remote_load_cap_sum = gstCapStorgeInfo.remote_load_cap_sum;
    gstTmpCapStorgeInfo.gen_elec_cap_sum = gstCapStorgeInfo.gen_elec_cap_sum;
    gstTmpCapStorgeInfo.ev_chg_cap_sum = gstCapStorgeInfo.ev_chg_cap_sum;
    gstTmpCapStorgeInfo.ev_dsg_cap_sum = gstCapStorgeInfo.ev_dsg_cap_sum;
    return storage_status;
}

void GetHistoryCapInfoFromFlash(DevRtc_t st_rtc_temp)
{
    uint32_t    cur_year_addr = 0;
    uint32_t    cur_month_addr = 0;
    uint32_t    cur_day_cap_addr = 0;
    DevRtc_t    st_rtc_time_temp;
    uint16_t    i = 0;
    uint16_t    j = 0;
    uint32_t    k = 0;
    uint32_t    count = 0;
    uint32_t    temp_addr;
    uint8_t     arr_before_falsh_buf[0x100];
    uint8_t     arr_today_falsh_buf[CAP_STORAGE_SIZE];
    int32_t     year_tmp_addr = 0;
    int32_t     month_tmp_addr = 0;

    memset(arr_before_falsh_buf, 0, 0x100);
    memset(arr_today_falsh_buf, 0, CAP_STORAGE_SIZE);

    /*开机读取历史数据*/
    if((st_rtc_temp.year > END_STORAGE_YEAR) || (st_rtc_temp.year < INIT_STORAGE_YEAR))
    {
        return;
    }
    st_rtc_time_temp = st_rtc_temp;//gstRtcTime;
    st_rtc_time_temp.month = 1;
    st_rtc_time_temp.day = 1;
    st_rtc_time_temp.hour = 0;
    st_rtc_time_temp.minute = 0;
    st_rtc_time_temp.second = 0;
    GetDayCapFlashAddr(&cur_year_addr, st_rtc_time_temp); //获取当年1月1号的地址

    memset((uint8_t *)(&gstStartTodayCaPStorgeInfo.cap_storge_id), 0, CAP_STORAGE_SIZE);
    memset((uint8_t *)(&gstStartMonthCaPStorgeInfo.cap_storge_id), 0, CAP_STORAGE_SIZE);
    memset((uint8_t *)(&gstStartYearCaPStorgeInfo.cap_storge_id), 0, CAP_STORAGE_SIZE);

    //当天的地址
    cur_day_cap_addr = (cur_year_addr + ((gstRtcTime.month - 1) << 13) + ((gstRtcTime.day - 1) << 8));
    //读取当天的值
    SpiFlashReadData(cur_day_cap_addr, arr_today_falsh_buf, CAP_STORAGE_SIZE);

    cur_day_cap_addr -= CAP_DAY_SIZE;
    count = ((cur_day_cap_addr - CAP_STORAGE_BASE_ADDR) / CAP_DAY_SIZE) + 1;

    for(i = 0; i < count; i++)
    {
        temp_addr = cur_day_cap_addr - (i * CAP_DAY_SIZE);
        SpiFlashReadData(temp_addr, arr_before_falsh_buf, CAP_DAY_SIZE);
        if((arr_before_falsh_buf[0] == 0x0A) && (arr_before_falsh_buf[1] == 0x0A) && (arr_before_falsh_buf[2] == 0x0A) && (arr_before_falsh_buf[3] == 0x0A))
        {
            memcpy((uint8_t *)(&gstStartTodayCaPStorgeInfo.cap_storge_id), arr_before_falsh_buf, CAP_STORAGE_SIZE);
            break;   //找到当天前的第一个的有效值
        }

        if(i == count - 1) //往前推算 未找到有效值，则表示今天为第一次存储
        {
//            gstStartTodayCaPStorgeInfo.cap_storge_id = 0x0A0A0A0A;
            gstStartTodayCaPStorgeInfo.pv_generation_cap_sum = 0;
            gstStartTodayCaPStorgeInfo.purchase_cap_sum = 0;
            gstStartTodayCaPStorgeInfo.selling_cap_sum = 0;
            gstStartTodayCaPStorgeInfo.load_consum_cap_sum = 0;
            gstStartTodayCaPStorgeInfo.batt_chg_cap_sum = 0;
            gstStartTodayCaPStorgeInfo.batt_dsg_cap_sum = 0;
            gstStartTodayCaPStorgeInfo.remote_load_cap_sum = 0;
            gstStartTodayCaPStorgeInfo.gen_elec_cap_sum = 0;
            gstStartTodayCaPStorgeInfo.ev_chg_cap_sum = 0;
            gstStartTodayCaPStorgeInfo.ev_dsg_cap_sum = 0;
        }
    }

    if((arr_today_falsh_buf[0] == 0x0A) && (arr_today_falsh_buf[1] == 0x0A) && ((arr_today_falsh_buf[2] == 0x0A)) && (arr_today_falsh_buf[3] == 0x0A))
    {
        gTodayValueExisFlag = 1;
        memcpy((uint8_t *)&gstTodayCaPStorgeInfo.cap_storge_id, arr_today_falsh_buf, CAP_STORAGE_SIZE);
    }
    else
    {
        gTodayValueExisFlag = 0;
//        gstTodayCaPStorgeInfo.cap_storge_id = 0x0A0A0A0A;
        gstTodayCaPStorgeInfo.pv_generation_cap_sum = gstStartTodayCaPStorgeInfo.pv_generation_cap_sum;
        gstTodayCaPStorgeInfo.purchase_cap_sum = gstStartTodayCaPStorgeInfo.purchase_cap_sum;
        gstTodayCaPStorgeInfo.selling_cap_sum = gstStartTodayCaPStorgeInfo.selling_cap_sum;
        gstTodayCaPStorgeInfo.load_consum_cap_sum = gstStartTodayCaPStorgeInfo.load_consum_cap_sum;
        gstTodayCaPStorgeInfo.batt_chg_cap_sum = gstStartTodayCaPStorgeInfo.batt_chg_cap_sum;
        gstTodayCaPStorgeInfo.batt_dsg_cap_sum = gstStartTodayCaPStorgeInfo.batt_dsg_cap_sum;
        gstTodayCaPStorgeInfo.remote_load_cap_sum = gstStartTodayCaPStorgeInfo.remote_load_cap_sum;
        gstTodayCaPStorgeInfo.gen_elec_cap_sum = gstStartTodayCaPStorgeInfo.gen_elec_cap_sum;
        gstTodayCaPStorgeInfo.ev_chg_cap_sum = gstStartTodayCaPStorgeInfo.ev_dsg_cap_sum;
        gstTodayCaPStorgeInfo.ev_dsg_cap_sum = gstStartTodayCaPStorgeInfo.ev_dsg_cap_sum;
    }

    //当月的地址
    cur_month_addr = (cur_year_addr + (gstRtcTime.month - 1) * 0x2000);
    cur_month_addr -= CAP_DAY_SIZE;
    count = ((cur_month_addr - CAP_STORAGE_BASE_ADDR) / CAP_DAY_SIZE) + 1;

    for(j = 0; j < count; j++)
    {
        temp_addr = cur_month_addr - (j * CAP_DAY_SIZE);
        month_tmp_addr = (int32_t)temp_addr;
        if(month_tmp_addr <= 0)
        {
            gstStartMonthCaPStorgeInfo.pv_generation_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.purchase_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.selling_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.load_consum_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.batt_chg_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.batt_dsg_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.remote_load_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.gen_elec_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.ev_chg_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.ev_dsg_cap_sum = 0;
            break;
        }
        SpiFlashReadData(temp_addr, arr_before_falsh_buf, CAP_DAY_SIZE);
        if((arr_before_falsh_buf[0] == 0x0A) && (arr_before_falsh_buf[1] == 0x0A) && (arr_before_falsh_buf[2] == 0x0A) && (arr_before_falsh_buf[3] == 0x0A))
        {
            memcpy((uint8_t *)(&gstStartMonthCaPStorgeInfo.cap_storge_id), arr_before_falsh_buf, CAP_STORAGE_SIZE);
            break;   //从本月第一天往前找到第一个的有效值 作为本月的基准值
        }
        if(j == count - 1) //往前推算 未找到有效值，则表示今天为第一次存储
        {
//            gstStartMonthCaPStorgeInfo.cap_storge_id = 0x0A0A0A0A;
            gstStartMonthCaPStorgeInfo.pv_generation_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.purchase_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.selling_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.load_consum_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.batt_chg_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.batt_dsg_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.remote_load_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.gen_elec_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.ev_chg_cap_sum = 0;
            gstStartMonthCaPStorgeInfo.ev_dsg_cap_sum = 0;
        }
    }

    //2024年基准值
    count = (st_rtc_temp.month - 1) * 32 + st_rtc_temp.day - 1;
    if(cur_year_addr == CAP_STORAGE_BASE_ADDR)
    {
        temp_addr = cur_year_addr - CAP_DAY_SIZE;
        SpiFlashReadData(temp_addr, arr_before_falsh_buf, CAP_DAY_SIZE);
        if((arr_before_falsh_buf[0] == 0x0A) && (arr_before_falsh_buf[1] == 0x0A) && (arr_before_falsh_buf[2] == 0x0A) && (arr_before_falsh_buf[3] == 0x0A))
        {
            memcpy((uint8_t *)(&gstStartYearCaPStorgeInfo.cap_storge_id), arr_before_falsh_buf, CAP_STORAGE_SIZE);
        }
        else
        {
//            gstStartYearCaPStorgeInfo.cap_storge_id = 0x0A0A0A0A;
            gstStartYearCaPStorgeInfo.pv_generation_cap_sum = 0;
            gstStartYearCaPStorgeInfo.purchase_cap_sum = 0;
            gstStartYearCaPStorgeInfo.selling_cap_sum = 0;
            gstStartYearCaPStorgeInfo.load_consum_cap_sum = 0;
            gstStartYearCaPStorgeInfo.batt_chg_cap_sum = 0;
            gstStartYearCaPStorgeInfo.batt_dsg_cap_sum = 0;
            gstStartYearCaPStorgeInfo.remote_load_cap_sum = 0;
            gstStartYearCaPStorgeInfo.gen_elec_cap_sum = 0;
            gstStartYearCaPStorgeInfo.ev_chg_cap_sum = 0;
            gstStartYearCaPStorgeInfo.ev_dsg_cap_sum = 0;
        }
    }
    else  //大于2024年
    {
        cur_year_addr -= CAP_DAY_SIZE;
        count = ((cur_year_addr - CAP_STORAGE_BASE_ADDR) / CAP_DAY_SIZE) + 1;
        //当年基准值
        for(k = 0; k <= count; k++)
        {
            temp_addr = cur_year_addr - (k * CAP_DAY_SIZE);
            year_tmp_addr = (int32_t)temp_addr;
            if(year_tmp_addr <= 0)
            {
                gstStartYearCaPStorgeInfo.pv_generation_cap_sum = 0;
                gstStartYearCaPStorgeInfo.purchase_cap_sum = 0;
                gstStartYearCaPStorgeInfo.selling_cap_sum = 0;
                gstStartYearCaPStorgeInfo.load_consum_cap_sum = 0;
                gstStartYearCaPStorgeInfo.batt_chg_cap_sum = 0;
                gstStartYearCaPStorgeInfo.batt_dsg_cap_sum = 0;
                gstStartYearCaPStorgeInfo.remote_load_cap_sum = 0;
                gstStartYearCaPStorgeInfo.gen_elec_cap_sum = 0;
                gstStartYearCaPStorgeInfo.ev_chg_cap_sum = 0;
                gstStartYearCaPStorgeInfo.ev_dsg_cap_sum = 0;
                break;
            }
            SpiFlashReadData(temp_addr, arr_before_falsh_buf, CAP_DAY_SIZE);
            if((arr_before_falsh_buf[0] == 0x0A) && (arr_before_falsh_buf[1] == 0x0A) && (arr_before_falsh_buf[2] == 0x0A) && (arr_before_falsh_buf[3] == 0x0A))
            {
                memcpy((uint8_t *)(&gstStartYearCaPStorgeInfo.cap_storge_id), arr_before_falsh_buf, CAP_STORAGE_SIZE);
                break;   //从本年第一天往前找到第一个的有效值 作为本年的基准值
            }
            if(k == count - 1)
            {
//                gstStartYearCaPStorgeInfo.cap_storge_id = 0x0A0A0A0A;
                gstStartYearCaPStorgeInfo.pv_generation_cap_sum = 0;
                gstStartYearCaPStorgeInfo.purchase_cap_sum = 0;
                gstStartYearCaPStorgeInfo.selling_cap_sum = 0;
                gstStartYearCaPStorgeInfo.load_consum_cap_sum = 0;
                gstStartYearCaPStorgeInfo.batt_chg_cap_sum = 0;
                gstStartYearCaPStorgeInfo.batt_dsg_cap_sum = 0;
                gstStartYearCaPStorgeInfo.remote_load_cap_sum = 0;
                gstStartYearCaPStorgeInfo.gen_elec_cap_sum = 0;
                gstStartYearCaPStorgeInfo.ev_chg_cap_sum = 0;
                gstStartYearCaPStorgeInfo.ev_dsg_cap_sum = 0;
            }
        }
    }
}

uint32_t GetPVCapTotal(void)
{
    static uint32_t pv_chg_cap_sum = 0;
    uint32_t pv_chg_cap_range_sum = 0;

    pv_chg_cap_range_sum = (gstUdpPcsSlavePara.pcs_info.elec_stats.pv_elec_total[0] << 16) + gstUdpPcsSlavePara.pcs_info.elec_stats.pv_elec_total[1];
    if(pv_chg_cap_range_sum < HALF_AN_HOUR_CAP)
    {
        pv_chg_cap_sum = pv_chg_cap_range_sum;
    }
#if CAP_DEBUG_TEST
    static uint32_t time_out = 0;
    if(tim_difftime_from_now(time_out) > 1000 * 5)
    {
        pv_sum += 1;
        time_out = tim_get_systime();
    }
    pv_chg_cap_sum = pv_sum;
#endif

    return pv_chg_cap_sum;
}

uint32_t GetPcsLoadTotal(void)
{
    static uint32_t pcs_load_consum = 0;
    uint32_t pca_load_range_sum = 0;

    pca_load_range_sum = ((gstUdpPcsSlavePara.pcs_info.elec_stats.eps_elec_total[0] << 16 ) + gstUdpPcsSlavePara.pcs_info.elec_stats.eps_elec_total[1]);
    if(pca_load_range_sum < HALF_AN_HOUR_CAP)
    {
        pcs_load_consum = pca_load_range_sum;
    }
#if CAP_DEBUG_TEST
    static uint32_t time_out1 = 0;
    if(tim_difftime_from_now(time_out1) > 1000 * 5)
    {
        eps_sum += 1;
        time_out1 = tim_get_systime();
    }
    pcs_load_consum = eps_sum;
#endif
    return pcs_load_consum;
}

uint32_t GetPcsPurchaseTotal(void)
{
    static uint32_t pcs_purchase_consum;
    uint32_t pcs_purchase_range_sum = 0;

    pcs_purchase_range_sum = ((gstUdpPcsSlavePara.pcs_info.elec_stats.buy_elec_total[0] << 16) + gstUdpPcsSlavePara.pcs_info.elec_stats.buy_elec_total[1]);
    if(pcs_purchase_range_sum < HALF_AN_HOUR_CAP)
    {
        pcs_purchase_consum = pcs_purchase_range_sum;
    }
    #if CAP_DEBUG_TEST
    static uint32_t time_out2 = 0;
    if(tim_difftime_from_now(time_out2) > 1000 * 5)
    {
        buy_sum += 1;
        time_out2 = tim_get_systime();
    }
    pcs_purchase_consum = buy_sum;
#endif
    return pcs_purchase_consum;
}

uint32_t GetPcsSaleTotal(void)
{
    static uint32_t pcs_sale_consum = 0;
    uint32_t pcs_sale_range_sum = 0;

    pcs_sale_range_sum = ((gstUdpPcsSlavePara.pcs_info.elec_stats.sell_elec_total[0] << 16) + gstUdpPcsSlavePara.pcs_info.elec_stats.sell_elec_total[1]);
    if(pcs_sale_range_sum < HALF_AN_HOUR_CAP)
    {
        pcs_sale_consum = pcs_sale_range_sum;
    }

#if CAP_DEBUG_TEST
    static uint32_t time_out3 = 0;
    if(tim_difftime_from_now(time_out3) > 1000 * 5)
    {
        sell_sum += 1;
        time_out3 = tim_get_systime();
    }
    pcs_sale_consum = sell_sum;
#endif

    return pcs_sale_consum;
}

uint32_t GetBatteryChgTotal(void)
{
    static uint32_t chg_battery_sum_wh = 0;
    uint32_t chg_battery_range_sum = 0;

    chg_battery_range_sum = (gstUdpPcsSlavePara.pcs_info.elec_stats.bat_elec_chg_total[0] << 16) + gstUdpPcsSlavePara.pcs_info.elec_stats.bat_elec_chg_total[1];
    if(chg_battery_range_sum < HALF_AN_HOUR_CAP)
    {
        chg_battery_sum_wh = chg_battery_range_sum;
    }
#if CAP_DEBUG_TEST
    static uint32_t time_out4 = 0;
    if(tim_difftime_from_now(time_out4) > 1000 * 5)
    {
        batt_chg_sum += 1;
        time_out4 = tim_get_systime();
    }
    chg_battery_sum_wh = batt_chg_sum;
#endif
    return chg_battery_sum_wh;
}

uint32_t GetBatteryDschgTotal(void)
{
    static uint32_t dsg_battery_sum_wh = 0;
    uint32_t dsg_battery_range_sum = 0;

    dsg_battery_range_sum += (gstUdpPcsSlavePara.pcs_info.elec_stats.bat_elec_dsg_total[0] << 16) + gstUdpPcsSlavePara.pcs_info.elec_stats.bat_elec_dsg_total[1];
    if(dsg_battery_range_sum < HALF_AN_HOUR_CAP)
    {
        dsg_battery_sum_wh = dsg_battery_range_sum;
    }  
#if CAP_DEBUG_TEST
    static uint32_t time_out5 = 0;
    if(tim_difftime_from_now(time_out5) > 1000 * 5)
    {
        batt_dsg_sum += 1;
        time_out5 = tim_get_systime();
    }
    dsg_battery_sum_wh = batt_dsg_sum;
#endif

    return dsg_battery_sum_wh;
}

uint32_t GetRemoteLoadTotal(void)
{
    static uint32_t remote_load_sum = 0;
    uint32_t remote_load_range_sum = 0;

    remote_load_range_sum = (gstUdpPcsSlavePara.pcs_info.elec_stats.remote_load_elec_total[0] << 16) + gstUdpPcsSlavePara.pcs_info.elec_stats.remote_load_elec_total[1];
    if(remote_load_range_sum < HALF_AN_HOUR_CAP)
    {
        remote_load_sum = remote_load_range_sum;
    }
#if CAP_DEBUG_TEST
    static uint32_t time_out6 = 0;
    if(tim_difftime_from_now(time_out6) > 1000 * 5)
    {
        remote_sum += 1;
        time_out6 = tim_get_systime();
    }
    remote_load_sum = remote_sum;
#endif
    return remote_load_sum;
}

uint32_t GetGenTotal(void)
{
    static uint32_t gen_sum = 0;
    uint32_t gen_range_sum = 0;

    gen_range_sum = (gstUdpPcsSlavePara.pcs_info.elec_stats.gen_elec_total[0] << 16) + gstUdpPcsSlavePara.pcs_info.elec_stats.gen_elec_total[1];
    if(gen_range_sum < HALF_AN_HOUR_CAP)
    {
        gen_sum = gen_range_sum;
    }
#if CAP_DEBUG_TEST
    static uint32_t time_out7 = 0;
    if(tim_difftime_from_now(time_out7) > 1000 * 5)
    {
        gen_elec_sum += 1;
        time_out7 = tim_get_systime();
    }
    gen_sum = gen_elec_sum;
#endif
    return gen_sum;
}

uint32_t GetEVChgTotal(void)
{
    static uint32_t EV_chg_sum = 0;
    uint32_t ev_chg_range_sum = 0;

    ev_chg_range_sum = (gstUdpPcsSlavePara.pcs_info.elec_stats.ev_elec_chg_total[0] << 16) + gstUdpPcsSlavePara.pcs_info.elec_stats.ev_elec_chg_total[1];
    if(ev_chg_range_sum < HALF_AN_HOUR_CAP)
    {
        EV_chg_sum = ev_chg_range_sum;
    }
#if CAP_DEBUG_TEST
    static uint32_t time_out8 = 0;
    if(tim_difftime_from_now(time_out8) > 1000 * 5)
    {
        ev_chg_sum += 1;
        time_out8 = tim_get_systime();
    }
    EV_chg_sum = ev_chg_sum;
#endif

    return EV_chg_sum;
}

uint32_t GetEVDsgTotal(void)
{
    static uint32_t EV_dsg_sum = 0;
    uint32_t ev_dsg_range_sum = 0;

    ev_dsg_range_sum = (gstUdpPcsSlavePara.pcs_info.elec_stats.ev_elec_dsg_total[0] << 16) + gstUdpPcsSlavePara.pcs_info.elec_stats.ev_elec_dsg_total[1];
    if(ev_dsg_range_sum < HALF_AN_HOUR_CAP)
    {
        EV_dsg_sum = ev_dsg_range_sum;
    }
#if CAP_DEBUG_TEST
    static uint32_t time_out9 = 0;
    if(tim_difftime_from_now(time_out9) > 1000 * 5)
    {
        ev_dsg_sum += 1;
        time_out9 = tim_get_systime();
    }
    EV_dsg_sum = ev_dsg_sum;
#endif
    return EV_dsg_sum;
}


void GetCapFormPcs(void)
{
    uint32_t    temp_data = 0;
    uint32_t    pcs_purchase_consum = 0;
    uint32_t    pcs_load_consum = 0;
    uint32_t    pcs_sale_consum = 0;
    uint32_t    pv_chg_cap_sum = 0;
    uint32_t    pv_temp_data = 0;
    uint32_t    batt_chg_cap_sum = 0;
    uint32_t    batt_dschg_cap_sum = 0;

    gstCapStorgeInfo.cap_storge_id = 0X0A0A0A0A;
    gstCapStorgeInfo.pv_generation_cap_sum = gstTmpCapStorgeInfo.pv_generation_cap_sum + gstCurrentCapPcsInfo.pv_total;
    gstCapStorgeInfo.purchase_cap_sum = gstTmpCapStorgeInfo.purchase_cap_sum + gstCurrentCapPcsInfo.purchase_total;
    gstCapStorgeInfo.load_consum_cap_sum = gstTmpCapStorgeInfo.load_consum_cap_sum + gstCurrentCapPcsInfo.load_total;
    gstCapStorgeInfo.selling_cap_sum = gstTmpCapStorgeInfo.selling_cap_sum + gstCurrentCapPcsInfo.selling_total;
    gstCapStorgeInfo.batt_chg_cap_sum = gstTmpCapStorgeInfo.batt_chg_cap_sum + gstCurrentCapPcsInfo.batt_chg_total;
    gstCapStorgeInfo.batt_dsg_cap_sum = gstTmpCapStorgeInfo.batt_dsg_cap_sum + gstCurrentCapPcsInfo.batt_dsg_total;
    gstCapStorgeInfo.remote_load_cap_sum = gstTmpCapStorgeInfo.remote_load_cap_sum + gstCurrentCapPcsInfo.remote_load_total;
    gstCapStorgeInfo.gen_elec_cap_sum = gstTmpCapStorgeInfo.gen_elec_cap_sum + gstCurrentCapPcsInfo.gen_elec_total;
    gstCapStorgeInfo.ev_chg_cap_sum = gstTmpCapStorgeInfo.ev_chg_cap_sum + gstCurrentCapPcsInfo.ev_chg_total;
    gstCapStorgeInfo.ev_dsg_cap_sum = gstTmpCapStorgeInfo.ev_dsg_cap_sum + gstCurrentCapPcsInfo.ev_dsg_total;
}

//容错操作
static void _FaultTolerantOperation(void)
{
    //容错操作   //天
    if ((gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day > gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month))
    {
        gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day =  gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day > gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month))
    {
        gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day > gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month))
    {
        gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day > gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month))
    {
        gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_day > gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month))
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_day = gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_day > gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month))
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_day = gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day > gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month))
    {
        gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day = gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_day > gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month))
    {
        gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_day = gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_day > gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month))
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_day = gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_day > gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month))
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_day = gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month;
    }

    //月
    if ((gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month > gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year))
    {
        gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month = gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month > gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year))
    {
        gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month > gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year))
    {
        gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month > gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year))
    {
        gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month > gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_year))
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month = gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_year;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month > gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_year))
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month = gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_year;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month > gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_year))
    {
        gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month = gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_year;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month > gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_year))
    {
        gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month = gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_year;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month > gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_year))
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month = gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_year;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month > gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_year))
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month = gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_year;
    }

    //年
    if ((gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year > gstCollectInfo.maincard_data.elec_stats_info.pv_elec_total))
    {
        gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year = gstCollectInfo.maincard_data.elec_stats_info.pv_elec_total;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year > gstCollectInfo.maincard_data.elec_stats_info.eps_elec_total))
    {
        gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year = gstCollectInfo.maincard_data.elec_stats_info.eps_elec_total;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year > gstCollectInfo.maincard_data.elec_stats_info.buy_elec_total))
    {
        gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year = gstCollectInfo.maincard_data.elec_stats_info.buy_elec_total;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year > gstCollectInfo.maincard_data.elec_stats_info.sell_elec_total))
    {
        gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year = gstCollectInfo.maincard_data.elec_stats_info.sell_elec_total;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_year > gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_total))
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_year = gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_total;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_year > gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_total))
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_year = gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_total;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_year > gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_total))
    {
        gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_year = gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_total;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_year > gstCollectInfo.maincard_data.elec_stats_info.gen_elec_total))
    {
        gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_year = gstCollectInfo.maincard_data.elec_stats_info.gen_elec_total;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_year > gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_total))
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_year = gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_total;
    }
    if((gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_year > gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_total))
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_year = gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_total;
    }
}


void RefreshHistoryData(void)
{
    int32_t pv_day = 0;
    int32_t purchase_day = 0;
    int32_t sale_day = 0;
    int32_t load_day = 0;
    int32_t batt_chg_day = 0;
    int32_t batt_dschg_day = 0;
    int32_t remote_day = 0;
    int32_t gen_day = 0;
    int32_t ev_chg_day = 0;
    int32_t ev_dsg_day = 0;

    int32_t pv_month = 0;
    int32_t purchase_month = 0;
    int32_t sale_month = 0;
    int32_t load_month = 0;
    int32_t batt_chg_month = 0;
    int32_t batt_dschg_month = 0;
    int32_t remote_month = 0;
    int32_t gen_month = 0;
    int32_t ev_chg_month = 0;
    int32_t ev_dsg_month = 0;

    int32_t pv_year = 0;
    int32_t purchase_year = 0;
    int32_t sale_year = 0;
    int32_t load_year = 0;
    int32_t batt_chg_year = 0;
    int32_t batt_dschg_year = 0;
    int32_t remote_year = 0;
    int32_t gen_year = 0;
    int32_t ev_chg_year = 0;
    int32_t ev_dsg_year = 0;

    int32_t pv_total = 0;
    int32_t purchase_total = 0;
    int32_t sale_total = 0;
    int32_t load_total = 0;
    int32_t batt_chg_total = 0;
    int32_t batt_dschg_total = 0;
    int32_t remote_total = 0;
    int32_t gen_total = 0;
    int32_t ev_chg_total = 0;
    int32_t ev_dsg_total = 0;

    //天
    gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day = (gstCapStorgeInfo.pv_generation_cap_sum - gstStartTodayCaPStorgeInfo.pv_generation_cap_sum);
    gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day = (gstCapStorgeInfo.load_consum_cap_sum - gstStartTodayCaPStorgeInfo.load_consum_cap_sum);
    gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day = (gstCapStorgeInfo.purchase_cap_sum - gstStartTodayCaPStorgeInfo.purchase_cap_sum);
    gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day = (gstCapStorgeInfo.selling_cap_sum - gstStartTodayCaPStorgeInfo.selling_cap_sum);
    gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_day = (gstCapStorgeInfo.batt_chg_cap_sum - gstStartTodayCaPStorgeInfo.batt_chg_cap_sum);
    gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_day = (gstCapStorgeInfo.batt_dsg_cap_sum - gstStartTodayCaPStorgeInfo.batt_dsg_cap_sum);
    gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day = (gstCapStorgeInfo.remote_load_cap_sum - gstStartTodayCaPStorgeInfo.remote_load_cap_sum);
    gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_day = (gstCapStorgeInfo.gen_elec_cap_sum - gstStartTodayCaPStorgeInfo.gen_elec_cap_sum);
    gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_day = (gstCapStorgeInfo.ev_chg_cap_sum - gstStartTodayCaPStorgeInfo.ev_chg_cap_sum);
    gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_day = (gstCapStorgeInfo.ev_dsg_cap_sum - gstStartTodayCaPStorgeInfo.ev_dsg_cap_sum);

    pv_day          = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day;
    load_day        = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day;
    purchase_day    = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day;
    sale_day        = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day;
    batt_chg_day    = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_day;
    batt_dschg_day  = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_day;
    remote_day      = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day;
    gen_day         = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_day;
    ev_chg_day      = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_day;
    ev_dsg_day      = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_day;
    if((pv_day < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day =  (gstStartTodayCaPStorgeInfo.pv_generation_cap_sum + GetPVCapTotal()) - gstStartTodayCaPStorgeInfo.pv_generation_cap_sum;
    }
    if((load_day < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day = (gstStartTodayCaPStorgeInfo.load_consum_cap_sum + GetPcsLoadTotal()) - gstStartTodayCaPStorgeInfo.load_consum_cap_sum;
    }
    if((purchase_day < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day = (gstStartTodayCaPStorgeInfo.purchase_cap_sum + GetPcsPurchaseTotal()) - gstStartTodayCaPStorgeInfo.purchase_cap_sum;
    }
    if((sale_day < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day = (gstStartTodayCaPStorgeInfo.selling_cap_sum + GetPcsSaleTotal()) - gstStartTodayCaPStorgeInfo.selling_cap_sum;
    }
    if((batt_chg_day) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_day = (gstStartTodayCaPStorgeInfo.batt_chg_cap_sum + GetBatteryChgTotal()) - gstStartTodayCaPStorgeInfo.batt_chg_cap_sum;
    }
    if((batt_dschg_day) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_day = (gstStartTodayCaPStorgeInfo.batt_dsg_cap_sum + GetBatteryDschgTotal()) - gstStartTodayCaPStorgeInfo.batt_dsg_cap_sum;
    }
    if((remote_day) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day = (gstStartTodayCaPStorgeInfo.remote_load_cap_sum + GetRemoteLoadTotal()) - gstStartTodayCaPStorgeInfo.remote_load_cap_sum;
    }
    if((gen_day) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_day = (gstStartTodayCaPStorgeInfo.gen_elec_cap_sum + GetGenTotal()) - gstStartTodayCaPStorgeInfo.gen_elec_cap_sum;
    }
    if((ev_chg_day) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_day = (gstStartTodayCaPStorgeInfo.ev_chg_cap_sum + GetEVChgTotal()) - gstStartTodayCaPStorgeInfo.ev_chg_cap_sum;
    }
    if((ev_dsg_day) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_day = (gstStartTodayCaPStorgeInfo.ev_dsg_cap_sum + GetEVDsgTotal()) - gstStartTodayCaPStorgeInfo.ev_dsg_cap_sum;
    }
    //月
    gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month = gstCapStorgeInfo.pv_generation_cap_sum - gstStartMonthCaPStorgeInfo.pv_generation_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month = gstCapStorgeInfo.load_consum_cap_sum - gstStartMonthCaPStorgeInfo.load_consum_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month = gstCapStorgeInfo.purchase_cap_sum - gstStartMonthCaPStorgeInfo.purchase_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month = gstCapStorgeInfo.selling_cap_sum - gstStartMonthCaPStorgeInfo.selling_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month = gstCapStorgeInfo.batt_chg_cap_sum - gstStartMonthCaPStorgeInfo.batt_chg_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month = gstCapStorgeInfo.batt_dsg_cap_sum - gstStartMonthCaPStorgeInfo.batt_dsg_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month = gstCapStorgeInfo.remote_load_cap_sum - gstStartMonthCaPStorgeInfo.remote_load_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month = gstCapStorgeInfo.gen_elec_cap_sum - gstStartMonthCaPStorgeInfo.gen_elec_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month = gstCapStorgeInfo.ev_chg_cap_sum - gstStartMonthCaPStorgeInfo.ev_chg_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month = gstCapStorgeInfo.ev_dsg_cap_sum - gstStartMonthCaPStorgeInfo.ev_dsg_cap_sum;

    pv_month         = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month;
    load_month       = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month;
    purchase_month   = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month;
    sale_month       = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month;
    batt_chg_month   = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month;
    batt_dschg_month = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month;
    remote_month     = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month;
    gen_month        = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month;
    ev_chg_month     = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month;
    ev_dsg_month     = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month;

    if((pv_month < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month = (gstStartTodayCaPStorgeInfo.pv_generation_cap_sum + GetPVCapTotal()) - gstStartMonthCaPStorgeInfo.pv_generation_cap_sum;
    }
    if((load_month < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month = (gstStartTodayCaPStorgeInfo.load_consum_cap_sum + GetPcsLoadTotal()) - gstStartMonthCaPStorgeInfo.load_consum_cap_sum;
    }
    if((purchase_month < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month = (gstStartTodayCaPStorgeInfo.purchase_cap_sum + GetPcsPurchaseTotal()) - gstStartMonthCaPStorgeInfo.purchase_cap_sum;
    }
    if((sale_month < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month = (gstStartTodayCaPStorgeInfo.selling_cap_sum + GetPcsSaleTotal()) - gstStartMonthCaPStorgeInfo.selling_cap_sum;
    }
    if((batt_chg_month) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month = (gstStartTodayCaPStorgeInfo.batt_chg_cap_sum + GetBatteryChgTotal()) - gstStartMonthCaPStorgeInfo.batt_chg_cap_sum;
    }
    if((batt_dschg_month) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month = (gstStartTodayCaPStorgeInfo.batt_dsg_cap_sum + GetBatteryDschgTotal()) - gstStartMonthCaPStorgeInfo.batt_dsg_cap_sum;
    }
    if((remote_month) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month = (gstStartTodayCaPStorgeInfo.remote_load_cap_sum + GetRemoteLoadTotal()) - gstStartMonthCaPStorgeInfo.remote_load_cap_sum;
    }
    if((gen_month) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month = (gstStartTodayCaPStorgeInfo.gen_elec_cap_sum + GetGenTotal()) - gstStartMonthCaPStorgeInfo.gen_elec_cap_sum;
    }
    if((ev_chg_month) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month = (gstStartTodayCaPStorgeInfo.ev_chg_cap_sum + GetEVChgTotal()) - gstStartMonthCaPStorgeInfo.ev_chg_cap_sum;
    }
    if((ev_dsg_month) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month = (gstStartTodayCaPStorgeInfo.ev_dsg_cap_sum + GetEVDsgTotal()) - gstStartMonthCaPStorgeInfo.ev_dsg_cap_sum;
    }

    //年
    gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year = gstCapStorgeInfo.pv_generation_cap_sum - gstStartYearCaPStorgeInfo.pv_generation_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year = gstCapStorgeInfo.load_consum_cap_sum - gstStartYearCaPStorgeInfo.load_consum_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year = gstCapStorgeInfo.purchase_cap_sum - gstStartYearCaPStorgeInfo.purchase_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year = gstCapStorgeInfo.selling_cap_sum - gstStartYearCaPStorgeInfo.selling_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_year = gstCapStorgeInfo.batt_chg_cap_sum - gstStartYearCaPStorgeInfo.batt_chg_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_year = gstCapStorgeInfo.batt_dsg_cap_sum - gstStartYearCaPStorgeInfo.batt_dsg_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_year = gstCapStorgeInfo.remote_load_cap_sum - gstStartYearCaPStorgeInfo.remote_load_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_year = gstCapStorgeInfo.gen_elec_cap_sum - gstStartYearCaPStorgeInfo.gen_elec_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_year = gstCapStorgeInfo.ev_chg_cap_sum - gstStartYearCaPStorgeInfo.ev_chg_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_year = gstCapStorgeInfo.ev_dsg_cap_sum - gstStartYearCaPStorgeInfo.ev_dsg_cap_sum;

    pv_year         = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year;
    load_year       = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year;
    purchase_year   = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year;
    sale_year       = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year;
    batt_chg_year   = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_year;
    batt_dschg_year = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_year;
    remote_year     = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_year;
    gen_year        = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_year;
    ev_chg_year     = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_year;
    ev_dsg_year     = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_year;

    if((pv_year < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year = (gstStartTodayCaPStorgeInfo.pv_generation_cap_sum + GetPVCapTotal()) - gstStartYearCaPStorgeInfo.pv_generation_cap_sum;
    }
    if((load_year < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year = (gstStartTodayCaPStorgeInfo.load_consum_cap_sum + GetPcsLoadTotal()) - gstStartYearCaPStorgeInfo.load_consum_cap_sum;
    }
    if((purchase_year < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year = (gstStartTodayCaPStorgeInfo.purchase_cap_sum + GetPcsPurchaseTotal()) - gstStartYearCaPStorgeInfo.purchase_cap_sum;
    }
    if((sale_year < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year = (gstStartTodayCaPStorgeInfo.selling_cap_sum + GetPcsSaleTotal()) - gstStartYearCaPStorgeInfo.selling_cap_sum;
    }
    if((batt_chg_year) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_year = (gstStartTodayCaPStorgeInfo.batt_chg_cap_sum + GetBatteryChgTotal()) - gstStartYearCaPStorgeInfo.batt_chg_cap_sum;
    }
    if((batt_dschg_year) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_year = (gstStartTodayCaPStorgeInfo.batt_dsg_cap_sum + GetBatteryDschgTotal()) - gstStartYearCaPStorgeInfo.batt_dsg_cap_sum;
    }
    if((remote_year < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_year = (gstStartTodayCaPStorgeInfo.remote_load_cap_sum + GetRemoteLoadTotal()) - gstStartYearCaPStorgeInfo.remote_load_cap_sum;
    }
    if((gen_year < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_year = (gstStartTodayCaPStorgeInfo.gen_elec_cap_sum + GetGenTotal()) - gstStartYearCaPStorgeInfo.gen_elec_cap_sum;
    }
    if((ev_chg_year) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_year = (gstStartTodayCaPStorgeInfo.ev_chg_cap_sum + GetEVChgTotal()) - gstStartYearCaPStorgeInfo.ev_chg_cap_sum;
    }
    if((ev_dsg_year) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_year = (gstStartTodayCaPStorgeInfo.ev_dsg_cap_sum + GetEVDsgTotal()) - gstStartYearCaPStorgeInfo.ev_dsg_cap_sum;
    }

    //总量
    gstCollectInfo.maincard_data.elec_stats_info.pv_elec_total = gstCapStorgeInfo.pv_generation_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.eps_elec_total = gstCapStorgeInfo.load_consum_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.buy_elec_total = gstCapStorgeInfo.purchase_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.sell_elec_total = gstCapStorgeInfo.selling_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_total = gstCapStorgeInfo.batt_chg_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_total = gstCapStorgeInfo.batt_dsg_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_total = gstCapStorgeInfo.remote_load_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.gen_elec_total = gstCapStorgeInfo.gen_elec_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_total = gstCapStorgeInfo.ev_chg_cap_sum;
    gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_total = gstCapStorgeInfo.ev_dsg_cap_sum;

    pv_total         = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.pv_elec_total;
    load_total       = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.eps_elec_total;
    purchase_total   = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.buy_elec_total;
    sale_total       = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.sell_elec_total;
    batt_chg_total   = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_total;
    batt_dschg_total = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_total;
    remote_total     = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_total;
    gen_total        = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.gen_elec_total;
    ev_chg_total     = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_total;
    ev_dsg_total     = (int32_t)gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_total;

    if((pv_total < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.pv_elec_total = (gstStartTodayCaPStorgeInfo.pv_generation_cap_sum + GetPVCapTotal());
    }
    if((load_total < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.eps_elec_total = (gstStartTodayCaPStorgeInfo.load_consum_cap_sum + GetPcsLoadTotal());
    }
    if((purchase_total < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.buy_elec_total = (gstStartTodayCaPStorgeInfo.purchase_cap_sum + GetPcsPurchaseTotal());
    }
    if((sale_total < 0))
    {
        gstCollectInfo.maincard_data.elec_stats_info.sell_elec_total = (gstStartTodayCaPStorgeInfo.selling_cap_sum + GetPcsSaleTotal());
    }
    if((batt_chg_total) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_total = (gstStartTodayCaPStorgeInfo.batt_chg_cap_sum + GetBatteryChgTotal());
    }
    if((batt_dschg_total) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_total = (gstStartTodayCaPStorgeInfo.batt_dsg_cap_sum + GetBatteryDschgTotal());
    }
    if((remote_total) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_total = (gstStartTodayCaPStorgeInfo.remote_load_cap_sum + GetRemoteLoadTotal());
    }
    if((gen_total) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.gen_elec_total = (gstStartTodayCaPStorgeInfo.gen_elec_cap_sum + GetGenTotal());
    }
    if((ev_chg_total) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_total = (gstStartTodayCaPStorgeInfo.ev_chg_cap_sum + GetEVChgTotal());
    }
    if((ev_dsg_total) < 0)
    {
        gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_total = (gstStartTodayCaPStorgeInfo.ev_dsg_cap_sum + GetEVDsgTotal());
    }

    //容错操作
    _FaultTolerantOperation();
}
void CalculateNewCap(void)
{
    GetCapFormPcs();
    RefreshHistoryData();
}

void CapInitl(void)
{
    uint32_t  temp_data;
    uint32_t  temp_data1;

    DevRtcReadTime(&gstRtcTime);
    RtcDat2Covert2Sec(&gstRtcTime);
    _gstOldRtcTime = gstRtcTime;
    GetHistoryCapInfoFromFlash(gstRtcTime);   //上电从FLASH 里边获取容量信息

    if(gChangeFlag == 0)
    {
        if(gTodayValueExisFlag == 1)
        {
            gstTmpCapStorgeInfo.pv_generation_cap_sum = gstTodayCaPStorgeInfo.pv_generation_cap_sum;             //临时总量赋值
            gstTmpCapStorgeInfo.purchase_cap_sum = gstTodayCaPStorgeInfo.purchase_cap_sum;                       //临时总量赋值
            gstTmpCapStorgeInfo.load_consum_cap_sum = gstTodayCaPStorgeInfo.load_consum_cap_sum;                 //临时总量赋值
            gstTmpCapStorgeInfo.selling_cap_sum = gstTodayCaPStorgeInfo.selling_cap_sum;                         //临时总量赋值
            gstTmpCapStorgeInfo.batt_chg_cap_sum = gstTodayCaPStorgeInfo.batt_chg_cap_sum;                       //临时总量赋值
            gstTmpCapStorgeInfo.batt_dsg_cap_sum = gstTodayCaPStorgeInfo.batt_dsg_cap_sum;                       //临时总量赋值
            gstTmpCapStorgeInfo.remote_load_cap_sum = gstTodayCaPStorgeInfo.remote_load_cap_sum;                 //临时总量赋值
            gstTmpCapStorgeInfo.gen_elec_cap_sum = gstTodayCaPStorgeInfo.gen_elec_cap_sum;                       //临时总量赋值
            gstTmpCapStorgeInfo.ev_chg_cap_sum = gstTodayCaPStorgeInfo.ev_chg_cap_sum;                           //临时总量赋值
            gstTmpCapStorgeInfo.ev_dsg_cap_sum = gstTodayCaPStorgeInfo.ev_dsg_cap_sum;                           //临时总量赋值
        }
        else
        {
            gstTmpCapStorgeInfo.pv_generation_cap_sum = gstStartTodayCaPStorgeInfo.pv_generation_cap_sum;          //临时总量赋值
            gstTmpCapStorgeInfo.purchase_cap_sum = gstStartTodayCaPStorgeInfo.purchase_cap_sum;                    //临时总量赋值
            gstTmpCapStorgeInfo.load_consum_cap_sum = gstStartTodayCaPStorgeInfo.load_consum_cap_sum;              //临时总量赋值
            gstTmpCapStorgeInfo.selling_cap_sum = gstStartTodayCaPStorgeInfo.selling_cap_sum;                      //临时总量赋值
            gstTmpCapStorgeInfo.batt_chg_cap_sum = gstStartTodayCaPStorgeInfo.batt_chg_cap_sum;                    //临时总量赋值
            gstTmpCapStorgeInfo.batt_dsg_cap_sum = gstStartTodayCaPStorgeInfo.batt_dsg_cap_sum;                    //临时总量赋值
            gstTmpCapStorgeInfo.remote_load_cap_sum = gstStartTodayCaPStorgeInfo.remote_load_cap_sum;              //临时总量赋值
            gstTmpCapStorgeInfo.gen_elec_cap_sum = gstStartTodayCaPStorgeInfo.gen_elec_cap_sum;                    //临时总量赋值
            gstTmpCapStorgeInfo.ev_chg_cap_sum = gstStartTodayCaPStorgeInfo.ev_chg_cap_sum;                        //临时总量赋值
            gstTmpCapStorgeInfo.ev_dsg_cap_sum = gstStartTodayCaPStorgeInfo.ev_dsg_cap_sum;                        //临时总量赋值
        }
        gChangeFlag = 1;
    }
}

void CapCtrl(void)
{
    if(GetUpdataStatus() == 1)
    {
        //升级中 不进行历史数据统计
        return;
    }

    static  uint32_t  old_rtc_stamp;
    gstRtcTime.time_stamp = GetRtcTimeStamp();
//获取到时间戳然后转为时间
    RtcSecConvert2Date(&gstRtcTime);
    if(old_rtc_stamp == gstRtcTime.time_stamp)
    {
        return;
    }
    old_rtc_stamp = gstRtcTime.time_stamp;
//每隔1S进来一次

    if(gOldRtcUpdtaFlag == 1)
    {
        gOldRtcUpdtaFlag = 0;//系统重新校准时间，需要将_gstOldRtcTime也进行校准
        _gstOldRtcTime = gstRtcTime;

    }
    //获取当前pcs传过来的值
    gstCurrentCapPcsInfo.pv_total = GetPVCapTotal();
    gstCurrentCapPcsInfo.purchase_total = GetPcsPurchaseTotal();
    gstCurrentCapPcsInfo.load_total = GetPcsLoadTotal();
    gstCurrentCapPcsInfo.selling_total = GetPcsSaleTotal();
    gstCurrentCapPcsInfo.batt_chg_total = GetBatteryChgTotal();
    gstCurrentCapPcsInfo.batt_dsg_total = GetBatteryDschgTotal();
    gstCurrentCapPcsInfo.remote_load_total = GetRemoteLoadTotal();
    gstCurrentCapPcsInfo.gen_elec_total = GetGenTotal();
    gstCurrentCapPcsInfo.ev_chg_total = GetEVChgTotal();
    gstCurrentCapPcsInfo.ev_dsg_total = GetEVDsgTotal();

    if(GetTurnOffStatus() == 1) //关机前进行存储
    {
        gWriteCapFlag = 1;
    }
    else   //统计当天产生的量 每半小时累加总量
    {
        if(gstRtcTime.time_stamp >= _gstOldRtcTime.time_stamp)
        {
            if((gstRtcTime.minute % 30 == 0))
            {
                //kk=1
                if((gstRtcTime.second % 15 == 0) && (gstRtcTime.second != 0))
                {
                    gCleanPcsCapFlag = 1;
                }

                if((gstRtcTime.second > 10) && (gstRtcTime.second < 15))
                {
                    gWriteCapFlag = 1;
                    gRefreshCapFlag = 1;
                }
            }
            else
            {
                gRefreshCapFlag = 0;
            }
        }
    }

    if(gCleanPcsCapFlag == 1)
    {
        gCleanPcsCapFlag = 0;
        CleanPcsCapValue();
#if CAP_DEBUG_TEST
        pv_sum = 0;
        buy_sum = 0;
        sell_sum = 0;
        eps_sum = 0;
        batt_chg_sum = 0;
        batt_dsg_sum = 0;
        remote_sum = 0;
        gen_elec_sum = 0;
        ev_chg_sum = 0;
        ev_dsg_sum = 0;
#endif
    }

    if(gWriteCapFlag == 1)
    {
        gWriteCapFlag = 0;

        if(CapacityStorageFun(gstCapStorgeInfo) == true)
        {
            if(gRefreshCapFlag == 0)
            {
                CalculateNewCap();
            }
        }

        if(_gstOldRtcTime.day != gstRtcTime.day)
        {
            CapacityStorageOldDayFun(gstCapStorgeInfo);
            GetHistoryCapInfoFromFlash(gstRtcTime);   //最小的颗粒度是天，因此在日期变化的时候，需要更新下历史用电信息。
            //校准存储值校准
            gstTmpCapStorgeInfo.pv_generation_cap_sum = gstCapStorgeInfo.pv_generation_cap_sum;
            gstTmpCapStorgeInfo.purchase_cap_sum = gstCapStorgeInfo.purchase_cap_sum;
            gstTmpCapStorgeInfo.load_consum_cap_sum = gstCapStorgeInfo.load_consum_cap_sum;
            gstTmpCapStorgeInfo.selling_cap_sum = gstCapStorgeInfo.selling_cap_sum;
            gstTmpCapStorgeInfo.batt_chg_cap_sum = gstCapStorgeInfo.batt_chg_cap_sum;
            gstTmpCapStorgeInfo.batt_dsg_cap_sum = gstCapStorgeInfo.batt_dsg_cap_sum;
            gstTmpCapStorgeInfo.remote_load_cap_sum = gstCapStorgeInfo.remote_load_cap_sum;
            gstTmpCapStorgeInfo.gen_elec_cap_sum = gstCapStorgeInfo.gen_elec_cap_sum;
            gstTmpCapStorgeInfo.ev_chg_cap_sum = gstCapStorgeInfo.ev_chg_cap_sum;
            gstTmpCapStorgeInfo.ev_dsg_cap_sum = gstCapStorgeInfo.ev_dsg_cap_sum;

            gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day = 0;
            gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day = 0;
            gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day = 0;
            gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day = 0;
            gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_day = 0;
            gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_day = 0;
            gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day = 0;
            gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_day = 0;
            gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_day = 0;
            gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_day = 0;

            if(_gstOldRtcTime.month != gstRtcTime.month)
            {
                gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month = 0;
                gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month = 0;
                gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month = 0;
                gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month = 0;
                gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month = 0;
                gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month = 0;
                gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month = 0;
                gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month = 0;
                gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month = 0;
                gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month = 0;
            }
            if(_gstOldRtcTime.year != gstRtcTime.year)
            {
                gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_year = 0;
                gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_year = 0;
                gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_year = 0;
                gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_year = 0;
                gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_year = 0;
                gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_year = 0;
                gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_year = 0;
                gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_year = 0;
                gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_year = 0;
                gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_year = 0;
            }
            _gstOldRtcTime = gstRtcTime;
        }
    }
    else
    {
        //时间未到，使用之前的数据进行送显
        if(gRefreshCapFlag == 0)
        {
            CalculateNewCap();
        }
    }

    if(erase_test == 1)
    {
        erase_test = 0;
        SpiFlashAutoEraseSectors(0x258000, (0x2000 * 12 * 5)); //0X1000 ->4k 半个月数据 *2=一个月 *12=一年数据 *4=4年数据   2024开始-2028年底       0x1000
    }
}



