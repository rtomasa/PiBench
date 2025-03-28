#include "pibench.h"

// Pico‑8 16‑color palette (XRGB888 format)
static const uint32_t pico_palette[16] = {
    0x000000, // 0: Black
    0x1D2B53, // 1: Dark Blue
    0x7E2553, // 2: Purple
    0x008751, // 3: Green
    0xAB5236, // 4: Brown
    0x5F574F, // 5: Dark Gray
    0xC2C3C7, // 6: Light Gray
    0xFFF1E8, // 7: White
    0xFF004D, // 8: Red
    0xFFA300, // 9: Orange
    0xFFEC27, // 10: Yellow
    0x00E436, // 11: Lime Green
    0x29ADFF, // 12: Blue
    0x83769C, // 13: Lavender
    0xFF77A8, // 14: Pink
    0xFFCCAA  // 15: Peach
};

void render_laser(float time)
{
    const int CENTER_X = VIDEO_WIDTH / 2;
    const int CENTER_Y = VIDEO_HEIGHT / 2;
    const float BASE_RADIUS = 180.0f;
#define LINE_THICKNESS 4 // Added thickness control

    static const uint8_t color_remap[16] = {0, 0, 0, 0, 8, 8, 14, 14, 7, 7, 7, 7, 7, 7, 7, 7};
    static uint8_t pixel_buffer[VIDEO_HEIGHT][VIDEO_WIDTH] = {0};

    time *= 15.0f;
    memset(pixel_buffer, 0, sizeof(pixel_buffer));

    for (float i = 0; i < 24.0f; i += 0.25f)
    {
        float angle0 = i * time / 240.0f;
        float angle1 = angle0 + i * time / 160.0f;

        float x0 = CENTER_X + cosf(angle0) * BASE_RADIUS;
        float y0 = CENTER_Y + sinf(angle0) * BASE_RADIUS;
        float x1 = CENTER_X + cosf(angle1) * BASE_RADIUS;
        float y1 = CENTER_Y + sinf(angle1) * BASE_RADIUS;

        float dx = (x1 - x0) / 128.0f;
        float dy = (y1 - y0) / 128.0f;

        float x = x0, y = y0;
        for (int j = 0; j < 128; j++)
        {
            int ix = (int)x;
            int iy = (int)y;

            // Draw thick points
            for (int tx = -LINE_THICKNESS / 2; tx <= LINE_THICKNESS / 2; tx++)
            {
                for (int ty = -LINE_THICKNESS / 2; ty <= LINE_THICKNESS / 2; ty++)
                {
                    int nx = ix + tx;
                    int ny = iy + ty;
                    if (nx >= 0 && nx < VIDEO_WIDTH && ny >= 0 && ny < VIDEO_HEIGHT)
                    {
                        pixel_buffer[ny][nx] = (pixel_buffer[ny][nx] + 1) % 16;
                    }
                }
            }

            x += dx;
            y += dy;
        }
    }

    for (int y = 0; y < VIDEO_HEIGHT; y++)
    {
        for (int x = 0; x < VIDEO_WIDTH; x++)
        {
            uint8_t color_idx = color_remap[pixel_buffer[y][x]];
            ((uint32_t *)frame_buf)[y * VIDEO_WIDTH + x] = pico_palette[color_idx];
        }
    }
}