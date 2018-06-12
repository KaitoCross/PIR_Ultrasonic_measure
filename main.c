#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#define RED 27
#define YELLOW 28
#define GREEN 3
#define READ_PIR 25
#define READ_USO 8
#define TRIGGER_USO 7

void p3_thread1()
{
struct timeval start, ende;
long sec, usec;
    while(1)
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
        {
            if (gettimeofday(&ende,(struct timezone*)0))
            {
                printf("error\n");
                exit(1);
            }
        }
        sec = ende.tv_sec - start.tv_sec;
        usec = ende.tv_usec - start.tv_usec;
    }
}

void p3_thread2()
{

}

void p3_thread3()
{

}

int main() {
    printf("Hello, World!\n");
    return 0;
}