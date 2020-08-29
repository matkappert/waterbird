#define useProjectLibrarys 
#ifdef useProjectLibrarys
#include "src/pubsubclient-2.8/src/PubSubClient.h"
#include "src/advancedSerial/src/advancedSerial.h"
#else
#include <PubSubClient.h>
#include <advancedSerial.h>
#endif

#include <ESP8266WiFi.h>

WiFiClient espClient;
PubSubClient client(espClient);




/*********************************************************************
// 		User Configurations.
**********************************************************************/

//
//	wifi settings
//
#define wifi_ssid      "LittleHouse"
#define Wifi_password  "appletree"

//
//	mqtt server ip address
//
#define mqtt_server    "10.0.0.2"

//
//	set the number of outputs to control
//
#define number_of_outputs 2

//
//	this is an array of the output pin, this must be equal to #number_of_outputs
//
uint8_t output_pins[number_of_outputs] = { 
	BUILTIN_LED, 
	D1 
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





typedef struct {
  uint8_t pin;
  uint32_t timer;
} output_struct;
output_struct outputs[number_of_outputs];


#ifdef printHelloWorld
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
uint16_t value = 0;
#endif




void setup() {
  /***********************
    // Arduino Setup.
  ************************/
  Serial.begin(115200);
  aSerial.setPrinter(Serial);
  aSerial.setFilter(Level::vvvv); // The filtering threshold is set to Level::vv
  delay(3000);
  aSerial.pln("\nWaterbird Startup!");
  pinMode(BUILTIN_LED, OUTPUT);    


  /***********************
    // Channel Setup.
  ************************/
  aSerial.vv().p("Number of outputs: ").p(number_of_outputs).pln(".");
  for(uint8_t x=0; x<number_of_outputs; x++){
  	outputs[x].pin = output_pins[x];
  	outputs[x].timer = -1;
  	pinMode(outputs[x].pin, OUTPUT);
  	digitalWrite(outputs[x].pin, LOW^output_pin_invert); 
  	aSerial.vvvv().p("Ch:").p(x).p(" GPIO - ").p(outputs[x].pin).pln(".");
  }
  


  /***********************
    // WiFi Setup.
  ************************/
  aSerial.vv().p("Connecting to '").p(wifi_ssid).pln("'");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, Wifi_password);
  // Loop until we're connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    aSerial.vvvv().p(".");
  }
  aSerial.vvvv().pln();
  aSerial.vv().pln("\nWiFi connected").p("IP address: ").pln(WiFi.localIP());


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


  /***********************
    // Channel timeouts.
  ************************/
  for(uint8_t x=0; x<number_of_outputs; x++){
  	if( outputs[x].timer < millis()  ){
  		digitalWrite(outputs[x].pin, LOW^output_pin_invert); 
  		aSerial.v().p("ERROR: ch:").p(x+1).pln(" - timeout.");
  		outputs[x].timer = -1;
  	}
  }


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
  if( String(topic).startsWith("waterbird/set/ch")){
  	 bool state = (char)payload[0]=='o'&&(char)payload[1]=='n'?true:false;
  	 bool isError = true;

  	 for(uint8_t x=0; x<number_of_outputs; x++){
  	 	if( String(topic).startsWith("waterbird/set/ch" + String(x+1))  ){
  	 		aSerial.v().p("ch:").p(x+1).p(" - state changed to ").pln(state?"'ON'":"'OFF'");
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
  	 if(isError){
		aSerial.v().p("ERROR: unknown MQTT request - \"").p(topic).pln("\"");
	}

}

}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    aSerial.vv().pln("\nAttempting MQTT connection...");

    // Create a random client ID
    String clientId = "WaterbirdClient-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
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
