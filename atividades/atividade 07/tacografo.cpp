#include <stdio.h> // Biblioteca padrão C, usada aqui para a função sprintf

unsigned long lastLCDRefresh = 0; // Armazena o tempo (millis) da última atualização do LCD

#define DADOS_LCD PORTD // Define o PORTD como o registrador para os dados do LCD
#define nibble_dados 1 // 0 = 4 bits de dados nos pinos LSBs (PD0-PD3)
// 1 = 4 bits de dados nos pinos MSBs (PD4-PD7)
#define CONTR_LCD PORTB // Define o PORTB como o registrador para o controle do LCD
#define E PB4 // Pino Enable (E) do LCD conectado ao PINO PB4
#define RS PB3 // Pino Register Select (RS) do LCD conectado ao PINO PB3

//sinal de habilitação para o LCD
// Macro que gera o pulso de 'Enable' (E) necessário para o LCD ler os dados
#define pulso_enable() _delay_us(1); CONTR_LCD|=(1<<E); _delay_us(1); CONTR_LCD&=~(1<<E); _delay_us(45)

//protótipo das funções
void cmd_LCD(unsigned char c, char cd); // Protótipo da função de enviar comando/dado ao LCD
void inic_LCD_4bits(); // Protótipo da função de inicialização do LCD

//---------------------------------------------------------------------------------------------
// Sub-rotina para enviar caracteres e comandos ao LCD com via de dados de 4 bits
//---------------------------------------------------------------------------------------------
void cmd_LCD(unsigned char c, char cd) // c = dado (byte), cd = 0 para instrução, 1 para caractere
{
 if(cd==0)
 CONTR_LCD&=~(1<<RS); // Se for instrução (cd=0), coloca RS em NÍVEL BAIXO
 else
 CONTR_LCD|=(1<<RS); // Se for caractere (cd=1), coloca RS em NÍVEL ALTO

 //primeiro nibble de dados - 4 MSB (envia os 4 bits mais significativos do byte 'c')
 #if (nibble_dados) // Se nibble_dados == 1 (usando pinos PD4-PD7)
 DADOS_LCD = (DADOS_LCD & 0b00001111) | (0b11110000 & c); // Combina os 4 MSBs de 'c' com os 4 MSBs do PORTD, preservando os 4 LSBs do PORTD
 #else // Se nibble_dados == 0 (usando pinos PD0-PD3)
 DADOS_LCD = (DADOS_LCD & 0xF0)|(c>>4); // Desloca os 4 MSBs de 'c' para a direita e combina com os 4 LSBs do PORTD
 #endif
 
 pulso_enable(); // Pulsa 'Enable' para o LCD ler o primeiro nibble

 //segundo nibble de dados - 4 LSB (envia os 4 bits menos significativos do byte 'c')
 #if (nibble_dados) // Se nibble_dados == 1 (usando pinos PD4-PD7)
 DADOS_LCD = (DADOS_LCD & 0b00001111) | (0b11110000 & (c<<4)); // Desloca os 4 LSBs de 'c' para a esquerda e combina com os 4 MSBs do PORTD
 #else // Se nibble_dados == 0 (usando pinos PD0-PD3)
 DADOS_LCD = (DADOS_LCD & 0xF0) | (0x0F & c); // Combina os 4 LSBs de 'c' com os 4 LSBs do PORTD
 #endif
 
 pulso_enable(); // Pulsa 'Enable' para o LCD ler o segundo nibble
 
 if((cd==0) && (c<4)) // Se for instrução de "Clear Display" (0x01) ou "Return Home" (0x02)
 _delay_ms(2); // Espera 2ms, pois são comandos lentos
}
//---------------------------------------------------------------------------------------------
//Sub-rotina para inicialização do LCD com via de dados de 4 bits
//---------------------------------------------------------------------------------------------
void inic_LCD_4bits() // Sequência de inicialização baseada no datasheet do HD44780
{ // O pino R/W do LCD deve estar aterrado (sempre em modo escrita)

 CONTR_LCD&=~(1<<RS); // RS em zero (modo instrução)
 CONTR_LCD&=~(1<<E); // Pino de habilitação em zero
 
 _delay_ms(20); // Atraso inicial (20ms) para estabilização da tensão do LCD
 
 cmd_LCD(0x30,0); // 1º comando de inicialização (ainda em modo 8 bits)
 
 pulso_enable(); // Envia o pulso
 _delay_ms(5); // Atraso obrigatório de 5ms
 pulso_enable(); // Envia o 2º pulso
 _delay_us(200); // Atraso obrigatório de 200us
 pulso_enable(); // Envia o 3º pulso (finaliza a tentativa de 8 bits)
 
 //interface de 4 bits, deve ser enviado duas vezes (a outra está abaixo)
 cmd_LCD(0x20,0); // Comando para definir a interface de 4 bits
 
 pulso_enable(); 
 cmd_LCD(0x28,0); // Define interface de 4 bits, 2 linhas, fonte 5x8
 cmd_LCD(0x08,0); // Desliga o display (temporariamente)
 cmd_LCD(0x01,0); // Limpa todo o display
 cmd_LCD(0x0C,0); // Liga o display, desliga o cursor, cursor não pisca
 cmd_LCD(0x80,0); // Move o cursor para a primeira posição da primeira linha (Endereço 0x80)
}

