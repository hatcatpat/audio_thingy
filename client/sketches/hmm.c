#include "../../util.h"

enum B { b_del, b_del2, nb };
enum O { o_a, o_b, o_c, o_d, o_e, no };
enum F { f_p, f_f, nf };

void reload(S *s) {

  s->o[o_b].f = 2.0;
  s->o[o_c].f = 0.5;

  s->f[f_p] = 0.0;

  s->b[b_del].r = (int)floor(10000.0 * 0.5);
  // s->b[b_del2].r = (int)floor(s->b[b_del].r * 2.0);
}

void run(S *s, int I, float *out) {
  for (int i = 0; i < I; i++) {

    add_smp(pan(sin(s->o[o_a].th), s->f[f_p]));
    // float x =
    //((s->t % 22000 < 10500 ? sin(s->o[o_a].th) : saw(s->o[o_a].th) * 0.5) +
    // pul(s->o[o_d].th)) *
    // pul(s->o[o_b].th);

    // add_smp(pan(x, (s->t % 22000 < 10500 ? sin(s->o[o_c].th) : s->f[f_p])));

    bufw(&s->b[b_del]);
    add_smp(mul(delr(&s->b[b_del]), 0.5));

    // bufw(&s->b[b_del2]);
    // add_smp(mul(delr(&s->b[b_del2]), 0.8));

    if (s->t % 5000 == 0) {
      s->o[o_a].f = floor(rrand(1.0, 4.0)) * 220.0;
      s->o[o_d].f = s->o[o_a].f / 2.0;
      s->f[f_p] = rrand(-1.0, 1.0);
    }

    update();
  }
}

CLIENT_MACRO
