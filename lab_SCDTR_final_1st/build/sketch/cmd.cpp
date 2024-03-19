#line 1 "/home/duarte/Arduino/lab_SCDTR_final_1st/cmd.cpp"
#include "main.hpp"

extern int16_t target_lux;
extern float actual_lux;
extern float pwm;

bool occupancy_state = 0;
bool anti_windup_state = 1;
bool feedback_state = 0;
uint16_t external_illuminance = 0;

extern float energy_consumption;
extern float visibility_error;
extern float flicker;
extern uint32_t pid_freq;

bool send_lux = false;
bool send_duty_cycle = false;
int16_t buffer_duty_cycle[LOG_SAMPLE_LENGTH] = {0};
int16_t buffer_lux[LOG_SAMPLE_LENGTH] = {0};

/**
 * @brief Recive the commands from the serial port
 */
bool recv_cmd(void *)
{
  uint8_t size;
  if (size = Serial.available()) // If there is data available
  {
    uint8_t str[size + 1];
    Serial.readBytes(str, size);
    char answer[MAX_BUFFER_ANSWER_SIZE] = {0};
    cmd_handler((char *)str, size, true, answer);
    if (strcmp(answer, "Not \\n\n") == 0)
    {
      Serial.println(
          "Waiting for more data from Serial"); // Waiting for more data
    }
    else if (strchr(answer, '\n') != NULL)
    {
      Serial.print("Answer: ");
      Serial.print(answer);
    }
  }
  return true;
}

/**
 * @brief Process the command
 *
 * @param cmd The command
 * @param size The size of the command
 * @param can_flag If the command comes from the CAN bus
 * @param answer The answer
 */
