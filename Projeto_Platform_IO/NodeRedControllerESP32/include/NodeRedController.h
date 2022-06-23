#ifndef NODE_RED_CONTROLLER_H
#define NODE_RED_CONTROLLER_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

class WiFiClient;
class PubSubClient;

enum NodeRedState
{
	NODE_RED_UNDEFINED = -9999,
	NODE_RED_DISCONNECTED = -1,
	NODE_RED_CONNECTED = 0,
	NODE_RED_CONNECTING = 1,
	NODE_RED_DISCONNECTING = 2,
	NODE_RED_KEEP_ALIVE = 3
};

class NodeRedController
{
private:
	WiFiClient ESPWiFiClient;
	PubSubClient mqttClient;
	NodeRedState state = NODE_RED_DISCONNECTED;

	// WiFik
	const char *wifi_ssid = "linksys";
	const char *wifi_password = "";
	int wifi_timeout = 10000;

	// MQTT
	const char *mqtt_username = "angelo";
	const char *mqtt_password = "angelo";
	const char *mqtt_broker = "192.168.1.114";
	int mqtt_port = 1883;
	int mqtt_timeout = 10000;
	String clientId = "ESP32ClientAllan-";

	String last_topic;
	String last_msg;
	unsigned int last_length;

	void mqtt_callback(char* topic, byte* payload, unsigned int length);
	void connectWiFi();
	void configMQTT();
	void connectMQTT();
	void initWiFiMQTT();
	void publish(char *topic, char *msg);
	void subscribe(char *topic);
	void unsubscribe(char *topic);
	int forceSubNewMsg(String prev_msg, int timeout);
	NodeRedState connectToNode(int timeout, int max_attempts);
	void startKeepAlive();
	void keepAlive(String who);

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

	void init(int timeout);

	void loop() { mqttClient.loop(); }

	inline int getState(){
		return (int)state;
	}

	void checkLife(int max_attempts, int timeout);
};

#endif // NODE_RED_CONTROLLER_H
