
#include "current_control.h"
void PackCurrentControl(void)
{
	
	if((gPackStatus.mode == EN_PACK_CURRENT_CHARGE_MODE) || (gPackStatus.mode == EN_PACK_CURRENT_DISCHARGE_MODE))
	{
		if((gPackStatus.mode == EN_PACK_CURRENT_CHARGE_MODE)&&(gPackStatus.pack_current<-120000))
		{
			CCEnOn();//充电电流过大  打开限流模块
			CloseChargeMos();//关闭充电MOS
		}
		
		else if((gPackStatus.mode == EN_PACK_CURRENT_DISCHARGE_MODE)&&(gPackStatus.pack_current>5000))
		{
			CCEnOff();//输出电流大于5A  关闭限流模块
			OpenChargeMos();			
		}
		
		else
		{
			CCEnOff();//正常充放电模式  关闭限流模块
			OpenChargeMos();
		}	
	}
	
}