void cmd_handler(char *cmd, uint8_t size, bool can_flag, char *answer)
{
  if (strchr(cmd, '\n') == NULL) // If the command does not end with '\n'
  {
    sprintf(answer, "Not \\n\n");
    return;
  }

  char *token[size];
  char cmd_copy[size]; // Because strtok modifies the string
  strcpy(cmd_copy, cmd);
  token[0] = strtok(cmd_copy, " ");
  uint8_t n;
  for (n = 1; n < size; n++)
  {
    token[n] = strtok(NULL, " ");
    if (strchr(token[n], '\n') != NULL)
    {
      token[n] = strtok(token[n], "\n");
      break;
    }
  }

  if (n != 2 || strlen(token[0]) != 1) // Only 3 tokens are allowed
  {
    sprintf(answer, "err: 2 (%d,%d)\n", n != 2, strlen(token[0]) != 1);
    return;
  }

  Serial.println("Processing command...");

  uint16_t i;
  switch (*token[0])
  {
  case 'd': // Set directlt the duty cycle of luminaire i
    i = atoi(token[1]);
    if (i == can_dev_id)
    {
      uint16_t duty_cycle;
      duty_cycle = atoi(token[2]);

      if (duty_cycle < 0 || duty_cycle > DAC_RANGE)
        sprintf(answer, "err:3\n");
      else
      {
        analogWrite(LED_PIN, constrain(duty_cycle, 0, DAC_RANGE));
        sprintf(answer, "ack\n");
      }
    }
    else if (can_flag)
      send_to_CAN_bus((uint8_t *)cmd, size, ASK);

    break;

  case 'r': // Set the illuminance reference of luminaire i
    i = atoi(token[1]);
    if (i == can_dev_id)
    {
      mutex_enter_blocking(&pid_Mutex);
      target_lux = atoi(token[2]);
      mutex_exit(&pid_Mutex);
      sprintf(answer, "ack\n");
    }
    else if (can_flag)
      send_to_CAN_bus((uint8_t *)cmd, size, ASK);

    break;
  case 'o': // Set the occupancy state of desk i
    i = atoi(token[1]);
    if (i == can_dev_id)
    {
      uint8_t val;
      val = atoi(token[2]);
      if (val < 0 || val > 1)
        sprintf(answer, "err:4\n");
      else
        occupancy_state = val;
    }
    else if (can_flag)
      send_to_CAN_bus((uint8_t *)cmd, size, ASK);
    break;

  case 's': // Start the stream of the real-time variable x of desk i
    if (strlen(token[1]) != 1)
    {
      sprintf(answer, "err:5\n");
      return;
    }

    i = atoi(token[2]);
    if (i == can_dev_id)
    {
      switch (*token[1])
      {
      case 'l': // Start the stream of the real-time variable 'l' of desk i
        send_lux = true;
        break;
      case 'd': // Start the stream of the real-time variable 'd' of desk i
        send_duty_cycle = true;
        break;
      default:
        break;
      }
    }
    else if (can_flag)
      send_to_CAN_bus((uint8_t *)cmd, size, ASK);
    break;
  case 'S': // Stop the stream of the real-time variable x of desk i
    if (strlen(token[1]) != 1)
    {
      sprintf(answer, "err:6\n");
      return;
    }
    i = atoi(token[2]);
    if (i == can_dev_id)
    {
      switch (*token[1])
      {
      case 'l': // Stop the stream of the real-time variable 'l' of desk i
        send_lux = false;
        break;
      case 'd': // Stop the stream of the real-time variable 'd' of desk i
        send_duty_cycle = false;
        break;
      default:
        break;
      }
    }
    else if (can_flag)
      send_to_CAN_bus((uint8_t *)cmd, size, ASK);
    break;

  case 'g':
    if (strlen(token[1]) != 1)
    {
      sprintf(answer, "err:7\n");
      return;
    }
    if (*token[1] != 'b')
    {
      i = atoi(token[2]);
      if (i == can_dev_id)
      {
        switch (*token[1])
        {
        case 'd': // Get current duty cycle of luminaire i
          mutex_enter_blocking(&pid_Mutex);
          sprintf(answer, "d %d %d\n", i, (int)pwm);
          mutex_exit(&pid_Mutex);
          break;
        case 'r': // Get current illuminance reference of luminaire i
          mutex_enter_blocking(&pid_Mutex);
          sprintf(answer, "r %d %d\n", i, target_lux);
          mutex_exit(&pid_Mutex);
          break;
        case 'l': // Measure the illuminance of luminaire i
          mutex_enter_blocking(&pid_Mutex);
          sprintf(answer, "l %d %d\n", i, (uint16_t)actual_lux);
          mutex_exit(&pid_Mutex);
          break;
        case 'o': // Get the current occupancy state of desk i
          sprintf(answer, "o %d %d\n", i, occupancy_state);
          break;
        case 'a': // Get anti-windup state of desk i
          sprintf(answer, "a %d %d\n", i, anti_windup_state);
          break;
        case 'k': // Get feedback state of desk i
          sprintf(answer, "k %d %d\n", i, feedback_state);
          break;
        case 'x': // Get current external illuminance of desk i
          sprintf(answer, "x %d %d\n", i, external_illuminance);
          break;
        case 'e': // Get energy consumption of desk i
          mutex_enter_blocking(&log_Mutex);
          sprintf(answer, "e %d %d\n", i, (int)energy_consumption);
          mutex_exit(&log_Mutex);
          break;
        case 'p': // Get instantaneous power consumption of desk i
          mutex_enter_blocking(&pid_Mutex);
          sprintf(answer, "p %d %d\n", i, (int)(P_MAX * pwm / DAC_RANGE));
          mutex_exit(&pid_Mutex);
          break;
        case 't': // Get the elapsed time since the last restart
          sprintf(answer, "t %d %d\n", i, ((int)millis() / 1000));
          break;
        case 'v': // Get visibility error of desk i
          mutex_enter_blocking(&log_Mutex);
          sprintf(answer, "v %d %d\n", i, (int)visibility_error);
          mutex_exit(&log_Mutex);
          break;
        case 'f': // Get flicker of desk i
          mutex_enter_blocking(&log_Mutex);
          sprintf(answer, "f %d %d\n", i, (int)flicker);
          mutex_exit(&log_Mutex);
          break;
        case 'F': // Get the frequency of the PID thread
          mutex_enter_blocking(&log_Mutex);
          sprintf(answer, "F %d %d\n", i, pid_freq);
          mutex_exit(&log_Mutex);
          break;
        default:
          break;
        }
      }
      else if (can_flag)
        send_to_CAN_bus((uint8_t *)cmd, size, ASK);
    }
    else // token[1] == 'b'
    {
      i = atoi(token[3]);
      if (i == can_dev_id)
      {
        switch (*token[2])
        {
        case 'd': // Get buffer duty cycle of all luminaires
          sprintf(answer, "g b d %d :", i);
          mutex_enter_blocking(&log_Mutex);
          for (uint16_t i = 0; i < LOG_SAMPLE_LENGTH; i++)
          {
            sprintf(answer + strlen(answer), " %d,", &buffer_duty_cycle[i]);
          }
          mutex_exit(&log_Mutex);
          sprintf(answer + strlen(answer), "\n");
          break;
        case 'l': // Get buffer illuminance of all luminaires
          sprintf(answer, "g b l %d :", i);
          mutex_enter_blocking(&log_Mutex);
          for (uint16_t i = 0; i < LOG_SAMPLE_LENGTH; i++)
          {
            sprintf(answer + strlen(answer), " %d,", &buffer_lux[i]);
          }
          mutex_exit(&log_Mutex);
          sprintf(answer + strlen(answer), "\n");
          break;
        default:
          break;
        }
      }
      else if (can_flag)
        send_to_CAN_bus((uint8_t *)cmd, size, ASK);
    }
    break;

  default:
    break;
  }

  Serial.println("Command processed");
}
