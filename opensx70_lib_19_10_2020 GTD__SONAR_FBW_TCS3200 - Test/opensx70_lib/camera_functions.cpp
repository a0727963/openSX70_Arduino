#include "Arduino.h"
#include "camera_functions.h"
#include "meter.h"
#include "open_sx70.h"
#include "sx70_sonar_fbw_pcb.h"
//#include "Clickbutton.h"
#include "settings.h"
#include "uDongle2.h"

static byte multipleExposureMode = false;
int GTD = 0;
Camera::Camera(uDongle *dongle)
{
  _dongle = dongle;
  //   io_init();
  //  init_EEPROM();
}
//const uint8_t DEBOUNCECOUNT = 10;

#if SONAR
int Camera::getGTD() {
  //GTD = 1;
  int val =  10;
  int aGTD[val];
  int dvdGTD = 0;
  //int aGTD = 0;
  for (int i=0; i <= val; i++){
    aGTD[i] = analogRead(PIN_GTD);
  }
  for (int i=0; i <= val; i++){
    if(aGTD[i] >= 310){
      if(aGTD[i]==aGTD[i-1]){
        dvdGTD++;
      }
    }
  }
  Serial.print("dvdGTD: ");
  Serial.println(dvdGTD);
  Serial.print("aGTD[0]: ");
  Serial.println(aGTD[0]);
  if(dvdGTD>=(val-1)){
    if(aGTD[0] >= 310){
      GTD = 1;
      //delay(5000);
      Serial.println("GTD True");
      return GTD;
    }else if(aGTD[0] <= 309)
    {
      GTD = 0;
      return GTD;
    }
  }
  //GTD = digitalRead(PIN_GTD);
  return 0;
}

void Camera::S1F_Focus()
{
    //int i=0;
    #if BASICDEBUG
      Serial.println ("Focus on");
    #endif
    pinMode(PIN_S1F_FBW, OUTPUT);
    digitalWrite(PIN_S1F_FBW, HIGH);
    
    /*test(!getGTD());
    {
      i++;
      Serial.println("Wait for GTD to go 1");
      Serial.print("getGTD iteration: ");
      Serial.println(i);
      
      digitalWrite(PIN_S1F_FBW, HIGH);
    }*/
    return;
}

int Camera::S1F_Focus1()
{
    //int i=0;
    #if BASICDEBUG
      Serial.println ("Focus on");
    #endif
    pinMode(PIN_S1F_FBW, OUTPUT);
    digitalWrite(PIN_S1F_FBW, HIGH);
    int i = 0;
    while(getGTD()!=1){
      i++;
      Serial.println("Wait for GTD to go 1");
      if(i==20){
        break;
      }
    }
    return 1;
}

void Camera::S1F_Unfocus()
{
    #if BASICDEBUG
      Serial.println ("Focus off");
    #endif
    pinMode(PIN_S1F_FBW, OUTPUT);
    digitalWrite (PIN_S1F_FBW, LOW);
    return;
}
#endif

void Camera::SelfTimerMUP(){
    Serial.println("Selftimer preMirror Up");
    Camera::mirrorUP();
}


void Camera::shutterCLOSE()
{
  #if BASICDEBUG
    Serial.println ("shutterCLOSE");
  #endif
    Camera::HighSpeedPWM();
    analogWrite(PIN_SOL1, 255);
    delay (PowerDownDelay);
    analogWrite (PIN_SOL1, PowerDown);
    return;
}

void Camera::shutterOPEN()
{
  #if BASICDEBUG
    Serial.println ("shutterOPEN");
  #endif
    analogWrite (PIN_SOL1, 0);
    return;
}

void Camera::motorON()
{
#if BASICDEBUG
  Serial.println("motorON");
#endif
  digitalWrite(PIN_MOTOR, HIGH);
}

void Camera::motorOFF()
{
#if BASICDEBUG
  Serial.println ("motorOFF");
#endif
  digitalWrite(PIN_MOTOR, LOW);
}

void Camera::mirrorDOWN()
{
  #if BASICDEBUG
    Serial.println ("mirrorDOWN");
  #endif
  //unsigned long imillis = millis();
  Camera::motorON();
  pinMode(PIN_S5, INPUT_PULLUP); // GND
  while (Camera::DebouncedRead(PIN_S5) != LOW){
    Serial.println("Wait for PIN_S5 to go LOW");
  }
  motorOFF();
  return;
}

