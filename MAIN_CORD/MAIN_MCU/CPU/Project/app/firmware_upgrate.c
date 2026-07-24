
#include    "firmware_upgrate.h"
#include    "dev_modbus.h"
#include    "miniz.h"
#include    "slave_bms.h"
#include    "slave_coprocessor.h"
#include    "sys_info_storage.h"
#include    "md5.h"
FirmwareUpgrateInfo_t gstFirmwareUpgrateInfo;
uint8_t gCmdPcsRomoteAck = 0;
static uint8_t _gDelayOver = 0;
static uint32_t _gZipDleyTimeCmt = 5000; //500ms
static  uint32_t _gUpgrateStartTimeOut = 0;
static  uint8_t _garrUpgrateData[2148];//

uint8_t     gUpDataCheckSumErr = 0;
uint8_t     gBmsAddressingStartAddr = 0x30;
uint16_t    gUpdataDataSeqId = 0;
static uint32_t _bms_upgrade_time_out_cnt = 0;
static EN_FIRMWARE_UPGRATE_STATUS _geStatus = EN_FIRMWARE_UPGRATE_STATUS_IDLE;
static uint16_t _gFirmwareType = 0x00ff;
static uint16_t _gUpdataPackVerBuf = 0;
uint8_t   flag = 0 ;
static  uint8_t _RevPcsUpdataAnalysis(uint8_t *p, uint16_t size);
int32_t UpdataDeZip(uint32_t spi_base_addr, uint32_t file_size);
//firmware_type bit0 监控卡  bit1-PCS  bit2-ARC bit3-BMS bit4 UI-IMG   BIT5-UI-FONT
bool SetUpdataFirmwareType(uint16_t firmware_type, uint8_t confirm_status)
{
    //设置需要升级的固件
//bit 0--> 0 没有
    bool set_status = false;

    if(_geStatus == EN_FIRMWARE_UPGRATE_STATUS_IDLE)
    {
        _gFirmwareType = firmware_type;
        if(confirm_status == 1)
        {
            _geStatus = EN_FIRMWARE_UPGRATE_STATUS_CHECK;
        }
        set_status = true;
    }
    return set_status;
}
uint16_t GetCurUpdataType(void)
{
    return gstFirmwareUpgrateInfo.updata_slave_type;
}
EN_FIRMWARE_UPGRATE_STATUS GetCurUpdataStatus(void)
{
    return _geStatus;
}


static  void CalUpdataSendPersent(uint32_t size)
{
    uint32_t  temp = 0;
    uint8_t   updata_persent;
    gstFirmwareUpgrateInfo.updata_send_size += size;
    temp = gstFirmwareUpgrateInfo.updata_send_size * 100;
    if(gstFirmwareUpgrateInfo.updata_send_total_size == 0)
    {
        updata_persent = 0;

    }
    else
    {
        updata_persent = temp / gstFirmwareUpgrateInfo.updata_send_total_size;
    }

    if(updata_persent > 99)
    {
        updata_persent = 99;
    }
    gstFirmwareUpgrateInfo.updata_send_persent = updata_persent;
}


static  void CalUpdataGetPersent(uint32_t size)
{
    uint32_t  temp = 0;
    uint8_t   updata_persent;
    temp = size * 100;

    if(gstFirmwareUpgrateInfo.updata_get_total_size == 0)
    {
        updata_persent = 0;
    }
    else
    {
        updata_persent = temp / gstFirmwareUpgrateInfo.updata_get_total_size;
    }
    if(updata_persent > 99)
    {
        updata_persent = 99;
    }
    gstFirmwareUpgrateInfo.updata_get_persent = updata_persent;
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
    _garrUpgrateData[len++] = 0x01;
    _garrUpgrateData[len++] = 0x01;

    _garrUpgrateData[len++] = (uint8_t)cmd;
    _garrUpgrateData[len++] = cmd >> 8;

    _garrUpgrateData[len++] = data;

    checksum = _CheckSum(_garrUpgrateData, len);
    _garrUpgrateData[len++] = (uint8_t)checksum;
    _garrUpgrateData[len++] = checksum >> 8;

//调用发送
    ModbusCollectorRTUSend(_garrUpgrateData, len);
}

bool  CheckMd5(uint8_t *p_src_md5_data, uint32_t file_size)
{
    //0 表示计算错误  1 表示对比OK
    uint8_t arr_temp[2048];
    uint8_t arr_md5[16];
    MD5_CTX md5_ctx;
//计算和存储的是否一致。
    uint16_t i;
    uint32_t start_addr = FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR;
    uint32_t size = file_size;
    MD5Init(&md5_ctx);
    while(size > 2048)
    {
        SpiFlashReadData(start_addr, arr_temp, 2048);
        size -= 2048;
        start_addr += 2048;
        MD5Update(&md5_ctx, arr_temp, 2048);
    }
    SpiFlashReadData(start_addr, arr_temp, size);
    MD5Update(&md5_ctx, arr_temp, size);
    MD5Final(arr_md5, &md5_ctx);

    for(i = 0; i < 16; i++)
    {
        if(p_src_md5_data[i] != arr_md5[i])
        {
            return 0;
        }
    }
    return 1;
}

uint8_t up_start_temp[20];
/**
* @brief     给协处理器发送升级开始
  * @param   None
  * @return  None
  * @note    None
  */
void _UpgradePrepareUartSend(uint32_t lenth)
{
    uint16_t check_sum;
    memset(up_start_temp,0,20);
    
    up_start_temp[0] = 0x5A;
    up_start_temp[1] = 0x00;
    up_start_temp[2] = 0x80;
    up_start_temp[3] = CMD_UPGRATE_PREPARE & 0xFF;                  //cmd
    up_start_temp[4] = (CMD_UPGRATE_PREPARE >> 8) & 0xFF;         
    up_start_temp[5] = 0x05;                                        //携带数据长度
    up_start_temp[6] = 0x00;
    memcpy(&up_start_temp[7], &lenth, sizeof(uint32_t));            //固件包长度
    up_start_temp[11] = 2;                                          //拓展字节长度
    check_sum = _CheckSum(up_start_temp, 12);

    up_start_temp[12] = check_sum & 0xFF;
    up_start_temp[13] = (check_sum >> 8) & 0xFF;
    ModbusCoprocessorRTUSend(up_start_temp, 14);
}





uint8_t  test = 0;
/**
  * @brief   固件接收，接收采集器下发的升级命令
  * @param   None  
  * @return  None
  * @note    None
  */
static uint8_t  _ReqCmdAnalysis(uint8_t *p, uint16_t size)
{
// 采集器下发升级命令给监控卡
    uint16_t cmd        = 0;
    uint16_t data_len   = 0;
    uint8_t  *pBuf      = p;
    uint8_t  updata_status = 0;
    static   uint16_t old_updata_data_seqid = 0xffff;
    static   uint8_t  space_data_size = 0;
    uint8_t  f_md5 = 0;
    cmd      =  pBuf[0] | (pBuf[1] << 8);
    data_len =  pBuf[2] | (pBuf[3] << 8);
    switch(cmd)
    {
        case CMD_UPGRATE_PREPARE:
        {
            test = 1;
            //下发升级命令 升级开始
            _gZipDleyTimeCmt = 10000;//默认10S
            updata_status = UpgratePrepare();
//发送命令
            if(data_len != 0)
            {
                gstFirmwareUpgrateInfo.updata_get_total_size = (pBuf[7] << 24) + (pBuf[6] << 16) + (pBuf[5] << 8) + pBuf[4];
                old_updata_data_seqid = 0XFFFF;
                space_data_size = pBuf[8];//
                if(space_data_size > 10)
                {
                    //发送数据最多空10个字节
                    space_data_size = 10;
                }

                f_md5 = CheckMd5(&pBuf[9], gstFirmwareUpgrateInfo.updata_get_total_size); //pBuf[9] - pBuf[24] //标识md5
                if(f_md5 == 1)
                {
                    //和spi flash中的固件MD5一样。
                    updata_status = CMD_SAME_FIARMWARE;
                    test = 2;
                }
            }
            else
            {
                space_data_size = 0;
            }
            _AckUpdataCmd(CMD_UPGRATE_PREPARE, updata_status);
            break;
        }
        case CMD_UPGRATE_DATA:
        {
            //下发升级命令 下发升级数据
            test = 3;
            _gZipDleyTimeCmt = 5000;
            gUpdataDataSeqId = *(uint16_t *)&pBuf[4];
            if(gUpDataCheckSumErr == 1)
            {
                updata_status = 02;
            }
            else
            {
                if(space_data_size == 0)
                {
                    updata_status = UpgrateDataDeal(&pBuf[4 + space_data_size], (data_len - space_data_size)); //适配之前的协议。
                }
                else
                {
                    //根据流水号接收
                    if(old_updata_data_seqid !=  gUpdataDataSeqId)
                    {
                        old_updata_data_seqid = gUpdataDataSeqId;
                        updata_status = UpgrateDataDeal(&pBuf[4 + space_data_size], (data_len - space_data_size));
                    }
                }
            }
            _AckUpdataCmd(CMD_UPGRATE_DATA, updata_status);
            break;
        }
        case CMD_UPGRATE_COMPLETE:
        {
            //升级完成命令
            test = 4;
            UpgrateRevComplete();
            _AckUpdataCmd(CMD_UPGRATE_COMPLETE, gstFirmwareUpgrateInfo.upgrate_end_flag);
            break;
        }
        default:
            break;
    }

    return true;
}

static uint8_t  _AckCmdAnalysis(uint8_t *p, uint16_t size)
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

            gstFirmwareUpgrateInfo.slave_prepared_ack = error_code + 1;
//            if(error_code == 0)
////            {
////                gstFirmwareUpgrateInfo.slave_prepared_ack = 1;
////            }
////            else
////            {
////                gstFirmwareUpgrateInfo.slave_prepared_ack = 2;
////            }
            break;
        }
        case CMD_UPGRATE_DATA:
        {
            if(error_code == 0)
            {
                gstFirmwareUpgrateInfo.slave_data_ack = 1;
            }
            else
            {
                gstFirmwareUpgrateInfo.slave_data_ack = 2;
            }
            break;
        }
        case CMD_UPGRATE_COMPLETE:
        {

            gstFirmwareUpgrateInfo.slave_end_ack = (error_code + 1);
//            if(error_code == 0)
//            {
//                gstFirmwareUpgrateInfo.slave_end_ack = 1;
//            }
//            else
//            {
//                gstFirmwareUpgrateInfo.slave_end_ack = 2;
//            }
//            else
//            {
//                gstFirmwareUpgrateInfo.slave_end_ack = 3;
//            }
            break;
        }
        case BMS_CMD_ADDRESSING:
        {
            if(error_code == 0)
            {
                gstFirmwareUpgrateInfo.bms_addressing_ack = 1;
            }
            else
            {
                gstFirmwareUpgrateInfo.bms_addressing_ack = 2;
            }

            break;
        }
        case ROMOTE_SHUT_DOWN:
        {
            if(error_code == 0)
            {
                gCmdPcsRomoteAck =  1;
            }
            else
            {
                gCmdPcsRomoteAck =  2;
            }
            break;
        }


        default:
            break;
    }

    return true;
}

