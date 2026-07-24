/*
 * flash_default.c
 *
 * Created on: 2026-01-30
 * Author: Nova007457
 *
 * Factory defaults, serialization, and SPI Flash persistence for UdpDspHoldPara_t
 * (Modbus hold registers 0x6220~0x631F).
 */

#include "flash_default.h"
#include "ethernet_analysis.h"
#include "dev_flash.h"
#include "string.h"
#include <stddef.h>


#define FLASH_FRAME_WORDS          4u

/* Valid / invalid frame headers at FLASH_DspHoldFRAME_START_ADDR */
static const uint16_t Flash_DspHoldType[FLASH_FRAME_WORDS] = {0x55, 0xAA, 0x55, 0xAA};
static const uint16_t Flash_DspHoldType_init[FLASH_FRAME_WORDS] = {0x00, 0x00, 0x00, 0x00};

/* Shared workspace (file scope avoids large stacks on C28x) */
static uint8_t s_flash_byte_buf[2 * sizeof(UdpDspHoldPara_t)];
/* mirror: last saved copy; work: Flash readback + autosave compare scratch */
static UdpDspHoldPara_t s_hold_saved_mirror;
static UdpDspHoldPara_t s_hold_work;

static UdpDspHoldPara_t UdpPCS_DspHoldPara_Init =
{
    .Second_ID = 0,                                     /* 0x6220 Power segment ID: 0=default, 1=6kW .. 6=16kW */
    .work_mode = 0,                                     /* 0x6221 Work mode: 0=self-use, 1=battery first, 2=peak-shaving */
    .charging_input_type = 0,                           /* 0x6222 Forced gen input: 0=grid, 1=generator, 2=auto */
    .set_batt_chg_curr = 0,                             /* 0x6223 Max battery charge current, 0.1 A */
    .set_batt_chg_power = {0, 16000},                   /* 0x6224-0x6225 Max battery charge power, 1 W */
    .set_batt_dsg_curr = 0,                             /* 0x6226 Max battery discharge current, 0.1 A */
    .set_batt_dsg_power = {0, 16000},                   /* 0x6227-0x6228 Max battery discharge power, 1 W */
    .InvPVMaxPower = {0, 24000},                        /* 0x6229-0x622A Max PV power */
    .InvOnGridIFInMaxPower = {0, 24000},                /* 0x622B-0x622C Max on-grid port input power */
    .InvOnGridIFOutMaxPower = {0, 16000},               /* 0x622D-0x622E Max on-grid port output power */
    .InvGenMaxPower = {0, 4000},                        /* 0x622F-0x6230 Max generator power */
    .InvEVMaxInPower = {0, 4000},                       /* 0x6231-0x6232 Max EV charge (absorb) power */
    .InvEVMaxOutPower = {0, 4000},                      /* 0x6233-0x6234 Max EV discharge (supply) power */
    .NetworkPowerMaxPowerLimit = {0, 24000},             /* 0x6235-0x6236 Household import power limit */
    .on_grid_power = {0, 0},                            /* 0x6237-0x6238 Max feed-in / on-grid power */

    .pv_input_mode = 0,                                 /* 0x6250 PV input: 0=independent, 1=parallel, 2=constant V */
    .anti_reverse_flow_enable = 0,                      /* 0x6251 Anti-export: 0=block PV to grid, 1=allow */

    .reserve2 = {0, 0},                                  /* 0x6252-0x6253 Reserved */

    .x_boost = 0,                                        /* 0x6254 X-BOOST: 0=disable, 1=enable */
    .ups_delay_0ms = 1,                                 /* 0x6255 UPS 0 ms transfer: 0=disable, 1=enable */
    .backup_output_enable = 1,                          /* 0x6256 Backup load output: 0=disable, 1=enable */
    .program_ctrl_enable = 0,                           /* 0x6257 Program control: 0=disable, 1=enable */
    .over_volt_derating_enable = 0,                     /* 0x6258 Over-voltage derating */
    .over_freq_derating_enable = 0,                     /* 0x6259 Over-frequency derating */
    .under_volt_derating_enable = 0,                    /* 0x625A Under-voltage derating */
    .under_freq_derating_enable = 0,                    /* 0x625B Under-frequency derating */
    .island_enable = 1,                                /* 0x625C Anti-islanding: 0=disable, 1=enable */
    .ac_coupling_enable = 1,                            /* 0x625D AC coupling: 0=disable, 1=enable */
    .high_low_crossover_enable = 0,                     /* 0x625E HV/LV ride-through: 0=disable, 1=enable */
    .drm_state = 0x0010,                                /* 0x625F DRM bits; default DRM4 (bit4) */
    .LLCEnable = 0,                                     /* 0x6260 LLC after Pack precharge: 0=off, 1=on */

    .cto_meter = 0,                                      /* 0x6266 CT or meter: 0=CT, 1=meter */
    .ct_direction = 0,                                  /* 0x6267 CT direction: 0=forward, 1=reverse */
    .meter_brand = 0,                                    /* 0x6268 Meter brand: 0=CHNT, 1=Acrel */
    .grid_standards = 2,                                /* 0x6269 Grid standard: 2=Australia A, etc. */
    .grad_standards_segment = 0,                         /* 0x626A Grid standard sub-segment */
    .BatteryChgMaxSOC = 100,                             /* 0x626B Charge stop SOC % */
    .OnGridBatteryDsgMinSoc = 10,                       /* 0x626C On-grid discharge stop SOC % */
    .OffGridBatteryDsgMinSoc = 10,                      /* 0x626D Off-grid discharge stop SOC % */

    .reserve4 = {0, 0, 0, 0, 0, 0, 0, 0},               /* 0x626E-0x6275 Reserved */

    .reset_factory = 0,                                 /* 0x6276 Factory reset trigger: write 0x5AA5 */
    .on_grid_reconnect_time = 30,                        /* 0x6277 Grid reconnect delay, s (5-300) */
    .reactive_power_respones_time = 10,                 /* 0x6278 Reactive response time, s */
    .reactiveSet = 0,                                    /* 0x6279 Reactive setpoint -100..100 */
    .reactive_power_ctrl_method = 1,                     /* 0x627A Reactive ctrl: 0=PF, 1=Q, 2=QU, 3=QP */
    .ActiveSet = 100,                                    /* 0x627B Active power limit 0-100 % */
    .FactorSet = 0,                                      /* 0x627C Power factor setpoint */

    .reserve5 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* 0x627D-0x628F Reserved */

    .remote_shutdown_status = 0,                        /* 0x6290 Remote shutdown status */
    .PV_Switch = 1,                                      /* 0x6291 PV switch: 0=disable, 1=enable */
    .PF_Switch = 1,                                      /* 0x6292 PF switch: 0=disable, 1=enable */
    .dsp_wake_en = 3,                                    /* 0x6293 Wake: 1=PV wake, 2=PV wake done, 3=normal */
    .pcs_dsg_ctrl = 1,                                   /* 0x6294 Discharge ctrl: 0=block on fault, 1=allow, 2=forbid */
    .pcs_chg_ctrl = 1,                                   /* 0x6295 Charge ctrl: 0=block on fault, 1=allow, 2=forbid */
    .pcs_user_type = 0,                                 /* 0x6296 User type: 0=user .. 3=internal */
    .ignor_ct_err = 0,                                   /* 0x6297 Ignore CT fault: 0=no, 1=yes */
    .sys_upgrade_status = 0,                             /* 0x6298 System upgrade flag */
    .sys_elec_clear = 0,                                 /* 0x6299 Energy counter clear */
    .sys_shut_down = 0,                                  /* 0x629A Shutdown command */

    .reserve6 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* 0x629B-0x62AF Reserved */

    .batt_dsg_volt_point = 3700,                        /* 0x62B0 Battery discharge stop voltage, 0.1 V */
    .batt_chg_volt_point = 5500,                        /* 0x62B1 Battery charge voltage limit, 0.1 V */
    .pv_uvp_point = 180,                                 /* 0x62B2 PV under-voltage protection, 0.1 V */
    .pv_ovp_point = 1000,                                /* 0x62B3 PV over-voltage protection, 0.1 V */
    .ev_uvp_point = 0,                                   /* 0x62B4 EV under-voltage protection, 0.1 V */
    .ev_ovp_point = 0,                                   /* 0x62B5 EV over-voltage protection, 0.1 V */
    .grid_volt_low_limit = 1750,                        /* 0x62B6 Grid voltage low limit, 0.1 V */
    .grid_volt_upper_limit = 2750,                      /* 0x62B7 Grid voltage high limit, 0.1 V */
    .grid_freq_low_limit = 4500,                        /* 0x62B8 Grid frequency low limit, 0.01 Hz */
    .grid_freq_upper_limit = 5500,                      /* 0x62B9 Grid frequency high limit, 0.01 Hz */
    .bypass_volt_low_limit = 1750,                      /* 0x62BA Bypass voltage low limit, 0.1 V */
    .bypass_volt_upper_limit = 2750,                    /* 0x62BB Bypass voltage high limit, 0.1 V */
    .bypass_freq_low_limit = 4500,                      /* 0x62BC Bypass frequency low limit, 0.01 Hz */
    .bypass_freq_upper_limit = 5500,                    /* 0x62BD Bypass frequency high limit, 0.01 Hz */
    .over_volt_derating_point = 270,                    /* 0x62BE Over-voltage derating point, 1 % */
    .over_freq_derating_point = 5400,                   /* 0x62BF Over-frequency derating point */
    .under_volt_derating_point = 1800,                  /* 0x62C0 Under-voltage derating point, 1 % */
    .under_freq_derating_point = 4600,                  /* 0x62C1 Under-frequency derating point */

    .reserve7 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, /* 0x62C2-0x62CF Reserved */

    .CalibrationReset = 0,                               /* 0x62D0 Calibration reset */
    .reserve8 = {0, 0, 0, 0},                           /* 0x62D1-0x62D4 Reserved */
    .r_phase_volt_calibration_k_parm = 4096,           /* 0x62D5 R-phase voltage cal K (default 4096 = 1.0) */
    .r_phase_volt_calibration_b_parm = 4096,           /* 0x62D6 R-phase voltage cal B */
    .s_phase_volt_calibration_k_parm = 4096,           /* 0x62D7 S-phase voltage cal K */
    .s_phase_volt_calibration_b_parm = 4096,           /* 0x62D8 S-phase voltage cal B */
    .t_phase_volt_calibration_k_parm = 4096,           /* 0x62D9 T-phase voltage cal K */
    .t_phase_volt_calibration_b_parm = 4096,           /* 0x62DA T-phase voltage cal B */
    .r_phase_cur_calibration_k_parm = 4096,              /* 0x62DB R-phase current cal K */
    .r_phase_cur_calibration_b_parm = 4096,              /* 0x62DC R-phase current cal B */
    .s_phase_cur_calibration_k_parm = 4096,              /* 0x62DD S-phase current cal K */
    .s_phase_cur_calibration_b_parm = 4096,              /* 0x62DE S-phase current cal B */
    .t_phase_cur_calibration_k_parm = 4096,              /* 0x62DF T-phase current cal K */
    .t_phase_cur_calibration_b_parm = 4096,              /* 0x62F0 T-phase current cal B */

    .Inv_Battery_Volt_Calibration_CoefK = 4096,        /* 0x62F1 Battery voltage cal K */
    .Inv_Battery_Volt_Calibration_CoefB = 4096,        /* 0x62F2 Battery voltage cal B */
    .Inv_Battery_Cur_Calibration_CoefK = 4096,         /* 0x62F3 Battery current cal K */
    .Inv_Battery_Cur_Calibration_CoefB = 4096,         /* 0x62F4 Battery current cal B */
    .Inv_EV_Volt_Calibration_CoefK = 4096,             /* 0x62F5 EV voltage cal K */
    .Inv_EV_Volt_Calibration_CoefB = 4096,             /* 0x62F6 EV voltage cal B */
    .Inv_EV_Cur_Calibration_CoefK = 4096,              /* 0x62F7 EV current cal K */
    .Inv_EV_Cur_Calibration_CoefB = 4096,              /* 0x62F8 EV current cal B */

    .pv_ch1_volt_k_calibrate = 4096,                    /* 0x62F9 PV1 voltage cal K */
    .pv_ch1_volt_b_calibrate = 4096,                    /* 0x62FA PV1 voltage cal B */
    .pv_ch1_curr_k_calibrate = 4096,                    /* 0x62FB PV1 current cal K */
    .pv_ch1_curr_b_calibrate = 4096,                    /* 0x62FC PV1 current cal B */

    .pv_ch2_volt_k_calibrate = 4096,                    /* 0x62FD PV2 voltage cal K */
    .pv_ch2_volt_b_calibrate = 4096,                    /* 0x62FE PV2 voltage cal B */
    .pv_ch2_curr_k_calibrate = 4096,                    /* 0x62FF PV2 current cal K */
    .pv_ch2_curr_b_calibrate = 4096,                    /* 0x6300 PV2 current cal B */

    .pv_ch3_volt_k_calibrate = 4096,                    /* 0x6301 PV3 voltage cal K */
    .pv_ch3_volt_b_calibrate = 4096,                    /* 0x6302 PV3 voltage cal B */
    .pv_ch3_curr_k_calibrate = 4096,                    /* 0x6303 PV3 current cal K */
    .pv_ch3_curr_b_calibrate = 4096,                    /* 0x6304 PV3 current cal B */

    .pv_ch4_volt_k_calibrate = 4096,                    /* 0x6305 PV4 voltage cal K */
    .pv_ch4_volt_b_calibrate = 4096,                    /* 0x6306 PV4 voltage cal B */
    .pv_ch4_curr_k_calibrate = 4096,                    /* 0x6307 PV4 current cal K */
    .pv_ch4_curr_b_calibrate = 4096,                    /* 0x6308 PV4 current cal B */

    .reserve9 = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};


