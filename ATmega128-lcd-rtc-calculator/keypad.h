/*
 * keypad.h
 *
 * Created: 2026-06-29 14:15:07
 *  Author: kccistc
 */ 

#define  F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define KEYPAD_DDR DDRA
#define KEYPAD_PIN PINA
#define KEYPAD_PORT PORTA


void init_keypad(void);
uint8_t keypad_scan(void);
uint8_t get_key_button(int row, int col);