// Função auxiliar para escrever uma string (array de caracteres) no LCD
void LCD_write(char *c){
 for (; *c!= 0; *c++) cmd_LCD(*c, 1); // Itera pela string até encontrar o caractere nulo (fim da string)
}
 
unsigned long long mymillis; // Variável global para debug (verificar se millis() está contando)

int debounce = 50; // Tempo de debounce para os botões (50 milissegundos)
int upd_scr = 100; // Intervalo de atualização da tela (100 milissegundos) - (Não usado, valor 100ms "hardcoded" no loop)

char line1[17]; // Buffer (array) para armazenar o texto da linha 1 (16 caracteres + 1 caractere nulo)
char line2[17]; // Buffer (array) para armazenar o texto da linha 2

long long last_upd = 0; // Armazena o tempo (millis) da última atualização de cálculo da frequência
int freq = 0; // Armazena o valor da frequência calculada (em Hz)
char prop = 1; // Armazena o número de hélices (padrão 1, máximo 9)

int last_prop; // Armazena o último estado lido do botão de hélice (para debounce)
long long last_prop_change = 0; // Armazena o tempo (millis) da última mudança do botão de hélice

bool rpm = false; // Flag para alternar entre Hz (false) e RPM (true)
int last_rpm; // Armazena o último estado lido do botão RPM (para debounce)
long long last_rpm_change; // Armazena o tempo (millis) da última mudança do botão RPM

long long time = 0; // Armazena o tempo (millis) do pulso ATUAL da interrupção
long long last_time = 0; // Armazena o tempo (millis) do pulso ANTERIOR da interrupção
int counter = 0; // Contador de pulsos (usado para cálculo de alta frequência)
 
void setup(){
 // lcd output
 DDRB |= 0b00011000; // Configura pinos PB3(RS) e PB4(E) como SAÍDA
 DDRD |= 0b11110000; // Configura pinos PD4, PD5, PD6, PD7 (Dados D4-D7) como SAÍDA
 
 // Configuração do Sensor (Pino PD2 / INT0)
 DDRD &= ~(1 << PD2); // Configura pino PD2 (SENSOR / INT0) como ENTRADA
 PORTD |= (1 << PD2); // Habilita resistor de PULL-UP interno no pino PD2

 // Configuração do Botão de Hélice (Pino PB0)
 DDRB &= ~(1 << PB0); // Configura pino PB0 (Botão HÉLICE) como ENTRADA
 PORTB |= (1 << PB0); // Habilita resistor de PULL-UP interno no pino PB0
 last_prop = PINB & (1 << PB0); // Lê o estado inicial do botão de hélice

 // Configuração do Botão RPM/Hz (Pino PB1)
 DDRB &= ~(1 << PB1); // Configura pino PB1 (Botão RPM/Hz) como ENTRADA
 PORTB |= (1 << PB1); // Habilita resistor de PULL-UP interno no pino PB1
 last_rpm = PINB & (1 << PB1); // Lê o estado inicial do botão RPM

 inic_LCD_4bits(); // Chama a rotina de inicialização do display LCD

 // Configuração da Interrupção Externa (INT0 no pino PD2)
 ADCSRA |= (1<<ADIE); // Habilita interrupção do ADC (Não parece ser usado neste código)
 ADCSRA |= (1<<ADEN); // Habilita o conversor Analógico-Digital (Não parece ser usado neste código)
 EIMSK |= (1 << INT0); // Habilita a Interrupção Externa 0 (INT0)
 EICRA |= (1 << ISC01) | (1 << ISC00); // Configura INT0 para disparar na BORDA DE SUBIDA (rising edge)
}

void loop(){ 
 read_rpm(); // Verifica se o botão RPM/Hz foi pressionado
 read_prop(); // Verifica se o botão de hélices foi pressionado
 update_freq(); // Calcula a frequência (ou Hz ou RPM)
 
 if (mymillis!=millis()) { // Bloco de verificação simples para debug do millis()
 mymillis=millis(); 
 }
 
 // Bloco de temporização não-bloqueante para atualizar o LCD
 if (millis()>(lastLCDRefresh+100)) { // Executa a cada 100ms
 lastLCDRefresh = millis(); // Atualiza o tempo da última atualização do LCD
 cmd_LCD(0x80, 0); // Move o cursor para o início da linha 1 (garante a posição)
 
 update_screen(); // Chama a função que atualiza o texto no display
 }

 delay(1); // Only for simulation (Pequeno delay, útil em simuladores, pode ser removido)
}

