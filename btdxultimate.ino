
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
//#include <hiduniversal.h>
#include "HIDUniPIDVID.h"
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
//HIDUniversal Hid(&Usb);
HIDUniPIDVID Hid(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);
IIDXBTReport FinalReport;

uint16_t oldVID = 0x00;
uint16_t oldPID = 0x00;
uint16_t connectedVID = 0x00;
uint16_t connectedPID = 0x00;
int connectedDevice = 4;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


#define SERVICE_UUID "ff00"
#define CHARACTERISTIC_UUID "ff01"

#define DAO_PID 0x0268
#define DAO_VID 0x054C

#define PREMIUM_MODEL_PID 0x1CCF
#define PREMIUM_MODEL_VID 0x8018

#define ENTRY_MODEL_PID 0x1CCF
#define ENTRY_MODEL_VID 0x1018

#define CON_TYPE_DAO 0
#define CON_TYPE_PREMIUM_MODEL 1
#define CON_TYPE_ENTRY_MODEL 2
#define CON_TYPE_UNKNOWN 3


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



  while (1) {
    Usb.Task();

    if ( Usb.getUsbTaskState() == USB_STATE_RUNNING ) break;
  }
  connectedVID = Hid.getVID();
  connectedPID = Hid.getPID();
  oldVID = connectedVID;
  oldPID = connectedPID;
  Serial.print("VID: 0x");
  Serial.println(connectedVID, HEX);
  Serial.print("PID: 0x");
  Serial.println(connectedPID, HEX);
  //PrintHex<uint16_t> (connectedVID, 16);
  connectedDevice = getDeviceType(connectedVID, connectedPID);
  Serial.println(connectedDevice);
}

void loop() {
  // Check VID/PID if it changes
  Usb.Task();
  connectedPID = Hid.getPID();
  connectedVID = Hid.getVID();
  if (connectedPID != oldPID || connectedVID != oldVID){
    Serial.println("Device changed!");
    connectedDevice = getDeviceType(connectedVID, connectedPID);
    oldPID = connectedPID;
    oldVID = connectedVID;
  }
  
  
  // Read from HID device, get 2 frames of data
  Usb.Task();
  FinalReport = getReportSelect(connectedDevice);
  btReport[0] = FinalReport.TT;
  btReport[1] = 0x00;
  btReport[2] = FinalReport.Btn;
  btReport[3] = FinalReport.EBtn;

  Usb.Task();
  FinalReport = getReportSelect(connectedDevice);
  btReport[5] = FinalReport.TT;
  btReport[6] = 0x00;
  btReport[7] = FinalReport.Btn;
  btReport[8] = FinalReport.EBtn;

  // notify changed value
  unsigned long timeSpanNotify = millis() - lastNotify;
  if (deviceConnected && timeSpanNotify > 10) {
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

int getDeviceType(uint16_t VID, uint16_t PID) {
  if (PID == PREMIUM_MODEL_PID && VID == PREMIUM_MODEL_VID) {
    Serial.println("Controller: IIDX Premium model");
    return CON_TYPE_PREMIUM_MODEL;
  }
  else if (PID == ENTRY_MODEL_PID && VID == ENTRY_MODEL_VID) {
    Serial.println("Controller: IIDX Entry model");
    return CON_TYPE_ENTRY_MODEL;
  }
  else if (PID == DAO_PID && VID == DAO_VID) {
    Serial.println("Controller: DJ Dao SS002");
    return CON_TYPE_DAO;
  }
  else {
    Serial.println("Controller: Unknown device");
    return CON_TYPE_UNKNOWN;
  }

}

IIDXBTReport getReportSelect(int connectedDevice) {
  IIDXBTReport retReport;
  switch (connectedDevice) {
    case CON_TYPE_ENTRY_MODEL:
      retReport = JoyEvents.GetIIDXReport();
      break;
    case CON_TYPE_PREMIUM_MODEL:
      retReport = JoyEvents.GetIIDXReport();
      break;
    case CON_TYPE_DAO:
      retReport = JoyEvents.GetDAOReport();
      break;
    case CON_TYPE_UNKNOWN:
      retReport = JoyEvents.GetIIDXReport();
      break;
  }
  return retReport;
}

