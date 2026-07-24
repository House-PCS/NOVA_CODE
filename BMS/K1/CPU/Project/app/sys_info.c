
#include "sys_info.h"
#include "dev_spiflash.h"
#include "flash.h"
#include "rtc.h"
#define FLASH_PAGE_SIZE                         (256)

#define MAIN_CTL_FACTORY_INFO_FLASH_ADDR        (0x00100000)
static uint8_t ver[3] = {1, 3, 10};
SysInfo_t gSysInfo;

/*ModelID和Sn号写入FLASH*/
void DeviceBaseInfoWrite()
{
    uint8_t buf[256];
    uint8_t valid_flag[4];
    uint8_t i = 0;

    for(i = 0; i < 4; i++)
    {
        valid_flag[i] = 0x0A;
    }

    for(i = 0; i < 2; i++)//第二次循环是数据备份
    {
        spi_flash_auto_erase_sector(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i, 4096);
        memcpy(buf, (uint8_t*)&gSysInfo.model_id, 2);
        spi_flash_write_bytes_without_erase(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE, buf, 2);//MODELID
        memcpy(buf, (uint8_t*)gSysInfo.sn, sizeof(gSysInfo.sn));
        spi_flash_write_bytes_without_erase(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE * 2, buf, sizeof(gSysInfo.sn));//SN
        spi_flash_write_bytes_without_erase(MAIN_CTL_FACTORY_INFO_FLASH_ADDR, valid_flag, 4);
    }
}

/*ModelID和Sn号从FLASH读出*/
void DeviceBaseInfoRead()
{
    uint8_t buf[256];
    uint8_t i = 0;

    for(i = 0; i < 2; i++)//第二次循环是数据备份
    {
        spi_flash_read_bytes(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i , buf, 4);
        if(buf[0] == 0x0A && buf[1] == 0x0A && buf[2] == 0x0A && buf[3] == 0x0A)
        {
            break;
        }
    }
    if(i == 2)/*数据无效，使用默认值*/
    {
        gSysInfo.model_id = 0xC282;
        memset(gSysInfo.sn, 0x30, 20);
        sprintf(gSysInfo.sn, "BZSA17466W0X20000005");
    }
    else
    {
        spi_flash_read_bytes(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE, buf, 256);//ModelID
        memcpy((uint8_t*)&gSysInfo.model_id, buf, 2);
        spi_flash_read_bytes(MAIN_CTL_FACTORY_INFO_FLASH_ADDR + 0x1000 * i + FLASH_PAGE_SIZE * 2, buf, 256);//SN
        memcpy(gSysInfo.sn, buf, sizeof(gSysInfo.sn));
    }
    spi_flash_read_bytes(0x1000 + 256, buf, 4);
}

#define HISTORY_CAP_OFFSET_ADDR         0x400000
const uint16_t  DayMin[48] =
{
    0,  30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 330, 360, 390, 420, 450, 480, 510, 540, 570, 600, 630, 660, 690,
    720, 750, 780, 810, 840, 870, 900, 930, 960, 990, 1020, 1050, 1080, 1110, 1140, 1170, 1200, 1230, 1260, 1290, 1320, 1350, 1380, 1410
};//存储一天中的分钟
bool   GetCapAddr(RtcDateInfo_t *pDate, uint32_t *p_addr)
{

    uint16_t    min_data = 0;
    uint8_t     i = 0;
    uint32_t    write_start_addr;
    uint32_t    temp_data = 0;

    min_data = (pDate->hour * 60 + pDate->minute);
    write_start_addr = HISTORY_CAP_OFFSET_ADDR;
    if(pDate->year < 2023)
    {
        return false;
    }
    temp_data = (pDate->year - 2023);
    temp_data <<= 16;
    temp_data *= 3;
    write_start_addr += temp_data;

    temp_data = (pDate->month - 1);
    temp_data   <<= 14;
    write_start_addr += temp_data;

    temp_data = (pDate->day - 1);
    temp_data <<= 9;
    write_start_addr += temp_data;

    for(i = 0; i < 48; i++)
    {
        if(min_data == DayMin[i])
        {
            break;
        }
    }
    temp_data = i / 24;
    temp_data <<= 8;
    write_start_addr += temp_data;
    temp_data = i % 24;
    temp_data *= 10;
    write_start_addr += temp_data;
    *p_addr = write_start_addr;
    return true;
}





