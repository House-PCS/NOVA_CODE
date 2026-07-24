/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "debug.h"
#include "main.h"
#define LOG_TAG     "debug"


#if PRODUCT_TEST
#else
#define DEBUGUSART_IRQHandler   USART0_IRQHandler
#endif

static uint8_t garr_debug_rx_buf[DEBUG_RX_BUF_LEN] = {0};
static uint16_t gn_debug_recv_length = 0;

/*通信管理的数据结构*/
static comm_port_t sarr_com_port = {0};

/*通信管理的静态内存数据*/
static uint8_t sarr_debug_rx_buff[COMSERVICE_DEBUG_RX_SIZE] = {0};
static uint8_t sarr_debug_tx_buff[COMSERVICE_DEBUG_TX_SIZE] = {0};

/* log */
#define LOG_DEBUG_ENABLE_STRING_LEN (20)
#define LOG_DEBUG_ENABLE_NUM (5)
#define LOG_INFO_DISABLE_STRING_LEN (20)
#define LOG_INFO_DISABLE_NUM (5)
static uint8_t sc_log_lvl = 4;  //打印等级
static uint8_t sc_log_filter = 0;
static uint8_t sarr_log_debug_en[LOG_DEBUG_ENABLE_NUM][LOG_DEBUG_ENABLE_STRING_LEN] = {0};
static uint8_t sarr_log_info_disable[LOG_INFO_DISABLE_STRING_LEN][LOG_INFO_DISABLE_NUM] = {0};

