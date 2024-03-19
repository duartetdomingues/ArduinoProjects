#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#define rxGPS 3
#define txGPS 2
 
long lat, lon;
SoftwareSerial gpsSerial(rxGPS, txGPS);
TinyGPSPlus gps;

uint32_t count = 0;
 
void setup()
{
  delay(1000);
  Serial.begin(9600); // connect serial
  gpsSerial.begin(9600); // connect gps sensor
  while (gpsSerial==0)
  {
    Serial.println("GPS not detected");
    gpsSerial.begin(9600); // connect gps sensor
    delay(1000);
  }
  if
}
 
void loop()
{
  Serial.print("GPS Available:");
  Serial.print(gpsSerial.available());
  Serial.print(" count:");
  Serial.println(count);
  count++;

  while (gpsSerial.available())     // check for gps data
  {
    if (gps.encode(gpsSerial.read()))   // encode gps data
    {
      Serial.print("SATS: ");
      Serial.println(gps.satellites.value());
      Serial.print("LAT: ");
      Serial.println(gps.location.lat(), 6);
      Serial.print("LONG: ");
      Serial.println(gps.location.lng(), 6);
      Serial.print("ALT: ");
      Serial.println(gps.altitude.meters());
      Serial.print("SPEED: ");
      Serial.println(gps.speed.mps());
 
      Serial.print("Date: ");
      Serial.print(gps.date.day()); Serial.print("/");
      Serial.print(gps.date.month()); Serial.print("/");
      Serial.println(gps.date.year());
 
      Serial.print("Hour: ");
      Serial.print(gps.time.hour()); Serial.print(":");
      Serial.print(gps.time.minute()); Serial.print(":");
      Serial.println(gps.time.second());
      Serial.println("---------------------------");
      delay(4000);
    }
  }
  delay(200);
}