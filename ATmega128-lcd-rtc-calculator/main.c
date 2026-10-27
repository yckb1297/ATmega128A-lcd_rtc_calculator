/*
 * 15.LCD_RTC_CAL.c
 *
 * Created: 2026-07-01 9:35:02
 * Author : kccistc
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>	//interrupt 라이브러리. sei 등의 함수가 포함됨.
#include <stdio.h>

#include "queue.h"
#include "button.h"
#include "text_lcd.h"
#include "i2c.h"
#include "ds1307.h"
#include "keypad.h"
#include "def.h"

extern int ds1307main(void);

extern void insert_queue(uint8_t value);
extern uint8_t read_queue(void);

extern init_uart0(void);
extern void UART0_transmit(uint8_t data);

extern uint8_t rtc_sec, rtc_min, rtc_hour, rtc_day, rtc_date, rtc_month, rtc_year;
extern uint8_t timekeeper_register_data[TIMEKEEPER_REGISTER_SIZE-1];

void init_timer0(void);
void show_date_time(void);

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

uint8_t timekeeper_register_max[7] = { 99, 12, 31, 0, 23, 59, 59 };
uint8_t timekeeper_register_min[7] = { 0, 1, 1, 0, 0, 0, 0 };

uint8_t max_days_of_month[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	
SystemState current_state = STATE_RTC;
ClockState state = RUNNING;

char date_buff[16];
char time_buff[16];

volatile uint32_t btnms_count = 0;
volatile uint32_t msec_count = 0;
volatile uint32_t keypad_counter = 0;
volatile uint16_t ms_count = 0;
volatile uint16_t sec_count = 0;

int edit_idx = YEAR_IDX;              // 0:년, 1:월, 2:일, 4:시, 5:분, 6:초

static int blink_timer = 0;
int blink_flag = 0;

ISR(TIMER0_OVF_vect)
{	
	volatile uint8_t key_data = 0;
	
	TCNT0 = 6;	//TCNT0 6~256카운트, 250개(1ms) 카운트 후 INTERRUPT
	btnms_count++;
	msec_count++;
	ms_count++;
	if(ms_count >= 1000)
	{
		ms_count = 0;
		sec_count++;
	}
	if(++keypad_counter >= 60)
	{
		keypad_counter = 0;
		
		if(key_data = keypad_scan())	//keypad를 check해서 눌린 것이 있으면
		{	
			insert_queue(key_data);		//circular queue에 저장한다.
		}
	}
	
}

int main(void)
{
	static uint8_t prev_sec_count = 0;
	uint8_t key_value = 0;
	uint8_t button_val = 0;
	int button3_state = 0;
	
	init_button();
	init_timer0();
	init_uart0();
	init_keypad();
	init_lcd();
	init_i2c();
	init_ds1307();
	queue_init();
	
	stdout = &OUTPUT;	// printf가 동작 할 수 있도록 stdout을 설정.
	sei();	//전역(대문) interrupt 허용

	while (1)
	{
		uint8_t input_detected = 0;
		if(get_button(BUTTON4, BUTTON4PIN))
		{
			current_state = (current_state == STATE_RTC) ? STATE_CALC : STATE_RTC;
			lcd_clear();
			_delay_ms(100);
		}
		
		switch(current_state)
		{
			case STATE_RTC:
				if(get_button(BUTTON3, BUTTON3PIN))
				{
					if(state ==RUNNING)
					{
						state = PAUSED;
						blink_timer = 0;
						blink_flag = 0;
					}
					else
					{
						save_time();
						state = RUNNING;
						blink_flag = 0;
						lcd_timer();
					}
				}
				
				// [FSM] 보정 모드
				if (state == PAUSED)
				{
					blink_timer++;
					if(blink_timer > 7500)
					{
						blink_timer = 0;
						blink_flag = !blink_flag;
						lcd_timer();
					}
					// BTN2: 수정 항목 선택
					if (get_button(BUTTON2, BUTTON2PIN))
					{
						edit_idx = (edit_idx + 1) % 7;
						if(edit_idx == DAY_IDX) edit_idx = HOUR_IDX;
					}
					// BTN1: 값 증가
					else if (get_button(BUTTON1, BUTTON1PIN))
					{
						timekeeper_register_data[edit_idx]++;
						
						if(timekeeper_register_data[edit_idx] > timekeeper_register_max[edit_idx])
						{
							timekeeper_register_data[edit_idx] = timekeeper_register_min[edit_idx];
						}
						lcd_timer();
					}
					// BTN0: 값 감소
					else if (get_button(BUTTON0, BUTTON0PIN))
					{
						if (timekeeper_register_data[edit_idx] == timekeeper_register_min[edit_idx])
						{
							timekeeper_register_data[edit_idx] = timekeeper_register_max[edit_idx];
						}
						else
						{
							timekeeper_register_data[edit_idx]--;
						}
						lcd_timer();
					}
				}
				// [FSM] 시계 실행
				else
				{
					if (msec_count >= 200)
					{
						msec_count = 0;
						ds1307_read_time();
						lcd_timer();
					}
				}
			break;
			
			case STATE_CALC:
				input_detected = 0;
				if(queue_empty() != TRUE)
				{
					key_value = read_queue();
					save_equation(key_value);
					input_detected = 1;
				}
				
				if(get_button(BUTTON0, BUTTON0PIN))
				{
					button_val = '(';
					save_equation(button_val);
					input_detected = 1;
				}
				else if(get_button(BUTTON1, BUTTON1PIN))
				{
					button_val = ')';
					save_equation(button_val);
					input_detected = 1;
				}
				else if(get_button(BUTTON2, BUTTON2PIN))
				{
					button_val = 'c';
					save_equation(button_val);
					input_detected = 1;
				}
				else if(get_button(BUTTON3, BUTTON3PIN))
				{
					button_val = ' ';
					save_equation(button_val);
					input_detected = 1;
				}
				
				if(input_detected)
				{
					prev_sec_count = sec_count;
				}
				else
				{
					
					if(sec_count > prev_sec_count + 10)
					{
						current_state = STATE_RTC;
						prev_sec_count = sec_count;
						lcd_clear();
						_delay_ms(10);
					}
				}
			break;
		}
		
	}
}

void init_timer0(void)
{
	TCNT0 = 6;	//TCNT0 6~256카운트, 250개(1ms) 카운트 후 INTERRUPT
	TCCR0 &= ~(1<<CS02 | 1<<CS01 | 1 << CS00);	//TCCR0을 초기화. 쓰레기 값이 들어가는 것을 방지.
	TCCR0 |= 1<<CS02 | 0 << CS01 | 0 << CS00; 	//분주비 64로 설정(교재 표 13-1 참고) 0b00000100
	TIMSK |= 1<<TOIE0;	//TIMER0 Overflow interrupt 활성화
}


#if 0
int main(void)
{
	init_button();
	init_timer0();
	init_uart0();
	init_keypad();
	init_lcd();
	init_i2c();
	init_ds1307();
	
	_delay_ms(1000);
	
	stdout = &OUTPUT;	// printf가 동작 할 수 있도록 stdout을 설정.
	sei();	//전역(대문) interrupt 허용
	
	uint8_t key_value = 0;
	uint8_t button_val = 0;
	int button3_state = 0;
	

	while (1) 
    {	
		//show_date_time();
		
		if(queue_empty() != TRUE)
		{
			key_value = read_queue();
			printf("key_value: %c\n", key_value);
			save_equation(key_value);
		}
		
		if(get_button(BUTTON0, BUTTON0PIN))
		{
			button_val = '(';
			save_equation(button_val);
		}
		else if(get_button(BUTTON1, BUTTON1PIN))
		{
			button_val = ')';
			save_equation(button_val);
		}
		else if(get_button(BUTTON2, BUTTON2PIN))
		{
			button_val = 'c';
			save_equation(button_val);
		}
		else if(get_button(BUTTON3, BUTTON3PIN))
		{
			button3_state = !button3_state;
			lcd_clear();
		}
		
		if (button3_state == 1)
		{
			if (msec_count >= 200) // 1ms 인터럽트가 200번 쌓였을 때 (즉, 0.2초마다)
			{
				msec_count = 0;    // 카운터 초기화
				show_date_time();  // 시계 화면 갱신
			}
		}
    }
}

void show_date_time(void){
	
	ds1307_read_time(&rtc_sec, &rtc_min, &rtc_hour, &rtc_date, &rtc_month, &rtc_year);
	
	sprintf(date_buff, "20%02d-%02d-%02d", rtc_year, rtc_month, rtc_date);
	sprintf(time_buff, "%02d:%02d:%02d", rtc_hour, rtc_min, rtc_sec);
	
	lcd_goto_xy(0,3);
	lcd_write_string(date_buff);
	lcd_goto_xy(1,4);
	lcd_write_string(time_buff);
}
#endif

/*
1. timer0을 초기화
 - AVR에서 8bit timer 0/2 중에서 0번을 초기화한다.
 - 임베디드에서 가장 신경을 써야 할 부분이 초기화 하는 것임.
 - 초기화가 잘못 되면 이후가 다 꼬인다.
2. 8bit 가지고 1ms를 측정하는 timer/counter를 만들고자 한다.
 2.1. 분주비 설정 (64분주).
   - 16,000,000Hz / 64 = 250,000Hz
 2.2. 1주기가 잡아먹는 시간 계산.
   - T = 1/f = 1/250,000 = 0.000004sec = 4us = 0.004ms
 2.3. 8bit로 count하는 시간 계산. (8bit timer OV(Overflow)
   - 4us * 256 = 1024us = 1.024ms
   - 0.004ms * 250 = 0.001sec(1ms)
*/