uint8_t GetPcsRomoteAck(void)
{
    return gCmdPcsRomoteAck;
}
void CleanPcsRmoteAck(void)
{
    gCmdPcsRomoteAck = 0;
}


static  uint8_t _RevSlaveUpdataAnalysis(uint8_t *p, uint16_t size)
{

    uint16_t i;
    uint16_t checksum = 0;
    uint8_t  *pBuf = p;
    uint16_t  len = size;


    if(pBuf[0] != 0x5A && pBuf[0] != 0xA5)
    {
        return false;//
    }

    if(len < 7)
    {
        return false;
    }
    checksum = _CheckSum(pBuf, (len - 2));
    if(checksum != (pBuf[len - 2] | (pBuf[len - 1] << 8)))
    {
        gUpDataCheckSumErr = 1;
        return false;//
    }
    else
    {
        gUpDataCheckSumErr = 0;
    }

    if(pBuf[0] == 0x5A)
    {
        _ReqCmdAnalysis(&pBuf[3], len - 5);
    }
    else if(pBuf[0] == 0xA5)
    {
        //响应
        _AckCmdAnalysis(&pBuf[3], len - 5);
    }
    return true;



}

/*返回值作为errorcode*/
uint16_t UpgratePrepare()
{
    uint32_t Fimrware_addr = 0;
    uint32_t time_out = 0;
    gstFirmwareUpgrateInfo.upgrate_end_flag = 0;

    if(gstFirmwareUpgrateInfo.rev_start_flag == 0)
    {
        gstFirmwareUpgrateInfo.rev_start_flag = 1;
        gstFirmwareUpgrateInfo.upgrad_err_code = 0;
//        ShowEmUpgrading(1);
        gstFirmwareUpgrateInfo.upgrading_status = 1;
        gstFirmwareUpgrateInfo.write_addr = FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR;
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

//uint16_t Test_data_len;
uint16_t UpgrateDataDeal(uint8_t *pSrc, uint16_t rev_len)
{
    uint16_t i;
    if(gstFirmwareUpgrateInfo.rev_start_flag == 0)
    {
        return 1;
    }
    /*数据保存至FLASH*/
    if(gstFirmwareUpgrateInfo.write_addr + rev_len >= UPDATA_ZIP_RESOTRE_MAX_ADDR)//0x1FE000
    {
        return 1;
    }
    /*从第一包获取文件大小*/
    if(gstFirmwareUpgrateInfo.package_count == 0)
    {
        gstFirmwareUpgrateInfo.file_size_read = pSrc[4] | (pSrc[5] << 8) | (pSrc[6] << 16) | (pSrc[7] << 24);
    }
    SpiFlashWriteBytesWithErase(gstFirmwareUpgrateInfo.write_addr, pSrc, rev_len);
    gstFirmwareUpgrateInfo.write_addr += rev_len;
    _gUpgrateStartTimeOut = tim_get_systime();
    /*计算数据量*/
    gstFirmwareUpgrateInfo.file_size += rev_len;

    CalUpdataGetPersent(gstFirmwareUpgrateInfo.file_size);
    gstFirmwareUpgrateInfo.package_count++;

//    if(rev_len != 256)
//    {
//        Test_data_len = rev_len;
//    }
    return 0;
}

uint16_t UpgrateRevComplete()
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

//void ProgramflashImgWordData(uint32_t soruce_addr,uint32_t terminal_addr,uint32_t data_size)
//{
//    uint8_t arr_temp[FIRMWARE_IMA_READ_DATA_MAX];
//    uint16_t offset_seek = 0;
//    uint16_t size = data_size;
//    uint32_t write_addr = terminal_addr;
//
//    while(size > FIRMWARE_IMA_READ_DATA_MAX)
//    {
////                SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + gstFirmwareUpgrateInfo.ui_type1_word_offset + offset_seek,arr_temp, FIRMWARE_IMA_READ_DATA_MAX);
//        SpiFlashReadData(soruce_addr + offset_seek,arr_temp, FIRMWARE_IMA_READ_DATA_MAX);
//        SpiFlashWriteBytesWithErase(write_addr + offset_seek, arr_temp, FIRMWARE_IMA_READ_DATA_MAX);
//        size -=FIRMWARE_IMA_READ_DATA_MAX;
//        offset_seek += FIRMWARE_IMA_READ_DATA_MAX;
//    }
//    SpiFlashReadData(soruce_addr + offset_seek,arr_temp, size);
//    SpiFlashWriteBytesWithErase(write_addr + offset_seek, arr_temp, size);
//}


/**
  * @brief   图片资源升级
  * @param   void
  * @return  0:成功
  * @note    None
  */
static int32_t UpdateIcon(void)
{
    /**************************************************/
    /*第一张图片在新flash中的地址*/
    int32_t n_ret, i = 0;
    int32_t absolute_addr = 0;
    int32_t icondata_size = 0;
    uint32_t spi_pos = 0;
    uint8_t arr_buffer[BUFFER_SIZE] = {0};
    uint8_t arr_head_data[8] = {0};
    int32_t allicon_size = 0;
    uint32_t data_ofs = 0, data_read = BUFFER_SIZE;
    int32_t first_icon_addr = 0;

    uint32_t flash_addr = 0;

    flash_addr = UPDATE_DATA_START_ADDR + gstFirmwareUpgrateInfo.ui_type_img_offset;
//  flash_addr = GetIconFlashAddr();

    first_icon_addr = flash_addr + 256;

    //获取图片升级包文件大小
    SpiFlashReadData(flash_addr + 7, (uint8_t *)&allicon_size, sizeof(allicon_size));

//  uint8_t arr_temp[256] = {0};
//  SpiFlashReadData(flash_addr, arr_temp, sizeof(arr_temp));

    //获取所有图片的大小，以及在升级包中的真实地址
    for(i = 0; i < ICON_NUM; ++i, data_ofs = 0, data_read = BUFFER_SIZE)
    {
        //升级包中最后一个图片的偏移地址 大于等于 升级包中所有图片文件大小，则返回，升级完成
        if(spi_pos >= allicon_size - 256)  //-256是去除包头信息  剩余的则为图片信息大小
        {
            return n_ret;
        }

        //读取每一个图片的头信息（绝对地址和文件大小）
        SpiFlashReadData(first_icon_addr + spi_pos, arr_head_data, sizeof(arr_head_data));

        absolute_addr = arr_head_data[0] | arr_head_data[1] << 8 | arr_head_data[2] << 16 | arr_head_data[3] << 24;
        icondata_size = arr_head_data[4] | arr_head_data[5] << 8 | arr_head_data[6] << 16 | arr_head_data[7] << 24;

        //读取flash中每个图片的内容
        while(data_ofs < icondata_size)
        {
            if(icondata_size - data_ofs < data_read)
            {
                data_read = icondata_size - data_ofs;
            }
            SpiFlashReadData(first_icon_addr + 8 + spi_pos + data_ofs, arr_buffer, data_read);

            //将读到的数据写入旧flash地址中，达到更新图片的效果
            SpiFlashWriteBytesWithErase((4096 * 4096) + (absolute_addr - 4096) * 4096 + data_ofs, arr_buffer, data_read);
            data_ofs += data_read;
        }

        spi_pos += (sizeof(arr_head_data));
        spi_pos += icondata_size;
    }

    return n_ret;
}

/**
  * @brief   字体资源升级
  * @param   void
  * @return  0:成功
  * @note    None
  */
static int32_t UpdateFont(void)
{
    int32_t n_ret, i = 0;
    uint32_t flash_addr = 0;
    int32_t absolute_addr = 0;
    int32_t fontdata_size = 0;
    int32_t first_font_addr = 0;
    uint32_t allfont_size = 0;
    uint8_t arr_head_data[8] = {0};
    uint8_t arr_buffer[1024] = {0};
    uint32_t spi_pos = 0;
    uint32_t data_ofs = 0;
    uint32_t data_read = BUFFER_SIZE;

    flash_addr = UPDATE_DATA_START_ADDR + gstFirmwareUpgrateInfo.ui_type1_font_offset;

    //获取字库升级包中字体内容大小
    SpiFlashReadData(flash_addr + 7, (uint8_t *)&allfont_size, sizeof(allfont_size));

    first_font_addr = flash_addr + 256;

    for(i = 0; i < 5; i++, data_ofs = 0, data_read = BUFFER_SIZE)
    {
        //升级包中最后一个图片的偏移地址 大于等于 升级包中所有图片文件大小，则返回，升级完成
        if(spi_pos >= allfont_size - 256)  //-256是去除包头信息  剩余的则为图片信息大小
        {
            return n_ret;
        }
        //读取每一种字体的头信息
        SpiFlashReadData(first_font_addr + spi_pos, arr_head_data, sizeof(arr_head_data));

        absolute_addr = arr_head_data[0] | arr_head_data[1] << 8 | arr_head_data[2] << 16 | arr_head_data[3] << 24;
        fontdata_size = arr_head_data[4] | arr_head_data[5] << 8 | arr_head_data[6] << 16 | arr_head_data[7] << 24;

        //读取每中字体的数据
        while(data_ofs < fontdata_size)
        {
            if(fontdata_size - data_ofs < data_read)
            {
                data_read = fontdata_size - data_ofs;
            }
            //读取flash中的数据
            SpiFlashReadData(first_font_addr + 8 + data_ofs, arr_buffer, data_read);

            //写入旧flash中，达到更新数据
            SpiFlashWriteBytesWithErase((6144 * 4096) + ((absolute_addr - 6144) * 4096 ) + data_ofs, arr_buffer, data_read);

            data_ofs += data_read;
        }

        spi_pos += (sizeof(arr_head_data));
        spi_pos += fontdata_size;
    }

    return n_ret;
}


static bool _FirmwareCheck()
{

    uint8_t arr_temp[256];
    uint32_t size;

    uint32_t start_addr = 0;
    uint32_t check_sum_calc = 0;
    uint16_t i = 0;



    /*检查文件头*/
    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR, arr_temp, 256);
    if(arr_temp[0] != 0x5A || arr_temp[1] != 0xA5 || arr_temp[2] != 0x3C || arr_temp[3] != 0xC3)
    {
        return false;
    }
//    gstFirmwareUpgrateInfo.file_size = (arr_temp[4] | (arr_temp[5] << 8) | (arr_temp[6] << 16) | (arr_temp[7] << 24));
    /*文件大小与实际接收是否匹配*/
    if(gstFirmwareUpgrateInfo.updata_dezip_file_size != (arr_temp[4] | (arr_temp[5] << 8) | (arr_temp[6] << 16) | (arr_temp[7] << 24)))
    {
        return false;
    }
    if(gstFirmwareUpgrateInfo.updata_dezip_file_size == 0 || gstFirmwareUpgrateInfo.updata_dezip_file_size > DEZIP_DATA_SIZE_MAX) //2M
    {
        return false;
    }
    size = gstFirmwareUpgrateInfo.updata_dezip_file_size;

    /*文件和校验*/
    /*前期先不考虑重新读出来再进行校验*/
    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + size - 4, arr_temp, 4);
    gstFirmwareUpgrateInfo.file_check_sum = (arr_temp[0] | (arr_temp[1] << 8) | (arr_temp[2] << 16) | (arr_temp[3] << 24));

    size -= 4; //去掉最后四个字节checksum
    start_addr = FIRMWARE_EXTERNAL_FLASH_ADDR;
    while(size > 256)
    {
        SpiFlashReadData(start_addr, arr_temp, 256);
        size -= 256;
        start_addr += 256;
        for(i = 0; i < 256; i++)
        {
            check_sum_calc += arr_temp[i];
        }
    }
    SpiFlashReadData(start_addr, arr_temp, size);

    for(i = 0; i < size; i++)
    {
        check_sum_calc += arr_temp[i];
    }

    if(check_sum_calc != gstFirmwareUpgrateInfo.file_check_sum)
    {
        return false;
    }
    return true;
}

