#line 1 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
#include "Arduino.h"
#include "avr/boot.h"
    uint8_t lf;
    uint8_t hf;
    uint8_t ef;
    uint8_t lk;
#line 7 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
void setup();
#line 12 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
void loop();
#line 28 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
int mmain();
#line 7 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
void setup() {
  // put your setup code here, to run once:
    Serial.begin (9600);
}

void loop() {
  // put your main code here, to run repeatedly:
    mmain();  
    Serial.println("Welcome to the Optoflow FuseBitsReader");
    Serial.print("LowFuseBits: ");
    Serial.println(lf, HEX);

    Serial.print("HighFuseBits: ");
    Serial.println(hf, HEX);
    Serial.print("ExtendedFuseBits: ");
    Serial.println(ef, HEX);
    Serial.print("LockBits: ");
    Serial.println(lk, HEX);
    delay(100000);
}

int mmain(){
    lf = boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
    hf = boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
    ef = boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
    lk = boot_lock_fuse_bits_get(GET_LOCK_BITS);
    return 0;
  }