void Camera::mirrorUP()
{
  #if BASICDEBUG
    Serial.println ("mirrorUP");
  #endif
  //if(DebouncedRead(PIN_S5) == HIGH){
    motorON();
  //}
  pinMode(PIN_S5, INPUT_PULLUP); // GND
  //while (digitalRead(PIN_S5) != HIGH)            //waiting for S5 to OPEN do NOTHING
  while (DebouncedRead(PIN_S5) != HIGH)
  {
    Serial.println("Wait for PIN_S5 to go HIGH");
  }
  //S5 OPENS
  //S1 MAY BE OPEN NOW (DON'T KNOW HOW TO DO THIS YET)
  // Motor Brake
  motorOFF ();
  return;
}

void Camera::darkslideEJECT()
{
  #if SIMPLEDEBUG
    Serial.println ("darkslideEJECT");
  #endif
  Camera::shutterCLOSE();
  Camera::mirrorUP();
  Camera::mirrorDOWN();
  Camera::shutterOPEN();
  return;
}

void Camera::DongleFlashNormal()
{
  pinMode(PIN_S2, OUTPUT);
  digitalWrite(PIN_SOL2, LOW); //So FFA recognizes the flash as such
  digitalWrite(PIN_FF, HIGH);   //FLASH TRIGGERING
  delay (1);                 //FLASH TRIGGERING
  digitalWrite(PIN_FF, LOW);    //FLASH TRIGGERING
  pinMode(PIN_SOL2, INPUT_PULLUP);  //S2 back to dongle mode
}

void DongleFlashF8 ()
{ /*
    #if SIMPLEDEBUG
    Serial.println ("DONGLE FLASH F8");
    #endif
    //                 byte PictureType = 4;
    //                 CurrentPicture = EEPROM.read(4) ;
    //
    //                 eepromUpdate ();
    //  if (takePicture == true)
    {
     byte PictureType = 6;
    //    eepromUpdate ();
     //         HighSpeedPWM ();
     //         analogWrite(Solenoid2, 255);
     Camera::shutterCLOSE ();
     mirrorUP();   //Motor Starts: MIRROR COMES UP!!!
     ///////while (digitalRead(S3) != HIGH)            //waiting for S3 to OPEN
     while (DebouncedRead(S3) != HIGH)            //waiting for S3 to OPEN
       ;
     //         analogWrite (Solenoid2, 130);
    delay     (YDelay);                               //S3 is now open start Y-delay (40ms)
     shutterOPEN ();
     //                  delay (66);
     delay (80);
     Write_DS2408_PIO (7, 1); // this is for dongle (jack flash)
     //                  digitalWrite(FFA, HIGH); //this is for in-camera flash
     delay (1);
     //                  analogWrite (Solenoid2,0);
     //                  digitalWrite(FFA, LOW);
     Write_DS2408_PIO (7, 0);
     delay (10u);
     shutterCLOSE();
     delay (500);
     delay (200);                             //AGAIN is this delay necessary?
     mirrorDOWN ();                          //Motor starts, let bring the mirror DOWN
     delay (200);                             //AGAIN is this delay necessary?
     shutterOPEN();
     mxshots = 0;
     return;
    }*/
}

void Camera::Ydelay ()
{
  //Only a 120ms delay.
  delay (120);
  return;
}

bool Camera::DebouncedRead(uint8_t pin)
{
  bool lastState = digitalRead(pin);
  uint8_t stablecount = 0;
  while (stablecount < DEBOUNCECOUNT)
  {
    delay(1);
    bool thisState = digitalRead(pin);
    if (thisState == lastState)
      stablecount++;
    else
      stablecount = 0;
    lastState = thisState;
  }
  return lastState;
}

