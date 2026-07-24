/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        ethernet_analysis.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-1-17
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-1-17    <td>V1.0.0    <td>             <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#include "ethernet_analysis.h"
#include "CH390_Interface.h"
#include "string.h"
#include "CH390.h"
#include "dev_led.h"
#include "stdio.h"
#include "string.h"
#include "pwm.h"
#include "queue.h"
#include "dev_flash.h"
#include "stdbool.h"
#include "mid_comservice.h"
#include "firmware_upgrate.h"

#define CH390_IP_ADDRESS0 (192)
#define CH390_IP_ADDRESS1 (168)
#define CH390_IP_ADDRESS2 (1)
#define CH390_IP_ADDRESS3 (9)
#define CH390_PORT  (2001)


#define DESTINATION_LEN  (6)    //广播地址长度
#define MAC_LEN  (6)            //MAC地址长度
#define IPV4_LEN  (2)           //IPV4地址长度
#define INTRENET_LEN  (20)      //网络层地址长度
#define APP_LEN  (8)            //应用层地址长度
#define DATA_LEN  (16)          //数据地址长度

//#define TEST_DATA_LEN (DESTINATION_LEN + MAC_LEN + IPV4_LEN + INTRENET_LEN + APP_LEN + DATA_LEN) //总长度
#define TEST_DATA_LEN (DESTINATION_LEN + MAC_LEN + IPV4_LEN + INTRENET_LEN + APP_LEN) //总长度

static uint16_t _garrReceiveCH390Buff[1400];
static uint16_t _garrSendCH390Buff[1400];
static uint16_t _garrSendMonitorBuff[1400];
static uint16_t _garrReceiveDataBuff[1400];
static uint16_t _garrDebugReceiveDataBuff[10];
static uint16_t _garrPacketData[1400];

const uint16_t _gdebugReveiveport[2] = {0x8,0xFC};


UdpPCSPara_t  gstUdpPcsPara;
MonitorUdp_t  gstMonitor;
static NetworkInfo_t _gstNetworkInfo;



void MonitorDataUpload(void)
{
    //结构体赋值

    gstUdpPcsPara.udp_pcs_upload.mode_id = gstVersionInfo.pcs_version_info.model_id;
    gstUdpPcsPara.udp_pcs_upload.software_ver = gstVersionInfo.pcs_version_info.soft_version;
    memcpy((uint16_t*)&gstUdpPcsPara.udp_pcs_upload.version_0, (uint16_t*)&gstVersionInfo.pcs_version_info.extern_version, (int)sizeof(gstVersionInfo.pcs_version_info.extern_version));

}

/**
* @brief: GetCH390MacAddress
* @param: void
* @return:Nothing
* @note:获取CH390的MAC地址
*/
void GetCH390MacAddress(void)
{
    ch390_get_mac((uint8_t*)_gstNetworkInfo.arr_ch390_mac_address);
}
/**
* @brief: SendArpProtocol
* @param: void
* @return:Nothing
* @note:发送ARP协议
*/
void SendArpProtocol(void)
{
    for(int i = 0; i < 6; i++)
    {
        _garrSendCH390Buff[i] = _gstNetworkInfo.arr_master_mac_address[i];  //发送主机的MAC地址
    }
    for(int i = 6; i < 12; i++)
    {
        _garrSendCH390Buff[i] = _gstNetworkInfo.arr_ch390_mac_address[i];  //发送本机的MAC地址
    }
    _garrSendCH390Buff[12] = 0x08; //ARP协议
    _garrSendCH390Buff[13] = 0x06;

    _garrSendCH390Buff[14] = 0x00; //硬件为以太网
    _garrSendCH390Buff[15] = 0x01;

    _garrSendCH390Buff[16] = 0x08;  //IPV4协议
    _garrSendCH390Buff[17] = 0x00;

    _garrSendCH390Buff[18] = 0x06;  //MAC大小6
    _garrSendCH390Buff[19] = 0x04;  //协议为4

    _garrSendCH390Buff[20] = 0x00;  //回复端
    _garrSendCH390Buff[21] = 0x02;

    for(int i = 22; i < 28; i++)
    {
        _garrSendCH390Buff[i] = _gstNetworkInfo.arr_ch390_mac_address[i - 22];  //发送本机的MAC地址
    }
    for(int i = 28; i < 32; i++)
    {
        _garrSendCH390Buff[i] = _gstNetworkInfo.arr_ch390_ip_address[i - 28];  //发送本机的IP地址
    }
    for(int i = 32; i < 38; i++)
    {
        _garrSendCH390Buff[i] = _gstNetworkInfo.arr_master_mac_address[i - 32];  //发送主机的MAC地址
    }
    for(int i = 38; i < 42; i++)
    {
        _garrSendCH390Buff[i] = _gstNetworkInfo.arr_master_ip_address[i - 38];  //发送主机的IP地址
    }
    ch390_send_packet((uint8_t*)_garrSendCH390Buff, 42);
}

