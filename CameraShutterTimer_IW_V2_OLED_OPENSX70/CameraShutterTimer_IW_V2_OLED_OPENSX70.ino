// Arduino Shutter Timer inspired by CAMERADACTYL Cameras this extended and revised version of the code created by Ian Wallace (Instagram & Twitter: @ian_onahillroad) june 2020
// Original inspired by CAMERADACTYL hardware construction video  https://youtu.be/UwOh3da_Y8s
// This new version July 2020 by Ian Wallace (IG: @ian_onahillroad) provides some statistical information adds an ////lcd switch, data logging and a laser saftey switch.
// This version full described on my YouTube https://youtu.be/clALye887X4
// This source available from my github https://github.com/ianonahillroad/CameraShutterTimer
// Memory on the Aurdino is very short. Most strings moved to Flash (PROGMEM) ie. F("xyz")

#define BH1750S 0
#if BH1750S
#include <Arduino.h>
//BH1750 HP
#include <hp_BH1750.h>  //inlude the library
hp_BH1750 BH1750;       //create the sensor object
float lightIntensity = 0;
#endif

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
#define PIN_RESET 11
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
// Define proper RST_PIN if required.
#define RST_PIN -1
SSD1306AsciiAvrI2c oled;

const byte Version = 2;               // This application version
#define SDREADER 0
// Includes of Libraies
#include <EEPROM.h>
// SD Card - Micro SD Storage Board TF Card Memory Shield Module SPI For Arduino
#if SDREADER
  #include <SD.h>
#endif
// ////lcd - With I2C 2004 20X4 Character ////lcd Module Display
//#include <LiquidCrystal_I2C.h>
//const int ////lcdWIDTH = 20;

//LiquidCrystal_I2C ////lcd(0x27, ////lcdWIDTH, 4); // I2C address 0x27, 20 column and 4 rows

// Configuration Parameters
// There are no "incode" constants all configuration is available here and the code is more legible

//PINS
const byte LASERSENSORPIN = 2;          // Number of the digital pin to read the sensor interrups on
const byte LASERPOWERONPIN = 3;         // Number of the digital pin to read laser power ON interrups on
//const byte SDCARD_MOSI_PIN = 11;        // SD Card digital pin MOSI
//const byte SDCARD_MISO_PIN = 12;        // SD Card digital pin MISO
//const byte SDCARD_CLK_PIN = 13;         // SD Card digital pin CLK
//const byte SDCARD_CS_PIN = 4;           // SD Card digital pin CS

// CONFIG

const int  MAXEXPOSURES = 50;       // max size test exposure history tracked in the program.  Could be increased but uses memory -  40 is a practical limit when the lcds and data logger are present.
// WARNING!!! Increasing this value may not give you a compiler memory warning but can cause an program instability which will be seen as a subtly corrupted lcd diaplay! if you suspect this reduce to 10 and retest.
// if you have a different Aurdrino model with more memory than my nano you may be able to increase this.

const byte LONGESTEXPOSURESECS = 6;   // We will Normally igrnore any times that appear to be over LONGESTEXPOSURESECS seconds as setup the result of setup activity - you need to change this to check extra long exposures
const float SHOWASSECONDSOVER = 0.70;  // display shutter speeds over this number of seconds as a number of seconds speed not a fractional speed. This is arbitrary but work well at 0.7
const byte NFORSHOWSTATS = 3;          // Number of exposures required as a minimum before calculating and showing stats
const long EPROMTESTRUNNUMBERADDRESS = 0; // This is the address in the Ardiuno eprom where we store the next test run number.  We use 4 bytes at this address. Note the EPROM is really only designed for 100k updates cycles. - you might want to alter this if you use your arduino for other programs that also use the eprom
const long ARDUINOSERIALSPEED = 115200;   // this is the speed in the Arduino Monitor window. It can be 9600 but if you autowind the camera the slow output window may not keep up. hence the suggested 115200 baud
const byte LASERSWITCHBOUNCEDELAY = 50;   // remove bounce on laser on switch time in ms - switch tends to bounce when turned on.
const byte SHORTESTSENSIBLEEXP = 100;      // ignore any exposure that see, to be less than this number of microsoeconds

