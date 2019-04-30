#include <sys/types.h>// este e os 2 seguintes para o tema de mensaxes
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h> //para os prints e getChar
#include <stdlib.h>
#include <unistd.h> //para os fork
#include <pthread.h> //para os threads
#include <semaphore.h> //para os semaforos
#include <string.h> //para os strcopy

struct mensaxe {
  long mtype; //para filtrar o mensaxe
  int prio;
	int clk;
  int id_nodo;
};

//prototipos das funcións
int sendMsg();
void receiveMsg();
void *procesoReceptor();
void crearVector();
void *fillo();
void menu(int, int, int, int, int);

int N = 3; //Numero de nodos, poñoo en int para poder modificalo por parametros

#define REPLY -1
#define REQUEST 1

//Identificadores
int mi_id = 1;
int mi_prio = 0;

//Colas de mensaxes
int id_cola = 0;
int id_cola_ack = 0;

//Varibles que nos fan falla
int quero = 0;
int sc = 0;
int stop = 0;

//Semaforos de proteccion destas variables
sem_t sem_prot_quero;
sem_t sem_prot_sc;
sem_t sem_prot_stop;
sem_t sem_prot_contcolamas;
sem_t sem_prot_contcolamenos;
sem_t sem_prot_lectura;
sem_t sem_prot_ticket;

//Array de nodos
int *id_nodos;
int *id_nodos_pend;
int num_pend = 0;//nn e un array pero e sobre os nodos pendientes

//Semaforos de paso
sem_t sem_paso_fillo;
sem_t sem_paso_pai;
sem_t sem_paso_lectores;

//Semáforos de paso por tipo de proceso
sem_t sem_anulacions;
sem_t sem_pagos;
sem_t sem_reservas;
sem_t sem_lectores;

//Contadores
int num_fillos = 0;
int num_fillos_pend = 0;
int num_fillos_atend = 0;
int primeiro_paso = 1;

//Contadores de procesos
int n_anulacions = 0;
int n_pagos = 0;
int n_reservas = 0;
int n_lectores = 0;
int lector_naSC = 0;
int lectura = 0;
int lector_cola=0;

//Proteción de contadores
sem_t sem_n_anulacions;
sem_t sem_n_pagos;
sem_t sem_n_reservas;
sem_t sem_n_lectores;

//Petición de entrada / Aviso de saida
sem_t sem_entrada;
sem_t sem_salida;
int nodo_prio = 0;//este fainos falla para deterinar a donde cedemos a em

//"RELOJ GLOBAL"
int clk = 0;
int mi_clk = 0;

//variables para .sh
int exec_anulacions;
int exec_pagos;
int exec_reservas;
int exec_gradas;
int exec_eventos;

//Control tempos
FILE * Fout;
char nombre_ficheiro[30];
sem_t sem_escritura_fichero;


