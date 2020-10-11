//20200605 Peter added focus stuff

#include "Arduino.h"
#include "open_SX70.h"
//Version 15_05_2020_V3_Sonar_TCS3200 and UDONGLE_October2020
//Reworked the ISO Saving and using behavior to didn't write to often to EEPROM
// checkISOChange(); makes the Dongle blink on Dialing on Auto600 600BW and SX70 Mode to show what ISO is selected
//Exposure Helper is checking Exposure every 3 Cycles
//Exposure Helper now blinks on faster or slower predicted Exposure Values than possible Shutterspeeds with margin ~1 Aperture
//Stop the Timer on T and B Mode and after each Exposure made
//Has the Sonar S1 logic and restrictions so the camera is waiting for a new picture to make till the S1F is depressed again to prevent multiple Pictures on longpress
//Implemented Rotary Switch debounce
//Fixed Sonar Wireing Bug#
//Fixed NodongleMode
//Moved Switch1 and Switch2 Function to Camera_Function, added a extra Y_Delay for seltimer
//ClickButton sw_S1(PIN_S1, S1Logic, CLICKBTN_PULLUP);
ClickButton sw_S1(PIN_S1, S1Logic);
//ClickButton sw_S1(PIN_S1, HIGH);

int selector;
bool switch1;
bool switch2;
uDongle myDongle (PIN_S2);
Camera openSX70(&myDongle);
byte prev_selector = 0;
byte prev_switch1 = 0;
byte prev_switch2 = 0;
int prevDongle;
int nowDongle;
int savedISO;
int activeISO;
//static int checkedcount;
static int inizialized = 0;
static int metercount;
#if SONAR == 0 
  bool GTD = 1; //For non Sonar Models
#endif
#if SONAR
  //bool GTD = 0;
  bool GTD = 1; //Termporarily always 1 because i dont get GTD reading form the PCB
  bool FT = 0;
  bool S1F = 0;
  bool isFocused = 0;
  int currentPicOnFocus;
#endif
//long oldMillis = 0;
//byte firstRun = 0;

