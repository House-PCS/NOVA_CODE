
#include    "firmware_upgrate.h"
#include    "dev_modbus.h"
#include    "lcd_draw.h"
#include    "miniz.h"
FirmwareUpgrateInfo_t gFirmwareUpgrateInfo;

static uint32_t _upgrate_start_time_out = 0;
static uint8_t _upgrate_data[512];
static  uint8_t _RevPcsUpdataAnalysis(uint8_t *p,uint16_t size); 
int32_t UpdataDeZip(uint32_t spi_base_addr,uint32_t file_size);
uint8_t  UpDataCheckSumErr=0;
uint16_t  UpdataDataSeqId = 0;
#define UPGRATE_TIME_OUT                5000
#define UPGRATE_TIME_RECV_OUT           10000
#define SLAVE_UPGRATE_DATA_SIZE         256


static  void CalUpdataSendPersent(uint32_t size)
{
    uint32_t  temp =0;
    uint8_t   updata_persent;
    gFirmwareUpgrateInfo.updata_send_size += size;
    temp = gFirmwareUpgrateInfo.updata_send_size *100;
    
    if(gFirmwareUpgrateInfo.file_size == 0)
    {
        updata_persent = 0;
    }
    else
    {
        updata_persent = temp / gFirmwareUpgrateInfo.file_size;
    }
    if(updata_persent >99)
    {
        updata_persent = 99;
    }
    gFirmwareUpgrateInfo.updata_send_persent = updata_persent;
}



