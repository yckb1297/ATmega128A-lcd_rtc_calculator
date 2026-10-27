/*
 * text_lcd.c
 *
 * Created: 2026-07-01 9:59:21
 *  Author: kccistc
 */ 

#include "text_lcd.h"
#include "def.h"

#define TIMEKEEPER_REGISTER_SIZE 8
extern uint8_t timekeeper_register_data[TIMEKEEPER_REGISTER_SIZE-1];
extern int blink_timer;
extern int blink_flag;
extern ClockState state;
extern int edit_idx;

void init_lcd(void)
{
	_delay_ms(50);		//초기 구동 시간
	
	LCD_DATA_DDR |= 0xf0;	//PB4-7 출력으로 설정
	LCD_DATA_PORT = 0x00;	//data port low
	LCD_CONTROL_DDR |= 1 << RS_PIN | 1 << RW_PIN | 1 << E_PIN;	//제어핀 출력 설정
	LCD_CONTROL_PORT &= ~(1 << RW_PIN);		//r/w핀에 low 출력, 쓰기 전용으로 사용(r: 1, w: 0)
	
	lcd_write_command(0x02);	
	lcd_write_command(COMMAND_4_BIT_MODE);

	uint8_t command = 0x08 | (1 << COMMAND_DISPLAY_ON_OFF_BIT);		
	lcd_write_command(command);		//display on, cursor off, blink off
	
	lcd_clear();				//화면 지우기
	
	lcd_write_command(0x06);	//Entry mode set: 출력 후 커서를 오른쪽으로 옮김. DDRAM의 주소 증가, 화면 이동 X
	
	printf("init complete\n");
}

void lcd_pulse_enable(void)
{
	LCD_CONTROL_PORT |= (1 << E_PIN);	//E_pin: HIGH
	_delay_us(1);
	LCD_CONTROL_PORT &= ~(1 << E_PIN);	//E_pin: LOW, 하강 엣지에서 데이터 처리
	_delay_us(1);
	printf("pulse en\n");
}

void lcd_clear(void)
{
	lcd_write_command(COMMAND_CLEAR_DISPLAY);
	_delay_ms(2);
	printf("clear\n");
}

void lcd_write_command(uint8_t command)
{
	LCD_CONTROL_PORT &= ~(1 << RS_PIN);			//RS핀 0으로 설정, 0: 명령어, 1:데이터
	
	LCD_DATA_PORT = command & 0xf0;;		//상위니블 처리
	lcd_pulse_enable();
	LCD_DATA_PORT = (command << 4) & 0xf0;
	lcd_pulse_enable();
	
	_delay_ms(2);
	
	printf("command write: %d\n", command);
}

void lcd_write_data(uint8_t data)
{
	LCD_CONTROL_PORT |= 1 << RS_PIN;	//RS핀 1로 설정
	
	LCD_DATA_PORT = data & 0xf0;		//상위니블 처리
	lcd_pulse_enable();
	LCD_DATA_PORT = (data << 4) & 0xf0;		//하위니블 처리
	lcd_pulse_enable();
	
	_delay_ms(2);
	
	printf("data write: %d\n", data);
}

void lcd_write_string(char *string)
{
	for(uint8_t i = 0; string[i]; i++)
	{
		lcd_write_data(string[i]);
		printf("write_string: %c\n", string[i]);
	}
}

void lcd_goto_xy(uint8_t row, uint8_t col)
{
	col %= 16;	//0~15
	row %= 2;	//0~1
	
	//row1 시작 주소 0x00, row2 시작 주소 0x40
	uint8_t address = (0x40 * row) + col;
	uint8_t command = 0x80 + address;
	
	lcd_write_command(command);		//커서 이동
}

void lcd_write_dec_chars(uint8_t dec_data)
{
	uint8_t ten_char = (dec_data / 10) + '0';
	uint8_t one_char = (dec_data % 10) + '0';

	lcd_write_data(ten_char);
	lcd_write_data(one_char);
}

void lcd_timer(void)
{
	lcd_goto_xy(0, 0);
	
	lcd_write_data('2');
	lcd_write_data('0');
	
	if (state == PAUSED && edit_idx == 0 && blink_flag)
	{
		lcd_write_string("  "); // 깜박임 시 공백
	}
	else
	{
		lcd_write_dec_chars(timekeeper_register_data[0]);
	}
	lcd_write_data('-');
	
	if (state == PAUSED && edit_idx == 1 && blink_flag)
	{
		lcd_write_string("  "); // 깜박임 시 공백
	}
	else
	{
		lcd_write_dec_chars(timekeeper_register_data[1]);
	}
	lcd_write_data('-');
	
	if (state == PAUSED && edit_idx == 2 && blink_flag)
	{
		lcd_write_string("  "); // 깜박임 시 공백
	}
	else
	{
		lcd_write_dec_chars(timekeeper_register_data[2]);
	}
	lcd_write_data('-');   // date
	
	lcd_goto_xy(1,0);
	
	if (state == PAUSED && edit_idx == 4 && blink_flag)
	{
		lcd_write_string("  "); // 깜박임 시 공백
	}
	else
	{
		lcd_write_dec_chars(timekeeper_register_data[4]);
	}
	lcd_write_data(':');
	
	if (state == PAUSED && edit_idx == 5 && blink_flag)
	{
		lcd_write_string("  "); // 깜박임 시 공백
	}
	else
	{
		lcd_write_dec_chars(timekeeper_register_data[5]);
	}
	lcd_write_data(':');
	
	if (state == PAUSED && edit_idx == 6 && blink_flag)
	{
		lcd_write_string("  "); // 깜박임 시 공백
	}
	else
	{
		lcd_write_dec_chars(timekeeper_register_data[6]);
	}
}

/*
void lcd_timer(uint8_t data)
{
	lcd_clear();
	
 	_delay_ms(1000);
}
*/