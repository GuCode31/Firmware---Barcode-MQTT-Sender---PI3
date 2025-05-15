# ESP32 + DHT22 + Scanner de Código de Barras + MQTT

Este projeto utiliza um ESP32 para:

- Ler códigos de barras via UART (Serial2)
- Ler temperatura e umidade usando o sensor DHT22
- Enviar os dados coletados em formato JSON para um broker MQTT

## 📡 Conectividade

O ESP32 se conecta a uma rede Wi-Fi e publica os dados em um broker MQTT público (broker.emqx.io) no tópico `univesp/Pi3`.

## 🔧 Requisitos

### Hardware

- ESP32 (com suporte a duas portas seriais)
- Sensor DHT22
- Leitor de código de barras com saída serial TTL
- Jumpers e protoboard (opcional)
- Fonte 5V

### Software

- Arduino IDE
- Bibliotecas instaladas:
  - `DHT sensor library` da Adafruit
  - `PubSubClient` de Nick O'Leary
  - `WiFi` (padrão do ESP32)

## 📌 Conexões

| Componente         | Pino ESP32 |
|--------------------|------------|
| DHT22 (DATA)       | GPIO 4     |
| Código de barras TX| GPIO 16 (RXD2) |
| Código de barras RX| GPIO 17 (TXD2) |

> Obs: o código de barras deve estar configurado para enviar o código seguido de `\n` (quebra de linha)

## 📤 Formato da mensagem JSON enviada

```json
{
  "codigo": "1234567890123",
  "temperatura": 25.3,
  "umidade": 55.7
}