// SHARED DATA Declarations
//=========================

// General Shared Variables persisting until RESET
// ===============================================
//
// I had a nice 2 dimension array here but there just isn't enough memory so even though the code is less eleigant we have to have two arrays to save memory.
// Last time I has to worry about this little RAM was on a PDP 11 ! (You might need to look that up! (-: )
long ExposureHistory[MAXEXPOSURES];  //microsecs of exposure
byte IndexOfSpeed[MAXEXPOSURES];     //index into CheckTheseSpeeds matched to the exposure

// best match will be against this speed list you can just add more number in the list if needed these are fraction denominators ie. 4 is a 1/4 and 0.25 is 4 sec
//float CheckTheseSpeeds[] = {0.25, 0.333, 0.5, 1, 2, 3, 4, 5, 8, 10, 15, 25, 30, 45, 60, 125, 250, 300, 500, 750, 1000, 2000, 3000, 4000, 5000};
//float CheckTheseSpeeds[] = { 16, 20, 23, 25, 30, 35, 55, 55, 166, 302, 600, 1100};
float CheckTheseSpeeds[] = {25, 29, 32, 34, 39, 44, 54, 87, 175, 311, 609, 1109};

const int LONGEXPOSUREINDEX = -1; // dummy index used when and index in the array above is not found for a test exposure

int CountOfExposures = 0;         //number of exposures measured this session = since reset is 0 based ie. exposure -1 as used as array index

// Volatile declarations as accessed shared by interrupt routine
//==============================================================
volatile long Start = 0; // this is the time in microseconds that the shutter opens (the arduino runs a microsecond clock in the background always - it is reasonably accurate for this purpose)
volatile long Stop = 0;  // this is the time in microseconds that the shutter closes
volatile bool ShutterFired = false; // Communication from Interrupt that the shutter just fired.
volatile bool LaserON = false; // Communication from Interrupt that the Laser is ON or OFF (false).
volatile bool TimerLock = false; // Mutex to protect timer values from further interrupts during exposure recording.
volatile long LaserONOFFBounceTime =  millis(); // used to stop bounce in laser on off switch

// SETUP Routine
// =============

void setup() {      //This part of the program is run exactly once on boot
  Serial.begin(ARDUINOSERIALSPEED);                                                 //opens a serial connection.
  Serial.println(F("Shutter Timer Programme Setting up.." ));                          // Let the user know we are off.
  
  #if RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
  #else // RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
  #endif // RST_PIN >= 0
  // Call oled.setI2cClock(frequency) to change from the default frequency.
  oled.setFont(Adafruit5x7);
  uint32_t m = micros();
  oled.clear();
  oled.set2X();
  oled.println("openSX70");
  oled.set1X();
  oled.println("Shutterspeed Tester");
  oled.println();
  oled.print("\nmicros: ");
  oled.print(micros() - m);
  pinMode(PIN_RESET, INPUT_PULLUP);

  //BH1750 START
  //Wire.begin();
  //myBH1750.init(); // sets default values: mode = CHM, measuring time factor = 1.0
  //myBH1750.setMode(OTH);  // uncomment if you want to change default values
  //myBH1750.setMeasuringTimeFactor(0.45); // uncomment for selection of value between 0.45 and 3.68
  //BH1750 END

  #if BH1750S
  //BH1750HP
  bool avail = BH1750.begin(BH1750_TO_GROUND);   // will be false no sensor found // use BH1750_TO_GROUND or BH1750_TO_VCC depending how you wired the address pin of the sensor.                                                 
  // BH1750.calibrateTiming();  //uncomment this line if you want to speed up your sensor
  BH1750.start();     
  //
  #endif

  long testrunnumber;
  // update the test series run unique number
  // This is in EPROM and persists between restarts so that every test session will have a unique reference.
  EEPROM.get(EPROMTESTRUNNUMBERADDRESS, testrunnumber);
  testrunnumber++;
  EEPROM.put(EPROMTESTRUNNUMBERADDRESS, testrunnumber);
  Serial.print(F("Starting Test run "));
  Serial.println ( String(testrunnumber));

  #if SDREADER
  // Start Data Logger on SD
  Serial.println(F("Start SD Data Log File"));
  if (!SD.begin(SDCARD_CS_PIN)) {
    Serial.println(F("SD Card not present or initialization failed!"));
  } else {
    // can put debug startup test code for SD here
  }
  #endif

  // Start ////lcd Display
  //Serial.println(F("Start lcd Display"));
  //initialize the lcd
  //lcd.begin();
  //lcd.backlight(); //open the backlight MAY have no effect depends on board
  //lcd.clear();
  //lcd.print(F("Ready for run "));
  //lcd.print(String(testrunnumber));

  // check and initialise the beam state
  pinMode(LASERPOWERONPIN, INPUT);
  LaserPowerDetector();
  if (!LaserON) {
    // Warn the user the beam is off
    //////lcd.setCursor(0, 1);
    Serial.print(F("LASER OFF"));
  };

  // start listening to the Aurdrino interrupts
  attachInterrupt(digitalPinToInterrupt(LASERPOWERONPIN), LaserPowerDetector, CHANGE);    //run the function LaserPowerDetector, every time the voltage on pin changes.
  attachInterrupt(digitalPinToInterrupt(LASERSENSORPIN), ShutterChangeDetector, CHANGE);    //run the function ShutterChangeDetector, every time the voltage on pin changes.

  Serial.print(F("Shutter Timer Programme Started (V"));
  Serial.println ((String)Version + ")");
  Serial.println();
}

