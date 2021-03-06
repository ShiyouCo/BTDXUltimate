#include "hidjoystickrptparser.h"

JoystickReportParser::JoystickReportParser(JoystickEvents *evt) :
  joyEvents(evt),
  oldHat(0xDE),
  oldButtons(0) {
  for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
    oldPad[i] = 0xD;
}

void JoystickReportParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  bool match = true;


  // Checking if there are changes in report since the method was last called


  for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++)
    if (buf[i] != oldPad[i]) {
      match = false;
      break;
    }
  /*
    if (!match) {
    Serial.print("RAW DATA : ");
    for (uint8_t i = 0; i < RPT_GEMEPAD_LEN + 2; i++) {
     Serial.print("0x");
     Serial.print(buf[i], HEX);
     Serial.print(" ");
    }
    Serial.println("");
    }
  */
  // Calling Game Pad event handler
  if (!match && joyEvents) {
    joyEvents->OnGamePadChanged((const GamePadEventData*)buf);

    for (uint8_t i = 0; i < RPT_GEMEPAD_LEN; i++) oldPad[i] = buf[i];
  }

  uint8_t hat = (buf[5] & 0xF);

  // Calling Hat Switch event handler
  if (hat != oldHat && joyEvents) {
    joyEvents->OnHatSwitch(hat);
    oldHat = hat;
  }


  // Call dao TT event handler
  if (buf[6] != oldTT && joyEvents) {
    joyEvents->OnDaoTTChange(buf[6]);
    oldTT = buf[6];
  }

  uint16_t buttons = (0x0000 | buf[6]);
  buttons <<= 4;
  buttons |= (buf[5] >> 4);
  uint16_t changes = (buttons ^ oldButtons);
  /*
    if (changes) {
    Serial.print("RAW DATA : ");
    for (uint8_t i = 0; i < RPT_GEMEPAD_LEN + 2; i++) {
      Serial.print("0x");
      Serial.print(buf[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
    }
  */
  // Calling Button Event Handler for every button changed
  if (changes) {


    for (uint8_t i = 0; i < 0x0C; i++) {
      uint16_t mask = (0x0001 << i);

      if (((mask & changes) > 0) && joyEvents) {
        if ((buttons & mask) > 0)
          joyEvents->OnButtonDn(i + 1);
        else
          joyEvents->OnButtonUp(i + 1);
      }
    }
    oldButtons = buttons;
  }
}

void JoystickEvents::OnGamePadChanged(const GamePadEventData *evt) {
  Serial.print("X1: ");
  PrintHex<uint8_t > (evt->X, 0x80);
  Serial.print("\tY1: ");
  PrintHex<uint8_t > (evt->Y, 0x80);
  Serial.print("\tX2: ");
  PrintHex<uint8_t > (evt->Z1, 0x80);
  Serial.print("\tY2: ");
  PrintHex<uint8_t > (evt->Z2, 0x80);
  Serial.print("\tRz: ");
  PrintHex<uint8_t > (evt->Rz, 0x80);
  Serial.println("");
  // Set reports for entry model controller compatible format
  BTReport.TT = evt->Z2;
  BTReport.Btn = evt->Y;
  BTReport.EBtn = evt->Z1;

  // Set Reports for dao controller format
  uint8_t DaoEBtn = (0x01 & evt->Z1) | ((0x08 & evt->Z1) >> 2);
  DAOReport.EBtn = DaoEBtn;
  uint8_t DaoBtn = 0x00;
  DaoBtn |= (0x80 & evt->Z2) >> 7; //B1
  DaoBtn |= (0x04 & evt->Z2) >> 1; //B2
  DaoBtn |= (0x40 & evt->Z2) >> 4; //B3
  DaoBtn |= (0x08 & evt->Z2); //B4
  DaoBtn |= (0x20 & evt->Z2) >> 1; //B5
  DaoBtn |= (0x01 & evt->Z2) << 5; //B6
  DaoBtn |= (0x80 & evt->Z1) >> 1; //B7
  DAOReport.Btn = DaoBtn;


}

void JoystickEvents::OnHatSwitch(uint8_t hat) {
  Serial.print("Hat Switch: ");
  PrintHex<uint8_t > (hat, 0x80);
  Serial.println("");
}

void JoystickEvents::OnButtonUp(uint8_t but_id) {
  Serial.print("Up: ");
  Serial.println(but_id, DEC);
  /*
    switch (but_id) {
    case 1:
      BTReport.Btn &= ~0x01;
      break;
    case 2:
      BTReport.Btn &= ~0x02;
      break;
    case 3:
      BTReport.Btn &= ~0x04;
      break;
    case 4:
      BTReport.Btn &= ~0x08;
      break;
    case 5:
      BTReport.Btn &= ~0x10;
      break;
    case 6:
      BTReport.Btn &= ~0x20;
      break;
    case 7:
      BTReport.Btn &= ~0x41;
      break;
    case 9:
      BTReport.EBtn &= ~0x01;
      break;
    case 10:
      BTReport.EBtn &= ~0x02;
      break;
    }
  */
}

void JoystickEvents::OnButtonDn(uint8_t but_id) {
  Serial.print("Dn: ");
  Serial.println(but_id, DEC);
  /*
    switch (but_id) {
    case 1:
      BTReport.Btn |= 0x01;
      break;
    case 2:
      BTReport.Btn |= 0x02;
      break;
    case 3:
      BTReport.Btn |= 0x04;
      break;
    case 4:
      BTReport.Btn |= 0x08;
      break;
    case 5:
      BTReport.Btn |= 0x10;
      break;
    case 6:
      BTReport.Btn |= 0x20;
      break;
    case 7:
      BTReport.Btn |= 0x41;
      break;
    case 9:
      BTReport.EBtn |= 0x01;
      break;
    case 10:
      BTReport.EBtn |= 0x02;
      break;
    }
  */
}

void JoystickEvents::OnDaoTTChange(uint8_t ttValue) {
  DAOReport.TT = map(ttValue, 0, 255, 0, 511);
}

IIDXBTReport JoystickEvents::GetIIDXReport() {
  return BTReport;
}

IIDXBTReport JoystickEvents::GetDAOReport() {
  return DAOReport;
}


