#include "Arduino.h"
#include "open_SX70.h"
ClickButton sw_S1(PIN_S1, S1Logic);

int selector ;
bool switch1 ;
bool switch2 ;

uDongle myDongle (PIN_S2);
Camera openSX70(&myDongle);
byte prev_selector = 0;
byte prev_switch1 = 0;
byte prev_switch2 = 0;
int prevDongle ;
int pprevDongle ;
int nowDongle ;
int savedISO;
int activeISO;
//static int checkedcount;
static int inizialized = 0;
static int metercount;
//#if ALPHA
//bool GTD = 1; //For non Sonar Models
//bool isFocused = 1; //For non Sonar Models
//#endif
#if SONAR
bool FT = 0;
bool S1F = 0;
bool isFocused = 0; //neccessary? should be done by GTD???
int currentPicOnFocus; //dont know what this is for
#endif

void setup() {//setup - Inizialize
#if DEBUG
  Serial.begin (9600);
  Serial.println("Welcome to openSX70 Version: 18_10_2020_SONAR_FBW_TCS3200 GTD and UDONGLE - Focus on Selftimer End");
  Serial.println("Magic Number: A100=400 | A600 = 150");
  Serial.println("scaling = 100% | filter = clear");
  //Serial.println("16=2000, 20=1000, 23=500, 25=250, 30=125, 35=60, 45=30, 55=15, 166=8, 302=4, 600=2, 1100=1, POST, POSB, AUTO600, AUTO100");
#endif
  myDongle.initDS2408();
  init_EEPROM(); //#writes Default ISO to EEPROM
  // (These are default if not set, but changeable for convenience)
  sw_S1.debounceTime   = 15;   // Debounce timer in ms 15
  sw_S1.multiclickTime = 250;  // Time limit for multi clicks
  sw_S1.longClickTime  = 300; // time until "held-down clicks" register
  io_init();
  metercount = 0; //For the Lightmeter Helper Skipping Function
  checkFilmCount();
  inizialized++;
  if (digitalRead(PIN_S5) != LOW)
  {
    openSX70.mirrorDOWN();
    #if SIMPLEDEBUG
        Serial.println ("Initialize: mirrorDOWN");
    #endif
  }
  #if SIMPLEDEBUG
    Serial.print("Inizialized: ");
    Serial.println(inizialized);
  #endif
}

void loop() {//loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop
  selector = myDongle.selector();
  switch1 = myDongle.switch1();
  switch2 = myDongle.switch2();
  pprevDongle = prevDongle;
  prevDongle = nowDongle;

  #if SONAR
    if(isFocused != 1){
  #endif
    nowDongle = myDongle.checkDongle();
    currentPicture = ReadPicture();
    checkISOChange(); //Blink on ISOChange in non Saving Mode (no Switch1 and no Switch2 is active)
    //saveISOChange(); //Save the selected ISO Value when ISO Saving mode is active (Switch1 and Switch2 are high) and blink Red to validate the saving
    darkslideEject();
    ispackEmpty();
    DongleInserted();
    DongleInsertion(); //State when Dongle insertion is happening

  #if SONAR
    }
  #endif
  
  #if SONAR
    preFocus();
  #endif
  
  //printReadings();
  normalOperation();
  //-----------Picture Taking Functions START-----------------
  //sw_S1.Update();
  Auto100Exposure();
  noDongle();
  flashOperation();
  positionB();
  positionT();
  manualExposure();
  Auto600Exposure();
  //---------- Picture Taking Functions END-----------------

  #if SONAR
    unfocusing();
  #endif
}

void turnLedsOff() { //On Dongle, todo:move to camerafunction
  digitalWrite(PIN_LED1, LOW);
  digitalWrite(PIN_LED2, LOW);
  return;
  //delay(400);
}