void comservice_log_info_printf(en_comservice_log_type_t e_type, char *string, char *format, ...);
UartDebugReg_t  garrUartDebugReg = {0};
UartDataPoint_t gUartDebugDataPoint[] =
{
    {DEB_CELL_VOLT_0_ADDR,      CELL_VOLT_0_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[0]     },
    {DEB_CELL_VOLT_1_ADDR,      CELL_VOLT_1_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[1]     },
    {DEB_CELL_VOLT_2_ADDR,      CELL_VOLT_2_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[2]     },
    {DEB_CELL_VOLT_3_ADDR,      CELL_VOLT_3_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[3]     },
    {DEB_CELL_VOLT_4_ADDR,      CELL_VOLT_4_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[4]     },
    {DEB_CELL_VOLT_5_ADDR,      CELL_VOLT_5_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[5]     },
    {DEB_CELL_VOLT_6_ADDR,      CELL_VOLT_6_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[6]     },
    {DEB_CELL_VOLT_7_ADDR,      CELL_VOLT_7_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[7]     },
    {DEB_CELL_VOLT_8_ADDR,      CELL_VOLT_8_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[8]     },
    {DEB_CELL_VOLT_9_ADDR,      CELL_VOLT_9_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[9]     },
    {DEB_CELL_VOLT_10_ADDR,     CELL_VOLT_10_NUM,               EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[10]    },
    {DEB_CELL_VOLT_11_ADDR,     CELL_VOLT_11_NUM,               EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[11]    },
    {DEB_CELL_VOLT_12_ADDR,     CELL_VOLT_12_NUM,               EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[12]    },
    {DEB_CELL_VOLT_13_ADDR,     CELL_VOLT_13_NUM,               EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[13]    },
    {DEB_CELL_VOLT_14_ADDR,     CELL_VOLT_14_NUM,               EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[14]    },
    {DEB_CELL_VOLT_15_ADDR,     CELL_VOLT_15_NUM,               EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[15]    },
    {DEB_CELL_VOLT_16_ADDR,     CELL_VOLT_16_NUM,               EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[16]    },
    {DEB_CELL_VOLT_17_ADDR,     CELL_VOLT_17_NUM,               EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[17]    },
    {DEB_CELL_VOLT_18_ADDR,     CELL_VOLT_18_NUM,               EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[18]    },
    {DEB_CELL_VOLT_19_ADDR,     CELL_VOLT_19_NUM,               EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_vlt[19]    },
    {DEB_CELL_MAX_VOLT_ADDR,    CELL_MAX_VOLT_NUM,              EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_max_vlt    },
    {DEB_CELL_MIN_VOLT_ADDR,    CELL_MIN_VOLT_NUM,              EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_min_vlt    },
    {DEB_CELL_SUM_VOLT_ADDR,    CELL_SUM_VOLT_NUM,              EN_DATA_U32,   EN_ATTR_WO,        &garrUartDebugReg.cell_sum_vlt    },
    {DEB_CELL_TEMP_0_ADDR,      CELL_TEMP_0_NUM,                EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[0]    },
    {DEB_CELL_TEMP_1_ADDR,      CELL_TEMP_1_NUM,                EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[1]    },
    {DEB_CELL_TEMP_2_ADDR,      CELL_TEMP_2_NUM,                EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[2]    },
    {DEB_CELL_TEMP_3_ADDR,      CELL_TEMP_3_NUM,                EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[3]    },
    {DEB_CELL_TEMP_4_ADDR,      CELL_TEMP_4_NUM,                EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[4]    },
    {DEB_CELL_TEMP_5_ADDR,      CELL_TEMP_5_NUM,                EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[5]    },
    {DEB_CELL_TEMP_6_ADDR,      CELL_TEMP_6_NUM,                EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[6]    },
    {DEB_CELL_TEMP_7_ADDR,      CELL_TEMP_7_NUM,                EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[7]    },
    {DEB_CELL_TEMP_8_ADDR,      CELL_TEMP_8_NUM,                EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[8]    },
    {DEB_CELL_TEMP_9_ADDR,      CELL_TEMP_9_NUM,                EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[9]    },
    {DEB_CELL_TEMP_10_ADDR,     CELL_TEMP_10_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[10]   },
    {DEB_CELL_TEMP_11_ADDR,     CELL_TEMP_11_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[11]   },
    {DEB_CELL_TEMP_12_ADDR,     CELL_TEMP_12_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[12]   },
    {DEB_CELL_TEMP_13_ADDR,     CELL_TEMP_13_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[13]   },
    {DEB_CELL_TEMP_14_ADDR,     CELL_TEMP_14_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[14]   },
    {DEB_CELL_TEMP_15_ADDR,     CELL_TEMP_15_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[15]   },
    {DEB_CELL_TEMP_16_ADDR,     CELL_TEMP_16_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[16]   },
    {DEB_CELL_TEMP_17_ADDR,     CELL_TEMP_17_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[17]   },
    {DEB_CELL_TEMP_18_ADDR,     CELL_TEMP_18_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[18]   },
    {DEB_CELL_TEMP_19_ADDR,     CELL_TEMP_19_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_temp[19]   },
    {DEB_CELL_MAX_TEMP_ADDR,    CELL_MAX_TEMP_NUM,              EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_max_temp   },
    {DEB_CELL_MIN_TEMP_ADDR,    CELL_MIN_TEMP_NUM,              EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_min_temp   },
    {DEB_AFE_TEMP_ADDR,         AFE_TEMP_NUM,                   EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.afe_temp        },
    {DEB_MOS_TEMP_ADDR,         MOS_TEMP_NUM,                   EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.mos_temp        },
    {DEB_AMBIENT_TEMP_ADDR,     AMBIENT_TEMP_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.ambient_temp    },
    {DEB_PACK_CURRENT_ADDR,     PACK_CURRENT_NUM,               EN_DATA_S32,   EN_ATTR_WO,        &garrUartDebugReg.pack_current    },
    {DEB_CURRENT_POWER_ADDR,    CURRENT_POWER_NUM,              EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.current_power   },
    {DEB_HEAT_STATUS_ADDR,      HEAT_STATUS_NUM,                EN_DATA_U32,   EN_ATTR_WO,        &garrUartDebugReg.heat_status     },
    {DEB_SOC_ABT_CELL_0_ADDR,   CELL_SOC_ABT_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_soc_abt[0] },
    {DEB_SOC_ABT_CELL_1_ADDR,   CELL_SOC_ABT_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_soc_abt[1] },
    {DEB_SOC_ABT_CELL_2_ADDR,   CELL_SOC_ABT_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_soc_abt[2] },
    {DEB_SOC_ABT_CELL_3_ADDR,   CELL_SOC_ABT_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_soc_abt[3] },
    {DEB_SOC_ABT_CELL_4_ADDR,   CELL_SOC_ABT_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_soc_abt[4] },
    {DEB_SOC_ABT_CELL_5_ADDR,   CELL_SOC_ABT_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_soc_abt[5] },
    {DEB_SOC_ABT_CELL_6_ADDR,   CELL_SOC_ABT_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_soc_abt[6] },
    {DEB_SOC_ABT_CELL_7_ADDR,   CELL_SOC_ABT_NUM,               EN_DATA_S16,   EN_ATTR_WO,        &garrUartDebugReg.cell_soc_abt[7] },
    {DEB_CELL_SOC_MIN_RLT_ADDR, CELL_SOC_MIN_RLT_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_soc_min_rlt},
    {DEB_CELL_SOC_MAX_RLT_ADDR, CELL_SOC_MAX_RLT_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.cell_soc_max_rlt},
    {DEB_PACK_SOC_RLT_ADDR,     SOC_RLT_NUM,                    EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.pack_soc_rlt    },
    {DEB_PACK_SOC_DSP_ADDR,     SOC_DSP_NUM,                    EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.pack_soc_dsp    },
    {DEB_PACK_HEALTH_ADDR,      PACK_HEALTH_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.pack_health     },
    {DEB_PACK_STATUS_ADDR,      PACK_STATUS_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.pack_status     },
    {DEB_RECHARGING_TIME_ADDR,  RECHARGING_TIME_NUM,            EN_DATA_U32,   EN_ATTR_WO,        &garrUartDebugReg.recharge_time   },
    {DEB_MOS_STATUS_ADDR,       MOS_STATUS_NUM,                 EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.mos_status      },
    {DEB_BALANCE_STATUS_ADDR,   BALANCE_STATUS_NUM,             EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.balance_status  },
    {DEB_FAULT_LEVEL_ADDR,      FAULT_LEVEL_NUM,                EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_lvl         },
    {DEB_FAULT_NUM_CODE_0_ADDR, FAULT_NUM_CODE_0_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[0]      },
    {DEB_FAULT_NUM_CODE_1_ADDR, FAULT_NUM_CODE_1_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[1]      },
    {DEB_FAULT_NUM_CODE_2_ADDR, FAULT_NUM_CODE_2_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[2]      },
    {DEB_FAULT_NUM_CODE_3_ADDR, FAULT_NUM_CODE_3_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[3]      },
    {DEB_FAULT_NUM_CODE_4_ADDR, FAULT_NUM_CODE_4_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[4]      },
    {DEB_FAULT_NUM_CODE_5_ADDR, FAULT_NUM_CODE_5_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[5]      },
    {DEB_FAULT_NUM_CODE_6_ADDR, FAULT_NUM_CODE_6_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[6]      },
    {DEB_FAULT_NUM_CODE_7_ADDR, FAULT_NUM_CODE_7_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[7]      },
    {DEB_FAULT_NUM_CODE_8_ADDR, FAULT_NUM_CODE_8_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[8]      },
    {DEB_FAULT_NUM_CODE_9_ADDR, FAULT_NUM_CODE_9_NUM,           EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[9]      },
    {DEB_FAULT_NUM_CODE_10_ADDR, FAULT_NUM_CODE_10_NUM,         EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[10]     },
    {DEB_FAULT_NUM_CODE_11_ADDR, FAULT_NUM_CODE_11_NUM,         EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[11]     },
    {DEB_FAULT_NUM_CODE_12_ADDR, FAULT_NUM_CODE_12_NUM,         EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[12]     },
    {DEB_FAULT_NUM_CODE_13_ADDR, FAULT_NUM_CODE_13_NUM,         EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[13]     },
    {DEB_FAULT_NUM_CODE_14_ADDR, FAULT_NUM_CODE_14_NUM,         EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[14]     },
    {DEB_FAULT_NUM_CODE_15_ADDR, FAULT_NUM_CODE_15_NUM,         EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[15]     },
    {DEB_FAULT_NUM_CODE_16_ADDR, FAULT_NUM_CODE_16_NUM,         EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[16]     },
    {DEB_FAULT_NUM_CODE_17_ADDR, FAULT_NUM_CODE_17_NUM,         EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[17]     },
    {DEB_FAULT_NUM_CODE_18_ADDR, FAULT_NUM_CODE_18_NUM,         EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[18]     },
    {DEB_FAULT_NUM_CODE_19_ADDR, FAULT_NUM_CODE_19_NUM,         EN_DATA_U16,   EN_ATTR_WO,        &garrUartDebugReg.flt_dtc[19]     },
    {DEB_MAX_CHARGE_CURRENT_ADDR, DEB_MAX_CHARGE_CURRENT_NUM,    EN_DATA_U32,   EN_ATTR_WO,        &garrUartDebugReg.max_chg_current },
    {DEB_MAX_DS_CURRENT_ADDR,    DEB_MAX_DS_CURRENT_NUM,        EN_DATA_U32,   EN_ATTR_WO,        &garrUartDebugReg.max_dsg_current },
    {DEB_MAX_DS_CURRENT_30S_ADDR, DEB_MAX_DS_CURRENT_30S_NUM,    EN_DATA_U32,   EN_ATTR_WO,        &garrUartDebugReg.max_dsg_30s     },
    {DEB_MAX_DS_CURRENT_60S_ADDR, DEB_MAX_DS_CURRENT_60S_NUM,    EN_DATA_U32,   EN_ATTR_WO,        &garrUartDebugReg.max_dsg_60s     },
};
uint16_t gUartDebDataLen = (sizeof(gUartDebugDataPoint) / sizeof(UartDataPoint_t));
static int32_t _debug_data_cpy(uint8_t *p_data, uint32_t n_len)
{
    int32_t n_ret = 0;
    uint32_t *p_w_cnt = NULL;
    uint32_t n_r_cnt = NULL;

    if(p_data == NULL)
    {
        n_ret = 0xFF;
    }
    /*判断当前空间是否还有空余可以写入参数*/
    p_w_cnt = &sarr_com_port.rx_obj.n_w_cnt;
    n_r_cnt = sarr_com_port.rx_obj.n_r_cnt;

    if(sarr_com_port.rx_obj.p_buf == p_data)
    {
        /*为了减少互相拷贝的时间，共用了部分通信接口的通信内存*/
        *p_w_cnt = n_len;
    }
    else if(*p_w_cnt < n_r_cnt)
    {
        /*写指针在读指针之前(循环了一圈)*/
        if((*p_w_cnt + n_len) <= n_r_cnt)
        {
            memcpy(&sarr_com_port.rx_obj.p_buf[*p_w_cnt], p_data, n_len);
            *p_w_cnt += n_len;
        }
        else
        {
            n_ret = 0xFF;
        }
    }
    else
    {
        /*写指针在读指针之后(未循环)*/
        uint32_t n_w_remnant_len = sarr_com_port.rx_obj.n_mem_len - *p_w_cnt;

        if((*p_w_cnt + n_len) <= sarr_com_port.rx_obj.n_mem_len)
        {
            /*队列后方有空可以放下所有数据*/
            memcpy(&sarr_com_port.rx_obj.p_buf[*p_w_cnt], p_data, n_len);
            *p_w_cnt += n_len;
        }
        else if(((*p_w_cnt + n_len) > sarr_com_port.rx_obj.n_mem_len) && \
                ((n_len - n_w_remnant_len) <=  n_r_cnt))
        {
            /*队列后方 + 读指针之前有空可以放下所有数据*/
            memcpy(&sarr_com_port.rx_obj.p_buf[*p_w_cnt], p_data, n_w_remnant_len);
            memcpy(sarr_com_port.rx_obj.p_buf, &p_data[n_w_remnant_len], (n_len - n_w_remnant_len));
            *p_w_cnt = n_len - n_w_remnant_len;
        }
        else
        {
            n_ret = 0xFF;
        }
    }
    return n_ret;
}


