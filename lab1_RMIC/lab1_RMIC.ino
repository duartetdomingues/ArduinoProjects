#include <NewPing.h> // Library for Ultrasonic Sensor
#include "pitch.h"   // Library for pitch constants
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"



#define API_KEY "AIzaSyCv5ENhaH9l1SJ3ELNLkE6iUsabJ3c8CR0"
#define DATABASE_URL "https://iot-alarm-app-94123-default-rtdb.europe-west1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

#define WIFI_SSID "Labs-LSD"
#define WIFI_PASSWORD "aulaslsd"


#define ALERT_DISTANCE 20 // in centimeters

// Define the notes and durations for the Nokia tune
const int nokiaMelody[] = {
  NOTE_E5, NOTE_D5, NOTE_FS4, NOTE_D5, NOTE_FS4, NOTE_A4, NOTE_C5, NOTE_B4,
  NOTE_E4, NOTE_GS4, NOTE_E4, NOTE_GS4, NOTE_A4, NOTE_C5, NOTE_B4, NOTE_E4,
  NOTE_E5, NOTE_D5, NOTE_FS4, NOTE_D5, NOTE_FS4, NOTE_A4, NOTE_C5, NOTE_B4,
  NOTE_E4, NOTE_GS4, NOTE_E4, NOTE_GS4, NOTE_A4, NOTE_C5, NOTE_B4, NOTE_E4
};

const int nokiaNoteDurations[] = {
  8, 8, 16, 16, 16, 16, 8, 4,
  8, 16, 16, 16, 16, 8, 4, 8,
  8, 8, 16, 16, 16, 16, 8, 4,
  8, 16, 16, 16, 16, 8, 4, 8
};

bool enable_alert_sound = true; // Set to false if you don't want the buzzer to sound
bool alarm_status = false;
int measurement = 0;

//#define TEST



// for NodeMCU 1.0

#define ECHO_PIN D1
#define TRIGGER_PIN D0
#define BUZZER_PIN D2
#define LED_PIN D6
#endif


NewPing sonar(TRIGGER_PIN, ECHO_PIN,400);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signup = false;

void setup() {

  Serial.begin(115200);
  /* pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT); */
  pinMode(ECHO_PIN, INPUT);
  pinMode (TRIGGER_PIN,OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print("Connecting..\n");
    delay(300);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("Sing up");
    signup = true;
  }
  else{
    Serial.printf("Sing up Error");
  }

  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true); 

  if (Firebase.ready() && signup){
    if (Firebase.RTDB.getInt(&fbdo, "measurement")) {
      Serial.print("Read var:");
      int measurement =fbdo.intData();
      Serial.println(measurement);
      if (Firebase.RTDB.setInt(&fbdo, "measurement", measurement)){
        Serial.print("Set var to : ");
        Serial.println(measurement);
      }
      else{
        Serial.println("ERROR to set var");
      }

    }
    else{
      Serial.println("ERROR to set var");
    }
  }

}

int get_distance(){

   long duration, distance; // Variables to store duration and distance

  digitalWrite(TRIGGER_PIN, LOW); // Ensure trigger pin is low
  delayMicroseconds(2); // Wait for stabilization
  digitalWrite(TRIGGER_PIN, HIGH); // Send a 10us pulse to trigger pin
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH); // Read the pulse from echo pin
  distance = duration * 0.034 / 2; // Convert pulse duration to distance (cm)

  return distance;

}

void loop() {

  //get_distance();

  

  
  //int distance = get_distance(); // Read distance from sensor in cm

  long median = 0;
  int distance = 0;

  while(distance==0){
    // median = sonar.ping_median(5); // Read distance from sensor in cm
    // distance = sonar.convert_cm(median);
    distance = get_distance(); 
    Serial.print(".");
  }

  Serial.print("Distance:");
  Serial.print(distance);
  Serial.println(" cm");
  Firebase.RTDB.setInt(&fbdo, "measurement", distance);

 
  if (Firebase.RTDB.getInt(&fbdo, "alarm_status")){

    Serial.print("alarm_status:"); 
    Serial.println( fbdo.boolData()); 
    if (distance<ALERT_DISTANCE && fbdo.boolData()) {

      Firebase.RTDB.setInt(&fbdo, "enable_alert_sound", true);
      Serial.println("Set true enable_alert_sound"); 
      // Sound the buzzer if enabled and motion detected
      for (int i = 0; i < sizeof(nokiaMelody) / sizeof(nokiaMelody[0]); i++) {
        // Calculate the duration of the note
        int duration = 1000 / nokiaNoteDurations[i];
      
        // Play the tone for the current note
        tone(BUZZER_PIN, nokiaMelody[i]);
        
        // Delay for the duration of the note
        delay(duration * 1.5); // Adjust this multiplier if needed for better playback
      }
    } 
    noTone(BUZZER_PIN);
  }
 /*  if (distance<ALERT_DISTANCE) {
    for (int i = 0; i < sizeof(nokiaMelody) / sizeof(nokiaMelody[0]); i++) {
    // Calculate the duration of the note
    int duration = 1000 / nokiaNoteDurations[i];
    
    // Play the tone for the current note
    tone(BUZZER_PIN, nokiaMelody[i], duration);
    
    // Delay for the duration of the note
    delay(duration * 1); // Adjust this multiplier if needed for better playback
    }
  } */

  delay(500); // Wait for 1 second before the end of loop()
}
