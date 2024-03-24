#include "main.h"


int SENSOR_SIGN[9] = {1,1,1,-1,-1,-1,1,1,1}; //Correct directions x,y,z - gyro, accelerometer, magnetometer
int AN_OFFSET[6]={0,0,0,0,0,0}; //Array that stores the Offset of the sensors
int AN[6]; //array that stores the gyro and accelerometer data


LSM303 acc_mag; // (accelerometer and magnetometer)	

L3G gyro; // Gyroscope

LPS baro; // Barometer




//Initialize Gyro
void Gyro_Init()
{
  if (!gyro.init())
  {
    Serial.println("Failed to autodetect gyro type!");
    while (1);
  }

  gyro.enableDefault();
  
}

//Read Gyro
void Read_Gyro()
{
  gyro.read();

} 

//Initialize Accelerometer
void Accel_Mag_Init()
{
    acc_mag.init();
    acc_mag.enableDefault();
}

// Reads x,y and z accelerometer registers
void Read_Accel_Mag()
{
  acc_mag.read();
  char report[128]={0};
 
}

//Initialize Barometer
void Baro_Init()
{
  if (!baro.init())
  {
    Serial.println("Failed to autodetect barometer type!");
    while (1);
  }

  baro.enableDefault();
}

//Read Barometer
void Read_Baro()
{
 
}

/* //Initialize Compass
void Compass_Init()
{
  if (!mag.init())
  {
    Serial.println("Failed to detect and initialize magnetometer!");
  }
  mag.enableDefault(); // Sets default settings +/- 4 gauss full scale at 10Hz
}

//Initialize Compass (magnotometer)
void Read_Compass()
{
  mag.read();
  
  magnetom_x = SENSOR_SIGN[6] * mag.m.x;
  magnetom_y = SENSOR_SIGN[7] * mag.m.y;
  magnetom_z = SENSOR_SIGN[8] * mag.m.z;

  // 1 Gauge = 100,000 nT
} */