// Function Utility Routines
// =========================
void ExpCounterResetCHK(){
 if(CountOfExposures){
  if(!digitalRead(PIN_RESET)){
    Serial.println("Exp Count Reset");
    CountOfExposures = 0;
    oled.clear();
    oled.set2X();
    oled.println("ExpCounter");
    oled.println("Reset");
    oled.set1X();
  }
 }
}

void DebugPrintExposureHistory() {
  // used for debug only - shows used contents of internal exposure history array
  int j;
  do {
    Serial.print(j);
    Serial.print(F(" Match " )) ;
    Serial.print(IndexOfSpeed[j]);
    Serial.print(F(" Microsec " )) ;
    Serial.println(ExposureHistory[j]);
    j++;
  } 
  while (ExposureHistory[j] != 0);
}

int GetIndexOfNearestTime(long truespeed) {
  // Supporting utility function
  // Finds the best matching shutter speed in the checkthespeeds array and returns its index so that this can be used generally in the stats and check process
  // Access shared Array Variable CheckTheseSpeeds
  // PARAM truespeed    is the measured speed in **microsconds**

  long closetomicros;
  long errormicros;
  long besterror = (LONGESTEXPOSURESECS * 1000000) + 1; // starts bigger than longest exposure we measure
  int bestmatch = LONGEXPOSUREINDEX;
  int i;

  // check all the speeds against the recorded speed and pick the best one.  This loop could perhaps be made smarter.
  for (i = 0; i < (sizeof(CheckTheseSpeeds) / sizeof(int)) - 1; i++) {
    closetomicros = 1000000 / CheckTheseSpeeds[i];  //convert closeto to microseconds
    errormicros = abs(truespeed - closetomicros); //lose sign of error
    if  (errormicros < besterror) {  // best so far?
      besterror = errormicros; // update best error
      bestmatch = i; // update match index
    }
  }
  // for debug only
  Serial.print(" Match " ) ;
  Serial.println(bestmatch);
  return bestmatch;
}

