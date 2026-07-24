
#include "enet.h"


//#include "main.h"
#define USE_ENET_INTERRUPT

/* checksum options */
#define CHECKSUM_BY_HARDWARE                             /* computing and verifying the IP, UDP, TCP and ICMP checksums by hardware */


///* MII and RMII mode selection */
//#define RMII_MODE  // user have to provide the 50 MHz clock by soldering a 50 MHz oscillator
////#define MII_MODE

///* clock the PHY from external 25MHz crystal (only for MII mode) */
//#ifdef  MII_MODE
//#define PHY_CLOCK_MCO
//#endif

Enet_HandleType *enet_handle;

const uint8_t gd32_str[] = {"\r\n ############ Welcome GigaDevice ############\r\n"};
static __IO uint32_t enet_init_status = 0;
//static void enet_gpio_config(void);
static void enet_mac_dma_config(void);
#ifdef USE_ENET_INTERRUPT
static void nvic_configuration(void);
#endif /* USE_ENET_INTERRUPT */

static void enet_gpio_config(Enet_HandleType *pHandle)
{
//    rcu_periph_clock_enable(RCU_GPIOA);
//    rcu_periph_clock_enable(RCU_GPIOB);
//    rcu_periph_clock_enable(RCU_GPIOC);
//    rcu_periph_clock_enable(RCU_GPIOD);
//    rcu_periph_clock_enable(RCU_GPIOG);
//    rcu_periph_clock_enable(RCU_GPIOH);
//    rcu_periph_clock_enable(RCU_GPIOI);

//    rcu_periph_clock_enable(RCU_GPIOF);
//    gpio_af_set(GPIOA, GPIO_AF_0, GPIO_PIN_8);
//    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
//    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_8);

    /* enable SYSCFG clock */
    
    rcu_periph_clock_enable(RCU_SYSCFG);
    
    if(pHandle->rst_gpiox != 0)
    {
        gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_2);
        gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);
        gpio_bit_set(GPIOF, GPIO_PIN_2);
    }

    if(pHandle->smi_sim == true)
    {
        gpio_mode_set(pHandle->mdc_gpiox, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pHandle->mdc_gpio_pinx);
        gpio_output_options_set(pHandle->mdc_gpiox, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, pHandle->mdc_gpio_pinx);
        gpio_mode_set(pHandle->mdio_gpiox, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pHandle->mdio_gpio_pinx);
        gpio_output_options_set(pHandle->mdio_gpiox, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, pHandle->mdio_gpio_pinx);
        
    }
    else
    {
        /* PC1: ETH_MDC */
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);
        gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_1);

        /* PA2: ETH_MDIO */
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);
        gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_2);
    }

    if(pHandle->com_type == EN_ENET_COM_MII)
    {
        #ifdef PHY_CLOCK_MCO
            /* output HXTAL clock (25MHz) on CKOUT0 pin(PA8) to clock the PHY */
            rcu_ckout0_config(RCU_CKOUT0SRC_HXTAL, RCU_CKOUT0_DIV1);
        #endif /* PHY_CLOCK_MCO */

        syscfg_enet_phy_interface_config(SYSCFG_ENET_PHY_MII);
        
        /* PA1: ETH_MII_RX_CLK */
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);

        /* PA7: ETH_MII_RX_DV */
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_7);

        gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_1);
        gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_7);

        /* PG11: ETH_MII_TX_EN */
        gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
        gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_11);

        /* PG13: ETH_MII_TXD0 */
        gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
        gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_13);

        /* PG14: ETH_MII_TXD1 */
        gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_14);
        gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_14);

        gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_11);
        gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_13);
        gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_14);

        /* PC2: ETH_MII_TXD2 */
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);

        /* PC3: ETH_MII_TX_CLK */
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_3);

        /* PC4: ETH_MII_RXD0 */
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_4);

        /* PC5: ETH_MII_RXD1 */
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_5);

        gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_2);
        gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_3);
        gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_4);
        gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_5);

        /* PH2: ETH_MII_CRS */
        gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
        gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);

        /* PH3: ETH_MII_COL */
        gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_3);
        gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_3);

        /* PH6: ETH_MII_RXD2 */
        gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_6);
        gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_6);

        /* PH7: ETH_MII_RXD3 */
        gpio_mode_set(GPIOH, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
        gpio_output_options_set(GPIOH, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_7);

        gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_2);
        gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_3);
        gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_6);
        gpio_af_set(GPIOH, GPIO_AF_11, GPIO_PIN_7);

        /* PI8: ETH_INT */
        gpio_mode_set(GPIOI, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_8);

        /* PI10: ETH_MII_RX_ER */
        gpio_mode_set(GPIOI, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
        gpio_output_options_set(GPIOI, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_10);

        gpio_af_set(GPIOI, GPIO_AF_11, GPIO_PIN_10);

        /* PB8: ETH_MII_TXD3 */
        gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_8);
        gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_8);

        gpio_af_set(GPIOB, GPIO_AF_11, GPIO_PIN_8);
    }
    else if(pHandle->com_type == EN_ENET_COM_RMII)
    {
            /* choose DIV2 to get 50MHz from 200MHz on CKOUT0 pin (PA8) to clock the PHY */
//    rcu_ckout0_config(RCU_CKOUT0SRC_PLLP, RCU_CKOUT0_DIV4);
        syscfg_enet_phy_interface_config(SYSCFG_ENET_PHY_RMII);

        
        /* PA1: ETH_RMII_REF_CLK */
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_1);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);


        /* PA7: ETH_RMII_CRS_DV */
        gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_7);
        gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_7);

        gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_1);
        gpio_af_set(GPIOA, GPIO_AF_11, GPIO_PIN_7);

        /* PG11: ETH_RMII_TX_EN */
        gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_11);
        gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_11);

        /* PG13: ETH_RMII_TXD0 */
        gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_13);
        gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_13);

        /* PG14: ETH_RMII_TXD1 */
        gpio_mode_set(GPIOG, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_14);
        gpio_output_options_set(GPIOG, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_14);

        gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_11);
        gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_13);
        gpio_af_set(GPIOG, GPIO_AF_11, GPIO_PIN_14);


        /* PC4: ETH_RMII_RXD0 */
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_4);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_4);

        /* PC5: ETH_RMII_RXD1 */
        gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_5);
        gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_5);

        gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_4);
        gpio_af_set(GPIOC, GPIO_AF_11, GPIO_PIN_5);

    }
        
    
