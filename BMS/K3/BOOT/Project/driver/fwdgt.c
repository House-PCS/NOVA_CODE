/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
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
  * <tr><td>2022-11-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */



#include "fwdgt.h"

#define LOG_TAG     "fwdgt"
#define FEEDDOGTIMEOUT 5      //单位：s
#define FEEDDOGTIME    3      //单位：s

uint8_t gFwdgtRstFlag = 0;

en_wdg_state en_wdg = EN_WDG_IDLE;

void FeedDog(void)
{
    /* reload FWDGT counter */
    fwdgt_counter_reload();
//    LOG_DEBUG(LOG_TAG, "Feed Dog ok!\r\n");
}

void FwdgtOpenCmd(void)
{
    fwdgt_enable();
    /* reload FWDGT counter */
    FeedDog();
}

bool FwdgtInit(void)
{
    uint32_t timeout = 0xFFFFFFF;
    /* enable IRC40K */
    rcu_osci_on(RCU_IRC40K);

    /* wait till IRC40K is ready */
    while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K) && timeout --);

    if(timeout == 0)
    {
        return false;
    }

    /* confiure FWDGT counter clock: 40KHz(IRC40K) / 64 = 0.625 KHz */
    fwdgt_config(FEEDDOGTIMEOUT * 625, FWDGT_PSC_DIV64);

    /* check if the system has resumed from FWDGT reset */
    if(RESET != rcu_flag_get(RCU_FLAG_FWDGTRST))
    {

        /* clear the FWDGT reset flag */
        rcu_all_reset_flag_clear();
        gFwdgtRstFlag = 1;
    }
    else
    {
        gFwdgtRstFlag = 0;
    }
    return true;
}

void FwdgtProcess(void)
{
    static uint32_t sn_time = 0;

    switch(en_wdg)
    {
        case EN_WDG_IDLE:
        {
            break;
        }
        case EN_OPEN_WDG:
        {
            FwdgtOpenCmd();
            en_wdg = EN_WAIT_FEED;
            break;
        }
        case EN_STOP_FEED:
        {
            en_wdg = EN_WDG_IDLE;
            break;
        }
        case EN_WAIT_FEED:
        {
            if(tim_difftime_from_now(sn_time) > FEEDDOGTIME * 1000)
            {
                sn_time = tim_get_systime();
                /* reload FWDGT counter */
                FeedDog();
            }
            break;
        }
        default:
            break;
    }
}

void WdgtTest(uint32_t argc, uint8_t **argv)
{
    if((argc == 2) && (atoi((char *)argv[1]) == 1))
    {
        en_wdg = EN_OPEN_WDG;
    }
    else if((argc == 2) && (atoi((char *)argv[1]) == 2))
    {
        en_wdg = EN_STOP_FEED;
    }
    else if((argc == 2) && (atoi((char *)argv[1]) == 3))
    {
        /* reload FWDGT counter */
        FeedDog();
    }
}

MSH_CMD_EXPORT(WdgtTest, 1 - open_wdgt 2 - stop_feed 3 - feed_dog);