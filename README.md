### Introduction
The idea is simple. The transit board should display in how many minutes the U2, S1 and U3 depart from specific stations. In my case I want to:
- know when the next three U2 depart from Hasenbergl towards Messestadt Ost
- know the next three S1 departures from Feldmoching towards Freising
- know the next three U3 departures from Scheidplatz towards Fürstenried West
- (optional) display a short random quote at the bottom that changes every min. 

Since I travel to uni by taking the U2 and changing at Scheidplatz to U3, I want to know which U2 arrivals at Scheidplatz correspond exactly to U3 departures so that I don't have to wait. 

### Hardware
- 2.13inch e-Paper E-Ink Display HAT SPI Interface 250x122 from [AliExpress](https://de.aliexpress.com/item/33005909532.html) for €8,06 on 03.03.2026
- Waveshare ESP32 Epaper Driver Board for Waveshare SPI e-Paper from [AliExpress](https://de.aliexpress.com/item/1005009685786071.html) for €17,80 on 03.03.2026
- USB 3 to Type-C cable to flash data onto the board
- (optional) Small picture frame from Woolworth for around €2,00 or 3D print a custom case if you are very motivated and have a printer 

### Setup
#### Prerequisites
- You need to have Python installed and added to `$PATH` (e.g. 3.14.3)
- You need to have Arduino IDE installed (e.g. 2.3.6)

#### Arduino setup
In order for Arduino to recognize your Waveshare ESP32 board you need to:
- Let Arduino install all initial libraries
- Connect the Waveshare board to your PC
- Add `https://dl.espressif.com/dl/package_esp32_index.json` in _File/Preferences_ under _"Additional boards manager URLs"_
- Click on the _"Select Board"_ dropdown and from the _"Boards"_ dropdown choose _"ESP32 Dev Module"_ (it should now be active) and select your board USB connection under _"Ports"_

#### APIs
Since [MVG](https://www.mvg.de/) as of 05.04.2026 doesn't have an official public API, I am using the one from their app. For the quotes I am using [API Ninjas](https://api-ninjas.com/api/quotes).