String CompareToSpeed(float closeto, long truespeed) {
  // Supporting utility function - builds a formatted string
  // used to show error in shutter speed - format very brief due ////lcd width
  // PARAM closeto      is the denominator of the common speed to check eg.  2  for half a sec 125 for 1/125 ( can be fractional ie 0.25 for 4 sec)
  // PARAM truespeed    is the measured speed in **microsconds**
  long closetomicros;
  long errormicrosecs;
  String sCompared = "";
  float errpercentage = 0;
  // sanity check
  if (truespeed > 0 && closeto > 0) {
    closetomicros = 1000000 / closeto;  //convert closeto to microseconds
    errormicrosecs = truespeed - closetomicros;
    // prepare a comparison
    sCompared = " e";   //Error
    // display a + for clarity (- happens anyway)
    if ((errormicrosecs) > 0) {
      sCompared = sCompared + "+";
    };
    // care required mostly with ////lcd as space limited to 20 chars - more decimals on shorter times less than 10ms
    if (abs(errormicrosecs) < 10000) {
      sCompared = sCompared + String(((float) errormicrosecs / 1000.0), 2);
    } else {
      sCompared = sCompared + (String)(int)((float)errormicrosecs / 1000.0);      //String( x,0) seems highly unreliable in the Arduino - Avoid
    };
    sCompared = sCompared + "ms " ;
    if (errormicrosecs == 0) {
      sCompared = sCompared + "Perfect!";
    } else {
      errpercentage = (float)errormicrosecs * 100.0 / (float)closetomicros;
      sCompared = sCompared +  String(errpercentage, 1) + "%";
    }
  } else {
    // bad params
    sCompared = F("Bad CloseToSpeed Params: " );
    sCompared = sCompared + (String) closeto + " : " + (String) truespeed;
  }
  return sCompared;
}

// MAIN Code Loop
// ==============