/**
  * @brief  固件解析
  * @param   None  
  * @return  None
  * @note    None
  */
static EN_FIRMWARE_UPGRATE_STATUS _FirmwareAnalysis()
{
    uint8_t arr_temp[4096];
    uint32_t offset = 0;
    uint16_t pack_ver;
    uint16_t pack_ver1;

    gstFirmwareUpgrateInfo.bms_data_existed = 0;
    gstFirmwareUpgrateInfo.main_ctl_data_existed = 0;
    gstFirmwareUpgrateInfo.pcs_existed = 0;
    gstFirmwareUpgrateInfo.arc_existed = 0;
    gstFirmwareUpgrateInfo.ui_type_img_existed = 0;
    gstFirmwareUpgrateInfo.ui_type1_font_existed = 0;

    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR, arr_temp, 256);
//获取升级包版本
    _gUpdataPackVerBuf = 0;
    arr_temp[10] &= 0x0f;   //4bit
    arr_temp[11] &= 0x3f;   //6bit
    arr_temp[12] &= 0x3f;   //6bit
    pack_ver = arr_temp[10];
    pack_ver <<= 12;
    pack_ver1 = arr_temp[11];
    pack_ver1 <<= 6;

    _gUpdataPackVerBuf = (pack_ver | pack_ver1 | arr_temp[12]);

//    gstCollectInfo.maincard_data.inside_info.update_code_ver = (pack_ver|pack_ver1|arr_temp[12]);

    gstFirmwareUpgrateInfo.sys_firmware_packet_type.packet_type = 0;


    /*读取第一个文件头*/
    offset += 256;
    while(offset < gstFirmwareUpgrateInfo.updata_dezip_file_size - 4)//-4是去除总和校验
    {
        SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + offset, arr_temp, 256);
        if(arr_temp[0] != 0x0C || arr_temp[1] != 0x0C || arr_temp[2] != 0x0C)
        {
            return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
        }
        if(arr_temp[3] == UPDATA_MAIN_TYPE)/*主控*/  //0X00
        {
            if(gstFirmwareUpgrateInfo.main_ctl_data_existed == 1)/*固件包重复*/
            {
                return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gstFirmwareUpgrateInfo.main_ctl_data_existed = 1;
                gstFirmwareUpgrateInfo.main_ctl_data_offset = offset;
                gstFirmwareUpgrateInfo.main_ctl_data_len = (arr_temp[7] | (arr_temp[8] << 8) | (arr_temp[9] << 16) | (arr_temp[10] << 24));
                gstFirmwareUpgrateInfo.main_ctl_data_checksum = (arr_temp[252] | (arr_temp[253] << 8) | (arr_temp[254] << 16) | (arr_temp[255] << 24));
                offset += gstFirmwareUpgrateInfo.main_ctl_data_len;
            }
        }
        else if(arr_temp[3] == UPDATA_BMS_TYPE)/*BMS*/  //0X03
        {
            if(gstFirmwareUpgrateInfo.bms_data_existed == 1)/*固件包重复*/
            {
                return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gstFirmwareUpgrateInfo.bms_data_existed = 1;
                gstFirmwareUpgrateInfo.bms_data_offset = offset;

                gstFirmwareUpgrateInfo.bms_data_len = (arr_temp[7] | (arr_temp[8] << 8) | (arr_temp[9] << 16) | (arr_temp[10] << 24));
                gstFirmwareUpgrateInfo.bms_data_checksum = (arr_temp[252] | (arr_temp[253] << 8) | (arr_temp[254] << 16) | (arr_temp[255] << 24));
                offset += gstFirmwareUpgrateInfo.bms_data_len;
            }
        }


        else if(arr_temp[3] == UPDATA_FPGA_TYPE)/*FPGA*/  //0X04
        {
            if(gstFirmwareUpgrateInfo.fpga_data_existed == 1)/*固件包重复*/
            {
                return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gstFirmwareUpgrateInfo.fpga_data_existed = 1;
                gstFirmwareUpgrateInfo.fpga_data_offset = offset;

                gstFirmwareUpgrateInfo.fpga_data_len = (arr_temp[7] | (arr_temp[8] << 8) | (arr_temp[9] << 16) | (arr_temp[10] << 24));
                gstFirmwareUpgrateInfo.fpga_data_checksum = (arr_temp[252] | (arr_temp[253] << 8) | (arr_temp[254] << 16) | (arr_temp[255] << 24));
                offset += gstFirmwareUpgrateInfo.fpga_data_len;
            }
        }

        else if(arr_temp[3] == UPDATA_PCS_TYPE)/*PCS*/
        {
            if(gstFirmwareUpgrateInfo.pcs_existed == 1)
            {
                return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gstFirmwareUpgrateInfo.pcs_existed = 1;
                gstFirmwareUpgrateInfo.pcs_data_offset = offset;
                gstFirmwareUpgrateInfo.pcs_len = (arr_temp[7] | (arr_temp[8] << 8) | (arr_temp[9] << 16) | (arr_temp[10] << 24));
                gstFirmwareUpgrateInfo.pcs_checksum = (arr_temp[252] | (arr_temp[253] << 8) | (arr_temp[254] << 16) | (arr_temp[255] << 24));
                offset += gstFirmwareUpgrateInfo.pcs_len;
            }
        }

        else if(arr_temp[3] == UPDATA_ARC_TYPE)/*ARC*/
        {
            if(gstFirmwareUpgrateInfo.arc_existed == 1)
            {
                return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gstFirmwareUpgrateInfo.arc_existed = 1;
                gstFirmwareUpgrateInfo.arc_data_offset = offset;
                gstFirmwareUpgrateInfo.arc_len = (arr_temp[7] | (arr_temp[8] << 8) | (arr_temp[9] << 16) | (arr_temp[10] << 24));
                gstFirmwareUpgrateInfo.arc_checksum = (arr_temp[252] | (arr_temp[253] << 8) | (arr_temp[254] << 16) | (arr_temp[255] << 24));
                offset += gstFirmwareUpgrateInfo.arc_len;
            }
        }
        else if(arr_temp[3] == UPDATA_MAIN_COP_TYPE)/*监控卡协处理器*/  //0X80
        {
            if(gstFirmwareUpgrateInfo.main_cop_existed == 1)/*固件包重复*/
            {
                return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gstFirmwareUpgrateInfo.main_cop_existed = 1;
                gstFirmwareUpgrateInfo.main_cop_data_offset = offset;
                gstFirmwareUpgrateInfo.main_cop_len = (arr_temp[7] | (arr_temp[8] << 8) | (arr_temp[9] << 16) | (arr_temp[10] << 24));
                gstFirmwareUpgrateInfo.main_cop_checksum = (arr_temp[252] | (arr_temp[253] << 8) | (arr_temp[254] << 16) | (arr_temp[255] << 24));
                offset += gstFirmwareUpgrateInfo.main_cop_len;
            }
        }
        else if(arr_temp[3] == UPDATA_PCS_COP_TYPE)/*PCS协处理器*/  //0X81
        {
            if(gstFirmwareUpgrateInfo.pcs_cop_existed == 1)/*固件包重复*/
            {
                return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gstFirmwareUpgrateInfo.pcs_cop_existed = 1;
                gstFirmwareUpgrateInfo.pcs_cop_data_offset = offset;
                gstFirmwareUpgrateInfo.pcs_cop_len = (arr_temp[7] | (arr_temp[8] << 8) | (arr_temp[9] << 16) | (arr_temp[10] << 24));
                gstFirmwareUpgrateInfo.pcs_cop_checksum = (arr_temp[252] | (arr_temp[253] << 8) | (arr_temp[254] << 16) | (arr_temp[255] << 24));
                offset += gstFirmwareUpgrateInfo.pcs_cop_len;
            }
        }
        else if(arr_temp[3] == UPDATA_UI_TYPE)/*UI_TYPE*/
        {
            //存储位置 1200- 1455
            //直接在此处进行存储。先擦除
            if(gstFirmwareUpgrateInfo.ui_type_img_existed == 1)
            {
                return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gstFirmwareUpgrateInfo.ui_type_img_existed = 1;
                gstFirmwareUpgrateInfo.ui_type_img_offset = offset;
                gstFirmwareUpgrateInfo.ui_type_img_len = (arr_temp[7] | (arr_temp[8] << 8) | (arr_temp[9] << 16) | (arr_temp[10] << 24));
                gstFirmwareUpgrateInfo.ui_type_img_checksum = (arr_temp[252] | (arr_temp[253] << 8) | (arr_temp[254] << 16) | (arr_temp[255] << 24));
                offset += gstFirmwareUpgrateInfo.ui_type_img_len;
            }
        }
        else if(arr_temp[3] == UPDATA_UI_TYPE1)/*UI_TYPE1*/
        {
            //
            if(gstFirmwareUpgrateInfo.ui_type1_font_existed == 1)
            {
                return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            else
            {
                gstFirmwareUpgrateInfo.ui_type1_font_existed = 1;
                gstFirmwareUpgrateInfo.ui_type1_font_offset = offset;
                gstFirmwareUpgrateInfo.ui_type1_font_len = (arr_temp[7] | (arr_temp[8] << 8) | (arr_temp[9] << 16) | (arr_temp[10] << 24));
                gstFirmwareUpgrateInfo.ui_type1_font_checksum = (arr_temp[252] | (arr_temp[253] << 8) | (arr_temp[254] << 16) | (arr_temp[255] << 24));
                offset += gstFirmwareUpgrateInfo.ui_type1_font_len;
            }
        }
        else
        {
            return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
        }
    }
    gstFirmwareUpgrateInfo.slave_seek = 0;
    gstFirmwareUpgrateInfo.slave_prepared_ack = 0;
    gstFirmwareUpgrateInfo.slave_data_ack = 0;
    gstFirmwareUpgrateInfo.slave_end_ack = 0;
    gstFirmwareUpgrateInfo.updata_send_size = 0;

