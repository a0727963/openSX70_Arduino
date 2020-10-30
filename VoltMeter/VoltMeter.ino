#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


int analogInput = 1;
float Vout = 0.00;
float Vin = 0.00;
float R1 = 10000.00; // resistance of R1 (10K) 
float R2 = 1000.00; // resistance of R2 (1K) 
int val = 0;
void setup(){
   pinMode(analogInput, INPUT); //assigning the input port
   Serial.begin(9600); //BaudRate

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
}

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
  // display.display() is NOT necessary after every single drawing command,
  // unless that's what you want...rather, you can batch up a bunch of
  // drawing operations and then update the screen all at once by calling
  // display.display(). These examples demonstrate both approaches...

display.clearDisplay();                 // Löschen Display
display.setTextSize(1);                 // Schriftgröße 1
display.setCursor(0, 10);               // Cursor in der ersten Zeile auf die 10. Stelle gehen ...
display.println("Voltmeter");      // ... und dort Text ausgeben
display.display();                      // Die vorherigen 5 Befehle für Textausgabe verarbeiten und an das Display geben.
}

void loop(){
   
   val = analogRead(analogInput);//reads the analog input
   Vout = (val * 5.00) / 1024.00; // formula for calculating voltage out i.e. V+, here 5.00
   Vin = Vout / (R2/(R1+R2)); // formula for calculating voltage in i.e. GND
   if (Vin<0.09)//condition 
   {
     Vin=0.00;//statement to quash undesired reading !
  } 
//Serial.print("\t Voltage of the given source = ");
//Serial.println(Vin);
delay(100); //for maintaining the speed of the output in serial moniter

  display.clearDisplay(); // löscht vor jedem Schreiben das Display da ansonsten alles nur überschrieben würde und man nichts mehr lesen könnte
  display.setTextSize(3); // legt die Schriftgöße fest 
  display.setTextColor(WHITE);//schaltet so zu sagen  die Farbe der Schrift ein
  display.setCursor(20,20); ///hier werden dem Cursor bzw. dem Text die x und y Position zugewiesn
  display.print(Vin);
  display.println("V");  ////LANG LEBE DER "C64" !!!!!!
  display.display();//////??????
}
