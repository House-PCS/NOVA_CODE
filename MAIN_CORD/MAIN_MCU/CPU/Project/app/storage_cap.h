#ifndef STORAGE_CAP_HHH
#define STORAGE_CAP_HHH


#include    "dev_it7c4337.h"
#include    "dev_spiflash.h"
#include    "slave_collect.h"

#define     CAP_STORAGE_BASE_ADDR           0x258000
#define     CAP_TOTAL_SIZE                  (0x2000UL * 12UL * 20UL)  //20?¨º¦Ì?¨ºy?Y 
#define     CAP_MAX_DAY                     2000

 
#define     INIT_STORAGE_YEAR   2024
#define     END_STORAGE_YEAR    2044
#define     CAP_STORAGE_SIZE        (sizeof(CapStorageInfo_t))

#define     MIN(A,B)  ((A)<(B)?(A):(B))
#pragma pack(1)

typedef struct _cap_storage_stru_t
{
#if 0
    uint32_t    cap_storge_id;
    uint32_t    pv_generation_cap_sum;
    uint32_t    purchase_cap_sum;
    uint32_t    selling_cap_sum;
    uint32_t    load_consum_cap_sum;
    uint32_t    batt_chg_cap_sum;
    uint32_t    batt_dsg_cap_sum;
    uint32_t    reserve[1]; 
#else 
    uint32_t    cap_storge_id;
    uint32_t    pv_generation_cap_sum;
    uint32_t    purchase_cap_sum;
    uint32_t    selling_cap_sum;
    uint32_t    load_consum_cap_sum;
    uint32_t    batt_chg_cap_sum;
    uint32_t    batt_dsg_cap_sum;
    uint32_t    remote_load_cap_sum; 
    uint32_t    gen_elec_cap_sum;
    uint32_t    ev_chg_cap_sum;
    uint32_t    ev_dsg_cap_sum;
    uint32_t    reserved[5];
#endif
} CapStorageInfo_t;

typedef struct _cap_from_pcs_struct_t
{
    uint32_t    pv_total;
    uint32_t    purchase_total;
    uint32_t    selling_total;
    uint32_t    load_total;
    uint32_t    batt_chg_total;
    uint32_t    batt_dsg_total;  
    uint32_t    remote_load_total; 
    uint32_t    gen_elec_total;
    uint32_t    ev_chg_total;
    uint32_t    ev_dsg_total;  
} CapFromPcsValue_t;


extern DevRtc_t    gstRtcTime;

extern CapStorageInfo_t gstCapStorgeInfo;

#pragma pack()


void    CapInitl(void);
void    CapCtrl(void);
void    SetWirteCapEN(void);  //?CAP??
void    UpdataFlashDataEn(void);//????????,????????????????

void SetFlashData(CapStorageInfo_t cap_info);
bool CapacityStorageFun(CapStorageInfo_t cap_sum);


 
#endif