//    gstFirmwareUpgrateInfo.main_ctl_data_existed = 0;

    gstFirmwareUpgrateInfo.updata_send_total_size = 0;
    //**************总长度计算**********************
    //-------BMS
    if(gstFirmwareUpgrateInfo.bms_data_existed == 1)
    {
        gstFirmwareUpgrateInfo.updata_send_total_size += gstFirmwareUpgrateInfo.bms_data_len;
    }
    //-------FPGA
    if(gstFirmwareUpgrateInfo.fpga_data_existed == 1)
    {
        gstFirmwareUpgrateInfo.updata_send_total_size += gstFirmwareUpgrateInfo.fpga_data_len;
    }
    //-------PCS
    if(gstFirmwareUpgrateInfo.pcs_existed == 1)
    {
        gstFirmwareUpgrateInfo.updata_send_total_size += gstFirmwareUpgrateInfo.pcs_len;
    }
    //-------ARC
    if(gstFirmwareUpgrateInfo.arc_existed == 1)
    {
        gstFirmwareUpgrateInfo.updata_send_total_size += gstFirmwareUpgrateInfo.arc_len;
    }
    //-------MAIN协处理器
    if(gstFirmwareUpgrateInfo.main_cop_existed == 1)
    {
        gstFirmwareUpgrateInfo.updata_send_total_size += gstFirmwareUpgrateInfo.main_cop_len;
    }
    //-------PCS协处理器
    if(gstFirmwareUpgrateInfo.pcs_cop_existed == 1)
    {
        gstFirmwareUpgrateInfo.updata_send_total_size += gstFirmwareUpgrateInfo.pcs_cop_len;
    }
    if(gstFirmwareUpgrateInfo.ui_type_img_existed == 1)
    {
        gstFirmwareUpgrateInfo.updata_send_total_size += gstFirmwareUpgrateInfo.ui_type_img_len;
    }
    if(gstFirmwareUpgrateInfo.ui_type1_font_existed == 1)
    {
        gstFirmwareUpgrateInfo.updata_send_total_size += gstFirmwareUpgrateInfo.ui_type1_font_len;
    }
    if(gstFirmwareUpgrateInfo.main_ctl_data_existed == 1)
    {
        gstFirmwareUpgrateInfo.updata_send_total_size += gstFirmwareUpgrateInfo.main_ctl_data_len;
        CalUpdataSendPersent(gstFirmwareUpgrateInfo.main_ctl_data_len);//如果含有主控数据 初始值需要直接加入。
    }

//gstFirmwareUpgrateInfo.pcs_existed = 0;
//在此处确定需要下发数据的总大小。gstFirmwareUpgrateInfo.updata_send_total_size
    uint8_t temp_flag = 0;
    if(gstFirmwareUpgrateInfo.bms_data_existed == 1)
    {

        for(uint8_t i = 0; i < GetBmsNum(); i++)
        {

            if(gstPackSlavePara.pack_para[i].dcdc_info.ctrl_info.upgrade_status == 0)
            {
                temp_flag = 1;
            }
        }
        if(temp_flag == 0)
        {
            gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.bms_type = 1;
        }
        else
        {
            //BMS 没有进入升级状态，则需要跳出升级
            gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_BMS_FLAG_ERR;
            return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
        }
    }


    if(gstFirmwareUpgrateInfo.fpga_data_existed == 1)
    {

        for(uint8_t i = 0; i < GetBmsNum(); i++)
        {

            if(gstPackSlavePara.pack_para[i].dcdc_info.ctrl_info.upgrade_status == 0)
            {
                temp_flag = 1;
            }
        }
        if(temp_flag == 0)
        {
            gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.fpga_type = 1;
        }
        else
        {
            //BMS 没有进入升级状态，则需要跳出升级
            gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_BMS_FLAG_ERR;
            return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
        }
    }

    if(gstFirmwareUpgrateInfo.pcs_existed == 1)
    {

        if(gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status == 1)
        {
            gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_type = 1;
        }
        else
        {
            //pcs 没有进入升级状态，则需要跳出升级
            gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_PCS_FLAG_ERR;
            return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
        }
    }
    if(gstFirmwareUpgrateInfo.arc_existed == 1)
    {
        if(gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status == 1)
        {
            gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.arc_type = 1;
        }
        else
        {
            //拉弧没有进入升级状态，则需要跳出升级
            gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_PCS_FLAG_ERR;
            return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
        }

    }
    if(gstFirmwareUpgrateInfo.main_cop_existed == 1)
    {
        if(gstMainCardCopInfo.ctrl_info.upgrade_status == 1)
        {
            gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.main_cop_type = 1;
        }
        else
        {   //监控卡协处理器没有进入升级状态，则需要跳出升级
            gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_MAIN_COP_FLAG_ERR;
            return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
        }
    }
    else if(gstFirmwareUpgrateInfo.pcs_cop_existed == 1)
    {
        if(gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status == 1)
        {
            gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_cop_type = 1;
        }
        else
        {   //PCS协处理器没有进入升级状态，则需要跳出升级
            gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_PCS_COP_FLAG_ERR;
            return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
        }
    }
    //------BMS
    if(gstFirmwareUpgrateInfo.bms_data_existed == 1)
    {
        gstFirmwareUpgrateInfo.bms_data_existed = 0;
        gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.bms_data_len;
        gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.bms_data_offset;
        gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_BMS_TYPE;
        gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.bms_data_checksum;
        return EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
    }
    //------DCDC
    else if(gstFirmwareUpgrateInfo.fpga_data_existed == 1)
    {
        gstFirmwareUpgrateInfo.fpga_data_existed = 0;
        gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.fpga_data_len;
        gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.fpga_data_offset;
        gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_FPGA_TYPE;
        gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.fpga_data_checksum;
        return EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
    }
    //------PCS
    else if(gstFirmwareUpgrateInfo.pcs_existed == 1)
    {
        gstFirmwareUpgrateInfo.pcs_existed = 0;
        gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.pcs_len;
        gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.pcs_data_offset;
        gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_PCS_TYPE;
        gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.pcs_checksum;
        return  EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
    }
    //------ARC
    else if(gstFirmwareUpgrateInfo.arc_existed == 1)
    {
        gstFirmwareUpgrateInfo.arc_existed = 0;
        gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.arc_len;
        gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.arc_data_offset;
        gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_ARC_TYPE;
        gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.arc_checksum;
        return  EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
    }
    //------监控卡协处理器
    else if(gstFirmwareUpgrateInfo.main_cop_existed == 1)
    {
        gstFirmwareUpgrateInfo.main_cop_existed = 0;
        gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.main_cop_len;
        gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.main_cop_data_offset;
        gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_MAIN_COP_TYPE;
        gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.main_cop_checksum;
        return  EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
    }
    //------PCS协处理器
    else if(gstFirmwareUpgrateInfo.pcs_cop_existed == 1)
    {
        gstFirmwareUpgrateInfo.pcs_cop_existed = 0;
        gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.pcs_cop_len;
        gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.pcs_cop_data_offset;
        gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_PCS_COP_TYPE;
        gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.pcs_cop_checksum;
        return EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
    }
    else if((gstFirmwareUpgrateInfo.ui_type_img_existed == 1) || (gstFirmwareUpgrateInfo.ui_type1_font_existed == 1))
    {
        return  EN_FIRMWARE_UPGRATE_STATUS_DEZIP_IMG_FONT;
    }
    else if(gstFirmwareUpgrateInfo.main_ctl_data_existed == 1)
    {
        
        return EN_FIRMWARE_UPGRATE_STATUS_MAINCTL;
    }
    else
    {
        return EN_FIRMWARE_UPGRATE_STATUS_ERROR;
    }

}

/**
  * @brief  固件校验
  * @param   None  
  * @return  None
  * @note    None
  */
static bool _MainCtlFirmwareCheck()
{
    uint8_t arr_temp[256];
    uint16_t i;
    uint32_t check_sum_calc = 0;
    uint32_t start_addr = gstFirmwareUpgrateInfo.main_ctl_data_offset + FIRMWARE_EXTERNAL_FLASH_ADDR + 256;
    uint32_t size = gstFirmwareUpgrateInfo.main_ctl_data_len - 256;

    while(size > 256)
    {
        SpiFlashReadData(start_addr, arr_temp, 256);
        size -= 256;
        start_addr += 256;
        for(i = 0; i < 256; i++)
        {
            check_sum_calc += arr_temp[i];
        }

    }
    SpiFlashReadData(start_addr, arr_temp, size);
    for(i = 0; i < size; i++)
    {
        check_sum_calc += arr_temp[i];
    }

    if(check_sum_calc != gstFirmwareUpgrateInfo.main_ctl_data_checksum)
    {
        return false;
    }

    return true;
}

/**
  * @brief   监控卡新固件接收完毕标志设置
  * @param   void
  * @return  
  * @note    None
  */
static void _MainCtlFirmwareFlagUpdate()
{
    uint8_t arr_temp[256];

    memset(arr_temp, 0x0A, 256);
    SpiFlashWriteBytesWithErase(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, arr_temp, 256);
    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR, arr_temp, 256);

}