#if SONAR
void printReadings() {
  //Serial.print("GTD: ");
  //Serial.print(openSX70.getGTD());
  Serial.print("Is Focused: ");
  Serial.print(isFocused);
  Serial.print(" | GTD: ");
  Serial.print(analogRead(PIN_GTD));
  Serial.print(" | S1F: ");
  Serial.print(digitalRead(PIN_S1F));
  Serial.print(" | FT: ");
  Serial.print(analogRead(PIN_FT));
  Serial.print(" | FF: ");
  Serial.println(digitalRead(PIN_FF));
  return;
}

void preFocus() {
  if ((digitalRead(PIN_S1F) == HIGH) && (isFocused == 0)) { // S1F pressed
    openSX70.S1F_Focus();
    currentPicOnFocus = currentPicture;
    isFocused = 1;
    return;
  }
}

void unfocusing(){
  //delay(100);
  if ((digitalRead(PIN_S1F) == LOW) && (isFocused == 1)) { // S1F pressed  -- selftimer (doubleclick the red button) is not working this way
    //delay(100);
    openSX70.S1F_Unfocus();
    //currentPicOnFocus = currentPicture;
    isFocused = 0;
    turnLedsOff();
    return;
  }
}
#endif

void BlinkISO() { //read the default ISO and blink once for SX70 and twice for 600
  if ((switch2 != 1) || (switch1 != 1)) { //Not Save ISO //Changed to OR 01.06.2020
#if SIMPLEDEBUG
    Serial.println("Blink for the saved ISO setting on Dongle insertion.");
#endif
    //blinkAutomode();
    savedISO = ReadISO();
    turnLedsOff();
    if (savedISO == ISO_600) {
      myDongle.simpleBlink(2, GREEN);
      delay(500);
    }
    else if (savedISO == ISO_SX70) {
      myDongle.simpleBlink(1, GREEN);
      delay(500);
    }
    else {
#if SIMPLEDEBUG
      Serial.println("No ISO Selected");
      myDongle.simpleBlink(5, RED);
#endif
    }
    #if SIMPLEDEBUG
      Serial.print ("EEPROM READ ISO: ");
      Serial.println (savedISO);
    #endif

    prevDongle = nowDongle; //testweise auskommentiert
    checkFilmCount();
    return;
  }
}

void blinkAutomode() {
  if ((switch2 != 1) || (switch1 != 1)) { //Not Save ISO Mode
    turnLedsOff();
    if (ShutterSpeed[selector] == AUTO600) {
      myDongle.simpleBlink(2, GREEN);
    #if SIMPLEDEBUG
      Serial.println("Blink 2 times Green on Auto600 select");
    #endif
    }
    if (ShutterSpeed[selector] == AUTO100) {
      myDongle.simpleBlink(1, GREEN);
    #if SIMPLEDEBUG
      Serial.println("Blink 1 times Green on Auto100 select");
    #endif
    }
  }
  return;
}

void blinkSpecialmode() {

  //Modeselector LED T and B Mode
  if ((switch2 != 1) || (switch1 != 1)) { //Not Save ISO Mode
    turnLedsOff();
    delay(200);
    if (ShutterSpeed[selector] == POST) {
      myDongle.simpleBlink(1, GREEN);
      myDongle.simpleBlink(1, RED);
    }
    if (ShutterSpeed[selector] == POSB) {
      myDongle.simpleBlink(1, RED);
      myDongle.simpleBlink(1, GREEN);
    }
    delay(200);
    checkFilmCount();
    return;
  }
  if ((switch2 == 1) || (switch1 == 1)) { //Lightmeterhelper Activation Mode
    if (ShutterSpeed[selector] == POST) {
      myDongle.simpleBlink(1, GREEN);
      //myDongle.simpleBlink(1, RED);
    }
    if (ShutterSpeed[selector] == POSB) {
      myDongle.simpleBlink(1, RED);
      //myDongle.simpleBlink(1, GREEN);
    }
    checkFilmCount();
    return;
  }
}

