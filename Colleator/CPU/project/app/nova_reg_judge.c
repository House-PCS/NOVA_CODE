

#include "nova_reg_judge.h"



bool NovaRegBaseJudge(uint16_t reg_addr, uint16_t reg_data)
{
    //uint16_t reg_value = ((reg_data & 0x00FF) << 8) | ((reg_data & 0xFF00) >> 8);
    uint16_t reg_value = reg_data;
    if(reg_addr == 3044)/*系统应用场景*/
    {
        return true;
    }
    else if(reg_addr == 3078)/*电池放电功率调度*/
    {
        if(reg_value > 100)
        {
            return false;
        }
    }
    else if(reg_addr == 3079)/*电池充电功率调度*/
    {
        if(reg_value > 20)/*当前限制保守一些*/
        {
            return false;
        }
    }
    else if(reg_addr == 3182)/*电池充电电流设置值*/
    {
        if(reg_value > 20)/*当前限制保守一些，暂定20A*/
        {
            return false;
        }
    }
    else if(reg_addr == 3179)/*PV输入模式*/
    {
        if(reg_value >= 2)//仅支持输入独立和输入并联
        {
            return false;
        }
    }
    else if(reg_addr == 3184)/*BackUp输出使能*/
    {
        if(reg_value > 2)
        {
            return false;
        }
    }
    else if(reg_addr == 3184)/*市电充电最大SOC*/
    {
        if(reg_value > 100)
        {
            return false;
        }
    }
    else if(reg_addr == 3185)/*程控使能*/
    {
        return true;
    }
    else
    {
        return false;
    }
    return true;
}



