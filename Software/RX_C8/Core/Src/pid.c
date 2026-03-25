#include "pid.h"
#include "btm9011.h"  /* for BTM9011_DUTY_MAX */

/* -------------------------------------------------------------------------- */

static float pid_clampf(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* -------------------------------------------------------------------------- */

void PID_Init(PID_t *pid, float kp, float ki, float kd,
              float out_min, float out_max)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->out_min = out_min;
    pid->out_max = out_max;
    pid->integral_limit = (out_max - out_min) * 0.5f;  /* sensible default */
    pid->integral        = 0.0f;
    pid->prev_measurement = 0.0f;
}

void PID_Reset(PID_t *pid)
{
    pid->integral         = 0.0f;
    pid->prev_measurement = 0.0f;
}

void PID_SetTunings(PID_t *pid, float kp, float ki, float kd)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

void PID_SetIntegralLimit(PID_t *pid, float limit)
{
    pid->integral_limit = limit;
}

/* -------------------------------------------------------------------------- */

float PID_Compute(PID_t *pid, float setpoint, float measured, float dt)
{
    float error = setpoint - measured;

    /* Proportional */
    float p_term = pid->kp * error;

    /* Integral with anti-windup clamp */
    pid->integral += error * dt;
    pid->integral  = pid_clampf(pid->integral,
                                -pid->integral_limit,
                                 pid->integral_limit);
    float i_term = pid->ki * pid->integral;

    /*
     * Derivative on measurement (not error).
     * This avoids a large output spike when the setpoint changes suddenly.
     * The sign is negated because rising measurement means falling error.
     */
    float d_term = 0.0f;
    if (dt > 0.0f)
        d_term = -pid->kd * (measured - pid->prev_measurement) / dt;

    pid->prev_measurement = measured;

    return pid_clampf(p_term + i_term + d_term, pid->out_min, pid->out_max);
}

/* -------------------------------------------------------------------------- */

void PID_SyncMotors(PID_t *pid1, PID_t *pid2,
                    float speed1, float speed2,
                    float *correction1, float *correction2,
                    float dt)
{
    /* Both motors aim for the current average — they meet in the middle */
    float avg = (speed1 + speed2) * 0.5f;

    *correction1 = PID_Compute(pid1, avg, speed1, dt);
    *correction2 = PID_Compute(pid2, avg, speed2, dt);
}

/* -------------------------------------------------------------------------- */

uint8_t PID_ClampDuty(float duty)
{
    if (duty < 0.0f)                        return 0U;
    if (duty > (float)BTM9011_DUTY_MAX)     return (uint8_t)BTM9011_DUTY_MAX;
    return (uint8_t)duty;
}
