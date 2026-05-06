/*
 * btm9011_simple.c — Simplified BTM9011 H-bridge motor driver
 */

#include "btm9011_simple.h"

/* -----------------------------------------------------------------------
 * Pre-built command bytes.
 *
 * Each byte:  [DIS=0 | IN1 | IN2 | PWM4 | PWM3 | PWM2 | PWM1 | PWM0]
 *
 *   Forward  = 0101 1111 = 0x5F  (IN1=1, IN2=0, PWM=31)
 *   Reverse  = 0011 1111 = 0x3F  (IN1=0, IN2=1, PWM=31)
 *   Brake    = 0000 0000 = 0x00  (IN1=0, IN2=0, PWM=0)
 *
 * PWM is always set to 31 (full) because the chip's internal PWM does not
 * vary speed reliably. Speed control is done in software in main.c instead.
 * ----------------------------------------------------------------------- */
#define CMD_FORWARD  0x5F
#define CMD_REVERSE  0x3F
#define CMD_BRAKE    0x00

/* Module-level state — set once by btm_init() */
static SPI_HandleTypeDef *s_hspi;
static GPIO_TypeDef      *s_cs_port;
static uint16_t           s_cs_pin;


/* Helper: look up the raw SPI byte for a direction */
static uint8_t dir_to_cmd(uint8_t dir)
{
    if (dir == BTM_FORWARD) return CMD_FORWARD;
    if (dir == BTM_REVERSE) return CMD_REVERSE;
    return CMD_BRAKE;
}

/* Helper: clock two bytes into the daisy-chain and latch */
static void send_two(uint8_t far_chip, uint8_t near_chip)
{
    uint8_t tx[2] = { far_chip, near_chip };

    /* CS high → data clocked in → CS low latches output */
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_SET);
    HAL_SPI_Transmit(s_hspi, tx, 2, 100);
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_RESET);
}


/* ---------------------------------------------------------------------- */

void btm_init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    s_hspi    = hspi;
    s_cs_port = cs_port;
    s_cs_pin  = cs_pin;

    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET); /* CS idle (low) */
}

void btm_drive(uint8_t left_dir, uint8_t right_dir)
{
    /* The right motor is physically wired backwards on the PCB.
     * Flip its direction here so the caller never has to think about it. */
    uint8_t right_flipped;
    if      (right_dir == BTM_FORWARD) right_flipped = BTM_REVERSE;
    else if (right_dir == BTM_REVERSE) right_flipped = BTM_FORWARD;
    else                               right_flipped = BTM_BRAKE;

    /* Daisy-chain order: first byte → far chip (right), second → near chip (left) */
    send_two(dir_to_cmd(right_flipped), dir_to_cmd(left_dir));
}

void btm_brake(void)
{
    send_two(CMD_BRAKE, CMD_BRAKE);
}
