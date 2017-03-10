/***************************************************
 * The following frame buffer modes are supported:
 * - 24bpp - rgba 8/16,8/8,8/0,0/0
 * - 32bpp - rgba 8/24,8/16,8/8,0,0
 * 
 * Use "fbset -s" to get the currently used mode
 **************************************************/
#include "screenbuffer.h"

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
  
int8_t init_frame_buffer(void) {
  // Open the framebuffer file for reading and writing
  fbfd = open("/dev/fb0", O_RDWR);
  if (!fbfd) {
    printf("Error: cannot open framebuffer device.\n");
    return 0;
  }
  printf("The framebuffer device was opened successfully.\n");

  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
    printf("Error reading variable information.\n");
    return 0;
  }
  printf("Screen info %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    printf("Error reading fixed information.\n");
    return 0;
  }
  
  // Check if the framebuffer mode is supported
  if ((vinfo.bits_per_pixel == 24 || vinfo.bits_per_pixel == 32) &&
       vinfo.red.length == 8      && vinfo.red.offset == 16 &&
       vinfo.green.length == 8    && vinfo.green.offset == 8 &&
       vinfo.blue.length == 8     && vinfo.blue.offset == 0 &&
       vinfo.transp.length == 0   && vinfo.transp.offset == 0) {
    printf("Screen buffer mode is supported.\n");
  } else {
    printf("ERROR: Screen buffer mode is not supported.\n");
    return 0;
  }

  // Map fb to user mem 
  screensize = finfo.smem_len;
  fbp = (char*)mmap(0, 
                    screensize, 
                    PROT_READ | PROT_WRITE, 
                    MAP_SHARED, 
                    fbfd, 
                    0);

  if ((long)fbp == -1) {
    printf("Error: Failed to mmap.\n");
    return 0;
  }
  return 1;
}


void deinit_frame_buffer(void) {
  // cleanup
  munmap((void *)fbp, screensize);
  close(fbfd);
}

void write_pixel_to_screen(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  uint32_t pix_offset;
  uint16_t temp_r, temp_g, temp_b;
  
  // If the pixel is out of the screen there is not need to draw it
  if (x >= vinfo.xres || y >= vinfo.yres) {
    return;
  }

  //pix_offset = byte_pp * x + y * finfo.line_length;
  
  // If alpha is used calculate it, otherwise just paint the colors
  if (a != 0xFF) {
    // Get the old pixel
    if (vinfo.bits_per_pixel == 24) { // 24bpp format
      pix_offset = 3 * x + y * finfo.line_length;
      temp_r = fbp[pix_offset+2];
      temp_g = fbp[pix_offset+1];
      temp_b = fbp[pix_offset+0];
    } else if (vinfo.bits_per_pixel == 32) { // 32bpp format
      pix_offset = 4 * x + y * finfo.line_length;
      temp_r = fbp[pix_offset+2];
      temp_g = fbp[pix_offset+1];
      temp_b = fbp[pix_offset+0];
    } else {
      temp_r = 0;
      temp_g = 0;
      temp_b = 0;
    }
    //printf("%08x : %04x %04x %04x\n", pixel.pixel32, temp_r, temp_g, temp_b);
    // Apply alpha
    temp_r = (temp_r * (256-a))>>8;
    temp_r = temp_r + ((r * (a))>>8);
    temp_g = (temp_g * (256-a))>>8;
    temp_g = temp_g + ((g * (a))>>8);
    temp_b = (temp_b * (256-a))>>8;
    temp_b = temp_b + ((b * (a))>>8);
  } else {
    temp_r = r;
    temp_g = g;
    temp_b = b;
  }
  
  // Write new pixels to buffer
  if (vinfo.bits_per_pixel == 24) { // 24bpp format
    pix_offset = 3 * x + y * finfo.line_length;
  } else if (vinfo.bits_per_pixel == 32) { // 32bpp format
    pix_offset = 4 * x + y * finfo.line_length;
  }
  fbp[pix_offset+2] = temp_r;
  fbp[pix_offset+1] = temp_g;
  fbp[pix_offset+0] = temp_b;
}
