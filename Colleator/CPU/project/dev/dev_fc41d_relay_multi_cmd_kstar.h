#ifndef _DEV_FC41D_RELAY_MULTI_CMD_KSTAR_H_
#define _DEV_FC41D_RELAY_MULTI_CMD_KSTAR_H_

int32_t _DevFc41dReadRelayMultiCmdKstar(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len);

int32_t _DevFc41dWriteRelayMultiCmdKstar(DevFc41d_t *fc41d);

#endif /* _DEV_FC41D_RELAY_MULTI_CMD_KSTAR_H_ */

