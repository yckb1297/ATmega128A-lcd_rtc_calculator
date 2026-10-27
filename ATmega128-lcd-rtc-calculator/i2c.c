/*
 * i2c.c
 *
 * Created: 2026-07-02 9:44:53
 *  Author: kccistc
 */ 

#include "i2c.h"

void init_i2c(void)
{
	DDRD |= 1 << I2C_SCL;	//SCL, SDA 출력으로 설정
	DDRD |= 1 << I2C_SDA;
	
	TWSR = 0x00;	//TWS(상태코드)비트, TWPS비트(분주비 = 1) 초기화
	TWBR = 32;		//I2C 클록 주파수 설정 200kHz
	printf("init\n");
}

uint8_t i2c_receive_ACK(void)
{
	TWCR = 1 << TWINT | 1 << TWEN | 1<< TWEA;
	twi_wait();
	
	return TWDR;
}

uint8_t i2c_receive_NACK(void)
{
	TWCR = 1 << TWINT | 1 << TWEN;
	twi_wait();
	
	return TWDR;
}
static void twi_wait(void)
{
	while (!(TWCR & (1 << TWINT)));
	printf("wait\n");
}

uint8_t i2c_start(void)
{
	TWCR = 1 << TWINT | 1 << TWSTA | 1 << TWEN;		//INT flag, START sig, HW Enable
	twi_wait();
	
	uint8_t status = TWSR & 0xf8;	//상태코드 bit7~3 masking해서 추출
	return(status == TWSR_START || status == TWSR_REP_START) ? 0 : status; //성공: 0, 실패: status 반환
}


void i2c_stop(void)
{
	TWCR = 1 << TWINT | 1 << TWSTO | 1 << TWEN;		//INT flag, STOP sig, HW Enable
	printf("stop\n");
}

/* 데이터 바이트 전송 */
uint8_t i2c_write(uint8_t data)
{
	TWDR = data;
	TWCR = 1 << TWINT | 1 << TWEN;
	twi_wait();
	return TWSR & 0xF8;
}

/* SLA+W, SLA+R 전송 */
uint8_t i2c_slave_addr_send(uint8_t addr_rw)
{
	TWDR = addr_rw;
	TWCR = 1 << TWINT | 1 << TWEN | 0 << TWSTA;
	twi_wait();
	return TWSR & 0xF8;
}

uint8_t i2c_read_ack(void)
{
	TWCR = 1 << TWINT | 1 << TWEN | 1 << TWEA;  // TWEA=1  ACK 자동전송
	twi_wait();
	return TWDR;
}

/* 데이터 바이트 수신 –> NACK 전송 (Slave에게 종료 알림) */
uint8_t i2c_read_nack(void)
{
	TWCR = 1 << TWINT | 1 << TWEN;               // TWEA=0 : NACK 전송
	twi_wait();
	return TWDR;
}