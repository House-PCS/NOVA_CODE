

/* Includes ------------------------------------------------------------------*/
#include "afe_control.h"
#include "bsp.h"
#include "bms_protocol.h"

#define LOG_TAG                         "afe_control"

Cell_Info gCell_Info;
AFESampleInfo_t gAFESampleInfo;
uint8_t gFirst_AFE_Coved_Flag = 0;
static EN_AFE_CTL_FUNC _gAFECtlStat = EN_AFE_CTL_IDLE;

typedef enum
{
    EN_BALANCE_OPEN_STATUS_1,
    EN_BALANCE_OPEN_STATUS_2,
    EN_BALANCE_OPEN_STATUS_3,
    EN_BALANCE_OPEN_STATUS_4
} EN_BALANCE_OPEN_STATUS;




/** 
 * @brief    afe断线检测轮询处理
 * @param    NONE
 * @return   操作结果
 * @note:    
 */
//static EN_AFE_OP_RES _AFEOpenWireDetectProcess()
//{
//    static uint32_t time_count = 0;
//    static EN_AFE_OW_STA stat = EN_AFE_OW_STA_CMD_EVEN;
//    uint8_t type = 0; //0为Cell模式，1为S-Volt模式
//    
//    switch(stat)
//    {
//        case EN_AFE_OW_STA_CMD_EVEN: 
//        {
//            if(DevAFEWireCheckPrep() == true)
//            {
//                DevAFEWireCheckCmd(0, type);
//                stat = EN_AFE_OW_STA_WAIT_EVEN;
//                time_count = tim_get_systime();
//            }
//            else
//            {
//                return EN_AFE_OP_RES_ERROR;
//            }
//            break;
//        }
//        case EN_AFE_OW_STA_WAIT_EVEN:
//        {
//            EN_AFE_COMM_STATUS res = DevAFEWireCheckStat(0, type);
//            if(res == EN_AFE_COMM_STATUS_COMPLETE)
//            {
//                stat = EN_AFE_OW_STA_CMD_ODD;
//            }
//            else if(res == EN_AFE_COMM_STATUS_ERROR)
//            {
//                stat = EN_AFE_OW_STA_CMD_EVEN;
//                return EN_AFE_OP_RES_ERROR;
//            }
//            else
//            {
//                if((uint32_t)(tim_get_systime() - time_count) > 100)//暂定10ms作为超时时间
//                {
//                    stat = EN_AFE_OW_STA_CMD_EVEN;
//                    return EN_AFE_OP_RES_ERROR;
//                }
//            }
//            break;
//        }
//        case EN_AFE_OW_STA_CMD_ODD:
//        {
//            DevAFEWireCheckPrep();
//            DevAFEWireCheckCmd(1, type);
//            stat = EN_AFE_OW_STA_WAIT_ODD;
//            time_count = tim_get_systime();
//            break;
//        }
//        case EN_AFE_OW_STA_WAIT_ODD:
//        {
//            EN_AFE_COMM_STATUS res = DevAFEWireCheckStat(1, type);
//            if(res == EN_AFE_COMM_STATUS_COMPLETE)
//            {
//                /*将断线检测信息传递给gAFESampleInfo.cell_ow_stat*/
//                DevAFEWireCheckResultGet(gAFESampleInfo.cell_ow_stat, type);
//                stat = EN_AFE_OW_STA_CMD_EVEN;
//                return EN_AFE_OP_RES_COMPLETE;
//            }
//            else if(res == EN_AFE_COMM_STATUS_ERROR)
//            {
//                stat = EN_AFE_OW_STA_CMD_EVEN;
//                return EN_AFE_OP_RES_ERROR;
//            }
//            else
//            {
//                if((uint32_t)(tim_get_systime() - time_count) > 100)//暂定10ms作为超时时间
//                {
//                    stat = EN_AFE_OW_STA_CMD_EVEN;
//                    return EN_AFE_OP_RES_ERROR;
//                }
//            }
//            break;
//        }
//        default:break;
//    }
//    return EN_AFE_OP_RES_ING;
//}
static EN_AFE_OP_RES _AFEOpenWireDetectProcess()
{
    static uint32_t time_count = 0;
    static EN_AFE_OW_STA stat = EN_AFE_OW_STA_CMD_REF;
    uint8_t type = 1; //0为Cell模式，1为S-Volt模式
    
    switch(stat)
    {
        case EN_AFE_OW_STA_CMD_REF:
        {
            DevAFEWireCheckCmd(0, type);
            stat = EN_AFE_OW_STA_WAIT_REF;
            time_count = tim_get_systime();
            break;
        }
        case EN_AFE_OW_STA_WAIT_REF:
        {
            if((uint32_t)(tim_get_systime() - time_count) > 8)
            {
                EN_AFE_COMM_STATUS res = DevAFEWireCheckStat(0, type);
                if(res == EN_AFE_COMM_STATUS_COMPLETE)
                {
                    stat = EN_AFE_OW_STA_CMD_EVEN;
                }
                else if(res == EN_AFE_COMM_STATUS_ERROR)
                {
                    stat = EN_AFE_OW_STA_CMD_REF;
                    return EN_AFE_OP_RES_ERROR;
                }
                else
                {
                    if((uint32_t)(tim_get_systime() - time_count) > 100)//暂定10ms作为超时时间
                    {
                        stat = EN_AFE_OW_STA_CMD_REF;
                        return EN_AFE_OP_RES_ERROR;
                    }
                }
            }
            break;
        }
        case EN_AFE_OW_STA_CMD_EVEN: 
        {
            DevAFEWireCheckCmd(1, type);
            stat = EN_AFE_OW_STA_WAIT_EVEN;
            time_count = tim_get_systime();
            break;
        }
        case EN_AFE_OW_STA_WAIT_EVEN:
        {
            if((uint32_t)(tim_get_systime() - time_count) > 8)
            {
                EN_AFE_COMM_STATUS res = DevAFEWireCheckStat(1, type);
                if(res == EN_AFE_COMM_STATUS_COMPLETE)
                {
                    stat = EN_AFE_OW_STA_CMD_ODD;
                }
                else if(res == EN_AFE_COMM_STATUS_ERROR)
                {
                    stat = EN_AFE_OW_STA_CMD_REF;
                    return EN_AFE_OP_RES_ERROR;
                }
                else
                {
                    if((uint32_t)(tim_get_systime() - time_count) > 100)//暂定10ms作为超时时间
                    {
                        stat = EN_AFE_OW_STA_CMD_REF;
                        return EN_AFE_OP_RES_ERROR;
                    }
                }
            }
            break;
        }
        case EN_AFE_OW_STA_CMD_ODD:
        {
            DevAFEWireCheckCmd(2, type);
            stat = EN_AFE_OW_STA_WAIT_ODD;
            time_count = tim_get_systime();
            break;
        }
        case EN_AFE_OW_STA_WAIT_ODD:
        {
            if((uint32_t)(tim_get_systime() - time_count) > 8)
            {
                EN_AFE_COMM_STATUS res = DevAFEWireCheckStat(2, type);
                if(res == EN_AFE_COMM_STATUS_COMPLETE)
                {
                    stat = EN_AFE_OW_STA_RESULT;
                }
                else if(res == EN_AFE_COMM_STATUS_ERROR)
                {
                    stat = EN_AFE_OW_STA_CMD_REF;
                    return EN_AFE_OP_RES_ERROR;
                }
                else
                {
                    if((uint32_t)(tim_get_systime() - time_count) > 100)//暂定10ms作为超时时间
                    {
                        stat = EN_AFE_OW_STA_CMD_REF;
                        return EN_AFE_OP_RES_ERROR;
                    }
                }
            }
            break;
        }
        case EN_AFE_OW_STA_RESULT:
        {
            DevAFEWireCheckResultGet(gAFESampleInfo.cell_ow_stat, type);
            GetOwVal(gAFESampleInfo.cell_ow_val[0]);
            stat = EN_AFE_OW_STA_CMD_REF;
            return EN_AFE_OP_RES_COMPLETE;
            break;
        }
        default:break;
    }
    return EN_AFE_OP_RES_ING;
}




