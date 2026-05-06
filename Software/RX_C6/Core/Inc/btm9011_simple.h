/*
 * btm9011_simple.h — Simplified BTM9011 H-bridge motor driver
 *
 * The BTM9011 receives an 8-bit SPI command that sets direction and speed.
 *
 * SPI byte layout:
 *   Bit 7 (DIS) : 0 = outputs enabled,  1 = outputs disabled
 *   Bit 6 (IN1) : direction control bit 1
 *   Bit 5 (IN2) : direction control bit 2
 *   Bits 4:0    : PWM duty 0–31 (NOTE: internal PWM doesn't work on this chip,
 *                 so we always send 31 and do software PWM in main.c instead)
 *
 * Direction truth table:
 *   IN1=1, IN2=0  →  Forward
 *   IN1=0, IN2=1  →  Reverse
 *   IN1=0, IN2=0  →  Brake (motor shorts to GND — hard stop)
 *   IN1=1, IN2=1  →  Coast (motor freewheels — avoid using)
 *
 * Two chips are daisy-chained on the same SPI bus.
 * The first byte clocked out goes to the far chip (right motor),
 * the second byte goes to the near chip (left motor).
 */

#ifndef BTM9011_SIMPLE_H
#define BTM9011_SIMPLE_H

#include "stm32f1xx_hal.h"

/* Direction constants — pass these into btm_drive() */
#define BTM_FORWARD  1
#define BTM_REVERSE  2
#define BTM_BRAKE    0

/*
 * btm_init — store the SPI handle and CS pin for later use.
 * Call this once in main() before anything else.
 */
void btm_init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);

/*
 * btm_drive — send a direction command to both motors.
 *   left_dir / right_dir : BTM_FORWARD, BTM_REVERSE, or BTM_BRAKE
 *
 * The right motor is wired backwards on the PCB so its direction
 * is automatically flipped inside this function.
 */
void btm_drive(uint8_t left_dir, uint8_t right_dir);

/*
 * btm_brake — immediately stop both motors (hard brake).
 */
void btm_brake(void);

#endif /* BTM9011_SIMPLE_H */