/**
* @brief: _MonitorAnalysis
* @param: void                        
* @return: 
* @note: 
*
*/
static EN_ETH_ANALYSIS_STATUS _MonitorAnalysis(void)
{
    gstMonitor.DeviceType0 = _garrReceiveDataBuff[0]; //[7:0] 2 Byte位宽，设备类型。0x0000：逆变器模块；0x0001；Pack包模块。
    gstMonitor.DeviceType1 = _garrReceiveDataBuff[1]; //[15:8]
    gstMonitor.Reserved    = _garrReceiveDataBuff[2]; //[7:0] 1 Byte位宽，预留字节
    gstMonitor.OpCmd       = _garrReceiveDataBuff[3]; //[7:0] 1 Byte位宽，操作指令。0x45：写操作；0x43：读操作。
    gstMonitor.Addr0       = _garrReceiveDataBuff[4]; //[7:0] 4 Byte位宽，操作地址。表示读写指令需要操作的寄存器起始地址。
    gstMonitor.Addr1       = _garrReceiveDataBuff[5]; //[15:8]
    gstMonitor.Addr2       = _garrReceiveDataBuff[6]; //[23:16]
    gstMonitor.Addr3       = _garrReceiveDataBuff[7]; //[31:24]
    gstMonitor.Length0     = _garrReceiveDataBuff[8]; //[7:0] 2 Byte位宽，操作长度。表示数据用户数据总长度，该参数对应数据首部和负载有效数据长度总和。
    gstMonitor.Length1     = _garrReceiveDataBuff[9]; //[15:8]

    //获取监控下发的数据
    gstMonitor.Length = (uint16_t)gstMonitor.Length1 << 8 | (uint16_t)gstMonitor.Length0;
    gstMonitor.Length = ((gstMonitor.Length >= 1100) ? 1100 : gstMonitor.Length);
    gstMonitor.Addr = (uint32_t)gstMonitor.Addr3 << 24 | (uint32_t)gstMonitor.Addr2 << 16 | (uint32_t)gstMonitor.Addr1 << 8 | (uint32_t)gstMonitor.Addr0;

    if(gstMonitor.OpCmd == 0x43)//读操作
    {
        uint16_t* address = 0;
        uint16_t  num = 0;

        _garrSendMonitorBuff[0] = gstMonitor.DeviceType0; //[7:0] 2 Byte位宽，设备类型。0x0000：逆变器模块；0x0001；Pack包模块。
        _garrSendMonitorBuff[1] = gstMonitor.DeviceType1; //[15:8]
        _garrSendMonitorBuff[2] = gstMonitor.Reserved;    //[7:0] 1 Byte位宽，预留字节
        _garrSendMonitorBuff[3] = gstMonitor.OpCmd;       //[7:0] 1 Byte位宽，操作指令。0x45：写操作；0x43：读操作。
        _garrSendMonitorBuff[4] = gstMonitor.Addr0;       //[7:0] 4 Byte位宽，操作地址。表示读写指令需要操作的寄存器起始地址。
        _garrSendMonitorBuff[5] = gstMonitor.Addr1;       //[15:8]
        _garrSendMonitorBuff[6] = gstMonitor.Addr2;       //[23:16]
        _garrSendMonitorBuff[7] = gstMonitor.Addr3;       //[31:24]
        _garrSendMonitorBuff[8] = gstMonitor.Length0;     //[7:0] 2 Byte位宽，操作长度。表示数据用户数据总长度，该参数对应数据首部和负载有效数据长度总和。
        _garrSendMonitorBuff[9] = gstMonitor.Length1;     //[15:8]

        if((gstMonitor.Addr >= MODBUS_INPUT_REG_START_ADDR) && (gstMonitor.Addr <= MODBUS_INPUT_REG_END_ADDR))//监控的读操作
        {
            address = (uint16_t*)&gstUdpPcsPara.udp_pcs_upload;
            num = gstMonitor.Addr - MODBUS_INPUT_REG_START_ADDR;
            for(int i = 0; i < gstMonitor.Length; i++)//Monitor.Length实际寄存器数量,转换为字节数需要乘以2
            {
                _garrSendMonitorBuff[(i << 1) + 10] = (uint8_t)(*(address + i + num) & 0xFF);
                _garrSendMonitorBuff[(i << 1) + 11] = (uint8_t)(*(address + i + num) >> 8);
            }
            return EN_ETH_ANALYSIS_OK;
        }
        if(gstMonitor.Addr >= MODBUS_HOLD_REG_START_ADDR)
        {
            address = (uint16_t*)&gstUdpPcsPara.udp_pcs_hold;
            num = gstMonitor.Addr - MODBUS_HOLD_REG_START_ADDR;
            for(int i = 0; i < gstMonitor.Length; i++)//Monitor.Length实际寄存器数量,转换为字节数需要乘以2
            {
                _garrSendMonitorBuff[(i << 1) + 10] = (uint8_t)(*(address + i + num) & 0xFF);
                _garrSendMonitorBuff[(i << 1) + 11] = (uint8_t)(*(address + i + num) >> 8);
            }
            return EN_ETH_ANALYSIS_OK;
        }
    }

    if(gstMonitor.OpCmd == 0x45)
    {
        if((gstMonitor.Addr >= 0x24600000) && (gstMonitor.Addr <= 0x247FFFFF))//监控的在线升级(DSP、ARC)
        {
            EthernentUpgradeRecv(&gstMonitor, _garrReceiveDataBuff);
        }
        else if((gstMonitor.Addr >= MODBUS_HOLD_REG_START_ADDR) && (gstMonitor.Addr <= MODBUS_HOLD_REG_END_ADDR))//监控的写操作
        {
            uint8_t *set_address = 0;
            uint16_t  set_value = 0;

            set_address = (uint8_t*)&_garrReceiveDataBuff;

            for(int i = 0; i < gstMonitor.Length; i++)
            {
                set_value = (uint16_t)((*(set_address + 10 + (i << 1))) | (*(set_address + 11 + (i << 1)) << 8));
                switch(gstMonitor.Addr + i)//设置的目标地址
                {
                    case 0x6200:    //设置电池充电截止电压    0.1V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.set_batt_chg_volt)
                        {
                            gstUdpPcsPara.udp_pcs_hold.set_batt_chg_volt = set_value;
                        }
                        break;
                    }
                    case 0x6201:    //设置电池充电最大电流    0.1A
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.set_batt_chg_curr)
                        {
                            gstUdpPcsPara.udp_pcs_hold.set_batt_chg_curr = set_value;
                        }
                        break;
                    }
                    case 0x6202:    //设置电池充电最大功率    1.0W
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.set_batt_chg_power)
                        {
                            gstUdpPcsPara.udp_pcs_hold.set_batt_chg_power = set_value;
                        }
                        break;
                    }
                    case 0x6203:    //设置电池放电截止电压    0.1V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.set_batt_dsg_volt)
                        {
                            gstUdpPcsPara.udp_pcs_hold.set_batt_dsg_volt = set_value;
                        }
                        break;
                    }
                    case 0x6204:    //设置电池放电最大电流    0.1A
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.set_batt_dsg_curr)
                        {
                            gstUdpPcsPara.udp_pcs_hold.set_batt_dsg_curr = set_value;
                        }
                        break;
                    }
                    case 0x6205:    //设置电池放电最大功率    1.0W
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.set_batt_dsg_power)
                        {
                            gstUdpPcsPara.udp_pcs_hold.set_batt_dsg_power = set_value;
                        }
                        break;
                    }
                    case 0x6206:    //设置X-BOOST功能使能    1:使能 0:禁止
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.x_boost)
                        {
                            gstUdpPcsPara.udp_pcs_hold.x_boost = set_value;
                        }
                        break;
                    }
                    case 0x6207:    //设置Ups0毫秒切换功能使能    1:使能 0:禁止
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.ups_delay_0ms)
                        {
                            gstUdpPcsPara.udp_pcs_hold.ups_delay_0ms = set_value;
                        }
                        break;
                    }
                    case 0x6208:    //设置本地负载输出使能    1:使能 0:禁止
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.backup_output_enable)
                        {
                            gstUdpPcsPara.udp_pcs_hold.backup_output_enable = set_value;
                        }
                        break;
                    }
                    case 0x6209:    //设置强制发电机使能    1:强制为发电机 0:强制为市电
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.charging_input_type)
                        {
                            gstUdpPcsPara.udp_pcs_hold.charging_input_type = set_value;
                        }
                        break;
                    }
                    case 0x620A:    //设置工作模式    2:削峰填谷 1:电池优先 0:自发自用
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.work_mode)
                        {
                            gstUdpPcsPara.udp_pcs_hold.work_mode = set_value;
                        }
                        break;
                    }
                    case 0x6220:    //AC输出电压档位 00:电压不变 01:100V 02:120V 03:220V 04:230V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.ac_output_volt)
                        {
                            gstUdpPcsPara.udp_pcs_hold.ac_output_volt = set_value;
                        }
                        break;
                    }
                    case 0x6221:    //设置AC输出频率档位 00:频率不变 01:60HZ 02:50HZ
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.ac_output_freq)
                        {
                            gstUdpPcsPara.udp_pcs_hold.ac_output_freq = set_value;
                        }
                        break;
                    }
                    case 0x6222:    //设置市场协商电网标准 00:中国 01:德国 02:澳洲A 03:澳洲B 04:澳洲C 05:新西兰
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.grid_standards)
                        {
                            gstUdpPcsPara.udp_pcs_hold.grid_standards = set_value;
                        }
                        break;
                    }
                    case 0x6223:    //设置电网标准细分 详细见协议 不常用变量
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.grad_standards_segment)
                        {
                            gstUdpPcsPara.udp_pcs_hold.grad_standards_segment = set_value;
                        }
                        break;
                    }
                    case 0x6224:    //设置电池放电最大功率百分比 0% - 100%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.batt_dsg_power_schedule)
                        {
                            gstUdpPcsPara.udp_pcs_hold.batt_dsg_power_schedule = set_value;
                        }
                        break;
                    }
                    case 0x6225:    //设置电池充电最大功率百分比 0% - 100%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.batt_chg_power_schedule)
                        {
                            gstUdpPcsPara.udp_pcs_hold.batt_chg_power_schedule = set_value;
                        }
                        break;
                    }
                    case 0x6226:    //设置PV输入模式 00:输入独立模式 01:输入并联模式 02:输入恒压模式
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_input_mode)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_input_mode = set_value;
                        }
                        break;
                    }
                    case 0x6227:    //设置防逆流使能 00:禁止PV给电网送电 01:允许PV给电网送电
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.anti_reverse_flow_enable)
                        {
                            gstUdpPcsPara.udp_pcs_hold.anti_reverse_flow_enable = set_value;
                        }
                        break;
                    }
                    case 0x6228:    //设置程序控制使能 00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.program_ctrl_enable)
                        {
                            gstUdpPcsPara.udp_pcs_hold.program_ctrl_enable = set_value;
                        }
                        break;
                    }
                    case 0x6229:    //设置过压降额使能 00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.over_volt_derating_enable)
                        {
                            gstUdpPcsPara.udp_pcs_hold.over_volt_derating_enable = set_value;
                        }
                        break;
                    }
                    case 0x622A:    //设置过频降额使能 00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.over_freq_derating_enable)
                        {
                            gstUdpPcsPara.udp_pcs_hold.over_freq_derating_enable = set_value;
                        }
                        break;
                    }
                    case 0x622B:    //设置欠压降额使能 00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.under_volt_derating_enable)
                        {
                            gstUdpPcsPara.udp_pcs_hold.under_volt_derating_enable = set_value;
                        }
                        break;
                    }
                    case 0x622C:    //设置欠频降额使能 00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.under_freq_derating_enable)
                        {
                            gstUdpPcsPara.udp_pcs_hold.under_freq_derating_enable = set_value;
                        }
                        break;
                    }
                    case 0x622D:    //设置孤岛使能 00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.island_enable)
                        {
                            gstUdpPcsPara.udp_pcs_hold.island_enable = set_value;
                        }
                        break;
                    }
                    case 0x622E:    //设置AC耦合使能  00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.ac_coupling_enable)
                        {
                            gstUdpPcsPara.udp_pcs_hold.ac_coupling_enable = set_value;
                        }
                        break;
                    }
                    case 0x622F:    //设置高低穿使能  00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.high_low_crossover_enable)
                        {
                            gstUdpPcsPara.udp_pcs_hold.high_low_crossover_enable = set_value;
                        }
                        break;
                    }
                    case 0x6230:    //设置CT或电表   00:CT 01:meter
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.cto_meter)
                        {
                            gstUdpPcsPara.udp_pcs_hold.cto_meter = set_value;
                        }
                        break;
                    }
                    case 0x6231:    //设置电表品牌   详细见协议 不常用变量
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.meter_brand)
                        {
                            gstUdpPcsPara.udp_pcs_hold.meter_brand = set_value;
                        }
                        break;
                    }
                    case 0x6232:    //设置CT方向   00:正方向 01:负方向
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.ct_direction)
                        {
                            gstUdpPcsPara.udp_pcs_hold.ct_direction = set_value;
                        }
                        break;
                    }
                    case 0x6233:    //设置防逆流阈值(不允许并网) 0 - 5000W
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.anti_reverse_flow_threshold)
                        {
                            gstUdpPcsPara.udp_pcs_hold.anti_reverse_flow_threshold = set_value;
                        }
                        break;
                    }
                    case 0x6234:    //设置远程开关机 00:正常 01:关机 02:开机
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.remote_shutdown_status)
                        {
                            gstUdpPcsPara.udp_pcs_hold.remote_shutdown_status = set_value;
                        }
                        break;
                    }
                    case 0x6235:    //设置无功功率值
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.reactive_power_setting_value)
                        {
                            gstUdpPcsPara.udp_pcs_hold.reactive_power_setting_value = set_value;
                        }
                        break;
                    }
                    case 0x6236:    //设置无功控制方式 00:按照功率因数控制 01:按照无功功率控制 02:按照QU曲线控制 03:按照QP曲线控制
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.reactive_power_ctrl_method)
                        {
                            gstUdpPcsPara.udp_pcs_hold.reactive_power_ctrl_method = set_value;
                        }
                        break;
                    }
                    case 0x6237:    //设置功率因数值 80-100 无功为负,超前功因 0.8-1  默认:100-(80-100)    无功为正,滞后功因 0.8-1  默认:100
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.power_factor_setting_value)
                        {
                            gstUdpPcsPara.udp_pcs_hold.power_factor_setting_value = set_value;
                        }
                        break;
                    }
                    case 0x6238:    //设置PV切换 00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_switch)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_switch = set_value;
                        }
                        break;
                    }
                    case 0x6239:    //设置PF切换 00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pf_switch)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pf_switch = set_value;
                        }
                        break;
                    }
                    case 0x623A:    //设置市电功率软件限制 00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.on_grid_power_soft_limit_en)
                        {
                            gstUdpPcsPara.udp_pcs_hold.on_grid_power_soft_limit_en = set_value;
                        }
                        break;
                    }
                    case 0x623B:    //设置市电功率硬件限制 00:禁止 01:使能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.on_grid_power_hard_limit_en)
                        {
                            gstUdpPcsPara.udp_pcs_hold.on_grid_power_hard_limit_en = set_value;
                        }
                        break;
                    }
                    case 0x6250:    //设置电池放电截止电压 0.1V 范围 43.2 - 57.6V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.batt_dsg_volt_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.batt_dsg_volt_point = set_value;
                        }
                        break;
                    }
                    case 0x6251:    //设置电池充电电压保护电  0.1V 范围 43.2 - 57.6V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.batt_chg_volt_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.batt_chg_volt_point = set_value;
                        }
                        break;
                    }
                    case 0x6252:    //设置PV输入欠压保护电   0.1V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_uvp_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_uvp_point = set_value;
                        }
                        break;
                    }
                    case 0x6253:    //设置PV输入过压保护点   0.1V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ovp_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ovp_point = set_value;
                        }
                        break;
                    }
                    case 0x6254:    //设置有功设置值   0% - 100%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.active_power_setting_value)
                        {
                            gstUdpPcsPara.udp_pcs_hold.active_power_setting_value = set_value;
                        }
                        break;
                    }
                    case 0x6255:    //设置电网电压下限  0.1V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.grid_volt_low_limit)
                        {
                            gstUdpPcsPara.udp_pcs_hold.grid_volt_low_limit = set_value;
                        }
                        break;
                    }
                    case 0x6256:    //设置电网电压上限  0.1V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.grid_volt_upper_limit)
                        {
                            gstUdpPcsPara.udp_pcs_hold.grid_volt_upper_limit = set_value;
                        }
                        break;
                    }
                    case 0x6257:    //设置电网频率下限  0.01HZ
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.grid_freq_low_limit)
                        {
                            gstUdpPcsPara.udp_pcs_hold.grid_freq_low_limit = set_value;
                        }
                        break;
                    }
                    case 0x6258:    //设置电网频率上限  0.01HZ
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.grid_freq_upper_limit)
                        {
                            gstUdpPcsPara.udp_pcs_hold.grid_freq_upper_limit = set_value;
                        }
                        break;
                    }
                    case 0x6259:    //设置电网频率下限  0.01HZ
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.bypass_volt_low_limit)
                        {
                            gstUdpPcsPara.udp_pcs_hold.bypass_volt_low_limit = set_value;
                        }
                        break;
                    }
                    case 0x625A:    //设置旁路电压上限  0.1V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.bypass_volt_upper_limit)
                        {
                            gstUdpPcsPara.udp_pcs_hold.bypass_volt_upper_limit = set_value;
                        }
                        break;
                    }
                    case 0x625B:    //设置旁路频率下限  0.01HZ
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.bypass_freq_low_limit)
                        {
                            gstUdpPcsPara.udp_pcs_hold.bypass_freq_low_limit = set_value;
                        }
                        break;
                    }
                    case 0x625C:    //设置旁路频率上限  0.01HZ
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.bypass_freq_upper_limit)
                        {
                            gstUdpPcsPara.udp_pcs_hold.bypass_freq_upper_limit = set_value;
                        }
                        break;
                    }
                    case 0x625D:    //设置过压降额点    1%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.over_volt_derating_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.over_volt_derating_point = set_value;
                        }
                        break;
                    }
                    case 0x625E:    //设置过频降额点    1%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.over_freq_derating_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.over_freq_derating_point = set_value;
                        }
                        break;
                    }
                    case 0x625F:    //设置欠压降额点(充电)  1%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.under_volt_derating_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.under_volt_derating_point = set_value;
                        }
                        break;
                    }
                    case 0x6260:    //设置欠频降额点(充电)  1%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.under_freq_derating_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.under_freq_derating_point = set_value;
                        }
                        break;
                    }
                    case 0x6261:    //设置电网过压保护 1%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.grid_ov_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.grid_ov_point = set_value;
                        }
                        break;
                    }
                    case 0x6262:    //设置逆变低压保护点 1%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.inv_uvp_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.inv_uvp_point = set_value;
                        }
                        break;
                    }
                    case 0x6263:    //设置低频保护点 1%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.low_freq_protect_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.low_freq_protect_point = set_value;
                        }
                        break;
                    }
                    case 0x6264:    //设置过频保护点 1%
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.over_freq_protect_point)
                        {
                            gstUdpPcsPara.udp_pcs_hold.over_freq_protect_point = set_value;
                        }
                        break;
                    }
                    case 0x6265:    //设置并网重连时间(S)  5 - 300
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.on_grid_reconnect_time)
                        {
                            gstUdpPcsPara.udp_pcs_hold.on_grid_reconnect_time = set_value;
                        }
                        break;
                    }
                    case 0x6266:    //设置并网最大功率(W)  0 - 5000
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.on_grid_power)
                        {
                            gstUdpPcsPara.udp_pcs_hold.on_grid_power = set_value;
                        }
                        break;
                    }
                    case 0x6267:    //设置无功响应时间(S)
                        {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.reactive_power_respones_time)
                        {
                            gstUdpPcsPara.udp_pcs_hold.reactive_power_respones_time = set_value;
                        }
                        break;
                    }
                    case 0X6268:    //设置恢复出厂设置 写入 0X5AA5 则恢复出厂设置
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.reset_factory)
                        {
                            gstUdpPcsPara.udp_pcs_hold.reset_factory = set_value;
                        }
                        break;
                    }
                    case 0x6280:    //R(U/A)相电压校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.r_phase_volt_calibration_k_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.r_phase_volt_calibration_k_parm = set_value;
                        }
                        break;
                    }
                    case 0x6281:    //R(U/A)相电压校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.r_phase_volt_calibration_b_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.r_phase_volt_calibration_b_parm = set_value;
                        }
                        break;
                    }
                    case 0x6282:    //S(V/B)相电压校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.s_phase_volt_calibration_k_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.s_phase_volt_calibration_k_parm = set_value;
                        }
                        break;
                    }
                    case 0x6283:    //S(V/B)相电压校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.s_phase_volt_calibration_b_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.s_phase_volt_calibration_b_parm = set_value;
                        }
                        break;
                    }
                    case 0x6284:    //T(W/C)相电压校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.t_phase_volt_calibration_k_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.t_phase_volt_calibration_k_parm = set_value;
                        }
                        break;
                    }
                    case 0x6285:    //T(W/C)相电压校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.t_phase_volt_calibration_b_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.t_phase_volt_calibration_b_parm = set_value;
                        }
                        break;
                    }
                    case 0x6286:    //R(U/A)相电流校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.r_phase_cur_calibration_k_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.r_phase_cur_calibration_k_parm = set_value;
                        }
                        break;
                    }
                    case 0x6287:    //R(U/A)相电流校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.r_phase_cur_calibration_b_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.r_phase_cur_calibration_b_parm = set_value;
                        }
                        break;
                    }
                    case 0x6288:    //S(V/B)相电流校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.s_phase_cur_calibration_k_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.s_phase_cur_calibration_k_parm = set_value;
                        }
                        break;
                    }
                    case 0x6289:    //S(V/B)相电流校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.s_phase_cur_calibration_b_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.s_phase_cur_calibration_b_parm = set_value;
                        }
                        break;
                    }
                    case 0x628A:    //T(W/C)相电流校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.t_phase_cur_calibration_k_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.t_phase_cur_calibration_k_parm = set_value;
                        }
                        break;
                    }
                    case 0x628B:    //T(W/C)相电流校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.t_phase_cur_calibration_b_parm)
                        {
                            gstUdpPcsPara.udp_pcs_hold.t_phase_cur_calibration_b_parm = set_value;
                        }
                        break;
                    }
                    case 0x628C:    //PV1电流校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch1_curr_k_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch1_curr_k_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x628D:    //PV1电流校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch1_curr_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch1_curr_b_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x628E:    //PV2电流校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch2_curr_k_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch2_curr_k_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x628F:    //PV2电流校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch2_curr_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch2_curr_b_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x6290:    //PV3电流校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch3_curr_k_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch3_curr_k_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x6291:    //PV3电流校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch3_curr_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch3_curr_b_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x6292:    //PV4电流校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch4_curr_k_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch4_curr_k_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x6293:    //PV4电流校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch4_curr_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch4_curr_b_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x6294:    //PV1电压校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch1_volt_k_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch1_volt_k_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x6295:    //PV1电压校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch1_volt_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch1_volt_b_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x6296:    //PV2电压校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch2_volt_k_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch2_volt_k_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x6297:    //PV2电压校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch2_volt_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch2_volt_b_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x6298:    //PV3电压校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch3_volt_k_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch3_volt_k_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x6299:    //PV3电压校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch3_volt_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch3_volt_b_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x629A:    //PV4电压校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch4_volt_k_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch4_volt_k_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x629B:    //PV4电压校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_ch4_volt_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_ch4_volt_b_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x629C:    //电池电压校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.batt_volt_k_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.batt_volt_k_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x629D:    //电池电压校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.batt_volt_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.batt_volt_b_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x629E:    //电池电流校准系数K: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.batt_curr_k_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.batt_curr_k_calibrate = set_value;
                        }
                        break;
                    }
                    case 0x629F:    //电池电流校准系数B: 默认值 4096
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.batt_curr_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.batt_curr_b_calibrate = set_value;
                        }
                        break;
                    }
                    case 0X62B8:    //设置唤醒电池电压 0.1V
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.batt_curr_b_calibrate)
                        {
                            gstUdpPcsPara.udp_pcs_hold.batt_volt_to_dsp = set_value;
                        }
                        break;
                    }
                    case 0X62B9:    //设置激活使能 01:PV激活使能 02:PV激活完成 03:正常功能
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.dsp_wake_en)
                        {
                            gstUdpPcsPara.udp_pcs_hold.dsp_wake_en = set_value;
                        }
                        break;
                    }
                    case 0X62BA:    //设置PV最大功率 单位 W
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pv_max_chg_power)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pv_max_chg_power = set_value;
                        }
                        break;
                    }
                    case 0X62BB:    //设置最大逆变功率 单位W
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.max_inv_power)
                        {
                            gstUdpPcsPara.udp_pcs_hold.max_inv_power = set_value;
                        }
                        break;
                    }
                    case 0x62BC:    //设置放电控制 00:监控卡异常禁止放电 01:允许放电 02:禁止放电
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pcs_dsg_ctrl)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pcs_dsg_ctrl = set_value;
                        }
                        break;
                    }
                    case 0x62BD:    //设置充电控制 00:监控卡异常禁止充电 01:允许充电 02:禁止充电
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pcs_chg_ctrl)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pcs_chg_ctrl = set_value;
                        }
                        break;
                    }
                    case 0x62BE:    //设置用户类型 00:普通用户 01:管理员 02:安装调试 03:内部
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.pcs_user_type)
                        {
                            gstUdpPcsPara.udp_pcs_hold.pcs_user_type = set_value;
                        }
                        break;
                    }
                    case 0x62BF:    //设置消除CT故障 00:不消除CT告警 01:消除CT告警
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.ignor_ct_err)
                        {
                            gstUdpPcsPara.udp_pcs_hold.ignor_ct_err = set_value;
                        }
                        break;
                    }
                    case 0x62C0:    //下发当前系统状态 (BIT0: 00:未升级 01:升级中 02)(BIT1: 00:DSP未复位 01:DSP复位 02)(BIT2:监控卡测试未付状态位)(BIT3: 00:监控卡未复位 01: 监控卡复位中)
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.sys_upgrade_status)
                        {
                            gstUdpPcsPara.udp_pcs_hold.sys_upgrade_status = set_value;
                        }
                        break;
                    }
                    case 0x62C1:
                    {
                        if(set_value != gstUdpPcsPara.udp_pcs_hold.drm_state)
                        {
                            gstUdpPcsPara.udp_pcs_hold.drm_state = set_value;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }    
    // 此处表示解析成功不回包
    return EN_ETH_ANALYSIS_OK_NORESPONSE;
}
/**
 * @name Ch390SendMonitor
 * @brief
 */