static void _irq_rx_process(void)
{
    if(Uart0_Handle.pRxBuf == NULL)
    {
        return;
    }

    if(Uart0_Handle.uart_cfg->en_dma_rx == true)
    {
        /* 处理DMA接收到的数据 */
        // 计算实际接收的数据长度
        gn_debug_recv_length = DEBUG_RX_BUF_LEN - dma_transfer_number_get(Uart0_Handle.uart_cfg->dma_rx_peri, Uart0_Handle.uart_cfg->dma_rx_channel);
        _debug_data_cpy(garr_debug_rx_buf, gn_debug_recv_length);
//    garr_debug_rx_buf[gn_debug_recv_length] = '\0';
//    printf("debug recv len:%d\t", gn_debug_recv_length);
//    printf("g_recv_buff:%s\r\n",garr_debug_rx_buf);     // 打印接收的数据

        /* 重新设置DMA传输 */
        UartDMARxDisable(&Uart0_Handle);
        UartDMARxEnable(&Uart0_Handle);
    }
    else
    {
        _debug_data_cpy(Uart0_Handle.pRxBuf, 1);
    }

}

/**
  * @brief  mid comservice com port rx data
  * @param  None
  * @retval None
  */
static void _debug_comservice_rx(void)
{
    uint32_t i = 0;
    int32_t  n_ret       = 0;
    uint32_t n_len_first = 0;   /*第一段数据长度*/
    uint32_t n_len_second = 0;  /*第二段折叠回来的数据长度*/

    if(sarr_com_port.rx_obj.n_w_cnt != sarr_com_port.rx_obj.n_r_cnt)
    {
        if(sarr_com_port.rx_obj.n_w_cnt > sarr_com_port.rx_obj.n_r_cnt)
        {
            /*写指针在读指针之后*/
            n_len_first = sarr_com_port.rx_obj.n_w_cnt - sarr_com_port.rx_obj.n_r_cnt;
        }
        else
        {
            /*写指针在读指针之前*/
            n_len_first   = sarr_com_port.rx_obj.n_mem_len - sarr_com_port.rx_obj.n_r_cnt;
            n_len_second  = sarr_com_port.rx_obj.n_w_cnt;
        }

        /*debug 处理*/
        if(n_len_first != 0)
        {
            console_recive_handler(&sarr_com_port.rx_obj.p_buf[sarr_com_port.rx_obj.n_r_cnt], n_len_first);
            sarr_com_port.rx_obj.n_r_cnt += n_len_first;
        }
        if(n_len_second != 0)
        {
            console_recive_handler(sarr_com_port.rx_obj.p_buf, n_len_second);
            sarr_com_port.rx_obj.n_r_cnt = n_len_second;
        }
    }
}

