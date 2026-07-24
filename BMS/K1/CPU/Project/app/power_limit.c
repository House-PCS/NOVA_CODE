
#include "power_limit.h"
#include "afe_control.h"

#define LOG_TAG "POWER_LIMIT"

AllPackCurrentLimit gstAllPackLimit;

/**
 * @brief  : 充电末端功率系数设置
 * @para   : cell_max_vol : 最大节充电电压
 * @note   : None
 * @return : None
 */
float _ChgEndingFactorSet(uint16_t cell_max_vol, uint8_t bms_number)
{
    float full_chg_factor = 1.0f;       //充电末端放电系数
    static uint32_t pack_v_first_down_time  = 0;          //Pack电压首次低于3.4的时间点
    uint32_t pack_v_down_time  = 0;          //Pack电压低于3.4的时长
    
    //单节电芯最大电压在3.38V - 3.55V间，功率逐步从100%被限制到0%    
    if(cell_max_vol >= CHARGE_MAX_VOLTAGE - 220)
    {
        //3.38V-3.48V间，功率逐步从100%被限制到20%
        if(cell_max_vol <= (CHARGE_MAX_VOLTAGE - 120))
        {
            full_chg_factor = InterpolationOperation((CHARGE_MAX_VOLTAGE - 220), (CHARGE_MAX_VOLTAGE - 120), 100, 20, cell_max_vol) / 100;
        }
        else
        {
            //3.48V-3.55V间，功率逐步从20%被限制到0%
            full_chg_factor = InterpolationOperation((CHARGE_MAX_VOLTAGE - 120), (CHARGE_MAX_VOLTAGE - 50), 20, 0, cell_max_vol) / 100;
        }
        
        //最小值限制
        full_chg_factor = full_chg_factor < 0 ? 0 : full_chg_factor;
        
        //在这个范围内满充因数只能递减，不能增加
        if(full_chg_factor > gstAllPackLimit.chg_ending_factor[bms_number])
        {
            full_chg_factor = gstAllPackLimit.chg_ending_factor[bms_number];
        }
        //充电记录
        gstAllPackLimit.chg_ending_factor[bms_number] = full_chg_factor;
    }
    
    
    return full_chg_factor;
}


/**
 * @brief  : 放电末端功率系数设置（因为放电末端涉及断放电MOS，所以放电功率需要再提前0.05V改为0）
 * @para   : cell_max_vol : 最小节充电电压
 * @note   : None
 * @return : None
 */
float _DsgEndingFactorSet(uint16_t cell_min_vol, uint8_t bms_number)
{
    float empty_dsg_factor = 1.0f;      //放电末端放电系数

    static uint32_t pack_v_first_up_time  = 0;          //Pack电压首次高于2.9的时间点
    uint32_t pack_v_up_time  = 0;                       //Pack电压高于2.9的时长

    //单节电芯最小电压在2.95V - 2.75V间，放电允许功率逐步从100%被限制到0% 
    if(cell_min_vol <= DISCHARGE_MIN_VOLTAGE + 250)
    {
        empty_dsg_factor = InterpolationOperation((DISCHARGE_MIN_VOLTAGE + 50), (DISCHARGE_MIN_VOLTAGE + 250), 0, 100, cell_min_vol) / 100;
        //最小值限制
        empty_dsg_factor = empty_dsg_factor < 0 ? 0 : empty_dsg_factor;
        //在这个范围内满放因数只能递减，不能增加
        empty_dsg_factor = (gstAllPackLimit.dsg_ending_factor[bms_number] > empty_dsg_factor) ? empty_dsg_factor : gstAllPackLimit.dsg_ending_factor[bms_number];
        gstAllPackLimit.dsg_ending_factor[bms_number] = empty_dsg_factor;
    }
    
    
    return empty_dsg_factor;
}

/**
 * @brief  : 充电允许电流限制
 * @para   : None
 * @note   : None
 * @return : None
 */