void Ch390SendMonitor(void)
{
    for(int i = 0; i < 6; i++) //监控的MAC地址
    {
        _garrPacketData[i] = _gstNetworkInfo.arr_master_mac_address[i];
    }
    /*      CH390的MAC部分    */
    _garrPacketData[6] = _gstNetworkInfo.arr_ch390_mac_address[0];
    _garrPacketData[7] = _gstNetworkInfo.arr_ch390_mac_address[1];
    _garrPacketData[8] = _gstNetworkInfo.arr_ch390_mac_address[2];
    _garrPacketData[9] = _gstNetworkInfo.arr_ch390_mac_address[3];
    _garrPacketData[10] = _gstNetworkInfo.arr_ch390_mac_address[4];
    _garrPacketData[11] = _gstNetworkInfo.arr_ch390_mac_address[5];
    /*      IPV4    */
    _garrPacketData[12] = 0x08;
    _garrPacketData[13] = 0x00;
    /*      网络版本，如下为网络层    */
    _garrPacketData[14] = 0x45;//版本为4,网络层共20个字节
    _garrPacketData[15] = 0x00;//DSCP区分服务代码点
    _garrPacketData[16] = ((INTRENET_LEN + APP_LEN + (gstMonitor.Length << 1) + 10) >> 8);//长度高位，包含网络层+应用层+数据层长度
    _garrPacketData[17] = ((INTRENET_LEN + APP_LEN + (gstMonitor.Length << 1) + 10) & 0xFF);//长度低位，包含网络层+应用层+数据层长度
    _garrPacketData[18] = 0x66;//报文ID标识,高位
    _garrPacketData[19] = 0x88;//报文ID标识.低位
    _garrPacketData[20] = 0x00;//分包标识.高位
    _garrPacketData[21] = 0x00;//分包标识.高位
    _garrPacketData[22] = 0x80;//存活时间
    _garrPacketData[23] = 0x11;//UDP协议
    _garrPacketData[24] = 0x00;//头部检查高位
    _garrPacketData[25] = 0x00;//头部检查低位

    _garrPacketData[26] = _gstNetworkInfo.arr_ch390_ip_address[0];//源IP
    _garrPacketData[27] = _gstNetworkInfo.arr_ch390_ip_address[1];
    _garrPacketData[28] = _gstNetworkInfo.arr_ch390_ip_address[2];
    _garrPacketData[29] = _gstNetworkInfo.arr_ch390_ip_address[3];

    _garrPacketData[30] = _gstNetworkInfo.arr_master_ip_address[0];//目标IP
    _garrPacketData[31] = _gstNetworkInfo.arr_master_ip_address[1];
    _garrPacketData[32] = _gstNetworkInfo.arr_master_ip_address[2];
    _garrPacketData[33] = _gstNetworkInfo.arr_master_ip_address[3];
    /*      如下为应用层    */
    _garrPacketData[34] = _gstNetworkInfo.arr_ch390_udp_port[0];//源端口
    _garrPacketData[35] = _gstNetworkInfo.arr_ch390_udp_port[1];
    _garrPacketData[36] = _gstNetworkInfo.arr_master_udp_port[0];//目标端口
    _garrPacketData[37] = _gstNetworkInfo.arr_master_udp_port[1];
    _garrPacketData[38] = (APP_LEN + (gstMonitor.Length << 1) + 10) >> 8;//应用层加数据长度
    _garrPacketData[39] = (APP_LEN + (gstMonitor.Length << 1) + 10) & 0xFF;
    _garrPacketData[40] = 0x00;//校验码
    _garrPacketData[41] = 0x00;//校验码
    /*      如下为数据层    */
    for(int i = 42; i < (42 + (gstMonitor.Length << 1) + 10); i++)
    {
        _garrPacketData[i] = _garrSendMonitorBuff[i - 42];
    }
    ch390_send_packet((uint8_t*)_garrPacketData, TEST_DATA_LEN + (gstMonitor.Length << 1) + 10);
}
/**
* @brief: GetCH390ReceiveData
* @param: void
* @return:Nothing
* @note:解析数据包出来
*/
void EthernetProcess(void)
{
    uint8_t ch390_int_status = 0;
    uint32_t timeout_count = 0xFFFFFF;  

    if (ch390_get_int_pin() == 1)
    {
        ch390_int_status = ch390_read_reg(CH390_ISR);

        if(ch390_int_status & ISR_LNKCHG)
        {
//            DELAY_MS(65);
            if (ch390_get_link_status())
            {
                LedCtlBlink(LED_SRC_SYS, RUN_LED_BRINK_PRIED);
//                printf("link up\r\n");
            }
            else
            {
                LedCtlFailed(LED_SRC_SYS);
//                printf("link down\r\n");
            }
            ch390_write_reg(CH390_ISR, ISR_LNKCHG);
        }

        if(ch390_int_status & ISR_PR)
        {
//            printf("recv\r\n");
            memset(_garrReceiveCH390Buff, 0, sizeof(_garrReceiveCH390Buff));
            memset(_gstNetworkInfo.arr_master_mac_address, 0, sizeof(_gstNetworkInfo.arr_master_mac_address));
            memset(_gstNetworkInfo.arr_master_ip_address, 0, sizeof(_gstNetworkInfo.arr_master_ip_address));
            memset(_gstNetworkInfo.arr_recv_target_ipaddr, 0, sizeof(_gstNetworkInfo.arr_recv_target_ipaddr));

            do
            {
                _gstNetworkInfo.udp_data_len = ch390_receive_packet((uint8_t*)_garrReceiveCH390Buff, (uint8_t*)&_gstNetworkInfo.udp_packet_len);
                if(_gstNetworkInfo.udp_data_len == 0)
                {
                    break;  // 无更多数据
                }

                //前6个字节均为0xFF广播,解析ARP协议
                if(memcmp(_garrReceiveCH390Buff, _gstNetworkInfo.arr_broadcast_address, (int)sizeof(_gstNetworkInfo.arr_broadcast_address)) == 0)
                {
                    /*获取主机请求的从机IP地址*/
                    memcpy(_gstNetworkInfo.arr_recv_target_ipaddr, &_garrReceiveCH390Buff[38], (int)sizeof(_gstNetworkInfo.arr_recv_target_ipaddr));

                    if(memcmp(_gstNetworkInfo.arr_recv_target_ipaddr, _gstNetworkInfo.arr_ch390_ip_address, (int)sizeof(_gstNetworkInfo.arr_ch390_ip_address)) == 0)
                    {
                        /*获取主机的MAC地址*/
                        memcpy(_gstNetworkInfo.arr_master_mac_address, &_garrReceiveCH390Buff[6], (int)sizeof(_gstNetworkInfo.arr_master_mac_address));
                        /*获取主机的IP地址*/
                        memcpy(_gstNetworkInfo.arr_master_ip_address, &_garrReceiveCH390Buff[28], (int)sizeof(_gstNetworkInfo.arr_master_ip_address));
                        /*回复ARP数据*/
                        SendArpProtocol();
                        printf("ARP Response\r\n");
                    }
                }
                else if(memcmp(_garrReceiveCH390Buff, _gstNetworkInfo.arr_ch390_mac_address, (int)sizeof(_gstNetworkInfo.arr_ch390_mac_address)) == 0)
                {
                    if(memcmp(&_garrReceiveCH390Buff[36], _gstNetworkInfo.arr_ch390_udp_port, (int)sizeof(_gstNetworkInfo.arr_ch390_udp_port)) == 0)
                    {
                        /*获取主机的MAC地址*/
                        memcpy(_gstNetworkInfo.arr_master_mac_address, &_garrReceiveCH390Buff[6], (int)sizeof(_gstNetworkInfo.arr_master_mac_address));
                        /*获取主机的MAC地址*/
                        memcpy(_gstNetworkInfo.arr_recv_udp_ipv_type, &_garrReceiveCH390Buff[12], (int)sizeof(_gstNetworkInfo.arr_recv_udp_ipv_type));
                        /*UDP协议类型    */
                        _gstNetworkInfo.protocol_type = _garrReceiveCH390Buff[23];
                        /*获取主机的IP地址*/
                        memcpy(_gstNetworkInfo.arr_master_ip_address, &_garrReceiveCH390Buff[26], (int)sizeof(_gstNetworkInfo.arr_master_ip_address));
                        /*主机的需求地址*/
                        memcpy(_gstNetworkInfo.arr_recv_target_ipaddr, &_garrReceiveCH390Buff[30], (int)sizeof(_gstNetworkInfo.arr_recv_target_ipaddr));
                        /*主机UDP端口*/
                        memcpy(_gstNetworkInfo.arr_master_udp_port, &_garrReceiveCH390Buff[34], (int)sizeof(_gstNetworkInfo.arr_master_udp_port));
                        /*数据段长度    */
                        _gstNetworkInfo.udp_data_len = (_garrReceiveCH390Buff[38] << 8 | _garrReceiveCH390Buff[39]) - 8;
                        /*获取监控实际数据部分    */
                        memcpy(_garrReceiveDataBuff, &_garrReceiveCH390Buff[41 + 1], _gstNetworkInfo.udp_data_len);
                        MonitorDataUpload();
                        if(_MonitorAnalysis() == EN_ETH_ANALYSIS_OK) //监控数据包解析
                        {
                            Ch390SendMonitor();   //回复监控数据
                        }
//                        printf("packet Response\r\n");
                    }
                    else if(memcmp(&_garrReceiveCH390Buff[36], &_gdebugReveiveport[0], (int)sizeof(_gstNetworkInfo.arr_ch390_udp_port)) == 0)
                    {
                        /*获取主机的MAC地址*/
                        memcpy(_gstNetworkInfo.arr_master_mac_address, &_garrReceiveCH390Buff[6], (int)sizeof(_gstNetworkInfo.arr_master_mac_address));
                        /*获取主机的MAC地址*/
                        memcpy(_gstNetworkInfo.arr_recv_udp_ipv_type, &_garrReceiveCH390Buff[12], (int)sizeof(_gstNetworkInfo.arr_recv_udp_ipv_type));
                         /*UDP协议类型    */
                        _gstNetworkInfo.protocol_type = _garrReceiveCH390Buff[23];
                         /*获取主机的IP地址*/
                        memcpy(_gstNetworkInfo.arr_master_ip_address, &_garrReceiveCH390Buff[26], (int)sizeof(_gstNetworkInfo.arr_master_ip_address));
                        /*主机的需求地址*/
                        memcpy(_gstNetworkInfo.arr_recv_target_ipaddr, &_garrReceiveCH390Buff[30], (int)sizeof(_gstNetworkInfo.arr_recv_target_ipaddr));
                       /*主机UDP端口*/
                        memcpy(_gstNetworkInfo.arr_master_udp_port, &_garrReceiveCH390Buff[34], (int)sizeof(_gstNetworkInfo.arr_master_udp_port));
                        /*数据段长度    */
                        _gstNetworkInfo.udp_data_len = (_garrReceiveCH390Buff[38] << 8 | _garrReceiveCH390Buff[39]) - 8;
                        /*获取监控实际数据部分    */
                        memcpy(_garrDebugReceiveDataBuff, &_garrReceiveCH390Buff[41 + 1], _gstNetworkInfo.udp_data_len);
//                        DebugReveiveAnalysis(_garrDebugReceiveDataBuff , _gstNetworkInfo.udp_data_len);
                    }
                }
            }while(timeout_count--);
        }
        ch390_write_reg(CH390_ISR, ch390_int_status | ISR_PR | ISR_PT | ISR_ROS | ISR_ROO);//清除中断标志位
    }
}