//------------------------------- INICIO DO MAIN ------------------------------------
//meto por parametros id nºnodos prio nºfillos
int main (int argc, char const *argv[]){
  if (argc == 8){
    mi_id = atoi(argv[1]);
    N = atoi(argv[2]);
    exec_anulacions= atoi(argv[3]);
    exec_pagos= atoi(argv[4]);
    exec_reservas= atoi(argv[5]);
    exec_gradas= atoi(argv[6]);
    exec_eventos= atoi(argv[7]);
  } else{
    printf("Indroduzca: ID Nº_NODOS Nº_ANULACIONS Nº_PAGOS Nº_PRE-RESERVAS Nº_GRADAS Nº_EVENTOS.\n");
    exit(0);
  }

  int id_nodos_aux[N-1];
  id_nodos = id_nodos_aux;
  int id_nodos_pend_aux[N-1];
  id_nodos_pend = id_nodos_pend_aux;

  pthread_t fioReceptor;

  id_cola = msgget(ftok("/tmp" , 123 ), 0777 | IPC_CREAT);
	id_cola_ack = msgget(ftok("/tmp" , 1234), 0777 | IPC_CREAT);

  printf("ID da cola de peticions: %i\n", id_cola);
	printf("ID da cola de acks: %i\n", id_cola_ack);

  //semaforos de paso
  sem_init(&sem_paso_fillo,0,0);
  sem_init(&sem_paso_pai,0,0);
  sem_init(&sem_paso_lectores,0,0);

	//Semáforos de paso por tipo de proceso
	sem_init(&sem_anulacions,0,0);
	sem_init(&sem_pagos,0,0);
	sem_init(&sem_reservas,0,0);
	sem_init(&sem_lectores,0,0);
	sem_init(&sem_entrada,0,0);
	sem_init(&sem_salida,0,0);

  //Semaforos para as variables
  sem_init(&sem_prot_quero,0,1);
  sem_init(&sem_prot_sc,0,1);
  sem_init(&sem_prot_stop,0,1);
  sem_init(&sem_prot_contcolamas,0,1);
  sem_init(&sem_prot_contcolamenos,0,1);
  sem_init(&sem_prot_lectura,0,1);
	sem_init(&sem_n_anulacions,0,1);
	sem_init(&sem_n_pagos,0,1);
	sem_init(&sem_n_reservas,0,1);
	sem_init(&sem_n_lectores,0,1);
  sem_init(&sem_prot_ticket,0,1);

  //Semaforo mutex para tempos
  sem_init(&sem_escritura_fichero, 0, 1);

  //creamos o vector cos IDc dos veciños
  crearVector();

  //Facemos un thread para o proceso receptor
  menu(exec_anulacions, exec_pagos, exec_reservas, exec_gradas, exec_eventos);
  pthread_create(&fioReceptor,NULL,procesoReceptor,"");

  while (1){
    sem_wait(&sem_entrada);

    if(n_anulacions > 0) {
      mi_prio = 1;
      //sem_post(&sem_anulacions);
    } else if(n_pagos > 0) {
      mi_prio = 2;
      //sem_post(&sem_pagos);
    } else if(n_reservas > 0) {
      mi_prio = 3;
      //sem_post(&sem_reservas);
    } else if (n_lectores > 0){
      mi_prio = 4;
    }

    sem_wait(&sem_prot_quero);
    quero = 1;
    sem_post(&sem_prot_quero);

		struct mensaxe msg;

    sem_wait(&sem_prot_ticket);
		mi_clk = clk++;
    sem_post(&sem_prot_ticket);
    for (int i = 0; i < N-1; i++){
      sendMsg(REQUEST, id_nodos[i]);
		}
    for (int i = 0; i < N-1; i++){
      receiveMsg(id_cola_ack, &msg);
    }

    if(mi_prio == 4) {
      for (int i = 0; i < num_pend; i++) {
        sendMsg(REPLY, id_nodos_pend[i]);
        num_pend = 0;
      }
    }

    sem_wait(&sem_prot_sc);
    sc = 1;
    sem_post(&sem_prot_sc);

    //--------------------------------- INICIO DA SC -----------------------------
		printf("[MAIN] Procesos activos: %i, %i, %i, %i\n", n_anulacions, n_pagos, n_reservas, n_lectores);
    int auxiliarisima=0;
    if(mi_prio == 1) {
      printf("[MAIN] Avisado anulacions\n");
      sem_post(&sem_anulacions);
    } else if(mi_prio == 2) {
      printf("[MAIN] Avisado pagos\n");
      sem_post(&sem_pagos);
    } else if(mi_prio == 3) {
      printf("[MAIN] Avisado reservas\n");
      sem_post(&sem_reservas);
    } else if(mi_prio == 4) {
      printf("[MAIN] Avisado lector\n");
      for(auxiliarisima=0; auxiliarisima<n_lectores; auxiliarisima++)//ASI ENTRAN TODOS OS QUE ESTAN NA COLA
      sem_post(&sem_lectores);
    }
		sem_wait(&sem_salida);

    // ---------------------------------SALIMOS DA SC -----------------------------------
    sem_wait(&sem_prot_sc);
    sc = 0;
    sem_post(&sem_prot_sc);

    sem_wait(&sem_prot_quero);
    quero = 0;
    sem_post(&sem_prot_quero);

    for(int i = 0; i < num_pend; i++) {
      sendMsg(REPLY, id_nodos_pend[i]);
    }
    num_pend = 0;
    num_fillos_atend = 0;
  }//--------cerra while--------
}

