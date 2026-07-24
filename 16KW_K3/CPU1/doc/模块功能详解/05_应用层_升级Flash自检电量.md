# 05 应用层：升级、Flash、自检、电量、认证与上传

对应源码：`firmware_upgrate`、`flash_default`、`post`、`electric_calculate`、`authentication_func`、`sys_info`

---

## 1. `firmware_upgrate` — 固件升级

### 1.1 职责

经以太网接收 PCS/ARC/Copro 固件，写入外部 SPI Flash；再向 ARC/Copro 转发；协调复位。

### 1.2 Flash 分区（示意）

| 模块 | 标志区 | 数据区 |
|------|--------|--------|
| DSP | `0x00000000` | `0x00001000` |
| ARC | `0x00080000` | `0x00081000` |
| Copro | `0x00100000` | `0x00101000` |

### 1.3 关键结构

- `FirmwareRecvInfo_t`：类型、版本、长度、累计长度、校验、状态  
- `FirmwareUpgrateInfo_t`：转发状态、偏移、ACK、包号  
- `gstFirmwareInfo`：三者接收信息  
- `gstVersionInfo`：型号与版本  

### 1.4 `EthernentUpgradeRecv()`

1. 按监控地址识别 DSP/ARC/Copro 首包  
2. 初始化接收信息与 Flash 目标  
3. 负载写 Flash  
4. 首包解析类型/版本/长度/checksum  
5. 后续包读回累加 checksum  
6. 长度匹配后标记成功/失败；成功写完成标志页  

### 1.5 `SystemFirmwareUpdate()` 状态机

`IDLE → START → RUNNING → RESET → END / ERROR`

- 等升级寄存器 bit12 表示接收完成  
- 按 bit7/bit11/bit6 决定 ARC/PCS/Copro 是否参与  
- RUNNING 中跑 ARC、Copro 转发状态机  
- 全部完成后置上传成功  
- 等监控 reset bit：先复位 ARC/Copro，再 `SysRestart()` 重启 DSP  

### 1.6 子模块转发（ARC/Copro）

检查 Flash 头 → 发 start → 等 ACK → 首包 256B、后续 2KB → busy ACK 推进（失败最多重发 3 次）→ end → `upg_state=4/5`。

---

## 2. `flash_default` — 持有参数持久化

### 2.1 职责

`UdpDspHoldPara_t` 默认值、Flash 序列化、校验提交、自动保存、恢复出厂。

### 2.2 提交策略（两阶段）

1. 写 header 为无效  
2. 写参数本体  
3. 写有效 header（`55 AA 55 AA`）  
4. 读回比对  

### 2.3 关键函数

| 函数 | 行为 |
|------|------|
| `UdpPCS_DspHoldPara_Init` | 完整默认寄存器镜像（功率、模式、SOC、CT、标准、保护、校准等） |
| `UdpDspHoldPara_AutoSaveIfNeeded` | 与镜像比较，变化则保存 |
| `Init_System_Parameter` | header 无效→加载默认并写回；有效→反序列化 |
| `Restore_Factory_Settings` | true：写默认并更新 RAM；false：仅使 header 无效 |
| `ShutDown_SaveFlash` | 关机路径保存 |

比较/关机保存时排除瞬态字段：关机、LLC、远程关机、充输入类型、电池充放功率、恢复出厂、升级状态等。

### 2.4 注意

`Init_System_Parameter()` 当前 **未在 main 启动路径调用**。  
`Save_System_Parameter` 声明归属在 ethernet 头与 flash_default 之间不完全一致。

---

## 3. `post` — 上电自检

### 3.1 结构

- `ModuleTestCompleteState_t`：风扇、NTC、ADC、漏电、继电器、绝缘六位  
- `SysSelfTest_t`：完成、超时、重测、最大时间  

### 3.2 流程

1. `SysSelfTestInit`：清状态，超时约 25 s  
2. `SysSelfTest`：首次记录时间  
3. 先查 ADC、风扇  
4. ADC 正常后查 NTC、漏电、继电器、绝缘  
5. 六位全 1 且超过 15 s → 成功，断开全部继电器  
6. 超过 25 s → 超时失败，断开全部继电器  
7. `SysRetestRequestSet` 可触发重测  
8. `GetSysSelfTestFlag` / `GetSysTimeOutFlag` 供 Biz FSM  

继电器自检细节见 device 层 `relay_ctrl`。

---

## 4. `electric_calculate` — 电量累计

### 4.1 单位

以 **0.1 kWh** 为单位累计：PV、电池、电网、EPS、油机、远端负载、EV。

### 4.2 算法

1. 约每 100 ms 对正向功率积分：`power_accumulate_ws += W * 0.1`  
2. 600 次样本 ≈ 一分钟窗  
3. `Wh = Ws / 3600`  
4. 每满 100 Wh：`total_01kwh++`，余量进 `remainder_wh`  

`ElectricPowerAccumulate` 由 20 ms 任务调用，内部用时间门限保证约 100 ms 积分；约 60 s 完成一窗。

`ElectricParamClear`：响应通信清零命令。

---

## 5. `authentication_func` — 并网认证（预留）

头文件定义澳洲 A/B/C、新西兰等标准，电压/频率区间，无功模式（PF/固定 Q/QV/QP）及曲线参数结构。  

**实现文件中算法表格基本全部注释，当前不参与实际控制。**

---

## 6. `sys_info` — 上传信息组装

### 6.1 职责

把内部状态转换成监控上传寄存器布局 `UploadInfo_t`。

### 6.2 主要分区

| 分区 | 内容 |
|------|------|
| `upload_basic_info` | 型号、SN、软硬件版本（含 ARC） |
| `upload_module_status` | 输入状态、逆变状态、MPPT、温度 |
| `upload_Capacity_info` | 累计电量 |
| `upload_Power_info` | 三相 V/I/P/Q/S、PV、电池等 |
| `upload_fault_info` | 错误/告警/通知 |

### 6.3 关键函数

- `UploadInfoInit` / `SysInfoUpload`（总入口，UDP 上传调用）  
- `GetPcsModeId` / `GetPcsVersionInfo` / `GetInverterModel`  
- `GetSysInputType` / `GetSysState` / `GetInvState`  
- `GetCtOrMeterInfo`：每次更新一相，三次凑齐  
- `GetTemperatureValue`：温度 ×10  
- `ElectricInfoGet_UPload`：0.1kWh 拆高低 16 位  
- `GetElectricalInfo`：分相功率 + 全部 PV/母线/电池  

### 6.4 已知缺口

- `GetSysInputType` 中 MPPT2/3/4 分支可能误写 `pv1_input`  
- PV4 功率高低字赋值异常风险  
- `GetAlarmCode` / `GetErrorCode` 基本为空 → 上传故障区难反映 `gstFaultInfo`  
- 部分负载字段硬置 0  

---

## 7. 与其它模块的衔接

```
以太网升级写 ──► firmware_upgrate ──► SPI Flash ──► ARC/Copro 转发
持有寄存器变更 ──► flash_default 自动保存
BizFSM SELF_TEST ──► post
20ms ──► electric_calculate
UDP 上传 ──► sys_info ← 电量/三相/温度/状态
```

---

## 附录：本章源码

> 以下为本章涉及的全部自研源码原文（不含 TI `include/`、`source/` 库）。

### `app/firmware_upgrate.c`

