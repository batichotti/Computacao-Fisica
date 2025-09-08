void setup(){
 DDRB |= (1 << PB2); // pinMode(10, OUTPUT);
 DDRD |= (1 << PD5); //pinMode(5, OUTPUT);
}

void loop(){
 //LED 1
 PORTB |= (1 << PB2); // digitalWrite(10, HIGH);
 delay(50);

 PORTB &= ~(1 << PB2); // digitalWrite(10, LOW);

 //LED 2
 PORTD |= (1 << PD5); // digitalWrite(5, HIGH);
 delay(50);

 PORTD &= ~(1 << PD5); // digitalWrite(5, LOW);
 delay(450);
}