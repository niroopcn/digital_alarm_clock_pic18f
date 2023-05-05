#include <xc.h>
#include "i2c.h"
#include "main.h"
#include "clcd.h"
#include "ds1307.h"
#include "matrix_keypad.h"
#include "eeprom.h"

extern char toggle_dashboard, start_alarm;
extern unsigned char clock_reg[3];
extern unsigned char calender_reg[4];
extern unsigned char time[9];
extern unsigned char date[11];
extern unsigned char event[12];
extern unsigned char snooze[10];

void set_time_date(void)
{
	CLEAR_DISP_SCREEN;

	char exit_flag = 0;
	char pos = 0;

	while (!exit_flag)
	{
		clcd_print("SET TIME", LINE1(2));
		clcd_print("SET DATE", LINE2(2));

		if (!pos)
			clcd_putch(RIGHT_ARROW, LINE1(0));
		else
			clcd_putch(RIGHT_ARROW, LINE2(0));

		switch (read_switches(STATE_CHANGE))
		{
		case MK_SW1:
		{
			delay(6);
			// check for long press
			if (read_switches(LEVEL_CHANGE) == MK_SW1)
			{
				switch (pos)
				{
				case 0:
					set_time();
					break;
				case 1:
					set_date();
					break;
				}
			}
			else if (pos > 0)
			{
				CLEAR_DISP_SCREEN;
				pos--;
			}
		}
		break;
		case MK_SW2:
		{
			delay(6);
			// check for long press
			if (read_switches(LEVEL_CHANGE) == MK_SW2)
				exit_flag = 1;
			else if (pos < 1)
			{
				CLEAR_DISP_SCREEN;
				pos++;
			}
		}
		break;
		}
	}
	CLEAR_DISP_SCREEN;
}

void set_time(void)
{
	CLEAR_DISP_SCREEN;
	clcd_print("HH:MM:SS", LINE1(4));

	unsigned char dummy;

	/* Setting the CH bit of the RTC to Stop the Clock */
	dummy = read_ds1307(SEC_ADDR);
	write_ds1307(SEC_ADDR, dummy | 0x80);

	int hour = 0, min = 0, seconds = 0, field_select = 0;
	unsigned int blinker = 0;
	char time_buffer[12] = "00:00:00 AM";
	clcd_print(time_buffer, LINE2(4));
	char exit_flag = 0;

	while (!exit_flag)
	{

		/*Assign digits to time_buffer array*/
		time_buffer[0] = hour / 10 + '0';
		time_buffer[1] = hour % 10 + '0';
		time_buffer[3] = min / 10 + '0';
		time_buffer[4] = min % 10 + '0';
		time_buffer[6] = seconds / 10 + '0';
		time_buffer[7] = seconds % 10 + '0';

		/*clcd_putch(time_buffer[2], LINE2(6));
		clcd_putch(time_buffer[5], LINE2(9));*/

		if (blinker++ <= 1500)
		{
			switch (field_select)
			{
			case 0:
				clcd_putch(' ', LINE2(4));
				clcd_putch(' ', LINE2(5));
				break;
			case 1:
				clcd_putch(' ', LINE2(7));
				clcd_putch(' ', LINE2(8));
				break;
			case 2:
				clcd_putch(' ', LINE2(10));
				clcd_putch(' ', LINE2(11));
				break;
			case 3:
				clcd_putch(' ', LINE2(13));
				clcd_putch(' ', LINE2(14));
			}
		}
		else
		{
			if (blinker == 3000)
				blinker = 0;
			switch (field_select)
			{
			case 0:
				clcd_putch(time_buffer[0], LINE2(4));
				clcd_putch(time_buffer[1], LINE2(5));
				break;
			case 1:
				clcd_putch(time_buffer[3], LINE2(7));
				clcd_putch(time_buffer[4], LINE2(8));
				break;
			case 2:
				clcd_putch(time_buffer[6], LINE2(10));
				clcd_putch(time_buffer[7], LINE2(11));
				break;
			case 3:
				clcd_putch(time_buffer[9], LINE2(13));
				clcd_putch(time_buffer[10], LINE2(14));
				break;
			}
		}

		switch (read_switches(STATE_CHANGE))
		{
		case MK_SW1: /*Increment Time*/
			clcd_print(time_buffer, LINE2(4));
			field_select = (field_select + 1) % 4;
			break;
		case MK_SW2: /*Decrement Time*/
		{
			delay(6);
			// long press check
			if (read_switches(LEVEL_CHANGE) == MK_SW2)
				exit_flag = 1;
			else
			{
				switch (field_select)
				{
				case 0:
					hour = (hour + 1) % 12;
					break;
				case 1:
					min = (min + 1) % 60;
					break;
				case 2:
					seconds = (seconds + 1) % 60;
					break;
				case 3:
				{
					switch (time_buffer[9])
					{
					case 'A':
						time_buffer[9] = 'P';
						break;
					case 'P':
						time_buffer[9] = 'A';
						break;
					}
				}
				break;
				}
			}
		}
		break;
		}
	}
	CLEAR_DISP_SCREEN;

	/*Update Time and write to RTC*/
	static unsigned char clock_reg[3];

	clock_reg[0] = read_ds1307(HOUR_ADDR);
	clock_reg[1] = read_ds1307(MIN_ADDR);
	clock_reg[2] = read_ds1307(SEC_ADDR);

	/*Storing Values in BCD Form*/
	/*Update Hour - 0xCF to clear the AM/PM*/
	clock_reg[0] = (clock_reg[0] & 0xCF) | (hour / 10 << 4);

	if (time_buffer[9] == 'P')
		write_ds1307(HOUR_ADDR, clock_reg[0] | 1 << 5); /*PM(1) or AM(0)*/
	else
		write_ds1307(HOUR_ADDR, clock_reg[0]);

	clock_reg[0] = read_ds1307(HOUR_ADDR);

	write_ds1307(HOUR_ADDR, (clock_reg[0] & 0xF0) | (hour % 10));

	/*Update Minutes*/
	write_ds1307(MIN_ADDR, (clock_reg[1] & 0x0F) | (min / 10 << 4));
	write_ds1307(MIN_ADDR, (clock_reg[1] & 0xF0) | min % 10);

	/*Update Seconds*/
	write_ds1307(SEC_ADDR, (clock_reg[2] & 0x0F) | (seconds / 10 << 4));
	write_ds1307(SEC_ADDR, (clock_reg[2] & 0xF0) | seconds % 10);

	/* Clearing the CH bit of the RTC to Start the Clock */
	dummy = read_ds1307(SEC_ADDR);
	write_ds1307(SEC_ADDR, dummy & 0x7F);

	CLEAR_DISP_SCREEN;
	clcd_print("Time Updated!", LINE1(1));
	delay(8);
	CLEAR_DISP_SCREEN;
}