void MaxChgCurFactorProcess()
{
    int32_t  arr_pack_cur_temp[BMS_PACK_MAX_NUM];
    static float    all_pack_chg_factor;
    bool            all_pack_chg_ing_flag           = 1;            //所有包都处于充电状态标志
    int32_t         signal_pack_chg_current_max_val = 0;            //最大充电电流
    
    uint8_t         idle_pack_num                   = 0;            //不充电、不放电的Pack数量

    int32_t         max_chg_cur_limit_after         = 0;            //校准后最大充电电流
    int32_t         max_chg_cur_limit               = 0;            //校准后最大充电电流（最终使用值）
    static uint32_t default_cur_max_chg_current     = 500;          //默认充电电流,0.1A
    
    //所有bms充电模式判断
    for(int i = 0; i < garrModbusOnlyReadReg[0].bms_num; i++)
    {
        arr_pack_cur_temp[i] = (uint32_t)(garrModbusOnlyReadReg[i].cell_current_high << 16) | garrModbusOnlyReadReg[i].cell_current_low;
//        all_pack_chg_ing_flag = 1;
        if(arr_pack_cur_temp[i] > (PACK_CURRENT_MIN_THRESHOLD / 100)) //单位0.1A(不处于放电状态即判断为充电状态)
        {                                                             //（因为除了充电中的Pack，还有充满电断开充电MOS的Pack）
            all_pack_chg_ing_flag = 0;
            break;
        }
        //统计不充电、不放电的电池包数量
        if((arr_pack_cur_temp[i] > -(PACK_CURRENT_MIN_THRESHOLD / 100)) && (arr_pack_cur_temp[i] < (PACK_CURRENT_MIN_THRESHOLD / 100))) 
        {            
            //所有包都不充放电，不判断处于充电状态
            idle_pack_num++;
            if(idle_pack_num >= garrModbusOnlyReadReg[0].bms_num)
            {
                all_pack_chg_ing_flag = 0;
                break;
            }
        }
    }
    
    //取正值
    for(int i = 0; i < garrModbusOnlyReadReg[0].bms_num; i++)
    {
        if(arr_pack_cur_temp[i] < 0)
        {
            arr_pack_cur_temp[i] = -arr_pack_cur_temp[i];
        }
    }
    
    //全处于充电状态
    if(all_pack_chg_ing_flag == 1)
    {
        //系数变更
        for(int i = 0; i < garrModbusOnlyReadReg[0].bms_num; i++)
        {
            if(signal_pack_chg_current_max_val < arr_pack_cur_temp[i])
            {
                signal_pack_chg_current_max_val = arr_pack_cur_temp[i];
            }
        }
            
        //计算校准系数
        all_pack_chg_factor = (float)default_cur_max_chg_current / signal_pack_chg_current_max_val;

    
        //充电数据更新
        for(int i = 0; i < gUartPackInfo.bms_num; i++)
        {
            //单节电芯最大电压在3.4V - 3.6V间，功率逐步从100%被限制到0%     
            _ChgEndingFactorSet(garrModbusOnlyReadReg[i].cell_max_voltage, i);

            //计算校准后的最大允许电流
            //系数限值
            if(all_pack_chg_factor < 0.0f || signal_pack_chg_current_max_val < 15)
            {   //电流太小时使用
                max_chg_cur_limit = default_cur_max_chg_current * gstAllPackLimit.chg_ending_factor[i];   
            }
            else
            {   //正常
                max_chg_cur_limit = arr_pack_cur_temp[i] * all_pack_chg_factor * gstAllPackLimit.chg_ending_factor[i];   
            }
            
            //电池没有到充电末端没有检测到充电电流时，允许电流强制设置为5A
            if(garrModbusOnlyReadReg[i].cell_max_voltage < (CHARGE_MAX_VOLTAGE - 200))
            {
                if(max_chg_cur_limit < 20)
                {
                    max_chg_cur_limit = 50;
                }
            }
            //记录
            gstAllPackLimit.arr_chg_current_limit[i] = max_chg_cur_limit;

            //充电时恢复放电最大功率(充电末端功率缓慢增加，最大值为1)
            //SOC从0升至5时，从当前的功率系数逐渐恢复至1.0；
            if(garrModbusOnlyReadReg[i].soc_real <= 70)
            {
                gstAllPackLimit.dsg_ending_factor[i]         = InterpolationOperation(0, 50, 0, 100, garrModbusOnlyReadReg[i].soc_real) / 100;
                //上下限设置
                gstAllPackLimit.dsg_ending_factor[i]         = (gstAllPackLimit.dsg_ending_factor[i] > 1) ? 1 : gstAllPackLimit.dsg_ending_factor[i];
                gstAllPackLimit.dsg_ending_factor[i]         = (gstAllPackLimit.dsg_ending_factor[i] < 0.1f) ? 0.1f : gstAllPackLimit.dsg_ending_factor[i];
            }
            gstAllPackLimit.dsg_end_factor[i]                = 1;
            gstAllPackLimit.arr_dsg_current_limit_10s[i]     = DEFAULT_DSG_MAX_CURRENT_VAL * gstAllPackLimit.dsg_ending_factor[i];
            gstAllPackLimit.arr_dsg_current_limit_30s[i]     = DEFAULT_DSG_MAX_CURRENT_VAL * gstAllPackLimit.dsg_ending_factor[i];
            gstAllPackLimit.arr_dsg_current_limit_60s[i]     = DEFAULT_DSG_MAX_CURRENT_VAL * gstAllPackLimit.dsg_ending_factor[i];
        }
    }

}

