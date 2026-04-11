#ifndef __PID_H
#define __PID_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Generic floating-point PID controller.
 *
 * Designed for motor speed synchronisation on STM32 but usable for any
 * control loop.  No dynamic allocation; all state lives in PID_t.
 *
 * Typical dual-motor sync usage (encoders provide speed_rpm):
 *
 *   PID_t pid1, pid2;
 *   PID_Init(&pid1, 0.8f, 0.4f, 0.05f, -10.0f, 10.0f);
 *   PID_Init(&pid2, 0.8f, 0.4f, 0.05f, -10.0f, 10.0f);
 *
 *   // In control loop (call every dt seconds, e.g. 10 ms → dt = 0.01f):
 *   float c1, c2;
 *   PID_SyncMotors(&pid1, &pid2, speed1_rpm, speed2_rpm, &c1, &c2, dt);
 *
 *   uint8_t duty1 = PID_ClampDuty(base_duty + c1);
 *   uint8_t duty2 = PID_ClampDuty(base_duty + c2);
 *
 *   uint8_t cmds[2] = {
 *       BTM9011_BuildCmd(BTM9011_DIR_FORWARD, duty1),
 *       BTM9011_BuildCmd(BTM9011_DIR_FORWARD, duty2),
 *   };
 *   BTM9011_Send(&hbtm, cmds);
 */

/* PID controller state and tuning ----------------------------------------*/
typedef struct {
    /* Tuning */
    float kp;
    float ki;
    float kd;

    /* Internal state */
    float integral;
    float prev_measurement;  /* used for derivative-on-measurement */

    /* Output limits */
    float out_min;
    float out_max;

    /* Anti-windup: integral term is clamped to ±integral_limit */
    float integral_limit;
} PID_t;

/* Lifecycle ---------------------------------------------------------------*/

/**
 * @brief  Initialise a PID controller.
 *
 * integral_limit is set to half the output range by default; call
 * PID_SetIntegralLimit() afterwards to override.
 *
 * @param  pid     Controller to initialise.
 * @param  kp      Proportional gain.
 * @param  ki      Integral gain.
 * @param  kd      Derivative gain.
 * @param  out_min Minimum value the controller may return.
 * @param  out_max Maximum value the controller may return.
 */
void PID_Init(PID_t *pid, float kp, float ki, float kd,
              float out_min, float out_max);

/**
 * @brief  Clear integral accumulator and derivative history.
 *         Call when the controller is re-activated after a pause.
 */
void PID_Reset(PID_t *pid);

/**
 * @brief  Update gain values without resetting state.
 */
void PID_SetTunings(PID_t *pid, float kp, float ki, float kd);

/**
 * @brief  Override the default anti-windup clamp on the integral term.
 * @param  limit  Maximum absolute value of the integral accumulator.
 */
void PID_SetIntegralLimit(PID_t *pid, float limit);

/* Computation -------------------------------------------------------------*/

/**
 * @brief  Run one PID iteration.
 *
 * Derivative is calculated on the measurement (not the error) to avoid
 * a sudden output spike when the setpoint changes.
 *
 * @param  pid       Controller instance.
 * @param  setpoint  Desired value.
 * @param  measured  Current measured value.
 * @param  dt        Time since last call in seconds (must be > 0).
 * @retval Clamped controller output in [out_min, out_max].
 */
float PID_Compute(PID_t *pid, float setpoint, float measured, float dt);

/* Motor synchronisation helper -------------------------------------------*/

/**
 * @brief  Synchronise two motors toward their average speed.
 *
 * Computes setpoint = (speed1 + speed2) / 2, then runs each PID against
 * that shared setpoint.  The faster motor receives a negative correction
 * and the slower motor a positive one, meeting in the middle.
 *
 * @param  pid1         PID instance for motor 1.
 * @param  pid2         PID instance for motor 2.
 * @param  speed1       Measured speed of motor 1 (any consistent unit).
 * @param  speed2       Measured speed of motor 2.
 * @param  correction1  Output: additive duty correction for motor 1.
 * @param  correction2  Output: additive duty correction for motor 2.
 * @param  dt           Time since last call in seconds.
 */
void PID_SyncMotors(PID_t *pid1, PID_t *pid2,
                    float speed1, float speed2,
                    float *correction1, float *correction2,
                    float dt);

/* Utility -----------------------------------------------------------------*/

/**
 * @brief  Clamp a floating-point duty value to the BTM9011 0–31 range.
 * @param  duty  Raw duty (may be negative or > 31 after PID correction).
 * @retval Clamped uint8_t in [0, 31].
 */
uint8_t PID_ClampDuty(float duty);

#ifdef __cplusplus
}
#endif

#endif /* __PID_H */
