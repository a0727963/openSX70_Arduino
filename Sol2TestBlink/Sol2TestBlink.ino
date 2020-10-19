void setup() {
pinMode(11, OUTPUT);
pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
digitalWrite(11, HIGH); // turn the SOL2 on (HIGH is the voltage level)
digitalWrite(LED_BUILTIN, HIGH);
delay(1000); // wait for a second
digitalWrite(11, LOW); // turn the SOL2 off by making the voltage LOW
digitalWrite(LED_BUILTIN, LOW);
delay(1000); // wait for a second
}