/**
 * @brief  : 均衡顺序分组操作，16个均衡电阻，每4个为一组，共4组。
             交叉开启均衡，防止长时间开启同一区域的均衡电路导致烧坏均衡电阻
 * @para   : None
 * @note   : None
* @return : balance_path : 允许开启均衡的4个电阻的位置
 */
uint16_t _BalancePathSwitch()
{
    static EN_BALANCE_OPEN_STATUS   e_balance_flag  = EN_BALANCE_OPEN_STATUS_1;     //均衡顺序分组
    static uint16_t                 balance_path    = 0XAA55;                       //均衡顺序
    
    switch(e_balance_flag)
    {
        case EN_BALANCE_OPEN_STATUS_1:
        {
            balance_path = 0XAA00;        //交叉开启均衡，防止长时间开启同一区域的均衡电路导致烧坏均衡电阻
            e_balance_flag = EN_BALANCE_OPEN_STATUS_2;
            break;
        }
        case EN_BALANCE_OPEN_STATUS_2:
        {
            balance_path = 0X0055;
            e_balance_flag = EN_BALANCE_OPEN_STATUS_3;
            break;
        }
        case EN_BALANCE_OPEN_STATUS_3:
        {
            balance_path = 0X00AA;
            e_balance_flag = EN_BALANCE_OPEN_STATUS_4;
            break;
        }
        case EN_BALANCE_OPEN_STATUS_4:
        {
            balance_path = 0X5500;
            e_balance_flag = EN_BALANCE_OPEN_STATUS_1;
            break;
        }
        default:
            break;
    }

    return balance_path;
}


