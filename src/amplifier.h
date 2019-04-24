#ifndef _AMPLIFIER_H_
#define _AMPLIFIER_H_

void amp_init(int i2cBus, int i2cAdr, int pin_mute, int pin_standby, int initial_volume);
void amp_volume(int volume);

#endif
