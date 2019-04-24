#ifndef _EASYMPD_H_
#define _EASYMPD_H_

int easympd_setup();

void easympd_volume(int volume);

void easympd_earl(const char * earl);

void easympd_start();

void easympd_stop();

void easympd_status(char * tagName, char * tagTitle);

#endif
