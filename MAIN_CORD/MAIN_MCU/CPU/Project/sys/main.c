/*!
    \file    main.c
    \brief   led spark with systick

    \version 2016-08-15, V1.0.0, firmware for GD32F4xx
    \version 2018-12-12, V2.0.0, firmware for GD32F4xx
    \version 2020-09-30, V2.1.0, firmware for GD32F4xx
    \version 2022-03-09, V3.0.0, firmware for GD32F4xx
*/

/*
    Copyright (c) 2022, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32f4xx.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "bsp.h"
#include "GUI.h"
#include "dev_modbus.h"
#include "dev_spiflash.h"
#include "dev_key.h"
#include "dev_it7c4337.h"
#include "dev_mbi502x_led.h"
#include "em_menu.h"
#include "dev_lcd.h"

#include "slave_bms.h"
#include "slave_pcs.h"
#include "slave_collect.h"
#include "firmware_upgrate.h"
#include "system_ctrl.h"
#include "timed_polling.h"
#include "sys_info_storage.h"
#include "tools_program.h"

#include "md5.h"
#include "miniz.h"
#include "program_sn.h"
#include "can_tx_rx_process.h"
#include <cm_backtrace.h>
#include "slave_meter.h"



#include "gd32f4xx.h"
#include "netconf.h"
#include "main.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"
#include "dev_udp_comm.h"
#include "hello_gigadevice.h"
#include "netconf.h"

#include "slave_bms.h"
#include "rtk_api_ext.h"


#include "rtl8309n_asicdrv_ext.h"
#include "slave_coprocessor.h"

#include "locale.h"
#define SYSTEMTICK_PERIOD_MS  10
__IO uint32_t g_localtime = 0; /* for creating a time reference incremented by 10ms */
uint32_t g_timedelay;






