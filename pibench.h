#ifndef PIBENCH_H__
#define PIBENCH_H__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define VIDEO_PIXELS VIDEO_WIDTH *VIDEO_HEIGHT

#define BASE_SIZE 140      // Base rectangle size
#define PULSE_AMPLITUDE 50 // Size variation
#define PULSE_SPEED 0.06f  // Pulsation speed
#define SCROLL_SPEED 2     // Background animation
#define WARM_UP_FPS 2      // Number of warm up frames

typedef enum {
    STATE_MENU,
    STATE_2D_TEST
} app_state_t;

extern uint8_t *frame_buf;
extern struct retro_perf_callback perf;

void render_spiral(float);
int get_cpu_core_count(void);
float get_cpu_temperature(void);
float get_cpu_usage(void);
float get_process_cpu_usage(void);
void draw_text_alpha(int, int, const char *, uint32_t);
void draw_text_bg(int, int, const char *, uint32_t);

#endif