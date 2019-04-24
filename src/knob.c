#include <wiringPi.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "knob.h"
#include "loggy.h"

char* _KNOB_LOGGY = "Knob";

struct knob myKnob;

void knob_update() {
    int MSB = digitalRead(myKnob.pin_a);
    int LSB = digitalRead(myKnob.pin_b);

    int encoded = (MSB << 1) | LSB;
    int sum = (myKnob.last_encoded << 2) | encoded;

    int change = 0;

    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
        change = 1;
    }
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
        change = -1;
    }

    myKnob.last_encoded = encoded;
    myKnob.callback_rotary(change);
}

void knob_init(int pin_a, int pin_b, int pin_btn, void (*callback_rotary)(int), void (*callback_button)()) {
    loggy_info(_KNOB_LOGGY, "Initializing with pin-a %d, pin-b %d, pin-btn %d.", pin_a, pin_b, pin_btn);

    myKnob.active = 0;
    myKnob.pin_a = pin_a;
    myKnob.pin_b = pin_b;
    myKnob.pin_btn = pin_btn;

    myKnob.last_encoded = 0;
    myKnob.callback_rotary = callback_rotary;
    myKnob.callback_button = callback_button;


    if(pin_a == -1 || pin_b == -1 || pin_btn == -1) {
      loggy_info(_KNOB_LOGGY, "Deactivated.");
      return;
    }

    myKnob.active = 1;

    // Button
		pinMode(pin_btn,INPUT);
		pullUpDnControl(pin_btn, PUD_UP);
		wiringPiISR(pin_btn, INT_EDGE_FALLING, callback_button);

    // Rotary encoder
    pinMode(pin_a, INPUT);
    pinMode(pin_b, INPUT);
    pullUpDnControl(pin_a, PUD_UP);
    pullUpDnControl(pin_b, PUD_UP);
    wiringPiISR(pin_a,INT_EDGE_BOTH, knob_update);
    wiringPiISR(pin_b,INT_EDGE_BOTH, knob_update);
    loggy_info(_KNOB_LOGGY, "Init done.");
}
