#ifndef STORAGE_CAP_HHH
#define STORAGE_CAP_HHH


#include    "dev_it7c4337.h"
#include    "dev_spiflash.h"
#include    "slave_collect.h"
#define     CAP_STORAGE_BASE_ADDR           0x258000
#define     CAP_MAX_DAY                     2000
#pragma pack(1)

typedef struct _cap_storage
{
//    union
//    {
//        struct  
//        {
//            uint8_t     cap_num;
//            uint8_t     cap_id1;
//            uint8_t     cap_id2;
//            uint8_t     cap_id3;            
//        }bytes;
//        uint32_t        cap_id;
//    }IdValue;
    uint32_t    cap_storge_id;
    uint32_t    pv_generation_cap_diff;
    uint32_t    purchase_cap_diff;
    uint32_t    selling_cap_diff;
    uint32_t    load_consum_cap_diff; 
    uint32_t    reserve[3]; 
}tsCapStorageInfo;

extern DevRtc_t    RtcTime;




#pragma pack()


void    CapInitl(void);
void    CapCtrl(void);
void    SetWirteCapEN(void);  //Ð´CAPÊ¹ÄÜ





 
#endif
