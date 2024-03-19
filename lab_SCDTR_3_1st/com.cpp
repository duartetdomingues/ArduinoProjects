#include "main.hpp"
#include <ACAN2515.h>

extern uint16_t adc;
extern float pwm;
extern int16_t target_lux;
extern float actual_ldr_R;
extern float actual_lux;
extern uint16_t info_sample_time;
extern float Gain_lux_to_pwm;

// PID variables
extern float error;
extern float integral;
extern float derivative;

extern float lux_error;

extern float Kp;
extern float Ki;
extern float Kd;

extern float energy_consumption;
extern float visibility_error;
extern float flicker;
extern uint32_t pid_freq;

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
    Serial.print(" PWM:");
    Serial.print(pwm);
    Serial.print(" Gain:");
    Serial.print(Gain_lux_to_pwm);
    mutex_exit(&pid_Mutex);
    
    mutex_enter_blocking(&log_Mutex);
    Serial.print(" Freq:");
    Serial.print(pid_freq);
    Serial.print(" Energy:");
    Serial.print(energy_consumption);
    Serial.print(" Visibility:");
    Serial.print(visibility_error);
    Serial.print(" Flicker:");
    Serial.print(flicker);
    mutex_exit(&log_Mutex);


    Serial.print(" ADC:");
    mutex_enter_blocking(&adc_Mutex);
    Serial.print(adc);
    mutex_exit(&adc_Mutex);

    Serial.println();

    return true;
}

// Função para definir o CAN ID com base no ID exclusivo
uint16_t set_can_id(uint8_t unique_id[6]){
    // Use o unique_id[3] para definir o CAN ID
    // Mapeie valores específicos do unique_id[3] para IDs específicos do CAN
    switch (unique_id[5]) {
        case 0x64:
            return 0x01;
        case 0x42:
            return 0x02;
        case 0x52:
            return 0x03;
        default:
            return 0; // Valor padrão para outros casos
    }
}

ACAN2515 can(PIN_CS, SPI, PIN_INT);
static const uint32_t QUARTZ_FREQUENCY = 20UL * 1000UL * 1000UL; // 20 MHz
static const uint32_t CAN_BIT_RATE = 1000L * 1000UL;             // 1 Mb/s

/*
 * @brief Send a command to the CAN bus
 *
 * @param data: Pointer to the data to be sent
 * @param len: Length of the data
 */
void send_to_CAN_bus(uint8_t *data, uint8_t len, bool id_flag)
{
    // last bit is 1 for ask and 0 for answer
    Serial.print("Sending message to CAN bus Data:");
    Serial.print((char*)data);
    uint16_t msg_id = can_dev_id << 1 | id_flag;

    for (int i = 0; i < len;)
    {
        CANMessage message;
        uint8_t j;
        for (j = 0; j < 8 && i < len; j++, i++)
        {
            message.data[j] = data[i];
        }
        message.len = j;
        message.id = msg_id;
        bool e = can.tryToSend(message);
        if (e)
        {
            Serial.printf("Message send to CAN bus:");
            Serial.printf("ID:%d DLC:%d DATA: %d %d %d %d %d %d %d %d\n", message.id, message.len, message.data[0], message.data[1], message.data[2], message.data[3], message.data[4], message.data[5], message.data[6], message.data[7]);
        }
        else
        {
            Serial.println("Error sending message");
        }
    }
}

CANMessage Buffer_CAN_msg[BUFFER_CAN_MSG_SIZE];
uint8_t Buffer_CAN_msg_index = 0;
/*
 * @brief Read a message from the CAN bus
 *
 * @param data: Pointer to the data to be sent
 * @param len: Length of the data
 */
bool read_can_msg(void *)
{
    #ifdef DEBUG
    //Serial.println("Read CAN Msg");
    #endif

    if (can.available())
    {
        //Serial.println("Message received from CAN bus");
        CANMessage message;
        can.receive(message);
        if (message.id & 1) // If the last bit is 1, then it is an ask
        {
            uint8_t len = 0;
            char str[(Buffer_CAN_msg_index+1) * 8];
            if (Buffer_CAN_msg_index > 0)
            {
                printf("Data in the buffer\n", Buffer_CAN_msg_index);
                Buffer_CAN_msg[Buffer_CAN_msg_index] = message;
                Buffer_CAN_msg_index++;
                for (uint8_t i = 0; i < Buffer_CAN_msg_index; i++)
                {
                    strcpy(str + i * 8, (char *)Buffer_CAN_msg[i].data);
                    len += Buffer_CAN_msg[i].len;
                    if (Buffer_CAN_msg[i].id != message.id)
                    {
                        Serial.println("Error in the rcv can bufferg ");
                        Buffer_CAN_msg_index = 0; // Clear the buffer
                        return true;
                    }
                }
                Buffer_CAN_msg_index = 0;
            }
            else
            {
                //Serial.printf("No data in the buffer LEN:%d DATA:", message.len);
                strcpy(str, (char *)message.data);
                Serial.println(str);
                len = message.len;
            }
            Serial.printf("Ask received from CAN bus Data: %s ", str);
            char answer[MAX_BUFFER_ANSWER_SIZE]={0};
            cmd_handler(str, len, false, answer);
            if (strcmp(answer, "Not \\n\n") == 0 )
            {
                Serial.println("Waiting for more data from CAN bus");
                Buffer_CAN_msg[Buffer_CAN_msg_index] = message;
                Buffer_CAN_msg_index++;
                return true;
            }
            else if (strchr(answer, '\n') != NULL)
            {
                send_to_CAN_bus((uint8_t *)answer, strlen(answer), ANSWER);
            }
        }
        else // If the last bit is 0, then it is an answer
        {
            //Serial.print("Answer received from CAN bus Data:");
            for (uint8_t i = 0; i < message.len; i++)
                Serial.printf("%c", message.data[i]);

        }
    }
    return true;
}

void config_can()
{
    Serial.println("Config CAN bus");

    // Initialize SPI
    SPI.setSCK(PIN_SCK);
    SPI.setTX(PIN_MOSI);
    SPI.setRX(PIN_MISO);
    SPI.setCS(PIN_CS);
    SPI.begin();
    /*  spi_init(SPI_PORT, 1000000); // 1 MHz
    gpio_set_function(PINSCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SPI_CS); */

    ACAN2515Settings settings(QUARTZ_FREQUENCY, CAN_BIT_RATE);
    settings.mRequestedMode = ACAN2515Settings::NormalMode; // Requested mode is Normal0
    uint16_t errorCode = can.begin(settings, []
                                   { can.isr(); });
    if (errorCode == 0)
    {
        Serial.println("CAN init OK");
    }
    else
    {
        Serial.print("CAN init Error: ");
        Serial.println(errorCode);
    }

    // Enable the SPI Interrupt pin
    // gpio_set_irq_enabled_with_callback(PIN_INT, GPIO_IRQ_EDGE_RISE, true, &can_rx_callback);
    // attachInterrupt(digitalPinToInterrupt(PIN_INT), can_rx_callback, FALLING);
}