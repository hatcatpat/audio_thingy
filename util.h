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

#endif /* !UTIL */