CanProtoListTxInfo_t gCanProtoListInfoFactoryTestTx;
CanProtoListRxInfo_t gCanProtoListInfoFactoryTestRx;
/*
20241203    已经可以通过UDP 和 BMS/PCS进行通讯。
20241204    已经解决多IP接收问题。
20240103    已经调通8304
20240106    编址代码完成。
20250109    和BMS UDP 通讯已经完成
20250114    采集器下发命令进行透传OK
20240115    使用modbus 调试通BMS 写数据
20250117    编址初步OK PCS数据接收解析还有些问题
20250208    完成了32bit 数据通讯转化。 验证可行。
20250211    修改了转发上位机下发命令偏移地址问题，和BMS已经验证，写入和读取数据都正常。
20250214    1 增加按键灯按下效果，以及开机时按键灯效果 2 增加LCD显示三相频率 电压 电流信息

20250225    1 已经和BMS调试通自动编址功能
20250227    1 进行emwin中的部分寄存器修改和确定，并按照科2UI界面需求修改界面
20250228    1 修改电量统计的.c文件，后续还需修改未确定的寄存器

20250228    1 调试升级
20250307    1 程序加入drm检测。
20250310    1 优化界面显示 完善待确定的寄存器对应关系
20250312    1 更改了寄存器地址读取的bug.  2 加入了氛围灯控制存储
20250313    1 优化升级程序。
            2 氛围灯已经调试OK
20250314    1 加入了电池PACK均衡控制,氛围灯广播调试OK
20250319    1 按照科2的故障指示更新代码--更新已经完成。
20250321    1 修改电池充放电功率、并网功率最大到16000 2 优化界面的显示，调整布局
20250325    1 优化了升级代码， 控制程序中去掉了控制电压和电流相关代码
20250326    1 移植电量统计代码
20250403    1 增加了源代码版本号 2 升级错误码更新
20250414    1 增加了FPGA升级。2 lwip由5ms基准改为10ms
20250509    1 修改单相显示功率
20250512	1 修复测试模式向PCS数据下发有误bug
20250529    1 在高级设置->系统设置->界面下添加油机并网时的功率（寄存器暂未确定） 2 修改两个宏的警告(udp write read)
20250603    1 修改PV3显示问题 2 修改电池信息中Battery替换为Pack
20250604    1 修改油机功率显示退上一页的问题
20250609    1 新增DCDC错误在LCD屏上显示
---------------------------------------------------------------------------------------------------------------------------------
20250610    1 修改pack信息中pack功率显示所用的寄存器 改为pcs端的电池功率
            2 复获取两个以上的Pack数据时，所有Pack信息只会放到前两个结构体中的问题
---------------------------------------------------------------------------------------------------------------------------------
20250910    1.修复DRM引脚错位
            2.修复DRM帧检测结果异常
            3.增加二级均衡，均衡状态改为15S更新一次
            4.修复屏幕电池放电功率设置后千位变为1的bug
            5.单包SOC>90%时，充电功率系数改为0.8
            6.PCS输入寄存器增加拉弧版本号寄存器
            7.PCS保持寄存器增加入户功率设置寄存器、油机功率寄存器
            8.增加SOC融合算法
            9.修复485接口只能读取256格寄存器的bug
            10.更新PackInfo寄存器
            11.增加Pack离线判断
            12.修复Pack信息没有复制完全的bug
---------------------------------------------------------------------------------------------------------------------------------
20250910    1 新增入户网侧功率限制界面（lxt）
20250911    1 修改PACK信息寄存器，并替换bms中的某些寄存器为PACK中上报寄存器
---------------------------------------------------------------------------------------------------------------------------------
20251014    1.修复充放电标志一直关闭的问题
            2.增加并网功率限制寄存器
            3.向采集器汇报时不再单独汇报BMS数据
            4.增加重启测试代码
20251014    1 新增PV用电量统计寄存器和油机功率寄存器 2 对油机功率下发界面的寄存器完善
20251020    1 修改单逆变器下pcs无法进行逆变的问题     监控卡版本1.0.0
20251203    1 完成监控卡界面下发任务 2 完成满充时平滑操作 3 完成错误码的显示 4 使用新协议适配的第一次调试   监控版本->1.0.1
20251215    1 完成监控卡新界面的调试 2 完成电量统计的补充和自测试 3 修改部分界面中所用寄存器的地址修改 
20251217    1 修复均衡状态下发组包问题 2 修改基础信息中产品mode显示 3 修改往bms发送灯状态颜色数据异常问题
---------------------------------------------------------------------------------------------------------------------------------
20251225    1.DRM改为低电平触发
            2.增加工装寄存器
            3.增加协处理器寄存器
            4.调整PCS寄存器，增加PCS协处理器寄存器
            5.增加向DCDC通知所有Pack的平均电压
            6.增加LLC开启寄存器及LLC开启命令
            7.删除旧版PCS及BMS寄存器
            8.增加监控卡协处理器/PCS协处理器升级标志
            9.DCDC故障不再停止充放电  
20251229    1 修改PV线条不显示问题   T2转测
---------------------------------------------------------------------------------------------------------------------------------
20260108    1.增加工装测试代码
            2.修复电池功率显示
            3.增加协处理器升级
            4.更新恢复出厂设置默认值
            5.更新监控卡内部寄存器格式
20260109    1 修改PV显示界面的电压电流功率顺序 2 新增协处理器上报数据段
20260112    1.修复界面显示负载测功率为单项功率的问题
            2.取消外部Flash存储的版本号数据
            3.恢复出厂设置默认值更新
20260113    1 修改监控卡网络状态寄存器 2 修改恢复出厂默认值bug
20260114    1 修改电网线条变换问题 2 新增升级时发送协处理器的提示 3 修改电池放电深度被覆盖问题
20260123    1 修改设备基础信息栏中产品名称显示 2 新增关机提示界面
20260205    1 新增工装PCS串口调试代码 验证继电器IO是否可控 已OK
20260206    1 对PCS故障码进行重映射
20260210    1 合并嘉辰代码（升级代码 电池信息界面中使用DCDC端功率电流值 以及PCS端下发等信息）
20260305    1 修改温度界面中摄氏度的显示方式 2 修改PACK信息界面中的电压、电流、功率选取寄存器
20260310    1 修改主界面的电池功率选取寄存器  2 增加远程待机功能 
            3 新增监控卡、pcs、arc、协处理器、DCDC版本显示界面
            4 修改电池相关的故障码进行显示 5 增加按键关机信号到PCS
20260312    1 完善核对监控卡与pcs之间的故障码显示 2 增加电芯芯片断线检测的故障码  监控卡版本->0.1.3
20260313    1 修改监控卡下发PCS恢复出厂设置的下发值
20260317    1 修改监控卡获取最大BMS数量为6个所涉及的.c文件
20260318    1 优化主机面负载显示 2 版本转测->0.1.3
20260324    1 修改温度字体显示不全的问题
20260326    1 删除状态信息栏中的PV状态list 2 通过电池总有功功率的正负来判断电池是充电还是放电模式
20260327    1 修改主界面中的功率显示，将单位统一为0.1kW 2 优化单个电池包SOC小于5%的策略，改由BMS控制是否充放电
20260330    1 增加认证相关的4个限制开关及界面并添加所增加的寄存器 2 修复小于3个电池包的显示策略
20260331    1 修改界面中资源列表中的const变量为__SDRAM变量
20260402    1 修改监控卡端氛围灯与BMS端氛围灯不同步的问题
20260407    1 修改UART底层驱动代码 2 新增监控卡下发SOC给协处理器的寄存器，用于氛围灯显示SOC值
20260413    1 修改触发关机后背光处于打开的状态 2 修改关机后油机、PV还处于打开时的阈值确定
---------------------------------------------------------------------------------------------------------------------------------
20260414    1 修复FPGA版本显示异常
            2 增加DSP故障停止充放电判断
20260416    1 根据科二界面需求修改所涉及的界面更改和单词拼写
20260420    1 修改最大soc和最小soc差5%开始均衡
20260422    1 修改削峰填谷模式下买卖电时间段内都可以进行充放电
            2 核对界面更改点中的漏网之鱼
20260423    1 与DSP之间的配合出现问题重新修改方案，使用功率是否为0来控制是否充放电
            2 监控卡通过SOC值给DSP下发backup使能开关寄存器,让其开关负载继电器
20260424    1 修改界面排布，删除不用的界面 2 修复回读BMS和FPGA版本有误问题
20260427    1 修复Pack间均衡问题
20260428    1 修改监控卡获取到所有SOC的值后进行放电深度判断
            2 修改并网功率显示的数字加减有误问题
20260429    1 增加欠温一级欠温二级错误寄存器
20260507    1 修改恢复出厂设置后，需关机重启后才能触发
20260508    1 修改设置密码错误后重新输入数字上下翻异常问题 2 修改未完全读到BMS信息前不进行均衡操作
            3 修改多pack之间均衡不一致问题
20260513    1 版本升级 V0.1.4
20260514    1 修改与协处理器之间的灯效寄存器 2 版本转测 V0.1.4
---------------------------------------------------------------------------------------------------------------------------------

*/