void BlinkISORed() { //read the active ISO and blink once for SX70 and twice for 600 - on ISO change
  //if ((selector>=14)&&(selector <=15)){ //Blink only on AUTOMODE
  #if SIMPLEDEBUG
    Serial.print("Blink RED on ISO change: ");
  #endif
  turnLedsOff();
  if (activeISO == ISO_SX70) {
    myDongle.simpleBlink(1, RED);
  }
  else if (activeISO == ISO_600) {
    myDongle.simpleBlink(2, RED);
  }
  #if SIMPLEDEBUG
    Serial.print ("active ISO: ");
    Serial.println (activeISO);
  #endif
  checkFilmCount();
  return;
  //}
}

void saveISOChange() {
  int _selectedISO;
  if (nowDongle != 0) { //Donngle is present
    if (selector >= 14 && selector <= 15) { // Only Save on Automode selector slots! Need to be changed if the Selector order changes!!
      if ((switch2 == 1) && (switch1 == 1)) { //Save ISO Mode
        savedISO = ReadISO(); //read the savedISO from the EEPROM
        if (((ShutterSpeed[selector]) == AUTO600)) {
          if (_selectedISO != ISO_600) {
            _selectedISO = ISO_600;
          }
        }
        else if (((ShutterSpeed[selector]) == AUTO100)) {
          if (_selectedISO != ISO_SX70) {
            _selectedISO = ISO_SX70;
          }
        }
        else {
          //no ISO Selected
          _selectedISO = DEFAULT_ISO;
        }
        if (savedISO != _selectedISO) { //Check if new ISO is diffrent to the ISO saved in EEPROM
        #if SIMPLEDEBUG
          Serial.print("SaveISOChange() Function: ");
          Serial.print("ISO has changed, previos saved ISO (from EEPROM): ");
          Serial.println(savedISO);
          Serial.print("Saving new selected ISO ");
          Serial.print(_selectedISO);
          Serial.println(" to the EEPROM");
        #endif
          activeISO = _selectedISO; //Save selectedISO to volatile Variable activeISO
          WriteISO(_selectedISO); //Write ISO to EEPROM
          BlinkISORed(); //Blink ISO Red
          return;
        }
      }
    }
  } else if (nowDongle == 0) {
#if SIMPLEDEBUG
    Serial.println("savedISOChange() Function - no Dongle detected");
#endif
    return;
  }
}

void checkISOChange() {
  int _selectedISO;
  //savedISO = ReadISO(); //read the savedISO from the EEPROM
  if (nowDongle != 0) { //Donngle is present
    if ((switch2 != 1) && (switch1 != 1)) { //Save ISO Mode not active
      //savedISO = ReadISO(); //read the savedISO from the EEPROM
      if (((ShutterSpeed[selector]) == AUTO600)) {
        if (_selectedISO != ISO_600) {
          _selectedISO = ISO_600;
        }
      }
      else if (((ShutterSpeed[selector]) == AUTO100)) {
        if (_selectedISO != ISO_SX70) {
          _selectedISO = ISO_SX70;
        }
      }
      else { //All other modes
        _selectedISO = ReadISO();  //read from EEPROM
        //_selectedISO = DEFAULT_ISO;
        //Serial.println("ISO ERROR: selecting Default ISO as selected ISO");
      }
      
      if (activeISO != _selectedISO) {
        activeISO = _selectedISO;
        #if SIMPLEDEBUG
        Serial.print("checkISOChange() Function: ISO has changed, make activeISO = selectedISO, savedISO:");
        Serial.println(savedISO);
        Serial.print("selectedISO: ");
        Serial.print(_selectedISO);
        Serial.print(" activeISO: ");
        Serial.println(activeISO);
        #endif
        return;
        //blinkAutomode();
      }
    }
  }else { //no Dongle pressend
    savedISO = ReadISO(); //read the savedISO from the EEPROM
    if (activeISO != savedISO) {
      #if SIMPLEDEBUG
      Serial.print("checkISOChange Function - noDongle detected: ISO has changed, savedISO:");
      Serial.print(savedISO);
      Serial.print(" activeISO: ");
      Serial.println(activeISO);
      #endif
      activeISO = savedISO;
      #if SIMPLEDEBUG
      Serial.print(" make activeISO = selectedISO, savedISO:");
      Serial.print(savedISO);
      Serial.print(" activeISO: ");
      Serial.println(activeISO);
      #endif
      return;
    }
  }
}

