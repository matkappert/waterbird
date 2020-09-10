/**
 * @file    waterbird.h
 * @author  matkappert
 * @repo    github.com/matkappert/waterbird
 * @version V0.1.1
 * @date    04/09/20
 * @format  http://format.krzaq.cc (style: google)
*/

#define useProjectLibrarys

#ifdef useProjectLibrarys
#include "src/pubsubclient-2.8/src/PubSubClient.h"
#include "src/console/src/console.h"
#else
#include <PubSubClient.h>  // https://github.com/knolleary/pubsubclient
#include <console.h>       // https://github.com/matkappert/waterbird
#endif

_version version = {0, 1, 1};

#ifdef ESP8266
#include <ESP8266WiFi.h>
WiFiClient espClient;
PubSubClient client(espClient);
#endif

#include "menu.h"
#include <EEPROM.h>

/*********************************************************************
  //    User Configurations.
**********************************************************************/

//
//  this is an array of the output pin
//
#define  output_pins (const uint8_t[]) {D1, D2}

//
//  set the maximum active time in minutes in case of any errors
//
#define maximum_output_time 10  // 10 minutes

//
//  invert output pins?
//
#define output_pin_invert false  // true|false

/*********************************************************************
  //    End User Configurations.
**********************************************************************/

const uint8_t number_of_outputs = sizeof(output_pins) / sizeof(uint8_t);

typedef struct {
  uint8_t pin;
  uint32_t timer;
  bool state;
} output_struct;
output_struct outputs[number_of_outputs];

bool is_outputs_active;

#define active_pin_invert false
#define print_logo

#ifdef ESP8266
#include <Ticker.h>
Ticker flasher;
bool flashing = false;
#endif

void setup() {
/***********************
  // ESP Setup.
************************/
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, true);
  statusLedFastBlink();
#endif
#ifdef LED_BUILTIN_AUX
  pinMode(LED_BUILTIN_AUX, OUTPUT);
  digitalWrite(LED_BUILTIN_AUX, false);
#endif

  /***********************
    // Arduino Setup.
  ************************/
  Serial.begin(115200);
  console.version = version;
  console.setPrinter(Serial);
  console.setFilter(
      Level::vvvv);  // The filtering threshold is set to Level::vv
  delay(1000);
  console.pln("\n\r\r\n\r");
#ifdef print_logo
  console.vv()
      .pln("                _            _     _         _")
      .pln(" __      ____ _| |_ ___ _ __| |__ (_)_ __ __| |")
      .pln(" \\ \\ /\\ / / _` | __/ _ \\ '__| '_ \\| | '__/ _` |")
      .pln("  \\ V  V / (_| | ||  __/ |  | |_) | | | | (_| |")
      .p("   \\_/\\_/ \\__,_|\\__\\___|_|  |_.__/|_|_|  \\__,_| ")
      .pln();
  console.vv()
      .pln("      __")
      .pln("    .^o ~\\")
      .pln("   Y /'~) }      _____")
      .pln("   l/  / /    ,-~     ~~--.,_")
      .pln("      ( (    /  ~-._         ^.")
      .pln("       \\ \"--'--.    \"-._       \\")
      .pln("        \"-.________     ~--.,__ ^.")
      .pln("                  \\\"~r-.,___.-'-. ^.")
      .pln("                   YI    \\\\      ~-.\\")
      .pln("                   ||     \\\\        `\\")
      .pln("                   ||     //")
      .pln("                   ||    //")
      .pln("                   ()   //")
      .pln("                   ||  //")
      .pln("                   || ( c")
      .pln("      ___._ __  ___I|__`--__._ __  _")
      .pln();
#endif
  menuInit();
  delay(1000);

  /***********************
    // Channel Setup.
  ************************/
  console.vv().pln().p("Number of outputs: ").pln(number_of_outputs);
  for (uint8_t x = 0; x < number_of_outputs; x++) {
    outputs[x].pin = output_pins[x];
    outputs[x].timer = -1;
    outputs[x].state = false;
    pinMode(outputs[x].pin, OUTPUT);
    digitalWrite(outputs[x].pin, LOW ^ output_pin_invert);
    console.vvv().p("Ch-").p(x).p(" GPIO:").pln(outputs[x].pin);
  }

  /***********************
    // WiFi Setup.
  ************************/
  wifiConnection();

  /***********************
    // MQTT Setup.
  ************************/
  randomSeed(micros());
  if (!mqtt_params) {
    console.v().pln("\n#WARNING! No MQTT server IP settings.");
    client.setServer("127.0.0.0", 1883);
  } else {
    client.setServer((char *)mqtt_ip_address.buffer, 1883);
  }
  client.setCallback(mqttCallback);

} /* END SETUP */

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    wifiConnection();
  }
  if (!client.connected()) {
    mqttConnection();
  }
  client.loop();
  console.update();

  /***********************
    // Channel timeouts.
  ************************/
  for (uint8_t x = 0; x < number_of_outputs; x++) {
    if (outputs[x].timer < millis()) {
      outputs[x].state = false;
      digitalWrite(outputs[x].pin, false ^ output_pin_invert);
      console.v().p("\n#ERROR: ch:").p(x + 1).pln(" - timeout.");
      outputs[x].timer = -1;
    }
  }