// -------------------------------- PROCESO RECEPTOR --------------------------------
void *procesoReceptor(){
	printf("[RECIBIDO] Receptor habilitado\n");
  while (1){
    int id_nodo_orixe, prio_orixe, clk_orixe;

		struct mensaxe msg;
 		receiveMsg(id_cola, &msg);
    id_nodo_orixe = msg.id_nodo;
    prio_orixe = msg.prio;
		clk_orixe = msg.clk;
    printf("[RECIBIDO] Orixe: %i,Prio: %i,Ticket: %i,id_nodo: %li \n",id_nodo_orixe, prio_orixe, clk, msg.mtype);

      sem_wait(&sem_prot_ticket);
			if(msg.clk > clk) clk = msg.clk;
      sem_post(&sem_prot_ticket);
			if (quero == 0 || clk_orixe < mi_clk || (id_nodo_orixe < mi_id && sc != 1) ){
      sendMsg(REPLY, id_nodo_orixe);
    	} else{
        if(prio_orixe < 4){
  				sem_wait(&sem_prot_lectura);
  				lectura = 0;
  			  sem_post(&sem_prot_lectura);
  				nodo_prio = id_nodo_orixe;
          id_nodos_pend[num_pend++] = id_nodo_orixe;
        }
        if (prio_orixe == 4){
          if(lectura == 1) {
    				   sendMsg(REPLY, id_nodo_orixe);
          } else {
            id_nodos_pend[num_pend++] = id_nodo_orixe;
          }
        }
      }

  }//--------cerra while ------------
  pthread_exit(NULL);
}

// -------------------------------- sendMsg ----------------------------------------------
int sendMsg(int tipo, int id_destino){
  struct mensaxe msg;
  msg.id_nodo = mi_id;
  msg.mtype = id_destino;
	msg.clk = mi_clk;

  if (tipo == REPLY) {
    msg.prio = REPLY;
		printf("[ENVIADO] Orixe: %i, Destino %li, Ticket, %i, ACK\n", msg.id_nodo, msg.mtype, msg.clk);
    return msgsnd(id_cola_ack, &msg, 3*sizeof(int) ,0);
  } else {
    msg.prio = mi_prio;
		printf("[ENVIADO] Orixe: %i, Destino %li, Ticket, %i, Prio %i\n", msg.id_nodo, msg.mtype, msg.clk, msg.prio);
    return msgsnd(id_cola, &msg, 3*sizeof(int),0);
  }
}

//--------------------------------- receiveMsg --------------------------------------------
void receiveMsg(int id_cola, struct mensaxe *msg) {
  msgrcv (id_cola, (struct mensaxe *)msg,3*sizeof(int),mi_id, 0);
  return;
}

//------------------------------- crearVector ---------------------------------------------
void crearVector() {
  for(int i = 0, j = 0; i < N; i++){
    if(i + 1 != mi_id){
      id_nodos[j++] = (i + 1);
    }
  }
}

