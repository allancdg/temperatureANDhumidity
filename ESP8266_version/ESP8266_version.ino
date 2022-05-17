#include "DHT.h"
#include "ESP8266WiFi.h"
#include "PubSubClient.h"
#include "WiFiClient.h"
#include "time.h"

#define DHTTYPE DHT11

// ----------------------------------------------------------------------- //

//Definição dos pinos
int DHTPin = 4;

//Configuração do sensor DHT11
DHT dht(DHTPin, DHTTYPE);

// ----------------------------------------------------------------------- //

//Declaração das variáveis
float Temperature;
float Humidity;
String measurementTime = "";

WiFiClient ESPWiFiClient;
PubSubClient mqttClient(ESPWiFiClient);

const char* wifi_ssid = ""; // LEMBRAR DE CONFIGURAR SSID
const char* wifi_password = ""; // LEMBRAR DE CONFIGURAR PASSWORD
int wifi_timeout = 10000;

const char* mqtt_broker = "broker.hivemq.com";
const int mqtt_port = 1883;
int mqtt_timeout = 10000;

// ----------------------------------------------------------------------- // 

//FUNÇÕES
//CONFIGURAÇÃO DA WIFI
void connectWifi(){
  WiFi.mode(WIFI_STA); // "Station mode": permite que o ESP32 seja um cliente
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("Conectando à rede WiFi ..");

  //Captura o tempo, em milisegundos, desde que o programa começou a rodar o 'conectWifi'
  unsigned long startTime = millis();

  //Enquanto o status da conexão for diferente de conectado e não tenha ultrapassado o timeout
  while(WiFi.status() != WL_CONNECTED && (millis() - startTime < wifi_timeout)){
    Serial.print(".");
    delay(1000);
  }
  Serial.println();

  //Informa se a conexão falhou ou, caso tenha conectado, informa o IP
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Connection has failed!");
  } else {
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
  }
}

// ----------------------------------------------------------------------- //

//CONFIGURAÇÃO DO MQTT
void configMQTT(){
  mqttClient.setServer(mqtt_broker, mqtt_port);
  Serial.println("Configuração do broker efetuada!");
}

void connectMQTT(){
  Serial.print("Conectando ao MQTT Broker...");

  unsigned long startTime = millis();
  while(!mqttClient.connected() && (millis() - startTime < mqtt_timeout)){
    Serial.print(".");
    String clientId = "ESP8266ClientAngelo-";
    clientId += String(random(0xffff), HEX); // Geração de complemento do nome randomico

    if(mqttClient.connect(clientId.c_str())){
      Serial.println();
      Serial.print("Conectado ao broker MQTT!");
    }
    delay(1000);
  }
  if(!mqttClient.connected()){
    Serial.print("Conexão com o broker MQTT não estabelecida!");
  }
  Serial.println();
}

// ----------------------------------------------------------------------- //

//TRANSFORMAÇÃO DO RESULTADO EM STRING
String retornaComoString(float Temperatura, float Umidade, String tempoDaMedicao){
  String lineData = "";
  lineData += "Temperatura: ";
  lineData += String(Temperatura, 1);
  lineData += "ºC \t Umidade: ";
  lineData += String(Umidade);
  lineData += "% \t Data: ";
  lineData += tempoDaMedicao;
  return lineData;
}

// ----------------------------------------------------------------------- //

//PEGAR FORMATAÇÃO DE DATA
String getTime(char* fmt){
  time_t now = time(nullptr);
  struct tm *timeinfo;
  timeinfo = localtime(&now);
  char buffer[80];
  strftime(buffer, 80, fmt, timeinfo);
  String time_str(buffer);
  return time_str;
}

// ----------------------------------------------------------------------- //

void setup() {
  Serial.begin(115200);

  //Configuração de INPUT/OUTPUT dos sensores/atuadores 
  //Sensor DHT11 -> Pino D4 -> Recebimento dos dados
  pinMode(DHTPin, INPUT);

  //Inicializa o sensor DHT11
  dht.begin();

  //Conecta a wifi e configura conexão com broker
  connectWifi();
  if(WiFi.status() == WL_CONNECTED){
    configMQTT();
  }

  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  while (getTime("%Y") != "2022"){
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
}

// ----------------------------------------------------------------------- //

void loop() {

  // Leitura dos dados
  Temperature = dht.readTemperature();
  Humidity = dht.readHumidity();
  measurementTime = getTime("%H:%M:%S %d-%m-%Y");
  
  String lineDataFull = retornaComoString(Temperature, Humidity, measurementTime);
  Serial.println(lineDataFull);

  if(!mqttClient.connected()){
    connectMQTT();
  }

  if(mqttClient.connected()){
    mqttClient.loop();
    char a[10];
    sprintf(a, "%.2f", Temperature);
    mqttClient.publish("/imd0902/projeto/grupo02/temperatura", a, true);
    sprintf(a, "%.2f", Humidity);
    mqttClient.publish("/imd0902/projeto/grupo02/umidade", a, true);
  }

  delay(60000); // Tempo de amostragem = 60 segundos
}
