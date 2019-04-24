#ifndef LIMBORADIO_H
#define LIMBORADIO_H

#define LIMBO_MODE_PIR 0
#define LIMBO_MODE_LUX 1
#define LIMBO_MODE_BOTH 2

struct limbo_error {
	char key[255];
	char msg[1024];
};

struct limbo_stream {
	char name[255];
	char earl[1024];
};

// LimboRadio context
struct limbo_context {
  // Configuration (Persistable)
  int log;                    // log level (0 = ERROR, 1 = INFO, 2 = DEBUG, 3 = TRACE)
  int mode;                   // mode (0 = PIR, 1 = LUX, 2 = )
  int threshold;              // lux sensor threshold
  int timeout;                // time to off after PIR movement (seconds).
	int volume;	       	        // volume (0-100)

	int stream;                 // current stream idx
	int streamLen;              // number of streams
  struct limbo_stream streams[50];	// streams itself

	int changed;								// 1 = config param(s) changed and file needs to be saved on disk

  // State (Transient)
	int playing;								// Playing?
  float lux;                  // Latest lux sensor read
	int timer;									// Seconds to timeout for PIR movement

	char tagName[1024];					// Tag "name" submitted by current station
	char tagTitle[1024];        // Tag "title" submitted by current station

  char date[20];              // Current date as string
  char time[20];              // Current time as string

	char netinfo[255];					// Network info (addr, wifi vs. lan)
};

// Core
struct limbo_context* limboradio_ctx();
void limboradio_log(int level, int real);							// set log level
void limboradio_mode(int mode, int real);							// set mode (0-2)
void limboradio_volume(int volume, int real);					// set volume (0-100)
void limboradio_volume_inc(int volumeInc);						// set volume increment (-100-100)
void limboradio_stream(int idx, int real);						// set stream (index)
void limboradio_stream_next();												// set stream to next
void limboradio_threshold(int threshold, int real);		// set lux threshold
void limboradio_timeout(int timeout, int real);				// set PIR timeout
void limboradio_streams(struct limbo_stream *input, int length, int real);
void limboradio_wifi(const char *name, const char *pass, int real);					// set new wifi credentials and restart netctl

// Config persistence
void limboconfig_init();																								// Init (some regexes)
int  limboconfig_write_error(struct limbo_error* err, char *output);		// Write error as json to output.
int  limboconfig_write_bytes(char *output, int withConfig, int withStreams, int withTransient);
void limboconfig_write_file(char *filepath);														// Write ctx to config file.
struct limbo_error* limboconfig_read_bytes(char *input, int length); 		// Read config bytes and set ctx. For changes at runtime.
void limboconfig_read_file(char *filepath);															// Read config file (if existing) and set ctx. For startup init.

// Display stuff
void limbodisplay_init(int i2cBus, int i2cAdr);
void limbodisplay_refresh(struct limbo_context *ctx);

// Sensor lux
void limbolux_setup(int i2cBus, int i2cAdr);
float limbolux_read();

// Web
int web_init();
int web_destroy();

#endif
