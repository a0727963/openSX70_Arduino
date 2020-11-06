#define ANALOG_IN A0
#define SENSOR_OUT 5

const int S0_Pin = 3;
//const int S1_Pin = 2;                   //
const int S1_Pin = 9; //on sonar                   
const int S2_Pin = 8;//A0;
const int S3_Pin = 6;

void setup() {
  //Serial.begin(9600); 
  Serial.begin(115200); 

  pinMode(S0_Pin, OUTPUT);  
  pinMode(S1_Pin, OUTPUT);    
  pinMode(S2_Pin, OUTPUT);  
  pinMode(S3_Pin, OUTPUT);    
  //pinMode(OE_Pin, OUTPUT);    

  //S2 & S0 should be high can be modified via jumper in PCB 
  digitalWrite(S0_Pin, LOW); 
  digitalWrite(S1_Pin, LOW); //scaling LOW = 20% HIGH = 100%
  digitalWrite(S2_Pin, HIGH); 
  digitalWrite(S3_Pin, LOW); //filter LOW = clear HIGH = green
//  digitalWrite(OE_Pin, LOW); //LOW = enabled
  // S0 Low | S1 Low = Skalierunga us
  // S0 Low | S1 High = 2%
  // S0 H   | S1 L    = 20%
  // S0 H   | S1 H    = 100%
}

void loop() {
  //int val = pulseIn(SENSOR_OUT, LOW);
  int val = analogRead(ANALOG_IN);   
  //int val = digitalRead(SENSOR_OUT);
  Serial.println(val);                                           
  Serial.write( 0xff );                                                         
  Serial.write( (val >> 8) & 0xff );                                            
  Serial.write( val & 0xff );
}
