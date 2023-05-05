#include <xc.h>
#include "main.h"
#include "clcd.h"
#include "matrix_keypad.h"
#include "eeprom.h"

extern char toggle_dashboard, start_alarm;
extern unsigned char clock_reg[3];
extern unsigned char calender_reg[4];
extern unsigned char time[9];
extern unsigned char date[11];
extern unsigned char event[12];
extern unsigned char snooze[10];

int check_for_event(void)
{
	get_date();
	get_event();

	switch (event[9]) /*Check Trigger Mode (Daily, Weekly, Once)*/
	{
	case 'W':
	{
		if (calender_reg[3] != event[11]) /*If day of week doesn't match, then break out of switch*/
			break;
	}
	default:
	{
		if (((clock_reg[0] & 0x20) && (event[6] == 'P')) || (!(clock_reg[0] & 0x20) && (event[6] == 'A')))
		{
			if ((time[4] == event[4]) && (time[3] == event[3]) && (time[1] == event[1]) && (time[0] == event[0]))
			{
				/*Reset Event if Trigger is 'O' - Once*/
				if (event[9] != 'D')
					write_internal_eeprom(100, 0);
				return 0;
			}
		}
	}
	break;
	}
	return 69;
}

void turn_on_alarm(void)
{
	CLEAR_DISP_SCREEN;
	display_event();
	clcd_print("ALARM!! WAKE UP!", LINE2(0));
	while (!start_alarm)
	{
		software_pwm();
		switch (read_switches(STATE_CHANGE))
		{
		case MK_SW1:
		{
			/*Disable Timer1*/
			TMR1ON = 0;
			TMR1IE = 0;
			TMR1IF = 0;

			start_alarm = 1;
		}
		break;
		}
	}
	CLEAR_DISP_SCREEN;
}

#define PERIOD 100
unsigned char program_cycle;

void software_pwm(void)
{
	static unsigned char duty_change = 30;

	if (program_cycle < duty_change)
		PORTEbits.RE0 = 1; /*Turn ON Buzzer*/
	else
		PORTEbits.RE0 = 0; /*Turn OFF Buzzer*/

	if (++program_cycle == PERIOD)
		program_cycle = 0;
}