#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 4         // Pino onde o DHT está conectado
#define DHTTYPE DHT22    // Tipo do sensor

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Ravani"; // "WIFI_MESH_IST"; "Ravani";
const char* password = "senai12345"; // "ac1ce1ss6_mesh"; "senai12345";
const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);

#define RXD2 16
#define TXD2 17

char barcode[15] = {'\0',};
char *barcode_ptr = barcode;

void setup_wifi();
void reconnect();
void resetBarcodeBuffer();
void processaSerial2();


void setup_wifi() {

  Serial.println("Init Wi-fi begin");
  WiFi.begin(ssid, password);
  Serial.println("Wi-fi begin");
  unsigned long startAttemptTime = millis();
  delay(500);  // delay menor

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) { // tenta 10 segundos
    delay(100);  // delay menor
    yield();     // permite que o sistema cuide do watchdog
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Falha ao conectar WiFi");
    // opcional: tentar reset ou seguir sem WiFi
  } else {
    Serial.println("WiFi conectado");
  }
}

void reconnect() {

  if (!client.connected()) {
    Serial.println("Tentando conectar MQTT...");
    if (client.connect("Project_PIII")) {
      Serial.println("MQTT conectado");
       client.subscribe("univesp/Pi3");
    } else {
      Serial.print("Falha no MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Serial inicializada!");
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Serial 2 inicializada!");
  setup_wifi();
  Serial.println("Setup Wi-fi");

    client.setServer(mqtt_server, 1883);
    client.subscribe("univesp/Pi3");
    Serial.println("Setup MQTT");
  

  dht.begin();
  Serial.println("DHT");
}

void loop() {
  static unsigned long lastTick = 0;

  // Mantém conexão MQTT
  if (!client.connected()) {
    Serial.println("Reconectando MQTT...");
    reconnect();
  } else {
    client.loop();  // mantém a conexão ativa
  }

  // A cada 1 segundo, pode executar tarefas periódicas
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    Serial.println("1 segundo passou");
  }

  // Processa entrada da Serial2 (código de barras)
  processaSerial2();
}

void processaSerial2() {
  while (Serial2.available()) {
    char c = Serial2.read();

    // Exibe o caractere lido (opcional para debug)
    Serial.print("Lendo char: ");
    Serial.println(c);

    if (c == '\n') {
      Serial.printf("Código lido: %s\n", barcode);

      // Lê temperatura e umidade
      float temperatura = dht.readTemperature();
      float umidade = dht.readHumidity();

      if (isnan(temperatura) || isnan(umidade)) {
        Serial.println("Erro ao ler o sensor DHT22!");
        resetBarcodeBuffer();  // mesmo com erro, limpar o buffer
        return;
      }

      // Formata JSON
      static char jsonBuffer[80];
      snprintf(jsonBuffer, sizeof(jsonBuffer),
               "{\"codigo\":\"%s\",\"temperatura\":%.1f,\"umidade\":%.1f}",
               barcode, temperatura, umidade);

      // Publica no MQTT
      client.publish("univesp/Pi3", jsonBuffer);
      Serial.println("JSON enviado:");
      Serial.println(jsonBuffer);

      resetBarcodeBuffer();  // limpa buffer para próximo código
    } 
    
    else if (c != '\r') {
      size_t len = barcode_ptr - barcode;
      if (len < sizeof(barcode) - 1) {
        *barcode_ptr++ = c;
        *barcode_ptr = '\0';  // mantém terminação nula
      } else {
        Serial.println("Buffer do código de barras cheio! Reiniciando leitura.");
        resetBarcodeBuffer();
      }
    }
  }
}

void resetBarcodeBuffer() {
  barcode_ptr = barcode;
  memset(barcode, 0, sizeof(barcode));
}