/**
* @brief  发送开始升级指令
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
static void _SLAVEPrepareSend(uint8_t slave_type)
{
    switch(slave_type)
    {
        case UPDATA_BMS_TYPE:
            gstFirmwareUpgrateInfo.updata_send_base_addr = UPDATA_BMS_DATA_BASE_ADDR;
            break;
        case UPDATA_FPGA_TYPE:
            gstFirmwareUpgrateInfo.updata_send_base_addr = UPDATA_FPGA_DATA_BASE_ADDR;
            break;            
        case UPDATA_PCS_TYPE:
            gstFirmwareUpgrateInfo.updata_send_base_addr = UPDATA_DSP_DATA_BASE_ADDR;
            break;
        case UPDATA_ARC_TYPE:
            gstFirmwareUpgrateInfo.updata_send_base_addr = UDPDTA_ARC_DATA_BASE_ADDR;
            break;
        case UPDATA_MAIN_COP_TYPE:  //监控卡协处理器
            _UpgradePrepareUartSend(gstFirmwareUpgrateInfo.main_cop_len);
            gstFirmwareUpgrateInfo.updata_send_base_addr = UPDATA_MAIN_COP_DATA_BASE_ADDR;
            break;
        case UPDATA_PCS_COP_TYPE:   //PCS协处理器
            gstFirmwareUpgrateInfo.updata_send_base_addr = UDPDTA_PCS_COP_DATA_BASE_ADDR;
            break;
        default:
            break;
    }
}

uint32_t TestChecksum = 0;
/**
* @brief  发送升级固件
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
static void _SLAVEDataSend(uint8_t slave_type, uint16_t size, uint32_t offset, uint32_t addr)
{

    uint16_t check_sum;
    uint16_t len = 0;
    SpiFlashReadData(FIRMWARE_EXTERNAL_FLASH_ADDR + offset, &_garrUpgrateData[len], size);

    if(size != 256)
    {
        for(uint16_t i = 0; i < size; i++)
        {
            TestChecksum += _garrUpgrateData[i];
        }
    }
    else
    {
        TestChecksum = 0;
    }


    switch(slave_type)
    {
        case UPDATA_BMS_TYPE:
        case UPDATA_FPGA_TYPE:
            BroadCastByteDataToBms(addr, size, _garrUpgrateData);
            break;
        case UPDATA_PCS_TYPE:
        case UPDATA_ARC_TYPE:
        case UPDATA_PCS_COP_TYPE:   //PCS协处理器
            SendByteDataToPcs(addr, size, _garrUpgrateData);
            break;
        case UPDATA_MAIN_COP_TYPE:  //监控卡协处理器
            SendMainCopUpgradeData(gstFirmwareUpgrateInfo.firmware_updata_pack_num, size, _garrUpgrateData);
            break;
        default:
            break;
    }

}

#define ADRESSING_CMD           0X000D
static void _AddressFun(void)
{
    uint16_t check_sum;
    uint16_t size = 0x0001;
    _garrUpgrateData[0] = 0x5A;
    _garrUpgrateData[1] = 0x00;
    _garrUpgrateData[2] = 0x03;  //03
    _garrUpgrateData[3] = ADRESSING_CMD & 0xFF;
    _garrUpgrateData[4] = (ADRESSING_CMD >> 8) & 0xFF;
    _garrUpgrateData[5] = size & 0xFF;
    _garrUpgrateData[6] = (size >> 8) & 0xFF;  //数据长度是1
    _garrUpgrateData[7] = gBmsAddressingStartAddr;
    check_sum = _CheckSum(_garrUpgrateData, 8);
    _garrUpgrateData[8] = check_sum & 0xFF;
    _garrUpgrateData[9] = (check_sum >> 8) & 0xFF;
//    ModbusBMSRTUSend(_garrUpgrateData, 10);
}




/**
* @brief  发送升级结束
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
static void _SLAVEEndSend(uint8_t slave_type)
{
    uint16_t check_sum;
    _garrUpgrateData[0] = 0x5A;
    _garrUpgrateData[1] = 0x00;
    _garrUpgrateData[2] = slave_type;//0x03;
    _garrUpgrateData[3] = CMD_UPGRATE_COMPLETE & 0xFF;
    _garrUpgrateData[4] = (CMD_UPGRATE_COMPLETE >> 8) & 0xFF;
    _garrUpgrateData[5] = 0x00;
    _garrUpgrateData[6] = 0x00;

    check_sum = _CheckSum(_garrUpgrateData, 7);

    _garrUpgrateData[7] = check_sum & 0xFF;
    _garrUpgrateData[8] = (check_sum >> 8) & 0xFF;


    if(slave_type == UPDATA_MAIN_COP_TYPE)
    {
        ModbusCoprocessorRTUSend(_garrUpgrateData, 9);
    }
}




uint16_t GetUpdataPersent(void)
{
    uint32_t    temp_data = 0;
    temp_data = gstFirmwareUpgrateInfo.file_size;
    temp_data   *= 200;
    temp_data   /= gstFirmwareUpgrateInfo.file_size_read;
    if(temp_data > 200)
    {
        temp_data = 200;
    }
    return(uint16_t)temp_data;
}


uint8_t GetUpdataBaudStatusInfo(void)
{
    return gstFirmwareUpgrateInfo.enter_updata_buad_mode;
}


void SetUpdataErr(uint8_t fimeware_type)
{

    if(fimeware_type == UPDATA_PCS_TYPE)
    {
        gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_SEND_PCS_ERR;
    }
    else if(fimeware_type == UPDATA_ARC_TYPE)
    {
        gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_SEND_ARC_ERR;
    }
    else if(fimeware_type == UPDATA_BMS_TYPE)
    {
        gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_SEND_BMS_ERR;
    }
}

/**
  * @brief   设置BMS、PCS升级标志
  * @param   None
  * @return  None
  * @note    None
  */
void SetFirmareEnterUpdataMode(uint8_t bat_num)
{
    if(_gDelayOver == 1)
    {

        if(gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status == 0)
        {
            _gDelayOver = 0;
            SetPcsSysUpgradeStatus(SET_ENTER_UPGRADE_MODE);
        }
//        if(gstCollectParaData.arc_data.arc_hold_para.sys_upgrade_status == 0)
//        {
////            _gDelayOver = 0;
////            SetArcSysUpgradeStatus(SET_ENTER_UPGRADE_MODE);
//
//        }
        if(gstMainCardCopInfo.ctrl_info.upgrade_status == 0)
        {
            _gDelayOver = 0;
            SetMainCopSysUpgradeStatus(SET_ENTER_UPGRADE_MODE);
        }
        
        uint8_t temp_flag = 0;
        for(uint8_t i = 0; i < bat_num; i++)
        {
            
            if(gstPackSlavePara.pack_para[i].dcdc_info.ctrl_info.upgrade_status == 0)
            {
                temp_flag = 1;
            }

        }
        if(temp_flag == 1)
        {
            _gDelayOver = 0;
            SetBmsSysUpgradeStatus(SET_ENTER_UPGRADE_MODE);
        }
        

    }
}

/**
  * @brief  固件升级处理
  * @param   None  
  * @return  None
  * @note    None
  */
