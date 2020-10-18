# 1 "D:\\OneDrive - Uni Frankfurt\\OneDrive - stud.uni-frankfurt.de\\Fotografie\\OpenSX-70\\Oktober\\opensx70_lib_14_10_2020 GTD__SONAR_FBW_TCS3200\\opensx70_lib\\opensx70_lib.ino"
# 2 "D:\\OneDrive - Uni Frankfurt\\OneDrive - stud.uni-frankfurt.de\\Fotografie\\OpenSX-70\\Oktober\\opensx70_lib_14_10_2020 GTD__SONAR_FBW_TCS3200\\opensx70_lib\\opensx70_lib.ino" 2
# 3 "D:\\OneDrive - Uni Frankfurt\\OneDrive - stud.uni-frankfurt.de\\Fotografie\\OpenSX-70\\Oktober\\opensx70_lib_14_10_2020 GTD__SONAR_FBW_TCS3200\\opensx70_lib\\opensx70_lib.ino" 2
ClickButton sw_S1(PIN_S1, 0x1);

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

bool FT = 0;
bool S1F = 0;
bool isFocused = 0; //neccessary? should be done by GTD???
int currentPicOnFocus; //dont know what this is for


void setup() {//setup - Inizialize

  Serial.begin (9600);
  Serial.println("Welcome to openSX70 Version: 14_10_2020_SONAR_FBW_TCS3200 GTD and UDONGLE");
  Serial.println("Magic Number: A100=400 | A600 = 150");
  Serial.println("scaling = 100% | filter = clear");
  //Serial.println("16, 20, 23, 25, 30, 35, 45, 55, 166, 302, 600, 1100, POST, POSB, AUTO600, AUTO100");

  myDongle.initDS2408();
  init_EEPROM(); //#writes Default ISO to EEPROM
  // (These are default if not set, but changeable for convenience)
  sw_S1.debounceTime = 15; // Debounce timer in ms 15
  sw_S1.multiclickTime = 250; // Time limit for multi clicks
  sw_S1.longClickTime = 300; // time until "held-down clicks" register
  io_init();
  metercount = 0; //For the Lightmeter Helper Skipping Function
  checkFilmCount();
  inizialized++;
  if (digitalRead(PIN_S5) != 0x0)
  {
    openSX70.mirrorDOWN();

        Serial.println ("Initialize: mirrorDOWN");

  }

    Serial.print("Inizialized: ");
    Serial.println(inizialized);

}

void loop() {//loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop loop
  selector = myDongle.selector();
  switch1 = myDongle.switch1();
  switch2 = myDongle.switch2();
  pprevDongle = prevDongle;
  prevDongle = nowDongle;


  if(isFocused != 1){

    nowDongle = myDongle.checkDongle();
    currentPicture = ReadPicture();
    checkISOChange(); //Blink on ISOChange in non Saving Mode (no Switch1 and no Switch2 is active)
    //saveISOChange(); //Save the selected ISO Value when ISO Saving mode is active (Switch1 and Switch2 are high) and blink Red to validate the saving
    darkslideEject();
    ispackEmpty();
    DongleInserted();
    DongleInsertion(); //State when Dongle insertion is happening


  }



    preFocus();


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


    unfocusing();

}

void turnLedsOff() { //On Dongle, todo:move to camerafunction
  digitalWrite(PIN_LED1, 0x0);
  digitalWrite(PIN_LED2, 0x0);
  return;
  //delay(400);
}



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
  Serial.print(digitalRead(PIN_FT));
  Serial.print(" | FF: ");
  Serial.println(digitalRead(PIN_FF));
  return;
}

void preFocus() {
  if ((digitalRead(PIN_S1F) == 0x1) && (isFocused == 0)) { // S1F pressed
    openSX70.S1F_Focus(); //?!
    currentPicOnFocus = currentPicture;
    isFocused = 1;
    return;
  }
}

void unfocusing(){
  //delay(100);
  if ((digitalRead(PIN_S1F) == 0x0) && (isFocused == 1)) { // S1F pressed  -- selftimer (doubleclick the red button) is not working this way
    //delay(100);
    openSX70.S1F_Unfocus(); // ?!
    //currentPicOnFocus = currentPicture;
    isFocused = 0;
    turnLedsOff();
    return;
  }
}


