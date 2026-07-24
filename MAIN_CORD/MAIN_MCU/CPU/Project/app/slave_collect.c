#include "slave_collect.h"
#include "dev_modbus.h"
#include "dev_modbus_rtu.h"
#include "sys_info_storage.h"
#include "tools_program.h"
#include "dev_key.h"
typedef  struct
{
    uint8_t  len;
    uint8_t  *pData;//[256];
} CollectTxData_t;
EN_COLLECT_POLLING_STATUS   geCollectSlavePollingStatus;
uint8_t                     _garrComCollectTxDataBuf[COLLECT_WRITE_SIZE];
static  CollectTxData_t     _gstComCollectTxData; 
static  uint8_t             _gReadAllBmsDataOkFlag=0;
CollectPara_t               gstCollectParaData;
CollectInfo_t               gstCollectInfo;   //新协议用的变量
CollectInfo_t               gstInstallerCollectInfo;

CollectStorageInfo_t     gstCollectNormalStorageData;
CollectStorageInfo_t     gstCollectInstallerStorageData;


uint16_t    gstMinBattSoc = 0;
typedef struct _pcs_type_info_t
{
    uint16_t model_id;
    char model[32];
} PcsTypeInfo_t;

PcsTypeInfo_t garrPcsTypeInfo[] =
{
    { 0x00,         "Unknown" },
    { 0xC291,       "Roca3-16K"},
    { 0xC2B1,       "Roca3-15K"}
};

void ModbusReadParse(ModbusRtuFrame_t *ptFrame);
bool ModbusWriteParse(ModbusRtuFrame_t *ptFrame);


const char* GetPcsModelByModelId(uint16_t model_id)
{
    for (int32_t i = 0; i < sizeof(garrPcsTypeInfo) / sizeof(garrPcsTypeInfo[0]); ++i)
    {
        if (garrPcsTypeInfo[i].model_id == model_id)
        {
            return garrPcsTypeInfo[i].model;
        }
    }

    return NULL;
}

typedef struct _collect_ctrl
{

    struct _write_flag
    {
        uint16_t     correct_rtc_time_en : 1;
        uint16_t     write_bms_en  : 1;
        uint16_t     write_pcs_en  : 1;
        uint16_t     write_dcdc_en : 1;
        uint16_t     monitor_data_to_pcs_en : 1;
        uint16_t     monitor_data_to_bms_en : 1;
        uint16_t     write_monitor_base_info_en : 1;
        uint16_t     write_monitor_ctrl_info_en : 1;
        uint16_t     write_updata_normal_data_en:1;
        uint16_t     backlight_time_updata: 1;
        uint16_t     write_arc_en: 1;
        uint16_t     set_test_mode:1;
        uint16_t     f_device_ctl_write;        //DeviceCtlWrite;
        uint16_t     f_base_info_write;
        uint16_t     f_rest_write;
    } bits;
    uint16_t    factory_status;
} CollectCtrl_t;

static  CollectCtrl_t   _gstCollectCtrl;
uint16_t RestUserType = 0;
static  const ModbusDataPoint_t NomalUserDataPoint[] =
{
    {COLLECT_MODE_ID,                               MODE_ID_NUM,                                 EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.model_id                            },//0
    {COLLECT_SN_ADDR,                               COLLECT_SN_NUM,                              EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.sn[0]                               },//1
    {COLLECT_HW_VER_ADDR,                           COLLECT_HW_VER_NUM,                          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.hw_ver                              },//2
    {COLLECT_SW_VER_ADDR,                           COLLECT_SW_VER_NUM,                          EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.sw_ver                              },//3
    {COLLECT_SW_VER_EXTEND_ADDR,                    COLLECT_SW_VER_EXTEND_NUM,                   EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.sw_ver_ext                          },//4
    {COLLECT_USER_ADDR,                             COLLECT_USER_NUM,                            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.user_type                           },//5
    {COLLECT_NORMAL_PASSWORD_ADDR,                  COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.normal_password[0]                  },//6
    {COLLECT_MANAGER_PASSWORD_ADDR,                 COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.manager_password[0]                 },//7
    {COLLECT_INSTALLER_PASSWORD_ADDR,               COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.installer_password[0]               },//8
    {COLLECT_INTERNAL_PASSWORD_ADDR,                COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.internal_password[0]                },//9
    {COLLECT_SLAVE_ADDR_ADDR,                       COLLECT_SALVE_ADDR_NUM,                      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.modbus_addr                         },//10
    {COLLECT_BAUD_ADDR,                             COLLECT_BAUD_NUM,                            EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.baudrate                            },//11
    {COLLECT_RESERVED_1_ADDR,                       COLLECT_RESERVED_1_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.base_info.reserve[0]                          },//12
    {COLLECT_DEV_TYPE_ADDR,                         COLLECT_DEV_TYPE_NUM,                        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.dev_type                          },//13                                                                                                                                                                                                          
    {COLLECT_LCD_BL_TIME_ADDR,                      COLLECT_LCD_BL_TIME_NUM,                     EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_bl_time                       },//14
    {COLLECT_LCD_BL_BRINGHTNESS_ADDR,               COLLECT_LCD_BL_BRINGHTNESS_NUM,              EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_bl_brightness                 },//15
    {COLLECT_LCD_LANGUEAGE_ADDR,                    COLLECT_LCD_LANGUEAGE_NUM,                   EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_languege                      },//16
    {COLLECT_LED_FRONT_SWITCH_AND_RESE_ADDR,        COLLECT_LED_FRONT_SWITCH_AND_RESE_NUM,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.reserve                           },//17  //5034预留 5035前面板控制 5036预留
    {COLLECT_LED_AUX_MODE_ADDR,                     COLLECT_LED_AUX_MODE_NUM,                    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.led_aux_mode                      },//18
    {COLLECT_RESERVED_2_ADDR,                       COLLECT_RESERVED_2_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.device_info.reserve3[0]                       },//19
    {COLLECT_LOAD_PEAK_TIME_ADDR,                   COLLECT_PEAK_LOAD_TIME_NUM,                  EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.peak_time_info.chg_start_time1                },//20
    {COLLECT_RESERVED_3_ADDR,                       COLLECT_RESERVED_3_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.peak_time_info.reserve[0]                     },//21
    {COLLECT_RESERVED_4_ADDR,                       COLLECT_RESERVED_4_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.total_pack_info.reserve[0]                    },//22
    {COLLECT_PACK_NUM_SECTION_ADDR,                 COLLECT_PACK_NUM_SECTION_LEN,                EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.pack_num                      },//23
    {COLLECT_BMS_ERROR_SECTION_ADDR,                COLLECT_BMS_ERROR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.bms_error                     },//24
    {COLLECT_BMS_REVERSE_SECTION_ADDR,              COLLECT_BMS_REVERSE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.reserve2                      },//25
    {COLLECT_BMS_DSG_MIN_VOLT_SECTION_ADDR,         COLLECT_BMS_DSG_MIN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.total_pack_info.dsg_min_vol                   },//26                                                                                      
    {COLLECT_ELEC_PV_DAY_SECTION_ADDR,              COLLECT_ELEC_PV_DAY_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day               },//27
    {COLLECT_ELEC_PV_MONTH_SECTION_ADDR,            COLLECT_ELEC_PV_MONTH_SECTION_LEN,           EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month             },//28
    {COLLECT_ELEC_BUY_DAY_SECTION_ADDR,             COLLECT_ELEC_BUY_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day              },//29
    {COLLECT_ELEC_BUY_MONTH_SECTION_ADDR,           COLLECT_ELEC_BUY_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month            },//30
    {COLLECT_ELEC_SELL_DAY_SECTION_ADDR,            COLLECT_ELEC_SELL_DAY_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day             },//31
    {COLLECT_ELEC_SELL_MONTH_SECTION_ADDR,          COLLECT_ELEC_SELL_MONTH_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month           },//32
    {COLLECT_ELEC_BAT_CHG_DAY_SECTION_ADDR,         COLLECT_ELEC_BAT_CHG_DAY_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_day          },//33
    {COLLECT_ELEC_BAT_CHG_MONTH_SECTION_ADDR,       COLLECT_ELEC_BAT_CHG_MONTH_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month        },//34
    {COLLECT_ELEC_BAT_DSG_DAY_SECTION_ADDR,         COLLECT_ELEC_BAT_DSG_DAY_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_day          },//35
    {COLLECT_ELEC_BAT_DSG_MONTH_SECTION_ADDR,       COLLECT_ELEC_BAT_DSG_MONTH_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month        },//36
    {COLLECT_ELEC_EPS_DAY_SECTION_ADDR,             COLLECT_ELEC_EPS_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day              },//37
    {COLLECT_ELEC_EPS_MONTH_SECTION_ADDR,           COLLECT_ELEC_EPS_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month            },//38
    {COLLECT_ELEC_REMOTE_LOAD_DAY_SECTION_ADDR,     COLLECT_ELEC_REMOTE_LOAD_DAY_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day      },//39
    {COLLECT_ELEC_REMOTE_LOAD_MONTH_SECTION_ADDR,   COLLECT_ELEC_REMOTE_LOAD_MONTH_SECTION_LEN,  EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month    },//40
    {COLLECT_ELEC_GEN_DAY_SECTION_ADDR,             COLLECT_ELEC_GEN_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_day              },//41
    {COLLECT_ELEC_GEN_MONTH_SECTION_ADDR,           COLLECT_ELEC_GEN_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month            },//42
    {COLLECT_ELEC_EV_CHG_DAY_SECTION_ADDR,          COLLECT_ELEC_EV_CHG_DAY_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_day           },//43
    {COLLECT_ELEC_EV_CHG_MONTH_SECTION_ADDR,        COLLECT_ELEC_EV_CHG_MONTH_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month         },//44
    {COLLECT_ELEC_EV_DSG_DAY_SECTION_ADDR,          COLLECT_ELEC_EV_DSG_DAY_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_day           },//45
    {COLLECT_ELEC_EV_DSG_MONTH_SECTION_ADDR,        COLLECT_ELEC_EV_DSG_MONTH_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month         },//46                                                                                          
    {COLLECT_ELEC_REVERSE_SECTION_ADDR,             COLLECT_ELEC_REVERSE_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.reserve3                      },//47                                                                                                                         
    {COLLECT_DEV_RUN_STATUS_SECTION_ADDR,           COLLECT_DEV_RUN_STATUS_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.dev_run_state                        },//48  
    {COLLECT_DEV_RUN_TIME_SECTION_ADDR,             COLLECT_DEV_RUN_TIME_SECTION_LEN,            EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.run_time                             },//49 
    {COLLECT_DEV_UPGRADE_STATUS_SECTION_ADDR,       COLLECT_DEV_UPGRADE_STATUS_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.upgrade_state                        },//50
    {COLLECT_NET_CONN_SECTION_ADDR,                 COLLECT_NET_CONN_SECTION_LEN,                EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.net_conn_state                       },//51                                                                                            
    {COLLECT_RTC_TIME_SECTION_ADDR,                 COLLECT_RTC_TIME_SECTION_LEN,                EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.time_stamp                           },//52  
    {COLLECT_MAX_CHG_POWER_SET_ADDR,                COLLECT_MAX_CHG_POWER_SET_NUM,               EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.max_chg_power_set                    },//53 
    {COLLECT_MAX_CHG_CURRENT_SET_ADDR,              COLLECT_MAX_CHG_CURRENT_SET_NUM,             EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.max_chg_curr_set                     },//54 
    {COLLECT_DEV_SPECIAL_MODE_ADDR,                 COLLECT_DEV_SPECIAL_MODE_NUM,                EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.special_mode                         },//55 
    {COLLECT_REST_FACTORY_ADDR,                     COLLECT_REST_FACTORY_NUM,                    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.factory_reset                        },//56 
    {COLLECT_REMOTE_SHUTDOWN_ADDR,                  COLLECT_REMOTE_SHUTDOWN_NUM,                 EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.remote_shutdown                      },//57 
    {COLLECT_SYS_REVERSE_2_SECTION_ADDR,            COLLECT_SYS_REVERSE_2_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.reserve3                             },//58                                                                                                                                                                                                 
    {COLLECT_MAINCARD_INSIDE_INFO_ADDR,             COLLECT_MAINCARD_INSIDE_INFO_NUM,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.inside_info                                   },//59 
    /********功率信息********/      
    {COLLECT_POWER_PCS_MODEL_ID_SECTION_ADDR,       COLLECT_POWER_PCS_MODEL_ID_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.model_id                           },//60
    {COLLECT_POWER_PCS_SW_VER_SECTION_ADDR,         COLLECT_POWER_PCS_SW_VER_SECTION_LEN,        EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.sw_ver                             },//61
    {COLLECT_POWER_PCS_SW_VER_EXT_SECTION_ADDR,     COLLECT_POWER_PCS_SW_VER_EXT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.sw_ver_ext                         },//62
    {COLLECT_POWER_ARC_MODEL_ID_SECTION_ADDR,       COLLECT_POWER_ARC_MODEL_ID_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.model_id                           },//63
    {COLLECT_POWER_ARC_SW_VER_SECTION_ADDR,         COLLECT_POWER_ARC_SW_VER_SECTION_LEN,        EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.sw_ver                             },//64
    {COLLECT_POWER_ARC_SW_VER_EXT_SECTION_ADDR,     COLLECT_POWER_ARC_SW_VER_EXT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.sw_ver_ext                         },//65
    {COLLECT_POWER_INTERFACE_SECTION_ADDR,          COLLECT_POWER_INTERFACE_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.model_info.interface_state                       },//66
    {COLLECT_POWER_R_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_R_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_grid_vol                      },//67
    {COLLECT_POWER_R_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_R_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_meter_act_power               },//68
    {COLLECT_POWER_S_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_S_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_grid_vol                      },//69
    {COLLECT_POWER_S_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_S_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_meter_act_power               },//70
    {COLLECT_POWER_T_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_T_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_grid_vol                      },//71
    {COLLECT_POWER_T_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_T_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_meter_act_power               },//72
    {COLLECT_POWER_R_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_R_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_on_grid_vol                   },//73
    {COLLECT_POWER_R_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_R_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_on_grid_act_power             },//74
    {COLLECT_POWER_S_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_S_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_on_grid_vol                   },//75
    {COLLECT_POWER_S_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_S_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_on_grid_act_power             },//76
    {COLLECT_POWER_T_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_T_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_on_grid_vol                   },//77
    {COLLECT_POWER_T_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_T_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_on_grid_act_power             },//78
    {COLLECT_POWER_R_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_R_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_gen_vol                       },//79
    {COLLECT_POWER_R_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_R_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_gen_act_power                 },//80
    {COLLECT_POWER_S_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_S_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_gen_vol                       },//81
    {COLLECT_POWER_S_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_S_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_gen_act_power                 },//82
    {COLLECT_POWER_T_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_T_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_gen_vol                       },//83
    {COLLECT_POWER_T_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_T_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_gen_act_power                 },//84
    {COLLECT_POWER_R_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_R_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_backup_vol                    },//85
    {COLLECT_POWER_R_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_R_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_backup_act_power              },//86
    {COLLECT_POWER_S_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_S_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_backup_vol                    },//87
    {COLLECT_POWER_S_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_S_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_backup_act_power              },//88
    {COLLECT_POWER_T_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_T_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_backup_vol                    },//89
    {COLLECT_POWER_T_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_T_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_backup_act_power              },//90
    {COLLECT_POWER_R_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_R_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_nload_vol                     },//91
    {COLLECT_POWER_R_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_R_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_nload_act_power               },//92
    {COLLECT_POWER_S_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_S_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_nload_vol                     },//93
    {COLLECT_POWER_S_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_S_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_nload_act_power               },//94
    {COLLECT_POWER_T_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_T_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_nload_vol                     },//95
    {COLLECT_POWER_T_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_T_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_nload_act_power               },//96
    {COLLECT_POWER_R_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_R_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_third_party_vol               },//97
    {COLLECT_POWER_R_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_R_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_third_party_act_power         },//98
    {COLLECT_POWER_S_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_S_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_third_party_vol               },//99
    {COLLECT_POWER_S_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_S_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_third_party_act_power         },//100
    {COLLECT_POWER_T_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_T_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_third_party_vol               },//101
    {COLLECT_POWER_T_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_T_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_third_party_act_power         },//102
    {COLLECT_POWER_R_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_R_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_inv_vol                       },//103
    {COLLECT_POWER_R_INV_POWER_SECTION_ADDR,        COLLECT_POWER_R_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_inv_act_power                 },//104
    {COLLECT_POWER_S_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_S_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_inv_vol                       },//105
    {COLLECT_POWER_S_INV_POWER_SECTION_ADDR,        COLLECT_POWER_S_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_inv_act_power                 },//106
    {COLLECT_POWER_T_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_T_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_inv_vol                       },//107
    {COLLECT_POWER_T_INV_POWER_SECTION_ADDR,        COLLECT_POWER_T_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_inv_act_power                 },//108
    {COLLECT_POWER_REVERSE_SECTION_ADDR,            COLLECT_POWER_REVERSE_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.reserve                         },//109
    {COLLECT_POWER_BAT_VOLT_SECTION_ADDR,           COLLECT_POWER_BAT_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_bat_vol                     },//110
    {COLLECT_POWER_BAT_POWER_SECTION_ADDR,          COLLECT_POWER_BAT_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_bat_power                   },//111
    {COLLECT_POWER_EV_VOLT_SECTION_ADDR,            COLLECT_POWER_EV_VOLT_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_ev_vol                      },//112
    {COLLECT_POWER_EV_POWER_SECTION_ADDR,           COLLECT_POWER_EV_POWER_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_ev_power                    },//113
    {COLLECT_POWER_PV1_VOLT_SECTION_ADDR,           COLLECT_POWER_PV1_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_1_vol                        },//114
    {COLLECT_POWER_PV1_POWER_SECTION_ADDR,          COLLECT_POWER_PV1_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_1_power                      },//115
    {COLLECT_POWER_PV2_VOLT_SECTION_ADDR,           COLLECT_POWER_PV2_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_2_vol                        },//116
    {COLLECT_POWER_PV2_POWER_SECTION_ADDR,          COLLECT_POWER_PV2_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_2_power                      },//117
    {COLLECT_POWER_PV3_VOLT_SECTION_ADDR,           COLLECT_POWER_PV3_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_3_vol                        },//118
    {COLLECT_POWER_PV3_POWER_SECTION_ADDR,          COLLECT_POWER_PV3_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_3_power                      },//119
    {COLLECT_POWER_PV4_VOLT_SECTION_ADDR,           COLLECT_POWER_PV4_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_4_vol                        },//120
    {COLLECT_POWER_PV4_POWER_SECTION_ADDR,          COLLECT_POWER_PV4_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_4_power                      },//121
    {COLLECT_POWER_REVERSE2_SECTION_ADDR,           COLLECT_POWER_REVERSE2_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.reserve2                        },//122
    {COLLECT_POWER_WORK_MODE_SECTION_ADDR,          COLLECT_POWER_WORK_MODE_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.work_mode                         },//123
    {COLLECT_POWER_BAT_MAX_CHG_POWER_SECTION_ADDR,  COLLECT_POWER_BAT_MAX_CHG_POWER_SECTION_LEN, EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_power             },//124
    {COLLECT_POWER_BAT_MAX_DSG_CURR_SECTION_ADDR,   COLLECT_POWER_BAT_MAX_DSG_CURR_SECTION_LEN,  EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_current           },//125
    {COLLECT_POWER_BAT_MAX_DSG_POWER_SECTION_ADDR,  COLLECT_POWER_BAT_MAX_DSG_POWER_SECTION_LEN, EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_power             },//126
    {COLLECT_POWER_SET_ERVERSE_SECTION_ADDR,        COLLECT_POWER_SET_ERVERSE_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.reserve                           },//127
    {COLLECT_POWER_BAT_UV_LIMIT_SECTION_ADDR,       COLLECT_POWER_BAT_UV_LIMIT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.limit_info.bat_under_vol_threshold               },//128
    {COLLECT_POWER_CALIBRATION_SECTION_ADDR,        COLLECT_POWER_CALIBRATION_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.calibrate_info.calibration_reset                 },//129
    {COLLECT_POWER_ARC_ENABLE_SECTION_ADDR,         COLLECT_POWER_ARC_ENABLE_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.arc_info.reserve                                 },//130
    {COLLECT_POWER_INSIDE_SECTION_ADDR,             COLLECT_POWER_INSIDE_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.inside_info                                      },//131      
     /********pack信息********/                                                                                                       
    {COLLECT_PACK_MODEL_ID_SECTION_ADDR,            COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.model_id                             },//132
    {COLLECT_PACK_SW_VER_SECTION_ADDR,              COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.sw_ver                               },//133
    {COLLECT_PACK_SW_VER_EXT_SECTION_ADDR,          COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.sw_ver_ext                           },//134
    {COLLECT_PACK_ADDR_SECTION_ADDR,                COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.addr                                },//135
    {COLLECT_PACK_CHG_ENERGY_SECTION_ADDR,          COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.accu_chg_energy                     },//136
    {COLLECT_PACK_HEAT_SECTION_ADDR,                COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.heat_status                         },//137 
    {COLLECT_PACK_BALANCE_STATUS_SECTION_ADDR,      COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.balance_status                      },//138
    {COLLECT_PACK_HEAT_CTRL_SECTION_ADDR,           COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].ctrl_info.heat_ctrl                            },//139
    {COLLECT_PACK_CELL_VOLT_SECTION_ADDR,           COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].cell_info.cell_vol                             },//140
    {COLLECT_PACK_INSIDE_SECTION_ADDR,              COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].inside_info                                    },//141
        
    {COLLECT_PACK_2_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.model_id                             },//142
    {COLLECT_PACK_2_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.sw_ver                               },//143
    {COLLECT_PACK_2_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.sw_ver_ext                           },//144
    {COLLECT_PACK_2_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.addr                                },//145
    {COLLECT_PACK_2_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.accu_chg_energy                     },//146
    {COLLECT_PACK_2_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.heat_status                         },//147
    {COLLECT_PACK_2_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.balance_status                      },//148
    {COLLECT_PACK_2_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].ctrl_info.heat_ctrl                            },//149
    {COLLECT_PACK_2_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].cell_info.cell_vol                             },//150
    {COLLECT_PACK_2_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].inside_info                                    },//151
            
    {COLLECT_PACK_3_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.model_id                             },//152
    {COLLECT_PACK_3_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.sw_ver                               },//153
    {COLLECT_PACK_3_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.sw_ver_ext                           },//154
    {COLLECT_PACK_3_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.addr                                },//155
    {COLLECT_PACK_3_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.accu_chg_energy                     },//156
    {COLLECT_PACK_3_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.heat_status                         },//157
    {COLLECT_PACK_3_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.balance_status                      },//158
    {COLLECT_PACK_3_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].ctrl_info.heat_ctrl                            },//159
    {COLLECT_PACK_3_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].cell_info.cell_vol                             },//160
    {COLLECT_PACK_3_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].inside_info                                    },//161
            
    {COLLECT_PACK_4_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.model_id                             },//162
    {COLLECT_PACK_4_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.sw_ver                               },//163
    {COLLECT_PACK_4_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.sw_ver_ext                           },//164
    {COLLECT_PACK_4_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.addr                                },//165
    {COLLECT_PACK_4_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.accu_chg_energy                     },//166
    {COLLECT_PACK_4_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.heat_status                         },//167
    {COLLECT_PACK_4_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.balance_status                      },//168
    {COLLECT_PACK_4_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].ctrl_info.heat_ctrl                            },//169
    {COLLECT_PACK_4_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].cell_info.cell_vol                             },//170
    {COLLECT_PACK_4_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].inside_info                                    },//171
    
    {COLLECT_PACK_5_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.model_id                             },//172
    {COLLECT_PACK_5_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.sw_ver                               },//173
    {COLLECT_PACK_5_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.sw_ver_ext                           },//174
    {COLLECT_PACK_5_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.addr                                },//175
    {COLLECT_PACK_5_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.accu_chg_energy                     },//176
    {COLLECT_PACK_5_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.heat_status                         },//177
    {COLLECT_PACK_5_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.balance_status                      },//178
    {COLLECT_PACK_5_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].ctrl_info.heat_ctrl                            },//179
    {COLLECT_PACK_5_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].cell_info.cell_vol                             },//180
    {COLLECT_PACK_5_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].inside_info                                    },//181
                                                                                                                                                                                                                    
    {COLLECT_PACK_6_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.model_id                             },//182
    {COLLECT_PACK_6_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.sw_ver                               },//183
    {COLLECT_PACK_6_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.sw_ver_ext                           },//184
    {COLLECT_PACK_6_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.addr                                },//185
    {COLLECT_PACK_6_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.accu_chg_energy                     },//186
    {COLLECT_PACK_6_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.heat_status                         },//187
    {COLLECT_PACK_6_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.balance_status                      },//188
    {COLLECT_PACK_6_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].ctrl_info.heat_ctrl                            },//189
    {COLLECT_PACK_6_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].cell_info.cell_vol                             },//190
    {COLLECT_PACK_6_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].inside_info                                    },//191  

    {COLLECT_COP1_BASE_INFO_ADDR,                   COLLECT_COP1_BASE_INFO_NUM,                  EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.cop_data.cop_device1_info.cop_cpu_modelid                   },//192
    {COLLECT_COP2_BASE_INFO_ADDR,                   COLLECT_COP2_BASE_INFO_NUM,                  EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.cop_data.cop_device2_info.cop_cpu_modelid                   },//193
};                                                                                                                                                                                          
#define MORMAL_USER_NUM     (sizeof(NomalUserDataPoint)/sizeof(ModbusDataPoint_t))                                                                                                          