void setup() {//setup - Inizialize
  #if DEBUG
    Serial.begin (9600);
    Serial.println ("Welcome to openSX70 debug activated");
    Serial.println("Version: 09_10_2020_SONAR_FBW_TCS3200 V1 and UDONGLE");
  #endif
  myDongle.initDS2408();
  init_EEPROM(); //#writes Default ISO to EEPROM
  // (These are default if not set, but changeable for convenience)
  sw_S1.debounceTime   = 15;   // Debounce timer in ms 15
  sw_S1.multiclickTime = 250;  // Time limit for multi clicks
  sw_S1.longClickTime  = 300; // time until "held-down clicks" register
  io_init();
  metercount = 0; //For the Lightmeter Helper Skipping Function
  //checkedcount = 0; //For Filmcountercheck Inizializing
  checkFilmCount();
  //BlinkISO();
  inizialized++;
  //  Serial.println ("reset happened");
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
  //if(digitalRead(PIN_S1F) == S1Logic)
  
/*
  if(digitalRead(PIN_S1F) == S1Logic)
  {//Dont run DongleInserted Function on S1F pressed
  
    Serial.println ("Prefocus");

  if(digitalRead(PIN_S1) == S1Logic)
  {//Dont run DongleInserted Function on S1F pressed
  
    Serial.println ("Take");
    
    
  }

  getS1F();
  getGTD();
  getFT();
  if (S1F == S1Logic)
  {
    DoFocus();
  }
  else
  {
    Unfocus();
  }
  
printReadings();
  }
  if(digitalRead(PIN_S1F) == S1Logic)
  {//Dont run DongleInserted Function on S1F pressed
  unsigned long startMillis = millis();
  //  Serial.print ("start: ");
  //  Serial.println (startMillis);
 
  digitalWrite (PIN_S1F_FBW,HIGH);
  getGTD();
    Serial.print ("GTD: ");
    Serial.println (GTD);
  //
  bool GTD = getGTD;
   while(GTD != 1)
  ; 
  {
    unsigned long finalMillis = millis();
    //Serial.print ("end: ");
    //Serial.println (finalMillis);
 
    unsigned long focustime;
    focustime = finalMillis - startMillis;
    Serial.print ("Focus: ");
    Serial.println (focustime);
    return;
  }
  }
  else
  {
    digitalWrite (PIN_S1F_FBW,LOW);
    //Serial.println ("NO focus");
    return;
  }
  
  */

  if(digitalRead(PIN_S1)!= S1Logic){
    #if SONAR
      if(digitalRead(PIN_S1F) != S1Logic){//Dont run DongleInserted Function on S1F pressed
    #endif
      selector = myDongle.selector();
      switch1 = myDongle.switch1();
      switch2 = myDongle.switch2();
      prevDongle = nowDongle;
      nowDongle = myDongle.checkDongle();
      currentPicture = ReadPicture();
      checkISOChange(); //Blink on ISOChange in non Saving Mode (no Switch1 and no Switch2 is active)
      saveISOChange();
      //BlinkISO();
      darkslideEject();
      ispackEmpty();
      DongleInserted();
      printReadings();
  #if SONAR
    }
  #endif
  }
  
  #if SONAR
  getS1F();
  
  if (!S1F)
  {
    if (isFocused)
    {
      Unfocus();
    }
    return;
  }
    
  if (!isFocused)
  {
    DoFocus();
  }

  #endif
  
  normalOperation();
  noDongleMode();
  flashOperationMode();
  positionB();
  positionT();
  manualExposure();
  Auto600Exposure();
  Auto100Exposure();
  //Auto600BWExposure();
  #if SONAR
    if (currentPicture != currentPicOnFocus)
      Unfocus();
  #endif
  //LightMeterHelper(1);
   
} //end of loop
 

void turnLedsOff(){ //todo:move to camerafunction
   digitalWrite(PIN_LED1, LOW);
   digitalWrite(PIN_LED2, LOW);
   //delay(400); 
}

#if SONAR
void getGTD(){
  GTD = 1;
  //GTD = digitalRead(PIN_GTD);
}

void getS1F(){
  S1F = digitalRead(PIN_S1F);
}

void getFT(){
   FT = digitalRead(PIN_FT);
}

void DoFocus()
{
  startFocus();
  getGTD();
  //while(!GTD)
  //  getGTD();
  Serial.println("Now its Focused --> GTD");
  isFocused = 1;
  currentPicOnFocus = currentPicture;
}

void Unfocus()
{
  endFocus();
  isFocused = 0;
}

void startFocus() {
  digitalWrite (PIN_S1F_FBW, HIGH);
}

void endFocus() {
  digitalWrite (PIN_S1F_FBW, LOW);
}


void printReadings(){
  Serial.print("GTD: ");
  Serial.print(GTD);
  Serial.print(" | S1F: ");
  Serial.print(S1F);
  Serial.print(" | FT: ");
  Serial.print(FT);
  Serial.print(" | FF: ");
  Serial.println(digitalRead(PIN_FFA));
  }
#endif

void BlinkISO() { 
  //read the default ISO and blink once for SX70 and twice for 600BW and triple for 600 - on Dongle insertion
    if ((nowDongle != 0) && (prevDongle == 0))
     if((switch2 != 1) || (switch1 != 1)){ //Not on Save ISO Mode
    {
      #if SIMPLEDEBUG
        Serial.println("Blink ISO on Dongle insertion.");
      #endif
      savedISO = ReadISO();
      turnLedsOff();
      if (savedISO == ISO_600){
        myDongle.simpleBlink(2, GREEN);
      }
      else if (savedISO == ISO_SX70){
        myDongle.simpleBlink(1, GREEN);
        //delay(400);
      }/*
      else if (savedISO == ISO_600BW){
        myDongle.simpleBlink(2, GREEN);
        //delay(400);
      }*/
      else {
      #if SIMPLEDEBUG
        Serial.println("No ISO Selected");
      #endif
      }
      #if SIMPLEDEBUG
          Serial.print ("EEPROM READ ISO: ");
          Serial.println (savedISO);
      #endif
      prevDongle = nowDongle;
      checkFilmCount();
      return;
    }
  }
}

