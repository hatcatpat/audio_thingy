#ifndef DSP
#define DSP

#include <math.h>
#include <stdlib.h>

//================================================================================================
// DEFS
#define PI 3.1415926535897932384
#define PII PI / 2.0
#define TAU PI * 2.0
#define LOGTEN 2.302585092994046
#define BUFFER_MAX 44100
#define WAVE_FUNC_MACRO float (*wf)(float)

typedef unsigned int uint;

//================================================================================================
// STRUCTS

// an oscillator that contains a theta and frequency
typedef struct {
  float th;
  float f;
} osc;

// a 2-channel buffer, with a read and write counter
typedef struct {
  float d[2][BUFFER_MAX];
  float r;
  int w;
} buf;

// a struct containing our global state, which will persist over reloads
typedef struct {
  // objects
  buf *b;   // buffers
  int nb;   // num buffers
  osc *o;   // oscillators
  int no;   // num oscillators
  float *f; // floats
  int nf;   // num floats
  // properties
  float sr; // sample rate
  int nc;   // num chans
  int t;    // time
} S;

//================================================================================================
// WAVES
// float version of sin
static inline float fsin(float th) { return sin(th); }

// pulse wave
static inline float pul(float th) { return fmod(th, TAU) < PI ? 0.0 : 1.0; }

// saw wave
static inline float saw(float th) {
  return ((float)fmod(th, TAU) / TAU) * 2.0f - 1.0f;
}

//================================================================================================
// FUNCS

// correct modulo
static int mod(int x, int n) { return ((x % n) + n) % n; }

// maps input range to output range
static float map(float x, float inlo, float inhi, float outlo, float outhi) {
  if (inhi == inlo) {
    return outlo;
  } else {
    return (x - inlo) * (outhi - outlo) / (inhi - inlo) + outlo;
  }
}

// maps [-1,1] to output range
static inline float bimap(float x, float lo, float hi) {
  return (x + 1.0) * (hi - lo) / 2.0 + lo;
}

// maps [0,1] to output range
static inline float normmap(float x, float lo, float hi) {
  return x * (hi - lo) + lo;
}

// maps [-1,1] to [0,1]
static inline float bi2norm(float x) { return (x + 1.0) / 2.0; }

// maps [0,1] to [-1,1]
static inline float norm2bi(float x) { return (x - 0.5) * 2.0; }

// random float between 2 values
static inline float rrand(float lo, float hi) {
  return normmap(((float)rand()) / ((float)RAND_MAX), lo, hi);
}

// converts sample, channel and num channels to buffer index
static inline int get_index(int i, int c, int C) { return i * C + c; }

// adds sample to both channels of buffer, and frees input
static inline void _add_smp(S *s, float *v, int i, float *buf) {
  for (int c = 0; c < s->nc; c++)
    buf[get_index(i, c, s->nc)] += v[c];

  free(v);
}
#define add_smp(v) _add_smp(s, v, i, buf)

// pans sample to 2 channels
// LEAK WARNING !!!
static float *pan(float v, float p) {
  float *o = (float *)malloc(sizeof(float) * 2);
  float l = (p + 1.0) / 2.0;
  o[0] = v * (1.0 - l);
  o[1] = v * l;
  return o;
}

// converts [0,-100] db to [0,1] rms
static inline float db2rms(float db) {
  db = 100.0 + db;
  if (db <= 0)
    return 0.0;

  if (db > 485)
    db = 485;

  return exp((LOGTEN * 0.05) * (db - 100.0));
}

// applies gain to whole block
static void _gain(S *s, float v, int I, float *buf) {
  for (int c = 0; c < s->nc; c++)
    for (int i = 0; i < I; i++)
      buf[get_index(i, c, s->nc)] *= v;
}
#define gain(v) _gain(s, v, I, buf)

static inline void _gain_db(S *s, float db, int I, float *buf) {
  _gain(s, db2rms(db), I, buf);
}
#define gain_db(db) _gain_db(s, db, I, buf)

static float *mul(float *in, float v) {
  in[0] *= v;
  in[1] *= v;
  return in;
}

//================================================================================================
// BUFFERS

// get oscillator
#define g_buf(buf_id) s->b[buf_id]
#define p_buf(buf_id) &s->b[buf_id]

// resets buffer with 0.0
static void _init_buf(buf *b) {
  b->w = 0;
  b->r = 0;
  for (int c = 0; c < 2; c++)
    for (int i = 0; i < BUFFER_MAX; i++)
      b->d[c][i] = 0.0;
}
#define init_buf(buf_id) _init_buf(p_buf(buf_id))

// set buffer read position
static inline void _buf_r(buf *b, float r) {
  b->r = fmin(r < 0 ? -r : r, BUFFER_MAX);
}
#define buf_r(buf_id, r) _buf_r(p_buf(buf_id), r)

// write current sample into buffer
static void _bufw(buf *b, int i, float *out) {
  for (int c = 0; c < 2; c++)
    b->d[c][b->w] = out[get_index(i, c, 2)];
}
#define bufw(buf_id) _bufw(p_buf(buf_id), i, buf)

// writes current sample to buffer, with delay
static void _delw(buf *b, float fb, int i, float *out) {
  for (int c = 0; c < 2; c++)
    b->d[c][b->w] = out[get_index(i, c, 2)] + b->d[c][b->w] * fb;
}
#define delw(buf_id, fb) _delw(p_buf(buf_id), fb, i, buf)

// reads from the buffer and increases write position, treated as a delay line
// LEAK WARNING !!!
static float *_delr(buf *b) {
  float *o = (float *)malloc(sizeof(float) * 2);
  int p = mod(b->w - b->r, BUFFER_MAX);
  o[0] = b->d[0][p];
  o[1] = b->d[1][p];
  b->w = (b->w + 1) % BUFFER_MAX; // increase write position
  return o;
}
#define delr(buf_id) _delr(p_buf(buf_id))

//================================================================================================
// OSCILLATORS

// get oscillator
#define g_osc(osc_id) s->o[osc_id]
#define p_osc(osc_id) &s->o[osc_id]
// get oscillator theta
#define th_osc(osc_id) s->o[osc_id].th
// get oscillator freq
#define f_osc(osc_id) s->o[osc_id].f

// set oscillator freq
static inline void _osc_f(osc *o, float f) { o->f = f; }
#define osc_f(osc_id, f) _osc_f(p_osc(osc_id), f)

// resets oscillator
static inline void _init_osc(osc *o, float f) {
  o->th = 0.0;
  o->f = f;
}
#define init_osc(osc_id, f) _init_osc(p_osc(osc_id), f)

// increments oscillator
static inline void update_osc(S *s, osc *o) {
  o->th += o->f * TAU / s->sr;
  o->th = fmod(o->th, TAU);
}

// gets output from oscillator
#define sin_osc(osc_id) fsin(th_osc(osc_id))
#define saw_osc(osc_id) saw(th_osc(osc_id))
#define pul_osc(osc_id) pul(th_osc(osc_id))

// update the state every sample
// -  increments time
// -  updates all oscillators
static void _update(S *s, int nb, int no, int nf) {
  s->t++;
  for (int i = 0; i < no; i++)
    update_osc(s, &s->o[i]);
}
#define update() _update(s, nb, no, nf)

#endif /* !DSP */
