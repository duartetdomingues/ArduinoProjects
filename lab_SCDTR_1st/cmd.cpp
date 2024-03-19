#include "main.hpp"

extern uint16_t target_lux;
extern float actual_lux;
uint8_t my_id = 0;
extern float pwm;

bool occupancy_state = 0;
bool anti_windup_state = 0;
bool feedback_state = 0;
uint16_t external_illuminance = 0;
uint16_t power_consumption = 0;

bool send_lux = false;
bool send_duty_cycle = false;
uint16_t buffer_duty_cycle[600] = {0};
uint16_t buffer_lux[600] = {0};

bool recv_cmd(void *)
{
    uint8_t size;
    if (size = Serial.available())
    {
        Serial.printf("Size:%d \n", size);
        char str[size];
        Serial.readBytes(str, size);
        uint8_t i;
        uint16_t val;
        switch (str[0])
        {
        case 'd': // Set directlt the duty cycle of luminaire i
            uint16_t duty_cycle;
            sscanf(str, "d %d %d", &i, &duty_cycle);
            if (duty_cycle < 0 || duty_cycle > DAC_RANGE)
                Serial.println("err");
            else if (i == my_id)
            {
                analogWrite(LED_PIN, constrain(duty_cycle, 0, DAC_RANGE));
                Serial.println("ack");
            }
            else
            {
                send_cmd_to_CAN_bus(str);
            }
            break;

        case 'r': // Set the illuminance reference of luminaire i
            sscanf(str, "r %d %d", &i, &val);
            if (i == my_id)
            {
                target_lux = val;
                Serial.println("ack");
            }
            else
            {
                send_cmd_to_CAN_bus(str);
            }
            break;
        case 'o': // Set the occupancy state of desk i
            sscanf(str, "o %d %d", &i, &val);
            if (val < 0 || val > 1)
                Serial.println("err");
            else if (i == my_id)
                occupancy_state = val;
            else
            {
                send_cmd_to_CAN_bus(str);
            }
            break;

        case 's': // Start the stream of the real-time variable x of desk i
            switch (str[2])
            {
            case 'l': // Start the stream of the real-time variable 'l' of desk i
                if (i == my_id)
                    send_lux = true;
                else
                    send_cmd_to_CAN_bus(str);
                break;
            case 'd': // Start the stream of the real-time variable 'd' of desk i
                if (i == my_id)
                    send_duty_cycle = true;
                else
                    send_cmd_to_CAN_bus(str);
                break;
            default:
                break;
            }
            break;
        case 'S': // Stop the stream of the real-time variable x of desk i
            switch (str[2])
            {
            case 'l': // Stop the stream of the real-time variable 'l' of desk i
                if (i == my_id)
                    send_lux = false;
                else
                    send_cmd_to_CAN_bus(str);
                break;
            case 'd': // Stop the stream of the real-time variable 'd' of desk i
                if (i == my_id)
                    send_duty_cycle = false;
                else
                    send_cmd_to_CAN_bus(str);
                break;
            default:
                break;
            }
            break;

        case 'g':
            if (str[2] != 'b')
            {
                i = atoi(&str[4]);
                if (i == my_id)
                {
                    switch (str[2])
                    {
                    case 'd': // Get current duty cycle of luminaire i
                        Serial.printf("d %d %d\n", i, (int)pwm);
                        break;
                    case 'r': // Get current illuminance reference of luminaire i
                        Serial.printf("r %d %d\n", i, target_lux);
                        break;
                    case 'l': // Measure the illuminance of luminaire i
                        Serial.printf("l %d %d\n", i, (uint16_t)actual_lux);
                        break;
                    case 'o': // Get the current occupancy state of desk i
                        Serial.printf("o %d %d\n", i, occupancy_state);
                        break;
                    case 'a': // Get anti-windup state of desk i
                        Serial.printf("a %d %d\n", i, anti_windup_state);
                        break;
                    case 'k': // Get feedback state of desk i
                        Serial.printf("k %d %d\n", i, feedback_state);
                        break;
                    case 'x': // Get current external illuminance of desk i
                        Serial.printf("x %d %d\n", i, external_illuminance);
                        break;
                    case 'p': // Get instantaneous power consumption of desk i
                        Serial.printf("p %d %d\n", i, power_consumption);
                        break;
                    case 't': // Get the elapsed time since the last restart
                        Serial.printf("t %d %d\n", i, ((int)millis() / 1000));
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    send_cmd_to_CAN_bus(str);
                }
            }
            else // str[2] == 'b'
            {
                i = atoi(&str[7]);
                switch (str[5])
                {
                case 'd': // Get buffer duty cycle of all luminaires
                    if (i == my_id)
                    {
                        Serial.printf("g b d %d :", i);
                        for (uint16_t i = 0; i < 600; i++)
                        {
                            Serial.printf(" %d,", &buffer_duty_cycle[i]);
                        }
                        Serial.println();
                    }
                    else
                    {
                        send_cmd_to_CAN_bus(str);
                    }
                    break;
                case 'l': // Get buffer illuminance of all luminaires
                    if (i == my_id)
                    {
                        Serial.printf("g b l %d :", i);
                        for (uint16_t i = 0; i < 600; i++)
                        {
                            Serial.printf(" %d,", &buffer_lux[i]);
                        }
                        Serial.println();
                        break;
                    }
                    else
                    {
                        send_cmd_to_CAN_bus(str);
                    }
                }
            }
            break;

        default:
            break;
        }
    }
    return 1;
}