void BlinkISO2() { //read the default ISO and blink once for SX70 and twice for 600BW and triple for 600 - on ISO change.
    if((switch2 != 1) || (switch1 != 1)){ //Not on Save ISO Mode
      Serial.println("Blink ISO2");
      savedISO = ReadISO();
      turnLedsOff();
  
      if (activeISO == ISO_SX70){
        myDongle.simpleBlink(1, GREEN);
        delay(400);
      }/*
      else if (activeISO == ISO_600BW){
        myDongle.simpleBlink(2, GREEN);
        delay(400);
      }*/
      else if (activeISO == ISO_600){
        myDongle.simpleBlink(2, GREEN);
      }
      #if SIMPLEDEBUG
          Serial.print ("EEPROM READ ISO: ");
          Serial.println (savedISO);
      #endif
      //prevDongle = nowDongle;
      checkFilmCount();
      return;
    }
}


void BlinkAutomode(){
    if((switch2 != 1) || (switch1 != 1)){ //Not on Save ISO Mode
        #if SIMPLEDEBUG
          Serial.println("Blink Green on Automode select");
        #endif
        turnLedsOff();
        if(ShutterSpeed[selector]== AUTO600){
          myDongle.simpleBlink(2, GREEN);
          checkFilmCount();
        }/*
        if(ShutterSpeed[selector]== AUTO600BW){
          myDongle.simpleBlink(2, GREEN);
          checkFilmCount();
        }
        if(ShutterSpeed[selector]== AUTO100){
          myDongle.simpleBlink(1, GREEN);
          checkFilmCount();
        }
        if(ShutterSpeed[selector]== POST){
          myDongle.simpleBlink(1, GREEN);
          myDongle.simpleBlink(1, RED);
          checkFilmCount();
        }
        if(ShutterSpeed[selector]== POSB){
          myDongle.simpleBlink(1, RED);
          myDongle.simpleBlink(1, GREEN);
          checkFilmCount();
        }*/
    }
}

void BlinkISOGreen() { //read the active ISO and blink once for SX70 and twice for 600BW and triple for 600 - on ISO change.
    //savedISO = ReadISO();
    if((switch2 != 1) || (switch1 != 1)){ //Not on Save ISO Mode
      if ((selector>=11)&&(selector <=13)){ //Blink only on AUTOMODE
        #if SIMPLEDEBUG
            Serial.print("Blink Green on ISO change: ");
        #endif
        turnLedsOff();
        if (activeISO == ISO_SX70){
          myDongle.simpleBlink(1, GREEN);
          //delay(400);
        }/*
        else if (activeISO == ISO_600BW){
          myDongle.simpleBlink(2, GREEN);
          //delay(400);
        }*/
        else if (activeISO == ISO_600){
          myDongle.simpleBlink(2, GREEN);
        }
        #if SIMPLEDEBUG
            Serial.print ("active ISO: ");
            Serial.println (activeISO);
        #endif
        //prevDongle = nowDongle;
        checkFilmCount();
        return;
      }
    }
}

void BlinkISORed() { //read the active ISO and blink once for SX70 and twice for 600BW and triple for 600 - on ISO change.
    //savedISO = ReadISO();
    if ((selector>=11)&&(selector <=13)){ //Blink only on AUTOMODE
      #if SIMPLEDEBUG
          Serial.print("Blink Green on ISO change: ");
      #endif
      turnLedsOff();
      if (activeISO == ISO_SX70){
        myDongle.simpleBlink(1, RED);
        //delay(400);
      }/*
      else if (activeISO == ISO_600BW){
        myDongle.simpleBlink(2, RED);
        //delay(400);
      }*/
      else if (activeISO == ISO_600){
        myDongle.simpleBlink(2, RED);
      }
      #if SIMPLEDEBUG
          Serial.print ("active ISO: ");
          Serial.println (activeISO);
      #endif
      //prevDongle = nowDongle;
      checkFilmCount();
      return;
    }
}