static  void CalUpdataGetPersent(uint32_t size)
{
    uint32_t  temp =0;
    uint8_t   updata_persent;
    temp = size *100;
    
    if(gFirmwareUpgrateInfo.updata_get_total_size == 0)
    {
        updata_persent = 0;
    }
    else
    {
        updata_persent = temp / gFirmwareUpgrateInfo.updata_get_total_size;
    }
    if(updata_persent >99)
    {
        updata_persent = 99;
    }
    gFirmwareUpgrateInfo.updata_get_persent = updata_persent;
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

void _AckUpdataCmd(uint16_t cmd,uint8_t data)
{//低端
    uint8_t len =0;
    uint16_t checksum = 0;
    _upgrate_data[len++] = 0xA5;
    _upgrate_data[len++] = 0x01;
    _upgrate_data[len++] = 0x01;

    _upgrate_data[len++] = (uint8_t)cmd;
    _upgrate_data[len++] = cmd >> 8;

    _upgrate_data[len++] = data;

    checksum = _CheckSum(_upgrate_data, len);
    _upgrate_data[len++] = (uint8_t)checksum; 
    _upgrate_data[len++] = checksum >> 8;

 //调用发送
    ModbusCollectorRTUSend(_upgrate_data, len); 
}






static uint8_t  _ReqCmdAnalysis(uint8_t *p,uint16_t size)
{
    uint16_t cmd        = 0;
    uint16_t data_len   = 0;
    uint8_t  *pBuf      = p;
    uint8_t  updata_status =0;
    static   uint16_t old_updata_data_seqid = 0xffff;
    static   uint8_t  space_data_size =0;
    cmd      =  pBuf[0] | (pBuf[1] << 8);
    data_len =  pBuf[2] | (pBuf[3] << 8);
    switch(cmd)
    {
        case CMD_UPGRATE_PREPARE:
        {//下发升级命令 升级开始
            updata_status = UpgratePrepare();
//发送命令
            if(data_len != 0)
            {
                gFirmwareUpgrateInfo.updata_get_total_size = (pBuf[7]<<24) + (pBuf[6]<<16) +(pBuf[5]<<8) +pBuf[4];
                old_updata_data_seqid = 0XFFFF;
                space_data_size = pBuf[8];
                if(space_data_size >10)
                {//发送数据最多空10个字节
                    space_data_size = 10;
                }
            }
            else
            {
                space_data_size = 0;
            }
            _AckUpdataCmd(CMD_UPGRATE_PREPARE,updata_status); 
            break;
        }
        case CMD_UPGRATE_DATA:
        {//下发升级命令 下发升级数据
            UpdataDataSeqId = *(uint16_t *)&pBuf[4];      
            if(UpDataCheckSumErr == 1)
            {
                updata_status = 02; 
            }
            else
            {
                if(space_data_size == 0)
                {
                    updata_status = UpgrateDataDeal(&pBuf[4+space_data_size],(data_len -space_data_size));   //适配之前的协议。
                }
                else
                {
                    if(old_updata_data_seqid !=  UpdataDataSeqId)
                    {
                        old_updata_data_seqid = UpdataDataSeqId;
                        updata_status = UpgrateDataDeal(&pBuf[4+space_data_size],(data_len -space_data_size));   
                    }                     
                }
            }
            _AckUpdataCmd(CMD_UPGRATE_DATA,updata_status); 
            break;
        }
        case CMD_UPGRATE_COMPLETE:
        {//升级完成命令
            UpgrateRevComplete();
            _AckUpdataCmd(CMD_UPGRATE_COMPLETE,gFirmwareUpgrateInfo.upgrate_end_flag);
            break;
        }
        default:break;
    }
    
    return true;
}

static uint8_t  _AckCmdAnalysis(uint8_t *p,uint16_t size)
{

    uint16_t cmd        = 0;
    uint16_t data_len   = 0;
    uint8_t  *pBuf = p;
    cmd      =  pBuf[0] | (pBuf[1] << 8);
    uint8_t error_code = pBuf[2];
    switch(cmd)
    {
        case CMD_UPGRATE_PREPARE:
        {
            if(error_code == 0)
            {
                gFirmwareUpgrateInfo.slave_prepared_ack = 1;  
            }
            else
            {
                gFirmwareUpgrateInfo.slave_prepared_ack = 2;
            }
            break;
        }
        case CMD_UPGRATE_DATA:
        {
            if(error_code == 0)
            {
                gFirmwareUpgrateInfo.slave_data_ack = 1;
            }
            else
            {
                gFirmwareUpgrateInfo.slave_data_ack = 2;
            }
            break;
        }
        case CMD_UPGRATE_COMPLETE:
        {
            if(error_code == 0)
            {
                gFirmwareUpgrateInfo.slave_end_ack = 1;
            }
            else
            {
                gFirmwareUpgrateInfo.slave_end_ack = 2;
            }
            break;
        }
        default:break;
    }
    
    return true;
}


uint32_t   recv_count =0;
static  uint8_t _RevSlaveUpdataAnalysis(uint8_t *p,uint16_t size)
{

    uint16_t i;
    uint16_t checksum = 0;
    uint8_t  *pBuf = p;
    uint16_t  len = size;
//uint8_t  data_test[10] = {0,1,2,3,4,5,6,7,8};    
//    uint8_t  test_len =1;
//    uint16_t  test_sum =0;
//    test_sum = (data_test[test_len])+(data_test[test_len++]);

    if(pBuf[0] != 0x5A && pBuf[0] != 0xA5)
    {
        return false;//
    }
    
//    if(pBuf[1] != 0x03 && pBuf[2] != 0x00)
//    {
//        return false;
//    }
    
    if(len < 7)
    {
        return false;
    }
    checksum =_CheckSum(pBuf, (len-2));    
    if(checksum != (pBuf[len - 2] | (pBuf[len - 1] << 8)))
    {
        UpDataCheckSumErr = 1;
        recv_count ++;
        return false;//
    }
    else
    {
        UpDataCheckSumErr = 0;
    }
    
    if(pBuf[0] == 0x5A)
    {
        _ReqCmdAnalysis(&pBuf[3], len - 5);
    }
    else if(pBuf[0] == 0xA5)
    {//响应 
        _AckCmdAnalysis(&pBuf[3], len - 5);
    }
    return true;



}

/*返回值作为errorcode*/
uint16_t UpgratePrepare()
{
    uint32_t Fimrware_addr =0;
    uint8_t readdata[256];
    uint32_t time_out = 0;
    gFirmwareUpgrateInfo.upgrate_end_flag = 0;
    if(gFirmwareUpgrateInfo.rev_start_flag == 0)
    {
        gFirmwareUpgrateInfo.rev_start_flag = 1;
        gFirmwareUpgrateInfo.write_addr = FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR;
        _upgrate_start_time_out = tim_get_systime();        
        gFirmwareUpgrateInfo.file_size = 0;
        gFirmwareUpgrateInfo.file_check_sum = 0;
        gFirmwareUpgrateInfo.package_count = 0;
//发送升级反馈
        gFirmwareUpgrateInfo.updata_get_persent = 0;


//        uint32_t Fimrware_addr = FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR;
//        time_out = tim_get_systime();
//        for(uint16_t i = 0;i<200;i++)//800k
//        {
//            Fimrware_addr = FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR + (i*0x1000);
//            SpiFlashEraseSector(Fimrware_addr);
//        }
//        time_out = tim_get_systime() - time_out;
        return 0;
    }
    else
    {
        return 1;//已经有其他上位机在操作升级了
    }
}


uint16_t UpgrateDataDeal(uint8_t *pSrc, uint16_t rev_len)
{
    uint16_t i;
    if(gFirmwareUpgrateInfo.rev_start_flag == 0)
    {
        return 1;
    }
    /*数据保存至FLASH*/
    if(gFirmwareUpgrateInfo.write_addr + rev_len >= UPDATA_ZIP_RESOTRE_MAX_ADDR)//UPDATA_ZIP_RESOTRE_MAX_ADDR
    {
        return 1;
    }
    /*从第一包获取文件大小*/
    if(gFirmwareUpgrateInfo.package_count == 0)
    {
        gFirmwareUpgrateInfo.file_size_read = pSrc[4] | (pSrc[5] << 8) | (pSrc[6] << 16) | (pSrc[7] << 24);
    }
    SpiFlashWriteBytesWithErase(gFirmwareUpgrateInfo.write_addr, pSrc, rev_len);
    gFirmwareUpgrateInfo.write_addr += rev_len;
    _upgrate_start_time_out = tim_get_systime();
    /*计算数据量*/
    gFirmwareUpgrateInfo.file_size += rev_len;
    
//    /*过程增加校验计算*/
//    if(gFirmwareUpgrateInfo.file_size >= gFirmwareUpgrateInfo.file_size_read)
//    {
//        for(i = 0; i < rev_len - 4; i++)
//        {
//            gFirmwareUpgrateInfo.file_check_sum += pSrc[i];
//        }
//    }
//    else
//    {
//        for(i = 0; i < rev_len; i++)
//        {
//            gFirmwareUpgrateInfo.file_check_sum += pSrc[i];
//        }
//    }
    CalUpdataGetPersent(gFirmwareUpgrateInfo.file_size);   
    gFirmwareUpgrateInfo.package_count++;
    return 0;
}

uint16_t UpgrateRevComplete()
{
    if(gFirmwareUpgrateInfo.rev_start_flag == 0)
    {
        return 1;
    }
    else
    {
        gFirmwareUpgrateInfo.rev_end_flag = 1;
        return 0;
    }
}


static bool _FirmwareCheck()
{

    uint8_t temp[256];
    uint32_t size;


    uint32_t start_addr = 0;
    uint32_t check_sum_calc = 0;
    uint16_t i = 0;



    /*检查文件头*/
    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR, temp, 256);
    if(temp[0] != 0x5A || temp[1] != 0xA5 || temp[2] != 0x3C || temp[3] != 0xC3)
    {
        return false;
    }

//    gFirmwareUpgrateInfo.file_size = (temp[4] | (temp[5] << 8) | (temp[6] << 16) | (temp[7] << 24));
    /*文件大小与实际接收是否匹配*/
    if(gFirmwareUpgrateInfo.file_size != (temp[4] | (temp[5] << 8) | (temp[6] << 16) | (temp[7] << 24)))
    {
        return false;
    }
    if(gFirmwareUpgrateInfo.file_size == 0 || gFirmwareUpgrateInfo.file_size > DEZIP_DATA_SIZE_MAX) //4M
    {
        return false;
    }
    size = gFirmwareUpgrateInfo.file_size;
    
    /*文件和校验*/
    /*前期先不考虑重新读出来再进行校验*/
    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + size - 4, temp, 4);
    gFirmwareUpgrateInfo.file_check_sum = (temp[0] | (temp[1] << 8) | (temp[2] << 16) | (temp[3] << 24));

    size -=4;//去掉最后四个字节checksum
    start_addr = FIRMWARE_EXTERNAL_FLASH_ADDR;
    while(size > 256)
    {
        SpiFlashReadData(start_addr, temp, 256);
        size -= 256;
        start_addr += 256;
        for(i = 0; i < 256; i++)
        {
            check_sum_calc += temp[i];
        }
        
    }
    SpiFlashReadData(start_addr, temp, size);

    for(i = 0; i < size; i++)
    {
        check_sum_calc += temp[i];
    }
    
    if(check_sum_calc != gFirmwareUpgrateInfo.file_check_sum)
    {
        return false;
    }
    return true;
}


