#include <xc.h>
#include "timer1.h"

void init_timer1(void)
{
    /* Clearing timer1 overflow interrupt flag bit */
    TMR1IF = 0;

    /*For 16 bit timer, 1 sec delay, the count should be 62500 to get whole number
    the difference will be 3036*/
    TMR1 = 3036;

    /*Enable Timer 1*/
    TMR1IE = 1;

    /*Turn Timer1 ON*/
    TMR1ON  = 1;
}