/************************************************
函数名称 ： fputc
功    能 ： 串口重定向函数
参    数 ：
返 回 值 ：
作    者 ： LC
*************************************************/

int fputc(int ch, FILE *f)
{
    Uart0_Handle.pTxBuf = (uint8_t *)&ch;
    Uart0_Handle.tx_size = 1;
    UartSend(&Uart0_Handle);
    // 等待发送数据缓冲区标志置位
    return ch;
}

void Uart0Init(void)
{

    Uart0_Handle.irq_rx_process = _irq_rx_process;
    Uart0_Handle.rx_size = DEBUG_RX_BUF_LEN;
    Uart0_Handle.pRxBuf = garr_debug_rx_buf;

    if(Uart0_Handle.uart_cfg->en_dma_rx == true)
    {
        /* 重新设置DMA传输(初始化DMA还未映射rx_buf,上面映射完后重新初始化一下DMA)*/
        UartDMARxDisable(&Uart0_Handle);
        UartDMARxEnable(&Uart0_Handle);
    }
    Uart0BspInit();
    sarr_com_port.rx_obj.p_buf   = sarr_debug_rx_buff;
    sarr_com_port.rx_obj.n_mem_len = COMSERVICE_DEBUG_RX_SIZE;
    sarr_com_port.tx_obj.p_buf   = sarr_debug_tx_buff;
    sarr_com_port.tx_obj.n_mem_len = COMSERVICE_DEBUG_TX_SIZE;

    console_init();
}

