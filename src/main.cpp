#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 4         // Pino onde o DHT está conectado
#define DHTTYPE DHT22    // Tipo do sensor

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "WIFI_MESH_IST";
const char* password = "ac1ce1ss6_mesh";
const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);

#define RXD2 16
#define TXD2 17

char barcode[15] = {'\0',};
char *barcode_ptr = barcode;

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      // client.subscribe("topico/controle");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  dht.begin();
}

void loop() {
  static unsigned int Tick = 0;

  if (!client.connected()) {
    reconnect();
  }

  if (millis( ) - Tick >= 1000) {
    Tick = millis(); // 1 segundo
    client.loop();
  }

  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\n') {
      Serial.printf("Código lido: %s \n" ,barcode);

      // Faz a leitura do DHT22
      float temperatura = dht.readTemperature();
      float umidade = dht.readHumidity();

      // Verifica se houve erro na leitura
      if (isnan(temperatura) || isnan(umidade)) {
        Serial.println("Erro ao ler o sensor DHT22!");
        return;
      }

      // Buffer do JSON
      static char jsonBuffer[80];

      sprintf(jsonBuffer,
              "{\"codigo\":\"%s\",\"temperatura\":%02.1f,\"umidade\":%02.1f}",
              barcode, temperatura, umidade);

      // Publica no MQTT
      client.publish("univesp/Pi3", jsonBuffer);
      Serial.println("JSON enviado:");
      Serial.println(jsonBuffer);
      
      Serial2.end();
      Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
      // Limpa o buffer do código de barras
      barcode_ptr = barcode;
      memset(barcode, '\0', sizeof(barcode) / sizeof(barcode[0])); // Limpa o buffer
    } 

    else if (c != '\r') {
      *barcode_ptr = c;
      barcode_ptr++;
    }
  }
}
