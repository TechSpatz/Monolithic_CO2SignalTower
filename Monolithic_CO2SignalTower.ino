#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MHZ19.h>                    //https://github.com/strange-v/MHZ19
#include <Adafruit_NeoPixel.h>

#define PIXEL_PIN   D5                // Digital IO pin connected to the NeoPixels.
#define PIXEL_COUNT 12                // No. of LEDs
int CO2 = 400;
int CO2buffer = 400;
bool state = false;                           
int TEMP;
uint16_t pos = 0;
int tail = 3;
int bargraph;
int countdown;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
const long intervalCO2 = 5000;        //Measurement Interval
const long intervalPre = 15000;      //Preheat Interval

/* Recommended Settings for   
 * - Warn-Limit with Yellow Signal
 *      Normal:   1000
 *      Covid-19: 800
 *  
 * - Alert-Limit with Red Signal
 *      Normal:   1500
 *      Covid-19: 1200
*/
int WARN = 800;
int ALERT = 1200;

SoftwareSerial CO2Serial (D6, D7);    // RX, TX Pins Setup for Wemos Mini Pro to keep the HW I2C free for display and other sensors
MHZ19 mhz(&CO2Serial);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); 

void setup(){            
  
  CO2Serial.begin(9600);              //Softserial
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 
   
  u8g2.begin();                       //Display init
  u8g2.clear();
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  u8g2.setContrast(50);               //Setting Brightness of the OLED Display
  
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

      if (response == MHZ19_RESULT_OK && currentMillis - previousMillis >= intervalCO2){
          previousMillis = currentMillis;
          CO2 = mhz.getCO2();
          TEMP = mhz.getTemperature();
          digitalWrite(LED_BUILTIN, LOW);
          delay(50);
          digitalWrite(LED_BUILTIN, HIGH); 
          if (state == false){
            CO2buffer = CO2;
            state = true;
          }
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
  delay(20);
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
  delay(40); 
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
  int i=0;
  unsigned long startcount = millis();
  do{
    
       currentMillis = millis();
   if (currentMillis - previousMillis >= intervalPre){
      previousMillis = currentMillis;                                              
      i++;     
    }

     countdown = ((180000-millis()+startcount)/1000);                     
     bargraph =((180-countdown)*122/180);          //OLED Stuff
      
     u8g2.clearBuffer();
     u8g2.drawFrame(0, 0, 128, 16);
     u8g2.drawBox(3, 3, bargraph, 10);

     u8g2.setCursor(0, 20);
     u8g2.setFont(u8g2_font_profont15_tf);
     u8g2.println("Preheating... ");
     u8g2.setCursor(0, 40);
     u8g2.print("Ready in ");
     if (countdown < 100){
     u8g2.print(" ");}
     if (countdown < 10){
     u8g2.print(" ");}
     u8g2.print(countdown);
     u8g2.print("sec");

      pixels.setPixelColor(i,0,0,5);         
      pixels.show(); 
      u8g2.sendBuffer();
      delay(50);
  }while (i<pixels.numPixels() && countdown>0);
  pixels.clear();
  u8g2.clear();
}

void oled() {                           

do{
  u8g2.clearBuffer();
  if (CO2 > CO2buffer){
    CO2buffer++;
    u8g2.drawTriangle(119,20, 112,28, 126,28);  
  }
  else if (CO2 < CO2buffer){
    CO2buffer--;
    u8g2.drawTriangle(119,28, 112,20, 126,20);
  }

        bargraph = ((CO2buffer-400)*122/(WARN-400)); 
     if (bargraph >= 122){
        bargraph = 122;
      }

     u8g2.drawFrame(0, 0, 128, 16);
     u8g2.drawBox(3, 3, bargraph, 10);
     
     u8g2.setCursor(0, 20);
     u8g2.setFont(u8g2_font_logisoso42_tf);
      if (CO2 <1000){
      u8g2.print(" ");}

      u8g2.print(CO2buffer);

      u8g2.drawLine(110, 31, 128, 31);
     u8g2.setCursor(110, 34);
     u8g2.setFont(u8g2_font_profont12_tf);
      u8g2.print("CO2");

     u8g2.drawLine(110, 47, 128, 47); 
     u8g2.setCursor(110, 48);
      u8g2.print("ppm");
    u8g2.drawLine(110, 63, 128, 63);
    u8g2.sendBuffer();
    delay(30);

}while(CO2buffer != CO2); 
}
