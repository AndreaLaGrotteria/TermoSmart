/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>

int temp_set = 15; //valori standard per inizializzare
int temp_rec = 15;

int activate = 7; //pin per triggerare relay

BlynkTimer timer; //timer per processo 

WidgetLED led(V3); //led virtuale Blynk

// Auth Token
char auth[] = "";

// WiFi
char ssid[] = "";
char pass[] = "";

// Software Serial
#include <SoftwareSerial.h>
SoftwareSerial EspSerial(3, 2); // RX, TX

// DHT11
#include <SimpleDHT.h>
int pinDHT11 = 6;
SimpleDHT11 dht11;

// ESP8266 baud rate:
#define ESP8266_BAUD 9600

ESP8266 wifi(&EspSerial);

//funzione che aggiorna il valore della variabile temp_set quando viene cambiata dall'app
BLYNK_WRITE(V2)
{
  temp_set = temp_set + param.asFloat();  
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

  timer.setInterval(1000, core);

  pinMode(activate, OUTPUT);
}

void loop()
{
  Blynk.run();
  timer.run();
  
}

void core(){
  // aquisizione dati dht
  byte temperature = 0;
  byte humidity = 0;
  byte data[40] = {0};
  if (dht11.read(pinDHT11, &temperature, &humidity, data)) {
    return;
  }

  temp_rec = (int)temperature; 
  Blynk.virtualWrite(V0, temp_rec); //widget Blynk per temperature registrata
  Blynk.virtualWrite(V1, temp_set); //widget Blynk per temperature impostata
  
  // funzione per attivare impianto 
  if(temp_set < temp_rec){
    digitalWrite(activate, HIGH);
    led.on(); //led virtuale su Blynk
  } else{
    digitalWrite(activate, LOW);
    led.off(); 
  }
  
}