/**
 * @brief  : 放电允许电流限制
 * @para   : None
 * @note   : None
 * @return : None
 */
void MaxDsgCurFactorProcess()
{
    int32_t  arr_pack_cur_temp[BMS_PACK_MAX_NUM];
    static float    all_pack_dsg_factor;
    static uint32_t factor_change_interval_time         = 0;                //更改放电系数间隔时间
    bool            all_pack_dsg_ing_flag               = 0;                //所有包都处于放电状态标志
    int32_t  signal_pack_dsg_current_max_val            = 0;                //最大放充电电流
    uint8_t         idle_pack_num                       = 0;                //不充电、不放电的Pack数量
    
    static bool     first_enter_dsg_piont               = true;             //所有包首次进入放电状态标志
    static int32_t  chg_factor_start_piont[BMS_PACK_MAX_NUM];               //充电系数恢复的起始位置
    
    int32_t         max_dsg_cur_limit_val_10s           = 0;                //10s最大允许放电电流
    int32_t         max_dsg_cur_limit_val_30s           = 0;                //30s最大允许放电电流
    int32_t         max_dsg_cur_limit_val_60s           = 0;                //60s最大允许放电电流
    
    float           empty_dsg_factor                    = 1.0f;         //放电功率系数（快放空时使用）
    static uint16_t chg_recover_start_soc[BMS_PACK_MAX_NUM];
    
    //所有bms放电模式判断
    for(int i = 0; i < garrModbusOnlyReadReg[0].bms_num; i++)
    {
        arr_pack_cur_temp[i] = (uint32_t)(garrModbusOnlyReadReg[i].cell_current_high << 16) | garrModbusOnlyReadReg[i].cell_current_low;
          
        all_pack_dsg_ing_flag = 1;
        if(arr_pack_cur_temp[i] < -(PACK_CURRENT_MIN_THRESHOLD / 100)) //单位0.1A
        {
            first_enter_dsg_piont = true;
            all_pack_dsg_ing_flag = 0;
            break;
        }
        //统计不充电、不放电的电池包数量
        if((arr_pack_cur_temp[i] > -(PACK_CURRENT_MIN_THRESHOLD / 100)) && (arr_pack_cur_temp[i] < (PACK_CURRENT_MIN_THRESHOLD / 100))) 
        {            
            //所有包都不充放电，不判断处于放电状态
            idle_pack_num++;
            if(idle_pack_num >= garrModbusOnlyReadReg[0].bms_num)
            {
                first_enter_dsg_piont = true;
                all_pack_dsg_ing_flag = 0;
                break;
            }
        }
    }
    
    //全处于放电状态
    if(all_pack_dsg_ing_flag == 1)
    {
        //记录首次进入放电的时间点及恢复的起始点
        if(first_enter_dsg_piont)
        {
            first_enter_dsg_piont = false;
            for(int i = 0; i < BMS_PACK_MAX_NUM; i++)
            {
				chg_factor_start_piont[i] = (int32_t)(gstAllPackLimit.chg_ending_factor[i] * 100);
                chg_recover_start_soc[i] = garrModbusOnlyReadReg[i].soc_real;
            }
            
        }
        //查找Pack的最大电流
        for(int i = 0; i < garrModbusOnlyReadReg[0].bms_num; i++)
        {
            if(signal_pack_dsg_current_max_val < arr_pack_cur_temp[i])
            {
                signal_pack_dsg_current_max_val = arr_pack_cur_temp[i];
            }
        }

        //计算校准系数
        all_pack_dsg_factor = (float)DEFAULT_DSG_MAX_CURRENT_VAL / signal_pack_dsg_current_max_val;


        //放电数据更新
        for(int i = 0; i < garrModbusOnlyReadReg[0].bms_num; i++)
        {
            //放电末端功率系数计算
            _DsgEndingFactorSet(garrModbusOnlyReadReg[i].cell_min_voltage, i);
            
            //计算校准后的最大允许电流
            if((all_pack_dsg_factor < 0.0f) || (signal_pack_dsg_current_max_val < 30))
            {
                //小电流
                max_dsg_cur_limit_val_10s = DEFAULT_DSG_MAX_CURRENT_VAL * empty_dsg_factor;
                max_dsg_cur_limit_val_30s = DEFAULT_DSG_MAX_CURRENT_VAL * empty_dsg_factor;
                max_dsg_cur_limit_val_60s = DEFAULT_DSG_MAX_CURRENT_VAL * empty_dsg_factor;

            }
            else
            {
                //电流足够大的情况下计算
                max_dsg_cur_limit_val_10s = arr_pack_cur_temp[i] * all_pack_dsg_factor * empty_dsg_factor;
                max_dsg_cur_limit_val_30s = arr_pack_cur_temp[i] * all_pack_dsg_factor * empty_dsg_factor;
                max_dsg_cur_limit_val_60s = arr_pack_cur_temp[i] * all_pack_dsg_factor * empty_dsg_factor;
            }
                

            //电池没有到放电末端没有检测到放电电流时，允许放电电流强制设置为20A
            if(garrModbusOnlyReadReg[i].cell_min_voltage > (DISCHARGE_MIN_VOLTAGE + 200))
            {
                if(max_dsg_cur_limit_val_10s < 20)
                {
                    max_dsg_cur_limit_val_10s = 50;
                }
                if(max_dsg_cur_limit_val_30s < 20)
                {
                    max_dsg_cur_limit_val_30s = 50;
                }
                if(max_dsg_cur_limit_val_60s < 20)
                {
                    max_dsg_cur_limit_val_60s = 50;
                }
            }
            
        
            gstAllPackLimit.arr_dsg_current_limit_10s[i] = max_dsg_cur_limit_val_10s;
            gstAllPackLimit.arr_dsg_current_limit_30s[i] = max_dsg_cur_limit_val_30s;
            gstAllPackLimit.arr_dsg_current_limit_60s[i] = max_dsg_cur_limit_val_60s; 
            
            //放电时重置充电最大功率(充电末端功率缓慢增加，最大值为1)
            //SOC从99降至95时，从当前的功率系数逐渐恢复至1.0；
            if(garrModbusOnlyReadReg[i].soc_real >= 990)
            {
                gstAllPackLimit.chg_ending_factor[i] = 0.1;
            }
            else if((garrModbusOnlyReadReg[i].soc_real >= 940) && (garrModbusOnlyReadReg[i].soc_real < 990))
            {
                gstAllPackLimit.chg_ending_factor[i]         = InterpolationOperation(990, 950, 0, 100, garrModbusOnlyReadReg[i].soc_real) / 100;
                //上下限设置
                gstAllPackLimit.chg_ending_factor[i]         = (gstAllPackLimit.chg_ending_factor[i] > 1) ? 1 : gstAllPackLimit.chg_ending_factor[i];
                gstAllPackLimit.chg_ending_factor[i]         = (gstAllPackLimit.chg_ending_factor[i] < 0.1f) ? 0.1f : gstAllPackLimit.chg_ending_factor[i];
            }
            //解决SOC比真正的SOC低时，充电功率无法恢复的问题
            else if((garrModbusOnlyReadReg[i].soc_real > 200) && (garrModbusOnlyReadReg[i].soc_real < 940))
            {
                gstAllPackLimit.chg_ending_factor[i]         = InterpolationOperation(chg_recover_start_soc[i], ( chg_recover_start_soc[i] - 40), chg_factor_start_piont[i], 100, garrModbusOnlyReadReg[i].soc_real) / 100;
                //上下限设置
                gstAllPackLimit.chg_ending_factor[i]         = (gstAllPackLimit.chg_ending_factor[i] > 1) ? 1 : gstAllPackLimit.chg_ending_factor[i];
                gstAllPackLimit.chg_ending_factor[i]         = (gstAllPackLimit.chg_ending_factor[i] < 0.2f) ? 0.2f : gstAllPackLimit.chg_ending_factor[i];
            }
            else if((garrModbusOnlyReadReg[i].soc_real >= 0) && (garrModbusOnlyReadReg[i].soc_real <= 200))
            {
                gstAllPackLimit.chg_ending_factor[i] = 1;
            }    
            
            gstAllPackLimit.chg_end_factor[i]            = 1;
            gstAllPackLimit.arr_chg_current_limit[i]     = DEFAULT_CHG_MAX_CURRENT_VAL * gstAllPackLimit.chg_ending_factor[i];
        }
    }  
}

