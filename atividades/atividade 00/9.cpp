void setup() {
 Serial.begin(115200);
 pinMode(23, OUTPUT);
 pinMode(27, OUTPUT);
}

void loop() {
 //LED 1
 digitalWrite(23, HIGH);
 delay(50);

 digitalWrite(23, LOW);

 //LED 2
 digitalWrite(27, HIGH);
 delay(50);

 digitalWrite(27, LOW);
 delay(450);
}