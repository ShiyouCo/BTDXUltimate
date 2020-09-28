#if !defined(__HIDJOYSTICKRPTPARSER_H__)
#define __HIDJOYSTICKRPTPARSER_H__



#include <usbhid.h>

struct GamePadEventData {
        uint8_t X, Y, Z1, Z2, Rz;
};

struct IIDXBTReport {
        uint8_t TT, Btn, EBtn;
};

class JoystickEvents {
public:
        virtual void OnGamePadChanged(const GamePadEventData *evt);
        virtual void OnHatSwitch(uint8_t hat);
        virtual void OnButtonUp(uint8_t but_id);
        virtual void OnButtonDn(uint8_t but_id);
        virtual void OnDaoTTChange(uint8_t ttValue);
        //IIDXBTReport BTReport;
        IIDXBTReport GetIIDXReport();
        IIDXBTReport GetDAOReport();
private:
        IIDXBTReport BTReport;
        IIDXBTReport DAOReport;
};

#define RPT_GEMEPAD_LEN		5

class JoystickReportParser : public HIDReportParser {
        JoystickEvents *joyEvents;

        uint8_t oldPad[RPT_GEMEPAD_LEN];
        uint8_t oldHat;
        uint16_t oldButtons;
        uint8_t oldTT;

public:
        JoystickReportParser(JoystickEvents *evt);

        virtual void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
};

#endif // __HIDJOYSTICKRPTPARSER_H__
