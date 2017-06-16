#!/usr/bin/python
"""This is an udp / binary client

Inspired by the PixelFlut projector on eth0:winter 2016 and
code from https://github.com/defnull/pixelflut/
"""

__version__ = 0.3
__author__ = "Jan Klopper <jan@underdark.nl>"

import random
from vloed import PixelVloedClient, Packet, RGBPixel, MAX_PIXELS

def RandomFill(screen, width, height):
  """Generates a random number of pixels with a random color"""
  for _x in xrange(1, # at least one pixel
                   random.randint(10, MAX_PIXELS) # at most the max number of pixels
                   ):
    pixel = RGBPixel(random.randint(0, width), # select a random position on the width of the screen
                     random.randint(0, height), # select a random position on the height of the screen
                     random.randint(0, 255), # select a random value for red
                     random.randint(0, 255), # select a random value for green
                     random.randint(0, 255) # select a random value for blue
                     )
    screen.show(pixel) # lets push the pixel to the screen!

def RunClient(options):
  """Discover the servers and start sending to the first one"""

  client = PixelVloedClient(True, # start as soon as we find a server
                            options.debug, # show debugging output
                            options.ip, # ip of the server, None for autodetect
                            options.port, # port of the server None for autodetect
                            options.width, # Screen pixels wide, None for autodetect
                            options.height  # Screen pixels height, None for autodetect
                            )

  # Lets create a screen which buffers the pixels we add to it, and sends them to the actual screen.
  screen = Packet(client)
  # loop the effect until we cancel by pressing ctrl+c / exit the program
  while screen:

    # add some pixels to the screen with our functions
    # the width/height are read from the client's config
    RandomFill(screen, client.width, client.height)

if __name__ == '__main__':
  # if this script is called from the command line, and thus not imported
  # start a client and start sending messages
  import optparse
  parser = optparse.OptionParser()
  parser.add_option('-v', action="store_true", dest="debug", default=False)
  parser.add_option('-i', action="store", dest="ip", default=None)
  parser.add_option('-p', action="store", dest="port", default=None,
                    type="int")
  parser.add_option('-x', action="store", dest="width", default=None,
                    type="int")
  parser.add_option('-y', action="store", dest="height", default=None,
                    type="int")
  options, remainder = parser.parse_args()

  try:
    RunClient(options)
  except KeyboardInterrupt:
    print 'Closing client'
