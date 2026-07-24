#ifndef ZTRANSFER_H
#define ZTRANSFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "F2837xD_device.h"

/**
 * @brief 离散传递函数的结构体
*/
typedef struct ztransfer_obj_stru_t
{
    uint8_t max_order;  //系统的最大阶数，数值上等于系数数量-1
    float* ku;          //分子系数
    float* ky;          //分母系数
    float* un;          //输入序列
    float* yn;          //输出序列
}ZtransferObj_t;


#ifdef __cplusplus
}
#endif  

#endif
