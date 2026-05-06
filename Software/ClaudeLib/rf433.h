#ifndef __RF433_H
#define __RF433_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/*
 * Simple 433 MHz OOK packet radio driver for FS1000A (TX) / XY-MK-5V (RX).
 *
 * ── Packet format (RadioHead RH_ASK compatible) ────────────────────────────
 *   [ 12× 0x55 preamble ] [ 0x2D ][ 0xD4 ] [ LEN ] [ PAYLOAD… ] [ CRC-8 ]
 *
 *   LEN     = payload_len + 1  (includes the CRC byte itself)
 *   CRC-8   = computed over [ LEN + PAYLOAD ], polynomial 0x97
 *   Bit order = LSB first
 *
 * ── Timer setup ────────────────────────────────────────────────────────────
 *   The library requires a TIMx update interrupt at RF433_TIMER_HZ (16 000 Hz).
 *   Call RF433_TimerISR() from HAL_TIM_PeriodElapsedCallback or the raw IRQ.
 *
 *   8 MHz HSI, no PLL:
 *     TIMx->PSC = 0
 *     TIMx->ARR = 499        →  8 000 000 / 500 = 16 000 Hz  ✓
 *
 * ── Quick start ────────────────────────────────────────────────────────────
 *   RF433_HandleTypeDef hrf;
 *   RF433_Init(&hrf, RF_DATA_GPIO_Port, RF_DATA_Pin,   // FS1000A DATA
 *                    PB4_GPIO_Port,     PB4_Pin);       // XY-MK-5V DATA
 *
 *   // In HAL_TIM_PeriodElapsedCallback:
 *   RF433_TimerISR(&hrf);
 *
 *   // Send a packet:
 *   uint8_t tx[] = {0x01, 0x10, 0xFF};
 *   RF433_Send(&hrf, tx, sizeof(tx));
 *
 *   // Receive (poll in main loop):
 *   if (RF433_Available(&hrf)) {
 *       uint8_t buf[RF433_MAX_PAYLOAD];
 *       uint8_t len;
 *       RF433_Receive(&hrf, buf, &len);
 *   }
 */

/* ── Compile-time constants ──────────────────────────────────────────────── */

#define RF433_BAUD_RATE      2000U    /* bits per second                    */
#define RF433_OVERSAMPLE     8U       /* timer ticks per bit period         */
#define RF433_TIMER_HZ       (RF433_BAUD_RATE * RF433_OVERSAMPLE) /* 16 000 */

#define RF433_PREAMBLE_LEN   12U      /* 0x55 bytes transmitted before sync */
#define RF433_MIN_PREAMBLE   4U       /* minimum 0x55 bytes to accept sync  */
#define RF433_MAX_PAYLOAD    60U      /* maximum user payload bytes         */

/* TX buffer: preamble + 2 sync + 1 len + payload + 1 crc */
#define RF433_TX_BUF_SIZE    (RF433_PREAMBLE_LEN + 2U + 1U + RF433_MAX_PAYLOAD + 1U)

/* ── Internal state enums ────────────────────────────────────────────────── */

typedef enum {
    RF433_RX_IDLE = 0,   /* waiting for preamble start  */
    RF433_RX_PREAMBLE,   /* accumulating 0x55 bytes     */
    RF433_RX_SYNC2,      /* received 0x2D, want 0xD4    */
    RF433_RX_LENGTH,     /* reading LEN byte            */
    RF433_RX_DATA,       /* reading payload bytes       */
    RF433_RX_CRC,        /* reading and checking CRC    */
} RF433_RxState;

typedef enum {
    RF433_TX_IDLE = 0,
    RF433_TX_ACTIVE,
} RF433_TxState;

/* ── Handle struct ────────────────────────────────────────────────────────── */

