


void ssidCallback(const char *arg, const uint8_t length);
void passwordCallback(const char *arg, const uint8_t length);
void resetWifiCallback(const char *arg, const uint8_t length);
void mqttSetCallback(const char *arg, const uint8_t length);
void resetAllCallback(const char *arg, const uint8_t length);

cmd_t commands[] = {{"s", "ssid", ssidCallback, "\tSet WiFi SSID."},
                    {"p", "password", passwordCallback, "\tSet WiFi password."},
                    {"", "reset-wifi", resetWifiCallback, "Reset WiFi settings."},
                    {"", "mqtt", mqttSetCallback, "\tSet MQTT IP address."},
                    {"", "reset-all", resetAllCallback, "\tReset to default parameters & reboots."}};
const uint8_t menu_length = 5;

struct settings_s {
  uint8_t address;
  char buffer[30];
  uint8_t size;
};


typedef struct settings_s settings;
settings wifi_ssid = {00, "", 30};
settings wifi_password = {30, "", 30};
settings mqtt_ip_address = {60, "", 15};

bool wifi_params = true;
bool mqtt_params = true;