#ifndef __hidunipidvid_h__
#define __hidunipidvid_h__

#include <hiduniversal.h>

class HIDUniPIDVID : public HIDUniversal {
  public:
    HIDUniPIDVID(USB *p) : HIDUniversal(p) {};
    uint16_t getPID();
    uint16_t getVID();
};

#endif

