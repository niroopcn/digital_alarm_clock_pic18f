#include <xc.h>
#include "clcd.h"

extern char toggle_dashboard, start_alarm;
unsigned int count, alarm_count;

void interrupt isr()
{
    if (TMR0IF)
    {
        TMR0 = TMR0 + 8;

        // for 5 seconds with ticks as 250, and prescaler 1:4 the count is 25000
        // for 2 seconds, count is 10000, here I'm making this event occur after 5 seconds
        // so, 25000 + 10000 = 35000

        switch (++count)
        {
        case 25000:
        {
            toggle_dashboard = !toggle_dashboard;
            CLEAR_DISP_SCREEN;
        }
        break;
        case 35000:
        {
            toggle_dashboard = !toggle_dashboard;
            count = 0;
            CLEAR_DISP_SCREEN;
        }
        break;
        }

        TMR0IF = 0;
    }
    
    if (TMR1IF) // 16 bit timer
    {
        TMR1 = TMR1 + 3038; // preset ticks to 3038 + TMR1
        /*Alarm should ring for 60 seconds, 80 counts = 1 sec, so 4800 counts = 60 seconds*/
        if (++alarm_count >= 4800)
        {
            start_alarm = !start_alarm;
        }
        TMR1IF = 0;
    }
}
