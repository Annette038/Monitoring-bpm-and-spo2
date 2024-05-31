// Include required libraries
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

// Create name for sensor
MAX30105 particleSensor;

// Define setup
#define MAX_BRIGHTNESS 255
#define BPM_DURATION 12000
#define SPO2_DURATION 30000
#define ARRAY_RATE 522

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
//Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
//To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
uint16_t irBuffer[100]; //infrared LED sensor data
uint16_t redBuffer[100];  //red LED sensor data
#else
uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data
#endif

// Variables for sensor
const byte RATE_SIZE = 5; //Increase this for more averaging.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

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

byte ledBrightness = 60; //Options: 0=Off to 255=50mA
byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green.
byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
int pulseWidth = 411; //Options: 69, 118, 215, 411
int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

// Responder MAC Address (Replace with your responders MAC Address)
uint8_t broadcastAddress[] = {0xD4, 0xF9, 0x8D, 0x04, 0x59, 0x54};

// Define data structure
typedef struct struct_message {
  int a;
  int32_t b;
  int c;
} struct_message;

// Create structured data object
struct_message myData;

// Register peer
esp_now_peer_info_t peerInfo;

// Void for calculating BPM
void beatCheckVoid(){
  //We sensed a beat!
  Serial.println( "             BEAT !!!!             ");
  long delta = millis() - lastBeat;
  lastBeat = millis();

  beatsPerMinute = 60 / (delta / 1000.0);

  if (beatsPerMinute < 255 && beatsPerMinute > 20)
  {
    rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
    rateSpot %= RATE_SIZE; //Wrap variable

    //Take average of readings
    beatAvg = 0;
    for (byte x = 0 ; x < RATE_SIZE ; x++)
      beatAvg += rates[x];
    beatAvg /= RATE_SIZE;
  }
  myData.a = beatAvg;
}

// Sent data callback function
void OnDataSent(const uint8_t *macAddr, esp_now_send_status_t status)
{
  //Serial.print("  Last Packet Send Status: ");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void borrarArray() {
  for (int i = 0; i < ARRAY_RATE; i++) {
    bpmArray[i] = 0; // Asignar cero a cada elemento del array
  }
}

void setup() {

  // Setup Serial monitor
  Serial.begin(115200);
  delay(100); 

  // Set ESP32 WiFi mode to Station temporarly
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Define callback
  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  //Start with this configuration for first 100 values for spo2
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}



void loop() {
  bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps
  
  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();

    myData.c = irBuffer[i];
    // Send data
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  myData.b = spo2;

  while(1){
    uint32_t currentTime = millis();
    particleSensor.setup(); //Configure sensor with these settings for bpm

    if (currentTime - bpmStartTime < BPM_DURATION) {
      
      // Calculate BPM
      if (checkForBeat(particleSensor.getIR()) == true) {beatCheckVoid();}
      bpmArray[indexBPM] = (byte)beatAvg; //Store this reading in the array
      indexBPM++;
      count = 0;
    } 
    else if (currentTime - spo2StartTime < SPO2_DURATION) {
      indexBPM = 0;
      particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
      // Calculate SpO2
      //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
      for (byte i = 25; i < 100; i++)
      {    
        redBuffer[i - 25] = redBuffer[i];
        irBuffer[i - 25] = irBuffer[i];
        //count++;
        //myData.a = bpmArray[count];
      }

      //take 25 sets of samples before calculating the heart rate.
      for (byte i = 75; i < 100; i++)
      {  
        count++;
        myData.a = bpmArray[count++];
        while (particleSensor.available() == false) //do we have new data?
          particleSensor.check(); //Check the sensor for new data

        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
    

        particleSensor.nextSample(); //We're finished with this sample so move to next sample   
      }
      //Send data to calculate spo2 with algorithm
      maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
      myData.b = spo2;
      myData.c = particleSensor.getIR(); //IR values
      //Send data and print
      esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      Serial.print("SPO2: "); Serial.print(myData.b); Serial.print(" BPM: "); Serial.println(myData.a); 
    
    } else {
      // Reset timers
        //borrarArray();
        bpmStartTime = currentTime;
        spo2StartTime = currentTime;
    }  
  }  
}

