#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MHZ19.h>                    //https://github.com/strange-v/MHZ19
#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN   D5                // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 12                // No. of LEDs
int  CO2;                           
int TEMP;
uint16_t pos = 0;
int tail = 3;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long intervalCO2 = 5000;        //Measurement Interval

/* Recommended Settings for   
 * - Warn-Limit with Yellow Signal
 *      Normal:   1000
 *      Covid-19: 800
 *  
 * - Alert-Limit with Red Signal
 *      Normal:   1500
 *      Covid-19: 1200
*/

int ALERT = 1200;
int WARN = 800;

SoftwareSerial CO2Serial (D6, D7);    // RX, TX Pins Setup for Wemos Mini Pro to keep the HW I2C free for display and other sensors
MHZ19 mhz(&CO2Serial);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); 

void setup(){            
  
  CO2Serial.begin(9600);              //Softserial
   
  u8g2.begin();                       //Display init
  u8g2.clear();
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  
  pixels.begin();                     // WS2812B RGB init        
  pixels.show();
  Serial.begin(9600);                 // Start Serial
  Serial.println("");   
  delay(1000);
  Serial.println("Preheating... ~3min");
  preheat();                          // Preheating sequence 
  Serial.println("Start measurement"); 
}

void loop(){

currentMillis = millis();
MHZ19_RESULT response = mhz.retrieveData();

      if (response == MHZ19_RESULT_OK and currentMillis - previousMillis >= intervalCO2){
          previousMillis = currentMillis;
          CO2 = mhz.getCO2();
          TEMP = mhz.getTemperature();
          oled(); 
      Serial.print(CO2);
      Serial.println(" PPM");
      Serial.print(TEMP);
      Serial.println("°C");
        }

  if (CO2 >= ALERT){               // Alert - Red 

  for(uint16_t i=0; i<pixels.numPixels(); i++){
    pixels.setPixelColor(i, 0,0,0);
  }
    
  for(uint16_t i=0; i<tail; i++){
    int j = (pos + i) % pixels.numPixels();
    pixels.setPixelColor(j, 255,0,0);
  }

  pixels.show();
  delay(25); 
  pos = (pos + 1) % pixels.numPixels();  
}

 
  else if (CO2 >= WARN){          // Warning - Yellow

  for(uint16_t i=0; i<pixels.numPixels(); i++){
    pixels.setPixelColor(i, 0,0,0);
  }
    
  for(uint16_t i=0; i<tail; i++){
    int j = (pos + i) % pixels.numPixels();
    pixels.setPixelColor(j, 105,45,0);
  }

  pixels.show();
  delay(50); 
  pos = (pos + 1) % pixels.numPixels();  
  }


  else {                          // Normal - Green
    for(int i=0;i<pixels.numPixels();i++){
    pixels.clear();
    pixels.setPixelColor(1,0,25,0);
    pixels.setPixelColor(4,0,25,0);
    pixels.setPixelColor(7,0,25,0);
    pixels.setPixelColor(10,0,25,0);
    pixels.show();
    delay(100);
    }
  }    
}

void preheat() {                            // 180 sec preheating 
  for(int i=0; i<pixels.numPixels(); i++) { 
    pixels.setPixelColor(i,0,0,5);         
    pixels.show();                          
    
    int bargraph =((12-i)*122/12);     
      
     u8g2.clearBuffer();
     u8g2.drawFrame(0, 0, 128, 16);
     u8g2.drawBox(3, 3, bargraph, 10);

     u8g2.setCursor(0, 20);
     u8g2.setFont(u8g2_font_profont12_mf);
     u8g2.print("Ready in");
     u8g2.setCursor(92, 20);
     if (180-15*i < 100){
     u8g2.print(" ");}
     u8g2.print(180-15*i);
     u8g2.print("sec");
     
     u8g2.sendBuffer();

    delay(15000);                                                 
    }
    pixels.clear();
}

void oled() {          

     int bargraph = ((CO2-400)*122/(WARN-400)); 
     if (bargraph >= 122){
        bargraph = 122;
      }

     u8g2.clearBuffer();
     u8g2.drawFrame(0, 0, 128, 16);
     u8g2.drawBox(3, 3, bargraph, 10);

     u8g2.setCursor(0, 20);
     u8g2.setFont(u8g2_font_profont12_mf);
      u8g2.print("CO2: ");
     u8g2.setCursor(45, 20);
     u8g2.setFont(u8g2_font_profont22_mf);
      if (CO2 <1000){
      u8g2.print(" ");}
      u8g2.print(CO2);
      u8g2.print("ppm");

     u8g2.setCursor(0, 43);
     u8g2.setFont(u8g2_font_profont12_mf);
      u8g2.print("Temp: ");
     u8g2.setCursor(45, 43);
     u8g2.setFont(u8g2_font_profont22_mf);
      u8g2.print("  ");
      u8g2.print(TEMP);
      u8g2.print("C");

    u8g2.sendBuffer();
}