void UpgrateFirmwareProcess()
{
    static uint32_t timeout;
    static uint8_t addressing_cnt = 0;
    static uint32_t _set_sys_upgrade_delay_time = 0;
    uint8_t bat_num = 0;
    uint8_t temp_flag = 0;
    
    bat_num =  GetBmsNum();
    if(bat_num > MAX_PACK_NUM)
    {
        bat_num = MAX_PACK_NUM;
    }
    
    if((uint32_t)(tim_get_systime() - _set_sys_upgrade_delay_time) > 2000)
    {
        _set_sys_upgrade_delay_time = tim_get_systime();
        _gDelayOver = 1;
    }
    
    switch(_geStatus)
    {
        /* 空闲 */
        case EN_FIRMWARE_UPGRATE_STATUS_IDLE:
        {
            //检查是否接收到新的固件
            if(gstFirmwareUpgrateInfo.rev_start_flag == 1)
            {
                gstFirmwareUpgrateInfo.updata_send_persent = 0;
                if((uint32_t)(tim_get_systime() - _gUpgrateStartTimeOut) > UPGRATE_TIME_RECV_OUT)//UPGRATE_TIME_RECV_OUT   //将此超时时间加长，目的是防止上位机重发数据，导致超时。
                {
                    gstFirmwareUpgrateInfo.rev_start_flag = 0;
                    gstFirmwareUpgrateInfo.rev_end_flag = 0;
                    gstFirmwareUpgrateInfo.upgrading_status = 0;
                }
                //设置BMS、PCS升级标志
                SetFirmareEnterUpdataMode(bat_num);
            }
            
            //整机固件接收完成，开始解压、向下发送
            if(gstFirmwareUpgrateInfo.rev_end_flag == 1)
            {
                gstFirmwareUpgrateInfo.rev_end_flag = 0;
                timeout = tim_get_systime();
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_DEZIP;
            }
            gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_NO_TYPE;

            //自测试升级使用
            if(flag == 1)
            {
                flag = 0;
                //设置BMS、PCS升级标志
                SetFirmareEnterUpdataMode(bat_num);
                gstFirmwareUpgrateInfo.upgrading_status = 1;
                timeout = tim_get_systime();
                _gZipDleyTimeCmt = 10000; //延迟10s开始解压
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_DEZIP;
                gstFirmwareUpgrateInfo.updata_get_total_size = 59535;//522828;
//                gstFirmwareUpgrateInfo.updata_get_total_size = 25636;
            }

            if((gstFirmwareUpgrateInfo.rev_start_flag == 1) || (gstFirmwareUpgrateInfo.rev_end_flag == 1))
            {
                //进入升级波特率模式
                gstFirmwareUpgrateInfo.enter_updata_buad_mode = 1; 
            }
            else
            {
                //退出升级波特率模式
                gstFirmwareUpgrateInfo.enter_updata_buad_mode = 0; 

                if(_gDelayOver == 1)
                {
                    //需要给固件发送升级退出标志  每3S发送一次
                    if(gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status != 0)
                    {
                        _gDelayOver = 0;
                        SetPcsSysUpgradeStatus(SET_EXIT_UPGRADE_MODE);
                    }
//                    if(gstCollectParaData.arc_data.arc_hold_para.sys_upgrade_status == 1)
//                    {
//                        _gDelayOver = 0;
//                        SetArcSysUpgradeStatus(SET_EXIT_UPGRADE_MODE);
//
//                    }
                    if(gstMainCardCopInfo.ctrl_info.upgrade_status != 0)
                    {
                        _gDelayOver = 0;
                        SetMainCopSysUpgradeStatus(SET_EXIT_UPGRADE_MODE);
                    }
                    for(uint8_t i = 0; i < bat_num; i++)
                    {
                        uint8_t temp_flag = 0;
                        if(gstPackSlavePara.pack_para[i].dcdc_info.ctrl_info.upgrade_status != 0)
                        {
                            temp_flag = 1;
                        }
                        if(temp_flag == 1)
                        {
                            _gDelayOver = 0;
                            SetBmsSysUpgradeStatus(SET_EXIT_UPGRADE_MODE);
                        }
                    }
//                    if(gstCollectParaData.bms_data[0].bms_hold_para.sys_upgrade_status == 1)
//                    {
//                        _gDelayOver = 0;
//                        SetBmsSysUpgradeStatus(SET_EXIT_UPGRADE_MODE);
//                    }
                }
            }

            //编址程序已经执行完，并且编址失败，设置故障位
            if((BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_ok == 0)&&(BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_over ==1))
            {
                gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_ADRESS_ERR;
            }
            break;
        }
        /* 解压 */
        case EN_FIRMWARE_UPGRATE_STATUS_DEZIP:
        {

            SetFirmareEnterUpdataMode(bat_num);
            gstFirmwareUpgrateInfo.updata_get_persent = 100;
            if((uint32_t)(tim_get_systime() - timeout) < _gZipDleyTimeCmt)
            {
                //延时10S 确保已经通知到其他固件进入升级
                break;
            }
            timeout = tim_get_systime();
            gstFirmwareUpgrateInfo.rev_start_flag = 0;
            gstFirmwareUpgrateInfo.write_addr = FIRMWARE_EXTERNAL_FLASH_ADDR;
            uint32_t Fimrware_addr = 0;
            for(uint16_t i = 0; i < DEZIP_DATA_MAX_SECTOR_SIZE; i++) //4M
            {
                Fimrware_addr = FIRMWARE_EXTERNAL_FLASH_ADDR + (i * 0x1000);
                SpiFlashEraseSector(Fimrware_addr);
            }
            gstFirmwareUpgrateInfo.updata_dezip_file_size = 0;  //先清空，在解压完成的时候，可以获取当前解压之后的文件大小
            if(UpdataDeZip(FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR, gstFirmwareUpgrateInfo.updata_get_total_size) == MZ_TRUE)
            {
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_CHECK;
                StorgDezipFileSize(gstFirmwareUpgrateInfo.updata_dezip_file_size);
                if(gstFirmwareUpgrateInfo.updata_dezip_file_size != GetDezipFileSize())
                {
                    gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_ZIP_ERR;
                    _geStatus = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                }
            }
            else
            {
                gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_ZIP_ERR;
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
            }
            break;
        }
        case EN_FIRMWARE_UPGRATE_STATUS_CHECK:
        {
            gstFirmwareUpgrateInfo.updata_dezip_file_size = GetDezipFileSize();
            gstFirmwareUpgrateInfo.updata_send_persent = 0;
            if(_FirmwareCheck() == false)
            {
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_CHECK_ERR;
            }
            else
            {
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_TO_DO;
            }
            break;
        }
        /* 判断待升级固件类型 */
        case EN_FIRMWARE_UPGRATE_STATUS_TO_DO:
        {
            _geStatus = _FirmwareAnalysis();
            timeout = tim_get_systime();
            break;
        }
        /*增加解析图片和字符数据*/
        case EN_FIRMWARE_UPGRATE_STATUS_DEZIP_IMG_FONT:
        {
            if(gstFirmwareUpgrateInfo.ui_type_img_existed == 1)
            {
                gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_UI_TYPE;
                if((uint32_t)(tim_get_systime() - timeout) < 200)
                {
                    break;
                }
                timeout = tim_get_systime();
                gstFirmwareUpgrateInfo.ui_type_img_existed = 0;
                UpdateIcon();

                CalUpdataSendPersent(gstFirmwareUpgrateInfo.ui_type_img_len);
            }
            else if(gstFirmwareUpgrateInfo.ui_type1_font_existed == 1)
            {

                gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_UI_TYPE1;
                if((uint32_t)(tim_get_systime() - timeout) < 200)
                {
                    //目的是给UI进行刷新进度
                    break;
                }
                timeout = tim_get_systime();
                gstFirmwareUpgrateInfo.ui_type1_font_existed = 0;
                UpdateFont();
                CalUpdataSendPersent(gstFirmwareUpgrateInfo.ui_type1_font_len);
            }
            else if(gstFirmwareUpgrateInfo.main_ctl_data_existed == 1)
            {

                timeout = tim_get_systime();
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_MAINCTL;
                gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_MAIN_TYPE;
            }
            else
            {
                timeout = tim_get_systime();
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_OK;
            }


            break;
        }
        /* 发送升级准备 */
        case EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE:
        {
            if((uint32_t)(tim_get_systime() - timeout) < 3000)
            {
                break;
            }
            timeout = (uint32_t)(tim_get_systime());
//开始写升级中
//延迟1S开始升级，确保不受正常通讯数据影响
            gstFirmwareUpgrateInfo.slave_prepared_ack = 0;
            _SLAVEPrepareSend(gstFirmwareUpgrateInfo.updata_slave_type);
            _geStatus = EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_PREPARE_ACK;
            gstFirmwareUpgrateInfo.firmware_updata_pack_num = 0;
            timeout = tim_get_systime();

            break;
        }
        /* 等待升级准备应答 */
        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_PREPARE_ACK:
        {
            if(gstFirmwareUpgrateInfo.updata_slave_type == UPDATA_MAIN_COP_TYPE)
            {
                if((uint32_t)(tim_get_systime() - timeout) > 3000)
                {
                    timeout = (uint32_t)tim_get_systime();
                    gstFirmwareUpgrateInfo.re_send_cnt++;
                    if(gstFirmwareUpgrateInfo.re_send_cnt > 3)
                    {
                        gstFirmwareUpgrateInfo.re_send_cnt = 0;
                        _geStatus = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                        SetUpdataErr(gstFirmwareUpgrateInfo.updata_slave_type);
                    }
                    else
                    {
                        _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
                    }
                }
                if(gstFirmwareUpgrateInfo.slave_prepared_ack != 0)
                {
                    timeout = (uint32_t)(tim_get_systime());
                    gstFirmwareUpgrateInfo.firmware_updata_pack_num = 0;
                    if(gstFirmwareUpgrateInfo.slave_prepared_ack == 1)
                    {//接收OK
                        gstFirmwareUpgrateInfo.re_send_cnt = 0;
                        _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_DATA;
                    }
                    else if(gstFirmwareUpgrateInfo.slave_prepared_ack == 2)
                    {//接收错误
                        gstFirmwareUpgrateInfo.re_send_cnt++;
                        if(gstFirmwareUpgrateInfo.re_send_cnt > 3)
                        {
                            _geStatus = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                            SetUpdataErr(gstFirmwareUpgrateInfo.updata_slave_type);                        
                        }
                        else
                        {
                            _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
                        }
                    }
                    else
                    {//说明此时固件一样，不用再进行升级，直接发送升级完成标志。
                        _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_END;
                    }
                }
            }
            else
            {
                 _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_DATA;
            }
            break;
        }
        /* 发送升级固件 */
        case EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_DATA:
        {
            uint16_t fimware_per_size = 0;
            if((uint32_t)(tim_get_systime() - timeout) < 100)//50
            {
                break;
            }
            if(gstFirmwareUpgrateInfo.firmware_updata_pack_num == 0)
            {
                //第一包数据发送256字节 是固件头
                fimware_per_size = SLAVE_UPGRATE_DATA_SIZE_256;
            }
            else
            {
                if(gstFirmwareUpgrateInfo.updata_slave_type == UPDATA_MAIN_COP_TYPE)
                {
                    fimware_per_size = SLAVE_UPGRATE_DATA_SIZE_2048;
                }
                else if(gstFirmwareUpgrateInfo.updata_slave_type == UPDATA_FPGA_TYPE ||
                        gstFirmwareUpgrateInfo.updata_slave_type == UPDATA_BMS_TYPE)
                {
                    fimware_per_size = SLAVE_UPGRATE_DATA_SIZE_768;
                }
                else
                {
                    fimware_per_size = SLAVE_UPGRATE_DATA_SIZE_1024;
                }
            }
            if(gstFirmwareUpgrateInfo.slave_len - gstFirmwareUpgrateInfo.slave_seek < fimware_per_size)
            {
                gstFirmwareUpgrateInfo.size_per_transfer = gstFirmwareUpgrateInfo.slave_len - gstFirmwareUpgrateInfo.slave_seek;
            }
            else
            {
                gstFirmwareUpgrateInfo.size_per_transfer = fimware_per_size;
            }

            _SLAVEDataSend(gstFirmwareUpgrateInfo.updata_slave_type, gstFirmwareUpgrateInfo.size_per_transfer, \
                           gstFirmwareUpgrateInfo.slave_data_offset + gstFirmwareUpgrateInfo.slave_seek, gstFirmwareUpgrateInfo.updata_send_base_addr);

            gstFirmwareUpgrateInfo.slave_seek += gstFirmwareUpgrateInfo.size_per_transfer;


            gstFirmwareUpgrateInfo.updata_send_base_addr += gstFirmwareUpgrateInfo.size_per_transfer;
            gstFirmwareUpgrateInfo.firmware_updata_pack_num++;
            timeout = tim_get_systime();
            _geStatus = EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_DATA_ACK;
            gstFirmwareUpgrateInfo.slave_data_ack = 0;

            break;
        }
        /* 等待固件应答 */
        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_DATA_ACK:
        {
            timeout = (uint32_t)(tim_get_systime());
            //用于计算分发百分比
            CalUpdataSendPersent(gstFirmwareUpgrateInfo.size_per_transfer);                
            if(gstFirmwareUpgrateInfo.slave_seek >= gstFirmwareUpgrateInfo.slave_len)
            {
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_END;
            }
            else
            {
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_DATA;
            }
            break;
        }
        /* 发送升级结束 */
        case EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_END:
        {
            if(tim_difftime_from_now(timeout) > 1000)
            {
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_END_ACK;
                //需要判定是哪个固件升级完成
                _SLAVEEndSend(gstFirmwareUpgrateInfo.updata_slave_type);
                timeout = tim_get_systime();
            }
            break;
        }
        /* 等待升级结束应答 */
        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_SLAVE_END_ACK:
        {
            timeout = tim_get_systime();
            gstFirmwareUpgrateInfo.slave_seek = 0;
            gstFirmwareUpgrateInfo.slave_prepared_ack = 0;
            gstFirmwareUpgrateInfo.slave_data_ack = 0;
            gstFirmwareUpgrateInfo.slave_end_ack = 0;
//            OpenDspUdpConnect();
            if(gstFirmwareUpgrateInfo.bms_data_existed == 1)
            {
                gstFirmwareUpgrateInfo.bms_data_existed = 0;
                gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.bms_data_len;
                gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.bms_data_offset;
                gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_BMS_TYPE;
                gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.bms_data_checksum;
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
                break;
            }
            else if(gstFirmwareUpgrateInfo.fpga_data_existed == 1)
            {
                gstFirmwareUpgrateInfo.fpga_data_existed = 0;
                gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.fpga_data_len;
                gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.fpga_data_offset;
                gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_FPGA_TYPE;
                gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.fpga_data_checksum;
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
//                ColseDspUdpConnect();
                break;
            }
            else if(gstFirmwareUpgrateInfo.pcs_existed == 1)
            {
                gstFirmwareUpgrateInfo.pcs_existed = 0;
                gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.pcs_len;
                gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.pcs_data_offset;
                gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_PCS_TYPE;
                gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.pcs_checksum;
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
            }
            else if(gstFirmwareUpgrateInfo.arc_existed == 1)
            {
                gstFirmwareUpgrateInfo.arc_existed = 0;
                gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.arc_len;
                gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.arc_data_offset;
                gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_ARC_TYPE;
                gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.arc_checksum;
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;

            }
            else if(gstFirmwareUpgrateInfo.main_cop_existed == 1)
            {
                gstFirmwareUpgrateInfo.main_cop_existed = 0;
                gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.main_cop_len;
                gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.main_cop_data_offset;
                gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_MAIN_COP_TYPE;
                gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.main_cop_checksum;
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
            }
            else if(gstFirmwareUpgrateInfo.pcs_cop_existed == 1)
            {
                gstFirmwareUpgrateInfo.pcs_cop_existed = 0;
                gstFirmwareUpgrateInfo.slave_len = gstFirmwareUpgrateInfo.pcs_cop_len;
                gstFirmwareUpgrateInfo.slave_data_offset = gstFirmwareUpgrateInfo.pcs_cop_data_offset;
                gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_PCS_COP_TYPE;
                gstFirmwareUpgrateInfo.slave_checksum = gstFirmwareUpgrateInfo.pcs_cop_checksum;
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_SEND_SLAVE_PREPARE;
            }
            else if((gstFirmwareUpgrateInfo.ui_type_img_existed == 1) || (gstFirmwareUpgrateInfo.ui_type1_font_existed == 1))
            {

                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_DEZIP_IMG_FONT;
            }
            else
            {
                if(gstFirmwareUpgrateInfo.main_ctl_data_existed == 1)
                {
                    _geStatus = EN_FIRMWARE_UPGRATE_STATUS_MAINCTL;
                }
                else
                {
                    _geStatus = EN_FIRMWARE_UPGRATE_STATUS_OK;
                }
            }

            break;
        }
        /* 固件校验 */
        case EN_FIRMWARE_UPGRATE_STATUS_MAINCTL:
        {
            /*校验数据，如无误，则重启进入BOOT进行升级*/
            timeout = tim_get_systime();
            if(_MainCtlFirmwareCheck() == false)
            {
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_MAIN_CHECK_ERR;
            }
            else
            {
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_OK;
            }
            break;
        }
        /* 错误处理 */
        case EN_FIRMWARE_UPGRATE_STATUS_ERROR:
        {
            gstFirmwareUpgrateInfo.upgrading_status = 0;
            gstFirmwareUpgrateInfo.rev_start_flag = 0;
            gstFirmwareUpgrateInfo.upgrate_end_flag = 2;
            gstFirmwareUpgrateInfo.rev_end_flag = 0;
            _geStatus = EN_FIRMWARE_UPGRATE_STATUS_IDLE;
            break;
        }
        /* 固件下发成功 */
        case EN_FIRMWARE_UPGRATE_STATUS_OK:
        {

//此处需要延迟 以便有足够时间发送升级100信息给上位机
            gstFirmwareUpgrateInfo.re_send_cnt = 0;
            if((uint32_t)(tim_get_systime() - timeout) > 5000)  //5S延时
            {
                gstFirmwareUpgrateInfo.upgrate_end_flag = 1;
                gstFirmwareUpgrateInfo.rev_end_flag = 0;
                gstFirmwareUpgrateInfo.updata_slave_type = UPDATA_NO_TYPE;
                if(gstFirmwareUpgrateInfo.main_ctl_data_existed == 1)
                {
                    /*更新外部FLASH标志 FIRMWARE_EXTERNAL_FLASH_FLAG_ADDR*/
                    _MainCtlFirmwareFlagUpdate();
                    gstFirmwareUpgrateInfo.updata_send_persent = 100;

                    /*需要重启*/
                    _geStatus = EN_FIRMWARE_UPGRATE_STATUS_WAIT_RESTART;
                    timeout = tim_get_systime();
                }
                else
                {
                    _geStatus = EN_FIRMWARE_UPGRATE_STATUS_WAIT_RESTART;//EN_FIRMWARE_UPGRATE_STATUS_IDLE;
                }
            }

            break;
        }
        /* 等待重启 */
        case EN_FIRMWARE_UPGRATE_STATUS_WAIT_RESTART:
        {

            if((uint32_t)(tim_get_systime() - timeout) > UPGRATE_RESTART_TIME_OUT)
            {
                timeout = tim_get_systime();
//                /*重启*/
//                __set_FAULTMASK(1);
//                NVIC_SystemReset();
                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_RESET;
//存储版本
//                gstCollectInfo.maincard_data.base_info.sw_ver = _gUpdataPackVerBuf;         //?????更新版本号
                DeviceCtlWrite();           //写入版本号。
            }
            gstFirmwareUpgrateInfo.updata_reset_status = EN_FIRMWARE_RESET_START;
            break;
        }
        /* 开始重启 */
        case EN_FIRMWARE_UPGRATE_STATUS_RESET:
        {
            //给各个固件下发固件下发完成
            ////for循环 获取每个包的升级状态， 确保每个包都升级OK  否则2分钟超时退出升级
            //如果每个包都升级OK  则需要给从第二包和DSP开始下发复位命令  延时20S
            //给第一包发送复位命令，延时20S
            //复位监控卡。
            uint8_t check_flag = 0;
            switch(gstFirmwareUpgrateInfo.updata_reset_status)
            {
                /* 开始重启 */
                case EN_FIRMWARE_RESET_START:
                {

                    if((uint32_t)(tim_get_systime() - timeout) < 1000)
                    {
                        break;
                    }
                    timeout = tim_get_systime();
                    check_flag = 0; 
                    //没有固件
                    if((gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_type  == 0) &&   \
                       (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.arc_type  == 0) &&  \
                       (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.fpga_type == 0) && \
                       (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.bms_type  == 0) &&  \
                       (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.main_cop_type == 0) && \
                       (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_cop_type  == 0))
                    {
                        if(gstFirmwareUpgrateInfo.main_ctl_data_existed == 1)
                        {
                            gstFirmwareUpgrateInfo.updata_reset_status = EN_MOINTOR_RESET;
                        }
                        else
                        {
                            gstFirmwareUpgrateInfo.updata_reset_status = EN_MOINTOR_RESET_OVER;
                        }
                    }
                    else
                    {
                        //有PCS及ARC固件
                        if((gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_type == 1) || \
                           (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.arc_type == 1) || \
                           (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_cop_type == 1))
                        {
                            //未检测到固件下发完成标志
                            if((gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status & FIRMWARE_SEND_OVER_BIT_MASK) == 0)
                            {
                                //升级标志重新设置
                                check_flag = 1;
                                if(gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_type == 1)
                                {
                                    gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status |= PCS_FIRMWARE_BIT_MISK;
                                }
                                if(gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.arc_type == 1)
                                {
                                    gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status |= ARC_FIRMWARE_BIT_MASK;
                                }
                                if(gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_cop_type == 1)
                                {
                                    gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status |= PCS_COP_FIRMWARE_BIT_MASK;
                                }
                                gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status |= FIRMWARE_SEND_OVER_BIT_MASK;
                                SetPcsSysUpgradeStatus(gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status);
                            }
                        }
                        //有BMS及DCDC固件
                        if((gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.bms_type  == 1) ||     \
                           (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.fpga_type == 1))
                        {
                            //未检测到固件下发完成标志
                            for(uint8_t i = 0; i < bat_num; i++)
                            {
                                if((gstPackSlavePara.pack_para[i].dcdc_info.ctrl_info.upgrade_status & FIRMWARE_SEND_OVER_BIT_MASK) == 0)  //下位机已经升级完成
                                {
                                    //固件下发完成标志重新设置
                                    check_flag = 1;

                                    if(gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.bms_type == 1)
                                    {
                                        gstPackSlavePara.pack_para[0].dcdc_info.ctrl_info.upgrade_status |= BMS_FIRMWARE_BIT_MASK;
                                    }
                                    if(gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.fpga_type == 1)
                                    {
                                        gstPackSlavePara.pack_para[0].dcdc_info.ctrl_info.upgrade_status |= FPGA_FIRMWARE_BIT_MASK;
                                    }
                                    gstPackSlavePara.pack_para[0].dcdc_info.ctrl_info.upgrade_status |= FIRMWARE_SEND_OVER_BIT_MASK;
                                    SetBmsSysUpgradeStatus(gstPackSlavePara.pack_para[0].dcdc_info.ctrl_info.upgrade_status);
                                    break;
                                }
                            }
                        }
                        
                        //有协处理器固件
                        if(gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.main_cop_type == 1)
                        {
                            //未检测到固件下发完成标志
                            if((gstMainCardCopInfo.ctrl_info.upgrade_status & FIRMWARE_SEND_OVER_BIT_MASK) == 0)
                            {
                                //升级标志重新设置
                                check_flag = 1;
                                gstMainCardCopInfo.ctrl_info.upgrade_status |= MAIN_COP_FIRMWARE_BIT_MASK;
                                gstMainCardCopInfo.ctrl_info.upgrade_status |= FIRMWARE_SEND_OVER_BIT_MASK;
                                SetMainCopSysUpgradeStatus(gstMainCardCopInfo.ctrl_info.upgrade_status);
                            }
                        } 
                        
                        //固件下发完成标志重发
                        if(check_flag == 1)
                        {
                            if(++gstFirmwareUpgrateInfo.re_send_cnt > 3)
                            {
                                gstFirmwareUpgrateInfo.re_send_cnt = 0;
                                gstFirmwareUpgrateInfo.updata_reset_status = EN_MOINTOR_RESET_OVER;
                            }
                        }
                        else
                        {
                            //固件下发完成标志正常
                            gstFirmwareUpgrateInfo.re_send_cnt = 0;
                            gstFirmwareUpgrateInfo.updata_reset_status = EN_GET_FIRMWARE_RESET_STATUS;
                        }
                    }
                    break;
                }
//                case EN_CHECK_SEND_OVER_BIT:
//                {

//                    if((uint32_t)(tim_get_systime() - timeout) < 500)
//                    {
//                        break;
//                    }
//                    timeout = tim_get_systime();
//                    check_flag = 0;
//                    if(gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_type == 1)
//                    {
//                        if((gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status & FIRMWARE_SEND_OVER_BIT_MASK) == 0)
//                        {
//                            check_flag =1;
//
//                        }
//                    }
//                    if(gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.bms_type ==1)
//                    {
//                        for(uint8_t i =0;i<bat_num;i++)
//                        {
//                            if((PackInfo[i].dcdc_info.upgrade_status & FIRMWARE_SEND_OVER_BIT_MASK) == 0)  //下位机已经升级完成
//                            {//已经升级完成
//                                check_flag =1;
//                            }
//                        }
//                    }
//                    if(check_flag == 1)
//                    {
//                        if(++gstFirmwareUpgrateInfo.re_send_cnt>3)
//                        {
//                            gstFirmwareUpgrateInfo.re_send_cnt = 0;
//                            gstFirmwareUpgrateInfo.updata_reset_status = EN_MOINTOR_RESET_OVER;
//                        }
//                        else
//                        {
//                            gstFirmwareUpgrateInfo.updata_reset_status =   EN_FIRMWARE_RESET_START;
//                        }
//                    }
//                    else
//                    {
//                        gstFirmwareUpgrateInfo.re_send_cnt = 0;
//                        gstFirmwareUpgrateInfo.updata_reset_status = EN_GET_FIRMWARE_RESET_STATUS;
//                    }
//                    break;
//                }
                /* 获取重启状态 */
                case EN_GET_FIRMWARE_RESET_STATUS:
                {
                    //固件是否接收成功。
                    temp_flag = 0;
                    if((gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_type == 1) || 
                        (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.arc_type == 1) || 
                        (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_cop_type == 1))
                    {
                        //PCS固件接收是否完成
                        if((gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status & GET_FIMWARE_STATUS_BIT_MASK) != 0x2000)
                        {
                            if((gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status & GET_FIMWARE_STATUS_BIT_MASK) == 0x4000)
                            {
                                gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_SEND_PCS_ERR;
                                _geStatus = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                                break;
                            }
                            else
                            {
                                temp_flag = 1; 
                            }
                        }
                    }
                    //BMS及FPGA固件接收是否完成
                    if((gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.bms_type  == 1) ||     \
                       (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.fpga_type == 1))
                    {
                        for(uint8_t i = 0; i < bat_num; i++)
                        {
                            //判断固件是否接收完成（13,14位为升级位，00：接收中，01：接收成功，10：接收失败）
                            if((gstPackSlavePara.pack_para[i].dcdc_info.ctrl_info.upgrade_status & GET_FIMWARE_STATUS_BIT_MASK) != 0x2000)  //下位机已经升级完成
                            {
                                //未接收完成
                                if((gstPackSlavePara.pack_para[i].dcdc_info.ctrl_info.upgrade_status & GET_FIMWARE_STATUS_BIT_MASK) == 0x4000)
                                {
                                    gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_SEND_BMS_ERR;
                                    _geStatus = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                                    break;
                                }
                                else
                                {
                                    temp_flag = 1;
                                }
                            }
                        }
                    }
                    
                    //协处理器固件接收是否完成
                        if(gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.main_cop_type == 1)
                        {
                            //PCS固件接收是否完成
                            if((gstMainCardCopInfo.ctrl_info.upgrade_status & GET_FIMWARE_STATUS_BIT_MASK) != 0x2000)
                            {
                                if((gstMainCardCopInfo.ctrl_info.upgrade_status & GET_FIMWARE_STATUS_BIT_MASK) == 0x4000)
                                {
                                    gstFirmwareUpgrateInfo.upgrad_err_code |= UPGRADE_SEND_MAIN_COP_ERR;
                                    _geStatus = EN_FIRMWARE_UPGRATE_STATUS_ERROR;
                                    break;
                                }
                                else
                                {
                                    temp_flag = 1; 
                                }
                            }
                        } 
                        
                        
                    //接收完成，开始重启
                    if(temp_flag == 0)
                    {
//                        if(bat_num <=1)
//                        {
//                            gstFirmwareUpgrateInfo.updata_reset_status = EN_SEND_FIRST_BMS_RESET;
//                        }
//                        else
//                        {
                            gstFirmwareUpgrateInfo.updata_reset_status = EN_SEND_BMS_PCS_RESET_CMD;
//                        }
                        
                    }
                    
                    //超时1200S 确保下位机校验完成，并且给BMS发送完成
                    if((uint32_t)(tim_get_systime() - timeout) > (WAIT_FIRMWARE_UPDATA_DELAY * 10))
                    {
                        
                        timeout = tim_get_systime();
                        gstFirmwareUpgrateInfo.updata_reset_status = EN_MOINTOR_RESET_OVER;
                    }
                    break;
                }
                /* 发送重启指令 */
                case EN_SEND_BMS_PCS_RESET_CMD:
                {
                    if((uint32_t)(tim_get_systime() - timeout) < 1000)
                    {
                        break;
                    }
                    timeout = tim_get_systime();
                    
                    //使能PCS复位
                    if((gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_type == 1) || \
                       (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.arc_type == 1) || \
                       (gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.pcs_cop_type == 1))
                    {
                        gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status |= REST_EN_MASK_BIT;
                        SetPcsSysUpgradeStatus(gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status);
                    }
                    
                    //使能第二个pack 到最后一个 BMS复位
                    gstPackSlavePara.pack_para[0].dcdc_info.ctrl_info.upgrade_status |= REST_EN_MASK_BIT;
                    SetBmsSysUpgradeStatus(gstPackSlavePara.pack_para[0].dcdc_info.ctrl_info.upgrade_status);
                    //监控卡协处理器
                    if(gstFirmwareUpgrateInfo.sys_firmware_packet_type.bits.main_cop_type == 1)
                    {
                        gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status |= REST_EN_MASK_BIT;
                        SetMainCopSysUpgradeStatus(gstUdpPcsSlavePara.pcs_info.control_info.upgrade_status);
                    }

                    if(gstFirmwareUpgrateInfo.re_send_cnt++ > 3)
                    {
                        gstFirmwareUpgrateInfo.re_send_cnt = 0;
                        gstFirmwareUpgrateInfo.updata_reset_status = EN_WAIT_PCS_RESET_DELAY;

                    }
                    timeout = tim_get_systime();
                    break;
                }
                /* 等待PCS及BMS(除第一个BMS)重启 */ 
                case EN_WAIT_PCS_RESET_DELAY:
                {
                    static uint32_t send_time = 0;
                    //延时20S
                    if((uint32_t)(tim_get_systime() - timeout) < 25000)
                    {   //每隔5s广播一次PCS类型
                        if((uint32_t)(tim_get_systime() - send_time) > 5000)
                        {
                            SetDcdcPcsType(gstPackSlavePara.pack_para[0].dcdc_info.status_info.pcs_type);
                            send_time = tim_get_systime();
                        }
                        break;
                    }
                    else
                    {
                        //超时
                        gstFirmwareUpgrateInfo.updata_reset_status = EN_SEND_FIRST_BMS_RESET;
                    }
                    timeout = tim_get_systime();
                    break;
                }
                /* 第一个BMS重启 */
                case EN_SEND_FIRST_BMS_RESET:
                {
                    //使能第一个包复位
                    if((uint32_t)(tim_get_systime() - timeout) < 1000)
                    {
                        break;
                    }
                    
                    timeout = tim_get_systime();
                    gstPackSlavePara.pack_para[0].dcdc_info.ctrl_info.upgrade_status |= REST_EN_FIRST_PACK_BIT_MASK;
                    SetFirstPackRest(gstPackSlavePara.pack_para[0].dcdc_info.ctrl_info.upgrade_status);
                    if(gstFirmwareUpgrateInfo.re_send_cnt++ > 3)
                    {
                        gstFirmwareUpgrateInfo.re_send_cnt = 0;
                        gstFirmwareUpgrateInfo.updata_reset_status = EN_WAIT_FIRST_BMS_RESET;

                    }
                    break;
                }
                /* 等待BMS重启 */
                case EN_WAIT_FIRST_BMS_RESET:
                {
                    //延时20S

                    if((uint32_t)(tim_get_systime() - timeout) > 25000)
                    {
                        //超时
                        timeout = tim_get_systime();
                        gstFirmwareUpgrateInfo.updata_reset_status = EN_WAIT_MAIN_COP_RESET;
                    }
                    
                    
                    break;
                }
                /* 等待监控卡协处理器重启 */
                case EN_WAIT_MAIN_COP_RESET:
                {

                    if((uint32_t)(tim_get_systime() - timeout) > 1000)
                    {
                        //超时
                        timeout = tim_get_systime();
                        gstMainCardCopInfo.ctrl_info.upgrade_status |= REST_EN_MASK_BIT;
                        SetMainCopSysUpgradeStatus(gstMainCardCopInfo.ctrl_info.upgrade_status);
                        gstFirmwareUpgrateInfo.updata_reset_status = EN_MOINTOR_RESET;
                    } 
                    
                    break;
                }
                /* 监控卡重启 */
                case EN_MOINTOR_RESET:
                {
                    //延时2S后监控卡自杀
                    if((uint32_t)(tim_get_systime() - timeout) < 5000)
                    {
                        break;
                    } 
                    /*重启*/
                    gstFirmwareUpgrateInfo.upgrading_status = 0;
                    __set_FAULTMASK(1);
                    NVIC_SystemReset();

                    break;
                }
                /* 发送完成 */
                case EN_MOINTOR_RESET_OVER:
                {
                    //退出升级
                    gstFirmwareUpgrateInfo.upgrading_status = 0;
                    gstFirmwareUpgrateInfo.updata_reset_status = EN_FIRMWARE_RESET_START;
                    break;
                }
                default:
                    break;
            }


            break;
        }
        default:
            break;
    }


    gstCollectInfo.maincard_data.sys_info.upgrade_state = 0;
    gstCollectInfo.maincard_data.sys_info.upgrade_state = gstFirmwareUpgrateInfo.updata_slave_type;
    gstCollectInfo.maincard_data.sys_info.upgrade_state <<= 8;
    gstCollectInfo.maincard_data.sys_info.upgrade_state += gstFirmwareUpgrateInfo.updata_send_persent;
    if((_geStatus <= EN_FIRMWARE_UPGRATE_STATUS_WAIT_RESTART) && (_geStatus > EN_FIRMWARE_UPGRATE_STATUS_IDLE))
    {
        //此时处于下发固件阶段
        gstCollectInfo.maincard_data.sys_info.upgrade_state |= 0x0080;
    }
    else
    {
        gstCollectInfo.maincard_data.sys_info.upgrade_state &= 0xff7f;
    }
}

