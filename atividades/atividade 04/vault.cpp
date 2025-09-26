#include <avr/pgmspace.h>
// =======================================================================
// MATRIZ TECLADO (4x4)
const unsigned char teclado[4][4] PROGMEM = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// DIGITOS 7 SEGMENTOS
const unsigned char Tabela[] PROGMEM = {
  0x40, 0x79, 0x24, 0x30,
  0x19, 0x12, 0x02, 0x78,
  0x80, 0x18, 0x08, 0x03,
  0x46, 0x21, 0x06, 0x0E
};

char coluna = 0;
unsigned long last_column_change = 0;

unsigned char last_change_state[4];
unsigned long last_change_time = 0;
long debounce = 20;

bool locked = false; 

unsigned char idx = 0; 
unsigned char password[4]; 
unsigned char password_attempt[4]; 

// =======================================================================

void setup() {
  DDRB = 0xFF;   
  DDRD = 0x0F;   
  PORTD = 0xFF;  
  UCSR0B = 0x00; 

  DDRC = 0xFF;
  PORTC = 0xFF;
  PORTB |= 0x01;

  PORTB |= 0b00011111;

  last_column_change = millis();

  set_password();
}

void loop() {
  press_key();

  if (idx == 4) {
    check_password();
  }

  column_change();

  display_led();

  delay(1);
}

// =======================================================================
// TECLADO
unsigned char ler_teclado() {
  unsigned char j, tecla = 0xFF, linha;
  PORTD &= ~(1 << coluna); // desliga coluna atual

  linha = PIND >> 4;

  for (j = 0; j < 4; j++) {
    if (!(linha & (1 << j))) {
      tecla = pgm_read_byte(&teclado[j][coluna]);
    }
  }
  PORTD |= (1 << coluna); 
  return tecla;
}

void press_key() {
  unsigned char nr = ler_teclado();

  if (last_change_state[coluna] != nr && millis() - last_change_time > debounce) {
    last_change_time = millis();

    if (nr != 0xFF) {
      unsigned char val = pgm_read_byte(&Tabela[nr <= '9' ? nr - '0' : 10 + nr - 'A']);

      idx %= 4;
      if (idx < 4) {
        password_attempt[idx + 1] = password_attempt[idx];
        password_attempt[idx] = val;
      }
      idx++;
    }
    last_change_state[coluna] = nr;
  }
}

// =======================================================================
// COLUNA / MULTIPLEX
void column_change() {
  if (millis() > (last_column_change + 1)) {
    mult_disp(coluna);

    coluna++;
    coluna %= 4;
    last_column_change = millis();
  }
}

void write_disp(unsigned char val) {
  PORTC = val;
  PORTB &= ~(1 << PB0);
  PORTB |= (0x01) & (val >> 6);
}

void mult_disp(char coluna) {
  write_disp(0xFF);

  if (coluna == 0) {
    PORTB &= ~(1 << PB4);
    if (idx > 0 && !(idx == 4)) write_disp(password_attempt[idx - 1]);
    else write_disp(0xFF);
    PORTB |= (1 << PB1);
  }

  else if (coluna == 1) {
    PORTB &= ~(1 << PB1);
    if (idx > 1 && !(idx == 4)) write_disp(password_attempt[idx - 2]);
    else write_disp(0xFF);
    PORTB |= (1 << PB2);
  }

  else if (coluna == 2) {
    PORTB &= ~(1 << PB2);
    if (idx > 2 && !(idx == 4)) write_disp(password_attempt[idx - 3]);
    else write_disp(0xFF);
    PORTB |= (1 << PB3);
  }

  else if (coluna == 3) {
    PORTB &= ~(1 << PB3);
    if (idx > 3) {
      write_disp(0xFF);
      PORTB &= ~(1 << PB4);
    }
  }
}

// =======================================================================
// LED
void display_led() {
  if (locked) {
    PORTB &= ~(1 << PB5);
  } else {
    PORTB |= (1 << PB5);
  }
}

// =======================================================================
// SENHA
void check_password() {
  bool flag = true;
  for (int i = 0; i < 4; i++) {
    if (password[i] != password_attempt[i]) flag = false;
  }
  if (flag) locked = false;
  else locked = true;
}

void set_password() {
  while (idx <= 4) {
    press_key();
    if (idx == 4) idx++;
    column_change();
    display_led();
    delay(1);
  }
  for (int i = 0; i < 4; i++) {
    password[i] = password_attempt[i];
  }
  idx = 0;
  locked = true;
}