void DebugProcess(void)
{
    static uint32_t debug_time = 0;
    if(TimeDiffFromNow(debug_time) > 500)
    {
        debug_time = TimGetSystime();
        _debug_comservice_rx();
        DebugInfoProcess();
    }
}

/**
*******************************************************************************
* Function:  comservice_log_info_printf
* @brief:   printf the log info.
*
* @details:
*
* Parameters:
* @param [in] e_type : the type of the log.
*             *string : the string of the log.
*             *_format : the format of the log.
*
* @return: None
*
*******************************************************************************
*/

void comservice_log_info_printf(en_comservice_log_type_t e_type, char *string, char *_format, ...)
{
    char c_pbuf[COMSERVICE_DEBUG_TX_SIZE] = {0};
    va_list ap;
    uint32_t n_len = 0;
    uint32_t i = 0;

    if(e_type == EN_COMSERVICE_LOG_TYPE_STRING)
    {

    }
    else
    {
        if((e_type == EN_COMSERVICE_LOG_TYPE_DEBUG) || (e_type == EN_COMSERVICE_LOG_TYPE_DEBUG1))
        {
            if(sc_log_filter == 1)
            {
                for(i = 0; i < LOG_DEBUG_ENABLE_NUM; i++)
                {
                    if(memcmp(&sarr_log_debug_en[i][0], string, strlen(string)) == 0)
                    {
                        goto debug;
                    }
                }
            }
            if((sc_log_lvl <= 1) || (sc_log_lvl >= 3))
            {
                return;
            }
debug:
            if(e_type == EN_COMSERVICE_LOG_TYPE_DEBUG)
            {
                n_len += sprintf(c_pbuf, "[DEBUG] ");
            }
        }
        else if(e_type == EN_COMSERVICE_LOG_TYPE_INFO)
        {
            if((sc_log_lvl <= 0) || (sc_log_lvl >= 3))
            {
                return;
            }
            if(sc_log_filter == 1)
            {
                for(i = 0; i < LOG_INFO_DISABLE_NUM; i++)
                {
                    if(memcmp(&sarr_log_info_disable[i][0], string, strlen(string)) == 0)
                    {
                        return;
                    }
                }
            }
            n_len += sprintf(c_pbuf, "[INFO ] ");
        }
        else if(e_type == EN_COMSERVICE_LOG_TYPE_ERROR)
        {
            if(sc_log_lvl >= 3)
            {
                return;
            }
            n_len += sprintf(c_pbuf, "[ERROR] ");
        }
        if(e_type != EN_COMSERVICE_LOG_TYPE_DEBUG1)
        {
            n_len += sprintf(&c_pbuf[n_len], "[%s] ", string);
        }
    }
    va_start(ap, _format);
    n_len += vsnprintf(&c_pbuf[n_len], COMSERVICE_DEBUG_TX_SIZE, _format, ap);
    va_end(ap);

    if((e_type != EN_COMSERVICE_LOG_TYPE_STRING) && (e_type != EN_COMSERVICE_LOG_TYPE_DEBUG) && (e_type != EN_COMSERVICE_LOG_TYPE_DEBUG1))
    {
        n_len += sprintf(&c_pbuf[n_len], "\r\n");
    }
    Uart0_Handle.pTxBuf = (uint8_t *)&c_pbuf;
    Uart0_Handle.tx_size = n_len;
    UartSend(&Uart0_Handle);
}
/**
*******************************************************************************
* Function: comservice_log_info_hex_printf
* @brief:  printf the log info with hex.
*
* @details:
*
* Parameters:
* @param [in] e_type : the type of the log.
*             *string : the string of the log.
*             *_format : the format of the log.
*
* @return: None
*
*******************************************************************************
*/

