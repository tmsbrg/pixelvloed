#!/usr/bin/python
"""This is an udp / binary client

Inspired by the PixelFlut projector on eth0:winter 2016 and
code from https://github.com/defnull/pixelflut/
"""

__version__ = 0.3001
__author__ = "Thomas van der Berg <ik@thomasvanderberg.nl>"

import random
from PIL import Image
import cv
from vloed import *

def RandomFill(pixels, width, height):
  """Generates a random number of pixels with a random color"""
  for pixel in xrange(0, random.randint(10, MAX_PIXELS)):
    pixel = RGBPixel(random.randint(0, width),
                     random.randint(0, height),
                     random.randint(0, 255),
                     random.randint(0, 255),
                     random.randint(0, 255))
    pixels.append(pixel)

def ColorLine(pixels, width, y, r, g, b):
  """Generates a random number of pixels with a random color"""
  for pixel in xrange(0, random.randint(10, MAX_PIXELS)):
    pixel = RGBPixel(random.randint(0, width), y, r, g, b)
    pixels.append(pixel)

def DrawImage(image, pixels):
    width = image.size[0]
    height = image.size[1]
    for y in xrange(0, height):
        for x in xrange(0, width):
            rgb = image.getpixel((x, y))
            pixel = RGBPixel(x, y, rgb[0], rgb[1], rgb[2])
            pixels.append(pixel)

prepared_image_packets = []

def PrepareImage(image):
    image_pixels = []
    width = image.size[0]
    height = image.size[1]
    for y in xrange(0, height):
        for x in xrange(0, width):
            rgb = image.getpixel((x, y))
            pixel = RGBPixel(x, y, rgb[0], rgb[1], rgb[2])
            image_pixels.append(pixel)
    random.shuffle(image_pixels)
    for packet_start in xrange(0, len(image_pixels), MAX_PIXELS):
        msg = []
        InitMessage(msg)
        packet_end = packet_start + MAX_PIXELS
        msg += image_pixels[packet_start:packet_end]
        prepared_image_packets.append(''.join(msg))

def DrawPreparedImage(client):
    for packet in prepared_image_packets:
        client.SendPacket(packet)

def RunClient(options):
  """Discover the servers and start sending to the first one"""

  client = PixelVloedClient(True, # start as soon as we find a server
                            options.debug, # show debugging output
                            options.ip, # ip of the server, None for autodetect
                            options.port, # port of the server None for autodetect
                            options.width, # Screen pixels wide, None for autodetect
                            options.height  # Screen pixels height, None for autodetect
                            )

  image = Image.open("fracklogo.png")
  PrepareImage(image)

  # cap = cv.CaptureFromCAM(0)

  # loop the effect until we cancel by pressing ctrl+c / exit the program
  while True:
    # frame = cv.QueryFrame(cap)

    # packet will automatically send its pixels if gets to the maximum pixel length
    #pixels = Packet(client)

    # add some pixels to the output with our functions
    # the width/height are read from the client's config
    DrawPreparedImage(client)

    # send whatever pixels are left in the packet
    #pixels.flush()


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
