#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef u32
#define u32 unsigned long int
#endif

//#define DEBUG 1
#define MAXINTS 10
#define KEY 12345

#ifndef MAXLEN
#define MAXLEN 1024
#endif

typedef struct {
	int totalint;
	char name[MAXLEN];
	u32 lastsec;  		/* 最后一次的秒 */
	u32 lastmin;
	u32 lastt12min;
	u32 lastt2hour;
	u32 lastrp;
	u32 lastrb;
	u32 lasttp;
	u32 lasttb;
	u32 secdatarp[180];	/* 每秒一个数据，共３分钟 */
	u32 secdatarb[180];	/* 每秒一个数据，共３分钟 */
	u32 secdatatp[180];	/* 每秒一个数据，共３分钟 */
	u32 secdatatb[180];	/* 每秒一个数据，共３分钟 */

	u32 mindatarp[180];	/* 每分一个数据，共３小时 */
	u32 mindatarb[180];	/* 每分一个数据，共３小时 */
	u32 mindatatp[180];	/* 每分一个数据，共３小时 */
	u32 mindatatb[180];	/* 每分一个数据，共３小时 */

	u32 t12mindatarp[180];	/* 每12分一个数据，共1.5天*/
	u32 t12mindatarb[180];	/* 每12分一个数据，共1.5天*/
	u32 t12mindatatp[180];	/* 每12分一个数据，共1.5天*/
	u32 t12mindatatb[180];	/* 每12分一个数据，共1.5天*/

	u32 t2hourdatarp[180];	/* 每2小时一个数据，共15天 */
	u32 t2hourdatarb[180];	/* 每2小时一个数据，共15天 */
	u32 t2hourdatatp[180];	/* 每2小时一个数据，共15天 */
	u32 t2hourdatatb[180];	/* 每2小时一个数据，共15天 */
} RRD;

int semid=0;
struct sembuf psembuf={-1,SEM_UNDO},vsembuf={1,SEM_UNDO};

int p_oper()
{ return semop(semid,&psembuf,1);
}

int v_oper(semid)
{ return semop(semid,&vsembuf,1);
}

int pv_init(void){
	int initval[1];
	semid=semget(KEY,1,0777|IPC_CREAT);
	if(semid<0) {perror("semget"); return -1;}
	initval[0]=1;
	semctl(semid,1,SETALL,initval);
	return 0;
}

int shmid=0;
void *shm=NULL;

int getshm(int create){
	if(create) 
		shmid=shmget(KEY,(sizeof(RRD))*MAXINTS,0777|IPC_CREAT);
	else 	shmid=shmget(KEY,(sizeof(RRD))*MAXINTS,0777);
	if(shmid<0) {perror("shmget"); return -1;};
	shm=shmat(shmid,0,0);
	if(shm<0) {perror("shmat"); return -1;}
	if(create) memset(shm,0,sizeof(RRD)*MAXINTS);
	return 0;
}