/***********************
  // Active LED.
************************/
#ifdef LED_BUILTIN_AUX
  is_outputs_active = false;
  for (uint8_t x = 0; x < number_of_outputs; x++) {
    if (outputs[x].state) {
      is_outputs_active = true;
    }
  }
  pinMode(LED_BUILTIN_AUX, is_outputs_active ^ active_pin_invert);
#endif

} /* END LOOP */

void changeOuputState(uint8_t channel, bool state) {
  console.vv()
      .p("ch:")
      .p(channel + 1)
      .p(" - state changed to ")
      .pln(state ? "'ON'" : "'OFF'");
  outputs[channel].state = state;
  if (state) {
    digitalWrite(outputs[channel].pin, HIGH ^ output_pin_invert);
    outputs[channel].timer = millis() + (60000ul * maximum_output_time);
  } else {
    digitalWrite(outputs[channel].pin, LOW ^ output_pin_invert);
    outputs[channel].timer = -1;
  }
  // isError = false;
}

void mqttCallback(char *topic, byte *payload, uint8_t length) {
  if (String(topic).startsWith("waterbird/set/ch")) {
    char buffer[20];
    for (uint8_t y = 0; y < length; y++) {
      if (y >= 20) {
        console.v()
            .p("\n#ERROR! buffer overflow.")
            .p(__LINE__)
            .p(": ")
            .pln(__PRETTY_FUNCTION__);
        break;
      }
      buffer[y] = payload[y];
    }
    buffer[length] = '\0';
    bool state = strIsTrue((char *)buffer);
    bool isError = true;

    for (uint8_t x = 0; x < number_of_outputs; x++) {
      if (String(topic).startsWith("waterbird/set/ch" + String(x + 1))) {
        changeOuputState(x, state);
        isError = false;
      }
    }
    if (isError) {
      console.vv().p("\n#ERROR: unknown MQTT request - \"").p(topic).pln("\"");
    }
  }
}

/***********************
  // Connection.
************************/

void wifiConnection() {
  if (!wifi_params) {
    console.v().pln("\n#SYSTEM HALTED! No WiFi settings.");
    while (true) {
      console.update();
    }
  } else {
    console.vv().p("Connecting to '").p((char *)wifi_ssid.buffer).pln("'");
    WiFi.mode(WIFI_STA);
    WiFi.begin((char *)wifi_ssid.buffer, (char *)wifi_password.buffer);
    // Loop until we're connected
    while (WiFi.status() != WL_CONNECTED) {
      console.update();
      delay(250);
      console.vvvv().p(".");
    }
    console.vvvv().pln();
    console.vv().p("\nWiFi connected\tIP address: ").pln(WiFi.localIP());
#ifdef LED_BUILTIN
    statusLedStop();
    statusLedSlowBlink();
#endif
  }
}

void mqttConnection() {
  // Loop until we're reconnected
  if (mqtt_params) {
    while (!client.connected()) {
#ifdef LED_BUILTIN
      statusLedSlowBlink();
#endif
      console.update();
      console.vv().pln("Attempting MQTT connection...");

      // Create a random client ID
      String clientId = "WaterbirdClient-";
      clientId += String(random(0xffff), HEX);
      // Attempt to connect
      if (client.connect(clientId.c_str())) {
#ifdef LED_BUILTIN
        statusLedStop();
#endif
        console.vv().pln("MQTT connected.");

        client.publish("waterbird/status/msg", "Waterbird Startup");
        // ... and resubscribe
        client.subscribe("waterbird/set/#");

      } else {
        console.vv()
            .p("\n#ERROR! MQTT connection failed, rc=")
            .pln(client.state());
        delay(5000);
      }
    }
  } else {
#ifdef LED_BUILTIN
    statusLedSlowBlink();
#endif
  }
}

/***********************
  // Status LED.
************************/

#ifdef LED_BUILTIN
void statusLedSlowBlink() {
  if (!flashing) flasher.attach(0.5, statusLedFlip);
  flashing = true;
}

void statusLedFastBlink() {
  if (!flashing) flasher.attach(0.05, statusLedFlip);
  flashing = true;
}

void statusLedFlip() {
  bool state = digitalRead(LED_BUILTIN);
  digitalWrite(LED_BUILTIN, !state);
}

void statusLedStop() {
  flashing = false;
  flasher.detach();
  digitalWrite(LED_BUILTIN, true);
}
#endif