/**
 * @brief  : 最大电流根据故障进行降低
 * @para   : None
 * @note   : None
 * @return : None
 */
void FaultPowerFactorProcess()
{
    BMSErrorInfo_t  all_pack_error_info;       //所有包的故障信息
    int32_t max_dsg_cur_limit_val_10s = 0;     //10S最大放电允许电流
    int32_t max_dsg_cur_limit_val_30s = 0;     //30S最大放电允许电流
    int32_t max_dsg_cur_limit_val_60s = 0;     //60S最大放电允许电流
    int32_t max_chg_cur_limit         = 0;     //最大充电允许电流

    float chg_power_factor = 1.0f;             //充电功率系数
    float dsg_power_factor = 1.0f;             //放电功率系数
    
    memset(&all_pack_error_info, 0, sizeof(BMSErrorInfo_t));
    for(int i = 0; i < garrModbusOnlyReadReg[0].bms_num; i++)
    {
        all_pack_error_info.mos_error_code.value        |= garrModbusOnlyReadReg[i].mos_error_code;
        all_pack_error_info.voltage1_error_code.value   |= garrModbusOnlyReadReg[i].voltage1_error_code;
        all_pack_error_info.voltage2_error_code.value   |= garrModbusOnlyReadReg[i].voltage2_error_code;
        all_pack_error_info.curr_error_code.value       |= garrModbusOnlyReadReg[i].curr_error_code;
        all_pack_error_info.temper_error_code.value     |= garrModbusOnlyReadReg[i].temp_error_code;
        all_pack_error_info.state_error_code.value      |= garrModbusOnlyReadReg[i].state_error_code;
        all_pack_error_info.other1_error_code.value     |= garrModbusOnlyReadReg[i].other1_error_code;
    }
    
    //充放电温度高,功率降50%
    if(all_pack_error_info.temper_error_code.b.cell_high_temper1)
    {
        chg_power_factor = 0.5f;
        dsg_power_factor = 0.5f;
    }
    
    //mos过温一级功率降50%
    if(all_pack_error_info.mos_error_code.b.mos_high_temper1)
    {
        //不允许继续放电，充电功率减半
        chg_power_factor = 0.5f;
        dsg_power_factor = 0;
    }

    //mos过温2级功率降到0
    if(all_pack_error_info.mos_error_code.b.mos_high_temper2)
    {
        chg_power_factor = 0;
        dsg_power_factor = 0;
    }
    
    //电池二级过压，关闭充电MOS，打开放电mos
    if(all_pack_error_info.voltage1_error_code.b.cell_over_vol2)
    {
        chg_power_factor = 0;
    }
    
    //电池二级欠压，关闭放电MOS，打开充电MOS
    if(all_pack_error_info.voltage1_error_code.b.cell_under_vol2)
    {
        dsg_power_factor = 0;
        chg_power_factor = 0.2f;
    }
    
    //二级充电过流，关闭mos
    if(all_pack_error_info.curr_error_code.b.charge_overcurr2)//降功率为0
    {
        chg_power_factor = 0;
    }
    
    //二级放电过流，关闭mos
    if(all_pack_error_info.curr_error_code.b.discharge_overcurr2)//放电过流
    {
        dsg_power_factor = 0;
    }

    //充放电温度高,功率降到0%
    if(all_pack_error_info.temper_error_code.b.cell_high_temper2)
    {
        chg_power_factor = 0;
        dsg_power_factor = 0;
    }

    //充放电温度高功率降到0%
    if(all_pack_error_info.temper_error_code.b.cell_low_temper1)
    {
        chg_power_factor = 0;
        dsg_power_factor = 0;
    }
    
    //充放电数据更新
    for(int i = 0; i < garrModbusOnlyReadReg[0].bms_num; i++)
    {
        //接收源数据
        max_chg_cur_limit           = gstAllPackLimit.arr_chg_current_limit[i];
        max_dsg_cur_limit_val_10s   = gstAllPackLimit.arr_dsg_current_limit_10s[i];
        max_dsg_cur_limit_val_30s   = gstAllPackLimit.arr_dsg_current_limit_30s[i];
        max_dsg_cur_limit_val_60s   = gstAllPackLimit.arr_dsg_current_limit_60s[i];
        
        //数据更新
        max_chg_cur_limit          *= chg_power_factor;
        max_dsg_cur_limit_val_10s  *= dsg_power_factor;
        max_dsg_cur_limit_val_30s  *= dsg_power_factor;
        max_dsg_cur_limit_val_60s  *= dsg_power_factor;
        
        //限制最大充电电流
        gstAllPackLimit.arr_chg_current_limit[i]   = max_chg_cur_limit;
        if(gstAllPackLimit.arr_chg_current_limit[i] >= 500)
        {
            int x = 0;
            x++;
        }
        
        //限制最大放电电流
        gstAllPackLimit.arr_dsg_current_limit_10s[i]   = max_dsg_cur_limit_val_10s;
        gstAllPackLimit.arr_dsg_current_limit_30s[i]   = max_dsg_cur_limit_val_30s;
        gstAllPackLimit.arr_dsg_current_limit_60s[i]   = max_dsg_cur_limit_val_60s;

    }

}


