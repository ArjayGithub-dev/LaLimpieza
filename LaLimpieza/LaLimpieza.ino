#define BLYNK_TEMPLATE_ID "TMPL65QoQjG_x"
#define BLYNK_TEMPLATE_NAME "Blynk Smart Trashbin"

#define BLYNK_FIRMWARE_VERSION "0.1.1"

#include "LaLimpieza.h"

#ifdef ESP8266
#include <BlynkSimpleEsp8266.h>
#elif defined(ESP32)
#include <BlynkSimpleEsp32.h>
#else
#error "Board not found"
#endif

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <ESP32Servo.h>


credentials Credentials;

//Variables
char auth_token[33];
bool connected_to_internet = 0;
const int Erasing_button = 0;


//Provide credentials for your ESP server
char* esp_ssid = "La Limpieza";
char* esp_pass = "";

#define BIN_HEIGHT 85
#define DISTANCE_DETECTION 50
#define OPEN_SERVO_VAL  120
#define CLOSE_SERVO_VAL 0
#define WAIT_TIME 3000

#define DISTANCE_TRAVELED(x) (int) (x*0.034)/2

static const uint8_t RXPin = 35, TXPin = 34;
static const uint32_t GPSBaud = 9600;
static const uint8_t SERVO_PIN = 13;
static const uint8_t BOTTOM_LID = 12;
static const uint8_t TRASH_LEVEL_SENSOR[2] = {26, 25}; // TRIG, ECHO
static const uint8_t FIRE_DETECT_SENSOR[2] = {18, 19}; // AO, DO
static const uint8_t LED[3] = {15,2,0}; // BGR

double latitude = 0;
double longitude = 0;
unsigned int prevTrashLevel = 0;
bool lidstatus = false;
bool flameDetected = false;

class UltraSonicSensor {
  public:
    ~UltraSonicSensor() {

    }
    UltraSonicSensor(uint8_t t, uint8_t e, unsigned int x_d = 150)
      : trigger_pin(t)
      , echo_pin(e)
      , max_distance(x_d) 
      { 
        pinMode(this->trigger_pin, OUTPUT);
        pinMode(this->echo_pin, INPUT);
      }

      unsigned int getDistance() {
        digitalWrite(this->trigger_pin, LOW);
        delayMicroseconds(2);
        digitalWrite(this->trigger_pin, HIGH);
        delayMicroseconds(10);
        digitalWrite(this->trigger_pin, LOW);

        unsigned long duration = pulseIn(this->echo_pin, HIGH, 10000);

        return (duration == 0 || DISTANCE_TRAVELED(duration) >= this->max_distance) 
          ? 0 
          : (DISTANCE_TRAVELED(duration));
      }

  private:
    uint8_t trigger_pin;
    uint8_t echo_pin;
    unsigned int max_distance;
};

Servo binServo;
BlynkTimer timer;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
// UltraSonicSensor eventDetection(EVENT_DETECT_SENSOR[0], EVENT_DETECT_SENSOR[1], DISTANCE_DETECTION);
// UltraSonicSensor trashLevel_1(TRASH_LEVEL_SENSOR[1][0], TRASH_LEVEL_SENSOR[1][1], BIN_HEIGHT);
UltraSonicSensor trashLevel(TRASH_LEVEL_SENSOR[0], TRASH_LEVEL_SENSOR[1], BIN_HEIGHT);

void BLYNK_WRITE_BIN_LEVEL(int arg) {
  prevTrashLevel = arg == 0 ? 0 : BIN_HEIGHT - arg;
  Serial.print("Raw data: ");
  Serial.print(arg);
  Serial.print(", Distance val: ");
  Serial.println(prevTrashLevel);
  Blynk.virtualWrite(V1, prevTrashLevel);

  if (prevTrashLevel >= 50)
    fullLedStatus();
  else if (prevTrashLevel >= 35 && prevTrashLevel < 50)
    halfLedStatus();
  else if (prevTrashLevel < 35)
    emptyLedStatus();
}

void BLYNK_FIRE_DETECT(bool fire_detected) {
  Blynk.virtualWrite(V4, fire_detected);
}

