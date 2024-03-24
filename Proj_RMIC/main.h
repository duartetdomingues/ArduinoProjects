#ifndef Main_h
#define Main_h

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <PulseSensorPlayground.h>
#include <Wire.h>
#include <LSM303.h>
#include <L3G.h>
#include <LPS.h>


void Accel_Mag_Init();
void Read_Accel_Mag();

void Gyro_Init();
void Read_Gyro();

void Baro_Init();
void Read_Baro();


void Read_Accel();
void Read_Mag();
void Read_Compass();

#endif /* Main_h */