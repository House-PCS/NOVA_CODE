
#ifndef __DEV_CUR_DETECT_H_
#define __DEV_CUR_DETECT_H_


#include "bsp.h"

#define SAMPLE_NUM      1024

typedef enum
{
    EN_ARC_CUR_ID_134,
    EN_ARC_CUR_ID_2
}EN_ARC_CUR_ID;

#pragma pack(1)
typedef struct _dev_cur_detect_stru_t
{
    uint16_t   arc_cur_data_1[SAMPLE_NUM];
    uint16_t   arc_cur_data_2[SAMPLE_NUM];
    uint16_t   arc_cur_data_3[SAMPLE_NUM];
    uint16_t   arc_cur_data_4[SAMPLE_NUM];
	
    
    bool       arc_cur_complete_1;
    bool       arc_cur_complete_2;
    bool       arc_cur_complete_3;
    bool       arc_cur_complete_4;
    
    void       (*start_sample)(EN_ARC_CUR_ID);
    void       (*start_all_sample)();           //º¯ÊýÖ¸Õë
}DevCurDetect_t;
#pragma pack()

extern DevCurDetect_t gDevCurDetect;

void DevAdcCurDetectInit();
void ArcHandleClearInit();

#endif






