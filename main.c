#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <softTone.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include <zconf.h>
#include <signal.h>

#define RED 27
#define YELLOW 28
#define GREEN 3
#define READ_PIR 25
#define READ_USO 7
#define TRIGGER_USO 8
#define SNDOUT 24
#define WLOCK -2
#define WUNLOCK 2
#define KEY 1336
#define LOCK       -1
#define UNLOCK      1
#define PERM 0666      /* Zugriffsrechte */

static struct sembuf semaphore;
static int semid;
static int semid_2;//Zwischen P2 (writer) und reader P3 & P4
static int semid_3;

static int init_semaphore (int *sema_id, short initval, int semkey) {
    /* Testen, ob das Semaphor bereits existiert */
    *sema_id = semget (semkey, 0, IPC_PRIVATE);
    if (*sema_id < 0) {
        /* ... existiert noch nicht, also anlegen        */
        /* Alle Zugriffsrechte der Dateikreierungsmaske */
        /* erlauben                                     */
        umask(0);
        *sema_id = semget (semkey, 1, IPC_CREAT | IPC_EXCL | PERM);
        if (*sema_id < 0) {
            printf ("Fehler beim Anlegen des Semaphors ...\n");
            return -1;
        }
        printf ("(angelegt) Semaphor-ID : %d\n", *sema_id);
        /* Semaphor mit initval initialisieren */
        if (semctl (*sema_id, 0, SETVAL, initval) == -1)
            return -1;
    }
    return 1;
}
static int semaphore_operation (int sem_id, short op) {
    semaphore.sem_op = op;
    //semaphore.sem_flg = SEM_UNDO;
    if( semop (sem_id, &semaphore, 1) == -1) {
        perror(" semop ");
        exit (EXIT_FAILURE);
    }
    return 1;
}


struct argsForpthread
{
    long double distance;
    short evaluationDone;
    short detectedMove;
    short alive;
};

long double measureDistance(short echopin, short triggerpin)
{
    struct timeval start, ende;
    long sec, usec;

    digitalWrite(triggerpin,1);
    delayMicroseconds(10);
    digitalWrite(triggerpin,0);
    if (gettimeofday(&start,(struct timezone*)0))
    {
        printf("error\n");
        exit(1);
    }
    printf("%d\n",digitalRead(echopin));
    while (digitalRead(echopin) == 0)
    {
        if (gettimeofday(&start,(struct timezone*)0))
        {
            printf("error\n");
            exit(1);
        }
    }
    while (digitalRead(echopin)==HIGH)
    {
        if (gettimeofday(&ende,(struct timezone*)0))
        {
            printf("error\n");
            exit(1);
        }
    }
//}
//	printf("START uSEC %ld\n",start.tv_usec);
//	printf("END uSEC %ld\n",ende.tv_usec);
    sec = ende.tv_sec - start.tv_sec;
    usec = ende.tv_usec - start.tv_usec;
    printf("DIFF in uSEC: %ld\n", usec);
    long double totaldiff = (double)sec + (double)usec/1000000.0;
    long double tdistance = /*100*((usec/1000000.0)*340.29)/2;*/ ((totaldiff*34300.0)/2);
    printf("DISTANCE MEASURED! Distance: %lf\n",tdistance);
    return tdistance;
}

void p3_thread1(struct argsForpthread *demArgs)
{
    while(demArgs->alive)
    {
        semaphore_operation(semid_3,WLOCK);
        semaphore_operation(semid,WLOCK);
        printf("THREAD1 LOCK\n");
        while (digitalRead(READ_PIR) != 1) {
            demArgs->detectedMove=0;
	delay(10);
        }
        demArgs->detectedMove=1;
        semaphore_operation(semid,WUNLOCK);
        semaphore_operation(semid_3,WUNLOCK);
        printf("MOVEMENT DETECTED\n");
        delay(1);
    }
}

