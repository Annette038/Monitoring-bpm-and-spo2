/*
  ESP-NOW Remote Sensor - Receiver
  Receives Spo2 & Bpm data from other ESP32 via ESP-NOW and displays in OLED
*/

// Include required libraries
#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_GFX.h>    //OLED libraries
#include <Adafruit_SSD1306.h> //OLED libraries
#include <Adafruit_NeoPixel.h> //Leds library

//Sleep Mode settings
#define DEFAULT_WAKEUP_PIN    D1
#define DEFAULT_WAKEUP_LEVEL  ESP_GPIO_WAKEUP_GPIO_HIGH

//OLED settings
#define SCREEN_WIDTH 128 //OLED width
#define SCREEN_HEIGHT 64 //OLED height
#define OLED_RESET    -1 //Reset pin

//PIN settings
#define PIN_LED D10 //LED
#define NUM_LEDS 16 //Number of LEDs
const int buzzerPin = D8; // Buzzer
const int buttonPin = D1; // Button

//Variable definition
int RecvVal;
int lastTime;
int sensor;
int timeCalibrate;
int buttonState = 0;
unsigned long startTime = 0; 
int buttonPressCount = 0;
unsigned long lastButtonPressTime = 0;
unsigned long currentTime;
int spo2 ;
int bpm;

//Sleep mode configuration
const gpio_config_t config = {
    .pin_bit_mask = BIT(DEFAULT_WAKEUP_PIN),
    .mode         = GPIO_MODE_INPUT,
};

// Define data structure
typedef struct struct_message {
  int a;
  int32_t b;
  int c;
} struct_message;

// Create structured data object
struct_message myData;
struct_message newData;

//Names and settings for OLED and LEDs
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Declaring the display name (display)
Adafruit_NeoPixel tiraled = Adafruit_NeoPixel(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800); //Declaring function of LEDs

// Heartbeat small chart
static const unsigned char PROGMEM logo2_bmp[] =
{ 0x03, 0xC0, 0xF0, 0x06, 0x71, 0x8C, 0x0C, 0x1B, 0x06, 0x18, 0x0E, 0x02, 0x10, 0x0C, 0x03, 0x10,        
  0x04, 0x01, 0x10, 0x04, 0x01, 0x10, 0x40, 0x01, 0x10, 0x40, 0x01, 0x10, 0xC0, 0x03, 0x08, 0x88,
  0x02, 0x08, 0xB8, 0x04, 0xFF, 0x37, 0x08, 0x01, 0x30, 0x18, 0x01, 0x90, 0x30, 0x00, 0xC0, 0x60,
  0x00, 0x60, 0xC0, 0x00, 0x31, 0x80, 0x00, 0x1B, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x04, 0x00,
};
// Oxigen diagram
static const unsigned char PROGMEM O2_bmp[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x3f, 0xc3, 0xf8, 0x00, 0xff, 0xf3, 0xfc,
  0x03, 0xff, 0xff, 0xfe, 0x07, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0xff, 0xfe, 0x0f, 0xff, 0xff, 0x7e,
  0x1f, 0x80, 0xff, 0xfc, 0x1f, 0x00, 0x7f, 0xb8, 0x3e, 0x3e, 0x3f, 0xb0, 0x3e, 0x3f, 0x3f, 0xc0,
  0x3e, 0x3f, 0x1f, 0xc0, 0x3e, 0x3f, 0x1f, 0xc0, 0x3e, 0x3f, 0x1f, 0xc0, 0x3e, 0x3e, 0x2f, 0xc0,
  0x3e, 0x3f, 0x0f, 0x80, 0x1f, 0x1c, 0x2f, 0x80, 0x1f, 0x80, 0xcf, 0x80, 0x1f, 0xe3, 0x9f, 0x00,
  0x0f, 0xff, 0x3f, 0x00, 0x07, 0xfe, 0xfe, 0x00, 0x0b, 0xfe, 0x0c, 0x00, 0x1d, 0xff, 0xf8, 0x00,
  0x1e, 0xff, 0xe0, 0x00, 0x1f, 0xff, 0x00, 0x00, 0x1f, 0xf0, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00,
  0x0f, 0xe0, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Led Function
void colorFull (uint32_t color) {
  for (int i= 0; i< tiraled.numPixels (); i++){
    tiraled.setPixelColor(i, color);
  }
  tiraled.show();
}

// Warning Levels Function
void warning(){
  
  for (int i = 0; i < 100; i++) {
    digitalWrite(buzzerPin, HIGH); // Set the pin high (turn on buzzer)
    delayMicroseconds(500); // Delay for half cycle (adjust for desired frequency)
    digitalWrite(buzzerPin, LOW); // Set the pin low (turn off buzzer)
    delayMicroseconds(500); // Delay for half cycle (adjust for desired frequency)
  }
  
  delay(500);

  display.clearDisplay();//Clear the screen
  display.setTextSize(2);//Set text size
  display.setTextColor(WHITE);//Text color
  display.setCursor(5,5);//Set the cursor position
  display.println("Warning");
  display.print("spo: "); display.println(spo2);//Display heartbeat value
  display.display();//Display screen
  colorFull(tiraled.Color(255,0,0)); //Turn led on in red color

  delay(1000);
}

// Callback function
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) { 
  memcpy(&newData, incomingData, sizeof(newData)); // Get incoming data
  Serial.print("BPM: "); Serial.print (newData.a); Serial.print("  "); Serial.println(bpm);
  Serial.print("SPO: "); Serial.print (newData.b); Serial.print("  "); Serial.println(spo2);
  RecvVal = 1;
}

