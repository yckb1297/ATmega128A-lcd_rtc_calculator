/*
 * i2c.h
 *
 * Created: 2026-07-02 9:42:40
 *  Author: kccistc
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#define I2C_SCL	0
#define I2C_SDA	1

// ----- TWI 상태 코드 -----
//TWSR(TWI Status Register): 7번~3번 bit(5bit를 참조): I2C 통신의 전송 상태나 오류를 나타냄.
#define TWSR_START 0x08				//START 조건 전송 완료
#define TWSR_REP_START 0x10   // TWSR_REP_START (0x10) - Repeated START
#define TWSR_MT_SLA_ACK 0x18		//Master가 Slave ADDR + W(0) 전송 후 ACK 수신
#define TWSR_MT_DATA_ACK 0x28		//Master가 Slave에 DATA를 전송 후 ACK 수신
#define TWSR_MR_SLA_ACK 0x40		//Master가 Slave ADDR + R 전송 후 ACK 수신
#define TWSR_MR_DATA_ACK 0x50		//Master가 DATA 수신 후 ACK 전송
#define TWSR_MR_DATA_NACK 0x58		//Master가 DATA 수신 후 NACK 전송

void init_i2c(void);
uint8_t i2c_receive_ACK(void);
uint8_t i2c_receive_NACK(void);
static void twi_wait(void);
uint8_t i2c_start(void);
void i2c_transmit(uint8_t data);
void i2c_stop(void);