void Camera::HighSpeedPWM()
{
  //const byte n = 224;  // for example, 71.111 kHz
  const byte n =224;
  //PWM high speed
  //one N_Mosfet powerdown
  //taken from: https://www.gammon.com.au/forum/?id=11504
  /*
    Timer 0
    input     T0     pin  6  (D4)
    output    OC0A   pin 12  (D6)
    output    OC0B   pin 11  (D5)

    Timer 1
    input     T1     pin 11  (D5)
    output    OC1A   pin 15  (D9)
    output    OC1B   pin 16  (D10)

    Timer 2
    output    OC2A   pin 17  (D11)
    output    OC2B   pin  5  (D3)
  */
  TCCR2A = bit (WGM20) | bit (WGM21) | bit (COM2B1); // fast PWM, clear OC2A on compare
  TCCR2B = bit (WGM22) | bit (CS20);         // fast PWM, no prescaler
  OCR2A =  n;                                // Value to count to - from table
  OCR2B = ((n + 1) / 2) - 1;                 // 50% duty cycle
  //THIS AFFECTS OUTPUT 3 (Solenoid1) AND OUTPUT 11 (Solenoid2)
}

void Camera::BlinkTimerDelay(byte led1, byte led2, byte time) {
  // DONGLE-LED BLINKS ON COUNTDOWN (10secs)
  // this is a test function to do the progressing blinking of the LED using my blink function
  // it last exactly 10 seconds (2500x4) and I could not accomplish this with the delay()
  // everytime the led (in pin 5) blinks faster 1000, 700, 400, and 100.
  //unsigned long startTimer = millis();
  //*******************************************************
  unsigned long steps = (time * 1000) / 4;
  // DS2408 LED
  #if SONAR
    S1F_Unfocus();
  #endif
  Camera::Blink (1000, steps, led1, 2);
  Camera::Blink (600, steps, led1, 2);
  Camera::Blink (200, steps, led1, 2);
  steps = steps / 2;
  #if SONAR
    S1F_Focus();
  #endif
  //delay(1000);
  Camera::Blink (80, steps, led1, 2);
  Camera::Blink (80, steps, led2, 2);
}

// blink (blink interval=blinking speed, timer=duration blinking, Pin=pin of LED
//type 1 = LED
//type 0 = Piezo //REMOVED NO MORE PIEZO!
//type 2 = DS2480 //REMOVED DON'T KNOW HOW TO DO ITT
// blink is a standalone function

void Camera::Blink (unsigned int interval, int timer, int Pin, byte type)
{
  int ledState = LOW;             // ledState used to set the LED
  pinMode(Pin, OUTPUT);
  unsigned long previousMillis = 0;        // will store last time LED was updated
  unsigned long currentMillisTimer = millis();
  while (millis() < (currentMillisTimer + timer))
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      // if the LED is off turn it on and vice-versa:
      if (ledState == 0) {
        ledState = 1;
      } else {
        ledState = 0;
      }
      // set the LED with the ledState of the variable:
      if (type == 1) {
        digitalWrite (Pin, ledState);
      }  else if (type == 2) {
        //        Serial.println ("TYPE 2");
        _dongle->Write_DS2408_PIO (Pin, ledState);
      }
    }
  }
}

void Camera::ManualExposure(int _selector) {//ManualExposure
  Camera::ExposureStart();
  if ((_dongle->checkDongle() > 0) && (_dongle->switch1() == 1)){ //Switch 1 set ON --> Multiple Exposure Mode
    multipleExposure(0); //MX Manual
  }
  else if (((_dongle->checkDongle() > 0) &&  (mxshots >= 1) && (_dongle->switch1() == 0))){ //Multiexposure last Redbutton Click
    multipleExposureLastClick();
    return;
  }
  else{//Normal Expsoure
    currentPicture++; 
    WritePicture(currentPicture);
    #if SIMPLEDEBUG
      Serial.print("take single Picture on  Manual Mode");
      Serial.print(", current Picture: ");
      Serial.println(currentPicture);
   #endif
  }
  Camera::shutterCLOSE ();
  delay (100); //added to fix bad photos WITH LESS delay
  Camera::mirrorUP();   //Motor Starts: MIRROR COMES UP!!!
  pinMode(PIN_S3, INPUT_PULLUP); // GND
  while (digitalRead(PIN_S3) != HIGH){
    //waiting for S3 to OPEN
    Serial.println("wait for s3");
  }
  delay (YDelay);
  // startCounterCalibration();
  //int ShutterSpeed[] = {17, 20, 23, 25, 30, 35, 45, 55, 68, 102, 166, AUTO600BW, AUTO600, AUTO100, POST, POSB }; //reduced speeds from 25 (slot5) to compensate flash firing
  int ShutterSpeedDelay = ((ShutterSpeed[_selector]) + ShutterConstant) ; //This is the time the Shutter stays open + a constant time for shutter differences on diffrent cameras
  if (_selector >= 6)
  {
    ShutterSpeedDelay = (ShutterSpeedDelay - flashDelay);
  }
  #if ADVANCEDDEBUG
    extern int selector;
    Serial.print("Manual Exposure Debug: ");
    Serial.print("ShutterSpeed[");
    Serial.print(_selector);
    Serial.print("] :");
    Serial.println(ShutterSpeed[_selector]);
    Serial.print("ShutterConstant:");
    Serial.println(ShutterConstant);
    Serial.print("ShutterSpeedDelay:");
    Serial.println(ShutterSpeedDelay);
  #endif

  //sei(); //Interupts restart -- Take the Picture
  Camera::shutterOPEN();
  unsigned long shutterOpenTime = millis();
  //delay (ShutterSpeedDelay);
  while (millis() <= (shutterOpenTime + ShutterSpeedDelay))
    ;
  if (_selector >= 3) // changed the flash selection
  {
    #if SIMPLEDEBUG
        Serial.println("FF - Fill Flash");
    #endif
    Camera::FastFlash();
  }
  #if LMDEBUG
    unsigned long shutterCloseTime = millis(); //Shutter Debug
  #endif
  Camera::ExposureFinish();
  #if LMDEBUG
      unsigned long exposureTime = shutterCloseTime - shutterOpenTime; //Shutter Debug
      Serial.print("ExposureTime on Manualmode: ");
      Serial.println(exposureTime);
  #endif
  return;
}

