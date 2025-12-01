#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <DHT.h>

#define wifi_SSID1 "*****"
#define wifi_PASSWORD1 "*****"
#define wifi_SSID2 "*****"
#define wifi_PASSWORD2 "*****"
#define wifi_SSID3 "*****"
#define wifi_PASSWORD3 "*****"

const String baseURL = "*****";
const String authToken = "*****";

#define DHT_PIN 15
#define DHT_TYPE DHT11

float tem,humi,retem,rehumi;

DHT dht(DHT_PIN,DHT_TYPE);
WiFiMulti wifiMulti;

void setup() {
  Serial.begin(115200);

  dht.begin();

  wifiMulti.addAP(wifi_SSID1, wifi_PASSWORD1);
  wifiMulti.addAP(wifi_SSID2, wifi_PASSWORD2);
  Serial.println("Connecting to WiFi...");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(1000);
    Serial.print(F("."));
  }
  Serial.println("\nConnected to WiFi ");
}
void dht_sensor(){
  tem=round(dht.readTemperature()*10)/10.0;
  humi=round(dht.readHumidity()*10)/10.0;
  if(tem!=retem || humi!=rehumi)
    putDataToFirebase("TEMP","{\"tem\": "+String(tem)+",\"humi\": "+String(humi)+"}");
  if (isnan(tem) || isnan(humi)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  retem=tem;rehumi=humi;}
String getDataFromFirebase(String path) {
  HTTPClient http;
  String url = baseURL + path + ".json?auth=" + authToken;
  http.begin(url);

  int httpResponseCode = http.GET();
  String payload;

  if (httpResponseCode > 0) {
    payload = http.getString();
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return payload;}
void putDataToFirebase(String path, String jsonData) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = baseURL + path + ".json?auth=" + authToken;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.PUT(jsonData); 

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.print("Error on PUT request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }}
void loop() {
  if (wifiMulti.run() == WL_CONNECTED){
    dht_sensor();
  }
  else {
    Serial.println("WiFi not connected");
    delay(1000);
  }}
