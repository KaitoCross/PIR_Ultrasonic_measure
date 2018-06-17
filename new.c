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


int main(void)
{

    wiringPiSetup();
    pinMode(READ_PIR,INPUT);
    pinMode(READ_USO,INPUT);
    pinMode(RED,OUTPUT);
    pinMode(YELLOW,OUTPUT);
    pinMode(GREEN,OUTPUT);
    pinMode(TRIGGER_USO,OUTPUT);
    pinMode(SNDOUT,SOFT_TONE_OUTPUT);
	struct timeval start, ende;
	long sec, usec;

        digitalWrite(TRIGGER_USO,1);
        delayMicroseconds(10);
        digitalWrite(TRIGGER_USO,0);
        if (gettimeofday(&start,(struct timezone*)0))
        {
            printf("error\n");
            exit(1);
        }
	printf("%d\n",digitalRead(READ_USO));
//	printf("START usec %ld\n", start.tv_usec); //SEHR WICHTIG!
//	if (digitalRead(READ_USO)==LOW)
//{
//	short readUso = digitalRead(READ_USO);
        while (digitalRead(READ_USO) == 0)
        {
            if (gettimeofday(&start,(struct timezone*)0))
            {
                printf("error\n");
                exit(1);
            }
//	readUso = digitalRead(READ_USO);
//	printf("LOWAMK\n");
        }
        while (digitalRead(READ_USO)==HIGH)
        {
            if (gettimeofday(&ende,(struct timezone*)0))
            {
                printf("error\n");
                exit(1);
            }
//	printf("I HAVE THE HIGH GROUND\n");
        }
//}
//	printf("START uSEC %ld\n",start.tv_usec);
//	printf("END uSEC %ld\n",ende.tv_usec);
        sec = ende.tv_sec - start.tv_sec;
        usec = ende.tv_usec - start.tv_usec;
	printf("DIFF in uSEC: %ld\n", usec);
        long double totaldiff = (double)sec + (double)usec/1000000.0;
        long double tdistance = /*100*((usec/1000000.0)*340.29)/2;*/ ((totaldiff*34300.0)/2);
//        semaphore_operation(semid,WUNLOCK);
//        semaphore_operation(semid_2,WUNLOCK);
        printf("DISTANCE MEASURED! Distance: %lf\n",tdistance);
return 1;
}
