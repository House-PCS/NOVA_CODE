#include "config.h"
#include "dev_fc41d_common.h"
#include "pcs_op.h"
#include "pcs_op_nova.h"

#define DEV_CMD_RUN_TIMEOUT			5*1000

extern int32_t _DevFc41dTaskHeadSetStat(DevFc41d_t *fc41d, int8_t stat);

static int32_t _DevFc41dNextTasks(DevFc41d_t *fc41d, int32_t cmd)
{
	switch (cmd)
	{
	case PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END:
		break;
	}

	return 0;
}

int32_t _DevFc41dReadRelayMultiCmdKstar(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

    uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;

	uint32_t count = tim_get_systime() - fc41d->priv_data->write_stat.relay_low_machine;
	// wait 485  result
	int32_t status = DevOpWriteStatusGet();

	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	if (status == EN_DEV_WRITE_COMM_STATUS_ACK_OK)
	{
		fc41d->task_head_del(fc41d);
		gDevSetting = 0;
		return 0;
	}
	else if (status == EN_DEV_WRITE_COMM_STATUS_NO_ACK || status == EN_DEV_WRITE_COMM_STATUS_ACK_ERROR)
	{
		// failed
		UniformProtocolBodyHead_t *write_body = (UniformProtocolBodyHead_t *)write_ctx->uniform_body;

		write_body->tlv.err = status+101;
		fc41d->task_head_del(fc41d);
		gDevSetting = 0;
		return 0;
	}

	if (count > DEV_CMD_RUN_TIMEOUT)
	{
		// failed
		UniformProtocolBodyHead_t *write_body = (UniformProtocolBodyHead_t *)write_ctx->uniform_body;

		write_body->tlv.err = status+100;
		fc41d->task_head_del(fc41d);
		gDevSetting = 0;
		return 0;
	}

	delay_1ms(50);

	return 0;
}

int32_t _DevFc41dReadRelayMultiCmdNova(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

    uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;

	uint32_t count = tim_get_systime() - fc41d->priv_data->write_stat.relay_low_machine;
	// wait 485  result
	int32_t status = DevOpWriteStatusGet();

	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	if (status == EN_DEV_WRITE_COMM_STATUS_ACK_OK)
	{
		HardFaultInfo_t *hardfault = (HardFaultInfo_t *)NovaDbgGetMonitorHardfualtInfo();
		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)hardfault, 
							sizeof(HardFaultInfo_t));

		fc41d->task_head_del(fc41d);
		gDevSetting = 0;
		return 0;
	}
	else if (status == EN_DEV_WRITE_COMM_STATUS_NO_ACK || status == EN_DEV_WRITE_COMM_STATUS_ACK_ERROR)
	{
		// failed
		UniformProtocolBodyHead_t *write_body = (UniformProtocolBodyHead_t *)write_ctx->uniform_body;

		write_body->tlv.err = status+101;
		fc41d->task_head_del(fc41d);
		gDevSetting = 0;
		return 0;
	}

	if (count > DEV_CMD_RUN_TIMEOUT)
	{
		// failed
		UniformProtocolBodyHead_t *write_body = (UniformProtocolBodyHead_t *)write_ctx->uniform_body;

		write_body->tlv.err = status+100;
		fc41d->task_head_del(fc41d);
		gDevSetting = 0;
		return 0;
	}

	delay_1ms(10);

	return 0;
}

int32_t _DevFc41dWriteRelayMultiCmdKstar(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	return 0;
}


