#!/usr/bin/python
"""This is an udp / binary client

Inspired by the PixelFlut beamer on eth0:winter 2016 and
code from https://github.com/defnull/pixelflut/
"""

__version__ = 0.1
__author__ = "Jan Klopper <jan@underdark.nl>"

import socket
import struct
import random
import time

class MaxSizeList(list):
  maxsize = 0

  def __init__(self, maxcount=100):
    self.maxsize = maxcount
    super( MaxSizeList, self ).__init__()

  def append(self, item):
    super( MaxSizeList, self ).append(item)
    if self.__len__() == self.maxsize:
      raise IndexError('max size reached')


def RGBPixel(x, y, r, g, b, a=None): # pylint: disable=C0103
  """Generates the packed data for a pixel"""
  if a:
    return struct.pack("<2H4B", x, y, r, g, b, a)
  return struct.pack("<2H3B", x, y, r, g, b)

def SetRGBAMode(mode):
  """Generate the rgb/rgba bit"""
  return struct.pack("<?", mode)

def SetVersionBit(protocol=1):
  """Generate the Version bit"""
  return struct.pack("<B", protocol)

def RandomFill(width=640, height=480):
  """Generates a random number of pixels with a random color"""
  message = MaxSizeList(140)
  message.append(SetRGBAMode(False))
  message.append(SetVersionBit())
  for pixel in xrange(0, random.randint(10, 100)):
    pixel = RGBPixel(random.randint(0, width),
                     random.randint(0, height),
                     random.randint(0, 255),
                     random.randint(0, 255),
                     random.randint(0, 255))
    try:
      message.append(pixel)
    except IndexError:
      yield ''.join(message)
      message[:] = []
      message.append(pixel)
  yield ''.join(message)

def SendPacket(ipaddress, port, message):
  """Sends the message to the udp server"""
  sock = socket.socket(socket.AF_INET, # Internet
                       socket.SOCK_DGRAM) # UDP
  sock.sendto(message, (ipaddress, port))

def main():
  """Sends some random pixels untill we break with ctrl+c"""
  ipaddress = "127.0.0.1"
  port = 5005

  while True:
    for packet in RandomFill():
      time.sleep(0.01)
      SendPacket(ipaddress, port, packet)

if __name__ == '__main__':
  main()
