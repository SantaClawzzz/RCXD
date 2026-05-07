/*
 * Lihtsustatud BTM9011 teek
 *
 * Saadab 8-bitise SPI käsu
 *
 * SPI byte (SDI, MSB enne):
 *   Bit 7 (SDO_SEL) : 0 = echo control byte on SDO,  1 = echo status byte
 *   Bit 6 (INA)     : suuna kontroll A
 *   Bit 5 (INB)     : suuna kontroll B
 *   Bit 4 (PWM)     : 1 = täis kiirus,  0 = null kiirus
 *   Bit 3 (SEL)     : daisy-chain seade valimine
 *   Bit 2 (SEN_EN)  : voolumõõtmis lubamine (1 - jah, 0 - ei)
 *   Bit 1 (SR)      : pöördekiiruse valimine
 *   Bit 0 (EN)      : kiibi töötamine (1 - töötab, 0 - kinni)
 *
 * Tõeväärtustabel:
 *   INA=1, INB=0  →  Edasi
 *   INA=0, INB=1  →  Tagasi
 *   INA=0, INB=0  →  Pidurdus
 *   INA=1, INB=1  →  Vaba veeremine - ei kasuta
 *
 */

#ifndef BTM9011_SIMPLE_H
#define BTM9011_SIMPLE_H

#include "stm32f1xx_hal.h"

// Suunade määramine
#define BTM_FORWARD  1
#define BTM_REVERSE  2
#define BTM_BRAKE    0

/*
 * Salvestab SPI, port ja pini
 * Kutsu käsi ühe korra enne main();
 */
void btm_init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *cs_port, uint16_t cs_pin);

/*
 *	btm_drive - funktsioon, mis paneb mootorid liikuma
 *	BTM_FORWARD - edasi
 *	BTM_REVERSE - tagurpidi
 *	BTM_BRAKE	- pidurdus

 * 	Parem mootor on tagurpidi
 * 	Juba vahetatud funktsiooni sees
 * 	Alati mõlemad otse või tagurpidi
 */
void btm_drive(uint8_t left_dir, uint8_t right_dir);

// Pidurdus funktsioon, ei vaja sisendit
void btm_brake(void);

#endif /* BTM9011_SIMPLE_H */
