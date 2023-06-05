#include <WiFi.h>
#include <PubSubClient.h>
#include <EmonLib.h>

const char* SSID = "*********";
const char* PASSWORD = "*******";
WiFiClient espClient;

const char* BROKER_MQTT = "*****";
int BROKER_PORT = 1883;                               

#define DHTTYPE DHT11
#define ID_MQTT "****"
#define TOPIC_PUBLISH "prometeusi***" 
#define TOPIC1_PUBLISH "ArcoAbertoprometeusid0002"
#define TOPIC2_PUBLISH "ArcoFechadoprometeusid0002"

PubSubClient MQTT(espClient);

unsigned long int tempoAnterior = 0;
int pinSCT = 34;
int pinSctValue = 0;
int tensao = 380;
int i = 0;
int potencia;
int corrente;

int opStart = 1;
int opStop = 1;

EnergyMonitor SCT013;

void setup() {
  SCT013.current(pinSCT, 6.0607);
  Serial.begin(9600);
}

void loop() {
  mantemConexoes();
  enviaValores();
}

void mantemConexoes() {
  conectaWiFi();
  if (!MQTT.connected()) {
    conectaMQTT();
  }
}

void conectaWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.print("Conectando-se na rede: ");
  Serial.print(SSID);
  Serial.println("  Aguarde!");

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso, na rede: ");
  Serial.print(SSID);
  Serial.print("  IP obtido: ");
  Serial.println(WiFi.localIP());
}

void conectaMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);

  while (!MQTT.connected()) {
    Serial.print("Conectando ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado ao Broker com sucesso!");
    } else {
      Serial.println("Noo foi possivel se conectar ao broker.");
      Serial.println("Nova tentatica de conexao em 5s");
      delay(5000);
    }
  }
}

void enviaValores() {
  double Irms = SCT013.calcIrms(1480);
  potencia = Irms * tensao;
  corrente = Irms * 6.70;
  corrente = (int)corrente;
  char b[10] = "";
  sprintf(b, "%d", corrente);

  if (millis() - tempoAnterior >= 1000) {
    tempoAnterior = millis();

    while (corrente >= 0 && corrente < 400) {
      MQTT.publish(TOPIC_PUBLISH, b);
      Serial.println(corrente);

      if (corrente > 20) {
        char h[10] = "";
        sprintf(h, "%d", opStart);
        MQTT.publish(TOPIC1_PUBLISH, h);
        opStart = opStart + 1;

      } else {
        char y[10] = "";
        sprintf(y, "%d", opStop);
        MQTT.publish(TOPIC2_PUBLISH, y);
        opStop = opStop + 1;
      }
      break;
    }
  }
}