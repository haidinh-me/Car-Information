#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#define wifi_SSID1 "*****"
#define wifi_PASSWORD1 "*****"
#define wifi_SSID2 "*****"
#define wifi_PASSWORD2 "*****"
#define wifi_SSID3 "*****"
#define wifi_PASSWORD3 "*****"

const String baseURL = "*****";
const String authToken = "*****";

#define buttonfl_pin 16
#define buttonrl_pin 5
#define buttonfr_pin 19
#define buttonrr_pin 23
#define ledfl_pin 13
#define ledrl_pin 12
#define ledfr_pin 14
#define ledrr_pin 27

volatile bool flag_fl = false;
volatile bool flag_rl = false;
volatile bool flag_fr = false;
volatile bool flag_rr = false;

WiFiMulti wifiMulti;

void setup() {
  Serial.begin(115200);

  pinMode(buttonfl_pin,INPUT_PULLUP);pinMode(buttonrl_pin,INPUT_PULLUP);pinMode(buttonfr_pin,INPUT_PULLUP);pinMode(buttonrr_pin,INPUT_PULLUP);
  pinMode(ledfl_pin,OUTPUT);pinMode(ledfr_pin,OUTPUT);pinMode(ledrl_pin,OUTPUT);pinMode(ledrr_pin,OUTPUT);

  attachInterrupt(digitalPinToInterrupt(buttonfl_pin),changeled_fl,FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonrl_pin),changeled_rl,FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonfr_pin),changeled_fr,FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonrr_pin),changeled_rr,FALLING);

  wifiMulti.addAP(wifi_SSID1, wifi_PASSWORD1);
  wifiMulti.addAP(wifi_SSID2, wifi_PASSWORD2);
  wifiMulti.addAP(wifi_SSID3, wifi_PASSWORD3);
  Serial.println("Connecting to WiFi...");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(1000);
    Serial.print(F("."));
  }
  Serial.println("\nConnected to WiFi ");}
void loop() {
  if (wifiMulti.run() == WL_CONNECTED) {
    check_update();
    door();
  } else {
    Serial.println("WiFi not connected");
    delay(4000);
  }
}
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
void patchDataToFirebase(String objectPath, String fieldName, String newValue) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = baseURL + objectPath + ".json?auth=" + authToken;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    String jsonData = "{\"" + fieldName + "\": \"" + newValue + "\"}";
    int httpResponseCode = http.sendRequest("PATCH", jsonData);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Response: " + response);
    } else {
      Serial.print("Error on PATCH request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }}
void door(){
  if(flag_fl){
    flag_fl=false;
    if(digitalRead(ledfl_pin))
      patchDataToFirebase("DOOR","fl","true");
    else
      patchDataToFirebase("DOOR","fl","false");
    }
  if(flag_rl){
    flag_rl=false;
    if(digitalRead(ledrl_pin))
      patchDataToFirebase("DOOR","rl","true");
    else
      patchDataToFirebase("DOOR","rl","false");
    }
    if(flag_fr){
      flag_fr=false;
      if(digitalRead(ledfr_pin))
        patchDataToFirebase("DOOR","fr","true");
    else
        patchDataToFirebase("DOOR","fr","false");
    }
    if(flag_rr){
      flag_rr=false;
      if(digitalRead(ledrr_pin))
        patchDataToFirebase("DOOR","rr","true");
    else
        patchDataToFirebase("DOOR","rr","false");
    }}
void check_update(){
  StaticJsonDocument<200> doc;
  deserializeJson(doc, getDataFromFirebase("DOOR"));
  if(digitalRead(ledfl_pin)!=stringToBool(doc["fl"]) && flag_fl==false) digitalWrite(ledfl_pin,stringToBool(doc["fl"]));
  if(digitalRead(ledfr_pin)!=stringToBool(doc["fr"]) && flag_fr==false) digitalWrite(ledfr_pin,stringToBool(doc["fr"]));
  if(digitalRead(ledrl_pin)!=stringToBool(doc["rl"]) && flag_rl==false) digitalWrite(ledrl_pin,stringToBool(doc["rl"]));
  if(digitalRead(ledrr_pin)!=stringToBool(doc["rr"]) && flag_rr==false) digitalWrite(ledrr_pin,stringToBool(doc["rr"]));
  delay(1000);}
bool stringToBool(String str) {
    return (str == "true");}
void changeled_fl(){
  digitalWrite(ledfl_pin,!digitalRead(ledfl_pin));
  flag_fl=true;}
void changeled_rl(){
  digitalWrite(ledrl_pin,!digitalRead(ledrl_pin));
  flag_rl=true;}
void changeled_fr(){
  digitalWrite(ledfr_pin,!digitalRead(ledfr_pin));
  flag_fr=true;}
void changeled_rr(){
  digitalWrite(ledrr_pin,!digitalRead(ledrr_pin));
  flag_rr=true;}