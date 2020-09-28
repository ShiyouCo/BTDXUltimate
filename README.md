
# BTDXUltimate

Convert Infinitas compatible USB IIDX Controllers into bluetooth IIDX Entry model controller via ESP32 and USB Host Shield.

Since IIDX Ultimate Mobile doesn't support USB controllers on iOS, the idea is that you can plug in compatible USB IIDX controllers to the adapter and it will carry the controller data to IIDX Ultimate Mobile via bluetooth. This way, you can theoretically use any USB controller to play IIDX UM on iOS.

## Requirements

- ESP32 development board
- USB Host Shield Mini (or full size USB Host Shield)
- ESP32 IDE and USB Host Shield Library 2.0 installed on Arduino IDE.

## Compatibility

Here are the list of PCBs tested and confirmed(?) working with the code :

- Arduino Pro Micro with LeoDXUltimate firmware (this is my custom code)
- Dao SS001 PCB on latest firmware

## Wiring

NOTE: The clone version of USB Host Mini V2.0 doesn't have VBUS jumper which means it will only work with 3.3V USB devices. You have to cut a trace in order to be able to use 5 Volt USB devices.

You can check which trace needs to be cut here : 

(https://www.hackster.io/139994/plug-any-usb-device-on-an-esp8266-e0ca8a) 

but don't follow the ESP32 wiring there because it's incorrect.

ESP32 and USB Host Shield Mini communicates via SPI. Here are the pinouts used to connect them together : 

| ESP32 | USB Host Shield Mini |
|----|----|
|5V|5V|
|3V3|3V3|
|GND|GND|
|GPIO5|SS|
|GPIO19|MISO|
|GPIO23|MOSI|
|GPIO18|SCK|
|GPIO17|INT|
|EN|RST|

You might also want to provide power to the boards externally since running the wireless module while driving the controller, especially the one that has lights requires a lot of current.

## Credits

- Nagato for reverse engineering the IIDX Entry model bluetooth protocol


---

