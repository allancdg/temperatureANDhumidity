#include "NodeRedController.h"

NodeRedController::NodeRedController(
	const char *wifissid,
	const char *wifipasswd,
	int wifitimeout,
	const char *mqttusername,
	const char *mqttpasswd,
	const char *mqttbroker,
	int mqttport,
	int mqtttimeout)
{
	wifi_ssid = wifissid;
	wifi_password = wifipasswd;
	wifi_timeout = wifitimeout;

	mqtt_username = mqttusername;
	mqtt_password = mqttpasswd;
	mqtt_broker = mqttbroker;
	mqtt_port = mqttport;
	mqtt_timeout = mqtttimeout;

	mqttClient.setClient(ESPWiFiClient);
	clientId += String(random(0xffff), HEX); // Geração de complemento do nome randomico
}

void NodeRedController::connectWiFi()
{
	WiFi.mode(WIFI_STA); // "Station mode": permite que o ESP32 seja um cliente
	WiFi.begin(wifi_ssid, wifi_password);
	Serial.print("Conectando à rede WiFi ..");

	// Captura o tempo, em milisegundos, desde que o programa começou a rodar o 'conectWifi'
	unsigned long startTime = millis();

	// Enquanto o status da conexão for diferente de conectado e não tenha ultrapassado o timeout
	while (WiFi.status() != WL_CONNECTED && ((int)(millis() - startTime) < wifi_timeout))
	{
		Serial.print(".");
		delay(1000);
	}
	Serial.println();

	// Informa se a conexão falhou ou, caso tenha conectado, informa o IP
	if (WiFi.status() != WL_CONNECTED)
	{
		Serial.println("Connection has failed!");
	}
	else
	{
		Serial.print("Connected with IP: ");
		Serial.println(WiFi.localIP());
	}
}

void NodeRedController::mqtt_callback(char *topic, byte *payload, unsigned int length)
{
	String msg_broker;
	char c;
	for (unsigned int i = 0; i < length; i++)
	{
		c = (char)payload[i];
		msg_broker += c;
	}

	last_topic = topic;
	last_msg = msg_broker;
	last_length = length;

	if (last_topic == "/connect/keepAlive")
	{
		keepAlive(last_msg);
	}
}

void NodeRedController::configMQTT()
{
	mqttClient.setServer(mqtt_broker, mqtt_port);
	mqttClient.setCallback([this](char *topic, byte *payload, unsigned int length)
						   { this->mqtt_callback(topic, payload, length); });
	Serial.println("Configuração do broker efetuada!");
}

void NodeRedController::connectMQTT()
{
	String p = mqtt_broker;
	Serial.print("Conectando ao MQTT Broker "+ p +" ...");

	unsigned long startTime = millis();
	while (!mqttClient.connected() && ((int)(millis() - startTime) < mqtt_timeout))
	{
		Serial.print(".");

		if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password))
		{
			Serial.println();
			Serial.print("Conectado ao broker MQTT!");
		}
		delay(1000);
	}
	if (!mqttClient.connected())
	{
		Serial.print("Conexão com o broker MQTT não estabelecida!");
	}
	Serial.println();
}

void NodeRedController::initWiFiMQTT()
{
	connectWiFi();
	if (WiFi.status() == WL_CONNECTED)
	{
		configMQTT();
		connectMQTT();
	}
}

void NodeRedController::publish(char *topic, char *msg)
{
	if (!mqttClient.connected())
	{
		connectMQTT();
	}

	if (mqttClient.connected())
	{
		mqttClient.loop();
		mqttClient.publish(topic, msg, true);
	}
}

void NodeRedController::subscribe(char *topic)
{
	if (!mqttClient.connected())
	{
		connectMQTT();
	}

	if (mqttClient.connected())
	{
		mqttClient.loop();
		mqttClient.subscribe(topic);
	}
}

void NodeRedController::unsubscribe(char *topic)
{
	if (!mqttClient.connected())
	{
		connectMQTT();
	}

	if (mqttClient.connected())
	{
		mqttClient.loop();
		mqttClient.unsubscribe(topic);
	}
}

int NodeRedController::forceSubNewMsg(String prev_msg, int timeout)
{
	unsigned long startTime = millis();
	while ((last_msg == prev_msg) && ((int)(millis() - startTime) < timeout))
		loop();

	if (last_msg == prev_msg)
		return 1;
	return 0;
}

