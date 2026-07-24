#include "firmware_upgrate.h"
#include "dev_modbus_h.h"
#include "slave_ledctl.h"

#define UPGRATE_TIME_RECV_OUT               60000

#define  MODE_ID        0XC2FF;

volatile const SoftwareVer_t gstSoftwareVer = {{1, 0, 0}}; //LOW(6BIT) MID(6BIT) HI(4BIT)   {{6, 0, 1}}; 1-0-6
volatile const uint8_t arrHwVer[2] = {1, 0}; //A1      //{0, 2}; C0
SoftwareExternVer_t gstSoftExternVer;

FirmwareUpgrateInfo_t gstFirmwareUpgrateInfo;
static  uint8_t _garrUpgrateData[2148];//
static  uint32_t _gUpgrateStartTimeOut = 0;
static uint8_t _RcvBufv[2100];
uint8_t gUpDataCheckSumErr = 0;
static uint32_t upgrade_timeout = 0;

void SysInfoInit(void)
{
    (void)gstSoftwareVer;
    (void)arrHwVer;
    memset(&gstSoftExternVer, 0, sizeof(SoftwareExternVer_t));

    
    memcpy((uint8_t *)gstMainCopParaData.base_info.sw_ver,     (uint8_t*)&gstSoftwareVer,sizeof(SoftwareVer_t));
    memcpy((uint8_t *)&gstMainCopParaData.base_info.hw_ver,    (uint8_t*)&arrHwVer, sizeof(arrHwVer));
    memcpy((uint8_t *)gstMainCopParaData.base_info.sw_ver_ext, (uint8_t*)&gstSoftExternVer, sizeof(SoftwareExternVer_t));

    gstMainCopParaData.base_info.model_id = MODE_ID;
    sprintf((char *)gstMainCopParaData.base_info.sn, "25C250123456789"); ///
}

static uint16_t _CheckSum(uint8_t *p, uint16_t size)
{
    uint16_t i;
    uint16_t ret = 0;
    for(i = 0; i < size; i++)
    {
        ret += p[i];
    }
    return ret;
}


void _AckUpdataCmd(uint16_t cmd, uint8_t data)
{
    //低端
    uint8_t len = 0;
    uint16_t checksum = 0;
    _garrUpgrateData[len++] = 0xA5;
    _garrUpgrateData[len++] = 0x80;
    _garrUpgrateData[len++] = 0x00;

    _garrUpgrateData[len++] = (uint8_t)cmd;
    _garrUpgrateData[len++] = cmd >> 8;

    _garrUpgrateData[len++] = data;

    checksum = _CheckSum(_garrUpgrateData, len);
    _garrUpgrateData[len++] = (uint8_t)checksum;
    _garrUpgrateData[len++] = checksum >> 8;

//调用发送
    ModbusRTUSendWithoutCheckSum(_garrUpgrateData, len);
//    DbgModbusRTUSendWithoutCheckSum(_garrUpgrateData, len);
}

/*返回值作为errorcode*/
static uint16_t _UpgratePrepare()
{
    uint32_t Fimrware_addr = 0;
    uint32_t time_out = 0;
    gstFirmwareUpgrateInfo.upgrate_end_flag = 0;

    if(gstFirmwareUpgrateInfo.rev_start_flag == 0)
    {
        gstFirmwareUpgrateInfo.rev_start_flag = 1;
        gstFirmwareUpgrateInfo.upgrad_err_code = 0; 
        gstFirmwareUpgrateInfo.upgrading_status = 1;
        gstFirmwareUpgrateInfo.write_addr = FIRMWARE_EXTERNAL_FLASH_ADDR;
        _gUpgrateStartTimeOut = tim_get_systime();
        gstFirmwareUpgrateInfo.file_size = 0;
        gstFirmwareUpgrateInfo.file_check_sum = 0;
        gstFirmwareUpgrateInfo.package_count = 0;
//发送升级反馈
        gstFirmwareUpgrateInfo.updata_get_persent = 0;
        gstFirmwareUpgrateInfo.updata_send_persent = 0;
        return 0;
    }
    else
    {
        return 1;//已经有其他上位机在操作升级了
    }
}



