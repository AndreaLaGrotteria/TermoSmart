#define BLYNK_PRINT Serial

#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>

float temp_set = 15; //valori standard per inizializzare
float temp_rec = 15;

int activate = LED_BUILTIN; //pin per triggerare relay
int button_up = 4;
int button_down = 5;

bool go = true; //solo per primo avvio, manda temperatura al widget

unsigned long tempo = 0;

BlynkTimer timer; //timer per processo 

WidgetLED led(V3); //led virtuale Blynk
WidgetLED led2(V4); //led virtuale Blynk

// Auth Token
char auth[] = "0b60b39e8b234b9399148c68facee802";

// WiFi
char ssid[] = "La Grotteria - Maina Network";
char pass[] = "Giapi@0123456789";

// Software Serial
#include <SoftwareSerial.h>
SoftwareSerial EspSerial(3, 2); // RX, TX

//LCD
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// DHT11
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#define DHTPIN 6
#define DHTTYPE DHT22    
DHT_Unified dht(DHTPIN, DHTTYPE);


// ESP8266 baud rate:
#define ESP8266_BAUD 9600

ESP8266 wifi(&EspSerial);

//funzione che aggiorna il valore della variabile temp_set quando viene cambiata dall'app
BLYNK_WRITE(V2)
{
  temp_set = param.asFloat();  
  Blynk.virtualWrite(V2, temp_set); //widget Blynk per temperature impostata
}


void setup()
{
  // Debug console
  Serial.begin(9600);

  delay(10);

  // SP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  delay(10);

  Blynk.begin(auth, wifi, ssid, pass);

  timer.setInterval(300, core);

  pinMode(activate, OUTPUT);
  pinMode(button_up, INPUT);
  pinMode(button_down, INPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  //DHT setup
  dht.begin();
  sensor_t sensor;
  
  delay(500);
}

void loop()
{
  Blynk.run();
  timer.run();
}


void core(){

  Blynk.virtualWrite(V1, temp_set); //widget Blynk per temperature impostata
  
  if(go == true){
    Blynk.virtualWrite(V2, temp_set);
    go = false;
  }
  
  // aquisizione dati dht
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  temp_rec = event.temperature;
  
  Blynk.virtualWrite(V0, temp_rec); //widget Blynk per temperature registrata
  
  // funzione per attivare impianto 
  if(temp_set < temp_rec){
    digitalWrite(activate, LOW);
    led.off(); //led virtuale su Blynk
  } else{
    digitalWrite(activate, HIGH);
    led.on(); 
  }

   //funzione per bottoni fisici
  if((digitalRead(button_up)) == HIGH){
    temp_set = temp_set + 0.5;
    Blynk.virtualWrite(V2, temp_set); //widget Blynk per temperature impostata
    led2.on();
    delay(100);
  }
  if((digitalRead(button_down)) == HIGH){
    temp_set = temp_set - 0.5;
    Blynk.virtualWrite(V2, temp_set); //widget Blynk per temperature impostata
    led2.off();
    delay(100);
  }
  
  //Mostra Temperatura misurata sul display
  lcd.setCursor(0, 0);
  lcd.print("T rec " + String(temp_rec));
  
  //Mostra Temperatura impostata sul display
  lcd.setCursor(0, 1);
  lcd.print("T set " + String(temp_set));
}