#ifdef MII_MODE



#elif defined RMII_MODE

#endif /* MII_MODE */

#ifdef MII_MODE
{
    
}
#elif defined RMII_MODE

#endif /* MII_MODE */

}

///////////////////
static void enet_bsp_delay(uint32_t ncount)
{
    __IO uint32_t delay_time = 0U;

    for(delay_time = ncount; delay_time != 0U; delay_time--) {
    }
}
static void enet_bsp_default_init(void)
{
    uint32_t reg_value = 0U;

    /* MAC */
    /* configure ENET_MAC_CFG register */
    reg_value = ENET_MAC_CFG;
    reg_value &= MAC_CFG_MASK;
    reg_value |= ENET_WATCHDOG_ENABLE | ENET_JABBER_ENABLE | ENET_INTERFRAMEGAP_96BIT \
                 | ENET_SPEEDMODE_10M | ENET_MODE_HALFDUPLEX | ENET_LOOPBACKMODE_DISABLE \
                 | ENET_CARRIERSENSE_ENABLE | ENET_RECEIVEOWN_ENABLE \
                 | ENET_RETRYTRANSMISSION_ENABLE | ENET_BACKOFFLIMIT_10 \
                 | ENET_DEFERRALCHECK_DISABLE \
                 | ENET_TYPEFRAME_CRC_DROP_DISABLE \
                 | ENET_AUTO_PADCRC_DROP_DISABLE \
                 | ENET_CHECKSUMOFFLOAD_DISABLE;
    ENET_MAC_CFG = reg_value;

    /* configure ENET_MAC_FRMF register */
    ENET_MAC_FRMF = ENET_SRC_FILTER_DISABLE | ENET_DEST_FILTER_INVERSE_DISABLE \
                    | ENET_MULTICAST_FILTER_PERFECT | ENET_UNICAST_FILTER_PERFECT \
                    | ENET_PCFRM_PREVENT_ALL | ENET_BROADCASTFRAMES_ENABLE \
                    | ENET_PROMISCUOUS_DISABLE | ENET_RX_FILTER_ENABLE;

    /* configure ENET_MAC_HLH, ENET_MAC_HLL register */
    ENET_MAC_HLH = 0x0U;

    ENET_MAC_HLL = 0x0U;

    /* configure ENET_MAC_FCTL, ENET_MAC_FCTH register */
    reg_value = ENET_MAC_FCTL;
    reg_value &= MAC_FCTL_MASK;
    reg_value |= MAC_FCTL_PTM(0) | ENET_ZERO_QUANTA_PAUSE_DISABLE \
                 | ENET_PAUSETIME_MINUS4 | ENET_UNIQUE_PAUSEDETECT \
                 | ENET_RX_FLOWCONTROL_DISABLE | ENET_TX_FLOWCONTROL_DISABLE;
    ENET_MAC_FCTL = reg_value;

    /* configure ENET_MAC_VLT register */
    ENET_MAC_VLT = ENET_VLANTAGCOMPARISON_16BIT | MAC_VLT_VLTI(0);

    /* DMA */
    /* configure ENET_DMA_CTL register */
    reg_value = ENET_DMA_CTL;
    reg_value &= DMA_CTL_MASK;
    reg_value |= ENET_TCPIP_CKSUMERROR_DROP | ENET_RX_MODE_STOREFORWARD \
                 | ENET_FLUSH_RXFRAME_ENABLE | ENET_TX_MODE_STOREFORWARD \
                 | ENET_TX_THRESHOLD_64BYTES | ENET_RX_THRESHOLD_64BYTES \
                 | ENET_SECONDFRAME_OPT_DISABLE;
    ENET_DMA_CTL = reg_value;

    /* configure ENET_DMA_BCTL register */
    reg_value = ENET_DMA_BCTL;
    reg_value &= DMA_BCTL_MASK;
    reg_value = ENET_ADDRESS_ALIGN_ENABLE | ENET_ARBITRATION_RXTX_2_1 \
                | ENET_RXDP_32BEAT | ENET_PGBL_32BEAT | ENET_RXTX_DIFFERENT_PGBL \
                | ENET_FIXED_BURST_ENABLE | ENET_MIXED_BURST_DISABLE \
                | ENET_NORMAL_DESCRIPTOR;
    ENET_DMA_BCTL = reg_value;
}

