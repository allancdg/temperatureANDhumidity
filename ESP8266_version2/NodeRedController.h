#ifndef NODE_RED_CONTROLLER_H
#define NODE_RED_CONTROLLER_H

#include "PubSubClient.h"
#include "ESP8266WiFi.h"
#include "WiFiClient.h"

class WiFiClient;
class PubSubClient;

class NodeRedController {
  private:
    WiFiClient ESPWiFiClient;
    PubSubClient mqttClient;

    //WiFi
    char* wifi_ssid = "linksys";
    char* wifi_password = "";
    int wifi_timeout = 10000;

    //MQTT
    char* mqtt_username = "angelo";
    char* mqtt_password = "angelo";
    char* mqtt_broker = "192.168.1.110";
    int mqtt_port = 1883;
    int mqtt_timeout = 10000;

    void connectWiFi();
    void configMQTT();
    void connectMQTT();

  public:
    NodeRedController() {
      PubSubClient mqttClient1(ESPWiFiClient);
      mqttClient = mqttClient1;
    }

    ~NodeRedController();

    void setWiFiSSID(char* ssid);
    void setWiFiPasswd(char* passwd);
    void setWiFiTimeout(int timeout);
    void setMQTTUsername(char* username);
    void setMQTTPasswd(char* passwd);
    void setMQTTBroker(char* broker);
    void setMQTTPort(int port);
    void setMQTTTimeout(int timeout);

    void initWiFiMQTT();

    void publish(char *topic, char *msg);
};

#endif  // NODE_RED_CONTROLLER_H
