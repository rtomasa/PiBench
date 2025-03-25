#include "pibench.h"
#include "font.h"
#include "libretro.h"

// Global extern
uint8_t *frame_buf;
struct retro_perf_callback perf;

// Retro callbacks
static retro_environment_t environ_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static struct retro_perf_counter frame_counter;
static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static float last_aspect;
static float last_sample_rate;
char retro_base_directory[4096];
char retro_game_path[4096];
static char fps_str[32] = "REAL TIME FPS: 0";
static char fps_avg_str[32] = "AVERAGE FPS: 0";
static char cpu_str[32] = "CPU MULTI-CORE: ---%";
static char cpu_proc_str[32] = "CPU SINGLE-CORE: ---%";
static char temp_str[32] = "CPU TEMPERATURE: ---C";
static app_state_t current_state = STATE_MENU;

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
    (void)level;
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

void retro_init(void)
{
    frame_buf = (uint8_t *)aligned_alloc(16, VIDEO_PIXELS * sizeof(uint32_t));

    const char *dir = NULL;
    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
    {
        snprintf(retro_base_directory, sizeof(retro_base_directory), "%s", dir);
    }

    // Initialize performance counter according to API specs
    frame_counter.ident = "frame";
    frame_counter.start = 0;
    frame_counter.total = 0;
    frame_counter.call_cnt = 0;
    frame_counter.registered = false; // Must initialize as false

    if (environ_cb(RETRO_ENVIRONMENT_GET_PERF_INTERFACE, &perf))
    {
        perf.perf_register(&frame_counter);
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

void retro_reset(void)
{
}

static void update_input(void)
{
    input_poll_cb();
    if (current_state == STATE_MENU)
    {
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
        {
            current_state = STATE_2D_TEST;
        }
    }
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
    static uint64_t last_log_time = 0;
    static uint64_t current_frames = 0;
    static uint64_t avg_counter = 0;
    static uint64_t warm_up_counter = 0;
    static double avg_fps = 0;

    // Start timing
    uint64_t frame_start = perf.get_time_usec ? perf.get_time_usec() : 0;
    if (perf.perf_start)
        perf.perf_start(&frame_counter);

    update_input();

    bool updated = false;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
        check_variables();

    // For demoscene spiral
    static float start_time = 0;
    if (start_time == 0 && perf.get_time_usec)
        start_time = perf.get_time_usec() / 1000000.0f;

    float time = (perf.get_time_usec() / 1000000.0f) - start_time;

    // Clear screen
    memset(frame_buf, 0, VIDEO_PIXELS * sizeof(uint32_t));

    if (current_state == STATE_MENU)
    {
        // Draw menu text
        const char *msg = "PRESS START TO BEGIN 2D TEST";
        int msg_width = strlen(msg) * 8;
        int x = (VIDEO_WIDTH - msg_width) / 2;
        int y = VIDEO_HEIGHT / 2 - 4;
        draw_text_bg(x, y, msg, 0xFFFFFFFF);
    }
    else
    {
        // draw_2d_test();
        render_spiral(time);
        // Display on-screen info in top left
        draw_text_bg(32, 32, fps_str, 0xFFFFFFFF);
        draw_text_bg(32, 40, fps_avg_str, 0xFFFFFFFF);
        draw_text_bg(32, 48, cpu_str, 0xFFFFFFFF);
        draw_text_bg(32, 56, cpu_proc_str, 0xFFFFFFFF);
        draw_text_bg(32, 64, temp_str, 0xFFFFFFFF);
    }

    // Submit frame
    unsigned pitch = VIDEO_WIDTH * sizeof(uint32_t);
    video_cb(frame_buf, VIDEO_WIDTH, VIDEO_HEIGHT, pitch);

    // Stop timing
    if (perf.perf_stop)
        perf.perf_stop(&frame_counter);
    uint64_t frame_end = perf.get_time_usec ? perf.get_time_usec() : 0;

    // Update counters
    current_frames++;

    // Log every second
    if (frame_end - last_log_time >= 1000000)
    {
        warm_up_counter++;

        double current_fps = current_frames;
        if (warm_up_counter > WARM_UP_FPS)
        {
            avg_fps = avg_fps + current_frames;
            avg_counter++;
        }

#ifdef __linux__
        // Update CPU usage
        float cpu_usage = get_cpu_usage();
        if (cpu_usage >= 0)
        {
            snprintf(cpu_str, sizeof(cpu_str), "CPU MULTI-CORE (%d): %d%%", get_cpu_core_count(), (int)cpu_usage);
        }
        float cpu_proc_usage = get_process_cpu_usage();
        if (cpu_proc_usage >= 0)
        {
            snprintf(cpu_proc_str, sizeof(cpu_proc_str), "CPU SINGLE-CORE: %d%%", (int)cpu_proc_usage);
        }

        // Update CPU temperature
        float temp = get_cpu_temperature();
        if (temp >= 0)
        {
            snprintf(temp_str, sizeof(temp_str), "CPU TEMPERATURE: %dC", (int)temp);
        }
        else
        {
            strncpy(temp_str, "CPU TEMPERATURE: ---C", sizeof(temp_str));
        }
#endif

        // Update FPS display string
        snprintf(fps_str, sizeof(fps_str), "REAL TIME FPS: %d", (int)current_fps);
        snprintf(fps_avg_str, sizeof(fps_avg_str), "AVERAGE FPS: %d", (int)(avg_fps / (double)avg_counter));

        if (perf.perf_log)
            perf.perf_log();

        log_cb(RETRO_LOG_INFO,
               "FPS: %d, AVG: %d",
               (int)current_fps,
               (int)(avg_fps / (double)avg_counter));

        // Reset counters
        last_log_time = frame_end;
        current_frames = 0;
        frame_counter.total = 0; // Reset performance counters
        frame_counter.call_cnt = 0;
    }
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
    (void)type;
    (void)info;
    (void)num;
    return false;
}

size_t retro_serialize_size(void)
{
    return 0;
}

bool retro_serialize(void *data_, size_t size)
{
    (void)data_;
    (void)size;
    return false;
}

bool retro_unserialize(const void *data_, size_t size)
{
    (void)data_;
    (void)size;
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
