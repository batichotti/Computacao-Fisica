unsigned long clique = 0;

char last_state_clk;
char last_state_sw;

void setup() {
  DDRC &= ~(1<<PC0);
  DDRC &= ~(1<<PC1);
  DDRC &= ~(1<<PC2);
  
  PORTC |= (1<<PC0);
  PORTC |= (1<<PC1);
  PORTC |= (1<<PC2);

  DDRD |= (1<<PD0);
  DDRD |= (1<<PD1);
  DDRD |= (1<<PD2);
  DDRD |= (1<<PD3);
  DDRD |= (1<<PD4);
  DDRD |= (1<<PD5);
  DDRD |= (1<<PD6);
  DDRD |= (1<<PD7);

  last_state_clk = PINC & (1<<PC2);
  last_state_sw  = PINC & (1<<PC0);
}

void loop() {
  // char leitura_clk = PINC & (1<<PC2);

  // if (leitura_clk != last_state_clk && (millis()-clique)>1) {
  //   clique = millis();

  //   if (!(PINC & (1<<PC1))) {
  //     PORTD >>= 1;
  //   } else {
  //     if (PORTD == 0) PORTD = 0x01;
  //     else PORTD = (PORTD << 1) | 0x01;
  //   }

  //   last_state_clk = leitura_clk;
  // }
/////////////////////////////////////////////////////////////////////////////
  char leitura_sw = PINC & (1<<PC0);

  if (leitura_sw != last_state_sw && (millis()-clique)>1) {
    clique = millis();

    if (leitura_sw == 0) {
      PORTD = ~PORTD;
    }

    last_state_sw = leitura_sw;
  } 
  
  if (!(leitura_sw) && (millis()-clique)>900){
    PORTD &= ~(PORTD);
  }

}


Consegui fazer a parte do 'clica e inverte' e 'clica por 900ms e desliga'