static const ModbusDataPoint_t ManagerUserDataPoint[] =                                                                                                                                     
{                                                                                                                                                                                           
    {COLLECT_MODE_ID,                               MODE_ID_NUM,                                 EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.model_id                            },//0
    {COLLECT_SN_ADDR,                               COLLECT_SN_NUM,                              EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.sn[0]                               },//1
    {COLLECT_HW_VER_ADDR,                           COLLECT_HW_VER_NUM,                          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.hw_ver                              },//2
    {COLLECT_SW_VER_ADDR,                           COLLECT_SW_VER_NUM,                          EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.sw_ver                              },//3
    {COLLECT_SW_VER_EXTEND_ADDR,                    COLLECT_SW_VER_EXTEND_NUM,                   EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.sw_ver_ext                          },//4
    {COLLECT_USER_ADDR,                             COLLECT_USER_NUM,                            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.user_type                           },//5
    {COLLECT_NORMAL_PASSWORD_ADDR,                  COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.normal_password[0]                  },//6
    {COLLECT_MANAGER_PASSWORD_ADDR,                 COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.manager_password[0]                 },//7
    {COLLECT_INSTALLER_PASSWORD_ADDR,               COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.installer_password[0]               },//8
    {COLLECT_INTERNAL_PASSWORD_ADDR,                COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.internal_password[0]                },//9
    {COLLECT_SLAVE_ADDR_ADDR,                       COLLECT_SALVE_ADDR_NUM,                      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.modbus_addr                         },//10
    {COLLECT_BAUD_ADDR,                             COLLECT_BAUD_NUM,                            EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.baudrate                            },//11
    {COLLECT_RESERVED_1_ADDR,                       COLLECT_RESERVED_1_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.base_info.reserve[0]                          },//12
    {COLLECT_DEV_TYPE_ADDR,                         COLLECT_DEV_TYPE_NUM,                        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.dev_type                          },//13                                                                                                                                                                                                              
    {COLLECT_LCD_BL_TIME_ADDR,                      COLLECT_LCD_BL_TIME_NUM,                     EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_bl_time                       },//14
    {COLLECT_LCD_BL_BRINGHTNESS_ADDR,               COLLECT_LCD_BL_BRINGHTNESS_NUM,              EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_bl_brightness                 },//15
    {COLLECT_LCD_LANGUEAGE_ADDR,                    COLLECT_LCD_LANGUEAGE_NUM,                   EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_languege                      },//16
    {COLLECT_LED_FRONT_SWITCH_AND_RESE_ADDR,        COLLECT_LED_FRONT_SWITCH_AND_RESE_NUM,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.reserve                           },//17  //5034预留 5035前面板控制 5036预留
    {COLLECT_LED_AUX_MODE_ADDR,                     COLLECT_LED_AUX_MODE_NUM,                    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.led_aux_mode                      },//18
    {COLLECT_RESERVED_2_ADDR,                       COLLECT_RESERVED_2_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.device_info.reserve3[0]                       },//19
    {COLLECT_LOAD_PEAK_TIME_ADDR,                   COLLECT_PEAK_LOAD_TIME_NUM,                  EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.peak_time_info.chg_start_time1                },//20
    {COLLECT_RESERVED_3_ADDR,                       COLLECT_RESERVED_3_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.peak_time_info.reserve[0]                     },//21
    {COLLECT_RESERVED_4_ADDR,                       COLLECT_RESERVED_4_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.total_pack_info.reserve[0]                    },//22
    {COLLECT_PACK_NUM_SECTION_ADDR,                 COLLECT_PACK_NUM_SECTION_LEN,                EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.pack_num                      },//23
    {COLLECT_BMS_ERROR_SECTION_ADDR,                COLLECT_BMS_ERROR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.bms_error                     },//24
    {COLLECT_BMS_REVERSE_SECTION_ADDR,              COLLECT_BMS_REVERSE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.reserve2                      },//25
    {COLLECT_BMS_DSG_MIN_VOLT_SECTION_ADDR,         COLLECT_BMS_DSG_MIN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.total_pack_info.dsg_min_vol                   },//26                                                                                          
    {COLLECT_ELEC_PV_DAY_SECTION_ADDR,              COLLECT_ELEC_PV_DAY_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day               },//27
    {COLLECT_ELEC_PV_MONTH_SECTION_ADDR,            COLLECT_ELEC_PV_MONTH_SECTION_LEN,           EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month             },//28
    {COLLECT_ELEC_BUY_DAY_SECTION_ADDR,             COLLECT_ELEC_BUY_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day              },//29
    {COLLECT_ELEC_BUY_MONTH_SECTION_ADDR,           COLLECT_ELEC_BUY_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month            },//30
    {COLLECT_ELEC_SELL_DAY_SECTION_ADDR,            COLLECT_ELEC_SELL_DAY_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day             },//31
    {COLLECT_ELEC_SELL_MONTH_SECTION_ADDR,          COLLECT_ELEC_SELL_MONTH_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month           },//32
    {COLLECT_ELEC_BAT_CHG_DAY_SECTION_ADDR,         COLLECT_ELEC_BAT_CHG_DAY_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_day          },//33
    {COLLECT_ELEC_BAT_CHG_MONTH_SECTION_ADDR,       COLLECT_ELEC_BAT_CHG_MONTH_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month        },//34
    {COLLECT_ELEC_BAT_DSG_DAY_SECTION_ADDR,         COLLECT_ELEC_BAT_DSG_DAY_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_day          },//35
    {COLLECT_ELEC_BAT_DSG_MONTH_SECTION_ADDR,       COLLECT_ELEC_BAT_DSG_MONTH_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month        },//36
    {COLLECT_ELEC_EPS_DAY_SECTION_ADDR,             COLLECT_ELEC_EPS_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day              },//37
    {COLLECT_ELEC_EPS_MONTH_SECTION_ADDR,           COLLECT_ELEC_EPS_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month            },//38
    {COLLECT_ELEC_REMOTE_LOAD_DAY_SECTION_ADDR,     COLLECT_ELEC_REMOTE_LOAD_DAY_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day      },//39
    {COLLECT_ELEC_REMOTE_LOAD_MONTH_SECTION_ADDR,   COLLECT_ELEC_REMOTE_LOAD_MONTH_SECTION_LEN,  EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month    },//40
    {COLLECT_ELEC_GEN_DAY_SECTION_ADDR,             COLLECT_ELEC_GEN_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_day              },//41
    {COLLECT_ELEC_GEN_MONTH_SECTION_ADDR,           COLLECT_ELEC_GEN_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month            },//42
    {COLLECT_ELEC_EV_CHG_DAY_SECTION_ADDR,          COLLECT_ELEC_EV_CHG_DAY_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_day           },//43
    {COLLECT_ELEC_EV_CHG_MONTH_SECTION_ADDR,        COLLECT_ELEC_EV_CHG_MONTH_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month         },//44
    {COLLECT_ELEC_EV_DSG_DAY_SECTION_ADDR,          COLLECT_ELEC_EV_DSG_DAY_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_day           },//45
    {COLLECT_ELEC_EV_DSG_MONTH_SECTION_ADDR,        COLLECT_ELEC_EV_DSG_MONTH_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month         },//46                                                                                              
    {COLLECT_ELEC_REVERSE_SECTION_ADDR,             COLLECT_ELEC_REVERSE_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.reserve3                      },//47                                                                                                                             
    {COLLECT_DEV_RUN_STATUS_SECTION_ADDR,           COLLECT_DEV_RUN_STATUS_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.dev_run_state                        },//48  
    {COLLECT_DEV_RUN_TIME_SECTION_ADDR,             COLLECT_DEV_RUN_TIME_SECTION_LEN,            EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.run_time                             },//49 
    {COLLECT_DEV_UPGRADE_STATUS_SECTION_ADDR,       COLLECT_DEV_UPGRADE_STATUS_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.upgrade_state                        },//50
    {COLLECT_NET_CONN_SECTION_ADDR,                 COLLECT_NET_CONN_SECTION_LEN,                EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.net_conn_state                       },//51                                                                                                
    {COLLECT_RTC_TIME_SECTION_ADDR,                 COLLECT_RTC_TIME_SECTION_LEN,                EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.time_stamp                           },//52  
    {COLLECT_MAX_CHG_POWER_SET_ADDR,                COLLECT_MAX_CHG_POWER_SET_NUM,               EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.max_chg_power_set                    },//53 
    {COLLECT_MAX_CHG_CURRENT_SET_ADDR,              COLLECT_MAX_CHG_CURRENT_SET_NUM,             EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.max_chg_curr_set                     },//54 
    {COLLECT_DEV_SPECIAL_MODE_ADDR,                 COLLECT_DEV_SPECIAL_MODE_NUM,                EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.special_mode                         },//55 
    {COLLECT_REST_FACTORY_ADDR,                     COLLECT_REST_FACTORY_NUM,                    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.factory_reset                        },//56 
    {COLLECT_REMOTE_SHUTDOWN_ADDR,                  COLLECT_REMOTE_SHUTDOWN_NUM,                 EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.remote_shutdown                      },//57 
    {COLLECT_SYS_REVERSE_2_SECTION_ADDR,            COLLECT_SYS_REVERSE_2_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.reserve3                             },//58                                                                                                                                                                                                                
    {COLLECT_MAINCARD_INSIDE_INFO_ADDR,             COLLECT_MAINCARD_INSIDE_INFO_NUM,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.inside_info                                   },//59 
    /********功率信息********/      
    {COLLECT_POWER_PCS_MODEL_ID_SECTION_ADDR,       COLLECT_POWER_PCS_MODEL_ID_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.model_id                           },//60
    {COLLECT_POWER_PCS_SW_VER_SECTION_ADDR,         COLLECT_POWER_PCS_SW_VER_SECTION_LEN,        EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.sw_ver                             },//61
    {COLLECT_POWER_PCS_SW_VER_EXT_SECTION_ADDR,     COLLECT_POWER_PCS_SW_VER_EXT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.sw_ver_ext                         },//62
    {COLLECT_POWER_ARC_MODEL_ID_SECTION_ADDR,       COLLECT_POWER_ARC_MODEL_ID_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.model_id                           },//63
    {COLLECT_POWER_ARC_SW_VER_SECTION_ADDR,         COLLECT_POWER_ARC_SW_VER_SECTION_LEN,        EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.sw_ver                             },//64
    {COLLECT_POWER_ARC_SW_VER_EXT_SECTION_ADDR,     COLLECT_POWER_ARC_SW_VER_EXT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.sw_ver_ext                         },//65
    {COLLECT_POWER_INTERFACE_SECTION_ADDR,          COLLECT_POWER_INTERFACE_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.model_info.interface_state                       },//66
    {COLLECT_POWER_R_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_R_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_grid_vol                      },//67
    {COLLECT_POWER_R_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_R_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_meter_act_power               },//68
    {COLLECT_POWER_S_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_S_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_grid_vol                      },//69
    {COLLECT_POWER_S_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_S_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_meter_act_power               },//70
    {COLLECT_POWER_T_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_T_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_grid_vol                      },//71
    {COLLECT_POWER_T_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_T_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_meter_act_power               },//72
    {COLLECT_POWER_R_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_R_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_on_grid_vol                   },//73
    {COLLECT_POWER_R_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_R_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_on_grid_act_power             },//74
    {COLLECT_POWER_S_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_S_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_on_grid_vol                   },//75
    {COLLECT_POWER_S_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_S_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_on_grid_act_power             },//76
    {COLLECT_POWER_T_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_T_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_on_grid_vol                   },//77
    {COLLECT_POWER_T_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_T_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_on_grid_act_power             },//78
    {COLLECT_POWER_R_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_R_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_gen_vol                       },//79
    {COLLECT_POWER_R_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_R_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_gen_act_power                 },//80
    {COLLECT_POWER_S_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_S_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_gen_vol                       },//81
    {COLLECT_POWER_S_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_S_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_gen_act_power                 },//82
    {COLLECT_POWER_T_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_T_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_gen_vol                       },//83
    {COLLECT_POWER_T_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_T_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_gen_act_power                 },//84
    {COLLECT_POWER_R_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_R_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_backup_vol                    },//85
    {COLLECT_POWER_R_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_R_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_backup_act_power              },//86
    {COLLECT_POWER_S_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_S_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_backup_vol                    },//87
    {COLLECT_POWER_S_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_S_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_backup_act_power              },//88
    {COLLECT_POWER_T_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_T_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_backup_vol                    },//89
    {COLLECT_POWER_T_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_T_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_backup_act_power              },//90
    {COLLECT_POWER_R_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_R_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_nload_vol                     },//91
    {COLLECT_POWER_R_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_R_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_nload_act_power               },//92
    {COLLECT_POWER_S_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_S_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_nload_vol                     },//93
    {COLLECT_POWER_S_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_S_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_nload_act_power               },//94
    {COLLECT_POWER_T_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_T_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_nload_vol                     },//95
    {COLLECT_POWER_T_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_T_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_nload_act_power               },//96
    {COLLECT_POWER_R_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_R_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_third_party_vol               },//97
    {COLLECT_POWER_R_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_R_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_third_party_act_power         },//98
    {COLLECT_POWER_S_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_S_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_third_party_vol               },//99
    {COLLECT_POWER_S_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_S_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_third_party_act_power         },//100
    {COLLECT_POWER_T_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_T_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_third_party_vol               },//101
    {COLLECT_POWER_T_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_T_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_third_party_act_power         },//102
    {COLLECT_POWER_R_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_R_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_inv_vol                       },//103
    {COLLECT_POWER_R_INV_POWER_SECTION_ADDR,        COLLECT_POWER_R_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_inv_act_power                 },//104
    {COLLECT_POWER_S_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_S_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_inv_vol                       },//105
    {COLLECT_POWER_S_INV_POWER_SECTION_ADDR,        COLLECT_POWER_S_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_inv_act_power                 },//106
    {COLLECT_POWER_T_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_T_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_inv_vol                       },//107
    {COLLECT_POWER_T_INV_POWER_SECTION_ADDR,        COLLECT_POWER_T_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_inv_act_power                 },//108
    {COLLECT_POWER_REVERSE_SECTION_ADDR,            COLLECT_POWER_REVERSE_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.reserve                         },//109
    {COLLECT_POWER_BAT_VOLT_SECTION_ADDR,           COLLECT_POWER_BAT_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_bat_vol                     },//110
    {COLLECT_POWER_BAT_POWER_SECTION_ADDR,          COLLECT_POWER_BAT_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_bat_power                   },//111
    {COLLECT_POWER_EV_VOLT_SECTION_ADDR,            COLLECT_POWER_EV_VOLT_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_ev_vol                      },//112
    {COLLECT_POWER_EV_POWER_SECTION_ADDR,           COLLECT_POWER_EV_POWER_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_ev_power                    },//113
    {COLLECT_POWER_PV1_VOLT_SECTION_ADDR,           COLLECT_POWER_PV1_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_1_vol                        },//114
    {COLLECT_POWER_PV1_POWER_SECTION_ADDR,          COLLECT_POWER_PV1_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_1_power                      },//115
    {COLLECT_POWER_PV2_VOLT_SECTION_ADDR,           COLLECT_POWER_PV2_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_2_vol                        },//116
    {COLLECT_POWER_PV2_POWER_SECTION_ADDR,          COLLECT_POWER_PV2_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_2_power                      },//117
    {COLLECT_POWER_PV3_VOLT_SECTION_ADDR,           COLLECT_POWER_PV3_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_3_vol                        },//118
    {COLLECT_POWER_PV3_POWER_SECTION_ADDR,          COLLECT_POWER_PV3_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_3_power                      },//119
    {COLLECT_POWER_PV4_VOLT_SECTION_ADDR,           COLLECT_POWER_PV4_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_4_vol                        },//120
    {COLLECT_POWER_PV4_POWER_SECTION_ADDR,          COLLECT_POWER_PV4_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_4_power                      },//121
    {COLLECT_POWER_REVERSE2_SECTION_ADDR,           COLLECT_POWER_REVERSE2_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.reserve2                        },//122
    {COLLECT_POWER_WORK_MODE_SECTION_ADDR,          COLLECT_POWER_WORK_MODE_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.work_mode                         },//123
    {COLLECT_POWER_BAT_MAX_CHG_POWER_SECTION_ADDR,  COLLECT_POWER_BAT_MAX_CHG_POWER_SECTION_LEN, EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_power             },//124
    {COLLECT_POWER_BAT_MAX_DSG_CURR_SECTION_ADDR,   COLLECT_POWER_BAT_MAX_DSG_CURR_SECTION_LEN,  EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_current           },//125
    {COLLECT_POWER_BAT_MAX_DSG_POWER_SECTION_ADDR,  COLLECT_POWER_BAT_MAX_DSG_POWER_SECTION_LEN, EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_power             },//126
    {COLLECT_POWER_SET_ERVERSE_SECTION_ADDR,        COLLECT_POWER_SET_ERVERSE_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.reserve                           },//127
    {COLLECT_POWER_BAT_UV_LIMIT_SECTION_ADDR,       COLLECT_POWER_BAT_UV_LIMIT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.limit_info.bat_under_vol_threshold               },//128
    {COLLECT_POWER_CALIBRATION_SECTION_ADDR,        COLLECT_POWER_CALIBRATION_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.calibrate_info.calibration_reset                 },//129
    {COLLECT_POWER_ARC_ENABLE_SECTION_ADDR,         COLLECT_POWER_ARC_ENABLE_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.arc_info.reserve                                 },//130
    {COLLECT_POWER_INSIDE_SECTION_ADDR,             COLLECT_POWER_INSIDE_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.inside_info                                      },//131     
     /********pack信息********/                                                                                                       
    {COLLECT_PACK_MODEL_ID_SECTION_ADDR,            COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.model_id                             },//132
    {COLLECT_PACK_SW_VER_SECTION_ADDR,              COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.sw_ver                               },//133
    {COLLECT_PACK_SW_VER_EXT_SECTION_ADDR,          COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.sw_ver_ext                           },//134
    {COLLECT_PACK_ADDR_SECTION_ADDR,                COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.addr                                },//135
    {COLLECT_PACK_CHG_ENERGY_SECTION_ADDR,          COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.accu_chg_energy                     },//136
    {COLLECT_PACK_HEAT_SECTION_ADDR,                COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.heat_status                         },//137 
    {COLLECT_PACK_BALANCE_STATUS_SECTION_ADDR,      COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.balance_status                      },//138
    {COLLECT_PACK_HEAT_CTRL_SECTION_ADDR,           COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].ctrl_info.heat_ctrl                            },//139
    {COLLECT_PACK_CELL_VOLT_SECTION_ADDR,           COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].cell_info.cell_vol                             },//140
    {COLLECT_PACK_INSIDE_SECTION_ADDR,              COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].inside_info                                    },//141
                                                                                                                                                                                                                 
    {COLLECT_PACK_2_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.model_id                             },//142
    {COLLECT_PACK_2_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.sw_ver                               },//143
    {COLLECT_PACK_2_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.sw_ver_ext                           },//144
    {COLLECT_PACK_2_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.addr                                },//145
    {COLLECT_PACK_2_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.accu_chg_energy                     },//146
    {COLLECT_PACK_2_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.heat_status                         },//147
    {COLLECT_PACK_2_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.balance_status                      },//148
    {COLLECT_PACK_2_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].ctrl_info.heat_ctrl                            },//149
    {COLLECT_PACK_2_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].cell_info.cell_vol                             },//150
    {COLLECT_PACK_2_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].inside_info                                    },//151
                                                                                                                                                                                                                 
    {COLLECT_PACK_3_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.model_id                             },//152
    {COLLECT_PACK_3_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.sw_ver                               },//153
    {COLLECT_PACK_3_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.sw_ver_ext                           },//154
    {COLLECT_PACK_3_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.addr                                },//155
    {COLLECT_PACK_3_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.accu_chg_energy                     },//156
    {COLLECT_PACK_3_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.heat_status                         },//157
    {COLLECT_PACK_3_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.balance_status                      },//158
    {COLLECT_PACK_3_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].ctrl_info.heat_ctrl                            },//159
    {COLLECT_PACK_3_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].cell_info.cell_vol                             },//160
    {COLLECT_PACK_3_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].inside_info                                    },//161
                                                                                                                                                                                                                 
    {COLLECT_PACK_4_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.model_id                             },//162
    {COLLECT_PACK_4_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.sw_ver                               },//163
    {COLLECT_PACK_4_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.sw_ver_ext                           },//164
    {COLLECT_PACK_4_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.addr                                },//165
    {COLLECT_PACK_4_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.accu_chg_energy                     },//166
    {COLLECT_PACK_4_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.heat_status                         },//167
    {COLLECT_PACK_4_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.balance_status                      },//168
    {COLLECT_PACK_4_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].ctrl_info.heat_ctrl                            },//169
    {COLLECT_PACK_4_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].cell_info.cell_vol                             },//170
    {COLLECT_PACK_4_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].inside_info                                    },//171
                                                                                                                                                                                                                 
    {COLLECT_PACK_5_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.model_id                             },//172
    {COLLECT_PACK_5_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.sw_ver                               },//173
    {COLLECT_PACK_5_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.sw_ver_ext                           },//174
    {COLLECT_PACK_5_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.addr                                },//175
    {COLLECT_PACK_5_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.accu_chg_energy                     },//176
    {COLLECT_PACK_5_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.heat_status                         },//177
    {COLLECT_PACK_5_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.balance_status                      },//178
    {COLLECT_PACK_5_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].ctrl_info.heat_ctrl                            },//179
    {COLLECT_PACK_5_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].cell_info.cell_vol                             },//180
    {COLLECT_PACK_5_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].inside_info                                    },//181
                                                                                                                                                                                                                    
    {COLLECT_PACK_6_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.model_id                             },//182
    {COLLECT_PACK_6_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.sw_ver                               },//183
    {COLLECT_PACK_6_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.sw_ver_ext                           },//184
    {COLLECT_PACK_6_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.addr                                },//185
    {COLLECT_PACK_6_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.accu_chg_energy                     },//186
    {COLLECT_PACK_6_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.heat_status                         },//187
    {COLLECT_PACK_6_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.balance_status                      },//188
    {COLLECT_PACK_6_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].ctrl_info.heat_ctrl                            },//189
    {COLLECT_PACK_6_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].cell_info.cell_vol                             },//190
    {COLLECT_PACK_6_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].inside_info                                    },//191  
                                                                                                                                                                                                                 
    {COLLECT_COP1_BASE_INFO_ADDR,                   COLLECT_COP1_BASE_INFO_NUM,                  EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.cop_data.cop_device1_info.cop_cpu_modelid                   },//192
    {COLLECT_COP2_BASE_INFO_ADDR,                   COLLECT_COP2_BASE_INFO_NUM,                  EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.cop_data.cop_device2_info.cop_cpu_modelid                   },//193
}; 
#define MANAGER_USER_NUM     (sizeof(ManagerUserDataPoint)/sizeof(ModbusDataPoint_t))                                                                                                                                                                                                                                                                                                                                                                                      