void BLYNK_OPEN_BIN_LID() {
  Blynk.virtualWrite(V0, "Open");
  delay(500);
  binServo.write(OPEN_SERVO_VAL);
}

void BLYNK_CLOSE_BIN_LID() {
  Serial.println("Lid Closing");
  Blynk.virtualWrite(V0, "Close");
  delay(500);
  binServo.write(CLOSE_SERVO_VAL);
}

void updateLocation() {
  //Serial.print(F("Location: "));
  if (gps.location.isValid()) {
    //Serial.print(gps.location.lat(), 6);
    latitude = gps.location.lat();
    //Serial.print(F(","));
    //Serial.print(gps.location.lng(), 6);
    longitude = gps.location.lng();
  } else {
    //Serial.print(F("INVALID"));
    longitude = 0;
    latitude = 0;
  }
  //Serial.println();
}

void fullLedStatus() {
  analogWrite(LED[0], 0);
  analogWrite(LED[1], 0);
  analogWrite(LED[2], 250);
}

void halfLedStatus() {
  analogWrite(LED[0], 255);
  analogWrite(LED[1], 0);
  analogWrite(LED[2], 0);
}

void emptyLedStatus() {
  analogWrite(LED[0], 0);
  analogWrite(LED[1], 255);
  analogWrite(LED[2], 0);
}

void updateBinLevel() {
  unsigned int val = trashLevel.getDistance();
  BLYNK_WRITE_BIN_LEVEL(val);
}

void updateGPS() {
  Blynk.virtualWrite(V2, longitude, latitude);   //lat
}

void setup() {
  
  Serial.begin(115200);
  pinMode(Erasing_button, INPUT);


  for (uint8_t t = 4; t > 0; t--) {
    Serial.println(t);
    delay(1000);
  }

  // Press and hold the button to erase all the credentials
  if (digitalRead(Erasing_button) == LOW)
  {
    Credentials.Erase_eeprom();

  }

  String auth_string = Credentials.EEPROM_Config();
  auth_string.toCharArray(auth_token, 33);

  if (Credentials.credentials_get())
  {

    Blynk.config(auth_token);
    connected_to_internet = 1;

  }
  else
  {
    Credentials.setupAP(esp_ssid, esp_pass);
    connected_to_internet = 0;
  }


  if (connected_to_internet) {
    ss.begin(GPSBaud);
    binServo.attach(SERVO_PIN);
    binServo.setPeriodHertz(50);
    binServo.write(CLOSE_SERVO_VAL);

    pinMode(BOTTOM_LID, INPUT_PULLUP);
    pinMode(FIRE_DETECT_SENSOR[1], INPUT);
    pinMode(LED[0], OUTPUT);
    pinMode(LED[1], OUTPUT);
    pinMode(LED[2], OUTPUT);

    timer.setInterval(100000L, updateGPS);
    timer.setInterval(2000L, updateBinLevel);

    BLYNK_FIRE_DETECT(false);
    emptyLedStatus();
    Serial.println("running System");
  }
}



void loop() {
  Credentials.server_loops();
  
  if (connected_to_internet) {
    
    if (digitalRead(FIRE_DETECT_SENSOR[1]) == LOW) {
    Serial.println("FIRE DETECTED!");
    BLYNK_FIRE_DETECT(true);
  }
  if (prevTrashLevel >= (BIN_HEIGHT-20)) {
    delay(8000);
    unsigned int temp_dist = trashLevel.getDistance(); 
    unsigned int currentLevel = temp_dist == 0 ? 0 : BIN_HEIGHT - temp_dist;
    if (currentLevel >= BIN_HEIGHT-20) {
      Serial.println("BIN almost full");
      delay(500);
      binServo.write(OPEN_SERVO_VAL);
    }
  } else {
    if (digitalRead(BOTTOM_LID) == HIGH) {
    if (!lidstatus) BLYNK_OPEN_BIN_LID();
    lidstatus = true;
    } else {
      if (lidstatus) BLYNK_CLOSE_BIN_LID();
      lidstatus = false;
    }
  }

  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      updateLocation();

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
  }
    Blynk.run();
    timer.run();
  }
}