void    FirmwareUpgrateInit(void)
{
    RegisterSlaveUpdataFunPoint(_RevSlaveUpdataAnalysis);
}
uint8_t GetAdressStatus(void)
{

    uint8_t satus = 0;
    if((gstFirmwareUpgrateInfo.upgrad_err_code & UPGRADE_ADRESS_ERR) != 0)
    {
        //编制失败
        satus = 1;
    }
    else
    {
        satus = 0;
    }
    return satus;
}

uint8_t  GetUpdataStatus(void)
{
    uint8_t upgrate_status = 0;
    if(((_geStatus <= EN_FIRMWARE_UPGRATE_STATUS_WAIT_RESTART) && (_geStatus > EN_FIRMWARE_UPGRATE_STATUS_IDLE)) || (gstFirmwareUpgrateInfo.rev_start_flag == 1))
    {
        upgrate_status = 1;
    }
    return (uint8_t)upgrate_status;
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
    uint32_t spi_addr = FIRMWARE_EXTERNAL_FLASH_ZIP_DATA_ADDR;//(4096*4096 + 727 * 4096);
    SpiFlashReadData(spi_addr + file_ofs, pBuf, s);

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
    mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
    MyZipCtx_t *my_zip_ctx = (MyZipCtx_t *)pZip->userdata;

    SpiFlashWriteBytesWithoutErasenoAligned(gstFirmwareUpgrateInfo.write_addr, (uint8_t *)pBuf, n);
    gstFirmwareUpgrateInfo.write_addr += n;

    gstFirmwareUpgrateInfo.updata_dezip_file_size += n;
//    test_zip_file_size +=n;
    return n;
}


