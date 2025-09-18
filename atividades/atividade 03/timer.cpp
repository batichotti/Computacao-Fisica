/* 

-- Intruções --
- Gire o encoder rotativo para a direita ou esquerda para alterar o tempo;
- Um clique simples no botão alterna a edição entre minutos e segundos;
- Um clique longo (1000ms) alterna entre o modo de edição e o modo cronômetro. 

*/

char Posicao[][2] = {{0x08, 0}, {0x10, 0}, {0x20, 0}, {0x04, 0}};
const unsigned char Numero[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10};
int alg = 0;
long long int tempo = 0;
long long int buzzer = 0;

unsigned long clique = 0;
char lastCLK;
char lastSW;
char lastDT;
long long int cronometro = 0;
bool press = true;
bool configMode = true;
bool confMin = false;
long long int swPress = 0;
long long int doisPontos = 0;
bool doisPOn = true;

bool buttonPressed = false;
unsigned long buttonPressTime = 0;
const unsigned long longPressTime = 1000;

unsigned long blinkTime = 0;
bool blinkState = true;
const unsigned long blinkInterval = 500;

void setup() {
  DDRB &= ~(1<<PB0);
  DDRB &= ~(1<<PB1);
  DDRB &= ~(1<<PB2);
  
  PORTB |= (1<<PB0);
  PORTB |= (1<<PB1);
  PORTB |= (1<<PB2);

  DDRB |= (1<<PB3);
  DDRB |= (1<<PB4);
  
  DDRC |= (1<<PC2);
  DDRC |= (1<<PC3);
  DDRC |= (1<<PC4);
  DDRC |= (1<<PC5);
  
  DDRD |= (1<<PD0);
  DDRD |= (1<<PD1);
  DDRD |= (1<<PD2);
  DDRD |= (1<<PD3);
  DDRD |= (1<<PD4);
  DDRD |= (1<<PD5);
  DDRD |= (1<<PD6);

  PORTB &= ~(1<<PB3);
  PORTB &= ~(1<<PB4);
}

void pisca(int piscada){
  if((millis() - tempo) > piscada){
    PORTC &= ~(0x3C);
    
    if(configMode){
      if(confMin && (alg == 0 || alg == 1) && !blinkState){
        PORTD = 0xFF;
      }
      else if(!confMin && (alg == 2 || alg == 3) && !blinkState){
        PORTD = 0xFF;
      }
      else{
        PORTD = Numero[Posicao[alg][1]];
      }
    }
    else{
      PORTD = Numero[Posicao[alg][1]];
    }
    
    PORTC |= Posicao[alg][0];
    
    alg++;
    if(alg >= 4) alg = 0;
    
    tempo = millis();
  }
}

void acres(int quanto = 1){
  Posicao[3][1] += quanto;
  
  if(Posicao[3][1] >= 10){
    Posicao[3][1] = 0;
    Posicao[2][1]++;
    
    if(Posicao[2][1] >= 6){
      Posicao[2][1] = 0;
      Posicao[1][1]++;
            
      if(Posicao[1][1] >= 10){
        Posicao[1][1] = 0;
        Posicao[0][1]++;
        
        if(Posicao[0][1] >= 6) Posicao[0][1] = 0;
      }
    }
  }
}

void acresm(int quanto = 1){
  Posicao[1][1] += quanto;
            
  if(Posicao[1][1] >= 10){
    Posicao[1][1] = 0;
    Posicao[0][1]++;
        
    if(Posicao[0][1] >= 6) Posicao[0][1] = 0;
  }
}

void decres(int quanto = 1){
  Posicao[3][1] -= quanto;
        
  if(Posicao[3][1] < 0){
    Posicao[3][1] = 9;
    Posicao[2][1]--;
          
    if(Posicao[2][1] < 0){
      Posicao[2][1] = 5;
      Posicao[1][1]--;
            
      if(Posicao[1][1] < 0){
        Posicao[1][1] = 9;
        Posicao[0][1]--;
        if(Posicao[0][1] < 0) Posicao[0][1] = 5;
      }
    }
  }
}

void decresm(int quanto = 1){
  Posicao[1][1] -= quanto;
            
  if(Posicao[1][1] < 0){
    Posicao[1][1] = 9;
    Posicao[0][1]--;
    if(Posicao[0][1] < 0) Posicao[0][1] = 5;
  }
}

void loop() {
  char CLK = PINB & (1 << PB0);
  char DT  = PINB & (1 << PB1);

  static unsigned long CLKTime = 0;
  static unsigned long lastPulse = 0;  
  const unsigned long debounce = 5;
  
  if (CLK != lastCLK && (millis() - CLKTime) > debounce) {
    CLKTime = millis();

    if (CLK == 0) {
      unsigned long intervalo = millis() - lastPulse;
      lastPulse = millis();

      int passo = (intervalo < 200) ? 5 : 1;

      if (configMode) {
        if (confMin) {
          if (DT != 0) acresm();
          else         decresm();
        } else {
          if (DT != 0) acres(passo);
          else         decres(passo);
        }
      }
    }
  }

  lastCLK = CLK;
  lastDT = DT;

  char SW = PINB & (1<<PB2);
  
  if (SW == 0 && !buttonPressed) {
    buttonPressed = true;
    buttonPressTime = millis();
  }
  
  if (SW != 0 && buttonPressed) {
    buttonPressed = false;
    unsigned long pressDuration = millis() - buttonPressTime;
    
    if (pressDuration < longPressTime) {
      if (configMode) {
        confMin = !confMin;
      } else {
        press = !press;
      }
    } else {
      configMode = !configMode;
      if (configMode) {
        press = true;
        confMin = false;
      }
    }
  }

  if(configMode && (millis() - blinkTime) > blinkInterval){
    blinkState = !blinkState;
    blinkTime = millis();
  }

  if (!press && !configMode) {
    if ((millis() - cronometro) > 1000) {
      decres();
      cronometro = millis();

      if((millis() - doisPontos) > 500){
        if(doisPOn) PORTB |= (1<<PB3);
        else PORTB &= ~(1<<PB3);
        doisPOn = !doisPOn;
        doisPontos = millis();
      }

      if(Posicao[0][1] == 0 && Posicao[1][1] == 0 && 
         Posicao[2][1] == 0 && Posicao[3][1] == 0){
        press = true;
        PORTB &= ~(1<<PB4);
        buzzer = millis();
        PORTB &= ~(1<<PB3);
        doisPOn = true;
      }
    }
  } else {
    PORTB |= (1<<PB3);
  }

  if((millis() - buzzer) > 300 && buzzer != 0) {
    PORTB |= (1<<PB4);
  }

  lastSW = SW;
  pisca(5);
}