/* Pack struct as little-endian byte stream (C28x: sizeof is word count) */
static void struct_to_bytes(UdpDspHoldPara_t *src, uint8_t *dest)
{
    uint16_t *p = (uint16_t *)src;
    size_t i;
    size_t n = sizeof(UdpDspHoldPara_t);

    for (i = 0; i < n; i++)
    {
        dest[2 * i]     = (uint8_t)(p[i] & 0x00FF);
        dest[2 * i + 1] = (uint8_t)((p[i] >> 8) & 0x00FF);
    }
}

/* Unpack little-endian byte stream into struct */
static void bytes_to_struct(uint8_t *src, UdpDspHoldPara_t *dest)
{
    uint16_t *p = (uint16_t *)dest;
    size_t i;
    size_t n = sizeof(UdpDspHoldPara_t);

    for (i = 0; i < n; i++)
    {
        p[i] = (uint16_t)((src[2 * i] & 0x00FF) | ((src[2 * i + 1] & 0x00FF) << 8));
    }
}

static void flash_frame_read(uint16_t *hdr)
{
    SpiFlashReadBytes(FLASH_DspHoldFRAME_START_ADDR, hdr, FLASH_FRAME_WORDS);
}

static bool flash_frame_is_valid(const uint16_t *hdr)
{
    return (hdr[0] == Flash_DspHoldType[0]
         && hdr[1] == Flash_DspHoldType[1]
         && hdr[2] == Flash_DspHoldType[2]
         && hdr[3] == Flash_DspHoldType[3]);
}

