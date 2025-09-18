/* 
================ INSTRUÇÕES DE USO =================

- Link do Circuito: https://wokwi.com/projects/441915019957786625

- Gire o encoder rotativo:
    - Direita → aumenta tempo
    - Esquerda → diminui tempo

- Botão do encoder:
    - Clique simples → alterna entre edição de minutos e segundos
    - Clique longo (≥ 1000ms) → alterna entre:
        -> modo de edição (ajuste de tempo)
        -> modo cronômetro (contagem regressiva)
====================================================
*/

// ---------- CONFIGURAÇÃO DO DISPLAY ----------
char Posicao[][2] = {
  {0x08, 0}, // Dígito 1 (dezena de minutos)
  {0x10, 0}, // Dígito 2 (unidade de minutos)
  {0x20, 0}, // Dígito 3 (dezena de segundos)
  {0x04, 0}  // Dígito 4 (unidade de segundos)
};

// Mapa dos números para display 7 segmentos (0–9)
const unsigned char Numero[] = {
  0x40, // 0
  0x79, // 1
  0x24, // 2
  0x30, // 3
  0x19, // 4
  0x12, // 5
  0x02, // 6
  0x78, // 7
  0x00, // 8
  0x10  // 9
};

// ---------- VARIÁVEIS DE CONTROLE ----------
int alg = 0;                 // Índice do dígito ativo (para multiplexação)
long long int tempo = 0;     // Controle de tempo da função pisca()
long long int buzzer = 0;    // Tempo em que o buzzer foi ativado

// Estado do encoder e botão
char lastCLK;
char lastSW;
char lastDT;

// Controle do cronômetro
long long int cronometro = 0;
bool press = true;           // true = cronômetro parado, false = rodando
bool configMode = true;      // true = modo configuração, false = cronômetro ativo
bool confMin = false;        // true = edição de minutos, false = edição de segundos

// Piscar dos dois pontos ":"
long long int doisPontos = 0;
bool doisPOn = true;

// Controle de botão
bool buttonPressed = false;
unsigned long buttonPressTime = 0;
const unsigned long longPressTime = 1000; // 1s para clique longo

// Controle de piscar dígito em edição
unsigned long blinkTime = 0;
bool blinkState = true;
const unsigned long blinkInterval = 500; // pisca a cada 500ms


// ---------- CONFIGURAÇÃO DE PINOS ----------
void setup() {
  // Entradas do encoder (CLK, DT, SW)
  DDRB &= ~(1<<PB0);
  DDRB &= ~(1<<PB1);
  DDRB &= ~(1<<PB2);
  
  // Habilita pull-up interno
  PORTB |= (1<<PB0);
  PORTB |= (1<<PB1);
  PORTB |= (1<<PB2);

  // Saídas: dois pontos (PB3), buzzer (PB4)
  DDRB |= (1<<PB3);
  DDRB |= (1<<PB4);
  
  // Saídas: controle de dígitos do display
  DDRC |= (1<<PC2) | (1<<PC3) | (1<<PC4) | (1<<PC5);
  
  // Saídas: segmentos do display (a–g)
  DDRD |= (1<<PD0)|(1<<PD1)|(1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6);

  // Inicialmente desligados
  PORTB &= ~(1<<PB3);
  PORTB &= ~(1<<PB4);
}


// ---------- FUNÇÃO DE MULTIPLEXAÇÃO DO DISPLAY ----------
void pisca(int piscada){
  if((millis() - tempo) > piscada){
    PORTC &= ~(0x3C); // limpa dígitos (desliga todos)
    
    // Caso em configuração: pisca dígito sendo editado
    if(configMode){
      if(confMin && (alg == 0 || alg == 1) && !blinkState){
        PORTD = 0xFF; // apaga dígito
      }
      else if(!confMin && (alg == 2 || alg == 3) && !blinkState){
        PORTD = 0xFF; // apaga dígito
      }
      else{
        PORTD = Numero[Posicao[alg][1]]; // mostra número
      }
    }
    else{
      PORTD = Numero[Posicao[alg][1]];   // modo cronômetro → mostra sempre
    }
    
    PORTC |= Posicao[alg][0]; // ativa dígito atual
    
    alg++;
    if(alg >= 4) alg = 0; // próximo dígito
    
    tempo = millis(); // atualiza tempo da multiplexação
  }
}


