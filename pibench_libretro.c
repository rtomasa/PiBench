#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "libretro.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define VIDEO_PIXELS VIDEO_WIDTH *VIDEO_HEIGHT

// Configuration constants
#define BASE_SIZE 140      // Base rectangle size
#define PULSE_AMPLITUDE 50 // Size variation
#define PULSE_SPEED 0.06f  // Pulsation speed
#define SCROLL_SPEED 2     // Background animation
#define COLOR_SPEED 0.03f  // Color cycling speed

static uint8_t *frame_buf;
static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static float last_aspect;
static float last_sample_rate;
char retro_base_directory[4096];
char retro_game_path[4096];

// Add performance counters
static uint64_t frame_count = 0;
static uint64_t last_frame_time = 0;

#if 0
// Add performance function prototypes
uint64_t retro_get_performance_counter(void);
uint64_t retro_get_performance_frequency(void);
#endif

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
    (void)level;
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

static retro_environment_t environ_cb;

// 2D Test Variables
static int pattern_offset = 0;
static float rotation_angle = 0.0f;

void draw_2d_test()
{
    uint32_t *ptr = (uint32_t *)frame_buf;
    static float color_angle = 0.0f;
    static float pulse_angle = 0.0f;

    // C89-compatible declarations
    int y, x, size, center_x, center_y;
    int y_start, y_end, x_start, x_end;
    uint8_t r, g, b;
    uint32_t color;

    // Updated scroll with speed constant
    pattern_offset += SCROLL_SPEED;

    // Background pattern
    for (y = 0; y < VIDEO_HEIGHT; y++)
    {
        for (x = 0; x < VIDEO_WIDTH; x++)
        {
            r = (x + pattern_offset) % 256;
            g = (y + pattern_offset / 2) % 256;
            b = (x + y + pattern_offset / 4) % 256;
            ptr[y * VIDEO_WIDTH + x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
        }
    }

    center_x = VIDEO_WIDTH / 2;
    center_y = VIDEO_HEIGHT / 2;

    // Use configuration speeds
    color_angle += COLOR_SPEED;
    pulse_angle += PULSE_SPEED;

    // Wrap angles properly
    color_angle = fmodf(color_angle, 2 * M_PI);
    pulse_angle = fmodf(pulse_angle, 2 * M_PI);

    // Use configured size parameters
    size = BASE_SIZE + PULSE_AMPLITUDE * sinf(pulse_angle);

    // Color calculation with configured speed
    r = (sinf(color_angle) + 1) * 127.5f;
    g = (sinf(color_angle + 2 * M_PI / 3) + 1) * 127.5f;
    b = (sinf(color_angle + 4 * M_PI / 3) + 1) * 127.5f;
    color = (0xFF << 24) | (r << 16) | (g << 8) | b;

    // Calculate bounds using resolution-safe macros
    y_start = MAX(center_y - size, 0);
    y_end = MIN(center_y + size, VIDEO_HEIGHT);
    x_start = MAX(center_x - size, 0);
    x_end = MIN(center_x + size, VIDEO_WIDTH);

    // Draw optimized rectangle
    for (y = y_start; y < y_end; y++)
    {
        for (x = x_start; x < x_end; x++)
        {
            ptr[y * VIDEO_WIDTH + x] = color;
        }
    }
}

void retro_init(void)
{
    frame_buf = (uint8_t *)aligned_alloc(16, VIDEO_PIXELS * sizeof(uint32_t));

    const char *dir = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
    {
        snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", dir);
    }
}

void retro_deinit(void)
{
    free(frame_buf);
    frame_buf = NULL;
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name = "PiBench";
    info->library_version = "0.1";
    info->need_fullpath = true;
    info->valid_extensions = "";
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    float aspect = 0.0f;
    float sampling_rate = 48000.0f;

    info->geometry.base_width = VIDEO_WIDTH;
    info->geometry.base_height = VIDEO_HEIGHT;
    info->geometry.max_width = VIDEO_WIDTH;
    info->geometry.max_height = VIDEO_HEIGHT;
    info->geometry.aspect_ratio = aspect;

    last_aspect = aspect;
    last_sample_rate = sampling_rate;
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    else
        log_cb = fallback_log;

    static const struct retro_controller_description controllers[] = {
        {"Retropad", RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)},
    };

    static const struct retro_controller_info ports[] = {
        {controllers, 1},
        {NULL, 0},
    };

    cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void *)ports);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

static unsigned phase;

void retro_reset(void)
{
}

static void update_input(void)
{
}

static void check_variables(void)
{
}

static void audio_callback(void)
{
}

static void audio_set_state(bool enable)
{
    (void)enable;
}

void retro_run(void)
{
#if 0
    // Performance measurement
    uint64_t current_time = retro_get_performance_counter();
    if (last_frame_time == 0)
        last_frame_time = current_time;
    double delta = (current_time - last_frame_time) / (double)retro_get_performance_frequency();
#endif
    update_input();

    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
        check_variables();

    // Clear screen
    memset(frame_buf, 0, VIDEO_PIXELS * sizeof(uint32_t));

    // Perform 2D drawing
    draw_2d_test();

    // Submit frame
    unsigned pitch = VIDEO_WIDTH * sizeof(uint32_t);
    video_cb(frame_buf, VIDEO_WIDTH, VIDEO_HEIGHT, pitch);
#if 0
    // Log performance every 60 frames
    if (frame_count++ % 60 == 0)
    {
        double fps = 1.0 / delta;
        log_cb(RETRO_LOG_INFO, "Frame time: %.2fms (%.1f FPS)\n", delta * 1000, fps);
    }
    last_frame_time = current_time;
#endif
}

bool retro_load_game(const struct retro_game_info *info)
{
    struct retro_input_descriptor desc[] = {
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right"},
        {0},
    };

    environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
        return false;
    }

    snprintf(retro_game_path, sizeof(retro_game_path), "%s", info->path);
    struct retro_audio_callback audio_cb = {NULL, NULL};
    environ_cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_cb);

    check_variables();

    (void)info;
    return true;
}

void retro_unload_game(void)
{
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
    return false;
}

size_t retro_serialize_size(void)
{
    return 0;
}

bool retro_serialize(void *data_, size_t size)
{
    return false;
}

bool retro_unserialize(const void *data_, size_t size)
{
    return false;
}

void *retro_get_memory_data(unsigned id)
{
    (void)id;
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    (void)id;
    return 0;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    (void)index;
    (void)enabled;
    (void)code;
}
