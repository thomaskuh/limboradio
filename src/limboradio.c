#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <signal.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "lux.h"
#include "easympd.h"
#include "knob.h"
#include "countdown.h"
#include "limbonet.h"

#include "loggy.h"
#include "limboradio.h"

// Hardware:
#define HW_PIR_PIN 0            // PIR movement sensor gpio
#define HW_ROT_PIN_BTN 25       // Rotary encoder button gpio
#define HW_ROT_PIN_A 3          // Rotary encoder A gpio
#define HW_ROT_PIN_B 2          // Rotary encoder B gpio
#define HW_LUX_I2C_BUS 1        // Lux sensor i2c bus (0/1)
#define HW_LUX_I2C_ADR 0x4A     // Lux sensor i2c address on bus
#define HW_DISP_I2C_BUS 0       // Display i2c bus (0/1)
#define HW_DISP_I2C_ADR 0x3C    // Display i2c address on bus

#define LIMBO_CFG_LOCATION "/etc/limboradio.json"

char* _CORE_LOGGY = "Core";

static volatile int running = 1;

struct limbo_context ctx;

// Features:
struct countdown cntdwn;
struct lux sensorLux;

pthread_t fred_display;
pthread_t fred_status;

void stopHandler(int dummy) {
  running = 0;
}

void ctx_init() {
  // default config
  ctx.log = 1;
  ctx.mode = LIMBO_MODE_LUX;
  ctx.threshold = 5;
  ctx.timeout = 60;
  ctx.volume = 40;
  ctx.stream = 0;

  ctx.streamLen = 8;
  strcpy(ctx.streams[0].name, "EgoFM");
  strcpy(ctx.streams[0].earl, "http://mp3ad.egofm.c.nmdn.net/ps-egofm_128/livestream.mp3");
  strcpy(ctx.streams[1].name, "Antenne");
  strcpy(ctx.streams[1].earl, "http://mp3channels.webradio.antenne.de/antenne");
  strcpy(ctx.streams[2].name, "Charivari");
  strcpy(ctx.streams[2].earl, "http://rs5.stream24.net:80/stream");
  strcpy(ctx.streams[3].name, "FFH");
  strcpy(ctx.streams[3].earl, "http://mp3.ffh.de/radioffh/hqlivestream.mp3");
  strcpy(ctx.streams[4].name, "FFH 90s");
  strcpy(ctx.streams[4].earl, "http://mp3.ffh.de/ffhchannels/hq90er.mp3");
  strcpy(ctx.streams[5].name, "LoungeFM");
  strcpy(ctx.streams[5].earl, "http://stream.lounge.fm/");
  strcpy(ctx.streams[6].name, "Country");
  strcpy(ctx.streams[6].earl, "http://185.33.21.112:80/acountry_128");
  strcpy(ctx.streams[7].name, "KEXP");
  strcpy(ctx.streams[7].earl, "http://live-mp3-128.kexp.org:8000/kexp128.mp3");

  ctx.changed = 0;

  // overwrite with config file
  limboconfig_read_file(LIMBO_CFG_LOCATION);

  // initial state
  ctx.playing = 0;
  ctx.lux = 0;
  ctx.timer = 0;
  strcpy(ctx.date, "00.00.0000");
  strcpy(ctx.time, "00:00:00");
  strcpy(ctx.netinfo, "Unknown");

  loggy_info(_CORE_LOGGY, "Config -> Log: %d, Mode: %d, Threshold: %d, Timeout: %d, Volume: %d, Stream: %d.", ctx.log, ctx.mode, ctx.threshold, ctx.timeout, ctx.volume, ctx.stream);
  for(int i = 0; i < ctx.streamLen; i++) {
    loggy_info(_CORE_LOGGY, "Stream %d -> %s - %s.", i, ctx.streams[i].name, ctx.streams[i].earl);
  }
}

