#include <jansson.h>
#include <string.h>
#include <regex.h>

#include "loggy.h"
#include "limboradio.h"

char* _CONFIG_LOGGY = "Config";

regex_t rxWifiName;
regex_t rxWifiPass;
regex_t rxStreamName;
regex_t rxStreamEarl;

// ============
// = INTERNAL =
// ============

void lc_dump(json_t *json) {
  char *result = json_dumps(json, JSON_INDENT(2));
  loggy_debug(_CONFIG_LOGGY, "Dumping json:\n%s\n", result);
  free(result);
}

struct limbo_error* make_error(const char* key, const char* msg) {
  struct limbo_error * e = malloc(sizeof(struct limbo_error));
  strcpy(e->key, key);
  strcpy(e->msg, msg);
  return e;
}

json_t * lc_err_to_json(struct limbo_error* err) {
  json_t *joRoot = json_object();
  json_object_set(joRoot, "key", json_string(err->key));
  json_object_set(joRoot, "msg", json_string(err->msg));
  return joRoot;
}


struct limbo_error* lc_json_to_ctx(json_t *joRoot, int real) {
  struct limbo_context* ctx = limboradio_ctx();

  // Get that field references
  json_t *jiLog = json_object_get(joRoot, "log");
  json_t *jiMode = json_object_get(joRoot, "mode");
  json_t *jiVol = json_object_get(joRoot, "volume");
  json_t *jiStream = json_object_get(joRoot, "stream");
  json_t *jiThreshold = json_object_get(joRoot, "threshold");
  json_t *jiTimeout = json_object_get(joRoot, "timeout");
  json_t *jsWifiName = json_object_get(joRoot, "wifiName");
  json_t *jsWifiPass = json_object_get(joRoot, "wifiPass");
  json_t *jaStreams = json_object_get(joRoot, "streams");

  // Validate wifi and parse
  const char* sWifiName = NULL;
  const char* sWifiPass = NULL;
  if(jsWifiName != NULL || jsWifiPass != NULL) {
    sWifiName = json_string_value(jsWifiName);
    sWifiPass = json_string_value(jsWifiPass);
    if(sWifiName == NULL || regexec(&rxWifiName, sWifiName, 0, NULL, 0)) return make_error("WIFI_INVALID", "Invalid wifi name.");
    if(sWifiPass == NULL || regexec(&rxWifiPass, sWifiPass, 0, NULL, 0)) return make_error("WIFI_INVALID", "Invalid wifi pass.");
  }

  // Validate streams and parse
  struct limbo_stream streams[50];
  int streamsLen = 0;

  if(jaStreams != NULL) {
    streamsLen = json_array_size(jaStreams);
    if(streamsLen < 1 || streamsLen > 50) {
      return make_error("STREAMS_SIZE", "");
    }

    size_t index;
    json_t *jStream, *jStreamName, *jStreamEarl;
    char buffer [1024];

    // const char *sName, *sEarl;
    json_array_foreach(jaStreams, index, jStream) {
      loggy_debug(_CONFIG_LOGGY, "ARRAY IDX: %d.", index);
      jStreamName = json_object_get(jStream, "name");
      jStreamEarl = json_object_get(jStream, "earl");

      if(jStreamName == NULL || jStreamEarl == NULL) {
        return make_error("STREAMS_EMPTY", "");
      }

      const char *sName = json_string_value(jStreamName);
      const char *sEarl = json_string_value(jStreamEarl);

      if(sName == NULL || sEarl == NULL) {
        return make_error("STREAMS_EMPTY", "");
      }

      if(regexec(&rxStreamName, sName, 0, NULL, 0)) {
        sprintf(buffer, "Stream name: %s.", sName);
        return make_error("STREAMS_NAME", buffer);
      }
      if(regexec(&rxStreamEarl, sEarl, 0, NULL, 0)) {
        sprintf(buffer, "Stream URL: %s.", sEarl);
        return make_error("STREAMS_URL", buffer);
      }

      strcpy(streams[index].name, sName);
      strcpy(streams[index].earl, sEarl);
    }
  }

  // Save simples
  if(jiLog != NULL) limboradio_log(json_integer_value(jiLog), real);
  if(jiMode != NULL) limboradio_mode(json_integer_value(jiMode), real);
  if(jiVol != NULL) limboradio_volume(json_integer_value(jiVol), real);
  if(jiStream != NULL) limboradio_stream(json_integer_value(jiStream), real);
  if(jiThreshold != NULL) limboradio_threshold(json_integer_value(jiThreshold), real);
  if(jiTimeout != NULL) limboradio_timeout(json_integer_value(jiTimeout), real);

  // Save wifi
  if(sWifiName != NULL && sWifiPass != NULL) {
    limboradio_wifi(sWifiName, sWifiPass, real);
  }

  // Save STREAMS_SIZE
  if(streamsLen > 0) {
    limboradio_streams(streams, streamsLen, real);
  }

  return NULL;
}


