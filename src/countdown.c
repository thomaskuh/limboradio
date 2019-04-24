#include <time.h>

#include "loggy.h"
#include "countdown.h"

char* _COUNTDOWN_LOGGY = "CountDown";

void countdown_init(struct countdown* cd, int* timerSeconds, void (*callback_start)(), void (*callback_stop)()) {
  loggy_info(_COUNTDOWN_LOGGY, "Initializing.");
	cd->active = 0;
	cd->ts_stop = 0;
	cd->seconds_left = 0;
  cd->seconds_left_ext = timerSeconds;
	cd->callback_start = callback_start;
	cd->callback_stop = callback_stop;
}

void countdown_check(struct countdown* cd) {
	if(cd->active) {
		cd->seconds_left = cd->ts_stop - (long)time(NULL);
    *cd->seconds_left_ext = cd->seconds_left;

    loggy_trace(_COUNTDOWN_LOGGY, "Checking. Active with seconds left: %ld.", cd->seconds_left);

		if(cd->seconds_left <= 0) {
      loggy_debug(_COUNTDOWN_LOGGY, "Stopping.");
			cd->active = 0;
			cd->ts_stop = 0;
			cd->seconds_left = 0;
      *cd->seconds_left_ext = 0;
			cd->callback_stop();
		}
	}
	else if(cd->ts_stop > 0) {
    loggy_debug(_COUNTDOWN_LOGGY, "Starting.");
		cd->active = 1;
		cd->callback_start();
	}
}

void countdown_set(struct countdown* cd, int seconds) {
  loggy_debug(_COUNTDOWN_LOGGY, "Setting to %d seconds.", seconds);
	cd->ts_stop = (long)time(NULL) + seconds;
}
