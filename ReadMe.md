## Introduction
Project repository for RC-XD<br>
A remote controlled car project, currently only containing the reciever project files<br>
The project design description can be found [HERE](https://docs.google.com/document/d/1IkSoV4714EhC4LcSWzFsmU4UmifhLQG45a5PKD9-OiM/edit?tab=t.0)<br>
The project timeline is [HERE](https://docs.google.com/document/d/1Baed8mpfqgpyJ5yqUqOmzXT9Kg_jJRIP0yAwD-ANLqw/edit?usp=sharing)

## Components
* MCU - [STM32F103Cx](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
* Driver - [TMC5160/A](https://www.analog.com/media/en/technical-documentation/data-sheets/TMC5160A_datasheet_rev1.18.pdf)
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
* Mechanical size - 78x44 (Will be 80x44 after update)