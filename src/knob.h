#ifndef _KNOB_H_
#define _KNOB_H_

struct knob {
  int active;
  int pin_a;
  int pin_b;
  int pin_btn;
  volatile int last_encoded;
	void (*callback_rotary)(int valueChange);
  void (*callback_button)();
};

void knob_init(int pin_a, int pin_b, int pin_btn, void (*callback_rotary)(int), void (*callback_button)());

#endif
