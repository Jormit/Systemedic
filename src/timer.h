#define TIMER_DIVIDER 16

void timer_setup(int group, int timer);
void timer_restart(int group, int timer);
double timer_get_seconds(int group, int timer);