static AFEBalanceStatus_t _gAfeBalanceSta;

/**
 * @brief  : 均衡处理
 * @para   : None
 * @note   : None
 * @return : None
 */
static bool _AFEBalanceOpen()
{
    static uint16_t balance_k = 0X5500;         //均衡系数，每次只均衡单数节电芯或者双数节电芯
    uint16_t rd_dcc = 0;                        //读取dcc(暂时无用)

    //更换均衡分组
    balance_k = _BalancePathSwitch();
    _gAfeBalanceSta.now_b_status = 0;
    
    for(uint8_t i = 0; i < AFE_CELL_NUM; i ++)
    {
        //未开启的电芯中找出所有应该开启的电芯(根据电压)
        //电芯>3.0V，比最小电压大，开启
        if(((_gAfeBalanceSta.last_b_status & (0X0001 << i)) == 0) &&
           (gCell_Info.cell_voltage[i] - gCell_Info.cell_min_voltage > BALANCE_OPEN_V_DIFF_VAL)
          )
        {
            if(_gAfeBalanceSta.b_accu_count[i] >= CELL_VOLT_FILTER_WIN)
            {
                _gAfeBalanceSta.now_b_status |= (0X0001 << i);
            }
            else
            {
                _gAfeBalanceSta.b_accu_count[i]++;
            }
        }
        //已开启且时间未到的电芯，电芯>3.0V，开启均衡
        if(((_gAfeBalanceSta.last_b_status & (0X0001 << i)) != 0) &&
            (_gAfeBalanceSta.balance_cnt[i] < (BALANCE_CNT))
        )
        {
            _gAfeBalanceSta.now_b_status |= (0X0001 << i);
        }
        //关闭最低节电芯及电压低于3.0V电芯的均衡标志
        if((gCell_Info.cell_voltage[i] == gCell_Info.cell_min_voltage) ||
           (gCell_Info.cell_voltage[i] < 3000)
          )
        {
            _gAfeBalanceSta.now_b_status &= ~(0X0001 << i);
        }
        //压差不超过阈值时，开启前的计数清零
        if((gCell_Info.cell_voltage[i] - gCell_Info.cell_min_voltage <= BALANCE_OPEN_V_DIFF_VAL))
        {
            if(_gAfeBalanceSta.b_accu_count[i] > 0)
            {
                _gAfeBalanceSta.b_accu_count[i]--;
            }
        }
    }
    //结合均衡分组算出本次开启的均衡电芯
    _gAfeBalanceSta.real_b_status = balance_k &  _gAfeBalanceSta.now_b_status;
    _gAfeBalanceSta.disp_b_status = _gAfeBalanceSta.real_b_status;
    //设置
    if(DevAFEBalanceStart(&_gAfeBalanceSta.real_b_status,&rd_dcc) == false)
    {   //设置失败
        return false;
    }

    for(uint8_t i = 0; i < AFE_CELL_NUM; i ++)
    {
        //last=0,now=1   代表刚刚开启
        //last=1,now=0   代表刚刚关闭
        //清除计数，重新开始计数
        if((_gAfeBalanceSta.last_b_status & (0X0001 << i)) != (_gAfeBalanceSta.now_b_status  & (0X0001 << i)))
        {
            _gAfeBalanceSta.balance_cnt[i] = 0;
        }
        //本次均衡开启的电芯，增加时间计数
        if((_gAfeBalanceSta.real_b_status & (0X0001 << i)) != 0)
        {
            _gAfeBalanceSta.balance_cnt[i]++;
        }  
    }
    
    //记录均衡状态
    _gAfeBalanceSta.last_b_status = _gAfeBalanceSta.now_b_status;

    return true;
}


/**
 * @brief  : 获取断线检测的最小值
 * @para   : None
 * @note   : None
 * @return : None
 */
uint16_t GetAfeOwMinVal()
{
    uint16_t min_val = 100;
    
    //寻找最小值
    for(int i=0; i<AFE_CELL_NUM; i++)
    {
        min_val = (min_val < gAFESampleInfo.cell_ow_val[0][i]) ? 
                   min_val : gAFESampleInfo.cell_ow_val[0][i];
    }
    return min_val;
}

/**
 * @brief  : 获取当前电芯的断线状态
 * @para   : None
 * @note   : None
 * @return : None
 */
uint16_t GetAfeOwStatus()
{
    return gAFESampleInfo.cell_ow_stat[0];
}

/**
 * @brief  : 获取当前电芯的均衡状态
 * @para   : None
 * @note   : None
 * @return : None
 */
