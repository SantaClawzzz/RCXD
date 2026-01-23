## Introduction
Project repository for RC-XD<br>
A remote controlled car project, currently only containing the reciever project files<br>
The project design description can be found [HERE](https://docs.google.com/document/d/1IkSoV4714EhC4LcSWzFsmU4UmifhLQG45a5PKD9-OiM/edit?tab=t.0)<br>
The project timeline is [HERE](https://docs.google.com/document/d/1Baed8mpfqgpyJ5yqUqOmzXT9Kg_jJRIP0yAwD-ANLqw/edit?usp=sharing)

The project is divided into 3 parts: Electronics, Mechanics, Programming<br>
Currently its all under one branch and also there is only the electronics part.<br>
The Projects are seperated by folders only, in the future will be by folders.

## Components
The project has had 2 MCU changes and 1 motor driver change:
MCU:
  * ATMega328p-AU
  * ESP32-C6-WROOM-1U
  * STM32F103Cx

The STM chip not only has good ADC readings, USB OTG, reliable, librarys for STM32CubeIDE and Arduino
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

## Parameters
* 3S 12V battery, since motors are 12V
* 5A stall current but under load should hover less than 1A
* USB-C for programming
* Includes headers for:
  * 3x [TOF](https://www.pololu.com/product/3415) sensors (I2C)
  * 1x [OLED](https://www.adafruit.com/product/931?srsltid=AfmBOooS0SgYaJRqBKaiZJ_inS_Hm55apfdVYE57VtUnGSEqd8KzymMf) screen (I2C) - No RST or VIN
  * 1x [XY-MK-5V](https://radiolux.com.ua/files/pdf/RFmodule.pdf) (GPIO)
* Mechanical size - 37x40mm

Main issue right now is the power for the motors, since they take 1A probably by normal and 5A at stall. So the traces should withstand 5A, but they at max withstand 2A.<br>
This should be fine but might need a redesign with v2 or even before the first batch.
Also an idea is active cooling, so if the PCB can't withstand it maybe with active cooling it can
