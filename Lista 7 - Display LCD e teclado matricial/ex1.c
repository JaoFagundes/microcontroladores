/*
 * File:   teclado_lcd.c
 * Author: joaop
 *
 * Descri��o:
 * - O programa exibe no LCD a tecla pressionada do teclado matricial 4x4.
 * - O LCD exibe os caracteres pressionados da esquerda para a direita.
 * - Quando a primeira linha (16 caracteres) for preenchida, o cursor vai para a segunda linha.
 * - Quando a segunda linha for preenchida, a pr�xima tecla limpa o display e reinicia a exibi��o.
 */

#include <xc.h>
#include <stdio.h> // Biblioteca necess�ria para sprintf
#define _XTAL_FREQ 20000000  // Define a frequ�ncia do oscilador para 20MHz

// CONFIGURA��O DO MICROCONTROLADOR
#pragma config FOSC = HS        // Oscilador em modo HS (High Speed)
#pragma config WDTE = OFF       // Desabilita o Watchdog Timer
#pragma config PWRTE = OFF      // Desabilita o Power-up Timer
#pragma config BOREN = OFF      // Desabilita o Brown-out Reset
#pragma config LVP = OFF        // Desabilita Low-Voltage Programming
#pragma config CPD = OFF        // Desabilita prote��o do c�digo na EEPROM
#pragma config WRT = OFF        // Desabilita prote��o de escrita na mem�ria Flash
#pragma config CP = OFF         // Desabilita prote��o do c�digo na Flash

// Defini��o dos pinos do LCD
#define RS PORTEbits.RE0  // Pino de controle do LCD (Registro de Comando/Dados)
#define EN PORTEbits.RE1  // Pino de controle do LCD (Pulso de habilita��o)

// Defini��o dos pinos do teclado matricial 4x4 (linhas e colunas)
#define c0 PORTCbits.RC0
#define c1 PORTCbits.RC1
#define c2 PORTCbits.RC2
#define c3 PORTCbits.RC3
#define b0 PORTBbits.RB0
#define b1 PORTBbits.RB1
#define b2 PORTBbits.RB2
#define b3 PORTBbits.RB3

unsigned char cursor_pos = 0x80; // Posi��o inicial do cursor no LCD (primeira linha)

/**
 * Fun��o para enviar um caractere ao LCD.
 * 
 * @param data Caractere a ser enviado ao LCD
 */
void lcd_data(unsigned char data) {
    PORTD = data;  // Envia o dado para o barramento do LCD
    RS = 1;  // Configura o LCD para modo de dados
    EN = 1;  // Pulso de ativa��o
    __delay_ms(5);
    EN = 0;  // Finaliza o pulso
}

/**
 * Fun��o para enviar um comando ao LCD.
 * Exemplos: Limpar tela, mover cursor, configurar display.
 * 
 * @param cmd C�digo do comando a ser enviado
 */
void lcd_command(unsigned char cmd) {
    PORTD = cmd;  // Envia o comando para o barramento do LCD
    RS = 0;  // Configura o LCD para modo de comando
    EN = 1;  // Pulso de ativa��o
    __delay_ms(5);
    EN = 0;  // Finaliza o pulso
}

/**
 * Fun��o para exibir uma string no LCD.
 * 
 * @param str Ponteiro para a string a ser exibida
 */
void lcd_string(const char *str) {
    while (*str) {
        lcd_data(*str++);  // Envia cada caractere para o LCD
    }
}

/**
 * Fun��o para inicializar o LCD no modo 8 bits.
 */
void lcd_initialise() {
    lcd_command(0x38);  // Configura��o: Modo 8 bits, 2 linhas
    lcd_command(0x06);  // Cursor avan�a automaticamente
    lcd_command(0x0C);  // Display ligado, cursor desligado
    lcd_command(0x01);  // Limpa display
}

/**
 * Fun��o para evitar leituras erradas no bot�o (debounce).
 */
void debounce() {
    __delay_ms(50);  // Delay de 50ms para evitar falsos acionamentos
}

/**
 * Fun��o para detectar qual tecla foi pressionada no teclado matricial.
 * 
 * @return Retorna o n�mero da tecla pressionada (0-15) ou 0xFF se nenhuma tecla foi pressionada.
 */
unsigned char teclado() {
    unsigned char tecla = 0xFF;  // Inicializa como "nenhuma tecla pressionada"

    // Verifica a primeira coluna
    c0 = 0; c1 = 1; c2 = 1; c3 = 1;
    if (b0 == 0) tecla = 0;
    if (b1 == 0) tecla = 1;
    if (b2 == 0) tecla = 2;
    if (b3 == 0) tecla = 3;

    // Verifica a segunda coluna
    c0 = 1; c1 = 0; c2 = 1; c3 = 1;
    if (b0 == 0) tecla = 4;
    if (b1 == 0) tecla = 5;
    if (b2 == 0) tecla = 6;
    if (b3 == 0) tecla = 7;

    // Verifica a terceira coluna
    c0 = 1; c1 = 1; c2 = 0; c3 = 1;
    if (b0 == 0) tecla = 8;
    if (b1 == 0) tecla = 9;
    if (b2 == 0) tecla = 10;
    if (b3 == 0) tecla = 11;

    // Verifica a quarta coluna
    c0 = 1; c1 = 1; c2 = 1; c3 = 0;
    if (b0 == 0) tecla = 12;
    if (b1 == 0) tecla = 13;
    if (b2 == 0) tecla = 14;
    if (b3 == 0) tecla = 15;

    return tecla;
}

/**
 * Atualiza o LCD com a tecla pressionada e controla a posi��o do cursor.
 * 
 * @param tecla C�digo da tecla pressionada
 */
void atualiza_lcd(unsigned char tecla) {
    if (cursor_pos == 0xD0) {  // Se as duas linhas estiverem preenchidas (16x2)
        lcd_command(0x01);  // Limpa o display
        cursor_pos = 0x80;  // Reinicia a posi��o do cursor
        return;  // Sai da fun��o para aguardar o pr�ximo clique
    }

    lcd_command(cursor_pos);  // Define a posi��o do cursor no LCD

    if (tecla < 10) {  // Teclas 0-9 (n�meros)
        lcd_data(tecla + '0');  // Converte o n�mero em caractere ASCII
    } else {  // Teclas 10-15 (A-F)
        lcd_data(tecla - 10 + 'A');  // Converte o n�mero em letra (A-F)
    }

    cursor_pos++;  // Move o cursor para a pr�xima posi��o

    if (cursor_pos == 0x90) {  // Se chegou ao final da primeira linha
        cursor_pos = 0xC0;  // Muda para o in�cio da segunda linha
    }
}

/**
 * Fun��o principal do programa.
 */
void main(void) {
    // CONFIGURA��O DOS PINOS
    TRISE = 0x00;  
    TRISD = 0x00;  
    TRISC = 0x00;  
    TRISB = 0xFF;  

    lcd_initialise();  // Inicializa o LCD

    while (1) {  // Loop infinito
        unsigned char tecla = teclado();  

        if (tecla != 0xFF) {  
            debounce();  
            atualiza_lcd(tecla);  
            while (teclado() != 0xFF);  
        }
    }
}