void comservice_log_info_hex_printf(en_comservice_log_type_t e_type, char *string, uint8_t *p_data, uint32_t n_len)
{
    uint32_t i = 0;

    if(e_type == EN_COMSERVICE_LOG_TYPE_DEBUG)
    {
        if(sc_log_filter == 1)
        {
            for(i = 0; i < LOG_DEBUG_ENABLE_NUM; i++)
            {
                if(memcmp(&sarr_log_debug_en[i][0], string, strlen(string)) == 0)
                {
                    goto debug;
                }
            }
        }
        if((sc_log_lvl <= 1) || (sc_log_lvl >= 3))
        {
            return;
        }
debug:
        comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, "[DEBUG]", "[%s] ", string);
    }
    else if(e_type == EN_COMSERVICE_LOG_TYPE_INFO)
    {
        if((sc_log_lvl <= 0) || (sc_log_lvl >= 3))
        {
            return;
        }
        if(sc_log_filter == 1)
        {
            for(i = 0; i < LOG_INFO_DISABLE_NUM; i++)
            {
                if(memcmp(&sarr_log_info_disable[i][0], string, strlen(string)) == 0)
                {
                    return;
                }
            }
        }
        comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, "[INFO ]", "[%s] ", string);
    }
    else if(e_type == EN_COMSERVICE_LOG_TYPE_ERROR)
    {
        comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, "[ERROR]", "[%s] ", string);
    }

    for(i = 0; i < n_len; i++)
    {
        if(i == (COMSERVICE_DEBUG_TX_SIZE - 10))
        {
            comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, " ", "...");
            break;
        }
        if(i % 16 == 0)
        {
            comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, " ", "\r\n");
        }
        comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, " ", "0x%02x ", (char)p_data[i]);
    }
    comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, " ", "\r\n");
}
/**
*******************************************************************************
* Function:  log_filter
* @brief:  log filter base on log level.
*
* @details:
*
* Parameters:
* @param [in]  argc: the number of the parma.
*              **argv: the point of the parma.
*
* @return: None
*
*******************************************************************************
*/

static int32_t log_filter(uint32_t argc, uint8_t **argv)
{

    if((argc == 2) && (atoi((char *)argv[1]) == 9))
    {
        sc_log_filter = 0;
        memset(sarr_log_info_disable, 0 , sizeof(uint8_t)*LOG_INFO_DISABLE_STRING_LEN * LOG_INFO_DISABLE_NUM);
        memset(sarr_log_debug_en, 0 , sizeof(uint8_t)*LOG_DEBUG_ENABLE_STRING_LEN * LOG_DEBUG_ENABLE_NUM);
        LOG_STRING(LOG_TAG, "clear fiter\r\n");
    }

    LOG_STRING(LOG_TAG, "----------------------------------\r\n");
    LOG_STRING(LOG_TAG, "log global filter\r\n");
    if(sc_log_lvl == 0)
    {
        LOG_STRING(LOG_TAG, "[level]: ERROR\r\n");
    }
    else if(sc_log_lvl == 1)
    {
        LOG_STRING(LOG_TAG, "[level]: INFO\r\n");
    }
    else if(sc_log_lvl == 2)
    {
        LOG_STRING(LOG_TAG, "[level]: DEBUG\r\n");
    }
    else
    {
        LOG_STRING(LOG_TAG, "[level]: NOPRINTF\r\n");
    }
    LOG_STRING(LOG_TAG, "[ INFO] DIS: %s %s %s %s %s \r\n", sarr_log_info_disable[0], sarr_log_info_disable[1], sarr_log_info_disable[2], sarr_log_info_disable[3], sarr_log_info_disable[4]);
    LOG_STRING(LOG_TAG, "[DEBUG]  EN: %s %s %s %s %s \r\n", sarr_log_debug_en[0], sarr_log_debug_en[1], sarr_log_debug_en[2], sarr_log_debug_en[3], sarr_log_debug_en[4]);
    LOG_STRING(LOG_TAG, "----------------------------------\r\n");

    return 0;
}
/**
*******************************************************************************
* Function: log_lvl
* @brief: dete the level of the log.
*
* @details:
*
* Parameters:
* @param [in] argc: the number of the parma.
*             **argv: the point of the parma.
*
* @return: None
*
*******************************************************************************
*/

