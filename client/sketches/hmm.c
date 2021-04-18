#include "../../util.h"

enum B { b_del, nb };
enum O { o_a, o_b, o_c, o_d, o_e, no };
enum F { f_p, f_f, nf };

void reload(S *s) {

  s->o[o_a].f = 440.0 + 110.0;
  s->o[o_b].f = 2.0;
  s->o[o_c].f = 0.5;
  s->o[o_d].f = 220.0;

  s->f[f_p] = 0.0;

  s->b[b_del].r = 100;
}

void run(S *s, int I, float *out) {
  for (int i = 0; i < I; i++) {

    float x =
        ((s->t % 22000 < 10500 ? sin(s->o[o_a].th) : saw(s->o[o_a].th) * 0.5) +
         pul(s->o[o_d].th)) *
        pul(s->o[o_b].th);

    add_smp(pan(x, (s->t % 22000 < 10500 ? sin(s->o[o_c].th) : s->f[f_p])));

    //_bufw(&s->b[b_del], i, buf);
    bufw(&s->b[b_del]);
    // add_smp(delr(&s->b[b_del]));

    if (s->t % 22000 == 0) {
      s->o[o_b].f = floor(rrand(1.0, 16.0));
      s->o[o_d].f = s->o[o_a].f / 2.0;
      s->f[f_p] = rrand(-1.0, 1.0);
    }

    update();
  }
}

CLIENT_MACRO
