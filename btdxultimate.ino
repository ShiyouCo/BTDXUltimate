
/*
 HID to IIDX Entry model adapter
 Built for ESP32 + USB Host Mini shield
 Thanks to nagato for reverse engineering the IIDX Entry Model controller protocol

 Requires USB Host Shield Library 2.0

 For Dao controllers, use infinitas mode 
*/

#include <BLECharacteristic.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>

// Satisfy IDE, which only needs to see the include statment in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#include "hidjoystickrptparser.h"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint64_t value = 0;
uint8_t frameCounter = 0;
uint8_t btReport[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//uint64_t value1 = 0x23000100002300010000;
//uint64_t value2 = 0x22000100002200010000;
uint8_t iteration = 0;

unsigned long lastNotify = 0;

USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);
IIDXBTReport FinalReport;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


#define SERVICE_UUID "ff00"
#define CHARACTERISTIC_UUID "ff01"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



void setup() {
  Serial.begin(115200);

#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  Serial.println("Start");


  // Setup USB
  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay(200);

  if (!Hid.SetReportParser(0, &Joy))
    ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);

  // Create the BLE Device
  BLEDevice::init("IIDX Entry model");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  /*
    pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
  */
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  frameCounter = 0;
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  // Read from HID device, get 2 frames of data
  Usb.Task();
  FinalReport = JoyEvents.GetIIDXReport();
  btReport[0] = FinalReport.TT;
  btReport[1] = 0x00;
  btReport[2] = FinalReport.Btn;
  btReport[3] = FinalReport.EBtn;

  Usb.Task();
  FinalReport = JoyEvents.GetIIDXReport();
  btReport[5] = FinalReport.TT;
  btReport[6] = 0x00;
  btReport[7] = FinalReport.Btn;
  btReport[8] = FinalReport.EBtn;

  // notify changed value
  unsigned long timeSpanNotify = millis() - lastNotify;
  if (deviceConnected && timeSpanNotify > 50) {
    btReport[4] = frameCounter++;
    btReport[9] = frameCounter++;
    pCharacteristic->setValue((uint8_t*)&btReport, 10);
    pCharacteristic->notify();
    lastNotify = millis();
    //Serial.println("Notified");
    //delay(50); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
    frameCounter = 0;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("Connecting");
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    frameCounter = 0;
  }
}
