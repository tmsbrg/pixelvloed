#!/usr/bin/python
"""This is an udp / binary client

Inspired by the PixelFlut beamer on eth0:winter 2016 and
code from https://github.com/defnull/pixelflut/
"""

__version__ = 0.3
__author__ = "Jan Klopper <jan@underdark.nl>"

import random
from vloed import PixelVloedClient, NewMessage, RGBPixel

def RandomFill(message, width, height):
  """Generates a random number of pixels with a random color"""
  for pixel in xrange(0, random.randint(10, 140)):
    pixel = RGBPixel(random.randint(0, width),
                     random.randint(0, height),
                     random.randint(0, 255),
                     random.randint(0, 255),
                     random.randint(0, 255))
    try:
      message.append(pixel)
    except IndexError:
      yield ''.join(message)
      message[2:] = []
  yield ''.join(message)

def RunClient():
  """Discover the servers and start sending to the first one"""

  client = PixelVloedClient(True, # start as soon as we find a server
                            False, # show debugging output
                            None, # ip of the server, None for autodetect
                            None, # port of the server None for autodetect
                            None, # Screen pixels wide, or Autodetect
                            None  # Screen pixels height, or Autodetect
                            )
  message = NewMessage() #create a new message that buffers the output etc

  # loop the effect untill we cancel by pressing ctrl+c / exit the program
  while True:
    # create a new message and send if everytime the buffer is full
    # the width/height are read from the client's config
    for packet in RandomFill(message, client.width, client.height):
      # send the message we just filled with random pixelfs
      client.SendPacket(packet)

if __name__ == '__main__':
  # if this script is called from the commandline, and thus not imported
  # Start a client and start sending messages
  RunClient()