void Camera::AutoExposure(int _myISO)
{
    Camera::ExposureStart();    
    if ((_dongle->checkDongle() > 0) && (_dongle->switch1() == 1)){ //Switch 1 set ON --> Multiple Exposure Mode
      multipleExposure(1); //MX Auto 
    }
    else if (((_dongle->checkDongle() > 0) &&  (mxshots >= 1) && (_dongle->switch1() == 0))){ //Multiexposure last Redbutton Click
      multipleExposureLastClick();
      return;
    }
    else{//Normal Expsoure
      currentPicture++; 
      WritePicture(currentPicture);
          #if SIMPLEDEBUG
             Serial.print("take a picture on Auto Mode with ISO: ");
             Serial.print(_myISO);
             Serial.print(", current Picture: ");
             Serial.println(currentPicture);
          #endif
    }
    meter_set_iso(_myISO); //Set the correct compare Table for the set ISO
    Camera::shutterCLOSE();
    Camera::mirrorUP();   //Motor Starts: MIRROR COMES UP!!!
    pinMode(PIN_S3, INPUT_PULLUP); // GND
    while (digitalRead(PIN_S3) != HIGH){
      #if ADVANCEDEBUG
        Serial.println("Wait for S3 to open - Mirror Up Endposition");
     #endif
    }
    delay(YDelay);                               //S3 is now open start Y-delay (40ms)
    //startCounter();
    meter_init();
    meter_integrate();
    Camera::shutterOPEN ();
    #if LMDEBUG
      unsigned long shutterOpenTime = millis(); //Shutter Debug
    #endif
    while (meter_update() == false){
    }
    #if LMDEBUG
      unsigned long shutterCloseTime = millis(); //Shutter Debug
    #endif
    Camera::ExposureFinish();
    #if LMDEBUG
      unsigned long exposureTime = shutterCloseTime - shutterOpenTime; //Shutter Debug
      Serial.print("ExposureTime on Automode: ");
      Serial.println(exposureTime);
    #endif
    return;
}

void Camera::FlashBAR() //FlashBAR
{
  Camera::ExposureStart();
  #if SIMPLEDEBUG
     Serial.print("take Camera Flashbar picture");
     Serial.print(", current Picture: ");
     Serial.println(currentPicture);
  #endif
  Camera::HighSpeedPWM();
  analogWrite(PIN_SOL2, 255);
  Camera::shutterCLOSE ();
  Camera::mirrorUP();   //Motor Starts: MIRROR COMES UP!!!
  pinMode(PIN_S3, INPUT_PULLUP); // GND
  while (digitalRead(PIN_S3) != HIGH)            //waiting for S3 to OPEN
    ;
  analogWrite (PIN_SOL2, 130);
  delay (YDelay);                               //S3 is now open start Y-delay (40ms)
  Camera::shutterOPEN ();
  delay (66);
  digitalWrite(PIN_FF, HIGH);
  delay (2);
  analogWrite (PIN_SOL2, 0);
  digitalWrite(PIN_FF, LOW);
  delay (20);
  Camera::shutterCLOSE();
  #if SONAR
      delay(100);
      S1F_Unfocus();
      #if BASICDEBUG
      Serial.println("Unfocus");
      #endif
  #endif
  delay (200);                             //AGAIN is this delay necessary?
  Camera::mirrorDOWN();                          //Motor starts, let bring the mirror DOWN
  delay (200);                             //AGAIN is this delay necessary?
  Camera::shutterOPEN();
  currentPicture++; 
  WritePicture(currentPicture);
  return;
}

