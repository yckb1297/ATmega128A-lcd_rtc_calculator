/*
 * ds1307.c
 *
 * Created: 2026-07-02 9:05:39
 *  Author: kccistc
 */ 

#include "ds1307.h"
#include "i2c.h"

#define TIMEKEEPER_REGISTER_SIZE 8

uint8_t timekeeper_register_data[TIMEKEEPER_REGISTER_SIZE-1] = { 0 };

uint8_t rtc_sec, rtc_min, rtc_hour, rtc_day, rtc_date, rtc_month, rtc_year;

uint8_t initial_time_arr[7] = {
	0x00,      // Second => oscillator 활성화를 위한 CH 비트 0으로 클리어 (주소 0x00 bit 7)
	0x10,      // Minute
	0x17,      // Hour
	0x04,      // Day
	0x01,      // Date
	0x07,      // Month
	0x26       // Year
};

void init_ds1307(void)
{
	/* Preserve the battery-backed time on normal startup. */
	ds1307_read_time();
}

void ds1307_set_time(void)
{
	i2c_start();	//i2c 시작
	i2c_slave_addr_send(RTC_ADDR << 1);	//m->s, slave(rtc) 주소 전송, 쓰기모드
	i2c_slave_addr_send(0);	//rtc에 데이터를 기록할 메모리 시작 주소 전송
	
	for(int i = 0; i < 7; i++)
	{
		i2c_slave_addr_send(initial_time_arr[i]);
	}
	
	i2c_stop();
}

void ds1307_read_time(void)
{
	volatile uint8_t dummy;
	
	i2c_start();
	i2c_slave_addr_send(RTC_ADDR << 1);
	i2c_write(0x00);
	
	i2c_start();
	i2c_slave_addr_send((RTC_ADDR << 1)+ 1);
	
	for(int i = TIMEKEEPER_REGISTER_SIZE-1-1;i > 0;i--)
	{
		timekeeper_register_data[i] = bcd2dec(i2c_read_ack());
	}
	timekeeper_register_data[0] = bcd2dec(i2c_read_nack());
	
	i2c_stop();
	_delay_ms(5);
}

void save_time(void)
{
	i2c_start();
	i2c_slave_addr_send(RTC_ADDR << 1);
	i2c_write(0x00);

	i2c_write(dec2bcd(timekeeper_register_data[6])); // Sec
	i2c_write(dec2bcd(timekeeper_register_data[5])); // Min
	i2c_write(dec2bcd(timekeeper_register_data[4])); // Hour
	i2c_write(0x01);                              // Day
	i2c_write(dec2bcd(timekeeper_register_data[2])); // Date
	i2c_write(dec2bcd(timekeeper_register_data[1])); // Month
	i2c_write(dec2bcd(timekeeper_register_data[0])); // Year
	
	i2c_stop();
}

uint8_t dec2bcd(uint8_t data)	//dec -> bcd
{
	return (((data / 10) << 4) + (data % 10));	
}

uint8_t bcd2dec(uint8_t data)	//bcd -> dec
{
	return (((data >> 4) * 10) + (data & 0x0f));	
}