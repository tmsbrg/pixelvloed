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
struct fb_var_screeninfo orig_vinfo;
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
  printf("Original %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, 
         vinfo.bits_per_pixel );

  // Store for reset (copy vinfo to vinfo_orig)
  memcpy(&orig_vinfo, &vinfo, sizeof(struct fb_var_screeninfo));

  // Change variable info
  vinfo.bits_per_pixel = 24;
  if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo)) {
    printf("Error setting variable information.\n");
    return 0;
  }
  
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
  if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo)) {
    printf("Error re-setting variable information.\n");
  }
  close(fbfd);
}

void write_pixel_to_screen(uint16_t x, uint16_t y, uint16_t r, uint16_t g, uint16_t b, uint8_t a){
  unsigned int pix_offset;
  unsigned bpp = vinfo.bits_per_pixel / 8;
  if(x < vinfo.xres && y < vinfo.yres){
    pix_offset = bpp * x + y * finfo.line_length;
    fbp[pix_offset++] = r;
    fbp[pix_offset++] = g;
    fbp[pix_offset] = b;
  }
}