void p3_thread2(struct argsForpthread * demArgs)
{
    struct timeval start, ende;
    long sec, usec;
    while(demArgs->alive)
    {
        semaphore_operation(semid_2,WLOCK);
        printf("THREAD2 S2 LOCK\n");
        semaphore_operation(semid,WLOCK);
        printf("THREAD2 S0 LOCK\n");
        /*digitalWrite(TRIGGER_USO,1);
        delay(10);
        digitalWrite(TRIGGER_USO,0);
        if (gettimeofday(&start,(struct timezone*)0))
        {
            printf("error\n");
            exit(1);
        }
	printf("PIN %d status: %d\n",READ_USO, digitalRead(READ_USO));
        while (digitalRead(READ_USO)==LOW)
        {
            if (gettimeofday(&start,(struct timezone*)0))
            {
                printf("error\n");
                exit(1);
            }
        }TRIGGER_USO
        while (digitalRead(READ_USO)==HIGH)
        {
            if (gettimeofday(&ende,(struct timezone*)0))
            {
                printf("error\n");
                exit(1);
            }
        }
        sec = ende.tv_sec - start.tv_sec;
        usec = ende.tv_usec - start.tv_usec;
       	long double totaldiff = (double)sec + (double)usec/1000000.0;
        demArgs->distance = ((totaldiff*34300.0)/2.0);*/
        demArgs->distance = measureDistance(READ_USO,TRIGGER_USO);
        semaphore_operation(semid,WUNLOCK);
        semaphore_operation(semid_2,WUNLOCK);
        printf("DISTANCE MEASURED! SemID2 : %d\n",semid_2);
	delay(100);
    }
}

void p3_thread3(struct argsForpthread *demArgs)
{
    while(demArgs->alive) {
        semaphore_operation(semid_3,LOCK);
        printf("THREAD3 S3 LOCKED\n");
        semaphore_operation(semid_2,LOCK);
        printf("THREAD3 S2 LOCKED\n");
        digitalWrite(GREEN, 0);
        digitalWrite(YELLOW, 0);
        digitalWrite(RED, 0);
        if(demArgs->distance<10)
        {
            digitalWrite(RED,1);
        }
        else if (demArgs->distance >= 10 && demArgs->distance <= 20)
        {
            digitalWrite(YELLOW,1);
        }
        else if (demArgs->distance > 20)
        {
            digitalWrite(GREEN,1);
        }
        semaphore_operation(semid_2,UNLOCK);
        semaphore_operation(semid_3,UNLOCK);
        printf("UNLOCKED, DID SET LEDS, SEMID %d\n",semid);
    }
}

void p3_thread4(struct argsForpthread *demArgs)
{
    while(demArgs->alive)
    {
        semaphore_operation(semid_3,LOCK);
        printf("THREAD4 S3 LOCKED\n");
        semaphore_operation(semid_2,LOCK);
        printf("THREAD4 S2 LOCKED\n");
        if(demArgs->distance < 10.0)
        {
            softToneWrite(SNDOUT,100);
            delay(300);
        }
        semaphore_operation(semid_2,UNLOCK);
        semaphore_operation(semid_3,UNLOCK);
        printf("Distance: %lf\nSEMID %d\n",demArgs->distance,semid);
    }
}

pthread_t readPIR, calcDist, doLED, doSound;

void killsems(int sig)
{
    semctl (semid, 0, IPC_RMID, 0);
    semctl (semid_2, 0, IPC_RMID, 0);
    semctl (semid_3, 0, IPC_RMID, 0);
    printf("Semaphores deleted");
    softToneWrite(SNDOUT,0);
}


