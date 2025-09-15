const unsigned char Digitos[] = {0x08, 0x10, 0x20, 0x04};
const unsigned char Tabela[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x18};
int alg = 0;
long long int tempo = 0;

void setup() {
  DDRB &= ~(1<<PB0);
  DDRB &= ~(1<<PB1);
  DDRB &= ~(1<<PB2);
  
  PORTB |= (1<<PB0); // CLK
  PORTB |= (1<<PB1); // DT
  PORTB |= (1<<PB2); // SW

  PORTB &= ~(1<<PB3); // DOIS PONTOS
  PORTB &= ~(1<<PB4); // BUZZER

  PORTC |= (1<<PC2);
  PORTC |= (1<<PC3);
  PORTC |= (1<<PC4);
  PORTC |= (1<<PC5);

  DDRD |= (1<<PD0);
  DDRD |= (1<<PD1);
  DDRD |= (1<<PD2);
  DDRD |= (1<<PD3);
  DDRD |= (1<<PD4);
  DDRD |= (1<<PD5);
  DDRD |= (1<<PD6);

  DDRB |= (1<<PB4); // BUZZER
  DDRB |= (1<<PB3); // DOIS PONTOS

  DDRC |= (1<<PC2);
  DDRC |= (1<<PC3);
  DDRC |= (1<<PC4);
  DDRC |= (1<<PC5);
}

void loop() {
  PORTD = Tabela[0];

  if((millis() - tempo) > 5){
    PORTC &= ~(PORTC);
    PORTC = Digitos[alg++];
    alg %= 4;
    tempo = millis();
  }
}