void saveISOChange(){
  int _selectedISO = ReadISO();
    if(nowDongle != 0){ //Donngle is present
      if((switch2 == 1) && (switch1 == 1)){ //Save ISO
        savedISO = ReadISO(); //read the savedISO from the EEPROM
          if (((ShutterSpeed[selector]) == AUTO600)){
              if(_selectedISO != ISO_600){
                _selectedISO = ISO_600; 
              }
          }
          else if (((ShutterSpeed[selector]) == AUTO100)){
            if(_selectedISO != ISO_SX70){
              _selectedISO = ISO_SX70;
            }
          }/*
          else if (((ShutterSpeed[selector]) == AUTO600BW)){
            if(_selectedISO != ISO_600BW){
              _selectedISO = ISO_600BW;
            }*/
          }else{
           //no ISO Selected
           _selectedISO = DEFAULT_ISO;
          }
          if(savedISO != _selectedISO){
            #if SIMPLEDEBUG
              Serial.print("SaveISOChange() Function: ");
              Serial.print("ISO has changed, previos saved ISO(from EEPROM): ");
              Serial.println(savedISO);
              Serial.print("Saving new selected ISO ");
              Serial.print(_selectedISO);
              Serial.println(" to the EEPROM");
            #endif
            //setISO(_selectedISO);
            activeISO = _selectedISO; //Save selectedISO to volatile Variable activeISO
            WriteISO(_selectedISO); //Write ISO to EEPROM
            BlinkISORed(); //Blink ISO and load saved ISO from EEPROM
            return;
          }
        
      
    } else if (nowDongle == 0){
      #if SIMPLEDEBUG
        //Serial.println("savedISOChange() - no Dongle detected");
      #endif
      return;
    }
}

void checkISOChange(){
  int _selectedISO = ReadISO();
  //savedISO = ReadISO(); //read the savedISO from the EEPROM
  if(nowDongle != 0){ //Donngle is present
    if(switch2 == 0 || switch1 == 0){ //Save ISO Function not active
          //savedISO = ReadISO(); //read the savedISO from the EEPROM
          if (((ShutterSpeed[selector]) == AUTO600)){
              if(_selectedISO != ISO_600){
                _selectedISO = ISO_600; 
              }
          }
          else if (((ShutterSpeed[selector]) == AUTO100)){
            if(_selectedISO != ISO_SX70){
              _selectedISO = ISO_SX70;
            }
          }/*
          else if (((ShutterSpeed[selector]) == AUTO600BW)){
            if(_selectedISO != ISO_600BW){
              _selectedISO = ISO_600BW;
            }*/
          }
          else{//All other modes
          _selectedISO = ReadISO();  //read from EEPROM -- why?
          //_selectedISO = DEFAULT_ISO;
          //Serial.println("ISO ERROR: selecting Default ISO as selected ISO");
          }
          if(activeISO != _selectedISO){
            #if SIMPLEDEBUG
              Serial.print("checkISOChange() Function: ISO has changed, make activeISO = selectedISO, savedISO:");
              Serial.println(savedISO);
              Serial.print("selectedISO: ");
              Serial.println(_selectedISO);
            #endif
            activeISO = _selectedISO;
            //BlinkISOGreen(); //Blink Green on ISO select - only check no save
        }
    }
   else { //no Dongle pressend
       savedISO = ReadISO(); //read the savedISO from the EEPROM
       if(activeISO != savedISO){
        #if SIMPLEDEBUG
          Serial.print("checkISOChange Function - noDongle: ISO has changed, savedISO:");
          Serial.print(savedISO);
          Serial.print(" activeISO: ");
          Serial.print(activeISO);
        #endif
        activeISO = savedISO;
        #if SIMPLEDEBUG
          Serial.print(" make activeISO = selectedISO, savedISO:");
          Serial.print(savedISO);
          Serial.print(" activeISO: ");
          Serial.println(activeISO);
        #endif
        //BlinkISOGreen(); // not in NODONGLE MODE???
        }
  }
}

