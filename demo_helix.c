#include "pibench.h"

static uint32_t hsv_to_rgb(float h, float s, float v)
{
    float r, g, b;
    int i = floor(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6)
    {
    case 0:
        r = v, g = t, b = p;
        break;
    case 1:
        r = q, g = v, b = p;
        break;
    case 2:
        r = p, g = v, b = t;
        break;
    case 3:
        r = p, g = q, b = v;
        break;
    case 4:
        r = t, g = p, b = v;
        break;
    case 5:
        r = v, g = p, b = q;
        break;
    }

    return (0xFF << 24) | ((uint8_t)(r * 255) << 16) | ((uint8_t)(g * 255) << 8) | (uint8_t)(b * 255);
}

static void draw_circle(int x0, int y0, int radius, uint32_t color)
{
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
        for (int i = x0 - x; i <= x0 + x; i++)
        {
            if (i >= 0 && i < VIDEO_WIDTH)
            {
                if (y0 + y >= 0 && y0 + y < VIDEO_HEIGHT)
                    ((uint32_t *)frame_buf)[(y0 + y) * VIDEO_WIDTH + i] = color;
                if (y0 - y >= 0 && y0 - y < VIDEO_HEIGHT)
                    ((uint32_t *)frame_buf)[(y0 - y) * VIDEO_WIDTH + i] = color;
            }
        }
        for (int i = x0 - y; i <= x0 + y; i++)
        {
            if (i >= 0 && i < VIDEO_WIDTH)
            {
                if (y0 + x >= 0 && y0 + x < VIDEO_HEIGHT)
                    ((uint32_t *)frame_buf)[(y0 + x) * VIDEO_WIDTH + i] = color;
                if (y0 - x >= 0 && y0 - x < VIDEO_HEIGHT)
                    ((uint32_t *)frame_buf)[(y0 - x) * VIDEO_WIDTH + i] = color;
            }
        }

        y += 1;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0)
        {
            x -= 1;
            err += 1 - 2 * x;
        }
    }
}

void render_helix(float time)
{
    // Speed control parameters
    const float GLOBAL_SPEED = 5.0f;       // Overall speed multiplier
    const float ARM_ROTATION_SPEED = 1.2f; // Spiral arm rotation
    const float COLOR_SPEED = 1.3f;        // Color cycling speed

    time *= GLOBAL_SPEED;

    const float scale = 2.5f;
    const int center_x = VIDEO_WIDTH / 2;
    const int center_y = VIDEO_HEIGHT / 2;

    for (float y = -4.0f; y <= 4.0f; y += 0.04f)
    {
        // Modified calculations with speed parameters
        float q = cosf(y / (7.0f + cosf(time / 7.0f) * 3.0f) + time / 18.0f * ARM_ROTATION_SPEED) / 15.0f;
        float r = cosf(time / 12.0f + y / 14.0f) * 0.7f;

        for (int a_step = 0; a_step < 8; a_step++)
        {
            float a = (a_step / 8.0f) * M_PI * 2 + time * q;
            float x = cosf(a) * r;
            float z = r * sinf(a);
            z += 1.0f;

            if (z > 0.1f)
            {
                float sz = 1.5f / z * scale;
                int px = center_x + (x * VIDEO_WIDTH / 4) / z;
                int py = center_y + (y * VIDEO_HEIGHT / 4) / z;

                if (sz > 0.5f)
                {
                    uint32_t col = hsv_to_rgb(
                        fmodf(time * 0.1f * COLOR_SPEED + a / (M_PI * 2), 1.0f),
                        0.8f,
                        1.0f);
                    draw_circle(px, py, (int)sz, col);
                }
            }
        }
    }
}