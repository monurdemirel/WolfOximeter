#include <Wire.h>
#include "MAX30100_PulseOximeter.h" 

#include <WiFi.h>               //#include <ESP8266WiFi.h> comment out if ESP8266 board is used 
#include <BlynkSimpleEsp32.h>   //#include <BlynkSimpleEsp8266.h> comment out ESP8266 board is used

#include <SPI.h>
#include <Adafruit_SSD1331.h>



PulseOximeter pox;
int BPM, SpO2; //float BPM, SpO2;
uint32_t tsLastReport = 0;
#define REPORTING_PERIOD_MS 1500
// connecting SCL PIN, SDA PIN, INT PIN (INT not wired, not needed in this project) 
// wiring       // ESP32 pin              // ESP8266 pin
// SCL_PIN      // 22, GPIO 22            // D1, GPIO 5
// SDA_PIN      // 21, GPIO 21            // D2, GPIO 4
// INT_PIN      // 23, GPIO 23            // D0, GPIO 16 //not wired


char auth[] = "***";                        //  You should get Auth Token in the Blynk App. replace *** with your Auth Token
char ssid[] = "***";                        //  Your WiFi credentials. replace *** with your wifi name
char pass[] = "***";                        //  Your WiFi credentials. replace *** with your wifi password
BlynkTimer timer;


// Option 1: use any pins but a little slower
// Option 2: use the hardware SPI pins faster and stable
                    // alternative    // ESP32 prefered pin     // ESP8266 prefered pin
#define sclk 18     // sck, clk, D0   // ESP32 HW pin: GPIO 18  // ESP8266 HW pin: D5
#define mosi 23     // sda, D1, DIN   // ESP32 HW pin: GPIO 23  // ESP8266 HW pin: D7
#define cs   5      // chip select    // ESP32 HW pin: GPIO 5   // ESP8266 HW pin: D8
#define rst  17     // reset          // GPIO 17 or tx2         // D3
#define dc   16     // A0             // GPIO 16 or rx2         // D2

Adafruit_SSD1331 oled = Adafruit_SSD1331(cs, dc, mosi, sclk, rst);

// Color definitions
#define WHITE           0xFFFF
#define RED             0xF800
#define BLACK           0x0000
#define GREEN           0x07E0
#define CYAN            0x07FF
#define YELLOW          0xFFE0
#define BLUE            0x001F

const unsigned char kurt [] PROGMEM = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 
0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 
0x40, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 
0x70, 0x03, 0x80, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 
0x3e, 0x7f, 0xfe, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 
0x3f, 0xff, 0x80, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 
0x1f, 0xfe, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 
0x0f, 0xf0, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 
0x03, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char kurtkucuk [] PROGMEM = {
0x00, 0x00, 0x04, 0x00, 0x00, 
0x00, 0x00, 0x06, 0x00, 0x00, 
0x00, 0x00, 0x07, 0x00, 0x00, 
0x00, 0x00, 0x0f, 0x00, 0x00, 
0x00, 0x07, 0xff, 0x80, 0x00, 
0x00, 0x0f, 0xff, 0x80, 0x00, 
0x1f, 0xff, 0xff, 0xc0, 0x00, 
0x1f, 0xff, 0xff, 0xe0, 0x00, 
0x0f, 0xff, 0xff, 0xe0, 0x00, 
0x07, 0xff, 0xff, 0xf0, 0x00, 
0xc0, 0x3f, 0xff, 0xf0, 0x00, 
0xfe, 0x1f, 0xff, 0xf8, 0x00, 
0x70, 0xff, 0xff, 0xfc, 0x00, 
0x01, 0xff, 0xff, 0xfc, 0x00, 
0x00, 0xff, 0xff, 0xfe, 0x00, 
0x00, 0x01, 0xff, 0xff, 0x00, 
0x00, 0x00, 0xff, 0xff, 0x00, 
0x00, 0x00, 0x7f, 0xff, 0x00, 
0x00, 0x00, 0x3f, 0xff, 0x80, 
0x00, 0x00, 0x3f, 0xff, 0xc0, 
0x00, 0x00, 0x3f, 0xff, 0xe0, 
0x00, 0x00, 0x3f, 0xff, 0xe0, 
0x00, 0x00, 0x3f, 0xff, 0xf0, 
0x00, 0x00, 0x3f, 0xff, 0xfe
};
 
 

void setup()
{

    oled.begin();
    oled.fillScreen(WHITE);
    delay(100);
    oled.drawBitmap(0, 0, kurt, 96, 64, RED);
    delay(2000);
    oled.begin();
    
    pinMode(16, OUTPUT);
    Blynk.begin(auth, ssid, pass);
    timer.setInterval(2000L, BlynkSendData);
    
//    Serial.begin(115200);     
//    Serial.print("Initializing Pulse Oximeter..");
 
    if (!pox.begin())
    {

         oled.fillScreen(BLACK);
         oled.setTextSize(1);
         oled.setTextColor(WHITE);
         oled.setCursor(0, 0);
         oled.println("FAILED");

         //         Serial.println("FAILED");
         
         for(;;);
    }
    else
    {
         oled.fillScreen(WHITE);
         oled.setTextSize(1);
         oled.setTextColor(RED);
         oled.setCursor(0, 0);
         oled.println("...");
         
//         Serial.println("SUCCESS");

    }
 
         pox.setOnBeatDetectedCallback(onBeatDetected);         
//       The default current for the IR LED is 50mA and it could be changed by uncommenting the following line.
//         pox.setIRLedCurrent(MAX30100_LED_CURR_7_24MA);
          
}


void onBeatDetected()
{

oled.drawBitmap( 50, 36, kurtkucuk, 40, 24, RED);
//    Serial.println("Beat Detected!");

}


void BlynkSendData()
{

    Blynk.virtualWrite(V3, BPM);
    Blynk.virtualWrite(V4, SpO2);
//    Serial.println("Blynk data sent");
       
}

 
void loop()
{

    Blynk.run();
    timer.run();
    pox.update();
     
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    if (millis() - tsLastReport > REPORTING_PERIOD_MS)
    {
     
        oled.fillScreen(WHITE);
        oled.setTextSize(1);
        oled.setTextColor(BLACK);
        oled.setCursor(1,17);
        oled.println(pox.getHeartRate());
 
        oled.setTextSize(1);
        oled.setTextColor(BLACK);
        oled.setCursor(1, 1);
        oled.println("Heart BPM");
 
        oled.setTextSize(1);
        oled.setTextColor(RED);
        oled.setCursor(1, 31);
        oled.println("SpO2");
 
        oled.setTextSize(1);
        oled.setTextColor(RED);
        oled.setCursor(1,46);
        oled.println(pox.getSpO2());

//        Serial.print("Heart rate:");
//        Serial.print(pox.getHeartRate());
//        Serial.print("bpm / SpO2:");
//        Serial.print(pox.getSpO2());
//        Serial.println("%");
 
        tsLastReport = millis();
    }
}