void BlinkISO() { //read the default ISO and blink once for SX70 and twice for 600
  if ((switch2 != 1) || (switch1 != 1)) { //Not Save ISO //Changed to OR 01.06.2020

    Serial.println("Blink for the saved ISO setting on Dongle insertion.");

    //blinkAutomode();
    savedISO = ReadISO();
    turnLedsOff();
    if (savedISO == 640) {
      myDongle.simpleBlink(2, 6 /*DONGLE GREEN LED ADRESS*/);
      delay(500);
    }
    else if (savedISO == 125) {
      myDongle.simpleBlink(1, 6 /*DONGLE GREEN LED ADRESS*/);
      delay(500);
    }
    else {

      Serial.println("No ISO Selected");
      myDongle.simpleBlink(5, 7 /*DONGLE RED LED ADRESS*/);

    }

      Serial.print ("EEPROM READ ISO: ");
      Serial.println (savedISO);


    prevDongle = nowDongle; //testweise auskommentiert
    checkFilmCount();
    return;
  }
}

void blinkAutomode() {
  if ((switch2 != 1) || (switch1 != 1)) { //Not Save ISO Mode
    turnLedsOff();
    if (ShutterSpeed[selector] == AUTO600) {
      myDongle.simpleBlink(2, 6 /*DONGLE GREEN LED ADRESS*/);

      Serial.println("Blink 2 times Green on Auto600 select");

    }
    if (ShutterSpeed[selector] == AUTO100) {
      myDongle.simpleBlink(1, 6 /*DONGLE GREEN LED ADRESS*/);

      Serial.println("Blink 1 times Green on Auto100 select");

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
      myDongle.simpleBlink(1, 6 /*DONGLE GREEN LED ADRESS*/);
      myDongle.simpleBlink(1, 7 /*DONGLE RED LED ADRESS*/);
    }
    if (ShutterSpeed[selector] == POSB) {
      myDongle.simpleBlink(1, 7 /*DONGLE RED LED ADRESS*/);
      myDongle.simpleBlink(1, 6 /*DONGLE GREEN LED ADRESS*/);
    }
    delay(200);
    checkFilmCount();
    return;
  }
  if ((switch2 == 1) || (switch1 == 1)) { //Lightmeterhelper Activation Mode
    if (ShutterSpeed[selector] == POST) {
      myDongle.simpleBlink(1, 6 /*DONGLE GREEN LED ADRESS*/);
      //myDongle.simpleBlink(1, RED);
    }
    if (ShutterSpeed[selector] == POSB) {
      myDongle.simpleBlink(1, 7 /*DONGLE RED LED ADRESS*/);
      //myDongle.simpleBlink(1, GREEN);
    }
    checkFilmCount();
    return;
  }
}

