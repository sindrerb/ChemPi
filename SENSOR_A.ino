#include <OneWire.h>            //SENSOR
#include <DallasTemperature.h>  //SENSOR
#include "RF24.h"               //RADIO
#include <SPI.h>                //RADIO
#include <Wire.h>               //LCD
#include <LiquidCrystal_I2C.h>  //LCD

//MODULE
int ledPin = 3;
char myID = 'A';
float delta = 0;

//TEMPERATURSENSOR
#define ONE_WIRE_BUS 2         //Pin 4
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);


//LCD DISPLAY
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void lcdSetup(){
  lcd.init(); 
  lcd.backlight();
}
void lcdWrite(String lcdDisplay = "Value",float value = 0,int a = 1){
  //lcd.clear();
  lcd.setCursor(0,a);
  lcd.print(lcdDisplay);
  lcd.print(": ");
  lcd.print(value);
}

//RADIO
byte addresses[][6] = {"1Node","2Node"};
RF24 radio(7,8); //Pin 7 & 8 on arduino
int myPipe = 1;
char message;
struct dataStruct{
  float value;
  char ID;
}myData;

float data(){
      sensors.requestTemperatures();
      return sensors.getTempCByIndex(0);
}

void radioSetup(){
  pinMode(ledPin,OUTPUT);  // Indicates activity
  digitalWrite(ledPin,1);  // Test LED
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(addresses[myPipe]);
  radio.openReadingPipe(1,addresses[0]);
  radio.startListening();
  bool active = 0;
  delay(200);              // Your time to shine!
  digitalWrite(ledPin,0);  //Ready for signal
  Serial.println("Waiting for signal");  
  while(!active){
    while (!radio.available() ){
      // Waiting...
    }
    radio.read( &message,sizeof(message)); 
    if(message==myID){
      myData.ID = myID;
      radio.stopListening();
      radio.write(&myID,sizeof(myID));
      radio.startListening();
      digitalWrite(ledPin,1);
      active = 1;
    }
    delay(5);
  }
  Serial.println("Now starting");
}

void radioWrite(float value = 0.0){
    if(radio.available()){
    Serial.println("Recives a call..");  
    radio.read(&message, sizeof(message));
    if(message == myID){
      Serial.println("It's for me!");  
      radio.stopListening();
      myData.value = value;
      Serial.print("Sending my data..");
      Serial.println(myData.value);
      radio.write(&myData,sizeof(myData));
      radio.startListening();
    }
    }
}


void setup() {
  Serial.begin(115200);   // Open serial information, for debugging..
  lcdSetup();
  radioSetup();
}

void loop() {
  float Data = data();
  radioWrite(Data);
  lcdWrite("Temp  (C)",Data,1);
  lcdWrite("Delta (C)",Data-delta,2);
  delta = Data;
  delay(50);
}
