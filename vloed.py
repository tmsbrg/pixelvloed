#!/usr/bin/python
"""This is a udp / binary version of PixelFlut

Inspired by the PixelFlut beamer on eth0:winter 2016 and
code from https://github.com/defnull/pixelflut/
"""

__version__ = 0.2
__author__ = "Jan Klopper <jan@underdark.nl>"

import pygame
from pygame.locals import *
import struct
import time
from gevent import spawn, socket, monkey
from gevent.server import DatagramServer
from gevent.queue import Queue

monkey.patch_all()
UDP_IP = "127.0.0.1"
UDP_PORT= 5005

def main():
  """Runs a pixelvloed server"""
  PixelVloed(':%d' %(UDP_PORT)).serve_forever()

class Canvas(object):
  """PixelVloed server class"""

  def __init__(self, queue, debug=False):
    """Init the pixelVloed server"""
    self.debug = debug
    self.pixeloffset = 2
    self.fps = 30
    self.screen = None
    self.udp_ip = UDP_IP
    self.udp_port = UDP_PORT
    self.canvas()
    self.set_title()
    self.queue = queue

  @staticmethod
  def set_title(text=None):
    """Sets the window title"""
    title = 'PixelVloed %0.02f' % __version__
    if text:
      title += ' ' + text
    pygame.display.set_caption(title)

  def canvas(self, width=1366, height=768):
    """Init the pygame canvas"""
    pygame.init()
    pygame.mixer.quit()
    flags = DOUBLEBUF
    self.screen = pygame.display.set_mode((width, height), flags)

  def clear(self, r=0, g=0, b=0): # pylint: disable=C0103
    """ Fill the entire screen with a solid colour (default: black)"""
    self.screen.fill((r, g, b))

  def Pixel(self, x, y, r, g, b, a=255): # pylint: disable=C0103
    """Print a pixel to the screen"""
    self.screen.set_at((x, y), (r, g, b, a))

  def CanvasUpdate(self):
    """Updates the screen according to self.fps"""
    lasttime = time.time()
    changed = False
    while True:
      nextdraw = time.time() + (1 / self.fps)
      changed = self.Draw(nextdraw) or changed
      if time.time() - lasttime >= 1 / self.fps:
        pygame.display.flip()
        changed = False
        lasttime = time.time()

  def Draw(self, returntime):
    """Draws pixels specified in the received packages in the queue"""
    if self.queue.empty():
      return False
    while not self.queue.empty() and time.time() < returntime:
      data = self.queue.get()
      preamble = struct.unpack_from("<?", data)[0]
      protocol = struct.unpack_from("<B", data, 1)[0]
      packetformat = ("<2H4B" if preamble else "<2H3B")
      pixellength = 7 #xx,yy,r,g,b
      if preamble:
        pixellength = 8 #xx,yy,r,g,b,a
      pixelcount = (len(data)-1) / pixellength
      if self.debug:
        print '%d pixels received, protocol V %d ' % (pixelcount, protocol)
      for i in xrange(0, pixelcount):
        pixel = struct.unpack_from(
            packetformat,
            data,
            self.pixeloffset + (i*pixellength))
        if self.debug:
          print pixel
        self.Pixel(*pixel)
    return True

class PixelVloed(DatagramServer):
  """PixelVloed server class"""
  queue = Queue()
  pixelcanvas = Canvas(queue)
  __request_processing_greenlet = spawn(pixelcanvas.CanvasUpdate)

  def handle(self, data, address):
    """Is called by the DataGramServer whenever a package is received"""
    self.queue.put(data)

if __name__ == '__main__':
  main()