static EN_FIRMWARE_UPGRATE_STATUS _FirmwareAnalysis()
{
    uint8_t temp[256];
    uint32_t offset = 0;
    
    gFirmwareUpgrateInfo.bms_data_existed = 0;
    gFirmwareUpgrateInfo.main_ctl_data_existed = 0;
    gFirmwareUpgrateInfo.pcs_existed = 0;
    gFirmwareUpgrateInfo.arc_existed = 0;    
    /*读取第一个文件头*/
    offset += 256;
    while(offset < gFirmwareUpgrateInfo.file_size - 4)//-4是去除总和校验
    {
        SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + offset, temp, 256);
        if(temp[0] != 0x0C || temp[1] != 0x0C || temp[2] != 0x0C)
        {
            return FIRMWARE_UPGRATE_STATUS_ERROR;
        }
        if(temp[3] == UPDATA_MAIN_TYPE)/*主控*/  //0X00
        {
            if(gFirmwareUpgrateInfo.main_ctl_data_existed == 1)/*固件包重复*/
            {
                return FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gFirmwareUpgrateInfo.main_ctl_data_existed = 1;
                gFirmwareUpgrateInfo.main_ctl_data_offset = offset;
                gFirmwareUpgrateInfo.main_ctl_data_len = (temp[7] | (temp[8] << 8) | (temp[9] << 16) | (temp[10] << 24));
                gFirmwareUpgrateInfo.main_ctl_data_checksum = (temp[252] | (temp[253] << 8) | (temp[254] << 16) | (temp[255] << 24));
                offset += gFirmwareUpgrateInfo.main_ctl_data_len;
                CalUpdataSendPersent(gFirmwareUpgrateInfo.main_ctl_data_len);//如果含有主控数据 初始值需要直接加入。
            }
        }
        else if(temp[3] == UPDATA_BMS_TYPE)/*BMS*/  //0X03
        {
            if(gFirmwareUpgrateInfo.bms_data_existed == 1)/*固件包重复*/
            {
                return FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gFirmwareUpgrateInfo.bms_data_existed = 1;
                gFirmwareUpgrateInfo.bms_data_offset = offset;

                gFirmwareUpgrateInfo.bms_data_len = (temp[7] | (temp[8] << 8) | (temp[9] << 16) | (temp[10] << 24));
                gFirmwareUpgrateInfo.bms_data_checksum = (temp[252] | (temp[253] << 8) | (temp[254] << 16) | (temp[255] << 24));
                
                offset += gFirmwareUpgrateInfo.bms_data_len;
            }
        }
        else if(temp[3] == UPDATA_PCS_TYPE)/*PCS*/  
        {
            
            if(gFirmwareUpgrateInfo.pcs_existed == 1)
            {
                return FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gFirmwareUpgrateInfo.pcs_existed = 1;
                gFirmwareUpgrateInfo.pcs_data_offset = offset;
                gFirmwareUpgrateInfo.pcs_len = (temp[7] | (temp[8] << 8) | (temp[9] << 16) | (temp[10] << 24));
                gFirmwareUpgrateInfo.pcs_checksum = (temp[252] | (temp[253] << 8) | (temp[254] << 16) | (temp[255] << 24));
                offset += gFirmwareUpgrateInfo.pcs_len;
            }
        }
        else if(temp[3] == UPDATA_ARC_TYPE)/*ARC*/
        {   
            if(gFirmwareUpgrateInfo.arc_existed == 1)
            {
                return FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gFirmwareUpgrateInfo.arc_existed = 1;
                gFirmwareUpgrateInfo.arc_data_offset = offset;
                gFirmwareUpgrateInfo.arc_len = (temp[7] | (temp[8] << 8) | (temp[9] << 16) | (temp[10] << 24));
                gFirmwareUpgrateInfo.arc_checksum = (temp[252] | (temp[253] << 8) | (temp[254] << 16) | (temp[255] << 24));
                offset += gFirmwareUpgrateInfo.arc_len;
            }
        }
        else if(temp[3] == UPDATA_UI_TYPE)/*UI_TYPE*/
        {//存储位置 1200- 1455
     //直接在此处进行存储。先擦除
            break;
            if(gFirmwareUpgrateInfo.ui_type_img_existed == 1)
            {
                return FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gFirmwareUpgrateInfo.ui_type_img_existed = 1;
                gFirmwareUpgrateInfo.ui_type_img_offset = offset;
                gFirmwareUpgrateInfo.ui_type_img_len = (temp[7] | (temp[8] << 8) | (temp[9] << 16) | (temp[10] << 24));
                gFirmwareUpgrateInfo.ui_type_img_checksum = (temp[252] | (temp[253] << 8) | (temp[254] << 16) | (temp[255] << 24));
                offset += gFirmwareUpgrateInfo.ui_type_img_len;
            }           
        }
        else if(temp[3] == UPDATA_UI_TYPE1)/*UI_TYPE1*/
        {

            break;
            if(gFirmwareUpgrateInfo.ui_type1_font_existed == 1)
            {
                return FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gFirmwareUpgrateInfo.ui_type1_font_existed = 1;
                gFirmwareUpgrateInfo.ui_type1_font_offset = offset;
                gFirmwareUpgrateInfo.ui_type1_font_len = (temp[7] | (temp[8] << 8) | (temp[9] << 16) | (temp[10] << 24));
                gFirmwareUpgrateInfo.ui_type1_font_checksum = (temp[252] | (temp[253] << 8) | (temp[254] << 16) | (temp[255] << 24));
                offset += gFirmwareUpgrateInfo.ui_type1_font_len;
            } 
        }
        else
        {
            return FIRMWARE_UPGRATE_STATUS_ERROR;
        }
    }
    gFirmwareUpgrateInfo.slave_seek = 0;
    gFirmwareUpgrateInfo.slave_prepared_ack = 0;
    gFirmwareUpgrateInfo.slave_data_ack = 0;
    gFirmwareUpgrateInfo.slave_end_ack = 0;
 


    gFirmwareUpgrateInfo.bms_data_existed = 0;
    gFirmwareUpgrateInfo.pcs_existed = 0;
    gFirmwareUpgrateInfo.arc_existed = 0;
    gFirmwareUpgrateInfo.ui_type1_font_existed = 0;
    gFirmwareUpgrateInfo.ui_type_img_existed = 0;