static bool flash_frame_is_invalid(const uint16_t *hdr)
{
    return (hdr[0] == Flash_DspHoldType_init[0]
         && hdr[1] == Flash_DspHoldType_init[1]
         && hdr[2] == Flash_DspHoldType_init[2]
         && hdr[3] == Flash_DspHoldType_init[3]);
}

static void hold_snapshot_sync_from(const UdpDspHoldPara_t *src)
{
    memcpy(&s_hold_saved_mirror, src, sizeof(s_hold_saved_mirror));
}

/* Clear transient / command fields before auto-save equality check */
#define HOLD_MIRROR_TRANSIENT_WORDS  10u

static void hold_mask_excluded_for_compare(UdpDspHoldPara_t *p)
{
    p->sys_shut_down = 0;
    p->LLCEnable = 0;
    p->remote_shutdown_status = 0;
    p->charging_input_type = 0;
    p->set_batt_chg_power[0] = 0;
    p->set_batt_chg_power[1] = 0;
    p->set_batt_dsg_power[0] = 0;
    p->set_batt_dsg_power[1] = 0;
    p->reset_factory = 0;
    p->sys_upgrade_status = 0;
}

static void hold_mirror_transients_save(uint16_t bak[HOLD_MIRROR_TRANSIENT_WORDS],
                                        const UdpDspHoldPara_t *m)
{
    bak[0] = m->sys_shut_down;
    bak[1] = m->LLCEnable;
    bak[2] = m->remote_shutdown_status;
    bak[3] = m->charging_input_type;
    bak[4] = m->set_batt_chg_power[0];
    bak[5] = m->set_batt_chg_power[1];
    bak[6] = m->set_batt_dsg_power[0];
    bak[7] = m->set_batt_dsg_power[1];
    bak[8] = m->reset_factory;
    bak[9] = m->sys_upgrade_status;
}

