#include "pibench.h"

void render_test(float current_time) {
    // Clear screen to black (XRGB8888 format)
    memset(frame_buf, 0, VIDEO_WIDTH * VIDEO_HEIGHT * sizeof(uint32_t));

    for (int q = 1; q <= 3; q++) {
        int p = 1 << q;  // p = 2, 4, 8
        float phase_x = current_time / 6.0f + p / 3.0f;
        float phase_y = current_time / 5.0f + p / 5.0f;
        
        // Calculate center position (scaled to 640x480)
        int center_x = 320 + (int)(cosf(phase_x) * 21.0f);
        int center_y = 240 + (int)(cosf(phase_y) * 25.0f);

        for (int r = 128; r >= 4; r -= 4) {
            // Determine color - only draw colored pixels
            uint32_t color = 0;
            if (r & 4) {  // Only draw colored bands
                switch (p) {
                    case 2:  color = 0x00FF0000; break;  // Red
                    case 4:  color = 0x0000FF00; break;  // Green
                    case 8:  color = 0x000000FF; break;  // Blue
                }
            }
            if (!color) continue;  // Skip transparent pixels

            // Draw filled circle (only colored regions)
            for (int y = -r; y <= r; y++) {
                int current_y = center_y + y;
                if (current_y < 0 || current_y >= VIDEO_HEIGHT) continue;
                
                int width = (int)sqrtf(r * r - y * y);
                int start_x = center_x - width;
                int end_x = center_x + width;
                
                // Clamp to screen bounds
                start_x = start_x < 0 ? 0 : start_x;
                end_x = end_x >= VIDEO_WIDTH ? VIDEO_WIDTH - 1 : end_x;

                // Fill scanline with color
                uint32_t *row = (uint32_t *)frame_buf + current_y * VIDEO_WIDTH;
                for (int x = start_x; x <= end_x; x++) {
                    row[x] = color;
                }
            }
        }
    }
}