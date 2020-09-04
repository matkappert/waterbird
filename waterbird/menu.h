void ssidCallback(const char *arg, const uint8_t length, const char *cmd);
void passwordCallback(const char *arg, const uint8_t length, const char *cmd);
void resetWifiCallback(const char *arg, const uint8_t length, const char *cmd);
void mqttSetCallback(const char *arg, const uint8_t length, const char *cmd);
void resetAllCallback(const char *arg, const uint8_t length, const char *cmd);
void channelCallback(const char *arg, const uint8_t length, const char *cmd);

#define menu_length 6
cmd_t commands[menu_length + 8] = {
    {"s", "ssid", ssidCallback, "\tSet WiFi SSID."},
    {"p", "password", passwordCallback, "\tSet WiFi password."},
    {"", "reset-wifi", resetWifiCallback, "Reset WiFi settings."},
    {"", "mqtt", mqttSetCallback, "\tSet MQTT IP address."},
    {"", "reset-all", resetAllCallback,
     "\tReset to default parameters & reboots."},
    {"", "", nullptr, ""}};

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

char *channels[][3] = {{"1", "ch-1"}, {"2", "ch-2"}, {"3", "ch-3"},
                       {"4", "ch-4"}, {"5", "ch-5"}, {"6", "ch-6"},
                       {"7", "ch-7"}, {"8", "ch-8"}, {"9", "ch-9"}};

bool strIsTrue(const char *buffer) {
  return strcmp(buffer, "true") == 0 || strcmp(buffer, "on") == 0 ||
         strcmp(buffer, "1") == 0;
}