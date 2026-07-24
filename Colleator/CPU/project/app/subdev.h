#ifndef _SUB_DEV_H
#define _SUB_DEV_H

#include <stdint.h>
#include <string.h>

typedef struct _collector_sub_dev_map_t
{
	uint16_t modelid;
	const char dev[64];
	const char desc[64];	// 描述信息
} CollectorSubDevMap_t;

extern CollectorSubDevMap_t *gCollectorSubDevMap;

uint16_t CollectorSubDevMapGetModelIdByDev(const char *dev);

int32_t CollectorSubDevMapGetIdxByModelId(uint16_t modelid);

#endif /* _SUB_DEV_H */