void switch1Function() {
  //Switch One Function

}

void switch2Function(int mode) {
  //0 Manual, 1 Auto600, 2 AutoSX70, FlashBar
  if ((switch2 == 1) || (sw_S1.clicks == 2))
  {
    if (mode == 0) {
      openSX70.S1F_Unfocus();
      openSX70.SelfTimerMUP();
      digitalWrite(PIN_LED2, LOW);
      digitalWrite(PIN_LED1, LOW);
      openSX70.BlinkTimerDelay (GREEN, RED, 10);
      //preFocus();
      openSX70.S1F_Focus();
      delay(2000);
    }
    else if (mode == 1) {
      openSX70.S1F_Unfocus();
      openSX70.SelfTimerMUP();
      digitalWrite(PIN_LED2, LOW);
      digitalWrite(PIN_LED1, LOW);
      openSX70.BlinkTimerDelay (GREEN, RED, 10);
      //preFocus();
      openSX70.S1F_Focus();
      delay(2000);
    } else if (mode == 2) {
      openSX70.S1F_Unfocus();
      openSX70.SelfTimerMUP();
      digitalWrite(PIN_LED2, LOW);
      digitalWrite(PIN_LED1, LOW);
      openSX70.BlinkTimerDelay (GREEN, RED, 10);
      //preFocus();
      openSX70.S1F_Focus();
      delay(2000);
    } else if (mode == 3) {
      openSX70.S1F_Unfocus();
      openSX70.SelfTimerMUP();
      Serial.println("Selftimer");
      delay (10000); //NoDongleMode
      //preFocus();
      openSX70.S1F_Focus();
      delay(2000);
    }
    else {
      //return false;
      return;
    }
  }
  return;
}

void checkFilmCount() {
  if ((currentPicture == 8) || currentPicture == 9) {
  #if SIMPLEDEBUG
    Serial.print("Two Frames left!");
    Serial.print(", currentPicture on Two Frames left: ");
    Serial.println(currentPicture);
  #endif
    //myDongle.simpleBlink(2, RED);
    myDongle.dongleLed(RED, LOW);
    myDongle.dongleLed(GREEN, HIGH);
    return;
  }
  else if (currentPicture == 10) {
  #if SIMPLEDEBUG
    Serial.print("Ten Frames shot!");
    Serial.print(", currentPicture: ");
    Serial.println(currentPicture);
  #endif
    myDongle.dongleLed(GREEN, LOW);
    myDongle.dongleLed(RED, HIGH);
    return;
  }
}

void darkslideEject() {
  // STATE 1: EJECT DARKSLIDE:
  if (digitalRead(PIN_S8) == HIGH && digitalRead(PIN_S9) == LOW)
  {
    currentPicture = 0;
    WritePicture(currentPicture);
    checkFilmCount(); //For Filmpack Status
    //OPTION TURN ON AND OFF LED WHILE DARKSLIDE EJECT
    if (nowDongle != 0) {
      myDongle.dongleLed (GREEN, HIGH); //green uDongle LED on while ejecting Darkslide
    }
    openSX70.darkslideEJECT(); //Disabled Darkslide eject to change Filmpack in Darkroom
    if (nowDongle != 0) {
      myDongle.dongleLed (GREEN, LOW); //switching off green uDongle LED
    }
  #if SIMPLEDEBUG
    Serial.println("STATE1: EJECT DARK SLIDE");
    Serial.print("currentPicture on Darkslide eject: ");
    Serial.println(currentPicture);
  #endif
    return;
  }
}

