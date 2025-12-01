#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2); 

#define wifi_SSID1 "*****"
#define wifi_PASSWORD1 "*****"
#define wifi_SSID2 "*****"
#define wifi_PASSWORD2 "*****"
#define wifi_SSID3 "*****"
#define wifi_PASSWORD3 "*****"

const String baseURL = "*****";
const String authToken = "*****";

#define RX_PIN 16
#define TX_PIN 17
#define trig 32
#define echo 35
#define ena_pin 19
#define in1_pin 18
#define in2_pin 5

HardwareSerial mySerial(2); // Sử dụng UART2


WiFiMulti wifiMulti;
StaticJsonDocument<5000> key;
StaticJsonDocument<5000> doc;
String data; int rep=0;
String data_fb; int revalue_speed=0;
bool flag=true;
unsigned long t; int h,p;

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); // RX_PIN=16, TX_PIN=17

  pinMode(trig,OUTPUT);
  pinMode(echo,INPUT);
  pinMode(ena_pin,OUTPUT);
  pinMode(in1_pin,OUTPUT);
  pinMode(in2_pin,OUTPUT);

  digitalWrite(in1_pin,HIGH);
  digitalWrite(in2_pin,LOW);

  lcd.init();                    
  lcd.backlight();

  lcd.setCursor(0,0);lcd.print("Engine: ");
  lcd.setCursor(0,1);lcd.print("Speed: ");

  wifiMulti.addAP(wifi_SSID1, wifi_PASSWORD1);
  wifiMulti.addAP(wifi_SSID2, wifi_PASSWORD2);
  wifiMulti.addAP(wifi_SSID3, wifi_PASSWORD3);
  Serial.println("Connecting to WiFi...");
  if(wifiMulti.run() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi ");
  }}
void fuel(){
  t=0;h=0;
  digitalWrite(trig,0);
  delayMicroseconds(2);
  digitalWrite(trig,1);
  delayMicroseconds(5);
  digitalWrite(trig,0);

  t = pulseIn(echo,HIGH);
  h = int(t/2/29.412);
  p = ((10 - h) / 8.0) * 100;
  if (h >= 10){
   p = 0; 
  } else if (h < 2){
   p = 100;
  }
  if(p!=rep){
    lcd.setCursor(13,0);lcd.print(String(p)+" ");
    patchDataToFirebase("FUEL","status",String(p));
  }
  rep=p;}
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
void loop() {
  if(mySerial.available()>0){
    data=mySerial.readStringUntil('\n');
    DeserializationError error = deserializeJson(key, data);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    Serial.println(data);
    if(key["engine"]==true){
    lcd.setCursor(8,0);lcd.print("ON ");
    if(key["value_speed"]==48) {analogWrite(ena_pin,0);lcd.setCursor(7,1);lcd.print(0);}
    else if(key["value_speed"]==49) {analogWrite(ena_pin,85);lcd.setCursor(7,1);lcd.print(1);}
    else if(key["value_speed"]==50) {analogWrite(ena_pin,170);lcd.setCursor(7,1);lcd.print(2);}
    else if(key["value_speed"]==51) {analogWrite(ena_pin,255);lcd.setCursor(7,1);lcd.print(3);}
    if(key["maxcool"]) {lcd.setCursor(13,1);lcd.print("max");}
    else if(!key["maxcool"]) {lcd.setCursor(13,1);lcd.print("   ");}}
    putDataToFirebase("FAN",data);return;}
  fuel();
  if (wifiMulti.run() == WL_CONNECTED) {
    data_fb=getDataFromFirebase("FAN");
    deserializeJson(doc,data_fb);

    if((doc["engine"]==false || doc["engine"]=="false") && flag) {lcd.setCursor(8,0);lcd.print("OFF");
    analogWrite(ena_pin,0);Serial.println(0);lcd.setCursor(7,1);lcd.print(0);lcd.setCursor(13,1);lcd.print("   ");flag=false;return;}
    if(doc["engine"]==true || doc["engine"]=="true"){lcd.setCursor(8,0);lcd.print("ON ");flag=true;}
    if(doc["value_speed"].as<int>()!=revalue_speed && (doc["engine"]==true ||doc["engine"]=="true")){
    if(doc["value_speed"].as<int>()==48) {analogWrite(ena_pin,0);Serial.println(0);lcd.setCursor(7,1);lcd.print(0);}
    else if(doc["value_speed"].as<int>()==49) {analogWrite(ena_pin,85);Serial.println(85);lcd.setCursor(7,1);lcd.print(1);}
    else if(doc["value_speed"].as<int>()==50) {analogWrite(ena_pin,170);Serial.println(170);lcd.setCursor(7,1);lcd.print(2);}
    else if(doc["value_speed"].as<int>()==51) {analogWrite(ena_pin,255);Serial.println(255);lcd.setCursor(7,1);lcd.print(3);}
    if(doc["maxcool"]==true ||doc["maxcool"]=="true") {lcd.setCursor(13,1);lcd.print("max");}
    else if(doc["maxcool"]==false ||doc["maxcool"]=="false") {lcd.setCursor(13,1);lcd.print("   ");}
    revalue_speed=doc["value_speed"].as<int>();flag=true;}
  } else {
    Serial.println("WiFi not connected");
    delay(1000);
  }}