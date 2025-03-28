#include "pibench.h"

// Bresenham's line algorithm implementation
static void draw_line(float x0, float y0, float x1, float y1, uint32_t color)
{
    int ix0 = (int)x0, iy0 = (int)y0;
    int ix1 = (int)x1, iy1 = (int)y1;

    int dx = abs(ix1 - ix0), sx = ix0 < ix1 ? 1 : -1;
    int dy = -abs(iy1 - iy0), sy = iy0 < iy1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1)
    {
        if (ix0 >= 0 && ix0 < VIDEO_WIDTH && iy0 >= 0 && iy0 < VIDEO_HEIGHT)
        {
            ((uint32_t *)frame_buf)[iy0 * VIDEO_WIDTH + ix0] = color;
        }

        if (ix0 == ix1 && iy0 == iy1)
            break;
        e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            ix0 += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            iy0 += sy;
        }
    }
}

void render_radial_lines(float time)
{
    const float SCALE = 5.0f; // 128 -> 640 (5x scale)
    const int CENTER_X = VIDEO_WIDTH / 2;
    const int CENTER_Y = VIDEO_HEIGHT / 2;
    
    // PICO-8 inspired palette (index 8-15)
    static const uint32_t pal[] = {
        0xFFFF004D,  // Red       (8)
        0xFFFFA300,  // Orange    (9)
        0xFFFFEC27,  // Yellow    (10)
        0xFF00E436,  // Green     (11)
        0xFF29ADFF,  // Blue      (12)
        0xFF83769C,  // Lavender  (13)
        0xFFFF77A8,  // Pink      (14)
        0xFFFFCCAA   // Peach     (15)
    };

    for(int r = 4; r <= 128; r += 4) {
        for(float i = 0.0f; i < 1.75f; i += 0.25f) {
            float q = fmodf(time * (1.0f + r/32.0f) / 8.0f, 1.0f);
            float v0 = fmaxf(fminf((q - i) * 4.0f, 1.0f), 0.0f);
            float v1 = fmaxf(fminf((q - i) * 4.0f + 2.0f, 1.0f), 0.0f);

            if(v1 > v0) {
                // Calculate start position
                float a = i - 0.125f;
                float x = CENTER_X + cosf(a * M_PI * 2) * r * 0.71f * SCALE;
                float y = CENTER_Y + sinf(a * M_PI * 2) * r * 0.71f * SCALE;

                // Calculate direction vector
                a += 0.375f; // 3/8 converted to 0.375
                float u = cosf(a * M_PI * 2);
                float v = sinf(a * M_PI * 2);

                // Calculate line endpoints
                float x0 = x + u * v0 * r * SCALE;
                float y0 = y + v * v0 * r * SCALE;
                float x1 = x + u * v1 * r * SCALE;
                float y1 = y + v * v1 * r * SCALE;

                // Select color from palette
                int color_idx = 8 + ((r/4) % 8);
                draw_line(x0, y0, x1, y1, pal[color_idx - 8]);
            }
        }
    }
}