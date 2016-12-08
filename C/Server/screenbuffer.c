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
  

int8_t init_frame_buffer(void)
{
  // Open the file for reading and writing
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
  printf("Screen info %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, 
         vinfo.bits_per_pixel );

  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    printf("Error reading fixed information.\n");
    return 0;
  }

  // map fb to user mem 
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


void deinit_frame_buffer(void)
{
  // cleanup
  munmap((void *)fbp, screensize);
  close(fbfd);
}

void write_pixel_to_screen(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a){
  uint32_t pix_offset;
  uint8_t bpp = vinfo.bits_per_pixel / 8;
  union {
    uint32_t pixel32;
    uint8_t pixel8[4];
  } pixel;
  uint8_t i;
  
  if(x < vinfo.xres && y < vinfo.yres){
    pixel.pixel32 = 0;
    pixel.pixel32 |= ((uint32_t)r >> (8 - vinfo.red.length)) << vinfo.red.offset;
    pixel.pixel32 |= ((uint32_t)g >> (8 - vinfo.green.length)) << vinfo.green.offset;
    pixel.pixel32 |= ((uint32_t)b >> (8 - vinfo.blue.length)) << vinfo.blue.offset;
    
    pix_offset = bpp * x + y * finfo.line_length;
    for (i=0; i<bpp; i++) {
      fbp[pix_offset+i] = pixel.pixel8[i];
    }
  }
}
