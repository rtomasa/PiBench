#include "pibench.h"
#include "font.h"
#include "libretro.h"

#ifdef __linux__

static uint64_t last_total = 0;
static uint64_t last_active = 0;

int get_cpu_core_count(void)
{
    return sysconf(_SC_NPROCESSORS_ONLN); // Online/available cores
    // or _SC_NPROCESSORS_CONF for configured cores
}

float get_cpu_temperature(void)
{
    static int last_thermal_fd = -1;
    static char thermal_path[256] = {0};
    char buf[16];

    // Find thermal zone (try first 6 zones)
    if (last_thermal_fd == -1)
    {
        for (int i = 0; i < 6; i++)
        {
            snprintf(thermal_path, sizeof(thermal_path),
                     "/sys/class/thermal/thermal_zone%d/temp", i);
            int fd = open(thermal_path, O_RDONLY);
            if (fd != -1)
            {
                last_thermal_fd = fd;
                break;
            }
        }
        if (last_thermal_fd == -1)
            return -1;
    }

    lseek(last_thermal_fd, 0, SEEK_SET);
    ssize_t bytes = read(last_thermal_fd, buf, sizeof(buf) - 1);
    if (bytes <= 0)
        return -1;
    buf[bytes] = '\0';

    long temp = strtol(buf, NULL, 10);
    return temp / 1000.0f; // Convert millidegrees to Celsius
}

float get_cpu_usage(void)
{
    static int fd = -1;
    char buf[512];
    ssize_t bytes;
    uint64_t user, nice, system, idle, iowait, irq, softirq;
    uint64_t total, active;

    if (fd == -1)
        fd = open("/proc/stat", O_RDONLY);
    if (fd == -1)
        return -1;

    lseek(fd, 0, SEEK_SET);
    bytes = read(fd, buf, sizeof(buf) - 1);
    if (bytes <= 0)
        return -1;
    buf[bytes] = '\0';

    // Parse first line containing aggregate CPU stats
    sscanf(buf, "cpu  %lu %lu %lu %lu %lu %lu %lu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq);

    active = user + nice + system + irq + softirq;
    total = active + idle + iowait;

    // Calculate differentials
    uint64_t total_diff = total - last_total;
    uint64_t active_diff = active - last_active;

    last_total = total;
    last_active = active;

    return (total_diff > 0) ? (active_diff * 100.0f / total_diff) : 0.0f;
}

float get_process_cpu_usage(void)
{
    static uint64_t last_time = 0;
    static uint64_t last_utime = 0;
    static uint64_t last_stime = 0;

    int fd = open("/proc/self/stat", O_RDONLY);
    if (fd == -1)
        return -1;

    char buf[1024];
    ssize_t bytes = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (bytes <= 0)
        return -1;
    buf[bytes] = '\0';

    // Parse process stats (fields 14-17 are utime, stime, cutime, cstime)
    unsigned long utime, stime, cutime, cstime;
    sscanf(buf, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %lu %lu",
           &utime, &stime, &cutime, &cstime);

    uint64_t total_time = (utime + stime + cutime + cstime);
    uint64_t current_time = perf.get_time_usec();

    float usage = 0.0f;
    if (last_time != 0 && current_time > last_time)
    {
        uint64_t time_diff = current_time - last_time;
        uint64_t cpu_diff = (total_time - (last_utime + last_stime)) * 1000000 / sysconf(_SC_CLK_TCK);

        usage = (cpu_diff * 100.0f) / time_diff;
    }

    last_time = current_time;
    last_utime = utime;
    last_stime = stime;

    return usage;
}
#endif

// Function to draw text using the font
void draw_text_alpha(int x, int y, const char *text, uint32_t color)
{
    uint32_t *ptr = (uint32_t *)frame_buf;
    int start_x = x;
    for (const char *c = text; *c; ++c)
    {
        uint8_t ch = (uint8_t)*c;
        if (ch >= 128)
            continue;
        for (int row = 0; row < 8; row++)
        {
            uint8_t bits = font[ch][row];
            for (int col = 0; col < 8; col++)
            {
                if (bits & (0x80 >> col))
                {
                    int px = x + col;
                    int py = y + row;
                    if (px >= 0 && px < VIDEO_WIDTH && py >= 0 && py < VIDEO_HEIGHT)
                    {
                        ptr[py * VIDEO_WIDTH + px] = color;
                    }
                }
            }
        }
        x += 8; // Move to next character
    }
}

void draw_text_bg(int x, int y, const char *text, uint32_t color)
{
    uint32_t *ptr = (uint32_t *)frame_buf;
    const uint32_t background = 0xFF000000; // Black background

    for (const char *c = text; *c; ++c)
    {
        uint8_t ch = (uint8_t)*c;
        if (ch >= 128)
            continue;

        // Draw 8x8 character block
        for (int row = 0; row < 8; row++)
        {
            uint8_t bits = font[ch][row];
            for (int col = 0; col < 8; col++)
            {
                int px = x + col;
                int py = y + row;

                if (px >= 0 && px < VIDEO_WIDTH && py >= 0 && py < VIDEO_HEIGHT)
                {
                    // Set pixel to either character color or background
                    ptr[py * VIDEO_WIDTH + px] = (bits & (0x80 >> col)) ? color : background;
                }
            }
        }
        x += 8; // Move to next character
    }
}