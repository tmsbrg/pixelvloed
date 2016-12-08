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
  

  char packet[1120];
  ssize_t packet_size;

  int sock_fd = get_udp_socket(UDP_PORT);
  
  // Init screen buffer
  if (!init_frame_buffer()) {
    die("Failed to init screen buffer.\n");
  }

  // draw...
  int x, y;
  uint8_t r, g, b, a;
  int pixeloffset = 1;
  int pixellength = 0;
  
  while( (packet_size = get_udp_packet(sock_fd, packet, sizeof(packet))) >= 0 ) {
    if((uint8_t)packet[0] == 1){
      pixellength = 8;
    } else {
      pixellength = 7;
    }

    // how many pixels
    //pixelcount = (packet_size-1)/pixellength;
    //printf("%d pixels in packet of size: %d using a pixel size of: %d \n", pixelcount, packet_size, pixellength);
    // fetch a pixel from the udp socket
    int i = 0;
    for(i=pixeloffset;  // skip pixeloffset
        i<packet_size-(pixellength-1); // walk trough all bytes untill we get to the end minus one package length
        i+=pixellength){
	    x = (uint8_t)packet[i  ] + (((uint8_t)packet[i+1])<<8);
	    y = (uint8_t)packet[i+2] + (((uint8_t)packet[i+3])<<8);
	    r = packet[i+4];
	    g = packet[i+5];
	    b = packet[i+6];
	    a = 0;
      //printf("%d %d %d %d %d %d\n", x, y, r, g, b, a);
      // Write pixel to screen
      write_pixel_to_screen(x, y, r, g, b, a);
    }
  }
  
  // De init screen buffer
  deinit_frame_buffer();
  
  return 0; 
}