static void hold_mirror_transients_restore(UdpDspHoldPara_t *m,
                                           const uint16_t bak[HOLD_MIRROR_TRANSIENT_WORDS])
{
    m->sys_shut_down = bak[0];
    m->LLCEnable = bak[1];
    m->remote_shutdown_status = bak[2];
    m->charging_input_type = bak[3];
    m->set_batt_chg_power[0] = bak[4];
    m->set_batt_chg_power[1] = bak[5];
    m->set_batt_dsg_power[0] = bak[6];
    m->set_batt_dsg_power[1] = bak[7];
    m->reset_factory = bak[8];
    m->sys_upgrade_status = bak[9];
}

/* Compare live RAM vs mirror using one scratch (mirror masked in place, then restored) */
static bool hold_live_equals_saved_mirror(void)
{
    uint16_t bak[HOLD_MIRROR_TRANSIENT_WORDS];
    bool same;

    hold_mirror_transients_save(bak, &s_hold_saved_mirror);
    hold_mask_excluded_for_compare(&s_hold_saved_mirror);

    memcpy(&s_hold_work, &gstUdpPcsPara.udp_pcs_hold, sizeof(s_hold_work));
    hold_mask_excluded_for_compare(&s_hold_work);

    same = (memcmp(&s_hold_work, &s_hold_saved_mirror, sizeof(s_hold_work)) == 0);

    hold_mirror_transients_restore(&s_hold_saved_mirror, bak);
    return same;
}

