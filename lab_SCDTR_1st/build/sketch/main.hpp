#line 1 "/home/duarte/Arduino/lab_SCDTR_1st/main.hpp"
#ifndef MAIN_HPP
#define MAIN_HPP

#include <RunningMedian.h>
#include <arduino-timer.h>
#include <Arduino.h>
#include <iostream>

// Pin definitions
#define LED_PIN 15

// Constants
#define DAC_RANGE  4096
#define VCC  3300
#define R_GND  10000
#define P_MAX  208 // 208mW

extern mutex_t pwm_Mutex; // Mutex for PWM
extern mutex_t adc_Mutex; // Mutex for ADC
extern mutex_t pid_Mutex; // Mutex for PID VAR

void something();
bool send_info(void *);
void pid();
bool read_adc(void *);
float ldr_R(uint16_t read_adc);
float convert_to_lux(float res);
float get_duty_cycle(int LUX);
void adc_init();
bool recv_cmd(void *);
void send_cmd_to_CAN_bus(char *str);

#endif // MAIN_HP