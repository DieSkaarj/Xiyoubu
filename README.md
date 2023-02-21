# Xiyoubu - Mega Drive Does What Genesis-done.

Xiyoubu is a hardware console mod for the Mega Drive/Genesis that targets ****MH-ATtiny88**** boards. It lets the Mega Drive change between 50Hz and 60Hz for video output (through scart) and enables language options for compatible software on the board. It also details a method for decreasing the power intake of the unit by way of a USB-C power port.

Xiyoubu will load the last saved profile on console power on.

***Xiyoubu has only been tested and confirmed working on a PAL Mega Drive II.***

Default config...

#### Controller
Menu I: **(START + A + C + [...])**
- **B**  - In-Game Reset
- **D-PAD**
- - **RIGHT** - Region Cycle Forward (see: fig. 1)
- - **LEFT** - Region Cycle Backward (see: fig. 1)
- - **UP** - Save current region to EEPROM
- - **DOWN** -  Check Overclocking (see: fig. 2)
Menu II: **(START + B + [...])**
- **D-PAD**
- - **UP** - Overclock (+) (Step up MAJ)
- - **RIGHT** - Overclock (+) (Step up MIN)
- - **DOWN** - Overclock (-) (Step down MAJ)
- - **LEFT** - Overclock (-) (Step down MIN)


#### Console
- Hold **RESET** to cycle regions
- **Tap once** to reset
- **Tap twice** to save region
- **Tap thrice** to enable/disable Xiyoubu controller handling (this option is saved and will be remembered after poweroff.)

*Fig. 1*
 |Region|LED| 
|---|---|
|USA|Cyan| 
|PAL|Red|
|JAP|Magenta|

*Fig. 2*
|Mhz|Color|
|---|---|
|7+|Green|
|8+|Yellow|
|9+|Red|
|10+|Magenta|
|11+|Blue|
|12+|White|

### Status
- v1.04
- - Tested on: MD2 VA1 PAL

### Features
- [X] Controller Region Cycling
- [X] Physical Region Cycling
- [X] Restore Region On Power On
- [X] In-Game Reset
- [X] Overclock
- [ ] Installation Walkthrough
- [X] Use Guide

### Other Mega Drive related projects from the Author
* Ko-fi blog discussing the development of Xiyoubu: https://ko-fi.com/dieskaarj
* BlastEd - Mega Drive/Genesis Art Tool: https://dieskaarj.itch.io/blasted

### Thank you for your hard work, research and resources
* **SukkoPera:** https://github.com/SukkoPera/MegaDrivePlusPlus
* **TurBoss:** https://github.com/TurBoss/MegaDrivePlusPlus/tree/overclock
* **Smudger D:** https://stompville.co.uk/?p=853
* **SpenceKonde:** https://github.com/SpenceKonde/ATTinyCore
* **Raspberryfield:** https://www.raspberryfield.life/2019/03/25/sega-mega-drive-genesis-6-button-xyz-controller/
* **Charles MacDonald:** https://gendev.spritesmind.net/forum/viewtopic.php?t=1262
* **Liam Riordan:** https://www.analog.com/media/en/technical-documentation/application-notes/AN-1070.pdf
* **Atmel:** http://ww1.microchip.com/downloads/en/DeviceDoc/atmel-9157-automotive-microcontrollers-attiny88_datasheet.pdf
* **Analog Devices:** https://www.analog.com/media/en/technical-documentation/data-sheets/ad9833.pdf

### License
Xiyoubu (software) is Copyright © 2023 by David 'Erosion' Oberlin.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your discretion) a later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see http://www.gnu.org/licenses/.
