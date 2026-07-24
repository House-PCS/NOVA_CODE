#include "dev_fc41d_common.h"
#include "dev_fc41d_relay_multi_cmd_kstar.h"

static int32_t _DevFc41dRelayMultiCmdStatusGet(DevFc41d_t *fc41d)
{
	return (fc41d==NULL?FC41D_PROCESS_RELAY_MULTI_CMD_NONE
			:fc41d->priv_data->relay_multi_cmd.status);
}

int32_t _DevFc41dRelayMultiCmdStatusSet(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}
	fc41d->priv_data->relay_multi_cmd.status = status;
	return 0;
}

int32_t _DevFc41dReadRelayMultiCmdKstar(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dRelayMultiCmdStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_RELAY_MULTI_CMD_KSTAR:
#if KSTAR_PCS_BAUD_RATE
		return _DevFc41dReadRelayMultiCmdKstar(fc41d, data, len);
#endif 
	case FC41D_PROCESS_RELAY_MULTI_CMD_NOVA_GET_MONITOR_HARDFAULT:
		return _DevFc41dReadRelayMultiCmdNova(fc41d, data, len);
	}

	return 0;
}

int32_t _DevFc41dProcessWriteRelayMultiCmd(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_RELAY;

	int32_t status = _DevFc41dRelayMultiCmdStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_RELAY_MULTI_CMD_KSTAR:
		_DevFc41dWriteRelayMultiCmdKstar(fc41d);
		break;
	case FC41D_PROCESS_RELAY_MULTI_CMD_NOVA:
		_DevFc41dWriteRelayMultiCmdKstar(fc41d);
		break;
	}

	return 0;
}

