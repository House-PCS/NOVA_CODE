/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        ins_res_detect.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-09
  * @brief
  ******************************************************************************
  * @attention
  * @relation
    
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-09    <td>V1.0.0    <td>          <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "ins_res_detect.h"
#include "math.h"
#include "peri_ctl_stat_api.h"

/*
 * 1.该模块涉及到的逻辑电阻有8个，从R1~R8
 * 2.等效阻抗2个，分别是Rp和Rn
 * 3.按照工商业BCU A4版本原理图:
 *     R1 = (R67 + R68 + R69 + R73 + R74 + R75)
 *     R2 = (R91 + R88 + R92 + R94 + R95 + R96)
 *     R3 = R341
 *     R4 = R342
 *     R5 = (R64 + R65 + R66)
 *     R6 = (R70 + R71 + R72 + R77 + R78 + R79)
 *     R7 = (R85 + R83 + R86 + R89 + R93 + R90)
 *     R8 = (R98 + R99 + R100)
 * 4.分别测量S1闭合，S2断开和S1断开，S2闭合两种状态下的R3和R4电压，分别表示为Vp1,Vn1,   Vp2,Vn2
 * 5.按照初中知识，有如下公式: (其中A ∪ B = AB / (A + B))
 *     Vp1 * ((R3 + R1) / R3) = Vbat * (((R3 + R1) ∪ R5 ∪ Rp) / (((R3 + R1) ∪ R5 ∪ Rp) + ((R4 + R2) ∪ (R7 + R8) ∪ Rn)))             ①
 *    -Vn1 * ((R4 + R2) / R4) = Vbat * (((R4 + R2) ∪ (R7 + R8) ∪ Rn) / (((R3 + R1) ∪ R5 ∪ Rp) + ((R4 + R2) ∪ (R7 + R8) ∪ Rn)))      ②
 *     Vp2 * ((R3 + R1) / R3) = Vbat * (((R3 + R1) ∪ (R5 + R6) ∪ Rp) / (((R3 + R1) ∪ (R5 + R6) ∪ Rp) + ((R4 + R2) ∪ R8 ∪ Rn)))      ③
 *    -Vn2 * ((R4 + R2) / R4) = Vbat * (((R4 + R2) ∪ R8 ∪ Rn) / (((R3 + R1) ∪ (R5 + R6) ∪ Rp) + ((R4 + R2) ∪ R8 ∪ Rn)))             ④
 * 6.解上面四元一次方程组(实际是三元  Vbat, Rp, Rn), 并按照实际设计R1 = R2, R3 = R4, R5 = R8, R6 = R7 备注：Vbat可直接按照R1，R3和R2，R4分压原理计算
 *     Vbat = (Vp1 - Vn1) * (R3 + R1) / R3  
 *   或Vbat = (Vp2 - Vn2) * (R3 + R1) / R3
 *     -Vn1 / Vp1 = ((R1 + R3) ∪ (R5 + R6) ∪ Rn ) / ((R1 + R3) ∪ R5 ∪ Rp)    (② / ①)
 *     -Vn2 / Vp2 = ((R1 + R3) ∪ R5 ∪ Rn ) / ((R1 + R3) ∪ (R5 + R6) ∪ Rp)    (④ / ③)
 *     Rp = R5 / ((R6 / (R5 + R6)) * ((Vp2 - Vn2) * Vn1 / (Vp2 * Vn1 - Vn2 * Vp1)) - R5 / (R3 + R1) - 1)
 *     Rn = (R5 + R6) / ((R6 / R5) * ((Vp2 - Vn2) * Vp1 / (Vn2 * Vp1 - Vp2 * Vn1)) - (R5 + R6) / (R3 + R1) - 1)
 * 7.具体执行步骤如下
 *    a.闭合S3、S1，HT7017采集电压Vp1和Vn1
 *    b.断开S1,闭合S2，HT7017采集电压Vp2和Vn2
 *    c.断开S3,S2，求解绝缘阻抗
 */