/*
void setISO(int newISO) { //saves the selected ISO to the EEPROM | maybe obsolete
  if ((nowDongle != 0) && (prevDongle == 0)) //read the default ISO and blink once for SX70 and twice for 600.
  {
    WriteISO(newISO); //WriteISO to EEPROM
    BlinkISO2();
    #if SIMPLEDEBUG
        Serial.println("setISO");
        Serial.print("EEPROM READ ISO: ");
        Serial.println(savedISO);
        Serial.print("Saved ISO to EEPROM: ");
        Serial.println(newISO);
    #endif
    prevDongle = nowDongle;
    return;
  }
}
*/
/*
void switch1Function(){
 //Switch One Function
  
}

void switch2Function(int mode){
  //0 Manual, 1 Auto600, 2 AutoSX70, FlashBar
      
  if ((switch2 == 1) || (sw_S1.clicks == 2))
  {
    if(mode==0){
       digitalWrite(PIN_LED2, LOW);
       digitalWrite(PIN_LED1, LOW);
       openSX70.BlinkTimerDelay (GREEN, RED, 10);
    }
    else if(mode==1){
      digitalWrite(PIN_LED2, LOW);
      digitalWrite(PIN_LED1, LOW);
      openSX70.BlinkTimerDelay (GREEN, RED, 10);
    }else if(mode==2){
      digitalWrite(PIN_LED2, LOW);
      digitalWrite(PIN_LED1, LOW);
      openSX70.BlinkTimerDelay (GREEN, RED, 10);
    }else if(mode==3){
        //openSX70.BlinkTimerDelay (GREEN, RED,10);
        delay (10000); //???
      }
    else{
        return false;  
    }
  }
}
*/

void checkFilmCount(){
  if ((currentPicture == 8) || currentPicture == 9){
      #if SIMPLEDEBUG
        Serial.print("Two Frames left!");
        Serial.print(", currentPicture on Two Frames left: ");
        Serial.println(currentPicture);
      #endif
      delay(200);
      myDongle.dongleLed(RED, LOW);
      myDongle.dongleLed(GREEN, HIGH);
      return;
  }
  else if(currentPicture == 10){ 
    #if SIMPLEDEBUG
      Serial.print("Ten Frames shot!");
      Serial.print(", currentPicture: ");
      Serial.println(currentPicture);
    #endif
    delay(200);
    myDongle.dongleLed(GREEN, LOW);
    myDongle.dongleLed(RED, HIGH);
    return;
  }
}

