#include "Arduino.h"
//#include <EEPROM.h>
//#include "Arduino.h"
#include "sx70_sonar_fbw_pcb.h"
#include "open_SX70.h"
#include "meter.h"

//For SONAR PCB
void io_init() {
  // Output definition
  pinMode(PIN_SOL1, OUTPUT);
  pinMode(PIN_SOL2, OUTPUT);
  pinMode(PIN_FF, OUTPUT);
  pinMode(PIN_MOTOR, OUTPUT);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);

  // output default state set.
  digitalWrite(PIN_SOL1, LOW);
  digitalWrite(PIN_SOL2, LOW);
  digitalWrite(PIN_FF, LOW);
  digitalWrite(PIN_MOTOR, LOW);
  digitalWrite(PIN_LED1, LOW);
  digitalWrite(PIN_LED2, LOW);

  // input definition
  #if SONAR == 0
  pinMode(PIN_S1, INPUT_PULLUP); //S1T on Sonar
  #endif
  pinMode(PIN_S2, INPUT_PULLUP); // GND
  pinMode(PIN_S3, INPUT_PULLUP); // GND
  //pinMode(PIN_S4, INPUT); //Analoge Input only - Sol1 on Model2 -- Experimental
  pinMode(PIN_S5, INPUT_PULLUP); // GND
  pinMode(PIN_S8, INPUT); //6V
  pinMode(PIN_S9, INPUT); //6V

 
  #if SONAR
  // Output definition
  pinMode(PIN_S1F_FBW, OUTPUT); //sends 6V to the Sonar S1F to prefocuse
  // output default state set.
  digitalWrite(PIN_S1F_FBW, LOW);
  // input definition
  pinMode(PIN_S1, INPUT); //S1T on Sonar 6V
  pinMode(PIN_S1F, INPUT); //6V
  pinMode(PIN_GTD, INPUT); //6V
  pinMode(PIN_FT, INPUT); // FT on Sonar 6V? -- CAVE Analogue Input only!!! -- Experimental
  #endif
  
}
