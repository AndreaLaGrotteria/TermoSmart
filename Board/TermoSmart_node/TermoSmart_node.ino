#include <Wire.h>
#include "SSD1306.h"

#include <DHT.h>

#include <Arduino.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial

ESP8266WiFiMulti WiFiMulti;

String token = "";

float temp_set = 15; //valori standard per inizializzare
float temp_rec = 15;

String temp_set_s;
String temp_rec_s;

#define DHTPIN 5
#define DHTTYPE 22

int activate = 0; //pin per triggerare relay
int button_up = 13;
int button_down = 15;

unsigned long prevTime;

DHT dht(DHTPIN, DHTTYPE);

SSD1306  display(0x3c, D3, D5);

void setup() {


    USE_SERIAL.begin(9600);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    dht.begin();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    pinMode(activate, OUTPUT);
    pinMode(button_up, INPUT);
    pinMode(button_down, INPUT);

    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP("ssid", "pwd");

}

void loop() {

  float t = dht.readTemperature();

  if (isnan(t)){
    USE_SERIAL.println("Failed to read from DHT sensor!");
  } else{
    temp_rec = t;
  }

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "TERMOSMART");
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 12, "T rec: " + String(temp_rec));
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 38, "T set: " + String(temp_set));
  display.display();

  // funzione per attivare impianto 
  if(temp_set < temp_rec){
    digitalWrite(activate, LOW);
  } else{
    digitalWrite(activate, HIGH);
  }

  //funzione per bottoni fisici
  if((digitalRead(button_up)) == HIGH){
    temp_set = temp_set + 0.5;
    delay(100);
  }
  if((digitalRead(button_down)) == HIGH){
    temp_set = temp_set - 0.5;
    delay(100);
  }

  if((millis()-prevTime) > 5000){
    if(WiFiMulti.run() == WL_CONNECTED){
      display.setFont(ArialMT_Plain_10);
      display.drawString(90, 0, "Sync...");
      display.display();
      httpUpdate();        
    }
    prevTime = millis();
  }

  delay(200);
}

void httpUpdate(){
        StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
        JsonObject& JSONencoder = JSONbuffer.createObject();

        temp_rec_s = String(temp_rec);
        temp_set_s = String(temp_set);

        JSONencoder["token"] = token;
        JSONencoder["tempRec"] = temp_rec_s;
        JSONencoder["tempSet"] = temp_set_s;

        char JSONmessageBuffer[300];
        JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        Serial.println(JSONmessageBuffer);
        
        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
        http.begin("url"); //HTTP
        http.addHeader("Content-Type", "application/json"); 

        USE_SERIAL.print("[HTTP] POST...\n");
        
        
         
        // start connection and send HTTP header
        int httpCode = http.POST(JSONmessageBuffer);

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
                if(payload != "ok"){
                  temp_set = payload.toFloat();
                }
            }
        } else {
            USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
