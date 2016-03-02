#!/usr/bin/python
"""This is a udp / binary version of PixelFlut

Inspired by the PixelFlut beamer on eth0:winter 2016 and
code from https://github.com/defnull/pixelflut/
"""

__version__ = 0.1
__author__ = "Jan Klopper <jan@underdark.nl>"

import pygame
import socket
import struct

def main():
  """Runs a pixelvloed server"""
  PixelVloed(debug=True)

class PixelVloed(object):
  """PixelVloed server class"""

  def __init__(self, udp_ip="127.0.0.1", udp_port=5005, debug=False):
    """Init the pixelVloed server"""
    self.debug = debug
    self.pixeloffset = 2
    self.screen = None
    self.canvas()
    self.set_title()
    self.Socket(udp_ip, udp_port)

  @staticmethod
  def set_title(text=None):
    """Sets the window title"""
    title = 'PixelVloed %0.02f' % __version__
    if text:
      title += ' ' + text
    pygame.display.set_caption(title)

  def canvas(self, width=640, height=480):
    """Init the pygame canvas"""
    pygame.init()
    self.screen = pygame.display.set_mode((width, height))

  def clear(self, r=0, g=0, b=0): # pylint: disable=C0103
    """ Fill the entire screen with a solid colour (default: black)"""
    self.screen.fill((r, g, b))

  def Pixel(self, x, y, r, g, b, a=255): # pylint: disable=C0103
    """Print a pixel to the screen"""
    self.screen.set_at((x, y), (r, g, b, a))

  def Socket(self, ipaddress, port):
    """Sets up a udp listening socket and handles the incoming binary data"""
    sock = socket.socket(socket.AF_INET, # Internet
                         socket.SOCK_DGRAM) # UDP
    sock.bind((ipaddress, port))

    while True:
      data, addr = sock.recvfrom(1024)
      preamble = struct.unpack_from("<?", data)[0]
      protocol = struct.unpack_from("<B", data, 1)[0]
      pixellength = 7 #xx,yy,r,g,b
      if preamble:
        if self.debug:
          print 'rgba mode for %s:%d' % (addr[0], addr[1])
        pixellength = 8 #xx,yy,r,g,b,a
      pixelcount = (len(data)-1) / pixellength
      if self.debug:
        print '%d pixels received, protocol V %d ' % (pixelcount, protocol)
      for i in xrange(0, pixelcount):
        pixel = struct.unpack_from(
            ("<2H4B" if preamble else "<2H3B"),
            data,
            self.pixeloffset + (i*pixellength))
        if self.debug:
          print pixel
        self.Pixel(*pixel)
      pygame.display.flip()

if __name__ == '__main__':
  main()
