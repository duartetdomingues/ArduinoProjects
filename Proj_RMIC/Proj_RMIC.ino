#include "main.h"
 
#define GRAVITY 256  //this equivalent to 1G in the raw data coming from the accelerometer
#define LED_PIN 13
#define BUTTON_PIN 11
#define PULSE_SENSOR 0 // Pulse Sensor Analog Pin 0

PulseSensorPlayground pulseSensor;

TinyGPSPlus gps;

uint32_t count = 0;

extern LSM303 acc_mag;  // (accelerometer and magnetometer)	
extern L3G gyro;        // Gyroscope
extern LPS baro;     // Barometer

extern output_data_union mydata;

extern osjob_t sendjob, checkjob;

long lat, lon;
 
void setup()
{
  delay(1000);
  Serial.begin(115200); // connect serial
  Serial1.begin(9600); // connect gps sensor

  Serial.println("Size of Output Data: " + String(sizeof(mydata)));

  pinMode(12, OUTPUT);  // set the Heartbeat Sensor as output
  digitalWrite(12, HIGH);  // turn on the Heartbeat Sensor 
  
  pulseSensor.analogInput(PULSE_SENSOR, INPUT);   // set the Heartbeat Sensor as input
  pulseSensor.blinkOnPulse(LED_PIN);  // turn on the LED when a heartbeat is detected
  int Threshold = 550;
  pulseSensor.setThreshold(Threshold);

  if (pulseSensor.begin()) {
    Serial.println("Pulse Sensor Init");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }
  pulseSensor.sawStartOfBeat();  // set the Pulse Sensor to start looking for the heartbeat

  pinMode(BUTTON_PIN, INPUT);   // Button as input


  while (Serial1==0)
  {
    Serial.println("GPS not detected");
    Serial1.begin(9600); // connect gps sensor
    delay(1000);
  }
  Serial.println("GPS Init");


  Wire.begin(); // IMU sensor
  Serial.println("SPI Init");

  Gyro_Init();
  Serial.println("Gyro Init");

  Baro_Init();
  Serial.println("Baro Init");

  pulseSensor.analogInput(A0);

  Serial.println("Pulse Init");



  /* Compass_Init();
  Serial.println("Compass Init");

  Gyro_Init();
  Serial.println("Gyro Init");

  for(int i=0;i<32;i++)    // We take some readings...
    {
      Serial.println("Calibrating the sensors " + String(i) + "/32");

    Read_Gyro();
    Read_Accel();
    for(int y=0; y<6; y++)   // Cumulate values
      AN_OFFSET[y] += AN[y];
    delay(20);
    }

  for(int y=0; y<6; y++)
    AN_OFFSET[y] = AN_OFFSET[y]/32;

  AN_OFFSET[5]-=GRAVITY*SENSOR_SIGN[5];

   Serial.println("IMU Init");
 */

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

  // Start job (sending automatically starts OTAA too)
  do_send(&sendjob);

  check_sensor(&checkjob);

}

void loop()
{
  os_runloop_once();
}
/*
void loop()
{
  count++;

  while ( Serial1.available())     // check for gps data
  {
    if (gps.encode( Serial1.read()))   // encode gps data
    {
      logData();
    }
  }

  Read_Accel_Mag();     // Read I2C accelerometer

  Read_Gyro();   // This read gyro data 

  Read_Baro();    // Read I2C magnetometer 
  



  delay(100);
}
*/

void logData()
{

  Serial.print("GPS Data: ");
  Serial.print("SATS: ");
  Serial.print(gps.satellites.value());
  Serial.print("    LAT: ");
  Serial.print(gps.location.lat(), 6);
  Serial.print("    LONG: ");
  Serial.print(gps.location.lng(), 6);
  Serial.print("    ALT: ");
  Serial.print(gps.altitude.meters());
  Serial.print("    SPEED: ");
  Serial.print(gps.speed.mps());
  Serial.print("    Date: ");
  Serial.print(gps.date.day()); Serial.print("/");
  Serial.print(gps.date.month()); Serial.print("/");
  Serial.print(gps.date.year());
  Serial.print("    Hour: ");
  Serial.print(gps.time.hour()); Serial.print(":");
  Serial.print(gps.time.minute()); Serial.print(":");
  Serial.println(gps.time.second());


  char report_acc_mag[128]={0};
  snprintf(report_acc_mag, sizeof(report_acc_mag), "ACC: X:%6d    Y:%6d     Z:%6d    MAG: X:%6d     Y:%6d     Z:%6d", acc_mag.a.x, acc_mag.a.y, acc_mag.a.z, acc_mag.m.x, acc_mag.m.y, acc_mag.m.z);
  Serial.print(report_acc_mag);
  Serial.println();

  char report_gyro[128]={0};
  snprintf(report_gyro, sizeof(report_gyro), "GYRO: X:%6d    Y:%6d     Z:%6d", gyro.g.x, gyro.g.y, gyro.g.z);
  Serial.print(report_gyro);
  Serial.println();
 
  Serial.print("BARO: ");
  Serial.print(baro.readPressureMillibars());
  Serial.print("    ");
  Serial.print("TEMP: ");
  Serial.print(baro.readTemperatureC());
  Serial.print("    ");
  Serial.print("ALT: ");
  Serial.print(baro.pressureToAltitudeMeters(baro.readPressureMillibars()));
  Serial.println();

  digitalRead(BUTTON_PIN) == HIGH ? Serial.println("Button: ON") : Serial.println("Button: OFF");
  pulseSensor.sawStartOfBeat();
  int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
  Serial.print("♥  A HeartBeat Happened ! -> "); // If test is "true", print a message "a heartbeat happened".
  Serial.print("BPM: ");                        // Print phrase "BPM: " 
  Serial.println(myBPM);                        // Print the value inside of myBPM. 
  Serial.println("--------------------------------------------------------------------------------------------");
  
}

uint8_t emergency(){
  bool button = (digitalRead(BUTTON_PIN) == HIGH);
  bool fall = false; //TODO fall logic
  return button + (fall << 1);
}

void check_sensor(osjob_t* j){

  count++;

  // Read everything from every sensor
  while ( Serial1.available())     // check for gps data
  {
    if (gps.encode( Serial1.read()))   // encode gps data
    {
      logData();
    }
  }

  Read_Accel_Mag();     // Read I2C accelerometer

  Read_Gyro();   // This read gyro data 

  Read_Baro();    // Read I2C magnetometer 

  pulseSensor.sawStartOfBeat();
  int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".


  // Store all values
  mydata.struct_out.baro = baro.readPressureMillibars();
  mydata.struct_out.bmp = myBPM;
  mydata.struct_out.emergency = emergency();
  mydata.struct_out.temp = baro.readTemperatureC()*1000;
  
  mydata.struct_out.gps.day = gps.date.day();
  mydata.struct_out.gps.month = gps.date.month();
  mydata.struct_out.gps.year = gps.date.year();
  mydata.struct_out.gps.hour = gps.time.hour();
  mydata.struct_out.gps.min = gps.time.minute();
  mydata.struct_out.gps.lat = gps.location.lat();
  mydata.struct_out.gps.lng = gps.location.lng();

  os_setTimedCallback(&checkjob, os_getTime()+sec2osticks(CS_INTERVAL), check_sensor);   
}
