#! /usr/bin/env python3
# server che fornisce l'elenco dei primi in un dato intervallo 
# gestisce più clienti contemporaneamente usando i thread
import sys, struct, socket, threading, os, socketserver
# modulo che contiene già un server bastao sui socket

# host e porta di default
HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)
 
class Printa(socketserver.StreamRequestHandler):
  def handle(self):
    # print(f"Gestisco richiesta da {self.client_address}")
    # sostanzialmente stesso codice di gestisci_connessione
    # tranne che usaio self.rfile e self.wfile invece del socket
    # ---- attendo due interi da 32 bit
    data = self.rfile.read(4)
    assert len(data)==4
    lunghezza = struct.unpack("!i",data[:4])[0]
    data2 = self.rfile.read(lunghezza)
    stringa = data2.decode('ascii')
    if(stringa == "fine"):
      self.server._BaseServer__shutdown_request = True
    else:
      print(f"La somma e il nome del file sono:\n{stringa}")
      # print(f"Finito con {self.client_address}")


# classe che specifica che voglio un TCP server gestito con i thread 
class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    daemon_threads = True
    allow_reuse_address = True
    
    
def main(host=HOST,port=PORT):
  with ThreadedTCPServer((host, port), Printa) as server:
    try:
      print('In attesa dei client...')
      server.serve_forever()
    except KeyboardInterrupt:
      pass
    print('Va bene smetto...')
    server.shutdown()


if len(sys.argv)==1:
  main()
else:
  print("Non serve input")
  main()