void ispackEmpty() {
  static int firstRun = 0;
  //STATE 2: PACK IS EMPTY--> NO WASTE OF FLASH
  //Camera Counter is Zero and Switch S9 is CLOSED
  // changed this to allow shooting until counter is actually 0, in case "something" happens and I loose count!
  if ((digitalRead(PIN_S8) == LOW && digitalRead(PIN_S9) == HIGH) && (currentPicture >= 8))
  {
    if (firstRun == 0) { //Run only one time when Switch S9 change to HIGH
      firstRun++;
      if  (nowDongle != 0) {
        //Serial.println("STATE2: Set LED RED to High");
        //myDongle.dongleLed(RED, HIGH);
      }
    #if SIMPLEDEBUG
      Serial.print("STATE2: PACK IS EMPTY - S9 Closed");
      Serial.print(", Current Picture on Empty Pack: ");
      Serial.println(currentPicture);
    #endif
    return;
    }
  }
}

void normalOperation() {
  //STATE 3: NORMAL OPERATION *************************************************************************************************************************************************
  if (digitalRead(PIN_S8) == LOW && digitalRead(PIN_S9) == LOW) //S8 and S9 are OPEN
  {
    //WHAT TO DO WHEN POWER-UP:
    //  S8     S9
    // closed  open  --> EJECT DARKSLIDE (DEFAULT)
    // open  closed --> FILM REACH 0 (NO FLASH)
    // open   open  --> NORMAL OPERATION 10 TO 1
    // ///////////////////////////////////PICTURE TAKING OPERATION//////////////////////////////////////////////////
    //    FOUR CASES:
    //   *  CASE 1 NORMAL OPERATION: FULL CYCLE
    //   *  SELECTOR = NORMAL (LOW)
    //   *  NXSHOTS = 0
    //   *  PIN_S1 = LOW (RED BUTTON PRESSED)
    //   *
    //   *  CASE 2 DOUBLE EXPOSURE FIRST SHOT: MIRROR DOWN AND FIRST PICTURE (CLICK: SHUTTER OPERATION REMAINING CLOSED)
    //   *  SELECTOR = DOUBLE (HIGH)
    //   *  MXSHOTS = 0
    //   *  PIN_S1 = LOW (RED BUTTON PRESSED)
    //   *
    //   *  CASE 3 DOUBLE EXPOSURE ULTERIOR MXSHOTS: NO MOTOR OPERATION JUST PICTURE (CLICK: SHUTTER OPERATION REMAINING CLOSED)
    //   *  SELECTOR = DOUBLE (HIGH)
    //   *  NXSHOTS >= 1
    //   *  PIN_S1 = LOW (RED BUTTON PRESSED)
    //   *
    //   *  CASE 4 PICTURE EXPULSION AFTER DOUBLE EXPOSURE: MIRROR DOWN AND SHUTTER OPENING (NO PICTURE TAKEN)
    //   *
    //   *  SELECTOR = NORMAL (LOW)
    //   *  MXSHOTS >= 1
    sw_S1.Update(); //Test Dissabled
    return; 
  }
}

void noDongle() {
  //Case No Dongle
  //Serial.println("no Dongle Exposure");
  if ((selector == 200) && (myDongle.checkDongle() == 0))
  {
    savedISO = ReadISO();
    //LightMeterHelper(0);
    LightMeterHelper(1); //Added 05.06.2020
    if (((sw_S1.clicks == -1) || (sw_S1.clicks == 1)))
    {
      LightMeterHelper(0);
      switch1 = 0; //necessary?
      openSX70.AutoExposure(savedISO);
      sw_S1.Reset();
      return;
    }
    if (sw_S1.clicks == 2) //Doubleclick of Longpress the Red Button with no Dongle inserted
    {
      LightMeterHelper(0);
      switch1 = 0; //necessary?
      openSX70.AutoExposure(savedISO);
      sw_S1.Reset();
      return;
    }
    
  }
}

void flashOperation() {
  if ((selector == 100) && (myDongle.checkDongle() == 0)) //CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED
  {
    //Serial.println ("FLASH INSERTED");
    if ((sw_S1.clicks == -1) || (sw_S1.clicks == 1))
    {
      openSX70.FlashBAR();
      sw_S1.Reset();
      checkFilmCount();
      return;
    }
    if (sw_S1.clicks == 2)
    {
      //switch2Function(3); //Switch Two Function in Flash Mode -- Doesnt work in Sonar PCB
      openSX70.FlashBAR();
      sw_S1.Reset();
      checkFilmCount();
      return;
    }
    
  }
}