```c
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        firmware_upgrate.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-3-10
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-3-10    <td>V1.0.0    <td>            <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "firmware_upgrate.h"
#include "dev_flash.h"

#define UPGRATE_DATA_SIZE       2048//���ݴ�С
#define UPGRATE_TIME_OUT        300000
#define ARC_COM_ADDR        0x02
#define COPRO_COM_ADDR      0x81
static uint16_t  _upgrate_data[2100];
static uint16_t pack_num_index = 0;
static  uint8_t resend_count = 0;
FirmwareUpgrateInfo_t gFirmwareUpgrateInfo_arc; //ARC�̼������ṹ��
FirmwareUpgrateInfo_t gFirmwareUpgrateInfo_copro; //Э�������̼������ṹ��
uint8_t pack_count = 0;
uint8_t gBMSHardVersion[3] = {0};
uint32_t gBMSHardwareLen = 0;
uint16_t pack_size = 0;
uint32_t gFirmwareChecksum = 0;
uint8_t space_num = 0;
uint16_t garrArcFirmwareType[4] = {0x0C, 0x0C, 0x0C, 0x02};
uint16_t garrDspFirmwareType[4] = {0x0C, 0x0C, 0x0C, 0x01};
uint16_t garrCoproFirmwareType[4] = {0x0C, 0x0C, 0x0C, 0x81};

FirmwareRecvInfoS_t  gstFirmwareInfo;
VersionInfoS_t gstVersionInfo;


void SystemFirmwareUpdate(void) //ϵͳ�̼���������
{
    static SystemFirmwareUpdateState_t  SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_IDLE;
    static uint16_t upgrade_reg = 0;
    static uint16_t arc_being = 0;
    static uint16_t pcs_being = 0;
    static uint16_t copro_being = 0;
    static uint16_t sys_reset_en = 0;
    static uint16_t en_arc_update = 0;
    static uint16_t en_pcs_update = 0;
    static uint16_t en_copro_update = 0;
    static uint32_t reset_timecount = 0;

    /*��ȡ�����Ĵ���*/
    upgrade_reg = UpgradeRegRead();
    arc_being = (upgrade_reg >> 7) & 0x01;
    pcs_being = (upgrade_reg >> 11) & 0x01;
    copro_being = (upgrade_reg >> 6) & 0x01;
    sys_reset_en = (upgrade_reg >> 1) & 0x01;        //sys_update
    
    switch (SysUpdateState)
    {
        case EN_SYSTEM_FIRMWARE_UPDATE_IDLE:  //��ͨ���н��ܹ̼������ȴ��̼��·����
        {    
            reset_timecount = 0;
            /*�жϹ̼��Ƿ��·����*/
            if(!((upgrade_reg >> 12) & 0x01))
            {
                break;
            }
            else
            {
                SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_START;
            }
            break;
        }
        case EN_SYSTEM_FIRMWARE_UPDATE_START: //�ж�����Щ�̼�������
        {
            en_arc_update = arc_being ? (gstFirmwareInfo.arc_firmware_info.upg_state == 2 ? 1 : 0) : 1;
            en_pcs_update = pcs_being ? (gstFirmwareInfo.pcs_firmware_info.upg_state == 2 ? 1 : 0) : 1;
            en_copro_update = copro_being ? (gstFirmwareInfo.copro_firmware_info.upg_state == 2 ? 1 : 0) : 1;
            if (en_arc_update && en_pcs_update && en_copro_update && (arc_being||copro_being||pcs_being))
            {
                SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_RUNNING;
            }
            // else
            // {
            //     UpLoadStatus(0);
            //     SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_ERROR;
            // }
            
            // if (pcs_being && (gstFirmwareInfo.pcs_firmware_info.upg_state == 3))
            // {
            //     UpLoadStatus(0);
            //     SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_ERROR;
            // }   
            
            break;
        }
        case EN_SYSTEM_FIRMWARE_UPDATE_RUNNING: // �̼�����������
        {
            ArcUpgrateFirmwareProcess();
            CoproUpgrateFirmwareProcess();
            /*�жϹ̼������Ƿ����*/
            if(((!arc_being) || (gstFirmwareInfo.arc_firmware_info.upg_state == 4)) &&
               ((!copro_being) || (gstFirmwareInfo.copro_firmware_info.upg_state == 4)) &&
               ((!pcs_being) || (gstFirmwareInfo.pcs_firmware_info.upg_state == 2)))
            {
                SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_RESET;
                UpLoadStatus(1);
            }

            break;
        }
        case EN_SYSTEM_FIRMWARE_UPDATE_RESET:  //�̼��·����
        {    
            if (sys_reset_en == 1) //����Ƿ��·�����ָ��
            {
                if (((!arc_being) || SetArcSysReset() == true) && 
                    ((!(copro_being)) || SetCoproSysReset() == true))
                {
                    SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_END;
                    SysRestart(); //��λDSP
                    reset_timecount = 0;
                }
                else
                {
                    reset_timecount++;
                    SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_RESET;
                }
                if (reset_timecount > UPGRATE_TIME_OUT)
                {    
                    SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_ERROR;
                }
            }
            else    
            {
                reset_timecount++;
                SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_RESET;
            }
            if (reset_timecount > 800000)
            {    
                SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_ERROR;
            }
            break;
        }
        case EN_SYSTEM_FIRMWARE_UPDATE_END:   //�̼�����������Э�������ȸ�λ�ɹ����ж��������
        {
            if ((pcs_being == 1) && (gstFirmwareInfo.pcs_firmware_info.upg_state != 2))
            {
                SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_ERROR;
            }
            else 
            {
                SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_IDLE;
            }
            break;
        }
        case EN_SYSTEM_FIRMWARE_UPDATE_ERROR: //�̼���������
        {
            SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_IDLE;
            break;
        }
        default:
            {
                SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_IDLE;        
                break;
            }
    }

}


bool ArcFirmwareSending(void)
{
    uint16_t upgrade_reg = 0;
    uint16_t arc_being = 0;

    /*��ȡ�����Ĵ���*/
    upgrade_reg = UpgradeRegRead();
    arc_being = (upgrade_reg >> 7) & 0x01;

    if(arc_being)
    {
        if((gstFirmwareInfo.arc_firmware_info.upg_state == 1) || (gstFirmwareInfo.arc_firmware_info.upg_state == 2))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

bool CoproFirmwareSending(void)
{
    uint16_t upgrade_reg = 0;
    uint16_t copro_being = 0;

    /*��ȡ�����Ĵ���*/
    upgrade_reg = UpgradeRegRead();
    copro_being = (upgrade_reg >> 6) & 0x01;        //TODO: copro_being����ȷ��Э��

    if(copro_being)
    {
        if((gstFirmwareInfo.copro_firmware_info.upg_state == 1) || (gstFirmwareInfo.copro_firmware_info.upg_state == 2))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}



static bool _ArcUpgradeCheck(void)
{
    /* 0��idle 1�����й̼��·��ɹ�           2���й̼��·�ʧ��          3����������ARC��DSP�·��ɹ��� 
            4����������ARC��DSP�·�ʧ�ܣ� 5������DSP��ARC�ɹ� 6������DSP��ARCʧ��  7:ARCת���ɹ�  8��ARCת��ʧ�� */
//    uint16_t upgrade_state = 0;
    uint16_t upgrade_reg = 0; 
//    uint16_t pcs_being = 0;
    uint16_t arc_being = 0;

    /*��ȡ�����Ĵ���*/
    upgrade_reg = UpgradeRegRead();

    /*�жϹ̼��Ƿ��·����*/
    if(!((upgrade_reg >> 12) & 0x01))
    {
        return false;
    }

    /*ϵͳ��λ�ж�*/
    // if(((upgrade_reg >> 1) & 0x01))
    // {
    //     /*ARC�յ���λ */
    //     if(SetArcSysReset() == true)
    //     {
    //         /*��λ����*/
    //         //TODO
    //         //SysRestart();
    //     }        
    // }
    
    //pcs_being = (upgrade_reg >> 11) & 0x01;
    arc_being = (upgrade_reg >> 7) & 0x01;

    if (arc_being)
    {
        if (gstFirmwareInfo.arc_firmware_info.upg_state == 2) //arcת������
        {
            return true;
        }
        else 
        {
            return false;
        }
        
    }
    return false;
    

    /*pcs��arc�̼�������*/
    // if(pcs_being && arc_being)
    // {
    //     /*pcs��arc�̼����·����*/
    //     if((gstFirmwareInfo.pcs_firmware_info.upg_state == 2) && (gstFirmwareInfo.arc_firmware_info.upg_state == 2))
    //     {
    //         upgrade_state = 1;
    //     }
    //     else if((gstFirmwareInfo.pcs_firmware_info.upg_state == 3) || (gstFirmwareInfo.arc_firmware_info.upg_state == 3))
    //     {
    //         upgrade_state = 2;
    //     }
    //     else if(gstFirmwareInfo.arc_firmware_info.upg_state == 4)
    //     {
    //         upgrade_state = 7;
    //     }
    //     else if(gstFirmwareInfo.arc_firmware_info.upg_state == 5)
    //     {
    //         upgrade_state = 8;
    //     }
    // }
    // else if(pcs_being && !arc_being)
    // {
    //     if(gstFirmwareInfo.pcs_firmware_info.upg_state == 2)
    //     {
    //         upgrade_state = 3;
    //     }
    //     else if(gstFirmwareInfo.pcs_firmware_info.upg_state == 3)
    //     {
    //         upgrade_state = 4;
    //     }
    // }
    // else if(!pcs_being && arc_being)
    // {
    //     if(gstFirmwareInfo.arc_firmware_info.upg_state == 2)
    //     {
    //         upgrade_state = 5;
    //     }
    //     else if(gstFirmwareInfo.arc_firmware_info.upg_state == 3)
    //     {
    //         upgrade_state = 6;
    //     }
    //     else if(gstFirmwareInfo.arc_firmware_info.upg_state == 4)
    //     {
    //         upgrade_state = 7;
    //     }
    //     else if(gstFirmwareInfo.arc_firmware_info.upg_state == 5)
    //     {
    //         upgrade_state = 8;
    //     }
    // }

    // if(upgrade_state == 3)
    // {
    //     UpLoadStatus(1);
    // }

    // if((upgrade_state == 2) || (upgrade_state == 4) || (upgrade_state == 6))
    // {
    //     UpLoadStatus(0);
    // }

    // if((upgrade_state == 1) || (upgrade_state == 5))
    // {
    //     /*��������ARC*/
    //     return true;
    // }

    // if((upgrade_state == 7) || (upgrade_state == 8))
    // {
    //     /* ARC ת������ */
    //     return false;
    // }

    // /*������ARC*/
    // return false;
}

static bool _CoproUpgradeCheck(void)
{
    /* 0��idle 1�����й̼��·��ɹ�           2���й̼��·�ʧ��          3����������ARC��DSP�·��ɹ��� 
            4����������ARC��DSP�·�ʧ�ܣ� 5������DSP��ARC�ɹ� 6������DSP��ARCʧ��  7:ARCת���ɹ�  8��ARCת��ʧ�� */
//    uint16_t upgrade_state = 0;
    uint16_t upgrade_reg = 0; 
//    uint16_t pcs_being = 0;
    uint16_t copro_being = 0;

    /*��ȡ�����Ĵ���*/
    upgrade_reg = UpgradeRegRead();

    /*�жϹ̼��Ƿ��·����*/
    if(!((upgrade_reg >> 12) & 0x01))
    {
        return false;
    }

    /*ϵͳ��λ�ж�*/
    // if(((upgrade_reg >> 1) & 0x01))
    // {
    //     /*Copro�յ���λ */
    //     if(SetCoproSysReset() == true)
    //     {
    //         /*��λ����*/
    //         //TODO
    //         //SysRestart();
    //         ClearUpdateFlag();
    //     }        
    // }
    
    // pcs_being = (upgrade_reg >> 11) & 0x01;
    copro_being = (upgrade_reg >> 6) & 0x01;

    if (copro_being)
    {
        if (gstFirmwareInfo.copro_firmware_info.upg_state == 2) //coproת������
        {
            return true;
        }
        else 
        {
            return false;
        }
        
    }
    return false;

    // /*pcs��copro�̼�������*/
    // if(pcs_being && copro_being)
    // {
    //     /*pcs��copro�̼����·����*/
    //     if((gstFirmwareInfo.pcs_firmware_info.upg_state == 2) && (gstFirmwareInfo.copro_firmware_info.upg_state == 2))
    //     {
    //         upgrade_state = 1;
    //     }
    //     else if((gstFirmwareInfo.pcs_firmware_info.upg_state == 3) || (gstFirmwareInfo.copro_firmware_info.upg_state == 3))
    //     {
    //         upgrade_state = 2;
    //     }
    //     else if(gstFirmwareInfo.copro_firmware_info.upg_state == 4)
    //     {
    //         upgrade_state = 7;
    //     }
    //     else if(gstFirmwareInfo.copro_firmware_info.upg_state == 5)
    //     {
    //         upgrade_state = 8;
    //     }
    // }
    // else if(pcs_being && !copro_being)
    // {
    //     if(gstFirmwareInfo.pcs_firmware_info.upg_state == 2)
    //     {
    //         upgrade_state = 3;
    //     }
    //     else if(gstFirmwareInfo.pcs_firmware_info.upg_state == 3)
    //     {
    //         upgrade_state = 4;
    //     }
    // }
    // else if(!pcs_being && copro_being)
    // {
    //     if(gstFirmwareInfo.copro_firmware_info.upg_state == 2)
    //     {
    //         upgrade_state = 5;
    //     }
    //     else if(gstFirmwareInfo.copro_firmware_info.upg_state == 3)
    //     {
    //         upgrade_state = 6;
    //     }
    //     else if(gstFirmwareInfo.copro_firmware_info.upg_state == 4)
    //     {
    //         upgrade_state = 7;
    //     }
    //     else if(gstFirmwareInfo.copro_firmware_info.upg_state == 5)
    //     {
    //         upgrade_state = 8;
    //     }
    // }

    // if(upgrade_state == 3)
    // {
    //     UpLoadStatus(1);
    // }

    // if((upgrade_state == 2) || (upgrade_state == 4) || (upgrade_state == 6))
    // {
    //     UpLoadStatus(0);
    // }

    // if((upgrade_state == 1) || (upgrade_state == 5))
    // {
    //     /*��������Copro*/
    //     return true;
    // }

    // if((upgrade_state == 7) || (upgrade_state == 8))
    // {
    //     /* Copro ת������ */
    //     return false;
    // }

    /*������Copro*/
//    return false;
}

static void _ArcUpgraderadeSendStart()
{
    uint16_t check_sum;

    pack_num_index = 0;
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = COM_ADDR;
    _upgrate_data[2] = ARC_COM_ADDR;
    _upgrate_data[3] = CMD_UPGRATE_START & 0xFF;
    _upgrate_data[4] = (CMD_UPGRATE_START >> 8) & 0xFF;
    _upgrate_data[5] = 0x05;//�����򳤶�
    _upgrate_data[6] = 0x00;

    _upgrate_data[7] = gFirmwareUpgrateInfo_arc.ack_firmware_len & 0xFF;//�ļ��ܳ���
    _upgrate_data[8] = (gFirmwareUpgrateInfo_arc.ack_firmware_len >> 8) & 0xFF;
    _upgrate_data[9] = (gFirmwareUpgrateInfo_arc.ack_firmware_len >> 16) & 0xFF;
    _upgrate_data[10] = (gFirmwareUpgrateInfo_arc.ack_firmware_len >> 24) & 0xFF;
    _upgrate_data[11] = 0x02;//space num

    space_num = _upgrate_data[11];
    check_sum = CheckSum((uint8_t*)&_upgrate_data, 12);
    _upgrate_data[12] = check_sum & 0xFF;
    _upgrate_data[13] = (check_sum >> 8) & 0xFF;
//    Uart0Send(_upgrate_data, 12);
    PrivateDataSend(EN_COMSERVICE_COM_INDEX0, (uint8_t*)&_upgrate_data, 14);
}

static void _CoproUpgraderadeSendStart()
{
    uint16_t check_sum;

    pack_num_index = 0;
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = COM_ADDR;
    _upgrate_data[2] = COPRO_COM_ADDR;
    _upgrate_data[3] = CMD_UPGRATE_START & 0xFF;
    _upgrate_data[4] = (CMD_UPGRATE_START >> 8) & 0xFF;
    _upgrate_data[5] = 0x05;//�����򳤶�
    _upgrate_data[6] = 0x00;

    _upgrate_data[7] = (gFirmwareUpgrateInfo_copro.ack_firmware_len) & 0xFF;//�ļ��ܳ���
    _upgrate_data[8] = (gFirmwareUpgrateInfo_copro.ack_firmware_len >> 8) & 0xFF;
    _upgrate_data[9] = (gFirmwareUpgrateInfo_copro.ack_firmware_len >> 16) & 0xFF;
    _upgrate_data[10] = (gFirmwareUpgrateInfo_copro.ack_firmware_len >> 24) & 0xFF;   //todo: �޸�Ϊ copro �̼�����
    _upgrate_data[11] = 0x02;//space num

    space_num = _upgrate_data[11];
    check_sum = CheckSum((uint8_t*)&_upgrate_data, 12);
    _upgrate_data[12] = check_sum & 0xFF;
    _upgrate_data[13] = (check_sum >> 8) & 0xFF;
    // _upgrate_data[7] = check_sum & 0xFF;
    // _upgrate_data[8] = (check_sum >> 8) & 0xFF;
//    Uart0Send(_upgrate_data, 12);
     PrivateDataSend(EN_COMSERVICE_COM_INDEX2, (uint8_t*)&_upgrate_data, 14);
//    PrivateDataSend(EN_COMSERVICE_COM_INDEX2, (uint8_t*)&_upgrate_data, 9);
}


static void _ArsUpgradeSendBusy(uint16_t size, uint32_t offset)
{
    uint16_t check_sum;

    pack_num_index ++;
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = COM_ADDR;
    _upgrate_data[2] = ARC_COM_ADDR;
    _upgrate_data[3] = CMD_UPGRATE_BUSY & 0xFF;
    _upgrate_data[4] = (CMD_UPGRATE_BUSY >> 8) & 0xFF;
    _upgrate_data[5] = (size + space_num) & 0xFF;//�����򳤶�
    _upgrate_data[6] = ((size + space_num) >> 8) & 0xFF;
    _upgrate_data[7] = pack_num_index & 0xFF;//�������
    _upgrate_data[8] = (pack_num_index >> 8) & 0xFF;

    SpiFlashReadBytes(ARC_FIRMWARE_ADDR + offset, &_upgrate_data[9], size);
    check_sum = CheckSum((uint8_t*)&_upgrate_data, size + 9);
    _upgrate_data[9 + size] = check_sum & 0xFF;
    _upgrate_data[10 + size] = (check_sum >> 8) & 0xFF;
    
//    Uart0Send(_upgrate_data, size + 11);
    PrivateDataSend(EN_COMSERVICE_COM_INDEX0, (uint8_t*)&_upgrate_data, size + 11);
}


static void _CoproUpgradeSendBusy(uint16_t size, uint32_t offset)
{
    uint16_t check_sum;

    pack_num_index ++;
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = COM_ADDR;
    _upgrate_data[2] = COPRO_COM_ADDR;
    _upgrate_data[3] = CMD_UPGRATE_BUSY & 0xFF;
    _upgrate_data[4] = (CMD_UPGRATE_BUSY >> 8) & 0xFF;
    _upgrate_data[5] = (size + space_num) & 0xFF;//�����򳤶�
    _upgrate_data[6] = ((size + space_num) >> 8) & 0xFF;
    _upgrate_data[7] = pack_num_index & 0xFF;//�������
    _upgrate_data[8] = (pack_num_index >> 8) & 0xFF;

    SpiFlashReadBytes(COPRO_FIRMWARE_ADDR + offset, &_upgrate_data[9], size);
    check_sum = CheckSum((uint8_t*)&_upgrate_data, size + 9);
    _upgrate_data[9 + size] = check_sum & 0xFF;
    _upgrate_data[10 + size] = (check_sum >> 8) & 0xFF;
    
//    Uart0Send(_upgrate_data, size + 11);
    PrivateDataSend(EN_COMSERVICE_COM_INDEX2, (uint8_t*)&_upgrate_data, size + 11);
}

static void _ArsUpgradeSendEnd()
{
    uint16_t check_sum;
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = COM_ADDR;
    _upgrate_data[2] = ARC_COM_ADDR;
    _upgrate_data[3] = CMD_UPGRATE_END & 0xFF;
    _upgrate_data[4] = (CMD_UPGRATE_END >> 8) & 0xFF;
    _upgrate_data[5] = 0x00;
    _upgrate_data[6] = 0x00;
    check_sum = CheckSum((uint8_t*)&_upgrate_data, 7);
    _upgrate_data[7] = check_sum & 0xFF;
    _upgrate_data[8] = (check_sum >> 8) & 0xFF;
//    Uart0Send(_upgrate_data, 9);
    PrivateDataSend(EN_COMSERVICE_COM_INDEX0, (uint8_t*)&_upgrate_data, 9);
}


static void _CoproUpgradeSendEnd()
{
    uint16_t check_sum;
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = COM_ADDR;
    _upgrate_data[2] = COPRO_COM_ADDR;
    _upgrate_data[3] = CMD_UPGRATE_END & 0xFF;
    _upgrate_data[4] = (CMD_UPGRATE_END >> 8) & 0xFF;
    _upgrate_data[5] = 0x00;
    _upgrate_data[6] = 0x00;
    check_sum = CheckSum((uint8_t*)&_upgrate_data, 7);
    _upgrate_data[7] = check_sum & 0xFF;
    _upgrate_data[8] = (check_sum >> 8) & 0xFF;
//    Uart0Send(_upgrate_data, 9);
    PrivateDataSend(EN_COMSERVICE_COM_INDEX2, (uint8_t*)&_upgrate_data, 9);
}

static EN_BMS_FIRMWARE_UPGRATE_STATUS _ArsFirmwareAnalysis()//�����ķ���������ö�ٱ���
{
    uint16_t temp[256];    
    uint32_t offset = 0;//ƫ����
//    uint32_t i = 0;
//    uint32_t pack_num = 0;
//    uint32_t last_data = 0;
//    uint32_t check_sum = 0;

    offset = 0;
    //��flash�ж�ȡ����

    SpiFlashReadBytes(ARC_FIRMWARE_ADDR, temp, 256);
    if(temp[0] != garrArcFirmwareType[0] || temp[1] != garrArcFirmwareType[1] || temp[2] != garrArcFirmwareType[2])//    /*��ȡ��һ���ļ�ͷ*/��ʼ������ͷ�Ƿ�Ϊ0X0C
    {
        return EN_FIRMWARE_UPGRATE_STATUS_ERROR;//�̼���������
    }
    if(temp[3] == garrArcFirmwareType[3])
    {
        gFirmwareUpgrateInfo_arc.ack_offset = offset;
        gFirmwareUpgrateInfo_arc.ack_firmware_len = ((uint32_t)temp[7] | ((uint32_t)temp[8] << 8) | ((uint32_t)temp[9] << 16) | ((uint32_t)temp[10] << 24));///* �̼����� */
        gFirmwareUpgrateInfo_arc.ack_checksum = ((uint32_t)temp[252] | ((uint32_t)temp[253] << 8) | ((uint32_t)temp[254] << 16) | ((uint32_t)temp[255] << 24));//��λ��� /* �̼�У��� ������У��� */
        gFirmwareUpgrateInfo_arc.ack_existed = 1;
        gFirmwareUpgrateInfo_arc.ack_seek = 0;

//        pack_num = gFirmwareUpgrateInfo.arc_firmware_len / 256;
//        last_data = gFirmwareUpgrateInfo.arc_firmware_len % 256;
//        if(last_data)
//        {
//            pack_num++;
//        }

//        for(i = 1; i < pack_num; i++)
//        {
//            SpiFlashReadBytes(ARC_FIRMWARE_ADDR + i * 256, temp, 256);

//            if((i== pack_num - 1) && (last_data != 0))
//            {
//                check_sum += CheckSumFun((uint8_t*)temp, last_data, 1);
//            }
//            else
//            {
//                check_sum += CheckSumFun((uint8_t*)temp, 256, 1);
//            }            
//        }

//        if(check_sum == gFirmwareUpgrateInfo.arc_checksum)
//        {
//            printf("Check Arc Firmware ok!");
//        }
//        else
//        {
//            printf("Check Arc Firmware Error!");
//        }
        return EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE;//����׼��
    }
    else
    {
        return EN_FIRMWARE_UPGRATE_STATUS_ERROR;//�̼�ͷ������ֱ���ж�Ϊ����
    }
}


static EN_BMS_FIRMWARE_UPGRATE_STATUS _CoproFirmwareAnalysis()//�����ķ���������ö�ٱ���
{
    uint16_t temp[256];    
    uint32_t offset = 0;//ƫ����

    offset = 0;
    //��flash�ж�ȡ����

    SpiFlashReadBytes(COPRO_FIRMWARE_ADDR, temp, 256);
    if(temp[0] != garrCoproFirmwareType[0] || temp[1] != garrCoproFirmwareType[1] || temp[2] != garrCoproFirmwareType[2])//    /*��ȡ��һ���ļ�ͷ*/��ʼ������ͷ�Ƿ�Ϊ0X0C
    {
        return EN_FIRMWARE_UPGRATE_STATUS_ERROR;//�̼���������
    }
    if(temp[3] == garrCoproFirmwareType[3])
    {
        gFirmwareUpgrateInfo_copro.ack_offset = offset;
        gFirmwareUpgrateInfo_copro.ack_firmware_len = ((uint32_t)temp[7] | ((uint32_t)temp[8] << 8) | ((uint32_t)temp[9] << 16) | ((uint32_t)temp[10] << 24));///* �̼����� */
        gFirmwareUpgrateInfo_copro.ack_checksum = ((uint32_t)temp[252] | ((uint32_t)temp[253] << 8) | ((uint32_t)temp[254] << 16) | ((uint32_t)temp[255] << 24));//��λ��� /* �̼�У��� ������У��� */
        gFirmwareUpgrateInfo_copro.ack_existed = 1;
        gFirmwareUpgrateInfo_copro.ack_seek = 0;
        return EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE;//����׼��
    }
    else
    {
        return EN_FIRMWARE_UPGRATE_STATUS_ERROR;//�̼�ͷ������ֱ���ж�Ϊ����
    }
}



void ArcUpgrateEnable(void)
{
    gFirmwareUpgrateInfo_arc.ack_upgrate_en = 1;
}


void ArcUpgrateFirmwareProcess(void)
{
    static EN_BMS_FIRMWARE_UPGRATE_STATUS arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_IDLE;
    static uint32_t timeout;
    switch(arc_upgrate_status)
    {
        case EN_FIRMWARE_UPGRATE_STATUS_IDLE:
        {
            if(_ArcUpgradeCheck() == true)
//            if(true)
            {
                memset((uint16_t*)&gFirmwareUpgrateInfo_arc, 0x00, (int)sizeof(FirmwareUpgrateInfo_t));
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_START;//��ʼ����
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_START:
        {
            arc_upgrate_status = _ArsFirmwareAnalysis();//�������̼�У��
            if (arc_upgrate_status == EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE)
            {
                gstFirmwareInfo.arc_firmware_info.upg_state = 1;
            }
            
            timeout = TimGetSystime();
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE:
        {
            if((TimGetSystime() - timeout) < 1500)
            {
                break;
            }

            if(gFirmwareUpgrateInfo_arc.ack_existed == 1)
            {
                gFirmwareUpgrateInfo_arc.ack_existed = 0;
            }
            _ArcUpgraderadeSendStart();//��ʼ����
            arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE_ACK;
            //rintf("EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE\r\n");
            timeout = TimGetSystime();
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE_ACK:
        {
            if((TimGetSystime() - timeout) < 100)
            {
                break;
            }

            if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX0].cmd_upgrate_start == 0x00)
            {
                ResetWriteResAckState(EN_COMSERVICE_COM_INDEX0, CMD_UPGRATE_START);
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY;
                timeout = TimGetSystime();
            }
                        
            if((TimGetSystime() - timeout) > UPGRATE_TIME_OUT)
            {
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY:
        {
            uint16_t size_per_transfer = 0;
            if((TimGetSystime() - timeout) < 100)
            {
                break;
            }

            if(gFirmwareUpgrateInfo_arc.ack_seek == 0)
            {
                /*��һ����256��ͷ*/
                size_per_transfer = 256;
            }
            else if(gFirmwareUpgrateInfo_arc.ack_firmware_len - gFirmwareUpgrateInfo_arc.ack_seek < UPGRATE_DATA_SIZE)
            {
                size_per_transfer = gFirmwareUpgrateInfo_arc.ack_firmware_len - gFirmwareUpgrateInfo_arc.ack_seek;//���һ������
            }
            else
            {
                size_per_transfer = UPGRATE_DATA_SIZE;//ÿ���̶�2k
            }
            pack_size = size_per_transfer;
            _ArsUpgradeSendBusy(size_per_transfer, gFirmwareUpgrateInfo_arc.ack_offset + gFirmwareUpgrateInfo_arc.ack_seek);//��������
            gFirmwareUpgrateInfo_arc.ack_seek += size_per_transfer;//����ƫ�Ƶ�ַ
            timeout = TimGetSystime();
            arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_WAIT_DATA_BUSY_ACK;
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_DATA_BUSY_ACK:
        {
            if((TimGetSystime() - timeout) < 100)
            {
                break;
            }
            /* ���� */
            if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX0].cmd_upgrate_busy == 1)
            {
                ResetWriteResAckState(EN_COMSERVICE_COM_INDEX0, CMD_UPGRATE_BUSY);
                resend_count ++;
                gFirmwareUpgrateInfo_arc.ack_seek -= pack_size;//����ƫ�Ƶ�ַ
                pack_num_index = pack_num_index - 1;
                if(resend_count > 3)
                {
                    arc_upgrate_status =  EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                }
            }
            /* ������ȷ */
            else if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX0].cmd_upgrate_busy == 0)
            {
                resend_count = 0;
                ResetWriteResAckState(EN_COMSERVICE_COM_INDEX0, CMD_UPGRATE_BUSY);
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY;
                timeout = TimGetSystime();
            }

            if(gFirmwareUpgrateInfo_arc.ack_seek >= gFirmwareUpgrateInfo_arc.ack_firmware_len)
            {
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_END;//����
                timeout = TimGetSystime();
            }

            if((TimGetSystime() - timeout) > UPGRATE_TIME_OUT)
            {
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_END:
        {
            /* ��Ͳ���С��150ms�����������ʧ�� */
            if((TimGetSystime() - timeout) < 300)
            {
                break;
            }
            _ArsUpgradeSendEnd();
            arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK;//�������
            timeout = TimGetSystime();
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK:
        {
            if((TimGetSystime() - timeout) > UPGRATE_TIME_OUT)
            {
    //            printf("upgrade error\r\n");
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                /*���߼�ؿ�����ʧ��*/
                UpLoadStatus(0);
                gstFirmwareInfo.arc_firmware_info.upg_state = 5;    //arcת��ʧ��
            }
            else
            {
                if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX0].cmd_upgrate_end == 0)
                {
                    ResetWriteResAckState(EN_COMSERVICE_COM_INDEX0, CMD_UPGRATE_END);
                    arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_OK;
                    /*���߼�ؿ������ɹ�*/
                    UpLoadStatus(1);
                    gstFirmwareInfo.arc_firmware_info.upg_state = 4;    //arcת���ɹ�
         //           printf("upgrade ok\r\n");
                }                
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_ERROR:
        {
            UpLoadStatus(0);
            gstFirmwareInfo.arc_firmware_info.upg_state = 5;    //arcת��ʧ��
            resend_count = 0;
            gFirmwareUpgrateInfo_arc.ack_upg_state = 2;
            gFirmwareUpgrateInfo_arc.ack_existed = 0;
            gFirmwareUpgrateInfo_arc.ack_seek = 0;
            arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_IDLE;
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_OK:
        {
            resend_count = 0;
            gFirmwareUpgrateInfo_arc.ack_upg_state = 1;
            gFirmwareUpgrateInfo_arc.ack_existed = 0;
            gFirmwareUpgrateInfo_arc.ack_seek = 0;
            arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_IDLE;
            break;
        }
        default:
            break;
    }
}

void CoproUpgrateFirmwareProcess(void)
{
    static EN_BMS_FIRMWARE_UPGRATE_STATUS copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_IDLE;
    static uint32_t timeout;
    switch(copro_upgrate_status)
    {
        case EN_FIRMWARE_UPGRATE_STATUS_IDLE:
        {
            if(_CoproUpgradeCheck() == true)
            //if(true)
            {
                memset((uint16_t*)&gFirmwareUpgrateInfo_copro, 0x00, (int)sizeof(FirmwareUpgrateInfo_t));
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_START;//��ʼ����
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_START:
        {
            copro_upgrate_status = _CoproFirmwareAnalysis();//�������̼�У��
            if (copro_upgrate_status == EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE)
            {
                gstFirmwareInfo.copro_firmware_info.upg_state = 1;
            }
            
            timeout = TimGetSystime();
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE:
        {
            if((TimGetSystime() - timeout) < 1500)
            {
                break;
            }

            if(gFirmwareUpgrateInfo_copro.ack_existed == 1)
            {
                gFirmwareUpgrateInfo_copro.ack_existed = 0;
            }
            _CoproUpgraderadeSendStart();//��ʼ����
            copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE_ACK;
            //printf("EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE\r\n");
            timeout = TimGetSystime();
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE_ACK:
        {
            if((TimGetSystime() - timeout) < 100)
            {
                break;
            }

            if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX2].cmd_upgrate_start == 0x00)
            {
                ResetWriteResAckState(EN_COMSERVICE_COM_INDEX2, CMD_UPGRATE_START);
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY;
                timeout = TimGetSystime();
            }

            if((TimGetSystime() - timeout) > UPGRATE_TIME_OUT)
            {
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY:
        {
            uint16_t size_per_transfer = 0;
            if((TimGetSystime() - timeout) < 100)
            {
                break;
            }

            // if(gFirmwareUpgrateInfo_copro.ack_upg_state == 0)
            // {
            //     /*��һ����256��ͷ*/
            //     size_per_transfer = 256;
            // }

            if(gFirmwareUpgrateInfo_copro.ack_pack_num == 0)
            {
                size_per_transfer = 256;
            }
            else 
            {
                size_per_transfer = UPGRATE_DATA_SIZE;
            }

            if(gFirmwareUpgrateInfo_copro.ack_firmware_len - gFirmwareUpgrateInfo_copro.ack_seek < UPGRATE_DATA_SIZE)
            {
                pack_size = gFirmwareUpgrateInfo_copro.ack_firmware_len - gFirmwareUpgrateInfo_copro.ack_seek;//���һ������
            }
            else
            {
                pack_size = size_per_transfer;//ÿ���̶�2k
            }
            _CoproUpgradeSendBusy(pack_size, gFirmwareUpgrateInfo_copro.ack_offset + gFirmwareUpgrateInfo_copro.ack_seek);//��������
            gFirmwareUpgrateInfo_copro.ack_seek += pack_size;//����ƫ�Ƶ�ַ
            gFirmwareUpgrateInfo_copro.ack_pack_num++;
            timeout = TimGetSystime();
            copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_WAIT_DATA_BUSY_ACK;
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_DATA_BUSY_ACK:
        {
            if((TimGetSystime() - timeout) < 100)
            {
                break;
            }
            /* ���� */
            if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX2].cmd_upgrate_busy == 1)
            {
                ResetWriteResAckState(EN_COMSERVICE_COM_INDEX2, CMD_UPGRATE_BUSY);
                resend_count ++;
                gFirmwareUpgrateInfo_copro.ack_seek -= pack_size;//����ƫ�Ƶ�ַ
                pack_num_index = pack_num_index - 1;
                if(resend_count > 3)
                {
                    copro_upgrate_status =  EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                }
            }
            /* ������ȷ */
            else if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX2].cmd_upgrate_busy == 0)
            {
                resend_count = 0;
                ResetWriteResAckState(EN_COMSERVICE_COM_INDEX2, CMD_UPGRATE_BUSY);
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY;
                timeout = TimGetSystime();
            }

            if(gFirmwareUpgrateInfo_copro.ack_seek >= gFirmwareUpgrateInfo_copro.ack_firmware_len)
            {
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_END;//����
                timeout = TimGetSystime();
            }

            if((TimGetSystime() - timeout) > UPGRATE_TIME_OUT)
            {
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_SEND_END:
        {
            /* ��Ͳ���С��150ms�����������ʧ�� */
            if((TimGetSystime() - timeout) < 300)
            {
                break;
            }
            _CoproUpgradeSendEnd();
            copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK;//�������
            timeout = TimGetSystime();
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK:
        {
            if((TimGetSystime() - timeout) > UPGRATE_TIME_OUT)
            {
                //printf("upgrade error\r\n");
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                /*���߼�ؿ�����ʧ��*/
                UpLoadStatus(0);
                gstFirmwareInfo.copro_firmware_info.upg_state = 5;    //coproת��ʧ��
            }
            else
            {
                if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX2].cmd_upgrate_end == 0)
                {
                    ResetWriteResAckState(EN_COMSERVICE_COM_INDEX2, CMD_UPGRATE_END);
                    copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_OK;
                    /*���߼�ؿ������ɹ�*/
                    UpLoadStatus(1);
                    gstFirmwareInfo.copro_firmware_info.upg_state = 4;    //coproת���ɹ�
                    //printf("upgrade ok\r\n") ;
                }                
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_ERROR:
        {
            UpLoadStatus(0);
            gstFirmwareInfo.copro_firmware_info.upg_state = 5;    //coproת��ʧ��
            resend_count = 0;
            gFirmwareUpgrateInfo_copro.ack_upg_state = 2;
            gFirmwareUpgrateInfo_copro.ack_existed = 0;
            gFirmwareUpgrateInfo_copro.ack_seek = 0;
            copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_IDLE;
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_OK:
        {
            resend_count = 0;
            gFirmwareUpgrateInfo_copro.ack_upg_state = 1;
            gFirmwareUpgrateInfo_copro.ack_existed = 0;
            gFirmwareUpgrateInfo_copro.ack_seek = 0;
            copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_IDLE;
            break;
        }
        default:
            break;
    }
}


bool EthernentUpgradeRecv(MonitorUdp_t *p_monitor_frame, uint16_t *p_data)
{
    uint32_t start_address = 0;
    uint16_t first_pack_flag = 0;
    static FirmwareRecvInfo_t *p_firmware_info = NULL;
    static uint32_t firmware_start_addr = 0;
    static uint32_t firmware_check_addr = 0;

    if((p_monitor_frame->Length == 256) && (p_monitor_frame->Addr == 0x24600000))//��һ���İ�ͷDSP
    {
        memset((uint16_t*)&gstFirmwareInfo.pcs_firmware_info, 0x00, (int)sizeof(FirmwareRecvInfoS_t));
        p_firmware_info = &gstFirmwareInfo.pcs_firmware_info;
        firmware_start_addr = DSP_FIRMWARE_ADDR;
        firmware_check_addr = DSP_FIRMWARE_FRAME_ADDR;
        /*�洢������ʼ��־*/
        p_firmware_info->upg_state = 1;
        first_pack_flag = 1;
//        printf("gstFirmwareInfo.pcs_firmware_info.finish reg:0x%04X\r\n", gstFirmwareInfo.pcs_firmware_info.upg_state);
//        printf("gstFirmwareInfo.arc_firmware_info.finish reg:0x%04X\r\n", gstFirmwareInfo.arc_firmware_info.upg_state);
//        printf("checksum:0X%04X%04X,  checksumtemp:0X%04X%04X\r\n", (uint16_t)(p_firmware_info->checksum >> 16), (uint16_t)p_firmware_info->checksum, (uint16_t)(p_firmware_info->checksumtemp >> 16), (uint16_t)p_firmware_info->checksumtemp);
//        printf("dsp 1 OK!\r\n");
    }
    else if((p_monitor_frame->Length == 256) && (p_monitor_frame->Addr == 0x24700000))//��һ���İ�ͷARC
    {
        memset((uint16_t*)&gstFirmwareInfo.arc_firmware_info, 0x00, (int)sizeof(FirmwareRecvInfoS_t));
        p_firmware_info = &gstFirmwareInfo.arc_firmware_info;
        firmware_start_addr = ARC_FIRMWARE_ADDR;
        firmware_check_addr = ARC_FIRMWARE_FRAME_ADDR;
        /*�洢������ʼ��־*/
        p_firmware_info->upg_state = 1;
        first_pack_flag = 1;
//        printf("gstFirmwareInfo.pcs_firmware_info.finish reg:0x%04X\r\n", gstFirmwareInfo.pcs_firmware_info.upg_state);
//        printf("gstFirmwareInfo.arc_firmware_info.finish reg:0x%04X\r\n", gstFirmwareInfo.arc_firmware_info.upg_state);
//        printf("checksum:0X%04X%04X,  checksumtemp:0X%04X%04X\r\n", (uint16_t)(p_firmware_info->checksum >> 16), (uint16_t)p_firmware_info->checksum, (uint16_t)(p_firmware_info->checksumtemp >> 16), (uint16_t)p_firmware_info->checksumtemp);
//        printf("arc 1 OK!\r\n");
    }
    else if((p_monitor_frame->Length == 256) && (p_monitor_frame->Addr == 0x24800000))//��һ���İ�ͷcopro
    {
        memset((uint16_t*)&gstFirmwareInfo.copro_firmware_info, 0x00, (int)sizeof(FirmwareRecvInfoS_t));
        p_firmware_info = &gstFirmwareInfo.copro_firmware_info;
        firmware_start_addr = COPRO_FIRMWARE_ADDR;
        firmware_check_addr = COPRO_FIRMWARE_FRAME_ADDR;
        /*�洢������ʼ��־*/
        p_firmware_info->upg_state = 1;
        first_pack_flag = 1;
    }
    

    if(p_firmware_info == NULL)
    {
        // �˴���ʾ��Чָ��
        return false;
    }

    start_address = (p_monitor_frame->Addr & 0x000FFFFF) + firmware_start_addr;
    SpiFlashWriteBytesWithErase(start_address, &p_data[10], p_monitor_frame->Length);
    if(first_pack_flag)
    {
        //������ظ������ݵĹ̼�������
        memcpy((uint16_t*)&p_firmware_info->firmware_type, &p_data[10], 4);
        //������ظ������ݵİ汾��
        memcpy((uint16_t*)&p_firmware_info->firmware_version, &p_data[14], 3);
        //������ظ������ݵĳ���
        p_firmware_info->firmware_len = (uint32_t)p_data[20] << 24 | (uint32_t)p_data[19] << 16 | (uint32_t)p_data[18] << 8 | (uint32_t)p_data[17];
        p_firmware_info->firmware_len_temp = p_monitor_frame->Length;
        //������ظ������ݵ�У����
        p_firmware_info->checksum = (uint32_t)p_data[265] << 24 | (uint32_t)p_data[264] << 16 | (uint32_t)p_data[263] << 8 | (uint32_t)p_data[262];
//        printf("arc first OK!\r\n");
    }
    else
    {
        /*����flash�����ݽ���У��ͼ���*/
        SpiFlashReadBytes(start_address, _upgrate_data, p_monitor_frame->Length);
//        printf("write addr:0x%04X%04X\r\n", (uint16_t)(p_monitor_frame->Addr >> 16), (uint16_t)p_monitor_frame->Addr);
        p_firmware_info->checksumtemp += CheckSumFun((uint8_t*)&_upgrate_data, p_monitor_frame->Length, 1);
//        printf("checksumtemp:0x%04X%04X\r\n", (uint16_t)(p_firmware_info->checksumtemp >> 16), (uint16_t)p_firmware_info->checksumtemp);

        //���������ȼӺ�
        p_firmware_info->firmware_len_temp += p_monitor_frame->Length;
        //���������λ
        if(p_firmware_info->firmware_len_temp == p_firmware_info->firmware_len)
        {
            /*�洢�Ĺ̼�����У���Ƿ���ȷ*/
            if(p_firmware_info->checksum == p_firmware_info->checksumtemp)
            {
//                printf("checksum:0X%04X%04X,  checksumtemp:0X%04X%04X\r\n", (uint16_t)(p_firmware_info->checksum >> 16), (uint16_t)p_firmware_info->checksum, (uint16_t)(p_firmware_info->checksumtemp >> 16), (uint16_t)p_firmware_info->checksumtemp);
                /*���������ɹ�*/
                p_firmware_info->upg_state = 2;

                /*�洢�̼������У��OK������_garrReceiveDataBuff����ռ���flashд��̼����±�־*/
                memset(p_data, 0xA0, 256);
                SpiFlashWriteBytesWithErase(firmware_check_addr, p_data, 256);
                return true;
            }
            else
            {
//                printf("checksum:0X%04X%04X,  checksumtemp:0X%04X%04X\r\n", (uint16_t)(p_firmware_info->checksum >> 16), (uint16_t)p_firmware_info->checksum, (uint16_t)(p_firmware_info->checksumtemp >> 16), (uint16_t)p_firmware_info->checksumtemp);
                /*��������ʧ��*/
                p_firmware_info->upg_state = 3;
            }

            //TODO ����ʹ�ã��޸�hold�Ĵ��������߼�ع̼������ɹ���
//            if((gstFirmwareInfo.pcs_firmware_info.upg_state == 2) && (gstFirmwareInfo.arc_firmware_info.upg_state == 2))
//            {
//                UpLoadStatus(1);

//                printf("Upgrade OK! \r\n");
//            }
            p_firmware_info = NULL;
        }
    }

    return false;
}


void VersionInfoGet(void)
{
    uint16_t temp[25];
    memset((uint16_t*)&gstVersionInfo, 0x00, (int)sizeof(VersionInfoS_t));

    SpiFlashReadBytes(DSP_FIRMWARE_ADDR, temp, 25);
    gstVersionInfo.pcs_version_info.model_id = 0xC2C1;
    gstVersionInfo.pcs_version_info.soft_version = (temp[4] << 12) | (temp[5] << 6) | temp[6];

    for(int i = 0; i < 6; i++)
    {
        gstVersionInfo.pcs_version_info.extern_version[i] = temp[11 + 2 * i] | (temp[11 + 2 * i + 1] << 8);
    }
    gstVersionInfo.pcs_version_info.device_type = (uint16_t)EN_ROCA3_PCS_15KHT;
}

void FirmwareParamInit(void)
{
    VersionInfoGet();
}
```

