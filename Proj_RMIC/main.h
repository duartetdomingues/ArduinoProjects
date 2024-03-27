#ifndef Main_h
#define Main_h

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <PulseSensorPlayground.h>
#include <Wire.h>
#include <LSM303.h>
#include <L3G.h>
#include <LPS.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <stdint.h>

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
#define TX_INTERVAL 60
#define CS_INTERVAL  2

void Accel_Mag_Init();
void Read_Accel_Mag();

void Gyro_Init();
void Read_Gyro();

void Baro_Init();
void Read_Baro();


void Read_Accel();
void Read_Mag();
void Read_Compass();

void do_send(osjob_t* j);
void onEvent (ev_t ev);

typedef struct gps_data{
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t hour;
  uint8_t min;
  float lat;
  float lng;
};



typedef struct output_data{
  uint16_t baro;
  uint8_t bmp;
  uint8_t emergency;
  gps_data gps;
  uint16_t temp;
};

union output_data_union{
  struct output_data struct_out;
  uint8_t data[19];
};

#endif /* Main_h */