static const ModbusDataPoint_t InstallerUserDataPoint[] =                                                                                                                                   
{                                                                                                                                                                                           
    {COLLECT_MODE_ID,                               MODE_ID_NUM,                                 EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.model_id                            },//0
    {COLLECT_SN_ADDR,                               COLLECT_SN_NUM,                              EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.sn[0]                               },//1
    {COLLECT_HW_VER_ADDR,                           COLLECT_HW_VER_NUM,                          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.hw_ver                              },//2
    {COLLECT_SW_VER_ADDR,                           COLLECT_SW_VER_NUM,                          EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.sw_ver                              },//3
    {COLLECT_SW_VER_EXTEND_ADDR,                    COLLECT_SW_VER_EXTEND_NUM,                   EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.sw_ver_ext                          },//4
    {COLLECT_USER_ADDR,                             COLLECT_USER_NUM,                            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.user_type                           },//5
    {COLLECT_NORMAL_PASSWORD_ADDR,                  COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.normal_password[0]                  },//6
    {COLLECT_MANAGER_PASSWORD_ADDR,                 COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.manager_password[0]                 },//7
    {COLLECT_INSTALLER_PASSWORD_ADDR,               COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.installer_password[0]               },//8
    {COLLECT_INTERNAL_PASSWORD_ADDR,                COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.internal_password[0]                },//9
    {COLLECT_SLAVE_ADDR_ADDR,                       COLLECT_SALVE_ADDR_NUM,                      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.modbus_addr                         },//10
    {COLLECT_BAUD_ADDR,                             COLLECT_BAUD_NUM,                            EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.baudrate                            },//11
    {COLLECT_RESERVED_1_ADDR,                       COLLECT_RESERVED_1_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.base_info.reserve[0]                          },//12
    {COLLECT_DEV_TYPE_ADDR,                         COLLECT_DEV_TYPE_NUM,                        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.dev_type                          },//13                                                                                                                                                                                              
    {COLLECT_LCD_BL_TIME_ADDR,                      COLLECT_LCD_BL_TIME_NUM,                     EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_bl_time                       },//14
    {COLLECT_LCD_BL_BRINGHTNESS_ADDR,               COLLECT_LCD_BL_BRINGHTNESS_NUM,              EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_bl_brightness                 },//15
    {COLLECT_LCD_LANGUEAGE_ADDR,                    COLLECT_LCD_LANGUEAGE_NUM,                   EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_languege                      },//16
    {COLLECT_LED_FRONT_SWITCH_AND_RESE_ADDR,        COLLECT_LED_FRONT_SWITCH_AND_RESE_NUM,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.reserve                           },//17  //5034预留 5035前面板控制 5036预留
    {COLLECT_LED_AUX_MODE_ADDR,                     COLLECT_LED_AUX_MODE_NUM,                    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.led_aux_mode                      },//18
    {COLLECT_RESERVED_2_ADDR,                       COLLECT_RESERVED_2_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.device_info.reserve3[0]                       },//19
    {COLLECT_LOAD_PEAK_TIME_ADDR,                   COLLECT_PEAK_LOAD_TIME_NUM,                  EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.peak_time_info.chg_start_time1                },//20
    {COLLECT_RESERVED_3_ADDR,                       COLLECT_RESERVED_3_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.peak_time_info.reserve[0]                     },//21
    {COLLECT_RESERVED_4_ADDR,                       COLLECT_RESERVED_4_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.total_pack_info.reserve[0]                    },//22
    {COLLECT_PACK_NUM_SECTION_ADDR,                 COLLECT_PACK_NUM_SECTION_LEN,                EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.pack_num                      },//23
    {COLLECT_BMS_ERROR_SECTION_ADDR,                COLLECT_BMS_ERROR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.bms_error                     },//24
    {COLLECT_BMS_REVERSE_SECTION_ADDR,              COLLECT_BMS_REVERSE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.reserve2                      },//25
    {COLLECT_BMS_DSG_MIN_VOLT_SECTION_ADDR,         COLLECT_BMS_DSG_MIN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.total_pack_info.dsg_min_vol                   },//26                                                                          
    {COLLECT_ELEC_PV_DAY_SECTION_ADDR,              COLLECT_ELEC_PV_DAY_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day               },//27
    {COLLECT_ELEC_PV_MONTH_SECTION_ADDR,            COLLECT_ELEC_PV_MONTH_SECTION_LEN,           EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month             },//28
    {COLLECT_ELEC_BUY_DAY_SECTION_ADDR,             COLLECT_ELEC_BUY_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day              },//29
    {COLLECT_ELEC_BUY_MONTH_SECTION_ADDR,           COLLECT_ELEC_BUY_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month            },//30
    {COLLECT_ELEC_SELL_DAY_SECTION_ADDR,            COLLECT_ELEC_SELL_DAY_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day             },//31
    {COLLECT_ELEC_SELL_MONTH_SECTION_ADDR,          COLLECT_ELEC_SELL_MONTH_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month           },//32
    {COLLECT_ELEC_BAT_CHG_DAY_SECTION_ADDR,         COLLECT_ELEC_BAT_CHG_DAY_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_day          },//33
    {COLLECT_ELEC_BAT_CHG_MONTH_SECTION_ADDR,       COLLECT_ELEC_BAT_CHG_MONTH_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month        },//34
    {COLLECT_ELEC_BAT_DSG_DAY_SECTION_ADDR,         COLLECT_ELEC_BAT_DSG_DAY_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_day          },//35
    {COLLECT_ELEC_BAT_DSG_MONTH_SECTION_ADDR,       COLLECT_ELEC_BAT_DSG_MONTH_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month        },//36
    {COLLECT_ELEC_EPS_DAY_SECTION_ADDR,             COLLECT_ELEC_EPS_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day              },//37
    {COLLECT_ELEC_EPS_MONTH_SECTION_ADDR,           COLLECT_ELEC_EPS_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month            },//38
    {COLLECT_ELEC_REMOTE_LOAD_DAY_SECTION_ADDR,     COLLECT_ELEC_REMOTE_LOAD_DAY_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day      },//39
    {COLLECT_ELEC_REMOTE_LOAD_MONTH_SECTION_ADDR,   COLLECT_ELEC_REMOTE_LOAD_MONTH_SECTION_LEN,  EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month    },//40
    {COLLECT_ELEC_GEN_DAY_SECTION_ADDR,             COLLECT_ELEC_GEN_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_day              },//41
    {COLLECT_ELEC_GEN_MONTH_SECTION_ADDR,           COLLECT_ELEC_GEN_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month            },//42
    {COLLECT_ELEC_EV_CHG_DAY_SECTION_ADDR,          COLLECT_ELEC_EV_CHG_DAY_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_day           },//43
    {COLLECT_ELEC_EV_CHG_MONTH_SECTION_ADDR,        COLLECT_ELEC_EV_CHG_MONTH_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month         },//44
    {COLLECT_ELEC_EV_DSG_DAY_SECTION_ADDR,          COLLECT_ELEC_EV_DSG_DAY_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_day           },//45
    {COLLECT_ELEC_EV_DSG_MONTH_SECTION_ADDR,        COLLECT_ELEC_EV_DSG_MONTH_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month         },//46                                                                              
    {COLLECT_ELEC_REVERSE_SECTION_ADDR,             COLLECT_ELEC_REVERSE_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.reserve3                      },//47                                                                                                             
    {COLLECT_DEV_RUN_STATUS_SECTION_ADDR,           COLLECT_DEV_RUN_STATUS_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.dev_run_state                        },//48  
    {COLLECT_DEV_RUN_TIME_SECTION_ADDR,             COLLECT_DEV_RUN_TIME_SECTION_LEN,            EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.run_time                             },//49 
    {COLLECT_DEV_UPGRADE_STATUS_SECTION_ADDR,       COLLECT_DEV_UPGRADE_STATUS_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.upgrade_state                        },//50
    {COLLECT_NET_CONN_SECTION_ADDR,                 COLLECT_NET_CONN_SECTION_LEN,                EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.net_conn_state                       },//51                                                                                
    {COLLECT_RTC_TIME_SECTION_ADDR,                 COLLECT_RTC_TIME_SECTION_LEN,                EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.time_stamp                           },//52  
    {COLLECT_MAX_CHG_POWER_SET_ADDR,                COLLECT_MAX_CHG_POWER_SET_NUM,               EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.max_chg_power_set                    },//53 
    {COLLECT_MAX_CHG_CURRENT_SET_ADDR,              COLLECT_MAX_CHG_CURRENT_SET_NUM,             EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.max_chg_curr_set                     },//54 
    {COLLECT_DEV_SPECIAL_MODE_ADDR,                 COLLECT_DEV_SPECIAL_MODE_NUM,                EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.special_mode                         },//55 
    {COLLECT_REST_FACTORY_ADDR,                     COLLECT_REST_FACTORY_NUM,                    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.factory_reset                        },//56 
    {COLLECT_REMOTE_SHUTDOWN_ADDR,                  COLLECT_REMOTE_SHUTDOWN_NUM,                 EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.remote_shutdown                      },//57 
    {COLLECT_SYS_REVERSE_2_SECTION_ADDR,            COLLECT_SYS_REVERSE_2_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.reserve3                             },//58                                                                                                                                                                                                     
    {COLLECT_MAINCARD_INSIDE_INFO_ADDR,             COLLECT_MAINCARD_INSIDE_INFO_NUM,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.inside_info                                   },//59 
    /********功率信息********/      
    {COLLECT_POWER_PCS_MODEL_ID_SECTION_ADDR,       COLLECT_POWER_PCS_MODEL_ID_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.model_id                           },//60
    {COLLECT_POWER_PCS_SW_VER_SECTION_ADDR,         COLLECT_POWER_PCS_SW_VER_SECTION_LEN,        EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.sw_ver                             },//61
    {COLLECT_POWER_PCS_SW_VER_EXT_SECTION_ADDR,     COLLECT_POWER_PCS_SW_VER_EXT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.sw_ver_ext                         },//62
    {COLLECT_POWER_ARC_MODEL_ID_SECTION_ADDR,       COLLECT_POWER_ARC_MODEL_ID_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.model_id                           },//63
    {COLLECT_POWER_ARC_SW_VER_SECTION_ADDR,         COLLECT_POWER_ARC_SW_VER_SECTION_LEN,        EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.sw_ver                             },//64
    {COLLECT_POWER_ARC_SW_VER_EXT_SECTION_ADDR,     COLLECT_POWER_ARC_SW_VER_EXT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.sw_ver_ext                         },//65
    {COLLECT_POWER_INTERFACE_SECTION_ADDR,          COLLECT_POWER_INTERFACE_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.model_info.interface_state                       },//66
    {COLLECT_POWER_R_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_R_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_grid_vol                      },//67
    {COLLECT_POWER_R_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_R_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_meter_act_power               },//68
    {COLLECT_POWER_S_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_S_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_grid_vol                      },//69
    {COLLECT_POWER_S_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_S_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_meter_act_power               },//70
    {COLLECT_POWER_T_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_T_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_grid_vol                      },//71
    {COLLECT_POWER_T_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_T_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_meter_act_power               },//72
    {COLLECT_POWER_R_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_R_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_on_grid_vol                   },//73
    {COLLECT_POWER_R_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_R_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_on_grid_act_power             },//74
    {COLLECT_POWER_S_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_S_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_on_grid_vol                   },//75
    {COLLECT_POWER_S_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_S_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_on_grid_act_power             },//76
    {COLLECT_POWER_T_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_T_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_on_grid_vol                   },//77
    {COLLECT_POWER_T_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_T_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_on_grid_act_power             },//78
    {COLLECT_POWER_R_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_R_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_gen_vol                       },//79
    {COLLECT_POWER_R_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_R_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_gen_act_power                 },//80
    {COLLECT_POWER_S_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_S_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_gen_vol                       },//81
    {COLLECT_POWER_S_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_S_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_gen_act_power                 },//82
    {COLLECT_POWER_T_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_T_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_gen_vol                       },//83
    {COLLECT_POWER_T_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_T_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_gen_act_power                 },//84
    {COLLECT_POWER_R_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_R_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_backup_vol                    },//85
    {COLLECT_POWER_R_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_R_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_backup_act_power              },//86
    {COLLECT_POWER_S_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_S_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_backup_vol                    },//87
    {COLLECT_POWER_S_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_S_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_backup_act_power              },//88
    {COLLECT_POWER_T_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_T_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_backup_vol                    },//89
    {COLLECT_POWER_T_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_T_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_backup_act_power              },//90
    {COLLECT_POWER_R_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_R_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_nload_vol                     },//91
    {COLLECT_POWER_R_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_R_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_nload_act_power               },//92
    {COLLECT_POWER_S_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_S_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_nload_vol                     },//93
    {COLLECT_POWER_S_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_S_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_nload_act_power               },//94
    {COLLECT_POWER_T_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_T_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_nload_vol                     },//95
    {COLLECT_POWER_T_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_T_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_nload_act_power               },//96
    {COLLECT_POWER_R_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_R_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_third_party_vol               },//97
    {COLLECT_POWER_R_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_R_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_third_party_act_power         },//98
    {COLLECT_POWER_S_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_S_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_third_party_vol               },//99
    {COLLECT_POWER_S_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_S_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_third_party_act_power         },//100
    {COLLECT_POWER_T_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_T_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_third_party_vol               },//101
    {COLLECT_POWER_T_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_T_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_third_party_act_power         },//102
    {COLLECT_POWER_R_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_R_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_inv_vol                       },//103
    {COLLECT_POWER_R_INV_POWER_SECTION_ADDR,        COLLECT_POWER_R_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_inv_act_power                 },//104
    {COLLECT_POWER_S_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_S_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_inv_vol                       },//105
    {COLLECT_POWER_S_INV_POWER_SECTION_ADDR,        COLLECT_POWER_S_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_inv_act_power                 },//106
    {COLLECT_POWER_T_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_T_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_inv_vol                       },//107
    {COLLECT_POWER_T_INV_POWER_SECTION_ADDR,        COLLECT_POWER_T_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_inv_act_power                 },//108
    {COLLECT_POWER_REVERSE_SECTION_ADDR,            COLLECT_POWER_REVERSE_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.reserve                         },//109
    {COLLECT_POWER_BAT_VOLT_SECTION_ADDR,           COLLECT_POWER_BAT_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_bat_vol                     },//110
    {COLLECT_POWER_BAT_POWER_SECTION_ADDR,          COLLECT_POWER_BAT_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_bat_power                   },//111
    {COLLECT_POWER_EV_VOLT_SECTION_ADDR,            COLLECT_POWER_EV_VOLT_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_ev_vol                      },//112
    {COLLECT_POWER_EV_POWER_SECTION_ADDR,           COLLECT_POWER_EV_POWER_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_ev_power                    },//113
    {COLLECT_POWER_PV1_VOLT_SECTION_ADDR,           COLLECT_POWER_PV1_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_1_vol                        },//114
    {COLLECT_POWER_PV1_POWER_SECTION_ADDR,          COLLECT_POWER_PV1_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_1_power                      },//115
    {COLLECT_POWER_PV2_VOLT_SECTION_ADDR,           COLLECT_POWER_PV2_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_2_vol                        },//116
    {COLLECT_POWER_PV2_POWER_SECTION_ADDR,          COLLECT_POWER_PV2_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_2_power                      },//117
    {COLLECT_POWER_PV3_VOLT_SECTION_ADDR,           COLLECT_POWER_PV3_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_3_vol                        },//118
    {COLLECT_POWER_PV3_POWER_SECTION_ADDR,          COLLECT_POWER_PV3_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_3_power                      },//119
    {COLLECT_POWER_PV4_VOLT_SECTION_ADDR,           COLLECT_POWER_PV4_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_4_vol                        },//120
    {COLLECT_POWER_PV4_POWER_SECTION_ADDR,          COLLECT_POWER_PV4_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_4_power                      },//121
    {COLLECT_POWER_REVERSE2_SECTION_ADDR,           COLLECT_POWER_REVERSE2_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.reserve2                        },//122
    {COLLECT_POWER_WORK_MODE_SECTION_ADDR,          COLLECT_POWER_WORK_MODE_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.work_mode                         },//123
    {COLLECT_POWER_BAT_MAX_CHG_POWER_SECTION_ADDR,  COLLECT_POWER_BAT_MAX_CHG_POWER_SECTION_LEN, EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_power             },//124
    {COLLECT_POWER_BAT_MAX_DSG_CURR_SECTION_ADDR,   COLLECT_POWER_BAT_MAX_DSG_CURR_SECTION_LEN,  EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_current           },//125
    {COLLECT_POWER_BAT_MAX_DSG_POWER_SECTION_ADDR,  COLLECT_POWER_BAT_MAX_DSG_POWER_SECTION_LEN, EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_power             },//126
    {COLLECT_POWER_SET_ERVERSE_SECTION_ADDR,        COLLECT_POWER_SET_ERVERSE_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.reserve                           },//127
    {COLLECT_POWER_BAT_UV_LIMIT_SECTION_ADDR,       COLLECT_POWER_BAT_UV_LIMIT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.limit_info.bat_under_vol_threshold               },//128
    {COLLECT_POWER_CALIBRATION_SECTION_ADDR,        COLLECT_POWER_CALIBRATION_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.calibrate_info.calibration_reset                 },//129
    {COLLECT_POWER_ARC_ENABLE_SECTION_ADDR,         COLLECT_POWER_ARC_ENABLE_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.arc_info.reserve                                 },//130
    {COLLECT_POWER_INSIDE_SECTION_ADDR,             COLLECT_POWER_INSIDE_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.inside_info                                      },//131
     /********pack信息********/                                                                                                       
    {COLLECT_PACK_MODEL_ID_SECTION_ADDR,            COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.model_id                             },//132
    {COLLECT_PACK_SW_VER_SECTION_ADDR,              COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.sw_ver                               },//133
    {COLLECT_PACK_SW_VER_EXT_SECTION_ADDR,          COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.sw_ver_ext                           },//134
    {COLLECT_PACK_ADDR_SECTION_ADDR,                COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.addr                                },//135
    {COLLECT_PACK_CHG_ENERGY_SECTION_ADDR,          COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.accu_chg_energy                     },//136
    {COLLECT_PACK_HEAT_SECTION_ADDR,                COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.heat_status                         },//137 
    {COLLECT_PACK_BALANCE_STATUS_SECTION_ADDR,      COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.balance_status                      },//138
    {COLLECT_PACK_HEAT_CTRL_SECTION_ADDR,           COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].ctrl_info.heat_ctrl                            },//139
    {COLLECT_PACK_CELL_VOLT_SECTION_ADDR,           COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].cell_info.cell_vol                             },//140
    {COLLECT_PACK_INSIDE_SECTION_ADDR,              COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].inside_info                                    },//141
                                                                                                                                                                                                                 
    {COLLECT_PACK_2_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.model_id                             },//142
    {COLLECT_PACK_2_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.sw_ver                               },//143
    {COLLECT_PACK_2_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.sw_ver_ext                           },//144
    {COLLECT_PACK_2_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.addr                                },//145
    {COLLECT_PACK_2_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.accu_chg_energy                     },//146
    {COLLECT_PACK_2_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.heat_status                         },//147
    {COLLECT_PACK_2_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.balance_status                      },//148
    {COLLECT_PACK_2_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].ctrl_info.heat_ctrl                            },//149
    {COLLECT_PACK_2_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].cell_info.cell_vol                             },//150
    {COLLECT_PACK_2_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].inside_info                                    },//151
                                                                                                                                                                                                                 
    {COLLECT_PACK_3_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.model_id                             },//152
    {COLLECT_PACK_3_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.sw_ver                               },//153
    {COLLECT_PACK_3_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.sw_ver_ext                           },//154
    {COLLECT_PACK_3_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.addr                                },//155
    {COLLECT_PACK_3_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.accu_chg_energy                     },//156
    {COLLECT_PACK_3_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.heat_status                         },//157
    {COLLECT_PACK_3_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.balance_status                      },//158
    {COLLECT_PACK_3_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].ctrl_info.heat_ctrl                            },//159
    {COLLECT_PACK_3_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].cell_info.cell_vol                             },//160
    {COLLECT_PACK_3_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].inside_info                                    },//161
                                                                                                                                                                                                                 
    {COLLECT_PACK_4_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.model_id                             },//162
    {COLLECT_PACK_4_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.sw_ver                               },//163
    {COLLECT_PACK_4_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.sw_ver_ext                           },//164
    {COLLECT_PACK_4_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.addr                                },//165
    {COLLECT_PACK_4_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.accu_chg_energy                     },//166
    {COLLECT_PACK_4_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.heat_status                         },//167
    {COLLECT_PACK_4_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.balance_status                      },//168
    {COLLECT_PACK_4_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].ctrl_info.heat_ctrl                            },//169
    {COLLECT_PACK_4_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].cell_info.cell_vol                             },//170
    {COLLECT_PACK_4_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].inside_info                                    },//171
                                                                                                                                                                                                                 
    {COLLECT_PACK_5_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.model_id                             },//172
    {COLLECT_PACK_5_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.sw_ver                               },//173
    {COLLECT_PACK_5_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.sw_ver_ext                           },//174
    {COLLECT_PACK_5_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.addr                                },//175
    {COLLECT_PACK_5_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.accu_chg_energy                     },//176
    {COLLECT_PACK_5_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.heat_status                         },//177
    {COLLECT_PACK_5_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.balance_status                      },//178
    {COLLECT_PACK_5_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].ctrl_info.heat_ctrl                            },//179
    {COLLECT_PACK_5_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].cell_info.cell_vol                             },//180
    {COLLECT_PACK_5_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].inside_info                                    },//181
                                                                                                                                                                                                                    
    {COLLECT_PACK_6_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.model_id                             },//182
    {COLLECT_PACK_6_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.sw_ver                               },//183
    {COLLECT_PACK_6_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.sw_ver_ext                           },//184
    {COLLECT_PACK_6_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.addr                                },//185
    {COLLECT_PACK_6_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.accu_chg_energy                     },//186
    {COLLECT_PACK_6_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.heat_status                         },//187
    {COLLECT_PACK_6_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.balance_status                      },//188
    {COLLECT_PACK_6_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].ctrl_info.heat_ctrl                            },//189
    {COLLECT_PACK_6_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].cell_info.cell_vol                             },//190
    {COLLECT_PACK_6_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].inside_info                                    },//191  
                                                                                                                                                                                                                 
    {COLLECT_COP1_BASE_INFO_ADDR,                   COLLECT_COP1_BASE_INFO_NUM,                  EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.cop_data.cop_device1_info.cop_cpu_modelid                   },//192
    {COLLECT_COP2_BASE_INFO_ADDR,                   COLLECT_COP2_BASE_INFO_NUM,                  EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.cop_data.cop_device2_info.cop_cpu_modelid                   },//193
};                                                                                                                                                                                          
#define INSTALLER_USER_NUM     (sizeof(InstallerUserDataPoint)/sizeof(ModbusDataPoint_t))                                                                                                                                                                                                                                                                                              
static const ModbusDataPoint_t InternalUserDataPoint[] =                                                                                                                                    
{                                                                                                                                                                                           
    {COLLECT_MODE_ID,                               MODE_ID_NUM,                                 EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.model_id                            },//0
    {COLLECT_SN_ADDR,                               COLLECT_SN_NUM,                              EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.sn[0]                               },//1
    {COLLECT_HW_VER_ADDR,                           COLLECT_HW_VER_NUM,                          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.hw_ver                              },//2
    {COLLECT_SW_VER_ADDR,                           COLLECT_SW_VER_NUM,                          EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.sw_ver                              },//3
    {COLLECT_SW_VER_EXTEND_ADDR,                    COLLECT_SW_VER_EXTEND_NUM,                   EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.sw_ver_ext                          },//4
    {COLLECT_USER_ADDR,                             COLLECT_USER_NUM,                            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.user_type                           },//5
    {COLLECT_NORMAL_PASSWORD_ADDR,                  COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.normal_password[0]                  },//6
    {COLLECT_MANAGER_PASSWORD_ADDR,                 COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.manager_password[0]                 },//7
    {COLLECT_INSTALLER_PASSWORD_ADDR,               COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.installer_password[0]               },//8
    {COLLECT_INTERNAL_PASSWORD_ADDR,                COLLECT_PASSWORD_NUM,                        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.base_info.internal_password[0]                },//9
    {COLLECT_SLAVE_ADDR_ADDR,                       COLLECT_SALVE_ADDR_NUM,                      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.modbus_addr                         },//10
    {COLLECT_BAUD_ADDR,                             COLLECT_BAUD_NUM,                            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.base_info.baudrate                            },//11
    {COLLECT_RESERVED_1_ADDR,                       COLLECT_RESERVED_1_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.base_info.reserve[0]                          },//12
    {COLLECT_DEV_TYPE_ADDR,                         COLLECT_DEV_TYPE_NUM,                        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.dev_type                          },//13                                                                                                                                                                                                      
    {COLLECT_LCD_BL_TIME_ADDR,                      COLLECT_LCD_BL_TIME_NUM,                     EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_bl_time                       },//14
    {COLLECT_LCD_BL_BRINGHTNESS_ADDR,               COLLECT_LCD_BL_BRINGHTNESS_NUM,              EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_bl_brightness                 },//15
    {COLLECT_LCD_LANGUEAGE_ADDR,                    COLLECT_LCD_LANGUEAGE_NUM,                   EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.lcd_languege                      },//16
    {COLLECT_LED_FRONT_SWITCH_AND_RESE_ADDR,        COLLECT_LED_FRONT_SWITCH_AND_RESE_NUM,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.reserve                           },//17  //5034预留 5035前面板控制 5036预留
    {COLLECT_LED_AUX_MODE_ADDR,                     COLLECT_LED_AUX_MODE_NUM,                    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.device_info.led_aux_mode                      },//18
    {COLLECT_RESERVED_2_ADDR,                       COLLECT_RESERVED_2_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.device_info.reserve3[0]                       },//19
    {COLLECT_LOAD_PEAK_TIME_ADDR,                   COLLECT_PEAK_LOAD_TIME_NUM,                  EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.peak_time_info.chg_start_time1                },//20
    {COLLECT_RESERVED_3_ADDR,                       COLLECT_RESERVED_3_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.peak_time_info.reserve[0]                     },//21
    {COLLECT_RESERVED_4_ADDR,                       COLLECT_RESERVED_4_NUM,                      EN_DATA_U16,   EN_ATTR_INVALID,   &gstCollectInfo.maincard_data.total_pack_info.reserve[0]                    },//22
    {COLLECT_PACK_NUM_SECTION_ADDR,                 COLLECT_PACK_NUM_SECTION_LEN,                EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.pack_num                      },//23
    {COLLECT_BMS_ERROR_SECTION_ADDR,                COLLECT_BMS_ERROR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.bms_error                     },//24
    {COLLECT_BMS_REVERSE_SECTION_ADDR,              COLLECT_BMS_REVERSE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.total_pack_info.reserve2                      },//25
    {COLLECT_BMS_DSG_MIN_VOLT_SECTION_ADDR,         COLLECT_BMS_DSG_MIN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.total_pack_info.dsg_min_vol                   },//26                                                                                  
    {COLLECT_ELEC_PV_DAY_SECTION_ADDR,              COLLECT_ELEC_PV_DAY_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_day               },//27
    {COLLECT_ELEC_PV_MONTH_SECTION_ADDR,            COLLECT_ELEC_PV_MONTH_SECTION_LEN,           EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.pv_elec_per_month             },//28
    {COLLECT_ELEC_BUY_DAY_SECTION_ADDR,             COLLECT_ELEC_BUY_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_day              },//29
    {COLLECT_ELEC_BUY_MONTH_SECTION_ADDR,           COLLECT_ELEC_BUY_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.buy_elec_per_month            },//30
    {COLLECT_ELEC_SELL_DAY_SECTION_ADDR,            COLLECT_ELEC_SELL_DAY_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_day             },//31
    {COLLECT_ELEC_SELL_MONTH_SECTION_ADDR,          COLLECT_ELEC_SELL_MONTH_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.sell_elec_per_month           },//32
    {COLLECT_ELEC_BAT_CHG_DAY_SECTION_ADDR,         COLLECT_ELEC_BAT_CHG_DAY_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_day          },//33
    {COLLECT_ELEC_BAT_CHG_MONTH_SECTION_ADDR,       COLLECT_ELEC_BAT_CHG_MONTH_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_chg_per_month        },//34
    {COLLECT_ELEC_BAT_DSG_DAY_SECTION_ADDR,         COLLECT_ELEC_BAT_DSG_DAY_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_day          },//35
    {COLLECT_ELEC_BAT_DSG_MONTH_SECTION_ADDR,       COLLECT_ELEC_BAT_DSG_MONTH_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.bat_elec_dsg_per_month        },//36
    {COLLECT_ELEC_EPS_DAY_SECTION_ADDR,             COLLECT_ELEC_EPS_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_day              },//37
    {COLLECT_ELEC_EPS_MONTH_SECTION_ADDR,           COLLECT_ELEC_EPS_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.eps_elec_per_month            },//38
    {COLLECT_ELEC_REMOTE_LOAD_DAY_SECTION_ADDR,     COLLECT_ELEC_REMOTE_LOAD_DAY_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_day      },//39
    {COLLECT_ELEC_REMOTE_LOAD_MONTH_SECTION_ADDR,   COLLECT_ELEC_REMOTE_LOAD_MONTH_SECTION_LEN,  EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.remote_load_elec_per_month    },//40
    {COLLECT_ELEC_GEN_DAY_SECTION_ADDR,             COLLECT_ELEC_GEN_DAY_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_day              },//41
    {COLLECT_ELEC_GEN_MONTH_SECTION_ADDR,           COLLECT_ELEC_GEN_MONTH_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.gen_elec_per_month            },//42
    {COLLECT_ELEC_EV_CHG_DAY_SECTION_ADDR,          COLLECT_ELEC_EV_CHG_DAY_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_day           },//43
    {COLLECT_ELEC_EV_CHG_MONTH_SECTION_ADDR,        COLLECT_ELEC_EV_CHG_MONTH_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_chg_per_month         },//44
    {COLLECT_ELEC_EV_DSG_DAY_SECTION_ADDR,          COLLECT_ELEC_EV_DSG_DAY_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_day           },//45
    {COLLECT_ELEC_EV_DSG_MONTH_SECTION_ADDR,        COLLECT_ELEC_EV_DSG_MONTH_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.ev_elec_dsg_per_month         },//46                                                                                      
    {COLLECT_ELEC_REVERSE_SECTION_ADDR,             COLLECT_ELEC_REVERSE_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.elec_stats_info.reserve3                      },//47                                                                                                                     
    {COLLECT_DEV_RUN_STATUS_SECTION_ADDR,           COLLECT_DEV_RUN_STATUS_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.dev_run_state                        },//48  
    {COLLECT_DEV_RUN_TIME_SECTION_ADDR,             COLLECT_DEV_RUN_TIME_SECTION_LEN,            EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.run_time                             },//49 
    {COLLECT_DEV_UPGRADE_STATUS_SECTION_ADDR,       COLLECT_DEV_UPGRADE_STATUS_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.maincard_data.sys_info.upgrade_state                        },//50
    {COLLECT_NET_CONN_SECTION_ADDR,                 COLLECT_NET_CONN_SECTION_LEN,                EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.net_conn_state                       },//51                                                                                        
    {COLLECT_RTC_TIME_SECTION_ADDR,                 COLLECT_RTC_TIME_SECTION_LEN,                EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.time_stamp                           },//52  
    {COLLECT_MAX_CHG_POWER_SET_ADDR,                COLLECT_MAX_CHG_POWER_SET_NUM,               EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.max_chg_power_set                    },//53 
    {COLLECT_MAX_CHG_CURRENT_SET_ADDR,              COLLECT_MAX_CHG_CURRENT_SET_NUM,             EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.max_chg_curr_set                     },//54 
    {COLLECT_DEV_SPECIAL_MODE_ADDR,                 COLLECT_DEV_SPECIAL_MODE_NUM,                EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.special_mode                         },//55 
    {COLLECT_REST_FACTORY_ADDR,                     COLLECT_REST_FACTORY_NUM,                    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.factory_reset                        },//56 
    {COLLECT_REMOTE_SHUTDOWN_ADDR,                  COLLECT_REMOTE_SHUTDOWN_NUM,                 EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.remote_shutdown                      },//57 
    {COLLECT_SYS_REVERSE_2_SECTION_ADDR,            COLLECT_SYS_REVERSE_2_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.sys_info.reserve3                             },//58                                                                                                                                                                                                    
    {COLLECT_MAINCARD_INSIDE_INFO_ADDR,             COLLECT_MAINCARD_INSIDE_INFO_NUM,            EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.maincard_data.inside_info                                   },//59 
    /********功率信息********/      
    {COLLECT_POWER_PCS_MODEL_ID_SECTION_ADDR,       COLLECT_POWER_PCS_MODEL_ID_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.model_id                           },//60
    {COLLECT_POWER_PCS_SW_VER_SECTION_ADDR,         COLLECT_POWER_PCS_SW_VER_SECTION_LEN,        EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.sw_ver                             },//61
    {COLLECT_POWER_PCS_SW_VER_EXT_SECTION_ADDR,     COLLECT_POWER_PCS_SW_VER_EXT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.pcs_base_info.sw_ver_ext                         },//62
    {COLLECT_POWER_ARC_MODEL_ID_SECTION_ADDR,       COLLECT_POWER_ARC_MODEL_ID_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.model_id                           },//63
    {COLLECT_POWER_ARC_SW_VER_SECTION_ADDR,         COLLECT_POWER_ARC_SW_VER_SECTION_LEN,        EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.sw_ver                             },//64
    {COLLECT_POWER_ARC_SW_VER_EXT_SECTION_ADDR,     COLLECT_POWER_ARC_SW_VER_EXT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.arc_base_info.sw_ver_ext                         },//65
    {COLLECT_POWER_INTERFACE_SECTION_ADDR,          COLLECT_POWER_INTERFACE_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.model_info.interface_state                       },//66
    {COLLECT_POWER_R_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_R_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_grid_vol                      },//67
    {COLLECT_POWER_R_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_R_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_meter_act_power               },//68
    {COLLECT_POWER_S_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_S_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_grid_vol                      },//69
    {COLLECT_POWER_S_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_S_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_meter_act_power               },//70
    {COLLECT_POWER_T_GRID_VOLT_SECTION_ADDR,        COLLECT_POWER_T_GRID_VOLT_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_grid_vol                      },//71
    {COLLECT_POWER_T_GRID_POWER_SECTION_ADDR,       COLLECT_POWER_T_GRID_POWER_SECTION_LEN,      EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_meter_act_power               },//72
    {COLLECT_POWER_R_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_R_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_on_grid_vol                   },//73
    {COLLECT_POWER_R_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_R_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_on_grid_act_power             },//74
    {COLLECT_POWER_S_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_S_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_on_grid_vol                   },//75
    {COLLECT_POWER_S_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_S_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_on_grid_act_power             },//76
    {COLLECT_POWER_T_ON_GRID_VOLT_SECTION_ADDR,     COLLECT_POWER_T_ON_GRID_VOLT_SECTION_LEN,    EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_on_grid_vol                   },//77
    {COLLECT_POWER_T_ON_GRID_POWER_SECTION_ADDR,    COLLECT_POWER_T_ON_GRID_POWER_SECTION_LEN,   EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_on_grid_act_power             },//78
    {COLLECT_POWER_R_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_R_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_gen_vol                       },//79
    {COLLECT_POWER_R_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_R_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_gen_act_power                 },//80
    {COLLECT_POWER_S_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_S_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_gen_vol                       },//81
    {COLLECT_POWER_S_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_S_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_gen_act_power                 },//82
    {COLLECT_POWER_T_GEN_VOLT_SECTION_ADDR,         COLLECT_POWER_T_GEN_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_gen_vol                       },//83
    {COLLECT_POWER_T_GEN_POWER_SECTION_ADDR,        COLLECT_POWER_T_GEN_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_gen_act_power                 },//84
    {COLLECT_POWER_R_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_R_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_backup_vol                    },//85
    {COLLECT_POWER_R_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_R_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_backup_act_power              },//86
    {COLLECT_POWER_S_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_S_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_backup_vol                    },//87
    {COLLECT_POWER_S_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_S_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_backup_act_power              },//88
    {COLLECT_POWER_T_BACKUP_VOLT_SECTION_ADDR,      COLLECT_POWER_T_BACKUP_VOLT_SECTION_LEN,     EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_backup_vol                    },//89
    {COLLECT_POWER_T_BACKUP_POWER_SECTION_ADDR,     COLLECT_POWER_T_BACKUP_POWER_SECTION_LEN,    EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_backup_act_power              },//90
    {COLLECT_POWER_R_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_R_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_nload_vol                     },//91
    {COLLECT_POWER_R_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_R_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_nload_act_power               },//92
    {COLLECT_POWER_S_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_S_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_nload_vol                     },//93
    {COLLECT_POWER_S_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_S_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_nload_act_power               },//94
    {COLLECT_POWER_T_NLOAD_VOLT_SECTION_ADDR,       COLLECT_POWER_T_NLOAD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_nload_vol                     },//95
    {COLLECT_POWER_T_NLOAD_POWER_SECTION_ADDR,      COLLECT_POWER_T_NLOAD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_nload_act_power               },//96
    {COLLECT_POWER_R_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_R_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_third_party_vol               },//97
    {COLLECT_POWER_R_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_R_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_third_party_act_power         },//98
    {COLLECT_POWER_S_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_S_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_third_party_vol               },//99
    {COLLECT_POWER_S_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_S_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_third_party_act_power         },//100
    {COLLECT_POWER_T_THIRD_VOLT_SECTION_ADDR,       COLLECT_POWER_T_THIRD_VOLT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_third_party_vol               },//101
    {COLLECT_POWER_T_THIRD_POWER_SECTION_ADDR,      COLLECT_POWER_T_THIRD_POWER_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_third_party_act_power         },//102
    {COLLECT_POWER_R_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_R_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_inv_vol                       },//103
    {COLLECT_POWER_R_INV_POWER_SECTION_ADDR,        COLLECT_POWER_R_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.r_inv_act_power                 },//104
    {COLLECT_POWER_S_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_S_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_inv_vol                       },//105
    {COLLECT_POWER_S_INV_POWER_SECTION_ADDR,        COLLECT_POWER_S_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.s_inv_act_power                 },//106
    {COLLECT_POWER_T_INV_VOLT_SECTION_ADDR,         COLLECT_POWER_T_INV_VOLT_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_inv_vol                       },//107
    {COLLECT_POWER_T_INV_POWER_SECTION_ADDR,        COLLECT_POWER_T_INV_POWER_SECTION_LEN,       EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.t_inv_act_power                 },//108
    {COLLECT_POWER_REVERSE_SECTION_ADDR,            COLLECT_POWER_REVERSE_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.reserve                         },//109
    {COLLECT_POWER_BAT_VOLT_SECTION_ADDR,           COLLECT_POWER_BAT_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_bat_vol                     },//110
    {COLLECT_POWER_BAT_POWER_SECTION_ADDR,          COLLECT_POWER_BAT_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_bat_power                   },//111
    {COLLECT_POWER_EV_VOLT_SECTION_ADDR,            COLLECT_POWER_EV_VOLT_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_ev_vol                      },//112
    {COLLECT_POWER_EV_POWER_SECTION_ADDR,           COLLECT_POWER_EV_POWER_SECTION_LEN,          EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.inv_ev_power                    },//113
    {COLLECT_POWER_PV1_VOLT_SECTION_ADDR,           COLLECT_POWER_PV1_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_1_vol                        },//114
    {COLLECT_POWER_PV1_POWER_SECTION_ADDR,          COLLECT_POWER_PV1_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_1_power                      },//115
    {COLLECT_POWER_PV2_VOLT_SECTION_ADDR,           COLLECT_POWER_PV2_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_2_vol                        },//116
    {COLLECT_POWER_PV2_POWER_SECTION_ADDR,          COLLECT_POWER_PV2_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_2_power                      },//117
    {COLLECT_POWER_PV3_VOLT_SECTION_ADDR,           COLLECT_POWER_PV3_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_3_vol                        },//118
    {COLLECT_POWER_PV3_POWER_SECTION_ADDR,          COLLECT_POWER_PV3_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_3_power                      },//119
    {COLLECT_POWER_PV4_VOLT_SECTION_ADDR,           COLLECT_POWER_PV4_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_4_vol                        },//120
    {COLLECT_POWER_PV4_POWER_SECTION_ADDR,          COLLECT_POWER_PV4_POWER_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.pv_4_power                      },//121
    {COLLECT_POWER_REVERSE2_SECTION_ADDR,           COLLECT_POWER_REVERSE2_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.power_value_info.reserve2                        },//122
    {COLLECT_POWER_WORK_MODE_SECTION_ADDR,          COLLECT_POWER_WORK_MODE_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.work_mode                         },//123
    {COLLECT_POWER_BAT_MAX_CHG_POWER_SECTION_ADDR,  COLLECT_POWER_BAT_MAX_CHG_POWER_SECTION_LEN, EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_power             },//124
    {COLLECT_POWER_BAT_MAX_DSG_CURR_SECTION_ADDR,   COLLECT_POWER_BAT_MAX_DSG_CURR_SECTION_LEN,  EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_current           },//125
    {COLLECT_POWER_BAT_MAX_DSG_POWER_SECTION_ADDR,  COLLECT_POWER_BAT_MAX_DSG_POWER_SECTION_LEN, EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_power             },//126
    {COLLECT_POWER_SET_ERVERSE_SECTION_ADDR,        COLLECT_POWER_SET_ERVERSE_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.power_set_info.reserve                           },//127
    {COLLECT_POWER_BAT_UV_LIMIT_SECTION_ADDR,       COLLECT_POWER_BAT_UV_LIMIT_SECTION_LEN,      EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.limit_info.bat_under_vol_threshold               },//128
    {COLLECT_POWER_CALIBRATION_SECTION_ADDR,        COLLECT_POWER_CALIBRATION_SECTION_LEN,       EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.power_data.calibrate_info.calibration_reset                 },//129
    {COLLECT_POWER_ARC_ENABLE_SECTION_ADDR,         COLLECT_POWER_ARC_ENABLE_SECTION_LEN,        EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.arc_info.reserve                                 },//130
    {COLLECT_POWER_INSIDE_SECTION_ADDR,             COLLECT_POWER_INSIDE_SECTION_LEN,            EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.power_data.inside_info                                      },//131
     /********pack信息********/                                                                                                       
    {COLLECT_PACK_MODEL_ID_SECTION_ADDR,            COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.model_id                             },//132
    {COLLECT_PACK_SW_VER_SECTION_ADDR,              COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.sw_ver                               },//133
    {COLLECT_PACK_SW_VER_EXT_SECTION_ADDR,          COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].base_info.sw_ver_ext                           },//134
    {COLLECT_PACK_ADDR_SECTION_ADDR,                COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.addr                                },//135
    {COLLECT_PACK_CHG_ENERGY_SECTION_ADDR,          COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.accu_chg_energy                     },//136
    {COLLECT_PACK_HEAT_SECTION_ADDR,                COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.heat_status                         },//137 
    {COLLECT_PACK_BALANCE_STATUS_SECTION_ADDR,      COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].state_info.balance_status                      },//138
    {COLLECT_PACK_HEAT_CTRL_SECTION_ADDR,           COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[0].ctrl_info.heat_ctrl                            },//139
    {COLLECT_PACK_CELL_VOLT_SECTION_ADDR,           COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].cell_info.cell_vol                             },//140
    {COLLECT_PACK_INSIDE_SECTION_ADDR,              COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[0].inside_info                                    },//141
                                                                                                                                                                                                                 
    {COLLECT_PACK_2_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.model_id                             },//142
    {COLLECT_PACK_2_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.sw_ver                               },//143
    {COLLECT_PACK_2_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].base_info.sw_ver_ext                           },//144
    {COLLECT_PACK_2_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.addr                                },//145
    {COLLECT_PACK_2_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.accu_chg_energy                     },//146
    {COLLECT_PACK_2_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.heat_status                         },//147
    {COLLECT_PACK_2_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].state_info.balance_status                      },//148
    {COLLECT_PACK_2_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[1].ctrl_info.heat_ctrl                            },//149
    {COLLECT_PACK_2_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].cell_info.cell_vol                             },//150
    {COLLECT_PACK_2_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[1].inside_info                                    },//151
                                                                                                                                                                                                                 
    {COLLECT_PACK_3_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.model_id                             },//152
    {COLLECT_PACK_3_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.sw_ver                               },//153
    {COLLECT_PACK_3_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].base_info.sw_ver_ext                           },//154
    {COLLECT_PACK_3_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.addr                                },//155
    {COLLECT_PACK_3_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.accu_chg_energy                     },//156
    {COLLECT_PACK_3_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.heat_status                         },//157
    {COLLECT_PACK_3_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].state_info.balance_status                      },//158
    {COLLECT_PACK_3_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[2].ctrl_info.heat_ctrl                            },//159
    {COLLECT_PACK_3_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].cell_info.cell_vol                             },//160
    {COLLECT_PACK_3_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[2].inside_info                                    },//161
                                                                                                                                                                                                                 
    {COLLECT_PACK_4_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.model_id                             },//162
    {COLLECT_PACK_4_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.sw_ver                               },//163
    {COLLECT_PACK_4_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].base_info.sw_ver_ext                           },//164
    {COLLECT_PACK_4_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.addr                                },//165
    {COLLECT_PACK_4_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.accu_chg_energy                     },//166
    {COLLECT_PACK_4_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.heat_status                         },//167
    {COLLECT_PACK_4_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].state_info.balance_status                      },//168
    {COLLECT_PACK_4_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[3].ctrl_info.heat_ctrl                            },//169
    {COLLECT_PACK_4_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].cell_info.cell_vol                             },//170
    {COLLECT_PACK_4_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[3].inside_info                                    },//171
                                                                                                                                                                                                                 
    {COLLECT_PACK_5_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.model_id                             },//172
    {COLLECT_PACK_5_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.sw_ver                               },//173
    {COLLECT_PACK_5_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].base_info.sw_ver_ext                           },//174
    {COLLECT_PACK_5_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.addr                                },//175
    {COLLECT_PACK_5_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.accu_chg_energy                     },//176
    {COLLECT_PACK_5_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.heat_status                         },//177
    {COLLECT_PACK_5_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].state_info.balance_status                      },//178
    {COLLECT_PACK_5_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[4].ctrl_info.heat_ctrl                            },//179
    {COLLECT_PACK_5_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].cell_info.cell_vol                             },//180
    {COLLECT_PACK_5_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[4].inside_info                                    },//181
                                                                                                                                                                                                                    
    {COLLECT_PACK_6_MODEL_ID_SECTION_ADDR,          COLLECT_PACK_MODEL_ID_SECTION_LEN,           EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.model_id                             },//182
    {COLLECT_PACK_6_SW_VER_SECTION_ADDR,            COLLECT_PACK_SW_VER_SECTION_LEN,             EN_DATA_U32,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.sw_ver                               },//183
    {COLLECT_PACK_6_SW_VER_EXT_SECTION_ADDR,        COLLECT_PACK_SW_VER_EXT_SECTION_LEN,         EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].base_info.sw_ver_ext                           },//184
    {COLLECT_PACK_6_ADDR_SECTION_ADDR,              COLLECT_PACK_ADDR_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.addr                                },//185
    {COLLECT_PACK_6_CHG_ENERGY_SECTION_ADDR,        COLLECT_PACK_CHG_ENERGY_SECTION_LEN,         EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.accu_chg_energy                     },//186
    {COLLECT_PACK_6_HEAT_SECTION_ADDR,              COLLECT_PACK_HEAT_SECTION_LEN,               EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.heat_status                         },//187
    {COLLECT_PACK_6_BALANCE_STATUS_SECTION_ADDR,    COLLECT_PACK_BALANCE_STATUS_SECTION_LEN,     EN_DATA_U32,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].state_info.balance_status                      },//188
    {COLLECT_PACK_6_HEAT_CTRL_SECTION_ADDR,         COLLECT_PACK_HEAT_CTRL_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RW,        &gstCollectInfo.pack_data[5].ctrl_info.heat_ctrl                            },//189
    {COLLECT_PACK_6_CELL_VOLT_SECTION_ADDR,         COLLECT_PACK_CELL_VOLT_SECTION_LEN,          EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].cell_info.cell_vol                             },//190
    {COLLECT_PACK_6_INSIDE_SECTION_ADDR,            COLLECT_PACK_INSIDE_SECTION_LEN,             EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.pack_data[5].inside_info                                    },//191     
                                                                                                                                                                                                                 
    {COLLECT_COP1_BASE_INFO_ADDR,                   COLLECT_COP1_BASE_INFO_NUM,                  EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.cop_data.cop_device1_info.cop_cpu_modelid                   },//192
    {COLLECT_COP2_BASE_INFO_ADDR,                   COLLECT_COP2_BASE_INFO_NUM,                  EN_DATA_U16,   EN_ATTR_RO,        &gstCollectInfo.cop_data.cop_device2_info.cop_cpu_modelid                   },//193
};
#define INTERNALL_USER_NUM     (sizeof(InternalUserDataPoint)/sizeof(ModbusDataPoint_t))

