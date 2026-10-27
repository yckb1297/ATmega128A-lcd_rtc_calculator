/*
 * uart.c
 *
 * Created: 2026-06-16 오전 10:33:06
 *  Author: kccistc
 */ 


/*
 * uart0.c
 *
 * Created: 2026-06-16 오전 9:57:59
 *  Author: user
 */ 
#include "uart0.h"

void init_uart0(void);
void UART0_transmit(uint8_t data);
void pc_comand_processing(void);

// P278 표12-3
// PC로부터 1byte가 들어 오면 자동적으로 이곳으로 진입 한다. 
// 예) led_all_on\n 이면 11번 이곳으로 진입 한다. 
ISR(USART0_RX_vect)
{
	volatile uint8_t data;
	volatile static int i=0;
	
	data = UDR0;  // UDR0의 내용이 data에 복사된후 UDR0의 내용은 빈 상태로 된다.
	
	if (data == '\n' || data == '\r')
	{
		if ( (u_rear+1) % QUEUE_SIZE == u_front % QUEUE_SIZE) {
			return;   // queue full 상태
		}
		rx_buff[u_front][i] = '\0';   // 문장의 끝인 NULL을 넣는다 
		i=0;  // 다음 string을 저장 하기 위해서 i를 0으로 만든다 
		u_rear = (u_rear + 1) % QUEUE_SIZE;  // 0~9
	}
	else
	{
		if ( (u_rear+1) % QUEUE_SIZE == u_front % QUEUE_SIZE) {
			return;   // queue full 상태
		}
		rx_buff[u_rear][i++]  = data;
	}
}
/*
1. 전송속도 : 9600bps 
2. start/stop 설정
3. RX(수신) : interrupt 로 설정 
*/
void init_uart0(void)
{
	// 1. 전송속도 : 9600bps 
	UBRR0H = 0x00;
	UBRR0L = 207;   //9600BPS 표8-9
	UCSR0A |= 1 << U2X0;  // 2배속 설정 (sampling 8)
	// UART0 를 송신.수신이 다 가능하고 RX INT가 가능 하도록 설정 한다.
	UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;   
}

// UART0 로 1byte를 전송 하는 함수 
void UART0_transmit(uint8_t data)
{
	while ( !(UCSR0A & 1 << UDRE0))   // data가 송신중이면 송신이 끝날때 까지 기다림
		;     // No operation
	UDR0 = data;   // HW 전송 register에 data를 송신 한다. 
}


void pc_comand_processing(void)
{
	if (u_front != u_rear)    // data가 rx_buff에 존재 하는지 check   
	{
		printf("%s", rx_buff[u_front]);  // printf("%s", &rx_buff[front][0])
		if (strncmp((char *) rx_buff[u_front], "led_shift_left_on", strlen("led_shift_left_on")) == 0)
		{
			;
		} 
		else if (strncmp((char *) rx_buff[u_front], "led_shift_right_on", strlen("led_shift_right_on")) == 0)
		{
			;
		}
		u_front = (u_front + 1) % QUEUE_SIZE;
	}
}