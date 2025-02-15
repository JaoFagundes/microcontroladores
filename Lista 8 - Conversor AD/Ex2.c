#include <xc.h>

// Configura��es do microcontrolador (Fuses)
#pragma config FOSC = HS       // Oscilador de alta velocidade (High Speed)
#pragma config WDTE = OFF      // Watchdog Timer desativado
#pragma config PWRTE = OFF     // Power-up Timer desativado
#pragma config CP = OFF        // Prote��o de c�digo desativada

#define _XTAL_FREQ 16000000    // Define a frequ�ncia do oscilador como 16 MHz

/**
 * Fun��o para inicializar o ADC.
 * Configura o m�dulo de Convers�o Anal�gica-Digital (ADC).
 */
void inicializar_ADC() {
    ADCON0 = 0b10011001;  // Configura��o do ADC:
                          // - Bit 7 = 1 -> Habilita ADC
                          // - Bits 6-3 = 1001 -> Seleciona AN3 (RA3)
                          // - Bits 2-0 = 001 -> Frequ�ncia de convers�o = Fosc/8

    ADCON1 = 0b10000010;  // Configura��o do ADCON1:
                          // - Bit 7 = 1 -> Justifica��o � direita (o valor do ADC ser� de 10 bits)
                          // - Bits 6-5 = 00 -> Vref+ e Vref- conectados a VDD e VSS (alimenta��o padr�o)
                          // - Bits 4-0 = 00010 -> Apenas AN3 configurado como anal�gico
}

/**
 * Fun��o para realizar a leitura do ADC em um canal espec�fico.
 * 
 * @param canal N�mero do canal ADC (0 a 7)
 * @return Valor lido pelo ADC (0 a 1023)
 */
unsigned int realizar_leitura_ADC(unsigned char canal) {
    ADCON0 &= 0xC5;               // Limpa os bits de sele��o do canal
    ADCON0 |= (canal << 3);       // Configura o canal desejado
    __delay_ms(2);                // Tempo para estabilizar a leitura
    ADCON0bits.GO_DONE = 1;       // Inicia a convers�o A/D
    while (ADCON0bits.GO_DONE);   // Aguarda a convers�o ser conclu�da
    return ((unsigned int) ADRESH << 8) | ADRESL; // Retorna os 10 bits combinados (ADRESH + ADRESL)
}

/**
 * Fun��o para configurar o PWM no pino RC2.
 */
void configurar_PWM() {
    CCP1CON = 0b00001100; // Configura CCP1 em modo PWM (PWM ativo)
    PR2 = 0xFF;           // Define o per�odo do PWM (valor m�ximo = 255)
    T2CON = 0b00000100;   // Habilita Timer2 com prescaler 1:1
    CCPR1L = 0x00;        // Inicializa Duty Cycle em 0
}

/**
 * Fun��o para ajustar o Duty Cycle do PWM com base no valor lido pelo ADC.
 * 
 * @param valor Valor do ADC (0 a 1023), que ser� convertido para Duty Cycle.
 */
void ajustar_duty_PWM(unsigned int valor) {
    valor &= 0x03FF;                 // Garante que o valor esteja entre 0 e 1023
    CCPR1L = (unsigned char)(valor >> 2);  // Carrega os 8 bits mais significativos
    CCP1CONbits.CCP1X = (valor >> 1) & 1;  // Ajusta o bit menos significativo
    CCP1CONbits.CCP1Y = valor & 1;         // Ajusta o segundo bit menos significativo
}

/**
 * Fun��o principal (Main).
 */
void main() {
    unsigned int leitura_ADC, duty_PWM;

    // CONFIGURA��O DOS PORTS
    TRISA = 0xFF;           // Configura PORTA como entrada (RA3 ser� usado para ADC)
    TRISCbits.TRISC2 = 0;   // Configura RC2 como sa�da (para sinal PWM)

    // Inicializa o ADC e o PWM
    inicializar_ADC();
    configurar_PWM();

    while (1) {
        // L� o valor do ADC no canal AN3 (RA3)
        leitura_ADC = realizar_leitura_ADC(3);

        // Ajusta o valor do PWM conforme a leitura do ADC
        duty_PWM = leitura_ADC & 0x03FF;   // Garante que esteja no intervalo correto (0 a 1023)
        ajustar_duty_PWM(duty_PWM);        // Atualiza o PWM com o novo Duty Cycle

        __delay_ms(5);  // Pequeno atraso para estabilidade
    }
}