/*
void Camera::FlashBAR() //FlashBAR Testprocedure
{
  //currentPicture++; 
  //WritePicture(currentPicture);
  #if SIMPLEDEBUG
     Serial.print("take Camera Flashbar picture");
     Serial.print(", current Picture: ");
     Serial.println(currentPicture);
  #endif
  //Camera::HighSpeedPWM();
  //analogWrite(PIN_SOL2, 255);
  //Camera::shutterCLOSE ();
  //Camera::mirrorUP();   //Motor Starts: MIRROR COMES UP!!!
  //while (digitalRead(PIN_S3) != HIGH)            //waiting for S3 to OPEN
  //  ;
  //analogWrite (PIN_SOL2, 130);
  //delay (YDelay);                               //S3 is now open start Y-delay (40ms)
  //Camera::shutterOPEN ();
  //delay (66);
  //     delay (2);
  
  //     delay (2);
  digitalWrite(PIN_FF, HIGH);    //FLASH TRIGGERING
  //digitalWrite(A7, HIGH);
  delay (1);                      //FLASH TRIGGERING
  digitalWrite(PIN_FF, LOW);     //FLASH TRIGGERING
  //digitalWrite(A7, LOW);
  Serial.println("FFA HIGH");
  //analogWrite(4, 255);

 // delay (2);

  //analogWrite (PIN_SOL2, 0);
  Serial.println("FFA LOw");
  
  //analogWrite(4, 0);

  //analogWrite(PIN_FT LOW);
  //delay (20);
  //Camera::shutterCLOSE();
  //delay (200);                             //AGAIN is this delay necessary?
  //Camera::mirrorDOWN();                          //Motor starts, let bring the mirror DOWN
  //delay (200); 
  Serial.println("Finished FF");//AGAIN is this delay necessary?
  Camera::shutterOPEN();
  return;
}*/

void Camera::ShutterB()
{
  Camera::ExposureStart();
  #if SIMPLEDEBUG
     Serial.print("take B Mode Picture");
     Serial.print(", current Picture: ");
     Serial.println(currentPicture);
  #endif
  Camera::shutterCLOSE ();
  Camera::mirrorUP();   //Motor Starts: MIRROR COMES UP!!!
  pinMode(PIN_S3, INPUT_PULLUP); // GND
  while (digitalRead(PIN_S3) != HIGH){            //waiting for S3 to OPEN˚
    //Serial.println("Wait for s3 to open (HIGH)");
    //  while (openSX70.DebouncedRead(PIN_S3) != HIGH)            //waiting for S3 to OPEN˚
  }
  delay (40);                               //S3 is now open start Y-delay (40ms)
  if (_dongle->switch2() ==  1) //CASE Activate Solenoid 2 for Camfollower Function
  {
    analogWrite(PIN_SOL2, 255); //Solenoid 2 activating
  }
  //MutlipleExposure Finish()
  Camera::shutterOPEN ();
  if (_dongle->switch2() ==  1) //CASE Activate Solenoid 2 for Camfollower Function
  {
    analogWrite(PIN_SOL2, 130);
  }
  while (digitalRead(PIN_S1) == S1Logic){
    //  while (sw_S1.depressed)
    //Serial.println("Red Button Pressed)");
  }
  Camera::FastFlash();
  Camera::shutterCLOSE ();
  if (_dongle->switch2() ==  1) //CASE Deactivate Solenoid 2 for Camfollower Function
  {
    analogWrite(PIN_SOL2, 0);
  }  
  #if SONAR
      delay(100);
      S1F_Unfocus(); //neccesary???
      #if BASICDEBUG
        Serial.println("Unfocus");
      #endif
  #endif
  delay (200);                             //AGAIN is this delay necessary?

  //Camera::ExposureFinish();//new
  Camera::mirrorDOWN ();                          //Motor starts, let bring the mirror DOWN
  delay (200);                             //AGAIN is this delay necessary?
  Camera::shutterOPEN();
  currentPicture++; 
  WritePicture(currentPicture);
  return;
}

