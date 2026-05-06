#include "rf433.h"
#include <string.h>

#define PREAMBLE_BYTE  0x55U
#define SYNC_HI        0x2DU
#define SYNC_LO        0xD4U

/* ── CRC-8, polynomial 0x97 (RadioHead compatible) ────────────────────────── */

static uint8_t crc8_byte(uint8_t crc, uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++) {
        if ((crc ^ data) & 0x80U)
            crc = (uint8_t)((crc << 1) ^ 0x97U);
        else
            crc = (uint8_t)(crc << 1);
        data <<= 1;
    }
    return crc;
}

static uint8_t crc8_buf(const uint8_t *buf, uint8_t len)
{
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++)
        crc = crc8_byte(crc, buf[i]);
    return crc;
}

/* ── Bit helpers ──────────────────────────────────────────────────────────── */

/*
 * Majority vote over 8 samples.
 * Uses Brian Kernighan popcount — no division, no lookup table.
 */
static uint8_t bit_value(uint8_t samples)
{
    uint8_t cnt = 0, v = samples;
    while (v) { cnt++; v &= (uint8_t)(v - 1U); }
    return (cnt >= 4U) ? 1U : 0U;
}

/* ── RX helpers ───────────────────────────────────────────────────────────── */

/*
 * Push one received bit (LSB first) into hrf->rx_byte.
 * Returns 1 when a full byte is ready (rx_bit has wrapped to 0).
 */
static uint8_t rx_push_bit(RF433_HandleTypeDef *hrf, uint8_t bit)
{
    hrf->rx_byte |= (uint8_t)(bit << hrf->rx_bit);
    if (++hrf->rx_bit < 8)
        return 0;

    hrf->rx_bit = 0;
    return 1;
}

/* Reset RX state machine to IDLE.
 * Pass advance_phase=1 on decode failure to scan to the next sample window.
 * Pass advance_phase=0 on success to keep the phase that just worked. */
static void rx_reset(RF433_HandleTypeDef *hrf, uint8_t advance_phase)
{
    if (advance_phase)
        hrf->rx_phase_scan = (hrf->rx_phase_scan + 1U) & 0x07U;
    hrf->rx_phase        = hrf->rx_phase_scan;
    hrf->rx_state        = RF433_RX_IDLE;
    hrf->rx_byte         = 0;
    hrf->rx_bit          = 0;
    hrf->rx_preamble_cnt = 0;
    hrf->rx_crc          = 0;
    hrf->rx_idx          = 0;
    hrf->rx_expected     = 0;
    hrf->rx_samples      = 0;
}

/* ── Public API ───────────────────────────────────────────────────────────── */

HAL_StatusTypeDef RF433_Init(RF433_HandleTypeDef *hrf,
                              GPIO_TypeDef *tx_port, uint16_t tx_pin,
                              GPIO_TypeDef *rx_port, uint16_t rx_pin)
{
    if (!hrf || !tx_port || !rx_port)
        return HAL_ERROR;

    memset(hrf, 0, sizeof(*hrf));
    hrf->tx_port = tx_port;
    hrf->tx_pin  = tx_pin;
    hrf->rx_port = rx_port;
    hrf->rx_pin  = rx_pin;

    HAL_GPIO_WritePin(tx_port, tx_pin, GPIO_PIN_RESET); /* TX idle low */
    return HAL_OK;
}

HAL_StatusTypeDef RF433_Send(RF433_HandleTypeDef *hrf,
                              const uint8_t *data, uint8_t len)
{
    if (!hrf || !data || len == 0 || len > RF433_MAX_PAYLOAD)
        return HAL_ERROR;

    /* Block until any ongoing transmission finishes */
    while (hrf->tx_state == RF433_TX_ACTIVE)
        ;

    uint8_t  *p   = hrf->tx_buf;
    uint16_t  idx = 0;

    /* Preamble — 0x55 LSB-first starts with '1', letting the receiver AGC
     * settle and giving the RX state machine time to achieve bit sync.      */
    for (uint8_t i = 0; i < RF433_PREAMBLE_LEN; i++)
        p[idx++] = PREAMBLE_BYTE;

    /* Sync word */
    p[idx++] = SYNC_HI;
    p[idx++] = SYNC_LO;

    /* LEN byte = payload length + 1 CRC byte */
    uint8_t pkt_len = len + 1U;
    p[idx++] = pkt_len;

    /* Payload */
    memcpy(&p[idx], data, len);
    idx += len;

    /* CRC-8 over [ LEN byte, PAYLOAD bytes ] */
    p[idx++] = crc8_buf(&p[RF433_PREAMBLE_LEN + 2U], 1U + len);

    hrf->tx_total = idx;
    hrf->tx_byte  = 0;
    hrf->tx_bit   = 0;
    hrf->tx_phase = 0;
    hrf->tx_state = RF433_TX_ACTIVE; /* arm the ISR */

    return HAL_OK;
}

uint8_t RF433_Available(RF433_HandleTypeDef *hrf)
{
    return (hrf != NULL) ? hrf->rx_ready : 0U;
}

HAL_StatusTypeDef RF433_Receive(RF433_HandleTypeDef *hrf,
                                 uint8_t *data, uint8_t *len)
{
    if (!hrf || !data || !len || !hrf->rx_ready)
        return HAL_ERROR;

    *len = hrf->rx_pkt_len;
    memcpy(data, hrf->rx_pkt, hrf->rx_pkt_len);
    hrf->rx_ready = 0;
    return HAL_OK;
}

/* ── Timer ISR — call at RF433_TIMER_HZ from TIMx update interrupt ────────── */

