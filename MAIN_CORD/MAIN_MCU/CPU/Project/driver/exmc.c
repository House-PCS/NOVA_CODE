/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        exmc.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-21
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-21    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "exmc.h"
#include "systick.h"


/**
  * @brief  EXMC GPIO init.
  * @param  Exmc_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
static void _ExmcGpioInit(Exmc_HandleType *pHandle)
{
    if(pHandle->exmc_config->gpio_c_pinx != 0)
    {
        gpio_af_set(GPIOC, GPIO_AF_12, pHandle->exmc_config->gpio_c_pinx);
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pHandle->exmc_config->gpio_c_pinx);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->exmc_config->gpio_c_pinx);
    }
    if(pHandle->exmc_config->gpio_d_pinx != 0)
    {
        gpio_af_set(GPIOD, GPIO_AF_12, pHandle->exmc_config->gpio_d_pinx);
        gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pHandle->exmc_config->gpio_d_pinx);
        gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->exmc_config->gpio_d_pinx);
    }
    if(pHandle->exmc_config->gpio_e_pinx != 0)
    {
        gpio_af_set(GPIOE, GPIO_AF_12, pHandle->exmc_config->gpio_e_pinx);
        gpio_mode_set(GPIOE, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pHandle->exmc_config->gpio_e_pinx);
        gpio_output_options_set(GPIOE, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->exmc_config->gpio_e_pinx);
    }
    if(pHandle->exmc_config->gpio_f_pinx != 0)
    {
        gpio_af_set(GPIOF, GPIO_AF_12, pHandle->exmc_config->gpio_f_pinx);
        gpio_mode_set(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pHandle->exmc_config->gpio_f_pinx);
        gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->exmc_config->gpio_f_pinx);
    }
    if(pHandle->exmc_config->gpio_g_pinx != 0)
    {
        gpio_af_set(GPIOG, GPIO_AF_12, pHandle->exmc_config->gpio_g_pinx);
        gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pHandle->exmc_config->gpio_g_pinx);
        gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->exmc_config->gpio_g_pinx);
    }
    if(pHandle->exmc_config->gpio_h_pinx != 0)
    {
        gpio_af_set(GPIOH, GPIO_AF_12, pHandle->exmc_config->gpio_h_pinx);
        gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pHandle->exmc_config->gpio_h_pinx);
        gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->exmc_config->gpio_h_pinx);
    }
    if(pHandle->exmc_config->gpio_i_pinx != 0)
    {
        gpio_af_set(GPIOI, GPIO_AF_12, pHandle->exmc_config->gpio_i_pinx);
        gpio_mode_set(GPIOI, GPIO_MODE_AF, GPIO_PUPD_PULLUP, pHandle->exmc_config->gpio_i_pinx);
        gpio_output_options_set(GPIOI, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pHandle->exmc_config->gpio_i_pinx);
    }

}



/**
  * @brief  SDRAM init.
  * @param  Exmc_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void ExmcSdramInit(Exmc_HandleType *pHandle)
{
    exmc_sdram_parameter_struct             sdram_init_struct;
    exmc_sdram_timing_parameter_struct      sdram_timing_init_struct;
    exmc_sdram_command_parameter_struct     sdram_command_init_struct;

    uint32_t command_content = 0, bank_select;
    uint32_t timeout = SDRAM_TIMEOUT;

    /* specify which SDRAM to read and write */
    if(EXMC_SDRAM_DEVICE0 == pHandle->exmc_config->sdram_device_id)
    {
        bank_select = EXMC_SDRAM_DEVICE0_SELECT;
    }
    else
    {
        bank_select = EXMC_SDRAM_DEVICE1_SELECT;
    }

    /* EXMC SDRAM device initialization sequence --------------------------------*/
    /* Step 1 : configure SDRAM timing registers --------------------------------*/
    /* LMRD: 2 clock cycles */
    sdram_timing_init_struct.load_mode_register_delay = 2;
    /* XSRD: min = 67ns */
    sdram_timing_init_struct.exit_selfrefresh_delay = 9;
    /* RASD: min=42ns , max=120k (ns) */
    sdram_timing_init_struct.row_address_select_delay = 5;
    /* ARFD: min=60ns */
    sdram_timing_init_struct.auto_refresh_delay = 8;
    /* WRD:  min=1 Clock cycles +6ns */
    sdram_timing_init_struct.write_recovery_delay = 3;
    /* RPD:  min=18ns */
    sdram_timing_init_struct.row_precharge_delay = 3;
    /* RCD:  min=18ns */
    sdram_timing_init_struct.row_to_column_delay = 3;

    /* step 2 : configure SDRAM control registers ---------------------------------*/
    /* EXMC SDRAM bank initialization */
