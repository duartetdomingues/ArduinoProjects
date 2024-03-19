const int LED_PIN = 15;
const int DAC_RANGE = 4096;
const int VCC = 3300; 
const int R_GND = 10000;
const int P_MAX = 208; //208mW 

const int GAIN = 2.667;
const int B = 7.2587;



long voltage=0;
long resistence=0;
unsigned long last_ms = 0;

long counter = 0;
void setup() {// the setup function runs once
 Serial.begin(115200);
 analogReadResolution(12); //default is 10
 analogWriteFreq(60000); //60KHz, about max
 analogWriteRange(DAC_RANGE); //100% duty cycle
}


void loop() {// the loop function runs cyclically

 int read_adc;
 analogWrite(LED_PIN, counter); // set led PWM
 delay(1); //delay 1ms
 read_adc = analogRead(A0); // read analog voltage
 voltage=read_adc*VCC/DAC_RANGE;

 resistence=(R_GND*VCC)/voltage-R_GND;
 int duty_cycle= pmw/DAC_RANGE;
 int ms= millis();


 


 counter = counter + 1;
 if (counter > DAC_RANGE) // if counter saturates
  counter = 0; // reset counter
 //format that Serial Plotter likes
 //Serial.print(0); Seial.print(" ");
 Serial.print(read_adc); 
 Serial.print(" ");
 Serial.print(counter); 
 Serial.print(" ");
 Serial.print(voltage);
 Serial.print(" ");
 Serial.print(resistence);
 Serial.println();

 delay(15);
}