void UdpDspHoldPara_AutoSaveIfNeeded(uint16_t *err_cnt)
{
    if (hold_live_equals_saved_mirror())
    {
        return;
    }

    if (!Save_System_Parameter(&gstUdpPcsPara.udp_pcs_hold) && (err_cnt != NULL))
    {
        (*err_cnt)++;
    }
}


void Init_System_Parameter(void)
{
    uint16_t flash_hdr[FLASH_FRAME_WORDS];

    flash_frame_read(flash_hdr);
    if (!flash_frame_is_valid(flash_hdr))
    {
        memcpy((uint16_t *)&gstUdpPcsPara.udp_pcs_hold, (uint16_t *)&UdpPCS_DspHoldPara_Init,
               sizeof(UdpDspHoldPara_t));
        /* Mirror updated inside Save_System_Parameter only on success */
        (void)Save_System_Parameter(&gstUdpPcsPara.udp_pcs_hold);
    }
    else
    {
        SpiFlashReadBytes(FLASH_DspHoldPara_START_ADDR, (uint16_t *)s_flash_byte_buf,
                          sizeof(UdpDspHoldPara_t));
        bytes_to_struct(s_flash_byte_buf, &s_hold_work);
        memcpy((uint16_t *)&gstUdpPcsPara.udp_pcs_hold, (uint16_t *)&s_hold_work,
               sizeof(UdpDspHoldPara_t));
        hold_snapshot_sync_from(&gstUdpPcsPara.udp_pcs_hold);
    }
}


