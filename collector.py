#! /usr/bin/env python3

import sys, struct, socket, threading, os, socketserver


HOST = "127.0.0.1" # localhost
PORT = 65432 
 
class Printa(socketserver.StreamRequestHandler):
  def handle(self):
    data = self.rfile.read(4)
    assert len(data)==4
    lunghezza = struct.unpack("!i",data[:4])[0]
    data2 = self.rfile.read(lunghezza)
    stringa = data2.decode('ascii')
    if(stringa == "fine"):
      self.server._BaseServer__shutdown_request = True
    else:
      print(f"{stringa}")


class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    daemon_threads = True
    allow_reuse_address = True
    
    
def main(host=HOST,port=PORT):
  with ThreadedTCPServer((host, port), Printa) as server:
    # print('In attesa dei client')
    server.serve_forever()
    # print('Chiuso il server')


if len(sys.argv)==1:
  main()
else:
  print("Non serve input")
  main()


