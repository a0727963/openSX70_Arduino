#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//TCS3200
#define SENSOR_S0 5
#define SENSOR_S1 4
#define SENSOR_S2 7
#define SENSOR_S3 6
#define SENSOR_OUT 8

// Stores frequency read by the photodiodes
unsigned long red = 0;
unsigned long green = 0;
unsigned long blue = 0;
unsigned long nofilter = 0;

//Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup(){
   Serial.begin(9600); //BaudRate

    //TCS3200 I/O
    pinMode(SENSOR_S0, OUTPUT);
    pinMode(SENSOR_S1, OUTPUT);
    pinMode(SENSOR_S2, OUTPUT);
    pinMode(SENSOR_S3, OUTPUT);
    pinMode(SENSOR_OUT, INPUT);
  
    // Setting frequency-scaling to 20%
    digitalWrite(SENSOR_S0, HIGH);
    digitalWrite(SENSOR_S1, LOW);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
//  display.display();
//  delay(2000); // Pause for 2 seconds

  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...

display.clearDisplay();                 // Löschen Display
display.setTextSize(1);                 // Schriftgröße 1
display.setTextColor(WHITE);
display.setCursor(20, 20);               // Cursor in der ersten Zeile auf die 10. Stelle gehen ...
display.println("TCS3200 Test");      // ... und dort Text ausgeben
display.display();                      // Die vorherigen 5 Befehle für Textausgabe verarbeiten und an das Display geben.
}

void color() {
  // Setting RED (R) filtered photodiodes to be read
  digitalWrite(SENSOR_S2,LOW);
  digitalWrite(SENSOR_S3,LOW);
  // Reading the output frequency
  red = pulseIn(SENSOR_OUT, LOW);
  delay(10);
  
  // Setting GREEN (G) filtered photodiodes to be read
  digitalWrite(SENSOR_S2,HIGH);
  digitalWrite(SENSOR_S3,HIGH);
  // Reading the output frequency
  green = pulseIn(SENSOR_OUT, LOW);
  delay(10);
 
  // Setting BLUE (B) filtered photodiodes to be read
  digitalWrite(SENSOR_S2,LOW);
  digitalWrite(SENSOR_S3,HIGH);
  // Reading the output frequency
  blue = pulseIn(SENSOR_OUT, LOW);
  delay(10);
}

void loop(){
  //TCS3200 START
  // Setting RED (R) filtered photodiodes to be read
  digitalWrite(SENSOR_S2,LOW);
  digitalWrite(SENSOR_S3,LOW);
  
  // Reading the output frequency
  red = pulseIn(SENSOR_OUT, LOW);
  
   // Printing the RED (R) value
  Serial.print("R = ");
  Serial.print(red);
  delay(100);
  
  // Setting GREEN (G) filtered photodiodes to be read
  digitalWrite(SENSOR_S2,HIGH);
  digitalWrite(SENSOR_S3,HIGH);
  
  // Reading the output frequency
  green = pulseIn(SENSOR_OUT, LOW);
  
  // Printing the GREEN (G) value  
  Serial.print(" G = ");
  Serial.print(green);
  delay(100);
 
  // Setting BLUE (B) filtered photodiodes to be read
  digitalWrite(SENSOR_S2,LOW);
  digitalWrite(SENSOR_S3,HIGH);
  
  // Reading the output frequency
  blue = pulseIn(SENSOR_OUT, LOW);
  
  // Printing the BLUE (B) value 
  Serial.print(" B = ");
  Serial.println(blue);
  delay(100);

 // Setting BLUE (B) filtered photodiodes to be read
  digitalWrite(SENSOR_S2,HIGH);
  digitalWrite(SENSOR_S3,LOW);
  
  // Reading the output frequency
  nofilter = pulseIn(SENSOR_OUT, LOW);
  
  // Printing the BLUE (B) value 
  Serial.print(" Clear = ");
  Serial.println(nofilter);
  delay(100);

  color();

  display.clearDisplay();                 // Löschen Display
  display.setTextSize(1);                 // Schriftgröße 1
  display.setTextColor(WHITE);
  display.setCursor(0, 0);               // Cursor in der ersten Zeile auf die 10. Stelle gehen ...
  display.print("green: ");
  display.println(green);
  display.print("blue: ");
  display.println(blue);
  display.print("red: ");
  display.println(red);
  display.print("clear: ");
  display.println(nofilter);
  display.display();
  delay(100);
 
  //display.clearDisplay();
  display.setTextSize(2);
  //display.setTextColor(WHITE);
  //display.setCursor(0,0);
   if(red<blue && red<green && red<40){
      display.setCursor(0,50);
      display.print("RED");
      display.display();
      delay(100);
   }
   else if(blue < red && blue < green) {
      display.setCursor(0,50);
      display.print("BLUE");
      display.display();
      delay(100);
   }
   else if (green < red && green < blue) {
      display.setCursor(0,50);
      display.print("GREEN");
      display.display();
      delay(100);
   }  
   else {
      display.setCursor(0,40);
      display.setTextSize(1);
      display.println("   NO COLOR   "); 
      display.println("   DETECTED   ");
      display.display();//////??????
      delay(500);
  }
}