//阻值定义，如上述注释可知，逻辑电阻只需定义R1, R3, R5, R6，单位K
#define IMS_RES_LOGIC_R1        (double)6000 
#define IMS_RES_LOGIC_R3        (double)3
#define IMS_RES_LOGIC_R5        (double)300
#define IMS_RES_LOGIC_R6        (double)6000


static double _vp1, _vn1, _vp2, _vn2;//单位mV

InsResInfo_t gInsResInfo;

static EN_INS_RES_EXEC_STEP _gStatus = EN_INS_RES_EXEC_STEP_IDLE;

#define INS_RES_INIT_NONBLOCKING_MODE   1

/** 
* @brief    电量计配置
* @param    cmd：命令
* @param    p_value：要写入的值
* @return   none
* @note: 
*/
static EN_OP_RES _ElecMeterCfgPolling(uint8_t cmd,  uint8_t *p_value)
{
    static EN_ELEC_METER_OP_STAT cfg_stat = EN_ELEC_METER_OP_STAT_IDLE;
    static uint32_t time_count = 0;
    if(cfg_stat == EN_ELEC_METER_OP_STAT_IDLE)
    {
        DevHT7017WriteCmd(cmd, p_value);
        cfg_stat = EN_ELEC_METER_OP_STAT_BUSY;
        time_count = tim_get_systime();
        return EN_OP_RES_ING;
    }
    else if(cfg_stat == EN_ELEC_METER_OP_STAT_BUSY)
    {
        EN_DEV_HT7017_ACK_RES res = DevHT7017WriteAck();
        if(res == EN_DEV_HT7017_ACK_RES_OK)//正常回复
        {
            cfg_stat = EN_ELEC_METER_OP_STAT_IDLE;
            return EN_OP_RES_COMPLETE;
        }
        else if(res == EN_DEV_HT7017_ACK_RES_NONE)//尚未回复
        {
            if((uint32_t)(tim_get_systime() - time_count) > 500)
            {
                cfg_stat = EN_ELEC_METER_OP_STAT_IDLE;
                return EN_OP_RES_FAIL;
            }
            else
            {
                return EN_OP_RES_ING;
            }
        }
        else//其他回复错误
        {
            cfg_stat = EN_ELEC_METER_OP_STAT_IDLE;
            return EN_OP_RES_FAIL;
        }
    }
}

/** 
* @brief    电量计回读
* @param    cmd：命令
* @param    p_value：要读取的值
* @return   none
* @note: 
*/
static EN_OP_RES _ElecMeterReadPolling(uint8_t cmd,  uint8_t *p_value)
{
    static EN_ELEC_METER_OP_STAT read_stat = EN_ELEC_METER_OP_STAT_IDLE;
    static uint32_t time_count = 0;
    if(read_stat == EN_ELEC_METER_OP_STAT_IDLE)
    {
        DevHT7017ReadCmd(cmd, p_value);
        read_stat = EN_ELEC_METER_OP_STAT_BUSY;
        time_count = tim_get_systime();
        return EN_OP_RES_ING;
    }
    else if(read_stat == EN_ELEC_METER_OP_STAT_BUSY)
    {
        EN_DEV_HT7017_ACK_RES res = DevHT7017ReadAck(p_value);
        if(res == EN_DEV_HT7017_ACK_RES_OK)//正常回复
        {
            read_stat = EN_ELEC_METER_OP_STAT_IDLE;
            return EN_OP_RES_COMPLETE;
        }
        else if(res == EN_DEV_HT7017_ACK_RES_NONE)//尚未回复
        {
            if((uint32_t)(tim_get_systime() - time_count) > 500)
            {
                read_stat = EN_ELEC_METER_OP_STAT_IDLE;
                return EN_OP_RES_FAIL;
            }
            else
            {
                return EN_OP_RES_ING;
            }
        }
        else//其他回复错误
        {
            read_stat = EN_ELEC_METER_OP_STAT_IDLE;
            return EN_OP_RES_FAIL;
        }
    }
}


