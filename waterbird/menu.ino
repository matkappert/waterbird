

void menuInit(){
	console.begin(commands, menu_length);

	#ifdef ESP8266
	  EEPROM.begin(512);
	#endif

	  readEEPROM(wifi_ssid.address, wifi_ssid.buffer, wifi_ssid.size);
	  readEEPROM(wifi_password.address, wifi_password.buffer, wifi_password.size);
	  readEEPROM(mqtt_ip_address.address, mqtt_ip_address.buffer, mqtt_ip_address.size);
	 
	  if (strlen(wifi_ssid.buffer) <= 0) {
	    console.pln("No WiFi SSID set");
	    wifi_params = false;
	  }
	  if (strlen(wifi_password.buffer) <= 0) {
	    console.pln("No WiFi password set");
	    wifi_params = false;
	  }
	  if (strlen(mqtt_ip_address.buffer) <= 0) {
	    console.pln("No MQTT IP address");
	    mqtt_params = false;
	  }
}


/***********************
  // Callbacks.
************************/

void ssidCallback(const char *arg, const uint8_t length) {
  if (length && length <= wifi_ssid.size) {
      writeEEPROM(wifi_ssid.address, (char *)arg);
      console.vv().pln("Reboot system to apply WiFi settings.");
    } else if(length==0) {
      readEEPROM(wifi_ssid.address, wifi_ssid.buffer, wifi_ssid.size);
      console.vv().p("SSID: ").pln(wifi_ssid.buffer);
    }else{
      console.v().pln("\n#ERROR!");
    }
}

void passwordCallback(const char *arg, const uint8_t length) {
  if (length && length <= wifi_password.size) {
     writeEEPROM(wifi_password.address, (char *)arg);
     console.vv().pln("Reboot system to apply WiFi settings.");
   } else if(length==0) {
     readEEPROM(wifi_password.address, wifi_password.buffer, wifi_password.size);
     console.vv().p("Password: ").pln(wifi_password.buffer);
   }else{
     console.v().pln("\n#ERROR!");
   }
}

void resetWifiCallback(const char *arg, const uint8_t length) {
  writeEEPROM(wifi_ssid.address, "");
  writeEEPROM(wifi_password.address, "");
}

void mqttSetCallback(const char *arg, const uint8_t length) {
	if (length && length <= mqtt_ip_address.size) {
	   writeEEPROM(mqtt_ip_address.address, (char *)arg);
	   console.vv().pln("MQTT IP address saved.");
	   client.setServer((char *)arg, 1883);
	   mqtt_params = true;
	   client.disconnect(); 
	 } else if(length==0) {
	   readEEPROM(mqtt_ip_address.address, mqtt_ip_address.buffer, mqtt_ip_address.size);
	   console.vv().p("MQTT IP: ").pln(mqtt_ip_address.buffer);
	 }else{
	   console.v().pln("\n#ERROR!");
	 }
}

void resetAllCallback(const char *arg, const uint8_t length) {
	resetWifiCallback("",NULL);
	writeEEPROM(mqtt_ip_address.address, "");
	#ifdef ESP8266
	    ESP.restart();
	#endif
}


/***********************
  // EEPROM.
************************/

void readEEPROM(uint8_t start_address, char *buffer, uint8_t buffer_size) {
  uint8_t buffer_address = 0;
  for (uint8_t i = start_address; i < start_address + 50; i++) {
    char in = EEPROM.read(i);
    if (buffer_address >= 50 || buffer_address >= buffer_size + start_address) {
      console.v().pln("\n#ERROR! buffer overflow.");
      buffer[buffer_address] = '\0';
      break;
    }
    if (in == '\0') {
      buffer[buffer_address] = '\0';
      break;
    }
    buffer[buffer_address++] = in;
  }
}

void writeEEPROM(uint8_t start_address, char *buffer) {
  uint8_t buffer_length = strlen(buffer);
  uint8_t buffer_address = 0;
  for (int i = start_address; i <= start_address + buffer_length; i++) {
    if (i >= start_address + buffer_length)
      EEPROM.write(i, '\0');
    else
      EEPROM.write(i, buffer[buffer_address++]);
  }
#ifdef ESP8266
  EEPROM.commit();
#endif
}