typedef struct {
    /* Hardware */
    GPIO_TypeDef *tx_port;
    uint16_t      tx_pin;
    GPIO_TypeDef *rx_port;
    uint16_t      rx_pin;

    /* TX state machine */
    volatile RF433_TxState tx_state;
    uint8_t  tx_buf[RF433_TX_BUF_SIZE];
    uint16_t tx_total;      /* total bytes in tx_buf              */
    uint16_t tx_byte;       /* index of byte currently being sent */
    uint8_t  tx_bit;        /* bit index within current byte      */
    uint8_t  tx_phase;      /* oversample tick counter 0–7        */

    /* RX state machine */
    RF433_RxState rx_state;
    uint8_t  rx_phase;          /* oversample tick counter 0–7        */
    uint8_t  rx_phase_scan;     /* persists across resets, advances 1 per reset */
    uint8_t  rx_samples;        /* 8 pin samples for current period   */
    uint8_t  rx_byte;           /* byte being assembled               */
    uint8_t  rx_bit;            /* bit index within rx_byte (0–7)     */
    uint8_t  rx_preamble_cnt;   /* consecutive valid 0x55 bytes seen  */
    uint8_t  rx_buf[RF433_MAX_PAYLOAD];
    uint8_t  rx_expected;       /* payload bytes expected             */
    uint8_t  rx_idx;            /* payload bytes received so far      */
    uint8_t  rx_crc;            /* running CRC-8                      */

    /* Debug: last byte assembled in PREAMBLE state (sticky, for debugger) */
    uint8_t dbg_last_preamble_byte;

    /* Received packet, written by ISR, consumed by application */
    volatile uint8_t rx_ready;
    uint8_t rx_pkt[RF433_MAX_PAYLOAD];
    uint8_t rx_pkt_len;
} RF433_HandleTypeDef;

/* ── Public API ───────────────────────────────────────────────────────────── */

/**
 * @brief  Initialise the RF433 handle. TX pin is driven low (idle).
 * @param  hrf      Handle to populate.
 * @param  tx_port  GPIO port connected to FS1000A DATA.
 * @param  tx_pin   GPIO pin mask of the TX data line.
 * @param  rx_port  GPIO port connected to XY-MK-5V DATA.
 * @param  rx_pin   GPIO pin mask of the RX data line.
 * @retval HAL_OK / HAL_ERROR on NULL arguments.
 */
HAL_StatusTypeDef RF433_Init(RF433_HandleTypeDef *hrf,
                              GPIO_TypeDef *tx_port, uint16_t tx_pin,
                              GPIO_TypeDef *rx_port, uint16_t rx_pin);

/**
 * @brief  Transmit a data packet.
 *         If a transmission is already in progress this call blocks until
 *         it finishes, then starts the new one.
 * @param  data  Payload (1–RF433_MAX_PAYLOAD bytes).
 * @param  len   Number of payload bytes.
 * @retval HAL_OK on success, HAL_ERROR if arguments are invalid.
 */
HAL_StatusTypeDef RF433_Send(RF433_HandleTypeDef *hrf,
                              const uint8_t *data, uint8_t len);

/**
 * @brief  Returns 1 if a validated packet is waiting, 0 otherwise.
 */
uint8_t RF433_Available(RF433_HandleTypeDef *hrf);

/**
 * @brief  Copy the last received packet into caller-supplied buffers.
 *         Clears the ready flag so the next packet can be received.
 * @param  data  Destination buffer (must be ≥ RF433_MAX_PAYLOAD bytes).
 * @param  len   Set to the payload length on return.
 * @retval HAL_OK if a packet was waiting, HAL_ERROR if not.
 */
HAL_StatusTypeDef RF433_Receive(RF433_HandleTypeDef *hrf,
                                 uint8_t *data, uint8_t *len);

/**
 * @brief  Must be called from the timer update ISR at exactly RF433_TIMER_HZ.
 *         Drives the TX state machine and samples the RX pin.
 */
void RF433_TimerISR(RF433_HandleTypeDef *hrf);

#ifdef __cplusplus
}
#endif

#endif /* __RF433_H */