### `app/firmware_upgrate.h`

```c

#ifndef __FIRMWARE_UPGRATE_H_
#define __FIRMWARE_UPGRATE_H_
#include "dev_flash.h"
#include "private_analysis.h"
#include "ethernet_analysis.h"
#include "modbus_analysis.h"
#include "system.h"


#define UPGRADE_START           0x80F0
#define UPGRADE_BUSY            0x80F1
#define UPGRADE_END             0x80FF

#define COPRO_FIRMWARE_FRAME_ADDR       0x00100000
#define COPRO_FIRMWARE_ADDR             0x00101000
#define ARC_FIRMWARE_FRAME_ADDR         0x00080000
#define ARC_FIRMWARE_ADDR               0x00081000
#define DSP_FIRMWARE_FRAME_ADDR         0x00000000
#define DSP_FIRMWARE_ADDR               0x00001000

#define UPGRADE_FLAG   0xA0

typedef enum
{
    EN_ROCA3_PCS_16KHS = 0x43,
    EN_ROCA3_PCS_15KHT = 0xC0,
    EN_PCS_TYPE_NUM = 0xFF,
} EN_PCS_TYPE;

typedef enum
{
    EN_FIRMWARE_UPGRATE_STATUS_IDLE,//����ģʽ
    EN_FIRMWARE_UPGRATE_STATUS_START,//������ʼ
    EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE,//Ԥ��״̬
    EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE_ACK,
    EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY,//��������
    EN_FIRMWARE_UPGRATE_STATUS_WAIT_DATA_BUSY_ACK,//�����еȴ����
    EN_FIRMWARE_UPGRATE_STATUS_SEND_END,//���ͽ�ֹ
    EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK,//���ͽ�ֹ���
    EN_FIRMWARE_UPGRATE_STATUS_ERROR,//״̬����
    EN_FIRMWARE_UPGRATE_STATUS_OK,//OK
} EN_BMS_FIRMWARE_UPGRATE_STATUS;

typedef struct _firmware_upgrate_info_stru_t
{
    // uint8_t         arc_upgrate_en;
    // uint8_t         arc_existed;
    // uint32_t        arc_offset;
    // uint32_t        arc_firmware_len;
    // uint32_t        arc_checksum;
    // uint32_t        arc_seek;
    // uint8_t         arc_ack;
    // uint8_t         arc_end_ack;
    // uint8_t         arc_upg_state;  // 0:��Ч 1���洢flash�ɹ� 2���洢flashʧ�� 3���·���ģ��ɹ� 4���·���ģ��ʧ��

    uint8_t         ack_upgrate_en;
    uint8_t         ack_existed;
    uint32_t        ack_offset;
    uint32_t        ack_firmware_len;
    uint32_t        ack_checksum;
    uint32_t        ack_seek;
    uint8_t         ack_ack;
    uint8_t         ack_end_ack;
    uint8_t         ack_upg_state;  // 0:��Ч 1���洢
    uint16_t        ack_pack_num;

} FirmwareUpgrateInfo_t;

typedef struct _firmware_recv_info_stru_t
{
    uint16_t        firmware_type[4];       //�����̼�����
    uint16_t        firmware_version[3];    //�����İ汾��
    uint32_t        firmware_len;           //�̼�����
    uint32_t        firmware_len_temp;      //�̼�������ʱֵ
    uint32_t        checksum;                   //32λУ��Ӻ�
    uint32_t        checksumtemp;               //32λУ��Ӻ���ʱֵ
    uint16_t        upg_state;                     //����������״̬ 0����Ч 1�������� 2�������ɹ� 3������ʧ�� 4:ת���ɹ� 5��ת��ʧ��
} FirmwareRecvInfo_t;

typedef struct _firmware_recv_infos_stru_t
{
    FirmwareRecvInfo_t  pcs_firmware_info;
    FirmwareRecvInfo_t  arc_firmware_info;
    FirmwareRecvInfo_t  copro_firmware_info; // MCUЭ�������̼���Ϣ , coprocessor
} FirmwareRecvInfoS_t;

typedef struct _version_info_stru_t
{
    uint16_t        model_id;
    uint16_t        soft_version;       //����汾
    uint16_t        extern_version[6];  //��չ�汾
    uint16_t        device_type;        //�豸���� 0x00-0x7F���������   0x80-0xFF���������
} VersionInfo_t;

typedef struct _version_infos_stru_t
{
    VersionInfo_t  pcs_version_info;
    VersionInfo_t  arc_version_info;
} VersionInfoS_t;




extern FirmwareRecvInfoS_t  gstFirmwareInfo;
extern VersionInfoS_t gstVersionInfo;
extern uint16_t garrDspFirmwareType[4];

void ArcUpgrateFirmwareProcess(void);
void CoproUpgrateFirmwareProcess(void);
void ArcUpgrateEnable(void);
bool EthernentUpgradeRecv(MonitorUdp_t *p_monitor_frame, uint16_t *p_data);
void FirmwareParamInit(void);
bool ArcFirmwareSending(void);
bool CoproFirmwareSending(void);
bool FirmwareUpgrateIsBusy(void);

#endif
```

### `app/flash_default.c`

```c
/*
 * flash_default.c
 *
 * Created on: 2026-01-30
 * Author: Nova007457
 *
 * Factory defaults, serialization, and SPI Flash persistence for UdpDspHoldPara_t
 * (Modbus hold registers 0x6220~0x631F).
 */

#include "flash_default.h"
#include "ethernet_analysis.h"
#include "dev_flash.h"
#include "string.h"
#include <stddef.h>


#define FLASH_FRAME_WORDS          4u

/* Valid / invalid frame headers at FLASH_DspHoldFRAME_START_ADDR */
static const uint16_t Flash_DspHoldType[FLASH_FRAME_WORDS] = {0x55, 0xAA, 0x55, 0xAA};
static const uint16_t Flash_DspHoldType_init[FLASH_FRAME_WORDS] = {0x00, 0x00, 0x00, 0x00};

/* Shared workspace (file scope avoids large stacks on C28x) */
static uint8_t s_flash_byte_buf[2 * sizeof(UdpDspHoldPara_t)];
/* mirror: last saved copy; work: Flash readback + autosave compare scratch */
static UdpDspHoldPara_t s_hold_saved_mirror;
static UdpDspHoldPara_t s_hold_work;

static UdpDspHoldPara_t UdpPCS_DspHoldPara_Init =
{
    .Second_ID = 0,                                     /* 0x6220 Power segment ID: 0=default, 1=6kW .. 6=16kW */
    .work_mode = 0,                                     /* 0x6221 Work mode: 0=self-use, 1=battery first, 2=peak-shaving */
    .charging_input_type = 0,                           /* 0x6222 Forced gen input: 0=grid, 1=generator, 2=auto */
    .set_batt_chg_curr = 0,                             /* 0x6223 Max battery charge current, 0.1 A */
    .set_batt_chg_power = {0, 16000},                   /* 0x6224-0x6225 Max battery charge power, 1 W */
    .set_batt_dsg_curr = 0,                             /* 0x6226 Max battery discharge current, 0.1 A */
    .set_batt_dsg_power = {0, 16000},                   /* 0x6227-0x6228 Max battery discharge power, 1 W */
    .InvPVMaxPower = {0, 24000},                        /* 0x6229-0x622A Max PV power */
    .InvOnGridIFInMaxPower = {0, 24000},                /* 0x622B-0x622C Max on-grid port input power */
    .InvOnGridIFOutMaxPower = {0, 16000},               /* 0x622D-0x622E Max on-grid port output power */
    .InvGenMaxPower = {0, 4000},                        /* 0x622F-0x6230 Max generator power */
    .InvEVMaxInPower = {0, 4000},                       /* 0x6231-0x6232 Max EV charge (absorb) power */
    .InvEVMaxOutPower = {0, 4000},                      /* 0x6233-0x6234 Max EV discharge (supply) power */
    .NetworkPowerMaxPowerLimit = {0, 24000},             /* 0x6235-0x6236 Household import power limit */
    .on_grid_power = {0, 0},                            /* 0x6237-0x6238 Max feed-in / on-grid power */

    .pv_input_mode = 0,                                 /* 0x6250 PV input: 0=independent, 1=parallel, 2=constant V */
    .anti_reverse_flow_enable = 0,                      /* 0x6251 Anti-export: 0=block PV to grid, 1=allow */

    .reserve2 = {0, 0},                                  /* 0x6252-0x6253 Reserved */

    .x_boost = 0,                                        /* 0x6254 X-BOOST: 0=disable, 1=enable */
    .ups_delay_0ms = 1,                                 /* 0x6255 UPS 0 ms transfer: 0=disable, 1=enable */
    .backup_output_enable = 1,                          /* 0x6256 Backup load output: 0=disable, 1=enable */
    .program_ctrl_enable = 0,                           /* 0x6257 Program control: 0=disable, 1=enable */
    .over_volt_derating_enable = 0,                     /* 0x6258 Over-voltage derating */
    .over_freq_derating_enable = 0,                     /* 0x6259 Over-frequency derating */
    .under_volt_derating_enable = 0,                    /* 0x625A Under-voltage derating */
    .under_freq_derating_enable = 0,                    /* 0x625B Under-frequency derating */
    .island_enable = 1,                                /* 0x625C Anti-islanding: 0=disable, 1=enable */
    .ac_coupling_enable = 1,                            /* 0x625D AC coupling: 0=disable, 1=enable */
    .high_low_crossover_enable = 0,                     /* 0x625E HV/LV ride-through: 0=disable, 1=enable */
    .drm_state = 0x0010,                                /* 0x625F DRM bits; default DRM4 (bit4) */
    .LLCEnable = 0,                                     /* 0x6260 LLC after Pack precharge: 0=off, 1=on */

    .cto_meter = 0,                                      /* 0x6266 CT or meter: 0=CT, 1=meter */
    .ct_direction = 0,                                  /* 0x6267 CT direction: 0=forward, 1=reverse */
    .meter_brand = 0,                                    /* 0x6268 Meter brand: 0=CHNT, 1=Acrel */
    .grid_standards = 2,                                /* 0x6269 Grid standard: 2=Australia A, etc. */
    .grad_standards_segment = 0,                         /* 0x626A Grid standard sub-segment */
    .BatteryChgMaxSOC = 100,                             /* 0x626B Charge stop SOC % */
    .OnGridBatteryDsgMinSoc = 10,                       /* 0x626C On-grid discharge stop SOC % */
    .OffGridBatteryDsgMinSoc = 10,                      /* 0x626D Off-grid discharge stop SOC % */

    .reserve4 = {0, 0, 0, 0, 0, 0, 0, 0},               /* 0x626E-0x6275 Reserved */

    .reset_factory = 0,                                 /* 0x6276 Factory reset trigger: write 0x5AA5 */
    .on_grid_reconnect_time = 30,                        /* 0x6277 Grid reconnect delay, s (5-300) */
    .reactive_power_respones_time = 10,                 /* 0x6278 Reactive response time, s */
    .reactiveSet = 0,                                    /* 0x6279 Reactive setpoint -100..100 */
    .reactive_power_ctrl_method = 1,                     /* 0x627A Reactive ctrl: 0=PF, 1=Q, 2=QU, 3=QP */
    .ActiveSet = 100,                                    /* 0x627B Active power limit 0-100 % */
    .FactorSet = 0,                                      /* 0x627C Power factor setpoint */

    .reserve5 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* 0x627D-0x628F Reserved */

    .remote_shutdown_status = 0,                        /* 0x6290 Remote shutdown status */
    .PV_Switch = 1,                                      /* 0x6291 PV switch: 0=disable, 1=enable */
    .PF_Switch = 1,                                      /* 0x6292 PF switch: 0=disable, 1=enable */
    .dsp_wake_en = 3,                                    /* 0x6293 Wake: 1=PV wake, 2=PV wake done, 3=normal */
    .pcs_dsg_ctrl = 1,                                   /* 0x6294 Discharge ctrl: 0=block on fault, 1=allow, 2=forbid */
    .pcs_chg_ctrl = 1,                                   /* 0x6295 Charge ctrl: 0=block on fault, 1=allow, 2=forbid */
    .pcs_user_type = 0,                                 /* 0x6296 User type: 0=user .. 3=internal */
    .ignor_ct_err = 0,                                   /* 0x6297 Ignore CT fault: 0=no, 1=yes */
    .sys_upgrade_status = 0,                             /* 0x6298 System upgrade flag */
    .sys_elec_clear = 0,                                 /* 0x6299 Energy counter clear */
    .sys_shut_down = 0,                                  /* 0x629A Shutdown command */

    .reserve6 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* 0x629B-0x62AF Reserved */

    .batt_dsg_volt_point = 3700,                        /* 0x62B0 Battery discharge stop voltage, 0.1 V */
    .batt_chg_volt_point = 5500,                        /* 0x62B1 Battery charge voltage limit, 0.1 V */
    .pv_uvp_point = 180,                                 /* 0x62B2 PV under-voltage protection, 0.1 V */
    .pv_ovp_point = 1000,                                /* 0x62B3 PV over-voltage protection, 0.1 V */
    .ev_uvp_point = 0,                                   /* 0x62B4 EV under-voltage protection, 0.1 V */
    .ev_ovp_point = 0,                                   /* 0x62B5 EV over-voltage protection, 0.1 V */
    .grid_volt_low_limit = 1750,                        /* 0x62B6 Grid voltage low limit, 0.1 V */
    .grid_volt_upper_limit = 2750,                      /* 0x62B7 Grid voltage high limit, 0.1 V */
    .grid_freq_low_limit = 4500,                        /* 0x62B8 Grid frequency low limit, 0.01 Hz */
    .grid_freq_upper_limit = 5500,                      /* 0x62B9 Grid frequency high limit, 0.01 Hz */
    .bypass_volt_low_limit = 1750,                      /* 0x62BA Bypass voltage low limit, 0.1 V */
    .bypass_volt_upper_limit = 2750,                    /* 0x62BB Bypass voltage high limit, 0.1 V */
    .bypass_freq_low_limit = 4500,                      /* 0x62BC Bypass frequency low limit, 0.01 Hz */
    .bypass_freq_upper_limit = 5500,                    /* 0x62BD Bypass frequency high limit, 0.01 Hz */
    .over_volt_derating_point = 270,                    /* 0x62BE Over-voltage derating point, 1 % */
    .over_freq_derating_point = 5400,                   /* 0x62BF Over-frequency derating point */
    .under_volt_derating_point = 1800,                  /* 0x62C0 Under-voltage derating point, 1 % */
    .under_freq_derating_point = 4600,                  /* 0x62C1 Under-frequency derating point */

    .reserve7 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* 0x62C2-0x62CF Reserved */

    .CalibrationReset = 0,                               /* 0x62D0 Calibration reset */
    .reserve8 = {0, 0, 0, 0},                           /* 0x62D1-0x62D4 Reserved */
    .r_phase_volt_calibration_k_parm = 4096,           /* 0x62D5 R-phase voltage cal K (default 4096 = 1.0) */
    .r_phase_volt_calibration_b_parm = 4096,           /* 0x62D6 R-phase voltage cal B */
    .s_phase_volt_calibration_k_parm = 4096,           /* 0x62D7 S-phase voltage cal K */
    .s_phase_volt_calibration_b_parm = 4096,           /* 0x62D8 S-phase voltage cal B */
    .t_phase_volt_calibration_k_parm = 4096,           /* 0x62D9 T-phase voltage cal K */
    .t_phase_volt_calibration_b_parm = 4096,           /* 0x62DA T-phase voltage cal B */
    .r_phase_cur_calibration_k_parm = 4096,              /* 0x62DB R-phase current cal K */
    .r_phase_cur_calibration_b_parm = 4096,              /* 0x62DC R-phase current cal B */
    .s_phase_cur_calibration_k_parm = 4096,              /* 0x62DD S-phase current cal K */
    .s_phase_cur_calibration_b_parm = 4096,              /* 0x62DE S-phase current cal B */
    .t_phase_cur_calibration_k_parm = 4096,              /* 0x62DF T-phase current cal K */
    .t_phase_cur_calibration_b_parm = 4096,              /* 0x62F0 T-phase current cal B */

    .Inv_Battery_Volt_Calibration_CoefK = 4096,        /* 0x62F1 Battery voltage cal K */
    .Inv_Battery_Volt_Calibration_CoefB = 4096,        /* 0x62F2 Battery voltage cal B */
    .Inv_Battery_Cur_Calibration_CoefK = 4096,         /* 0x62F3 Battery current cal K */
    .Inv_Battery_Cur_Calibration_CoefB = 4096,         /* 0x62F4 Battery current cal B */
    .Inv_EV_Volt_Calibration_CoefK = 4096,             /* 0x62F5 EV voltage cal K */
    .Inv_EV_Volt_Calibration_CoefB = 4096,             /* 0x62F6 EV voltage cal B */
    .Inv_EV_Cur_Calibration_CoefK = 4096,              /* 0x62F7 EV current cal K */
    .Inv_EV_Cur_Calibration_CoefB = 4096,              /* 0x62F8 EV current cal B */

    .pv_ch1_volt_k_calibrate = 4096,                    /* 0x62F9 PV1 voltage cal K */
    .pv_ch1_volt_b_calibrate = 4096,                    /* 0x62FA PV1 voltage cal B */
    .pv_ch1_curr_k_calibrate = 4096,                    /* 0x62FB PV1 current cal K */
    .pv_ch1_curr_b_calibrate = 4096,                    /* 0x62FC PV1 current cal B */

    .pv_ch2_volt_k_calibrate = 4096,                    /* 0x62FD PV2 voltage cal K */
    .pv_ch2_volt_b_calibrate = 4096,                    /* 0x62FE PV2 voltage cal B */
    .pv_ch2_curr_k_calibrate = 4096,                    /* 0x62FF PV2 current cal K */
    .pv_ch2_curr_b_calibrate = 4096,                    /* 0x6300 PV2 current cal B */

    .pv_ch3_volt_k_calibrate = 4096,                    /* 0x6301 PV3 voltage cal K */
    .pv_ch3_volt_b_calibrate = 4096,                    /* 0x6302 PV3 voltage cal B */
    .pv_ch3_curr_k_calibrate = 4096,                    /* 0x6303 PV3 current cal K */
    .pv_ch3_curr_b_calibrate = 4096,                    /* 0x6304 PV3 current cal B */

    .pv_ch4_volt_k_calibrate = 4096,                    /* 0x6305 PV4 voltage cal K */
    .pv_ch4_volt_b_calibrate = 4096,                    /* 0x6306 PV4 voltage cal B */
    .pv_ch4_curr_k_calibrate = 4096,                    /* 0x6307 PV4 current cal K */
    .pv_ch4_curr_b_calibrate = 4096,                    /* 0x6308 PV4 current cal B */

    .reserve9 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};


/* Pack struct as little-endian byte stream (C28x: sizeof is word count) */
static void struct_to_bytes(UdpDspHoldPara_t *src, uint8_t *dest)
{
    uint16_t *p = (uint16_t *)src;
    size_t i;
    size_t n = sizeof(UdpDspHoldPara_t);

    for (i = 0; i < n; i++)
    {
        dest[2 * i]     = (uint8_t)(p[i] & 0x00FF);
        dest[2 * i + 1] = (uint8_t)((p[i] >> 8) & 0x00FF);
    }
}

/* Unpack little-endian byte stream into struct */
static void bytes_to_struct(uint8_t *src, UdpDspHoldPara_t *dest)
{
    uint16_t *p = (uint16_t *)dest;
    size_t i;
    size_t n = sizeof(UdpDspHoldPara_t);

    for (i = 0; i < n; i++)
    {
        p[i] = (uint16_t)((src[2 * i] & 0x00FF) | ((src[2 * i + 1] & 0x00FF) << 8));
    }
}

static void flash_frame_read(uint16_t *hdr)
{
    SpiFlashReadBytes(FLASH_DspHoldFRAME_START_ADDR, hdr, FLASH_FRAME_WORDS);
}

static bool flash_frame_is_valid(const uint16_t *hdr)
{
    return (hdr[0] == Flash_DspHoldType[0]
         && hdr[1] == Flash_DspHoldType[1]
         && hdr[2] == Flash_DspHoldType[2]
         && hdr[3] == Flash_DspHoldType[3]);
}

static bool flash_frame_is_invalid(const uint16_t *hdr)
{
    return (hdr[0] == Flash_DspHoldType_init[0]
         && hdr[1] == Flash_DspHoldType_init[1]
         && hdr[2] == Flash_DspHoldType_init[2]
         && hdr[3] == Flash_DspHoldType_init[3]);
}

static void hold_snapshot_sync_from(const UdpDspHoldPara_t *src)
{
    memcpy(&s_hold_saved_mirror, src, sizeof(s_hold_saved_mirror));
}

/* Clear transient / command fields before auto-save equality check */
#define HOLD_MIRROR_TRANSIENT_WORDS  10u

static void hold_mask_excluded_for_compare(UdpDspHoldPara_t *p)
{
    p->sys_shut_down = 0;
    p->LLCEnable = 0;
    p->remote_shutdown_status = 0;
    p->charging_input_type = 0;
    p->set_batt_chg_power[0] = 0;
    p->set_batt_chg_power[1] = 0;
    p->set_batt_dsg_power[0] = 0;
    p->set_batt_dsg_power[1] = 0;
    p->reset_factory = 0;
    p->sys_upgrade_status = 0;
}

static void hold_mirror_transients_save(uint16_t bak[HOLD_MIRROR_TRANSIENT_WORDS],
                                        const UdpDspHoldPara_t *m)
{
    bak[0] = m->sys_shut_down;
    bak[1] = m->LLCEnable;
    bak[2] = m->remote_shutdown_status;
    bak[3] = m->charging_input_type;
    bak[4] = m->set_batt_chg_power[0];
    bak[5] = m->set_batt_chg_power[1];
    bak[6] = m->set_batt_dsg_power[0];
    bak[7] = m->set_batt_dsg_power[1];
    bak[8] = m->reset_factory;
    bak[9] = m->sys_upgrade_status;
}

static void hold_mirror_transients_restore(UdpDspHoldPara_t *m,
                                           const uint16_t bak[HOLD_MIRROR_TRANSIENT_WORDS])
{
    m->sys_shut_down = bak[0];
    m->LLCEnable = bak[1];
    m->remote_shutdown_status = bak[2];
    m->charging_input_type = bak[3];
    m->set_batt_chg_power[0] = bak[4];
    m->set_batt_chg_power[1] = bak[5];
    m->set_batt_dsg_power[0] = bak[6];
    m->set_batt_dsg_power[1] = bak[7];
    m->reset_factory = bak[8];
    m->sys_upgrade_status = bak[9];
}

/* Compare live RAM vs mirror using one scratch (mirror masked in place, then restored) */
static bool hold_live_equals_saved_mirror(void)
{
    uint16_t bak[HOLD_MIRROR_TRANSIENT_WORDS];
    bool same;

    hold_mirror_transients_save(bak, &s_hold_saved_mirror);
    hold_mask_excluded_for_compare(&s_hold_saved_mirror);

    memcpy(&s_hold_work, &gstUdpPcsPara.udp_pcs_hold, sizeof(s_hold_work));
    hold_mask_excluded_for_compare(&s_hold_work);

    same = (memcmp(&s_hold_work, &s_hold_saved_mirror, sizeof(s_hold_work)) == 0);

    hold_mirror_transients_restore(&s_hold_saved_mirror, bak);
    return same;
}

void UdpDspHoldPara_AutoSaveIfNeeded(uint16_t *err_cnt)
{
    if (hold_live_equals_saved_mirror())
    {
        return;
    }

    if (!Save_System_Parameter(&gstUdpPcsPara.udp_pcs_hold) && (err_cnt != NULL))
    {
        (*err_cnt)++;
    }
}


void Init_System_Parameter(void)
{
    uint16_t flash_hdr[FLASH_FRAME_WORDS];

    flash_frame_read(flash_hdr);
    if (!flash_frame_is_valid(flash_hdr))
    {
        memcpy((uint16_t *)&gstUdpPcsPara.udp_pcs_hold, (uint16_t *)&UdpPCS_DspHoldPara_Init,
               sizeof(UdpDspHoldPara_t));
        /* Mirror updated inside Save_System_Parameter only on success */
        (void)Save_System_Parameter(&gstUdpPcsPara.udp_pcs_hold);
    }
    else
    {
        SpiFlashReadBytes(FLASH_DspHoldPara_START_ADDR, (uint16_t *)s_flash_byte_buf,
                          sizeof(UdpDspHoldPara_t));
        bytes_to_struct(s_flash_byte_buf, &s_hold_work);
        memcpy((uint16_t *)&gstUdpPcsPara.udp_pcs_hold, (uint16_t *)&s_hold_work,
               sizeof(UdpDspHoldPara_t));
        hold_snapshot_sync_from(&gstUdpPcsPara.udp_pcs_hold);
    }
}


bool Save_System_Parameter(UdpDspHoldPara_t *para)
{
    uint16_t flash_hdr[FLASH_FRAME_WORDS];

    if (para == NULL)
    {
        return false;
    }

    /* Two-phase commit: invalidate header -> write data -> validate header */
    SpiFlashWriteBytesWithErase(FLASH_DspHoldFRAME_START_ADDR, (uint16_t *)&Flash_DspHoldType_init,
                                FLASH_FRAME_WORDS);
    struct_to_bytes(para, s_flash_byte_buf);
    SpiFlashWriteBytesWithErase(FLASH_DspHoldPara_START_ADDR, (uint16_t *)s_flash_byte_buf,
                                sizeof(UdpDspHoldPara_t));
    SpiFlashWriteBytesWithErase(FLASH_DspHoldFRAME_START_ADDR, (uint16_t *)&Flash_DspHoldType,
                                FLASH_FRAME_WORDS);

    flash_frame_read(flash_hdr);
    if (!flash_frame_is_valid(flash_hdr))
    {
        return false;
    }

    SpiFlashReadBytes(FLASH_DspHoldPara_START_ADDR, (uint16_t *)s_flash_byte_buf, sizeof(UdpDspHoldPara_t));
    bytes_to_struct(s_flash_byte_buf, &s_hold_work);
    if (memcmp((uint16_t *)para, (uint16_t *)&s_hold_work, sizeof(UdpDspHoldPara_t)) != 0)
    {
        return false;
    }

    hold_snapshot_sync_from(para);
    return true;
}


bool ShutDown_SaveFlash(uint16_t *err_cnt)
{
    memcpy(&s_hold_work, &gstUdpPcsPara.udp_pcs_hold, sizeof(s_hold_work));
    hold_mask_excluded_for_compare(&s_hold_work);
    if (!Save_System_Parameter(&s_hold_work))
    {
        if (err_cnt != NULL)
        {
            (*err_cnt)++;
        }
        return false;
    }
    return true;
}


bool Restore_Factory_Settings(uint16_t *reset_cnt,bool actual_reset_factory)
{

    uint16_t flash_hdr[FLASH_FRAME_WORDS];

    if (actual_reset_factory)
    {
        if (!Save_System_Parameter(&UdpPCS_DspHoldPara_Init))
        {
            if (reset_cnt != NULL)
            {
                (*reset_cnt)++;
            }
            return false;
        }
        memcpy((uint16_t *)&gstUdpPcsPara.udp_pcs_hold, (uint16_t *)&UdpPCS_DspHoldPara_Init,
               sizeof(UdpDspHoldPara_t));
    }
    else
    {
        flash_frame_read(flash_hdr);
        if (flash_frame_is_valid(flash_hdr))
        {
            SpiFlashWriteBytesWithErase(FLASH_DspHoldFRAME_START_ADDR,
                                    (uint16_t *)&Flash_DspHoldType_init, FLASH_FRAME_WORDS);

            if (reset_cnt != NULL)
            {
                (*reset_cnt)++;
            }
            return false;
        }
    }
    return true;
}
```

