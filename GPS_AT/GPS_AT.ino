
#include <SoftwareSerial.h>
#include <Arduino.h>

void setup()
{
  delay(1000);
  Serial.begin(115200); // Arduino serial
  Serial1.begin(38400); // GPS serial

  if (Serial1==0) {
    Serial.println("GPS Serial Error");
  } else {
    Serial.println("GPS Serial OK");
  }

//  Factory reset
  byte Factory[] = {160, 161, 0, 2, 4, 1, 5, 13, 10}; // Factory reset 			A0 A1 00 02 04 01 05 0D 0A 

//  Reboot
//  byte Hot[] = {160, 161, 00, 15, 01, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 13, 10};  // Hot reboot		A0 A1 00 0F 01 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0D 0A
//  byte Warm[] = {160, 161, 00, 15, 01, 02, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 03, 13, 10}; // Warm reboot	A0 A1 00 0F 01 02 00 00 00 00 00 00 00 00 00 00 00 00 00 03 0D 0A 
//  byte Cold[] = {160, 161, 00, 15, 01, 03, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 02, 13, 10}; // Cold reboot	A0 A1 00 0F 01 03 00 00 00 00 00 00 00 00 00 00 00 00 00 02 0D 0A

// RAM
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 00, 00, 05, 13, 10}; // 4800		A0 A1 00 04 05 00 00 00 05 0D 0A 
    byte baudrateCmd[] = {160, 161, 0, 4, 5, 0, 1, 0, 4, 13, 10}; // 9600		A0 A1 00 04 05 00 01 00 04 0D 0A
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 02, 00, 07, 13, 10}; // 19200		A0 A1 00 04 05 00 02 00 07 0D 0A
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 03, 00, 06, 13, 10}; // 38400		A0 A1 00 04 05 00 03 00 06 0D 0A
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 04, 00, 01, 13, 10}; // 57600		A0 A1 00 04 05 00 04 00 01 0D 0A 
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 05, 00, 00, 13, 10}; // 115200		A0 A1 00 04 05 00 05 00 00 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 01, 00, 15, 13, 10}; // 1 Hz			A0 A1 00 03 0E 01 00 0F 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 02, 00, 12, 13, 10}; // 2 Hz			A0 A1 00 03 0E 02 00 0C 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 04, 00, 10, 13, 10}; // 4 Hz			A0 A1 00 03 0E 04 00 0A 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 05, 00, 11, 13, 10}; // 5 Hz			A0 A1 00 03 0E 05 00 0B 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 08, 00, 06, 13, 10}; // 8 Hz			A0 A1 00 03 0E 08 00 06 0D 0A
  byte updateRateCmd[] = {160, 161, 0, 3, 14, 10, 0, 4, 13, 10}; // 10 Hz			A0 A1 00 03 0E 0A 00 04 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 20, 00, 26, 13, 10}; // 20 Hz			A0 A1 00 03 0E 14 00 1A 0D 0A
  byte waas[] = {160, 161, 00, 3, 55, 01, 00, 54, 13, 10}; // WAAS ON				A0 A1 00 03 37 01 00 36 0D 0A
//  byte waas[] = {160, 161, 00, 03, 55, 00, 00, 55, 13, 10}; // WAAS OFF				A0 A1 00 03 37 00 00 37 0D 0A
	byte mode[] = {160, 161, 00, 3, 9, 00, 00, 9, 13, 10}; // aucun					A0 A1 00 03 09 00 00 09 0D 0A
//  byte mode[] = {160, 161, 00, 03, 09, 01, 00, 08, 13, 10}; // NMEA					A0 A1 00 03 09 01 00 08 0D 0A
//  byte mode[] = {160, 161, 00, 03, 09, 02, 00, 11, 13, 10}; // Binary					A0 A1 00 03 09 02 00 0B 0D 0A

// Flash
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 00, 01, 03, 13, 10}; // 4800		A0 A1 00 04 05 00 00 01 03 0D 0A
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 01, 01, 05, 13, 10}; // 9600		A0 A1 00 04 05 00 01 01 05 0D 0A
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 02, 01, 06, 13, 10}; // 19200		A0 A1 00 04 05 00 02 01 06 0D 0A
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 03, 01, 07, 13, 10}; // 38400		A0 A1 00 04 05 00 03 01 07 0D 0A
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 04, 01, 00, 13, 10}; // 57600		A0 A1 00 04 05 00 04 01 00 0D 0A
//  byte baudrateCmd[] = {160, 161, 00, 04, 05, 00, 05, 01, 01, 13, 10}; // 115200		A0 A1 00 04 05 00 05 01 01 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 01, 01, 14, 13, 10}; // 1 Hz			A0 A1 00 03 0E 01 01 0E 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 02, 01, 13, 13, 10}; // 2 Hz			A0 A1 00 03 0E 02 01 0D 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 04, 01, 11, 13, 10}; // 4 Hz			A0 A1 00 03 0E 04 01 0B 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 05, 01, 10, 13, 10}; // 5 Hz			A0 A1 00 03 0E 05 01 0A 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 08, 01, 07, 13, 10}; // 8 Hz			A0 A1 00 03 0E 08 01 07 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 10, 01, 05, 13, 10}; // 10 Hz			A0 A1 00 03 0E 0A 01 05 0D 0A
//  byte updateRateCmd[] = {160, 161, 00, 03, 14, 20, 01, 27, 13, 10}; // 20 Hz			A0 A1 00 03 0E 14 01 1B 0D 0A
//  byte waas[] = {160, 161, 00, 03, 55, 01, 01, 55, 13, 10}; // WAAS ON				A0 A1 00 03 37 01 01 37 0D 0A
//  byte waas[] = {160, 161, 00, 03, 55, 00, 01, 54, 13, 10}; // WAAS OFF				A0 A1 00 03 37 00 01 36 0D 0A
//	byte mode[] = {160, 161, 00, 03, 09, 00, 01, 08, 13, 10}; // aucun					A0 A1 00 03 09 00 01 08 0D 0A
//  byte mode[] = {160, 161, 00, 03, 09, 01, 01, 09, 13, 10}; // NMEA					A0 A1 00 03 09 01 01 09 0D 0A
//  byte mode[] = {160, 161, 00, 03, 09, 02, 01, 10, 13, 10}; // Binary					A0 A1 00 03 09 02 01 0A 0D 0A

//  If you change the baudrate
  Serial1.write(baudrateCmd, sizeof(baudrateCmd));
  Serial1.flush();
  delay(10);
  Serial1.begin(9600); // New baudrate

  Serial1.write(Factory, sizeof(Factory));

//  Serial1.write(updateRateCmd, sizeof(updateRateCmd));
//  Serial1.write(waas, sizeof(waas));
//  Serial1.write(mode, sizeof(mode));
}

void loop()
{
  if (Serial1.available()){
    Serial.print("GPS: ");
    Serial.write(Serial1.read());
  }
  delay(100);
      
}