/*!
    \brief    main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
//0x4000000
//uint8_t test1[4096];
#define OFFSET_VECTORTABLE      ((uint32_t)0x08010000)//0x08010000 

#define TEST_CASE  0
#if TEST_CASE

#define my_max(a,b) (((a) > (b)) ? (a) : (b))
#define my_min(a,b) (((a) < (b)) ? (a) : (b))
#define BUF_SIZE (1024)

#if 0
int32_t test_zlib()
{

#define TEST_MD5        1

    uint32_t spi_addr = (4096 * 4096 + 727 * 4096);
    // Decompression.
    z_stream stream;
    uint32_t infile_remaining = 133835;
    uint32_t infile_total = 0;

    uint8_t s_inbuf[BUF_SIZE];
    uint8_t s_outbuf[BUF_SIZE];


#if TEST_MD5
    MD5_CTX mdContext[2];
    uint8_t md5[2][16];

    MD5Init(&mdContext[0]);
    MD5Init(&mdContext[1]);
#endif


    memset(&stream, 0, sizeof(stream));
    stream.next_in = s_inbuf;
    stream.avail_in = 0;
    stream.next_out = s_outbuf;
    stream.avail_out = BUF_SIZE;

    uint32_t cost_time[3][2] = {{0}};

    cost_time[0][0] = tim_get_systime();
    if (inflateInit(&stream))
    {
        return -1;
    }

    for ( ; ; )
    {
        int32_t status;
        if (!stream.avail_in)
        {
            // load buf from spi flash
            // Input buffer is empty, so read more bytes from input file.
            uint32_t n = my_min(BUF_SIZE, infile_remaining);

            cost_time[1][0] = tim_get_systime();
            SpiFlashReadData(spi_addr + infile_total, s_inbuf, n);
            cost_time[1][1] = tim_get_systime();
            //cost_time[1][1] = cost_time[1][1] - cost_time[1][0]; // read 1024 time

#if TEST_MD5
            MD5Update(&mdContext[0], s_inbuf, n);
#endif

            stream.next_in = s_inbuf;
            stream.avail_in = n;

            infile_remaining -= n;
            infile_total += n;
        }

        cost_time[2][0] = tim_get_systime();
        status = inflate(&stream, Z_SYNC_FLUSH);
        cost_time[2][1] = tim_get_systime();
        //cost_time[2][1] = cost_time[2][1] - cost_time[2][0];  // decode time

        if ((status == Z_STREAM_END) || (!stream.avail_out))
        {
            // Output buffer is full, or decompression is done, so write buffer to output file.
            uint32_t n = BUF_SIZE - stream.avail_out;

#if TEST_MD5
            MD5Update(&mdContext[1], s_outbuf, n);
#endif

            stream.next_out = s_outbuf;
            stream.avail_out = BUF_SIZE;
        }

        if (status == Z_STREAM_END)
        {
#if TEST_MD5
            MD5Final(md5[0], &mdContext[0]);
            MD5Final(md5[1], &mdContext[1]);
#endif
            break;
        }
        else if (status != Z_OK)
        {
            //printf("inflate() failed with status %i!\n", status);
            return -2;
        }
    }

    if (inflateEnd(&stream) != Z_OK)
    {
        //printf("inflateEnd() failed!\n");
        return -3;
    }

    cost_time[0][1] = tim_get_systime();
    //cost_time[0][1] = cost_time[0][1] - cost_time[0][0];

    return 0;
}
#endif

#if 1

typedef struct _MyZipCtx_t
{
    mz_zip_archive zip_ctx;
    uint32_t zip_size;
    MD5_CTX zip_md5_ctx;
    uint8_t zip_md5[16];
    mz_uint zip_flags;


    mz_uint32 file_idx;
    MD5_CTX file_md5_ctx;
    char file_name[256];
    uint8_t file_md5[16];

} MyZipCtx_t;

static size_t mz_zip_read_func(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n)
{
    mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
    size_t s = (file_ofs >= pZip->m_archive_size) ? 0 : (size_t)MZ_MIN(pZip->m_archive_size - file_ofs, n);

    // read data from spi flash
    uint32_t spi_addr = (4096 * 4096 + 727 * 4096);
    SpiFlashReadData(spi_addr + file_ofs, pBuf, s);

    /*
    MyZipCtx_t *my_zip_ctx = (MyZipCtx_t *)pZip->userdata;
    MD5Update(&my_zip_ctx->zip_md5_ctx, pBuf, s);
    */

    return s;
}


