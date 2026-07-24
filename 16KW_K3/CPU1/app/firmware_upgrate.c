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

#define UPGRATE_DATA_SIZE       2048//数据大小
#define UPGRATE_TIME_OUT        300000
#define ARC_COM_ADDR        0x02
#define COPRO_COM_ADDR      0x81
static uint16_t  _upgrate_data[2100];
static uint16_t pack_num_index = 0;
static  uint8_t resend_count = 0;
FirmwareUpgrateInfo_t gFirmwareUpgrateInfo_arc; //ARC固件升级结构体
FirmwareUpgrateInfo_t gFirmwareUpgrateInfo_copro; //协处理器固件升级结构体
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


void SystemFirmwareUpdate(void) //系统固件升级处理
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

    /*读取升级寄存器*/
    upgrade_reg = UpgradeRegRead();
    arc_being = (upgrade_reg >> 7) & 0x01;
    pcs_being = (upgrade_reg >> 11) & 0x01;
    copro_being = (upgrade_reg >> 6) & 0x01;
    sys_reset_en = (upgrade_reg >> 1) & 0x01;        //sys_update
    
    switch (SysUpdateState)
    {
        case EN_SYSTEM_FIRMWARE_UPDATE_IDLE:  //在通信中接受固件包，等待固件下发完成
        {    
            reset_timecount = 0;
            /*判断固件是否下发完成*/
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
        case EN_SYSTEM_FIRMWARE_UPDATE_START: //判断有哪些固件包升级
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
        case EN_SYSTEM_FIRMWARE_UPDATE_RUNNING: // 固件升级运行中
        {
            ArcUpgrateFirmwareProcess();
            CoproUpgrateFirmwareProcess();
            /*判断固件升级是否完成*/
            if(((!arc_being) || (gstFirmwareInfo.arc_firmware_info.upg_state == 4)) &&
               ((!copro_being) || (gstFirmwareInfo.copro_firmware_info.upg_state == 4)) &&
               ((!pcs_being) || (gstFirmwareInfo.pcs_firmware_info.upg_state == 2)))
            {
                SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_RESET;
                UpLoadStatus(1);
            }

            break;
        }
        case EN_SYSTEM_FIRMWARE_UPDATE_RESET:  //固件下发完成
        {    
            if (sys_reset_en == 1) //监控是否下发重启指令
            {
                if (((!arc_being) || SetArcSysReset() == true) && 
                    ((!(copro_being)) || SetCoproSysReset() == true))
                {
                    SysUpdateState = EN_SYSTEM_FIRMWARE_UPDATE_END;
                    SysRestart(); //复位DSP
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
        case EN_SYSTEM_FIRMWARE_UPDATE_END:   //固件升级结束，协处理器等复位成功，判断升级结果
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
        case EN_SYSTEM_FIRMWARE_UPDATE_ERROR: //固件升级出错
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

    /*读取升级寄存器*/
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

    /*读取升级寄存器*/
    upgrade_reg = UpgradeRegRead();
    copro_being = (upgrade_reg >> 6) & 0x01;        //TODO: copro_being，待确认协议

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
    /* 0：idle 1：所有固件下发成功           2：有固件下发失败          3：无需升级ARC（DSP下发成功） 
            4：无需升级ARC（DSP下发失败） 5：无需DSP、ARC成功 6：无需DSP、ARC失败  7:ARC转发成功  8：ARC转发失败 */
//    uint16_t upgrade_state = 0;
    uint16_t upgrade_reg = 0; 
//    uint16_t pcs_being = 0;
    uint16_t arc_being = 0;

    /*读取升级寄存器*/
    upgrade_reg = UpgradeRegRead();

    /*判断固件是否下发完成*/
    if(!((upgrade_reg >> 12) & 0x01))
    {
        return false;
    }

    /*系统复位判断*/
    // if(((upgrade_reg >> 1) & 0x01))
    // {
    //     /*ARC收到复位 */
    //     if(SetArcSysReset() == true)
    //     {
    //         /*复位自身*/
    //         //TODO
    //         //SysRestart();
    //     }        
    // }
    
    //pcs_being = (upgrade_reg >> 11) & 0x01;
    arc_being = (upgrade_reg >> 7) & 0x01;

    if (arc_being)
    {
        if (gstFirmwareInfo.arc_firmware_info.upg_state == 2) //arc转发升级
        {
            return true;
        }
        else 
        {
            return false;
        }
        
    }
    return false;
    

    /*pcs和arc固件都存在*/
    // if(pcs_being && arc_being)
    // {
    //     /*pcs和arc固件都下发完成*/
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
    //     /*可以升级ARC*/
    //     return true;
    // }

    // if((upgrade_state == 7) || (upgrade_state == 8))
    // {
    //     /* ARC 转发结束 */
    //     return false;
    // }

    // /*不升级ARC*/
    // return false;
}

static bool _CoproUpgradeCheck(void)
{
    /* 0：idle 1：所有固件下发成功           2：有固件下发失败          3：无需升级ARC（DSP下发成功） 
            4：无需升级ARC（DSP下发失败） 5：无需DSP、ARC成功 6：无需DSP、ARC失败  7:ARC转发成功  8：ARC转发失败 */
//    uint16_t upgrade_state = 0;
    uint16_t upgrade_reg = 0; 
//    uint16_t pcs_being = 0;
    uint16_t copro_being = 0;

    /*读取升级寄存器*/
    upgrade_reg = UpgradeRegRead();

    /*判断固件是否下发完成*/
    if(!((upgrade_reg >> 12) & 0x01))
    {
        return false;
    }

    /*系统复位判断*/
    // if(((upgrade_reg >> 1) & 0x01))
    // {
    //     /*Copro收到复位 */
    //     if(SetCoproSysReset() == true)
    //     {
    //         /*复位自身*/
    //         //TODO
    //         //SysRestart();
    //         ClearUpdateFlag();
    //     }        
    // }
    
    // pcs_being = (upgrade_reg >> 11) & 0x01;
    copro_being = (upgrade_reg >> 6) & 0x01;

    if (copro_being)
    {
        if (gstFirmwareInfo.copro_firmware_info.upg_state == 2) //copro转发升级
        {
            return true;
        }
        else 
        {
            return false;
        }
        
    }
    return false;

    // /*pcs和copro固件都存在*/
    // if(pcs_being && copro_being)
    // {
    //     /*pcs和copro固件都下发完成*/
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
    //     /*可以升级Copro*/
    //     return true;
    // }

    // if((upgrade_state == 7) || (upgrade_state == 8))
    // {
    //     /* Copro 转发结束 */
    //     return false;
    // }

    /*不升级Copro*/
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
    _upgrate_data[5] = 0x05;//数据域长度
    _upgrate_data[6] = 0x00;

    _upgrate_data[7] = gFirmwareUpgrateInfo_arc.ack_firmware_len & 0xFF;//文件总长度
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
    _upgrate_data[5] = 0x05;//数据域长度
    _upgrate_data[6] = 0x00;

    _upgrate_data[7] = (gFirmwareUpgrateInfo_copro.ack_firmware_len) & 0xFF;//文件总长度
    _upgrate_data[8] = (gFirmwareUpgrateInfo_copro.ack_firmware_len >> 8) & 0xFF;
    _upgrate_data[9] = (gFirmwareUpgrateInfo_copro.ack_firmware_len >> 16) & 0xFF;
    _upgrate_data[10] = (gFirmwareUpgrateInfo_copro.ack_firmware_len >> 24) & 0xFF;   //todo: 修改为 copro 固件长度
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
    _upgrate_data[5] = (size + space_num) & 0xFF;//数据域长度
    _upgrate_data[6] = ((size + space_num) >> 8) & 0xFF;
    _upgrate_data[7] = pack_num_index & 0xFF;//包的序号
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
    _upgrate_data[5] = (size + space_num) & 0xFF;//数据域长度
    _upgrate_data[6] = ((size + space_num) >> 8) & 0xFF;
    _upgrate_data[7] = pack_num_index & 0xFF;//包的序号
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

static EN_BMS_FIRMWARE_UPGRATE_STATUS _ArsFirmwareAnalysis()//函数的返回类型是枚举变量
{
    uint16_t temp[256];    
    uint32_t offset = 0;//偏移量
//    uint32_t i = 0;
//    uint32_t pack_num = 0;
//    uint32_t last_data = 0;
//    uint32_t check_sum = 0;

    offset = 0;
    //从flash中读取数据

    SpiFlashReadBytes(ARC_FIRMWARE_ADDR, temp, 256);
    if(temp[0] != garrArcFirmwareType[0] || temp[1] != garrArcFirmwareType[1] || temp[2] != garrArcFirmwareType[2])//    /*读取第一个文件头*/开始判数据头是否为0X0C
    {
        return EN_FIRMWARE_UPGRATE_STATUS_ERROR;//固件升级错误
    }
    if(temp[3] == garrArcFirmwareType[3])
    {
        gFirmwareUpgrateInfo_arc.ack_offset = offset;
        gFirmwareUpgrateInfo_arc.ack_firmware_len = ((uint32_t)temp[7] | ((uint32_t)temp[8] << 8) | ((uint32_t)temp[9] << 16) | ((uint32_t)temp[10] << 24));///* 固件长度 */
        gFirmwareUpgrateInfo_arc.ack_checksum = ((uint32_t)temp[252] | ((uint32_t)temp[253] << 8) | ((uint32_t)temp[254] << 16) | ((uint32_t)temp[255] << 24));//按位检查 /* 固件校验和 数据域校验和 */
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
        return EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE;//升级准备
    }
    else
    {
        return EN_FIRMWARE_UPGRATE_STATUS_ERROR;//固件头不对则直接判定为错误
    }
}


static EN_BMS_FIRMWARE_UPGRATE_STATUS _CoproFirmwareAnalysis()//函数的返回类型是枚举变量
{
    uint16_t temp[256];    
    uint32_t offset = 0;//偏移量

    offset = 0;
    //从flash中读取数据

    SpiFlashReadBytes(COPRO_FIRMWARE_ADDR, temp, 256);
    if(temp[0] != garrCoproFirmwareType[0] || temp[1] != garrCoproFirmwareType[1] || temp[2] != garrCoproFirmwareType[2])//    /*读取第一个文件头*/开始判数据头是否为0X0C
    {
        return EN_FIRMWARE_UPGRATE_STATUS_ERROR;//固件升级错误
    }
    if(temp[3] == garrCoproFirmwareType[3])
    {
        gFirmwareUpgrateInfo_copro.ack_offset = offset;
        gFirmwareUpgrateInfo_copro.ack_firmware_len = ((uint32_t)temp[7] | ((uint32_t)temp[8] << 8) | ((uint32_t)temp[9] << 16) | ((uint32_t)temp[10] << 24));///* 固件长度 */
        gFirmwareUpgrateInfo_copro.ack_checksum = ((uint32_t)temp[252] | ((uint32_t)temp[253] << 8) | ((uint32_t)temp[254] << 16) | ((uint32_t)temp[255] << 24));//按位检查 /* 固件校验和 数据域校验和 */
        gFirmwareUpgrateInfo_copro.ack_existed = 1;
        gFirmwareUpgrateInfo_copro.ack_seek = 0;
        return EN_FIRMWARE_UPGRATE_STATUS_SEND_PREPARE;//升级准备
    }
    else
    {
        return EN_FIRMWARE_UPGRATE_STATUS_ERROR;//固件头不对则直接判定为错误
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
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_START;//开始升级
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_START:
        {
            arc_upgrate_status = _ArsFirmwareAnalysis();//升级包固件校验
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
            _ArcUpgraderadeSendStart();//开始升级
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
                /*第一包发256包头*/
                size_per_transfer = 256;
            }
            else if(gFirmwareUpgrateInfo_arc.ack_firmware_len - gFirmwareUpgrateInfo_arc.ack_seek < UPGRATE_DATA_SIZE)
            {
                size_per_transfer = gFirmwareUpgrateInfo_arc.ack_firmware_len - gFirmwareUpgrateInfo_arc.ack_seek;//最后一包长度
            }
            else
            {
                size_per_transfer = UPGRATE_DATA_SIZE;//每包固定2k
            }
            pack_size = size_per_transfer;
            _ArsUpgradeSendBusy(size_per_transfer, gFirmwareUpgrateInfo_arc.ack_offset + gFirmwareUpgrateInfo_arc.ack_seek);//发送数据
            gFirmwareUpgrateInfo_arc.ack_seek += size_per_transfer;//加入偏移地址
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
            /* 错误 */
            if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX0].cmd_upgrate_busy == 1)
            {
                ResetWriteResAckState(EN_COMSERVICE_COM_INDEX0, CMD_UPGRATE_BUSY);
                resend_count ++;
                gFirmwareUpgrateInfo_arc.ack_seek -= pack_size;//加入偏移地址
                pack_num_index = pack_num_index - 1;
                if(resend_count > 3)
                {
                    arc_upgrate_status =  EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                }
            }
            /* 接收正确 */
            else if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX0].cmd_upgrate_busy == 0)
            {
                resend_count = 0;
                ResetWriteResAckState(EN_COMSERVICE_COM_INDEX0, CMD_UPGRATE_BUSY);
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY;
                timeout = TimGetSystime();
            }

            if(gFirmwareUpgrateInfo_arc.ack_seek >= gFirmwareUpgrateInfo_arc.ack_firmware_len)
            {
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_END;//结束
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
            /* 最低不能小于150ms，否则会升级失败 */
            if((TimGetSystime() - timeout) < 300)
            {
                break;
            }
            _ArsUpgradeSendEnd();
            arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK;//结束检查
            timeout = TimGetSystime();
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK:
        {
            if((TimGetSystime() - timeout) > UPGRATE_TIME_OUT)
            {
    //            printf("upgrade error\r\n");
                arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                /*告诉监控卡升级失败*/
                UpLoadStatus(0);
                gstFirmwareInfo.arc_firmware_info.upg_state = 5;    //arc转发失败
            }
            else
            {
                if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX0].cmd_upgrate_end == 0)
                {
                    ResetWriteResAckState(EN_COMSERVICE_COM_INDEX0, CMD_UPGRATE_END);
                    arc_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_OK;
                    /*告诉监控卡升级成功*/
                    UpLoadStatus(1);
                    gstFirmwareInfo.arc_firmware_info.upg_state = 4;    //arc转发成功
         //           printf("upgrade ok\r\n");
                }                
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_ERROR:
        {
            UpLoadStatus(0);
            gstFirmwareInfo.arc_firmware_info.upg_state = 5;    //arc转发失败
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
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_START;//开始升级
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_START:
        {
            copro_upgrate_status = _CoproFirmwareAnalysis();//升级包固件校验
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
            _CoproUpgraderadeSendStart();//开始升级
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
            //     /*第一包发256包头*/
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
                pack_size = gFirmwareUpgrateInfo_copro.ack_firmware_len - gFirmwareUpgrateInfo_copro.ack_seek;//最后一包长度
            }
            else
            {
                pack_size = size_per_transfer;//每包固定2k
            }
            _CoproUpgradeSendBusy(pack_size, gFirmwareUpgrateInfo_copro.ack_offset + gFirmwareUpgrateInfo_copro.ack_seek);//发送数据
            gFirmwareUpgrateInfo_copro.ack_seek += pack_size;//加入偏移地址
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
            /* 错误 */
            if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX2].cmd_upgrate_busy == 1)
            {
                ResetWriteResAckState(EN_COMSERVICE_COM_INDEX2, CMD_UPGRATE_BUSY);
                resend_count ++;
                gFirmwareUpgrateInfo_copro.ack_seek -= pack_size;//加入偏移地址
                pack_num_index = pack_num_index - 1;
                if(resend_count > 3)
                {
                    copro_upgrate_status =  EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                }
            }
            /* 接收正确 */
            else if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX2].cmd_upgrate_busy == 0)
            {
                resend_count = 0;
                ResetWriteResAckState(EN_COMSERVICE_COM_INDEX2, CMD_UPGRATE_BUSY);
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_DATA_BUSY;
                timeout = TimGetSystime();
            }

            if(gFirmwareUpgrateInfo_copro.ack_seek >= gFirmwareUpgrateInfo_copro.ack_firmware_len)
            {
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_SEND_END;//结束
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
            /* 最低不能小于150ms，否则会升级失败 */
            if((TimGetSystime() - timeout) < 300)
            {
                break;
            }
            _CoproUpgradeSendEnd();
            copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK;//结束检查
            timeout = TimGetSystime();
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_END_ACK:
        {
            if((TimGetSystime() - timeout) > UPGRATE_TIME_OUT)
            {
                //printf("upgrade error\r\n");
                copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                /*告诉监控卡升级失败*/
                UpLoadStatus(0);
                gstFirmwareInfo.copro_firmware_info.upg_state = 5;    //copro转发失败
            }
            else
            {
                if(garrWriteResAckState[EN_COMSERVICE_COM_INDEX2].cmd_upgrate_end == 0)
                {
                    ResetWriteResAckState(EN_COMSERVICE_COM_INDEX2, CMD_UPGRATE_END);
                    copro_upgrate_status = EN_FIRMWARE_UPGRATE_STATUS_OK;
                    /*告诉监控卡升级成功*/
                    UpLoadStatus(1);
                    gstFirmwareInfo.copro_firmware_info.upg_state = 4;    //copro转发成功
                    //printf("upgrade ok\r\n") ;
                }                
            }
            break;
        }

        case EN_FIRMWARE_UPGRATE_STATUS_ERROR:
        {
            UpLoadStatus(0);
            gstFirmwareInfo.copro_firmware_info.upg_state = 5;    //copro转发失败
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

    if((p_monitor_frame->Length == 256) && (p_monitor_frame->Addr == 0x24600000))//第一包的包头DSP
    {
        memset((uint16_t*)&gstFirmwareInfo.pcs_firmware_info, 0x00, (int)sizeof(FirmwareRecvInfoS_t));
        p_firmware_info = &gstFirmwareInfo.pcs_firmware_info;
        firmware_start_addr = DSP_FIRMWARE_ADDR;
        firmware_check_addr = DSP_FIRMWARE_FRAME_ADDR;
        /*存储升级开始标志*/
        p_firmware_info->upg_state = 1;
        first_pack_flag = 1;
//        printf("gstFirmwareInfo.pcs_firmware_info.finish reg:0x%04X\r\n", gstFirmwareInfo.pcs_firmware_info.upg_state);
//        printf("gstFirmwareInfo.arc_firmware_info.finish reg:0x%04X\r\n", gstFirmwareInfo.arc_firmware_info.upg_state);
//        printf("checksum:0X%04X%04X,  checksumtemp:0X%04X%04X\r\n", (uint16_t)(p_firmware_info->checksum >> 16), (uint16_t)p_firmware_info->checksum, (uint16_t)(p_firmware_info->checksumtemp >> 16), (uint16_t)p_firmware_info->checksumtemp);
//        printf("dsp 1 OK!\r\n");
    }
    else if((p_monitor_frame->Length == 256) && (p_monitor_frame->Addr == 0x24700000))//第一包的包头ARC
    {
        memset((uint16_t*)&gstFirmwareInfo.arc_firmware_info, 0x00, (int)sizeof(FirmwareRecvInfoS_t));
        p_firmware_info = &gstFirmwareInfo.arc_firmware_info;
        firmware_start_addr = ARC_FIRMWARE_ADDR;
        firmware_check_addr = ARC_FIRMWARE_FRAME_ADDR;
        /*存储升级开始标志*/
        p_firmware_info->upg_state = 1;
        first_pack_flag = 1;
//        printf("gstFirmwareInfo.pcs_firmware_info.finish reg:0x%04X\r\n", gstFirmwareInfo.pcs_firmware_info.upg_state);
//        printf("gstFirmwareInfo.arc_firmware_info.finish reg:0x%04X\r\n", gstFirmwareInfo.arc_firmware_info.upg_state);
//        printf("checksum:0X%04X%04X,  checksumtemp:0X%04X%04X\r\n", (uint16_t)(p_firmware_info->checksum >> 16), (uint16_t)p_firmware_info->checksum, (uint16_t)(p_firmware_info->checksumtemp >> 16), (uint16_t)p_firmware_info->checksumtemp);
//        printf("arc 1 OK!\r\n");
    }
    else if((p_monitor_frame->Length == 256) && (p_monitor_frame->Addr == 0x24800000))//第一包的包头copro
    {
        memset((uint16_t*)&gstFirmwareInfo.copro_firmware_info, 0x00, (int)sizeof(FirmwareRecvInfoS_t));
        p_firmware_info = &gstFirmwareInfo.copro_firmware_info;
        firmware_start_addr = COPRO_FIRMWARE_ADDR;
        firmware_check_addr = COPRO_FIRMWARE_FRAME_ADDR;
        /*存储升级开始标志*/
        p_firmware_info->upg_state = 1;
        first_pack_flag = 1;
    }
    

    if(p_firmware_info == NULL)
    {
        // 此处表示无效指针
        return false;
    }

    start_address = (p_monitor_frame->Addr & 0x000FFFFF) + firmware_start_addr;
    SpiFlashWriteBytesWithErase(start_address, &p_data[10], p_monitor_frame->Length);
    if(first_pack_flag)
    {
        //解析监控更新数据的固件包类型
        memcpy((uint16_t*)&p_firmware_info->firmware_type, &p_data[10], 4);
        //解析监控更新数据的版本号
        memcpy((uint16_t*)&p_firmware_info->firmware_version, &p_data[14], 3);
        //解析监控更新数据的长度
        p_firmware_info->firmware_len = (uint32_t)p_data[20] << 24 | (uint32_t)p_data[19] << 16 | (uint32_t)p_data[18] << 8 | (uint32_t)p_data[17];
        p_firmware_info->firmware_len_temp = p_monitor_frame->Length;
        //解析监控更新数据的校验码
        p_firmware_info->checksum = (uint32_t)p_data[265] << 24 | (uint32_t)p_data[264] << 16 | (uint32_t)p_data[263] << 8 | (uint32_t)p_data[262];
//        printf("arc first OK!\r\n");
    }
    else
    {
        /*读出flash的数据进行校验和计算*/
        SpiFlashReadBytes(start_address, _upgrate_data, p_monitor_frame->Length);
//        printf("write addr:0x%04X%04X\r\n", (uint16_t)(p_monitor_frame->Addr >> 16), (uint16_t)p_monitor_frame->Addr);
        p_firmware_info->checksumtemp += CheckSumFun((uint8_t*)&_upgrate_data, p_monitor_frame->Length, 1);
//        printf("checksumtemp:0x%04X%04X\r\n", (uint16_t)(p_firmware_info->checksumtemp >> 16), (uint16_t)p_firmware_info->checksumtemp);

        //升级包长度加和
        p_firmware_info->firmware_len_temp += p_monitor_frame->Length;
        //传输完成置位
        if(p_firmware_info->firmware_len_temp == p_firmware_info->firmware_len)
        {
            /*存储的固件包和校验是否正确*/
            if(p_firmware_info->checksum == p_firmware_info->checksumtemp)
            {
//                printf("checksum:0X%04X%04X,  checksumtemp:0X%04X%04X\r\n", (uint16_t)(p_firmware_info->checksum >> 16), (uint16_t)p_firmware_info->checksum, (uint16_t)(p_firmware_info->checksumtemp >> 16), (uint16_t)p_firmware_info->checksumtemp);
                /*程序升级成功*/
                p_firmware_info->upg_state = 2;

                /*存储固件完成且校验OK，借用_garrReceiveDataBuff数组空间往flash写入固件更新标志*/
                memset(p_data, 0xA0, 256);
                SpiFlashWriteBytesWithErase(firmware_check_addr, p_data, 256);
                return true;
            }
            else
            {
//                printf("checksum:0X%04X%04X,  checksumtemp:0X%04X%04X\r\n", (uint16_t)(p_firmware_info->checksum >> 16), (uint16_t)p_firmware_info->checksum, (uint16_t)(p_firmware_info->checksumtemp >> 16), (uint16_t)p_firmware_info->checksumtemp);
                /*程序升级失败*/
                p_firmware_info->upg_state = 3;
            }

            //TODO 调试使用，修改hold寄存器，告诉监控固件升级成功了
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