uint16_t GetNowBalanceStatus()
{
    return _gAfeBalanceSta.disp_b_status;
}

/**
 * @brief  : 获取左右电芯的均衡状态（未分组，所有应该均衡的全部显示）
 * @para   : None
 * @note   : None
 * @return : None
 */
uint16_t GetAllBalanceStatus()
{
    return _gAfeBalanceSta.now_b_status;
}

/**
 * @brief  : 均衡开启
 * @para   : None
 * @note   : None
 * @return : None
 */
static EN_AFE_OP_RES _AFEBalanceProcess()
{
    static uint32_t wait_time = 0;         //等待时间
    static EN_AFE_BALANCE_STA stat = EN_AFE_BALANCE_STA_CHECK;
    static uint8_t balance_end_flag = 0; 
    
    switch(stat)
    {
        /***检查是否达到开启均衡条件***/
        case EN_AFE_BALANCE_STA_CHECK:
        {
            //已经进行过电压采样和温度采样后
            //所有电芯 > 3.0V，非放电，无故障，压差够大，才能开启均衡
            if((gAFESampleInfo.volt_sample_flag == false) || 
            (gAFESampleInfo.temp_sample_flag == false) || 
//            (gPackStatus.mode == EN_PACK_CURRENT_DISCHARGE_MODE) || 
//            (gCell_Info.cell_min_voltage < 3000) ||
            (gBmsErrorInfo.voltage1_error_code.value != 0)
            )
            {
                balance_end_flag = false;
                _gAfeBalanceSta.disp_b_status = 0;
                return EN_AFE_OP_RES_COMPLETE;
            }
            stat = EN_AFE_BALANCE_STA_OPEN;
            break;
        }
        /***开启***/
        case EN_AFE_BALANCE_STA_OPEN:
        {
            //开启成功
            if(_AFEBalanceOpen() == true)
            {
                gAFESampleInfo.balance_bcu_open = true;
                wait_time = tim_get_systime();
                stat = EN_AFE_BALANCE_STA_WAIT;
                break;
            }
            else
            {
                stat = EN_AFE_BALANCE_STA_CHECK;
                return EN_AFE_OP_RES_ERROR;
            }
        }
        /***等待***/
        case EN_AFE_BALANCE_STA_WAIT:
        {
            //开启300ms关闭
            if(tim_difftime_from_now(wait_time) >= 500)
            {
                stat = EN_AFE_BALANCE_STA_CLOSE;
            }
            break;
        }
        /***关闭***/
        case EN_AFE_BALANCE_STA_CLOSE:
        {
            balance_end_flag = DevAFEBalanceStop(gAFESampleInfo.balance_stat_flag);
            stat = EN_AFE_BALANCE_STA_END;
            wait_time = tim_get_systime();
            break;
        }
        /***结束***/
        case EN_AFE_BALANCE_STA_END:
        {
            //100ms延迟
            if(tim_difftime_from_now(wait_time) >= 100)
            {
                stat = EN_AFE_BALANCE_STA_CHECK;
                if(balance_end_flag == true)
                {
                    gAFESampleInfo.balance_bcu_open = false;
                    return EN_AFE_OP_RES_COMPLETE;
                }
                else
                {
                    return EN_AFE_OP_RES_ERROR;
                }
            }
            break;
        }
        
    }
    
    return EN_AFE_OP_RES_ING;
}


/** 
 * @brief    中值滤波
 * @param    val：对象数组指针
 * @return   中值数值
 * @note:    
 */
static uint16_t _MidFilter(uint16_t val1,uint16_t val2,uint16_t val3)
{
    // 取出数组的3个元素
    uint16_t a = val1;
    uint16_t b = val2;
    uint16_t c = val3;
    
    uint16_t mid_val;
    
    // 条件判断找出中间值（3元素场景最优解法）
    if ((a >= b && a <= c) || (a <= b && a >= c))
    {
        mid_val = a;  // a是中间值
    }
    else if ((b >= a && b <= c) || (b <= a && b >= c))
    {
        mid_val = b;  // b是中间值
    }
    else
    {
        mid_val = c;  // c是中间值
    }
    
    return mid_val;
    
                            
}



/** 
 * @brief    电压采样噪声补偿
 * @param    NONE
 * @return   操作结果
 * @note:    
 */
