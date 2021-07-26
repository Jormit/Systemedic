#define TIMER_DIVIDER 16

// Initialise selected hardware timer.
void timer_setup(int group, int timer);

// Set timer to 0 and start the timer counting.
void timer_restart(int group, int timer);

// Get amount of seconds since timer was restarted.
double timer_get_seconds(int group, int timer);