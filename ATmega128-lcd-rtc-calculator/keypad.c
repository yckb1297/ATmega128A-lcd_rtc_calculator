/*
 * keypad.c
 *
 * Created: 2026-06-29 14:11:47
 *  Author: kccistc
 */ 

#include "keypad.h"


void init_keypad(void)
{
	KEYPAD_DDR = 0x0f;	// row: 입력, col: 출력
	KEYPAD_PORT = 0xff;
}

uint8_t keypad_scan(void)		//60ms마다 버튼이 눌렸는지 확인해서 data 반환
{
	uint8_t data = 0;	//key값 저장 변수
	
	for(int row = 0; row < 4; row++)
	{
		for(int col = 0; col <4; col++)
		{
			data = get_key_button(row, col);
			if(data)	// if data >= 1
			{
				return data;
			}
		}
	}
	
}

uint8_t get_key_button(int row, int col)
{
	uint8_t keypad_char[4][4] = {
		{'7', '8', '9', '='},
		{'4', '5', '6', '/'},
		{'1', '2', '3', '-'},
		{' ', '0', '+', '*'}
	};
	//ACTIVE-LOW로 동작
	static uint8_t prev_state[4][4] =		//초기 버튼 상태는 눌리지 않은 상태로 한다.
	{
		{ 1, 1, 1, 1},
		{ 1, 1, 1, 1},
		{ 1, 1, 1, 1},
		{ 1, 1, 1, 1}
	};
	
	uint8_t current_state = 1;
	KEYPAD_PORT = 0xff;
	KEYPAD_PORT &= ~(1 << 3-col);				//3-col: col4가 PA0에 연결되어 있으므로 해당 col에 전류를 흘린다
	
	for(int delay = 0; delay < 20; delay++)		//keypad check를 위한 delay
		;
		
	current_state = (KEYPAD_PIN & (1 << (row+4))) >> (row+4);
		//예) row 0: PA4에 연결
		// 76543210
		// 11101111		KEYPAD_PIN & (1 << (row+4))
		// 00001110		>> (row+4)
		
	//버튼 눌렸는지 확인	
	if(current_state == 0 && prev_state[row][col] == 1)		//처음 눌려진 상태
	{
		prev_state[row][col] = 0;
		return 0;
	}
	else if(current_state == 1 && prev_state[row][col] == 0)	//눌렸다 떼어진 상태
	{	// 이전에 버튼이 눌려지고 현재는 버튼을 뗀 상태이면 버튼을 1번 눌렀다 뗀 것으로 인정
		prev_state[row][col] = 1;	//prev_state를 초기화
		//printf("key: %c\n", keypad_char[row][col]);
		return keypad_char[row][col];
	}
	return 0;
}