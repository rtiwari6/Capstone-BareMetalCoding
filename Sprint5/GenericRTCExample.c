#include "mss_rtc.h"

int main(void)
{
    
    mss_rtc_init(MSS_RTC_BINARY_MODE);    
    mss_rtc_set_time_count(0);    
    mss_rtc_start();

    while (1)
    {
        //write code to show time on display
    }

    return 0;
}
