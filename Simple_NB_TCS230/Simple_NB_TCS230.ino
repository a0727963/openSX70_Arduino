#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// TCS230 sensor reading example
//
// This is just to show basic functionality without calibration.
// Utilises the non-blocking version of the reading function.
// Output uses the Serial console.
//
#include <MD_TCS230.h>
#include <FreqCount.h>

// Pin definitions
#define  S0_OUT 3
#define  S1_OUT 9
#define  S2_OUT  10
#define  S3_OUT  11
#define  OE_OUT   8    // LOW = ENABLED 


//MD_TCS230  CS(S2_OUT, S3_OUT, OE_OUT);
MD_TCS230  CS(S0_OUT, S1_OUT, S2_OUT, S3_OUT, OE_OUT);

void setup() 
{
  Serial.begin(57600);
  Serial.println("[TCS230 Simple NON_BLOCKING Example]");
  Serial.println("\nMove the sensor to different color to see the RGB value");
  Serial.println("Note: These values are being read in without sensor calibration");
  Serial.println("and are likely to be far from reality");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();                 // Löschen Display
  display.setTextSize(1);                 // Schriftgröße 1
  display.setTextColor(WHITE);
  display.setCursor(20, 20);               // Cursor in der ersten Zeile auf die 10. Stelle gehen ...
  display.println("TCS3200 Test");      // ... und dort Text ausgeben
  display.display(); 

  //TCS230_FREQ_HI 100%
  //TCS230_FREQ_MID 20%
  //TCS230_FREQ_LOW 2%
  //TCS230_FREQ_OFF
  CS.setFrequency(TCS230_FREQ_HI);
  CS.begin();
   
}

void readSensor()
{
  static  bool  waiting;
  
  if (!waiting)
  {
    CS.read();
    waiting = true;
  }
  else
  {
    if (CS.available()) 
    {
      colorData  rgb;
      
      CS.getRGB(&rgb);
      Serial.print("RGB [");
      Serial.print(rgb.value[TCS230_RGB_R]);
      Serial.print(",");
      Serial.print(rgb.value[TCS230_RGB_G]);
      Serial.print(",");
      Serial.print(rgb.value[TCS230_RGB_B]);
      Serial.println("]");

      display.clearDisplay();                 // Löschen Display
      display.setTextSize(1);                 // Schriftgröße 1
      display.setTextColor(WHITE);
      display.setCursor(0, 0);               // Cursor in der ersten Zeile auf die 10. Stelle gehen ...
      display.print("green: ");
      display.println(rgb.value[TCS230_RGB_G]);
      display.print("blue: ");
      display.println(rgb.value[TCS230_RGB_B]);
      display.print("red: ");
      display.println(rgb.value[TCS230_RGB_R]);
      display.print("clear: ");
      display.println(rgb.value[TCS230_RGB_X]);
      display.display();
      //delay(100);
      
      waiting = false;
    }
  }
}


void loop() 
{
  readSensor();
}