static const ModbusDataPoint_t ToollingUserDataPoint[] =
{
    {TOOLLING_ENTER_TEST_REG_ADDR,       TOOLLING_ENTER_TEST_REG_NUM,       EN_DATA_U16,   EN_ATTR_RW,      &gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode   },//0   0x5850
    {TOOLLING_GET_IO_REG_ADDR,           TOOLLING_GET_IO_REG_NUM,           EN_DATA_U16,   EN_ATTR_RW,      &gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.drm_enable        },//1   0x5851
    {TOOLLING_GET_RTC_REG_ADDR,          TOOLLING_GET_RTC_REG_NUM,          EN_DATA_U16,   EN_ATTR_RW,      &gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.rtc               },//2   0x5852
    {TOOLLING_IO_CTR_REG_ADDR,           TOOLLING_IO_CTR_REG_NUM,           EN_DATA_U16,   EN_ATTR_RW,      &gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.relay_io          },//3   0x5854

    {TOOLLING_TEST_FLASH_REG_ADDR,      TOOLLING_TEST_FLASH_REG_NUM,        EN_DATA_U16,   EN_ATTR_RW,      &gstCollectInfo.maincard_data.inside_info.tool_info.status_info.flash_result    },//4   0x5860
    {TOOLLING_IO_REG_RESULT_ADDR,       TOOLLING_IO_REG_RESULT_NUM,         EN_DATA_U16,   EN_ATTR_RW,      &gstCollectInfo.maincard_data.inside_info.tool_info.status_info.drm_status      },//5   0x5861
    {TOOLLING_RTC_REULT_REG_ADDR,       TOOLLING_RTC_REULT_REG_NUM,         EN_DATA_U16,   EN_ATTR_RW,      &gstCollectInfo.maincard_data.inside_info.tool_info.status_info.rtc_result      },//6   0x5862
    {TOOLLING_COP_UART_REG_ADDR,        TOOLLING_COP_UART_REG_NUM,          EN_DATA_U16,   EN_ATTR_RW,      &gstCollectInfo.maincard_data.inside_info.tool_info.status_info.cop_uart_result },//7   0x5863
    {TOOLLING_SW_REG_ADDR,              TOOLLING_SW_REG_NUM,                EN_DATA_U16,   EN_ATTR_RW,      &gstCollectInfo.maincard_data.inside_info.tool_info.status_info.main_version    },//8   0x5864
};
#define TOOLLING_USER_NUM     (sizeof(ToollingUserDataPoint)/sizeof(ModbusDataPoint_t))


ModbusRtuFrame_t gCollectModbusData;
const ModbusDataPoint_t *ModbusUserDataArr[MAX_USER_NUM] = {NomalUserDataPoint, ManagerUserDataPoint, InstallerUserDataPoint, InternalUserDataPoint, ToollingUserDataPoint};
const uint16_t          ModbusUserDataPointNumArr[MAX_USER_NUM] = {MORMAL_USER_NUM, MANAGER_USER_NUM, INSTALLER_USER_NUM, INTERNALL_USER_NUM, TOOLLING_USER_NUM};

static uint16_t _gCollectRegToPcsMap[][4] = 
{
    {0x6300, 0x6221},
    {0x6301, 0x6222},
    {0x6302, 0x6223},
    {0x6303, 0x6224},
    {0x6305, 0x6226},
    {0x6306, 0x6227},
    {0x6308, 0x6229},
    {0x630A, 0x622B},
    {0x630C, 0x622D},
    {0x630E, 0x622F},
    {0x6310, 0x6231},
    {0x6312, 0x6233},
    {0x6314, 0x6235},
    {0x6316, 0x6237},
    
    {0x6322, 0x6250},
    {0x6323, 0x6251},
    {0x6326, 0x6254},
    {0x6327, 0x6255},
    {0x6328, 0x6256},
    {0x6329, 0x6257},
    {0x632A, 0x6258},
    {0x632B, 0x6259},
    {0x632C, 0x625A},
    {0x632D, 0x625B},
    {0x632E, 0x625C},
    {0x632F, 0x625D},
    {0x6330, 0x625E},
    {0x6331, 0x625F},
    
    {0x6338, 0x6266},
    {0x6339, 0x6267},
    {0x633A, 0x6268},
    {0x633B, 0x6269},
    {0x633C, 0x626A},
    {0x633D, 0x626B},
    {0x633E, 0x626C},
    {0x633F, 0x626D},
    
    {0x6349, 0x6276},
    {0x634A, 0x6277},
    {0x634B, 0x6278},
    {0x634C, 0x6279},
    {0x634D, 0x627A},
    {0x634E, 0x627B},
    {0x634F, 0x627C},
    {0x63F0, 0x6280},
    
    {0x6400, 0x62B0},
    {0x6401, 0x62B1},
    {0x6402, 0x62B2},
    {0x6403, 0x62B3},
    {0x6404, 0x62B4},
    {0x6405, 0x62B5},
    {0x6406, 0x62B6},
    {0x6407, 0x62B7},
    {0x6408, 0x62B8},
    {0x6409, 0x62B9},
    {0x640A, 0x62BA},
    {0x640B, 0x62BB},
    {0x640C, 0x62BC},
    {0x640D, 0x62BD},
    {0x640E, 0x62BE},
    {0x640F, 0x62BF},
    {0x6410, 0x62C0},
    {0x6411, 0x62C1},
    
    {0x6480, 0x62D0},
    {0x6488, 0x62D5},
    {0x6489, 0x62D6},
    {0x648A, 0x62D7},
    {0x648B, 0x62D8},
    {0x648C, 0x62D9},
    {0x648D, 0x62DA},
    {0x648E, 0x62DB},
    {0x648F, 0x62DC},
    {0x6490, 0x62DD},
    {0x6491, 0x62DE},
    {0x6492, 0x62DF},
    {0x6493, 0x62E0},
    {0x6494, 0x62E1},
    {0x6495, 0x62E2},
    {0x6496, 0x62E3},
    {0x6496, 0x62E4},
    {0x6498, 0x62E5},
    {0x6499, 0x62E6},
    {0x649A, 0x62E7},
    {0x649B, 0x62E8},
    {0x649C, 0x62E9},
    {0x649D, 0x62EA},
    {0x649E, 0x62EB},
    {0x649F, 0x62EC},
    {0x64A0, 0x62ED},
    
    {0x64A1, 0x62EE},
    {0x64A2, 0x62EF},
    {0x64A3, 0x62F0},
    {0x64A4, 0x62F1},
    {0x64A5, 0x62F2},
    {0x64A6, 0x62F3},
    {0x64A7, 0x62F4},
    {0x64A8, 0x62F5},
    {0x64A9, 0x62F6},
    {0x64AA, 0x62F7},
    {0x64AB, 0x62F8},
    //arc
    {0x6500, 0x6400},
    {0x6508, 0x6408},
    {0x6509, 0x6409},  
};