NodeRedState NodeRedController::connectToNode(int timeout, int max_attempts)
{
	String pub_topic1 = "/connect/userID";
	String sub_topic1 = "/connect/replyTo";

	Serial.println("Connecting With Node Red Manager...");

	state = NODE_RED_CONNECTING;

	String prev_msg = last_msg;

	subscribe((char *)sub_topic1.c_str());
	publish((char *)pub_topic1.c_str(), (char *)clientId.c_str());

	// Wait for clientId
	int attempts = 0;
	while (last_msg != clientId && attempts < max_attempts)
	{
		if (forceSubNewMsg(prev_msg, timeout))
		{
			Serial.println("Took too long to recieve msg.");

			unsubscribe((char *)sub_topic1.c_str());
			return NODE_RED_DISCONNECTED;
		}
		prev_msg = last_msg;
		attempts += 1;
	}

	if (last_msg != clientId)
	{
		Serial.println("Node-RED is not responding to me. " + clientId + " != " + last_msg);
		unsubscribe((char *)sub_topic1.c_str());
		return NODE_RED_DISCONNECTED;
	}

	// Double check if connected
	String pub_topic2 = "/connect/isUserConnected";
	String sub_topic2 = "/connect/reply";

	unsubscribe((char *)sub_topic1.c_str());
	subscribe((char *)sub_topic2.c_str());
	publish((char *)pub_topic2.c_str(), (char *)clientId.c_str());

	// Wait for apropriate response
	attempts = 0;
	while (attempts < max_attempts)
	{
		if (forceSubNewMsg(prev_msg, timeout))
		{
			Serial.println("Took too long to recieve msg.");
			unsubscribe((char *)sub_topic2.c_str());
			return NODE_RED_DISCONNECTED;
		}

		if (last_msg == clientId + " is connected")
		{
			Serial.println("User " + clientId + " connected!");
			unsubscribe((char *)sub_topic2.c_str());
			return NODE_RED_CONNECTED;
		}
		else if (last_msg == clientId + " is not connected")
		{
			Serial.println("User " + clientId + " not connected.");
			unsubscribe((char *)sub_topic2.c_str());
			return NODE_RED_DISCONNECTED;
		}

		prev_msg = last_msg;
		attempts += 1;
	}

	Serial.println("Error Recieving Reply. " + last_msg);
	unsubscribe((char *)sub_topic2.c_str());
	return NODE_RED_DISCONNECTED;
}

void NodeRedController::startKeepAlive()
{
	String topic = "/connect/keepAlive";
	unsubscribe((char *)topic.c_str());
	subscribe((char *)topic.c_str());
}

void NodeRedController::keepAlive(String who)
{
	String pub_topic = "/connect/imAlive";
	
	if (who == clientId + " is set to die")
	{
		Serial.println("Sending Life Signal");
		publish((char *)pub_topic.c_str(), (char *)clientId.c_str());
	}
}

void NodeRedController::checkLife(int max_attempts, int timeout)
{
	// Double check if connected
	String pub_topic2 = "/connect/isUserConnected";
	String sub_topic2 = "/connect/reply";
	String prev_msg = last_msg;

	subscribe((char *)sub_topic2.c_str());
	publish((char *)pub_topic2.c_str(), (char *)clientId.c_str());

	// Wait for apropriate response
	int attempts = 0;
	while (attempts < max_attempts)
	{
		if (forceSubNewMsg(prev_msg, timeout))
		{
			Serial.println("Took too long to recieve msg.");
		}

		if (last_msg == clientId + " is connected")
		{
			Serial.println("User " + clientId + " is Alive!");
			state = NODE_RED_CONNECTED;
			unsubscribe((char *)sub_topic2.c_str());
			return;
		}
		else if (last_msg == clientId + " is not connected")
		{
			Serial.println("User " + clientId + " is Aliven't!");
			state = NODE_RED_DISCONNECTED;
			unsubscribe((char *)sub_topic2.c_str());
			return;
		}

		prev_msg = last_msg;
		attempts += 1;
	}

	Serial.println("Took too many attempts to recieve msg.");
	state = NODE_RED_UNDEFINED;
	unsubscribe((char *)sub_topic2.c_str());
}

void NodeRedController::init(int timeout)
{
	initWiFiMQTT();
	state = connectToNode(timeout, 10);
	if (state == NODE_RED_CONNECTED)
	{
		startKeepAlive();
	}
}