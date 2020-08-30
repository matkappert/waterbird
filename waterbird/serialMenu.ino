SUI::SerialUI MySUI(1);
MyInputsContainerSt MyInputs;


void SerialMenuLoop(){
	if (MySUI.checkForUser()) {
	  while (MySUI.userPresent()) {
	    MySUI.handleRequests();
	  }
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
  return true;
}


void ssidChangedCallback() {
  // MySUI.print(F("WiFi SSID changed to:"));
  // MySUI.println(MyInputs.wifi_ssid);
	aSerial.pln().p("WiFi SSID changed to: \"").p(MyInputs.wifi_ssid).pln("\"");
}

void passwordChangedCallback(){
	MySUI.print(F("WiFi password changed to:"));
	MySUI.println(MyInputs.wifi_password);
}