uint32_t  test_time_count = 0;
uint32_t  chg_cap_test = 0;
uint32_t  dsg_cap_test = 0;
uint32_t  soc_data = 0;
void    DeviceWriteCapInfo(uint32_t  time, uint32_t chg_cap, uint32_t dsg_cap, uint8_t soc_data)
{
    //先转化时间 year  month day  hour minute  seconed
    //找准数据位置   (year-2022)
    //读取是否之前已经有写过数据
    //写入数据，然后读取查看是否写成功
    //返回。
    static  uint8_t  write_flag = 0;
    uint16_t    min_data = 0;
    uint8_t     i = 0;
    uint32_t    write_start_addr;
    uint32_t    temp_data = 0;
    uint8_t     buf[10];
    uint8_t     len = 0;



    RtcDateInfo_t timr_data;
    if(time - test_time_count > 30)
    {
        test_time_count = time;
        chg_cap++;
        dsg_cap++;
        soc_data++;
        if(soc_data >= 100)
        {
            soc_data = 0;
        }
    }



    RtcSecCovert2Date(&timr_data, time); //将时间戳转换为日期

    if((timr_data.minute == 0) || (timr_data.minute == 30))
    {
        //写数据
        if(write_flag == 0)
        {
            write_flag = 1;
//此时已经是半小时颗粒度，需要进行数据写入。
//根据年 月 日  以及 MIN 确定写入地址。
//查表获取时间位置。
            if(GetCapAddr(&timr_data, &write_start_addr) == true)
            {

                buf[len++] = 0x5A;
                buf[len++] = soc_data;
                buf[len++] =  chg_cap & 0xFF;
                buf[len++] = (chg_cap >> 8) & 0xFF;
                buf[len++] = (chg_cap >> 16) & 0xFF;
                buf[len++] = (chg_cap >> 24) & 0xFF;
                buf[len++] =  dsg_cap & 0xFF;
                buf[len++] = (dsg_cap >> 8) & 0xFF;
                buf[len++] = (dsg_cap >> 16) & 0xFF;
                buf[len++] = (dsg_cap >> 24) & 0xFF;
                spi_flash_write_bytes_without_erase(write_start_addr, buf, 10);

            }
            else
            {
                write_flag = 0;
            }

        }
    }
    else
    {
        write_flag = 0;
    }
}
//根据时间戳读取数据
//1 将上位机发送回来的时间戳先转换为正点/半小时


