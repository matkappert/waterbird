// #ifndef _SerialUI_h
// #define _SerialUI_h

#include <SerialUI.h>
extern SUI::SerialUI MySUI;

#define request_inputstring_maxlen	50


void ssidChangedCallback();
void passwordChangedCallback();
void toggleChangedCallback();



typedef struct MyInputsContainerStruct {
  SerialUI::Menu::Item::Request::String wifi_ssid;
  SerialUI::Menu::Item::Request::String wifi_password;
  SerialUI::Menu::Item::Request::BoundedLong toggle_channel;

  MyInputsContainerStruct() :
    wifi_ssid("null", SUI_STR("ssid"), SUI_STR("Change WiFi SSID"), request_inputstring_maxlen, ssidChangedCallback),
    wifi_password("null", SUI_STR("password"), SUI_STR("Change WiFi password"), request_inputstring_maxlen, passwordChangedCallback),
    toggle_channel(0, SUI_STR("toggle"), SUI_STR("Toggle output channel"), 1, number_of_outputs, toggleChangedCallback)
  {}
} MyInputsContainerSt;

extern MyInputsContainerSt MyInputs;



// #endif
