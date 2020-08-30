SUI::SerialUI MySUI(1);
MyInputsContainerSt MyInputs;


void SerialMenuLoop() {
  if (MySUI.checkForUser()) {
    // while (MySUI.userPresent()) {
    MySUI.handleRequests();
    // }
  }
}

bool SetupSerialUI() {
  MySUI.begin(115200);
  MySUI.setMaxIdleMs(10000ul);
  SUI_FLASHSTRING CouldntAddItemErr = F("Could not add item?");

  SerialUI::Menu::Menu * topMenu = MySUI.topLevelMenu();
  if (! topMenu ) {
    MySUI.returnError(F("Very badness in sEriALui!1"));
    return false;
  }

  if ( ! topMenu->addRequest( MyInputs.wifi_ssid)) {
    MySUI.returnError(CouldntAddItemErr);
    return false;
  }

  if ( ! topMenu->addRequest(MyInputs.wifi_password)) {
    MySUI.returnError(CouldntAddItemErr);
    return false;
  }

  if ( ! topMenu->addRequest( MyInputs.toggle_channel)) {
    MySUI.returnError(CouldntAddItemErr);
    return false;
  }


  return true;
}


void ssidChangedCallback() {
  aSerial.p("WiFi SSID changed to: \"").p(MyInputs.wifi_ssid).pln("\"");
}

void passwordChangedCallback() {
  aSerial.p("WiFi password changed to: \"").p(MyInputs.wifi_ssid).pln("\"");
}


void toggleChangedCallback() {
  uint8_t x = MyInputs.toggle_channel - 1;
  if (x <= number_of_outputs - 1 && x >= 0) {
    outputs[x].state = !outputs[x].state;
    if (outputs[x].state) {
      digitalWrite(outputs[x].pin, HIGH ^ output_pin_invert);
      outputs[x].timer = millis() + maximum_output_time;
    } else {
      digitalWrite(outputs[x].pin, LOW ^ output_pin_invert);
      outputs[x].timer = -1;
    }
    aSerial.p("Toggled ch:").p(x + 1).p(" - state changed to ").pln(outputs[x].state ? "'ON'" : "'OFF'");
  }

}
