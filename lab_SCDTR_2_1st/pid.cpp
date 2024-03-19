#include "main.hpp"

/* float Kp = 10;
float Ki = 5;
float Kd = 5000; */

/* float Kp = 1.453616557998453;
float Ki = 15.698740055994110;
float Kd = 0.029649162291777; */

float Kp = 1.0;
/* float Ki=0.077963779270314/10; */
float Ki = 0.01;
float Kd = 0.001;

const int integralMax = 20000;

// PID variables
float error, last_error = 0;
float integral = 0;
float derivative;

float last_ms = 0;
float time_ms = 0;

uint16_t adc;
float pwm;
float actual_ldr_R;
float actual_lux;
float lux_error;
float lux_pid;

float freq = 0;

extern int16_t target_lux;

extern RunningMedian median;

extern int16_t buffer_duty_cycle[LOG_SAMPLE_LENGTH];
extern int16_t buffer_lux[LOG_SAMPLE_LENGTH];
int16_t buffer_target_lux[LOG_SAMPLE_LENGTH] = {0};

extern uint64_t n_samples;
extern uint16_t log_data_sample_time;

// Metrics Variables
float energy_consumption;
float visibility_error;
float flicker;
uint32_t pid_freq;

bool read_adc(void *)
{
#ifdef DEBUG
// Serial.println("READ ADC");
#endif

    // Lock the mutex
    mutex_enter_blocking(&adc_Mutex);

    median.add(analogRead(A0));

    // Unlock the mutex
    mutex_exit(&adc_Mutex);

    return true;
}

float log_time = 0, log_last_time = 0;

bool log_data(void *)
{
#ifdef DEBUG
// Serial.println("LOG DATA");
#endif

    n_samples++; // Increment the number of samples

    log_last_time = log_time;

    log_time = micros() / 1000.0;

    //digitalWrite(PCB_LED_PIN, !digitalRead(PCB_LED_PIN));

    uint16_t sample_n = n_samples % LOG_SAMPLE_LENGTH; // Get the sample number

    mutex_enter_blocking(&pid_Mutex);
    buffer_duty_cycle[sample_n] = pwm;
    buffer_lux[sample_n] = actual_lux;
    buffer_target_lux[sample_n] = target_lux;
    pid_freq = (uint32_t)(freq * (float)1000.0 / (log_time - log_last_time));
    // Serial.printf("Freq: %d\n", pid_freq);
    freq = 0;
    mutex_exit(&pid_Mutex);

    mutex_enter_blocking(&log_Mutex);

    // float fk = min(abs(buffer_duty_cycle[sample_n] - buffer_duty_cycle[sample_n - 1]) + abs(buffer_lux[sample_n - 1] - buffer_lux[sample_n - 2]), 0);
    float fk = 0;

    int n = min(n_samples, LOG_SAMPLE_LENGTH); // Get the number of samples in the buffer

    if (n_samples > 100) // Min 1 second of data to compute the metrics
    {
        flicker = 0;
        energy_consumption = 0;
        visibility_error = 0;
        for (int i = 0; i < n; i++)
        {
            if (((buffer_duty_cycle[(n + i) % n] - buffer_duty_cycle[(n + i - 1) % n]) * (buffer_duty_cycle[(n + i - 1) % n] - buffer_duty_cycle[(n + i - 2) % n])) < 0)
                fk = abs(buffer_duty_cycle[(n + i - 1) % n] - buffer_duty_cycle[(n + i - 1) % n]) + abs(buffer_duty_cycle[(n + i - 1) % n] - buffer_duty_cycle[(n + i - 2) % n]);
            else
                fk = 0;
            flicker += fk;
            energy_consumption += (float)P_MAX / 1000.0 * buffer_duty_cycle[i] / DAC_RANGE * (log_data_sample_time) / 1000.0;
            visibility_error += max(0, buffer_target_lux[i] - buffer_lux[i]);
        }
        flicker /= n;
        visibility_error /= n;
    }

    mutex_exit(&log_Mutex);
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

    // Get the actual LDR resistance
    actual_ldr_R = ldr_R(adc);
    // Convert the resistance to actual LUX
    actual_lux = convert_to_lux(actual_ldr_R);
    // Compute the error
    error = target_lux - actual_lux;

    if (error > 0 && pwm > 4095 || error < 0 && pwm < 0)
    {
        error = 0;
    }

    // energy_consumption += P_MAX * pwm / DAC_RANGE * (time_ms - last_ms);
    // visibility_error += n_samples / (n_samples + 1) * visibility_error + 1 / (n_samples + 1) * max(0, error);

    // Count the frequency
    freq++;

    // Compute derivative
    derivative = (error - last_error) / (time_ms - last_ms);
    // derivative = 0;
    //  Compute integral
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

    last_ms = time_ms;
    last_error = error;

    // Write the PWM

    pwm = get_duty_cycle(lux_pid);
    uint16_t pwm_write = constrain(pwm, 0, DAC_RANGE);
    mutex_exit(&pid_Mutex);

    // Set the PWM
    analogWrite(LED_PIN, pwm_write);
}
