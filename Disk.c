#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
struct slot
{
    char value[65];
    int status;
};

struct msgbuffDiskUP
{
    long mtype;  // 3 -> dist status 
    int numberOfFreeSlots;
    int status[10];
};

struct msgbuffDiskDOWN
{ 
    long mtype;     //2 -> disk operation
    char op; 
    char mtext[65];
};

int clk;
key_t msgUp;
key_t msgDown;
int freeSlots;
struct slot slots[10];
int rcv, snd;

// Handler for SIGUSR1.
void handler1 (int signum){
   // Building msgbuffer to be send to Kernel.
   struct msgbuffDiskUP msgbufSnd;
   msgbufSnd.mtype = 3;
   msgbufSnd.numberOfFreeSlots = freeSlots;
   for (int i=0; i<10; i++){
      msgbufSnd.status[i] = slots[i].status;
   }
   // Sending Message to Kernel.
   snd = msgsnd(msgUp, &msgbufSnd, sizeof(msgbufSnd)-sizeof(long), !IPC_NOWAIT);
   // If it failed to send.
   if (snd == -1){
      printf("Error in sending Message to Kernel.");
   }
}
void handler2(int signum){
   clk++;
}

// Where everything starts.
void main(int argc, char *argv[])
{
   signal(SIGUSR1, handler1);
   signal(SIGUSR2, handler2);
   msgUp = atoi(argv[1]);
   msgDown = atoi(argv[2]); 
   for(int i=0;i<10;i++)
        slots[i].status=0;
   freeSlots = 10;
   while(1){
      struct msgbuffDiskDOWN msgbufRec;
      rcv = msgrcv(msgDown,&msgbufRec,sizeof(msgbufRec)-sizeof(long), 0, !IPC_NOWAIT);
      if (rcv != -1){

         // If it is Add operation.
         if (msgbufRec.op == 'A'){
            for (int i=0; i<10; i++){
               if (slots[i].status == 0){
                  strcpy(slots[i].value, msgbufRec.mtext);
                  freeSlots--;
                  slots[i].status=1;
                  break;
               }
            }
         }

         // If it is Delete operation.
         else{
            int delId = msgbufRec.mtext[0] - '0';
            slots[delId].status = 0;
            freeSlots++;
         }

      }
   }
}
