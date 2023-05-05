#ifndef MAIN_H
#define MAIN_H

#define RIGHT_ARROW 0x7E
#define DOWN_ARROW 0x7D

static void init_config(void);
void delay(int ms);
void get_time(void);
void get_date(void);
void display_time(void);
void display_date(void);
void go_to_menu(void);
void set_view_event(void);
void set_time_date(void);
void get_event(void);
void display_event(void);
void no_event_present(void);
void set_event(void);
void view_event(void);
void set_time(void);
int check_for_event(void);
void set_date(void);
void turn_on_alarm(void);
void software_pwm(void);

#endif