bool CommCollectInit(void)
{
    bool ret;
    uint8_t  times = 0;
    _gstComCollectTxData.pData = _garrComCollectTxDataBuf;

//  QueueInit(&_gCollectWriteQueue, COLLECT_WRITE_QUEUE_DEPTH, sizeof(CollectTxData_t));
//  for(uint8_t i = 0; i < COLLECT_WRITE_QUEUE_DEPTH; i++)
//  {
//      _gCollectWriteData[i].pData = _CollectWriteDataBuf[i];
//  }

    gstCollectInfo.maincard_data.sys_info.dev_run_state = 1;        //初始化中......
    return true;
}


uint8_t GetAllBmsReadOverStatus(void)
{
    return _gReadAllBmsDataOkFlag;
}


#define  TEST_PCS_DATA   0

#if(TEST_PCS_DATA)
int16_t test_r_phase_meter_power = -2000;
int16_t test_batt_chg_dsg_power = -2001;
int16_t tset_r_phase_inverter_power = -3000;
int16_t test_r_phase_backup_apparent_power = -1999;
int16_t test_r_phase_load_apparent_power = -2002;
uint16_t test_pv_ch1_input_power = 1000;
uint16_t test_pv_ch2_input_power = 2000;

int16_t test_inverter_radiator_temp = -120;
int16_t test_chassis_internal_temp = -201;
int16_t test_soc = 0;
#endif

/**
  * @brief  SOC处理函数
  * @param  soc_ave : SOC均值
            soc_max : SOC最大值
  * @return 计算SOC
  * @note   NULL
  */
uint16_t _SocProcess(uint16_t soc_ave, uint16_t soc_max, uint16_t soc_min)
{
    uint16_t soc_temp[3] = {0,0,0};
    
    soc_temp[0] = ((soc_max * soc_max) + (1000 - soc_max) * soc_ave) / 1000;
    soc_temp[1] = ((1000 - soc_min) * soc_min + soc_min * soc_ave) / 1000;
    soc_temp[2] = (soc_max * soc_temp[0] + (1000 - soc_max) * soc_temp[1]) / 1000;
    
    return soc_temp[2];
}

/**
  * @brief  获取PCS数据
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void _GetPcsDataFunc(void)
{
    /*----PCS基础信息---*/
    gstCollectInfo.power_data.pcs_base_info.model_id       =    gstUdpPcsSlavePara.pcs_info.pcs_base_info.model_id             ;  //6000
    memcpy(gstCollectInfo.power_data.pcs_base_info.sn, gstUdpPcsSlavePara.pcs_info.pcs_base_info.sn, 10)                       ;  //6001~600A
    gstCollectInfo.power_data.pcs_base_info.hw_ver         =    gstUdpPcsSlavePara.pcs_info.pcs_base_info.hw_ver               ;  //600B
    gstCollectInfo.power_data.pcs_base_info.sw_ver         =    gstUdpPcsSlavePara.pcs_info.pcs_base_info.sw_ver[0] << 16 |
                                                                gstUdpPcsSlavePara.pcs_info.pcs_base_info.sw_ver[1]            ;  //600C~600D
    memcpy(gstCollectInfo.power_data.pcs_base_info.sw_ver_ext, gstUdpPcsSlavePara.pcs_info.pcs_base_info.sw_ver_ext, 12)       ;  //600E~6019
                                                                                                                                  //601D~601F

    /*----ARC基础信息---*/    
    gstCollectInfo.power_data.arc_base_info.model_id       =    gstUdpPcsSlavePara.pcs_info.arc_base_info.model_id             ;  //6020
    memcpy(gstCollectInfo.power_data.arc_base_info.sn, gstUdpPcsSlavePara.pcs_info.arc_base_info.sn, 10)                       ;  //6021~602A
    gstCollectInfo.power_data.arc_base_info.hw_ver         =    gstUdpPcsSlavePara.pcs_info.arc_base_info.hw_ver               ;  //602B
    gstCollectInfo.power_data.arc_base_info.sw_ver         =    gstUdpPcsSlavePara.pcs_info.arc_base_info.sw_ver[0] << 16 |               
                                                                gstUdpPcsSlavePara.pcs_info.arc_base_info.sw_ver[1]            ;  //602C~602D
    memcpy(gstCollectInfo.power_data.arc_base_info.sw_ver_ext, gstUdpPcsSlavePara.pcs_info.arc_base_info.sw_ver_ext, 12)       ;  //602E~6039
                                                                                                                                  //603D~603F

    /*----功率模块信息---*/

    gstCollectInfo.power_data.model_info.interface_state   =  gstUdpPcsSlavePara.pcs_info.model_status.interface_state          ;  //6100
    gstCollectInfo.power_data.model_info.inv_run_state     =  gstUdpPcsSlavePara.pcs_info.model_status.inv_run_state            ;  //6101
    gstCollectInfo.power_data.model_info.on_grid_state     =  gstUdpPcsSlavePara.pcs_info.model_status.inv_on_grid_state        ;  //6102
