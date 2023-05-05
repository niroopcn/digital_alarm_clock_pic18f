#include <xc.h>
#include "i2c.h"
#include "main.h"
#include "clcd.h"
#include "ds1307.h"
#include "matrix_keypad.h"
#include "eeprom.h"
#include "timer0.h"
#include "timer1.h"

char toggle_dashboard, start_alarm;
unsigned char clock_reg[3];
unsigned char calender_reg[4];
unsigned char time[9];
unsigned char date[11];
unsigned char event[12];
unsigned char snooze[10];

static void init_config(void)
{
	init_clcd();
	init_matrix_keypad();
	init_i2c();
	init_ds1307();
	TRISE0 = 0; /*Set Buzzer as Output*/
	init_timer0();
	TMR1IF = 0; /*Clear TMR1 Flag - just in case*/
	PEIE = 1;
	GIE = 1;
}

void main(void)
{
	init_config();

	while (1)
	{
		switch (read_switches(STATE_CHANGE))
		{
		case MK_SW1:
		{
			/*Disable Timer0 for toggling dashboard*/
			TMR0ON = 0;
			TMR0IE = 0;
			TMR0IF = 0;
			PEIE = 0;
			GIE = 0;

			go_to_menu();

			/*Enable Timer0 for toggling dashboard*/
			init_timer0();
			PEIE = 1;
			GIE = 1;
		}
		break;
		default:
		{
			get_time();
			display_time();

			/*Toggle Dashboard*/
			switch (toggle_dashboard)
			{
			case 0:
			{
				get_date();
				display_date();
			}
			break;
			case 1:
			{
				/*If event is set, load event, else display event not present*/
				if (read_internal_eeprom(100) == 0x69)
				{
					get_event();
					display_event();
					if (check_for_event() == 0)
					{
						/*Disable Timer0 for toggling dashboard*/
						TMR0ON = 0;
						TMR0IE = 0;
						TMR0IF = 0;

						init_timer1(); /*Turn ON Timer1 for 60 Seconds*/

						turn_on_alarm();	/*Turn ON Alarm for 60 seconds*/

						/*Disable Timer1*/
						TMR1ON = 0;
						TMR1IE = 0;
						TMR1IF = 0;
						
						/*Enable Timer0 for toggling dashboard*/
						init_timer0();
					}
				}
				else
					no_event_present();
			}
			break;
			}
		}
		break;
		}
	}
}

void go_to_menu(void)
{
	CLEAR_DISP_SCREEN;

	char exit_flag = 0;
	char pos = 0;

	while (!exit_flag)
	{
		clcd_print("SET/VIEW EVENT", LINE1(2));
		clcd_print("SET TIME/DATE", LINE2(2));

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
					set_view_event();
					break;
				case 1:
					set_time_date();
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

void delay(int ms)
{
	for (int i = 0; i < ms; i++)
	{
		for (unsigned int wait = 50000; wait--;)
			;
	}
}