static size_t mz_file_write_cb(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n)
{
    mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;

    // calc md5
    MyZipCtx_t *my_zip_ctx = (MyZipCtx_t *)pZip->userdata;




    MD5Update(&my_zip_ctx->file_md5_ctx, (unsigned char *)pBuf, n);


    return n;
}


int32_t test_zip()
{
#define TEST_MD5        1

    uint32_t spi_addr = (4096 * 4096 + 727 * 4096);

    static uint8_t __SDRAM write_buf[TINFL_LZ_DICT_SIZE] = {0};

    uint8_t s_inbuf[BUF_SIZE];
    uint8_t s_outbuf[BUF_SIZE];

    mz_bool status;
    mz_bool success = MZ_TRUE;
    mz_zip_error zip_err;
    MyZipCtx_t my_zip_ctx = {0};

    mz_zip_zero_struct(&my_zip_ctx.zip_ctx);
    my_zip_ctx.zip_size = 61663;
    MD5Init(&my_zip_ctx.zip_md5_ctx);
    my_zip_ctx.zip_flags = MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY;

    my_zip_ctx.zip_ctx.userdata = (void *)&my_zip_ctx;

    uint32_t cost_time[3][2] = {{0}};

    uint32_t file_ofs = 0, file_read = 0;
    while (file_ofs < my_zip_ctx.zip_size)
    {
        file_read = sizeof(s_outbuf);
        if (my_zip_ctx.zip_size - file_ofs < file_read)
        {
            file_read = my_zip_ctx.zip_size - file_ofs;
        }
        // read data from spi flash
        SpiFlashReadData(spi_addr + file_ofs, s_outbuf, file_read);
        MD5Update(&my_zip_ctx.zip_md5_ctx, s_outbuf, file_read);
        file_ofs += file_read;
    }
    MD5Final(my_zip_ctx.zip_md5, &my_zip_ctx.zip_md5_ctx);

    cost_time[0][0] = tim_get_systime();

    // init zip
    if (!mz_zip_reader_init_cb(&my_zip_ctx.zip_ctx, NULL, my_zip_ctx.zip_size,
                               my_zip_ctx.zip_flags, mz_zip_read_func))
    {
        zip_err = my_zip_ctx.zip_ctx.m_last_error;
        return MZ_FALSE;
    }

    char file_name[256] = {"icon_upg_Roca_G2_sub_0_20240219.bin"};
    snprintf(my_zip_ctx.file_name, sizeof(my_zip_ctx.file_name), "%s", file_name);
    if (!mz_zip_reader_locate_file_v2(&my_zip_ctx.zip_ctx, my_zip_ctx.file_name, NULL,
                                      my_zip_ctx.zip_flags, &my_zip_ctx.file_idx))
    {
        return MZ_FALSE;
    }

    MD5Init(&my_zip_ctx.file_md5_ctx);

#if 0
    if (!mz_zip_reader_extract_to_callback(&my_zip_ctx.zip_ctx, my_zip_ctx.file_idx,
                                           mz_file_write_cb, &my_zip_ctx.zip_ctx,
                                           my_zip_ctx.zip_flags))
#else
    if (!mz_zip_reader_extract_to_callback_stack(&my_zip_ctx.zip_ctx, my_zip_ctx.file_idx,
            mz_file_write_cb, &my_zip_ctx.zip_ctx,
            my_zip_ctx.zip_flags, write_buf))
#endif
    {
        return MZ_FALSE;
    }

    MD5Final(my_zip_ctx.file_md5, &my_zip_ctx.file_md5_ctx);

    mz_zip_reader_end(&my_zip_ctx.zip_ctx);

    cost_time[0][1] = tim_get_systime();

    return 0;
}
#endif  // if 0
#endif  // TEST_CASE

