#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <libwebsockets.h>
#include <jansson.h>

#include "limboradio.h"
#include "loggy.h"

char* _WEB_LOGGY = "Web";

// ============
// = PROTOCOL =
// ============

struct thebody {
  int handle;

  int reqMethod;                    // Request method. 0 = dontknowdontcare, 1 = GET, 2 = POST
  uint8_t reqPath[1000];            // Request path, stored in LWS_CALLBACK_HTTP for later usage in other callbacks
  uint8_t reqData[10000];           // Request body
  int reqLen;                       // Request body length

  unsigned int resCode;             // Return code
  uint8_t resData[LWS_PRE + 10000]; // Response body
  int resLen;                       // Response body length

  int doResHeader;                  // 1 if theres a header to send
  int doResBody;                    // 1 if theres a body (or more bytes in body) to send
};


static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
  struct thebody *thebody = (struct thebody *)user;

  switch (reason) {
    // callback for fresh http requests (in = path)
    case LWS_CALLBACK_HTTP:
      // Store things for later usage in upcoming callbacks
      thebody->handle = 0;
      thebody->reqMethod = 0;
      thebody->reqLen = 0;
      thebody->resCode = HTTP_STATUS_OK;
      thebody->resLen = 0;
      strcpy(thebody->reqPath, (const char *)in);
      if(lws_hdr_total_length(wsi, WSI_TOKEN_GET_URI)) thebody->reqMethod = 1;
      if(lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI)) thebody->reqMethod = 2;

      loggy_debug(_WEB_LOGGY, "LWS_CALLBACK_HTTP. Reason: %d, Method: %d -> %s", reason, thebody->reqMethod, thebody->reqPath);

      if(thebody->reqMethod == 1 && !strcmp(thebody->reqPath, "/full")) {
        thebody->resLen = limboconfig_write_bytes(&thebody->resData[LWS_PRE], 1, 1, 1);
      }
      else if(thebody->reqMethod == 1 && !strcmp(thebody->reqPath, "/state")) {
        thebody->resLen = limboconfig_write_bytes(&thebody->resData[LWS_PRE], 1, 0, 1);
      }
      else if(thebody->reqMethod == 1 && !strcmp(thebody->reqPath, "/streams")) {
        thebody->resLen = limboconfig_write_bytes(&thebody->resData[LWS_PRE], 0, 1, 0);
      }
      else if(thebody->reqMethod == 2 && !strcmp(thebody->reqPath, "/update")) {

      }
      else {
        // No mapping -> default handler -> 404
        break;
      }

      // Mark to send header & body and ask for callback on writable
      thebody->doResHeader = 1;
      thebody->doResBody = 1;
      lws_callback_on_writable(wsi);
      return 0;

    // callback for response writable (header+body)
    case LWS_CALLBACK_HTTP_WRITEABLE:
      // loggy_debug("WEB", "LWS_CALLBACK_HTTP_WRITEABLE %d -> %d -> %s", reason, len, (const char *)in);
      if(thebody->doResHeader) {
        // loggy_debug("WEB", "LWS_CALLBACK_HTTP_WRITEABLE --> Doing Header.");
        uint8_t buf[LWS_PRE + LWS_RECOMMENDED_MIN_HEADER_SPACE], *start = &buf[LWS_PRE], *p = start, *end = &buf[sizeof(buf) - 1];
        // LWS_ILLEGAL_HTTP_CONTENT_LEN -> no/unknown content length
        if (lws_add_http_common_headers(wsi, thebody->resCode, "application/json", thebody->resLen, &p, end)) return 1;
        if (lws_finalize_write_http_header(wsi, start, &p, end)) return 1;
        thebody->doResHeader = 0;
        if(thebody->resLen > 0) {
          lws_callback_on_writable(wsi);
          return 0;
        }
      }
      else if(thebody->doResBody) {
        // loggy_debug("WEB", "LWS_CALLBACK_HTTP_WRITEABLE --> Doing Body.");
        int result = lws_write(wsi, &thebody->resData[LWS_PRE], thebody->resLen, LWS_WRITE_HTTP_FINAL);
        thebody->doResBody = 0;
      }
      else {
        // loggy_debug("WEB", "LWS_CALLBACK_HTTP_WRITEABLE --> Nothing more to send.");
      }

      if (lws_http_transaction_completed(wsi)) return -1;
      return 0;

    // callback for request body readable (in = request body)
    case LWS_CALLBACK_HTTP_BODY:
      // loggy_debug("WEB", "LWS_CALLBACK_HTTP_BODY %d -> %d -> %s", reason, len, (const char *)in);

      // Save request body
      strncpy(&thebody->reqData[thebody->reqLen], in, len);
      thebody->reqLen += len;
      return 0;

    // callback for request body read completed
    case LWS_CALLBACK_HTTP_BODY_COMPLETION: ;
      // loggy_debug("WEB", "CALLBACK_HTTP_BODY_COMPLETE %d -> %d -> %s", reason, len, (const char *)in);

      // Parse and handle request body
      struct limbo_error* err = limboconfig_read_bytes(thebody->reqData, thebody->reqLen);
      if(err != NULL) {
        thebody->resLen = limboconfig_write_error(err, &thebody->resData[LWS_PRE]);
        thebody->resCode = HTTP_STATUS_BAD_REQUEST;
        free(err);
      }

      return 0;
    default:
      break;
  }

  // Return 0 = Keep connection alive and wait for further stuff, Non-0 = close/free connection.
  // If we're here, we didn't handle callback ourself and let the default dummy handler process that thing.
  int result = lws_callback_http_dummy(wsi, reason, user, in, len);
  // loggy_debug("WEB", "CALLBACK %d -> Sent to dummy, returning %d.", reason, result);
  return result;
}




