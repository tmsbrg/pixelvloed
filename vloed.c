#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define UDP_PORT 5005    // the port users will be connecting to
#define DISCOVER_PORT 5006    // the port on which the server will broadcast its details

static void die(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

static int get_udp_socket(int port)
{
	int sock_fd;
	struct sockaddr_in6 addr;

	memset(&addr, 0, sizeof(addr));

	if ( ( sock_fd = socket(PF_INET6, SOCK_DGRAM,0) ) < 0 )
		die("could not create socket");

	addr.sin6_port = htons(port);
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = in6addr_any;

	if ( bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
		die("could not bind to UDP");

	return sock_fd;
}


static ssize_t get_udp_packet(int sock_fd, char *packet, size_t size)
{
	ssize_t n_read;
	while ((n_read = recv(sock_fd, packet, size, 0)) < 0 );
	return n_read;
}

// PixelVloed C version

// application entry point
int main(int argc, char* argv[])
{
  int fbfd = 0;
  struct fb_var_screeninfo orig_vinfo;
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;
  long int screensize = 0;
  char *fbp = 0;

  char packet[1120];
  ssize_t packet_size;

  int sock_fd = get_udp_socket(UDP_PORT);

  // Open the file for reading and writing
  fbfd = open("/dev/fb0", O_RDWR);
  if (!fbfd) {
    die("Error: cannot open framebuffer device.\n");
  }
  printf("The framebuffer device was opened successfully.\n");

  // Get variable screen information
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
    printf("Error reading variable information.\n");
  }
  printf("Original %dx%d, %dbpp\n", vinfo.xres, vinfo.yres, 
         vinfo.bits_per_pixel );

  // Store for reset (copy vinfo to vinfo_orig)
  memcpy(&orig_vinfo, &vinfo, sizeof(struct fb_var_screeninfo));

  // Change variable info
  vinfo.bits_per_pixel = 24;
  if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo)) {
    printf("Error setting variable information.\n");
  }
  
  // Get fixed screen information
  if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo)) {
    printf("Error reading fixed information.\n");
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
    die("Failed to mmap.\n");
  }
  else {
    // draw...
    int x, y, pixelcount;
    unsigned int color = 0;
    int pixeloffset = 2;
    int pixellength = 0;
    unsigned int pix_offset;
    unsigned bpp = vinfo.bits_per_pixel / 8;
    
    while( (packet_size = get_udp_packet(sock_fd, packet, sizeof(packet))) >= 0 ) {
      if((uint8_t)packet[0] == 1){
        pixellength = 8;
      } else {
        pixellength = 7;
      }

      // how many pixels
      pixelcount = (packet_size-1)/pixellength;
      //printf("%d pixels in packet of size: %d using a pixel size of: %d \n", pixelcount, packet_size, pixellength);
      // fetch a pixel from the udp socket
      int i = 0;
      for(i=pixeloffset;  // skip pixeloffset
          i<packet_size-(pixellength-1); // walk trough all bytes untill we get to the end minus one package length
          i+=pixellength){
		    x = (uint8_t)packet[i  ] + (((uint8_t)packet[i+1])<<8);
		    y = (uint8_t)packet[i+2] + (((uint8_t)packet[i+3])<<8);
        //printf("%d %d %d %d %d %d\n", x, y, packet[i  ], packet[i+1], packet[i+2], packet[i+3]);
        if(x < vinfo.xres && y < vinfo.yres){
            pix_offset = bpp * x + y * finfo.line_length;
            fbp[pix_offset++] = packet[i+4];
            fbp[pix_offset++] = packet[i+5];
            fbp[pix_offset] = packet[i+6];
        }
      }
    }
  }
  // cleanup
  munmap((void *)fbp, screensize);
  if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &orig_vinfo)) {
    printf("Error re-setting variable information.\n");
  }
  close(fbfd);
  return 0; 
}