void Camera::ShutterT()
{
  Camera::ExposureStart();
  #if SIMPLEDEBUG
     Serial.print("take T Mode picture: ");
     Serial.print(", current Picture: ");
     Serial.println(currentPicture);
  #endif
  //Why turn off the LIGHT?
  //digitalWrite(PIN_LED2, LOW);
  //digitalWrite(PIN_LED1, LOW);
  Camera::shutterCLOSE ();
  Camera::mirrorUP();   //Motor Starts: MIRROR COMES UP!!!
  pinMode(PIN_S3, INPUT_PULLUP); // GND
  while (DebouncedRead(PIN_S3) != HIGH){
    //waiting for S3 to OPEN
  }
  delay (40);                               //S3 is now open start Y-delay (40ms)
  //MutlipleExposure Finish()
  #if SONAR
  while (digitalRead(PIN_S1F) == HIGH){
  }
  #endif
  Camera::shutterOPEN ();
  while (digitalRead(PIN_S1) == !S1Logic){
    //nothing
    //digitalWrite(PIN_LED2, HIGH); //Turn Blue Led on while Shutter is Open
  }
  if (digitalRead(PIN_S1) == S1Logic)
  {
    Camera::FastFlash();
    Camera::shutterCLOSE ();
  }
  #if SONAR
      delay(100);
      S1F_Unfocus(); //neccesary???
      #if BASICDEBUG
        Serial.println("Unfocus");
      #endif
  #endif
  delay (200);                             //AGAIN is this delay necessary?
  Camera::mirrorDOWN ();                          //Motor starts, let bring the mirror DOWN
  delay (200);                             //AGAIN is this delay necessary?
  Camera::shutterOPEN();
  currentPicture++; 
  WritePicture(currentPicture);
  return;
}

void Camera::multipleExposure(int exposureMode){
  static int multipleExposureCounter = 0;
  //multipleExposureMode = false;
  /*#if ADVANCEDEBUG 1
    Serial.print("multiplExposuremode: ");
    Serial.println(multipleExposureMode);
  #endif
  */
  
  //0 Manual, 1 Auto, 2 Flashbar, 3 B, 4 T
  if(exposureMode==0){
    //Manual Mode
      #if MXDEBUG
        Serial.println("MultiExp on Manual Mode");
      #endif
      if(multipleExposureMode == false){//First Run MultiExp
        //currentPicture++;
        multipleExposureCounter++;
        //WritePicture(currentPicture);
        multipleExposureMode = true;
        #if MXDEBUG
          Serial.print("current Picture (MX): ");
          Serial.println(currentPicture);
        #endif
        return;
      }else{
        multipleExposureCounter++; //Dont increase currentPicture counter on additional clicks of Multipleexposure
        return;
      }
  }else if(exposureMode==1){
    //Auto Mode
    #if MXDEBUG
      Serial.println("MultiExp on Auto Mode");
    #endif
    if(multipleExposureMode == false){//First Run MultiExp
      //currentPicture++;
      multipleExposureCounter++;
      //WritePicture(currentPicture);
      multipleExposureMode = true;
      #if MXDEBUG
        Serial.print("First run MX Mode, current Picture (MX): ");
        Serial.println(currentPicture);
      #endif
      return;
    }else{
      multipleExposureCounter++; //Dont increase currentPicture counter on additional clicks of Multipleexposure
      return;
    }
  }
}

void Camera::ExposureStart(){
  #if SONAR
    while(S1F_Focus1()!=1){
      Serial.println("Wait for GTD");
    }
    
    //while(getGTD()!=1){ //Not sure if this is nececcary!!!
    //if(GTD==1)
    //  break;
    //S1F_Focus();

    //Serial.println("getGTD");
    //delay(1000);
    //}
    
    return;
    //delay(200);
  #endif
}

