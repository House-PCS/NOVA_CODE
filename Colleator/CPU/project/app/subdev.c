#include "subdev.h"

CollectorSubDevMap_t _gCollectorSubDevMap[] = {
	{ 0x0000, "-ERROR-DEV",							"" },
	{ 0xC200,  "BluE-S 5000D M1",					"KSTAR-5KW" },
	{ 0xC201,  "E10KT",								"KSTAR-10KW" },

	{ 0xC210,  "K5",								"Megarevo-5KW" },
	{ 0xC211,  "K10",								"Megarevo-10KW" },

	{ 0xC220,  "SURO-8KK",							"Suro-8KW-OFFLINE" },
	{ 0xC221,  "", 									"Suro-10KW-OFFLINE" },

	{ 0xC280,  "Nova-5KW",							"" },
};

CollectorSubDevMap_t *gCollectorSubDevMap = _gCollectorSubDevMap;

#define COLLECTOR_SUB_DEV_MAP(i)			_gCollectorSubDevMap[i]
#define COLLECTOR_SUB_DEV_GET_MODEL_ID_BY_DEV(dev) \
    { \
		for (int32_t i = 0; i < sizeof(_gCollectorSubDevMap)/sizeof(_gCollectorSubDevMap[0]); ++i) { \
			if (!strcmp(COLLECTOR_SUB_DEV_MAP(i).dev, dev)) { \
				return COLLECTOR_SUB_DEV_MAP(i).modelid; \
			}\
		} \
		return 0; \
    }
#define COLLECTOR_SUB_DEV_GET_IDX_BY_MODEL_ID(modelid) \
    { \
		for (int32_t i = 0; i < sizeof(_gCollectorSubDevMap)/sizeof(_gCollectorSubDevMap[0]); ++i) { \
			if (COLLECTOR_SUB_DEV_MAP(i).modelid == modelid) { \
				return i; \
			}\
		} \
		return -1; \
    }

uint16_t CollectorSubDevMapGetModelIdByDev(const char *dev)
{
	COLLECTOR_SUB_DEV_GET_MODEL_ID_BY_DEV(dev);
}

int32_t CollectorSubDevMapGetIdxByModelId(uint16_t modelid)
{
	COLLECTOR_SUB_DEV_GET_IDX_BY_MODEL_ID(modelid);
}
