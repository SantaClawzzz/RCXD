/*
 * Lihtsustatud versioon, kus on ainult vajalik
 */

#include "btm9011_simple.h"

/* -----------------------------------------------------------------------
 * SDI byte: SDO_SEL | INA | INB | PWM | SEL | SEN_EN | SR | EN
 *
 *   Edasi  = 0101 1111 = 0x5F  (INA=1, INB=0, PWM=1, EN=1)
 *   Tagasi = 0011 1111 = 0x3F  (INA=0, INB=1, PWM=1, EN=1)
 *   Pidur  = 0001 1111 = 0x1F  (INA=0, INB=0, PWM=1, EN=1)
 *
 *	PWM on tehtud tarkvaras, sest ainult neljas bit on PWM jaoks (1 või 0)
 *
 * ----------------------------------------------------------------------- */

// Edasi, tagasi ja pidurdus byteid
#define CMD_FORWARD  0x5F
#define CMD_REVERSE  0x3F
#define CMD_BRAKE    0x1F

// Määrab btm_init käigus SPI, porti ja pini
static SPI_HandleTypeDef *s_hspi;
static GPIO_TypeDef      *s_cs_port;
static uint16_t           s_cs_pin;


// Saadab kahele SPI chipile info, mis on daisy-chain ühenduses
// Esimene on tegelikult kaskaadis viiane kiip
static void send_two(uint8_t far_chip, uint8_t near_chip)
{
	// Massiiv, kus on mõlema kiibi byte
    uint8_t tx[2] = { far_chip, near_chip };

    // Enne CS tuleb HIGH tõmmata, siis kirjutab info ja LOW tõmmates lukustub ja töötleb andmed
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_SET);
    HAL_SPI_Transmit(s_hspi, tx, 2, 100);
    HAL_GPIO_WritePin(s_cs_port, s_cs_pin, GPIO_PIN_RESET);
}


/* ---------------------------------------------------------------------- */

// Init, mis teeb setupis teiste initidega, määrab SPI, port ja pin ning tõmbab CS pinni LOW
void btm_init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin)
{
    s_hspi    = hspi;
    s_cs_port = cs_port;
    s_cs_pin  = cs_pin;

    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET); /* CS idle (low) */
}

//
void btm_drive(uint8_t left_dir, uint8_t right_dir)
{
    // Mootorid on samamoodi ühendatud, aga selletõttu on üks mootor tagurpidi
	// Siin parema mootori keerab tagurpidi tarkvaras
    uint8_t right_cmd;
    if      (right_dir == BTM_FORWARD) right_cmd = CMD_REVERSE;
    else if (right_dir == BTM_REVERSE) right_cmd = CMD_FORWARD;
    else                               right_cmd = CMD_BRAKE;

    uint8_t left_cmd;
    if      (left_dir == BTM_FORWARD) left_cmd = CMD_FORWARD;
    else if (left_dir == BTM_REVERSE) left_cmd = CMD_REVERSE;
    else                              left_cmd  = CMD_BRAKE;

    // Saadab info kasutades eelnevat funktsiooni
    send_two(right_cmd, left_cmd);
}

// Pidurdus funktsioon
void btm_brake(void)
{
    send_two(CMD_BRAKE, CMD_BRAKE);
}
