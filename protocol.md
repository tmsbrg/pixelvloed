# The protocol for the UDP pixelvloed client and server (v1)

Pixelvloed allows clients to send up to a configurable amount of pixels per message.
Each message should contain the RGB or RGBA mode indentifier, and the version bit.
Each pixel in the message is a set of (x,y,r,g,b) or (x,y,r,g,b,a)
  X and Y are the positions on screen, and should be unsigned shorts
  rgb and a are unsigned chars

All values should be little endian.

Mixing rgb and rgba pixels in one message is not supported.

# The Autodetection system

A pixelvloed server should broadcast a package advertising its settings with the following string content

"%s:%f %s:%d %d*%d" % (PROTOCOL_PREAMBLE, PROTOCOL_VERSION, UDP_IP, UDP_PORT, width, height)

A client should listen for these packages, split it on spaces and check the preample and protocol_version to see if it is capable of speaking the requested protocol.

An example of a broadcasted packet would be:

pixelvloed:1.00 192.168.0.1:5005 1920*1080
