#ifndef _SerialUI_h
#define _SerialUI_h

#include <SerialUI.h>
extern SUI::SerialUI MySUI;

#define request_inputstring_maxlen	50
// #define SUI_SERIALUI_TOP_MENU_NAME ""
// #define serial_input_terminator		'\n'


void ssidChangedCallback();
void passwordChangedCallback();



typedef struct MyInputsContainerStruct {
  SerialUI::Menu::Item::Request::String wifi_ssid;
  SerialUI::Menu::Item::Request::String wifi_password;
  MyInputsContainerStruct() :
    wifi_ssid("null", SUI_STR("ssid"), SUI_STR("Change WiFi SSID"), request_inputstring_maxlen, ssidChangedCallback),
    wifi_password("null", SUI_STR("password"), SUI_STR("Change WiFi password"), request_inputstring_maxlen, passwordChangedCallback)
  {}
} MyInputsContainerSt;

extern MyInputsContainerSt MyInputs;



#endif
