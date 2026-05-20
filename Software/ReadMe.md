# Info

This folder contains all STM32 firmware projects for the RC-XD, written in STM32CubeIDE using HAL.<br>
All three projects target the STM32F103Cx (C6 or C8 — identical pinout, differ only in flash/RAM size).

## Projects

### TX_C6 — 433 MHz Transmitter (deprecated)

Custom transmitter firmware for the TX PCB.<br>
Reads joystick position via ADC, encodes a BTM9011 motor command, and transmits it over 433 MHz using the FS1000A module with a custom RF433 driver (RadioHead RH_ASK compatible, 2000 bps OOK).<br>
Abandoned because 433 MHz performance was too unreliable.

### RX_C8 — 433 MHz Receiver (deprecated)

Companion receiver firmware for the RX PCB, paired with TX_C6.<br>
Receives 433 MHz packets via the XY-MK-5V module and forwards decoded commands to two daisy-chained BTM9011 motor drivers over SPI.<br>
Abandoned alongside TX_C6 when 433 MHz was dropped.

### RX_C6 — iBUS Receiver (current)

Current working firmware for the RX PCB.<br>
Replaces the custom 433 MHz link with the Flysky FS-A8s 2.4 GHz receiver, reading channel data via the iBUS protocol over UART with DMA.<br>
Controls two daisy-chained BTM9011 motor drivers and a servo, with deadzone handling and clamped PWM steps.
