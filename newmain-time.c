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
void *fillo(void *args);
void *menu();

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

//Array de nodos
int *id_nodos;
int *id_nodos_pend;
int num_pend = 0;//nn e un array pero e sobre os nodos pendientes

//Semaforos de paso
sem_t sem_paso_fillo;
sem_t sem_paso_pai;
sem_t sem_paso_lectores;
sem_t sem_paso_simu;

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
int lector_naSC=0;
int lectura=0;
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

//Control tempos
FILE * Fout;
char nombre_ficheiro[30];

//------------------------------- INICIO DO MAIN ------------------------------------
//meto por parametros id nºnodos prio nºfillos
int main (int argc, char const *argv[]){
  if (argc == 3){
    mi_id = atoi(argv[1]);
    N = atoi(argv[2]);
  } else{
    printf("Indroduzca: ID Nº_NODOS.\n");
    exit(0);
  }

  /*sprintf(nombre_ficheiro,"%ianulaciones.txt",N);
  Fout[1] = fopen (nombre_ficheiro, "a");
  sprintf(nombre_ficheiro,"%ipagos.txt",N);
  Fout[2] = fopen (nombre_ficheiro, "a");
  sprintf(nombre_ficheiro,"%iprereservas.txt",N);
  Fout[3] = fopen (nombre_ficheiro, "a");
  sprintf(nombre_ficheiro,"%igradas.txt",N);
  Fout[4] = fopen (nombre_ficheiro, "a");
  sprintf(nombre_ficheiro,"%ieventos.txt",N);
  Fout[5] = fopen (nombre_ficheiro, "a");*/

  int id_nodos_aux[N-1];
  id_nodos = id_nodos_aux;
  int id_nodos_pend_aux[N-1];
  id_nodos_pend = id_nodos_pend_aux;

  pthread_t fioReceptor;
	pthread_t fioMenu;

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

  //creamos o vector cos IDc dos veciños
  crearVector();

  //Facemos un thread para o proceso receptor
  pthread_create(&fioReceptor,NULL,procesoReceptor,"");
	pthread_create(&fioMenu,NULL,menu,"");

  while (1){
    sem_wait(&sem_entrada);

    printf("[MAIN] Buscado prio entre %i, %i, %i, %i\n", n_anulacions, n_pagos, n_reservas, n_lectores);
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

    //Se nn e a primeira vez, pulsamos unha tecla para crear o fillo
    //if (!quero && !primeiro_paso) {

      /* printf("Esperando os novos fillos. \n");
      getchar(); // solo vale para volver a intentar ejecutar outra vez todos os procesos fillo que xa fixemos antes
      for(int i = 0; i < num_fillos; i++){
	pthread_create(&fioFillo[i], NULL, fillo, "");
      }*/

      //--------------------------------------_MENUUU---------------------------------------------

      // }else {
      //primeiro_paso = 0;
      //}

    sem_wait(&sem_prot_quero);
    quero = 1;
    sem_post(&sem_prot_quero);

		struct mensaxe msg;

		mi_clk = clk + 1;
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
    printf("[MAIN] O nodo %i gana a 'exclusion mutua' entre nodos\n", mi_id);
		printf("[MAIN] Buscado prioritario entre %i, %i, %i, %i\n", n_anulacions, n_pagos, n_reservas, n_lectores);
    int auxiliarisima=0;
    if(n_anulacions > 0) {
      printf("[MAIN] Avisado anulacions\n");
      sem_post(&sem_anulacions);
    } else if(n_pagos > 0) {
      printf("[MAIN] Avisado pagos\n");
      sem_post(&sem_pagos);
    } else if(n_reservas > 0) {
      printf("[MAIN] Avisado reservas\n");
      sem_post(&sem_reservas);
    } else if(n_lectores > 0) {
      printf("[MAIN] Avisado lector\n");
      for(auxiliarisima=0; auxiliarisima<n_lectores; auxiliarisima++)//ASI ENTRAN TODOS OS QUE ESTAN NA COLA
      sem_post(&sem_lectores);
    }
		sem_wait(&sem_salida);

    /*num_fillos_pend = num_fillos - num_fillos_atend;
    for (int i = 0; i < num_fillos_pend; i++){
      sem_post(&sem_paso_fillo);
      num_fillos_atend ++;
      //para escritores
      if (mi_prio != 4){
	sem_wait(&sem_paso_pai);
      }
      sem_wait(&sem_prot_stop);
      if(stop == 1){
	sem_post(&sem_prot_stop);
	printf("Hai unha petición mais prioritara e deixo de dar paso aos procesos de prio menor.\n");
	sendMsg(REPLY, nodo_prio);
	break;
      }
      sem_post(&sem_prot_stop);
    }
    //para lectores
    if(mi_prio == 4){
      for (int i = 0; i < num_fillos; i++){
	printf("Esperando o lector %i\n" , i);
	sem_wait(&sem_paso_lectores);
	stop = 0;
      }
    }
    if (stop == 1){
      primeiro_paso = 1;
      stop = 0;
      continue;
    }
		*/
    // ---------------------------------SALIMOS DA SC -----------------------------------
    sem_wait(&sem_prot_sc);
    sc = 0;
    sem_post(&sem_prot_sc);

    sem_wait(&sem_prot_quero);
    quero = 0;
    sem_post(&sem_prot_quero);

    printf("[MAIN]Numero de nodos pendentes: %i\n",num_pend);
    for(int i = 0; i < num_pend; i++) {
      printf("[MAIN]Avisando a pendiente: %i\n",num_pend);
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
		printf("[RECIBIDO] Orixe: %i,Prio: %i,Ticket: %i,id_nodo: %li \n",msg.id_nodo, msg.prio, msg.clk, msg.mtype);
    id_nodo_orixe = msg.id_nodo;
    prio_orixe = msg.prio;
		clk_orixe = msg.clk;

			if(clk_orixe > clk) clk = clk_orixe;
			if (quero == 0 || /*(prio_orixe < mi_prio && sc != 1) ||*/ clk_orixe < mi_clk ||
					 /*(prio_orixe == mi_prio &&*/ (id_nodo_orixe < mi_id && sc != 1) ){
      sendMsg(REPLY, id_nodo_orixe);
    	} else{
      printf("[RECIBIDO]Añadido a pendentes\n");
      if(prio_orixe < 4){
				sem_wait(&sem_prot_lectura);
				lectura = 0;
			  sem_post(&sem_prot_lectura);
				printf("[RECIBIDO] Recibido escritura dende %i, por ende, desactivamos lectura\n", id_nodo_orixe);
				nodo_prio = id_nodo_orixe;

				if (prio_orixe == 4){
          if(lectura == 1) {
	 				   sendMsg(REPLY, id_nodo_orixe);
          } else {
            printf("[RECIBIDO]Lector añadido a pendentes\n");
            id_nodos_pend[num_pend++] = id_nodo_orixe;
          }
				}else{
          printf("[RECIBIDO]Escritor añadido a pendentes\n");
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
		printf("[ENVIADO] Orixe: %i, Destino %li, Ticket, %i, Prio %i\n", msg.id_nodo, msg.mtype, msg.clk, msg.prio);
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

  struct timeval tv;
  time_t curtime;
  char buffer[30];

  switch(prio) {
  case 1: strcpy(proceso, "anulacions");
    mia_prio=1;
		sem_contador = &sem_n_anulacions;
		sem_proceso = &sem_anulacions;
		contador = &n_anulacions;
		break;
  case 2: strcpy(proceso, "pagos");
    mia_prio=2;
		sem_contador = &sem_n_pagos;
		sem_proceso = &sem_pagos;
		contador = &n_pagos;
		break;
  case 3: strcpy(proceso, "pre-reservas");
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
	printf("Creado proceso de %s\n", proceso);
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

  //----------------------------------------------------------//
  sprintf(nombre_ficheiro,"%i%s.txt",N,proceso);
  Fout = fopen(nombre_ficheiro, "a");





  gettimeofday(&tv, NULL);
  curtime=tv.tv_sec;
  strftime(buffer,30,"%T.",localtime(&curtime));
  printf("%li\n",sizeof(buffer));
  fprintf(Fout, "Proceso de %s %i entra - %s%ld\n",proceso,getpid(),buffer,tv.tv_usec);

  //----------------------------------------------------------//

  printf("[PROCESO] %s na SC PULSA ENTER PARA CONTINUAR\n", proceso);
	// paramos dentro da sc ata que lle damos enter
  sem_wait(&sem_paso_simu);
  printf("Proceso de %s salindo da SC\n", proceso);

  //----------------------------------------------------------//

  gettimeofday(&tv, NULL);
  curtime=tv.tv_sec;
  strftime(buffer,30,"%T.",localtime(&curtime));
  fprintf(Fout, "Proceso de %s %i sale - %s%ld\n",proceso,getpid(),buffer,tv.tv_usec);
  //fputs("Hola", Fout[prio]);
  fclose(Fout);

  //----------------------------------------------------------//

  sem_wait(sem_contador);
	*contador = *contador - 1;
  printf("A VER,O NUMERO DE LECTORES TOTAL E %i, E OS LECTORES EN COLA SON %i  \n",n_lectores,lector_cola);

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
  /*if (mi_prio == 4){
    sem_post(&sem_paso_lectores);
  }*/ //                      SEÑORES ESTO NON SE EXECUTA NUNCA!!! QUE MI_PRIO DA VALORES MALOS, E O SEM_POST TAMPOUCO É DE FLEIKYS
  pthread_exit(NULL);
}

//----------------------------------------MENU--------------------------------------
void *menu() {
  char mander[30];
  int prio_novos= 5;
  int numero_fillos_novos = 0;

  while(1) {
    printf("[menu] ENTER para continuar ou numero e prio de novos procesos\n");
    fgets(mander,30,stdin);
    fflush(stdin);
    if (mander[0] == '\n') {
      sem_post(&sem_paso_simu);
    } else {
      prio_novos =  atoi( &mander[2]);
      numero_fillos_novos = atoi(&mander[0]);
      num_fillos =+ numero_fillos_novos;
      pthread_t fioFillo[1000];
      printf("[menu] Creamos %i procesos con prio %i \n", numero_fillos_novos, prio_novos);
      //Creacion de fillos
      for(int i = 0; i < numero_fillos_novos; i++){
        pthread_create(&fioFillo[i],NULL, fillo, (void*) &prio_novos);
        sem_wait(&sem_paso_fillo);
      }
    }
  }
}