//    gstCollectInfo.power_data.model_info.dcdc_run_state    =  gstUdpPcsSlavePara.pcs_info.model_status.dcdc_run_state           ;  //6103  DCDC状态由监控卡通过电池有功功率进行判断
    gstCollectInfo.power_data.model_info.drm_state         =  gstUdpPcsSlavePara.pcs_info.model_status.drm_state                ;  //6104
    memcpy(gstCollectInfo.power_data.model_info.mppt_run_state, gstUdpPcsSlavePara.pcs_info.model_status.mppt_run_state, 4)     ;  //6105~6108
                                                                                                                                   //6109~6117
    
                                                                                                                                   //6124~6129
    gstCollectInfo.power_data.model_info.inv_radiator_temp  =  gstUdpPcsSlavePara.pcs_info.model_status.inv_radiator_temp       ;  //612A
    gstCollectInfo.power_data.model_info.inv_box_temp       =  gstUdpPcsSlavePara.pcs_info.model_status.inv_box_temp            ;  //612B
                                                                                                                                   //612C~617F

    /*----功率数值信息---*/
    gstCollectInfo.power_data.power_value_info.r_grid_vol                    =  gstUdpPcsSlavePara.pcs_info.power_info.r_grid_vol                  ;   //R相电网电压            //6180
    gstCollectInfo.power_data.power_value_info.r_meter_current               =  gstUdpPcsSlavePara.pcs_info.power_info.r_meter_current             ;   //R相电表电流            //6181
    gstCollectInfo.power_data.power_value_info.r_grid_freq                   =  gstUdpPcsSlavePara.pcs_info.power_info.r_grid_freq                 ;   //R相电网频率            //6182
    gstCollectInfo.power_data.power_value_info.r_meter_act_power             =  gstUdpPcsSlavePara.pcs_info.power_info.r_meter_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_meter_act_power[1]        ;   //R相电表有功功率        //6183~6184
    gstCollectInfo.power_data.power_value_info.r_meter_react_power           =  gstUdpPcsSlavePara.pcs_info.power_info.r_meter_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_meter_react_power[1]      ;   //R相电表无功功率        //6185~6186
    gstCollectInfo.power_data.power_value_info.r_meter_apparent_power        =  gstUdpPcsSlavePara.pcs_info.power_info.r_meter_apparent_power[0] << 16 | 
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_meter_apparent_power[1]   ;   //R相电表视在功率        //6187~6188
    gstCollectInfo.power_data.power_value_info.s_grid_vol                    =  gstUdpPcsSlavePara.pcs_info.power_info.s_grid_vol                  ;   //S相电网电压            //6189
    gstCollectInfo.power_data.power_value_info.s_meter_current               =  gstUdpPcsSlavePara.pcs_info.power_info.s_meter_current             ;   //S相电表电流            //618A
    gstCollectInfo.power_data.power_value_info.s_grid_freq                   =  gstUdpPcsSlavePara.pcs_info.power_info.s_grid_freq                 ;   //S相电网频率            //618B
    gstCollectInfo.power_data.power_value_info.s_meter_act_power             =  gstUdpPcsSlavePara.pcs_info.power_info.s_meter_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_meter_act_power[1]        ;   //S相电表有功功率        //618C~618D
    gstCollectInfo.power_data.power_value_info.s_meter_react_power           =  gstUdpPcsSlavePara.pcs_info.power_info.s_meter_react_power[0] << 16 |         
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_meter_react_power[1]      ;   //S相电表无功功率        //618E~618F
    gstCollectInfo.power_data.power_value_info.s_meter_apparent_power        =  gstUdpPcsSlavePara.pcs_info.power_info.s_meter_apparent_power[0] << 16 |   
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_meter_apparent_power[1]   ;   //S相电表视在功率        //6190~6191
    gstCollectInfo.power_data.power_value_info.t_grid_vol                    =  gstUdpPcsSlavePara.pcs_info.power_info.t_grid_vol                  ;   //T相电网电压            //6192
    gstCollectInfo.power_data.power_value_info.t_meter_current               =  gstUdpPcsSlavePara.pcs_info.power_info.t_meter_current             ;   //T相电表电流            //6193
    gstCollectInfo.power_data.power_value_info.t_grid_freq                   =  gstUdpPcsSlavePara.pcs_info.power_info.t_grid_freq                 ;   //T相电网频率            //6194
    gstCollectInfo.power_data.power_value_info.t_meter_act_power             =  gstUdpPcsSlavePara.pcs_info.power_info.t_meter_act_power[0] << 16 |  
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_meter_act_power[1]       ;   //T相电表有功功率        //6195~6196
    gstCollectInfo.power_data.power_value_info.t_meter_react_power           =  gstUdpPcsSlavePara.pcs_info.power_info.t_meter_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_meter_react_power[1]      ;   //T相电表无功功率        //6197~6198
    gstCollectInfo.power_data.power_value_info.t_meter_apparent_power        =  gstUdpPcsSlavePara.pcs_info.power_info.t_meter_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_meter_apparent_power[1]   ;   //T相电表视在功率        //6199~619A
                                                                                                                      
    gstCollectInfo.power_data.power_value_info.r_on_grid_vol                 =  gstUdpPcsSlavePara.pcs_info.power_info.r_on_grid_vol               ;   //R相并网口电压           //619B
    gstCollectInfo.power_data.power_value_info.r_on_grid_current             =  gstUdpPcsSlavePara.pcs_info.power_info.r_on_grid_current           ;   //R相并网口电流           //619C
    gstCollectInfo.power_data.power_value_info.r_on_grid_freq                =  gstUdpPcsSlavePara.pcs_info.power_info.r_on_grid_freq              ;   //R相并网口频率           //619D
    gstCollectInfo.power_data.power_value_info.r_on_grid_act_power           =  gstUdpPcsSlavePara.pcs_info.power_info.r_on_grid_act_power[0] << 16 |         
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_on_grid_act_power[1]      ;   //R相并网口有功功率       //619E~619F
    gstCollectInfo.power_data.power_value_info.r_on_grid_react_power         =  gstUdpPcsSlavePara.pcs_info.power_info.r_on_grid_react_power[0] << 16 |   
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_on_grid_react_power[1]    ;   //R相并网口无功功率       //61A0~61A1
    gstCollectInfo.power_data.power_value_info.r_on_grid_apparent_power      =  gstUdpPcsSlavePara.pcs_info.power_info.r_on_grid_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_on_grid_apparent_power[1] ;   //R相并网口视在功率       //61A2~61A3
    gstCollectInfo.power_data.power_value_info.s_on_grid_vol                 =  gstUdpPcsSlavePara.pcs_info.power_info.s_on_grid_vol               ;   //S相并网口电压           //61A4
    gstCollectInfo.power_data.power_value_info.s_on_grid_current             =  gstUdpPcsSlavePara.pcs_info.power_info.s_on_grid_current           ;   //S相并网口电流           //61A5
    gstCollectInfo.power_data.power_value_info.s_on_grid_freq                =  gstUdpPcsSlavePara.pcs_info.power_info.s_on_grid_freq              ;   //S相并网口频率           //61A6
    gstCollectInfo.power_data.power_value_info.s_on_grid_act_power           =  gstUdpPcsSlavePara.pcs_info.power_info.s_on_grid_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_on_grid_act_power[1]      ;   //S相并网口有功功率       //61A7~61A8
    gstCollectInfo.power_data.power_value_info.s_on_grid_react_power         =  gstUdpPcsSlavePara.pcs_info.power_info.s_on_grid_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_on_grid_react_power[1]    ;   //S相并网口无功功率       //61A9~61AA
    gstCollectInfo.power_data.power_value_info.s_on_grid_apparent_power      =  gstUdpPcsSlavePara.pcs_info.power_info.s_on_grid_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_on_grid_apparent_power[1] ;   //S相并网口视在功率       //61AB~61AC
    gstCollectInfo.power_data.power_value_info.t_on_grid_vol                 =  gstUdpPcsSlavePara.pcs_info.power_info.t_on_grid_vol               ;   //T相并网口电压           //61AD
    gstCollectInfo.power_data.power_value_info.t_on_grid_current             =  gstUdpPcsSlavePara.pcs_info.power_info.t_on_grid_current           ;   //T相并网口电流           //61AE
    gstCollectInfo.power_data.power_value_info.t_on_grid_freq                =  gstUdpPcsSlavePara.pcs_info.power_info.t_on_grid_freq              ;   //T相并网口频率           //61AF
    gstCollectInfo.power_data.power_value_info.t_on_grid_act_power           =  gstUdpPcsSlavePara.pcs_info.power_info.t_on_grid_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_on_grid_act_power[1]      ;   //T相并网口有功功率       //61B0~61B1
    gstCollectInfo.power_data.power_value_info.t_on_grid_react_power         =  gstUdpPcsSlavePara.pcs_info.power_info.t_on_grid_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_on_grid_react_power[1]    ;   //T相并网口无功功率       //61B2~61B3
    gstCollectInfo.power_data.power_value_info.t_on_grid_apparent_power      =  gstUdpPcsSlavePara.pcs_info.power_info.t_on_grid_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_on_grid_apparent_power[1] ;   //T相并网口视在功率       //61B4~61B5
                                                                                                                      
    gstCollectInfo.power_data.power_value_info.r_gen_vol                     =  gstUdpPcsSlavePara.pcs_info.power_info.r_gen_vol                   ;   //R相发电机电压           //61B6
    gstCollectInfo.power_data.power_value_info.r_gen_current                 =  gstUdpPcsSlavePara.pcs_info.power_info.r_gen_current               ;   //R相发电机电流           //61B7
    gstCollectInfo.power_data.power_value_info.r_gen_freq                    =  gstUdpPcsSlavePara.pcs_info.power_info.r_gen_freq                  ;   //R相发电机频率           //61B8
    gstCollectInfo.power_data.power_value_info.r_gen_act_power               =  gstUdpPcsSlavePara.pcs_info.power_info.r_gen_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_gen_act_power[1]          ;   //R相发电机有功功率       //61B9~61BA
    gstCollectInfo.power_data.power_value_info.r_gen_react_power             =  gstUdpPcsSlavePara.pcs_info.power_info.r_gen_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_gen_react_power[1]        ;   //R相发电机无功功率       //61BB~61BC
    gstCollectInfo.power_data.power_value_info.r_gen_apparent_power          =  gstUdpPcsSlavePara.pcs_info.power_info.r_gen_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_gen_apparent_power[1]     ;   //R相发电机视在功率       //61BD~61BE
    gstCollectInfo.power_data.power_value_info.s_gen_vol                     =  gstUdpPcsSlavePara.pcs_info.power_info.s_gen_vol                   ;   //S相发电机电压           //61BF
    gstCollectInfo.power_data.power_value_info.s_gen_current                 =  gstUdpPcsSlavePara.pcs_info.power_info.s_gen_current               ;   //S相发电机电流           //61C0
    gstCollectInfo.power_data.power_value_info.s_gen_freq                    =  gstUdpPcsSlavePara.pcs_info.power_info.s_gen_freq                  ;   //S相发电机频率           //61C1
    gstCollectInfo.power_data.power_value_info.s_gen_act_power               =  gstUdpPcsSlavePara.pcs_info.power_info.s_gen_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_gen_act_power[1]          ;   //S相发电机有功功率       //61C2~61C3
    gstCollectInfo.power_data.power_value_info.s_gen_react_power             =  gstUdpPcsSlavePara.pcs_info.power_info.s_gen_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_gen_react_power[1]        ;   //S相发电机无功功率       //61C4~61C5
    gstCollectInfo.power_data.power_value_info.s_gen_apparent_power          =  gstUdpPcsSlavePara.pcs_info.power_info.s_gen_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_gen_apparent_power[1]     ;   //S相发电机视在功率       //61C6~61C7
    gstCollectInfo.power_data.power_value_info.t_gen_vol                     =  gstUdpPcsSlavePara.pcs_info.power_info.t_gen_vol                   ;   //T相发电机电压           //61C8
    gstCollectInfo.power_data.power_value_info.t_gen_current                 =  gstUdpPcsSlavePara.pcs_info.power_info.t_gen_current               ;   //T相发电机电流           //61C9
    gstCollectInfo.power_data.power_value_info.t_gen_freq                    =  gstUdpPcsSlavePara.pcs_info.power_info.t_gen_freq                  ;   //T相发电机频率           //61CA
    gstCollectInfo.power_data.power_value_info.t_gen_act_power               =  gstUdpPcsSlavePara.pcs_info.power_info.t_gen_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_gen_act_power[1]          ;   //T相发电机有功功率       //61CB~61CC
    gstCollectInfo.power_data.power_value_info.t_gen_react_power             =  gstUdpPcsSlavePara.pcs_info.power_info.t_gen_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_gen_react_power[1]        ;   //T相发电机无功功率       //61CD~61CE
    gstCollectInfo.power_data.power_value_info.t_gen_apparent_power          =  gstUdpPcsSlavePara.pcs_info.power_info.t_gen_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_gen_apparent_power[1]     ;   //T相发电机视在功率       //61CF~61D0
                                                                                                                      
    gstCollectInfo.power_data.power_value_info.r_backup_vol                  =  gstUdpPcsSlavePara.pcs_info.power_info.r_backup_vol                ;   //R相负载电压             //61D1
    gstCollectInfo.power_data.power_value_info.r_backup_current              =  gstUdpPcsSlavePara.pcs_info.power_info.r_backup_current            ;   //R相负载电流             //61D2
    gstCollectInfo.power_data.power_value_info.r_backup_freq                 =  gstUdpPcsSlavePara.pcs_info.power_info.r_backup_freq               ;   //R相负载频率             //61D3
    gstCollectInfo.power_data.power_value_info.r_backup_act_power            =  gstUdpPcsSlavePara.pcs_info.power_info.r_backup_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_backup_act_power[1]       ;   //R相负载有功功率         //61D4~61D5
    gstCollectInfo.power_data.power_value_info.r_backup_react_power          =  gstUdpPcsSlavePara.pcs_info.power_info.r_backup_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_backup_react_power[1]     ;   //R相负载无功功率         //61D6~61D7
    gstCollectInfo.power_data.power_value_info.r_backup_apparent_power       =  gstUdpPcsSlavePara.pcs_info.power_info.r_backup_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_backup_apparent_power[1]  ;   //R相负载视在功率         //61D8~61D9
    gstCollectInfo.power_data.power_value_info.s_backup_vol                  =  gstUdpPcsSlavePara.pcs_info.power_info.s_backup_vol                ;   //S相负载电压             //61DA
    gstCollectInfo.power_data.power_value_info.s_backup_current              =  gstUdpPcsSlavePara.pcs_info.power_info.s_backup_current            ;   //S相负载电流             //61DB
    gstCollectInfo.power_data.power_value_info.s_backup_freq                 =  gstUdpPcsSlavePara.pcs_info.power_info.s_backup_freq               ;   //S相负载频率             //61DC
    gstCollectInfo.power_data.power_value_info.s_backup_act_power            =  gstUdpPcsSlavePara.pcs_info.power_info.s_backup_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_backup_act_power[1]       ;   //S相负载有功功率         //61DD~61DE
    gstCollectInfo.power_data.power_value_info.s_backup_react_power          =  gstUdpPcsSlavePara.pcs_info.power_info.s_backup_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_backup_act_power[1]       ;   //S相负载无功功率         //61DF~61E0
    gstCollectInfo.power_data.power_value_info.s_backup_apparent_power       =  gstUdpPcsSlavePara.pcs_info.power_info.s_backup_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_backup_apparent_power[1]  ;   //S相负载视在功率         //61E1~61E2
    gstCollectInfo.power_data.power_value_info.t_backup_vol                  =  gstUdpPcsSlavePara.pcs_info.power_info.t_backup_vol                ;   //T相负载电压            //61E3
    gstCollectInfo.power_data.power_value_info.t_backup_current              =  gstUdpPcsSlavePara.pcs_info.power_info.t_backup_current            ;   //T相负载电流            //61E4
    gstCollectInfo.power_data.power_value_info.t_backup_freq                 =  gstUdpPcsSlavePara.pcs_info.power_info.t_backup_freq               ;   //T相负载频率            //61E5
    gstCollectInfo.power_data.power_value_info.t_backup_act_power            =  gstUdpPcsSlavePara.pcs_info.power_info.t_backup_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_backup_act_power[1]       ;   //T相负载有功功率        //61E6~61E7
    gstCollectInfo.power_data.power_value_info.t_backup_react_power          =  gstUdpPcsSlavePara.pcs_info.power_info.t_backup_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_backup_react_power[1]     ;   //T相负载无功功率        //61E8~61E9
    gstCollectInfo.power_data.power_value_info.t_backup_apparent_power       =  gstUdpPcsSlavePara.pcs_info.power_info.t_backup_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_backup_apparent_power[1]  ;   //T相负载视在功率        //61EA~61EB
                                                                                                                      
    gstCollectInfo.power_data.power_value_info.r_nload_vol                   =  gstUdpPcsSlavePara.pcs_info.power_info.r_nload_vol                 ;   //R相远端负载电压        //61EC
    gstCollectInfo.power_data.power_value_info.r_nload_current               =  gstUdpPcsSlavePara.pcs_info.power_info.r_nload_current             ;   //R相远端负载电流        //61ED
    gstCollectInfo.power_data.power_value_info.r_nload_freq                  =  gstUdpPcsSlavePara.pcs_info.power_info.r_nload_freq                ;   //R相远端负载频率        //61EE
    gstCollectInfo.power_data.power_value_info.r_nload_act_power             =  gstUdpPcsSlavePara.pcs_info.power_info.r_nload_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_nload_act_power[1]        ;   //R相远端负载有功功率    //61EF~61F0
    gstCollectInfo.power_data.power_value_info.r_nload_react_power           =  gstUdpPcsSlavePara.pcs_info.power_info.r_nload_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_nload_react_power[1];   //R相远端负载无功功率    //61F1~61F2
    gstCollectInfo.power_data.power_value_info.r_nload_apparent_power        =  gstUdpPcsSlavePara.pcs_info.power_info.r_nload_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_nload_apparent_power[1]   ;   //R相远端负载视在功率    //61F3~61F4
    gstCollectInfo.power_data.power_value_info.s_nload_vol                   =  gstUdpPcsSlavePara.pcs_info.power_info.s_nload_vol                 ;   //S相远端负载电压        //61F5
    gstCollectInfo.power_data.power_value_info.s_nload_current               =  gstUdpPcsSlavePara.pcs_info.power_info.s_nload_current             ;   //S相远端负载电流        //61F6
    gstCollectInfo.power_data.power_value_info.s_nload_freq                  =  gstUdpPcsSlavePara.pcs_info.power_info.s_nload_freq                ;   //S相远端负载频率        //61F7
    gstCollectInfo.power_data.power_value_info.s_nload_act_power             =  gstUdpPcsSlavePara.pcs_info.power_info.s_nload_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_nload_act_power[1]        ;   //S相远端负载有功功率    //61F8~61F9
    gstCollectInfo.power_data.power_value_info.s_nload_react_power           =  gstUdpPcsSlavePara.pcs_info.power_info.s_nload_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_nload_react_power[1]      ;   //S相远端负载无功功率    //61FA~61FB
    gstCollectInfo.power_data.power_value_info.s_nload_apparent_power        =  gstUdpPcsSlavePara.pcs_info.power_info.s_nload_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_nload_apparent_power[1]   ;   //S相远端负载视在功率    //61FC~61FD
    gstCollectInfo.power_data.power_value_info.t_nload_vol                   =  gstUdpPcsSlavePara.pcs_info.power_info.t_nload_vol                 ;   //T相远端负载电压        //61FE
    gstCollectInfo.power_data.power_value_info.t_nload_current               =  gstUdpPcsSlavePara.pcs_info.power_info.t_nload_current             ;   //T相远端负载电流        //61FF
    gstCollectInfo.power_data.power_value_info.t_nload_freq                  =  gstUdpPcsSlavePara.pcs_info.power_info.t_nload_freq                ;   //T相远端负载频率        //6200
    gstCollectInfo.power_data.power_value_info.t_nload_act_power             =  gstUdpPcsSlavePara.pcs_info.power_info.t_nload_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_nload_act_power[1]        ;   //T相远端负载有功功率    //6201~6202
    gstCollectInfo.power_data.power_value_info.t_nload_react_power           =  gstUdpPcsSlavePara.pcs_info.power_info.t_nload_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_nload_react_power[1]     ;    //T相远端负载无功功率    //6203~6204
    gstCollectInfo.power_data.power_value_info.t_nload_apparent_power        =  gstUdpPcsSlavePara.pcs_info.power_info.t_nload_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_nload_apparent_power[1]   ;   //T相远端负载视在功率    //6205~6206
                                                                                                                      
    gstCollectInfo.power_data.power_value_info.r_third_party_vol             =  gstUdpPcsSlavePara.pcs_info.power_info.r_third_party_vol           ;   //R相三方逆变器电压      //6207
    gstCollectInfo.power_data.power_value_info.r_third_party_current         =  gstUdpPcsSlavePara.pcs_info.power_info.r_third_party_current       ;   //R相三方逆变器电流      //6208
    gstCollectInfo.power_data.power_value_info.r_third_party_freq            =  gstUdpPcsSlavePara.pcs_info.power_info.r_third_party_freq          ;   //R相三方逆变器频率      //6209
    gstCollectInfo.power_data.power_value_info.r_third_party_act_power       =  gstUdpPcsSlavePara.pcs_info.power_info.r_third_party_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_third_party_act_power[1];   //R相三方逆变器有功功率  //620A~620B
    gstCollectInfo.power_data.power_value_info.r_third_party_react_power     =  gstUdpPcsSlavePara.pcs_info.power_info.r_third_party_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_third_party_react_power[1];   //R相三方逆变器无功功率  //620C~620D
    gstCollectInfo.power_data.power_value_info.r_third_party_apparent_power  =  gstUdpPcsSlavePara.pcs_info.power_info.r_third_party_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_third_party_apparent_power[1];   //R相三方逆变器视在功率  //620E~620F
    gstCollectInfo.power_data.power_value_info.s_third_party_vol             =  gstUdpPcsSlavePara.pcs_info.power_info.s_third_party_vol           ;   //S相三方逆变器电压      //6210
    gstCollectInfo.power_data.power_value_info.s_third_party_current         =  gstUdpPcsSlavePara.pcs_info.power_info.s_third_party_current       ;   //S相三方逆变器电流      //6211
    gstCollectInfo.power_data.power_value_info.s_third_party_freq            =  gstUdpPcsSlavePara.pcs_info.power_info.s_third_party_freq          ;   //S相三方逆变器频率      //6212
    gstCollectInfo.power_data.power_value_info.s_third_party_act_power       =  gstUdpPcsSlavePara.pcs_info.power_info.s_third_party_act_power[0] << 16 | 
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_third_party_act_power[1];   //S相三方逆变器有功功率  //6213~6214
    gstCollectInfo.power_data.power_value_info.s_third_party_react_power     =  gstUdpPcsSlavePara.pcs_info.power_info.s_third_party_react_power[0] << 16 | 
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_third_party_react_power[1];   //S相三方逆变器无功功率  //6215~6216
    gstCollectInfo.power_data.power_value_info.s_third_party_apparent_power  =  gstUdpPcsSlavePara.pcs_info.power_info.s_third_party_apparent_power[0] << 16 | 
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_third_party_apparent_power[1];   //S相三方逆变器视在功率  //6217~6218
    gstCollectInfo.power_data.power_value_info.t_third_party_vol             =  gstUdpPcsSlavePara.pcs_info.power_info.t_third_party_vol           ;   //T相三方逆变器电压      //6219
    gstCollectInfo.power_data.power_value_info.t_third_party_current         =  gstUdpPcsSlavePara.pcs_info.power_info.t_third_party_current       ;   //T相三方逆变器电流      //621A
    gstCollectInfo.power_data.power_value_info.t_third_party_freq            =  gstUdpPcsSlavePara.pcs_info.power_info.t_third_party_freq          ;   //T相三方逆变器频率      //621B
    gstCollectInfo.power_data.power_value_info.t_third_party_act_power       =  gstUdpPcsSlavePara.pcs_info.power_info.t_third_party_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_third_party_act_power[1];   //T相三方逆变器有功功率  //621C~621D
    gstCollectInfo.power_data.power_value_info.t_third_party_react_power     =  gstUdpPcsSlavePara.pcs_info.power_info.t_third_party_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_third_party_react_power[1];   //T相三方逆变器无功功率  //621E~621F
    gstCollectInfo.power_data.power_value_info.t_third_party_apparent_power  =  gstUdpPcsSlavePara.pcs_info.power_info.t_third_party_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_third_party_apparent_power[1];   //T相三方逆变器视在功率  //6220~6221
                                                                                                                      
    gstCollectInfo.power_data.power_value_info.r_inv_vol                     =  gstUdpPcsSlavePara.pcs_info.power_info.r_inv_vol                   ;   //R相逆变侧电压          //6222
    gstCollectInfo.power_data.power_value_info.r_inv_current                 =  gstUdpPcsSlavePara.pcs_info.power_info.r_inv_current               ;   //R相逆变侧电流          //6223
    gstCollectInfo.power_data.power_value_info.r_inv_freq                    =  gstUdpPcsSlavePara.pcs_info.power_info.r_inv_freq                  ;   //R相逆变侧频率          //6224
    gstCollectInfo.power_data.power_value_info.r_inv_act_power               =  gstUdpPcsSlavePara.pcs_info.power_info.r_inv_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_inv_act_power[1];   //R相逆变侧有功功率      //6225~6226
    gstCollectInfo.power_data.power_value_info.r_inv_react_power             =  gstUdpPcsSlavePara.pcs_info.power_info.r_inv_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_inv_react_power[1];   //R相逆变侧无功功率      //6227~6228
    gstCollectInfo.power_data.power_value_info.r_inv_apparent_power          =  gstUdpPcsSlavePara.pcs_info.power_info.r_inv_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.r_inv_apparent_power[1];   //R相逆变侧视在功率      //6229~622A
    gstCollectInfo.power_data.power_value_info.s_inv_vol                     =  gstUdpPcsSlavePara.pcs_info.power_info.s_inv_vol                   ;   //S相逆变侧电压          //622B
    gstCollectInfo.power_data.power_value_info.s_inv_current                 =  gstUdpPcsSlavePara.pcs_info.power_info.s_inv_current               ;   //S相逆变侧电流          //622C
    gstCollectInfo.power_data.power_value_info.s_inv_freq                    =  gstUdpPcsSlavePara.pcs_info.power_info.s_inv_freq                  ;   //S相逆变侧频率          //622D
    gstCollectInfo.power_data.power_value_info.s_inv_act_power               =  gstUdpPcsSlavePara.pcs_info.power_info.s_inv_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_inv_act_power[1]          ;   //S相逆变侧有功功率      //622E~622F
    gstCollectInfo.power_data.power_value_info.s_inv_react_power             =  gstUdpPcsSlavePara.pcs_info.power_info.s_inv_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_inv_react_power[1]        ;   //S相逆变侧无功功率      //6230~6231
    gstCollectInfo.power_data.power_value_info.s_inv_apparent_power          =  gstUdpPcsSlavePara.pcs_info.power_info.s_inv_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.s_inv_apparent_power[1]     ;   //S相逆变侧视在功率      //6232~6233
    gstCollectInfo.power_data.power_value_info.t_inv_vol                     =  gstUdpPcsSlavePara.pcs_info.power_info.t_inv_vol                   ;   //T相逆变侧电压          //6234
    gstCollectInfo.power_data.power_value_info.t_inv_current                 =  gstUdpPcsSlavePara.pcs_info.power_info.t_inv_current               ;   //T相逆变侧电流          //6235
    gstCollectInfo.power_data.power_value_info.t_inv_freq                    =  gstUdpPcsSlavePara.pcs_info.power_info.t_inv_freq                  ;   //T相逆变侧频率          //6236
    gstCollectInfo.power_data.power_value_info.t_inv_act_power               =  gstUdpPcsSlavePara.pcs_info.power_info.t_inv_act_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_inv_act_power[1] ;   //T相逆变侧有功功率      //6237~6238
    gstCollectInfo.power_data.power_value_info.t_inv_react_power             =  gstUdpPcsSlavePara.pcs_info.power_info.t_inv_react_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_inv_react_power[1]     ;   //T相逆变侧无功功率      //6239~623A
    gstCollectInfo.power_data.power_value_info.t_inv_apparent_power          =  gstUdpPcsSlavePara.pcs_info.power_info.t_inv_apparent_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.t_inv_apparent_power[1] ;   //T相逆变侧视在功率      //623B~623C
                                                                                                                                                
                                                                                                                                                                                //623D~6257
    gstCollectInfo.power_data.power_value_info.inv_bat_vol                   =  gstUdpPcsSlavePara.pcs_info.power_info.inv_bat_vol                 ;   //电池侧电压             //6258
    gstCollectInfo.power_data.power_value_info.inv_bat_current               =  gstUdpPcsSlavePara.pcs_info.power_info.inv_bat_current             ;   //电池侧电流             //6259
    gstCollectInfo.power_data.power_value_info.inv_bat_power                 =  gstUdpPcsSlavePara.pcs_info.power_info.inv_bat_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.inv_bat_power[1]            ;   //电池侧频率             //625A~625B                         
    gstCollectInfo.power_data.power_value_info.inv_ev_vol                    =  gstUdpPcsSlavePara.pcs_info.power_info.inv_ev_vol                  ;   //EV侧电压               //625C
    gstCollectInfo.power_data.power_value_info.inv_ev_current                =  gstUdpPcsSlavePara.pcs_info.power_info.inv_ev_current              ;   //EV侧电流               //625D
    gstCollectInfo.power_data.power_value_info.inv_ev_power                  =  gstUdpPcsSlavePara.pcs_info.power_info.inv_ev_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.inv_ev_power[1]             ;   //EV侧频率               //625E~625F
    gstCollectInfo.power_data.power_value_info.pv_1_vol                      =  gstUdpPcsSlavePara.pcs_info.power_info.pv_1_vol                    ;   //PV1电压                //6260
    gstCollectInfo.power_data.power_value_info.pv_1_current                  =  gstUdpPcsSlavePara.pcs_info.power_info.pv_1_current                ;   //PV1电流                //6261
    gstCollectInfo.power_data.power_value_info.pv_1_power                    =  gstUdpPcsSlavePara.pcs_info.power_info.pv_1_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.pv_1_power[1]               ;   //PV1频率                //6262~6263
    gstCollectInfo.power_data.power_value_info.pv_2_vol                      =  gstUdpPcsSlavePara.pcs_info.power_info.pv_2_vol                    ;   //PV2电压                //6264
    gstCollectInfo.power_data.power_value_info.pv_2_current                  =  gstUdpPcsSlavePara.pcs_info.power_info.pv_2_current                ;   //PV2电流                //6265
    gstCollectInfo.power_data.power_value_info.pv_2_power                    =  gstUdpPcsSlavePara.pcs_info.power_info.pv_2_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.pv_2_power[1]               ;   //PV2频率                //6266~6267
    gstCollectInfo.power_data.power_value_info.pv_3_vol                      =  gstUdpPcsSlavePara.pcs_info.power_info.pv_3_vol                    ;   //PV3电压                //6268
    gstCollectInfo.power_data.power_value_info.pv_3_current                  =  gstUdpPcsSlavePara.pcs_info.power_info.pv_3_current                ;   //PV3电流                //6269
    gstCollectInfo.power_data.power_value_info.pv_3_power                    =  gstUdpPcsSlavePara.pcs_info.power_info.pv_3_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.pv_3_power[1]               ;   //PV3频率                //626A~626B
    gstCollectInfo.power_data.power_value_info.pv_4_vol                      =  gstUdpPcsSlavePara.pcs_info.power_info.pv_4_vol                    ;   //PV4电压                //626C
    gstCollectInfo.power_data.power_value_info.pv_4_current                  =  gstUdpPcsSlavePara.pcs_info.power_info.pv_4_current                ;   //PV4电流                //626D
    gstCollectInfo.power_data.power_value_info.pv_4_power                    =  gstUdpPcsSlavePara.pcs_info.power_info.pv_4_power[0] << 16 |
                                                                                gstUdpPcsSlavePara.pcs_info.power_info.pv_4_power[1]               ;   //PV4频率                //626E~626F
                                                                                                                                                                                //6270~628F
    gstCollectInfo.power_data.power_value_info.bus_vol                       =  gstUdpPcsSlavePara.pcs_info.power_info.bus_vol                     ;   //母线电压               //6290
                                                                                                                                                                                //6291
    gstCollectInfo.power_data.power_value_info.phase_seq                     =  gstUdpPcsSlavePara.pcs_info.power_info.phase_seq                   ;   //市电相序               //6292
                                                                                                        //6293~62FF

    /*----功率配置信息---*/
    gstCollectInfo.power_data.power_set_info.work_mode                     =  gstUdpPcsSlavePara.pcs_info.set_info.work_mode                     ; //6300
    gstCollectInfo.power_data.power_set_info.gen_mode                      =  gstUdpPcsSlavePara.pcs_info.set_info.gen_mode                      ; //6301
    gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_current       =  gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_chg_current       ; //6302
    gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_power         =  gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_chg_power[0] << 16 |
                                                                              gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_chg_power[1]      ; //6303~6304
    gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_current       =  gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_dsg_current       ; //6305
    gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_power         =  gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_dsg_power[0] << 16 | 
                                                                              gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_dsg_power[1]     ; //6306~6307
    gstCollectInfo.power_data.power_set_info.inv_pv_max_power              =  gstUdpPcsSlavePara.pcs_info.set_info.inv_pv_max_power[0] << 16 | 
                                                                              gstUdpPcsSlavePara.pcs_info.set_info.inv_pv_max_power[1]          ; //6308~6309
    gstCollectInfo.power_data.power_set_info.inv_on_grid_in_max_power      =  gstUdpPcsSlavePara.pcs_info.set_info.inv_on_grid_in_max_power[0] << 16 | 
                                                                              gstUdpPcsSlavePara.pcs_info.set_info.inv_on_grid_in_max_power[1]  ; //630A~630B
    gstCollectInfo.power_data.power_set_info.inv_on_grid_out_max_power     =  gstUdpPcsSlavePara.pcs_info.set_info.inv_on_grid_out_max_power[0] << 16 | 
                                                                              gstUdpPcsSlavePara.pcs_info.set_info.inv_on_grid_out_max_power[1] ; //630C~630D
    gstCollectInfo.power_data.power_set_info.inv_gen_max_power             =  gstUdpPcsSlavePara.pcs_info.set_info.inv_gen_max_power[0] << 16 | 
                                                                              gstUdpPcsSlavePara.pcs_info.set_info.inv_gen_max_power[1]         ; //630E~630F
    gstCollectInfo.power_data.power_set_info.inv_ev_in_max_power           =  gstUdpPcsSlavePara.pcs_info.set_info.inv_ev_in_max_power[0] << 16 | 
                                                                              gstUdpPcsSlavePara.pcs_info.set_info.inv_ev_in_max_power[1]       ; //6310~6311
    gstCollectInfo.power_data.power_set_info.inv_ev_out_max_power          =  gstUdpPcsSlavePara.pcs_info.set_info.inv_ev_out_max_power[0] << 16 | 
                                                                              gstUdpPcsSlavePara.pcs_info.set_info.inv_ev_out_max_power[1]      ; //6312~6313
    gstCollectInfo.power_data.power_set_info.network_max_power_limit       =  gstUdpPcsSlavePara.pcs_info.set_info.network_max_power_limit[0] << 16 | 
                                                                              gstUdpPcsSlavePara.pcs_info.set_info.network_max_power_limit[1]   ; //6314~6315
    gstCollectInfo.power_data.power_set_info.on_grid_max_power             =  gstUdpPcsSlavePara.pcs_info.set_info.on_grid_max_power[0] << 16 | 
                                                                              gstUdpPcsSlavePara.pcs_info.set_info.on_grid_max_power[1]         ; //6316~6317
                                                                                                                                                   //6318~6321
    gstCollectInfo.power_data.power_set_info.pv_input_mode                 =  gstUdpPcsSlavePara.pcs_info.set_info.pv_input_mode                 ; //6322
    gstCollectInfo.power_data.power_set_info.prohibit_reverse_flow_enable  =  gstUdpPcsSlavePara.pcs_info.set_info.prohibit_reverse_flow_enable  ; //6323
                                                                                                                                                   //6324~6325
    gstCollectInfo.power_data.power_set_info.x_boost_enable                =  gstUdpPcsSlavePara.pcs_info.set_info.x_boost_enable                ; //6326
    gstCollectInfo.power_data.power_set_info.ups_0_delay_enable            =  gstUdpPcsSlavePara.pcs_info.set_info.ups_0_delay_enable            ; //6327
    gstCollectInfo.power_data.power_set_info.backup_enable                 =  gstUdpPcsSlavePara.pcs_info.set_info.backup_enable                 ; //6328
    gstCollectInfo.power_data.power_set_info.program_ctrl_enable           =  gstUdpPcsSlavePara.pcs_info.set_info.program_ctrl_enable           ; //6329
    gstCollectInfo.power_data.power_set_info.over_vol_derating_enable      =  gstUdpPcsSlavePara.pcs_info.set_info.over_vol_derating_enable      ; //632A
    gstCollectInfo.power_data.power_set_info.over_freq_derating_enable     =  gstUdpPcsSlavePara.pcs_info.set_info.over_freq_derating_enable     ; //632B
    gstCollectInfo.power_data.power_set_info.under_vol_derating_enable     =  gstUdpPcsSlavePara.pcs_info.set_info.under_vol_derating_enable     ; //632C
    gstCollectInfo.power_data.power_set_info.under_freq_derating_enable    =  gstUdpPcsSlavePara.pcs_info.set_info.under_freq_derating_enable    ; //632D
    gstCollectInfo.power_data.power_set_info.island_enable                 =  gstUdpPcsSlavePara.pcs_info.set_info.island_enable                 ; //632E
    gstCollectInfo.power_data.power_set_info.ac_coupling_enable            =  gstUdpPcsSlavePara.pcs_info.set_info.ac_coupling_enable            ; //632F
    gstCollectInfo.power_data.power_set_info.high_low_crossover_enable     =  gstUdpPcsSlavePara.pcs_info.set_info.high_low_crossover_enable     ; //6330
                                                                                                                                                 
    gstCollectInfo.power_data.power_set_info.ct_or_meter                   =  gstUdpPcsSlavePara.pcs_info.set_info.ct_or_meter                   ; //6338
    gstCollectInfo.power_data.power_set_info.ct_dir                        =  gstUdpPcsSlavePara.pcs_info.set_info.ct_dir                        ; //6339
    gstCollectInfo.power_data.power_set_info.meter_brand                   =  gstUdpPcsSlavePara.pcs_info.set_info.meter_brand                   ; //633A
    gstCollectInfo.power_data.power_set_info.grid_standard                 =  gstUdpPcsSlavePara.pcs_info.set_info.grid_standard                 ; //633B
    gstCollectInfo.power_data.power_set_info.grid_sub_standard             =  gstUdpPcsSlavePara.pcs_info.set_info.grid_sub_standard             ; //633C
    gstCollectInfo.power_data.power_set_info.bat_chg_max_soc               =  gstUdpPcsSlavePara.pcs_info.set_info.bat_chg_max_soc               ; //633D
    gstCollectInfo.power_data.power_set_info.on_grid_bat_dsg_min_soc       =  gstUdpPcsSlavePara.pcs_info.set_info.on_grid_bat_dsg_min_soc       ; //633E
    gstCollectInfo.power_data.power_set_info.off_grid_bat_dsg_min_soc      =  gstUdpPcsSlavePara.pcs_info.set_info.off_grid_bat_dsg_min_soc      ; //633F
                                                                                                                                                    //6340~6348
    gstCollectInfo.power_data.power_set_info.inv_factory_reset             =  gstUdpPcsSlavePara.pcs_info.set_info.inv_factory_reset             ; //6349
    gstCollectInfo.power_data.power_set_info.on_grid_reconnect_time        =  gstUdpPcsSlavePara.pcs_info.set_info.on_grid_reconnect_time        ; //634A
    gstCollectInfo.power_data.power_set_info.reactive_response_time        =  gstUdpPcsSlavePara.pcs_info.set_info.reactive_response_time        ; //634B
    gstCollectInfo.power_data.power_set_info.reactive_set                  =  gstUdpPcsSlavePara.pcs_info.set_info.reactive_set                  ; //634C
    gstCollectInfo.power_data.power_set_info.reactive_ctrl_type            =  gstUdpPcsSlavePara.pcs_info.set_info.reactive_ctrl_type            ; //634D
    gstCollectInfo.power_data.power_set_info.active_set                    =  gstUdpPcsSlavePara.pcs_info.set_info.active_set                    ; //634E
    gstCollectInfo.power_data.power_set_info.power_factor_set              =  gstUdpPcsSlavePara.pcs_info.set_info.power_factor_set              ; //634F
    gstCollectInfo.power_data.power_set_info.certification_test_switch     =  gstUdpPcsSlavePara.pcs_info.set_info.certification_test_switch     ; //63F0                                                                                                                                         //6350~63FF

    /*----功率阈值信息---*/
    gstCollectInfo.power_data.limit_info.bat_under_vol_threshold       =  gstUdpPcsSlavePara.pcs_info.limit_info.bat_under_vol_threshold     ; //6400
    gstCollectInfo.power_data.limit_info.bat_over_vol_threshold        =  gstUdpPcsSlavePara.pcs_info.limit_info.bat_over_vol_threshold      ; //6401
    gstCollectInfo.power_data.limit_info.pv_under_vol_threshold        =  gstUdpPcsSlavePara.pcs_info.limit_info.pv_under_vol_threshold      ; //6402
    gstCollectInfo.power_data.limit_info.pv_over_vol_threshold         =  gstUdpPcsSlavePara.pcs_info.limit_info.pv_over_vol_threshold       ; //6403
    gstCollectInfo.power_data.limit_info.ev_under_vol_threshold        =  gstUdpPcsSlavePara.pcs_info.limit_info.ev_under_vol_threshold      ; //6404
    gstCollectInfo.power_data.limit_info.ev_over_vol_threshold         =  gstUdpPcsSlavePara.pcs_info.limit_info.ev_over_vol_threshold       ; //6405
    gstCollectInfo.power_data.limit_info.grid_under_vol_threshold      =  gstUdpPcsSlavePara.pcs_info.limit_info.grid_under_vol_threshold    ; //6406
    gstCollectInfo.power_data.limit_info.grid_over_vol_threshold       =  gstUdpPcsSlavePara.pcs_info.limit_info.grid_over_vol_threshold     ; //6407
    gstCollectInfo.power_data.limit_info.grid_under_freq_threshold     =  gstUdpPcsSlavePara.pcs_info.limit_info.grid_under_freq_threshold   ; //6408
    gstCollectInfo.power_data.limit_info.grid_over_freq_threshold      =  gstUdpPcsSlavePara.pcs_info.limit_info.grid_over_freq_threshold    ; //6409
    gstCollectInfo.power_data.limit_info.bypass_under_vol_threshold    =  gstUdpPcsSlavePara.pcs_info.limit_info.bypass_under_vol_threshold  ; //640A
    gstCollectInfo.power_data.limit_info.bypass_over_vol_threshold     =  gstUdpPcsSlavePara.pcs_info.limit_info.bypass_over_vol_threshold   ; //640B
    gstCollectInfo.power_data.limit_info.bypass_under_freq_threshold   =  gstUdpPcsSlavePara.pcs_info.limit_info.bypass_under_freq_threshold ; //640C
    gstCollectInfo.power_data.limit_info.bypass_over_freq_threshold    =  gstUdpPcsSlavePara.pcs_info.limit_info.bypass_over_freq_threshold  ; //640D
    gstCollectInfo.power_data.limit_info.over_vol_derating_point       =  gstUdpPcsSlavePara.pcs_info.limit_info.over_vol_derating_point     ; //640E
    gstCollectInfo.power_data.limit_info.over_freq_derating_point      =  gstUdpPcsSlavePara.pcs_info.limit_info.over_freq_derating_point    ; //640F
    gstCollectInfo.power_data.limit_info.under_vol_derating_point      =  gstUdpPcsSlavePara.pcs_info.limit_info.under_vol_derating_point    ; //6410
    gstCollectInfo.power_data.limit_info.under_freq_derating_point     =  gstUdpPcsSlavePara.pcs_info.limit_info.under_freq_derating_point   ; //6411
                                                                                                                                               //6412~647F

    /*----功率采样校准信息---*/
    gstCollectInfo.power_data.calibrate_info.calibration_reset         = gstUdpPcsSlavePara.pcs_info.calibrate_info.calibration_reset           ; //6480
                                                                                                                                                //6481~6487
    gstCollectInfo.power_data.calibrate_info.inv_R_vol_calib_k         = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_R_vol_calib_k           ; //6488
    gstCollectInfo.power_data.calibrate_info.inv_R_vol_calib_b         = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_R_vol_calib_b           ; //6489
    gstCollectInfo.power_data.calibrate_info.inv_S_vol_calib_k         = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_S_vol_calib_k           ; //648A
    gstCollectInfo.power_data.calibrate_info.inv_S_vol_calib_b         = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_S_vol_calib_b           ; //648B
    gstCollectInfo.power_data.calibrate_info.inv_T_vol_calib_k         = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_T_vol_calib_k           ; //648C
    gstCollectInfo.power_data.calibrate_info.inv_T_vol_calib_b         = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_T_vol_calib_b           ; //648D
    gstCollectInfo.power_data.calibrate_info.inv_R_current_calib_k     = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_R_current_calib_k       ; //648E
    gstCollectInfo.power_data.calibrate_info.inv_R_current_calib_b     = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_R_current_calib_b       ; //648F
    gstCollectInfo.power_data.calibrate_info.inv_S_current_calib_k     = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_S_current_calib_k       ; //6490
    gstCollectInfo.power_data.calibrate_info.inv_S_current_calib_b     = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_S_current_calib_b       ; //6491
    gstCollectInfo.power_data.calibrate_info.inv_T_current_calib_k     = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_T_current_calib_k       ; //6492
    gstCollectInfo.power_data.calibrate_info.inv_T_current_calib_b     = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_T_current_calib_b       ; //6493
    gstCollectInfo.power_data.calibrate_info.inv_bat_vol_calib_k       = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_bat_vol_calib_k         ; //6494
    gstCollectInfo.power_data.calibrate_info.inv_bat_vol_calib_b       = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_bat_vol_calib_b         ; //6495
    gstCollectInfo.power_data.calibrate_info.inv_bat_current_calib_k   = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_bat_current_calib_k     ; //6496
    gstCollectInfo.power_data.calibrate_info.inv_bat_current_calib_b   = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_bat_current_calib_b     ; //6497
    gstCollectInfo.power_data.calibrate_info.inv_ev_vol_calib_k        = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_ev_vol_calib_k          ; //6498
    gstCollectInfo.power_data.calibrate_info.inv_ev_vol_calib_b        = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_ev_vol_calib_b          ; //6499
    gstCollectInfo.power_data.calibrate_info.inv_ev_current_calib_k    = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_ev_current_calib_k      ; //649A
    gstCollectInfo.power_data.calibrate_info.inv_ev_current_calib_b    = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_ev_current_calib_b      ; //649B
    gstCollectInfo.power_data.calibrate_info.inv_pv_1_vol_calib_k      = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_1_vol_calib_k        ; //649C
    gstCollectInfo.power_data.calibrate_info.inv_pv_1_vol_calib_b      = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_1_vol_calib_b        ; //649D
    gstCollectInfo.power_data.calibrate_info.inv_pv_1_current_calib_k  = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_1_current_calib_k    ; //649E
    gstCollectInfo.power_data.calibrate_info.inv_pv_1_current_calib_b  = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_1_current_calib_b    ; //649F
    gstCollectInfo.power_data.calibrate_info.inv_pv_2_vol_calib_k      = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_2_vol_calib_k        ; //64A0
    gstCollectInfo.power_data.calibrate_info.inv_pv_2_vol_calib_b      = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_2_vol_calib_b        ; //64A1
    gstCollectInfo.power_data.calibrate_info.inv_pv_2_current_calib_k  = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_2_current_calib_k    ; //64A2
    gstCollectInfo.power_data.calibrate_info.inv_pv_2_current_calib_b  = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_2_current_calib_b    ; //64A3
    gstCollectInfo.power_data.calibrate_info.inv_pv_3_vol_calib_k      = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_3_vol_calib_k        ; //64A4
    gstCollectInfo.power_data.calibrate_info.inv_pv_3_vol_calib_b      = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_3_vol_calib_b        ; //64A5
    gstCollectInfo.power_data.calibrate_info.inv_pv_3_current_calib_k  = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_3_current_calib_k    ; //64A6
    gstCollectInfo.power_data.calibrate_info.inv_pv_3_current_calib_b  = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_3_current_calib_b    ; //64A7
    gstCollectInfo.power_data.calibrate_info.inv_pv_4_vol_calib_k      = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_4_vol_calib_k        ; //64A8
    gstCollectInfo.power_data.calibrate_info.inv_pv_4_vol_calib_b      = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_4_vol_calib_b        ; //64A9
    gstCollectInfo.power_data.calibrate_info.inv_pv_4_current_calib_k  = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_4_current_calib_k    ; //64AA
    gstCollectInfo.power_data.calibrate_info.inv_pv_4_current_calib_b  = gstUdpPcsSlavePara.pcs_info.calibrate_info.inv_pv_4_current_calib_b    ; //64AB
                                                                                                                                                  //64AC~64FF                       
                                               

    /*----功率内部信息---*/
    gstCollectInfo.power_data.inside_info.pcs_chg_ctrl = gstUdpPcsSlavePara.pcs_info.control_info.chg_ctrl;
    gstCollectInfo.power_data.inside_info.pcs_dsg_ctrl = gstUdpPcsSlavePara.pcs_info.control_info.dsg_ctrl;
    gstCollectInfo.power_data.inside_info.pf_switch    = gstUdpPcsSlavePara.pcs_info.control_info.pf_switch;
    gstCollectInfo.power_data.inside_info.pv_switch    = gstUdpPcsSlavePara.pcs_info.control_info.pv_switch;
    gstCollectInfo.power_data.inside_info.ignor_ct_err = gstUdpPcsSlavePara.pcs_info.control_info.ignore_ct_error;
    
