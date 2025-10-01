unsigned char d = 0;
long long int tempo_hz = 0;
int conta_hz = 0;
unsigned long lastDispRefresh = 0, lastSerialRefresh = 0;
unsigned char Tabela[] = {0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x80, 0x18, 0x06, 0x2F};
unsigned char todisp[3] = {10, 11, 11};
int aceleracao = 0;

char ch = 0;

volatile int sensorValue[] = {0, 0};
volatile int maxSensor[] = {833, 413};
volatile int minSensor[] = {190, 87};
volatile int outputValue[] = {0, 0};

void setup()
{
  DDRD |= 0b01111111;
  DDRB |= 0b00000111;

  DDRC &= ~(1 << PC0); // PC0 as input
  DDRC &= ~(1 << PC1); // PC1 as input

  ADMUX &= ~((1 << REFS0) | (1 << REFS1)); // tensao de referencia
  ADMUX |= (1 << REFS0);                   // AVCC

  ADCSRB = 0; // valor padrão

  ADCSRA &= 0b11111000;
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ADCCLK = CLK/128

  ADCSRA |= (1 << ADIE); // habilita interrupção
  ADCSRA |= (1 << ADEN); // habilita o ADC
  ADCSRA |= (1 << ADSC); // inicia a primeira conversão
}

void loop()
{

  delay(1); // Only for simulation
}

// busy waiting version
signed int ler_adc(unsigned char canal)
{
  ADMUX &= 0b11110000;
  ADMUX |= (0b00001111 & canal); // seleciona o canal ch no MUX
  DIDR0 = (1 << canal);

  ADCSRA |= (1 << ADSC); // inicia a conversão

  while (ADCSRA & (1 << ADSC))
    ; // espera a conversão ser finalizada

  return ADC;
}

// interrupt handler
ISR(ADC_vect)
{
  sensorValue[ch] = ADC;
  ch++;
  ch %= 2;

  ADMUX &= 0b11110000;
  ADMUX |= (0b00001111 & ch); // seleciona o canal ch no MUX
  DIDR0 = (1 << ch);

  ADCSRA |= (1 << ADSC); // inicia a conversão

  if (conta_hz <= 1000)
  {
    outputValue[0] = map(sensorValue[0], minSensor[0], maxSensor[0], 0, 100);
    outputValue[1] = map(sensorValue[1], minSensor[1], maxSensor[1], 0, 100);
    aceleracao = (int)(outputValue[0] + outputValue[1]) / 2;

    if (abs(outputValue[0] - outputValue[1]) > 10)
    {
      todisp[0] = 10;
      todisp[1] = 11;
      todisp[2] = 11;
    }
    else
    {
      todisp[0] = ((int)aceleracao / 100);
      todisp[1] = (aceleracao >= 10 && aceleracao != 100) ? ((int)aceleracao / 10) : (0);
      todisp[2] = (aceleracao <= 100) ? (aceleracao % 10) : (0);
    }
    conta_hz++;
  }

  long long int temp = millis();
  if (temp >= tempo_hz + 1000)
  {
    tempo_hz = temp;
    conta_hz = 0;
  }
  
  if (millis() > (lastDispRefresh))
    {
      lastDispRefresh = millis();
      d++;
      d %= 3;
    }

    PORTB = (PORTB & 0b11111000) | (1 << d); // ativa o display correspondente ao d

    PORTD = Tabela[todisp[d]];
}

/*
Multiplexar os displays dentro da ISR do ADC é viável porque:
- O ADC gera interrupções em uma frequência suficientemente alta (ordem de kHz), garantindo que os displays sejam chaveados rápido o bastante para evitar flick.
- O olho humano não percebe variações acima de 60 Hz, então uma taxa de 1 kHz assegura uma exibição contínua.
- Este código chaveia cada digito a aproximadamente 333hz, o que da próximo de 1Khz.
*/
