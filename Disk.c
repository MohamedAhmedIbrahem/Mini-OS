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

int clk = 0;
key_t msgUp;
key_t msgDown;
int freeSlots;
struct slot slots[10];
int rcv, snd;
struct msgbuff msgbufSnd;

struct msgbuff
{
   long mtype;
   int freSlots;
   int slots[10];
   char mtext[65];
   char op;
};

// Handler for SIGUSR1.
void handler1 (int sig_num){

   // Building msgbuffer to be send to Kernel.
   msgbufSnd.freSlots = freeSlots;
   msgbufSnd.mtype = 0;

   for (int i=0; i<10; i++){
      msgbufSnd.slots[i] = slots[i].status;
   }
   
   // Sending Message to Kernel.
   snd = msgsnd(msgUp, &msgbufSnd, sizeof(msgbufSnd) - sizeof (long), !IPC_NOWAIT);

   // If it failed to send.
   if (snd != 0){
      printf("Error in sending Message to Kernel.");
   }
   return;
}
void handler2(int sig_num){
   clk = clk + 1;
   return;
}

// Where everything starts.
void main(int argc, char **argv)
{
   msgUp = atoi(argv[1]);
   msgDown = atoi(argv[2]); 

   for(int i=0; i<10; i++){
      slots[i].status = 0;
   }

   freeSlots = 10;
   signal(SIGUSR1, handler1);
   signal(SIGUSR2, handler2);

   int busy = 0;
   struct msgbuff* msgbufRec;
   int delId;

   while(1){
      rcv = msgrcv(msgDown,msgbufRec,sizeof(msgbufRec)-sizeof(long), 2, IPC_NOWAIT);
      if (rcv == 0){

         // If it is Add operation.
         if (msgbufRec->op == 'A'){
            for (int i=0; i<10; i++){
               if (slots[i].status == 0){
                  strcpy(slots[i].value, msgbufRec->mtext);
                  freeSlots = freeSlots - 1;
               }
            }
         }

         // If it is Delete operation.
         else{
            delId = msgbufRec->mtext[0] - '0';
            if ( delId >= 0 && delId < 10){
               slots[delId].status = 0;
               freeSlots = freeSlots + 1;
            }
         }
      }
   }
}
