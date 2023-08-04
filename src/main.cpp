#include <Arduino.h>

#include <ArduinoJson.h> // Thư viện để tạo JSON
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Wire.h"
#include "SHT31.h"
#include <BH1750.h>

#define SHT31_ADDRESS   0x44

BH1750 lightMeter(0x23);

// Update these with values suitable for your network.

const char* ssid = "MLTECH_SHOP";
const char* password = "mltech@2019";
const char* mqtt_server = "mqtt.viis.tech";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(200)
char msg[MSG_BUFFER_SIZE];
int value = 0;

SHT31 sht;

const char* id = "107c5ac0-31b9-11ee-812e-0b4f8d4da0ea";
const char* user = "5946ed19-e0c5-4273-a8d5-3cdd65f97e46";
const char* pass = ""; // Mật khẩu không có.

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(id, user, pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Wire.begin();
  sht.begin(SHT31_ADDRESS);
  ////Wire.setClock(100000);

  uint16_t stat = sht.readStatus();
  Serial.print(stat, HEX);
  Serial.println();

  // begin returns a boolean that can be used to detect setup problems.
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(F("BH1750 Advanced begin"));
  } else {
    Serial.println(F("Error initialising BH1750"));
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;

    if (lightMeter.measurementReady()) {
    float lux = lightMeter.readLightLevel();
    }

     sht.read();   
    // Tạo một bộ đệm cho JSON
    StaticJsonDocument<JSON_OBJECT_SIZE(3)> jsonBuffer;

     // Thêm giá trị "temp" và "humi" vào JSON
    jsonBuffer["temp"] = sht.getTemperature();
    jsonBuffer["humi"] = sht.getHumidity();
    jsonBuffer["light"] = lightMeter.readLightLevel();; // Thêm giá trị "light" vào JSO

    // Chuyển JSON thành chuỗi
    char msg[MSG_BUFFER_SIZE];
    serializeJson(jsonBuffer, msg, MSG_BUFFER_SIZE);

    // In ra Serial Monitor để kiểm tra chuỗi JSON
    Serial.print("Publish message: ");
    Serial.println(msg);

    // Xuất bản JSON
    client.publish("v1/devices/me/telemetry", msg);

    Serial.print("\t");
    Serial.print(sht.getTemperature(), 1);
    Serial.print("\t");
    Serial.println(sht.getHumidity(), 1);

    Serial.print("Light: ");
    Serial.print(lightMeter.readLightLevel());
    Serial.println(" lx");
        delay(1000);
  }
  }


