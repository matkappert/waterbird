

void menuInit() {
  uint8_t menu_items = menu_length;
  for (uint8_t x = 0; x < number_of_outputs; x++) {
    commands[menu_items] = {channels[x][0], channels[x][1], channelCallback, "\tToggle output channel."};
    menu_items++;
  }
  console.begin(commands, menu_items);

#ifdef ESP32
  if (!EEPROM.begin(512)) {
    console.v().pln("ERROR! failed to initialise EEPROM");
    while (true) {
      delay(500);
    }
  }
#elif ESP8266
  EEPROM.begin(512);
#endif

  bool wifi_ssid_overflow = readEEPROM(wifi_ssid.address, wifi_ssid.buffer, wifi_ssid.size);
  bool wifi_password_overflow = readEEPROM(wifi_password.address, wifi_password.buffer, wifi_password.size);
  bool mqtt_ip_overflow = readEEPROM(mqtt_ip_address.address, mqtt_ip_address.buffer, mqtt_ip_address.size);

  if (strlen(wifi_ssid.buffer) <= 0 || !wifi_ssid_overflow) {
    console.vv().pln("No WiFi SSID set");
    wifi_params = false;
  }
  if (strlen(wifi_password.buffer) <= 0 || !wifi_password_overflow) {
    console.vv().pln("No WiFi password set");
    wifi_params = false;
  }
  if (strlen(mqtt_ip_address.buffer) <= 0 || !mqtt_ip_overflow) {
    console.vv().pln("No MQTT IP address");
    mqtt_params = false;
  }
}

/***********************
    // Callbacks.
************************/

void ssidCallback(const char *arg, const uint8_t length, const char *cmd) {
  if (length && length <= wifi_ssid.size) {
    writeEEPROM(wifi_ssid.address, (char *)arg);
    console.vvv().pln("Reboot system to apply WiFi settings.");
  } else if (length == 0) {
    readEEPROM(wifi_ssid.address, wifi_ssid.buffer, wifi_ssid.size);
    console.vvv().p("SSID: ").pln(wifi_ssid.buffer);
  } else {
    console.v().p("\n#ERROR! ").p(__LINE__).p(": ").p(__PRETTY_FUNCTION__).p(", Length:").p(length).p(", arg:").pln(arg).pln(wifi_ssid.size);
  }
}

void passwordCallback(const char *arg, const uint8_t length, const char *cmd) {
  if (length && length <= wifi_password.size) {
    writeEEPROM(wifi_password.address, (char *)arg);
    console.vvv().pln("Reboot system to apply WiFi settings.");
  } else if (length == 0) {
    readEEPROM(wifi_password.address, wifi_password.buffer, wifi_password.size);
    console.vvv().p("Password: ").pln(wifi_password.buffer);
  } else {
    console.v().p("\n#ERROR! ").p(__LINE__).p(": ").p(__PRETTY_FUNCTION__).p(", Length:").p(length).p(", arg:").pln(arg);
  }
}

void resetWifiCallback(const char *arg, const uint8_t length, const char *cmd) {
  writeEEPROM(wifi_ssid.address, "");
  writeEEPROM(wifi_password.address, "");
}

void mqttSetCallback(const char *arg, const uint8_t length, const char *cmd) {
  if (length && length <= mqtt_ip_address.size) {
    writeEEPROM(mqtt_ip_address.address, (char *)arg);
    console.vvv().pln("MQTT IP address saved.");
    client.setServer((char *)arg, 1883);
    mqtt_params = true;
    client.disconnect();
  } else if (length == 0) {
    readEEPROM(mqtt_ip_address.address, mqtt_ip_address.buffer,
           mqtt_ip_address.size);
    console.vvv().p("MQTT IP: ").pln(mqtt_ip_address.buffer);
  } else {
    console.v().p("\n#ERROR! ").p(__LINE__).p(": ").p(__PRETTY_FUNCTION__).p(", Length:").p(length).p(", arg:").pln(arg);
  }
}

void resetAllCallback(const char *arg, const uint8_t length, const char *cmd) {
  resetWifiCallback(nullptr, NULL, nullptr);
  writeEEPROM(mqtt_ip_address.address, "");
#if defined(ESP8266) || defined(ESP32)
  ESP.restart();
#endif
}

void channelCallback(const char *arg, const uint8_t length, const char *cmd) {
  uint8_t x = atoi(cmd);
  if (length && length <= 5) {
    bool state = strIsTrue((char *)arg);
    changeOuputState(x - 1, state);
  } else if (length == 0) {
    if (x > 0 && x <= number_of_outputs) {
      changeOuputState(x - 1, !outputs[x - 1].state);
    }
  } else {
    console.v().p("\n#ERROR! ").p(__LINE__).p(": ").p(__PRETTY_FUNCTION__).p(", Length:").p(length).p(", arg:").pln(arg);
  }
}

/***********************
    // EEPROM.
************************/

bool readEEPROM(uint8_t start_address, char *buffer, uint8_t buffer_size) {
  uint8_t buffer_address = 0;
  console.vvvv().p("buffer_size: ").p(buffer_size).p(", start_address: ").pln(start_address);
  for (uint8_t i = start_address; i < start_address + 50; i++) {
    char in = EEPROM.read(i);
    if (buffer_address >= 50 || buffer_address >= buffer_size - 1) {
      console.vvvv().p(buffer_address).p(": ").pln("<OVER>").pln();
      console.v().p("\n#ERROR! buffer overflow.").p(__LINE__).p(": ").pln(__PRETTY_FUNCTION__);
      buffer[buffer_address] = '\0';
      return false;
    }
    if (in == '\0') {
      buffer[buffer_address] = '\0';
      console.vvvv().p(buffer_address).p(": ").pln("<END>").pln();
      return true;
    }
    buffer[buffer_address++] = in;
    console.vvvv().p(buffer_address - 1).p(": ").pln(in);
  }
}

void writeEEPROM(uint8_t start_address, char *buffer) {
  uint8_t buffer_length = strlen(buffer);
  uint8_t buffer_address = 0;
  for (int i = start_address; i <= start_address + buffer_length; i++) {
    if (i >= start_address + buffer_length) {
      EEPROM.write(i, '\0');
      console.vvvv().p(i).p(": ").pln("<END>").pln();
    } else {
      EEPROM.write(i, buffer[buffer_address++]);
      console.vvvv().p(i).p(": ").pln(buffer[buffer_address - 1]);
    }
  }
#if defined(ESP8266) || defined(ESP32)
  EEPROM.commit();
#endif
}