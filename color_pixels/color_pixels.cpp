/** Color pixels library using WS2812B and nRF51822 (16Hz)
 *  It's for
 *    + http://www.seeedstudio.com/depot/Digital-RGB-LED-FlexiStrip-60-LED-1-Meter-p-1666.html
 *    + http://www.seeedstudio.com/depot/WS2812B-Digital-RGB-LED-Waterproof-FlexiStrip-144-LEDmeter-2-meter-p-1869.html
 *    + http://www.seeedstudio.com/depot/WS2812B-RGB-LED-with-Integrated-Driver-Chip-10-PCs-pack-p-1675.html
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Seeed Technology Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "color_pixels.h"
#include "nrf51822.h"

#if !defined ( __GNUC__ )
// Generate a high level pulse (0.81us) of WS2812B's 0 code (0.9us +- 0.15us)
#define COLOR_PIXELS_ONE_HIGH(mask)     \
            NRF_GPIO->OUTSET = (mask);  \
            __ASM ( \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                ); \
            NRF_GPIO->OUTCLR = (mask)

// Generate a high level pulse (0.31us) of WS2812B's 0 code (0.35us +- 0.15us)
#define COLOR_PIXELS_ZERO_HIGH(mask)    \
            NRF_GPIO->OUTSET = (mask);  \
            __ASM (  \
                    " NOP\n\t"  \
                );  \
            NRF_GPIO->OUTCLR = (mask)
#else
// Generate a high level pulse (0.94us) of WS2812B's 0 code (0.9us +- 0.15us)
#define COLOR_PIXELS_ONE_HIGH(mask)     \
            NRF_GPIO->OUTSET = (mask);  \
            __ASM ( \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                    " NOP\n\t" \
                ); \
            NRF_GPIO->OUTCLR = (mask)

// Generate a high level pulse (0.44us) of WS2812B's 0 code (0.35us +- 0.15us)
#define COLOR_PIXELS_ZERO_HIGH(mask)    \
            NRF_GPIO->OUTSET = (mask);  \
            NRF_GPIO->OUTCLR = (mask)
#endif

#if defined ( __CC_ARM   )
static __ASM void __INLINE delay_us(uint32_t volatile number_of_us)
{
loop
        SUBS    R0, R0, #1
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        BNE    loop
        BX     LR
}
#elif defined ( __ICCARM__ )
static void __INLINE delay_us(uint32_t volatile number_of_us)
{
__ASM (
"loop:\n\t"
       " SUBS R0, R0, #1\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " NOP\n\t"
       " BNE loop\n\t");
}
#elif defined   (  __GNUC__  )
static void __INLINE delay_us(uint32_t volatile number_of_us)
{
    do 
    {
    __ASM volatile (
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
        "NOP\n\t"
    );
    } while (--number_of_us);
}
#endif

ColorPixels::ColorPixels(uint8_t pin, uint16_t num)
{
    this->pin = pin;
    this->num = num;

    NRF_GPIO->PIN_CNF[pin] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                             | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                             | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                             | (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)
                             | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos);
    NRF_GPIO->OUTCLR = (1UL << pin);

    colors = new color_t[num];
    for (int i = 0; i < num; i++) {
        colors[i].grb = 0;
    }
}

ColorPixels::~ColorPixels()
{
    delete colors;
}


void ColorPixels::set_color(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index < num) {
        colors[index].r = r;
        colors[index].g = g;
        colors[index].b = b;
    }
}

void ColorPixels::update()
{
    uint32_t mask =  1 << pin;
    NRF_GPIO->OUTCLR = mask;
    delay_us(50);
    for (int i = 0; i < num; i++) {
        uint32_t color = colors[i].grb;
        for (int bit = 0; bit < 24; bit++) {
            if (color & 1) {
                COLOR_PIXELS_ONE_HIGH(mask);
            } else {
                COLOR_PIXELS_ZERO_HIGH(mask);
            }
            color >>= 1;
        }
    }
}

void ColorPixels::clear()
{
    for (int i = 0; i < num; i++) {
        colors[i].grb = 0;
    }

    update();
}