//boot 中只升级监控卡，不升级其他固件。

    if(gFirmwareUpgrateInfo.bms_data_existed == 1)
    {

        gFirmwareUpgrateInfo.bms_data_existed = 0;
        gFirmwareUpgrateInfo.slave_len = gFirmwareUpgrateInfo.bms_data_len;
        gFirmwareUpgrateInfo.slave_data_offset = gFirmwareUpgrateInfo.bms_data_offset;
        gFirmwareUpgrateInfo.updata_slave_type = UPDATA_BMS_TYPE;

        return FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
    }

    else if(gFirmwareUpgrateInfo.pcs_existed == 1)
    {
        gFirmwareUpgrateInfo.pcs_existed = 0;
        gFirmwareUpgrateInfo.slave_len = gFirmwareUpgrateInfo.pcs_len;
        gFirmwareUpgrateInfo.slave_data_offset = gFirmwareUpgrateInfo.pcs_data_offset;
        gFirmwareUpgrateInfo.updata_slave_type = UPDATA_PCS_TYPE;
        return  FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
    }
    else if(gFirmwareUpgrateInfo.arc_existed ==1)
    {
        gFirmwareUpgrateInfo.arc_existed = 0;
        gFirmwareUpgrateInfo.slave_len = gFirmwareUpgrateInfo.arc_len;
        gFirmwareUpgrateInfo.slave_data_offset = gFirmwareUpgrateInfo.arc_data_offset;  
        gFirmwareUpgrateInfo.updata_slave_type = UPDATA_ARC_TYPE;
 
        return  FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;                         
    }

    else if(gFirmwareUpgrateInfo.main_ctl_data_existed == 1)
    {
        return FIRMWARE_UPGRATE_STATUS_MAINCTL;
    }
    else
    {
        return FIRMWARE_UPGRATE_STATUS_ERROR;
    }
}


