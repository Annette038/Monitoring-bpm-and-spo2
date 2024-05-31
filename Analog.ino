// Include required libraries
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

// Define pins for LEDs and analog input
#define RED_LED_PIN 19
#define IR_LED_PIN 18
#define ANALOG_INPUT_PIN 14 
// Durations for loops
#define BPM_DURATION 12000
#define SPO2_DURATION 30000
#define ARRAY_RATE 522

// Variables
const byte RATE_SIZE = 5; // Increase this for more averaging
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;
int ValueAnalog;
int Espo2;
int spo2New;

byte bpmArray[ARRAY_RATE]; //Array of heart rates for sending
int indexBPM;
unsigned int count = 0;

uint32_t bpmStartTime = 0;
uint32_t spo2StartTime = 0;

int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid


// Arrays for storing sensor data
uint32_t irBuffer[100]; // Infrared LED sensor data
uint32_t redBuffer[100]; // Red LED sensor data

// Function for calculating BPM
void beatCheckVoid() {
  // We sensed a beat!
  Serial.println("             BEAT !!!!             ");
  long delta = millis() - lastBeat;
  lastBeat = millis();

  beatsPerMinute = 60 / (delta / 1000.0);

  if (beatsPerMinute < 255 && beatsPerMinute > 20) {
    rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
    rateSpot %= RATE_SIZE; // Wrap variable

    // Take average of readings
    beatAvg = 0;
    for (byte x = 0; x < RATE_SIZE; x++) {
      beatAvg += rates[x];
    }
    beatAvg /= RATE_SIZE;
  }
}

void setup() {
  // Setup Serial monitor
  Serial.begin(115200);
  delay(100);

  // Initialize LED pins
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(IR_LED_PIN, OUTPUT);
}

void loop() {
  // To check inputs
  //digitalWrite(IR_LED_PIN, HIGH);
  //Serial.println(analogRead(ANALOG_INPUT_PIN));
  //delay(100);
  //digitalWrite(IR_LED_PIN, HIGH);
  //digitalWrite(RED_LED_PIN, HIGH);
  //Serial.println(analogRead(ANALOG_INPUT_PIN));
  //delay(100);

  //digitalWrite(IR_LED_PIN, LOW); 

  // CODE START
  bufferLength = 100; // Buffer length of 100 stores 4 seconds of samples running at 25sps 
  // Read the first 100 samples, and determine the signal range
  for (byte i = 0; i < bufferLength; i++) {
    digitalWrite(RED_LED_PIN, HIGH);
    delay(40); // Ensure LED is on for a bit
    redBuffer[i] = analogRead(ANALOG_INPUT_PIN);
    digitalWrite(RED_LED_PIN, LOW);

    digitalWrite(IR_LED_PIN, HIGH);
    delay(40); // Ensure LED is on for a bit
    irBuffer[i] = analogRead(ANALOG_INPUT_PIN);
    digitalWrite(IR_LED_PIN, LOW);

    //delay(10); // Delay to match sampling rate
  }
  // Calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &Espo2, &validSPO2, &heartRate, &validHeartRate);
  
  // Continuously taking samples
  while (1) {
    uint32_t currentTime = millis();
    // BPM Section
    if (currentTime - bpmStartTime < BPM_DURATION) {
      digitalWrite(IR_LED_PIN, HIGH);
      int ir = analogRead(ANALOG_INPUT_PIN);
      if (ir > 1600) { beatCheckVoid(); }
      //bpmArray[indexBPM] = (byte)beatAvg; //Store this reading in the array
      Serial.print(" BPM= ");
      Serial.println(beatAvg);
      delay(200);
      
      indexBPM++;
      count = 0; 
    }
    // SPO2 Section
    else if (currentTime - spo2StartTime < SPO2_DURATION) {
      indexBPM = 0;
      // Dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
      for (byte i = 25; i < 100; i++) {
        redBuffer[i - 25] = redBuffer[i];
        irBuffer[i - 25] = irBuffer[i];
      }

      // Take 25 sets of samples before calculating the heart rate.
      for (byte i = 75; i < 100; i++) {
        digitalWrite(RED_LED_PIN, HIGH);
        delay(40); // Ensure LED is on for a bit
        redBuffer[i] = analogRead(ANALOG_INPUT_PIN);
        digitalWrite(RED_LED_PIN, LOW);

        digitalWrite(IR_LED_PIN, HIGH);
        delay(40); // Ensure LED is on for a bit
        irBuffer[i] = analogRead(ANALOG_INPUT_PIN);
        digitalWrite(IR_LED_PIN, LOW);
        
        //Serial.print(" red= ");
        //Serial.print(redBuffer[i]);
        //Serial.print(" ir= ");
        //Serial.println(irBuffer[i]);
      }

        Serial.print(" BPM= ");
        Serial.print(beatAvg);

        Serial.print(F(" SPO2= "));
        Serial.println(spo2, DEC);

        //delay(10); // Delay if need to match sampling rate
        maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &Espo2, &validSPO2, &heartRate, &validHeartRate);
        
        // Calibration based on test
        if(Espo2>=86){
          spo2New = 100;
        }else{
          spo2New = 0.8255*Espo2 + 28.849;
        }

        if ((spo2- spo2New) > 15) {
        }else{
        spo2 = spo2New;
        }
    }
    
    else {
        // Reset timers
        //borrarArray();
        bpmStartTime = currentTime;
        spo2StartTime = currentTime;
    }
    
  } 
}
