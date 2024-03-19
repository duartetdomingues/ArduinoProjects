#include <Arduino.h>
#line 1 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino"
#include <RunningMedian.h>

const int LED_PIN = 15;
const int DAC_RANGE = 4096;
const int VCC = 3300; 
const int R_GND = 10000;
const int P_MAX = 208; //208mW 

const float GAIN = 2.667 ;
const float B = 6.152183;

RunningMedian median(5);

const float Kp = 0.02;
const float Ki = 0.25;
const float Kd = 0.015;
const int integralMax = 10000; 
const int sampleTime = 100; // Sampling time in milliseconds


double error, last_error = 0;
double integral = 0;
double derivative;
int lux_out=0;
int lux = 10;

int read_lux_max;
float G;
int j=0;

#line 31 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino"
void calibre();
#line 56 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino"
void setup();
#line 66 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino"
int get_duty_cycle(int LUX);
#line 71 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino"
int ldr_R(int read_adc);
#line 77 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino"
int convert_to_lux(int r);
#line 83 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino"
void loop();
#line 31 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino"
void calibre(){
  analogWrite(LED_PIN, DAC_RANGE); 
  delay(1000);
  int read_adc = analogRead(A0);
  int res=ldr_R(read_adc);
  read_lux_max = convert_to_lux(res);
  Serial.print("LUX(MAX):");
  Serial.println(read_lux_max);


  analogWrite(LED_PIN, 9); 
  delay(1000);
  read_adc = analogRead(A0);
  res=ldr_R(read_adc);
  int read_lux_0 = convert_to_lux(res);
  Serial.print("LUX(0):");
  Serial.println(read_lux_0);

  G=(DAC_RANGE)/(read_lux_max-read_lux_0);
  Serial.print("GAIN:");
  Serial.println(G);


}

void setup() {// the setup function runs once
 Serial.begin(115200);
 delay(7000);
 analogReadResolution(12); //default is 10
 analogWriteFreq(60000); //60KHz, about max
 analogWriteRange(DAC_RANGE); //100% duty cycle
 calibre();
}

//PWM=LUX^G  PWM=[0,1]
int get_duty_cycle(int LUX){

  return constrain(LUX * G, 0 ,DAC_RANGE);
}

int ldr_R(int read_adc){
  int voltage=read_adc*VCC/DAC_RANGE;
  int resistence=(R_GND*VCC)/voltage-R_GND;
  return resistence;
}

int convert_to_lux(int r){
  return pow(10, -0.8 *log(r)+ B);
}

int i;
int read_adc;
void loop() {// the loop function runs cyclically
  
 int duty_cycle=get_duty_cycle(lux+lux_out);
 analogWrite(LED_PIN, duty_cycle); 


 delay(sampleTime); //delay 10ms
 for (i=0;i<5;i++){
  median.add(analogRead(A0));
 }
 read_adc= median.getMedian();
 
 int r=ldr_R(read_adc);
 int r_lux = convert_to_lux(r);


// Compute the error
  error = lux - r_lux;
  
  // Compute integral
  integral += error;
  // Integral term
  //integral += error * elapsedTime;
  // Clamp the integral term
  if (integral > integralMax) {
      integral = integralMax;
  } else if (integral < -integralMax) {
     integral = -integralMax;
  }
  
  // Compute derivative
  derivative = (error - last_error);
  
  // Compute PID output
  lux_out = (int)(Kp * error + Ki * integral + Kd * derivative);
  
  // Update last error
  last_error = error;

 //format that Serial Plotter likes
 //Serial.print(0); Seial.print(" ");
 Serial.print("LUX:"); 
 Serial.print(lux); 
 Serial.print(" ");
 Serial.print("Read_LUX:"); 
 Serial.print(r_lux); 
 Serial.print(" ");
 /* Serial.print("PWM:"); 
 Serial.print(duty_cycle);
 Serial.print(" "); */
 Serial.print("R:"); 
 Serial.print(r);
 Serial.print(" Adc:"); 
 Serial.print(read_adc);
 Serial.print(" Error:"); 
 Serial.print(error);
 Serial.print(" Integral:"); 
 Serial.print(integral);
 Serial.print(" Derivative:"); 
 Serial.print(derivative);
 Serial.print(" Out:"); 
 Serial.print(lux_out);
 Serial.println();


 j++;
 if(j>100){
  j=0;
  lux=(lux+9) % (read_lux_max*2);;

  
 }
  

}
