
#include "sys_info.h"


KstarInvBaseInfo_t gKstarInvBaseInfo;
KstarInvSysInfo_t  gKstarInvSysInfo;
KstarDevSet_t      gKstarDevSet;
KstarRemoteCmd_t   gKstarRemoteCmd;

CollectorDevInfo_t gCollectorDevInfo;


void SysInfoInit()
{
    gCollectorDevInfo.read_pcs_period = 10;
    gCollectorDevInfo.report_period = 5 * 60;
    
    gCollectorDevInfo.kstar_base_info_updated = false;
    gCollectorDevInfo.kstar_sys_info_updated = false;
    gCollectorDevInfo.kstar_pack_num = 0;
    
    
}




