# 1 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
# 2 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino" 2
# 3 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino" 2
    
# 3 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
   uint8_t lf;
    uint8_t hf;
    uint8_t ef;
    uint8_t lk;
void setup() {
  // put your setup code here, to run once:
    Serial.begin (9600);
}

void loop() {
  // put your main code here, to run repeatedly:
    mmain();
    Serial.println("Welcome to the Optoflow FuseBitsReader");
    Serial.print("LowFuseBits: ");
    Serial.println(lf, 16);

    Serial.print("HighFuseBits: ");
    Serial.println(hf, 16);
    Serial.print("ExtendedFuseBits: ");
    Serial.println(ef, 16);
    Serial.print("LockBits: ");
    Serial.println(lk, 16);
    delay(100000);
}

int mmain(){
    lf = 
# 29 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino" 3
        (__extension__({ uint8_t __result; __asm__ __volatile__ ( "sts %1, %2\n\t" "lpm %0, Z\n\t" : "=r" (__result) : "i" (((uint16_t) &((*(volatile uint8_t *)((0x37) + 0x20))))), "r" ((uint8_t)(((1 << (0)) | (1 << (3))))), "z" ((uint16_t)((0x0000))) ); __result; }))
# 29 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
                                                  ;
    hf = 
# 30 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino" 3
        (__extension__({ uint8_t __result; __asm__ __volatile__ ( "sts %1, %2\n\t" "lpm %0, Z\n\t" : "=r" (__result) : "i" (((uint16_t) &((*(volatile uint8_t *)((0x37) + 0x20))))), "r" ((uint8_t)(((1 << (0)) | (1 << (3))))), "z" ((uint16_t)((0x0003))) ); __result; }))
# 30 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
                                                   ;
    ef = 
# 31 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino" 3
        (__extension__({ uint8_t __result; __asm__ __volatile__ ( "sts %1, %2\n\t" "lpm %0, Z\n\t" : "=r" (__result) : "i" (((uint16_t) &((*(volatile uint8_t *)((0x37) + 0x20))))), "r" ((uint8_t)(((1 << (0)) | (1 << (3))))), "z" ((uint16_t)((0x0002))) ); __result; }))
# 31 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
                                                       ;
    lk = 
# 32 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino" 3
        (__extension__({ uint8_t __result; __asm__ __volatile__ ( "sts %1, %2\n\t" "lpm %0, Z\n\t" : "=r" (__result) : "i" (((uint16_t) &((*(volatile uint8_t *)((0x37) + 0x20))))), "r" ((uint8_t)(((1 << (0)) | (1 << (3))))), "z" ((uint16_t)((0x0001))) ); __result; }))
# 32 "C:\\Users\\Hannes\\Desktop\\fusebitsread\\fusebitsread.ino"
                                              ;
    return 0;
  }
