#line 1 "/home/duarte/Arduino/lab_SCDTR_final_1st/main.hpp"
#ifndef MAIN_HPP
#define MAIN_HPP

#include <Arduino.h>
#include <RunningMedian.h>
#include <arduino-timer.h>
#include <iostream>
#include <limits.h>

// #define DEBUG  // Debug flag

// Constants
#define DAC_RANGE 4096
#define VCC 3300
#define R_GND 10000
#define P_MAX 208 // 208mW

// Pins
#define PIN_MISO 16 // GPIO16
#define PIN_SCK 18  // GPIO18
#define PIN_MOSI 19 // GPIO19
#define PIN_CS 17   // GPIO17
#define PIN_INT 20  // Interrupt pin  GPIO20

#define LED_PIN 15     // Defina o pino GPIO ao qual o LED está conectado
#define PCB_LED_PIN 25 // Defina o pino GPIO ao qual o LED da PCB está conectado

#define MAX_BUFFER_ANSWER_SIZE 100 // Max buffer size for answer
#define BUFFER_CAN_MSG_SIZE 8      // Buffer size for CAN messages
extern uint16_t can_dev_id;        // CAN ID

#define LOG_SAMPLE_LENGTH 6000 // 1 minute

// Mutex
extern mutex_t adc_Mutex; // Mutex for ADC
extern mutex_t pid_Mutex; // Mutex for PID VAR
extern mutex_t log_Mutex; // Mutex for CAN bus

typedef enum
{
    ANSWER,
    ASK
} cmd_type;

void something();
bool send_info(void *);
void pid();
bool read_adc(void *);

void calibre_lux();

float ldr_R(uint16_t read_adc);
float convert_to_lux(float res);
float convert_to_adc(float res);
float convert_to_ldr_r(float lux);
float get_duty_cycle(float LUX);

void adc_init();
bool recv_cmd(void *);
uint16_t set_can_id(uint8_t unique_id[6]);
void send_to_CAN_bus(uint8_t *data, uint8_t len, bool id_flag);
void config_can();
bool read_can_msg(void *);
bool log_data(void *);
void cmd_handler(char *cmd, uint8_t size, bool can_flag, char *answer);

#endif // MAIN_HP