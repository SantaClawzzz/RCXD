# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a **bare-metal STM32 remote control transmitter** firmware project. It transmits motor control commands over 433 MHz RF to a paired receiver, which drives BTM9011 motor drivers. The primary use case is a wireless remote-controlled vehicle.

- **MCU**: STM32F103C6Tx (ARM Cortex-M3, 48 MHz via HSE+PLL, 32 KB Flash, 10 KB RAM)
- **IDE/Build**: STM32CubeIDE (Eclipse CDT + GNU Make, arm-none-eabi-gcc)
- **Framework**: STM32 HAL

## Building

Open the project in **STM32CubeIDE** and use the IDE's build commands. There is no standalone Makefile — the Eclipse CDT `.cproject` drives the build.

- **Debug build**: Full debug symbols (`-g3`), no optimization, defines `DEBUG`
- **Release build**: Size-optimized (`-Os`), no debug symbols

To rebuild from the command line (requires STM32CubeIDE's toolchain in PATH):
```bash
# From project root, using Eclipse headless build
# arm-none-eabi-gcc with defines: USE_HAL_DRIVER, STM32F103x6
```

**Preprocessor defines** (both configs): `USE_HAL_DRIVER`, `STM32F103x6`

**Linker script**: `STM32F103C6TX_FLASH.ld` — FLASH: 32 KB at `0x8000000`, RAM: 10 KB at `0x20000000`, min heap: 512 B, min stack: 1 KB.

## Architecture

### Custom Drivers

**`Core/Src/rf433.c` / `Core/Inc/rf433.h`** — 433 MHz OOK radio (RadioHead RH_ASK compatible):
- Hardware: FS1000A TX on PA3, XY-MK-5V RX on PB11
- 2000 bps, 8× oversample → requires a 16 kHz timer ISR calling `RF433_TimerISR()`
- Packet format: `[12× 0x55 preamble][0x2D][0xD4][LEN][PAYLOAD...][CRC-8 poly=0x97]`, LSB-first
- Max payload: 60 bytes
- The timer ISR must be wired up externally; the driver is ISR-agnostic

**`Core/Src/btm9011.c` / `Core/Inc/btm9011.h`** — SPI daisy-chain motor driver:
- Up to 8 BTM9011 drivers per chain
- Command byte: `[0, IN1, IN2, PWM4..PWM0]` — 5-bit PWM (0–31), directions: Forward/Reverse/Coast/Brake
- Use `BTM9011_BuildCmd()` to construct command bytes, `BTM9011_Send()` to shift out the chain

### Main Loop

`Core/Src/main.c` initializes GPIO, I2C1 (400 kHz), and the RF433 driver, then continuously transmits 2-byte BTM9011 command packets. Each byte drives one motor. The debug LED on PB10 toggles on received packets.

### HAL Configuration

`Core/Inc/stm32f1xx_hal_conf.h` — only GPIO, I2C, RCC, DMA, Flash, Power, EXTI, and Cortex HAL modules are enabled. SPI, UART, ADC, TIM, CAN, etc. are disabled to reduce code size.

### Interrupt Architecture

- `Core/Src/stm32f1xx_it.c` — interrupt handlers; the 16 kHz timer handler must call `RF433_TimerISR()`
- `Core/Startup/startup_stm32f103c6tx.s` — vector table and reset handler (copies `.data`, zeros `.bss`, calls `SystemInit()` then `main()`)

### Pin Assignments

| Pin | Function |
|-----|----------|
| PA0 | Joystick vertical (analog) |
| PA1 | Joystick horizontal (analog) |
| PA2 | Joystick select button |
| PA3 | RF433 TX data (FS1000A) |
| PB10 | Debug LED |
| PB11 | RF433 RX data (XY-MK-5V) |

I2C1 (PB6/PB7) is initialized but not currently used in the main loop.

## Modifying with STM32CubeMX

The `.ioc` file (`TX_C6.ioc`) was generated with CubeMX 6.16.1. Re-generating from CubeMX will overwrite HAL init code in `main.c` (inside `/* USER CODE BEGIN/END */` blocks are preserved, but check `stm32f1xx_hal_msp.c` and `stm32f1xx_it.c` after regeneration).
