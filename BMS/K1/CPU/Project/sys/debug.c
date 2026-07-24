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
#define LOG_TAG     "debug"
#define DEBUGUSART_IRQHandler   USART0_IRQHandler

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
static uint8_t sc_log_lvl = 2;  //打印等级
static uint8_t sc_log_filter = 0;
static uint8_t sarr_log_debug_en[LOG_DEBUG_ENABLE_NUM][LOG_DEBUG_ENABLE_STRING_LEN] = {0};
static uint8_t sarr_log_info_disable[LOG_INFO_DISABLE_STRING_LEN][LOG_INFO_DISABLE_NUM] = {0};

void comservice_log_info_printf(en_comservice_log_type_t e_type, char *string, char *format,...);

static int32_t _debug_data_cpy(uint8_t *p_data, uint32_t n_len)
{
    int32_t n_ret = 0;
    uint32_t *p_w_cnt =NULL;
    uint32_t n_r_cnt =NULL;
    
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
            if(p_data != NULL)
            {
                memcpy(&sarr_com_port.rx_obj.p_buf[*p_w_cnt], p_data, n_len);
                *p_w_cnt += n_len;
            }
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
        else if(((*p_w_cnt + n_len) > sarr_com_port.rx_obj.n_mem_len) &&\
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
    if(Uart0_Debug_Handle.pRxBuf == NULL)
    {
        return;
    }
    
    if(Uart0_Debug_Handle.uart_cfg->en_dma_rx == true)
    {
        /* 处理DMA接收到的数据 */
        // 计算实际接收的数据长度
        gn_debug_recv_length = DEBUG_RX_BUF_LEN - dma_transfer_number_get(Uart0_Debug_Handle.uart_cfg->dma_rx_peri,Uart0_Debug_Handle.uart_cfg->dma_rx_channel);			 
        _debug_data_cpy(garr_debug_rx_buf, gn_debug_recv_length);
//    garr_debug_rx_buf[gn_debug_recv_length] = '\0';
//    printf("debug recv len:%d\t", gn_debug_recv_length);
//    printf("g_recv_buff:%s\r\n",garr_debug_rx_buf);		// 打印接收的数据
    
        /* 重新设置DMA传输 */		 
        UartDMARxDisable(&Uart0_Debug_Handle);
        UartDMARxEnable(&Uart0_Debug_Handle);
    }
    else
    {
        _debug_data_cpy(Uart0_Debug_Handle.pRxBuf, 1);
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
    Uart0_Debug_Handle.pTxBuf = (uint8_t *)&ch;
    Uart0_Debug_Handle.tx_size = 1;
    UartSend(&Uart0_Debug_Handle);
     // 等待发送数据缓冲区标志置位
     return ch;
}

void DebugInit(void)
{
    Uart0DebugBspInit();
    
    Uart0_Debug_Handle.irq_rx_process = _irq_rx_process;
    Uart0_Debug_Handle.rx_size = DEBUG_RX_BUF_LEN;
    Uart0_Debug_Handle.pRxBuf = garr_debug_rx_buf; 
    
    if(Uart0_Debug_Handle.uart_cfg->en_dma_rx == true)
    {    
        /* 重新设置DMA传输(初始化DMA还未映射rx_buf,上面映射完后重新初始化一下DMA)*/		 
        UartDMARxDisable(&Uart0_Debug_Handle);
        UartDMARxEnable(&Uart0_Debug_Handle);
    }
    
    sarr_com_port.rx_obj.p_buf   = sarr_debug_rx_buff;
    sarr_com_port.rx_obj.n_mem_len = COMSERVICE_DEBUG_RX_SIZE;
    sarr_com_port.tx_obj.p_buf   = sarr_debug_tx_buff;
    sarr_com_port.tx_obj.n_mem_len = COMSERVICE_DEBUG_TX_SIZE;
    
    console_init();
}

void DebugProcess(void)
{
    _debug_comservice_rx();
}

void DEBUGUSART_IRQHandler(void)
{
    UartIrqProcess(&Uart0_Debug_Handle);                                                                                                                           // 接收完成 	
}

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
    
    Uart0_Debug_Handle.pTxBuf = (uint8_t *)&c_pbuf;
    Uart0_Debug_Handle.tx_size = n_len;
    UartSend(&Uart0_Debug_Handle);   
}

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
        comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, "[DEBUG]","[%s] ", string);   
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
        comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, "[INFO ]","[%s] ", string);  
    }
    else if(e_type == EN_COMSERVICE_LOG_TYPE_ERROR)
    {
        comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, "[ERROR]","[%s] ", string);  
    }    
    
    for(i = 0; i < n_len; i++)
    {
        if(i == (COMSERVICE_DEBUG_TX_SIZE - 10))
        {
            comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, " ", "...");
            break;
        }
        if(i%16 == 0)
        {
            comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, " ", "\r\n");
        }
        comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, " ", "0x%02x ", (char)p_data[i]);
    }
    comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, " ", "\r\n");
}

static int32_t log_filter(uint32_t argc, uint8_t **argv) 
{
    
    if((argc == 2)&&(atoi((char *)argv[1]) == 9))
    {
        sc_log_filter = 0;
        memset(sarr_log_info_disable, 0 , sizeof(uint8_t)*LOG_INFO_DISABLE_STRING_LEN*LOG_INFO_DISABLE_NUM);
        memset(sarr_log_debug_en, 0 , sizeof(uint8_t)*LOG_DEBUG_ENABLE_STRING_LEN*LOG_DEBUG_ENABLE_NUM);
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
    LOG_STRING(LOG_TAG, "[ INFO] DIS: %s %s %s %s %s \r\n", sarr_log_info_disable[0],sarr_log_info_disable[1],sarr_log_info_disable[2],sarr_log_info_disable[3],sarr_log_info_disable[4]);
    LOG_STRING(LOG_TAG, "[DEBUG]  EN: %s %s %s %s %s \r\n", sarr_log_debug_en[0],sarr_log_debug_en[1],sarr_log_debug_en[2],sarr_log_debug_en[3],sarr_log_debug_en[4]);
    LOG_STRING(LOG_TAG, "----------------------------------\r\n");
    
    return 0;
}

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

MSH_CMD_EXPORT(log_filter, 9 clear all filter);
MSH_CMD_EXPORT(log_debug_tag, string);
MSH_CMD_EXPORT(log_info_tag, string);
MSH_CMD_EXPORT(log_lvl, 0 error 1 info 2 debug 3 noprintf);



union Fp32
{
    uint32_t u;
    float f;
};

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