void set_date(void)
{
	CLEAR_DISP_SCREEN;
	calender_reg[0] = read_ds1307(YEAR_ADDR);
	calender_reg[1] = read_ds1307(MONTH_ADDR);
	calender_reg[2] = read_ds1307(DATE_ADDR);

	date[0] = '0' + ((calender_reg[2] >> 4) & 0x0F);
	date[1] = '0' + (calender_reg[2] & 0x0F);
	date[2] = '-';
	date[3] = '0' + ((calender_reg[1] >> 4) & 0x0F);
	date[4] = '0' + (calender_reg[1] & 0x0F);
	date[5] = '-';
	date[6] = '0' + ((calender_reg[0] >> 4) & 0x0F);
	date[7] = '0' + (calender_reg[0] & 0x0F);
	date[8] = '\0';

	clcd_print("DD:MM:YY", LINE1(4));

	int date = 0, month = 0, year = 0, field_select = 0;
	unsigned int blinker = 0;
	char date_buffer[9] = "00-00-00";
	clcd_print(date_buffer, LINE2(4));
	char exit_flag = 0;

	while (!exit_flag)
	{

		/*Assign digits to time_buffer array*/
		date_buffer[0] = date / 10 + '0';
		date_buffer[1] = date % 10 + '0';
		date_buffer[3] = month / 10 + '0';
		date_buffer[4] = month % 10 + '0';
		date_buffer[6] = year / 10 + '0';
		date_buffer[7] = year % 10 + '0';

		if (blinker++ <= 1500)
		{
			switch (field_select)
			{
			case 0:
				clcd_putch(' ', LINE2(4));
				clcd_putch(' ', LINE2(5));
				break;
			case 1:
				clcd_putch(' ', LINE2(7));
				clcd_putch(' ', LINE2(8));
				break;
			case 2:
				clcd_putch(' ', LINE2(10));
				clcd_putch(' ', LINE2(11));
				break;
			}
		}
		else
		{
			if (blinker >= 3000)
				blinker = 0;
			switch (field_select)
			{
			case 0:
				clcd_putch(date_buffer[0], LINE2(4));
				clcd_putch(date_buffer[1], LINE2(5));
				break;
			case 1:
				clcd_putch(date_buffer[3], LINE2(7));
				clcd_putch(date_buffer[4], LINE2(8));
				break;
			case 2:
				clcd_putch(date_buffer[6], LINE2(10));
				clcd_putch(date_buffer[7], LINE2(11));
				break;
			}
		}

		switch (read_switches(STATE_CHANGE))
		{
		case MK_SW1: /*Increment Time*/
			clcd_print(date_buffer, LINE2(4));
			field_select = (field_select + 1) % 3;
			break;
		case MK_SW2: /*Decrement Time*/
		{
			delay(6);
			// long press check
			if (read_switches(LEVEL_CHANGE) == MK_SW2)
				exit_flag = 1;
			else
			{
				switch (field_select)
				{
				case 0:
					date = (date + 1) % 32;
					break;
				case 1:
					month = (month + 1) % 13;
					break;
				case 2:
					year = (year + 1) % 100;
					break;
				}
			}
		}
		break;
		}
	}

	write_ds1307(YEAR_ADDR, (((year / 10) % 10) << 4) | year % 10);
	write_ds1307(MONTH_ADDR, (((month / 10) % 10) << 4) | month % 10);
	write_ds1307(DATE_ADDR, (((date / 10) % 10) << 4) | (date % 10));

	CLEAR_DISP_SCREEN;
}