bool NovaRegInvJudge(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data)
{
    if(reg_addr == 3160)/*削峰填谷时间设置*/
    {
		return true;
        uint16_t h_chg_start_1, m_chg_start_1, h_chg_start_2, m_chg_start_2;
        uint16_t h_dsg_start_1, m_dsg_start_1, h_dsg_start_2, m_dsg_start_2;
        uint16_t h_chg_end_1, m_chg_end_1, h_chg_end_2, m_chg_end_2;
        uint16_t h_dsg_end_1, m_dsg_end_1, h_dsg_end_2, m_dsg_end_2;
        if(reg_num == 8)
        {
            h_chg_start_1 = (p_reg_data[0] - 0x30) + (p_reg_data[1] - 0x30) * 10;
            m_chg_start_1 = (p_reg_data[2] - 0x30) + (p_reg_data[3] - 0x30) * 10;
            h_chg_end_1 = (p_reg_data[4] - 0x30) + (p_reg_data[5] - 0x30) * 10;
            m_chg_end_1 = (p_reg_data[6] - 0x30) + (p_reg_data[7] - 0x30) * 10;
            h_dsg_start_1 = (p_reg_data[8] - 0x30) + (p_reg_data[9] - 0x30) * 10;
            m_dsg_start_1 = (p_reg_data[10] - 0x30) + (p_reg_data[11] - 0x30) * 10;
            h_dsg_end_1 = (p_reg_data[12] - 0x30) + (p_reg_data[13] - 0x30) * 10;
            m_dsg_end_1 = (p_reg_data[14] - 0x30) + (p_reg_data[15] - 0x30) * 10;
            if(h_chg_start_1 >= 24 || h_chg_end_1 >= 24 || m_chg_start_1 >= 60 || m_chg_end_1 >= 60)//时间数据不合法
            {
                return false;
            }
            if(h_dsg_start_1 >= 24 || h_dsg_end_1 >= 24 || m_dsg_start_1 >= 60 || m_dsg_end_1 >= 60)//时间数据不合法
            {
                return false;
            }
            
            if(h_chg_start_1 * 60 + m_chg_start_1 > h_chg_end_1 * 60 + m_chg_end_1)
            {
                return false;
            }
            if(h_dsg_start_1 * 60 + m_dsg_start_1 > h_dsg_end_1 * 60 + m_dsg_end_1)
            {
                return false;
            }
        }
        else if(reg_num == 16)
        {
            h_chg_start_1 = (p_reg_data[0] - 0x30) + (p_reg_data[1] - 0x30) * 10;
            m_chg_start_1 = (p_reg_data[2] - 0x30) + (p_reg_data[3] - 0x30) * 10;
            h_chg_end_1 = (p_reg_data[4] - 0x30) + (p_reg_data[5] - 0x30) * 10;
            m_chg_end_1 = (p_reg_data[6] - 0x30) + (p_reg_data[7] - 0x30) * 10;
            h_dsg_start_1 = (p_reg_data[8] - 0x30) + (p_reg_data[9] - 0x30) * 10;
            m_dsg_start_1 = (p_reg_data[10] - 0x30) + (p_reg_data[11] - 0x30) * 10;
            h_dsg_end_1 = (p_reg_data[12] - 0x30) + (p_reg_data[13] - 0x30) * 10;
            m_dsg_end_1 = (p_reg_data[14] - 0x30) + (p_reg_data[15] - 0x30) * 10;
            
            h_chg_start_2 = (p_reg_data[16] - 0x30) + (p_reg_data[17] - 0x30) * 10;
            m_chg_start_2 = (p_reg_data[18] - 0x30) + (p_reg_data[19] - 0x30) * 10;
            h_chg_end_2 = (p_reg_data[20] - 0x30) + (p_reg_data[21] - 0x30) * 10;
            m_chg_end_2 = (p_reg_data[22] - 0x30) + (p_reg_data[23] - 0x30) * 10;
            h_dsg_start_2 = (p_reg_data[24] - 0x30) + (p_reg_data[25] - 0x30) * 10;
            m_dsg_start_2 = (p_reg_data[26] - 0x30) + (p_reg_data[27] - 0x30) * 10;
            h_dsg_end_2 = (p_reg_data[28] - 0x30) + (p_reg_data[29] - 0x30) * 10;
            m_dsg_end_2 = (p_reg_data[30] - 0x30) + (p_reg_data[31] - 0x30) * 10;
            
            if(h_chg_start_1 >= 24 || h_chg_end_1 >= 24 || m_chg_start_1 >= 60 || m_chg_end_1 >= 60)//时间数据不合法
            {
                return false;
            }
            if(h_dsg_start_1 >= 24 || h_dsg_end_1 >= 24 || m_dsg_start_1 >= 60 || m_dsg_end_1 >= 60)//时间数据不合法
            {
                return false;
            }
            if(h_chg_start_2 >= 24 || h_chg_end_2 >= 24 || m_chg_start_2 >= 60 || m_chg_end_2 >= 60)//时间数据不合法
            {
                return false;
            }
            if(h_dsg_start_2 >= 24 || h_dsg_end_2 >= 24 || m_dsg_start_2 >= 60 || m_dsg_end_2 >= 60)//时间数据不合法
            {
                return false;
            }
            
            if(h_chg_start_1 * 60 + m_chg_start_1 > h_chg_end_1 * 60 + m_chg_end_1)
            {
                return false;
            }
            if(h_dsg_start_1 * 60 + m_dsg_start_1 > h_dsg_end_1 * 60 + m_dsg_end_1)
            {
                return false;
            }
            if(h_chg_start_2 * 60 + m_chg_start_2 > h_chg_end_2 * 60 + m_chg_end_2)
            {
                return false;
            }
            if(h_dsg_start_2 * 60 + m_dsg_start_2 > h_dsg_end_2 * 60 + m_dsg_end_2)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    if(reg_addr == 3168)/*削峰填谷时间设置2*/
    {
        uint16_t h_chg_start_2, m_chg_start_2;
        uint16_t h_dsg_start_2, m_dsg_start_2;
        uint16_t h_chg_end_2, m_chg_end_2;
        uint16_t h_dsg_end_2, m_dsg_end_2;
        if(reg_num == 8)
        {
            h_chg_start_2 = (p_reg_data[0] - 0x30) + (p_reg_data[1] - 0x30) * 10;
            m_chg_start_2 = (p_reg_data[2] - 0x30) + (p_reg_data[3] - 0x30) * 10;
            h_chg_end_2 = (p_reg_data[4] - 0x30) + (p_reg_data[5] - 0x30) * 10;
            m_chg_end_2 = (p_reg_data[6] - 0x30) + (p_reg_data[7] - 0x30) * 10;
            h_dsg_start_2 = (p_reg_data[8] - 0x30) + (p_reg_data[9] - 0x30) * 10;
            m_dsg_start_2 = (p_reg_data[10] - 0x30) + (p_reg_data[11] - 0x30) * 10;
            h_dsg_end_2 = (p_reg_data[12] - 0x30) + (p_reg_data[13] - 0x30) * 10;
            m_dsg_end_2 = (p_reg_data[14] - 0x30) + (p_reg_data[15] - 0x30) * 10;
            if(h_chg_start_2 >= 24 || h_chg_end_2 >= 24 || m_chg_start_2 >= 60 || m_chg_end_2 >= 60)//时间数据不合法
            {
                return false;
            }
            if(h_dsg_start_2 >= 24 || h_dsg_end_2 >= 24 || m_dsg_start_2 >= 60 || m_dsg_end_2 >= 60)//时间数据不合法
            {
                return false;
            }
            
            if(h_chg_start_2 * 60 + m_chg_start_2 > h_chg_end_2 * 60 + m_chg_end_2)
            {
                return false;
            }
            if(h_dsg_start_2 * 60 + m_dsg_start_2 > h_dsg_end_2 * 60 + m_dsg_end_2)
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else if(reg_addr == 3239)
    {
        if(reg_num == 7)
        {
			return true;
            uint16_t year, month, day, hour, minute, second, week;
            
            year = (p_reg_data[0] - 0x30) + (p_reg_data[1] - 0x30) * 10;
            month = (p_reg_data[2] - 0x30) + (p_reg_data[3] - 0x30) * 10;
            day = (p_reg_data[4] - 0x30) + (p_reg_data[5] - 0x30) * 10;
            hour = (p_reg_data[6] - 0x30) + (p_reg_data[7] - 0x30) * 10;
            minute = (p_reg_data[8] - 0x30) + (p_reg_data[9] - 0x30) * 10;
            second = (p_reg_data[10] - 0x30) + (p_reg_data[11] - 0x30) * 10;
            week = (p_reg_data[12] - 0x30);//待定，协议描述不清楚
            
            if(year > 99 || month > 12 && day > 31 || hour >= 24 || minute >= 60 || second >= 60)
            {
                return false;
            }
            if(week > 7)//待定
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}


bool NovaRegRemoteJudge(uint16_t reg_addr, uint16_t reg_data)
{
    //uint16_t reg_value = ((reg_data & 0x00FF) << 8) | ((reg_data & 0xFF00) >> 8);
    uint16_t reg_value = reg_data;
    if(reg_addr == 3241)/*远程关机*/
	{
		return true;
	}
    else if(reg_addr == 3242)/*远程开机*/
	{
		return true;
	}
    return false;//暂不开放
}









