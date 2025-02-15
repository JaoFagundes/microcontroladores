#include <xc.h>
#include <stdio.h>  // Biblioteca necessária para sprintf (conversão de float para string)
#define _XTAL_FREQ 20000000  // Define a frequência do oscilador para 20MHz

// CONFIGURAÇÕES DO MICROCONTROLADOR
#pragma config FOSC = HS        // Oscilador em modo HS (High Speed)
#pragma config WDTE = OFF       // Desabilita o Watchdog Timer
#pragma config PWRTE = OFF      // Desabilita o Power-up Timer
#pragma config BOREN = OFF      // Desabilita o Brown-out Reset
#pragma config LVP = OFF        // Desabilita Low-Voltage Programming
#pragma config CPD = OFF        // Desabilita proteção do código na EEPROM
#pragma config WRT = OFF        // Desabilita proteção de escrita na memória Flash
#pragma config CP = OFF         // Desabilita proteção do código na Flash

// Definições para controle do LCD
#define RS PORTEbits.RE0  // Pino de controle RS do LCD (Registro de Comando/Dados)
#define EN PORTEbits.RE1  // Pino de controle EN do LCD (Pulso de habilitação)

// Protótipos das funções
void lcd_command(unsigned char cmd);  
void lcd_data(unsigned char data);  
void lcd_initialise();  
void lcd_string(const char *str);  
void adc_initialise();  
unsigned int read_adc(unsigned char canal);  

// Buffer para armazenar o texto que será mostrado no LCD
char buffer[16];

void main() {
    // CONFIGURAÇÃO DOS PORTS
    TRISE = 0x00;  // Configura PORT E como saída (Controle do LCD)
    TRISD = 0x00;  // Configura PORT D como saída (Dados do LCD)
    TRISA = 0xFF;  // Configura PORT A como entrada (ADC)

    // Inicializa o LCD e o módulo ADC
    lcd_initialise();
    adc_initialise();

    // Define a posição do cursor na primeira linha e exibe o texto "Tensao:"
    lcd_command(0x80);  
    lcd_string("Tensao:");

    while (1) {
        // Lê o valor do ADC no canal AN3 (pino RA3)
        unsigned int adc_value = read_adc(3);

        // Converte o valor do ADC para tensão (0V a 5V)
        float voltage = (adc_value * 5.0f) / 1023.0f;

        // Posiciona o cursor na segunda linha do LCD
        lcd_command(0xC0);  

        // Formata a string com o valor da tensão em volts (com 2 casas decimais)
        sprintf(buffer, "V: %.2f V", voltage);

        // Exibe a tensão medida no LCD
        lcd_string(buffer);

        __delay_ms(500);  // Atualiza a leitura a cada 500ms
    }
}

/**
 * Configuração do módulo ADC (Conversor Analógico-Digital)
 */
void adc_initialise() {
    ADCON0 = 0b10011001;  // Configuração do ADC:
                          // - Bit 7 = 1 -> Habilita ADC
                          // - Bits 6-3 = 1001 -> Seleciona AN3 (RA3)
                          // - Bits 2-0 = 001 -> Frequência de conversão = Fosc/8

    ADCON1 = 0b10000010;  // Configuração do ADCON1:
                          // - Bit 7 = 1 -> Justificação à direita (o valor do ADC será de 10 bits)
                          // - Bits 6-5 = 00 -> Vref+ e Vref- conectados a Vdd e Vss (alimentação padrão)
                          // - Bits 4-0 = 00010 -> Apenas AN3 configurado como analógico
}

/**
 * Função para ler o valor do ADC em um canal específico.
 * 
 * @param canal O canal ADC (de 0 a 7) que será lido
 * @return Valor convertido (0 a 1023)
 */
unsigned int read_adc(unsigned char canal) {
    ADCON0 &= 0xC5;  // Limpa os bits de seleção do canal
    ADCON0 |= (canal << 3);  // Configura o canal desejado
    __delay_us(20);  // Pequeno atraso para estabilizar a leitura
    ADCON0bits.GO_nDONE = 1;  // Inicia a conversão A/D
    while (ADCON0bits.GO_nDONE);  // Aguarda a conversão ser concluída
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
    __delay_ms(2);  // Pequeno atraso após a limpeza
}

/**
 * Envia um comando para o LCD.
 * 
 * @param cmd Comando a ser enviado
 */
void lcd_command(unsigned char cmd) {
    PORTD = cmd;  // Envia o comando para o barramento do LCD
    RS = 0;  // Configura o LCD para modo de comando
    EN = 1;  // Pulso de ativação
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
    EN = 1;  // Pulso de ativação
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
