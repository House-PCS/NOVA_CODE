#ifndef _DEV_FC41D_NOVA_DEBUG_H
#define _DEV_FC41D_NOVA_DEBUG_H

int32_t _DevFc41dNovaRequestReadTestMqttSub(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);

int32_t _DevFc41dNovaRequestReadDebugResp(DevFc41d_t *fc41d, 
								int32_t upload_type,
                                UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);

int32_t _DevFc41dNovaRequestReadDebugGetUpgradeFile(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);

int32_t _DevFc41dNovaRequestReadDebugGetUpgradeMd5(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);

int32_t _DevFc41dNovaRequestReadDebug(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);

#if 0
int32_t _DevFc41dNovaRequestReadDebugAes(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);
#endif 

int32_t _DevFc41dNovaRequestReadDebugGetProto(DevFc41d_t *fc41d, 
								int32_t upload_type,
								UniformProtocolHead_t *uniform,
								const uint8_t *data, uint16_t len);

int32_t _DevFc41dNovaRequestWriteDebugSetPcsUpgrade(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								const uint8_t *data, uint16_t len);

int32_t _DevFc41dNovaRequestWriteDebugWriteMonitorSn(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);

int32_t _DevFc41dNovaRequestDebugMonitorAuth(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);

int32_t _DevFc41dNovaRequestDebugClsMonitorTotalFlash(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);

int32_t _DevFc41dNovaRequestDebugOtaFc41d(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);

int32_t _DevFc41dNovaRequestDebugGetMonitorHardfault(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body);

int32_t _DevFc41dUrcNovaDebug(DevFc41d_t *fc41d, const char *data, int32_t len);

#endif 