### `app/flash_default.h`

```c
/**
 * @file    flash_default.h
 * @brief   Factory defaults and SPI Flash persistence for DSP hold registers (0x6220~0x631F)
 * @date    2026-01-30
 * @author  Nova007457
 */

#ifndef FLASH_DEFAULT_H
#define FLASH_DEFAULT_H

#include "F28x_Project.h"
#include <stdbool.h>

/** Load hold parameters from Flash at boot; if invalid, write factory defaults and valid frame header */
void Init_System_Parameter(void);

/**
 * Save hold parameters to Flash after clearing transient command bits.
 * @param err_cnt  Incremented on save failure (may be NULL)
 * @return true if saved and verified
 */
bool ShutDown_SaveFlash(uint16_t *err_cnt);

/**
 * Factory reset: invalidate frame header, or write factory defaults when actual_reset_factory is true.
 * @param reset_cnt  Incremented on failure (may be NULL)
 * @param actual_reset_factory  true: write UdpPCS_DspHoldPara_Init to Flash
 * @return true on success
 */
bool Restore_Factory_Settings(uint16_t *reset_cnt, bool actual_reset_factory);

/**
 * Auto-save hold parameters to Flash when changed (transient fields excluded from compare).
 * @param err_cnt  Incremented on save failure (may be NULL)
 */
void UdpDspHoldPara_AutoSaveIfNeeded(uint16_t *err_cnt);

#endif /* FLASH_DEFAULT_H */
```

### `app/post.c`

```c
/*
 * post.c
 *
 *  Created on: 2025-11-10
 *      Author: Nova006348
 */
#include "post.h"
#include <fan.h>
#include <ntc.h>
#include "rcmu.h"
#include "relay_ctrl.h"
#include "cpu_timers.h"
#include "fault.h"
#include "cla_cpu_share.h"
#include "ph3_calc_ifc_update.h"
#include "algo_interface.h"
#include "debug_analysis.h"
#include <sample_update.h>
SysSelfTest_t _gSysSelfTest;

/**
* @brief: SysSelfTestInit
* @param: Nothing
* @return:Nothing
* @note:System self-test initialization
*/
void SysSelfTestInit(void)
{
    _gSysSelfTest.complete_flag = false;
    _gSysSelfTest.retest_request_flag = false;
    _gSysSelfTest.max_test_time = SYS_SELF_TEST_TIMEOUT;
    _gSysSelfTest.module_state.byte = 0;
    _gSysSelfTest.timeout_flag = false;
}
/**
* @brief: _gFanSelfTest
* @param: Nothing
* @return:Nothing
* @note:Fan self-test
*/
static void _gFanSelfTest(void)
{
    if(!GetFanFaultState())
    {
        _gSysSelfTest.module_state.bit.fan_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.fan_test_complete = 0;
    }
}
/**
* @brief: _gNtcSelfTest
* @param: Nothing
* @return:Nothing
* @note:NTC self-test
*/
static void _gNtcSelfTest(void)
{
    if(!GetNTCFault())
    {
        _gSysSelfTest.module_state.bit.ntc_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.ntc_test_complete = 0;
    }
}
/**
* @brief: _gAdcSelfTest
* @param: Nothing
* @return:Nothing
* @note:ADC self-test
*/
static void _gAdcSelfTest(void)
{
    if(!GetError_Adc_Calibration_Fault())
    {
        _gSysSelfTest.module_state.bit.adc_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.adc_test_complete = 0;
    }
}
/**
* @brief: _gLeakCurrentSelfTest
* @param: Nothing
* @return:Nothing
* @note:Leakage current self-test
*/
static void _gLeakCurrentSelfTest(void)
{
    if(GetLeakCurrSelfTestState() == 1)
    {
        _gSysSelfTest.module_state.bit.Leak_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.Leak_test_complete = 0;
    }
}
/**
* @brief: _gRelaySelfTestProcess
* @param: Nothing
* @return:Nothing
* @note:Relay self-test
*/
static void _gRelaySelfTestProcess(void)
{
    RelaySelfTestInValue_t* relay_in_value = GetRelaySelfInputValue();
    relay_in_value -> grid_a_vol_instant = gstAdcResult.grid_volt_phase_a;
    relay_in_value -> grid_b_vol_instant = gstAdcResult.grid_volt_phase_b;
    relay_in_value -> grid_c_vol_instant = gstAdcResult.grid_volt_phase_c;

    relay_in_value -> mid_a_vol_instant = gstAdcResult.relay_middle_volt_phase_a;
    relay_in_value -> mid_b_vol_instant = gstAdcResult.relay_middle_volt_phase_b;
    relay_in_value -> mid_c_vol_instant = gstAdcResult.relay_middle_volt_phase_c;

    relay_in_value -> grid_a_vol_rms = gstPh3CalcInfo.grid_vol.a_phase_rms;
    relay_in_value -> grid_b_vol_rms = gstPh3CalcInfo.grid_vol.b_phase_rms;
    relay_in_value -> grid_c_vol_rms = gstPh3CalcInfo.grid_vol.c_phase_rms;

    relay_in_value -> mid_a_vol_rms = gstPh3CalcInfo.mid_vol.a_phase_rms;
    relay_in_value -> mid_b_vol_rms = gstPh3CalcInfo.mid_vol.b_phase_rms;
    relay_in_value -> mid_c_vol_rms = gstPh3CalcInfo.mid_vol.c_phase_rms;

    relay_in_value ->Inv_a_vol_rms = gstPh3CalcInfo.inv_vol.a_phase_rms;
    relay_in_value ->Inv_b_vol_rms = gstPh3CalcInfo.inv_vol.b_phase_rms;
    relay_in_value ->Inv_c_vol_rms = gstPh3CalcInfo.inv_vol.c_phase_rms;

    relay_in_value ->Gen_a_vol_rms = gstPh3CalcInfo.gen_vol.a_phase_rms;
    relay_in_value ->Gen_b_vol_rms = gstPh3CalcInfo.gen_vol.b_phase_rms;
    relay_in_value ->Gen_c_vol_rms = gstPh3CalcInfo.gen_vol.c_phase_rms;

    relay_in_value ->Load_a_vol_rms = gstPh3CalcInfo.load_vol.a_phase_rms;
    relay_in_value ->Load_b_vol_rms = gstPh3CalcInfo.load_vol.b_phase_rms;
    relay_in_value ->Load_c_vol_rms = gstPh3CalcInfo.load_vol.c_phase_rms;
    RelaySelfTest();

    if(GetRelaySelfTestResult() == 1)
    {
        _gSysSelfTest.module_state.bit.relay_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.relay_test_complete = 0;
    }
}
/**
* @brief: _gISOSelfTest
* @param: Nothing
* @return:Nothing
* @note:ISO self-test
*/
static void _gISOSelfTest(void)
{
    if(!GetError_IsoAndArcFault())
    {
        _gSysSelfTest.module_state.bit.iso_test_complete = 1;
    }
    else
    {
        _gSysSelfTest.module_state.bit.iso_test_complete = 0;
    }
}

/**
* @brief: SysSelfTest
* @param: void
* @return:Nothing
* @note:System self-test; PV/inverter run only after flag is set
*/
/************************************/
// * TODO: gen relay/start-mode detection on hardware start
//If no gen start, enable PV start
//Verify hardware BOOT detection
 /************************************/
void SysSelfTest(void)
{

    static uint32_t timeout;
    static bool first_run_flag = true;
    if(first_run_flag)
    {
        first_run_flag = false;
        timeout = TimGetSystime();
    }
//        SetCpuDebugData(10,_gSysSelfTest.module_state.bit.adc_test_complete);//26
//        SetCpuDebugData(11,_gSysSelfTest.module_state.bit.fan_test_complete);
//        SetCpuDebugData(12,_gSysSelfTest.module_state.bit.ntc_test_complete);
//        SetCpuDebugData(13,_gSysSelfTest.module_state.bit.Leak_test_complete);
//        SetCpuDebugData(14,_gSysSelfTest.module_state.bit.relay_test_complete);
//        SetCpuDebugData(15,_gSysSelfTest.module_state.bit.iso_test_complete);//31
    if(_gSysSelfTest.retest_request_flag)       /*Re-run self-test*/
    {
        _gSysSelfTest.retest_request_flag = false;
        _gSysSelfTest.timeout_flag = false;
        timeout = TimGetSystime();
        first_run_flag = true;
    }
    if(_gSysSelfTest.complete_flag || _gSysSelfTest.timeout_flag)     /*Exit on self-test success*/
    {
        return;
    }
    /*Self-test code*/
    _gAdcSelfTest();
    _gFanSelfTest();
    if(_gSysSelfTest.module_state.bit.adc_test_complete)
    {
        _gNtcSelfTest();
        _gLeakCurrentSelfTest();
        _gRelaySelfTestProcess();
        _gISOSelfTest();
    }
    if(_gSysSelfTest.module_state.byte == 63 && TimDifftimeFromNow(timeout)  > SYS_TEST_TIME)
    {
        _gSysSelfTest.complete_flag = true;
        RelayControl(EN_RELAT_CTRL_TYPE_INV,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_GRID,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_GEN,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_LOAD,RELAY_OFF);
    }

    if(TimDifftimeFromNow(timeout)  > SYS_SELF_TEST_TIMEOUT)    /*Timeout handling*/
    {
        _gSysSelfTest.timeout_flag = true;
        _gSysSelfTest.complete_flag =false;
        RelayControl(EN_RELAT_CTRL_TYPE_INV,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_GRID,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_GEN,RELAY_OFF);
        RelayControl(EN_RELAT_CTRL_TYPE_LOAD,RELAY_OFF);
    }

}

/**
* @brief: SysRetestRequestSet
* @param: value
* @return:Nothing
* @note:Set system self-test request
*/
void SysRetestRequestSet(bool value)
{
    _gSysSelfTest.retest_request_flag = value;
}
/**
* @brief: GetSysTimeOutFlag
* @param: Nothing
* @return:_gSelfTestSysPermit
* @note:Get system timeout flag
*/
bool GetSysTimeOutFlag(void)
{
    return _gSysSelfTest.timeout_flag;
}
/**
* @brief: GetSysSelfTestFlag
* @param: Nothing
* @return:_gSelfTestSysPermit
* @note:Get system self-test result
*/
bool GetSysSelfTestFlag(void)
{
    return _gSysSelfTest.complete_flag;
}
```

### `app/post.h`

```c
/*
 * post.h
 *
 *  Created on: 2025��11��10��
 *      Author: Nova006348
 */
//POWER ON SELF_TEST
#ifndef APP_POST_H_
#define APP_POST_H_

#include "F28x_Project.h"
#include "system.h"
#include "cpu_timers.h"
#ifdef __cplusplus
extern "C" {
#endif

#define SYS_SELF_TEST_TIMEOUT     25*1000      /*��ʱʱ��*/
#define SYS_TEST_TIME              15*1000      /*�Լ�ʱ��*/

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t fan_test_complete         : 1;
        uint16_t ntc_test_complete         : 1;
        uint16_t adc_test_complete         : 1;
        uint16_t Leak_test_complete        : 1;
        uint16_t relay_test_complete       : 1;
        uint16_t iso_test_complete         : 1;
        uint16_t reserve                   : 10;
    } bit;
} ModuleTestCompleteState_t;

typedef struct _Self_Test_stru_t
{
    ModuleTestCompleteState_t        module_state;         //DSP���ϱ����ڲ��澯
    bool                             complete_flag;        //�ɹ���־
    uint32_t                         max_test_time;        //��ʱʱ��
    bool                             timeout_flag;
    bool                             retest_request_flag;  //�����Լ�Flag
} SysSelfTest_t;

void SysSelfTestInit(void);
void SysSelfTest(void);
void SysRetestRequestSet(bool value);
bool GetSysTimeOutFlag(void);
bool GetSysSelfTestFlag(void);
#endif /* APP_POST_H_ */
```

### `app/electric_calculate.c`

```c
/*
 * electric_calculate.c
 *
 *  Created on: 2025-11-19
 *      Author: Nova006348
 */

#include "electric_calculate.h"
#include "string.h"
#include "cpu_timers.h"
#include "cla_cpu_share.h"
#include "ph3_calc_ifc_update.h"
#include "algo_interface.h"
#include "ct.h"

static SysElectricMeter_t _gSysElectricMeter;
static void _gSysElectricMeterUpdate(void);

void ElectricParamClear(uint16_t *flag)
{
    if (flag != NULL && (*flag != 0U))
    {
        _gSysElectricMeter.bat_chg_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.bat_dsg_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.buy_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.eps_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.ev_chg_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.ev_dsg_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.gen_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.pv_energr_meter[0].total_01kwh = 0;
        _gSysElectricMeter.pv_energr_meter[1].total_01kwh = 0;
        _gSysElectricMeter.pv_energr_meter[2].total_01kwh = 0;
        _gSysElectricMeter.remote_load_electric_meter.total_01kwh = 0;
        _gSysElectricMeter.sell_electric_meter.total_01kwh = 0;
        *flag = 0U;
    }
}

/**
* @brief: ElectricParamInit()
* @param: nothing
* @return:nothing
* @note: Energy meter data initialization
*/
static void _gElectricParamInit(void)
{
    memset(&_gSysElectricMeter, 0, sizeof(SysElectricMeter_t));
}
/**
* @brief: _gEnergyAccumulate()
* @param: nothing
* @return:nothing
* @note: Accumulate power
*/
bool _gEnergyAccumulate(ElectricCalculate_t *acc, float power)
{
    if (acc == NULL || power <= 0)
    {
        return false;
    }
    // Accumulate power (0.1s sample to watt-seconds)
    acc->power_accumulate_ws += power * 0.1f;
    acc->sample_count++;

    // Check 1-minute sample and auto-calc enable
    if (acc->sample_count >= SAMPLES_PER_MINUTE)
    {
        acc ->calc_active_flag = true;
        acc ->power_minute_ws = acc->power_accumulate_ws;
        acc->sample_count = 0;
        acc->power_accumulate_ws = 0;
    }
    return true;
}

/**
* @brief: _gEnergyCalculate()
* @param: nothing
* @return:nothing
* @note: Calculate energy (read variables within one minute)
*/
void _gEnergyCalculate(ElectricCalculate_t *acc)
{
    if (acc == NULL || (acc ->calc_active_flag == false))
    {
        return ;
    }
    // Convert accumulation to Wh (divide by 3600s)
    float minute_wh = acc ->power_minute_ws / 3600.0f;
    // Total Wh = 1-min sum + remainder
    float total_wh = minute_wh + acc->remainder_wh;
    // Compute 0.1 kWh integer and remainder
    uint32_t integer_part = (uint32_t)(total_wh / 100.0f);     /*Convert to 0.1 kWh*/
    float new_remainder = total_wh - (integer_part * 100.0f);  /*Remainder from last conversion*/

    // Accumulate total 0.1 kWh
    acc->total_01kwh += integer_part;
    acc->remainder_wh = new_remainder;

    // Reset accumulator
    acc->sample_count = 0;
    acc ->calc_active_flag = false;
}

/**
* @brief: ElectricPowerAccumulate()
* @param: nothing
* @return:nothing
* @note: Energy accumulation should run at accurate timing
*/
void ElectricPowerAccumulate(void)
{
    static uint16_t i = 0;
    static uint32_t time_out = 0;
    static bool first_run_flag = true;
    if(first_run_flag)
    {
        first_run_flag = false;
        _gElectricParamInit();
        time_out = TimGetSystime();
    }
    if(TimDifftimeFromNow(time_out)  > 100)          /*Accumulate energy every 100 ms*/
    {
        _gEnergyAccumulate(&_gSysElectricMeter.bat_chg_electric_meter,(NON_NEGATIVE(-1 * gstPh3CalcInfo.bus.power_filter)));
        _gEnergyAccumulate(&_gSysElectricMeter.bat_dsg_electric_meter,(NON_NEGATIVE(gstPh3CalcInfo.bus.power_filter)));

        _gEnergyAccumulate(&_gSysElectricMeter.buy_electric_meter,(NON_NEGATIVE(-1 * gstPh3CalcInfo.grid_power.total_active)));
        _gEnergyAccumulate(&_gSysElectricMeter.sell_electric_meter,(NON_NEGATIVE(gstPh3CalcInfo.grid_power.total_active)));

        _gEnergyAccumulate(&_gSysElectricMeter.eps_electric_meter,(NON_NEGATIVE(-1 * gstPh3CalcInfo.load_power.total_active)));

        _gEnergyAccumulate(&_gSysElectricMeter.ev_chg_electric_meter,(NON_NEGATIVE(-1 * 0 )));
        _gEnergyAccumulate(&_gSysElectricMeter.ev_dsg_electric_meter,(NON_NEGATIVE( 0 )));

        _gEnergyAccumulate(&_gSysElectricMeter.gen_electric_meter,(NON_NEGATIVE( gstPh3CalcInfo.gen_power.total_active)));
        _gEnergyAccumulate(&_gSysElectricMeter.remote_load_electric_meter,(NON_NEGATIVE(gCtDataInfo.ct_power.total_active - gstPh3CalcInfo.grid_power.total_active)));

        _gEnergyAccumulate(&_gSysElectricMeter.pv_energr_meter[0],PvPwrAveRead(0));
        _gEnergyAccumulate(&_gSysElectricMeter.pv_energr_meter[1],PvPwrAveRead(1));
        _gEnergyAccumulate(&_gSysElectricMeter.pv_energr_meter[2],PvPwrAveRead(2));
        time_out = TimGetSystime();
    }
    i++;
    if(i >= 10 * 30)
    {
        _gSysElectricMeterUpdate();
        i = 0;
    }
}
/**
* @brief: SysElectricMeter()
* @param: nothing
* @return:nothing
* @note: Read energy within one minute
*/
static void _gSysElectricMeterUpdate(void)
{
    _gEnergyCalculate(&_gSysElectricMeter.bat_chg_electric_meter);
    _gEnergyCalculate(&_gSysElectricMeter.bat_dsg_electric_meter);

    _gEnergyCalculate(&_gSysElectricMeter.buy_electric_meter);
    _gEnergyCalculate(&_gSysElectricMeter.sell_electric_meter);

    _gEnergyCalculate(&_gSysElectricMeter.eps_electric_meter);

    _gEnergyCalculate(&_gSysElectricMeter.ev_chg_electric_meter);
    _gEnergyCalculate(&_gSysElectricMeter.ev_dsg_electric_meter);

    _gEnergyCalculate(&_gSysElectricMeter.gen_electric_meter);
    _gEnergyCalculate(&_gSysElectricMeter.remote_load_electric_meter);

    _gEnergyCalculate(&_gSysElectricMeter.pv_energr_meter[0]);
    _gEnergyCalculate(&_gSysElectricMeter.pv_energr_meter[1]);
    _gEnergyCalculate(&_gSysElectricMeter.pv_energr_meter[2]);
}
/**
* @brief: GetPvTotalElectric()
* @param: nothing
* @return:nothing
* @note: Get total PV energy
*/
uint16_t GetPvTotalElectric(void)
{
    return (_gSysElectricMeter.pv_energr_meter[0].total_01kwh + _gSysElectricMeter.pv_energr_meter[1].total_01kwh +
                                                                _gSysElectricMeter.pv_energr_meter[2].total_01kwh);
}
/**
* @brief: GetPv1Electric()
* @param: nothing
* @return:nothing
* @note: Get PV1 energy
*/
uint16_t GetPv1Electric(void)
{
    return (_gSysElectricMeter.pv_energr_meter[0].total_01kwh);
}
/**
* @brief: GetPv2Electric()
* @param: nothing
* @return:nothing
* @note: Get PV2 energy
*/
uint16_t GetPv2Electric(void)
{
    return (_gSysElectricMeter.pv_energr_meter[1].total_01kwh);
}
/**
* @brief: GetPv3Electric()
* @param: nothing
* @return:nothing
* @note: Get PV3 energy
*/
uint16_t GetPv3Electric(void)
{
    return (_gSysElectricMeter.pv_energr_meter[2].total_01kwh);
}
/**
* @brief: GetBuyElectric()
* @param: nothing
* @return:nothing
* @note: Get import energy
*/
uint16_t GetBuyElectric(void)
{
    return (_gSysElectricMeter.buy_electric_meter.total_01kwh);
}
/**
* @brief: GetSellElectric()
* @param: nothing
* @return:nothing
* @note: Get export energy
*/
uint16_t GetSellElectric(void)
{
    return (_gSysElectricMeter.sell_electric_meter.total_01kwh);
}
/**
* @brief: GetBatChgElectric()
* @param: nothing
* @return:nothing
* @note: Get battery charge energy
*/
uint16_t GetBatChgElectric(void)
{
    return (_gSysElectricMeter.bat_chg_electric_meter.total_01kwh);
}
/**
* @brief: GetBatDsgElectric()
* @param: nothing
* @return:nothing
* @note: Get battery discharge energy
*/
uint16_t GetBatDsgElectric(void)
{
    return (_gSysElectricMeter.bat_dsg_electric_meter.total_01kwh);
}
/**
* @brief: GetEpsElectric()
* @param: nothing
* @return:nothing
* @note: Get local load (EPS) energy
*/
uint16_t GetEpsElectric(void)
{
    return (_gSysElectricMeter.eps_electric_meter.total_01kwh);
}
/**
* @brief: GetGenElectric()
* @param: nothing
* @return:nothing
* @note: Get generator energy
*/
uint16_t GetGenElectric(void)
{
    return (_gSysElectricMeter.gen_electric_meter.total_01kwh);
}
/**
* @brief: GetRemoteLoadElectric()
* @param: nothing
* @return:nothing
* @note: Get remote load energy
*/
uint16_t GetRemoteLoadElectric(void)
{
    return (_gSysElectricMeter.remote_load_electric_meter.total_01kwh);
}
/**
* @brief: GetEvChgElectric()
* @param: nothing
* @return:nothing
* @note: Get EV charge energy
*/
uint16_t GetEvChgElectric(void)
{
    return (_gSysElectricMeter.ev_chg_electric_meter.total_01kwh);
}
/**
* @brief: GetEvDsgElectric()
* @param: nothing
* @return:nothing
* @note: Get EV discharge energy
*/
uint16_t GetEvDsgElectric(void)
{
    return (_gSysElectricMeter.ev_dsg_electric_meter.total_01kwh);
}
```