//    uint16_t    dsp_wake_en;
//    uint16_t    dsp_wake_up_status;
//    uint16_t    ignor_ct_err;
//    CurDirection_un  cur_direct_status;         //电流方向
//    uint16_t    drm_enable;
    /*----拉弧信息---*/
    
}

/**
  * @brief  获取单Pack数据
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void _GetSignalPackDataFunc(void)
{
    for(int i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        /*----Pack基础信息---*/
        gstCollectInfo.pack_data[i].base_info.model_id       =    gstPackSlavePara.pack_para[i].bms_info.base_info.model_id             ;  //7000
        memcpy(gstCollectInfo.pack_data[i].base_info.sn, gstPackSlavePara.pack_para[i].bms_info.base_info.sn, 10)                       ;  //7001~700A
        gstCollectInfo.pack_data[i].base_info.hw_ver         =    gstPackSlavePara.pack_para[i].bms_info.base_info.hw_ver               ;  //700B 
        gstCollectInfo.pack_data[i].base_info.sw_ver         =   (gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver[0] << 16 |
                                                                  gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver[1])           ;  //700C~700D
        memcpy(gstCollectInfo.pack_data[i].base_info.sw_ver_ext, gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver_ext, 12)       ;  //700E~7019
                                                                                                                                           //701D~701F
                                                                                                                                           //7020~703F
        /*----Pack状态信息---*/
        gstCollectInfo.pack_data[i].state_info.addr             = gstPackSlavePara.pack_para[i].bms_info.status_info.addr                       ;  //7040
        gstCollectInfo.pack_data[i].state_info.work_status      = gstPackSlavePara.pack_para[i].bms_info.status_info.work_status                ;  //7041
        gstCollectInfo.pack_data[i].state_info.total_vol        = gstPackSlavePara.pack_para[i].bms_info.status_info.total_vol                  ;  //7042
        gstCollectInfo.pack_data[i].state_info.current          = gstPackSlavePara.pack_para[i].bms_info.status_info.total_curr                 ;  //7043
        gstCollectInfo.pack_data[i].state_info.soc_real         = gstPackSlavePara.pack_para[i].bms_info.status_info.soc_real                   ;  //7044
        gstCollectInfo.pack_data[i].state_info.soc_disp         = gstPackSlavePara.pack_para[i].bms_info.status_info.soc_disp                   ;  //7045
        gstCollectInfo.pack_data[i].state_info.soh              = gstPackSlavePara.pack_para[i].bms_info.status_info.soh                        ;  //7046
        gstCollectInfo.pack_data[i].state_info.max_chg_current  = gstPackSlavePara.pack_para[i].bms_info.status_info.max_chg_allow_curr         ;  //7047
        gstCollectInfo.pack_data[i].state_info.max_dsg_current  = gstPackSlavePara.pack_para[i].bms_info.status_info.max_dsg_allow_curr         ;  //7048
                                                                                                                                                //7049~704A
        gstCollectInfo.pack_data[i].state_info.design_cap       = gstPackSlavePara.pack_para[i].bms_info.status_info.design_cap                 ;  //704B
        gstCollectInfo.pack_data[i].state_info.total_cap        = gstPackSlavePara.pack_para[i].bms_info.status_info.total_cap                  ;  //704C
        gstCollectInfo.pack_data[i].state_info.residual_cap     = gstPackSlavePara.pack_para[i].bms_info.status_info.residual_cap               ;  //704D
        gstCollectInfo.pack_data[i].state_info.cycle_count      = gstPackSlavePara.pack_para[i].bms_info.status_info.cycle_count                ;  //704E
        gstCollectInfo.pack_data[i].state_info.accu_chg_energy  = gstPackSlavePara.pack_para[i].bms_info.status_info.accu_chg_energy[0] << 16 |    
                                                                  gstPackSlavePara.pack_para[i].bms_info.status_info.accu_chg_energy[1]         ;  //704F~7050
        gstCollectInfo.pack_data[i].state_info.accu_dsg_energy  = gstPackSlavePara.pack_para[i].bms_info.status_info.accu_dsg_energy[0] << 16 |    
                                                                  gstPackSlavePara.pack_para[i].bms_info.status_info.accu_dsg_energy[1]         ;  //7051~7052
        gstCollectInfo.pack_data[i].state_info.heat_status      = gstPackSlavePara.pack_para[i].bms_info.status_info.heat_status                ;  //7053
        gstCollectInfo.pack_data[i].state_info.mos_status       = gstPackSlavePara.pack_para[i].bms_info.status_info.mos_status                 ;  //7054
        gstCollectInfo.pack_data[i].state_info.cell_max_vol     = gstPackSlavePara.pack_para[i].bms_info.status_info.cell_max_vol               ;  //7055
        gstCollectInfo.pack_data[i].state_info.cell_min_vol     = gstPackSlavePara.pack_para[i].bms_info.status_info.cell_min_vol               ;  //7056
        gstCollectInfo.pack_data[i].state_info.cell_ave_vol     = gstPackSlavePara.pack_para[i].bms_info.status_info.cell_ave_vol               ;  //7057
        gstCollectInfo.pack_data[i].state_info.cell_max_temp    = gstPackSlavePara.pack_para[i].bms_info.status_info.cell_max_temp              ;  //7058
        gstCollectInfo.pack_data[i].state_info.cell_min_temp    = gstPackSlavePara.pack_para[i].bms_info.status_info.cell_min_temp              ;  //7059
        gstCollectInfo.pack_data[i].state_info.cell_ave_temp    = gstPackSlavePara.pack_para[i].bms_info.status_info.cell_ave_temp              ;  //705A
        gstCollectInfo.pack_data[i].state_info.mos_max_temp     = gstPackSlavePara.pack_para[i].bms_info.status_info.mos_max_temp               ;  //705B
        gstCollectInfo.pack_data[i].state_info.mos_min_temp     = gstPackSlavePara.pack_para[i].bms_info.status_info.mos_min_temp               ;  //705C
        gstCollectInfo.pack_data[i].state_info.mos_ave_temp     = gstPackSlavePara.pack_para[i].bms_info.status_info.mos_ave_temp               ;  //705D
        gstCollectInfo.pack_data[i].state_info.pack_max_temp    = gstPackSlavePara.pack_para[i].bms_info.status_info.pack_max_temp              ;  //705E
        gstCollectInfo.pack_data[i].state_info.pack_min_temp    = gstPackSlavePara.pack_para[i].bms_info.status_info.pack_min_temp              ;  //705F
        gstCollectInfo.pack_data[i].state_info.pack_ave_temp    = gstPackSlavePara.pack_para[i].bms_info.status_info.pack_ave_temp              ;  //7060
                                                                                                                   //7061~706C 
                                                                                                                                           //706D~7078                                                                                  
        gstCollectInfo.pack_data[i].state_info.balance_status   = gstPackSlavePara.pack_para[i].bms_info.status_info.balance_status[0] << 16 |
                                                                  gstPackSlavePara.pack_para[i].bms_info.status_info.balance_status[1]          ;  //7079~707A
                                        //707B~707F

        /*----Pack控制信息---*/
        gstCollectInfo.pack_data[i].ctrl_info.heat_ctrl          = gstPackSlavePara.pack_para[i].bms_info.ctrl_info.heat_ctrl           ;   //7080
        gstCollectInfo.pack_data[i].ctrl_info.dsg_mos_ctrl       = gstPackSlavePara.pack_para[i].bms_info.ctrl_info.dsg_mos_ctrl        ;   //7081
        gstCollectInfo.pack_data[i].ctrl_info.chg_mos_ctrl       = gstPackSlavePara.pack_para[i].bms_info.ctrl_info.chg_mos_ctrl        ;   //7082
        gstCollectInfo.pack_data[i].ctrl_info.pre_dsg_mos_ctrl   = gstPackSlavePara.pack_para[i].bms_info.ctrl_info.pre_dsg_mos_ctrl    ;   //7083
        gstCollectInfo.pack_data[i].ctrl_info.current_limit_ctrl = gstPackSlavePara.pack_para[i].bms_info.ctrl_info.curr_limit_ctrl     ;   //7084
                                        //7085~709F

        /*----Pack单体信息---*/                    
        memcpy(gstCollectInfo.pack_data[i].cell_info.cell_vol, gstPackSlavePara.pack_para[i].bms_info.cell_info.cell_vol, 16)           ;   //70A0~70BF
        memcpy(gstCollectInfo.pack_data[i].cell_info.cell_temp, gstPackSlavePara.pack_para[i].bms_info.cell_info.cell_temp, 16)         ;   //70C0~70DF
               
                                                                                                                                            //70E0~70FF
        gstCollectInfo.pack_data[i].inside_info.dcdc_forward_status = gstPackSlavePara.pack_para[i].dcdc_info.ctrl_info.forward_status;
        gstCollectInfo.pack_data[i].inside_info.dcdc_upgrade_status = gstPackSlavePara.pack_para[i].dcdc_info.ctrl_info.upgrade_status;
        gstCollectInfo.pack_data[i].inside_info.dcdc_balance_status = gstPackSlavePara.pack_para[i].dcdc_info.ctrl_info.pack_balance_ctrl;
        
        memcpy(&gstCollectInfo.pack_data[i].inside_info.dcdc_error_info, &gstPackSlavePara.pack_para[i].dcdc_info.error_info, sizeof(DcdcErrorInfo_t));
        memcpy(&gstCollectInfo.pack_data[i].inside_info.bms_error_info, &gstPackSlavePara.pack_para[i].bms_info.error_info, sizeof(BmsErrorInfo_t));
        
        //初始值为1
        BmsCtrlInfo.bms_ctrl_info_value.bits.polling_bms_data = 1;
    }
    
    for(int i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        if((gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver[0] << 16 | gstPackSlavePara.pack_para[i].bms_info.base_info.sw_ver[1]) == 0)
        {
            BmsCtrlInfo.bms_ctrl_info_value.bits.polling_bms_data = 0;
        } 
    }
}



/**
  * @brief  获取协处理器数据
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void _GetCopDataFunc(void)
{
    memcpy(&gstCollectInfo.maincard_data.inside_info.main_cop_info.base_info, &gstMainCardCopInfo.base_info, sizeof(MainCardCopBaseInfo_t));
    memcpy(&gstCollectInfo.maincard_data.inside_info.main_cop_info.ctrl_info, &gstMainCardCopInfo.ctrl_info, sizeof(MainCardCopCtrlInfo_t));
    memcpy(&gstCollectInfo.maincard_data.inside_info.main_cop_info.status_info, &gstMainCardCopInfo.status_info, sizeof(MainCardCopStatusInfo_t));
    
    //监控卡协处理器
    gstCollectInfo.cop_data.cop_device1_info.cop_cpu_modelid = gstCollectInfo.maincard_data.base_info.model_id;
    gstCollectInfo.cop_data.cop_device1_info.cop_cpu_module_num.moudle_num = 0;
    memcpy(&gstCollectInfo.cop_data.cop_device1_info.cop_sw_version, &gstCollectInfo.maincard_data.inside_info.main_cop_info.base_info.sw_ver, sizeof(uint32_t));
    memcpy(&gstCollectInfo.cop_data.cop_device1_info.cop_sw_extern_ver, &gstCollectInfo.maincard_data.inside_info.main_cop_info.base_info.sw_ver_ext, sizeof(uint16_t) * 12);
    memset(&gstCollectInfo.cop_data.cop_device1_info.cop_resrved, 0, sizeof(uint16_t) * 12);
    //pcs协处理器
    gstCollectInfo.cop_data.cop_device2_info.cop_cpu_modelid = gstCollectInfo.power_data.pcs_base_info.model_id;
    gstCollectInfo.cop_data.cop_device2_info.cop_cpu_module_num.moudle_num = 0;
    /*后续需修改*/
    memcpy(&gstCollectInfo.cop_data.cop_device2_info.cop_sw_version, &gstCollectInfo.power_data.pcs_base_info.sw_ver, sizeof(uint32_t));
    memcpy(&gstCollectInfo.cop_data.cop_device2_info.cop_sw_extern_ver, &gstCollectInfo.power_data.pcs_base_info.sw_ver_ext, sizeof(uint16_t) * 12);
    memset(&gstCollectInfo.cop_data.cop_device2_info.cop_resrved, 0, sizeof(uint16_t) * 12);
}
/**
  * @brief  获取监控数据
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void _GetMonitorTotalPackDataFunc(void)
{
    /* ??????重构 */
    uint32_t  batt_max_chg_power        = 0;
    uint32_t  batt_max_dsg_power        = 0;
    int32_t   total_chg_curr            = 0;    //充电电流
    int32_t   total_dsg_curr            = 0;    //放电电流
    uint32_t  chg_power_temp            = 0;    //充电功率
    uint32_t  dsg_power_temp            = 0;    //放电功率
    uint16_t  min_soh                   = 1000; //最小soh
    uint16_t soc_sum                    = 0;    //soc总和
    uint16_t soc_ave                    = 0;    //soc均值
    uint16_t soc_max                    = 0;    //soc最大值
    uint16_t soc_min                    = 0;    //soc最小值
    int16_t  bat_power_temp             = 0;    //所有电池包有功功率
    
    //Pack数量
    gstCollectInfo.maincard_data.total_pack_info.pack_num = GetBmsNum();

    if(gstCollectInfo.maincard_data.total_pack_info.pack_num > BMS_MAX_NUM)
    {
        gstCollectInfo.maincard_data.total_pack_info.pack_num = BMS_MAX_NUM;
    }
    
    //初始值
    gstCollectInfo.maincard_data.total_pack_info.total_vol      = gstPackSlavePara.pack_para[0].bms_info.status_info.total_vol;
    gstCollectInfo.maincard_data.total_pack_info.soh            = gstPackSlavePara.pack_para[0].bms_info.status_info.soh;
    gstCollectInfo.maincard_data.total_pack_info.total_current  = 0;
    gstCollectInfo.maincard_data.total_pack_info.total_power    = 0;
    gstCollectInfo.maincard_data.total_pack_info.rated_cap      = 0;
    gstCollectInfo.maincard_data.total_pack_info.rated_energy   = 0;
    gstCollectInfo.maincard_data.total_pack_info.allow_max_chg_current = 0;
    gstCollectInfo.maincard_data.total_pack_info.allow_max_dsg_current = 0;
    gstCollectInfo.maincard_data.total_pack_info.allow_max_chg_power   = 0;
    gstCollectInfo.maincard_data.total_pack_info.allow_max_dsg_power   = 0;
    soc_sum = 0;
    
    soc_min = gstCollectInfo.pack_data[0].state_info.soc_disp;
    soc_max = gstCollectInfo.pack_data[0].state_info.soc_disp;
    
    for(int i=0; i<gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        //电流
        gstCollectInfo.maincard_data.total_pack_info.total_current += gstPackSlavePara.pack_para[i].bms_info.status_info.total_curr;
        //功率
        gstCollectInfo.maincard_data.total_pack_info.total_power   += gstPackSlavePara.pack_para[i].bms_info.status_info.total_power;
        //SOH
        min_soh = (min_soh < gstPackSlavePara.pack_para[i].bms_info.status_info.total_power) ? 
                   min_soh : gstPackSlavePara.pack_para[i].bms_info.status_info.total_power;
        //额定容量Ah
        gstCollectInfo.maincard_data.total_pack_info.rated_cap += gstPackSlavePara.pack_para[i].bms_info.status_info.design_cap;
        //额定能量wh
        gstCollectInfo.maincard_data.total_pack_info.rated_energy += (gstCollectInfo.pack_data[i].state_info.design_cap / 10) * 25.6f;  //Ah * Vol  ?????

        //获取SOC最大最小值
        if(soc_min > gstCollectInfo.pack_data[i].state_info.soc_disp)
        {
            soc_min = gstCollectInfo.pack_data[i].state_info.soc_disp;
        }
        
        if(soc_max < gstCollectInfo.pack_data[i].state_info.soc_disp)
        {
            soc_max = gstCollectInfo.pack_data[i].state_info.soc_disp;
        }
        
        soc_sum += gstCollectInfo.pack_data[i].state_info.soc_disp;
    }
    gstMinBattSoc = soc_min;
    
    //获取SOC均值
    soc_ave = soc_sum / gstCollectInfo.maincard_data.total_pack_info.pack_num;
    //计算SOC
    gstCollectInfo.maincard_data.total_pack_info.soc = _SocProcess(soc_ave, soc_max, soc_min);
    
    /* 总Pack信息 */
    //允许电流和允许功率
    for(int i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {   
        //Pack总允许电流
        gstCollectInfo.maincard_data.total_pack_info.allow_max_chg_current +=  gstCollectInfo.pack_data[i].state_info.max_chg_current;
        gstCollectInfo.maincard_data.total_pack_info.allow_max_dsg_current +=  gstCollectInfo.pack_data[i].state_info.max_dsg_current;
        
        //chg_power与dsg_power(单包最大4KW)，测试模式下不限制
        chg_power_temp = (gstCollectInfo.pack_data[i].state_info.total_vol * gstCollectInfo.pack_data[i].state_info.max_chg_current) / 1000;
        dsg_power_temp = (gstCollectInfo.pack_data[i].state_info.total_vol * gstCollectInfo.pack_data[i].state_info.max_dsg_current) / 1000;
#if(TEST_MODE != 2)
        chg_power_temp = chg_power_temp > 4000 ? 4000 : chg_power_temp;
        dsg_power_temp = dsg_power_temp > 4000 ? 4000 : dsg_power_temp;
#endif
        gstCollectInfo.maincard_data.total_pack_info.allow_max_chg_power += chg_power_temp;
        gstCollectInfo.maincard_data.total_pack_info.allow_max_dsg_power += dsg_power_temp;
    }
    
    //根据电池有功功率判断充电 放电 静止
    for(uint16_t i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        bat_power_temp += (int16_t)gstPackSlavePara.pack_para[i].dcdc_info.power_info.output_power;
    }
    if(bat_power_temp >=-200 && bat_power_temp <= 200)
    {
        gstCollectInfo.power_data.model_info.dcdc_run_state = 0;  //静置
    }
    else if(bat_power_temp > 0)
    {
        gstCollectInfo.power_data.model_info.dcdc_run_state = 3;  //放电模式
    }
    else 
    {
        gstCollectInfo.power_data.model_info.dcdc_run_state = 2;  //充电模式
    }

    //远程待机
    if(gstUdpPcsSlavePara.pcs_info.control_info.remote_shutdown == 0x00)
    {
        gstCollectInfo.maincard_data.sys_info.remote_shutdown = 0xAA;
    }
    else if(gstUdpPcsSlavePara.pcs_info.control_info.remote_shutdown == 0x01)
    {
        gstCollectInfo.maincard_data.sys_info.remote_shutdown = 0x55;
    }
}

