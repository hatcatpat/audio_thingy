#include <math.h>
#include <stdio.h>
#include <time.h>

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MINIAUDIO_IMPLEMENTATION
#define DEVICE_FORMAT ma_format_f32
#define DEVICE_CHANNELS 2
#define DEVICE_SAMPLE_RATE 44100
#include "libs/miniaudio.h"

#include "util.h"

#define CLIENT_FILE_NAME_A "client/client_a.so"
#define CLIENT_FILE_NAME_B "client/client_b.so"

#ifdef DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

//================================================
// BUILDING
//================================================
int use_client_A = 0;

int build() {
  printf("building %c...\n", use_client_A ? 'a' : 'b');
  int result = system("cd client && gcc -c client.c -fPIC");
  if (result == 0) {
    char client_so_file_name[50];
    sprintf(client_so_file_name,
            "cd client && gcc -shared -o client_%c.so client.o",
            use_client_A ? 'a' : 'b');
    result = system(client_so_file_name);
    if (result == 0) {
      printf("[BUILD] client shared library created successfuly!\n");
      return 0;
    } else {
      printf("[BUILD] client shared library failed!\n");
      return -1;
    }
  } else {
    printf("[BUILD] client compiling failed!\n");
    return -1;
  }
}

//================================================
// STATE
//================================================

S s;

static inline void init_S(S *s, int sr, int nc) {
  s->o = NULL;
  s->no = 0;

  s->sr = sr;
  s->nc = nc;
  s->t = 0;
};

// nearest power of 2 to input
static inline uint np2(uint x) {
  uint v = x;
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v == x ? 2 * v : v;
}

#define RESIZE_IMPL(NUM, NAME, TYPE, INIT)                                     \
  if (NUM) {                                                                   \
    if (!s->NAME || s->NUM != NUM) {                                           \
      s->NAME = (TYPE *)realloc(s->NAME, (size_t)np2(NUM * sizeof(TYPE)));     \
      if (NUM > s->NUM) {                                                      \
        for (int i = s->NUM; i < NUM; i++) {                                   \
          INIT;                                                                \
        }                                                                      \
      }                                                                        \
      s->NUM = NUM;                                                            \
    }                                                                          \
  } else {                                                                     \
    free(s->NAME);                                                             \
  }

// allocates memory for the arrays in S
static void resize_S(S *s, int nb, int no, int nf) {

  RESIZE_IMPL(nb, b, buf, init_buf(i));
  RESIZE_IMPL(no, o, osc, init_osc(i, 0.0));
  RESIZE_IMPL(nf, f, float, s->f[i] = 0.0);

  if (DEBUG) {
    printf("[DEBUG] nb = %i\n", s->nb);
    printf("[DEBUG] no = %i\n", s->no);
    printf("[DEBUG] nf = %i\n", s->nf);
  }
}

//================================================
// RELOADING
//================================================

// function typedefs

// called from audio thread
#define CLIENT_RUN_NAME "run"
typedef void CLIENT_RUN(S *s, int frame_count, float *outbuf);

// called every time the client is reloaded
#define CLIENT_RELOAD_NAME "reload"
typedef void CLIENT_RELOAD(S *s);

// get the correct number of elements in our dynamic structs
#define CLIENT_SET_SIZES_NAME "set_sizes"
typedef void CLIENT_SET_SIZES(int *nb, int *no, int *nf);

//================================================
// dynamic libraries
void *client_lib;
CLIENT_RUN *client_run;
int nb = 0;
int no = 0;
int nf = 0;
int client_run_lock = 0;

// loads the dynamic libraries
void load_client() {

  void *lib = dlopen(use_client_A ? "client/client_a.so" : "client/client_b.so",
                     RTLD_NOW | RTLD_LOCAL);

  if (lib) {
    printf("[RELOAD] dl library loaded\n");

    use_client_A = !use_client_A;

    CLIENT_RUN *run = (CLIENT_RUN *)dlsym(lib, CLIENT_RUN_NAME);

    if (run) {
      printf("[RELOAD] client run loaded\n");

      CLIENT_RELOAD *reload = (CLIENT_RELOAD *)dlsym(lib, CLIENT_RELOAD_NAME);
      CLIENT_SET_SIZES *set_sizes =
          (CLIENT_SET_SIZES *)dlsym(lib, CLIENT_SET_SIZES_NAME);

      if (reload && set_sizes) {
        printf("[RELOAD] client reload/set_sizes loaded\n");

        // spin-lock to wait for audio to release client
        while (client_run_lock)
          continue;

        client_run_lock = 1;

        // sets our n_ values to match the client
        set_sizes(&nb, &no, &nf);
        // allocates correct memory for our new S
        resize_S(&s, nb, no, nf);
        // calls client reload function
        reload(&s);

        client_run = run;

        client_run_lock = 0;

        if (client_lib)
          dlclose(client_lib);
        client_lib = lib;

        printf("[RELOAD] reload suceeded :)\n");
      } else {
        printf("[RELOAD] client reload/set_sizes failed to load!\n");
      }

    } else {
      printf("[RELOAD] client run failed to load!\n");
    }
  } else {
    printf("[RELOAD] dl failed to load!\n");
    dlclose(lib);
  }
}

//================================================
// AUDIO
//================================================
void data_callback(ma_device *device, void *output, const void *input,
                   ma_uint32 frame_count) {
  float *outbuf = (float *)output;

  while (client_run_lock)
    continue;

  client_run_lock = 1;
  if (client_run) {
    client_run(&s, frame_count, outbuf);
  }
  client_run_lock = 0;
}

//================================================
int main(int argc, char **argv) {
  // MA
  ma_device_config deviceConfig;
  ma_device device;

  deviceConfig = ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format = DEVICE_FORMAT;
  deviceConfig.playback.channels = DEVICE_CHANNELS;
  deviceConfig.sampleRate = DEVICE_SAMPLE_RATE;
  deviceConfig.dataCallback = data_callback;

  use_client_A = 0;
  if (build())
    return -1;
  use_client_A = 1;
  if (build())
    return -1;

  srand(time(NULL));
  init_S(&s, DEVICE_SAMPLE_RATE, DEVICE_CHANNELS);
  load_client();

  if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
    printf("[MA] failed to open playback device\n");
    return -4;
  }

  printf("[MA] device name: %s\n", device.playback.name);

  if (ma_device_start(&device) != MA_SUCCESS) {
    printf("[MA] Failed to start playback device.\n");
    ma_device_uninit(&device);
    return -5;
  }

  printf("\nplaying ... type 'quit' to quit\n");

  char line[100];
  while (1) {
    printf(">> ");
    fgets(line, sizeof(line), stdin);
    if (strstr(line, "reload") != NULL) {
      build();
      // for (int i = 0; i < 100; i++)
      load_client();
    } else if (strstr(line, "quit") != NULL) {
      printf("quit!\n");
      break;
    } else if (strstr(line, "build") != NULL) {
      build();
    }
  }

  ma_device_uninit(&device);

  (void)argc;
  (void)argv;
  return 0;
}
