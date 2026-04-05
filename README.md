### Introduction

### Hardware
- 2.13inch e-Paper E-Ink Display HAT SPI Interface 250x122 from [AliExpress](https://de.aliexpress.com/item/33005909532.html) for €8,06 on 03.03.2026
- Waveshare ESP32 Epaper Driver Board for Waveshare SPI e-Paper from [AliExpress](https://de.aliexpress.com/item/1005009685786071.html) for €17,80 on 03.03.2026
- USB 3 to Type-C cable to flash data onto the board 

### Setup
#### Prerequisites
- You need to have Python installed and added to ´$PATH´ (e.g. 3.14.3)
- You need to have Arduino IDE installed (e.g. 2.3.6)

#### Arduino setup
In order for Arduino to recognize your Waveshare ESP32 board you need to:
- Let Arduino install all initial libraries
- Connect the Waveshare board to your PC
- Add ´https://dl.espressif.com/dl/package_esp32_index.json´ in _File/Preferences_ under _"Additional boards manager URLs"_
- Click on the _"Select Board"_ dropdown and from the _"Boards"_ dropdown choose _"ESP32 Dev Module"_ (it should now be active) and select your board USB connection under _"Ports"_