// Check mode vs conditions (lux, pir) and start/stop if not already in that state.
void checkStartStop() {
  int shouldPlay = 0;
  switch (ctx.mode) {
    case LIMBO_MODE_PIR:
      shouldPlay = ctx.timer > 0;
      break;
    case LIMBO_MODE_LUX:
      shouldPlay = ctx.lux >= ctx.threshold;
      break;
    case LIMBO_MODE_BOTH:
      shouldPlay = ctx.timer > 0 && ctx.lux >= ctx.threshold;
      break;
    default:
      break;
  }

  if(ctx.playing && !shouldPlay) {
    loggy_info(_CORE_LOGGY, "Playback STOP. Mode: %d, Lux: %.3f, Timer: %d.", ctx.mode, ctx.lux, ctx.timer);
    ctx.playing = 0;
    easympd_stop();
  }
  else if(!ctx.playing && shouldPlay) {
    loggy_info(_CORE_LOGGY, "Playback START. Mode: %d, Lux: %.3f, Timer: %d.", ctx.mode, ctx.lux, ctx.timer);
    ctx.playing = 1;
    easympd_start();
  }
}

void datetime_refresh() {
    time_t timeraw;
    struct tm * timeinfo;
    time ( &timeraw ); // fill time
    timeinfo = localtime ( &timeraw ); // get local
    strftime(ctx.date,80,"%d.%m.%Y", timeinfo);
    strftime(ctx.time,80,"%X", timeinfo);
}


void powertimer_callback() {
    int val = digitalRead(HW_PIR_PIN);
    loggy_debug("SensorMove", "Triggered state: %d.", val);
    if(val) countdown_set(&cntdwn, ctx.timeout);
}

void powertimer_setup() {
  if(HW_PIR_PIN == -1) {
    loggy_info("SensorMove", "Deactivated.");
  }
  else {
    loggy_info("SensorMove", "Initializing on pin %d with a timeout of %d seconds.", HW_PIR_PIN, ctx.timeout);
    pinMode(HW_PIR_PIN,INPUT);
		wiringPiISR(HW_PIR_PIN,INT_EDGE_BOTH, powertimer_callback);
	}
}

void lux_refresh() {
    ctx.lux = lux_read(&sensorLux);
    checkStartStop();
}

void* fred_display_runner(void *unused) {
  while(running) {
    datetime_refresh();
    limbodisplay_refresh(&ctx);
  }
}

void* fred_status_runner(void *unused) {
  while(running) {
    usleep(10000000);
    limbonet_info(ctx.netinfo);
    easympd_status(ctx.tagName, ctx.tagTitle);
    if(ctx.changed) {
      loggy_info("Core", "Config changed ... saving to disk.");
      ctx.changed = 0;
      limboconfig_write_file(LIMBO_CFG_LOCATION);
    }
  }
}

int main(void) {
  loggy_info(_CORE_LOGGY, "Kickin' off that LimboRadio thang...");

  signal(SIGINT, stopHandler);

  limboconfig_init();
  ctx_init();
  loggy_level(ctx.log);

  // WiringPi init
  int wiringPiRc = wiringPiSetup();
	if(wiringPiRc != 0) {
    loggy_error(_CORE_LOGGY, "GPIO init failed with RC %d. Exit.", wiringPiRc);
		return 1;
	}

  // Setup stuff we trigger manually
  limbodisplay_init(HW_DISP_I2C_BUS, HW_DISP_I2C_ADR);
  lux_init(&sensorLux, HW_LUX_I2C_BUS, HW_LUX_I2C_ADR);

  // Mpd
  easympd_setup();
  easympd_stop();
  easympd_volume(ctx.volume);
  easympd_earl(ctx.streams[ctx.stream].earl);


  // Setup stuff triggering callbacks
  knob_init(HW_ROT_PIN_A, HW_ROT_PIN_B, HW_ROT_PIN_BTN, limboradio_volume_inc, limboradio_stream_next);
  countdown_init(&cntdwn, &ctx.timer, checkStartStop, checkStartStop);
  powertimer_setup();

  web_init();

  // Thread with minimal sleep -> Refresh display.
  pthread_create (&fred_display, NULL, fred_display_runner, NULL);

  // Thread with 15sec sleep -> Refresh MPD (station, title), MPD keepalive, config persist on change.
  pthread_create (&fred_status, NULL, fred_status_runner, NULL);

  while(running) {
    usleep(200000);
    lux_refresh();
    countdown_check(&cntdwn);
  }

  loggy_info(_CORE_LOGGY, "Puttin' it down...");
  web_destroy();
  loggy_info(_CORE_LOGGY, "Ciaoi Maui!");

	return 0;
}


