#include <DHT.h>
#include <time.h>
#include "NodeRedController.h"

#define DHTTYPE DHT11

// Definição dos pinos
int DHTPin = 4;

// Configuração do sensor DHT11
DHT dht(DHTPin, DHTTYPE);


// Declaração das variáveis
float Temperature;
float Humidity;
String measurementTime;

// Configuração do Controlador
NodeRedController node;

// TRANSFORMAÇÃO DO RESULTADO EM STRING
String retornaComoString(float Temperatura, float Umidade,
                         String tempoDaMedicao)
{
  String lineData = "";
  lineData += "Temperatura: ";
  lineData += String(Temperatura, 1);
  lineData += "ºC \t Umidade: ";
  lineData += String(Umidade);
  lineData += "% \t Data: ";
  lineData += tempoDaMedicao;
  return lineData;
}

// PEGAR FORMATAÇÃO DE DATA
String getTime(String fmt)
{
  time_t now = time(nullptr);
  struct tm *timeinfo;
  timeinfo = localtime(&now);
  char buffer[80];
  strftime(buffer, 80, fmt.c_str(), timeinfo);
  String time_str(buffer);
  return time_str;
}

// ----------------------------------------------------------------------- //

void setup()
{
  Serial.begin(115200);

  // Configuração de INPUT/OUTPUT dos sensores/atuadores
  // Sensor DHT11 -> Pino D4 -> Recebimento dos dados
  pinMode(DHTPin, INPUT);

  // Inicializa o sensor DHT11
  dht.begin();

  // Conecta a wifi e configura conexão com broker
  node.initWiFiMQTT();

  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("\nWaiting for time");
  while (!time(nullptr))
  {
    Serial.print(".");
    delay(1000);
  }
  while (getTime("%Y") != "2022")
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
}

// ----------------------------------------------------------------------- //

void loop()
{
  // Leitura dos dados
  Temperature = dht.readTemperature();
  Humidity = dht.readHumidity();
  measurementTime = getTime("%H:%M:%S %d-%m-%Y");

  String lineDataFull = retornaComoString(Temperature, Humidity, measurementTime);
  Serial.println(lineDataFull);

  char a[10];
  sprintf(a, "%.2f", Temperature);
  char *topic = (char *)"/dados/temperatura";
  node.publish(topic, a);

  delay(30000); // Tempo de amostragem = 60 segundos
}
