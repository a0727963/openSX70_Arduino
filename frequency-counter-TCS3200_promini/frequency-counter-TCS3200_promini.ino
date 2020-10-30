#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Timer and Counter example
// Author: Nick Gammon
// Date: 17th January 2012

// Input: Pin D5

//Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//TCS3200 openSX70 sonar PCB

const int S0_Pin = 3;
//const int S1_Pin = 2;                   //
const int S1_Pin = 9; //on sonar                   
const int S2_Pin = A0;
const int S3_Pin = 6;
//const int OE_Pin = 9;



// these are checked for in the main program
volatile unsigned long timerCounts;
volatile boolean counterReady;

// internal to counting routine
unsigned long overflowCount;
unsigned int timerTicks;
unsigned int timerPeriod;

void startCounting (unsigned int ms) 
  {
  counterReady = false;         // time not up yet
  timerPeriod = ms;             // how many 1 ms counts to do
  timerTicks = 0;               // reset interrupt counter
  overflowCount = 0;            // no overflows yet

  // reset Timer 1 and Timer 2
  TCCR1A = 0;             
  TCCR1B = 0;              
  TCCR2A = 0;
  TCCR2B = 0;

  // Timer 1 - counts events on pin D5
  TIMSK1 = bit (TOIE1);   // interrupt on Timer 1 overflow

  // Timer 2 - gives us our 1 ms counting interval
  // 16 MHz clock (62.5 ns per tick) - prescaled by 128
  //  counter increments every 8 µs. 
  // So we count 125 of them, giving exactly 1000 µs (1 ms)
  TCCR2A = bit (WGM21) ;   // CTC mode
  OCR2A  = 124;            // count up to 125  (zero relative!!!!)

  // Timer 2 - interrupt on match (ie. e    very 1 ms)
  TIMSK2 = bit (OCIE2A);   // enable Timer2 Interrupt

  TCNT1 = 0;      // Both counters to zero
  TCNT2 = 0;     

  // Reset prescalers
  GTCCR = bit (PSRASY);        // reset prescaler now
  // start Timer 2
  TCCR2B =  bit (CS20) | bit (CS22) ;  // prescaler of 128
  // start Timer 1
  // External clock source on T1 pin (D5). Clock on rising edge.
  TCCR1B =  bit (CS10) | bit (CS11) | bit (CS12);
  }  // end of startCounting

ISR (TIMER1_OVF_vect)
  {
  ++overflowCount;               // count number of Counter1 overflows  
  }  // end of TIMER1_OVF_vect


//******************************************************************
//  Timer2 Interrupt Service is invoked by hardware Timer 2 every 1 ms = 1000 Hz
//  16Mhz / 128 / 125 = 1000 Hz

ISR (TIMER2_COMPA_vect) 
  {
  // grab counter value before it changes any more
  unsigned int timer1CounterValue;
  timer1CounterValue = TCNT1;  // see datasheet, page 117 (accessing 16-bit registers)
  unsigned long overflowCopy = overflowCount;

  // see if we have reached timing period
  if (++timerTicks < timerPeriod) 
    return;  // not yet

  // if just missed an overflow
  if ((TIFR1 & bit (TOV1)) && timer1CounterValue < 256)
    overflowCopy++;

  // end of gate time, measurement ready

  TCCR1A = 0;    // stop timer 1
  TCCR1B = 0;    

  TCCR2A = 0;    // stop timer 2
  TCCR2B = 0;    

  TIMSK1 = 0;    // disable Timer1 Interrupt
  TIMSK2 = 0;    // disable Timer2 Interrupt
    
  // calculate total count
  timerCounts = (overflowCopy << 16) + timer1CounterValue;  // each overflow is 65536 more
  counterReady = true;              // set global flag for end count period
  }  // end of TIMER2_COMPA_vect

void setup () 
  {
  Serial.begin(9600);       
  Serial.println("Frequency Counter");
  pinMode(S0_Pin, OUTPUT);  
  pinMode(S1_Pin, OUTPUT);    
  pinMode(S2_Pin, OUTPUT);  
  pinMode(S3_Pin, OUTPUT);    
  //pinMode(OE_Pin, OUTPUT);    

  //S2 & S0 should be high can be modified via jumper in PCB 
  digitalWrite(S0_Pin, HIGH); 
  digitalWrite(S1_Pin, HIGH); //scaling LOW = 20% HIGH = 100%
  digitalWrite(S2_Pin, HIGH); 
  digitalWrite(S3_Pin, HIGH); //filter LOW = clear HIGH = green
//  digitalWrite(OE_Pin, LOW); //LOW = enabled
  // S0 Low | S1 Low = Skalierunga us
  // S0 Low | S1 High = 2%
  // S0 H   | S1 L    = 20%
  // S0 H   | S1 H    = 100%

  //S2 | S3 | Filter
  //L  | L  | Red
  //L  | H  | Blue
  //H  | L  | White
  //H  | H  | Green
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }
  display.clearDisplay();                 // Löschen Display
  display.setTextSize(1);                 // Schriftgröße 1
  display.setTextColor(WHITE);
  display.setCursor(20, 20);               // Cursor in der ersten Zeile auf die 10. Stelle gehen ...
  display.println("TCS3200 Frequencytest");      // ... und dort Text ausgeben
  display.display();                      // Die vorherigen 5 Befehle für Textausgabe verarbeiten und an das Display geben.

  } // end of setup

void loop () 
  {
  // stop Timer 0 interrupts from throwing the count out
  byte oldTCCR0A = TCCR0A;
  byte oldTCCR0B = TCCR0B;
  TCCR0A = 0;    // stop timer 0
  TCCR0B = 0;    
  
  startCounting (1000);  // how many ms to count for

  while (!counterReady) 
     { }  // loop until count over

  // adjust counts by counting interval to give frequency in Hz
  //float frq = (timerCounts *  1000.0) / timerPeriod;
  float frq = (timerCounts *  1.0) / timerPeriod;

  //Serial.print ("Frequency: ");
  //Serial.print ((unsigned long) frq);
  //Serial.print (frq);
  //Serial.println (" kHz.");

  display.clearDisplay();                 // Löschen Display
  display.setTextSize(2);                 // Schriftgröße 1
  display.setTextColor(WHITE);
  display.setCursor(0, 0);               // Cursor in der ersten Zeile auf die 10. Stelle gehen ...
  display.println("Frequency: ");
  display.print(frq);
  display.println("Hz");
  display.display();
  //delay(100);
  
  // restart timer 0
  TCCR0A = oldTCCR0A;
  TCCR0B = oldTCCR0B;
  
  // let serial stuff finish
  //delay(200);
  }   // end of loop
