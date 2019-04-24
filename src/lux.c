#include <stdio.h>
#include <math.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "lux.h"
#include "loggy.h"

char* _LUX_LOGGY = "SensorLux";

void lux_init(struct lux* ref, int i2cBus, int i2cAdr) {
  loggy_info(_LUX_LOGGY, "Initializing on bus %d and address %x ...", i2cBus, i2cAdr);

  if(i2cBus == -1 || i2cAdr == -1) {
    loggy_info(_LUX_LOGGY, "Deactivated.");
    ref->device = -1;
    return;
  }

  ref->device = wiringPiI2CSetupInterface(i2cBus == 0 ? "/dev/i2c-0" : "/dev/i2c-1", i2cAdr);
  if(ref->device < 0) loggy_error(_LUX_LOGGY, "Init failed. Device not found.");
  else loggy_info(_LUX_LOGGY, "Init done.");
}

// from https://github.com/leon-anavi/rpi-examples/tree/master/MAX44009
float lux_read(struct lux* ref) {
    if(ref->device < 0) return 0;

    int vhigh = wiringPiI2CReadReg8(ref->device, 0x03);
  	int vlow  = wiringPiI2CReadReg8(ref->device, 0x04);

  	int exponent=( vhigh & 0xF0 ) >> 4;
  	int mantisa= ( vhigh & 0x0F ) << 4 | vlow;

  	float result = ( ( pow(2,(double)exponent) ) * (double)mantisa ) * 0.045;
    loggy_trace(_LUX_LOGGY, "Read %.3f luxes.", result);
    if(result < 0) result = 0.0f;
  	return result;
}
