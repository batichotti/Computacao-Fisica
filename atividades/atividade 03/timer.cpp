/* Falta:
- Buzzer ativo por 300ms ao terminar tempo do cronometro;
- Piscar dois pontos durante a contagem regressiva;
- Configurar minuto e segundo de forma separada ao segurar o botao por xxms; *
- Avancar mais rapidamente o tempo ao girar mais rapido. *

*(Extra)

*/

char Posicao[][2] = {{0x08, 0}, {0x10, 0}, {0x20, 0}, {0x04, 0}}; // {QUEM, O QUE} DM UM : DS US
const unsigned char Numero[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10};
int alg = 0;
long long int tempo = 0;
long long int buzzer = 0;

unsigned long clique = 0;
char lastCLK;
char lastSW;
char lastDT;
int cronometro = 0;
bool press = 1;
int doisPontos = 0;
bool doisPOn = 1;

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

void pisca(int piscada){
  if((millis() - tempo) > piscada){
    PORTC &= ~(PORTC);
    PORTD = Numero[Posicao[alg][1]];
    PORTC = Posicao[alg++][0];
    alg %= 4;
    tempo = millis();
  }
}

void acres(){
  Posicao[3][1]++;

  if(Posicao[3][1] >= 10){
    Posicao[3][1] %= 10;
    Posicao[2][1]++;

    if(Posicao[2][1] >= 6){
      Posicao[2][1] %= 6;
      Posicao[1][1]++;
            
      if(Posicao[1][1] >= 10){
        Posicao[1][1] %= 10;
        Posicao[0][1]++;
        
        if(Posicao[0][1] >= 6) Posicao[0][1] %= 6;
      }
    }
  }
}

void decres(){
  Posicao[3][1]--;
        
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

void loop(){
  char CLK = PINB & (1<<PB0);
  char DT = PINB & (1<<PB1);
  
  static unsigned long CLKTime = 0;
  const unsigned long debounce = 5;

  if (CLK != lastCLK && (millis() - CLKTime) > debounce){
    CLKTime = millis();

    if (CLK == 0){

      if (DT != 0){ // Horário
        acres();

      } else { // Anti-horário
        decres();
      }
    }  
  }
  lastCLK = CLK;
  lastDT = DT;

  char SW = PINB & (1<<PB2);

  if (SW == 0 && lastSW != 0 && (millis() - clique) > 50) {
    clique = millis();
    press = !press;
  }

  if (!press) {
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
          press = !press;
          PORTB &= ~(1<<PB4);
          buzzer = millis();

          PORTB &= ~(1<<PB3);
          doisPOn = 1;
        }
    }
  }

  if((millis() - buzzer) > 300 && buzzer != 0) PORTB |= (1<<PB4);

  lastSW = SW;
  pisca(5);
}