/**
  * @brief  PCS及BMS数据处理
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void GetPcsBmsInfoProc(void)
{
    //BMSPara_t
    uint16_t temp_max_data = 0;
    uint16_t temp_min_data = 0;
    uint16_t soc_sum       = 0;     //soc总和
    uint16_t soc_ave       = 0;     //soc均值
    uint16_t soc_max       = 0;     //soc最大值
    uint16_t soc_min       = 0;     //soc最小值
    uint32_t temp_cap_data = 0;
    uint32_t temp_cap_kwh_data = 0;
    uint8_t  flag=0;
    
    //将电池信息进行汇总
    _GetSignalPackDataFunc();
    _GetMonitorTotalPackDataFunc();
    //获取PCS数据
    _GetPcsDataFunc();
    _GetCopDataFunc();

    //此处需要使用BMS 或者PCS 读取到的数据  更新 监控卡中只读变量
    for(uint8_t i = 0; i < gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        if(gstCollectInfo.pack_data[i].base_info.sw_ver == 0)
        {
            flag = 1;// 
        }
    }
    if(flag == 1 || gstCollectInfo.maincard_data.total_pack_info.pack_num == 0)
    {
        _gReadAllBmsDataOkFlag = 0;
    }
    else
    {
        _gReadAllBmsDataOkFlag = 1;
    }
                                                
#if(TEST_PCS_DATA)                              
                                                
    gstCollectParaData.pcs_data.udp_pcs_input_para.batt_chg_dsg_power = test_batt_chg_dsg_power;

    gstCollectInfo.power_data.power_value_info.r_grid_vol = test_r_phase_meter_power + 2000;
    gstCollectInfo.power_data.power_value_info.r_grid_freq = test_r_phase_meter_power / 100;
    gstCollectInfo.power_data.power_value_info.r_meter_current = test_r_phase_meter_power + 1000;
    gstCollectInfo.power_data.power_value_info.r_meter_apparent_power = test_r_phase_meter_power;
    gstCollectInfo.power_data.power_value_info.r_meter_act_power = test_r_phase_meter_power + 100;
    gstCollectInfo.power_data.power_value_info.r_meter_react_power = test_r_phase_meter_power + 200;

    gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_volt = tset_r_phase_inverter_power + 100;
    gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_freq = tset_r_phase_inverter_power / 100;
    gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_curr = tset_r_phase_inverter_power + 200;
    gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_power = tset_r_phase_inverter_power;
    gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_active_power = tset_r_phase_inverter_power + 300;
    gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_inverter_reactive_power = tset_r_phase_inverter_power + 400;

    gstCollectInfo.power_data.model_info.inv_radiator_temp = test_inverter_radiator_temp;
    gstCollectInfo.power_data.model_info.inv_box_temp = test_chassis_internal_temp;
    gstCollectParaData.bms_data[0].bms_back_para.cell_average_temperature = test_chassis_internal_temp + 105;


    gstCollectInfo.power_data.power_value_info.r_backup_vol = test_r_phase_backup_apparent_power + 2000;
    gstCollectInfo.power_data.power_value_info.r_backup_current = test_r_phase_backup_apparent_power / 100;
    gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_backup_apparent_power = test_r_phase_backup_apparent_power + 1000;
    gstCollectInfo.power_data.power_value_info.r_backup_act_power = test_r_phase_backup_apparent_power;
    gstCollectInfo.power_data.power_value_info.r_backup_react_power = test_r_phase_backup_apparent_power + 100;

    gstCollectParaData.pcs_data.udp_pcs_input_para.r_phase_load_apparent_power = test_r_phase_load_apparent_power;
    gstCollectInfo.power_data.power_value_info.pv_1_power =  test_pv_ch1_input_power;
    gstCollectInfo.power_data.power_value_info.pv_2_power =  test_pv_ch2_input_power;

    gstCollectInfo.maincard_data.total_pack_info.soc = test_soc;
#endif

//在此处需要根据PCS信息来判定电流方向
}

uint16_t GetBattMinCellSoc(void)
{
    return gstMinBattSoc;
}

void SlaveCollectModbusCommProcess(void)
{
    static ModbusRTURevData_t *p;
    uint16_t mbrtu_state;
    uint16_t collect_recv_len;
    static  uint32_t collect_comm_time_delay_cnt = 0;
    static  uint32_t comm_time_out =  0;
    static  uint8_t  comm_timeout_cnt = 0;
//    GetPcsBmsInfoProc();
    switch(geCollectSlavePollingStatus)
    {
        case EN_COLLECT_POLLING_STATUS_IDLE:
        {
            //从串口队列中获取数据
            if(QueueGet(&gstDevModbusCollector.rev_queue, (void *)&p) == true)
            {
                //对数据进行解析

                comm_timeout_cnt = 0;
                mbrtu_state = MBRTU_ParseUp(p->p_data, p->data_size, &gCollectModbusData, &collect_recv_len);

                if(MBRTU_LEN_ERR == mbrtu_state)
                {
                    //数据长度
                    break;
                }
                if(MBRTU_OK != mbrtu_state)
                {
                    //需要返回异常
                    gCollectModbusData.fun |= 0x80;
                    if(MBRTU_FC_ERR == mbrtu_state)
                    {
                        gCollectModbusData.data_space.arr_data[0] = INVALID_FUNC;
                    }
                    else if(MBRTU_ADR_ERR == mbrtu_state)
                    {
                        gCollectModbusData.data_space.arr_data[0] = INVALID_ADDR;
                    }
                    else
                    {
                        gCollectModbusData.data_space.arr_data[0] = INVALID_DATA;
                    }
                    _gstComCollectTxData.len = MBRTU_PackUp(_gstComCollectTxData.pData, &gCollectModbusData);
                    geCollectSlavePollingStatus = EN_COLLECT_POLLING_STATUS_WAITING_TX;
                    collect_comm_time_delay_cnt = tim_get_systime();

                }
                else
                {
                    if((MBRTU_FC_READ_STANDREG == gCollectModbusData.fun) || (MBRTU_FC_READ_ORREG == gCollectModbusData.fun) )
                    {
                        //读取数据
                        ModbusReadParse(&gCollectModbusData);
                        _gstComCollectTxData.len = MBRTU_PackUp(_gstComCollectTxData.pData, &gCollectModbusData);
                        geCollectSlavePollingStatus = EN_COLLECT_POLLING_STATUS_WAITING_TX;
                        collect_comm_time_delay_cnt = tim_get_systime();

                    }
                    else if( (MBRTU_FC_SWRITE_STANDREG == gCollectModbusData.fun) || (MBRTU_FC_WRITE_STANDREG == gCollectModbusData.fun) )
                    {
                        //写数据
                        ModbusWriteParse(&gCollectModbusData);
                        _gstComCollectTxData.len = MBRTU_PackUp(_gstComCollectTxData.pData, &gCollectModbusData);
                        geCollectSlavePollingStatus = EN_COLLECT_POLLING_STATUS_WAITING_TX;
                        collect_comm_time_delay_cnt = tim_get_systime();
                    }
                }
            }
            else
            {
                //查看是否屏幕有控制

                //长时间35S  没有485通讯   则需要将联网状态置为OFF
                if((uint32_t)(tim_get_systime() - comm_time_out) > 1000)
                {
                    comm_time_out = tim_get_systime();
                    if(comm_timeout_cnt++ > 35) //35S
                    {
                        comm_timeout_cnt = 0;
                        gstCollectInfo.maincard_data.sys_info.net_conn_state = 0;   //off
                    }
                }

//                if(QueueGet(&_gCollectWriteQueue, (void *)&p) == true)
//                {//屏幕控制
//                    if(MBRTU_OK ==MBRTU_ParseUp(p->p_data,p->data_size,&gCollectModbusData,&collect_recv_len))
//                    {
//                        if( (MBRTU_FC_SWRITE_STANDREG == gCollectModbusData.fun)|| (MBRTU_FC_WRITE_STANDREG == gCollectModbusData.fun) )
//                        {//是写数据 否则不予处理
//                            ModbusWriteParse(&gCollectModbusData);
//                        }

//                    }
//                }
            }
            break;
        }
        case EN_COLLECT_POLLING_STATUS_REV:
        {
            break;
        }
        case EN_COLLECT_POLLING_STATUS_REV_OVERTIME:
        {
            break;
        }
        case EN_COLLECT_POLLING_STATUS_WAITING_TX:
        {
            if((uint32_t)(tim_get_systime() - collect_comm_time_delay_cnt) > 50)
            {
                collect_comm_time_delay_cnt = tim_get_systime();
                geCollectSlavePollingStatus = EN_COLLECT_POLLING_STATUS_INTERVAL_WAIT;
                ModbusCollectorRTUSend(_gstComCollectTxData.pData, _gstComCollectTxData.len);
            }
            break;
        }
        case EN_COLLECT_POLLING_STATUS_INTERVAL_WAIT:
        {
            if((uint32_t)(tim_get_systime() - collect_comm_time_delay_cnt) > 50)
            {
                geCollectSlavePollingStatus = EN_COLLECT_POLLING_STATUS_IDLE;
            }
            break;
        }
        default:
            break;
    }
    GetPcsBmsInfoProc();
}
void WordExchange(uint16_t *pword)
{
    uint16_t buffer;

    buffer = *pword;
    *pword = (buffer >> 8) | (buffer << 8);
}
/*
将数据从modbus_point 对应位置读取到 ptFrame-> data_space
以便 modeBus再进行转换为串口发送数组中
*/
void ModbusReadParse(ModbusRtuFrame_t *ptFrame)
{
    uint16_t start_addr;
    uint16_t reg_num;
    uint8_t buffer[256];
    uint16_t index;
    uint16_t offset;
    uint16_t i;
    uint16_t reg_addr;
    uint16_t w_buffer;

    const ModbusDataPoint_t   *modbus_point;
    uint16_t modbus_point_num;


    if(gstCollectInfo.maincard_data.base_info.user_type > MAX_USER_NUM)
    {
        gstCollectInfo.maincard_data.base_info.user_type = NORMAL_USER;
    }
    if(gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 1)
    {
        modbus_point = ModbusUserDataArr[4];
        modbus_point_num = ModbusUserDataPointNumArr[4];
    }
    else
    {
        modbus_point = ModbusUserDataArr[0];//gstCollectParaData.collect_data.user_type
        modbus_point_num = ModbusUserDataPointNumArr[0];//gstCollectParaData.collect_data.user_type//
    }


    start_addr = ptFrame->data_space.start_addr;
    reg_num = ptFrame->data_space.reg_len;
    memset(buffer, 0, 256);

    for(i = 0; i < reg_num; i++)
    {
        reg_addr = start_addr + i;
        for(index = 0; index < modbus_point_num; index++)
        {
            if((NULL != modbus_point[index].pdata) && ((EN_ATTR_RO == modbus_point[index].attr_rw) || (EN_ATTR_RW == modbus_point[index].attr_rw)))
            {
                if((reg_addr >= modbus_point[index].base_addr) && (reg_addr < (modbus_point[index].base_addr + modbus_point[index].num)))
                {
                    offset = reg_addr - modbus_point[index].base_addr;
                    if( (EN_DATA_U8 == modbus_point[index].data_type) || (EN_DATA_S8 == modbus_point[index].data_type) )
                    {
                        w_buffer = *((uint8_t *)modbus_point[index].pdata + offset);
                    }
                    else if( (EN_DATA_U32 == modbus_point[index].data_type) || (EN_DATA_S32 == modbus_point[index].data_type) )
                    {
                        if((offset % 2) == 0)
                        {
                            w_buffer = *((uint16_t *)modbus_point[index].pdata + offset + 1);
                        }
                        else
                        {
                            w_buffer = *((uint16_t *)modbus_point[index].pdata + offset - 1);
                        }
                    }
                    else if(EN_DATA_STRING == modbus_point[index].data_type)
                    {
                        w_buffer = *((uint16_t *)modbus_point[index].pdata + offset);
                        WordExchange(&w_buffer);
                    }
                    else
                    {
                        w_buffer = *((uint16_t *)modbus_point[index].pdata + offset);
                    }
                    memmove(buffer + i * 2, &w_buffer, 2);
                    break;
                }
            }
        }
    }
    DataModeChange(ptFrame->data_space.arr_data, buffer, USING_MODE_BIG, reg_num * 2);
    _gstCollectCtrl.factory_status = 0;
}


#define COLLCET_END_INDEX           60
#define PCS_END_INDEX               132
#define BMS_END_INDEX               142
#define DCDC_END_INDEX              142

#define BMS2_END_INDEX              152         
#define BMS3_END_INDEX              162
#define BMS4_END_INDEX              172          
#define BMS5_END_INDEX              182
#define BMS6_END_INDEX              192

#define AFE_TEST_MODE_INDEX     (BMS6_END_INDEX + 1)   //设置测试还是工作模式  
#define ARC_START_INDEX         (AFE_TEST_MODE_INDEX+1)
#define ARC_END_INDEX           (AFE_TEST_MODE_INDEX+1)


uint16_t Get_Cfg_Slave(uint8_t index)
{
    uint16_t state = true;
    if(1 == gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode)
    {
        if(index == 2)
        {
            gToolsGetTimeFlag = 1;
        }
        return false;
    }

    if(index == 56)
    {
        switch(_gstCollectCtrl.factory_status)
        {
            case 0:
            {
                if(gstCollectInfo.maincard_data.sys_info.factory_reset == 1)
                {
                    _gstCollectCtrl.factory_status  = 1;
                }
                break;
            }
            case 1:
            {
                if(gstCollectInfo.maincard_data.sys_info.factory_reset == 2)
                {
                    _gstCollectCtrl.factory_status  = 3;   //需要恢复到出厂设置
                    
//                    gstCollectParaData.collect_data.reset_factory = 0;
                }
                else if(gstCollectInfo.maincard_data.sys_info.factory_reset == 1)
                {
                    break;
                }
                else
                {
                    _gstCollectCtrl.factory_status = 0;
                }
                break;
            }
            case 3:
            {
                if(gstCollectInfo.maincard_data.sys_info.factory_reset == 1)
                {
                    _gstCollectCtrl.factory_status = 1;
                }
                else
                {
                    _gstCollectCtrl.factory_status = 0;
                }

                break;
            }
            default:
                _gstCollectCtrl.factory_status = 0;
                break;
        }
    }
    else
    {
        _gstCollectCtrl.factory_status = 0;
    }

//以上是恢复出厂设置
    if((index == 0) || (index == 1))
    {
        _gstCollectCtrl.bits.write_monitor_base_info_en = 1;
        _gstCollectCtrl.bits.write_updata_normal_data_en = 0; 

    }
    else if(index < COLLCET_END_INDEX)
    {
        switch(index)
        {
            case 5:
            case 10:
            case 11:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 20:
            case 26:
            case 53:
            case 54:
            case 55:
            {
                //如果是安装模式，需要同时修改普通用户模式
                if(gstCollectInfo.maincard_data.base_info.user_type > 1)
                {
                    _gstCollectCtrl.bits.write_updata_normal_data_en = 1; 
                }
                else
                {
                    _gstCollectCtrl.bits.write_updata_normal_data_en =0;  
                }
                
                break;
            }
            default:
            {
                 _gstCollectCtrl.bits.write_updata_normal_data_en =0;  //在其他状态下不能置1
                break;
            }

        }

        if(index == 52)
        {
            _gstCollectCtrl.bits.correct_rtc_time_en = 1;
        }
        else if(index == 14)
        {
            _gstCollectCtrl.bits.backlight_time_updata = 1;
            _gstCollectCtrl.bits.write_monitor_ctrl_info_en = 1;  
        }
        else if(index <= 54)
        {
            _gstCollectCtrl.bits.write_monitor_ctrl_info_en = 1;
        }
        else
        {
            
        }
    }
    else if(index < PCS_END_INDEX)
    {
        _gstCollectCtrl.bits.write_pcs_en = 1;
    }
    else if(index < BMS_END_INDEX)
    {
        _gstCollectCtrl.bits.write_bms_en = 1;

    }
    else if(index < DCDC_END_INDEX)
    {
        _gstCollectCtrl.bits.write_dcdc_en = 1;
    }
    else  if(index == AFE_TEST_MODE_INDEX)
    {
        _gstCollectCtrl.bits.set_test_mode = 1;
    }
    else
    {
        state = false;
        return state;
    }
    return state;
}

bool PcsRegNoDownSendFunc(uint32_t reg_addr)
{
    switch(reg_addr)
    {
        case 0X6331:
             return true;
        default:
            break;
    }
    return false;
}

/*
此函数的作用  时将MODEBUS中的数据转换到实际的对应寄存器中。
将ptFrame 中的数据转化到modbus_point 所对应的映射地址中
将从上位机接收到的数据通过解析完成之后 在写入MCU
*/
bool ModbusWriteParse(ModbusRtuFrame_t *ptFrame)
{
    uint16_t start_addr;
    uint16_t reg_num;
    uint8_t index;
    uint16_t offset;
    uint16_t i;
    uint32_t n = 0, j = 0;
    j = sizeof(_gCollectRegToPcsMap) / sizeof(_gCollectRegToPcsMap[0]);
    uint16_t reg_addr;
    uint16_t w_buffer;
    uint16_t cfg_slave = 0;
    uint8_t  write_en = 0;
    start_addr = ptFrame->data_space.start_addr;
    reg_num = ptFrame->data_space.reg_len;
    uint32_t updata_addr_offset = 0;
    bool  write_status = true;

    const ModbusDataPoint_t   *modbus_point;
    uint16_t modbus_point_num;

    updata_addr_offset = start_addr - COLLECT_MODE_ID;
    

    if(gstCollectInfo.maincard_data.base_info.user_type > MAX_USER_NUM)
    {
        gstCollectInfo.maincard_data.base_info.user_type = NORMAL_USER;
    }
    //进入工装测试模式
    if(gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 1)
    {
        modbus_point = ModbusUserDataArr[4];
        modbus_point_num = ModbusUserDataPointNumArr[4];
    }
    else  //正常程序
    {
        modbus_point = ModbusUserDataArr[gstCollectInfo.maincard_data.base_info.user_type];
        modbus_point_num = ModbusUserDataPointNumArr[gstCollectInfo.maincard_data.base_info.user_type];
    }


    for(i = 0; i < reg_num; i++)
    {
        reg_addr = start_addr + i;
        for(index = 0; index < modbus_point_num; index++)
        {
//            if((NULL != modbus_point[index].pdata)&&((EN_ATTR_WO == modbus_point[index].attr_rw)||(EN_ATTR_RW == modbus_point[index].attr_rw)))
            if((reg_addr >= modbus_point[index].base_addr) && (reg_addr < (modbus_point[index].base_addr + modbus_point[index].num)) )
            {
//                if((reg_addr >= modbus_point[index].base_addr)&&(reg_addr < (modbus_point[index].base_addr + modbus_point[index].num)) )
                if((NULL != modbus_point[index].pdata) && ((EN_ATTR_WO == modbus_point[index].attr_rw) || (EN_ATTR_RW == modbus_point[index].attr_rw)))
                {
                    offset = reg_addr - modbus_point[index].base_addr;
                    w_buffer = DataToUnshort(ptFrame->data_space.arr_data + i * 2, USING_MODE_BIG);

                    if( (EN_DATA_U8 == modbus_point[index].data_type) || (EN_DATA_S8 == modbus_point[index].data_type) )
                    {
                        *((uint8_t *)modbus_point[index].pdata + offset) = (uint8_t)w_buffer;
                    }
                    else if( (EN_DATA_U32 == modbus_point[index].data_type) || (EN_DATA_S32 == modbus_point[index].data_type) )
                    {
                        if((offset % 2) == 0)
                        {
                            *((uint16_t *)modbus_point[index].pdata + offset + 1) = w_buffer;
                        }
                        else
                        {
                            *((uint16_t *)modbus_point[index].pdata + offset - 1) = w_buffer;
                        }
                    }
                    else if(EN_DATA_STRING == modbus_point[index].data_type)
                    {
                        WordExchange(&w_buffer);
                        *((uint16_t *)modbus_point[index].pdata + offset) = w_buffer;
                    }
                    else
                    {
                        *((uint16_t *)modbus_point[index].pdata + offset) = w_buffer;
                    }
                    if(0 != Get_Cfg_Slave(index))
                    {
                        cfg_slave = 1;
                    }
                    break;
                }
                else
                {
                    write_status = false;
                    break;
                }
            }
        }
        if((index == modbus_point_num) || (write_status == false))
        {
            write_status = false;
            break;
        }
    }
//在此处 需要按照监控卡对应的数据 写入对应的BMS PCS ARC 中
    if(cfg_slave == 1)
    {
        //同时需要判定 哪些是需要写入flash中 //比如用户名 密码 以及通讯 波特率

        if(_gstCollectCtrl.bits.write_monitor_base_info_en == 1 )
        {
            _gstCollectCtrl.bits.write_monitor_base_info_en = 0;
            _gstCollectCtrl.bits.f_base_info_write = 1;
//            DeviceBaseInfoWrite();
        }
        if(_gstCollectCtrl.bits.write_monitor_ctrl_info_en == 1)
        {
            _gstCollectCtrl.bits.write_monitor_ctrl_info_en = 0;
            if(gstCollectInfo.maincard_data.base_info.user_type >1)
            {//此时是管理员模式，需要用当前的配置 更新下
            
                if(_gstCollectCtrl.bits.write_updata_normal_data_en == 1)
                {
                    _gstCollectCtrl.bits.write_updata_normal_data_en = 0;
                    memcpy((uint8_t *)(&gstCollectInfo.maincard_data.base_info.model_id + updata_addr_offset),(uint8_t *)(&gstInstallerCollectInfo.maincard_data.base_info.model_id + updata_addr_offset),reg_num<<1);
                    WriteInfoToStorgeData(&gstCollectInstallerStorageData,&gstInstallerCollectInfo); //更新存储值?????
                    ReadDataToStorageData(&gstCollectInfo,&gstCollectInstallerStorageData); //更新当前变量
                    gstCollectNormalStorageData = gstCollectInstallerStorageData;  //同步更新普通用户的设置值
                }                 
            }
            else
            {
                _gstCollectCtrl.bits.write_updata_normal_data_en = 0;
                WriteInfoToStorgeData(&gstCollectNormalStorageData,&gstCollectInfo);
            }
            _gstCollectCtrl.bits.f_device_ctl_write = 1;
//            DeviceCtlWrite();
        }
        if(_gstCollectCtrl.factory_status == 3)
        {
            _gstCollectCtrl.factory_status = 0;
            SetPcsResetFactory();
            StorageDataRest(gstCollectInfo.maincard_data.base_info.user_type);
        }
        if((ptFrame->data_space.start_addr >= COLLECT_POWER_WORK_MODE_SECTION_ADDR) && (ptFrame->data_space.start_addr <= COLLECT_POWER_INSIDE_SECTION_ADDR) && (_gstCollectCtrl.bits.write_pcs_en == 1))
        {
            //部分寄存器不下发
            if(PcsRegNoDownSendFunc(ptFrame->data_space.start_addr) == false)
            {
                //监控卡透传给PCS
                _gstCollectCtrl.bits.write_pcs_en = 0;
                for(n = 0; n < j; n++)
                {
                    if(ptFrame->data_space.start_addr == _gCollectRegToPcsMap[n][0])
                    {
                        ptFrame->data_space.start_addr = _gCollectRegToPcsMap[n][1];
                        break;
                    }
                }
                CollectWritePcsFun(ptFrame->data_space.start_addr, ptFrame->data_space.reg_len, (uint8_t *)ptFrame->data_space.arr_data, USING_MODE_BIG);
            }
        }
        if((ptFrame->data_space.start_addr >= (MONTIOR_BMS_HOLD_ADDR_START)) && (ptFrame->data_space.start_addr <= (MONTIOR_BMS_REG_HOLD_END_ADDR)) && (_gstCollectCtrl.bits.write_bms_en == 1))
        {
            #if 0      
            //监控卡透传给BMS     bms和pack重要信息归总到pack进行上报 无下发写
            _gstCollectCtrl.bits.write_bms_en = 0;
            CollectWriteBmsFun((ptFrame->data_space.start_addr-MONTIOR_BMS_BASE_ADDR), ptFrame->data_space.reg_len, ptFrame->data_space.arr_data, 0x0, USING_MODE_BIG); //从机地址需要根据数据偏移地址进行映射
            #endif
        }

        if((ptFrame->data_space.start_addr >= (MONTIOR_DCDC_HOLD_ADDR_START)) && (ptFrame->data_space.start_addr <= (MONTIOR_DCDC_REG_HOLD_END_ADDR)) && (_gstCollectCtrl.bits.write_dcdc_en == 1))
        {
            //监控卡透传给PACK
            _gstCollectCtrl.bits.write_dcdc_en = 0;
            CollectWriteDcdcFun((ptFrame->data_space.start_addr-MONTIOR_DCDC_BASE_ADDR), ptFrame->data_space.reg_len, ptFrame->data_space.arr_data, 0x0, USING_MODE_BIG); 
        }
        if(_gstCollectCtrl.bits.set_test_mode == 1)
        {
            _gstCollectCtrl.bits.set_test_mode = 0;
            if(gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 2)
            {
                SetSysEnterTestMode(gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode);
                gstCollectInfo.maincard_data.base_info.user_type = 1;//
            }
            
        }

        write_status = true;
    }
    return write_status;
}

void    CtrlInfoStorageProcess(void)
{
     if(_gstCollectCtrl.bits.f_device_ctl_write == 1)
     {
          _gstCollectCtrl.bits.f_device_ctl_write = 0;
         DeviceCtlWrite();
     }
     if(_gstCollectCtrl.bits.f_base_info_write == 1)
     {
        _gstCollectCtrl.bits.f_base_info_write = 0;
        DeviceBaseInfoWrite();
     }
     if(_gstCollectCtrl.bits.f_rest_write == 1)
     {
        _gstCollectCtrl.bits.f_rest_write = 0;
        StorageDataRest(RestUserType);
     }
    
}

bool GetCollectData(uint16_t data_start_addr, uint16_t reg_len, uint8_t *p_data)
{
    //读取数据地址
    //获取相对地址。
    uint16_t    start_addr = COLLECT_MODE_ID;
    uint16_t    offset =    (data_start_addr - COLLECT_MODE_ID);
    uint8_t     *p_source_data;
    if((data_start_addr < COLLECT_MODE_ID) || (p_data == NULL))
    {
        return false;
    }
    p_source_data = (uint8_t *)&gstCollectInfo.maincard_data.base_info.model_id;
    p_source_data += (offset << 1);
    memcpy(p_data, p_source_data, sizeof(reg_len << 1));
    return true;
}
//pdata
bool  SetCollectData(uint8_t *pdata, uint16_t len)
{
    //256+ 9
    bool set_status = true;
    ModbusRtuFrame_t gCollectModbusData;
    if((pdata == NULL) || (len > COLLECT_WRITE_SIZE))
    {
        return false;
    }
//    _gCollectWriteData[_CollectWriteDataBufIndex].len = len;
//    memcpy(_gCollectWriteData[_CollectWriteDataBufIndex].pData,pdata,len);
//    if(QueuePut(&_gCollectWriteQueue, &_gCollectWriteData[_CollectWriteDataBufIndex]) == false)
//    {
//        return false;
//    }
//    _CollectWriteDataBufIndex++;
//    _CollectWriteDataBufIndex %= COLLECT_WRITE_QUEUE_DEPTH;

    if(MBRTU_OK == MBRTU_ParseUp(pdata, len, &gCollectModbusData, 0))
    {
        if( (MBRTU_FC_SWRITE_STANDREG == gCollectModbusData.fun) || (MBRTU_FC_WRITE_STANDREG == gCollectModbusData.fun) )
        {
            //是写数据 否则不予处理
            set_status = ModbusWriteParse(&gCollectModbusData);
        }
        else
        {
            set_status = false;
        }
    }
    else
    {
        set_status = false;
    }

    return set_status;
}
uint8_t GetCorrectRtcStatus(void)
{
    return  _gstCollectCtrl.bits.correct_rtc_time_en;
}
void CleanCorrectRtcFlag(void)
{
    _gstCollectCtrl.bits.correct_rtc_time_en = 0;
}
uint16_t    GetCollectBaud()
{
    return  gstCollectInfo.maincard_data.base_info.baudrate;
}
uint8_t GetBackLightUpdataFlag(void)
{
    return (uint8_t)(_gstCollectCtrl.bits.backlight_time_updata);
}
void CleanBackLightUpdataFlag(void)
{
    _gstCollectCtrl.bits.backlight_time_updata = 0;
}


/**
  * @brief  Send Modbus func04 Data.
  * @param  modbus format
  * @return NULL
  * @note   读输入寄存器
  */
void ModbusReadInput(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
    uint16_t crc = 0;

    p_packet[0] = slave_addr;
    p_packet[1] = MBRTU_FC_READ_ORREG;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;

    crc = CrcModbus(p_packet, 6);
    p_packet[6] = crc & 0xFF;
    p_packet[7] = (crc >> 8) & 0xFF;

    SetCollectData(p_packet, 8);
}


/**
  * @brief  Send Modbus func03 Data.
  * @param  modbus format
  * @return NULL
  * @note   读保持寄存器
  */
void ModbusReadRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
    uint16_t crc = 0;

    p_packet[0] = slave_addr;
    p_packet[1] = MBRTU_FC_READ_STANDREG;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;

    crc = CrcModbus(p_packet, 6);
    p_packet[6] = crc & 0xFF;
    p_packet[7] = (crc >> 8) & 0xFF;

    SetCollectData(p_packet, 8);
}


/**
  * @brief  Send Modbus func10 Data.
  * @param  modbus format
  * @return NULL
  * @note   写多个寄存器
  */
void ModbusWriteMultiRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_value)
{
    uint16_t crc = 0;

    p_packet[0] = slave_addr;
    p_packet[1] = MBRTU_FC_WRITE_STANDREG;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;
    p_packet[6] = reg_num * 2;

    memcpy(&p_packet[7], p_reg_value, p_packet[6]);

    crc = CrcModbus(p_packet, p_packet[6] + 7);
    p_packet[7 +  p_packet[6]] = crc & 0xFF;
    p_packet[7 +  p_packet[6] + 1] = (crc >> 8) & 0xFF;


    SetCollectData(p_packet, p_packet[6] + 2 * p_packet[6] + 1 );
}


/**
  * @brief  Send Modbus func06 Data.
  * @param  modbus format
  * @return NULL
  * @note   写单个寄存器
  */
void ModbusWriteSingleRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t value)
{
    uint16_t crc = 0;

    p_packet[0] = slave_addr;
    p_packet[1] = MBRTU_FC_SWRITE_STANDREG;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = value & 0xFF;
    p_packet[5] = (value >> 8)  & 0xFF;

    crc = CrcModbus(p_packet, 6);
    p_packet[6] = crc & 0xFF;
    p_packet[7] = (crc >> 8) & 0xFF;

    SetCollectData(p_packet, 8);
}