json_t * lc_ctx_to_json(int withConfig, int withStreams, int withTransient) {
  struct limbo_context* ctx = limboradio_ctx();

  json_t *joRoot = json_object();
  if(withConfig) {
    json_object_set(joRoot, "log", json_integer(ctx->log));
    json_object_set(joRoot, "mode", json_integer(ctx->mode));
    json_object_set(joRoot, "threshold", json_integer(ctx->threshold));
    json_object_set(joRoot, "timeout", json_integer(ctx->timeout));
    json_object_set(joRoot, "volume", json_integer(ctx->volume));
    json_object_set(joRoot, "stream", json_integer(ctx->stream));
  }

  if(withStreams) {
    json_t *jaStreams = json_array();
    for(int i = 0; i < ctx->streamLen; i++) {
      json_t *joStream = json_object();
      json_object_set(joStream, "name", json_string(ctx->streams[i].name));
      json_object_set(joStream, "earl", json_string(ctx->streams[i].earl));
      json_array_append_new(jaStreams, joStream);
    }
    json_object_set(joRoot, "streams", jaStreams);
  }

  if(withTransient) {
    json_object_set(joRoot, "playing", json_integer(ctx->playing));
    json_object_set(joRoot, "lux", json_real(ctx->lux));
    json_object_set(joRoot, "timer", json_integer(ctx->timer));

    json_object_set(joRoot, "tagName", json_string(ctx->tagName));
    json_object_set(joRoot, "tagTitle", json_string(ctx->tagTitle));

    json_object_set(joRoot, "date", json_string(ctx->date));
    json_object_set(joRoot, "time", json_string(ctx->time));

    json_object_set(joRoot, "netinfo", json_string(ctx->netinfo));
  }

  return joRoot;
}


// ==============
// = PUBLIC API =
// ==============

void limboconfig_init() {
  int r1 = regcomp(&rxWifiName, "^[-[:alnum:]äÄöÖüÜß _/?#=.:]\\{1,100\\}$", 0);
  int r2 = regcomp(&rxWifiPass, "^[-[:alnum:]äÄöÖüÜß _/?#=.:]\\{1,100\\}$", 0);
  int r3 = regcomp(&rxStreamName, "^[-[:alnum:]äÄöÖüÜß _/?#=.:]\\{1,10\\}$", 0);
  int r4 = regcomp(&rxStreamEarl, "^https\\?://[-[:alnum:]äÄöÖüÜß _/?#=.:]\\{1,500\\}$", 0);
  if(r1 || r2 || r3 || r4) loggy_error(_CONFIG_LOGGY, "Failed to compile regex.");
}

int  limboconfig_write_error(struct limbo_error* err, char *output) {
  json_t *joRoot = lc_err_to_json(err);
  int result = json_dumpb(joRoot, output, 1024*1024, JSON_INDENT(2));
  free(joRoot);
  return result;
}

int  limboconfig_write_bytes(char *output, int withConfig, int withStreams, int withTransient) {
  json_t *joRoot = lc_ctx_to_json(withConfig, withStreams, withTransient);
  int result = json_dumpb(joRoot, output, 1024*1024, JSON_INDENT(2));
  free(joRoot);
  return result;
}

void limboconfig_write_file(char *filepath) {
  loggy_debug(_CONFIG_LOGGY, "Writing config to file %s.", filepath);
  FILE *cfgfile = fopen(filepath, "w");
  if(cfgfile == NULL) {
    loggy_error(_CONFIG_LOGGY, "Failed. Cannot open file for write: %s.", filepath);
    return;
  }
  json_t *joRoot = lc_ctx_to_json(1, 1, 0);
  int result = json_dumpf(joRoot, cfgfile, JSON_INDENT(2));
  free(joRoot);
  fclose(cfgfile);
}

struct limbo_error* limboconfig_read_bytes(char *input, int length) {
  loggy_debug(_CONFIG_LOGGY, "Reading config from input.");

  json_error_t jerror;
  json_t *joRoot = json_loadb(input, length, 0, &jerror);
  if(joRoot == NULL) {
    char temp[1024];
    sprintf(temp, "Json format error in line %d: %s.", jerror.line, jerror.text);
    loggy_error(_CONFIG_LOGGY, temp);
    return make_error("JSON_FORMAT", temp);
  }

  struct limbo_error* err = lc_json_to_ctx(joRoot, 1);
  free(joRoot);
  return err;
}

void limboconfig_read_file(char *filepath) {
  loggy_info(_CONFIG_LOGGY, "Reading config from file %s.", filepath);

  FILE *cfgfile = fopen(filepath, "r");
  if(cfgfile == NULL) {
    loggy_info(_CONFIG_LOGGY, "File not found. Using defaults.");
    return;
  }

  json_error_t jerror;
  json_t *joRoot = json_loadf(cfgfile, 0, &jerror);
  fclose(cfgfile);
  if(joRoot == NULL) {
    loggy_error(_CONFIG_LOGGY, "File format wrong. Using defaults. Line: %d -> %s.", jerror.line, jerror.text);
    return;
  }

  struct limbo_error* err = lc_json_to_ctx(joRoot, 0);
  if(err != NULL) {
    loggy_error(_CONFIG_LOGGY, "Validation error. Using defaults. %s -> %s.", err->key, err->msg);
    free(err);
  }
  free(joRoot);
}
