//Display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//END DISPLAY

int eingang= A0; //Das Wort „eingang“ steht jetzt für den Wert „A0“ (Bezeichnung vom Analogport 0)
int LED = 10; //Das Wort „LED“ steht jetzt für den Wert 10
int sensorWert = 0; //Variable für den Sensorwert mit 0 als Startwert

void setup()//Hier beginnt das Setup.
{
Serial.begin(115200); //Die Kommunikation mit dem seriellen Port wird gestartet. Das benötigt man, um sich den tatsächlich ausgelesenen Wert später im serial monitor anzeigen zu lassen.
pinMode (LED, OUTPUT); //Der Pin mit der LED (Pin 10) ist jetzt ein Ausgang
//Der analoge Pin muss nicht definiert werden.

//Display
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
}

void loop()
{//Mit dieser Klammer wird der Loop-Teil geöffnet.
sensorWert =analogRead(eingang); //Die Spannung an dem Fotowiderstand auslesen und unter der Variable „sensorWert“ abspeichern.
//Serial.print("Sensorwert = " ); //Ausgabe am Serial-Monitor: Das Wort „Sensorwert: „
Serial.println(sensorWert); //Ausgabe am Serial-Monitor. Mit dem Befehl Serial.print wird der Sensorwert des Fotowiderstandes in Form einer Zahl zwischen 0 und 1023 an den serial monitor gesendet.

display.clearDisplay();                 // Löschen Display
display.setTextSize(2);                 // Schriftgröße 1
display.setTextColor(WHITE);
display.setCursor(0, 0);               // Cursor in der ersten Zeile auf die 10. Stelle gehen ...
display.print("LM: ");      // ... und dort Text ausgeben
display.println(sensorWert);
display.display();


unsigned long startLT = 0;
unsigned long stopLT = 0;
int EXP = 0;
int treshhold = 20;
if (sensorWert > treshhold ) //Wenn der Sensorwert über 512 beträgt….
{
//digitalWrite(LED, HIGH); //…soll die LED leuchten…
//display.clearDisplay();                 // Löschen Display
display.setTextSize(2);                 // Schriftgröße 1
display.setTextColor(WHITE);
display.setCursor(20, 30);               // Cursor in der ersten Zeile auf die 10. Stelle gehen ...
display.print("--LED--");      // ... und dort Text ausgeben
display.display();
startLT = micros();
EXP=1;
}

while(sensorWert>treshhold) //andernfalls…
{
sensorWert =analogRead(eingang);
//digitalWrite(LED, LOW); //….soll sie nicht leuchten.
}
if(EXP==1){
  stopLT = micros();
  EXP=0;
  display.clearDisplay();
  display.setTextSize(2);                 // Schriftgröße 1
  display.setTextColor(WHITE);
  display.setCursor(0, 20);               // Cursor in der ersten Zeile auf die 10. Stelle gehen ...
  display.println("EXP Time");      // ... und dort Text ausgeben
  display.println(stopLT-startLT);
  display.display();
  delay(5000);
}


//delay (50);//Eine kurze Pause, in der die LED an oder aus ist

}//Mit dieser letzten Klammer wird der Loop-Teil geschlossen.

//Wenn nun der Sensorwert bei normaler Helligkeit bspw. nur den Wert 100 hat 
//(Der Wert ist abhängig von den verwendeten Widerständen, von der 
//Helligkeit und von der Stromrichtung), dann nimmt man anstelle des Wertes 
//512 einen wesentlich kleineren Wert, bei dem die LED zu leuchten beginnen
//soll. Bspw. nimmt man dann den Wert 90. Den aktuellen Sensorwert kann 
//man sich nun mit Hilfe des „Serial monitor“ anzeigen lassen. Dazu klickt man 
//oben auf „Tools“ und anschließend auf „serial monitor“. 