void AFEVoltageSampleNoiseCompensation()
{
    static uint32_t exec_count = 0;
    bool slide_accu = false;
    uint16_t max_voltage[AFE_CELL_NUM];
    uint16_t mad_voltage[AFE_CELL_NUM];
    static uint16_t cell_voltage_hist[CELL_VOLT_FILTER_WIN][AFE_CELL_NUM];
    static bool hist_noinit_flag = false;
    
    if(hist_noinit_flag == false)
    {
        memset((uint8_t *)&cell_voltage_hist[0][0], 0x00, sizeof(cell_voltage_hist));
        hist_noinit_flag = true;
    }
    
    if(exec_count < CELL_VOLT_FILTER_WIN)
    {
        exec_count++;
        slide_accu = false;
    }
    else
    {
        slide_accu = true;
    }
    #if 0
    //无再次中值滤波
    for(int i = 0; i < AFE_CELL_NUM; i++)
    {
        FilterCalcSlideMaxValue16u(&gAFESampleInfo.cell_voltage[0][0][i], AFE_CELL_NUM, &max_voltage[i], CELL_VOLT_FILTER_WIN, slide_accu);
        FilterCalcSlideMadMidianValue16u(&gAFESampleInfo.cell_voltage[0][0][i], AFE_CELL_NUM, &mad_voltage[i], CELL_VOLT_FILTER_WIN, slide_accu);
        gAFESampleInfo.cell_voltage_filter[0][i] = max_voltage[i] + mad_voltage[i];
    }
    //处理历史数据
    for(int i = 0; i < AFE_CELL_NUM; i++)
    {
        for(int j = CELL_VOLT_FILTER_WIN - 1; j > 0; j--)
        {
            gAFESampleInfo.cell_voltage[j][0][i] = gAFESampleInfo.cell_voltage[j - 1][0][i];
        }
    }
    #else
    //再次增加中值滤波
    for(int i = 0; i < AFE_CELL_NUM; i++)
    {
        FilterCalcSlideMaxValue16u(&gAFESampleInfo.cell_voltage[0][0][i], AFE_CELL_NUM, &max_voltage[i], CELL_VOLT_FILTER_WIN, slide_accu);
        FilterCalcSlideMadMidianValue16u(&gAFESampleInfo.cell_voltage[0][0][i], AFE_CELL_NUM, &mad_voltage[i], CELL_VOLT_FILTER_WIN, slide_accu);
        cell_voltage_hist[0][i] = max_voltage[i] + mad_voltage[i];
        FilterCalcSlideMidianValue16u(&cell_voltage_hist[0][i], AFE_CELL_NUM, &gAFESampleInfo.cell_voltage_filter[0][i], CELL_VOLT_FILTER_WIN, slide_accu);
    }
    //处理历史数据
    for(int i = 0; i < AFE_CELL_NUM; i++)
    {
        for(int j = CELL_VOLT_FILTER_WIN - 1; j > 0; j--)
        {
            gAFESampleInfo.cell_voltage[j][0][i] = gAFESampleInfo.cell_voltage[j - 1][0][i];
        }
        for(int j = CELL_VOLT_FILTER_WIN - 1; j > 0; j--)
        {
            cell_voltage_hist[j][i] = cell_voltage_hist[j - 1][i];
        }
    }
    #endif
    
    for(int i = 0; i < AFE_CELL_NUM; i++)
    {
        gCell_Info.cell_voltage[i] = gAFESampleInfo.cell_voltage_filter[0][i];
    }
}




/** 
 * @brief    afe获取电压
 * @param    NONE
 * @return   操作结果
 * @note:    
 */