#define TEST_CODE    0


/**
  * @brief   用于指示MCU运行状态
  * @param   None
  * @return  None
  * @note    None
  */
static void _McuWorkLed(void)
{
    static uint8_t sc_flag;
    static uint32_t sn_time;

    if(tim_get_systime() - sn_time > 500)
    {
        sn_time = tim_get_systime();
        if(sc_flag == 1)
        {
            GpioBitSet(&McuWorkLedHandle);
            sc_flag = 0;
        }
        else
        {
            GpioBitReset(&McuWorkLedHandle);
            sc_flag = 1;
        }
    }
}

/**
  * @brief   用于测试模式下的指示MCU运行状态
  * @param   None
  * @return  None
  * @note    None
  */
static void _TestModeMcuWorkLed(void)
{
    static uint8_t sc_test_flag;
    static uint32_t wait_time = 0;
    if(tim_get_systime() - wait_time > 200)
    {
        wait_time = tim_get_systime();
        if(sc_test_flag == 1)
        {
            GpioBitSet(&McuWorkLedHandle);
            sc_test_flag = 0;
        }
        else
        {
            GpioBitReset(&McuWorkLedHandle);
            sc_test_flag = 1;
        }
    }
}


static uint8_t __SDRAM test11[5] = {0};
uint32_t udp_send_time_out = 0;
uint8_t test_flag = 0;
uint32_t pvalue = 0;
uint32_t  time_delay = 0;


//rtk_port_phy_ability_t  phy_ability;
//uint32_t phy_port_num;
int main(void)
{
    __set_PRIMASK(1);   // 关闭总中断
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, OFFSET_VECTORTABLE);
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);  // 优先级分组
    systick_config();
    BaseBspInit();
    DebugInit();
    cm_backtrace_init("IntegratedMonitor", __DATE__, __TIME__);
//    DevSwitchLedInit();
    McuVersionTypeInit();
    DRMConnectIOInit();
    TimedPollingInit();
    if(gDeviceVersionType == DEVICE_TYPE_LED) //LED版本执行
    {

    }
    ExmcBspInit();

    DevSpiFlashInit();
    DevRtcInit();
    DevModbusRTUInit();
    DevKeyInit();
    DevMBI502xLedInit();
    ProgramFunInital();

    SystemCtrlInit();
    CommCollectInit();

    test11[0] = 1;


    __set_PRIMASK(0);   // 打开总中断
