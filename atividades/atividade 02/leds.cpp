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
  
  static unsigned long lastClkChangeTime = 0;
  const unsigned long DEBOUNCE_TIME = 5;
  const int LED_COUNT = 8;

  if (current_clk != last_state_clk && (millis() - lastClkChangeTime) > DEBOUNCE_TIME) {
    lastClkChangeTime = millis();

    if (current_clk == 0) {
      if (current_dt != 0) { // Anti-horário
        if (led_count > 0) {
          PORTD &= ~(1 << (led_count - 1));
          led_count--;
        }
      } else { // Horário
        if (led_count < LED_COUNT) {
          PORTD |= (1 << led_count);
          led_count++;
        }
      }
    }
    last_state_clk = current_clk;
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