void NetworkInit(void)
{
    /*监控下发数据结构体初始化*/
    memset((uint16_t*)&gstMonitor, 0x00, (int)sizeof(MonitorUdp_t));
    /*通信寄存器数据初始化*/
    memset((uint16_t*)&gstUdpPcsPara, 0x00, (int)sizeof(UdpPCSPara_t));
    
    /*以太网配置初始化*/
    _gstNetworkInfo.arr_ch390_ip_address[0] = CH390_IP_ADDRESS0;
    _gstNetworkInfo.arr_ch390_ip_address[1] = CH390_IP_ADDRESS1;
    _gstNetworkInfo.arr_ch390_ip_address[2] = CH390_IP_ADDRESS2;
    _gstNetworkInfo.arr_ch390_ip_address[3] = CH390_IP_ADDRESS3;
    _gstNetworkInfo.arr_ch390_udp_port[0]   = (CH390_PORT >> 8) & 0x00FF;
    _gstNetworkInfo.arr_ch390_udp_port[1]   = CH390_PORT & 0x00FF;
    memset(_gstNetworkInfo.arr_broadcast_address, 0xFF, (int)sizeof(_gstNetworkInfo.arr_broadcast_address));

    if(CH390Init() == true)
    {
        LedCtlBlink(LED_SRC_SYS, RUN_LED_BRINK_PRIED);
        GetCH390MacAddress();
        printf("MAC: ");
        for (int i = 0; i < 6; i++)
        {
            printf("%02X ", _gstNetworkInfo.arr_ch390_mac_address[i]);
        }
        printf("\r\n");
    }
    else
    {
        LedCtlFailed(LED_SRC_SYS);
    }
}

