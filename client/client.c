#include "../util.h"

enum B { b_del, nb };
enum O { o_a, o_b, o_c, o_d, o_e, no };
enum F { f_p, f_f, nf };

RELOAD {
  s->o[o_b].f = 16.0;
  // osc_f(o_b, 2.0);
  osc_f(o_c, 0.5);
  osc_f(o_d, 8.0);
  s->f[f_p] = 0.0;
  s->f[f_f] = 220.0;
}

RUN {
  LOOP {
    osc_f(o_a, bimap(saw_osc(o_d), 0.9, 1.1) * s->f[f_f]);
    add_smp(pan(pul_osc(o_a) * pul_osc(o_b), s->f[f_p]));

    bufw(b_del);
    add_smp(mul(delr(b_del), 0.5));

    if (s->t % 22050 == 0) {
      buf_r(b_del, rrand(1.0, BUFFER_MAX));
      s->f[f_f] = floor(rrand(1.0, 4.0)) * 110.0;
      s->f[f_p] = 0.25 * floor(rrand(-1.0, 1.0) / 0.25);
      // osc_f(o_b, floor(rrand(1.0, 8.0)));
      s->o[o_b].f = 16.0;
    }

    update();
  }
}

CLIENT_MACRO