/**
 * @brief  : 满充或放空时，充放电因数设置处理
* @para    : None
 * @note   : None
 * @return : None
 */
void FullOrEmptyFactorProcess()
{
    int32_t max_dsg_cur_limit_val_10s = 0;     //10S最大放电允许电流
    int32_t max_dsg_cur_limit_val_30s = 0;     //30S最大放电允许电流
    int32_t max_dsg_cur_limit_val_60s = 0;     //60S最大放电允许电流
    int32_t max_chg_cur_limit         = 0;     //最大充电允许电流

    static int32_t dsg_power_factor   = 1;     //放电功率系数
    
    //充满或放空时，充放电最大允许电流改为0
    //充放电数据更新
    for(int i = 0; i < gUartPackInfo.bms_num; i++)
    {
        //充电因数设置
        if(garrModbusOnlyReadReg[i].cell_max_voltage > CHARGE_MAX_VOLTAGE - 10)        //mV
        {
            gstAllPackLimit.chg_end_factor[i] = 0;
        }
        else if(garrModbusOnlyReadReg[i].cell_max_voltage < (CHARGE_MAX_VOLTAGE - 200))
        {
            gstAllPackLimit.chg_end_factor[i] = 1;
        }
        //放电因数设置
        if(garrModbusOnlyReadReg[i].cell_min_voltage < DISCHARGE_MIN_VOLTAGE + 10)        //mV
        {
            gstAllPackLimit.dsg_end_factor[i] = 0;
        }
        else if(garrModbusOnlyReadReg[i].cell_min_voltage > (DISCHARGE_MIN_VOLTAGE + 200))
        {
            gstAllPackLimit.dsg_end_factor[i] = 1;
        }
        
        //接收源数据
        max_chg_cur_limit           = gstAllPackLimit.arr_chg_current_limit[i];
        max_dsg_cur_limit_val_10s   = gstAllPackLimit.arr_dsg_current_limit_10s[i];
        max_dsg_cur_limit_val_30s   = gstAllPackLimit.arr_dsg_current_limit_30s[i];
        max_dsg_cur_limit_val_60s   = gstAllPackLimit.arr_dsg_current_limit_60s[i];
        
        //数据更新
        max_chg_cur_limit          *= gstAllPackLimit.chg_end_factor[i];
        max_dsg_cur_limit_val_10s  *= gstAllPackLimit.dsg_end_factor[i];
        max_dsg_cur_limit_val_30s  *= gstAllPackLimit.dsg_end_factor[i];
        max_dsg_cur_limit_val_60s  *= gstAllPackLimit.dsg_end_factor[i];
        
        //限制最大充电电流
        gstAllPackLimit.arr_chg_current_limit[i]   = max_chg_cur_limit;
        //限制最大放电电流
        gstAllPackLimit.arr_dsg_current_limit_10s[i]   = max_dsg_cur_limit_val_10s;
        gstAllPackLimit.arr_dsg_current_limit_30s[i]   = max_dsg_cur_limit_val_30s;
        gstAllPackLimit.arr_dsg_current_limit_60s[i]   = max_dsg_cur_limit_val_60s;
    
    }    
}


