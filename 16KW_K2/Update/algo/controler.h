#ifndef __CONTROLER_H_
#define __CONTROLER_H_
typedef struct forward_pi_loop_stru_t
{
    float  Ref;         // Input: reference set-point
    float  Fbk;         // Input: feedback
    float  Out_pi;      // Output: controller output
    float  Out_Ki;      // ????
    float  Out;         // Output: controller output
    float  Kp;          // Parameter: proportional loop gain
    float  Ki;          // Parameter: integral gain
    float  Kd;          // Parameter: derivative gain (reserved)

    float  delta_max;
    float  delta;
    float  Umax;        // Parameter: upper saturation limit
    float  Umin;        // Parameter: lower saturation limit
    float  erro;        // Data: current error
    float  erro_up;     // Data: previous error
} ForWardPi_t;
float AddPIControlerForward(ForWardPi_t * loop, float ref, float fbk, float forward);
float PIControlerForward(ForWardPi_t * loop, float ref, float fbk, float forward);
void AddPIControlerForwardInit(ForWardPi_t * loop, float kp, float ki, float delta_max, float umax);
#endif
