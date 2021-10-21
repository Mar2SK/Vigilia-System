#include <TinyGPS++.h>
#include <Blynk.h>
#include <ThingSpeak.h>
#include <SoftwareSerial.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

static const int RXPin = 4, TXPin = 5;   //GPIO 4 = D2 (conectar Tx de GPS) y GPIO 5 = D1 (Conectar Rx de GPS
static const uint32_t GPSBaud = 9600; //si la velocidad en baudios 9600 no funcionó en su caso, use 4800 

TinyGPSPlus gps; // El objeto TinyGPS ++ 
WidgetMap myMap(V0);  // V0 para pin virtual de Map Widget 

SoftwareSerial ss(RXPin, TXPin);  // La conexión en serie al dispositivo GPS 
BlynkTimer timer;

float spd;       //Variable para almacenar la velocidad 
float sats;      //Variable para almacenar no. de respuesta de los satélites 
String bearing;  //Variable para almacenar la orientación o dirección del GPS 
char auth[] = "VN3u5BvbIBw6vtKV-ZkpRnXMl3_o-nMu";              //La clave de autenticación de su proyecto
char ssid[] = "Estudiantes";                                       // Nombre de su red (nombre de HotSpot o enrutador) 
char pass[] = "educar_2018";                                      // Contraseña correspondiente
unsigned long myChannelNumber = 1428692; // Thingspeak número de canal
const char * myWriteAPIKey = "V78BGKMHQUFW18TN"; // ThingSpeak write API Key

//unsigned int move_index;         // índice móvil, para usar más tarde 
unsigned int move_index = 1;       // ubicación fija por ahora 
  
WiFiClient client;

void setup()
{
  ThingSpeak.begin(client); // Inicia ThingSpeak
  Serial.begin(115200);
  Serial.println();
  ss.begin(GPSBaud);
  Blynk.begin(auth, ssid, pass);
  timer.setInterval(5000L, checkGPS); // cada 5 segundos compruebe si el GPS está conectado, solo es necesario hacerlo una vez
}

void checkGPS(){
  if (gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
      Blynk.virtualWrite(V4, "GPS ERROR");  // Widget de visualización de valor en V4 si no se detecta GPS 
  }
}

void loop()
{
    while (ss.available() > 0) 
    {
      // El boceto muestra información cada vez que se codifica correctamente una nueva oración. 
      if (gps.encode(ss.read()))
        displayInfo();
  }
  Blynk.run();
  timer.run();
}

void displayInfo()
{ 
  if (gps.location.isValid() ) 
  {    
    float latitude = (gps.location.lat());     //Almacenar el Lat. y Lon. 
    float longitude = (gps.location.lng()); 
    
    Serial.print("LAT:  ");
    Serial.println(latitude, 6);  // flotar a x posiciones decimales 
    Serial.print("LONG: ");
    Serial.println(longitude, 6);
    Blynk.virtualWrite(V1, String(latitude, 6));   
    Blynk.virtualWrite(V2, String(longitude, 6));  
    myMap.location(move_index, latitude, longitude, "GPS_Location");
    spd = gps.speed.kmph();               //conseguir velocidad 
       Blynk.virtualWrite(V3, spd);
       
       sats = gps.satellites.value();    //conseguir datos de los satelites
       Blynk.virtualWrite(V4, sats);

       bearing = TinyGPSPlus::cardinal(gps.course.value()); // conseguir la direccion
       Blynk.virtualWrite(V5, bearing); 

                         // Cargar los valores a enviar a thing speak
ThingSpeak.setField(1, latitude);
ThingSpeak.setField(2, longitude);

// Escribe todos los campos a la vez.
ThingSpeak.writeFields(1428692, "V78BGKMHQUFW18TN");
  }
  
 Serial.println();
}