### `app/electric_calculate.h`

```c
/*
 * electric_calculate.h
 *
 *  Created on: Nov 19, 2025
 *      Author: Nova006348
 */

#ifndef APP_ELECTRIC_CALCULATE_H_
#define APP_ELECTRIC_CALCULATE_H_

#include "F28x_Project.h"

#define NON_NEGATIVE(x) (((x) > 0) ? (x) : 0)

/* Actual sample period is 100ms in electric_calculate.c (600 samples = 60s) */
#define SAMPLE_INTERVAL_MS   100 * 20
#define SAMPLES_PER_MINUTE   600

/* Per-channel energy accumulator */
typedef struct {
    float power_accumulate_ws;      /* Running sum (W??s) */
    float power_minute_ws;          /* Completed 60s window sum (W??s) */
    uint32_t total_01kwh;           /* Total energy in 0.1 kWh units */
    float remainder_wh;             /* Fractional remainder (Wh) */

    uint16_t sample_count;          /* Sample count in current window */
    bool calc_active_flag;          /* Set when a window is ready to convert */
} ElectricCalculate_t;


typedef struct {

    ElectricCalculate_t pv_energr_meter[3];
    ElectricCalculate_t buy_electric_meter;
    ElectricCalculate_t sell_electric_meter;
    ElectricCalculate_t bat_chg_electric_meter;
    ElectricCalculate_t bat_dsg_electric_meter;
    ElectricCalculate_t eps_electric_meter;
    ElectricCalculate_t remote_load_electric_meter;
    ElectricCalculate_t gen_electric_meter;
    ElectricCalculate_t ev_chg_electric_meter;
    ElectricCalculate_t ev_dsg_electric_meter;
}SysElectricMeter_t;

typedef struct {

    float pv1_power;
    float pv2_power;
    float pv3_power;
    float gird_active_power;
    float ct_active_power;
    float bat_power;
    float eps_active_power;
    float gen_active_power;
    float ev_power;
}ElectricMeterInValue_t;

void ElectricPowerAccumulate(void);
ElectricMeterInValue_t* GetElectricPowerInfo(void);

/* Energy readback (0.1 kWh units) */
uint16_t GetPvTotalElectric(void);
uint16_t GetPv1Electric(void);
uint16_t GetPv2Electric(void);
uint16_t GetPv3Electric(void);
uint16_t GetBuyElectric(void);
uint16_t GetSellElectric(void);
uint16_t GetBatChgElectric(void);
uint16_t GetBatDsgElectric(void);
uint16_t GetEpsElectric(void);
uint16_t GetGenElectric(void);
uint16_t GetRemoteLoadElectric(void);
uint16_t GetEvChgElectric(void);
uint16_t GetEvDsgElectric(void);
void ElectricParamClear(uint16_t *flag);
#endif /* APP_ELECTRIC_CALCULATE_H_ */
```

### `app/authentication_func.c`

```c
/*
 * authentication_fun.c
 *
 *  Created on: 2025��3��21��
 *      Author: Nova006352
 */



/*
 * authentication_function.c
 *
 *  Created on: 2024��9��30��
 *      Author: Nova006352
 */

#define QV_REGIONDEFAULT_VALUEV_NUM        (5)
#define PV_DSG_REGIONDEFAULT_VALUEV_NUM    (2)
#define PV_CHG_REGIONDEFAULT_VALUEV_NUM    (2)
#define PF_REGIONDEFAULT_VALUEF_NUM        (4)

#include "authentication_func.h"


//InverterPwr_t _gInvPwr;
//
//InvReactiveCoefficient_t gstInvReactiveCoefficient;
//ApprovalStdInfo_t gApprovalStdInfo;
//EN_NATIONAL_STANDARDS e_national_standards = EN_AUSTRALIA_A;
//
//EN_REATIVE_PWR_CTRL_MODE e_reative_pwr_mode = EN_POWER_FACTOR_MODE;
///*QVģʽ��ͬ���ұ�׼�ĵ�ѹ�����*/
//const uint16_t garrQvRegiondefaultValueV[EN_NATIONAL_STANDARDS_NUM][QV_REGIONDEFAULT_VALUEV_NUM] =
//{
//    {0, 207, 220, 240, 258},
//    {0, 205, 220, 235, 255},
//    {0, 215, 230, 240, 255},
//    {0, 207, 220, 235, 244},
//};
//
///*QVģʽ��ͬ���ұ�׼��kֵ*/
//const float garrQvCoefficientK[EN_NATIONAL_STANDARDS_NUM][QV_REGIONDEFAULT_VALUEV_NUM] =
//{
//    {0, -0.033846, 0, -0.033333, 0},//��3ԭΪ1
//    {0, -0.02,     0, -0.02,     0},
//    {0, -0.029333, 0, -0.04,     0},
//    {0, -0.046154, 0, -0.066667, 0},
//};
//
///*QVģʽ��ͬ���ұ�׼��bֵ*/
//const float garrQvCoefficientB[EN_NATIONAL_STANDARDS_NUM][QV_REGIONDEFAULT_VALUEV_NUM] =
//{
//    {0.44, 7.446124, 0, 8, -0.60},
//    {0.30, 4.4,      0, 4.7, -0.40},
//    {0.44, 6.746667, 0, 9.6, -0.60},
//    {0.60, 10.153846,0, 15.666667, -0.60},
//};
//
///*PVģʽ�²�ͬ���ұ�׼�¡��ŵ�����¡���ѹ�ж������*/
//const uint16_t garrPvDsgRegiondefaultValueV[EN_NATIONAL_STANDARDS_NUM][PV_DSG_REGIONDEFAULT_VALUEV_NUM] =
//{
//    {253, 260},
//    {250, 260},
//    {253, 260},
//    {242, 250},
//};
//
///*PVģʽ�²�ͬ���ұ�׼�¡��������¡���ѹ�ж������*/
//const uint16_t garrPvChgRegiondefaultValueV[EN_NATIONAL_STANDARDS_NUM][PV_CHG_REGIONDEFAULT_VALUEV_NUM] =
//{
//    {207, 215},
//    {195, 215},
//    {207, 215},
//    {216, 224},
//};
//
///*PFģʽ�²�ͬ���ұ�׼�¡�Ƶ���ж�����Σ�����ֵ*/
//float garrPfRegionValueF[EN_NATIONAL_STANDARDS_NUM][PF_REGIONDEFAULT_VALUEF_NUM] =
//{
//    {47, 49.75, 50.25, 52},
//    {47, 49.85, 50.15, 52},
//    {45, 49.5, 50.5, 55},
//    {45, 49.8, 50.2, 55},
//};
//
///*PFģʽ�²�ͬ���ұ�׼�¡�Ĭ��Ƶ���ж�����Σ�ѡ��Ƶ/��Ƶ���ߣ�*/
//const float garrPfRegionDefaultValueF[EN_NATIONAL_STANDARDS_NUM][PF_REGIONDEFAULT_VALUEF_NUM] =
//{
//    {47, 49.75, 50.25, 52},
//    {47, 49.85, 50.15, 52},
//    {45, 49.5, 50.5, 55},
//    {45, 49.8, 50.2, 55},
//};
//
///*PFģʽ�²�ͬ���ұ�׼�¡�Ƶ���ж�����Σ�ѡ��Ƶ/��Ƶ���ߣ�+�ͻ�*/
//const float garrPfRegionDelayValueF[EN_NATIONAL_STANDARDS_NUM][PF_REGIONDEFAULT_VALUEF_NUM] =
//{
//    {47, 49.85, 50.15, 52},
//    {47, 49.95, 50.05, 52},
//    {45, 49.55, 50.45, 55},
//    {45, 49.9, 50.1, 55},
//};
//
///*PFģʽ�����Ӧ�Ĳ���*/
//const float garrPfParaValue[EN_NATIONAL_STANDARDS_NUM][PF_REGIONDEFAULT_VALUEF_NUM] =
//{
//    {48, 49, 50.75, 52},
//    {48, 49, 50.75, 52},
//    {47, 48.25, 51.75, 53},
//    {48, 49, 51, 52},
//};
//
//static uint16_t VoltIntervalJudgment(EN_NATIONAL_STANDARDS e_national_standards,uint16_t dsg_chg_mode,float grid_volt);
//static float PvModePwrCtrlPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_volt,float inv_act_pwr,float apparent_pwr);
//static float QvModePwrCtrlPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_volt,float apparent_pwr);
//static float PfModeUnderFreqPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_freq,float inv_act_pwr,float apparent_pwr);
//
///**
// * @brief
// * @param 1.���ұ�׼;2.��ŵ�ģʽ;3.ʵ�ʲɼ���ѹ;
// * @param PV��ѹ�����жϺ���
// * @return ��ѹ�����
// */
//uint16_t VoltIntervalJudgment(EN_NATIONAL_STANDARDS e_national_standards,uint16_t dsg_chg_mode,float grid_volt)
//{
//    uint16_t i = 0;
//    if(dsg_chg_mode == 0)//���
//    {
//        for(i = 0;i < PV_CHG_REGIONDEFAULT_VALUEV_NUM; i++)
//        {
//            if(grid_volt < garrPvChgRegiondefaultValueV[e_national_standards][i])
//            {
//                break;
//            }
//        }
//    }
//    if(dsg_chg_mode == 1)//�ŵ�
//    {
//        for(i = 0;i < PV_DSG_REGIONDEFAULT_VALUEV_NUM; i++)
//        {
//            if(grid_volt < garrPvDsgRegiondefaultValueV[e_national_standards][i])
//            {
//                break;
//            }
//        }
//    }
//    return i ;//ȷ��i����i-1
//}
//
///**
// * @brief PV����ģʽ����
// * @param 1.���ұ�׼;2.ʵ�ʲɼ���ѹ;3.������빦��;4.���ڹ���;
// * @return ��������ŵ��й�����
// */
//float PvModePwrCtrlPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_volt,float inv_act_pwr,float apparent_pwr)
//{
//    float max_chg_pwr = apparent_pwr;
//    float max_dsg_pwr = -1 * apparent_pwr;
//    float coe = 0;
//    float max_active_pwr = 0;
//    uint16_t dsg_chg_mode = 0;
//    uint16_t volt_range = 0;
//
//    if(inv_act_pwr >= 0)
//    {
//        dsg_chg_mode = 0;//���
//    }
//    else if(inv_act_pwr < 0)
//    {
//        dsg_chg_mode = 1;//�ŵ�
//    }
//
//    volt_range = VoltIntervalJudgment(e_national_standards,dsg_chg_mode,grid_volt);//��ѹ�����ж�
//
//    if(dsg_chg_mode == 0)//���
//    {
//        switch(volt_range)
//        {
//            case EN_VOLT_SECTION_1 :
//            {
//                coe = 0.2;
//                break;
//            }
//            case EN_VOLT_SECTION_2 :
//            {
//                coe = (grid_volt - garrPvChgRegiondefaultValueV[e_national_standards][0]) * 0.8/
//                (garrPvChgRegiondefaultValueV[e_national_standards][1] - garrPvChgRegiondefaultValueV[e_national_standards][0]) + 0.2;
//                break;
//            }
//            case EN_VOLT_SECTION_3 :
//            {
//                coe = 1;
//                break;
//            }
//            default:
//                break;
//        }
//        max_active_pwr = max_chg_pwr * coe;
//    }
//    if(dsg_chg_mode == 1)//���
//    {
//        switch(volt_range)
//        {
//            case EN_VOLT_SECTION_1 :
//            {
//                coe = 1;
//                break;
//            }
//            case EN_VOLT_SECTION_2 :
//            {
//                coe = (garrPvDsgRegiondefaultValueV[e_national_standards][1] - grid_volt) * 0.8/
//                (garrPvDsgRegiondefaultValueV[e_national_standards][1] - garrPvDsgRegiondefaultValueV[e_national_standards][0]) + 0.2;
//                break;
//            }
//            case EN_VOLT_SECTION_3 :
//            {
//                coe = 0.2;
//                break;
//            }
//            default:
//                break;
//        }
//        max_active_pwr = max_dsg_pwr * coe;
//    }
//    return max_active_pwr;
//}
//
///**
// * @brief QV����ģʽ����
// * @param 1.���ұ�׼;2.ʵ�ʵ�����ѹ;3.������ڹ���;
// * @return �޹�����
// */
//float QvModePwrCtrlPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_volt,float apparent_pwr)//��������й����޹�
//{
//    uint16_t i = 0;
//    float coe = 0;
//    float reactive_pwr = 0;
//    for(i = 0; i < QV_REGIONDEFAULT_VALUEV_NUM; i++)
//    {
//        if(grid_volt < garrQvRegiondefaultValueV[e_national_standards][i])
//        {
//            break;
//        }
//    }
//    coe = garrQvCoefficientK[e_national_standards][i-1] * grid_volt + garrQvCoefficientB[e_national_standards][i-1];
//    reactive_pwr = apparent_pwr * coe;//���ڹ�����ô����
//    return reactive_pwr;
//}
//
//
///**
// * @brief PF����ģʽ����
// * @param 1.���ұ�׼;2.ʵ�ʵ�����ѹ;3.��������й�����;4.������ڹ���;
// * @return �й�����
// */
//float PfModeUnderFreqPrcess(EN_NATIONAL_STANDARDS e_national_standards,float grid_freq,float inv_act_pwr,float apparent_pwr)
//{
//    static float coe = 0; //PF����Ƶϵ��
//    static float latch_value = 0;//�Ŷ�ǰ��������ֵ
//    static float last_freq = 0;//������һ��Ƶ��ֵ
//    static float active_pwr = 0; //�����й�����
//    float max_dsg_pwr = -1 * apparent_pwr;//�������ŵ繦��
//    float max_chg_pwr = apparent_pwr;//��������繦��
//    uint16_t i = 0;
//    for(i = 0; i < PF_REGIONDEFAULT_VALUEF_NUM; i++)//Ƶ�������ж�
//    {
//         if(grid_freq <= garrPfRegionValueF[e_national_standards][i])
//         {
//             if(grid_freq <= garrPfRegionValueF[e_national_standards][1])
//             {
//                 garrPfRegionValueF[e_national_standards][1] = garrPfRegionDelayValueF[e_national_standards][1];//fLLCO + 0.1/0.1/0.05/0.1 �ͻ�Ƶ��
//             }
//             if(grid_freq > garrPfRegionValueF[e_national_standards][2])
//             {
//                 garrPfRegionValueF[e_national_standards][2] = garrPfRegionDelayValueF[e_national_standards][2];//fULCO + 0.1/0.1/0.05/0.1 �ͻ�Ƶ��
//             }
//             if(grid_freq > garrPfRegionValueF[e_national_standards][1])
//             {
//                 garrPfRegionValueF[e_national_standards][1] = garrPfRegionDefaultValueF[e_national_standards][1];
//             }
//             if(grid_freq <= garrPfRegionValueF[e_national_standards][2])
//             {
//                 garrPfRegionValueF[e_national_standards][2] = garrPfRegionDefaultValueF[e_national_standards][2];
//             }
//             break;
//         }
//    }
//    if((grid_freq > garrPfRegionValueF[e_national_standards][1]) && (grid_freq < garrPfRegionValueF[e_national_standards][2]))
//    {
//        latch_value = inv_act_pwr;
//    }
//    switch(i)
//    {
//        case EN_UNDER_FREQ_SECTION://ǷƵ����
//        {
//            coe = 0;
//            active_pwr = latch_value * coe;
//            break;
//        }
//        case EN_LOWER_FREQ_SECTION://��Ƶ����
//        {
//            if(grid_freq >= garrPfParaValue[e_national_standards][1])
//            {
//                if(inv_act_pwr <= 0)//���
//                {
//                    active_pwr = latch_value;
//                }
//                else//���
//                {
//                    if(grid_freq <= last_freq)//����Ƶ���䷢��Ƶ�ʻ��ˣ���Ƶ�����򱣳�ϵ�����䣻��������Ƶ�����¼���ϵ��
//                    {
//                        coe = (garrPfRegionDefaultValueF[e_national_standards][1] - grid_freq)/(garrPfRegionDefaultValueF[e_national_standards][1] - garrPfParaValue[e_national_standards][1]);
//                        last_freq = grid_freq;//��������ֵ
//                        active_pwr = latch_value * (1 - coe);
//                    }
//
//                }
//            }
//            else
//            {
//                if(grid_freq <= last_freq)//�ڽ�Ƶ���䷢��Ƶ�ʻ��ˣ���Ƶ�����򱣳�ϵ�����䣻��������Ƶ�����¼���ϵ��
//                {
//                    coe = (garrPfParaValue[e_national_standards][1] - grid_freq)/(garrPfParaValue[e_national_standards][1] - garrPfParaValue[e_national_standards][0]);
//                    last_freq = grid_freq;//��������ֵ
//                }
//                if(inv_act_pwr >= 0)//����������/���
//                {
//                    active_pwr = max_dsg_pwr * coe;
//                }
//                else//���
//                {
//                    active_pwr = latch_value + (max_dsg_pwr - latch_value) * coe;
//                }
//            }
//            break;
//        }
//        case EN_NORMAL_FREQ_SECTION ://��������
//        {
//            coe = 1;
//            last_freq = grid_freq;
//            active_pwr = latch_value * coe;
//            break;
//        }
//        case EN_UPPER_FREQ_SECTION ://��Ƶ����
//        {
//            if(grid_freq <= garrPfParaValue[e_national_standards][2])
//            {
//                if(inv_act_pwr >= 0)//���
//                {
//                    active_pwr = latch_value;
//                }
//                else//�ŵ�
//                {
//                    if(grid_freq >= last_freq)//����Ƶ���䷢��Ƶ�ʻ��ˣ���Ƶ�����򱣳�ϵ�����䣻��������Ƶ�����¼���ϵ��
//                    {
//                        coe = (grid_freq - garrPfRegionDefaultValueF[e_national_standards][2])/(garrPfParaValue[e_national_standards][2] - garrPfRegionDefaultValueF[e_national_standards][2]);
//                        last_freq = grid_freq;//��������ֵ
//                        active_pwr = latch_value * (1 - coe);
//                    }
//
//                }
//            }
//            else
//            {
//                if(grid_freq >= last_freq)//����Ƶ���䷢��Ƶ�ʻ��ˣ���Ƶ�����򱣳�ϵ�����䣻��������Ƶ�����¼���ϵ��
//                {
//                    coe = (grid_freq - garrPfParaValue[e_national_standards][2])/(garrPfParaValue[e_national_standards][3] - garrPfParaValue[e_national_standards][2]);
//                    last_freq = grid_freq;//��������ֵ
//                }
//                if(inv_act_pwr >= 0)//����������/���
//                {
//                    active_pwr = latch_value + (max_chg_pwr - latch_value) * coe;
//                }
//                else//���
//                {
//                    active_pwr = max_chg_pwr * coe;
//                }
//            }
//            break;
//        }
//        case EN_OVER_FREQ_SECTION ://��Ƶ����
//        {
//            coe = 0;
//            active_pwr = latch_value * coe;
//            break;
//        }
//        default:
//            break;
//    }
//    if(active_pwr < -5000)
//    {
//        active_pwr = -5000;
//    }
//    if(active_pwr > 5000)
//    {
//        active_pwr = 5000;
//    }
//    return active_pwr;
//
//}
//
//
///**
// * @brief ����ģʽѡ��
// * @param 1.���ұ�׼;2.��������й�����;3.�������й�����;4.�������޹�����;
// */
///**/
//void PowerCtrlModeHandle(EN_NATIONAL_STANDARDS e_national_standards,float inv_active_pwr,InverterPwr_t* InvPwr)
//{
//    float pf_active_pwr = 0;
//    float pv_max_active_pwr = 0;
//    float qv_reactive_pwr = 0;
//    float reactive_factor = 0;
//    float allow_active_pwr = 0;
//    float actual_active_pwr = 0;
//    float allow_reactive_pwr = 0;
//    float apparent_pwr = 5000;//���ڹ��ʼ��㷽ʽ�����书��
//    float grid_freq = _gState_algo.grid_freq;//Ƶ��
//    float grid_volt = _gState_algo.grid_vol_rms;//��ѹ
//
////    float grid_volt = (float)gSysSetInfo.uv_derating_enable;
//////    float grid_freq = (float)gSysSetInfo.uf_derating_enable;
////    float grid_freq = gSysSetInfo.freq_given;
//    uint16_t index = 0;
//
//    /*pf�й����ʼ���*/
//    // pf_active_pwr = PfModeUnderFreqPrcess(e_national_standards,grid_freq,inv_active_pwr,apparent_pwr);
//    pf_active_pwr = FrequencyResponse(grid_freq,inv_active_pwr,gSysSetInfo.pf_switch); //�滻Ϊŷ����֤��׼
//
//    /*pV����й����ʼ���*/
//    pv_max_active_pwr = PUCurveResponse(grid_volt);
//
//    ReactivepwrMode();//�޹�ģʽѡ��
//    /*�޹����ʼ���*/
//    switch(e_reative_pwr_mode)
//    {
//        case EN_POWER_FACTOR_MODE://����
//        {
//            float apparent_pwr_factor = 0;
//            reactive_factor = (float)gSysSetInfo.power_factor_setting_value / 100;//��������,���·�ֵ��Ҫ��С100��
//            apparent_pwr_factor = inv_active_pwr / reactive_factor;//������ڹ���,����������
//            if(fabs(apparent_pwr_factor) > 5000)
//            {
//                apparent_pwr_factor = 5000;//��������������
//            }
//            qv_reactive_pwr = sqrtf(apparent_pwr_factor * apparent_pwr_factor - inv_active_pwr * inv_active_pwr);//����޹�
//            qv_reactive_pwr = reactive_factor >= 0 ? qv_reactive_pwr : (-1 * qv_reactive_pwr);//���ݹ��������ж��޹���������
//            break;
//        }
//        case EN_REATIVE_PWR_MODE://�޹����ʿ���,ֱ�Ӹ����޹�����
//        {
//            qv_reactive_pwr = (float)gSysSetInfo.reactive_power_setting_value;
//            break;
//        }
//        case EN_QV_PWR_MODE://QVģʽ
//        {
////            qv_reactive_pwr = QvModePwrCtrlPrcess(e_national_standards,grid_volt,apparent_pwr);
//            qv_reactive_pwr = EUQvModePwrCtrlPrcess(grid_volt, inv_active_pwr);
//            break;
//        }
//        case EN_QP_PWR_MODE:
//        {
//            qv_reactive_pwr = QpCurveResponse(inv_active_pwr, grid_volt, 1);
//            //QPģʽ�ݲ�����
//            break;
//        }
//        default:
//            break;
//    }
//
//    /*����ģʽѡ�����*/
//    index = gSysSetInfo.pf_switch << 2 | gSysSetInfo.pv_switch << 1 | 1;
//    switch(index)
//    {
//        case 0://������
//        {
//            break;
//        }
//        case 1://ֻ��QV��Ҳ�ּ��֣�1.����2.�޹����ƣ�3.QV���ƣ�4.QPģʽ�������޹�
//        {
//
//            allow_active_pwr = sqrtf(apparent_pwr * apparent_pwr - qv_reactive_pwr * qv_reactive_pwr);//�й���ֵ
//            if(fabs(inv_active_pwr) > allow_active_pwr)
//            {
//                InvPwr->inv_output_act_pwr = inv_active_pwr >= 0 ? allow_active_pwr : (-1 * allow_active_pwr);
//            }
//            else
//            {
//                InvPwr->inv_output_act_pwr = inv_active_pwr;
//            }
//            InvPwr->inv_output_reactive_pwr = qv_reactive_pwr;
//            break;
//        }
//        case 2://PVģʽ
//        {
//            //�޹�ģʽһֱ��
//            break;
//        }
//        case 3://PV��QVģʽ,�����޹�
//        {
//
//            allow_active_pwr = sqrtf(apparent_pwr * apparent_pwr - qv_reactive_pwr * qv_reactive_pwr);
//            if(pv_max_active_pwr <= inv_active_pwr)
//            {
//                actual_active_pwr = inv_active_pwr;
//            }
//            else
//            {
//                actual_active_pwr = pv_max_active_pwr;
//            }
//
//            if(fabs(actual_active_pwr) > fabs(allow_active_pwr))
//            {
//                InvPwr->inv_output_act_pwr = actual_active_pwr >= 0 ? allow_active_pwr : (-1 * allow_active_pwr);
//            }
//            else
//            {
//                InvPwr->inv_output_act_pwr = actual_active_pwr;
//            }
//
//            InvPwr->inv_output_reactive_pwr = qv_reactive_pwr;
//            break;
//        }
//        case 4://PFģʽ
//        {
//            //�޹�ģʽһֱ��
//            break;
//        }
//        case 5://PF��QVģʽ�������й�
//        {
//            allow_reactive_pwr = sqrtf(apparent_pwr * apparent_pwr - pf_active_pwr * pf_active_pwr);
//
//            if(fabs(qv_reactive_pwr) > allow_reactive_pwr)
//            {
//                InvPwr->inv_output_reactive_pwr = qv_reactive_pwr >= 0 ? allow_reactive_pwr : (-1 * allow_reactive_pwr);
//            }
//            else
//            {
//                InvPwr->inv_output_reactive_pwr = qv_reactive_pwr;
//            }
//
//            InvPwr->inv_output_act_pwr = pf_active_pwr;
//            break;
//        }
//        case 6://PF��PVģʽ
//        {
//            //�޹�ģʽһֱ��
//            break;
//        }
//        case 7://ȫ�����ڣ�QV��PV��PF��
//        {
//            allow_reactive_pwr = sqrtf(apparent_pwr * apparent_pwr - pf_active_pwr * pf_active_pwr);
//
//            if(fabs(qv_reactive_pwr) > allow_reactive_pwr)
//            {
//                InvPwr->inv_output_reactive_pwr = qv_reactive_pwr >= 0 ? allow_reactive_pwr : (-1 * allow_reactive_pwr);
//            }
//            else
//            {
//                InvPwr->inv_output_reactive_pwr = qv_reactive_pwr;
//            }
//
//            InvPwr->inv_output_act_pwr = pf_active_pwr;
//            break;
//        }
//        default:
//            break;
//    }
//}
//
//
//void ApprovalStdInfoGet(void)
//{
//    if(gSysSetInfo.grid_standards == 0x02)
//    {
//        e_national_standards = EN_AUSTRALIA_A;
//    }
//    else if(gSysSetInfo.grid_standards == 0x03)
//    {
//        e_national_standards = EN_AUSTRALIA_B;
//    }
//    else if(gSysSetInfo.grid_standards == 0x04)
//    {
//        e_national_standards = EN_AUSTRALIA_C;
//    }
//    else if(gSysSetInfo.grid_standards == 0x05)
//    {
//        e_national_standards = EN_NEW_ZEALAND;
//    }
//    else
//    {
//        e_national_standards = EN_AUSTRALIA_A;
//    }
//    switch(e_national_standards)
//    {
//        case EN_AUSTRALIA_A:
//        {
//            gApprovalStdInfo.over_vol_level1 = 275;
//            gApprovalStdInfo.over_vol_level2 = 265;
//            gApprovalStdInfo.under_vol_level1 = 180;
//            gApprovalStdInfo.under_vol_level2 = 70;
//            gApprovalStdInfo.vol_nom_max = 258;
//            gApprovalStdInfo.over_vol_stop_out_activepwr = 260;
//            gApprovalStdInfo.under_vol_stop_out_activepwr = 180;
//            gApprovalStdInfo.recover_over_vol_stop_out_activepwr = 253;
//            gApprovalStdInfo.recover_under_vol_stop_out_activepwr = 205;
//            gApprovalStdInfo.over_freq = 52;
//            gApprovalStdInfo.under_freq = 47;
//            gApprovalStdInfo.recover_over_freq = 50.15;
//            gApprovalStdInfo.recover_under_freq = 47.7;
//            gApprovalStdInfo.pf_low_freq = 49.75;
//            gApprovalStdInfo.pf_high_freq = 50.25;
//            gApprovalStdInfo.pf_stop_under_freq = 47;
//            gApprovalStdInfo.pf_stop_upper_freq = 52;
//            gApprovalStdInfo.pf_freq_pwr_max = 48;
//            gApprovalStdInfo.pf_stop_chg_freq = 49;
//            gApprovalStdInfo.pf_stop_dsg_freq = 50.75;
//            gApprovalStdInfo.pf_freq_hyst = 0.1;
//            gApprovalStdInfo.qv_ful_reac_out_Volth = 207;
//            gApprovalStdInfo.qv_start_out_reac_Volth = 205;
//            gApprovalStdInfo.qv_start_abs_reac_Volth = 215;
//            gApprovalStdInfo.qv_ful_reac_abs_Volth = 207;
//            gApprovalStdInfo.qv_max_prov_Preac = 0.44;
//            gApprovalStdInfo.qv_max_abs_Preac = 0.6;
//            gApprovalStdInfo.pv_dsg_start_derate_Volth = 253;
//            gApprovalStdInfo.pv_dsg_min_out_Volth = 260;
//            gApprovalStdInfo.pv_chg_start_derate_Volth = 207;
//            gApprovalStdInfo.pv_chg_min_out_Volth = 215;
//            gApprovalStdInfo.pv_chg_Pmin = 0.2;
//            break;
//        }
//        case EN_AUSTRALIA_B:
//        {
//            gApprovalStdInfo.over_vol_level1 = 275;
//            gApprovalStdInfo.over_vol_level2 = 265;
//            gApprovalStdInfo.under_vol_level1 = 180;
//            gApprovalStdInfo.under_vol_level2 = 70;
//            gApprovalStdInfo.vol_nom_max = 258;
//            gApprovalStdInfo.over_vol_stop_out_activepwr = 260;
//            gApprovalStdInfo.under_vol_stop_out_activepwr = 180;
//            gApprovalStdInfo.recover_over_vol_stop_out_activepwr = 253;
//            gApprovalStdInfo.recover_under_vol_stop_out_activepwr = 205;
//            gApprovalStdInfo.over_freq = 52;
//            gApprovalStdInfo.under_freq = 47;
//            gApprovalStdInfo.recover_over_freq = 50.15;
//            gApprovalStdInfo.recover_under_freq = 47.7;
//            gApprovalStdInfo.pf_low_freq = 49.85;
//            gApprovalStdInfo.pf_high_freq = 50.15;
//            gApprovalStdInfo.pf_stop_under_freq = 47;
//            gApprovalStdInfo.pf_stop_upper_freq = 52;
//            gApprovalStdInfo.pf_freq_pwr_max = 48;
//            gApprovalStdInfo.pf_stop_chg_freq = 49;
//            gApprovalStdInfo.pf_stop_dsg_freq = 50.75;
//            gApprovalStdInfo.pf_freq_hyst = 0.1;
//            gApprovalStdInfo.qv_ful_reac_out_Volth = 220;
//            gApprovalStdInfo.qv_start_out_reac_Volth = 220;
//            gApprovalStdInfo.qv_start_abs_reac_Volth = 230;
//            gApprovalStdInfo.qv_ful_reac_abs_Volth = 220;
//            gApprovalStdInfo.qv_max_prov_Preac = 0.3;
//            gApprovalStdInfo.qv_max_abs_Preac = 0.4;
//            gApprovalStdInfo.pv_dsg_start_derate_Volth = 250;
//            gApprovalStdInfo.pv_dsg_min_out_Volth = 260;
//            gApprovalStdInfo.pv_chg_start_derate_Volth = 195;
//            gApprovalStdInfo.pv_chg_min_out_Volth = 215;
//            gApprovalStdInfo.pv_chg_Pmin = 0;
//            break;
//        }
//        case EN_AUSTRALIA_C:
//        {
//            gApprovalStdInfo.over_vol_level1 = 275;
//            gApprovalStdInfo.over_vol_level2 = 265;
//            gApprovalStdInfo.under_vol_level1 = 180;
//            gApprovalStdInfo.under_vol_level2 = 70;
//            gApprovalStdInfo.vol_nom_max = 258;
//            gApprovalStdInfo.over_vol_stop_out_activepwr = 260;
//            gApprovalStdInfo.under_vol_stop_out_activepwr = 180;
//            gApprovalStdInfo.recover_over_vol_stop_out_activepwr = 253;
//            gApprovalStdInfo.recover_under_vol_stop_out_activepwr = 205;
//            gApprovalStdInfo.over_freq = 55;
//            gApprovalStdInfo.under_freq = 45;
//            gApprovalStdInfo.recover_over_freq = 50.15;
//            gApprovalStdInfo.recover_under_freq = 47.7;
//            gApprovalStdInfo.pf_low_freq = 49.5;
//            gApprovalStdInfo.pf_high_freq = 50.5;
//            gApprovalStdInfo.pf_stop_under_freq = 45;
//            gApprovalStdInfo.pf_stop_upper_freq = 55;
//            gApprovalStdInfo.pf_freq_pwr_max = 47;
//            gApprovalStdInfo.pf_stop_chg_freq = 48.25;
//            gApprovalStdInfo.pf_stop_dsg_freq = 51.75;
//            gApprovalStdInfo.pf_freq_hyst = 0.05;
//            gApprovalStdInfo.qv_ful_reac_out_Volth = 240;
//            gApprovalStdInfo.qv_start_out_reac_Volth = 235;
//            gApprovalStdInfo.qv_start_abs_reac_Volth = 240;
//            gApprovalStdInfo.qv_ful_reac_abs_Volth = 235;
//            gApprovalStdInfo.qv_max_prov_Preac = 0.44;
//            gApprovalStdInfo.qv_max_abs_Preac = 0.6;
//            gApprovalStdInfo.pv_dsg_start_derate_Volth = 253;
//            gApprovalStdInfo.pv_dsg_min_out_Volth = 260;
//            gApprovalStdInfo.pv_chg_start_derate_Volth = 207;
//            gApprovalStdInfo.pv_chg_min_out_Volth = 215;
//            gApprovalStdInfo.pv_chg_Pmin = 0.2;
//            break;
//        }
//        case EN_NEW_ZEALAND:
//        {
//            gApprovalStdInfo.over_vol_level1 = 275;
//            gApprovalStdInfo.over_vol_level2 = 265;
//            gApprovalStdInfo.under_vol_level1 = 180;
//            gApprovalStdInfo.under_vol_level2 = 70;
//            gApprovalStdInfo.vol_nom_max = 249;
//            gApprovalStdInfo.over_vol_stop_out_activepwr = 260;
//            gApprovalStdInfo.under_vol_stop_out_activepwr = 180;
//            gApprovalStdInfo.recover_over_vol_stop_out_activepwr = 253;
//            gApprovalStdInfo.recover_under_vol_stop_out_activepwr = 205;
//            gApprovalStdInfo.over_freq = 55;
//            gApprovalStdInfo.under_freq = 45;
//            gApprovalStdInfo.recover_over_freq = 50.15;
//            gApprovalStdInfo.recover_under_freq = 47.7;
//            gApprovalStdInfo.pf_low_freq = 49.8;
//            gApprovalStdInfo.pf_high_freq = 50.2;
//            gApprovalStdInfo.pf_stop_under_freq = 45;
//            gApprovalStdInfo.pf_stop_upper_freq = 55;
//            gApprovalStdInfo.pf_freq_pwr_max = 48;
//            gApprovalStdInfo.pf_stop_chg_freq = 49;
//            gApprovalStdInfo.pf_stop_dsg_freq = 51;
//            gApprovalStdInfo.pf_freq_hyst = 0.1;
//            gApprovalStdInfo.qv_ful_reac_out_Volth = 258;
//            gApprovalStdInfo.qv_start_out_reac_Volth = 255;
//            gApprovalStdInfo.qv_start_abs_reac_Volth = 255;
//            gApprovalStdInfo.qv_ful_reac_abs_Volth = 244;
//            gApprovalStdInfo.qv_max_prov_Preac = 0.6;
//            gApprovalStdInfo.qv_max_abs_Preac = 0.6;
//            gApprovalStdInfo.pv_dsg_start_derate_Volth = 242;
//            gApprovalStdInfo.pv_dsg_min_out_Volth = 216;
//            gApprovalStdInfo.pv_chg_min_out_Volth = 224;
//            gApprovalStdInfo.pv_chg_Pmin = 0.2;
//            break;
//        }
//        default:
//            break;
//    }
//}
//
//void ReactivepwrMode(void)
//{
//    if(gSysSetInfo.reactive_power_control_method == 0)
//    {
//        e_reative_pwr_mode = EN_POWER_FACTOR_MODE;
//    }
//    else if(gSysSetInfo.reactive_power_control_method == 1)
//    {
//        e_reative_pwr_mode = EN_REATIVE_PWR_MODE;
//    }
//    else if(gSysSetInfo.reactive_power_control_method == 2)
//    {
//        e_reative_pwr_mode = EN_QV_PWR_MODE;
//    }
//    else if(gSysSetInfo.reactive_power_control_method == 3)
//    {
//        e_reative_pwr_mode = EN_QP_PWR_MODE;
//    }
//}
```