// ------------------------------- fillo ------------------------------------------------
void *fillo (void *args) {
  int mia_prio;
  char proceso[50];
  int prio = 0;
  prio = *(int *)args;
  sem_post(&sem_paso_fillo);
	sem_t* sem_contador;
	sem_t* sem_proceso;
	int *contador;
  int *entrar=&lector_naSC;
  int *puntlectura=&lectura;
  int *plector_cola=&lector_cola;

  struct timeval tv_in;
  struct timeval tv_out;
  time_t curtime_in;
  time_t curtime_out;
  char buffer_in[30];
  char buffer_out[30];

  switch(prio) {
  case 1: strcpy(proceso, "anulacions");
    mia_prio=1;
		sem_contador = &sem_n_anulacions;
		sem_proceso = &sem_anulacions;
		contador = &n_anulacions;
    sem_wait(&sem_prot_lectura);
    lectura = 0;
    sem_post(&sem_prot_lectura);
		break;
  case 2: strcpy(proceso, "pagos");
    mia_prio=2;
		sem_contador = &sem_n_pagos;
		sem_proceso = &sem_pagos;
		contador = &n_pagos;
    sem_wait(&sem_prot_lectura);
    lectura = 0;
    sem_post(&sem_prot_lectura);
		break;
  case 3: strcpy(proceso, "prereservas");
    mia_prio=3;
		sem_contador = &sem_n_reservas;
		sem_proceso = &sem_reservas;
		contador = &n_reservas;
		break;
  case 4: strcpy(proceso, "gradas");
    prio=4;
    mia_prio=4;
		sem_contador = &sem_n_lectores;
		sem_proceso = &sem_lectores;
		contador = &n_lectores;
		break;
	case 5: strcpy(proceso, "eventos");
		prio= 4;
    mia_prio=4;
    sem_contador = &sem_n_lectores;
		sem_proceso = &sem_lectores;
    contador = &n_lectores;
		break;
  default : strcpy(proceso, "proceso desconocido"); break;
  }

  struct entrada {
    char proceso[30];
    int pid;
    char buffer[60];
    struct timeval tv;
  };
  struct salida {
    char proceso[30];
    int pid;
    char buffer[60];
    struct timeval tv;
  };

  struct entrada in;
  struct salida out;

  //Collemos tempos de entrada no sistema
  gettimeofday(&tv_in, NULL);
  curtime_in=tv_in.tv_sec;
  strftime(buffer_in,30,"%T.",localtime(&curtime_in));
  strcpy(in.proceso,proceso);
  in.pid = getpid();
  strcpy(in.buffer,buffer_in);
  in.tv = tv_in;
  //--------------------------------------

  if(mia_prio!=4) {
    sem_wait(&sem_prot_lectura);
    *puntlectura=0;
    sem_post(&sem_prot_lectura);
  }

	sem_wait(sem_contador);
	*contador = *contador + 1;
  if(!(mia_prio==4&&n_lectores>1)){
    sem_post(&sem_entrada);
  }
  sem_post(sem_contador);

  if(mia_prio==4&&lector_naSC==1&&lectura==1) {
  }else if(mia_prio==4) {
    sem_wait(&sem_prot_contcolamas);
    *plector_cola=*plector_cola+1;
    sem_post(&sem_prot_contcolamas);
    sem_wait(sem_proceso);
    sem_wait(&sem_prot_contcolamenos);
    *plector_cola=*plector_cola-1;
    sem_post(&sem_prot_contcolamenos);
  } else {
    sem_wait(sem_proceso);
  }

  if(mia_prio==4) {
    sem_wait(&sem_prot_lectura);
    *puntlectura=1;
    sem_post(&sem_prot_lectura);
    *entrar=1;
  }
  printf("Proceso de %s ejecuta SC\n", proceso);
  int i;
  int j;
  for (i=0;i<30;i++) {
    for (j=0;j<500000;j++) {
    }
  }

  //Collemos tempos de salida da SC
  gettimeofday(&tv_out, NULL);
  curtime_out=tv_out.tv_sec;
  strftime(buffer_out,30,"%T.",localtime(&curtime_out));
  strcpy(out.proceso,proceso);
  out.pid = getpid();
  strcpy(out.buffer,buffer_out);
  out.tv = tv_out;
  //-------------------------------
  //E gardamos no ficheiro correspondente
  sem_wait(&sem_escritura_fichero);
  sprintf(nombre_ficheiro,"%i%s.txt",N,proceso);
  Fout = fopen(nombre_ficheiro, "a");
  fprintf(Fout, "Proceso de %s %i crease - %s%ld\n", in.proceso, in.pid, in.buffer, in.tv.tv_usec);
  fprintf(Fout, "Proceso de %s %i sale - %s%ld\n", out.proceso, out.pid, out.buffer, out.tv.tv_usec);
  fclose(Fout);
  sem_post(&sem_escritura_fichero);
  //-------------------------------------

  sem_wait(sem_contador);
	*contador = *contador - 1;

  if (mia_prio==4 && n_lectores!=lector_cola ) {
    sem_post(sem_contador);
    pthread_exit(NULL);
  }

  if(mia_prio==4&&lector_cola!=0){
    sem_post(&sem_entrada);
  }

  *entrar=0;
  sem_post(sem_contador);
  sem_post(&sem_salida) ;
  pthread_exit(NULL);
}

//----------------------------------------MENU--------------------------------------
void menu(int anulacions, int pagos, int reservas, int gradas, int eventos) {

      pthread_t anulacionsFillo[anulacions];
      pthread_t pagosFillo[pagos];
      pthread_t reservasFillo[reservas];
      pthread_t gradasFillo[gradas];
      pthread_t eventosFillo[eventos];

      int prioAnulacions = 1;
      int prioPagos = 2;
      int prioReservas = 3;
      int prioGradas = 4;
      int prioEventos = 5;

      for(int i = 0; i < anulacions; i++){
        pthread_create(&anulacionsFillo[i],NULL, fillo, (void*) &prioAnulacions);
        sem_wait(&sem_paso_fillo);
      }

      for(int i = 0; i < pagos; i++){
        pthread_create(&pagosFillo[i],NULL, fillo, (void*) &prioPagos);
        sem_wait(&sem_paso_fillo);
      }
      for(int i = 0; i < reservas; i++){
        pthread_create(&reservasFillo[i],NULL, fillo, (void*) &prioReservas);
        sem_wait(&sem_paso_fillo);
      }
      for(int i = 0; i < gradas; i++){
        pthread_create(&gradasFillo[i],NULL, fillo, (void*) &prioGradas);
        sem_wait(&sem_paso_fillo);
      }
      for(int i = 0; i < eventos; i++){
        pthread_create(&eventosFillo[i],NULL, fillo, (void*) &prioEventos);
        sem_wait(&sem_paso_fillo);
    }
    return;
}