static bool _MainCtlFirmwareCheck()
{
    uint8_t temp[256];
    uint16_t i;
    uint32_t check_sum_calc = 0;
    uint32_t start_addr = gFirmwareUpgrateInfo.main_ctl_data_offset + FIRMWARE_EXTERNAL_FLASH_ADDR + 256;
    uint32_t size = gFirmwareUpgrateInfo.main_ctl_data_len - 256;
    
    while(size > 256)
    {
        SpiFlashReadData(start_addr, temp, 256);
        size -= 256;
        start_addr += 256;
        for(i = 0; i < 256; i++)
        {
            check_sum_calc += temp[i];
        }
        
    }
    SpiFlashReadData(start_addr, temp, size);
    for(i = 0; i < size; i++)
    {
        check_sum_calc += temp[i];
    }
    
    if(check_sum_calc != gFirmwareUpgrateInfo.main_ctl_data_checksum)
    {
        return false;
    }
    
    return true;
}

static void _MainCtlFirmwareFlagUpdate()
{
    uint8_t temp[256];
    uint8_t data_temp[256];
    memset(temp, 0x0A, 256);
    SpiFlashWriteBytesWithErase(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, temp, 256);
    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, data_temp, 256); 
}

static void _SLAVEPrepareSend(uint8_t slave_type)
{
    uint16_t check_sum;
    void    (*pfun)(uint8_t *,uint16_t len);

    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = 0x00;
    _upgrate_data[2] = slave_type;//0x03;
    _upgrate_data[3] = CMD_UPGRATE_PREPARE & 0xFF;
    _upgrate_data[4] = (CMD_UPGRATE_PREPARE >> 8) & 0xFF;
    _upgrate_data[5] = 0x00;
    _upgrate_data[6] = 0x00; 
    
    check_sum = _CheckSum(_upgrate_data, 7);
    
    _upgrate_data[7] = check_sum & 0xFF;
    _upgrate_data[8] = (check_sum >> 8) & 0xFF;
    if(slave_type == UPDATA_BMS_TYPE)
    {
       ModbusBMSRTUSend(_upgrate_data, 9);
    }
    else
    {
       ModbusPCSRTUSend(_upgrate_data, 9);     
    }
}

static void _SLAVEDataSend(uint8_t slave_type,uint16_t size, uint32_t offset)
{
    uint16_t check_sum;
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = 0x00;
    _upgrate_data[2] = slave_type;//0x03;
    _upgrate_data[3] = CMD_UPGRATE_DATA & 0xFF;
    _upgrate_data[4] = (CMD_UPGRATE_DATA >> 8) & 0xFF;
    _upgrate_data[5] = size & 0xFF;
    _upgrate_data[6] = (size >> 8) & 0xFF;
    
    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + offset, &_upgrate_data[7], size);
    
    check_sum = _CheckSum(_upgrate_data, size + 7);
    
    _upgrate_data[7 + size] = check_sum & 0xFF;
    _upgrate_data[8 + size] = (check_sum >> 8) & 0xFF;
    


    if(slave_type == UPDATA_BMS_TYPE)
    {
       ModbusBMSRTUSend(_upgrate_data, size + 9);
    }
    else
    {
       ModbusPCSRTUSend(_upgrate_data, size + 9);
    }
}


  

static void _SLAVEEndSend(uint8_t slave_type)
{
    uint16_t check_sum;
    _upgrate_data[0] = 0x5A;
    _upgrate_data[1] = 0x00;
    _upgrate_data[2] = slave_type;//0x03;
    _upgrate_data[3] = CMD_UPGRATE_COMPLETE & 0xFF;
    _upgrate_data[4] = (CMD_UPGRATE_COMPLETE >> 8) & 0xFF;
    _upgrate_data[5] = 0x00;
    _upgrate_data[6] = 0x00; 
    
    check_sum = _CheckSum(_upgrate_data, 7);
    
    _upgrate_data[7] = check_sum & 0xFF;
    _upgrate_data[8] = (check_sum >> 8) & 0xFF;
    

    if(slave_type == UPDATA_BMS_TYPE)
    {
       ModbusBMSRTUSend(_upgrate_data, 9);
    }
    else
    {
       ModbusPCSRTUSend(_upgrate_data, 9);
    }
}


uint8_t GetUpdataBaudStatusInfo(void)
{
    return gFirmwareUpgrateInfo.enter_updata_buad_mode;
}


