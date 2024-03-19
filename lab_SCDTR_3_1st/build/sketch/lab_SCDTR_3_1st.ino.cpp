#include <Arduino.h>
#line 1 "/home/duarte/Arduino/lab_SCDTR_3_1st/lab_SCDTR_3_1st.ino"
#include "main.hpp"
#include "hardware/flash.h"

// Sample times
uint16_t adc_sample_time = 1;
uint16_t pid_sample_time = 10;
uint16_t info_sample_time = 25;
uint16_t rcv_cmd_sample_time = 10;
uint16_t rcv_can_sample_time = 10;
uint16_t log_data_sample_time = 10;

// CAN variables
uint16_t can_dev_id=0;

// Global variables
float Gain_lux_to_pwm;
float Offset_lux_to_pwm;
float read_lux_max;
float read_lux_min;
int16_t target_lux;

extern float pwm;
extern float last_ms;

uint64_t n_samples = 0;

auto timer = timer_create_default();
auto timer1 = timer_create_default();

mutex_t adc_Mutex; // Mutex for ADC
mutex_t pid_Mutex; // Mutex for PID VAR
mutex_t log_Mutex; // Mutex for LOG

pthread_t thread;

RunningMedian median(pid_sample_time / adc_sample_time);

bool flag_init = true;

#line 40 "/home/duarte/Arduino/lab_SCDTR_3_1st/lab_SCDTR_3_1st.ino"
void setup();
#line 83 "/home/duarte/Arduino/lab_SCDTR_3_1st/lab_SCDTR_3_1st.ino"
void setup1();
#line 97 "/home/duarte/Arduino/lab_SCDTR_3_1st/lab_SCDTR_3_1st.ino"
void loop();
#line 132 "/home/duarte/Arduino/lab_SCDTR_3_1st/lab_SCDTR_3_1st.ino"
void loop1();
#line 40 "/home/duarte/Arduino/lab_SCDTR_3_1st/lab_SCDTR_3_1st.ino"
void setup()
{ // the setup function runs once

    pinMode(LED_PIN, OUTPUT);    // Set the LED pin as an output
    pinMode(PCB_LED_PIN, OUTPUT); // Set the PCB LED pin as an output
    analogReadResolution(12);    // default is 10
    analogWriteFreq(60000);      // 60KHz, about max
    analogWriteRange(DAC_RANGE); // 100% duty cycle
    Serial.begin(115200);        // Start the serial communication
    calibre_lux();               // Calibrate the LDR

    uint8_t unique_id[6];
    flash_get_unique_id(unique_id); // Read the unique ID of the device
    Serial.printf("Unique ID: %02X:%02X:%02X:%02X:%02X:%02X\n", unique_id[0], unique_id[1], unique_id[2], unique_id[3], unique_id[4], unique_id[5]);
    can_dev_id=set_can_id(unique_id); // Set the CAN ID
    Serial.printf("CAN ID: %d\n", can_dev_id);
    mutex_init(&adc_Mutex); // Initialize the mutex
    mutex_init(&pid_Mutex); // Initialize the mutex
    mutex_init(&log_Mutex); // Initialize the mutex

    config_can(); // Configure the CAN bus

    target_lux = (read_lux_min+read_lux_max)/2; // Set the target LUX to half of the max LUX

    pwm = get_duty_cycle(target_lux);
    analogWrite(LED_PIN, constrain(pwm, 0, DAC_RANGE)); // Set the initial duty cycle

    Serial.printf("First pwm %f\n", pwm);

    delay(100); // Wait for the LDR to stabilize
    adc_init();

    // Shedule tasks
    timer.every(info_sample_time, send_info); // loop() every info_sample_time
    timer.every(rcv_cmd_sample_time, recv_cmd); // read_adc() every adc_sample_time
    timer.every(rcv_can_sample_time, read_can_msg); // read_adc() every adc_sample_time
    timer.every(log_data_sample_time, log_data); // loop() every pid_sample_time

    flag_init = false; // Set the flag to false to run pid thread
}



void setup1()
{
    while (flag_init)
    {
        delay(1);
    }
    Serial.println("PID Thread");
    last_ms = (float)micros() / 1000;
    timer1.every(adc_sample_time, read_adc); // read_adc() every adc_sample_time
}

int j = 1;
bool flag = true;
extern float Ki;
void loop()
{
    timer.tick();
    if (micros() > j * 5e6)
    {
        j++;
       
        /* if (pwm > 100 )
            flag = false;
        else if (pwm < 100)
            flag = true; */
/* 
        if (target_lux > 2*read_lux_max)
            flag = false;

        if (target_lux < 1)
            flag = true; */
        if (j==11)
            exit(0);

        mutex_enter_blocking(&pid_Mutex);
        /* if (flag)
            target_lux += 30;
        else
            target_lux -= 30; */

        target_lux = random(0, 40);

        mutex_exit(&pid_Mutex);
        median.clear();
        adc_init();
    }
    read_adc(NULL);
}

void loop1()
{
    pid();
    //timer1.tick();

}