static EN_AFE_OP_RES _AFEVoltageSampleProcess()
{
    static uint32_t time_count = 0;
    static uint32_t wait_time = 0;
    static uint16_t cell_v_temp[3][AFE_CELL_NUM];   //电芯电压存储（中值滤波用）
    static EN_AFE_V_STA stat = EN_AFE_V_STA_CMD;
    static uint32_t exec_count = 0, slide_win = 5;
    static float cell_v_avg_sum[AFE_CELL_NUM];
    bool slide_accu = false;
    
    switch(stat)
    {
        case EN_AFE_V_STA_CMD:
        {
            DevAFEVoltageSampleCmd();
            stat = EN_AFE_V_STA_WAIT;
            time_count = tim_get_systime();
            break;
        }
        case EN_AFE_V_STA_WAIT:
        {
            EN_AFE_COMM_STATUS res = DevAFEVoltageSampleStat(&gAFESampleInfo.cell_voltage[0][0][0]);
            if(res == EN_AFE_COMM_STATUS_COMPLETE)
            {
                stat = EN_AFE_V_STA_COMPUTE;
                return EN_AFE_OP_RES_ING;
            }
            else if(res == EN_AFE_COMM_STATUS_ERROR)
            {
                stat = EN_AFE_V_STA_CMD;
                return EN_AFE_OP_RES_ERROR;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - time_count) > 10)//暂定10ms作为超时时间
                {
                    stat = EN_AFE_V_STA_CMD;
                    return EN_AFE_OP_RES_ERROR;
                }
            }
            break;
        }
        case EN_AFE_V_STA_COMPUTE:
        {
            
            //前移,接收新数据
#if 0            
            //前移,接收新数据
            memcpy(cell_v_temp[0],  cell_v_temp[1],                   2*AFE_CELL_NUM*sizeof(cell_v_temp[0][0]));
            memcpy(cell_v_temp[2],  gAFESampleInfo.cell_voltage[0],   AFE_CELL_NUM*sizeof(cell_v_temp[0][0]));
            //接收滤波后的电压
            for(int i = 0; i < AFE_CELL_NUM; i ++)
            {   
              gCell_Info.cell_voltage[i] = _MidFilter(cell_v_temp[0][i],cell_v_temp[1][i],cell_v_temp[2][i]);
            }
#else
//            if(exec_count < slide_win)
//            {
//                exec_count++;
//                slide_accu = false;
//            }
//            else
//            {
//                slide_accu = true;
//            }
//            for(int i = 0; i < AFE_CELL_NUM; i++)
//            {
//                gAFESampleInfo.cell_voltage_filter[0][i] = gAFESampleInfo.cell_voltage[0][0][i];
//                FilterCalcSlide16u(gAFESampleInfo.cell_voltage_filter[0][i], &cell_v_avg_sum[i], &gCell_Info.cell_voltage[i], slide_win, slide_accu);
//            }    
            
            AFEVoltageSampleNoiseCompensation();//增加噪声补偿
                    
#endif  
            
            //查找最高节电芯电压及序号
            gCell_Info.cell_max_voltage = gCell_Info.cell_voltage[0];
            gCell_Info.cell_min_voltage = gCell_Info.cell_voltage[0];
            gCell_Info.total_cell_voltage = 0;
            for(int i = 0; i < AFE_CELL_NUM; i ++)
            {    
//                gCell_Info.cell_voltage[i] = gAFESampleInfo.cell_voltage[0][i];
                gCell_Info.total_cell_voltage += gCell_Info.cell_voltage[i];
                
                if(gCell_Info.cell_voltage[i] >= gCell_Info.cell_max_voltage)
                {
                    gCell_Info.cell_max_voltage = gCell_Info.cell_voltage[i];
                    gCell_Info.cell_max_vol_id = i;
                }

                if(gCell_Info.cell_voltage[i] <= gCell_Info.cell_min_voltage)
                {
                    gCell_Info.cell_min_voltage = gCell_Info.cell_voltage[i];
                    gCell_Info.cell_min_vol_id = i;
                }
            }
            //压差及平均电压
            gCell_Info.cell_diff_voltage = gCell_Info.cell_max_voltage - gCell_Info.cell_min_voltage;
            gCell_Info.cell_ave_voltage = gCell_Info.total_cell_voltage / AFE_CELL_NUM;
                      
            //////////////////////////////////////////////////////////
            wait_time = tim_get_systime();
            stat = EN_AFE_V_STA_END;
            break;
            
        }
        case EN_AFE_V_STA_END:
        {
            if(tim_difftime_from_now(wait_time) > 10)
            {
                stat = EN_AFE_V_STA_CMD;
                return EN_AFE_OP_RES_COMPLETE;
            }
        }
        default:break;
    }
    return EN_AFE_OP_RES_ING;
}

void GetBalanceCnt(uint16_t* p)
{
//    p[0] = test_max_v;
//    p[1] = test_min_v;
    for(int i=0; i<16; i++)
    {
      p[i+2] = _gAfeBalanceSta.balance_cnt[i];
    }
    
}

/** 
 * @brief    afe温度采集轮询处理
 * @param    NONE
 * @return   操作结果
 * @note:    
 */