#if 0
    exmc_sdram_init(&pHandle->exmc_config->sdram_para);
#else
    sdram_init_struct.sdram_device = pHandle->exmc_config->sdram_device_id;
    sdram_init_struct.column_address_width = EXMC_SDRAM_COW_ADDRESS_9;
    sdram_init_struct.row_address_width = EXMC_SDRAM_ROW_ADDRESS_13;
    sdram_init_struct.data_width = EXMC_SDRAM_DATABUS_WIDTH_16B;
    sdram_init_struct.internal_bank_number = EXMC_SDRAM_4_INTER_BANK;
    sdram_init_struct.cas_latency = EXMC_CAS_LATENCY_3_SDCLK;
    sdram_init_struct.write_protection = DISABLE;
    sdram_init_struct.sdclock_config = EXMC_SDCLK_PERIODS_3_HCLK;
    sdram_init_struct.burst_read_switch = ENABLE;
    sdram_init_struct.pipeline_read_delay = EXMC_PIPELINE_DELAY_2_HCLK;
    sdram_init_struct.timing  = &sdram_timing_init_struct;
    /* EXMC SDRAM bank initialization */
    exmc_sdram_init(&sdram_init_struct);
#endif

    /* step 3 : configure CKE high command---------------------------------------*/
    sdram_command_init_struct.command = EXMC_SDRAM_CLOCK_ENABLE;
    sdram_command_init_struct.bank_select = bank_select;
    sdram_command_init_struct.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_2_SDCLK;
    sdram_command_init_struct.mode_register_content = 0;
    /* wait until the SDRAM controller is ready */
    while((exmc_flag_get(pHandle->exmc_config->sdram_device_id, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0))
    {
        timeout--;
    }
    /* send the command */
    exmc_sdram_command_config(&sdram_command_init_struct);

    /* step 4 : insert 10ms delay----------------------------------------------*/
//    delay_1ms(10);
    soft_delayms(10);

    /* step 5 : configure precharge all command----------------------------------*/
    sdram_command_init_struct.command = EXMC_SDRAM_PRECHARGE_ALL;
    sdram_command_init_struct.bank_select = bank_select;
    sdram_command_init_struct.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_2_SDCLK;
    sdram_command_init_struct.mode_register_content = 0;
    /* wait until the SDRAM controller is ready */
    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(pHandle->exmc_config->sdram_device_id, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0))
    {
        timeout--;
    }
    /* send the command */
    exmc_sdram_command_config(&sdram_command_init_struct);

    /* step 6 : configure Auto-Refresh command-----------------------------------*/
    sdram_command_init_struct.command = EXMC_SDRAM_AUTO_REFRESH;
    sdram_command_init_struct.bank_select = bank_select;
    sdram_command_init_struct.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_9_SDCLK;
    sdram_command_init_struct.mode_register_content = 0;
    /* wait until the SDRAM controller is ready */
    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(pHandle->exmc_config->sdram_device_id, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0))
    {
        timeout--;
    }
    /* send the command */
    exmc_sdram_command_config(&sdram_command_init_struct);

    /* step 7 : configure load mode register command-----------------------------*/
    /* program mode register */
    command_content = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1        |
                      SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
                      SDRAM_MODEREG_CAS_LATENCY_3           |
                      SDRAM_MODEREG_OPERATING_MODE_STANDARD |
                      SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

    sdram_command_init_struct.command = EXMC_SDRAM_LOAD_MODE_REGISTER;
    sdram_command_init_struct.bank_select = bank_select;
    sdram_command_init_struct.auto_refresh_number = EXMC_SDRAM_AUTO_REFLESH_2_SDCLK;
    sdram_command_init_struct.mode_register_content = command_content;
    /* wait until the SDRAM controller is ready */
    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(pHandle->exmc_config->sdram_device_id, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0))
    {
        timeout--;
    }
    /* send the command */
    exmc_sdram_command_config(&sdram_command_init_struct);

    /* step 8 : set the auto-refresh rate counter--------------------------------*/
    /* 64ms, 8192-cycle refresh, 64ms/8192=7.81us */
    /* SDCLK_Freq = SYS_Freq/2 */
    /* (7.81 us * SDCLK_Freq) - 20 */
    exmc_sdram_refresh_count_set(761);//SYSCLK = 200M
