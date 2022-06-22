#include "DHT.h"
#include "NodeRedController.h"

#define DHTTYPE DHT11
int DHTPin = 4;
DHT dht(DHTPin, DHTTYPE);

class NodeRedController;
//Declaração das variáveis
NodeRedController node;

float Temperature;
float Humidity;
String measurementTime = "";

//TRANSFORMAÇÃO DO RESULTADO EM STRING
String retornaComoString(float Temperatura, float Umidade, String tempoDaMedicao) {
  String lineData = "";
  lineData += "Temperatura: ";
  lineData += String(Temperatura, 1);
  lineData += "ºC \t Umidade: ";
  lineData += String(Umidade);
  lineData += "% \t Data: ";
  lineData += tempoDaMedicao;
  return lineData;
}

// SETUP
void setup() {
  Serial.begin(115200);

  //Configuração de INPUT/OUTPUT dos sensores/atuadores
  //Sensor DHT11 -> Pino D4 -> Recebimento dos dados
  pinMode(DHTPin, INPUT);

  //Inicializa o sensor DHT11
  dht.begin();

  //Conecta a wifi e configura conexão com broker
  node.initWiFiMQTT();

  Serial.println("");
}

// LOOP
void loop() {

  // Leitura dos dados
  Temperature = dht.readTemperature();
  Humidity = dht.readHumidity();

  String lineDataFull = retornaComoString(Temperature, Humidity, measurementTime);
  Serial.println(lineDataFull);


  char a[10];
  sprintf(a, "%.2f", Temperature);
  node.publish("/dados/temperatura", a);

  delay(10000); // Tempo de amostragem = 10 segundos
}
