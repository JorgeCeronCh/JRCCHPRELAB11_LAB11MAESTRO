/*
 * File:   PreLab11.c
 * Author: jorge
 *
 * Created on 10 de mayo de 2022, 12:25 PM
 */
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)
// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>             // int8_t, unit8_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _XTAL_FREQ 1000000

int val_pot = 0;                  // Variable valor de potenciometro

void __interrupt() isr (void){
    if(PIR1bits.ADIF){                  // Interrupción por ADC
        if(ADCON0bits.CHS == 0){        // Interrupción por AN0
            val_pot = ADRESH;           // Valor del ADRESH al valor del potenciometro
            
        if(PORTAbits.RA5){              // Maestro
            if(SSPSTATbits.BF){         // Se revisa que no exista comuncacion en ese momento
                SSPBUF = val_pot;       // Se mueve el valor del potenciometro al SSPBUF
            }
        }
        }
        PIR1bits.ADIF = 0;              // Limpiar la bandera de ADC
    }
    if(PIR1bits.SSPIF){                 // Interrupcion si el esclavo recibió la información
        PORTD = SSPBUF;                 // Valor del SSPBUF al PORTD del esclavo para mostrarlo
        PIR1bits.SSPIF = 0;             // Limpiar la bandera de SPI
    }
    return;
}

void setup(void){
    ANSEL = 0b00000001;         // AN0 entrada analógica        
    ANSELH = 0;                 // I/O digitales
    
    TRISA = 0b00100001;         // RA0 Y RA5 como entradas
    PORTA = 0;                  // Se limpia PORTA

    // Configuración del oscilador
    OSCCONbits.IRCF = 0b0100;   // 1 MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuración ADC
    ADCON0bits.ADCS = 0b01;         // FOSC/8
    ADCON1bits.VCFG0 = 0;           // VDD
    ADCON1bits.VCFG1 = 0;           // VSS
    ADCON0bits.CHS = 0b0000;        // Selecciona el AN0
    ADCON1bits.ADFM = 0;            // Justificador a la izquierda
    ADCON0bits.ADON = 1;            // Habilitar modulo ADC
    __delay_us(40);                // Sample time
    
     // Configuración de interrupciones
    INTCONbits.GIE = 1;         // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitar interrupciones de periféricos
    PIR1bits.ADIF = 0;          // Limpiar la bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitar interrupciones de ADC
    
    // Configuración de Maestro
    if(PORTAbits.RA5){
        TRISC = 0b00010000;         // SD1 entrada, SCK y SD0 como salida
        PORTC = 0;                  // Se limpia PORTC
        // Configuración de SPI
        SSPCONbits.SSPM = 0b0000;   // SPI Maestro, Reloj -> FOSC/4  (250 KBIS/s)
        SSPCONbits.CKP = 0;         // Reloj inactivo
        SSPCONbits.SSPEN = 1;       // Habilitar pines de SPI
        SSPSTATbits.CKE = 1;        // Dato enviado cada flanco de subida
        SSPSTATbits.SMP = 1;        // Dato al final del pulso de reloj
        SSPBUF = val_pot;
    }
    else{
        TRISC = 0b00011000;         // SD1 salida, SCK y SD0 como entrada
        PORTC = 0;                  // Se limpia PORTC
        
        TRISD = 0;                  // PORTD como salida
        PORTD = 0;                  // Se limpia PORTD
        
        // Configuración de SPI
        SSPCONbits.SSPM = 0b0100;   // SPI Esclavo, SS habilitado
        SSPCONbits.CKP = 0;         // Reloj inactivo
        SSPCONbits.SSPEN = 1;       // Habilitar pines de SPI
        SSPSTATbits.CKE = 1;        // Dato enviado cada flanco de subida
        SSPSTATbits.SMP = 0;        // Dato al final del pulso de reloj
        
        PIR1bits.SSPIF = 0;         // Limpiar bandera de SPI
        PIE1bits.SSPIE = 1;         // Habilitar interrupcion de SPI
        INTCONbits.GIE = 1;         // Habilitar interrupciones globales
        INTCONbits.PEIE = 1;        // Habilitar interrupciones de periféricos
    }           
    
}

void main(void) {
    setup();
    while(1){
        if(ADCON0bits.GO == 0){
            ADCON0bits.GO = 1;
        }
    }
    return;
}
