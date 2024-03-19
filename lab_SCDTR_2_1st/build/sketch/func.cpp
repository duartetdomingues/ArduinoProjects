#line 1 "/home/duarte/Arduino/lab_SCDTR_2_1st/func.cpp"
#include "main.hpp"

extern float Gain_lux_to_pwm;
extern uint16_t read_lux_max;
extern uint16_t read_lux_min;
extern int16_t target_lux;

// Constants
const float GAIN = 2.667;
const float B = 6.152183;
const float M = -0.8;

// PWM=LUX^G  PWM=[0,4096]]
/**
 * @brief Convert the LUX to duty cycle
 */
float get_duty_cycle(float LUX)
{
    return LUX * Gain_lux_to_pwm;
}

/**
 * @brief Convert the LDR voltage to resistance
 */
float ldr_R(uint16_t read_adc)
{
    float voltage = (float)read_adc * VCC / DAC_RANGE;
    float resistence = (R_GND * VCC) / voltage - R_GND;
    return resistence;
}

/**
 * @brief Convert the LDR voltage to resistance
 */
float convert_to_adc(float res)
{
    float voltage = (VCC * R_GND) / (res + R_GND);
    return (voltage * DAC_RANGE) / VCC;
}

/**
 * @brief Convert the resistance to LUX
 */
float convert_to_lux(float res)
{
    return (float)pow(res / pow(10.0, B), 1.0 / M);
}

float convert_to_ldr_r(float lux)
{
    return (float)pow(10, B) * pow(lux, M);
}

void adc_init()
{
    for (uint8_t i = 0; i < 5; i++)
    {
        read_adc(NULL);
    }
}

void calibre_lux()
{
    analogWrite(LED_PIN, DAC_RANGE);    // Turn on the LED
    delay(1000);                        // Wait for the LDR to stabilize
    int read_adc = analogRead(A0);      // Read Max LDR voltage
    float res = ldr_R(read_adc);        // Convert to resistance
    read_lux_max = convert_to_lux(res); // Convert to LUX
    Serial.print("LUX(MAX):");
    Serial.println(read_lux_max);

    analogWrite(LED_PIN, 0);            // Turn off the LED
    delay(1000);                        // Wait for the LDR to stabilize
    read_adc = analogRead(A0);          // Read Min LDR voltage
    res = ldr_R(read_adc);              // Convert to resistance
    read_lux_min = convert_to_lux(res); // Convert to LUX
    Serial.print("LUX(0):");
    Serial.println(read_lux_min);

    Gain_lux_to_pwm = (float)(DAC_RANGE) / (read_lux_max - read_lux_min); // Calculate the gain for the LDR
    Serial.print("GAIN:");
    Serial.println(Gain_lux_to_pwm);
}