//        RtcDateInfo_t timr_data;
uint32_t   cap_test_dat;
uint16_t    DeviceReadCapInfo(uint32_t start_time, uint32_t end_time, uint8_t *p_buf, uint16_t *p_data_size)//pbuf--数组小于1024
{
//
    uint16_t    data_count;
    uint16_t    half_hour_secs_add = 0;
    uint32_t    temp_data = 0;
    uint8_t     i = 0;
    uint16_t    min_data = 0;
    uint32_t    timr_sec;
    uint32_t    flash_cap_addr;
    RtcDateInfo_t timr_data;



    if(end_time < start_time)
    {
        return 0;
    }
    RtcSecCovert2Date(&timr_data, end_time);
    if(timr_data.minute < 30)
    {
        timr_data.minute = 0;
    }
    else
    {
        timr_data.minute = 30;
    }
    timr_data.second = 0;
    end_time = RtcDat2Covert2Sec(&timr_data);


    RtcSecCovert2Date(&timr_data, start_time);
    if(timr_data.minute < 30)
    {
        if((timr_data.minute == 0) && (timr_data.second == 0))
        {
            half_hour_secs_add = 0;
        }
        else
        {
            half_hour_secs_add = 1800;
        }
        timr_data.minute = 0;
    }
    else
    {
        timr_data.minute = 30;
    }
    timr_data.second = 0;
    start_time = RtcDat2Covert2Sec(&timr_data);

    start_time = start_time + half_hour_secs_add; //从下个半小时开始计数

    temp_data = (end_time - start_time);
    temp_data /= 1800;
    data_count = temp_data + 1;  //获取到数据个数
    if(data_count > 49)
    {
        *p_data_size = 0;
        return 0;
    }

    for(i = 0; i < data_count; i++)
    {

        timr_sec = (start_time + 1800 * i);
        RtcSecCovert2Date(&timr_data, timr_sec);
//获取位置读取数据。
        if(GetCapAddr(&timr_data, &flash_cap_addr))
        {
//                spi_flash_read_bytes(flash_cap_addr, p_buf, 10);
//                spi_flash_read_bytes(flash_cap_addr, (p_buf+i*10+4), 10);
            *(p_buf + i * 14)      =    timr_sec & 0xFF;
            *(p_buf + i * 14 + 1)  =   (timr_sec >> 8) & 0xFF;
            *(p_buf + i * 14 + 2)  =   (timr_sec >> 16) & 0xFF;
            *(p_buf + i * 14 + 3)  =   (timr_sec >> 24) & 0xFF;
//            spi_flash_read_bytes(flash_cap_addr, (p_buf + i * 14 + 4), 10);

            *(p_buf + i * 14 + 4) = 0x5a;
            *(p_buf + i * 14 + 5) = i;
            cap_test_dat = i * 100ul; //充电量
            *(p_buf + i * 14 + 6) = cap_test_dat & 0xFF;
            *(p_buf + i * 14 + 7) = (cap_test_dat >> 8) & 0xFF;
            *(p_buf + i * 14 + 8) = (cap_test_dat >> 16) & 0xFF;
            *(p_buf + i * 14 + 9) = (cap_test_dat >> 24) & 0xFF;
            cap_test_dat = i * 200ul; //放电量
            *(p_buf + i * 14 + 10) = cap_test_dat & 0xFF;
            *(p_buf + i * 14 + 11) = (cap_test_dat >> 8) & 0xFF;
            *(p_buf + i * 14 + 12) = (cap_test_dat >> 16) & 0xFF;
            *(p_buf + i * 14 + 13) = (cap_test_dat >> 24) & 0xFF;



        }
    }
    *p_data_size = data_count * 14;

    return 1;
}


char uBinaryToAscii(unsigned int value, char *str)
{
    char i;
    char num_str[10] = { 0 };
    unsigned int quotient;
    unsigned char len;

    len = 0;

    quotient = value / 10;
    while (quotient != 0)
    {
        num_str[len] = value % 10 + '0';
        len++;
        value = quotient;
        quotient = value / 10;
    }
//    if (value != 0)
//    {
    num_str[len] = value + '0';
    len++;
//    }
    for (i = 0; i < len; i++)
    {
        *str = num_str[len - i - 1];
        str++;
    }

    return len;
}

static void _SysSw_HwInfoGet()
{
//    FlashRead(3, ver, 0x8007703);       //读主控软件版本号
    gSysInfo.sw_version = (ver[0] << 12) | (ver[1] << 6) | ver[2];
    gSysInfo.hw_version = 0;
}

/*初始化调用*/
void SysInfoInit()
{
    DeviceBaseInfoRead();
    _SysSw_HwInfoGet();
}

void SysRtcAlarmFun(void)
{
    DeviceWriteCapInfo(RtcGetTime(), gPackStatus.chg_accumulate_capa_wh, \
                       gPackStatus.dsg_accumulate_capa_wh, (uint8_t)gPackStatus.sox_info.soc_disp);
    SoxInfoSave(&gPackStatus);
    ModbusRegSave();
    SetRtcAlarm();
}