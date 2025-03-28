#include "pibench.h"

// Example function to fill the frame buffer with random “static”
void render_noise(float current_time)
{
    // Each pixel is 4 bytes in XRGB8888 format: [X, R, G, B]
    // We'll leave X = 0 and fill R, G, and B with random values.
    uint32_t *pixels = (uint32_t *)frame_buf;
    for (int i = 0; i < VIDEO_PIXELS; ++i)
    {
        // Generate random 8-bit components
        uint8_t r = (uint8_t)(rand() & 0xFF);
        uint8_t g = (uint8_t)(rand() & 0xFF);
        uint8_t b = (uint8_t)(rand() & 0xFF);

        // Combine them into a 32-bit XRGB8888 pixel (X = 0)
        uint32_t pixel = (0 << 24) | (r << 16) | (g << 8) | (b);

        pixels[i] = pixel;
    }
}