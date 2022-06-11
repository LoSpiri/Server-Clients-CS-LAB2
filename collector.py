#! /usr/bin/env python3
# server che fornisce l'elenco dei primi in un dato intervallo 
# gestisce più clienti contemporaneamente usando i thread
import sys, struct, socket, threading


# host e porta di default
HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
PORT = 65432  # Port to listen on (non-privileged ports are > 1023)

# riceve esattamente n byte e li restituisce in un array di byte
# il tipo restituto è "bytes": una sequenza immutabile di valori 0-255
# analoga alla readn che abbiamo visto nel C
def recv_all(conn,n):
  chunks = b''
  bytes_recd = 0
  while bytes_recd < n:
    chunk = conn.recv(min(n - bytes_recd, 1024))
    # if len(chunk) == 0:
      # raise RuntimeError("socket connection broken")
    chunks += chunk
    bytes_recd = bytes_recd + len(chunk)
  return chunks

# codice da eseguire nei singoli thread 
class ClientThread(threading.Thread):
    def __init__(self,conn,addr):
        threading.Thread.__init__(self)
        self.conn = conn
        self.addr = addr
    def run(self):
      # print("====", self.ident, "mi occupo di", self.addr)
      gestisci_connessione(self.conn,self.addr)
      # print("====", self.ident, "ho finito")



def main(host=HOST,port=PORT):
  # creiamo il server socket
  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    try:  
      s.bind((host, port))
      s.listen()
      while True:
        # print("In attesa di un client...")
        # mi metto in attesa di una connessione
        conn, addr = s.accept()
        # lavoro con la connessione appena ricevuta
        # t = threading.Thread(target=gestisci_connessione, args=(conn,addr))
        t = ClientThread(conn,addr)
        t.start()
    except KeyboardInterrupt:
      pass
    print('Va bene smetto...')
    s.shutdown(socket.SHUT_RDWR)
    

# gestisci una singola connessione
# con un client
def gestisci_connessione(conn,addr):
  with conn:
    # print(f"Contattato da {addr}")
    # data = recv_all(conn,4)
    data = conn.recv(4)
    size = struct.unpack("!i", data)[0]
    # print(f"La stringa sará lunga ",size)
    # data2 = recv_all(conn,size)
    data2 = conn.recv(size)
    stringa = data2.decode('ascii')
    print(f"La stringa é: {stringa}")
 


if len(sys.argv)==1:
  main()
else:
  print("Uso:\n\t %s nessun input" % sys.argv[0])


