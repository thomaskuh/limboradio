#include "easympd.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <mpd/client.h>
#include <mpd/status.h>
#include <mpd/entity.h>
#include <mpd/search.h>
#include <mpd/tag.h>
#include <mpd/message.h>

#include "loggy.h"

struct mpd_connection *easympd_int_conn;
struct mpd_status *easympd_int_status;
struct mpd_song *easympd_int_song;

int easympd_int_debug;

// synchronize mpd client access
pthread_mutex_t mpd_mutex;

char* _EASYMPD_LOGGY = "Mpd";

static int easympd_int_handle_error() {
	if(mpd_connection_get_error(easympd_int_conn) == MPD_ERROR_SUCCESS)
		return 0;

	loggy_error(_EASYMPD_LOGGY, "Error: %s.", mpd_connection_get_error_message(easympd_int_conn));
	// mpd_connection_free(c);
	return 1;
}

static int easympd_int_handle_rc(int rc) {
	if(!rc) {
		loggy_error(_EASYMPD_LOGGY, "Command failed with RC %d.", rc);
		easympd_int_handle_error();
	}
}

int easympd_setup() {
	loggy_info(_EASYMPD_LOGGY, "Initializing and connecting to localhost.");

	pthread_mutex_init (&mpd_mutex, NULL);
	while(1) {
		easympd_int_conn = mpd_connection_new("localhost", 0, 60000);
		if(easympd_int_handle_error()) {
			loggy_info(_EASYMPD_LOGGY, "Connection failed. Trying again in a sec.");
			sleep(2);
		}
		else {
			loggy_info(_EASYMPD_LOGGY, "Connected.");
			break;
		}
  }
	return 0;
}

void easympd_volume(int volume) {
	loggy_debug(_EASYMPD_LOGGY, "Voluming %d.", volume);
	pthread_mutex_lock (&mpd_mutex);
	easympd_int_handle_rc(mpd_run_set_volume(easympd_int_conn, volume));
	pthread_mutex_unlock (&mpd_mutex);
}

int easympd_int_playing() {
	int result = 0;
	easympd_int_status = mpd_run_status(easympd_int_conn);
	if(easympd_int_status == NULL)
		easympd_int_handle_error();
	else {
		result = (MPD_STATE_PLAY == mpd_status_get_state(easympd_int_status));
		mpd_status_free(easympd_int_status);
	}
	return result;
}

void easympd_earl(const char * earl) {
	loggy_debug(_EASYMPD_LOGGY, "Tuning in %s.", earl);
	pthread_mutex_lock (&mpd_mutex);

  // int playing = easympd_int_playing();
	easympd_int_handle_rc(mpd_run_clear(easympd_int_conn));
	easympd_int_handle_rc(mpd_run_add_id_to(easympd_int_conn,earl,0));
	// if(playing) mpd_run_play(easympd_int_conn);

	pthread_mutex_unlock (&mpd_mutex);
}

void easympd_start() {
	loggy_debug(_EASYMPD_LOGGY, "Starting...");
	pthread_mutex_lock (&mpd_mutex);
  mpd_run_play(easympd_int_conn);
	pthread_mutex_unlock (&mpd_mutex);
}

void easympd_stop() {
		loggy_debug(_EASYMPD_LOGGY, "Stopping...");
		pthread_mutex_lock (&mpd_mutex);
    mpd_run_stop(easympd_int_conn);
		pthread_mutex_unlock (&mpd_mutex);
}

void easympd_status(char * tagName, char * tagTitle) {
	loggy_trace(_EASYMPD_LOGGY, "Status check...");

	pthread_mutex_lock (&mpd_mutex);
	easympd_int_song = mpd_run_current_song(easympd_int_conn);
	pthread_mutex_unlock (&mpd_mutex);
	if(easympd_int_song == NULL) {
		strcpy(tagName, "");
		strcpy(tagTitle, "");
		easympd_int_handle_error();
	}
	else {
		// usually: name = stream name, title = artist - title
		const char *gotName  = mpd_song_get_tag(easympd_int_song, MPD_TAG_NAME, 0);
		const char *gotTitle = mpd_song_get_tag(easympd_int_song, MPD_TAG_TITLE, 0);
		strcpy(tagName, gotName == NULL ? "" : gotName);
		strcpy(tagTitle, gotTitle == NULL ? "" : gotTitle);
		mpd_song_free(easympd_int_song);
	}
	loggy_debug(_EASYMPD_LOGGY, "Status check done. Got name: %s, Title: %s.",tagName, tagTitle);
}