#define CLK_DELAY   30
ErrStatus enet_phy_bsp_write_read(enet_phydirection_enum direction, uint16_t phy_address, uint16_t phy_reg, uint16_t *pvalue)
{
//    uint32_t reg, phy_flag;
//    uint32_t timeout = 0U;
//    ErrStatus enet_state = ERROR;

    uint16_t data = 0;
    uint16_t command = 0;
    uint8_t i;
    
    // ????PRE????
    for(i = 0; i < 32; i++) {
        gpio_bit_reset(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
        for(uint8_t t = 0; t < CLK_DELAY; t++)
        {
            __NOP();
        }
        gpio_bit_set(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
        for(uint8_t t = 0; t < CLK_DELAY; t++)
        {
            __NOP();
        }
    }
    
    // ????ST????
    
    gpio_bit_reset(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
    gpio_bit_reset(enet_handle->mdio_gpiox, enet_handle->mdio_gpio_pinx);
    for(uint8_t t = 0; t < CLK_DELAY; t++)
        {
            __NOP();
        }
    gpio_bit_set(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
       for(uint8_t t = 0; t < CLK_DELAY; t++)
        {
            __NOP();
        } 
        gpio_bit_reset(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
    gpio_bit_set(enet_handle->mdio_gpiox, enet_handle->mdio_gpio_pinx);
   for(uint8_t t = 0; t < CLK_DELAY; t++)
        {
            __NOP();
        }
    gpio_bit_set(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
       for(uint8_t t = 0; t < CLK_DELAY; t++)
        {
            __NOP();
        } 
    // ????OP????
    if(ENET_PHY_READ == direction) {
        command = ((phy_address & 0x1F) << 7) | ((phy_reg & 0x1F) << 2) | 0x2000 | 0x2;
    }else
    {
        command = ((phy_address & 0x1F) << 7) | ((phy_reg & 0x1F) << 2) | 0x1000 | 0x2;
    }
    for(i = 0; i < 14; i++) {
        gpio_bit_reset(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
        gpio_bit_write(enet_handle->mdio_gpiox, enet_handle->mdio_gpio_pinx, (command >> (13-i)) & 0x01);
        
        for(uint8_t t = 0; t < CLK_DELAY; t++)
        {
            __NOP();
        }
        gpio_bit_set(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
        for(uint8_t t = 0; t < CLK_DELAY; t++)
        {
            __NOP();
        }
    }
    
    
    // ????????
    if(ENET_PHY_READ == direction) {
//        gpio_bit_set(GPIOF, SMI_DATA_Pin); // ????DATA???????????
        gpio_mode_set(enet_handle->mdio_gpiox, GPIO_MODE_INPUT, GPIO_PUPD_NONE, enet_handle->mdio_gpio_pinx);
        for(i = 0; i < 16; i++) {
            gpio_bit_reset(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
            for(uint8_t t = 0; t < CLK_DELAY; t++)
            {
                __NOP();
            }
            data |= gpio_input_bit_get(enet_handle->mdio_gpiox, enet_handle->mdio_gpio_pinx) << (15 - i);
            gpio_bit_set(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
            for(uint8_t t = 0; t < CLK_DELAY; t++)
            {
                __NOP();
            }
        }
        *pvalue = (uint16_t)data;
        gpio_mode_set(enet_handle->mdio_gpiox, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, enet_handle->mdio_gpio_pinx);
        gpio_output_options_set(enet_handle->mdio_gpiox, GPIO_OTYPE_OD, GPIO_OSPEED_MAX, enet_handle->mdio_gpio_pinx);
    }else
    {
        data = *pvalue;
        // ????DATA????
        for(i = 0; i < 16; i++) {
            gpio_bit_reset(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
            gpio_bit_write(enet_handle->mdio_gpiox, enet_handle->mdio_gpio_pinx, (data >> (15-i)) & 0x01);
            for(uint8_t t = 0; t < CLK_DELAY; t++)
            {
                __NOP();
            }
            gpio_bit_set(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
            for(uint8_t t = 0; t < CLK_DELAY; t++)
            {
                __NOP();
            }
        }
    }
    
    ////IdleState
    gpio_bit_set(enet_handle->mdio_gpiox, enet_handle->mdio_gpio_pinx);
    for(uint8_t t = 0; t < CLK_DELAY; t++)
    {
        __NOP();
    }
    gpio_bit_set(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
    for(uint8_t t = 0; t < CLK_DELAY; t++)
    {
        __NOP();
    }
    gpio_bit_reset(enet_handle->mdc_gpiox, enet_handle->mdc_gpio_pinx);
    for(uint8_t t = 0; t < CLK_DELAY; t++)
    {
        __NOP();
    }
    return SUCCESS;
}
ErrStatus enet_bsp_init(enet_mediamode_enum mediamode, enet_chksumconf_enum checksum, enet_frmrecept_enum recept)
{
    uint32_t reg_value = 0U, reg_temp = 0U, temp = 0U;
    uint32_t media_temp = 0U;
    uint32_t timeout = 0U;
    uint16_t phy_value = 0U;
    ErrStatus phy_state = ERROR, enet_state = ERROR;

    /* PHY interface configuration, configure SMI clock and reset PHY chip */
//    if(ERROR == enet_phy_config()) {
//        _ENET_DELAY_(PHY_RESETDELAY);
//        if(ERROR == enet_phy_config()) {
//            return enet_state;
//        }
//    }
    gpio_bit_reset(GPIOF, GPIO_PIN_2);
    enet_bsp_delay(PHY_RESETDELAY);
    gpio_bit_set(GPIOF, GPIO_PIN_2);
    
    
    /* initialize ENET peripheral with generally concerned parameters */
    enet_bsp_default_init();
//enet_bsp_delay(PHY_RESETDELAY);
enet_phy_bsp_write_read(ENET_PHY_READ, PHY_ADDRESS, 2, &phy_value);
enet_phy_bsp_write_read(ENET_PHY_READ, PHY_ADDRESS, 2, &phy_value);
    /* 1st, configure mediamode */
    media_temp = (uint32_t)mediamode;
    /* if is PHY auto negotiation */
    if((uint32_t)ENET_AUTO_NEGOTIATION == media_temp) {
        /* wait for PHY_LINKED_STATUS bit be set */
        do {
            enet_phy_bsp_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_BSR, &phy_value);
            phy_value &= PHY_LINKED_STATUS;
            timeout++;
        } while((RESET == phy_value) && (timeout < PHY_READ_TO));
        /* return ERROR due to timeout */
        if(PHY_READ_TO == timeout) {
            return enet_state;
        }
        /* reset timeout counter */
        timeout = 0U;

        /* enable auto-negotiation */
        phy_value = PHY_AUTONEGOTIATION;
        phy_state = enet_phy_bsp_write_read(ENET_PHY_WRITE, PHY_ADDRESS, PHY_REG_BCR, &phy_value);
        if(!phy_state) {
            /* return ERROR due to write timeout */
            return enet_state;
        }

        /* wait for the PHY_AUTONEGO_COMPLETE bit be set */
        do {
            enet_phy_bsp_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_BSR, &phy_value);
            phy_value &= PHY_AUTONEGO_COMPLETE;
            timeout++;
        } while((RESET == phy_value) && (timeout < (uint32_t)PHY_READ_TO));
        /* return ERROR due to timeout */
        if(PHY_READ_TO == timeout) {
            return enet_state;
        }
        phy_value = 7;
        enet_phy_bsp_write_read(ENET_PHY_WRITE, PHY_ADDRESS, 31, &phy_value);
        phy_value = 0x1FFE;
        enet_phy_bsp_write_read(ENET_PHY_WRITE, PHY_ADDRESS, 16, &phy_value);
        phy_value = 0;
        enet_phy_bsp_write_read(ENET_PHY_WRITE, PHY_ADDRESS, 31, &phy_value);
        /* reset timeout counter */
        timeout = 0U;

        /* read the result of the auto-negotiation */
        enet_phy_bsp_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_SR, &phy_value);
        /* configure the duplex mode of MAC following the auto-negotiation result */
        if((uint16_t)RESET != (phy_value & PHY_DUPLEX_STATUS)) {
            media_temp = ENET_MODE_FULLDUPLEX;
        } else {
            media_temp = ENET_MODE_HALFDUPLEX;
        }
        /* configure the communication speed of MAC following the auto-negotiation result */
        if((uint16_t)RESET != (phy_value & PHY_SPEED_STATUS)) {
            media_temp |= ENET_SPEEDMODE_10M;
        } else {
            media_temp |= ENET_SPEEDMODE_100M;
        }
    } else {
        phy_value = (uint16_t)((media_temp & ENET_MAC_CFG_DPM) >> 3);
        phy_value |= (uint16_t)((media_temp & ENET_MAC_CFG_SPD) >> 1);
        phy_state = enet_phy_bsp_write_read(ENET_PHY_WRITE, PHY_ADDRESS, PHY_REG_BCR, &phy_value);
        if(!phy_state) {
            /* return ERROR due to write timeout */
            return enet_state;
        }
        /* PHY configuration need some time */
        enet_bsp_delay(PHY_CONFIGDELAY);
    }
    /* after configuring the PHY, use mediamode to configure registers */
    reg_value = ENET_MAC_CFG;
    /* configure ENET_MAC_CFG register */
    reg_value &= (~(ENET_MAC_CFG_SPD | ENET_MAC_CFG_DPM | ENET_MAC_CFG_LBM));
    reg_value |= media_temp;
    ENET_MAC_CFG = reg_value;


    /* 2st, configure checksum */
    if(RESET != ((uint32_t)checksum & ENET_CHECKSUMOFFLOAD_ENABLE)) {
        ENET_MAC_CFG |= ENET_CHECKSUMOFFLOAD_ENABLE;

        reg_value = ENET_DMA_CTL;
        /* configure ENET_DMA_CTL register */
        reg_value &= ~ENET_DMA_CTL_DTCERFD;
        reg_value |= ((uint32_t)checksum & ENET_DMA_CTL_DTCERFD);
        ENET_DMA_CTL = reg_value;
    }

    /* 3rd, configure recept */
    ENET_MAC_FRMF |= (uint32_t)recept;


    enet_state = SUCCESS;
    return enet_state;
}

////////////////////////////////////////

/*!
    \brief      setup ethernet system(GPIOs, clocks, MAC, DMA, systick)
    \param[in]  none
    \param[out] none
    \retval     none
*/
void EnetInit(Enet_HandleType *pHandle)
{
    enet_handle = pHandle;
#ifdef USE_ENET_INTERRUPT
    nvic_configuration();
#endif /* USE_ENET_INTERRUPT */
    /* configure the GPIO ports for ethernet pins */
    enet_gpio_config(pHandle);

    /* configure the ethernet MAC/DMA */
    enet_mac_dma_config();

    if(0 == enet_init_status) {
//        while(1) {
//        }
    }

#ifdef USE_ENET_INTERRUPT
    enet_interrupt_enable(ENET_DMA_INT_NIE);
    enet_interrupt_enable(ENET_DMA_INT_RIE);
#endif /* USE_ENET_INTERRUPT */

#ifdef SELECT_DESCRIPTORS_ENHANCED_MODE
    enet_desc_select_enhanced_mode();
#endif /* SELECT_DESCRIPTORS_ENHANCED_MODE */

}
/*!
    \brief      configures the ethernet interface
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void enet_mac_dma_config(void)
{
    ErrStatus reval_state = ERROR;

    /* enable ethernet clock  */
    rcu_periph_clock_enable(RCU_ENET);
    rcu_periph_clock_enable(RCU_ENETTX);
    rcu_periph_clock_enable(RCU_ENETRX);

    /* reset ethernet on AHB bus */
    enet_deinit();

    reval_state = enet_software_reset();
    if(ERROR == reval_state) {
        while(1) {}
    }

    /* configure the parameters which are usually less cared for enet initialization */
//  enet_initpara_config(HALFDUPLEX_OPTION, ENET_CARRIERSENSE_ENABLE|ENET_RECEIVEOWN_ENABLE|ENET_RETRYTRANSMISSION_DISABLE|ENET_BACKOFFLIMIT_10|ENET_DEFERRALCHECK_DISABLE);
//  enet_initpara_config(DMA_OPTION, ENET_FLUSH_RXFRAME_ENABLE|ENET_SECONDFRAME_OPT_ENABLE|ENET_NORMAL_DESCRIPTOR);

#ifdef CHECKSUM_BY_HARDWARE
    if(enet_handle->smi_sim == true)
    {
        enet_init_status = enet_bsp_init(ENET_100M_FULLDUPLEX, ENET_AUTOCHECKSUM_DROP_FAILFRAMES, ENET_BROADCAST_FRAMES_PASS);
    }
    else
    {
        enet_init_status = enet_init(ENET_AUTO_NEGOTIATION, ENET_AUTOCHECKSUM_DROP_FAILFRAMES, ENET_BROADCAST_FRAMES_PASS);
    }
#else
    enet_init_status = enet_init(ENET_AUTO_NEGOTIATION, ENET_NO_AUTOCHECKSUM, ENET_BROADCAST_FRAMES_PASS);
#endif /* CHECKSUM_BY_HARDWARE */

}

#ifdef USE_ENET_INTERRUPT
/*!
    \brief      configures the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void nvic_configuration(void)
{
    nvic_irq_enable(ENET_IRQn, 0, 2);
}
#endif /* USE_ENET_INTERRUPT */

/*!
    \brief      configures the different GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
*/