int main() {
    signal(SIGINT,killsems);
    struct argsForpthread demArgs;
    demArgs.distance=0.0;
    demArgs.detectedMove=0;
    demArgs.evaluationDone=0;
    demArgs.alive=0;
    int res;
    res = init_semaphore(&semid,2,KEY);
    if (res < 0) {
        printf("ERROR CREATING SEMAPHORE");
        return EXIT_FAILURE;
    }
    res = init_semaphore(&semid_2,2,KEY+1);
    if (res < 0) {
        printf("ERROR CREATING SEMAPHORE");
        return EXIT_FAILURE;
    }
    res = init_semaphore(&semid_3,2,KEY+2);
    if (res < 0) {
        printf("ERROR CREATING SEMAPHORE");
        return EXIT_FAILURE;
    }
    wiringPiSetup();
    pinMode(READ_PIR,INPUT);
    pinMode(READ_USO,INPUT);
    pinMode(RED,OUTPUT);
    pinMode(YELLOW,OUTPUT);
    pinMode(GREEN,OUTPUT);
    pinMode(TRIGGER_USO,OUTPUT);
    pinMode(SNDOUT,SOFT_TONE_OUTPUT);
 /*   struct timeval _start, _ende;
    long _sec, _usec;
        digitalWrite(TRIGGER_USO,HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIGGER_USO,LOW);
        if (gettimeofday(&_start,(struct timezone*)0))
        {
            printf("error\n");
            exit(1);
        }
	printf("%d\n start usec %ld\n",digitalRead(READ_USO),_start.tv_usec);
        while (digitalRead(READ_USO) == LOW){
            if (gettimeofday(&_start,(struct timezone*)0))
            {
                printf("error\n");
                exit(1);
            }
        }
	while (digitalRead(READ_USO) == HIGH){
            if (gettimeofday(&_ende,(struct timezone*)0))
            {
                printf("error\n");
                exit(1);
            }
	}
	printf("END uSec %d\n",_ende.tv_usec);
        _sec = _ende.tv_sec - _start.tv_sec;
        _usec = _ende.tv_usec - _start.tv_usec; //_ende.tv_usec - _start.tv_usec;
        long double _totaldiff = ((long double)_usec)/1000000.0;
        //demArgs.distance = 100*((_usec/1000000.0)*340.29)/2;
        demArgs.distance = (_totaldiff*34300)/2;
	printf("Distance test %lf\n",demArgs.distance);
*/
    measureDistance(READ_USO,TRIGGER_USO);
	demArgs.alive=1;
    if (pthread_create(&readPIR,NULL,&p3_thread1,&demArgs)!=0)
    {
        printf("ERROR CREATING THREAD");
        return EXIT_FAILURE;
    }
    if (pthread_create(&calcDist,NULL,&p3_thread2,&demArgs)!=0)
    {
        printf("ERROR CREATING THREAD");
        return EXIT_FAILURE;
    }
    if (pthread_create(&doLED,NULL,&p3_thread3,&demArgs)!=0)
    {
        printf("ERROR CREATING THREAD");
        return EXIT_FAILURE;
    }
    if (pthread_create(&doSound,NULL,&p3_thread4,&demArgs)!=0)
    {
        printf("ERROR CREATING THREAD");
        return EXIT_FAILURE;
    }

    sleep(15);
    demArgs.alive=0;
    killsems(0);
    pthread_join(readPIR,NULL);
    pthread_join(calcDist,NULL);
    pthread_join(doSound,NULL);
    pthread_join(doLED,NULL);
//    struct timeval start, ende;
//    long sec, usec;
//       semaphore_operation(semid_2,WLOCK);
//       printf("THREAD2 S2 LOCK\n");
//       semaphore_operation(semid,WLOCK);
//        printf("THREAD2 S0 LOCK\n");
/*	if (demArgs.alive ==0)
{
        digitalWrite(TRIGGER_USO,1);
        delay(10);
        digitalWrite(TRIGGER_USO,0);
        if (gettimeofday(&start,(struct timezone*)0))
        {
            printf("error\n");
            exit(1);
        }
        while (digitalRead(READ_USO)==0)
        {
            if (gettimeofday(&start,(struct timezone*)0))
            {
                printf("error\n");
                exit(1);
            }
        }
        while (digitalRead(READ_USO)==1)
        {            if (gettimeofday(&ende,(struct timezone*)0))
            {
                printf("error\n");
                exit(1);
            }
        }
        sec = ende.tv_sec - start.tv_sec;
        usec = ende.tv_usec - start.tv_usec;
        double totaldiff = (double)sec + (double)usec/1000.0;
        demArgs.distance = ((totaldiff*34300)/2.0);
	printf("Distance last %d",demArgs.distance);
}*/
    return 0;
}