static int32_t log_lvl(uint32_t argc, uint8_t **argv)
{

    if(argc != 2)
    {
        LOG_STRING(LOG_TAG, "parma error!\r\n");
        return 1;
    }

    sc_log_lvl = atoi((char *)argv[1]);

    if(sc_log_lvl > 3)
    {
        sc_log_lvl = 3;
    }

    log_filter(0, 0);

    return 0;
}

static int32_t log_debug_tag(uint32_t argc, uint8_t **argv)
{
    int32_t i = 0;

    if(argc != 2)
    {
        LOG_STRING(LOG_TAG, "parma error!\r\n");
        return 1;
    }

    for(i = 0; i < LOG_DEBUG_ENABLE_NUM; i++)
    {
        if(sarr_log_debug_en[i][0] == 0)
        {
            memcpy(sarr_log_debug_en[i], argv[1], sizeof(uint8_t)*LOG_DEBUG_ENABLE_STRING_LEN);
            sc_log_filter = 1;
            break;
        }
    }

    log_filter(0, 0);

    return 0;
}
/**
*******************************************************************************
* Function:  log_info_tag
* @brief:
*
* @details:
*
* Parameters:
 * @param [in] argc: the number of the parma.
*             **argv: the point of the parma.
*
* @return: None
*
*******************************************************************************
*/

static int32_t log_info_tag(uint32_t argc, uint8_t **argv)
{
    int32_t i = 0;

    if(argc != 2)
    {
        LOG_STRING(LOG_TAG, "parma error!\r\n");
        return 1;
    }

    for(i = 0; i < LOG_INFO_DISABLE_NUM; i++)
    {
        if(sarr_log_info_disable[i][0] == 0)
        {
            memcpy(sarr_log_info_disable[i], argv[1], sizeof(uint8_t)*LOG_INFO_DISABLE_STRING_LEN);
            sc_log_filter = 1;
            break;
        }
    }

    log_filter(0, 0);

    return 0;
}

/**
*******************************************************************************
* Function:  float_cov_uint16
* @brief: convert float to uint16.
*
* @details:  value: the value of the float.
*
* Parameters:
* @param [in] value: the value of the float.
*
* @return: None
*
*******************************************************************************
*/

uint16_t float_cov_uint16(float value)
{

    const union Fp32 f32infty = { 255U << 23 };
    const union Fp32 f16infty = { 31U << 23 };
    const union Fp32 magic = { 15U << 23 };
    const uint32_t sign_mask = 0x80000000U;
    const uint32_t round_mask = ~0xFFFU;

    union Fp32 in;
    uint16_t out;

    in.f = value;

    uint32_t sign = in.u & sign_mask;
    in.u ^= sign;

    if (in.u >= f32infty.u) /* Inf or NaN (all exponent bits set) */
    {
        /* NaN->sNaN and Inf->Inf */
        out = (in.u > f32infty.u) ? 0x7FFFU : 0x7C00U;
    }
    else /* (De)normalized number or zero */
    {
        in.u &= round_mask;
        in.f *= magic.f;
        in.u -= round_mask;
        if (in.u > f16infty.u)
        {
            in.u = f16infty.u; /* Clamp to signed infinity if overflowed */
        }

        out = (uint16_t)(in.u >> 13); /* Take the bits! */
    }

    out = (uint16_t)(out | (sign >> 16));

    return out;
}
/**
*******************************************************************************
* Function:  uint16_cov_float
* @brief: convert uint16 to float.
*
* @details:
*
* Parameters:
* @param [in]  value: the value of the uint16.
*
* @return: None
*
*******************************************************************************
*/