#if TEST_CASE
    //test_zlib();
    test_zip();
#endif
    ConnectGPIOBspInit();  //工装输入IO初始化
    OutputGPIOBspInit();   //工装输出IO初始化
//    DevSwitchLedInit();
//    SlaveBmsCommInit();
    SlaveCorproCommInit();
    FirmwareUpgrateInit();
    CommTimedPollingInit();
    if(gDeviceVersionType == DEVICE_TYPE_LCD) //LCD版本执行
    {
        EmMenuInit();
    }
    DevLcdBLOpen();
    McuWorkLedBspInit();
    ToolingTestRs485Init();  //工装测试Rs485通信使用

    enet_system_setup();
    /* initilaize the LwIP stack */
    lwip_stack_init();


    BmsUdpParaInit();
    DspUdpParaInit();
    
    //进入测试模式
#if (TEST_MODE == 2)
        gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode = TEST_MODE;
#endif 
    
#if(FWDGT_EN)
    fwdgt_write_enable();
    fwdgt_config(3125, FWDGT_PSC_DIV256); //3125*6,4ms=20S  //设置预分频 40K/256 一个周期是 6.4ms//设置初值
    fwdgt_counter_reload(); //喂狗
    fwdgt_enable(); //使能独立看门狗
#endif

    while(1)
    {
#if(FWDGT_EN)
        fwdgt_counter_reload();
#endif

        if((uint32_t)(tim_get_systime() - udp_send_time_out) > 1000)
        {
            udp_send_time_out = tim_get_systime();
//            test_flag = 1;
        }
        if(test_flag == 1)
        {
            //给UT测试用代码
            rtl8309n_phyReg_set(0, 0, 0, 0X2100);
            rtl8309n_phyReg_set(0, 24, 0, 0x310);
            rtl8309n_phyReg_set(0, 28, 0, 0x40C2);
            rtl8309n_phyReg_get(0, 0, 0, &pvalue);
            rtl8309n_phyReg_get(0, 16, 14, &pvalue);
            test_flag = 0;
        }


//        rtk_port_phyAutoNegoAbility_get(phy_port_num, &phy_ability);

//rtk_api_ret_t rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t 
//*pAbility)
//rtk_enable_t  Test_enable =ENABLED;
//uint32_t ppp;
//rtk_rate_igrBandwidthCtrlRate_get(1,&ppp,&Test_enable);

#ifndef USE_ENET_INTERRUPT
        /* check if any packet received */
        if(enet_rxframe_size_get())
        {
            /* process received ethernet packet */
            lwip_pkt_handle();
        }
#endif /* USE_ENET_INTERRUPT */

        /* handle periodic timers for LwIP */
#ifdef TIMEOUT_CHECK_USE_LWIP
        sys_check_timeouts();

#ifdef USE_DHCP
        lwip_dhcp_process_handle();
#endif /* USE_DHCP */

#else
        lwip_periodic_handle(g_localtime);
#endif /* TIMEOUT_CHECK_USE_LWIP */

        if(gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 1)  //工装测试代码
        {
            DevModbusDirCtlProcess();
            SlaveCollectModbusCommProcess();
            TestToolsProcess();
            _TestModeMcuWorkLed();
        }
        else
        {
            _McuWorkLed();
            if(gDeviceVersionType == DEVICE_TYPE_LCD)  //LCD版本执行
            {
                MenuPolling();
            }

            GetMeterAllValue();
            GetMeterCurrValue();
            CheckMeterConnectStaus();
            SystemCtrlProc();
        }
    }
}

void lwip_netif_status_callback(struct netif *netif)
{
    if((netif->flags & NETIF_FLAG_UP) != 0)
    {
        /* initilaize the helloGigadevice module telnet 23 */
        hello_gigadevice_init();
    }
}

/*!
    \brief      insert a delay time
    \param[in]  ncount: number of 10ms periods to wait for
    \param[out] none
    \retval     none
*/
void delay_10ms(uint32_t ncount)
{
    /* capture the current local time */
    g_timedelay = g_localtime + ncount;

    /* wait until the desired delay finish */
    while(g_timedelay > g_localtime)
    {
    }
}

/*!
    \brief      updates the system local time
    \param[in]  none
    \param[out] none
    \retval     none
*/
void time_update(void)
{
    g_localtime += SYSTEMTICK_PERIOD_MS;
}



