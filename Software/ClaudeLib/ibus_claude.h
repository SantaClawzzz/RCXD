#ifndef __IBUS_H
#define __IBUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#define IBUS_CH_COUNT  14

typedef struct {
    uint16_t         ch[IBUS_CH_COUNT]; /* channel values, ~1000–2000 */
    volatile uint8_t updated;           /* set by ISR, cleared by application */
} IBUS_HandleTypeDef;

void IBUS_Init(IBUS_HandleTypeDef *hibus, UART_HandleTypeDef *huart);
void IBUS_RxCpltCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* __IBUS_H */