// ==================
// = SETUP & CONFIG =
// ==================

pthread_t web_fred_ref;

int web_running = 1;

static struct lws_protocols protocols[] = {
  { "http", callback_http, sizeof(struct thebody), 0 },
  { NULL, NULL, 0, 0 } /* terminator */
};

/* Libwebsockets linked list -> NEXT, CHILD, NAME, VALUE */
static const struct lws_protocol_vhost_options mime_woff2 = {NULL,NULL,".woff2","font/woff2"};
static const struct lws_protocol_vhost_options mime_json = {&mime_woff2,NULL,".json","application/json"};

// Mountpoint /api -> Serve dynamic json api
static const struct lws_http_mount mount_api = {
        /* .mount_next */		NULL,		/* linked-list "next" */
        /* .mountpoint */		"/api",		/* mountpoint URL */
        /* .origin */			NULL,
        /* .def */			NULL,
        /* .protocol */			"http",
        /* .cgienv */			NULL,
        /* .extra_mimetypes */		NULL,
        /* .interpret */		NULL,
        /* .cgi_timeout */		0,
        /* .cache_max_age */		0,
        /* .auth_mask */		0,
        /* .cache_reusable */		0,
        /* .cache_revalidate */		0,
        /* .cache_intermediaries */	0,
        /* .origin_protocol */		LWSMPRO_CALLBACK,	/* files in a dir */
        /* .mountpoint_len */		4,		/* char count */
        /* .basic_auth_login_file */	NULL,
};

// Default mountpoint / -> Serve static content
static const struct lws_http_mount mount = {
        /* .mount_next */		&mount_api,		/* linked-list "next" */
        /* .mountpoint */		"/",		/* mountpoint URL */
        /* .origin */			"/var/limboradio", /* serve from dir */
        /* .def */			"index.html",	/* default filename */
        /* .protocol */			NULL,
        /* .cgienv */			NULL,
        /* .extra_mimetypes */		&mime_json,
        /* .interpret */		NULL,
        /* .cgi_timeout */		0,
        /* .cache_max_age */		0,
        /* .auth_mask */		0,
        /* .cache_reusable */		0,
        /* .cache_revalidate */		0,
        /* .cache_intermediaries */	0,
        /* .origin_protocol */		LWSMPRO_FILE,	/* files in a dir */
        /* .mountpoint_len */		1,		/* char count */
        /* .basic_auth_login_file */	NULL,
};

struct lws_context *context;

static void *web_runner(void* val) {
  int n = 0;
  while (web_running && n >= 0)
    n = lws_service(context, 1000);
}

int web_destroy() {
  web_running = 0;
  lws_context_destroy(context);
}

int web_init() {
  loggy_info(_WEB_LOGGY, "Initializing web interface.");

  struct lws_context_creation_info info;
  const char *p;
  int n = 0;

  lws_set_log_level(LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE, NULL);

  memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
  info.port = 80;
  info.protocols = protocols;
  info.mounts = &mount;
  info.error_document_404 = "/404.html";

  // Security header stuff that dont let angular do inline styles, etc.
  // info.options = LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

  context = lws_create_context(&info);
  if (!context) {
          lwsl_err("lws init failed\n");
          return 1;
  }

  // Kickoff in fred
  int rc = pthread_create( &web_fred_ref, NULL, &web_runner, NULL );
  if( rc != 0 ) {
    printf("[Web] Thread startup failed.\n");
    return -1;
  }

  return 0;
}