void darkslideEject(){
  // STATE 1: EJECT DARKSLIDE:
  if (digitalRead(PIN_S8) == HIGH && digitalRead(PIN_S9) == LOW)
  {
    currentPicture = 0; 
    WritePicture(currentPicture);
    
    checkFilmCount(); //For Filmpack Status
    //OPTION TURN ON AND OFF LED WHILE DARKSLIDE EJECT
    #if SIMPLEDEBUG
      Serial.println("Darkslide Eject");
    #endif
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

void ispackEmpty(){
  static int firstRun = 0;
  //STATE 2: PACK IS EMPTY--> NO WASTE OF FLASH
  //Camera Counter is Zero and Switch S9 is CLOSED
  // changed this to allow shooting until counter is actually 0, in case "something" happens and I loose count!
  if ((digitalRead(PIN_S8) == LOW && digitalRead(PIN_S9) == HIGH) && (currentPicture >= 8))
  {
    if (firstRun==0){ //Run only one time when Switch S9 change to HIGH
      firstRun++;
      //if (nowDongle != 0) {
        //Serial.println("STATE2: Set LED RED to High");
        //myDongle.dongleLed(RED, HIGH);
      //}
      #if SIMPLEDEBUG
          Serial.print("STATE2: PACK IS EMPTY - S9 Closed");
          Serial.print(", Current Picture on Empty Pack: ");
          Serial.println(currentPicture);
      #endif
    }
  }
}

void normalOperation(){
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
      sw_S1.Update();
  }
}

void noDongleMode(){
  //Cse No Dongle
  //Serial.println("no Dongle Exposure");
  if ((selector == 200) && (myDongle.checkDongle() == 0)) 
  {
    //savedISO = ReadISO();
    LightMeterHelper(0);
    if ((sw_S1.clicks == -1) || (sw_S1.clicks == 1))
    {
      switch1 = 0;
      savedISO = ReadISO();
      openSX70.AutoExposure(savedISO, switch1, switch2);
      sw_S1.Reset();
      return;
    }
    if (sw_S1.clicks == 2) //Doubleclick the Red Button with no Dongle inserted
    {
      switch1 = 0;
      savedISO = ReadISO();
      openSX70.AutoExposure(savedISO, switch1, switch2);
      sw_S1.Reset();
      return;
    }
  }
}

void flashOperationMode(){
  if ((selector == 100) && (myDongle.checkDongle() == 0)) //CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED CASE FLASH INSERTED
  {
    //Serial.println ("FLASH INSERTED");
    if ((sw_S1.clicks == -1) || (sw_S1.clicks == 1))
    {
      openSX70.FlashBAR(switch1, switch2);
      sw_S1.Reset();
      checkFilmCount();
      return;
    }
    if (sw_S1.clicks == 2)
    {
      switch2Function(3); //Switch Two Function in Flash Mode
      openSX70.FlashBAR(switch1, switch2);
      sw_S1.Reset();
      checkFilmCount();
      return;  
    } 
  }
}

void DongleInserted(){ //Dongle is pressend LOOP
  if ((selector <= 15) && (myDongle.checkDongle() > 0)){  //CASE DONGLE INSERTED
    //selector = myDongle.selector();
    //switch1 = myDongle.switch1();
    //switch2 = myDongle.switch2();
    BlinkISO(); //check if dongle inserted, read the default ISO and blink once for SX70 and twice for 600.
      if ((selector != prev_selector)) //Update Dongle changes
      {
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
        BlinkAutomode();
        prev_selector = selector;
        return;
    }
    if( (switch1 != prev_switch1) || (switch2 != prev_switch2)){
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
}

void positionB(){
    //Position B
    if((ShutterSpeed[selector] == (POSB))){ //LightmeterHelper Deactivation
      if  ((switch2 == 1) && (switch1 == 1)){ 
        if(openSX70.getLIGHTMETER_HELPER()==true){
          openSX70.setLIGHTMETER_HELPER(false);
          digitalWrite(PIN_LED1, HIGH); //Blink RED -- LMH Off
          delay(100);
          digitalWrite(PIN_LED1, LOW);
        }
      }
    }
    if (((digitalRead(PIN_S1) == S1Logic) && ((ShutterSpeed[selector] == (POSB))))) //////////////POSITION B
    {
      #if SIMPLEDEBUG
         Serial.println("POS B");
      #endif
      timer_stop();
      turnLedsOff(); //why?
      sw_S1.Reset();
      openSX70.ShutterB(switch1, switch2);
      checkFilmCount();
    }
}

void positionT(){
  //Position T
  if((ShutterSpeed[selector] == (POST))){ //LightmeterHelper Activation
    if  ((switch2 == 1) && (switch1 == 1)){
      if(openSX70.getLIGHTMETER_HELPER()==false){
        openSX70.setLIGHTMETER_HELPER(true);
         digitalWrite(PIN_LED2, HIGH); //Blink Blue -- LMH On
         delay(100);
         digitalWrite(PIN_LED2, LOW);
      }
    }
  }
  if ((digitalRead(PIN_S1) == S1Logic) && (((ShutterSpeed[selector] == (POST))))) //////////////POSITION T
  {
    #if SIMPLEDEBUG
            Serial.println("POS T");
    #endif
    timer_stop();
    turnLedsOff(); //why?
    sw_S1.Reset();
    openSX70.ShutterT(switch1, switch2);
    checkFilmCount();
  }
}

void manualExposure(){
  //Manual Exposure
  if ((selector >= 0) && (selector < 12)) //Manual Exposure original
  //if ((selector >= 0) && (selector < 11)) //Manual Exposure (only 12 Selector Positions because the 13 Position is now 600BW)
  {
    LightMeterHelper(1);
    if ((sw_S1.clicks == -1) || (sw_S1.clicks > 0))
    {
      //switch2Function(0); //switch2Function Manual Mode
      sw_S1.Reset();
      openSX70.ManualExposure(selector, switch1, switch2);
      checkFilmCount();
      return;
    }
  }
}

void Auto600Exposure(){
  //Auto600BW
  if (((ShutterSpeed[selector]) == AUTO600)) //AUTO600BW WHEEL
  {
    LightMeterHelper(0);
    //if (((sw_S1.clicks == -1) || (sw_S1.clicks > 0))) // Checks if the Sonar is Gone that Distance -- is focused
    if ((sw_S1.clicks == -1) || (sw_S1.clicks > 0)) 
    {
      //switch2Function(1); //Switch 2 Function on AUTO600BW
      sw_S1.Reset();
      //openSX70.AutoExposure(activeISO);
      openSX70.AutoExposure(ISO_600, switch1, switch2);
      checkFilmCount();
      return;
    }
  }
}

void Auto100Exposure(){
  //Auto100
  if (((ShutterSpeed[selector]) == AUTO100)) //AUTO100 WHEEL
  {
    //savedISO = ISO_SX70;
    LightMeterHelper(0);
    if ((sw_S1.clicks == -1) || (sw_S1.clicks > 0)) 
    //if (((sw_S1.clicks == -1) || (sw_S1.clicks > 0))) // Checks if the Sonar is Gone that Distance -- is focused
    {
      //switch2Function(1); //Switch 2 Function on AUTO100
      sw_S1.Reset();
      //openSX70.AutoExposure(activeISO);
      openSX70.AutoExposure(ISO_SX70, switch1, switch2);
      checkFilmCount();
      return;
    }
  }
}
/*
void Auto600BWExposure(){
  //Auto600BW
  if (((ShutterSpeed[selector]) == AUTO600BW)) //AUTO600BW WHEEL
  {
    LightMeterHelper(0);
    //if (((sw_S1.clicks == -1) || (sw_S1.clicks > 0))) // Checks if the Sonar is Gone that Distance -- is focused
    if ((sw_S1.clicks == -1) || (sw_S1.clicks > 0)) 
    {
      //switch2Function(1); //Switch 2 Function on AUTO600BW
      sw_S1.Reset();
      //openSX70.AutoExposure(activeISO);
      openSX70.AutoExposure(ISO_600BW, switch1, switch2);
      checkFilmCount();
      return;
    }
  }
}*/

void LightMeterHelper(byte ExposureType){
    int helperstatus = openSX70.getLIGHTMETER_HELPER();
    /*#if SONAR
      if(digitalRead(PIN_S1F) == S1Logic){
    #endif
    */
    if(helperstatus==true){
      //if(metercount==2){ //Lightmeter only on every 3th Cycle of Loop
        meter_led(selector, ExposureType);
        metercount=0;
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
    } /*
    #if SONAR
      }
      else{
        digitalWrite(PIN_LED1, LOW);
        digitalWrite(PIN_LED2, LOW);
      }
      
    #endif SONAR
*/
}
