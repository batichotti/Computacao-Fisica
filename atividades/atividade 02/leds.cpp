unsigned long clique = 0;

char last_state_clk;
char last_state_sw;
char last_state_dt;
int led_count = 0;

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
  last_state_dt = PINC & (1<<PC1);
}

void loop() {
  char current_clk = PINC & (1<<PC2);
  char current_dt = PINC & (1<<PC1);
  
  if (last_state_clk && !current_clk) {
    if (current_dt) {
      if (led_count > 0) {
        led_count--;
        uint8_t pattern = 0;

        for (int i = 0; i < led_count; i++) {
          pattern |= (1 << i);
        }

        PORTD = pattern;
      }
    } else {
      if (led_count < 8) {
        led_count++;
        uint8_t pattern = 0;

        for (int i = 0; i < led_count; i++) {
          pattern |= (1 << i);
        }
        
        PORTD = pattern;
      }
    }
  }
  
  last_state_clk = current_clk;
  last_state_dt = current_dt;

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