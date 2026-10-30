/*
 * cal.h
 *
 * Created: 2026-06-29 16:08:32
 *  Author: kccistc
 */ 

#define  F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

#define MAX_STACK_SIZE 100

typedef struct
{
	char *data[MAX_STACK_SIZE];
	int top;	//인덱스 번호
} stack;

stack s;

void save_equation(uint8_t key_val);
char **trans(stack* s, char* expr);
char *calculate(stack* s, char** postfix);
void init(stack* s);
int is_full(stack* s);
int is_empty(stack* s);
void push(stack* s, char *in_char);
char* pop(stack* s);
int getPriority(char op);
int parsing(char *buff);