/**
 * @brief  : 电池包欠压，对充电功率进行限制
 * @para   : None
 * @note   : None
 * @return : None
 */
void UnderVFactorProcess()
{
    static bool uv_flag[BMS_PACK_MAX_NUM];         //欠压标志
    int uv_pack_num = 0;             //欠压Pack的数量
    int all_pack_chg_current = 0;    //允许充电电流
    
    //充电数据更新
    for(int i = 0; i < gUartPackInfo.bms_num; i++)
    {
        //最低节电芯电压在2.5V以下，所有Pack的充电功率固定为10A     
        if(garrModbusOnlyReadReg[i].cell_min_voltage < DISCHARGE_MIN_VOLTAGE -200)
        {
            uv_flag[i] = 1;
        }
        else if(garrModbusOnlyReadReg[i].cell_min_voltage > DISCHARGE_MIN_VOLTAGE + 200)
        {
            uv_flag[i] = 0;
        }
    }
    
    //没有电池包欠压，不进行该部分限制
    for(int i=0; i<gUartPackInfo.bms_num; i++)
    {
        if(uv_flag[i] == 1)
        {
            uv_pack_num++;
            break;
        }
    }
    
    if(uv_pack_num == 0)
    {
        return;
    }
    
    //计算所有Pack的允许充电功率
    all_pack_chg_current = 200 * uv_pack_num / gUartPackInfo.bms_num;
    
    //将功率分至所有Pack
    for(int i = 0; i < gUartPackInfo.bms_num; i++)
    {
        gstAllPackLimit.arr_chg_current_limit[i] = all_pack_chg_current;
    }
}

