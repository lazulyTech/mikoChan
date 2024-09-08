#include "mikoPalette.h"

/* namespace mikoPalette { */
/* uint16_t skin = lgfx::color565(243, 176, 139); */
/* uint16_t eye_upper = lgfx::color565(56, 73, 33); */
/* uint16_t eye_lower = lgfx::color565(138, 165, 92); */
/* uint16_t mouth = lgfx::color565(254, 171, 166); */
/* uint16_t cheek = lgfx::color565(235, 160, 156); */
/* uint16_t edge = lgfx::color565(0, 0, 0); */
/* }  // namespace mikoPalette */

namespace mikoPalette {
colorPalette skin(246, 205, 159);
/* colorPalette skin(250, 226, 207); */
colorPalette eye_upper(56, 73, 33);
colorPalette eye_lower(138, 165, 92);
colorPalette mouth(254, 171, 166);
colorPalette cheek(235, 160, 156);
colorPalette edge(0, 0, 0);
colorPalette hot(255, 0, 0);
colorPalette cold(50, 255, 255);
}  // namespace mikoPalette

colorPalette::colorPalette(uint8_t r, uint8_t g, uint8_t b) {
    rgb.r = r;
    rgb.g = g;
    rgb.b = b;
    c24 = r * 0x010000 + g * 0x000100 + b * 0x000001;
}

colorPalette::colorPalette(uint32_t c24) {
    this->c24 = c24;
}

colorPalette::colorPalette(HSL hsl) {
    this->hsl.h = hsl.h;
    this->hsl.s = hsl.s;
    this->hsl.l = hsl.l;
    c24 = HSLtoRGB24(hsl);
}

void colorPalette::set(RGB rgb) {
    this->rgb.r = rgb.r;
    this->rgb.g = rgb.g;
    this->rgb.b = rgb.b;
    c24 = rgb.r * 0x010000 + rgb.g * 0x000100 + rgb.b * 0x000001;
}

void colorPalette::set(uint32_t c24) {
    this->c24 = c24;
}

void colorPalette::set(HSL hsl) {
    this->hsl.h = hsl.h;
    this->hsl.s = hsl.s;
    this->hsl.l = hsl.l;
    c24 = HSLtoRGB24(hsl);
}

HSL* colorPalette::getHSL() {
    RGB24toHSL(c24, &hsl);
    return &hsl;
}
RGB* colorPalette::getRGB() {
    int r = (c24 & 0xFF0000) >> 4 * 4;
    int g = (c24 & 0x00FF00) >> 4 * 2;
    int b = (c24 & 0x0000FF) >> 4 * 0;
    return &rgb;
}
uint32_t colorPalette::getRGB24() {
    return c24;
}
uint16_t colorPalette::getRGB16() {
    return lgfx::LGFXBase::color24to16(c24);
}
uint8_t colorPalette::getRGB8() {
    return lgfx::LGFXBase::color332(rgb.r, rgb.g, rgb.b);
}

void colorPalette::RGB24toHSL(uint32_t color, HSL* hsl) {
    int r = (color & 0xFF0000) >> 4 * 4;
    int g = (color & 0x00FF00) >> 4 * 2;
    int b = (color & 0x0000FF) >> 4 * 0;
    /* printf("R:%d, G:%d, B:%d\n", color24_r, color24_g, color24_b); */
    int max = 0, min = 255;
    int maxC = 0;  // r:0, g:1, b:2
    if (r >= g && r >= b) {
        maxC = 0;
        max = r;
    }
    if (g >= r && g >= b) {
        maxC = 1;
        max = g;
    }
    if (b >= r && b >= g) {
        maxC = 2;
        max = b;
    }
    if (r <= g && r <= b) min = r;
    if (g <= r && g <= b) min = g;
    if (b <= r && b <= g) min = b;

    // from https://www.peko-step.com/tool/hslrgb.html
    // Hue
    if (r == g && g == b)
        hsl->h = 0;
    else {
        switch (maxC) {
            case 0:
                hsl->h = 60 * (1.0f * (g - b) / (max - min));
                break;
            case 1:
                hsl->h = 60 * (1.0f * (b - r) / (max - min)) + 120;
                break;
            case 2:
                hsl->h = 60 * (1.0f * (r - g) / (max - min)) + 240;
                break;
        }
    }

    if (hsl->h < 0) hsl->h += 360;

    // Saturation
    if ((max + min) / 2 <= 128)
        hsl->s = 100.0f * (max - min) / (max + min);
    else
        hsl->s = 100.0f * (max - min) / (510 - max - min);

    // Lightness
    hsl->l = 100.0f / 255.0f * (max + min) / 2;
}

uint32_t colorPalette::HSLtoRGB24(HSL hsl) {
    int r, g, b;
    double h = hsl.h, s = hsl.s, l = hsl.l;
    double max, min;
    if (hsl.l >= 50) {
        max = 2.55f * (l + (100 - l) * (s / 100.0f));
        min = 2.55f * (l - (100 - l) * (s / 100.0f));
    } else {
        max = 2.55f * (l + l * (s / 100.0f));
        min = 2.55f * (l - l * (s / 100.0f));
    }

    if (h >= 0 && h < 60) {
        r = max;
        g = (h / 60) * (max - min) + min;
        b = min;
    }
    if (h >= 60 && h < 120) {
        r = ((120 - h) / 60) * (max - min) + min;
        g = max;
        b = min;
    }
    if (h >= 120 && h < 180) {
        r = min;
        g = max;
        b = ((h - 120) / 60) * (max - min) + min;
    }
    if (h >= 180 && h < 240) {
        r = min;
        g = ((240 - h) / 60) * (max - min) + min;
        b = max;
    }
    if (h >= 240 && h < 300) {
        r = ((h - 240) / 60) * (max - min) + min;
        g = min;
        b = max;
    }
    if (h >= 300 && h <= 360) {
        r = max;
        g = min;
        b = ((360 - h) / 60) * (max - min) + min;
    }
    int color = (uint8_t)r * 0x010000 + (uint8_t)g * 0x000100 + (uint8_t)b * 0x000001;
    return color;
}
