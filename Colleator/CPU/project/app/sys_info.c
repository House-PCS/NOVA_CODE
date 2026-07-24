
#include "sys_info.h"
#include "pcs_op_nova.h"
#include "pcs_op_kstar.h"
#include "pcs_op_megarevo.h"
#include "pcs_op_suro.h"


//NovaInvSysInfo_t  gNovaInvSysInfo;
//NovaDevSet_t      gNovaDevSet;
//NovaRemoteCmd_t   gNovaRemoteCmd;

KstarInvBaseInfo_t gKstarInvBaseInfo;
KstarInvSysInfo_t  gKstarInvSysInfo;
KstarDevSet_t      gKstarDevSet;
KstarRemoteCmd_t   gKstarRemoteCmd;

uint8_t gCommonBaseInfo[2560] = {0};
NovaInvBaseInfo_t *gNovaInvBaseInfo = NULL;
MegarevoInvBaseInfo_t *gMegarevoInvBaseInfo = NULL;
SuroInvBaseInfo_t *gSuroInvBaseInfo = NULL;

CollectorDevInfo_t gCollectorDevInfo;


uint8_t gPcsDevDebugModebusInfo[64] = {0};


void SysInfoInit()
{
    gCollectorDevInfo.read_pcs_period = 21;//11;
    gCollectorDevInfo.report_period = 5 * 60;
    
    gCollectorDevInfo.pcs_info_updated = false;
    gCollectorDevInfo.kstar_sys_info_updated = false;
    gCollectorDevInfo.kstar_pack_num = 0;
    
}