void _MaxCurValSet()
{
    for(int i = 0; i < gUartPackInfo.bms_num; i++)
    {
        gstAllPackLimit.arr_chg_limit_final[i]         = gstAllPackLimit.arr_chg_current_limit[i]      ;
        gstAllPackLimit.arr_dsg_limit_10s_final[i]     = gstAllPackLimit.arr_dsg_current_limit_10s[i]  ;
        gstAllPackLimit.arr_dsg_limit_30s_final[i]     = gstAllPackLimit.arr_dsg_current_limit_30s[i]  ;     
        gstAllPackLimit.arr_dsg_limit_60s_final[i]     = gstAllPackLimit.arr_dsg_current_limit_60s[i]  ;
    }
}

/**
 * @brief  : 最大允许电流设置
* @para    : None
 * @note   : None
 * @return : None
 */
void MaxCurControl()
{
    //不是第一个电池包，此函数不运行
    if((gUartPackInfo.ModbusAddr != MODBUS_ADDR_PACK0) || (gUartPackInfo.com_success_flag == 0))
    {
        return;
    }
    
    //确保所有包的数据最少收集上来一轮
    for(int i=0; i<gUartPackInfo.bms_num; i++)
    {
        if(garrModbusOnlyReadReg[i].software_version == 0)
        {
            return;
        }
    }
    
    MaxChgCurFactorProcess();                  //充电因数处理
    MaxDsgCurFactorProcess();                  //放电因数处理
    FaultPowerFactorProcess();                 //故障降功率处理
    FullOrEmptyFactorProcess();                //满充空放降功率处理
    UnderVFactorProcess();                     //欠压充电功率限制（必须放在最后，充电功率设定为固定值）
    _MaxCurValSet();
}
