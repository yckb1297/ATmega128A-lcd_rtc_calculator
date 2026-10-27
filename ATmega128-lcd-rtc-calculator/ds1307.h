/*
 * ds1307.h
 *
 * Created: 2026-07-02 9:05:50
 *  Author: kccistc
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#define DS1307_DDR	DDRD
#define DS1307_PORT PORTD

#define DS1302_DAT_PIN	PIND

#define DS1307_SCL	0
#define DS1307_SDA	1

#define RTC_ADDR 0x68

#define TIMEKEEPER_REGISTER_SIZE   8


// ----- TWI 상태 코드 -----
//TWSR(TWI Status Register): 7번~3번 bit(5bit를 참조): I2C 통신의 전송 상태나 오류를 나타냄.
#define TWSR_START 0x08				//START 조건 전송 완료
#define TWSR_REP_START 0x10   // TWSR_REP_START (0x10) - Repeated START
#define TWSR_MT_SLA_ACK 0x18		//Master가 Slave ADDR + W(0) 전송 후 ACK 수신
#define TWSR_MT_DATA_ACK 0x28		//Master가 Slave에 DATA를 전송 후 ACK 수신
#define TWSR_MR_SLA_ACK 0x40		//Master가 Slave ADDR + R 전송 후 ACK 수신
#define TWSR_MR_DATA_ACK 0x50		//Master가 DATA 수신 후 ACK 전송
#define TWSR_MR_DATA_NACK 0x58		//Master가 DATA 수신 후 NACK 전송

void init_ds1307(void);
uint8_t dec2bcd(uint8_t data);
uint8_t bcd2dec(uint8_t data);
void ds1307_set_time(void);
void ds1307_read_time(void);
void save_time(void);
void lcd_timer(void);
