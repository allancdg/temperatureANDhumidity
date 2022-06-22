#ifndef NODE_RED_CONTROLLER_H
#define NODE_RED_CONTROLLER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

class WiFiClient;
class PubSubClient;

class NodeRedController
{
private:
  WiFiClient ESPWiFiClient;
  PubSubClient mqttClient;

  // WiFi
  const char *wifi_ssid = "Zil";
  const char *wifi_password = "angelo2430";
  int wifi_timeout = 10000;

  // MQTT
  const char *mqtt_username = "angelo";
  const char *mqtt_password = "angelo";
  const char *mqtt_broker = "192.168.1.9";
  int mqtt_port = 1883;
  int mqtt_timeout = 10000;
  String clientId = "ESP8266ClientAngelo-";

  void connectWiFi();
  void configMQTT();
  void connectMQTT();

  public:
  NodeRedController()
  {
    mqttClient.setClient(ESPWiFiClient);
    clientId += String(random(0xffff), HEX); // Geração de complemento do nome randomico
  }

  NodeRedController(
      const char *wifissid, const char *wifipasswd,
      int wifitimeout, const char *mqttusername,
      const char *mqttpasswd, const char *mqttbroker,
      int mqttport, int mqtttimeout);

  ~NodeRedController() = default;

  void initWiFiMQTT();

  void publish(char *topic, char *msg);
};

#endif // NODE_RED_CONTROLLER_H
