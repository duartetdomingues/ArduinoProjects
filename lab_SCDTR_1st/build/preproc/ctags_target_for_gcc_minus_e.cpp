# 1 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino"
# 2 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino" 2

// Constants
const float GAIN = 2.667;
const float B = 6.152183;
const float M = -0.8;

// Sample times
uint16_t adc_sample_time = 1;
uint16_t pid_sample_time = 10;
uint16_t info_sample_time = 1000;
uint16_t rcv_cmd_sample_time = 10;

// Global variables
float Gain_lux_to_pwm;
uint16_t read_lux_max;
uint16_t read_lux_min;
uint16_t target_lux;

extern float pwm;
extern float last_ms;

auto timer = timer_create_default();

mutex_t pwm_Mutex; // Mutex for PWM
mutex_t adc_Mutex; // Mutex for ADC
mutex_t pid_Mutex; // Mutex for PID VAR

pthread_t thread;

RunningMedian median(pid_sample_time / adc_sample_time);

void calibre_lux()
{
    Serial.begin(115200);
    analogWrite(15, 4096); // Turn on the LED
    delay(1000); // Wait for the LDR to stabilize
    int read_adc = analogRead(A0); // Read Max LDR voltage
    float res = ldr_R(read_adc); // Convert to resistance
    read_lux_max = convert_to_lux(res); // Convert to LUX
    Serial.print("LUX(MAX):");
    Serial.println(read_lux_max);

    analogWrite(15, 0); // Turn off the LED
    delay(1000); // Wait for the LDR to stabilize
    read_adc = analogRead(A0); // Read Min LDR voltage
    res = ldr_R(read_adc); // Convert to resistance
    read_lux_min = convert_to_lux(res); // Convert to LUX
    Serial.print("LUX(0):");
    Serial.println(read_lux_min);

    Gain_lux_to_pwm = (float)(4096) / (read_lux_max - read_lux_min); // Calculate the gain for the LDR
    Serial.print("GAIN:");
    Serial.println(Gain_lux_to_pwm);
}

bool flag_init = true;

void setup()
{ // the setup function runs once

    pinMode(15, OUTPUT); // Set the LED pin as an output
    analogReadResolution(12); // default is 10
    analogWriteFreq(60000); // 60KHz, about max
    analogWriteRange(4096); // 100% duty cycle
    calibre_lux(); // Calibrate the LDR

    mutex_init(&adc_Mutex); // Initialize the mutex
    mutex_init(&pwm_Mutex); // Initialize the mutex
    mutex_init(&pid_Mutex); // Initialize the mutex

    target_lux = read_lux_min; // Set the target LUX to half of the max LUX

    pwm = get_duty_cycle(target_lux);
    analogWrite(15, ((pwm)<(0)?(0):((pwm)>(4096)?(4096):(pwm)))); // Set the initial duty cycle

    Serial.printf("First pwm %f\n", pwm);

    delay(10); // Wait for the LDR to stabilize
    adc_init();

    // Shedule tasks
    timer.every(adc_sample_time, read_adc); // read_adc() every adc_sample_time
    timer.every(info_sample_time, send_info); // loop() every info_sample_time
    timer.every(rcv_cmd_sample_time, recv_cmd); // read_adc() every adc_sample_time

    flag_init = false; // Set the flag to false to run pid thread
}

void adc_init()
{
    for (uint8_t i = 0; i < 5; i++)
    {
        read_adc(
# 94 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino" 3 4
                __null
# 94 "/home/duarte/Arduino/lab_SCDTR_1st/lab_SCDTR_1st.ino"
                    );
    }
}

void setup1()
{
    while (flag_init)
    {
        delay(1);
    }
    Serial.println("PID Thread");
    last_ms = (float)micros() / 1000;
}

// PWM=LUX^G  PWM=[0,4096]]
/**
 * @brief Convert the LUX to duty cycle
 */
float get_duty_cycle(int LUX)
{
    return LUX * Gain_lux_to_pwm;
}

/**
 * @brief Convert the LDR voltage to resistance
 */
float ldr_R(uint16_t read_adc)
{
    float voltage = (float)read_adc * 3300 / 4096;
    float resistence = (10000 * 3300) / voltage - 10000;
    return resistence;
}

/**
 * @brief Convert the LDR voltage to resistance
 */
float convert_to_adc(float res)
{
    float voltage = (3300 * 10000) / (res + 10000);
    return (voltage * 4096) / 3300;
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

int j = 1;
bool flag = true;
extern float Ki;
void loop()
{
    timer.tick();
    if (micros() > j * 5e6)
    {
        j++;
        mutex_enter_blocking(&pid_Mutex);
        /* if (pwm > 100 )
            flag = false;
        else if (pwm < 100)
            flag = true; */

        if (target_lux > 2*read_lux_max)
            flag = false;

        if (target_lux < 0)
            flag = true;

        if (flag)
            target_lux += 1;
        else
            target_lux -= 1;

        mutex_exit(&pid_Mutex);
        mutex_enter_blocking(&pwm_Mutex);
        pwm = get_duty_cycle(target_lux);
        mutex_exit(&pwm_Mutex);
        median.clear();
        adc_init();
    }
}

void loop1()
{
    pid();
}
