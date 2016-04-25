#!/usr/bin/python
"""This is an udp / binary client

Inspired by the PixelFlut beamer on eth0:winter 2016 and
code from https://github.com/defnull/pixelflut/
"""

__version__ = 0.2
__author__ = "Jan Klopper <jan@underdark.nl>"

MAX_PROTOCOL_VERSION = 1;
PROTOCOL_PREAMBLE = 'pixelvloed'

import socket
import struct
import random
import time

class MaxSizeList(list):

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
      message[2:] = []
      message.append(pixel)
  yield ''.join(message)

def SendPacket(ipaddress, port, message):
  """Sends the message to the udp server"""
  sock = socket.socket(socket.AF_INET, # Internet
                       socket.SOCK_DGRAM) # UDP
  sock.sendto(message, (ipaddress, port))

def DiscoverServers(discoveryport, timeout=5):
  """Discover servers that send out the pixelfvloed preample"""
  DiscoverySock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  DiscoverySock.bind(('', discoveryport))
  starttime = time.time()
  servers = []
  foundhash = {}
  while (time.time() - timeout) < starttime:
    data, addr = DiscoverySock.recvfrom(1024)
    try:
      if data.startswith(PROTOCOL_PREAMBLE):
        dataset = data.split(' ')
        if float(dataset[0].split(':')[1]) <= MAX_PROTOCOL_VERSION:
          ip = dataset[1].split(':')[0]
          port = int(dataset[1].split(':')[1])
          width = int(dataset[2].split('*')[0])
          height = int(dataset[2].split('*')[1])
          if data not in foundhash:
            newserver = {'ip': ip,
                         'port': port,
                         'width': width,
                         'height': height}
            foundhash[data] = True
            print 'New pixelvloed screen found: %r' % newserver
            servers.append(newserver)
    except:
      pass
  if servers:
    return servers
  return False

def main():
  """Discover the servers and start sending to the first one"""
  discoveryport = 5006
  servers = False
  while servers == False:
    servers = DiscoverServers(discoveryport)
  print 'displaying on %(ip)s:%(port)d, %(width)d*%(height)dpx' % servers[0]
  while True:
    for packet in RandomFill(servers[0]['width'], servers[0]['height']):
      time.sleep(0.01)
      SendPacket(servers[0]['ip'], servers[0]['port'], packet)

if __name__ == '__main__':
  main()
