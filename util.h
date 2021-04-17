#ifndef UTIL
#define UTIL

#include <stdio.h>
#include <stdlib.h>

#include "dsp.h"

// macro to create a function which will return an up-to-date number of elements
// for each type
#define CLIENT_MACRO                                                           \
  void set_sizes(int *_nb, int *_no, int *_nf) {                               \
    *_nb = nb;                                                                 \
    *_no = no;                                                                 \
    *_nf = nf;                                                                 \
  }

// our run callback, called each audio block
#define RUN void run(S *s, int I, float *buf)

// iterate over audio block
#define LOOP for (int i = 0; i < I; i++)

// our reload callback, called each time the client reload
#define RELOAD void reload(S *s)

#endif /* !UTIL */
