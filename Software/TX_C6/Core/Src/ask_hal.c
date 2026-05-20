#include "ask_hal.h"
#include "main.h"

ask_t ask433;
ask_t ask315; /* unused stub */

/* ── Microsecond counter using SysTick ────────────────────────────────────── */
static uint32_t micros_impl(void)
{
    uint32_t ms, sub;
    do {
        ms  = HAL_GetTick();
        sub = SysTick->LOAD - SysTick->VAL;
    } while (HAL_GetTick() != ms);
    return ms * 1000U + sub / (SystemCoreClock / 1000000U);
}

/* ── 433 MHz — TX only (no RX module wired on this board) ────────────────── */

void ask_write_pin_433(bool data)
{
    HAL_GPIO_WritePin(RF_DATA_GPIO_Port, RF_DATA_Pin,
                      data ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool ask_read_pin_433(void) { return false; }

uint32_t ask_micros_433(void) { return micros_impl(); }

void ask_init_rx433(void) {}
void ask_init_tx433(void) {}

void ask_delay_ms_433(uint32_t ms) { HAL_Delay(ms); }

void ask_delay_us_433(uint32_t us)
{
    uint32_t start = micros_impl();
    while ((micros_impl() - start) < us);
}

/* ── 315 MHz stubs ────────────────────────────────────────────────────────── */

void     ask_write_pin_315(bool d)    { (void)d; }
bool     ask_read_pin_315(void)       { return false; }
uint32_t ask_micros_315(void)         { return micros_impl(); }
void     ask_init_rx315(void)         {}
void     ask_init_tx315(void)         {}
void     ask_delay_ms_315(uint32_t d) { HAL_Delay(d); }
void     ask_delay_us_315(uint32_t d) { ask_delay_us_433(d); }
