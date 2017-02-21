#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "screenbuffer.h"

#define UDP_PORT 5005    // the port users will be connecting to
#define DISCOVER_PORT 5006    // the port on which the server will broadcast its details

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

#define MAX_PIXELS 140

static void die(char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}

static int get_udp_socket(int port) {
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


static ssize_t get_udp_packet(int sock_fd, char *packet, size_t size) {
	ssize_t n_read;
	while ((n_read = recv(sock_fd, packet, size, 0)) < 0 );
	return n_read;
}

// PixelVloed C version

// application entry point
int main(int argc, char* argv[]) {
  uint8_t packet[ (8 * MAX_PIXELS) + 32]; // 8 bytes per pixel is the maximum size
  ssize_t packet_size;
  uint16_t x, y;
  uint8_t r, g, b, a;
  uint8_t pixellength = 0, protocol, version;
  uint16_t i;
  uint8_t offset = 2;

  int sock_fd = get_udp_socket(UDP_PORT);

  // Init screen buffer
  if (!init_frame_buffer()) {
    die("Failed to init screen buffer.\n");
  }

  // draw each udp packet
  while( (packet_size = get_udp_packet(sock_fd, (char*)&packet, sizeof(packet))) >= 0 ) {
    // Get protocol from the packet
    version = packet[1];    // the version
    protocol = packet[0];   // protocol switch
    
    //printf("V:%d, P:%d\n", version, protocol);
    
    switch (protocol) {
      // Protocol 0: xyrgb 16:16:8:8:8 specified for each pixel
      case 0:
        pixellength = 7;
        packet_size = MIN(offset + (pixellength * MAX_PIXELS), packet_size);
        for (i=offset; i<packet_size; i+=pixellength) {
          x = packet[i  ] | packet[i+1]<<8;
	        y = packet[i+2] | packet[i+3]<<8;
	        r = packet[i+4];
	        g = packet[i+5];
	        b = packet[i+6];
	        a = 0xFF;
	        write_pixel_to_screen(x, y, r, g, b, a);
	      }
	      break;

	    // Protocol 1: xyrgba 16:16:8:8:8:8 specified for each pixel
      case 1:
        pixellength = 8;
        packet_size = MIN(offset + (pixellength * MAX_PIXELS), packet_size);
        for (i=offset; i<packet_size; i+=pixellength) {
          x = packet[i  ] | packet[i+1]<<8;
	        y = packet[i+2] | packet[i+3]<<8;
	        r = packet[i+4];
	        g = packet[i+5];
	        b = packet[i+6];
	        a = packet[i+7];
	        write_pixel_to_screen(x, y, r, g, b, a);
	      }
	      break;

	    // Protocol 2: xyrgb 12:12:8:8:8 specified for each pixel
      case 2:
        pixellength = 6;
        packet_size = MIN(offset + (pixellength * MAX_PIXELS), packet_size);
        for (i=offset; i<packet_size; i+=pixellength) {
          x = packet[i  ] | (packet[i+1]&0xF0)<<4;
	        y = (packet[i+1]&0x0F) | packet[i+2]<<4;
	        r = packet[i+3];
	        g = packet[i+4];
	        b = packet[i+5];
	        a = 0xFF;
	        write_pixel_to_screen(x, y, r, g, b, a);
	      }
	      break;

	    // Protocol 3: xyrgba 12:12:8:8:8:8 specified for each pixel
      case 3:
        pixellength = 7;
        packet_size = MIN(offset + (pixellength * MAX_PIXELS), packet_size);
        for (i=offset; i<packet_size; i+=pixellength) {
          x = packet[i  ] | (packet[i+1]&0xF0)<<4;
	        y = (packet[i+1]&0x0F) | packet[i+2]<<4;
	        r = packet[i+3];
	        g = packet[i+4];
	        b = packet[i+5];
	        a = packet[i+6];
	        write_pixel_to_screen(x, y, r, g, b, a);
	      }
	      break;

	    // Protocol 4: xyrgb 12:12:3:3:2 specified for each pixel
      case 4:
        pixellength = 4;
        packet_size = MIN(offset + (pixellength * MAX_PIXELS), packet_size);
        for (i=offset; i<packet_size; i+=pixellength) {
          x = packet[i  ] | (packet[i+1]&0xF0)<<4;
	        y = (packet[i+1]&0x0F) | packet[i+2]<<4;
	        r = packet[i+3] & 0xE0;
	        g = packet[i+3]<<3 & 0xE0;
	        b = packet[i+3]<<6 & 0xC0;
	        a = 0xFF;
	        write_pixel_to_screen(x, y, r, g, b, a);
	      }
	      break;

	    // Protocol 5: xyrgba 12:12:2:2:2:2 specified for each pixel
      case 5:
        pixellength = 4;
        packet_size = MIN(offset + (pixellength * MAX_PIXELS), packet_size);
        for (i=offset; i<packet_size; i+=pixellength) {
          x = packet[i  ] | (packet[i+1]&0xF0)<<4;
	        y = (packet[i+1]&0x0F) | packet[i+2]<<4;
	        r = packet[i+3] & 0xC0;
	        g = packet[i+3]<<2 & 0xC0;
	        b = packet[i+3]<<4 & 0xC0;
	        a = packet[i+3]<<6 & 0xC0;
	        write_pixel_to_screen(x, y, r, g, b, a);
	      }
	      break;

	    // Error no recognied protocol defined
	    default:
	      break;
	  }
  }

  // Deinitialize screen buffer
  deinit_frame_buffer();
  return 0; 
}
