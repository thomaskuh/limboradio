#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "oled_fonts.h"
#include "ssd1306_i2c.h"

#include "limboradio.h"
#include "loggy.h"

char* _DISPLAY_LOGGY = "Display";

int limbodisplay_device = -1;

void limbodisplay_init(int i2cBus, int i2cAdr) {
  loggy_info(_DISPLAY_LOGGY, "Initializing on bus %d and address %x ...", i2cBus, i2cAdr);

  if(i2cBus == -1 || i2cAdr == -1) {
    loggy_info(_DISPLAY_LOGGY, "Deactivated.");
    return;
  }

  limbodisplay_device = wiringPiI2CSetupInterface(i2cBus == 0 ? "/dev/i2c-0" : "/dev/i2c-1", i2cAdr);
  if(limbodisplay_device < 0) {
    loggy_error(_DISPLAY_LOGGY, "Init failed. Device not found.");
  }
  else {
    ssd1306_begin(SSD1306_SWITCHCAPVCC, limbodisplay_device);
    ssd1306_dim(0);
    ssd1306_clearDisplay();
    ssd1306_setTextSize(1);
    ssd1306_drawString("Init....");
    ssd1306_display();
    loggy_info(_DISPLAY_LOGGY, "Init done.");
  }
}

void limbodisplay_refresh(struct limbo_context *ctx) {
    if(limbodisplay_device < 0) return;

    char buf[200];

    ssd1306_clearDisplay();

    // Date BIG
    ssd1306_setTextSize(2);
    sprintf(buf, "%s\n",ctx->time);
    ssd1306_drawString(buf);

    // Chan BIG
    // ssd1306_setTextSize(2);
    sprintf(buf, "%s%s",ctx->streams[ctx->stream].name, strlen(ctx->streams[ctx->stream].name) > 9 ? "" : "\n");
    ssd1306_drawString(buf);

    // Others SMALL
    ssd1306_setTextSize(1);
    sprintf(buf, "\nIP: %s", ctx->netinfo);
    ssd1306_drawString(buf);
    sprintf(buf, "\n%03ld %.2f", ctx->timer, ctx->lux);
    ssd1306_drawString(buf);

    sprintf(buf, "\n%d", ctx->volume);
    ssd1306_drawString(buf);

    ssd1306_fillRect(24, 56, ctx->volume, 7 , WHITE);

    ssd1306_display();
}
