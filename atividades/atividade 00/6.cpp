void setup(){
 pinMode(10, OUTPUT);
 pinMode(5, OUTPUT);
}

void loop(){
 //LED 1
 digitalWrite(10, HIGH);
 delay(50);

 digitalWrite(10, LOW);

 //LED 2
 digitalWrite(5, HIGH);
 delay(50);

 digitalWrite(5, LOW);
 delay(450);
}