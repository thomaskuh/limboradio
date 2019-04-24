#ifndef _LIMBONET_H_
#define _LIMBONET_H_

// Write netctl wifi profile file
void limbonet_write(const char *wifiName, const char *wifiPass);

// Restart netctl to read (new/updated) profiles
void limbonet_reset();

// Read info about current network/wifi state
void limbonet_info(char *output);

#endif