/** 
* @brief    电量计初始化(阻塞式)
* @param    none
* @return   none
* @note: 
*/
static bool _ElecMeterInit()
{
    EN_OP_RES res;
    static uint8_t cnt = 0;
    uint8_t  buf[3];//写操作用2个字节，读操作用3个字节
    //WPCFG配置
    buf[0] = 0x00;
    buf[1] = 0xBC;//禁止操作50H~7CH范围的寄存器
    do
    {
        DevHT7017Process();
        res = _ElecMeterCfgPolling(HT_CMD_WPCFG, buf);
    }while(res == EN_OP_RES_ING);
    if(res == EN_OP_RES_FAIL)
    {
        return false;
    }
    
    //ANAEN配置
    buf[0] = 0x00;
    buf[1] = 0x07;//打开各通道ADC开关信号
    do
    {
        DevHT7017Process();
        res = _ElecMeterCfgPolling(HT_CMD_ANAEN, buf);
    }while(res == EN_OP_RES_ING);
    if(res == EN_OP_RES_FAIL)
    {
        return false;
    }    
    
    //ANAEN回读，确认下发成功
    do
    {
        DevHT7017Process();
        res = _ElecMeterReadPolling(HT_CMD_ANAEN, buf);
    }while(res == EN_OP_RES_ING);
    if(res == EN_OP_RES_FAIL)
    {
        return false;
    } 
    if(buf[2] != 0x07)//下发和回读不一致
    {
        return false;
    }
    
    //MODULE_EN配置
    buf[0] = 0x00;
    buf[1] = 0x00;//关闭各通道高通滤波器
    do
    {
        DevHT7017Process();
        res = _ElecMeterCfgPolling(HT_CMD_MODULE_EN, buf);
    }while(res == EN_OP_RES_ING);
    if(res == EN_OP_RES_FAIL)
    {
        return false;
    }  
    
    //MODULE_EN配置回读，确认下发成功
    do
    {
        DevHT7017Process();
        res = _ElecMeterReadPolling(HT_CMD_MODULE_EN, buf);
    }while(res == EN_OP_RES_ING);
    if(res == EN_OP_RES_FAIL)
    {
        return false;
    } 
    if(buf[1] != 0x00 || buf[2] != 0x00)//下发和回读不一致
    {
        return false;
    }
    
    return true;
}





