#ifndef LUX_H
#define LUX_H

struct lux {
  int device;
};

void lux_init(struct lux* ref, int i2cBus, int i2cAdr);
float lux_read(struct lux* ref);

#endif