static EN_AFE_OP_RES _AFETempSampleProcess()
{
    static uint8_t sample_cnt = 0;//采样计数
    static EN_AFE_T_STA stat = EN_AFE_T_STA_T_1;
    
    switch(stat)
    {
        case EN_AFE_T_STA_T_1:
        {
            gCell_Info.arr_cell_temperature[0] = PackNtc1TemperatureGet() * 10;
            stat = EN_AFE_T_STA_T_2;
            break;
        }
        case EN_AFE_T_STA_T_2:
        {
            gCell_Info.arr_cell_temperature[1] = PackNtc2TemperatureGet() * 10;
            stat = EN_AFE_T_STA_T_3;
            break;
        }
        case EN_AFE_T_STA_T_3:
        {
            gCell_Info.arr_cell_temperature[2] = PackNtc3TemperatureGet() * 10;
            stat = EN_AFE_T_STA_T_4;
            break;
        }
        case EN_AFE_T_STA_T_4:
        {
            gCell_Info.arr_cell_temperature[3] = PackNtc4TemperatureGet() * 10;
            stat = EN_AFE_T_STA_T_COMPUTE;
            break;
        }
        case EN_AFE_T_STA_T_COMPUTE:
        {
            //平均温度
            gCell_Info.cell_ave_temperature =(gCell_Info.arr_cell_temperature[0] + 
                                              gCell_Info.arr_cell_temperature[1] + 
                                              gCell_Info.arr_cell_temperature[2] + 
                                              gCell_Info.arr_cell_temperature[3]) / 4;
            //最大最小温度
            gCell_Info.cell_max_temperature = gCell_Info.cell_ave_temperature;
            gCell_Info.cell_min_temperature = gCell_Info.cell_ave_temperature;
            
            for(int i=0; i<4; i++)
            {
                gCell_Info.cell_max_temperature = (gCell_Info.cell_max_temperature > gCell_Info.arr_cell_temperature[i]) ? 
                                                   gCell_Info.cell_max_temperature : gCell_Info.arr_cell_temperature[i];
                gCell_Info.cell_min_temperature = (gCell_Info.cell_min_temperature < gCell_Info.arr_cell_temperature[i]) ? 
                                                   gCell_Info.cell_min_temperature : gCell_Info.arr_cell_temperature[i];
            }
            stat = EN_AFE_T_STA_T_1;
            return EN_AFE_OP_RES_COMPLETE;
        }
        default:
            break;
    }
    //检测完成后返回完成
    return EN_AFE_OP_RES_ING;
}

bool AFEFirstSample(void)
{
    //通讯故障时间太长，返回采样完成（不代表成功）
    if((gAFESampleInfo.cell_v_error_num > 80) || gAFESampleInfo.cell_t_error_num > 80)
    {
        return true;
    }
    //采样完成
    return (gAFESampleInfo.volt_sample_flag & gAFESampleInfo.temp_sample_flag);
}

/**
 * @brief  : soc计算处理
 * @para   : None
 * @note   : None
 * @return : None
 */
void _SocProcess(void)
{
    static uint32_t wait_time = 0;
    if((gAFESampleInfo.volt_sample_flag == false) || 
       (gAFESampleInfo.temp_sample_flag == false) || 
       (tim_difftime_from_now(wait_time) < 1000)
      )
    {
        return ;
    }
    SOCAmpereHourIntegral(1000, &gPackStatus);
    wait_time = tim_get_systime();
}

//调度策略及时长配置，可根据业务情况更改
static AFECtlSche_t _gAFECtlSche[] = 
{
    {EN_AFE_CTL_V_GET,  10,  0},        //电压读取
    {EN_AFE_CTL_OW_DET, 10,  0},         //断线检测
    {EN_AFE_CTL_B_SET,  10,  0},         //均衡
    {EN_AFE_CTL_T_GET,  10,  0},         //温度
//    {EN_AFE_CTL_BT_GET, 300, 0},
};
static uint32_t test_time[8];
/**
 * @brief  : AFE轮询处理
 * @para   : None
 * @note   : None
 * @return : None
 */
