#include "xerrori.h"
#include <arpa/inet.h>
#include <sys/socket.h>

#define HOST "127.0.0.1"
#define PORT 65432

#define QUI __LINE__,__FILE__

//farm 101077744 z0.dat z1.dat 12-7803886 -n 7 -q 2 -t 100

/* Write "n" bytes to a descriptor */
ssize_t writen(int fd, void *ptr, size_t n) {  
   size_t   nleft;
   ssize_t  nwritten;
 
   nleft = n;
   while (nleft > 0) {
     if((nwritten = write(fd, ptr, nleft)) < 0) {
        if (nleft == n) return -1; /* error, return -1 */
        else break; /* error, return amount written so far */
     } else if (nwritten == 0) break; 
     nleft -= nwritten;
     ptr   += nwritten;
   }
   return(n - nleft); /* return >= 0 */
}

typedef struct {	
	int *qlen;
	int *cindex;
	char **buffer;
	pthread_mutex_t *cmutex;
	sem_t *sem_free_slots;
	sem_t *sem_data_items;
} dati;

typedef struct {
  int *flag;
} flag;

// funzione eseguita dai thread consumer
void *tbody(void *arg)
{  
  dati *a = (dati *)arg; 
	char *str;
  do {
    xsem_wait(a->sem_data_items,QUI);
		xpthread_mutex_lock(a->cmutex,QUI);
    str = a->buffer[*(a->cindex) % *(a->qlen)];
    *(a->cindex) += 1;
		xpthread_mutex_unlock(a->cmutex,QUI);
    xsem_post(a->sem_free_slots,QUI);

		if(strcmp(str,"fine")==0) break;
		
		// apro file, leggo numeri e sommo
		FILE *f = xfopen(str, "rb", QUI);

		int cont = 0;
		long num;
		long long somma = 0;
	  while(fread(&num, sizeof(long long), 1, f) == 1) {
			somma += num*cont;
			cont++;
	  }
		fclose(f);

		// invio tramite socket somma e str
		int fd_skt = 0;  // file descriptor associato al socket
	  struct sockaddr_in serv_addr;
	  size_t e;
		// crea socket
	  if ((fd_skt = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	    termina("Errore creazione socket");
	  // assegna indirizzo
	  serv_addr.sin_family = AF_INET;
	  // il numero della porta deve essere convertito 
	  // in network order 
	  serv_addr.sin_port = htons(PORT);
	  serv_addr.sin_addr.s_addr = inet_addr(HOST);
		if (connect(fd_skt, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    	termina("Errore apertura connessione");
		}
    // qui aggiungo la logica
		char s[256];
		sprintf(s, "%lld", somma);
		strcat(s," ");
		strcat(s,str);
    int len = sizeof(s);
    e = writen(fd_skt,&len,sizeof(len));
	  if(e!=sizeof(int)) termina("Errore writen");    
	  e = writen(fd_skt,&s,sizeof(s));
	  if(e!=sizeof(s)) termina("Errore writen");

    if(close(fd_skt)<0) perror("Errore chiusura socket");

  } while(true);
  pthread_exit(NULL); 
}     

// thread che effettua la gestione di sigint
void *tgestore(void *v) {
	flag *b = (flag *)v; 
  sigset_t mask;
  sigemptyset(&mask);
	sigaddset(&mask,SIGINT);
  int s;
  while(true) {
    int e = sigwait(&mask,&s);
    if(e!=0) perror("Errore sigwait");
    printf("preso il sigint\n");
		*(b->flag) = 0;
  }
  return NULL;
}



int main(int argc, char *argv[]) {
  // controlla numero argomenti
  if (argc < 2) {
    printf("Uso: %s file [file ...] \n", argv[0]);
    return 1;
  }

  sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask,SIGINT);
	pthread_sigmask(SIG_BLOCK,&mask,NULL); // blocco sigint

	//parso caratteri opzionali 
	int nthread = 4;
	int qlen = 8;
	int delay = 0;
	int opt;
	while((opt = getopt(argc, argv, ":n:q:t:")) != -1) 
    { 
        switch(opt) 
        { 
            case 'n': 
                nthread = atoi(optarg);
                break; 
            case 'q': 
                qlen = atoi(optarg);
								break;
            case 't': 
                delay = atoi(optarg);
                break; 
					 	case ':': 
                printf("option needs a value\n"); 
                break; 
            case '?': 
                printf("unknown option: %c\n", optopt);
                break; 
        } 
    } 

	//threads worker
	char *buffer[qlen];
	int pindex = 0, cindex = 0;
	pthread_mutex_t cmutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_t t[nthread];
	dati a[nthread];
	sem_t sem_free_slots, sem_data_items;
  xsem_init(&sem_free_slots,0,qlen,QUI);
  xsem_init(&sem_data_items,0,0,QUI);

  //thread gestore
  flag b[1];
  int flag = 1;
  b[0].flag = &flag;
  pthread_t gestore;
	xpthread_create(&gestore,NULL,tgestore,b,QUI);

  // faccio partire i thread 
	for(int i=0;i<nthread;i++) {
    a[i].buffer = buffer;
		a[i].qlen = &qlen;
		a[i].cindex = &cindex;
		a[i].cmutex = &cmutex;
    a[i].sem_data_items = &sem_data_items;
    a[i].sem_free_slots = &sem_free_slots;
    xpthread_create(&t[i],NULL,tbody,a+i,QUI);
  }

	//riempo buffer con nomi file
	for(; optind < argc; optind++){     
    if(flag == 0) break;
    xsem_wait(&sem_free_slots,__LINE__,__FILE__);
    buffer[pindex++ % qlen] = argv[optind];
		usleep(delay);
    xsem_post(&sem_data_items,__LINE__,__FILE__);
  }

	//termino thread con stringa fine
	for(int i=0;i<nthread;i++) {
    xsem_wait(&sem_free_slots,__LINE__,__FILE__);
    buffer[pindex++ % qlen] = "fine";
    xsem_post(&sem_data_items,__LINE__,__FILE__);
  }

	//join e calcolo
	for(int i=0;i<nthread;i++) {
    xpthread_join(t[i],NULL,__LINE__,__FILE__);
  }

  // invio tramite socket fine
  int fd_skt = 0;      // file descriptor associato al socket
  struct sockaddr_in serv_addr;
  size_t e;
  // crea socket
  if ((fd_skt = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    termina("Errore creazione socket");
  // assegna indirizzo
  serv_addr.sin_family = AF_INET;
  // il numero della porta deve essere convertito 
  // in network order 
  serv_addr.sin_port = htons(PORT);
  serv_addr.sin_addr.s_addr = inet_addr(HOST);
  if (connect(fd_skt, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    termina("Errore apertura connessione");
  }
  // qui aggiungo la logica
  char s[4] = "fine";
  int len = strlen(s);
  e = writen(fd_skt,&len,sizeof(len));
  if(e!=sizeof(int)) termina("Errore writen");    
  e = writen(fd_skt,&s,sizeof(s));
  if(e!=sizeof(s)) termina("Errore writen");

  printf("Ho oltrepassato fine\n");

  if(close(fd_skt)<0) perror("Errore chiusura socket");

  return 0;
}