static uint16_t _UpgrateDataDeal(uint8_t *pSrc, uint16_t rev_len)
{
    uint16_t i;
    if(gstFirmwareUpgrateInfo.rev_start_flag == 0)
    {
        return 1;
    }
    /*数据保存至FLASH*/
    if(gstFirmwareUpgrateInfo.write_addr + rev_len >= (FIRMWARE_EXTERNAL_FLASH_ADDR + 256 * 1024)) //0x40000
    {
        return 1;
    }
    /*从第一包获取文件大小*/
    if(gstFirmwareUpgrateInfo.package_count == 0)
    {
        if(pSrc[0] != 0x0C || pSrc[1] != 0x0C || pSrc[2] != 0x0C || pSrc[3] != 0x80)
        {
            return 1;
        }
        gstFirmwareUpgrateInfo.file_check_sum = *(uint32_t*)&pSrc[256 - 4];
        gstFirmwareUpgrateInfo.file_size_read = pSrc[7] | (pSrc[8] << 8) | (pSrc[9] << 16) | (pSrc[10] << 24);
    }
    
    InternalFlashWrite(gstFirmwareUpgrateInfo.write_addr, (uint32_t *)&pSrc[0], rev_len);
    InternalFlashRead(gstFirmwareUpgrateInfo.write_addr, _RcvBufv, rev_len);
    
    if(gstFirmwareUpgrateInfo.package_count == 0)
    {
        
    }
    else
    {
        //如果连续发两包相同的流水号，则减去旧的和校验，加上新的和校验
        if(gstFirmwareUpgrateInfo.old_pack_id == gstFirmwareUpgrateInfo.pack_id)
        {
            gstFirmwareUpgrateInfo.file_all_calc_check_sum -= gstFirmwareUpgrateInfo.file_one_pack_check_sum;
        }
        /* 计算 checksum */
        gstFirmwareUpgrateInfo.file_one_pack_check_sum = _CheckSum(_RcvBufv, rev_len);
        gstFirmwareUpgrateInfo.file_all_calc_check_sum += gstFirmwareUpgrateInfo.file_one_pack_check_sum;
        
    }
    
    gstFirmwareUpgrateInfo.write_addr += rev_len;
    gstFirmwareUpgrateInfo.old_pack_id = gstFirmwareUpgrateInfo.pack_id;
    _gUpgrateStartTimeOut = tim_get_systime();
    /*计算数据量*/
    gstFirmwareUpgrateInfo.file_size += rev_len;

    // GetPersent(gstFirmwareUpgrateInfo.file_size);
    gstFirmwareUpgrateInfo.package_count++;

    return 0;
}

static uint16_t _UpgrateRevComplete()
{
    if(gstFirmwareUpgrateInfo.rev_start_flag == 0)
    {
        return 1;
    }
    else
    {
        gstFirmwareUpgrateInfo.rev_end_flag = 1;
        return 0;
    }
}

static uint16_t _UpgrateFirmwareCheck()
{
    uint8_t arr_tmp[256];
    uint16_t i = 0;
    uint32_t check_sum_calc = 0;
    uint32_t start_addr = FIRMWARE_EXTERNAL_FLASH_ADDR + 256;
    uint32_t size = gstFirmwareUpgrateInfo.file_size_read - 0x100;

    memset(arr_tmp, 0, sizeof(arr_tmp));
    
    while(size > 256)
    {
        InternalFlashRead(start_addr, arr_tmp, 256);
        size -= 256;
        start_addr += 256;
        for(i = 0; i < 256; i++)
        {
            check_sum_calc += arr_tmp[i];
        }
    }
    InternalFlashRead(start_addr, arr_tmp, size);
    for(i = 0; i < size; i++)
    {
        check_sum_calc += arr_tmp[i];
    }

    if(check_sum_calc != gstFirmwareUpgrateInfo.file_check_sum)
    {
        return 1;
    }
    return 0;
}



static uint8_t _ReqCmdAnalysis(uint8_t *p, uint16_t size)
{
    //监控卡下发命令给协处理器
    static uint32_t time_out = 0;
    uint16_t cmd = 0;
    uint16_t data_field_len = 0;
    uint8_t *p_buf = p;
    uint8_t updata_staus = 0;
    cmd = *(uint16_t *)&p_buf[0];
    data_field_len = *(uint32_t *)&p_buf[2];  
     

    switch(cmd)
    {
        case CMD_UPGRATE_PREPARE:
        {
            updata_staus = _UpgratePrepare();

            if(data_field_len != 0)
            {
                gstFirmwareUpgrateInfo.old_pack_id = 0xFFFF;
                gstFirmwareUpgrateInfo.updata_get_total_size = (p_buf[7] << 24) + (p_buf[6] << 16) + (p_buf[5] << 8) + p_buf[4];  //升级包大小
                gstFirmwareUpgrateInfo.space_data_size = p_buf[8];
                if(gstFirmwareUpgrateInfo.space_data_size > 10)
                {
                    gstFirmwareUpgrateInfo.space_data_size = 10; //md5
                }
            }
            else
            {
                gstFirmwareUpgrateInfo.space_data_size = 0;
            }
            _AckUpdataCmd(CMD_UPGRATE_PREPARE, updata_staus);
            break;
        } 
        case CMD_UPGRATE_DATA:
        {

            gstFirmwareUpgrateInfo.pack_id = *(uint16_t *)&p_buf[4];
            if(1 == gUpDataCheckSumErr)
            {
                updata_staus = 0x02;
            }
            else
            {
                //第一包数据
                if(gstFirmwareUpgrateInfo.old_pack_id != gstFirmwareUpgrateInfo.pack_id)
                {
                    updata_staus = _UpgrateDataDeal(&p_buf[4 + gstFirmwareUpgrateInfo.space_data_size], data_field_len - gstFirmwareUpgrateInfo.space_data_size);
                }
            }
            _AckUpdataCmd(CMD_UPGRATE_DATA, updata_staus);
            time_out = tim_get_systime();
            break;
        }
        case CMD_UPGRATE_COMPLETE:
        {
            updata_staus = _UpgrateFirmwareCheck();
            if(!updata_staus)
            {
                //和校验通过
                _UpgrateRevComplete();
            }
            else
            {
                gstMainCopParaData.ctrl_info.upgrade_status.bits.upgrade_res1 = 1; 
                gstMainCopParaData.ctrl_info.upgrade_status.bits.upgrade_res2 = 1;
            }
            upgrade_timeout = tim_get_systime();
//            if((uint32_t)tim_get_systime() - time_out > 500)
//            {
//                _AckUpdataCmd(CMD_UPGRATE_COMPLETE, updata_staus);
//            }
            break;
        }
        default:
            break;
    }

    return true;
}