void DongleInsertion() {
  //Serial.print("pprevDongle: ");
  //Serial.print(pprevDongle);  
  //Serial.print("prevDongle: ");
  //Serial.print(prevDongle);
  //Serial.print(" | nowDongle: ");
  //Serial.println(nowDongle);
  
  if ((nowDongle != 0) && ((prevDongle == 0) && (pprevDongle == 0))) { //Dongle insertion happend
    #if SIMPLEDEBUG
      Serial.println("Dongle insertion happend");
    #endif
    selector = myDongle.selector();
    switch1 = myDongle.switch1();
    switch2 = myDongle.switch2();
    BlinkISO();
    return;
  }
}

void DongleInserted() { //Dongle is pressend LOOP
  if ((selector <= 15) && (myDongle.checkDongle() > 0))  //CASE DONGLE INSERTED
    if (digitalRead(PIN_S1) != S1Logic) {
    #if SONAR
      if (digitalRead(PIN_S1F) != S1Logic) { //Dont run DongleInserted Function on S1F pressed
    #endif
        { //Serial.println("S1F HIGH");
          selector = myDongle.selector();
          switch1 = myDongle.switch1();
          switch2 = myDongle.switch2();
          saveISOChange();//Moved here form loop 11.06.
          //BlinkISO(); //check if dongle inserted, read the default ISO and blink once for SX70 and twice for 600.
          if ((selector != prev_selector)) //Update Dongle changes
          {
            #if ADVANCEDEBUG
              Serial.print ("DONGLE Mode:  ");
              Serial.print ("Selector: ");
              Serial.print (selector);
              Serial.print (" Switch1: ");
              Serial.print (switch1);
              Serial.print (" Switch2: ");
              Serial.print (switch2);
              Serial.print (" speed: ");
              Serial.println (ShutterSpeed[selector]);
            #endif
            blinkAutomode();
            blinkSpecialmode(); //B and T Mode Selector LED Blink
            prev_selector = selector;
            return;
          }
          if ( (switch1 != prev_switch1) || (switch2 != prev_switch2)) {
            #if ADVANCEDEBUG
              Serial.print ("DONGLE Mode:  ");
              Serial.print ("Selector: ");
              Serial.print (selector);
              Serial.print ("     Switch1: ");
              Serial.print (switch1);
              Serial.print ("     Switch2: ");
              Serial.print (switch2);
              Serial.print ("        speed: ");
              Serial.println (ShutterSpeed[selector]);
            #endif
            prev_switch1 = switch1;
            prev_switch2 = switch2;
            return;
          }
        }
    #if SONAR
      }
    #endif
    }
}

void positionB() {
  //Position B
  if ((ShutterSpeed[selector] == (POSB)) &&  (myDongle.checkDongle() > 0)) { //LightmeterHelper Deactivation
    if ((switch2 == 1) && (switch1 == 1)) {
      if (openSX70.getLIGHTMETER_HELPER() == true) {
        openSX70.setLIGHTMETER_HELPER(false);
        turnLedsOff();
        digitalWrite(PIN_LED1, HIGH); //Blink RED -- LMH Off
        delay(100);
        digitalWrite(PIN_LED1, LOW);
      #if SIMPLEDEBUG
        Serial.println("Lightmeter is off");
      #endif
      }
    }
  }
  if ((digitalRead(PIN_S1) == S1Logic) && (ShutterSpeed[selector] == (POSB))) //////////////POSITION B
  {
    #if SIMPLEDEBUG
        Serial.println("POS B");
    #endif
    lmTimer_stop();
    turnLedsOff(); //why?
    sw_S1.Reset();
    openSX70.ShutterB();
    checkFilmCount();
  }
  return;
}