//    exmc_sdram_refresh_count_set(917);//SYSCLK = 240M
    /* wait until the SDRAM controller is ready */
    timeout = SDRAM_TIMEOUT;
    while((exmc_flag_get(pHandle->exmc_config->sdram_device_id, EXMC_SDRAM_FLAG_NREADY) != RESET) && (timeout > 0))
    {
        timeout--;
    }
}


/**
  * @brief  SRAM init.
  * @param  Exmc_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void ExmcSramInit(Exmc_HandleType *pHandle)
{
    exmc_norsram_parameter_struct nor_init_struct;
    exmc_norsram_timing_parameter_struct nor_timing_init_struct;

    /* configure timing parameter */
    nor_timing_init_struct.asyn_access_mode = EXMC_ACCESS_MODE_A;
    nor_timing_init_struct.syn_data_latency = EXMC_DATALAT_2_CLK;
    nor_timing_init_struct.syn_clk_division = EXMC_SYN_CLOCK_RATIO_2_CLK;
    nor_timing_init_struct.bus_latency = 0;
    nor_timing_init_struct.asyn_data_setuptime = 7;
    nor_timing_init_struct.asyn_address_holdtime = 2;
    nor_timing_init_struct.asyn_address_setuptime = 5;

    /* configure EXMC bus parameters */
    nor_init_struct.norsram_region = pHandle->exmc_config->sram_region_id;
    nor_init_struct.write_mode = EXMC_ASYN_WRITE;
    nor_init_struct.extended_mode = DISABLE;
    nor_init_struct.asyn_wait = DISABLE;
    nor_init_struct.nwait_signal = DISABLE;
    nor_init_struct.memory_write = ENABLE;
    nor_init_struct.nwait_config = EXMC_NWAIT_CONFIG_BEFORE;
    nor_init_struct.wrap_burst_mode = DISABLE;
    nor_init_struct.nwait_polarity = EXMC_NWAIT_POLARITY_LOW;
    nor_init_struct.burst_mode = DISABLE;
    nor_init_struct.databus_width = EXMC_NOR_DATABUS_WIDTH_16B;
    nor_init_struct.memory_type = EXMC_MEMORY_TYPE_SRAM;
    nor_init_struct.address_data_mux = DISABLE;
    nor_init_struct.read_write_timing = &nor_timing_init_struct;
    nor_init_struct.write_timing = &nor_timing_init_struct;
    exmc_norsram_init(&nor_init_struct);

    /* enable the EXMC bankx NORSRAM */
    exmc_norsram_enable(pHandle->exmc_config->sram_region_id);
}


/**
  * @brief  EXMC init.
  * @param  Exmc_HandleType: 串口需要用户配置的参数
  * @return NULL
  * @note   none
  */
void ExmcInit(Exmc_HandleType *pHandle, EN_EXMC_MODE_MASK mask)
{
    static bool initianliazed = false;

    if(initianliazed == false)
    {
        rcu_periph_clock_enable(RCU_EXMC);

        _ExmcGpioInit(pHandle);

        if((mask & EN_EXMC_MODE_MASK_SDRAM) != 0)
        {
            ExmcSdramInit(pHandle);
        }
        if((mask & EN_EXMC_MODE_MASK_SRAM) != 0)
        {
            ExmcSramInit(pHandle);
        }

//        initianliazed = true;
    }

}