void loop() {
  // this main loop runs continiously looking to see if the ISR flagged that the shutter just fired.
  // most user outputs are presented in miliseconds ms (ms seem esier to understand than microseconds!)
  long testrunnumber;
  int bestmatchindex;
  String sLogLine;
  ExpCounterResetCHK();
  if (ShutterFired) {
    #if BH1750S
    if (BH1750.hasValue() == true) {    // non blocking reading
        lightIntensity = BH1750.getLux();
        Serial.println(lightIntensity);
        BH1750.start();
    }
    #endif
    //  noInterrupts(); // block new spurious clock inerrupts while we deal with this - we dont want our clock times messed with!
    TimerLock = true;   // Timer lock protects existing values from further interrupts during recording. we can't use nointerrupts with the ////lcd or it wont display properly
    if (CountOfExposures < MAXEXPOSURES) {
      //   // we were recording - show the raw data - uncomment if you must see this - useful for debugging
      //Serial.println();
      //Serial.print("SHUTTER FIRED!! Timer Start: ");
      //Serial.print(Start);
      //Serial.print("  Timer Stop: ");
      //Serial.println(Stop);
      //Serial.println();

      long Speed = (Stop - Start);             // make a variable called speed, which is the total number of microseconds that the shutter is open for
      float Speedinsecs = (float)Speed / 1000000.0;  // make a variable Speedinsecs, which is how many fractioanl seconds that the shutter open for
      // if over max time or a stupidly show start up time forget it
      if ((Speedinsecs <= LONGESTEXPOSURESECS) && (Speed > SHORTESTSENSIBLEEXP) ) {
        bestmatchindex = GetIndexOfNearestTime(Speed);
        // log the exposure to memory
        IndexOfSpeed[CountOfExposures] = bestmatchindex;
        ExposureHistory[CountOfExposures] = Speed;
        EEPROM.get(EPROMTESTRUNNUMBERADDRESS, testrunnumber);
        // Open up the Log file
        #if SDREADER
        File fLogfile;
        #endif
        String sFilename = "ST_" + String(testrunnumber) + F(".txt");
        #if SDREADER
        fLogfile = SD.open(sFilename, FILE_WRITE);
        // if (!fLogfile) { Serial.println("Logfile " + sFilename + F(" Open Failed!"));};  // warn if we thought we failed to open it.
        //////lcd.clear();
        //////lcd.setCursor(0, 0);
        oled.clear();
        sLogLine = "";
        // approximate anything over SHOWASSECONDSOVER typically 700ms to be assumed to be a 1sec Plus exposure attemt and less as a fractional shutter speed
        //
        // debug only
        //Serial.print("Speedinsecs");
        //Serial.println(Speedinsecs);
        #endif
        if (Speedinsecs > SHOWASSECONDSOVER)  {
          // longer exposures shown in seconds
          
          sLogLine = (String)(CountOfExposures + 1) + ":" + String(Speedinsecs, 2) + "Sec"; // "1: 1 Sec"
          Serial.println(sLogLine);
          oled.clear();
          oled.set2X();
            oled.println(sLogLine);
          oled.set1X();
          #if SDREADER
          fLogfile.println(sLogLine);
          #endif

          sLogLine = (String)(int)(1.0 / (float) CheckTheseSpeeds[bestmatchindex]) + "Sec ";  //String( x,0) seems highly unreliable in the Arduino - Avoid
          sLogLine = sLogLine + CompareToSpeed(CheckTheseSpeeds[bestmatchindex], Speed); //ERR Percentage
          
          #if BH1750S
          sLogLine = sLogLine + (String) "\n" + (lightIntensity) + "Lux"; //BH1750 Lux read
          #endif

          Serial.print (sLogLine);
          oled.println(sLogLine);
          #if SDREADER
          fLogfile.print(sLogLine);
          #endif
          //oled.clear();
        }else
        {  // something fractional
          //Display Meassure Index
          sLogLine =  (String)(CountOfExposures + 1) + ":";
          
          //display total miliseconds of shutter interval .. more decimals for short times
          if (Speed < 10000) {
            sLogLine = sLogLine + String(((float)Speed / 1000.0), 2) + "ms ";
          } else {
            sLogLine = sLogLine + (String)(int)((float)Speed / 1000.0) + "ms ";      //String( x,0) seems highly unreliable in the Arduino - Avoid
          };
          oled.clear();
          oled.set2X();
          oled.println(sLogLine);
          oled.set1X();
          
          sLogLine = "1/" + String((float)1 / Speedinsecs, 1) + "ms";            //display the actual shutter speed. inverse of the Speedinsecs, or 1/ the shutter speed
          
          #if BH1750S
          sLogLine = sLogLine + (String) "\n" + (lightIntensity) + "Lux";
          #endif
          
          Serial.println (sLogLine);
          oled.println(sLogLine);
          #if SDREADER
          fLogfile.println(sLogLine);
          #endif
          
          // display assumed speed and accuracy
          sLogLine = (String)(int)CheckTheseSpeeds[bestmatchindex]; //String( x,0) seems highly unreliable in the Arduino - Avoid
          sLogLine.trim(); //note compiler restriction in use of syntax
          sLogLine = " 1/" + sLogLine + CompareToSpeed( CheckTheseSpeeds[bestmatchindex], Speed);
          Serial.print (sLogLine);
          oled.println(sLogLine);
          #if SDREADER
          fLogfile.print(sLogLine);
          #endif
          //oled.clear();
          //oled.println("");
        }
        Serial.println();
        oled.println("-----------------------------------");
        #if SDREADER
        fLogfile.println();
        #endif
        //oled.clear();
        // start calc of  stats
        float expsum = 0;
        float expmean = 0;
        float sumsquarevar = 0; // sum of the squared variations from the mean
        int k = 0 ;
        int n = 0; // we should know this but just in case of a discrepancy I'll count it with the data
        // sum - thus us a bit wastefull to do if we might not use it but I scrappped the array that kept counts to save memory
        do {
          if (IndexOfSpeed[k] == bestmatchindex) {
            expsum = expsum + ExposureHistory[k];
            n++;
          }
          k++;
        } 
        while (ExposureHistory[k] != 0);
        // show stats?
        if (n >= NFORSHOWSTATS) {
          //Average
          sLogLine = (F("Av "));
          expmean = expsum / (float)n / 1000.0;
          if (expmean < 100) {
            sLogLine = sLogLine + String(expmean, 1) + CompareToSpeed(CheckTheseSpeeds[bestmatchindex], expmean * 1000);
          } else {
            sLogLine = sLogLine + (String)(int)expmean + CompareToSpeed(CheckTheseSpeeds[bestmatchindex], expmean * 1000); //String( x,0) seems highly unreliable in the Arduino - Avoid
          }
          Serial.println (sLogLine);
          oled.println(sLogLine);
          //fLogfile.println(sLogLine);
          //////lcd.print(sLogLine.substring(0, ////lcdWIDTH));
          //////lcd.setCursor(0, 3);
          //oled.clear();
          //oled.println("");
          // Standard Deviation
          k = 0;
          do {
            if (IndexOfSpeed[k] == bestmatchindex) {
              sumsquarevar = sumsquarevar + sq(((float)ExposureHistory[k] / 1000.0) - expmean);
            }
            k++;
          } 
          while (ExposureHistory[k] != 0);
          sumsquarevar = sumsquarevar / (n - 1);
          //#if SDREADER
          sLogLine = "SDev=" + (String)sqrt(sumsquarevar) + "ms | n=" + (String)(n);
          Serial.println (sLogLine);
          oled.println(sLogLine);
          //fLogfile.println(sLogLine);
          //////lcd.print(sLogLine.substring(0, ////lcdWIDTH));
          //#endif
          //oled.clear();
        }
        Serial.println();
        #if SDREADER
        fLogfile.println();
        fLogfile.close();
        #endif
        // DebugPrintExposureHistory();
        CountOfExposures++; // we recorded an exposure step on for next time
      } else {
        // stupidly short or long ignore it
      }
    } else {
      // Max exposure memory reached
      Serial.println (F("Exposure Memory Full"));
      ////lcd.clear();
      ////lcd.print(F("Exposure Memory Full"));
      oled.clear();
      oled.set2X();
      oled.println("Exposure");
      oled.println("Memory");
      oled.println("Full");
      oled.set1X();
      ShutterFired = false;
    }
    // reset
    Start = 0;                         // reset Start to 0
    Stop = 0;                           //reset Stop to 0
    ShutterFired = false;
    TimerLock = false; // Timer lock protects existing values from further interrupts during recording. can't use nointerrupts with ////lcd
    // interrupts();  // ready to take the next oone
  }
}