### `app/authentication_func.h`

```c
/*
 * autheritication_func.h
 *
 *  Created on: 2025��3��21��
 *      Author: Nova006352
 */

#ifndef APP_AUTHENTICATION_FUNC_H_
#define APP_AUTHENTICATION_FUNC_H_

#include "F28x_Project.h"
/* ���ұ�׼ */
typedef enum
{
    EN_AUSTRALIA_A,  //���ޡ���A
    EN_AUSTRALIA_B,  //���ޡ���B
    EN_AUSTRALIA_C,  //���ޡ���C
    EN_NEW_ZEALAND,  //������
    EN_NATIONAL_STANDARDS_NUM,
} EN_NATIONAL_STANDARDS;

/* ��ѹ����� */
typedef enum
{
    EN_VOLT_SECTION_1,  //��ѹ�����1
    EN_VOLT_SECTION_2,  //��ѹ�����2
    EN_VOLT_SECTION_3,  //��ѹ�����3
    ENEN_VOLT_SECTION_NUM,
} EN_VOLT_SECTION;

/*����Ƶ����*/
typedef enum
{
    EN_UNDER_FREQ_SECTION,  //ǷƵ����
    EN_LOWER_FREQ_SECTION,  //��Ƶ����
    EN_NORMAL_FREQ_SECTION, //��������
    EN_UPPER_FREQ_SECTION,  //��Ƶ����
    EN_OVER_FREQ_SECTION,   //��Ƶ����
} EN_FREQUENCY_SECTION;

/*�޹�ģʽ*/
typedef enum
{
    EN_POWER_FACTOR_MODE,  //��������ģʽ
    EN_REATIVE_PWR_MODE,  //�޹�����ģʽ
    EN_QV_PWR_MODE, //QVģʽ
    EN_QP_PWR_MODE,  //QPģʽ
} EN_REATIVE_PWR_CTRL_MODE;


typedef struct _inv_reactive_coefficient_stru_t
{
    float qv_coefficient_k;
    float qv_coefficient_b;
    float pv_coefficient_k;
    float pv_coefficient_b;
    float pf_coefficient_k;
    float pf_coefficient_b;
} InvReactiveCoefficient_t;
typedef struct _approval_std_info_stru_t
{
    float over_vol_level1;
    float over_vol_level2;
    float under_vol_level1;
    float under_vol_level2;
    float vol_nom_max;
    float over_vol_stop_out_activepwr;
    float under_vol_stop_out_activepwr;
    float recover_over_vol_stop_out_activepwr;
    float recover_under_vol_stop_out_activepwr;
    float over_freq;
    float under_freq;
    float recover_over_freq;
    float recover_under_freq;
    float pf_low_freq;
    float pf_high_freq;
    float pf_stop_under_freq;
    float pf_stop_upper_freq;
    float pf_freq_pwr_max;
    float pf_stop_chg_freq;
    float pf_stop_dsg_freq;
    float pf_freq_hyst;
    float qv_ful_reac_out_Volth;
    float qv_start_out_reac_Volth;
    float qv_start_abs_reac_Volth;
    float qv_ful_reac_abs_Volth;
    float qv_max_prov_Preac;
    float qv_max_abs_Preac;
    float pv_dsg_start_derate_Volth;
    float pv_dsg_min_out_Volth;
    float pv_chg_start_derate_Volth;
    float pv_chg_min_out_Volth;
    float pv_chg_Pmin;
} ApprovalStdInfo_t;

/*��֤����*/
typedef struct inverter_pwr_stru_t
{
    float32 inv_output_act_pwr;
    float32 inv_output_reactive_pwr;
} InverterPwr_t;

extern InverterPwr_t _gInvPwr;
extern EN_NATIONAL_STANDARDS e_national_standards;
extern ApprovalStdInfo_t gApprovalStdInfo;
void ApprovalStdInfoGet(void);
void ReactivepwrMode(void);
//void PowerCtrlModeHandle(EN_NATIONAL_STANDARDS e_national_standards,float inv_active_pwr,float avtive_pwr,float reavtive_pwr);
void PowerCtrlModeHandle(EN_NATIONAL_STANDARDS e_national_standards,float inv_active_pwr,InverterPwr_t* InvPwr);


#endif /* APP_AUTHENTICATION_FUNC_H_ */
```

### `app/sys_info.c`