int32_t UpdataDeZip(uint32_t spi_base_addr, uint32_t file_size)
{
    uint32_t spi_addr = spi_base_addr;//(4096*4096 + 727 * 4096);

    static uint8_t __SDRAM write_buf[TINFL_LZ_DICT_SIZE] = {0};

    uint8_t s_inbuf[BUF_SIZE];
    uint8_t s_outbuf[BUF_SIZE];

    mz_bool status;
    mz_bool success = MZ_TRUE;
//    mz_zip_error zip_err;
    MyZipCtx_t my_zip_ctx = {0};

    mz_zip_zero_struct(&my_zip_ctx.zip_ctx);
    my_zip_ctx.zip_size = file_size;//61663;
    my_zip_ctx.zip_flags = MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY;

    my_zip_ctx.zip_ctx.userdata = (void *)&my_zip_ctx;

//  uint32_t cost_time[3][2] = {{0}};

//  uint32_t file_ofs = 0, file_read = 0;
//  while (file_ofs < my_zip_ctx.zip_size)
//  {
//      file_read = sizeof(s_outbuf);
//      if (my_zip_ctx.zip_size - file_ofs < file_read)
//      {
//          file_read = my_zip_ctx.zip_size - file_ofs;
//      }
//      // read data from spi flash
//      SpiFlashReadData(spi_addr+file_ofs, s_outbuf, file_read);
//      file_ofs += file_read;
//  }
//  cost_time[0][0] = tim_get_systime();

    // init zip
    if (!mz_zip_reader_init_cb(&my_zip_ctx.zip_ctx, NULL, my_zip_ctx.zip_size,
                               my_zip_ctx.zip_flags, mz_zip_read_func))
    {
//        zip_err = my_zip_ctx.zip_ctx.m_last_error;
        return MZ_FALSE;
    }

//  char file_name[256] = {"icon_upg_Roca_G2_sub_0_20240219.bin"};
//  snprintf(my_zip_ctx.file_name, sizeof(my_zip_ctx.file_name), "%s", file_name);
//  if (!mz_zip_reader_locate_file_v2(&my_zip_ctx.zip_ctx, my_zip_ctx.file_name, NULL,
//                                    my_zip_ctx.zip_flags, &my_zip_ctx.file_idx))
//  {
//      return MZ_FALSE;
//  }

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
//  cost_time[0][1] = tim_get_systime();
    return MZ_TRUE;
}
uint8_t GetUpgradeType(void)
{
    return (uint8_t )gstFirmwareUpgrateInfo.updata_slave_type;
}











