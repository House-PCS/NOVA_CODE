#ifndef FILTER_H
#define FILTER_H
#include "F28x_Project.h"

typedef enum
{
    EN_FILTER_TYPE_DEFAULT,
    EN_FILTER_TYPE_AVE,
    EN_FILTER_TYPE_COMPLEMENTARY,
} EN_FILTER_TYPE;

typedef struct filter_stru_t
{
    float32 input;
    float32 output;
    EN_FILTER_TYPE type;
} Filter_t;

inline void FilterInit(Filter_t* filter)
{
    filter->input = 0;
    filter->output = 0;
    filter->type = EN_FILTER_TYPE_DEFAULT;
}

typedef struct filter_ave_stru_t
{
    Filter_t filter_base;
    uint32_t length;
    uint32_t count;
    float32 sum;
} FilterAve_t;

inline void FilterAveInit(FilterAve_t* filter, uint32_t length)
{
    FilterInit((Filter_t*)filter);
    filter->filter_base.type = EN_FILTER_TYPE_AVE;
    filter->count = 0;
    filter->sum = 0;
    filter->length = length;
}

inline float32 FilterAveUpdate(FilterAve_t* filter, float32 value)
{
    filter->filter_base.input = value;

    filter->sum += filter->filter_base.input;
    if(++filter->count >= filter->length)
    {
        filter->filter_base.output = filter->sum / filter->length;
    }
    return filter->filter_base.output;
}

typedef struct filter_complementary_stru_t
{
    Filter_t filter_base;
    float32 k;
} FilterComplementary_t;

inline void FilterComplementaryInit(FilterComplementary_t* filter, float32 k)
{
    FilterInit((Filter_t*)filter);
    filter->filter_base.type = EN_FILTER_TYPE_COMPLEMENTARY;
    filter->k = k;
}

inline float32 FilterComplementaryUpdate(FilterComplementary_t* filter, float32 value)
{
    filter->filter_base.input = value;

    filter->filter_base.output = filter->filter_base.output * (1.0f - filter->k) + filter->filter_base.input * filter->k;
    return filter->filter_base.output;
}

inline float32 FilterUpdate(Filter_t* filter, float32 value)
{
    float32 output;
    switch(filter->type)
    {
    case EN_FILTER_TYPE_DEFAULT:
        output = value;
        break;
    case EN_FILTER_TYPE_AVE:
        output = FilterAveUpdate((FilterAve_t*)filter, value);
        break;
    case EN_FILTER_TYPE_COMPLEMENTARY:
        output = FilterComplementaryUpdate((FilterComplementary_t*)filter, value);
        break;
    default:
        output = value;
    }
    return output;
}

#endif
