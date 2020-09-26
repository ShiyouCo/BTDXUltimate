
# BTDXUltimate

Convert Infinitas-compatible USB IIDX Controllers into bluetooth IIDX Entry model controller via ESP32 and USB Host Shield.


## Requirements

- ESP32 development board
- USB Host Shield Mini (or full size USB Host Shield)
- ESP32 IDE and USB Host Shield Library 2.0 installed on Arduino IDE.

## Compatibility

Here are the list of PCBs tested and confirmed working with the code :

- None so far (this is just a template)

## Wiring

NOTE: The clone version of USB Host Mini V2.0 doesn't have VBUS jumper which means it will only work with 3.3V USB devices. You have to cut a trace in order to be able to use 5 Volt USB devices.

Check out this page for the wiring guide :
https://www.hackster.io/139994/plug-any-usb-device-on-an-esp8266-e0ca8a

ESP32 and USB Host Shield Mini communicates via SPI.
| ESP32 | USB Host Shield Mini |
|----|----|
|GPIO15|SS|
|GPIO12|MISO|
|GPIO13|MOSI|
|GPIO14|SCK|
|5V|5V|
|3V3|3V3|
|GND|GND|
|EN|RST|
|GPIO5|INT|


## Credits

- Nagato for reverse engineering the IIDX Entry model bluetooth protocol


---

