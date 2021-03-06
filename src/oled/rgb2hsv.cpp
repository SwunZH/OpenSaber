/*
  Copyright (c) Lee Thomason, Grinning Lizard Software

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

//
// RGB to HSV convert sample
//
//   see also : http://ja.wikipedia.org/wiki/HSV%E8%89%B2%E7%A9%BA%E9%96%93
//

#include <stdint.h>
#include <assert.h>

template<typename T> 
T minFunc(T a, T b) { return a < b ? a : b; }
template<typename T>
T maxFunc(T a, T b) { return a > b ? a : b; }
template<typename T>
T minFunc(T a, T b, T c) { return minFunc(a, minFunc(b, c)); }
template<typename T>
T maxFunc(T a, T b, T c) { return maxFunc(a, maxFunc(b, c)); }

#define SCALE 4096

// TODO: figure out range
// https://www.rapidtables.com/convert/color/rgb-to-hsv.html 
// uses 0-360, %, %
// ...or could stick with current model
// ...or stick with the current model, fix the wonky 180 thing

void rgb2hsv(uint8_t src_r, uint8_t src_g, uint8_t src_b, 
             uint8_t* dst_h, uint8_t* dst_s, uint8_t* dst_v)
{
    const int32_t r = src_r;
    const int32_t g = src_g;
    const int32_t b = src_b;

    int32_t h, s, v; // h:0-360, s:0-1024, v:0-1024

    int32_t max = maxFunc(r, g, b);
    int32_t min = minFunc(r, g, b);

    v = max * SCALE / 255;

    if (max == 0) {
        s = 0;
        h = 0;
    }
    else if (max - min == 0) {
        s = 0;
        h = 0;
    }
    else {
        s = SCALE * (max - min) / max;

        if (max == r) {
            h = 60 * (g - b) / (max - min) + 0;
        }
        else if (max == g) {
            h = 60 * (b - r) / (max - min) + 120;
        }
        else {
            h = 60 * (r - g) / (max - min) + 240;
        }
    }

    if (h < 0) h += 360;

    assert(h >= 0 && h <= 360);
    assert(s >= 0 && s <= SCALE);
    assert(v >= 0 && v <= SCALE);

    *dst_h = (uint8_t)(h / 2);          // dst_h : 0-180
    *dst_s = (uint8_t)((s * 255 + SCALE/2) / SCALE); // dst_s : 0-255
    *dst_v = (uint8_t)((v * 255 + SCALE/2) / SCALE); // dst_v : 0-255
}

void hsv2rgb(uint8_t src_h, uint8_t src_s, uint8_t src_v,
    uint8_t* dst_r, uint8_t* dst_g, uint8_t* dst_b)
{
    int32_t h = src_h * 2; // 0-360
    while (h < 0) h += 360;
    while (h >= 360) h -= 360;

    const int32_t s = src_s * SCALE / 255; // 0-SCALE
    const int32_t v = src_v * SCALE / 255; // 0-SCALE

    int32_t r = 0, g = 0, b = 0; // 0-SCALE

    const int32_t hi = h / 60;
    const int32_t f = (h - hi * 60) * SCALE / 60;
    const int32_t p = v * (SCALE - s) / SCALE;
    const int32_t q = v * (SCALE - s * f / SCALE) / SCALE;
    const int32_t t = v * (SCALE - s * (SCALE - f) / SCALE) / SCALE;

    switch (hi) {
    case 0: r = v, g = t, b = p; break;
    case 1: r = q, g = v, b = p; break;
    case 2: r = p, g = v, b = t; break;
    case 3: r = p, g = q, b = v; break;
    case 4: r = t, g = p, b = v; break;
    case 5: r = v, g = p, b = q; break;
    }

    assert(r >= 0 && r <= SCALE);
    assert(g >= 0 && g <= SCALE);
    assert(b >= 0 && b <= SCALE);

    *dst_r = (uint8_t)((r * 255 + SCALE / 2) / SCALE); // dst_r : 0-255
    *dst_g = (uint8_t)((g * 255 + SCALE / 2) / SCALE); // dst_r : 0-255
    *dst_b = (uint8_t)((b * 255 + SCALE / 2) / SCALE); // dst_r : 0-255
}