```c
/*
 * sys_info.c
 *
 *  Created on: 2025-3-20
 *      Author: Nova006352
 */

#include "sys_info.h"
#include "cla_cpu_share.h"
#include "ph3_calc_ifc_update.h"
#include "firmware_upgrate.h"
#include "adc.h"
#include "fault.h"
#include "algo_interface.h"
#include "fan.h"
#include "ntc.h"
#include "firmware_upgrate.h"
#include "electric_calculate.h"
#include "ct.h"
#include "debug_analysis.h"
#include "flash_default.h"
#include "control_state.h"
UploadInfo_t    gstUploadInfo;

#define PCS_HOLD_REG_FLASH_ADDR             (0x200000)    /*PCS hold register flash address*/
#define PCS_HOLD_REG_VALID_FLASH_ADDR       (0x200FF8)    /*PCS hold register valid flag address*/
#define BACK_UP_PAGE_NUM                    (20)          /*Number of local backup flash pages*/


/**
* @brief: DefaultInfoGet
* @param:
* @return:
* @note:
*
*/
void DefaultInfoGet(void)
{
}
#define INVERTER_MODEL 0X83

// Macro: low 16 bits of 32-bit value
#define GET_LOW_16_BITS(value) ((uint16_t)(((int32_t)(value)) & 0x0000FFFF))

// Macro: high 16 bits of 32-bit value
#define GET_HIGH_16_BITS(value) ((uint16_t)((((int32_t)(value)) >> 16) & 0x0000FFFF))

/**
* @brief: UploadInfoInit
* @param: void
* @return:Nothing
* @note:Upload info initialization
*/
void UploadInfoInit()
{
    memset((uint16_t*)&gstUploadInfo, 0x00, (int)sizeof(UploadInfo_t));
}
/**
* @brief: GetPcsModeId
* @param: void
* @return:Nothing
* @note:Get PCS mode ID
*/
void GetPcsModeId()
{
    gstUploadInfo.upload_basic_info.mode_id = gstVersionInfo.pcs_version_info.model_id;
}
/**
* @brief: GetPcsVersionInfo
* @param: void
* @return:Nothing
* @note:Get PCS hardware/software version
*/
void GetPcsVersionInfo()
{
    uint16_t temp[3] = {0x0000,0x0002,0x0000};
    gstUploadInfo.upload_basic_info.software_ver[0] = (temp[0] << 12) | (temp[1] << 6) | temp[2];
//    gstUploadInfo.software_ver = gstVersionInfo.pcs_version_info.soft_version;
    memcpy((uint16_t*)&gstUploadInfo.upload_basic_info.software_ver_Extend, (uint16_t*)&gstVersionInfo.pcs_version_info.extern_version, (int)sizeof(gstVersionInfo.pcs_version_info.extern_version));
}

/**
* @brief: GetInverterModel
* @param: void
* @return:Nothing
* @note:Get inverter model
*/
void GetInverterModel()
{
    gstUploadInfo.upload_basic_info.hw_version = INVERTER_MODEL;
}
/**
* @brief: GetSysInputType
* @param: void
* @return:Nothing
* @note:Get system input type
*/

void GetSysInputType()
{
    if(gstPcsInvRunState == INV_STATE_GRID_CONNECTED)//Grid connected
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.grid_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.grid_input = 0;
    }
    if(gstPcsInvRunState == INV_STATE_GEN_CONNECTED)//Generator
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.gen_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.gen_input = 0;
    }
    if(Read_Mppt_state(0) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 0;
    }
    if(Read_Mppt_state(1) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 0;
    }
    if(Read_Mppt_state(2) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 0;
    }
    if(Read_Mppt_state(3) == 1)
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 1;
    }
    else
    {
        gstUploadInfo.upload_module_status.sys_input_stat.bit.pv1_input = 0;
    }
}
/**
* @brief: GetSysState
* @param: void
* @return:Nothing
* @note:Get system status
*/
 void GetSysState()
 {
     //TODO: hybrid/bypass/shutdown not handled yet
     if(gstPcsInvRunState == INV_STATE_STANDBY)//Standby state
     {
         gstUploadInfo.upload_module_status.inv_status = 0;
     }
     else if(gstPcsInvRunState == INV_STATE_SELFTEST)//Self-test state
     {
         gstUploadInfo.upload_module_status.inv_status = 1;
     }
     else if(gstPcsInvRunState == INV_STATE_DISCONNECTED)
     {
         gstUploadInfo.upload_module_status.inv_status = 3;
     }
     else
     {
         gstUploadInfo.upload_module_status.inv_status = 0;
     }
 }
 /**
 * @brief: GetInvState
 * @param: void
 * @return:Nothing
 * @note:Get inverter status
 */
void GetInvState()
{
    if(gstPcsInvRunState == INV_STATE_STANDBY)//Standby
    {
        gstUploadInfo.upload_module_status.invOnGridState = 0;
    }
    else if(gstPcsInvRunState == INV_STATE_DISCONNECTED)//Off-grid
    {
        gstUploadInfo.upload_module_status.invOnGridState = 1;
    }
    else if(gstPcsInvRunState == INV_STATE_GRID_CONNECTED || gstPcsInvRunState == INV_STATE_GEN_CONNECTED )//Grid connected
    {
        gstUploadInfo.upload_module_status.invOnGridState = 2;
    }
    else if(gstPcsInvRunState == INV_STATE_GRID_CONNECTING || gstPcsInvRunState == INV_STATE_GEN_CONNECTING)//Off-grid to connect
    {
        gstUploadInfo.upload_module_status.invOnGridState = 3;
    }
    else if(gstPcsInvRunState == INV_STATE_GRID_DISCONNECTING || gstPcsInvRunState == INV_STATE_GEN_DISCONNECTING)//Connect to off-grid
    {
        gstUploadInfo.upload_module_status.invOnGridState = 4;
    }
    else
    {
        gstUploadInfo.upload_module_status.invOnGridState = 0;
    }
}

/**
* @brief: GetMppt1State
* @param: void
* @return:Nothing
* @note:Get MPPT1 status
*/
void GetMppt1State()
{
    gstUploadInfo.upload_module_status.mppt1_status = Read_Mppt_state(0);
}

/**
* @brief: GetMppt2State
* @param: void
* @return:Nothing
* @note:Get MPPT2 status
*/
void GetMppt2State()
{
    gstUploadInfo.upload_module_status.mppt2_status = Read_Mppt_state(1);
}



/**
* @brief: GetMppt3State
* @param: void
* @return:Nothing
* @note:Get MPPT3 status
*/
void GetMppt3State()
{
    gstUploadInfo.upload_module_status.mppt3_status = Read_Mppt_state(2);
}


/**
* @brief: GetMppt4State
* @param: void
* @return:Nothing
* @note:Get MPPT4 status
*/
void GetMppt4State()
{
    gstUploadInfo.upload_module_status.mppt4_status = Read_Mppt_state(3);
}

/**
* @brief: GetAlarmCode
* @param: void
* @return:Nothing
* @note:Get warning code
*/
void GetAlarmCode()
{
//    gstUploadInfo.upload_fault_info.GridAlarm = GetGridWarningHighValue();
//    gstUploadInfo.upload_fault_info.PVAlarm[0] = GetPvWarningLowValue();
//    gstUploadInfo.upload_fault_info.PVAlarm[1] = GetPvWarningHighValue();
//    gstUploadInfo.upload_fault_info.BusAlarm = GetBusWarningValue();
//    gstUploadInfo.upload_fault_info.InvAlarm = GetInvWarningValue();
//    gstUploadInfo.upload_fault_info.LoadAlarm = GetLoadWarningValue();
//    gstUploadInfo.upload_fault_info.TempAlarm = GetTempWarningValue();
//    gstUploadInfo.upload_fault_info.GenAlarm  = GetGenWarningValue();
//    gstUploadInfo.upload_fault_info.OtherAlarm = GetOtherWarningValue();

}

/**
* @brief: GetErrorCode
* @param: void
* @return:Nothing
* @note:Get fault code
*/
void GetErrorCode()
{
//    gstUploadInfo.upload_fault_info.GridError = GetGridErrorValue();
//    gstUploadInfo.upload_fault_info.PVError[0] = GetPvErrorLowValue();
//    gstUploadInfo.upload_fault_info.PVError[1] = GetPvErrorHighValue();
//    gstUploadInfo.upload_fault_info.BusError = GetBusErrorValue();
//    gstUploadInfo.upload_fault_info.InvError = GetInvErrorValue();
//    gstUploadInfo.upload_fault_info.LoadError = GetLoadErrorValue();
//    gstUploadInfo.upload_fault_info.TempError = GetTempErrorValue();
//    gstUploadInfo.upload_fault_info.GenError = GetGenErrorValue();
//    gstUploadInfo.upload_fault_info.OtherError = GetOtherErrorValue();
}

void GetNoticeCode()
{
    gstUploadInfo.upload_fault_info.GridNotice = 0x0000;
    gstUploadInfo.upload_fault_info.PVNotice[0] = 0x0000;
    gstUploadInfo.upload_fault_info.PVNotice[1] = 0x0000;
    gstUploadInfo.upload_fault_info.BusNotice = 0x0000;
    gstUploadInfo.upload_fault_info.InvNotice = 0x0000;
    gstUploadInfo.upload_fault_info.LoadNotice = 0x0000;
    gstUploadInfo.upload_fault_info.TempNotice = 0x0000;
    gstUploadInfo.upload_fault_info.GenNotice = 0x0000;
    gstUploadInfo.upload_fault_info.OtherNotice = 0x0000;
}

/**
* @brief: GetCtOrMeterInfo
* @param: void
* @return:Nothing
* @note:Get fault code
*/

void GetCtOrMeterInfo()
{
    static uint16_t cont = 0;
    if(MeterModeGet() == 1)//Meter
       {
           if(cont == 0)
           {
               gstUploadInfo.upload_Power_info.r_phase_meter_curr =  GetMeterRCurInfo() * 10;
               gstUploadInfo.upload_Power_info.r_phase_meter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_vol.a_phase_rms * GetMeterRCurInfo());
               gstUploadInfo.upload_Power_info.r_phase_meter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_vol.a_phase_rms * GetMeterRCurInfo());
               gstUploadInfo.upload_Power_info.r_phase_meter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.a_phase_active);
               gstUploadInfo.upload_Power_info.r_phase_meter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.a_phase_active);
               gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.a_phase_reactive);
               gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.a_phase_reactive);
           }
           else if(cont == 1)
           {
               gstUploadInfo.upload_Power_info.s_phase_meter_curr =  GetMeterSCurInfo() * 10;
               gstUploadInfo.upload_Power_info.s_phase_meter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_vol.b_phase_rms * GetMeterSCurInfo());
               gstUploadInfo.upload_Power_info.s_phase_meter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_vol.b_phase_rms * GetMeterSCurInfo());
               gstUploadInfo.upload_Power_info.s_phase_meter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.b_phase_active);
               gstUploadInfo.upload_Power_info.s_phase_meter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.b_phase_active);
               gstUploadInfo.upload_Power_info.s_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.b_phase_reactive);
               gstUploadInfo.upload_Power_info.s_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.b_phase_reactive);
           }
           else if(cont == 2)
           {
               gstUploadInfo.upload_Power_info.t_phase_meter_curr =  GetMeterTCurInfo() * 10;
               gstUploadInfo.upload_Power_info.t_phase_meter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_vol.c_phase_rms * GetMeterTCurInfo());
               gstUploadInfo.upload_Power_info.t_phase_meter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_vol.c_phase_rms * GetMeterTCurInfo());
               gstUploadInfo.upload_Power_info.t_phase_meter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.c_phase_active);
               gstUploadInfo.upload_Power_info.t_phase_meter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.c_phase_active);
               gstUploadInfo.upload_Power_info.t_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.grid_power.c_phase_reactive);
               gstUploadInfo.upload_Power_info.t_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.grid_power.c_phase_reactive);

           }
       }
       else//CT info
       {
           if(cont == 0)
           {
               gstUploadInfo.upload_Power_info.r_phase_meter_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.a_phase_apperant);
               gstUploadInfo.upload_Power_info.r_phase_meter_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.a_phase_apperant);
               gstUploadInfo.upload_Power_info.r_phase_meter_active_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.a_phase_active);
               gstUploadInfo.upload_Power_info.r_phase_meter_active_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.a_phase_active);
               gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.a_phase_reactive);
               gstUploadInfo.upload_Power_info.r_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.a_phase_reactive);
               if(gCtDataInfo.ct_power.a_phase_active < 0)
               {
                   gstUploadInfo.upload_Power_info.r_phase_meter_curr =  -1 * (int16_t)(gstPh3CalcInfo.ct_curr.a_phase_rms * 10);
               }
               else
               {
                   gstUploadInfo.upload_Power_info.r_phase_meter_curr = (int16_t)(gstPh3CalcInfo.ct_curr.a_phase_rms * 10);
               }
           }
           else if(cont == 1)
           {
               gstUploadInfo.upload_Power_info.s_phase_meter_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.b_phase_apperant);
               gstUploadInfo.upload_Power_info.s_phase_meter_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.b_phase_apperant);
               gstUploadInfo.upload_Power_info.s_phase_meter_active_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.b_phase_active);
               gstUploadInfo.upload_Power_info.s_phase_meter_active_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.b_phase_active);
               gstUploadInfo.upload_Power_info.s_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.b_phase_reactive);
               gstUploadInfo.upload_Power_info.s_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.b_phase_reactive);
               if(gCtDataInfo.ct_power.b_phase_active < 0)
               {
                   gstUploadInfo.upload_Power_info.s_phase_meter_curr =  -1 * (int16_t)(gstPh3CalcInfo.ct_curr.b_phase_rms * 10);
               }
               else
               {
                   gstUploadInfo.upload_Power_info.s_phase_meter_curr = (int16_t)(gstPh3CalcInfo.ct_curr.b_phase_rms * 10);
               }
           }
           else if(cont == 2)
           {
               gstUploadInfo.upload_Power_info.t_phase_meter_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.c_phase_apperant);
               gstUploadInfo.upload_Power_info.t_phase_meter_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.c_phase_apperant);
               gstUploadInfo.upload_Power_info.t_phase_meter_active_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.c_phase_active);
               gstUploadInfo.upload_Power_info.t_phase_meter_active_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.c_phase_active);
               gstUploadInfo.upload_Power_info.t_phase_meter_reactive_power[1] = GET_LOW_16_BITS(gCtDataInfo.ct_power.c_phase_reactive);
               gstUploadInfo.upload_Power_info.t_phase_meter_reactive_power[0] = GET_HIGH_16_BITS(gCtDataInfo.ct_power.c_phase_reactive);
               if(gCtDataInfo.ct_power.c_phase_active < 0)
               {
                   gstUploadInfo.upload_Power_info.t_phase_meter_curr =  -1 * (int16_t)(gstPh3CalcInfo.ct_curr.c_phase_rms * 10);
               }
               else
               {
                   gstUploadInfo.upload_Power_info.t_phase_meter_curr = (int16_t)(gstPh3CalcInfo.ct_curr.c_phase_rms * 10);
               }
           }
       }
    cont++;
    if(cont >= 3)
    {
        cont = 0;
    }
}

/**
* @brief: GetTemperatureValue
* @param: void
* @return:Nothing
* @note:Get fault code
*/

void GetTemperatureValue(void)
{
    gstUploadInfo.upload_module_status.chassis_internal_temp = (int16_t)(NTCGetFilterTemp(NTC_CHASSIS_TEMP) * 10);
    gstUploadInfo.upload_module_status.inverter_radiator_temp = (int16_t)(NTCGetFilterTemp(NTC_INV_TEMP) * 10);
    gstUploadInfo.upload_module_status.pv_temp = (int16_t)(NTCGetFilterTemp(NTC_PV_TEMP) * 10);
}


/**
* @brief: GetErrorCode
* @param: void
* @return:Nothing
* @note:Get all energy meter info
*/
void ElectricInfoGet_UPload(void)
{
    //Energy stats sent in 0.1 kWh units
    gstUploadInfo.upload_Capacity_info.PV_ElectricGenTotal[0] = GET_HIGH_16_BITS(GetPvTotalElectric); //Total PV generation
    gstUploadInfo.upload_Capacity_info.PV_ElectricGenTotal[1] = GET_LOW_16_BITS(GetPvTotalElectric);

    gstUploadInfo.upload_Capacity_info.elec_purchase_consum_total[0] = GET_HIGH_16_BITS(GetBuyElectric()); //Energy bought from grid
    gstUploadInfo.upload_Capacity_info.elec_purchase_consum_total[1] = GET_LOW_16_BITS(GetBuyElectric());

    gstUploadInfo.upload_Capacity_info.elec_sale_consum_total[0] = GET_HIGH_16_BITS(GetSellElectric());   //Energy sold to grid
    gstUploadInfo.upload_Capacity_info.elec_sale_consum_total[1] = GET_LOW_16_BITS(GetSellElectric());

    gstUploadInfo.upload_Capacity_info.Bat_ElectricChgTotal[0] = GET_HIGH_16_BITS(GetBatChgElectric());  //Total battery charge energy
    gstUploadInfo.upload_Capacity_info.Bat_ElectricChgTotal[1] = GET_LOW_16_BITS(GetBatChgElectric());

    gstUploadInfo.upload_Capacity_info.Bat_ElectricDsgTotal[0] = GET_HIGH_16_BITS(GetBatDsgElectric());   //Total battery discharge energy
    gstUploadInfo.upload_Capacity_info.Bat_ElectricDsgTotal[1] = GET_LOW_16_BITS(GetBatDsgElectric());

    gstUploadInfo.upload_Capacity_info.backup_total_sonsum_power[0] = GET_HIGH_16_BITS(GetEpsElectric());    //Total load energy
    gstUploadInfo.upload_Capacity_info.backup_total_sonsum_power[1] = GET_LOW_16_BITS(GetEpsElectric());

    gstUploadInfo.upload_Capacity_info.load_total_sonsum_power[0] = GET_HIGH_16_BITS(GetRemoteLoadElectric());     //Remote load energy
    gstUploadInfo.upload_Capacity_info.load_total_sonsum_power[1] = GET_LOW_16_BITS(GetRemoteLoadElectric());

    gstUploadInfo.upload_Capacity_info.Gen_ElectricTotal[0] = GET_HIGH_16_BITS(GetGenElectric());     //Total generator energy
    gstUploadInfo.upload_Capacity_info.Gen_ElectricTotal[1] = GET_LOW_16_BITS(GetGenElectric());

    gstUploadInfo.upload_Capacity_info.EV_ElectricChgTotal[0] = GET_HIGH_16_BITS(GetEvChgElectric());    //Total EV charge energy
    gstUploadInfo.upload_Capacity_info.EV_ElectricChgTotal[1] = GET_LOW_16_BITS(GetEvChgElectric());

    gstUploadInfo.upload_Capacity_info.EV_ElectricDsgTotal[0] = GET_HIGH_16_BITS(GetEvDsgElectric());     //Total EV discharge energy
    gstUploadInfo.upload_Capacity_info.EV_ElectricDsgTotal[1] = GET_LOW_16_BITS(GetEvDsgElectric());

    gstUploadInfo.upload_Capacity_info.PV1_ElectricGenTotal[0] = GET_HIGH_16_BITS(GetPv1Electric());   //PV1 total generation
    gstUploadInfo.upload_Capacity_info.PV1_ElectricGenTotal[1] = GET_LOW_16_BITS(GetPv1Electric());

    gstUploadInfo.upload_Capacity_info.PV2_ElectricGenTotal[0] = GET_HIGH_16_BITS(GetPv2Electric());   //PV2 total generation
    gstUploadInfo.upload_Capacity_info.PV2_ElectricGenTotal[1] = GET_LOW_16_BITS(GetPv2Electric());

    gstUploadInfo.upload_Capacity_info.PV3_ElectricGenTotal[0] = GET_HIGH_16_BITS(GetPv3Electric());   //PV3 total generation
    gstUploadInfo.upload_Capacity_info.PV3_ElectricGenTotal[1] = GET_LOW_16_BITS(GetPv3Electric());

    gstUploadInfo.upload_Capacity_info.PV4_ElectricGenTotal[0] = GET_HIGH_16_BITS(0);   //PV4 total generation
    gstUploadInfo.upload_Capacity_info.PV4_ElectricGenTotal[1] = GET_LOW_16_BITS(0);
}


/**
* @brief: GetElectricalInfo
* @param: void
* @return:Nothing
* @note:Get electrical info
*/
void GetElectricalInfo()
{
    static uint16_t cont = 0;
    GetCtOrMeterInfo();
    gstUploadInfo.upload_Power_info.bus_voltage = (uint16_t)((gstPh3CalcInfo.bus.pos_vol_filter +  gstPh3CalcInfo.bus.neg_vol_filter) * 10);

    gstUploadInfo.upload_Power_info.batt_volt = (uint16_t)gstUploadInfo.upload_Power_info.bus_voltage;
    gstUploadInfo.upload_Power_info.batt_current = gstPh3CalcInfo.bus.curr_filter * 10;
    gstUploadInfo.upload_Power_info.batt_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.bus.power_filter);
    gstUploadInfo.upload_Power_info.batt_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.bus.power_filter);
    if(cont == 0)
    {
        gstUploadInfo.upload_Power_info.r_phase_grid_volt = (uint16_t)(gstPh3CalcInfo.grid_vol.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_grid_freq = (uint16_t)(gstPh3CalcInfo.grid_vol.freq * 100);
        gstUploadInfo.upload_Power_info.r_phase_load_volt = (uint16_t)(0 * 10);
        gstUploadInfo.upload_Power_info.r_phase_load_cur = (uint16_t)(0 * 10);

        gstUploadInfo.upload_Power_info.r_phase_oil_volt = (uint16_t)(gstPh3CalcInfo.gen_vol.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_oil_freq = (uint16_t)(gstPh3CalcInfo.gen_vol.freq * 100);
        gstUploadInfo.upload_Power_info.r_phase_oil_curr = (uint16_t)(gstPh3CalcInfo.gen_curr.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_oil_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_oil_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_oil_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.a_phase_active);
        gstUploadInfo.upload_Power_info.r_phase_oil_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.a_phase_active);
        gstUploadInfo.upload_Power_info.r_phase_oil_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.a_phase_reactive);
        gstUploadInfo.upload_Power_info.r_phase_oil_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.a_phase_reactive);

        gstUploadInfo.upload_Power_info.r_phase_inverter_volt = (uint16_t)(gstPh3CalcInfo.inv_vol.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_inverter_curr = (uint16_t)(gstPh3CalcInfo.inv_curr.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_inverter_freq = (uint16_t)(gstPh3CalcInfo.inv_vol.freq * 100);
        gstUploadInfo.upload_Power_info.r_phase_inverter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_inverter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_inverter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.a_phase_active);
        gstUploadInfo.upload_Power_info.r_phase_inverter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.a_phase_active);
        gstUploadInfo.upload_Power_info.r_phase_inverter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.a_phase_reactive);
        gstUploadInfo.upload_Power_info.r_phase_inverter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.a_phase_reactive);

        gstUploadInfo.upload_Power_info.r_phase_backup_volt = (uint16_t)(gstPh3CalcInfo.load_vol.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_backup_curr = (uint16_t)(gstPh3CalcInfo.load_curr.a_phase_rms * 10);
        gstUploadInfo.upload_Power_info.r_phase_backup_fre = (uint16_t)(gstPh3CalcInfo.load_vol.freq * 100);
        gstUploadInfo.upload_Power_info.r_phase_backup_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.load_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_backup_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.load_power.a_phase_apperant);
        gstUploadInfo.upload_Power_info.r_phase_backup_active_power[1]  = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.a_phase_active));
        gstUploadInfo.upload_Power_info.r_phase_backup_active_power[0]  = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.a_phase_active));
        gstUploadInfo.upload_Power_info.r_phase_backup_reactive_power[1] = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.a_phase_reactive));//TODO: confirm protocol type for upload int
        gstUploadInfo.upload_Power_info.r_phase_backup_reactive_power[0] = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.a_phase_reactive));


    }
    else if(cont == 1)
    {
        gstUploadInfo.upload_Power_info.s_phase_grid_volt = (uint16_t)(gstPh3CalcInfo.grid_vol.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_grid_freq = (uint16_t)(gstPh3CalcInfo.grid_vol.freq * 100);
        gstUploadInfo.upload_Power_info.s_phase_load_volt = (uint16_t)(0 * 10);
        gstUploadInfo.upload_Power_info.s_phase_load_cur = (uint16_t)(0 * 10);

        gstUploadInfo.upload_Power_info.s_phase_oil_volt = (uint16_t)(gstPh3CalcInfo.gen_vol.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_oil_freq = (uint16_t)(gstPh3CalcInfo.gen_vol.freq * 100);
        gstUploadInfo.upload_Power_info.s_phase_oil_curr = (uint16_t)(gstPh3CalcInfo.gen_curr.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_oil_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_oil_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_oil_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.b_phase_active);
        gstUploadInfo.upload_Power_info.s_phase_oil_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.b_phase_active);
        gstUploadInfo.upload_Power_info.s_phase_oil_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.b_phase_reactive);
        gstUploadInfo.upload_Power_info.s_phase_oil_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.b_phase_reactive);

        gstUploadInfo.upload_Power_info.s_phase_inverter_volt = (uint16_t)(gstPh3CalcInfo.inv_vol.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_inverter_curr = (uint16_t)(gstPh3CalcInfo.inv_curr.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_inverter_freq = (uint16_t)(gstPh3CalcInfo.inv_vol.freq * 100);
        gstUploadInfo.upload_Power_info.s_phase_inverter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_inverter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_inverter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.b_phase_active);
        gstUploadInfo.upload_Power_info.s_phase_inverter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.b_phase_active);
        gstUploadInfo.upload_Power_info.s_phase_inverter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.b_phase_reactive);
        gstUploadInfo.upload_Power_info.s_phase_inverter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.b_phase_reactive);

        gstUploadInfo.upload_Power_info.s_phase_backup_volt = (uint16_t)(gstPh3CalcInfo.load_vol.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_backup_curr = (uint16_t)(gstPh3CalcInfo.load_curr.b_phase_rms * 10);
        gstUploadInfo.upload_Power_info.s_phase_backup_fre = (uint16_t)(gstPh3CalcInfo.load_vol.freq * 100);
        gstUploadInfo.upload_Power_info.s_phase_backup_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.load_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_backup_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.load_power.b_phase_apperant);
        gstUploadInfo.upload_Power_info.s_phase_backup_active_power[1]  = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.b_phase_active));
        gstUploadInfo.upload_Power_info.s_phase_backup_active_power[0]  = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.b_phase_active));
        gstUploadInfo.upload_Power_info.s_phase_backup_reactive_power[1] = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.b_phase_reactive));//TODO: confirm protocol type for upload int
        gstUploadInfo.upload_Power_info.s_phase_backup_reactive_power[0] = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.b_phase_reactive));


    }
    else if(cont == 2)
    {
        gstUploadInfo.upload_Power_info.t_phase_grid_volt = (uint16_t)(gstPh3CalcInfo.grid_vol.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_grid_freq = (uint16_t)(gstPh3CalcInfo.grid_vol.freq * 100);
        gstUploadInfo.upload_Power_info.t_phase_load_volt = (uint16_t)(0 * 10);
        gstUploadInfo.upload_Power_info.t_phase_load_cur = (uint16_t)(0 * 10);

        gstUploadInfo.upload_Power_info.t_phase_oil_volt = (uint16_t)(gstPh3CalcInfo.gen_vol.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_oil_freq = (uint16_t)(gstPh3CalcInfo.gen_vol.freq * 100);
        gstUploadInfo.upload_Power_info.t_phase_oil_curr = (uint16_t)(gstPh3CalcInfo.gen_curr.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_oil_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_oil_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_oil_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.c_phase_active);
        gstUploadInfo.upload_Power_info.t_phase_oil_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.c_phase_active);
        gstUploadInfo.upload_Power_info.t_phase_oil_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.gen_power.c_phase_reactive);
        gstUploadInfo.upload_Power_info.t_phase_oil_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.gen_power.c_phase_reactive);

        gstUploadInfo.upload_Power_info.t_phase_inverter_volt = (uint16_t)(gstPh3CalcInfo.inv_vol.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_inverter_curr = (uint16_t)(gstPh3CalcInfo.inv_curr.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_inverter_freq = (uint16_t)(gstPh3CalcInfo.inv_vol.freq * 100);
        gstUploadInfo.upload_Power_info.t_phase_inverter_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_inverter_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_inverter_active_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.c_phase_active);
        gstUploadInfo.upload_Power_info.t_phase_inverter_active_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.c_phase_active);
        gstUploadInfo.upload_Power_info.t_phase_inverter_reactive_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.inv_power.c_phase_reactive);
        gstUploadInfo.upload_Power_info.t_phase_inverter_reactive_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.inv_power.c_phase_reactive);

        gstUploadInfo.upload_Power_info.t_phase_backup_volt = (uint16_t)(gstPh3CalcInfo.load_vol.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_backup_curr = (uint16_t)(gstPh3CalcInfo.load_curr.c_phase_rms * 10);
        gstUploadInfo.upload_Power_info.t_phase_backup_fre = (uint16_t)(gstPh3CalcInfo.load_vol.freq * 100);
        gstUploadInfo.upload_Power_info.t_phase_backup_power[1] = GET_LOW_16_BITS(gstPh3CalcInfo.load_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_backup_power[0] = GET_HIGH_16_BITS(gstPh3CalcInfo.load_power.c_phase_apperant);
        gstUploadInfo.upload_Power_info.t_phase_backup_active_power[1]  = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.c_phase_active));
        gstUploadInfo.upload_Power_info.t_phase_backup_active_power[0]  = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.c_phase_active));
        gstUploadInfo.upload_Power_info.t_phase_backup_reactive_power[1] = GET_LOW_16_BITS(fabs(gstPh3CalcInfo.load_power.c_phase_reactive));//TODO: confirm protocol type for upload int
        gstUploadInfo.upload_Power_info.t_phase_backup_reactive_power[0] = GET_HIGH_16_BITS(fabs(gstPh3CalcInfo.load_power.c_phase_reactive));
    }
    if(PvVolAveRead(0) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch1_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch1_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch1_input_power[0] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch1_input_volt = (uint16_t)(PvVolAveRead(0) * 10);
        gstUploadInfo.upload_Power_info.pv_ch1_input_curr = (int16_t)(PvCurAveRead(0) * 10);
        gstUploadInfo.upload_Power_info.pv_ch1_input_power[0] = GET_HIGH_16_BITS(PvPwrAveRead(0));
        gstUploadInfo.upload_Power_info.pv_ch1_input_power[1] = GET_LOW_16_BITS(PvPwrAveRead(0));
    }
    if(PvVolAveRead(1) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch2_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch2_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch2_input_power[0] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch2_input_volt = (uint16_t)(PvVolAveRead(1) * 10);
        gstUploadInfo.upload_Power_info.pv_ch2_input_curr = (int16_t)(PvCurAveRead(1) * 10);
        gstUploadInfo.upload_Power_info.pv_ch2_input_power[0] = GET_HIGH_16_BITS(PvPwrAveRead(1));
        gstUploadInfo.upload_Power_info.pv_ch2_input_power[1] = GET_LOW_16_BITS(PvPwrAveRead(1));
    }
    if(PvVolAveRead(2) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch3_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch3_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch3_input_power[0] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch3_input_volt = (uint16_t)(PvVolAveRead(2) * 10);
        gstUploadInfo.upload_Power_info.pv_ch3_input_curr = (int16_t)(PvCurAveRead(2) * 10);
        gstUploadInfo.upload_Power_info.pv_ch3_input_power[0] = GET_HIGH_16_BITS(PvPwrAveRead(2));
        gstUploadInfo.upload_Power_info.pv_ch3_input_power[1] = GET_LOW_16_BITS(PvPwrAveRead(2));
    }
    if(PvVolAveRead(3) < 50)
    {
        gstUploadInfo.upload_Power_info.pv_ch4_input_volt = 0;
        gstUploadInfo.upload_Power_info.pv_ch4_input_curr = 0;
        gstUploadInfo.upload_Power_info.pv_ch4_input_power[0] = 0;
    }
    else
    {
        gstUploadInfo.upload_Power_info.pv_ch4_input_volt = (uint16_t)(PvVolAveRead(3) * 10);
        gstUploadInfo.upload_Power_info.pv_ch4_input_curr = (int16_t)(PvCurAveRead(3) * 10);
        gstUploadInfo.upload_Power_info.pv_ch4_input_power[0] = GET_HIGH_16_BITS(PvPwrAveRead(3));
        gstUploadInfo.upload_Power_info.pv_ch4_input_power[1] = GET_HIGH_16_BITS(PvPwrAveRead(3));
    }
    cont++;
    if(cont >= 3)
    {
        cont = 0;
    }
}

/**
* @brief: init_upload_info
* @param: void
* @return:Nothing
* @note:Comm protocol self-test code for integration debug only
*/
// void init_upload_info(UploadInfo_t* info)
// {
//     uint16_t *ptr = (uint16_t*)info;
//     size_t total_size = sizeof(UploadInfo_t);
//     size_t element_count = total_size / sizeof(uint16_t);
    
//     for (size_t i = 0; i < element_count; i++) 
//     {
//         ptr[i] = i;
//     }

//     gstUploadInfo.upload_basic_info.mode_id = 0x1111;   //0x6000
//     gstUploadInfo.upload_basic_info.arc_mode_id  = 0x2222;      //0x6020
//     gstUploadInfo.upload_module_status.sys_input_stat.byte = 0x3333;   //0x6060, PowerInterfaceState
//     gstUploadInfo.upload_Capacity_info.PV_ElectricGenTotal[0] = 0x4444;//0x6080 total PV energy
//     gstUploadInfo.upload_Power_info.r_phase_grid_volt = 0x5555; //0x60B0 R-phase grid voltage
//     gstUploadInfo.upload_fault_info.GridError = 0x6666; //0x61C0 grid fault
//     gstUploadInfo.upload_fault_info.fault_info_reserve[31] = 0x7777;//0x621F
// }


/**
* @brief: SysInfoUpload
* @param: void
* @return:Nothing
* @note:Get readable register info
*/
void SysInfoUpload()
{
    GetPcsModeId();
    GetPcsVersionInfo();
    GetInverterModel();
    GetSysInputType();
    GetSysState();
    GetInvState();
    GetMppt1State();
    GetMppt2State();
    GetMppt3State();
    GetMppt4State();//Whether MPPT values are combined
    GetAlarmCode();
    GetErrorCode();
    GetNoticeCode();
    GetTemperatureValue();
    GetElectricalInfo();
    ElectricInfoGet_UPload();
    //init_upload_info(&gstUploadInfo);  //Comm protocol self-test code
}
```