// INTERRUPT Routine
// =================

void ShutterChangeDetector() {
  // this is the interrupt function, which is called everytime the voltage on pin 2 changes - this follows the principle of minimising work in the ISR
  // the only job of this ISR is to detect the shutter firing and collect the start end times.
  if (!TimerLock && LaserON) {  // Timer lock protects existing values from further interrupts during recording. can't use nointerrupts with ////lcd
    ShutterFired = false; // assume not fired
    if (digitalRead(LASERSENSORPIN) == HIGH && Start == 0) {
      // shutter opened - we just start a new timing
      Start = micros();       //set the variable Start to current microseconds
      
    } else if ((digitalRead(LASERSENSORPIN) == LOW ) && (Start != 0)) {
      // Low - the shutter just closed
      Stop = micros();      // set the variable Stop to current microseconds

      //lightIntensity = myBH1750.getLux();
      if (Stop > Start) {
        ShutterFired = true;
      } else {
        //Audrino timer resets to 0 every 70 mins so these can get muddled/
        Start = 0;                         // reset Start to 0
        Stop = 0;                           //reset Stop to 0
      }
    } else {
      // a bit unexpected - starting stopping perhaps b- just reset
      Start = 0;                         // reset Start to 0
      Stop = 0;                           //reset Stop to 0
    }
  }
}

void LaserPowerDetector () {
  // This interrupt routine detects the switching on of of the laser by the laser power Double Pple Double Throw safety switch
  // The idea is by being aware of the beam state we can help avoid recording misleading light beam breaks of start/stop
  // Special code is needed as switches tend to suffer from "Bounce" and mine certainly did.
  // Check for switch Bounce
  if ((millis() - LaserONOFFBounceTime) > LASERSWITCHBOUNCEDELAY) {
    if (digitalRead(LASERPOWERONPIN) == HIGH) {
      // Laser ON
      //Serial.println (F("Laser ON"));
      LaserON = true;
    } else  {
      // Laser Off
      //Serial.println (F("Laser OFF"));
      LaserON = false;
    }
    LaserONOFFBounceTime = millis(); //set the current time
  }
}
