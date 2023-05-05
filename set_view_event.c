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

void set_view_event(void)
{
	CLEAR_DISP_SCREEN;

	char exit_flag = 0;
	char pos = 0;

	while (!exit_flag)
	{
		clcd_print("SET EVENT", LINE1(2));
		clcd_print("VIEW EVENT", LINE2(2));
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
					set_event();
					break;
				case 1:
					view_event();
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

void set_event(void)
{
	CLEAR_DISP_SCREEN;

	snooze[0] = '0';
	snooze[1] = '0';
	snooze[2] = ':';
	snooze[3] = '0';
	snooze[4] = '0';
	snooze[5] = ':';
	snooze[6] = '0';
	snooze[7] = '0';
	snooze[8] = ' ';
	snooze[9] = 'O';

	get_time();
	get_date();

	time[5] = ' ';
	if (clock_reg[0] & 0x20) /*Check AM or PM*/
	{
		time[6] = 'P';
	}
	else
	{
		time[6] = 'A';
	}
	time[7] = 'M';
	time[8] = '\0';

	int hour = 0, min = 0, seconds = 0, field_select = 0;
	unsigned int blinker = 0;

	clcd_putch(DOWN_ARROW, LINE1(15));
	char exit_flag = 0;

	while (!exit_flag)
	{
		clcd_print("TIME-", LINE1(0));
		clcd_print("DUR -", LINE2(0));

		clcd_print(time, LINE1(6));
		clcd_print(snooze, LINE2(6));

		if (blinker++ <= 1500)
		{
			switch (field_select)
			{
			case 0:
				clcd_putch(' ', LINE1(6));
				clcd_putch(' ', LINE1(7));
				break;
			case 1:
				clcd_putch(' ', LINE1(9));
				clcd_putch(' ', LINE1(10));
				break;
			case 2:
				clcd_putch(' ', LINE1(12));
				break;
			case 3:
				clcd_putch(' ', LINE2(6));
				clcd_putch(' ', LINE2(7));
				break;
			case 4:
				clcd_putch(' ', LINE2(9));
				clcd_putch(' ', LINE2(10));
				break;
			case 5:
				clcd_putch(' ', LINE2(12));
				clcd_putch(' ', LINE2(13));
				break;
			}
		}
		else
		{
			if (blinker >= 3000)
				blinker = 0;
			switch (field_select)
			{
			case 0: /*Hours*/
				clcd_putch(time[0], LINE1(6));
				clcd_putch(time[1], LINE1(7));
				break;
			case 1: /*Minutes*/
				clcd_putch(time[3], LINE1(9));
				clcd_putch(time[4], LINE1(10));
				break;
			case 2: /*AM or PM*/
				clcd_putch(time[6], LINE1(12));
				clcd_putch('M', LINE1(13));
				break;
			case 3:
				clcd_putch(snooze[0], LINE2(6));
				clcd_putch(snooze[1], LINE2(7));
				break;
			case 4:
				clcd_putch(snooze[3], LINE2(9));
				clcd_putch(snooze[4], LINE2(10));
				break;
			case 5:
				clcd_putch(snooze[6], LINE2(12));
				clcd_putch(snooze[7], LINE2(13));
				break;
			}
		}

		switch (read_switches(STATE_CHANGE))
		{
		case MK_SW1: /*Change Field*/
			clcd_print(time, LINE1(6));
			field_select = (field_select + 1) % 6;
			break;
		case MK_SW2: /*Increment & Set Time/Duration*/
		{
			delay(6);
			// long press check
			if (read_switches(LEVEL_CHANGE) == MK_SW2)
			{
				/*Store event time and duration/snooze in EEPROM*/
				write_internal_eeprom(0x00, time[0]);
				write_internal_eeprom(0x01, time[1]);
				write_internal_eeprom(0x02, time[3]);
				write_internal_eeprom(0x03, time[4]);
				write_internal_eeprom(0x04, time[6]); /*AM or PM*/

				/*0x05 stores Trigger(Once, Daily, Weekly)*/
				write_internal_eeprom(0x05, snooze[9]);

				write_internal_eeprom(0x06, snooze[0]);
				write_internal_eeprom(0x07, snooze[1]);
				write_internal_eeprom(0x08, snooze[3]);
				write_internal_eeprom(0x09, snooze[4]);
				write_internal_eeprom(0x0A, snooze[6]);
				write_internal_eeprom(0x0B, snooze[7]);

				write_internal_eeprom(100, 0x69);			 /*Event Set Flag*/
				write_internal_eeprom(101, calender_reg[3]); /*Storing Day (Sunday - 1, Monday - 2, etc)*/

				exit_flag = 1;
			}
			else
			{
				switch (field_select)
				{
				case 0:
				{
					hour = ((time[0] - '0') * 10) + (time[1] - '0');
					hour = (hour + 1) % 12;
					time[0] = ((hour / 10) % 10) + '0';
					time[1] = (hour % 10) + '0';
				}
				break;
				case 1:
				{
					min = ((time[3] - '0') * 10) + (time[4] - '0');
					min = (min + 1) % 60;
					time[3] = ((min / 10) % 10) + '0';
					time[4] = (min % 10) + '0';
				}
				break;
				case 2:
				{
					switch (time[6])
					{
					case 'A':
						time[6] = 'P';
						break;
					case 'P':
						time[6] = 'A';
						break;
					}
					break;
				}
				case 3:
				{
					hour = ((snooze[0] - '0') * 10) + (snooze[1] - '0');
					hour = (hour + 1) % 12;
					snooze[0] = ((hour / 10) % 10) + '0';
					snooze[1] = (hour % 10) + '0';
				}
				break;
				case 4:
				{
					min = ((snooze[3] - '0') * 10) + (snooze[4] - '0');
					min = (min + 1) % 60;
					snooze[3] = ((min / 10) % 10) + '0';
					snooze[4] = (min % 10) + '0';
				}
				break;
				case 5:
				{
					seconds = ((snooze[6] - '0') * 10) + (snooze[7] - '0');
					seconds = (seconds + 1) % 60;
					snooze[6] = ((seconds / 10) % 10) + '0';
					snooze[7] = (seconds % 10) + '0';
				}
				break;
				}
			}
		}
		break;
		case MK_SW3: /*Change Trigger (Once, Daily, Weekly)*/
		{
			switch (snooze[9])
			{
			case 'O':
				snooze[9] = 'D';
				break;
			case 'D':
				snooze[9] = 'W';
				break;
			case 'W':
				snooze[9] = 'O';
				break;
			}
			break;
		}
		}
	}
	CLEAR_DISP_SCREEN;
	clcd_print("EVENT SET", LINE1(3));
	clcd_print("SUCCESSFULLY!", LINE2(1));
	delay(8);
	CLEAR_DISP_SCREEN;
}

void view_event(void)
{
	CLEAR_DISP_SCREEN;

	if (read_internal_eeprom(100) == 0x69)
	{
		clcd_print("TIME- ", LINE1(0));
		clcd_print("DUR - ", LINE2(0));

		/*Display Event time and duration/snooze in EEPROM*/
		clcd_putch(read_internal_eeprom(0x00), LINE1(6));
		clcd_putch(read_internal_eeprom(0x01), LINE1(7));
		clcd_putch(':', LINE1(8));
		clcd_putch(read_internal_eeprom(0x02), LINE1(9));
		clcd_putch(read_internal_eeprom(0x03), LINE1(10));
		clcd_putch(' ', LINE1(11));
		clcd_putch(read_internal_eeprom(0x04), LINE1(12));
		clcd_putch('M', LINE1(13));
		clcd_putch(' ', LINE1(14));
		clcd_putch(' ', LINE1(15));

		clcd_putch(read_internal_eeprom(0x06), LINE2(6));
		clcd_putch(read_internal_eeprom(0x07), LINE2(7));
		clcd_putch(':', LINE1(8));
		clcd_putch(read_internal_eeprom(0x08), LINE2(9));
		clcd_putch(read_internal_eeprom(0x09), LINE2(10));
		clcd_putch(':', LINE1(11));
		clcd_putch(read_internal_eeprom(0x0A), LINE2(12));
		clcd_putch(read_internal_eeprom(0x0B), LINE2(13));
		clcd_putch(' ', LINE1(14));
		/*0x05 stores Trigger(Once, Daily, Weekly)*/
		clcd_putch(read_internal_eeprom(0x05), LINE2(15));

		unsigned char exit_flag = 0, key;
		while (!exit_flag)
		{
			key = read_switches(STATE_CHANGE);
			if (key == MK_SW1 || key == MK_SW2)
			{
				delay(6);
				// check long press
				key = read_switches(LEVEL_CHANGE);
				if (key == MK_SW1 || key == MK_SW2)
					exit_flag = 1;
			}
		}
	}
	else
		clcd_print("EVENT NOT SET", LINE1(1));

	delay(8);

	CLEAR_DISP_SCREEN;
}