#include "NodeRedController.h"

void NodeRedController::connectWiFi() {
  WiFi.mode(WIFI_STA); // "Station mode": permite que o ESP32 seja um cliente
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("Conectando à rede WiFi ..");

  //Captura o tempo, em milisegundos, desde que o programa começou a rodar o 'conectWifi'
  unsigned long startTime = millis();

  //Enquanto o status da conexão for diferente de conectado e não tenha ultrapassado o timeout
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime < wifi_timeout)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();

  //Informa se a conexão falhou ou, caso tenha conectado, informa o IP
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection has failed!");
  } else {
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
  }
}

void NodeRedController::configMQTT() {
  mqttClient.setServer(mqtt_broker, mqtt_port);
  Serial.println("Configuração do broker efetuada!");
}

void NodeRedController::connectMQTT() {
  Serial.print("Conectando ao MQTT Broker...");

  unsigned long startTime = millis();
  while (!mqttClient.connected() && (millis() - startTime < mqtt_timeout)) {
    Serial.print(".");
    String clientId = "ESP8266ClientAngelo-";
    clientId += String(random(0xffff), HEX); // Geração de complemento do nome randomico

    if (mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println();
      Serial.print("Conectado ao broker MQTT!");
    }
    delay(1000);
  }
  if (!mqttClient.connected()) {
    Serial.print("Conexão com o broker MQTT não estabelecida!");
  }
  Serial.println();
}

void NodeRedController::initWiFiMQTT() {
  connectWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    configMQTT();
  }
}

void NodeRedController::publish(char *topic, char *msg) {
  if (!mqttClient.connected()) {
    connectMQTT();
  }

  if (mqttClient.connected()) {
    mqttClient.loop();
    mqttClient.publish(topic, msg, true);
  }
}