// Rotina de Serviço da Interrupção (ISR) para INT0 (Pino PD2)
ISR(INT0_vect){
 // Este código é executado AUTOMATICAMENTE a cada borda de subida no pino PD2
 counter++; // Incrementa o contador de pulsos
 last_time = time; // Salva o tempo (millis) do pulso anterior
 time = millis(); // Registra o tempo (millis) do pulso atual
}

// Função para formatar e exibir os dados no LCD
void update_screen(){
 reset_line1(); // Limpa a primeira linha para evitar "lixo" de dados anteriores

 double temp = freq; // Cria uma variável temporária com a frequência atual (em Hz)
 if(rpm) temp = hz_to_rpm(); // Se o modo RPM estiver ativo, converte o valor para RPM

 long temp_int = (long)temp; // Pega a parte inteira do valor (ex: 120.5 -> 120)
 long temp_dec = (long)(temp * 10) % 10; // Pega a primeira casa decimal (ex: 120.5 -> (1205 % 10) -> 5)

 if (rpm){ // Se modo RPM, formata a string da linha 1 para RPM
 sprintf(line1, " %d.%dRPM", temp_int, temp_dec); // (Nota: espaços no início para alinhar)
 }else{ // Senão, formata a string da linha 1 para Hz
 sprintf(line1, " %d.%dHz", temp_int, temp_dec);
 }

 sprintf(line2, "Helices: %d", prop); // Formata a string da linha 2 com o número de hélices

 cmd_LCD(0x80,0); // Move o cursor para o início da linha 1
 LCD_write(line1); // Escreve a string da frequência na linha 1
 cmd_LCD(0xC0,0); // Move o cursor para o início da linha 2 (Endereço 0xC0)
 LCD_write(line2); // Escreve a string das hélices na linha 2
}

// resta a primeira linha
void reset_line1(){
 char temp[] = " "; // Define uma string de espaços em branco (16 espaços)
 cmd_LCD(0x80,0); // Move o cursor para o início da linha 1
 LCD_write(temp); // Escreve os espaços em branco para "limpar" a linha
}

// converte hz para rpm
float hz_to_rpm(){
 return freq*60; // 1Hz = 60RPM (Frequência em Hz * 60 = Rotações por Minuto)
}

// le o botao de helice
void read_prop(){
 unsigned char prop_state = PINB & (1 << PB0); // Lê o estado ATUAL do botão de hélice (PB0)
 // Verifica se o estado mudou (last_prop != prop_state) E se o tempo de debounce passou
 if (last_prop != prop_state && millis() - last_prop_change > debounce) { 
 last_prop_change = millis(); // Registra o tempo desta mudança

 if (!prop_state) { // Se o botão foi pressionado (estado LOW, pois usa PULL-UP)
 prop++; // Incrementa o número de hélices
 if(prop > 9) prop = 1; // Se passar de 9, volta para 1
 }

 last_prop = prop_state; // Salva o estado atual como o último estado
 }
}

// le a frequencia (calcula a frequência)
bool update_freq() {
 // Executa este bloco a cada 100ms (definido no "if")
 if (millis() - last_upd > 100) { 
 last_upd = millis(); // Atualiza o tempo do último cálculo

 // Lógica dupla de cálculo de frequência:
 if(time - last_time > 10){ // Se o tempo entre pulsos for > 10ms (Baixa Frequência, < 100Hz)
 freq = 1000 / (time - last_time); // Calcula a frequência pelo período (1000ms / tempo_entre_pulsos)
 }else{ // Se o tempo entre pulsos for <= 10ms (Alta Frequência, >= 100Hz)
 freq = counter * 10; // Calcula a frequência pela contagem (pulsos_em_100ms * 10 = pulsos_por_segundo)
 }
 counter = 0; // Zera o contador de pulsos para a próxima janela de 100ms
 freq /= prop; // Divide a frequência lida pelo número de hélices (correção)
 }
}

// verifica se trocou o rpm
void read_rpm(){
 unsigned char rpm_state = PINB & (1 << PB1); // Lê o estado ATUAL do botão RPM/Hz (PB1)
 // Verifica se o estado mudou (last_rpm != rpm_state) E se o tempo de debounce passou
 if (last_rpm != rpm_state && millis() - last_rpm_change > debounce) { 
 last_rpm_change = millis(); // Registra o tempo desta mudança

 if (!rpm_state) { // Se o botão foi pressionado (estado LOW, pois usa PULL-UP)
 rpm = !rpm; // Inverte o modo (Hz -> RPM ou RPM -> Hz)
 }

 last_rpm = rpm_state; // Salva o estado atual como o último estado
 }
}