#include "main.hpp"

/* float Kp = 10;
float Ki = 5;
float Kd = 5000; */

/* float Kp = 1.453616557998453;
float Ki = 15.698740055994110;
float Kd = 0.029649162291777; */

float Kp=1.0; 
/* float Ki=0.077963779270314/10; */
float Ki=0.01;
float Kd=0.001;

const int integralMax = 1000/Ki;

// PID variables
float error, last_error = 0;
float integral = 0;
float derivative;

float last_ms = 0;
float time_ms = 0;

uint16_t adc;
float pwm;
float actual_ldr_R ;
float actual_lux;
float lux_error;
float lux_pid;

float freq=0;

extern uint16_t target_lux;

extern RunningMedian median;

extern uint16_t buffer_duty_cycle[600];
extern uint16_t buffer_lux[600];

bool read_adc(void *)
{
    // Lock the mutex
    mutex_enter_blocking(&adc_Mutex);

    median.add(analogRead(A0));

    // Unlock the mutex
    mutex_exit(&adc_Mutex);

    return true;
}

/**
 * @brief PID thread
 */
void pid()
{
    // Lock the mutex
    mutex_enter_blocking(&adc_Mutex);
    // Read the ADC
    adc = median.getMedian();
    // Unlock the mutex
    mutex_exit(&adc_Mutex);

    // Get actual time
    time_ms = micros() / 1000.0;

    // Lock the PID mutex
    mutex_enter_blocking(&pid_Mutex);

    freq ++;

    // Get the actual LDR resistance
    actual_ldr_R = ldr_R(adc);
    // Convert the resistance to actual LUX
    actual_lux = convert_to_lux(actual_ldr_R);
    // Compute the error
    error = target_lux- actual_lux;
    
    // Compute derivative
    derivative = (error - last_error) / (time_ms - last_ms);
    //derivative = 0;
    // Compute integral
    integral += error * (time_ms - last_ms);
    
    // Clamp the integral term
    if (integral > integralMax)
    {
        integral = integralMax;
    }
    else if (integral < -integralMax)
    {
        integral = -integralMax;
    }

    // Compute the PID error
    lux_error = (Kp * error + Ki * integral + Kd * derivative);

    // Compute the PID output
    lux_pid = (float)lux_error;

    // Unlock the PID mutex
    mutex_exit(&pid_Mutex);

    last_ms = time_ms;
    last_error = error;

    // Write the PWM
    mutex_enter_blocking(&pwm_Mutex);
    pwm = get_duty_cycle(lux_pid);

    // Set the PWM
    analogWrite(LED_PIN, constrain(pwm, 0, DAC_RANGE) );

    mutex_exit(&pwm_Mutex);

}
