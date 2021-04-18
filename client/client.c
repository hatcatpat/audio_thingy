#include "../util.h"

enum B { b_del, nb };
enum O { o_a, o_b, o_c, o_d, o_e, no };
enum F { f_p, f_f, nf };

void reload(S *s) { s->o[o_a].f = 220.0; }

void run(S *s, int I, float *buf) {
  for (int i = 0; i < I; i++) {
    add_smp(pan(sin_osc(&s->o[o_a]) * 0.1, 0.0));

    update();
  }
}

CLIENT_MACRO
