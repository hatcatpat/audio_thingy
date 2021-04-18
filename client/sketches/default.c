// this file is a template as well as the default file that is used

#include "../../util.h"

enum B { nb };
enum O { no };
enum F { nf };

void reload(S *s) {}

void run(S *s, int I, float *buf) {
  for (int i = 0; i < I; i++) {
    update();
  }
}

CLIENT_MACRO