void RF433_TimerISR(RF433_HandleTypeDef *hrf)
{
    /* ── TX ──────────────────────────────────────────────────────────────── */
    if (hrf->tx_state == RF433_TX_ACTIVE) {
        if (hrf->tx_phase == 0) {
            /* Start of a new bit period: output next bit */
            if (hrf->tx_byte < hrf->tx_total) {
                uint8_t bit = (hrf->tx_buf[hrf->tx_byte] >> hrf->tx_bit) & 1U;
                HAL_GPIO_WritePin(hrf->tx_port, hrf->tx_pin,
                                  bit ? GPIO_PIN_SET : GPIO_PIN_RESET);

                if (++hrf->tx_bit == 8) {
                    hrf->tx_bit = 0;
                    hrf->tx_byte++;
                }
            } else {
                /* All bytes sent — drive TX low and go idle */
                HAL_GPIO_WritePin(hrf->tx_port, hrf->tx_pin, GPIO_PIN_RESET);
                hrf->tx_state = RF433_TX_IDLE;
            }
        }
        hrf->tx_phase = (hrf->tx_phase + 1U) & 0x07U;
    }

    /* ── RX sampling ─────────────────────────────────────────────────────── */
    uint8_t sample = (HAL_GPIO_ReadPin(hrf->rx_port, hrf->rx_pin) == GPIO_PIN_SET)
                     ? 1U : 0U;

    /* Pack 8 single-bit samples into a byte; shift in newest on the right */
    hrf->rx_samples = (uint8_t)((hrf->rx_samples << 1) | sample);
    hrf->rx_phase   = (hrf->rx_phase + 1U) & 0x07U;

    if (hrf->rx_phase != 0)
        return; /* still collecting samples for this bit period */

    /* ── RX decode: one complete bit period ──────────────────────────────── */
    uint8_t bit = bit_value(hrf->rx_samples);
    hrf->rx_samples = 0;

    switch (hrf->rx_state) {

    /* ── IDLE: wait for the first '1' (start of 0x55 preamble) ──────────── */
    case RF433_RX_IDLE:
        if (bit) {
            /* First bit of 0x55 is '1' (LSB=1), start byte assembly */
            hrf->rx_byte = 1U;
            hrf->rx_bit  = 1;
            hrf->rx_state = RF433_RX_PREAMBLE;
        }
        break;

    /* ── PREAMBLE: accumulate 0x55 bytes, then hand off to sync ─────────── */
    case RF433_RX_PREAMBLE:
        if (!rx_push_bit(hrf, bit))
            break; /* byte not yet complete */

        hrf->dbg_last_preamble_byte = hrf->rx_byte; /* sticky for debugger */

        if (hrf->rx_byte == PREAMBLE_BYTE) {
            hrf->rx_preamble_cnt++;
            hrf->rx_byte = 0;
        } else if (hrf->rx_preamble_cnt >= RF433_MIN_PREAMBLE &&
                   hrf->rx_byte == SYNC_HI) {
            /* End of preamble — first sync byte already decoded */
            hrf->rx_state = RF433_RX_SYNC2;
            hrf->rx_byte  = 0;
        } else {
            rx_reset(hrf, 1); /* spurious data — try next phase */
        }
        break;

    /* ── SYNC2: expect second sync byte 0xD4 ────────────────────────────── */
    case RF433_RX_SYNC2:
        if (!rx_push_bit(hrf, bit))
            break;

        if (hrf->rx_byte == SYNC_LO) {
            hrf->rx_state = RF433_RX_LENGTH;
            hrf->rx_byte  = 0;
        } else {
            rx_reset(hrf, 1);
        }
        break;

    /* ── LENGTH: read LEN byte, validate, start CRC ─────────────────────── */
    case RF433_RX_LENGTH: {
        if (!rx_push_bit(hrf, bit))
            break;

        uint8_t total = hrf->rx_byte;   /* payload + 1 CRC byte */

        if (total < 2U || (total - 1U) > RF433_MAX_PAYLOAD) {
            rx_reset(hrf, 1);
            break;
        }

        hrf->rx_crc      = crc8_byte(0U, total); /* CRC begins with LEN */
        hrf->rx_expected = total - 1U;            /* pure payload count  */
        hrf->rx_idx      = 0;
        hrf->rx_byte     = 0;
        hrf->rx_state    = RF433_RX_DATA;
        break;
    }

    /* ── DATA: read payload bytes, update running CRC ────────────────────── */
    case RF433_RX_DATA:
        if (!rx_push_bit(hrf, bit))
            break;

        hrf->rx_buf[hrf->rx_idx++] = hrf->rx_byte;
        hrf->rx_crc  = crc8_byte(hrf->rx_crc, hrf->rx_byte);
        hrf->rx_byte = 0;

        if (hrf->rx_idx == hrf->rx_expected)
            hrf->rx_state = RF433_RX_CRC;
        break;

    /* ── CRC: compare received byte with computed CRC ────────────────────── */
    case RF433_RX_CRC:
        if (!rx_push_bit(hrf, bit))
            break;

        if (hrf->rx_byte == hrf->rx_crc && !hrf->rx_ready) {
            /* Valid packet — hand off to application */
            memcpy(hrf->rx_pkt, hrf->rx_buf, hrf->rx_expected);
            hrf->rx_pkt_len = hrf->rx_expected;
            hrf->rx_ready   = 1;
            rx_reset(hrf, 0); /* keep the phase that decoded this packet */
        } else {
            rx_reset(hrf, 1); /* CRC fail — try next phase */
        }
        break;

    default:
        rx_reset(hrf, 1);
        break;
    }
}
