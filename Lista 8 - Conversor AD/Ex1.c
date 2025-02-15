#include <xc.h>
#include <stdio.h>  // Biblioteca necess�ria para sprintf (convers�o de float para string)
#define _XTAL_FREQ 20000000  // Define a frequ�ncia do oscilador para 20MHz

// CONFIGURA��ES DO MICROCONTROLADOR
#pragma config FOSC = HS        // Oscilador em modo HS (High Speed)
#pragma config WDTE = OFF       // Desabilita o Watchdog Timer
#pragma config PWRTE = OFF      // Desabilita o Power-up Timer
#pragma config BOREN = OFF      // Desabilita o Brown-out Reset
#pragma config LVP = OFF        // Desabilita Low-Voltage Programming
#pragma config CPD = OFF        // Desabilita prote��o do c�digo na EEPROM
#pragma config WRT = OFF        // Desabilita prote��o de escrita na mem�ria Flash
#pragma config CP = OFF         // Desabilita prote��o do c�digo na Flash

// Defini��es para controle do LCD
#define RS PORTEbits.RE0  // Pino de controle RS do LCD (Registro de Comando/Dados)
#define EN PORTEbits.RE1  // Pino de controle EN do LCD (Pulso de habilita��o)

// Prot�tipos das fun��es
void lcd_command(unsigned char cmd);  
void lcd_data(unsigned char data);  
void lcd_initialise();  
void lcd_string(const char *str);  
void adc_initialise();  
unsigned int read_adc(unsigned char canal);  

// Buffer para armazenar o texto que ser� mostrado no LCD
char buffer[16];

void main() {
    // CONFIGURA��O DOS PORTS
    TRISE = 0x00;  // Configura PORT E como sa�da (Controle do LCD)
    TRISD = 0x00;  // Configura PORT D como sa�da (Dados do LCD)
    TRISA = 0xFF;  // Configura PORT A como entrada (ADC)

    // Inicializa o LCD e o m�dulo ADC
    lcd_initialise();
    adc_initialise();

    // Define a posi��o do cursor na primeira linha e exibe o texto "Tensao:"
    lcd_command(0x80);  
    lcd_string("Tensao:");

    while (1) {
        // L� o valor do ADC no canal AN3 (pino RA3)
        unsigned int adc_value = read_adc(3);

        // Converte o valor do ADC para tens�o (0V a 5V)
        float voltage = (adc_value * 5.0f) / 1023.0f;

        // Posiciona o cursor na segunda linha do LCD
        lcd_command(0xC0);  

        // Formata a string com o valor da tens�o em volts (com 2 casas decimais)
        sprintf(buffer, "V: %.2f V", voltage);

        // Exibe a tens�o medida no LCD
        lcd_string(buffer);

        __delay_ms(500);  // Atualiza a leitura a cada 500ms
    }
}

/**
 * Configura��o do m�dulo ADC (Conversor Anal�gico-Digital)
 */
void adc_initialise() {
    ADCON0 = 0b10011001;  // Configura��o do ADC:
                          // - Bit 7 = 1 -> Habilita ADC
                          // - Bits 6-3 = 1001 -> Seleciona AN3 (RA3)
                          // - Bits 2-0 = 001 -> Frequ�ncia de convers�o = Fosc/8

    ADCON1 = 0b10000010;  // Configura��o do ADCON1:
                          // - Bit 7 = 1 -> Justifica��o � direita (o valor do ADC ser� de 10 bits)
                          // - Bits 6-5 = 00 -> Vref+ e Vref- conectados a Vdd e Vss (alimenta��o padr�o)
                          // - Bits 4-0 = 00010 -> Apenas AN3 configurado como anal�gico
}

/**
 * Fun��o para ler o valor do ADC em um canal espec�fico.
 * 
 * @param canal O canal ADC (de 0 a 7) que ser� lido
 * @return Valor convertido (0 a 1023)
 */
unsigned int read_adc(unsigned char canal) {
    ADCON0 &= 0xC5;  // Limpa os bits de sele��o do canal
    ADCON0 |= (canal << 3);  // Configura o canal desejado
    __delay_us(20);  // Pequeno atraso para estabilizar a leitura
    ADCON0bits.GO_nDONE = 1;  // Inicia a convers�o A/D
    while (ADCON0bits.GO_nDONE);  // Aguarda a convers�o ser conclu�da
    return ((ADRESH << 8) + ADRESL);  // Retorna os 10 bits combinados (ADRESH + ADRESL)
}

/**
 * Inicializa o LCD no modo 8 bits.
 */
void lcd_initialise() {
    lcd_command(0x38);  // Configura o LCD para modo 8 bits, 2 linhas
    lcd_command(0x0C);  // Liga o display, sem cursor piscante
    lcd_command(0x06);  // Define o modo de incremento do cursor
    lcd_command(0x01);  // Limpa o display
    __delay_ms(2);  // Pequeno atraso ap�s a limpeza
}

/**
 * Envia um comando para o LCD.
 * 
 * @param cmd Comando a ser enviado
 */
void lcd_command(unsigned char cmd) {
    PORTD = cmd;  // Envia o comando para o barramento do LCD
    RS = 0;  // Configura o LCD para modo de comando
    EN = 1;  // Pulso de ativa��o
    __delay_ms(2);
    EN = 0;  // Finaliza o pulso
}

/**
 * Envia um caractere para o LCD.
 * 
 * @param data Caractere a ser exibido no LCD
 */
void lcd_data(unsigned char data) {
    PORTD = data;  // Envia o dado para o barramento do LCD
    RS = 1;  // Configura o LCD para modo de dados
    EN = 1;  // Pulso de ativa��o
    __delay_ms(2);
    EN = 0;  // Finaliza o pulso
}

/**
 * Exibe uma string no LCD.
 * 
 * @param str Ponteiro para a string a ser exibida
 */
void lcd_string(const char *str) {
    while (*str) {
        lcd_data(*str++);  // Envia cada caractere da string para o LCD
    }
}
