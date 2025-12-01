#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Keypad.h>

#define rxPin 11 
#define txPin 12 

SoftwareSerial mySerial(rxPin, txPin);
LiquidCrystal_I2C lcd(0X27,16,2);

#define khoi_pin A0
#define chay_pin A1
#define coi_chay 13
#define quat 2


const byte rows=4; 
const byte columns=4; 

const int holdDelay=700; 
unsigned char temp=0; 


char keys[rows][columns] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'},
};


byte rowPins[rows] = {10,9,8,7};
byte columnPins[columns] = {6,5,4,3};


Keypad keypad = Keypad(makeKeymap(keys), rowPins, columnPins, rows, columns);
bool Engine=false;
bool Maxcool=false;
StaticJsonDocument<5000> key;

bool chay(){
  float value=0;
  value=analogRead(chay_pin);
  if(value < 100)
    {digitalWrite(coi_chay,1); return true;}
  else {digitalWrite(coi_chay,0);return false;}}
bool khoi(){
  float value_khoi=0;
  value_khoi=analogRead(khoi_pin);
  if(value_khoi > 1000)
    {digitalWrite(quat,1);return true;}
  else {digitalWrite(quat,0);return false;}}

void setup(){
  Serial.begin(115200);
  mySerial.begin(9600);
  pinMode(khoi_pin,INPUT);
  pinMode(chay_pin,INPUT);
  pinMode(quat,OUTPUT);
  pinMode(coi_chay,OUTPUT);

  key["Khoi"]=false;
  key["Chay"]=false;
  key["engine"]=Engine;
  key["maxcool"]=Maxcool;
  key["value_speed"]=temp;}
void loop(){
  key4x4();delay(50);
  key["Khoi"]=false;
  key["Chay"]=false;
  if(chay() || khoi()){
  key["Khoi"]=khoi();
  key["Chay"]=chay();
  String Data;
  serializeJson(key, Data);
  mySerial.println(Data);
  Serial.println(Data);}}
void key4x4() {
  temp = keypad.getKey();
  if (temp) { 
    KeyState state = keypad.getState(); 
    if (state == PRESSED) {
      if (temp == '*') {
        Engine = !Engine; 
      } else if (temp == 'A') {
        Maxcool = true;
        temp = '3'; 
      } else if (Maxcool && temp != '3') {
        Maxcool = false;
      }
      Serial.print("Phím nhấn: ");
      Serial.println(temp); 
   
      key["engine"] = Engine;
      key["maxcool"] = Maxcool;
      key["value_speed"] =temp;
      String Data;
      serializeJson(key, Data); 
      Serial.println(Data); 
      mySerial.println(Data);
    }
  }return;}