float uint16_cov_float(uint16_t value)
{
    const union Fp32 magic = { (254U - 15U) << 23 };
    const union Fp32 was_infnan = { (127U + 16U) << 23 };
    union Fp32 out;

    out.u = (value & 0x7FFFU) << 13;   /* exponent/mantissa bits */
    out.f *= magic.f;                  /* exponent adjust */
    if (out.f >= was_infnan.f)         /* make sure Inf/NaN survive */
    {
        out.u |= 255U << 23;
    }
    out.u |= (value & 0x8000U) << 16;  /* sign bit */

    return out.f;
}

/**
*******************************************************************************
* Function:  _UartDebReadParse
* @brief: read the parse data
*
* @details: Read from the RTE data points, and store into the array that will be send.
*
* Parameters:
* @param [in] ptFrame, data_len, send_buf.
*
* @return: None
*
*******************************************************************************
*/
static bool _UartDebReadParse(ProtocalRequestPacket_t *ptFrame, uint16_t data_len, uint8_t *send_buf)
{
    uint8_t buffer[1024], index, offset, w_buffer;
    uint16_t start_addr, reg_num;
    uint16_t i, reg_addr, uart_point_num;
    bool  write_status = true;

    const UartDataPoint_t   *uart_point;

    uart_point = gUartDebugDataPoint;
    uart_point_num = gUartDebDataLen;

    // get the start register addr and data length.
    start_addr = UART_DEBUG_REG_BASE_ADDR;
    reg_num = data_len;
    memset(buffer, 0, 1024);

    for(i = 0; i < reg_num; i++)
    {
        reg_addr = start_addr + i;
        for(index = 0; index < uart_point_num; index++)
        {
            if((NULL != uart_point[index].pdata) && ((EN_ATTR_WO == uart_point[index].attr_rw) || (EN_ATTR_RW == uart_point[index].attr_rw)))
            {
                if((reg_addr >= uart_point[index].base_addr) && (reg_addr < (uart_point[index].base_addr + uart_point[index].num)))
                {
                    offset = reg_addr - uart_point[index].base_addr;

                    // data type convertion
                    if( (EN_DATA_U8 == uart_point[index].data_type) || (EN_DATA_S8 == uart_point[index].data_type) )
                    {
                        w_buffer = *((uint8_t *)uart_point[index].pdata + offset);
                    }
                    else if( (EN_DATA_U32 == uart_point[index].data_type) || (EN_DATA_S32 == uart_point[index].data_type) )
                    {
                        // if MSB, it could be add convert lsb to msb code in here
                        w_buffer = *((uint8_t *)uart_point[index].pdata + offset);
                    }
                    else
                    {
                        w_buffer = *((uint8_t *)uart_point[index].pdata + offset);
                    }
                    memmove(buffer + i, &w_buffer, 1);
                    break;
                }
            }
            else
            {
                write_status = false;
                break;
            }
        }
        if((index == uart_point_num) || (write_status == false))
        {
            write_status = false;
            break;
        }
    }
    memcpy(send_buf, buffer, reg_num);

    return write_status;
}

/**
*******************************************************************************
* Function: _DebugInfoSend
* @brief: Bat info send, used for SOX test.
*
* @details:
*
* Parameters:
* @param [in]
*
* @return: None
*
*******************************************************************************
*/
static void _DebugInfoPackaging(uint8_t* p_data_buff)
{
    uint8_t i = 0;
    uint16_t modbus_read_reg_size_lsb = 0;
    uint8_t  *p_curr_addr  = p_data_buff + DEBUG_PACKET_LEN; // len of packet + buff
    uint16_t  modbus_read_reg_size = (uint16_t)(UART_DEBUG_REG_END_ADDR - UART_DEBUG_REG_BASE_ADDR + 1);

    // bms id.
    memcpy(p_curr_addr, &i, 1);
    p_curr_addr += 1;

    // len of data, MSB to LSB firstly.
    modbus_read_reg_size_lsb = UartCtrlMSB2LSBU16(&modbus_read_reg_size);
    memcpy(p_curr_addr, &modbus_read_reg_size_lsb, 2);
    p_curr_addr += 2;

    /* read the parse data. */
    _UartDebReadParse(&gstDebReadPacketData, modbus_read_reg_size, p_curr_addr);
    p_curr_addr += modbus_read_reg_size;
}

/**
*******************************************************************************
* Function:  DebugInfoProcess
* @brief: Bat info filling and Periodic sending.
*
* @details:
*
* Parameters:
* @param [in]
*
* @return: None
*
*******************************************************************************
*/
void DebugInfoProcess(void)
{
    // data field filling.
    _DebugInfoPackaging(sarr_debug_tx_buff);

    // packet data filling and tx buffer send.
    Uart0DebugSendPolling(sarr_debug_tx_buff);
}

