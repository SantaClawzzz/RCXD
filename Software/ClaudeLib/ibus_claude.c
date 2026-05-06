#include "ibus_claude.h"
#include <string.h>

#define PKT_LEN  32
#define HDR_0    0x20U
#define HDR_1    0x40U

static IBUS_HandleTypeDef *s_hibus;
static UART_HandleTypeDef *s_huart;
static uint8_t             s_rx;
static uint8_t             s_buf[PKT_LEN];
static uint8_t             s_idx;

void IBUS_Init(IBUS_HandleTypeDef *hibus, UART_HandleTypeDef *huart)
{
    s_hibus = hibus;
    s_huart = huart;
    memset(hibus, 0, sizeof(*hibus));
    s_idx = 0;
    HAL_UART_Receive_IT(huart, &s_rx, 1);
}

/* Call from HAL_UART_RxCpltCallback */
void IBUS_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != s_huart->Instance) return;

    uint8_t b = s_rx;
    HAL_UART_Receive_IT(s_huart, &s_rx, 1);

    if (s_idx == 0 && b != HDR_0) return;
    if (s_idx == 1 && b != HDR_1) { s_idx = 0; return; }

    s_buf[s_idx++] = b;
    if (s_idx < PKT_LEN) return;
    s_idx = 0;

    /* Checksum: 0xFFFF - sum of first 30 bytes */
    uint16_t sum = 0;
    for (uint8_t i = 0; i < 30; i++) sum += s_buf[i];
    uint16_t rx_ck = (uint16_t)(s_buf[30] | ((uint16_t)s_buf[31] << 8));
    if (rx_ck != (uint16_t)(0xFFFFU - sum)) return;

    for (uint8_t i = 0; i < IBUS_CH_COUNT; i++)
        s_hibus->ch[i] = (uint16_t)(s_buf[2 + i*2] | ((uint16_t)s_buf[3 + i*2] << 8));

    s_hibus->updated = 1;
}