// ---------- FUNÇÕES DE INCREMENTO ----------
void acres(int quanto = 1){ // segundos
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

void acresm(int quanto = 1){ // minutos
  Posicao[1][1] += quanto;
            
  if(Posicao[1][1] >= 10){
    Posicao[1][1] = 0;
    Posicao[0][1]++;
        
    if(Posicao[0][1] >= 6) Posicao[0][1] = 0;
  }
}


// ---------- FUNÇÕES DE DECREMENTO ----------
void decres(int quanto = 1){ // segundos
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

void decresm(int quanto = 1){ // minutos
  Posicao[1][1] -= quanto;
            
  if(Posicao[1][1] < 0){
    Posicao[1][1] = 9;
    Posicao[0][1]--;
    if(Posicao[0][1] < 0) Posicao[0][1] = 5;
  }
}


// ---------- LOOP PRINCIPAL ----------
void loop() {
  // ----- Leitura do encoder -----
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

      // Giro rápido → incrementa 5, lento → incrementa 1
      int passo = (intervalo < 200) ? 5 : 1;

      if (configMode) {
        if (confMin) {
          if (DT != 0) acresm();  // aumenta minutos
          else         decresm(); // diminui minutos
        } else {
          if (DT != 0) acres(passo);  // aumenta segundos
          else         decres(passo); // diminui segundos
        }
      }
    }
  }

  lastCLK = CLK;
  lastDT = DT;

  // ----- Leitura do botão -----
  char SW = PINB & (1<<PB2);
  
  if (SW == 0 && !buttonPressed) {
    buttonPressed = true;
    buttonPressTime = millis();
  }
  
  if (SW != 0 && buttonPressed) {
    buttonPressed = false;
    unsigned long pressDuration = millis() - buttonPressTime;
    
    if (pressDuration < longPressTime) {
      // Clique curto
      if (configMode) {
        confMin = !confMin; // alterna edição (min/seg)
      } else {
        press = !press;     // pausa/continua cronômetro
      }
    } else {
      // Clique longo
      configMode = !configMode; // alterna modo
      if (configMode) {
        press = true;      // pausa
        confMin = false;   // volta para segundos
      }
    }
  }

  // ----- Piscar dígito em edição -----
  if(configMode && (millis() - blinkTime) > blinkInterval){
    blinkState = !blinkState;
    blinkTime = millis();
  }

  // ----- Modo cronômetro -----
  if (!press && !configMode) {
    if ((millis() - cronometro) > 1000) {
      decres();                 // decrementa 1s
      cronometro = millis();

      // Piscar os dois pontos
      if((millis() - doisPontos) > 500){
        if(doisPOn) PORTB |= (1<<PB3);
        else        PORTB &= ~(1<<PB3);
        doisPOn = !doisPOn;
        doisPontos = millis();
      }

      // Se chegou em 00:00 → aciona buzzer
      if(Posicao[0][1] == 0 && Posicao[1][1] == 0 && 
         Posicao[2][1] == 0 && Posicao[3][1] == 0){
        press = true;           // pausa
        PORTB &= ~(1<<PB4);     // liga buzzer
        buzzer = millis();
        PORTB &= ~(1<<PB3);     // apaga dois pontos
        doisPOn = true;
      }
    }
  } else {
    PORTB |= (1<<PB3); // dois pontos acesos fixo em modo config
  }

  // ----- Controle de duração do buzzer -----
  if((millis() - buzzer) > 300 && buzzer != 0) {
    PORTB |= (1<<PB4); // desliga buzzer após 300ms
  }

  lastSW = SW;

  // Atualiza display
  pisca(5);
}