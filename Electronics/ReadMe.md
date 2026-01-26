# Info
The PCBs both have 2 major revisions, (TMC and PreIDC have never been ordered).<br>
The newer ones will be documented with only changes mentioned about the previous revisions.

No document except the design description is present.

# TX

The transmitter PCB is essentialy the controller. Most of it is buttons and joystick routing with IDC connectors.<br>
Also includes 3.3V and 5V LDO regulators, where 5V is unused and routed to external header. 3.3V powers the MCU and logic pull-ups.<br>

## Components

The project has had 2 MCU changes:
MCU:
  * ATMega328p-AU
  * ESP32-C6-WROOM-1U
  * STM32F103Cx

Buttons and joystick are routed to IDC headers which are easier to crimp by hand and allows for the mechanical design to be flexible. TX header is still the default 2.54mm header, which connection can be concerning as it doesn't have any locking mechanism.

Design is done with panelization in mind, so not every switch and joystick has to be ordered separately.

* MCU - [STM32F103Cx](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
* Battery - 7.4V [TBA]()
* Regulators:
  * [3V3 LDO](https://www.digikey.ee/en/products/detail/umw/AMS1117-3-3/17635254)
  * [5V LDO](https://www.digikey.ee/en/products/detail/evvo/ams1117-5-0/24370130)
* USB-C - [THT variant](https://www.mouser.com/catalog/specsheets/GCT_usb4085.pdf?_gl=1*15nugvh*_gcl_aw*R0NMLjE3Njg0MTMyNTIuQ2p3S0NBaUFtcDNMQmhBa0Vpd0FKTTJKVU44WDVSWVA4amZXTDBjeFJhZEJOdjlhMVdXcWZKU0NzN3hBcUdXb3V4eG5vY1Ayekc4ZEVCb0NtUHdRQXZEX0J3RQ..*_gcl_au*OTU4NjA5MDM5LjE3NjM5MTExODMuOTEwNDEwNzc4LjE3Njc0NjY5ODAuMTc2NzQ2Njk4MA..*_ga*MTEzMzQ4OTA2Ny4xNzYxNzQ3MjEz*_ga_15W4STQT4T*czE3Njg0MTIyODMkbzEzJGcxJHQxNzY4NDEzMjUyJGo1NyRsMCRoMA..)
* [Button](https://www.digikey.ee/en/products/detail/e-switch/tl2201eexb/1805495)
* [Joystick](https://www.digikey.ee/en/products/detail/c-k/thb001p/11687191)

## Mechanical parameters:
  * 34.5mm x 80mm
    * Joystick (30mm x 24mm)
    * Button (16mm x 19mm)
    * MCU (34.5mm x 27.5mm)
  * M2 screws

## Power

LDOs are used since max current is ~100mA. 5V LDO is unused but connects with USB-C VBUS and cascades into 3.3V LDO. 5V is routed to an external header for possible future use.
Switching regulators take more space, cost more and require more thought into the design. Since there is no need, there is no need to use extra resources.

## Logic

For the logic and data processing the STM32F103Cx is used. C6 and C8 both have identical pinouts and only differ by the internal RAM size.<br>
STM32F103C6/8 design is based of the "Blue pill" and can be programmed in Arduino or STM32. STM is preferable as it gives lower level access and has faster data processing.<br>
A lot of pins are unused and this could be considered overkill for this design but it's the same MCU used for the RX, which saves time on complications and differences on the two boards.

# RX

The receiver PCB is mostly the motor driver with logic being received from the RX PCB. So power paths are most crucial part of the design. It's quite tight and might have some errors.<br>
Instead of LDOs, switching regulators are used since the current draw is much higher.

## Components
The project has had 2 MCU and 1 motor driver change:
MCU:
  * ATMega328p-AU
  * ESP32-C6-WROOM-1U
  * STM32F103Cx

The STM chip has good ADC readings, USB OTG, reliable, librarys for STM32CubeIDE and Arduino and available resources / designs like the STM32 "Blue Pill"<br>
The video (which was planned at first with ESP32) will be done externally, with seperate VTX/RTX modules

DRV:
  * TMC5160/A
  * BTM9011

BTM chip was chosen after it was determined the TMC chip is for stepper motors and controls them assuming they're on one axis (meaning if ones slows down, is fked)
So with the BTM chip we gain:
  * Internal MOSFETS, so smaller design
  * Cheaper chip: 1.5€ vs 7€ (not including the external components)
  * Less complicated (PCB design as well as programming/datasheet)
  * Does what we actually want
  * Also it has current sensing and should have under- and overvoltage protection with current/stall protection

* MCU - [STM32F103Cx](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
* Driver - [BTM9011](https://www.infineon.com/assets/row/public/documents/10/49/infineon-infineon-btm901xep-ds-en-datasheet-en.pdf?fileId=8ac78c8c90530b3a01912d365ee4326f)
* Battery - 12V [TBA]()
* Motors - [Pololu 4842](https://www.pololu.com/product/4842)
* Regulators - [5V buck](https://www.mouser.com/catalog/specsheets/Renesas_01042024_REN_RAA211803-805_DST_20231002.pdf?_gl=1*10scs8a*_gcl_aw*R0NMLjE3NjgzOTA2NDEuQ2p3S0NBaUFtcDNMQmhBa0Vpd0FKTTJKVURzUk9TUXQxWmpXRGZkQ1J0RVJScEJabmRhX29NOGs4aXY4aWNjLXpVYzYyaE90TGw0a3Z4b0NjNW9RQXZEX0J3RQ..*_gcl_au*MTU4MDkwODY2NS4xNzY1MTg3OTk1LjEzODYxMzQwMDIuMTc2ODQwMDAxNi4xNzY4NDAxMDQy*_ga*MTA4Mzc2NDMyOC4xNzU5OTE3NDM2*_ga_15W4STQT4T*czE3Njg0MTMxMzgkbzMyJGcxJHQxNzY4NDEzMTQyJGo1NiRsMCRoMA..) and [3V3 buck](https://www.mouser.com/catalog/specsheets/Renesas_01042024_REN_RAA211803-805_DST_20231002.pdf?_gl=1*14azvdg*_gcl_aw*R0NMLjE3NjgzOTA2NDEuQ2p3S0NBaUFtcDNMQmhBa0Vpd0FKTTJKVURzUk9TUXQxWmpXRGZkQ1J0RVJScEJabmRhX29NOGs4aXY4aWNjLXpVYzYyaE90TGw0a3Z4b0NjNW9RQXZEX0J3RQ..*_gcl_au*MTU4MDkwODY2NS4xNzY1MTg3OTk1LjEzODYxMzQwMDIuMTc2ODQwMDAxNi4xNzY4NDAxMDQy*_ga*MTA4Mzc2NDMyOC4xNzU5OTE3NDM2*_ga_15W4STQT4T*czE3Njg0MTMxMzgkbzMyJGcxJHQxNzY4NDEzMTY0JGozNCRsMCRoMA..)
* USB-C - [THT variant](https://www.mouser.com/catalog/specsheets/GCT_usb4085.pdf?_gl=1*15nugvh*_gcl_aw*R0NMLjE3Njg0MTMyNTIuQ2p3S0NBaUFtcDNMQmhBa0Vpd0FKTTJKVU44WDVSWVA4amZXTDBjeFJhZEJOdjlhMVdXcWZKU0NzN3hBcUdXb3V4eG5vY1Ayekc4ZEVCb0NtUHdRQXZEX0J3RQ..*_gcl_au*OTU4NjA5MDM5LjE3NjM5MTExODMuOTEwNDEwNzc4LjE3Njc0NjY5ODAuMTc2NzQ2Njk4MA..*_ga*MTEzMzQ4OTA2Ny4xNzYxNzQ3MjEz*_ga_15W4STQT4T*czE3Njg0MTIyODMkbzEzJGcxJHQxNzY4NDEzMjUyJGo1NyRsMCRoMA..)

* Includes headers for:
  * 3x [TOF](https://www.pololu.com/product/3415) sensors (I2C)
  * 1x [OLED](https://www.adafruit.com/product/931?srsltid=AfmBOooS0SgYaJRqBKaiZJ_inS_Hm55apfdVYE57VtUnGSEqd8KzymMf) screen (I2C) - No RST or VIN
  * 1x [XY-MK-5V](https://radiolux.com.ua/files/pdf/RFmodule.pdf) (GPIO)

## Power
* 3S 12V battery, since motors are 12V
* 5A stall current but under load should hover less than 1A
* USB-C for programming and VBUS through diode to 5V

Main issue right now is the power for the motors, since they take 1A probably by normal and 5A at stall. So the traces should withstand 5A, but they at max withstand 2A.<br>
This should be fine but might need a redesign with v2 or even before the first batch.<br>
Also an idea is active cooling, so if the PCB can't withstand it maybe with active cooling it can

## Mechanical parameters
  * 37x40mm
  * M2 screws

## Logic

For the logic the same STM32F103Cx MCU is used.<br>
It takes in data from the TX PCB and gives signals to the drivers, gets data through SPI from drivers and also current sense which it has to do on RX board since it has no way of sending info to the TX board.<br>
Some extra pins are drawn to unused GPIO headers.