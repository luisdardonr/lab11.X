/* 
 * File:   lab11.c
 * Author: dellG3
 *
 * Created on 11 de mayo de 2022, 04:54 PM
 */
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
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

#include <xc.h>
#include <stdint.h>  // Para poder usar los int de 8 bits
#include <stdio.h>

#define _XTAL_FREQ 1000000
#define FLAG_SPI 0xFF

uint8_t poten;

void setup(void);

void __interrupt() isr (void){
    
    
    if (PIR1bits.ADIF){
        if (ADCON0bits.CHS == 1){
         
                
            poten = ADRESH;         
        }
            PIR1bits.ADIF = 0;          
    }
    return;
}


void main(void) {
  
    setup();
    
    while(1){
        if (ADCON0bits.GO == 0){    
            ADCON0bits.GO = 1;      
        }
        // cambio en el selector (SS) para generar respuesta del pic
        PORTAbits.RA6 = 1;      // Deshabilitamos el ss del esclavo
        __delay_ms(40);         // Delay para que el PIC pueda detectar el cambio en el pin
        PORTAbits.RA7 = 0;      // habilitamos nuevamente el escalvo
        __delay_ms(40);         // Delay para que el PIC pueda detectar el cambio en el pin
        // Enviamos el dato 0x55 
        SSPBUF = poten;   // Cargamos valor del contador al buffer
        while(!SSPSTATbits.BF){}// Esperamos a que termine el envio
        
        

        // cambio en el selector (SS) para generar respuesta del pic
        PORTAbits.RA7 = 1;      // Deshabilitamos el ss del esclavo
        __delay_ms(40);         // Delay para que el PIC pueda detectar el cambio en el pin
        PORTAbits.RA6 = 0;      // habilitamos nuevamente el escalvo
        __delay_ms(40);         // Delay para que el PIC pueda detectar el cambio en el pin
        SSPBUF = FLAG_SPI;      // Master inicia la comunicación y prende el clock
        while(!SSPSTATbits.BF){}// Esperamos a que se reciba un dato
        PORTD = SSPBUF;         // Mostramos dato recibido en PORTD

        __delay_ms(1000);       // Enviamos y pedimos datos cada 1 segundo
    }
    return;
}


void setup(void){
    ANSEL = 0b00000010;
    ANSELH = 0;                 // I/O digitales
    
    OSCCONbits.IRCF = 0b100;    // 1MHz
    OSCCONbits.SCS = 1;         // Reloj interno
    
    TRISA = 0b00000010;         // SS y RA0 como entradas
    
    PORTA = 0;
    
    TRISD = 0;
    PORTD = 0;

    // Configuración de SPI
    // Configs de Maestro
    
        
    TRISC = 0b00010000;         // -> SDI entrada, SCK y SD0 como salida
    PORTC = 0;
    
    // SSPCON <5:0>
    SSPCONbits.SSPM = 0b0000;   // -> SPI Maestro, Reloj -> Fosc/4 (250kbits/s)
    SSPCONbits.CKP = 0;         // -> Reloj inactivo en 0
    SSPCONbits.SSPEN = 1;       // -> Habilitamos pines de SPI
    // SSPSTAT<7:6>
    SSPSTATbits.CKE = 1;        // -> Dato enviado cada flanco de subida
    SSPSTATbits.SMP = 1;        // -> Dato al final del pulso de reloj
    SSPBUF = poten;              // Enviamos un dato inicial

    // Configuración ADC
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    ADCON1bits.VCFG0 = 0;       // VDD
    ADCON1bits.VCFG1 = 0;       // VSS
    ADCON0bits.CHS = 0b0000;    // Seleccionamos el AN0
    ADCON0bits.CHS = 0b0001;    // Seleccionamos el AN1
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON0bits.ADON = 1;        // Habilitamos modulo ADC
    __delay_us(40);             // Sample time

    // Configuracion interrupciones
    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    INTCONbits.PEIE = 1;          // Habilitamos int. de perifericos
    INTCONbits.GIE = 1;         // Habilitamos int. globales

}
