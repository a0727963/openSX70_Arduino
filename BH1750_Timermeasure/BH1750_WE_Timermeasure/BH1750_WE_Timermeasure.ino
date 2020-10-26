/***************************************************************************
* Example sketch for the BH1750_WE library
* 
* Mode selection / abbreviations:
* CHM:    Continuously H-Resolution Mode
* CHM_2:  Continuously H-Resolution Mode2
* CLM:    Continuously L-Resolution Mode
* OTH:    One Time H-Resolution Mode
* OTH_2:  One Time H-Resolution Mode2
* OTL:    One Time L-Resolution Mode
* 
* Measuring time factor:
* 1.0 ==> Mresuring Time Register = 69
* 0.45 ==> Measuring Time Register = 31
* 3.68 ==> Mesuring Time Register = 254
* 
* Other implemented functions, not used in the example:
* resetDataReg() --> rests Data Register
* powerOn() --> Wake Up!
* powerDown() --> Sleep well, my BH1750
* 
* If you change the measuring time factor for calibration purpose, 
* then you need to devide the light intensity by the measuring time factor 
* 
* Further information can be found on:
* http://wolles-elektronikkiste.de/bh1750fvi-lichtsensormodul
* Sorry - all in German!
***************************************************************************/

#include <Wire.h>
#include <BH1750_WE.h>
#define BH1750_ADDRESS 0x23

BH1750_WE myBH1750(BH1750_ADDRESS); 

unsigned long shutteropen = 0;
int mm = 0;

void setup(){
  Serial.begin(9600);
  Serial.println("Shuttertime Measure Tool");
  Wire.begin();
  myBH1750.init(); // sets default values: mode = CHM, measuring time factor = 1.0
  // myBH1750.setMode(CLM);  // uncomment if you want to change default values
   myBH1750.setMeasuringTimeFactor(0.45); // uncomment for selection of value between 0.45 and 3.68
}

void loop(){ 
  float lightIntensity = myBH1750.getLux();
  //Serial.print(F("Lichtstärke: "));
  //Serial.print(lightIntensity);
  //Serial.println(F(" Lux"));

  if (lightIntensity)
  {
    if(mm==0){
      mm = 1;
      shutteropen = millis();
      Serial.print(lightIntensity); Serial.println(" lux");
      Serial.print("Starttime: ");
      Serial.println(shutteropen);
      
      //Serial.print(event.light); Serial.println(" lux");
      //Serial.println(shutteropen);
    }
  }
  else
  {
      if(shutteropen){
        unsigned long x = millis()-shutteropen;
        Serial.print("Shutteropentime: ");
        Serial.println(x);
        Serial.println("------");
        mm = 0;
      }
      shutteropen = 0;
  }
  //delay(1000);
}
