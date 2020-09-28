#include "HIDUniPIDVID.h"

uint16_t HIDUniPIDVID::getPID(){
  uint16_t retPID = HIDUniversal::PID;
  return retPID;
}

uint16_t HIDUniPIDVID::getVID(){
  uint16_t retVID = HIDUniversal::VID;
  return HIDUniversal::VID;
}

