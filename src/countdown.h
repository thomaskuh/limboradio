#ifndef _COUNTDOWN_H_
#define _COUNTDOWN_H_

struct countdown {
    int active;
    long ts_stop;
    int seconds_left;           // Seconds left
    int* seconds_left_ext;      // Seconds left stored somewhere else
    void (*callback_start)();
    void (*callback_stop)();
};

// Initialize a new counter with a place to save seconds left and 2 callbacks called on countdown start/stop.
void countdown_init(struct countdown* cd, int* timerSeconds, void (*callback_start)(), void (*callback_stop)());

// Call check looped/scheduled to check for timer start/stop.
// This triggers callback_start / callback_stop if required.
void countdown_check(struct countdown* cd);

// Set time to run or refresh if already running.
// This will NOT trigger callback_start/callback_stop directly.
void countdown_set(struct countdown* cd, int seconds);

#endif
