#line 1 "/home/duarte/Arduino/lab_SCDTR_1st/func.cpp"
#include "main.hpp"

extern uint16_t adc;
extern float pwm ;
extern uint16_t target_lux;
extern float actual_ldr_R;
extern float actual_lux;
extern uint16_t info_sample_time;

extern float freq;

// PID variables
extern float error;
extern float integral ;
extern float derivative;

extern float lux_error;

extern float Kp;
extern float Ki;
extern float Kd;

bool send_info(void *)
{
    mutex_enter_blocking(&pid_Mutex);
    Serial.print("TARGET_LUX:");
    Serial.print(target_lux);
    Serial.print(" READ_LUX:");
    Serial.print(actual_lux);
    Serial.print(" Ldr_R:");
    Serial.print(actual_ldr_R);
    Serial.print(" Error:");
    Serial.print(error);
    Serial.print(" Integral:");
    Serial.print(integral);
    Serial.print(" Derivative:");
    Serial.print(derivative);
    Serial.print(" Kp:");
    Serial.print(Kp);
    Serial.print(" Ki:");
    Serial.print(Ki);
    Serial.print(" Kd:");
    Serial.print(Kd);
    Serial.print(" LUX_Err:");
    Serial.print(lux_error);
    Serial.print(" Freq:");
    Serial.print(freq*(float)1000.0/info_sample_time);
    freq=0;
    mutex_exit(&pid_Mutex);

    Serial.print(" PWM:");
    mutex_enter_blocking(&pwm_Mutex);    
    Serial.print(pwm);
    mutex_exit(&pwm_Mutex);

    Serial.print(" ADC:");
    mutex_enter_blocking(&adc_Mutex);
    Serial.print(adc);
    mutex_exit(&adc_Mutex);

    Serial.println();
    
    return true;
}

void send_cmd_to_CAN_bus(char *str)
{
    //Serial.println(str);
}