void BlinkISORed() { //read the active ISO and blink once for SX70 and twice for 600 - on ISO change
  //if ((selector>=14)&&(selector <=15)){ //Blink only on AUTOMODE

    Serial.print("Blink RED on ISO change: ");

  turnLedsOff();
  if (activeISO == 125) {
    myDongle.simpleBlink(1, 7 /*DONGLE RED LED ADRESS*/);
  }
  else if (activeISO == 640) {
    myDongle.simpleBlink(2, 7 /*DONGLE RED LED ADRESS*/);
  }

    Serial.print ("active ISO: ");
    Serial.println (activeISO);

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
          if (_selectedISO != 640) {
            _selectedISO = 640;
          }
        }
        else if (((ShutterSpeed[selector]) == AUTO100)) {
          if (_selectedISO != 125) {
            _selectedISO = 125;
          }
        }
        else {
          //no ISO Selected
          _selectedISO = 640;
        }
        if (savedISO != _selectedISO) { //Check if new ISO is diffrent to the ISO saved in EEPROM

          Serial.print("SaveISOChange() Function: ");
          Serial.print("ISO has changed, previos saved ISO (from EEPROM): ");
          Serial.println(savedISO);
          Serial.print("Saving new selected ISO ");
          Serial.print(_selectedISO);
          Serial.println(" to the EEPROM");

          activeISO = _selectedISO; //Save selectedISO to volatile Variable activeISO
          WriteISO(_selectedISO); //Write ISO to EEPROM
          BlinkISORed(); //Blink ISO Red
          return;
        }
      }
    }
  } else if (nowDongle == 0) {

    Serial.println("savedISOChange() Function - no Dongle detected");

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
        if (_selectedISO != 640) {
          _selectedISO = 640;
        }
      }
      else if (((ShutterSpeed[selector]) == AUTO100)) {
        if (_selectedISO != 125) {
          _selectedISO = 125;
        }
      }
      else { //All other modes
        _selectedISO = ReadISO(); //read from EEPROM
        //_selectedISO = DEFAULT_ISO;
        //Serial.println("ISO ERROR: selecting Default ISO as selected ISO");
      }

      if (activeISO != _selectedISO) {
        activeISO = _selectedISO;

        Serial.print("checkISOChange() Function: ISO has changed, make activeISO = selectedISO, savedISO:");
        Serial.println(savedISO);
        Serial.print("selectedISO: ");
        Serial.print(_selectedISO);
        Serial.print(" activeISO: ");
        Serial.println(activeISO);

        return;
        //blinkAutomode();
      }
    }
  }else { //no Dongle pressend
    savedISO = ReadISO(); //read the savedISO from the EEPROM
    if (activeISO != savedISO) {

      Serial.print("checkISOChange Function - noDongle detected: ISO has changed, savedISO:");
      Serial.print(savedISO);
      Serial.print(" activeISO: ");
      Serial.println(activeISO);

      activeISO = savedISO;

      Serial.print(" make activeISO = selectedISO, savedISO:");
      Serial.print(savedISO);
      Serial.print(" activeISO: ");
      Serial.println(activeISO);

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
      digitalWrite(PIN_LED2, 0x0);
      digitalWrite(PIN_LED1, 0x0);
      openSX70.BlinkTimerDelay (6 /*DONGLE GREEN LED ADRESS*/, 7 /*DONGLE RED LED ADRESS*/, 10);
    }
    else if (mode == 1) {
      digitalWrite(PIN_LED2, 0x0);
      digitalWrite(PIN_LED1, 0x0);
      openSX70.BlinkTimerDelay (6 /*DONGLE GREEN LED ADRESS*/, 7 /*DONGLE RED LED ADRESS*/, 10);
    } else if (mode == 2) {
      digitalWrite(PIN_LED2, 0x0);
      digitalWrite(PIN_LED1, 0x0);
      openSX70.BlinkTimerDelay (6 /*DONGLE GREEN LED ADRESS*/, 7 /*DONGLE RED LED ADRESS*/, 10);
    } else if (mode == 3) {
      Serial.println("Selftimer");
      delay (10000); //NoDongleMode
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

    Serial.print("Two Frames left!");
    Serial.print(", currentPicture on Two Frames left: ");
    Serial.println(currentPicture);

    //myDongle.simpleBlink(2, RED);
    myDongle.dongleLed(7 /*DONGLE RED LED ADRESS*/, 0x0);
    myDongle.dongleLed(6 /*DONGLE GREEN LED ADRESS*/, 0x1);
    return;
  }
  else if (currentPicture == 10) {

    Serial.print("Ten Frames shot!");
    Serial.print(", currentPicture: ");
    Serial.println(currentPicture);

    myDongle.dongleLed(6 /*DONGLE GREEN LED ADRESS*/, 0x0);
    myDongle.dongleLed(7 /*DONGLE RED LED ADRESS*/, 0x1);
    return;
  }
}

void darkslideEject() {
  // STATE 1: EJECT DARKSLIDE:
  if (digitalRead(PIN_S8) == 0x1 && digitalRead(PIN_S9) == 0x0)
  {
    currentPicture = 0;
    WritePicture(currentPicture);
    checkFilmCount(); //For Filmpack Status
    //OPTION TURN ON AND OFF LED WHILE DARKSLIDE EJECT
    if (nowDongle != 0) {
      myDongle.dongleLed (6 /*DONGLE GREEN LED ADRESS*/, 0x1); //green uDongle LED on while ejecting Darkslide
    }
    openSX70.darkslideEJECT(); //Disabled Darkslide eject to change Filmpack in Darkroom
    if (nowDongle != 0) {
      myDongle.dongleLed (6 /*DONGLE GREEN LED ADRESS*/, 0x0); //switching off green uDongle LED
    }

    Serial.println("STATE1: EJECT DARK SLIDE");
    Serial.print("currentPicture on Darkslide eject: ");
    Serial.println(currentPicture);

    return;
  }
}

void ispackEmpty() {
  static int firstRun = 0;
  //STATE 2: PACK IS EMPTY--> NO WASTE OF FLASH
  //Camera Counter is Zero and Switch S9 is CLOSED
  // changed this to allow shooting until counter is actually 0, in case "something" happens and I loose count!
  if ((digitalRead(PIN_S8) == 0x0 && digitalRead(PIN_S9) == 0x1) && (currentPicture >= 8))
  {
    if (firstRun == 0) { //Run only one time when Switch S9 change to HIGH
      firstRun++;
      if (nowDongle != 0) {
        //Serial.println("STATE2: Set LED RED to High");
        //myDongle.dongleLed(RED, HIGH);
      }

      Serial.print("STATE2: PACK IS EMPTY - S9 Closed");
      Serial.print(", Current Picture on Empty Pack: ");
      Serial.println(currentPicture);

    return;
    }
  }
}

