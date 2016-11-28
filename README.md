# pixelvloed
PixelVloed udp / binary transport PixelFlut version

You will find a bunch of files in this project, most noteworthy, A client, and 
a server for the pixelvloed protocol.

The server can be run on most machines and uses pygame or SDL to display the 
received pixels on the screen.

To start a server on a linux machine start:
  vloed.py

To start a server on a machine which has trouble getting Pygame to intall, eg a 
Mac OSX computer, start:
  sdlvloed.py
  
Both have a series of options:
  Options:
  -h, --help    show this help message and exit
  -v (displays the version)
  -i IP (The UDP listen ip address)
  -p PORT (The UDP listen port)
  -x WIDTH (The width of the display)
  -y HEIGHT (The height of the display)
  -m MAXPIXELS (the max amount of pixels people can send in one packet)
  -f FACTOR (an optional pixel zoom factor, received pixels will be zoomed by this factor)
  
If you want to run a pixelvloed server on a raspberry PI (zero) you can start:
  vloed

Which can be compiled doing:
  gcc vloed.c -o vloed
  
  
On the client side:

You can use the python version which can be started with:
  client.py
  
You can also use the javascript/node version:
  client.js
  
These by default try to find a pixelvloed server and start sending random pixels
Both clients also present options to set a static IP.

It is left as en exercise to the reader to code new fancy effects that target 
one or more screens.