bool Save_System_Parameter(UdpDspHoldPara_t *para)
{
    uint16_t flash_hdr[FLASH_FRAME_WORDS];

    if (para == NULL)
    {
        return false;
    }

    /* Two-phase commit: invalidate header -> write data -> validate header */
    SpiFlashWriteBytesWithErase(FLASH_DspHoldFRAME_START_ADDR, (uint16_t *)&Flash_DspHoldType_init,
                                FLASH_FRAME_WORDS);
    struct_to_bytes(para, s_flash_byte_buf);
    SpiFlashWriteBytesWithErase(FLASH_DspHoldPara_START_ADDR, (uint16_t *)s_flash_byte_buf,
                                sizeof(UdpDspHoldPara_t));
    SpiFlashWriteBytesWithErase(FLASH_DspHoldFRAME_START_ADDR, (uint16_t *)&Flash_DspHoldType,
                                FLASH_FRAME_WORDS);

    flash_frame_read(flash_hdr);
    if (!flash_frame_is_valid(flash_hdr))
    {
        return false;
    }

    SpiFlashReadBytes(FLASH_DspHoldPara_START_ADDR, (uint16_t *)s_flash_byte_buf, sizeof(UdpDspHoldPara_t));
    bytes_to_struct(s_flash_byte_buf, &s_hold_work);
    if (memcmp((uint16_t *)para, (uint16_t *)&s_hold_work, sizeof(UdpDspHoldPara_t)) != 0)
    {
        return false;
    }

    hold_snapshot_sync_from(para);
    return true;
}


bool ShutDown_SaveFlash(uint16_t *err_cnt)
{
    memcpy(&s_hold_work, &gstUdpPcsPara.udp_pcs_hold, sizeof(s_hold_work));
    hold_mask_excluded_for_compare(&s_hold_work);
    if (!Save_System_Parameter(&s_hold_work))
    {
        if (err_cnt != NULL)
        {
            (*err_cnt)++;
        }
        return false;
    }
    return true;
}


bool Restore_Factory_Settings(uint16_t *reset_cnt,bool actual_reset_factory)
{

    uint16_t flash_hdr[FLASH_FRAME_WORDS];

    if (actual_reset_factory)
    {
        if (!Save_System_Parameter(&UdpPCS_DspHoldPara_Init))
        {
            if (reset_cnt != NULL)
            {
                (*reset_cnt)++;
            }
            return false;
        }
        memcpy((uint16_t *)&gstUdpPcsPara.udp_pcs_hold, (uint16_t *)&UdpPCS_DspHoldPara_Init,
               sizeof(UdpDspHoldPara_t));
    }
    else
    {
        flash_frame_read(flash_hdr);
        if (flash_frame_is_valid(flash_hdr))
        {
            SpiFlashWriteBytesWithErase(FLASH_DspHoldFRAME_START_ADDR,
                                    (uint16_t *)&Flash_DspHoldType_init, FLASH_FRAME_WORDS);

            if (reset_cnt != NULL)
            {
                (*reset_cnt)++;
            }
            return false;
        }
    }
    return true;
}