/**
* @brief: UpLoadStatus
* @param: state   0: false 1:success                     
* @note: 
*
*/
void UpLoadStatus(uint8_t state)
{
    if(state)
    {
        gstUdpPcsPara.udp_pcs_hold.sys_upgrade_status |= (1 << 13);
    }
    else
    {
        gstUdpPcsPara.udp_pcs_hold.sys_upgrade_status |= (1 << 14);
    }
}

uint16_t UpgradeRegRead(void)
{
    return gstUdpPcsPara.udp_pcs_hold.sys_upgrade_status;
}

bool MeterModeGet(void)
{
    if(gstUdpPcsPara.udp_pcs_hold.cto_meter)
    {
        return true;
    }
    return false;
}

uint16_t MeterBrandGet(void)
{
    return gstUdpPcsPara.udp_pcs_hold.meter_brand;
}

/**
* @brief: GetEthSysUpgradeState
* @param: void
* @note:  return 1: Eth upgrade being  0:no upgrade
*
*/
bool GetEthSysUpgradeState(void)
{
    uint16_t sys_upgrade_status = 0;

    /*读取升级寄存器*/
    sys_upgrade_status = UpgradeRegRead();

    /*判断系统是否处于升级*/
    if((sys_upgrade_status & 0x01))
    {
        /*判断固件是否下发完成*/
        if(!((sys_upgrade_status >> 12) & 0x01))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;;
}