### `app/sys_info.h`

```c
/*
 * sys_info.h
 *
 *  Created on: 2025��3��20��
 *      Author: Nova006352
 */

#ifndef APP_SYS_INFO_H_
#define APP_SYS_INFO_H_

#include "F28x_Project.h"


typedef struct _upload_basic_info_stru_t
{
    /*0x6000-0x601f:�����������Ϣ��32�ֽ�*/
    uint16_t    mode_id;                            //0x6000
    uint16_t    sn[10];                             //0x6001-0x600A
    uint16_t    hw_version;                         //0x600b��Ӳ���汾�ţ�bit[15:8]:��汾��0~25����A~Z������ֵԤ���bit[7:0]:С�汾����0��ʼ��һ���Ӳ���汾����A0
    uint16_t    software_ver[2];                    //0x600C-0x600D���̼��汾��
    uint16_t    software_ver_Extend[12];           //0x600E-0x6019���̼��汾��Ԥ���12�ֽ�
    uint16_t    reserve1[6];                        //0x601A-0x601F��Ԥ��

    /*0x6020-0x603f:�����������Ϣ��32�ֽ�*/
    uint16_t    arc_mode_id;                         //0x6020
    uint16_t    arc_sn[10];                          //0x6021-0x602A
    uint16_t    arc_hw_version;                      //0x602B
    uint16_t    arc_software_ver[2];                 //0x602C-0x602D���̼��汾��
    uint16_t    arc_software_ver_Extend[12];        //0x602E-0x6039���̼��汾��Ԥ���12�ֽ�
    uint16_t    reserve2[6];                         //0x603A-0x603F��Ԥ��

    /*0x6040-0x605f:������ϢԤ���32�ֽ�*/
    uint16_t    reserve_basic_information[32];      //0x6040-0x605f��������ϢԤ��

} _upload_basic_info_t;

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t grid_input         : 1;          // bit:00    �е�����
        uint16_t gen_input          : 1;          // bit:01    �ͻ�����
        uint16_t pv1_input          : 1;          // bit:02    PV1����
        uint16_t pv2_input          : 1;          // bit:03    PV2����
        uint16_t pv3_input          : 1;          // bit:04    PV3����
        uint16_t pv4_input          : 1;          // bit:05    PV4����
        uint16_t reverse06_15       :10;
    } bit;
} SysInputState_t;


typedef struct _upload_module_status_t
{
    /*0x6060-0x607F:ģ��״̬*/
    SysInputState_t    sys_input_stat;              //0x6060��PowerInterfaceState�����ʽӿ�״̬: ÿ��bit����һ���ӿ�
    
    uint16_t    inv_status;                         //0x6061��InvRunState�����������״̬�����������״̬:00H������01H���Լ�02H����ϲ���03H������04H����·05H: �ػ�
    uint16_t    invOnGridState;                     //0x6062��������״̬
    uint16_t    dcdc_status;                        //0x6063��DCDC״̬

    uint16_t    DRMStatus;                          //0x6064

    uint16_t    mppt1_status;                       //0x6065
    uint16_t    mppt2_status;                       //0x6066
    uint16_t    mppt3_status;                       //0x6067
    uint16_t    mppt4_status;                       //0x6068
    uint16_t    Module_status_reserve[7];          //0x6069-0x606f,ģ��״̬��ϢԤ��

    uint16_t    WakeUpStatus;                       //0x6070
//    uint16_t    SysUpgradeStatus;                   //0x6071

    int16_t     inverter_radiator_temp;             //0x6071�������ɢ�����¶�
    int16_t     chassis_internal_temp;              //0x6072������������¶�
    int16_t     pv_temp;                            //0x6073��PV�¶�

    int16_t     temp_reserve[12];                   //0x6074~0x607F��Ԥ���¶��ֶ�


} _upload_module_status_t;


typedef struct _upload_Capacity_info_stru_t
{
    /*0x6080-0x60AF: Power Capacity information:������Ϣ*/
    uint16_t    PV_ElectricGenTotal[2];           //Pv�ܷ�����

    uint16_t    elec_purchase_consum_total[2]    ;//����ܵ��� 

    uint16_t    elec_sale_consum_total[2]        ;//����ܵ���

    uint16_t    Bat_ElectricChgTotal[2];        //����ܳ����

    uint16_t    Bat_ElectricDsgTotal[2];        //����ܷŵ���

    uint16_t    backup_total_sonsum_power[2]     ;//���ظ����ܵ��õ���    EPS_ElectricTotal 

    uint16_t    load_total_sonsum_power[2]       ;//Զ�˸����ܵ��õ���    KWH
    
    uint16_t    Gen_ElectricTotal[2];              //�ͻ��ܷ�����

    uint16_t    EV_ElectricChgTotal[2];            //���׮���豸�ܳ����

    uint16_t    EV_ElectricDsgTotal[2];            //���׮���豸�ܷŵ���

    uint16_t    Power_Information_reserve[4];       //������ϢԤ��
    
    uint16_t    PV1_ElectricGenTotal[2];               //Pv1�ܷ�����

    uint16_t    PV2_ElectricGenTotal[2];               //Pv2�ܷ�����

    uint16_t    PV3_ElectricGenTotal[2];               //Pv3�ܷ�����

    uint16_t    PV4_ElectricGenTotal[2];               //Pv4�ܷ�����

    uint16_t    pv_Capacity_reserve[16];               //pvԤ��

}_upload_Capacity_info_t;


typedef struct _upload_Power_info_stru_t
{
    /*0x60B0-0x61BF:Power_info��������Ϣ*/
    uint16_t    r_phase_grid_volt;              //R �������ѹ
    int16_t     r_phase_meter_curr;             //R �������
    uint16_t    r_phase_grid_freq;              //R �����Ƶ��
    int16_t     r_phase_meter_active_power[2];     //R �����й�����    
    int16_t     r_phase_meter_reactive_power[2];   //R �����޹�����    
    int16_t     r_phase_meter_power[2];            //R �������ڹ���

    uint16_t    s_phase_grid_volt;              //s �������ѹ
    int16_t     s_phase_meter_curr;             //s �������
    uint16_t    s_phase_grid_freq;              //s �����Ƶ��
    int16_t     s_phase_meter_active_power[2];     //s �����й�����    
    int16_t     s_phase_meter_reactive_power[2];   //s �����޹�����    
    int16_t     s_phase_meter_power[2];            //s �������ڹ��� 
    
    uint16_t    t_phase_grid_volt;              //t �������ѹ
    int16_t     t_phase_meter_curr;             //t �������
    uint16_t    t_phase_grid_freq;              //t �����Ƶ��
    int16_t     t_phase_meter_active_power[2];     //t �����й�����    
    int16_t     t_phase_meter_reactive_power[2];   //t �����޹�����    
    int16_t     t_phase_meter_power[2];            //t �������ڹ���  
    
    
    uint16_t    r_phase_ongrid_volt;            //R �ಢ���ڵ�ѹ
    int16_t     r_phase_ongrid_curr;            //R �ಢ���ڵ���
    uint16_t    r_phase_ongrid_fre;             //R �ಢ����Ƶ��
    int16_t     r_phase_ongrid_active_power[2]; //R �ಢ�����й�����
    int16_t     r_phase_ongrid_reactive_power[2];//R �ಢ�����޹�����
    int16_t     r_phase_ongrid_power[2];        //R �ಢ�������ڹ���

    uint16_t    s_phase_ongrid_volt;            //S �ಢ���ڵ�ѹ
    int16_t     s_phase_ongrid_curr;            //S �ಢ���ڵ���
    uint16_t    s_phase_ongrid_fre;             //S �ಢ����Ƶ��
    int16_t     s_phase_ongrid_active_power[2]; //S �ಢ�����й�����
    int16_t     s_phase_ongrid_reactive_power[2];//S �ಢ�����޹�����
    int16_t     s_phase_ongrid_power[2];        //S �ಢ�������ڹ���    

    uint16_t    t_phase_ongrid_volt;            //t �ಢ���ڵ�ѹ
    int16_t     t_phase_ongrid_curr;            //t �ಢ���ڵ���
    uint16_t    t_phase_ongrid_fre;             //t �ಢ����Ƶ��
    int16_t     t_phase_ongrid_active_power[2]; //t �ಢ�����й�����
    int16_t     t_phase_ongrid_reactive_power[2];//t �ಢ�����޹�����
    int16_t     t_phase_ongrid_power[2];        //t �ಢ�������ڹ���
    
    uint16_t    r_phase_oil_volt;               //R ���ͻ���ѹ 
    int16_t     r_phase_oil_curr;               //R ���ͻ�����
    uint16_t    r_phase_oil_freq;               //R ���ͻ�Ƶ��
    int16_t     r_phase_oil_active_power[2];       //R ���ͻ��й�����
    int16_t     r_phase_oil_reactive_power[2];     //R ���ͻ��޹�����
    int16_t     r_phase_oil_power[2];              //R ���ͻ����ڹ���

    uint16_t    s_phase_oil_volt;               //R ���ͻ���ѹ 
    int16_t     s_phase_oil_curr;               //R ���ͻ�����
    uint16_t    s_phase_oil_freq;               //R ���ͻ�Ƶ��
    int16_t     s_phase_oil_active_power[2];       //R ���ͻ��й�����
    int16_t     s_phase_oil_reactive_power[2];     //R ���ͻ��޹�����
    int16_t     s_phase_oil_power[2];              //R ���ͻ����ڹ���

    uint16_t    t_phase_oil_volt;               //R ���ͻ���ѹ 
    int16_t     t_phase_oil_curr;               //R ���ͻ�����
    uint16_t    t_phase_oil_freq;               //R ���ͻ�Ƶ��
    int16_t     t_phase_oil_active_power[2];       //R ���ͻ��й�����
    int16_t     t_phase_oil_reactive_power[2];     //R ���ͻ��޹�����
    int16_t     t_phase_oil_power[2];              //R ���ͻ����ڹ���

    uint16_t    r_phase_backup_volt             ;//R��BACKUP��ѹ
    int16_t     r_phase_backup_curr             ;//R��BACKUP����
    uint16_t    r_phase_backup_fre              ;//R��BACKUPƵ��    /*���� */
    int16_t     r_phase_backup_active_power[2]  ;//R��BACKUP���ù���
    int16_t     r_phase_backup_reactive_power[2];//R��BACKUP���ù���    
    int16_t     r_phase_backup_power[2]         ;//R��BACKUP���ڹ���

    uint16_t    s_phase_backup_volt             ;//s��BACKUP��ѹ
    int16_t     s_phase_backup_curr             ;//s��BACKUP����
    uint16_t    s_phase_backup_fre              ;//s��BACKUPƵ��    /*���� */
    int16_t     s_phase_backup_active_power[2]  ;//s��BACKUP���ù���
    int16_t     s_phase_backup_reactive_power[2];//s��BACKUP���ù���    
    int16_t     s_phase_backup_power[2]         ;//s��BACKUP���ڹ���

    uint16_t    t_phase_backup_volt             ;//t��BACKUP��ѹ
    int16_t     t_phase_backup_curr             ;//t��BACKUP����
    uint16_t    t_phase_backup_fre              ;//t��BACKUPƵ��    /*���� */
    int16_t     t_phase_backup_active_power[2]  ;//t��BACKUP���ù���
    int16_t     t_phase_backup_reactive_power[2];//t��BACKUP���ù���    
    int16_t     t_phase_backup_power[2]         ;//t��BACKUP���ڹ���

    int16_t     r_phase_load_volt;              //R ��Զ�˸��ص�ѹ  /*new */
    int16_t     r_phase_load_cur;               //R ��Զ�˸��ص���
    uint16_t    r_phase_load_fre;               //R ��Զ�˸���Ƶ��  /*new */
    int16_t     r_phase_load_active_power[2];      //R ��Զ�˸����й�����
    int16_t     r_phase_load_reactive_power[2];    //R ��Զ�˸����޹�����
    int16_t     r_phase_load_power[2];             //R ��Զ�˸������ڹ���    

    int16_t     s_phase_load_volt;              //S ��Զ�˸��ص�ѹ  /*new */
    int16_t     s_phase_load_cur;               //S ��Զ�˸��ص���
    uint16_t    s_phase_load_fre;               //S ��Զ�˸���Ƶ��  /*new */
    int16_t     s_phase_load_active_power[2];      //S ��Զ�˸����й�����
    int16_t     s_phase_load_reactive_power[2];    //S ��Զ�˸����޹�����
    int16_t     s_phase_load_power[2];             //S ��Զ�˸������ڹ���   

    int16_t     t_phase_load_volt;              //t ��Զ�˸��ص�ѹ  /*new */
    int16_t     t_phase_load_cur;               //t ��Զ�˸��ص���
    uint16_t    t_phase_load_fre;               //t ��Զ�˸���Ƶ��  /*new */
    int16_t     t_phase_load_active_power[2];      //t ��Զ�˸����й�����
    int16_t     t_phase_load_reactive_power[2];    //t ��Զ�˸����޹�����
    int16_t     t_phase_load_power[2];             //t ��Զ�˸������ڹ���  

    uint16_t    r_phase_ThirdParty_Volt;                //r ��AC��ϵ������������ѹ
    int16_t     r_phase_ThirdParty_cur;                 //r ��AC��ϵ��������������
    uint16_t    r_phase_ThirdParty_fre;                 //r ��AC��ϵ����������Ƶ��
    int16_t     r_phase_ThirdParty_active_power[2];      //r ��AC��ϵ�����������й�����
    int16_t     r_phase_ThirdParty_reactive_power[2];    //r ��AC��ϵ�����������޹�����
    int16_t     r_phase_ThirdParty_power[2];             //r ��AC��ϵ�������������ڹ���  

    uint16_t    s_phase_ThirdParty_Volt;                 //s ��AC��ϵ������������ѹ
    int16_t     s_phase_ThirdParty_cur;                  //s ��AC��ϵ��������������
    uint16_t    s_phase_ThirdParty_fre;                  //s ��AC��ϵ����������Ƶ��
    int16_t     s_phase_ThirdParty_active_power[2];      //s ��AC��ϵ�����������й�����
    int16_t     s_phase_ThirdParty_reactive_power[2];    //s ��AC��ϵ�����������޹�����
    int16_t     s_phase_ThirdParty_power[2];             //s ��AC��ϵ�������������ڹ���  

    uint16_t    t_phase_ThirdParty_Volt;                 //t ��AC��ϵ������������ѹ
    int16_t     t_phase_ThirdParty_cur;                  //t ��AC��ϵ��������������
    uint16_t    t_phase_ThirdParty_fre;                  //t ��AC��ϵ����������Ƶ��
    int16_t     t_phase_ThirdParty_active_power[2];      //t ��AC��ϵ�����������й�����
    int16_t     t_phase_ThirdParty_reactive_power[2];    //t ��AC��ϵ�����������޹�����
    int16_t     t_phase_ThirdParty_power[2];             //t ��AC��ϵ�������������ڹ���  
   
    uint16_t    r_phase_inverter_volt;          //R ������ѹ
    int16_t     r_phase_inverter_curr;          //R ��������
    uint16_t    r_phase_inverter_freq;          //R �����Ƶ��
    int16_t     r_phase_inverter_active_power[2];  //R ������й�����
    int16_t     r_phase_inverter_reactive_power[2];//R ������޹�����
    int16_t     r_phase_inverter_power[2];         //R ��������ڹ���

    uint16_t    s_phase_inverter_volt;          //s ������ѹ
    int16_t     s_phase_inverter_curr;          //s ��������
    uint16_t    s_phase_inverter_freq;          //s �����Ƶ��
    int16_t     s_phase_inverter_active_power[2];  //s ������й�����
    int16_t     s_phase_inverter_reactive_power[2];//s ������޹�����
    int16_t     s_phase_inverter_power[2];         //s ��������ڹ���

    uint16_t    t_phase_inverter_volt;          //t ������ѹ
    int16_t     t_phase_inverter_curr;          //t ��������
    uint16_t    t_phase_inverter_freq;          //t �����Ƶ��
    int16_t     t_phase_inverter_active_power[2];  //t ������й�����
    int16_t     t_phase_inverter_reactive_power[2];//t ������޹�����
    int16_t     t_phase_inverter_power[2];         //t ��������ڹ���

    uint16_t    inv_info_reserve[37];           //Ԥ��

    uint16_t    batt_volt;                      //��ز��ѹ0.1V
    uint16_t    batt_current;                   //��ز����0.1A
    int16_t     batt_power[2];                  //��ز๦��1W

    uint16_t    InvEV_volt;                     //EV���ѹ
    uint16_t    InvEV_cur;                      //EV�����
    int16_t     InvEV_power[2];                    //EV�๦��

    uint16_t    pv_ch1_input_volt               ;//PV1�����ѹ
    int16_t     pv_ch1_input_curr               ;//PV1�������
    uint16_t    pv_ch1_input_power[2]           ;//PV1���빦��   
    
    uint16_t    pv_ch2_input_volt               ;//PV2�����ѹ
    int16_t     pv_ch2_input_curr               ;//PV2�������
    uint16_t    pv_ch2_input_power[2]           ;//PV2���빦��

    uint16_t    pv_ch3_input_volt               ;//PV3�����ѹ
    int16_t     pv_ch3_input_curr               ;//PV3�������
    uint16_t    pv_ch3_input_power[2]           ;//PV3���빦��

    uint16_t    pv_ch4_input_volt               ;//PV4�����ѹ
    int16_t     pv_ch4_input_curr               ;//PV4�������
    uint16_t    pv_ch4_input_power[2]           ;//PV4���빦��

    uint16_t    pv_info_reserve[13];           //Ԥ��
    
    uint16_t    bus_voltage;  
    uint16_t    PhaseSequence;                  //�е�����

    uint16_t    Power_info_reserve[7];                  //Ԥ��

}_upload_Power_info_t;


typedef struct _upload_fault_info_stru_t
{
    /*������Ϣ��0x61C0-0x621F*/
    uint16_t    GridError;                          //0x61C0����������
    uint16_t    PVError[2];                         //0x61C1-0x61C2��Pv����
    uint16_t    BusError;                           //0x61C3��ĸ�߹���
    uint16_t    InvError;                           //0x61C4��������
    uint16_t    LoadError;                          //0x61C5�����ع���
    uint16_t    TempError;                          //0x61C6���¶ȹ���
    uint16_t    GenError;                           //0x61C7���ͻ�����
    uint16_t    OtherError;                         //0x61C8����������
    uint16_t    Error_reserve;                      //0x61C9������Ԥ��

    uint16_t    GridAlarm;                          //0x61CA����������
    uint16_t    PVAlarm[2];                         //0x61CB-0x61CC��Pv����
    uint16_t    BusAlarm;                           //0x61CD��ĸ�߾���
    uint16_t    InvAlarm;                           //0x61CE����侯��
    uint16_t    LoadAlarm;                          //0x61CF�����ؾ���
    uint16_t    TempAlarm;                          //0x61D0���¶Ⱦ���
    uint16_t    GenAlarm;                           //0x61D1���ͻ�����
    uint16_t    OtherAlarm;                         //0x61D2����������
    uint16_t    Alarm_reserve;                      //0x61D3������Ԥ��

    uint16_t    GridNotice;                          //0x61D4������֪ͨ
    uint16_t    PVNotice[2];                         //0x61D5-0x61D6��Pv֪ͨ
    uint16_t    BusNotice;                           //0x61D7��ĸ��֪ͨ
    uint16_t    InvNotice;                           //0x61D8�����֪ͨ
    uint16_t    LoadNotice;                          //0x61D9������֪ͨ
    uint16_t    TempNotice;                          //0x61DA���¶�֪ͨ
    uint16_t    GenNotice;                           //0x61DB���ͻ�֪ͨ
    uint16_t    OtherNotice;                         //0x61DC������֪ͨ
    uint16_t    Notice_reserve[3];                   //0x61DF��֪ͨԤ��

    uint16_t    fault_info_reserve[48];              //Ԥ��

}_upload_fault_info_t;



typedef struct _upload_info_stru_t
{
    _upload_basic_info_t        upload_basic_info;
    _upload_module_status_t     upload_module_status;
    _upload_Capacity_info_t     upload_Capacity_info;
    _upload_Power_info_t        upload_Power_info;
    _upload_fault_info_t        upload_fault_info;

} UploadInfo_t;


extern UploadInfo_t  gstUploadInfo;

void UploadInfoInit(void);
void SysInfoUpload(void);


#endif /* APP_SYS_INFO_H_ */
```

