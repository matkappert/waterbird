#define sketch_version "v1.0"
#define useProjectLibrarys
#ifdef useProjectLibrarys
#include "src/pubsubclient-2.8/src/PubSubClient.h"
#include "src/advancedSerial/src/advancedSerial.h"
// #include "src/SerialUI-3.2.2/src/SerialUI.h"
#else
#include <PubSubClient.h>
#include <advancedSerial.h>
#endif

#include <ESP8266WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);

#include <SerialUI.h>
#include "serialMenu.h"

/*********************************************************************
  // 		User Configurations.
**********************************************************************/

//
//	wifi settings
//
#define coded_wifi_ssid      ""
#define coded_wifi_password  ""

//
//	mqtt server ip address
//
#define mqtt_server    "10.0.0.2"

//
//	this is an array of the output pin
//
const uint8_t output_pins[] = {
  D1,
  D2
};

//
//	set the maximum active time in minutes in case of any errors
//
#define maximum_output_time 60000ul * 10 // 10 minutes

//
//	invert output pins?
//
#define output_pin_invert false // true|false

//
//	uncomment the line below to send debug messages to node-red ever 2seconds
//
// #define printHelloWorld

/*********************************************************************
  // 		End User Configurations.
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
#endif

#ifdef printHelloWorld
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
uint16_t value = 0;
#endif




void setup() {
  /***********************
    // ESP Setup.
  ************************/
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, true );
  statusLedFastBlink();
#endif
#ifdef LED_BUILTIN_AUX
  pinMode(LED_BUILTIN_AUX, OUTPUT);
  digitalWrite(LED_BUILTIN_AUX, false );
#endif


  /***********************
    // Arduino Setup.
  ************************/
  Serial.begin(115200);
  aSerial.setPrinter(Serial);
  aSerial.setFilter(Level::vvvv); // The filtering threshold is set to Level::vv
  delay(1000);
  aSerial.pln("\n\r\r\n\r");
#ifdef print_logo
  aSerial.pln("                _            _     _         _").pln(" __      ____ _| |_ ___ _ __| |__ (_)_ __ __| |").pln(" \\ \\ /\\ / / _` | __/ _ \\ '__| '_ \\| | '__/ _` |").pln("  \\ V  V / (_| | ||  __/ |  | |_) | | | | (_| |").p("   \\_/\\_/ \\__,_|\\__\\___|_|  |_.__/|_|_|  \\__,_| ").pln(sketch_version);
  aSerial.pln("    __").pln("  .^o ~\\").pln(" Y /'~) }      _____").pln(" l/  / /    ,-~     ~~--.,_").pln("    ( (    /  ~-._         ^.").pln("     \\ \"--'--.    \"-._       \\").pln("      \"-.________     ~--.,__ ^.").pln("                \\\"~r-.,___.-'-. ^.").pln("                 YI    \\\\      ~-.\\").pln("                 ||     \\\\        `\\").pln("                 ||     //").pln("                 ||    //").pln("                 ()   //").pln("                 ||  //").pln("                 || ( c").pln("    ___._ __  ___I|__`--__._ __  _");
#endif
  
  delay(500);
  if (!SetupSerialUI()) {
    aSerial.pln("Problem during setup");
  }else{
  	aSerial.pln("\n\n---------------------------------").pln("Enter '?' for available menu options").pln("---------------------------------");
  }
  delay(2000);


  /***********************
    // Channel Setup.
  ************************/
  aSerial.vv().p("Number of outputs: ").p(number_of_outputs).pln(".");
  for (uint8_t x = 0; x < number_of_outputs; x++) {
    outputs[x].pin = output_pins[x];
    outputs[x].timer = -1;
    outputs[x].state = false;
    pinMode(outputs[x].pin, OUTPUT);
    digitalWrite(outputs[x].pin, LOW ^ output_pin_invert);
    aSerial.vvvv().p("Ch:").p(x).p(" GPIO - ").p(outputs[x].pin).pln(".");
  }





  /***********************
    // WiFi Setup.
  ************************/
  aSerial.vv().p("Connecting to '").p(coded_wifi_ssid).pln("'");
  WiFi.mode(WIFI_STA);
  WiFi.begin(coded_wifi_ssid, coded_wifi_password);
  // Loop until we're connected
  while (WiFi.status() != WL_CONNECTED) {
  	SerialMenuLoop();
    delay(500);
    aSerial.vvvv().p(".");
  }
  aSerial.vvvv().pln();
  aSerial.vv().pln("\nWiFi connected").p("IP address: ").pln(WiFi.localIP());
#ifdef LED_BUILTIN
  statusLedSlowBlink();
#endif


  /***********************
    // MQTT Setup.
  ************************/
  randomSeed(micros());
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);


} /* END SETUP */



void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  SerialMenuLoop();


  /***********************
    // Channel timeouts.
  ************************/
  for (uint8_t x = 0; x < number_of_outputs; x++) {
    if ( outputs[x].timer < millis()  ) {
      digitalWrite(outputs[x].pin, LOW ^ output_pin_invert);
      aSerial.v().p("ERROR: ch:").p(x + 1).pln(" - timeout.");
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


  /***********************
    // Send debug messages.
  ************************/
#ifdef printHelloWorld
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    aSerial.vv().p("Message arrived [").p(topic).p("] ");
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("waterbird/status/msg", msg);
  }
#endif

} /* END LOOP */




void mqttCallback(char* topic, byte* payload, uint8_t length) {
  if ( String(topic).startsWith("waterbird/set/ch")) {
    bool state = (char)payload[0] == 'o' && (char)payload[1] == 'n' ? true : false;
    bool isError = true;

    for (uint8_t x = 0; x < number_of_outputs; x++) {
      if ( String(topic).startsWith("waterbird/set/ch" + String(x + 1))  ) {
        aSerial.v().p("ch:").p(x + 1).p(" - state changed to ").pln(state ? "'ON'" : "'OFF'");
        outputs[x].state = state;
        if (state) {
          digitalWrite(outputs[x].pin, HIGH ^ output_pin_invert);
          outputs[x].timer = millis() + maximum_output_time;
        } else {
          digitalWrite(outputs[x].pin, LOW ^ output_pin_invert);
          outputs[x].timer = -1;
        }
        isError = false;
      }

    }
    if (isError) {
      aSerial.v().p("ERROR: unknown MQTT request - \"").p(topic).pln("\"");
    }

  }

}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
#ifdef LED_BUILTIN
    statusLedSlowBlink();
#endif
    SerialMenuLoop();
    aSerial.vv().pln("\nAttempting MQTT connection...");

    // Create a random client ID
    String clientId = "WaterbirdClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
#ifdef LED_BUILTIN
      statusLedStop();
#endif
      aSerial.vv().pln("MQTT connected\n");

      client.publish("waterbird/status/msg", "Waterbird Startup");
      // ... and resubscribe
      client.subscribe("waterbird/set/#");

    } else {
      aSerial.vv().p("MQTT failed, rc=").pln(client.state());
      delay(5000);
    }
  }
}

#ifdef LED_BUILTIN
void statusLedSlowBlink() {
  flasher.attach(0.5, statusLedFlip);
}

void statusLedFastBlink() {
  flasher.attach(0.05, statusLedFlip);
}

void statusLedFlip() {
  bool state = digitalRead(LED_BUILTIN);
  digitalWrite(LED_BUILTIN, !state);
}

void statusLedStop() {
  flasher.detach();
  digitalWrite(LED_BUILTIN, true);
}
#endif