/** 
* @brief    电量计初始化(非阻塞式)
* @param    none
* @return   none
* @note: 
*/
static EN_OP_RES _ElecMeterInitNonBlocking()
{
    EN_OP_RES init_res = EN_OP_RES_ING;
    EN_OP_RES res;
    static EN_INS_RES_INIT_STEP init_stat = EN_INS_RES_EXEC_STEP_WPCFG;
    uint8_t  buf[3];//写操作用2个字节，读操作用3个字节
    
    switch(init_stat)
    {
        case EN_INS_RES_EXEC_STEP_WPCFG:
        {
            //WPCFG配置
            buf[0] = 0x00;
            buf[1] = 0xBC;//禁止操作50H~7CH范围的寄存器
            res = _ElecMeterCfgPolling(HT_CMD_WPCFG, buf);
            if(res == EN_OP_RES_COMPLETE)
            {
                init_stat = EN_INS_RES_EXEC_STEP_ANAEN;
            }
            else if(res == EN_OP_RES_FAIL)
            {
                init_res = res;
                init_stat = EN_INS_RES_EXEC_STEP_WPCFG;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_ANAEN:
        {
            //ANAEN配置
            buf[0] = 0x00;
            buf[1] = 0x07;//打开各通道ADC开关信号
            res = _ElecMeterCfgPolling(HT_CMD_ANAEN, buf);
            if(res == EN_OP_RES_COMPLETE)
            {
                init_stat = EN_INS_RES_EXEC_STEP_ANAEN_READ;
            }
            else if(res == EN_OP_RES_FAIL)
            {
                init_res = res;
                init_stat = EN_INS_RES_EXEC_STEP_WPCFG;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_ANAEN_READ:
        {
            res = _ElecMeterReadPolling(HT_CMD_ANAEN, buf);
            if(res == EN_OP_RES_COMPLETE)
            {
                if(buf[2] != 0x07)//下发和回读不一致
                {
                    init_res = EN_OP_RES_FAIL;
                    init_stat = EN_INS_RES_EXEC_STEP_WPCFG;
                }
                else
                {
                    init_stat = EN_INS_RES_EXEC_STEP_MODULE_EN;
                }
            }
            else if(res == EN_OP_RES_FAIL)
            {
                init_res = res;
                init_stat = EN_INS_RES_EXEC_STEP_WPCFG;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_MODULE_EN:
        {
            //MODULE_EN配置
            buf[0] = 0x00;
            buf[1] = 0x00;//关闭各通道高通滤波器
            res = _ElecMeterCfgPolling(HT_CMD_MODULE_EN, buf);
            if(res == EN_OP_RES_COMPLETE)
            {
                init_stat = EN_INS_RES_EXEC_STEP_MODULE_EN_READ;
            }
            else if(res == EN_OP_RES_FAIL)
            {
                init_res = res;
                init_stat = EN_INS_RES_EXEC_STEP_WPCFG;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_MODULE_EN_READ:
        {
            res = _ElecMeterReadPolling(HT_CMD_MODULE_EN, buf);
            if(res == EN_OP_RES_COMPLETE)
            {
                if(buf[1] != 0x00 || buf[2] != 0x00)//下发和回读不一致
                {
                    init_res = EN_OP_RES_FAIL;
                    init_stat = EN_INS_RES_EXEC_STEP_WPCFG;
                }
                else
                {
                    init_res = EN_OP_RES_COMPLETE;
                    init_stat = EN_INS_RES_EXEC_STEP_WPCFG;
                }
            }
            else if(res == EN_OP_RES_FAIL)
            {
                init_res = res;
                init_stat = EN_INS_RES_EXEC_STEP_WPCFG;
            }
            break;
        }
        default:break;
    } 
    return init_res;
}




/** 
* @brief    启动绝缘检测
* @param    none
* @return   none
* @note: 
*/
void InsResDetectStart()
{
    if(_gStatus != EN_INS_RES_EXEC_STEP_IDLE && _gStatus != EN_INS_RES_EXEC_STEP_FAIL)
    {
        return;
    }
    _gStatus = EN_INS_RES_EXEC_STEP_ELEC_METER_INIT;
}


/** 
* @brief    停止绝缘检测
* @param    none
* @return   none
* @note: 
*/
void InsResDetectStop()
{
    _gStatus = EN_INS_RES_EXEC_STEP_IDLE;
}


/** 
* @brief    绝缘检测计算大轮询
* @param    none
* @return   none
* @note: 
*/
void InsResDetectProcess()
{
    static uint32_t time_count = 0;
    uint8_t buf[3];
    EN_OP_RES res;
    
    switch(_gStatus)
    {
        case EN_INS_RES_EXEC_STEP_IDLE:
        {
            //确保继电器和两个光耦断开
            InsImpUpMosCtl(false);
            InsImpDownMosCtl(false);
            InsImpRelayCtl(false);
            gInsResInfo.detect_stat = EN_INS_RES_DETECT_STAT_NONE;
            time_count = tim_get_systime();
            break;
        }
        case EN_INS_RES_EXEC_STEP_ELEC_METER_INIT:
        {
            static uint8_t count = 0;
            
            gInsResInfo.detect_stat = EN_INS_RES_DETECT_STAT_ING;
#if (INS_RES_INIT_NONBLOCKING_MODE == 0)            
            if(_ElecMeterInit() == true)
            {
                _gStatus = EN_INS_RES_EXEC_STEP_S1_S3_CLOSE;
                count = 0;
            }
            else//初始化失败
            {
                count++;
                if(count >= 3)
                {
                    count = 0;
                    _gStatus = EN_INS_RES_EXEC_STEP_FAIL;
                }
            }
#else
            EN_OP_RES init_res;
            init_res = _ElecMeterInitNonBlocking();
            if(init_res == EN_OP_RES_COMPLETE)
            {
                _gStatus = EN_INS_RES_EXEC_STEP_S1_S3_CLOSE;
                count = 0;
            }
            else if(init_res == EN_OP_RES_FAIL)
            {
                count++;
                if(count >= 3)
                {
                    count = 0;
                    _gStatus = EN_INS_RES_EXEC_STEP_FAIL;
                }
            }
#endif
            break;
        }
        case EN_INS_RES_EXEC_STEP_S1_S3_CLOSE:
        {
            InsImpDownMosCtl(false);//断开下MOS
            InsImpRelayCtl(true);//闭合继电器
            InsImpUpMosCtl(true);//闭合上MOS
            time_count = tim_get_systime();
            _gStatus = EN_INS_RES_EXEC_STEP_WAIT1;
            break;
        }
        case EN_INS_RES_EXEC_STEP_WAIT1:
        {
            if((uint32_t)(tim_get_systime() - time_count) > 300)//延迟20ms，确保该闭合的闭合该断开的断开
            {
                _gStatus = EN_INS_RES_EXEC_STEP_GET_VP1;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_GET_VP1:
        {
            res = _ElecMeterReadPolling(HT_CMD_SPL_I2, buf);
            if(res == EN_OP_RES_COMPLETE)
            {
                int32_t rx_data = 0;
                rx_data = (buf[0] << 16) | (buf[1] << 8) | buf[2];
                //判断是否为负值，如果是，则bit24~31填入1
                if((rx_data & (1 << 18)) != 0)//负数
                {
                    rx_data |= (0xFF << 24);
                }
                //计算_vp1
                _vp1 = (double)rx_data * HT_VOLT_COVERT;
                _gStatus = EN_INS_RES_EXEC_STEP_GET_VN1;
            }
            else if(res == EN_OP_RES_FAIL)
            {
                _gStatus = EN_INS_RES_EXEC_STEP_FAIL;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_GET_VN1:
        {
            res = _ElecMeterReadPolling(HT_CMD_SPL_I1, buf);
            if(res == EN_OP_RES_COMPLETE)
            {
                int32_t rx_data = 0;
                rx_data = (buf[0] << 16) | (buf[1] << 8) | buf[2];
                //判断是否为负值，如果是，则bit24~31填入1
                if((rx_data & (1 << 18)) != 0)//负数
                {
                    rx_data |= (0xFF << 24);
                }
                //计算_vn1
                _vn1 = (double)rx_data * HT_VOLT_COVERT;
                _vn1 = - _vn1;
                _gStatus = EN_INS_RES_EXEC_STEP_S2_CLOSE;
            }
            else if(res == EN_OP_RES_FAIL)
            {
                _gStatus = EN_INS_RES_EXEC_STEP_FAIL;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_S2_CLOSE:
        {
            InsImpUpMosCtl(false);//断开上MOS
            InsImpDownMosCtl(true);//闭合下MOS
            time_count = tim_get_systime();
            _gStatus = EN_INS_RES_EXEC_STEP_WAIT2;
            break;
        }
        case EN_INS_RES_EXEC_STEP_WAIT2:
        {
            if((uint32_t)(tim_get_systime() - time_count) > 300)//延迟20ms，确保该闭合的闭合该断开的断开
            {
                _gStatus = EN_INS_RES_EXEC_STEP_GET_VP2;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_GET_VP2:
        {
            res = _ElecMeterReadPolling(HT_CMD_SPL_I2, buf);
            if(res == EN_OP_RES_COMPLETE)
            {
                int32_t rx_data = 0;
                rx_data = (buf[0] << 16) | (buf[1] << 8) | buf[2];
                //判断是否为负值，如果是，则bit24~31填入1
                if((rx_data & (1 << 18)) != 0)//负数
                {
                    rx_data |= (0xFF << 24);
                }
                //计算_vp2
                _vp2 = (double)rx_data * HT_VOLT_COVERT;
                _gStatus = EN_INS_RES_EXEC_STEP_GET_VN2;
            }
            else if(res == EN_OP_RES_FAIL)
            {
                _gStatus = EN_INS_RES_EXEC_STEP_FAIL;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_GET_VN2:
        {
            res = _ElecMeterReadPolling(HT_CMD_SPL_I1, buf);
            if(res == EN_OP_RES_COMPLETE)
            {
                int32_t rx_data = 0;
                rx_data = (buf[0] << 16) | (buf[1] << 8) | buf[2];
                //判断是否为负值，如果是，则bit24~31填入1
                if((rx_data & (1 << 18)) != 0)//负数
                {
                    rx_data |= (0xFF << 24);
                }
                //计算_vn2
                _vn2 = (double)rx_data * HT_VOLT_COVERT;
                _vn2 = -_vn2;
                _gStatus = EN_INS_RES_EXEC_STEP_COMPLETE;
            }
            else if(res == EN_OP_RES_FAIL)
            {
                _gStatus = EN_INS_RES_EXEC_STEP_FAIL;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_COMPLETE:
        {
            //进行绝缘阻抗计算
            double bp_res, bn_res;
            gInsResInfo.bat_volt = (_vp1 - _vn1) * (IMS_RES_LOGIC_R3 + IMS_RES_LOGIC_R1) / IMS_RES_LOGIC_R3 / 1000;//单位V
            bp_res = (double)IMS_RES_LOGIC_R5 / ((IMS_RES_LOGIC_R6 / (IMS_RES_LOGIC_R5 + IMS_RES_LOGIC_R6)) * \
                                                             ((_vp2 - _vn2) * _vn1 / (_vp2 * _vn1 - _vn2 * _vp1)) - \
                                                             IMS_RES_LOGIC_R5 / (IMS_RES_LOGIC_R3 + IMS_RES_LOGIC_R1) - 1);
            bn_res = (double)(IMS_RES_LOGIC_R5 + IMS_RES_LOGIC_R6) / ((IMS_RES_LOGIC_R6 / IMS_RES_LOGIC_R5) * \
                                                          ((_vp2 - _vn2) * _vp1 / (_vn2 * _vp1 - _vp2 * _vn1)) - \
                                                          (IMS_RES_LOGIC_R5 + IMS_RES_LOGIC_R6) / (IMS_RES_LOGIC_R3 + IMS_RES_LOGIC_R1) - 1);
            
            gInsResInfo.bp_res = fabs(bp_res);
            gInsResInfo.bn_res = fabs(bn_res);
            gInsResInfo.detect_stat = EN_INS_RES_DETECT_STAT_COMPLETE;
            _gStatus = EN_INS_RES_EXEC_STEP_GAP;
            time_count = tim_get_systime();
            break;
        }
        case EN_INS_RES_EXEC_STEP_GAP:
        {
            if((uint32_t)(tim_get_systime() - time_count) >= 1000)
            {
                _gStatus = EN_INS_RES_EXEC_STEP_S1_S3_CLOSE;
            }
            break;
        }
        case EN_INS_RES_EXEC_STEP_FAIL:
        {
            gInsResInfo.detect_stat = EN_INS_RES_DETECT_STAT_ERROR;
            //确保继电器和两个光耦断开
            InsImpUpMosCtl(false);
            InsImpDownMosCtl(false);
            InsImpRelayCtl(false);
            break;
        }
        default:break;
    }
}






