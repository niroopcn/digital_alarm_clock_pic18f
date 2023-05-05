#include <xc.h>
#include "i2c.h"
#include "main.h"
#include "clcd.h"
#include "ds1307.h"
#include "matrix_keypad.h"
#include "eeprom.h"
#include "timer0.h"

extern char toggle_dashboard, start_alarm;
extern unsigned char clock_reg[3];
extern unsigned char calender_reg[4];
extern unsigned char time[9];
extern unsigned char date[11];
extern unsigned char event[12];
extern unsigned char snooze[10];

void display_date(void)
{
	clcd_print("DATE ", LINE1(0));
	clcd_print(date, LINE1(5));
}

void display_time(void)
{
	clcd_print("TIME ", LINE2(0));
	clcd_print(time, LINE2(5));

	if (clock_reg[0] & 0x40) /*12 hr format*/
	{
		if (clock_reg[0] & 0x20) /*Check AM or PM*/
		{
			clcd_print("PM", LINE2(14));
		}
		else
		{
			clcd_print("AM", LINE2(14));
		}
	}
}

void get_time(void)
{
	clock_reg[0] = read_ds1307(HOUR_ADDR);
	clock_reg[1] = read_ds1307(MIN_ADDR);
	clock_reg[2] = read_ds1307(SEC_ADDR);

	if (clock_reg[0] & 0x40)
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x01);
		time[1] = '0' + (clock_reg[0] & 0x0F);
	}
	else
	{
		time[0] = '0' + ((clock_reg[0] >> 4) & 0x03);
		time[1] = '0' + (clock_reg[0] & 0x0F);
	}
	time[2] = ':';
	time[3] = '0' + ((clock_reg[1] >> 4) & 0x0F);
	time[4] = '0' + (clock_reg[1] & 0x0F);
	time[5] = ':';
	time[6] = '0' + ((clock_reg[2] >> 4) & 0x0F);
	time[7] = '0' + (clock_reg[2] & 0x0F);
	time[8] = '\0';
}

void get_date(void)
{
	calender_reg[0] = read_ds1307(YEAR_ADDR);
	calender_reg[1] = read_ds1307(MONTH_ADDR);
	calender_reg[2] = read_ds1307(DATE_ADDR);
	calender_reg[3] = read_ds1307(DAY_ADDR);

	date[0] = '0' + ((calender_reg[2] >> 4) & 0x0F);
	date[1] = '0' + (calender_reg[2] & 0x0F);
	date[2] = '-';
	date[3] = '0' + ((calender_reg[1] >> 4) & 0x0F);
	date[4] = '0' + (calender_reg[1] & 0x0F);
	date[5] = '-';
	date[6] = '2';
	date[7] = '0';
	date[8] = '0' + ((calender_reg[0] >> 4) & 0x0F);
	date[9] = '0' + (calender_reg[0] & 0x0F);
	date[10] = '\0';
}

void get_event(void)
{
	/*Hour*/
	event[0] = read_internal_eeprom(0x00);
	event[1] = read_internal_eeprom(0x01);
	event[2] = ':';

	/*Minutes*/
	event[3] = read_internal_eeprom(0x02);
	event[4] = read_internal_eeprom(0x03);
	event[5] = ' ';

	/*AM or PM*/
	event[6] = read_internal_eeprom(0x04);

	event[7] = 'M';

	event[8] = ' ';

	/*Trigger Once, Daily or Weekly*/
	event[9] = read_internal_eeprom(0x05);
	event[10] = '\0';
	event[11] = read_internal_eeprom(101); /*Read Day (Sunday - 1, Monday - 2)*/
}

void display_event(void)
{
	clcd_print("EVENT ", LINE1(0));
	clcd_print(event, LINE1(6));
}

void no_event_present(void)
{
	event[0] = 'N';
	event[1] = 'O';
	event[2] = 'T';
	event[3] = ' ';
	event[4] = 'S';
	event[5] = 'E';
	event[6] = 'T';
	event[7] = '\0';
	display_event();
}