void normalOperation() {
  //STATE 3: NORMAL OPERATION *************************************************************************************************************************************************
  if (digitalRead(PIN_S8) == 0x0 && digitalRead(PIN_S9) == 0x0) //S8 and S9 are OPEN
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

      Serial.println("Dongle insertion happend");

    selector = myDongle.selector();
    switch1 = myDongle.switch1();
    switch2 = myDongle.switch2();
    BlinkISO();
    return;
  }
}

void DongleInserted() { //Dongle is pressend LOOP
  if ((selector <= 15) && (myDongle.checkDongle() > 0)) //CASE DONGLE INSERTED
    if (digitalRead(PIN_S1) != 0x1) {

      if (digitalRead(PIN_S1F) != 0x1) { //Dont run DongleInserted Function on S1F pressed

        { //Serial.println("S1F HIGH");
          selector = myDongle.selector();
          switch1 = myDongle.switch1();
          switch2 = myDongle.switch2();
          saveISOChange();//Moved here form loop 11.06.
          //BlinkISO(); //check if dongle inserted, read the default ISO and blink once for SX70 and twice for 600.
          if ((selector != prev_selector)) //Update Dongle changes
          {
# 598 "D:\\OneDrive - Uni Frankfurt\\OneDrive - stud.uni-frankfurt.de\\Fotografie\\OpenSX-70\\Oktober\\opensx70_lib_14_10_2020 GTD__SONAR_FBW_TCS3200\\opensx70_lib\\opensx70_lib.ino"
            blinkAutomode();
            blinkSpecialmode(); //B and T Mode Selector LED Blink
            prev_selector = selector;
            return;
          }
          if ( (switch1 != prev_switch1) || (switch2 != prev_switch2)) {
# 615 "D:\\OneDrive - Uni Frankfurt\\OneDrive - stud.uni-frankfurt.de\\Fotografie\\OpenSX-70\\Oktober\\opensx70_lib_14_10_2020 GTD__SONAR_FBW_TCS3200\\opensx70_lib\\opensx70_lib.ino"
            prev_switch1 = switch1;
            prev_switch2 = switch2;
            return;
          }
        }

      }

    }
}

void positionB() {
  //Position B
  if ((ShutterSpeed[selector] == (POSB)) && (myDongle.checkDongle() > 0)) { //LightmeterHelper Deactivation
    if ((switch2 == 1) && (switch1 == 1)) {
      if (openSX70.getLIGHTMETER_HELPER() == true) {
        openSX70.setLIGHTMETER_HELPER(false);
        turnLedsOff();
        digitalWrite(PIN_LED1, 0x1); //Blink RED -- LMH Off
        delay(100);
        digitalWrite(PIN_LED1, 0x0);

        Serial.println("Lightmeter is off");

      }
    }
  }
  if ((digitalRead(PIN_S1) == 0x1) && (ShutterSpeed[selector] == (POSB))) //////////////POSITION B
  {

        Serial.println("POS B");

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
  if ((ShutterSpeed[selector] == (POST)) && (myDongle.checkDongle() > 0)) { //LightmeterHelper Activation
    if ((switch2 == 1) && (switch1 == 1)) {
      if (openSX70.getLIGHTMETER_HELPER() == false) {
        openSX70.setLIGHTMETER_HELPER(true);
        turnLedsOff();
        digitalWrite(PIN_LED2, 0x1); //Blink Blue -- LMH On
        delay(100);
        digitalWrite(PIN_LED2, 0x0);

          Serial.println("Lightmeter is on");

      }
    }
  }
  if ((digitalRead(PIN_S1) == 0x1) && (ShutterSpeed[selector] == POST)) //////////////POSITION T
  {

      Serial.println("POS T");

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
      openSX70.AutoExposure(640);
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
      openSX70.AutoExposure(125);
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
# 767 "D:\\OneDrive - Uni Frankfurt\\OneDrive - stud.uni-frankfurt.de\\Fotografie\\OpenSX-70\\Oktober\\opensx70_lib_14_10_2020 GTD__SONAR_FBW_TCS3200\\opensx70_lib\\opensx70_lib.ino"
      //}
      //else{
      //  metercount++;
      //}
    }
  }
  return;
}
