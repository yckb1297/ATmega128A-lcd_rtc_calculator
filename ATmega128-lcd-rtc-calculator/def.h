/*
 * def.h
 *
 * Created: 2026-07-02 14:31:38
 *  Author: kccistc
 */ 

typedef enum {RUNNING, PAUSED} ClockState;
typedef enum {STATE_RTC, STATE_CALC} SystemState;
	typedef enum { YEAR_IDX, MONTH_IDX, DATE_IDX, DAY_IDX, HOUR_IDX, MINUTE_IDX, SECOND_IDX} TimeKeeperRegisterIdx;
