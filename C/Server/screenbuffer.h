#ifndef _SCREENBUFFER_H
#define _SCREENBUFFER_H
#include <stdint.h>



int8_t init_frame_buffer(void);
void deinit_frame_buffer(void);
void write_pixel_to_screen(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

#endif
