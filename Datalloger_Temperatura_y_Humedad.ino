#include <SD.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <RTClib.h>    // incluye libreria para el manejo del modulo RTC
#define CS_PIN  15
#include "ESP8266WiFi.h"
#include "DHT.h"
#define SENSOR 5    // what digital pin we're connected to
                    //pin2 to D4 on esp board

RTC_DS3231 rtc;     // crea objeto del tipo RTC_DS3231
                                  // Al igual que el LCD, se conectan en los pines SDA = A4; SCL = A5

int TEMPERATURA;
int HUMEDAD;

DHT dht(SENSOR, DHT22);

const char WEBSITE[] = "api.pushingbox.com"; //pushingbox API server
const String devid = "vD94D30A35095846"; //device ID from Pushingbox 

const char* MY_SSID = "EVO-Sussini";
const char* MY_PWD =  "PAOLITO31";

//////////////////////////////// SETUP///////////////////////////////

void setup() {

Serial.begin(115200);

/////////////////INICIALIZACIÓN DEL SENSOR DE TEMPERATURA Y HUMEDAD ////////////////////
   
  dht.begin();      // inicializacion de sensor
  pinMode(SENSOR, INPUT);

/////////////////////////INICIALIZACIÓN DEL RELOJ/////////////////////

Wire.begin(4, 0);           // set I2C pins [SDA = GPIO4 (D2), SCL = GPIO0 (D3)], default clock is 100kHz

/////////////////////////////INICIALIZACIÓN DEL WIFI/////////////////

Serial.print("Conectando con "+*MY_SSID);
WiFi.begin(MY_SSID, MY_PWD);
Serial.println("going into wl connect");

while (WiFi.status() != WL_CONNECTED) //Sin conexion,..Espere mientras conecta
    {
      delay(1000);
      Serial.print(".");
    }
  Serial.println("wl conectado");
  Serial.println("");
  Serial.println("Credencial aceptada! Conectando a wifi\n ");
  Serial.println("");
 
  //////////////////////////////INICIALIZACIÓN DE RELOG RTC////////////////////////////////

   if (! rtc.begin()) {        // si falla la inicializacion del modulo
 Serial.println("Modulo RTC no encontrado !");  // muestra mensaje de error
 while (1);         // bucle infinito que detiene ejecucion del programa
 }
  //Le doy fecha y hora al reloj rtc, es necesario hacerlo una sola vez
  rtc.adjust(DateTime(__DATE__, __TIME__ ));

///////////////////// INICIALIZANDO LECTOR DE TARJETAS/////////////////////

// verifica se o cartão SD está presente e se pode ser inicializado
  if (!SD.begin(CS_PIN)) {
    Serial.println("Falla. Verifique que la tarjeta este correctamente colocada");
    //programa encerrrado
    return;
  }
   
  //se chegou aqui é porque o cartão foi inicializado corretamente  
  Serial.println("Tarjeta Inicializada.");
 
}

void loop() {

/////////////////////LECTURA DEL RELOJ/////////////////////

   DateTime fecha = rtc.now();
   
///////////////////////////COMPRUEBA QUE FUNCIONE EL SENSOR///////////////////////////////

 // Check if any reads failed and exit early (to try again).
  if (isnan(HUMEDAD) || isnan(TEMPERATURA)) {
    Serial.println("Falla en la lectura del sensor DHT!");
    return;
  }
////////////////////LECTURA DEL SENSOR DE °C Y H°///////////
   TEMPERATURA = dht.readTemperature();  // obtencion de valor de temperatura 
   HUMEDAD = dht.readHumidity();
   int hicData = dht.computeHeatIndex(TEMPERATURA, HUMEDAD, false);

    Serial.println("");
    Serial.print("Temperatura ");
    Serial.print(TEMPERATURA);
    Serial.println("");
    Serial.print("Humedad ");
    Serial.print(HUMEDAD);
    Serial.println("");
    Serial.print("Hora ");
    Serial.print(fecha.hour());
    Serial.print(":");
    Serial.print(fecha.minute());
  
////////////////////////////////GUARDADO EN TARJETA//////////////////////////////////

 File dataFile = SD.open("DataLoggerTH.csv", FILE_WRITE);
  // se o arquivo foi aberto corretamente, escreve os dados nele
  if (dataFile) {
    Serial.println("El archivo fue abierto correctamente");
      //formatação no arquivo: linha a linha >> UMIDADE | TEMPERATURA
      dataFile.print(fecha.day());
      dataFile.print(" | ");
      dataFile.print(fecha.month());
      dataFile.print(" | ");
      dataFile.print(fecha.year());
      dataFile.print(" , ");
      dataFile.print(fecha.hour());
      dataFile.print(" : ");
      dataFile.print(fecha.minute());
      dataFile.print(" , ");
      dataFile.print(TEMPERATURA);
      dataFile.print(" , ");
      dataFile.println(HUMEDAD);
      dataFile.print(" , ");
      dataFile.println(hicData);
      
      //Cerrar archivo despues de guardar
      dataFile.close();
  }
  // se o arquivo não pôde ser aberto os dados não serão gravados.
  else {
    Serial.println("Falla al abrir el archivo LOG.txt");
  }

/////////////////////////// ENVIO DE DATOS A LA NUBE////////

WiFiClient client;  //Instantiate WiFi object

    //Start or API service using our WiFi Client through PushingBox
    if (client.connect(WEBSITE, 80))
      { Serial.print("enviando mensaje a la nube");
         client.print("GET /pushingbox?devid=" + devid
       + "&humidityData=" + (String) HUMEDAD
       + "&celData="      + (String) TEMPERATURA
       + "&hicData="      + (String) hicData
         );

      client.println(" HTTP/1.1"); 
      client.print("Host: ");
      client.println(WEBSITE);
      client.println("User-Agent: ESP8266/1.0");
      client.println("Connection: close");
      client.println();
      }
  Serial.println("");
    Serial.print("Temperatura ");
    Serial.print(TEMPERATURA);
    Serial.println("");
    Serial.print("Humedad ");
    Serial.println(HUMEDAD);
    Serial.print(fecha.hour());
    Serial.print(":");
    Serial.print(fecha.minute());
  //intervalo de espera para uma nova leitura dos dados.
  delay(1000 * 60 * 60);
 
}
