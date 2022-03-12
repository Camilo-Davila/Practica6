
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <PubSubClient.h>

#include <Arduino.h>
#include <Wire.h>

#define REPORTING_PERIOD_MS 1000

#define BUTTON_LEFT 0

//*******Sensor de temperatura*****//
#include <DHT.h>
#include <string.h>

#define DHTPIN 27     // definicion del pin al que se conecta el sensor
#define DHTTYPE DHT11 // definir el tipo de dht

//#define LED1Pin 2 // 32
//#define LED2Pin 33

DHT dht(DHTPIN, DHTTYPE); // constructor

// Update these with values suitable for your network.

const char *ssid = "Suarez";               // WiFi name
const char *password = "52110001";         // WiFi password
const char *mqtt_server = "34.201.250.63"; // Your assign IP

TFT_eSPI tft = TFT_eSPI();

// float spO2 = 0;
char hString[8];

// float ritmo = 0;
char tString[8];

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

bool toggle = false;
char toggleString[25];

void isr()
{
  if (toggle)
  {
    Serial.println("on");
  }
  else
  {
    Serial.println("off");
  }
  toggle = !toggle;
}

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("..Message ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/led1")
  {
    if (messageTemp == "1")
    {
      Serial.println("led1 on");
      tft.fillCircle(200, 40, 20, TFT_GREEN);
    }
    else if (messageTemp == "0")
    {
      Serial.println("led1 off");
      tft.fillCircle(200, 40, 20, TFT_DARKGREY);
    }
  }

  if (String(topic) == "esp32/led2")
  {
    if (messageTemp == "1")
    {
      Serial.println("led2 on");
      tft.fillCircle(200, 90, 20, TFT_YELLOW);
    }
    else if (messageTemp == "0")
    {
      Serial.println("led2 off");
      tft.fillCircle(200, 90, 20, TFT_DARKGREY);
    }
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (client.connect("ESP32Client22"))
    {
      Serial.println("connected");

      // subscribe
      client.subscribe("esp32/led1");
      client.subscribe("esp32/led2");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  attachInterrupt(BUTTON_LEFT, isr, RISING);

  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();    // lee la humedad en fnc read y la almacena en h
  float t = dht.readTemperature(); // lee la temperatura en fnc read y la almacena en t
  /*if (isnan(h) || isnan(t))
  {
    //Serial.println(F("Failed to read from DHT sensor!")); // isnan nos devuelve un 1 en caso de ue exista un fallo o un error en la lectura de la vble

    tft.fillScreen(TFT_BLACK);
    tft.drawString("Failed to read from DHT sensor", 20, 50, 2); // X, Y, FONT

    return;
  }*/

  unsigned long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
    // tft.fillScreen(TFT_BLACK);

    dtostrf(h, 2, 0, hString); // variable, numero de digitos, numero decimales, arreglo donde guardarlo
    // snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Humedad: ");
    Serial.println(hString);
    client.publish("esp32/temp", hString);
    tft.setTextColor(TFT_SKYBLUE, TFT_BLACK);
    tft.drawString("Humedad (%):", 5, 70, 2);
    tft.drawString(hString, 50, 90, 6);

    dtostrf(t, 2, 1, tString); // variable, numero de digitos, numero decimales, arreglo donde guardarlo
    Serial.print("Temperatura: ");
    Serial.println(tString);
    client.publish("esp32/humedad", tString);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("Temperatura (°C):", 5, 0, 2);
    tft.drawString(tString, 30, 20, 6);

    sprintf(toggleString, "%d", toggle);
    Serial.print("Toggle: ");
    Serial.println(toggleString);
    client.publish("esp32/sw", toggleString);
  }
}