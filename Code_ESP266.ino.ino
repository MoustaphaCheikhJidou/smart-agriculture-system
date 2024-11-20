#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>

// Paramètres WiFi
const char* ssid = "Lemin";
const char* password = "12345678";

// Paramètres MQTT
const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "mqttx_f137ba5c";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.publish("/ThinkIOT/Publish", "Welcome");
      client.subscribe("/ThinkIOT/Subscribe");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup2() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop2() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (Serial.available()) {
    String jsonString = Serial.readStringUntil('\n');
    JSONVar jsonData = JSON.parse(jsonString);

    if (JSON.typeof(jsonData) == "undefined") {
      Serial.println("Parsing input failed!");
      return;
    }

    String temperature = JSON.stringify(jsonData["temperature"]);
    String humidity = JSON.stringify(jsonData["humidity"]);
    String soilMoisture = JSON.stringify(jsonData["soilMoisture"]);
    
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Soil Moisture: ");
    Serial.println(soilMoisture);

    client.publish("/Thinkitive/temp", temperature.c_str());
    client.publish("/Thinkitive/hum", humidity.c_str());
    client.publish("/Thinkitive/humsol", soilMoisture.c_str());
  }
}