void Camera::ExposureFinish(){
  Camera::shutterCLOSE();
  lmTimer_stop(); //Timer stop
  #if SONAR
    delay (100);                             //AGAIN is this delay necessary?
    S1F_Unfocus(); //neccesary???
    #if BASICDEBUG
      Serial.println("Unfocus");
    #endif
  #endif
  delay (200); //Was 20
  if ((_dongle->checkDongle() > 0) && (_dongle->switch1() == 1)){ // MX
    Camera::multipleExposureFinish();
    return;
  }
  else if ((_dongle->checkDongle() > 0) && (_dongle->switch1() == 0)){ //Dongle present
    delay (100);
    Camera::mirrorDOWN ();                          //Motor starts, let bring the mirror DOWN
    delay (300);                  //WAS 60           //AGAIN is this delay necessary?
    Camera::shutterOPEN();
    //mxshots = 0;
    #if SIMPLEDEBUG
      Serial.print("Exposure Finish - Dongle Mode, ");
      Serial.print("mxshots count: ");
      Serial.println(mxshots);
    #endif
    #if SONAR
      while(digitalRead(PIN_S1) == HIGH){
      Serial.println("Wait for S1 to go high again"); //prevent multiple picture Taking if S1 stays pressed
      }
    #endif
    return;
  }
  else if (_dongle->checkDongle() == 0){ //No Dongle
    /*#if SONAR
      delay(100);
      S1F_Unfocus(); //neccesary???
      #if BASICDEBUG
        Serial.println("Unfocus");
      #endif
    #endif*/
    delay (100);                             //AGAIN is this delay necessary?
    Camera::mirrorDOWN ();                          //Motor starts, let bring the mirror DOWN
    delay (300);                  //WAS 60           //AGAIN is this delay necessary?
    Camera::shutterOPEN();
    mxshots = 0;
    #if SIMPLEDEBUG
      Serial.print("Exposure Finish - No Dongle Mode, ");
      Serial.print("MX shots count: ");
      Serial.println(mxshots);
    #endif
    #if SONAR
      while(digitalRead(PIN_S1) == HIGH){
      Serial.println("Wait for S1 to go high again"); //prevent multiple picture Taking if S1 stays pressed
      }
    #endif
    return;  
  }
}

void Camera::multipleExposureLastClick(){
  {
      //sw_S1.Reset();
      #if MXDEBUG
            Serial.print("Multiexposure last Red Button Click, mxshots: ");
            Serial.print(mxshots);
            Serial.print(", CurrentPicture: ");
            Serial.println(currentPicture);
      #endif
      mxshots = 0;
      #if SONAR
        delay (100);                             //AGAIN is this delay necessary?
        S1F_Unfocus(); //neccesary???
        #if BASICDEBUG
          Serial.println("Unfocus");
        #endif
      #endif
      Camera::mirrorDOWN(); 
      delay(50);                             //AGAIN is this delay necessary? 100-->50
      Camera::shutterOPEN();
      multipleExposureMode = 0;
      return;
    }
}

void Camera::multipleExposureFinish(){
 //Switch1 Function Multiexposure Mode
    #if MXDEBUG
      Serial.print("Multiexposure shots count: ");
      Serial.println(mxshots);
    #endif
    /*#if SONAR
      S1F_Unfocus(); //neccesary???
      #if BASICDEBUG
        Serial.println("Unfocus");
      #endif
    #endif*/
    mxshots++;
    //return;
}

void Camera::FastFlash()
{
  #if BASICDEBUG
    Serial.println("FastFlash");
  #endif
  pinMode(PIN_S2, OUTPUT);
  //     delay (2);
  digitalWrite (PIN_S2, LOW);     //So FFA recognizes the flash as such
  //     delay (2);
  digitalWrite(PIN_FF, HIGH);    //FLASH TRIGGERING
  delay (1);                      //FLASH TRIGGERING
  digitalWrite(PIN_FF, LOW);     //FLASH TRIGGERING
  pinMode(PIN_S2, INPUT_PULLUP);  //S2 back to normal
}

bool Camera::setLIGHTMETER_HELPER(bool state){
  #if LMDEBUG
    Serial.print("Set Lightmeterhelper status: ");
    Serial.println(state);
  #endif
  lightmeterHelper = state;
  return state;
  //return;
}

bool Camera::getLIGHTMETER_HELPER(){
  #if LMDEBUG
    //Serial.println("Lightmeterhelper status: ");
    //Serial.println(lightmeterHelper));
  #endif
  return lightmeterHelper;
}
