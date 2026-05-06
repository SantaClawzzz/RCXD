#ifndef __BTM9011_H
#define __BTM9011_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#ifndef HAL_SPI_MODULE_ENABLED
typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
#endif

/* Maximum drivers supported in one daisy chain --------------------------------*/
#define BTM9011_MAX_DRIVERS  8U

/* BTM9011 SPI command bytes (IN1=1,IN2=1,PWM=25/31) --------------------------*/
#define BTM9011_CMD_BRAKE    0x00
#define BTM9011_CMD_STOP     0x39
#define BTM9011_CMD_FORWARD  0x79
#define BTM9011_CMD_REVERSE  0x59

/* BTM9011 max PWM duty value (5-bit field, 0–31) */
#define BTM9011_DUTY_MAX     31U

/* Motor direction for BTM9011_BuildCmd() */
typedef enum {
    BTM9011_DIR_BRAKE   = 0,  /* IN1=0, IN2=0 — both outputs low  */
    BTM9011_DIR_FORWARD = 1,  /* IN1=1, IN2=1 — forward           */
    BTM9011_DIR_REVERSE = 2,  /* IN1=1, IN2=0 — reverse           */
    BTM9011_DIR_COAST   = 3,  /* IN1=0, IN2=1 — free-wheel        */
} BTM9011_Dir;

/* Handle struct ---------------------------------------------------------------*/
typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef      *cs_port;
    uint16_t           cs_pin;
    uint8_t            num_drivers;
} BTM9011_HandleTypeDef;

/* Function prototypes ---------------------------------------------------------*/

/**
 * @brief  Initialise the BTM9011 driver handle.
 *
 * Stores the SPI peripheral, CS pin, and driver count.
 * CS is driven low (idle) before returning.
 *
 * @param  hbtm        Pointer to a BTM9011_HandleTypeDef to populate.
 * @param  hspi        Pointer to an initialised SPI handle.
 * @param  cs_port     GPIO port of the CS pin.
 * @param  cs_pin      GPIO pin mask of the CS pin.
 * @param  num_drivers Number of BTM9011 chips in the daisy chain (1–BTM9011_MAX_DRIVERS).
 * @retval HAL_OK on success, HAL_ERROR on invalid arguments.
 */
HAL_StatusTypeDef BTM9011_Init(BTM9011_HandleTypeDef *hbtm,
                                SPI_HandleTypeDef *hspi,
                                GPIO_TypeDef *cs_port,
								uint16_t cs_pin,
                                uint8_t num_drivers);

/**
 * @brief  Send one command byte to every driver in the chain.
 *
 * cmds[0] targets driver 1 (nearest chip), cmds[N-1] targets driver N
 * (furthest chip). Ordering is reversed internally before transmission.
 *
 * @param  hbtm  Pointer to an initialised BTM9011_HandleTypeDef.
 * @param  cmds  Array of command bytes, one entry per driver.
 *               Must contain at least hbtm->num_drivers elements.
 * @retval HAL status from HAL_SPI_Transmit, or HAL_ERROR on bad args.
 */
HAL_StatusTypeDef BTM9011_Send(BTM9011_HandleTypeDef *hbtm, const uint8_t *cmds);

/**
 * @brief  Build a single BTM9011 SPI command byte.
 *
 * SPI byte layout: [0, IN1, IN2, PWM4, PWM3, PWM2, PWM1, PWM0]
 * duty_0_31 is clamped to BTM9011_DUTY_MAX (31) if exceeded.
 *
 * @param  dir       Motor direction (BTM9011_Dir enum).
 * @param  duty_0_31 PWM duty cycle, 0 (off) to 31 (full speed).
 * @retval Composed SPI command byte.
 */
uint8_t BTM9011_BuildCmd(BTM9011_Dir dir, uint8_t duty_0_31);

#ifdef __cplusplus
}
#endif

#endif /* __BTM9011_H */