// ==============
// = PUBLIC API =
// ==============

struct limbo_context* limboradio_ctx() {
  return &ctx;
}

void limboradio_log(int newVal, int real) {
  if(newVal < 0 || newVal > 3 || newVal == ctx.log) {
    loggy_debug(_CORE_LOGGY, "Changing log failed. Same or illegal value detected: %d.", newVal);
    return;
  }
  ctx.log = newVal;
  if(real) {
    loggy_info(_CORE_LOGGY, "Changing log to: %d.", newVal);
    ctx.changed = 1;
  }
}

void limboradio_mode(int newVal, int real) {
  if(newVal < 0 || newVal > 2 || newVal == ctx.mode) {
    loggy_debug(_CORE_LOGGY, "Changing mode failed. Same or illegal value detected: %d.", newVal);
    return;
  }
  ctx.mode = newVal;
  if(real) {
    loggy_info(_CORE_LOGGY, "Changing mode to: %d.", newVal);
    ctx.changed = 1;
  }
}

void limboradio_volume(int newVal, int real) {
  if(newVal < 0 || newVal > 100 || newVal == ctx.volume) {
    loggy_debug(_CORE_LOGGY, "Changing volume failed. Same or illegal value detected: %d.", newVal);
    return;
  }
  ctx.volume = newVal;
  if(real) {
    loggy_info(_CORE_LOGGY, "Changing volume to: %d.", newVal);
    easympd_volume(ctx.volume);
    ctx.changed = 1;
  }
}

void limboradio_volume_inc(int volumeInc) {
    limboradio_volume(ctx.volume + volumeInc, TRUE);
}

void limboradio_stream(int newVal, int real) {
  if(newVal < 0 || newVal >= ctx.streamLen || newVal == ctx.stream) {
    loggy_debug(_CORE_LOGGY, "Changing stream failed. Same or illegal value detected: %d.", newVal);
    return;
  }
  ctx.stream = newVal;
  if(real) {
    loggy_info(_CORE_LOGGY, "Changing stream to: %d - %s - %s.", newVal, ctx.streams[newVal].name, ctx.streams[newVal].earl);
    easympd_earl(ctx.streams[newVal].earl);
    if(ctx.playing) easympd_start();
    ctx.changed = 1;
  }
}

void limboradio_stream_next() {
    int next = (ctx.stream + 1) % ctx.streamLen;
    limboradio_stream(next, TRUE);
}

void limboradio_threshold(int newVal, int real) {
  if(newVal < 0 || newVal > 10000 || newVal == ctx.threshold) {
    loggy_debug(_CORE_LOGGY, "Changing threshold failed. Same or illegal value detected: %d.", newVal);
    return;
  }
  ctx.threshold = newVal;
  if(real) {
    loggy_info(_CORE_LOGGY, "Changing threshold to: %d.", newVal);
    ctx.changed = 1;
  }
}

void limboradio_timeout(int newVal, int real) {
  if(newVal < 0 || newVal > 10000 || newVal == ctx.timeout) {
    loggy_debug(_CORE_LOGGY, "Changing timeout failed. Same or illegal value detected: %d.", newVal);
    return;
  }
  ctx.timeout = newVal;
  if(real) {
    loggy_info(_CORE_LOGGY, "Changing timeout to: %d.", newVal);
    ctx.changed = 1;
  }
}

void limboradio_streams(struct limbo_stream *input, int length, int real) {
  for(int i = 0; i < length; i++) {
    if(real) loggy_info(_CORE_LOGGY, "Changing stream %d. Name: %s, Earl: %s.", i, input[i].name, input[i].earl);
    strcpy(ctx.streams[i].name, input[i].name);
    strcpy(ctx.streams[i].earl, input[i].earl);
  }
  ctx.streamLen = length;
  if(ctx.stream >= ctx.streamLen) {
    limboradio_stream(0, real);
  }
  if(real) ctx.changed = 1;
}

void limboradio_wifi(const char *name, const char *pass, int real) {
  loggy_info(_CORE_LOGGY, "Changing wifi. Name: %s, Pass: %s.", name, pass);
  limbonet_write(name, pass);
  limbonet_reset();
}