void positionT() {
  //Position T
  if ((ShutterSpeed[selector] == (POST)) &&  (myDongle.checkDongle() > 0)) { //LightmeterHelper Activation
    if ((switch2 == 1) && (switch1 == 1)) {
      if (openSX70.getLIGHTMETER_HELPER() == false) {
        openSX70.setLIGHTMETER_HELPER(true);
        turnLedsOff();
        digitalWrite(PIN_LED2, HIGH); //Blink Blue -- LMH On
        delay(100);
        digitalWrite(PIN_LED2, LOW);
        #if SIMPLEDEBUG
          Serial.println("Lightmeter is on");
        #endif
      }
    }
  }
  if ((digitalRead(PIN_S1) == S1Logic) && (ShutterSpeed[selector] == POST)) //////////////POSITION T
  {
    #if SIMPLEDEBUG
      Serial.println("POS T");
    #endif
    lmTimer_stop();
    turnLedsOff();
    sw_S1.Reset();
    openSX70.ShutterT();
    checkFilmCount();
  }
  return;
}

void manualExposure() {
  //Manual Exposure
  if ((selector >= 0) && (selector < 12)) //Manual Exposure original
  {
    LightMeterHelper(2);
    if ((sw_S1.clicks == -1) || (sw_S1.clicks > 0))
    //if (((sw_S1.clicks == -1) || (sw_S1.clicks > 0)) && openSX70.getGTD() == 1) // Checks if the Sonar is Gone that Distance -- is focused
    {
      LightMeterHelper(0);
      switch2Function(0); //switch2Function Manual Mode
      sw_S1.Reset();
      openSX70.ManualExposure(selector);
      checkFilmCount();
      return;
    }
  }
  return;
}

void Auto600Exposure() {
  //Auto600
  if (((ShutterSpeed[selector]) == AUTO600)) //AUTO600
  {
    LightMeterHelper(1);
    //sw_S1.Update();
    if (((sw_S1.clicks == -1) || (sw_S1.clicks > 0))) // Checks if the Sonar is Gone that Distance -- is focused
    //if (((sw_S1.clicks == -1) || (sw_S1.clicks > 0)) && openSX70.getGTD() == 1) // Checks if the Sonar is Gone that Distance -- is focused
      //if ((sw_S1.clicks == -1) || (sw_S1.clicks > 0))
    {
      LightMeterHelper(0);
      switch2Function(1); //Switch 2 Function on AUTO600
      sw_S1.Reset();
      openSX70.AutoExposure(ISO_600);
      checkFilmCount();
      return;
    }
  }
  return;
}

void Auto100Exposure() {
  //Auto100
  if (((ShutterSpeed[selector]) == AUTO100)) //AUTO100 WHEEL
  {
    LightMeterHelper(1);
    //if(sw_S1.clicks>=1){
    //Serial.print("S1T click value: ");
    //Serial.println(sw_S1.clicks);
    //}
    //sw_S1.Reset();//test
    //sw_S1.Update();   
    if (((sw_S1.clicks == -1) || (sw_S1.clicks > 0))) // Checks if the Sonar is Gone that Distance -- is focused
    //if (((sw_S1.clicks == -1) || (sw_S1.clicks > 0)) && (openSX70.getGTD()))
    {
      LightMeterHelper(0);
      switch2Function(1); //Switch 2 Function on AUTO100
      sw_S1.Reset();
      //Serial.println("EXPOSURE");
      openSX70.AutoExposure(ISO_SX70);
      checkFilmCount();
      return;
    }
  }
  return;
}

void LightMeterHelper(byte ExposureType) {
  if(isFocused==1){
    int helperstatus = openSX70.getLIGHTMETER_HELPER();
    if (helperstatus == true) {
      //if(metercount==2){ //Lightmeter only on every 3th Cycle of Loop
      meter_led(selector, ExposureType);
      //metercount = 0;
      /*#if ADVANCEDEBUG
        Serial.print("Lightmeter Helper Status:");
        Serial.print(helperstatus);
        Serial.print(", ExposureType:  ");
        Serial.print(ExposureType);
        Serial.print(", Selector: ");
        Serial.println(selector);
        #endif*/
      //}
      //else{
      //  metercount++;
      //}
    }
  }
  return;
}