static uint8_t _RevSlaveUpdateAnalysis(uint8_t *p, uint16_t size)
{
    uint16_t i = 0;
    uint16_t check_sum = 0;
    uint8_t *p_buf = p;
    uint16_t len = size;

    if(p_buf[0] != 0x5A && p_buf[0] != 0xA5)
    {
        return false;   
    }

    if(len < 7)
    {
        return false;
    }

    check_sum = _CheckSum(p_buf,(len-2));
    if(check_sum != *(uint16_t *)&p_buf[len-sizeof(uint16_t)])
    {
        gUpDataCheckSumErr = 1;
        return false;
    }
    else
    {
        gUpDataCheckSumErr = 0;
    }

    if(p_buf[0] == 0x5A)
    {
        //请求包
        _ReqCmdAnalysis(&p_buf[3], len - 5);
    }
    else if(p_buf[0] == 0xA5)
    {
        //响应包
//        _AckCmdAnalysis(&p_buf[3], len - 5);        
    }
    return 0;
}


static void _FirmwareFlagUpdate(void)
{
    uint8_t arr_temp[256];

    memset(arr_temp, 0x0A, 256);
    InternalFlashWrite(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, (uint32_t *)arr_temp, 256);
    InternalFlashRead(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, arr_temp, 256);
}


static void _SysRestart(void)
{
    __set_PRIMASK(1);//关闭总中断
    __set_FAULTMASK(1);
    __NVIC_SystemReset();
}





void FirmwareUpgrateInit(void)
{
    RegisterSlaveUpdateFunPoint(_RevSlaveUpdateAnalysis);
}


void UpgrateFirmwareProcess(void)
{
    static uint32_t timeout = 0;
    static uint8_t _geStatus = 0;
    static uint16_t _gupgrade_reset_flag = 0;
    


    switch(_geStatus)
    {
        case 0: //idle
        {
            if(gstFirmwareUpgrateInfo.rev_start_flag == 1)
            {
                if((uint32_t)(tim_get_systime() - _gUpgrateStartTimeOut) > UPGRATE_TIME_RECV_OUT)
                {
                    gstFirmwareUpgrateInfo.rev_start_flag = 0;
                    gstFirmwareUpgrateInfo.rev_end_flag = 0;
                    gstFirmwareUpgrateInfo.upgrading_status = 0;
                }
            }
            if(gstFirmwareUpgrateInfo.rev_end_flag == 1) //从上位机接收完成
            {
                if((uint32_t)tim_get_systime() -  upgrade_timeout > 500)
                {
                    gstFirmwareUpgrateInfo.rev_end_flag = 0;
                    timeout = tim_get_systime();
                    _AckUpdataCmd(CMD_UPGRATE_COMPLETE, 0);
                    gstMainCopParaData.ctrl_info.upgrade_status.bits.upgrade_res1 = 1; 
                    gstMainCopParaData.ctrl_info.upgrade_status.bits.upgrade_res2 = 0;
                    _geStatus = 1;
                }      
            }
            break;
        }
        case 1: //升级OK 置0A标志
        {
            _FirmwareFlagUpdate();
            timeout = tim_get_systime();
            _geStatus = 2;
            break;
        }
        case 2://等待重启
        {
            //重启标志
            static uint8_t reset_flag = 0;
            if(gstMainCopParaData.ctrl_info.upgrade_status.value == 0)
            {
                reset_flag = 0;
            }

            if(reset_flag)
            {
                gstMainCopParaData.ctrl_info.upgrade_status.bits.restate_state = 1;
            }

            //复位标志已置起
            if((gstMainCopParaData.ctrl_info.upgrade_status.bits.restate_state != 0) && (reset_flag == 0))
            {
                //协处理器重启
                _gupgrade_reset_flag = 1;
                
                reset_flag = 1;
            }
            if(_gupgrade_reset_flag)
            {
                //不允许写测试位（BIT2）
                gstMainCopParaData.ctrl_info.upgrade_status.bits.mintor_test_restate = 0;
                if((uint32_t)(tim_get_systime() - timeout) > 10*1000)
                {
                    _geStatus = 3;
                    timeout = tim_get_systime();
                }
            }  
            break;
        }
        case 3://重启
        {
            gstFirmwareUpgrateInfo.rev_start_flag = 0;
            _SysRestart();
            _geStatus = 0;
            break;
        }
        default:
            break;
    }
}




