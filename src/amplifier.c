#include <stdio.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "loggy.h"
#include "amplifier.h"

char* _AMPLIFIER_LOGGY = "Amplifier";

int amp_device;

void amp_init(int i2cBus, int i2cAdr, int pin_mute, int pin_standby, int initial_volume) {
	loggy_info(_AMPLIFIER_LOGGY, "Initializing on bus %d and address %x ...", i2cBus, i2cAdr);

	if(i2cBus == -1 || i2cAdr == -1) {
    loggy_info(_AMPLIFIER_LOGGY, "Deactivated.");
    return;
  }

	amp_device = wiringPiI2CSetupInterface(i2cBus == 0 ? "/dev/i2c-0" : "/dev/i2c-1", i2cAdr);
  if(amp_device < 0) {
    loggy_error(_AMPLIFIER_LOGGY, "Init failed. Device not found.");
  }
	else {
		amp_volume(initial_volume);
		pinMode(pin_mute,OUTPUT);
		pinMode(pin_standby,OUTPUT);
		digitalWrite(pin_mute,HIGH);
		digitalWrite(pin_standby,HIGH);
    loggy_info(_AMPLIFIER_LOGGY, "Init done.");
  }
}

void amp_volume(int volume) {
	loggy_info(_AMPLIFIER_LOGGY, "Setting volume to %d.", volume);
	if(amp_device > 0) {
		wiringPiI2CWrite(amp_device,volume);
	}
}