void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
  tiraled.begin();
  tiraled.show();
  pinMode(buzzerPin, OUTPUT); //Buzzer
  pinMode(buttonPin, INPUT); // Button
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Start the OLED display
  display.display();   
  // How to what up from sleep mode
  ESP_ERROR_CHECK(gpio_config(&config));
  ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(BIT(DEFAULT_WAKEUP_PIN), DEFAULT_WAKEUP_LEVEL)); 
  ESP_LOGI("TAG", "Enabling GPIO wakeup on pins GPIO%d\n", DEFAULT_WAKEUP_PIN);


  // Start ESP32 in Station mode
  WiFi.mode(WIFI_STA);
  // Initalize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    colorFull(tiraled.Color(0,0,155)); //Turn led on in red color
    return;
  }
   
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);
}


void loop() {
  //LEDs off
  colorFull(tiraled.Color(0,0,0));
  // Disconection
  if(millis()-lastTime>15000){
    Serial.println("DESCONEXIÓN");
    display.clearDisplay();//Clear the screen
    display.setTextSize(2);//Set text size
    display.setTextColor(WHITE);//Text color
    display.setCursor(5,5);//Set the cursor position
    display.println("SIN");
    display.println("CONEXION");
    display.display();//Display screen
    colorFull(tiraled.Color(0,0,155));//Turn led on in blue color
    if (RecvVal==1){
      lastTime = millis();
      Serial.println("Data arrived");
    }
    RecvVal=0;

    // If button pressed 3 times, enter sleep mode
    currentTime = millis();
    if (digitalRead(buttonPin) == HIGH) { startTime = currentTime;}
    // Check if the button has been pressed within a certain time window
    if (currentTime - startTime <= 4000) {
      if (currentTime - lastButtonPressTime >= 200 && digitalRead(buttonPin) == HIGH) {
        buttonPressCount++;
        lastButtonPressTime = currentTime;
      }
    } else {
      buttonPressCount = 0; // Reset count if time window exceeded
    }
    // Sleep if buttonPressCount reaches 3
    if (buttonPressCount == 3 && digitalRead(buttonPin) == LOW) {
      // Turn everything off
      Serial.print("Goning to sleep");
      colorFull(tiraled.Color(0,0,0));
      digitalWrite(buzzerPin, LOW); // Set the pin low (turn off buzzer)
      display.clearDisplay(); // clear display
      display.display(); 

      delay(400);
      esp_deep_sleep_start();
    }
  }
  else{ //Not disconnected
    if (RecvVal==1){
      lastTime = millis();
      Serial.println("Data arrived");
    }
    RecvVal=0;
    // Sensor incorrectly positioned
    if (newData.c < 10000){
      colorFull(tiraled.Color(100,100,100)); //Turn led on in white color
    }
    if (newData.b < 0){
      sensor = 1;
      timeCalibrate = millis();
    }
    // Give 15 seconds to stabilize without sending alarm due to low values  
    if (millis()-timeCalibrate>15000){
      sensor = 0;
    }
    
    // Removing outliers by cheking the difference with the previous value
      // BPM
    if ((bpm - newData.a) > 20) {
      Serial.println("Difference greater than 20, ignoring BPM");
    } else {
      bpm = newData.a;
    }
      // SPO
    if ((spo2 - newData.b) > 10) {
      Serial.println("Difference greater than 15, ignoring SPO ");
      sensor = 1;
    } else {
      spo2 = newData.b;
    }
    
    // Print to Serial Monitor
    Serial.print("AvgBPM= ");
    Serial.print(bpm);
    
    Serial.print(",  SPO2= ");
    Serial.print(spo2); 

    Serial.print("           ");
    Serial.println(millis());

    //Check for warning and display in OLED
    if (spo2 <90 && sensor!= 1) {
      warning();
    }
    else{
      display.clearDisplay();
      display.display();
      colorFull(tiraled.Color(0,0,0)); // turn led off
      digitalWrite(buzzerPin, LOW); 
    }
    
    currentTime = millis();

    // Check if the button is pressed and turn on OLED if it is
    if (digitalRead(buttonPin) == HIGH) {
      display.clearDisplay();//Clear the screen
      display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);//Display a small heartbeat icon
      display.setTextSize(2);//Set text size
      display.setTextColor(WHITE);//Text color
      display.setCursor(42, 10);//Set the cursor position
      if (bpm > 30) display.print(String(bpm) + " BPM");//Display heartbeat value
      else display.print("---- BPM" );
      display.drawBitmap(0, 35, O2_bmp, 32, 32, WHITE);//Display oxygen icon
      display.setCursor(42, 40);//Set the cursor position
      display.print(String(spo2) + "%"); //Display spo2 value
      display.display();//Display screen
      startTime = currentTime;
    } else {
      display.clearDisplay(); // clear display
      display.display(); 
    }

    // Check if the button has been pressed within a certain time window
    if (currentTime - startTime <= 4000) {
      if (currentTime - lastButtonPressTime >= 200 && digitalRead(buttonPin) == HIGH) {
        buttonPressCount++;
        lastButtonPressTime = currentTime;
      }
    } else {
      buttonPressCount = 0; // Reset count if time window exceeded
    }

    // Sleep if buttonPressCount reaches 3
    if (buttonPressCount == 3 && digitalRead(buttonPin) == LOW) {
      Serial.print("Goning to sleep");
      colorFull(tiraled.Color(0,0,0));
      digitalWrite(buzzerPin, LOW); // Set the pin low (turn off buzzer)
      display.clearDisplay(); // clear display
      display.display(); 

      delay(400);
      esp_deep_sleep_start();
    }

  }

}