void AFEProcess(void)
{
    static uint32_t sub_timecount[EN_AFE_CTL_NUM];//用于计算各种操作所需时长
    static uint8_t sche_cnt = 0;
    
    //升级时，停止afe处理
    if(_gBMSSysInfo.bms_upgrade_ing_flag == true)
    {
        _gAFECtlStat = EN_AFE_CTL_IDLE;
        return;
    }
    
    //SOC计算
    _SocProcess();
    
    switch(_gAFECtlStat)
    {
        case EN_AFE_CTL_IDLE:
        {
            /*AFE操作调度*/
            //当前的策略是哪个操作时间到了就执行哪个，按顺序遍历
            if((uint32_t)(tim_get_systime() - _gAFECtlSche[sche_cnt].inverval_count) >= _gAFECtlSche[sche_cnt].min_interval)
            {
                _gAFECtlStat = _gAFECtlSche[sche_cnt].ctl_type;
                _gAFECtlSche[sche_cnt].inverval_count = tim_get_systime();
                sche_cnt++;
                sche_cnt %= sizeof(_gAFECtlSche)/sizeof(_gAFECtlSche[0]);
            }
            
            break;
        }
        case EN_AFE_CTL_V_GET:  //电压采样
        {
            EN_AFE_OP_RES res = _AFEVoltageSampleProcess();
            if(res != EN_AFE_OP_RES_ING)
            {
                test_time[0] = tim_get_systime();
                _gAFECtlStat = EN_AFE_CTL_IDLE;
                if(res == EN_AFE_OP_RES_ERROR)
                {
                    gAFESampleInfo.cell_v_error_num++;
                }
                else if(res == EN_AFE_OP_RES_COMPLETE)
                {
                    gAFESampleInfo.volt_sample_flag = true;
                    gAFESampleInfo.cell_v_error_num = 0;
                    //记录操作时长
                    gAFESampleInfo.cell_v_duration_time = (uint32_t)(tim_get_systime() - sub_timecount[EN_AFE_CTL_V_GET]);
                }
            }
            break;
        }
        case EN_AFE_CTL_T_GET:  //温度采样
        {
            EN_AFE_OP_RES res = _AFETempSampleProcess();
            if(res != EN_AFE_OP_RES_ING)
            {
                test_time[1] = tim_get_systime();
                _gAFECtlStat = EN_AFE_CTL_IDLE;
                if(res == EN_AFE_OP_RES_ERROR)
                {
                    gAFESampleInfo.cell_t_error_num++;
                }
                else if(res == EN_AFE_OP_RES_COMPLETE)
                {
                    gAFESampleInfo.temp_sample_flag = true;
                    gAFESampleInfo.cell_t_error_num = 0;
                    //记录操作时长
                    gAFESampleInfo.cell_t_duration_time = (uint32_t)(tim_get_systime() - sub_timecount[EN_AFE_CTL_T_GET]);
                }
            }
            break;
        }
        case EN_AFE_CTL_B_SET:  //均衡设置
        {
            EN_AFE_OP_RES res = _AFEBalanceProcess();
            if(res != EN_AFE_OP_RES_ING)
            {
                test_time[2] = tim_get_systime();
                _gAFECtlStat = EN_AFE_CTL_IDLE;
                if(res == EN_AFE_OP_RES_ERROR)
                {
                    gAFESampleInfo.balance_error_num++;
                }
                else if(res == EN_AFE_OP_RES_COMPLETE)
                {
                    //记录操作时长
                    gAFESampleInfo.balance_duration_time = (uint32_t)(tim_get_systime() - sub_timecount[EN_AFE_CTL_B_SET]);
                }
            }
            break;
        }
        case EN_AFE_CTL_OW_DET: //断线检测
        {
            EN_AFE_OP_RES res = _AFEOpenWireDetectProcess();
            if(res != EN_AFE_OP_RES_ING)
            {
                _gAFECtlStat = EN_AFE_CTL_IDLE;
                if(res == EN_AFE_OP_RES_ERROR)
                {
                    gAFESampleInfo.ow_error_num++;
                }
                else if(res == EN_AFE_OP_RES_COMPLETE)
                {
                    gAFESampleInfo.ow_sample_flag = true;
                    //记录操作时长
                    gAFESampleInfo.ow_duration_time = (uint32_t)(tim_get_systime() - sub_timecount[EN_AFE_CTL_OW_DET]);
                }
            }
            break;
        }
        default:break;
    }
}

//
///**
// * @brief  : AFE轮询处理
// * @para   : None
// * @note   : None
// * @return : None
// */
//void AFEProcess(void)
//{
//    static uint32_t sub_timecount[EN_AFE_CTL_NUM];//用于计算各种操作所需时长
//    static uint8_t sche_cnt = 0;
//    
//    //升级时，停止afe处理
//    if(_gBMSSysInfo.bms_upgrade_ing_flag == true)
//    {
//        _gAFECtlStat = EN_AFE_CTL_IDLE;
//        test_time[0] = tim_get_systime();
//        return;
//    }
//    
//    //SOC计算
//    _SocProcess();
//    
//    switch(_gAFECtlStat)
//    {
//        case EN_AFE_CTL_IDLE:
//        {
//            /*AFE操作调度*/
//            test_time[0] = tim_get_systime();
//                _gAFECtlStat = EN_AFE_CTL_V_GET;
//            break;
//        }
//        case EN_AFE_CTL_V_GET:  //电压采样
//        {
//            EN_AFE_OP_RES res = _AFEVoltageSampleProcess();
//            if(res != EN_AFE_OP_RES_ING)
//            {
//                test_time[1] = tim_get_systime();
//                gAFESampleInfo.volt_sample_flag = true;
//                _gAFECtlStat = EN_AFE_CTL_T_GET;
//            }
//            break;
//        }
//        case EN_AFE_CTL_T_GET:  //温度采样
//        {
//            EN_AFE_OP_RES res = _AFETempSampleProcess();
//            if(res != EN_AFE_OP_RES_ING)
//            {
//                gAFESampleInfo.temp_sample_flag = true;
//                _gAFECtlStat = EN_AFE_CTL_B_SET;
//                test_time[2] = tim_get_systime();
//            }
//            break;
//        }
//        case EN_AFE_CTL_B_SET:  //均衡设置
//        {
//            EN_AFE_OP_RES res = _AFEBalanceProcess();
//            if(res != EN_AFE_OP_RES_ING)
//            {
//                _gAFECtlStat = EN_AFE_CTL_IDLE;
//                test_time[3] = tim_get_systime();
//            }
//            break;
//        }
//        case EN_AFE_CTL_OW_DET: //断线检测
//        {
//        }
//        default:break;
//    }
//}