volatile uint8_t flag=0;
void UpgrateFirmwareProcess()
{
    static EN_FIRMWARE_UPGRATE_STATUS status = FIRMWARE_UPGRATE_STATUS_IDLE;
    static uint32_t timeout;
    lcd_progress_bar(gFirmwareUpgrateInfo.updata_get_persent<<1);

    switch(status)
    {
        case FIRMWARE_UPGRATE_STATUS_IDLE:
        {
            if(gFirmwareUpgrateInfo.rev_start_flag == 1)
            {
                gFirmwareUpgrateInfo.updata_send_persent = 0;
                if((uint32_t)(tim_get_systime() - _upgrate_start_time_out) > UPGRATE_TIME_RECV_OUT)   //将此超时时间加长，目的是防止上位机重发数据，导致超时。
                {
                    gFirmwareUpgrateInfo.rev_start_flag = 0;
                }
            }
            if(gFirmwareUpgrateInfo.rev_end_flag == 1)
            {
                
                status = FIRMWARE_UPGRATE_STATUS_DEZIP;
            }
            gFirmwareUpgrateInfo.updata_slave_type = UPDATA_NO_TYPE;


            if(flag == 1)
            {
                flag = 0;
                status = FIRMWARE_UPGRATE_STATUS_DEZIP;
//                gFirmwareUpgrateInfo.updata_get_total_size = 321314;
                gFirmwareUpgrateInfo.updata_get_total_size = 593025;
            }

            if((gFirmwareUpgrateInfo.rev_start_flag == 1)||(gFirmwareUpgrateInfo.rev_end_flag == 1))
            {
                gFirmwareUpgrateInfo.enter_updata_buad_mode =1;//进入升级波特率模式
            }
            else
            {
                gFirmwareUpgrateInfo.enter_updata_buad_mode =0;//退出升级波特率模式
            }

            break;
        }
        case FIRMWARE_UPGRATE_STATUS_DEZIP:
        {
            gFirmwareUpgrateInfo.write_addr = FIRMWARE_EXTERNAL_FLASH_ADDR;


            uint32_t Fimrware_addr = 0;
            for(uint16_t i = 0;i<DEZIP_DATA_MAX_SECTOR_SIZE;i++)//4M
            {
                Fimrware_addr = FIRMWARE_EXTERNAL_FLASH_ADDR + (i*0x1000);
                SpiFlashEraseSector(Fimrware_addr);
            }
            gFirmwareUpgrateInfo.file_size = 0;  //在解压的时候，获取当前解压之后的文件大小
            if(UpdataDeZip(FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR,gFirmwareUpgrateInfo.updata_get_total_size) == MZ_TRUE)
            {
                status = FIRMWARE_UPGRATE_STATUS_CHECK;
            }
            else
            {
                status = FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            break;
        }        


        case FIRMWARE_UPGRATE_STATUS_CHECK:
        {
            gFirmwareUpgrateInfo.rev_start_flag = 0;
            if(_FirmwareCheck() == false)
            {
                status = FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                status = FIRMWARE_UPGRATE_STATUS_TO_DO;
            }
            break;
        }


        case FIRMWARE_UPGRATE_STATUS_TO_DO:
        {
            status = _FirmwareAnalysis();
            timeout = tim_get_systime();
            break;
        }



        case FIRMWARE_UPGRATE_STATUS_ERROR:
        {
            gFirmwareUpgrateInfo.upgrate_end_flag = 2;
            gFirmwareUpgrateInfo.rev_end_flag = 0;
            status = FIRMWARE_UPGRATE_STATUS_IDLE;
            break;
        }
        case FIRMWARE_UPGRATE_STATUS_OK:
        {

//此处需要延迟 以便有足够时间发送升级100信息给上位机
            if((uint32_t)(tim_get_systime() - timeout) > 3000)  //3S超时来
            {
                gFirmwareUpgrateInfo.upgrate_end_flag = 1;
                gFirmwareUpgrateInfo.rev_end_flag = 0;
                if(gFirmwareUpgrateInfo.main_ctl_data_existed == 1)
                {
                    /*更新外部FLASH标志 FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR*/
                    _MainCtlFirmwareFlagUpdate();
                    
                    /*需要重启*/
                    status = FIRMWARE_UPGRATE_STATUS_WAIT_RESTART;
                    timeout = tim_get_systime();
                }
                else
                {
                    status = FIRMWARE_UPGRATE_STATUS_IDLE;
                }                    
            }
            gFirmwareUpgrateInfo.updata_send_persent = 100;
            gFirmwareUpgrateInfo.updata_get_persent = 100;
            break;
        }
        case FIRMWARE_UPGRATE_STATUS_WAIT_RESTART:
        {
            if((uint32_t)(tim_get_systime() - timeout) > UPGRATE_TIME_OUT)
            {
                /*重启*/
                __set_FAULTMASK(1);
                NVIC_SystemReset();
            }
            break;
        }

        
        case FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE:
        {
            if((uint32_t)(tim_get_systime() - timeout) < 1000)
            {
                break;
            }
            timeout = (uint32_t)(tim_get_systime());
//延迟1S开始升级，确保不受正常通讯数据影响
            gFirmwareUpgrateInfo.slave_prepared_ack=0;
            _SLAVEPrepareSend(gFirmwareUpgrateInfo.updata_slave_type);
            status = FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_PREPARE_ACK;
            timeout = tim_get_systime();
            break;
        }
        case FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_PREPARE_ACK:
        {
            if((uint32_t)(tim_get_systime() - timeout) > UPGRATE_TIME_OUT)
            {
                status = FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            if(gFirmwareUpgrateInfo.slave_prepared_ack != 0)
            {
                if(gFirmwareUpgrateInfo.slave_prepared_ack == 1)
                {
                    status = FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_DATA;
                }
                else
                {
                    status = FIRMWARE_UPGRATE_STATUS_ERROR;
                }
            }
            break;
        }
        case FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_DATA:
        {
            uint16_t size_per_transfer;
            if(gFirmwareUpgrateInfo.slave_len - gFirmwareUpgrateInfo.slave_seek < SLAVE_UPGRATE_DATA_SIZE)
            {
                size_per_transfer = gFirmwareUpgrateInfo.slave_len - gFirmwareUpgrateInfo.slave_seek;
            }
            else
            {
                size_per_transfer = SLAVE_UPGRATE_DATA_SIZE;
            }
            _SLAVEDataSend(gFirmwareUpgrateInfo.updata_slave_type,size_per_transfer, gFirmwareUpgrateInfo.slave_data_offset + gFirmwareUpgrateInfo.slave_seek);
            gFirmwareUpgrateInfo.slave_seek += size_per_transfer;
            CalUpdataSendPersent(size_per_transfer);                //用于计算分发百分比
            timeout = tim_get_systime();

            status = FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_DATA_ACK;
            gFirmwareUpgrateInfo.slave_data_ack=0;

            break;
        }
        case FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_DATA_ACK:
        {
            if((uint32_t)(tim_get_systime() - timeout) > UPGRATE_TIME_OUT)
            {
                status = FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            if(gFirmwareUpgrateInfo.slave_data_ack != 0)
            {
                if(gFirmwareUpgrateInfo.slave_data_ack == 1)
                {
                    if(gFirmwareUpgrateInfo.slave_seek >= gFirmwareUpgrateInfo.slave_len)
                    {
                        status = FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_END;
                        timeout = (uint32_t)(tim_get_systime());
                    }
                    else
                    {
                        status = FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_DATA;
                    }
                }
                else
                {
                    status = FIRMWARE_UPGRATE_STATUS_ERROR;
                }
            }
            break;
        }
        case FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_END:
        {
            status = FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_END_ACK;
//需要判定是哪个固件升级完成

            _SLAVEEndSend(gFirmwareUpgrateInfo.updata_slave_type);
            timeout = tim_get_systime();  
            gFirmwareUpgrateInfo.slave_end_ack=0;
            break;
        }
        case FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_END_ACK:
        {
            if((uint32_t)(tim_get_systime() - timeout) > UPGRATE_TIME_OUT)
            {
                status = FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            if(gFirmwareUpgrateInfo.slave_end_ack != 0)
            {
                timeout = tim_get_systime();
                if(gFirmwareUpgrateInfo.slave_end_ack == 1)
                {
                    gFirmwareUpgrateInfo.slave_seek = 0;
                    gFirmwareUpgrateInfo.slave_prepared_ack = 0;
                    gFirmwareUpgrateInfo.slave_data_ack = 0;
                    gFirmwareUpgrateInfo.slave_end_ack = 0; 
                    if(gFirmwareUpgrateInfo.bms_data_existed == 1)
                    {

                        gFirmwareUpgrateInfo.bms_data_existed = 0;
                        gFirmwareUpgrateInfo.slave_len = gFirmwareUpgrateInfo.bms_data_len;
                        gFirmwareUpgrateInfo.slave_data_offset = gFirmwareUpgrateInfo.bms_data_offset;
                        gFirmwareUpgrateInfo.updata_slave_type = UPDATA_BMS_TYPE;
                        status = FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
                        break;
                    }
                    else if(gFirmwareUpgrateInfo.pcs_existed == 1)
                    {
                        gFirmwareUpgrateInfo.pcs_existed = 0;
                        gFirmwareUpgrateInfo.slave_len = gFirmwareUpgrateInfo.pcs_len;
                        gFirmwareUpgrateInfo.slave_data_offset = gFirmwareUpgrateInfo.pcs_data_offset;
                        gFirmwareUpgrateInfo.updata_slave_type = UPDATA_PCS_TYPE;
                        status = FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
                    }
                    else if(gFirmwareUpgrateInfo.arc_existed ==1)
                    {
                        gFirmwareUpgrateInfo.arc_existed = 0;                        
                        gFirmwareUpgrateInfo.slave_len = gFirmwareUpgrateInfo.arc_len;
                        gFirmwareUpgrateInfo.slave_data_offset = gFirmwareUpgrateInfo.arc_data_offset;   
                        gFirmwareUpgrateInfo.updata_slave_type = UPDATA_ARC_TYPE;  
                        status = FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
                       
                    }
                    else
                    {
                        if(gFirmwareUpgrateInfo.main_ctl_data_existed == 1)
                        {
                            status = FIRMWARE_UPGRATE_STATUS_MAINCTL;
                        }
                        else
                        {
                            status = FIRMWARE_UPGRATE_STATUS_OK;
                        }
                    }
                }
                else
                {
                    status = FIRMWARE_UPGRATE_STATUS_ERROR;
                }
            }
            break;
        }
        case FIRMWARE_UPGRATE_STATUS_MAINCTL:
        {
            /*校验数据，如无误，则重启进入BOOT进行升级*/
            timeout = tim_get_systime();
            if(_MainCtlFirmwareCheck() == false)
            {
                status = FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                status = FIRMWARE_UPGRATE_STATUS_OK;
            }
        }
        default:break;
    }

    if(status != FIRMWARE_UPGRATE_STATUS_IDLE)
    {
        gFirmwareUpgrateInfo.updata_working = 1;
    }
    else
    {
        gFirmwareUpgrateInfo.updata_working = 0;
    }

//    if(status == FIRMWARE_UPGRATE_STATUS_IDLE)
//    {
//        gSysInfo.system_upgrate_status=0;
//        gSysInfo.pcs_llc_have_updata_data = 0;
//    }else
//    {
//        gSysInfo.system_upgrate_status=1;
//    }
}

void    FirmwareUpgrateInit(void)
{
    RegisterSlaveUpdataFunPoint(_RevSlaveUpdataAnalysis);        
}

uint8_t GetUpdataStuatus(void)
{
    return gFirmwareUpgrateInfo.updata_working;
}




#define  BUF_SIZE 1024

typedef struct _MyZipCtx_t 
{
	mz_zip_archive zip_ctx;
	uint32_t zip_size;
	uint8_t zip_md5[16];
	mz_uint zip_flags;


	mz_uint32 file_idx;
	char file_name[256];
	uint8_t file_md5[16];

} MyZipCtx_t;

static size_t mz_zip_read_func(void *pOpaque, mz_uint64 file_ofs, void *pBuf, size_t n)
{
    mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
    size_t s = (file_ofs >= pZip->m_archive_size) ? 0 : (size_t)MZ_MIN(pZip->m_archive_size - file_ofs, n);

    // read data from spi flash
	uint32_t spi_addr = FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR;
	SpiFlashReadData(spi_addr+file_ofs, pBuf, s);

	/*
	MyZipCtx_t *my_zip_ctx = (MyZipCtx_t *)pZip->userdata;
	MD5Update(&my_zip_ctx->zip_md5_ctx, pBuf, s);
	*/

    return s;
}

//uint32_t  zip_size =0;
//uint32_t  zip_checksum =0;
//uint8_t   err=0;
//uint32_t  zip_count=0;
//uint32_t  test_zip_file_size = 0;
static size_t mz_file_write_cb(void *pOpaque, mz_uint64 file_ofs, const void *pBuf, size_t n)
{
    uint8_t read_buf[1024];
    
    mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
	MyZipCtx_t *my_zip_ctx = (MyZipCtx_t *)pZip->userdata;

    SpiFlashWriteBytesWithoutErasenoAligned(gFirmwareUpgrateInfo.write_addr, (uint8_t *)pBuf, n);
    gFirmwareUpgrateInfo.write_addr += n;

    gFirmwareUpgrateInfo.file_size +=n;
//    test_zip_file_size +=n; 
	return n;
}


int32_t UpdataDeZip(uint32_t spi_base_addr,uint32_t file_size)
{
	uint32_t spi_addr = spi_base_addr;//(4096*4096 + 727 * 4096);

	static uint8_t __SDRAM write_buf[TINFL_LZ_DICT_SIZE] = {0};

	uint8_t s_inbuf[BUF_SIZE];
	uint8_t s_outbuf[BUF_SIZE];

	mz_bool status;
	mz_bool success = MZ_TRUE;
	mz_zip_error zip_err;
	MyZipCtx_t my_zip_ctx = {0};
	
	mz_zip_zero_struct(&my_zip_ctx.zip_ctx);
    my_zip_ctx.zip_size = file_size;//61663;
	my_zip_ctx.zip_flags = MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY;

	my_zip_ctx.zip_ctx.userdata = (void *)&my_zip_ctx;

//	uint32_t cost_time[3][2] = {{0}};

//	uint32_t file_ofs = 0, file_read = 0;
//	while (file_ofs < my_zip_ctx.zip_size)
//	{
//		file_read = sizeof(s_outbuf);
//		if (my_zip_ctx.zip_size - file_ofs < file_read)
//		{
//			file_read = my_zip_ctx.zip_size - file_ofs;
//		}
//		// read data from spi flash
//		SpiFlashReadData(spi_addr+file_ofs, s_outbuf, file_read);
//		file_ofs += file_read;
//	}
//	cost_time[0][0] = tim_get_systime();

	// init zip
	if (!mz_zip_reader_init_cb(&my_zip_ctx.zip_ctx, NULL, my_zip_ctx.zip_size, 
							   my_zip_ctx.zip_flags, mz_zip_read_func)) 
	{
		zip_err = my_zip_ctx.zip_ctx.m_last_error;
		return MZ_FALSE;
	}

//	char file_name[256] = {"icon_upg_Roca_G2_sub_0_20240219.bin"};
//	snprintf(my_zip_ctx.file_name, sizeof(my_zip_ctx.file_name), "%s", file_name);
//	if (!mz_zip_reader_locate_file_v2(&my_zip_ctx.zip_ctx, my_zip_ctx.file_name, NULL, 
//									  my_zip_ctx.zip_flags, &my_zip_ctx.file_idx)) 
//	{
//		return MZ_FALSE;
//	}

    my_zip_ctx.file_idx = 0;

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
	mz_zip_reader_end(&my_zip_ctx.zip_ctx);
//	cost_time[0][1] = tim_get_systime();
	return MZ_TRUE;
}












