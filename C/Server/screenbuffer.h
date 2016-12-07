#ifndef _SCREENBUFFER_H
#define _SCREENBUFFER_H
#include <stdint.h>



int8_t init_frame_buffer(void);
void deinit_frame_buffer(void);
void write_pixel_to_screen(uint16_t x, uint16_t y, uint16_t r, uint16_t g, uint16_t b, uint8_t a);

#endif
