#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
struct msgbuff
{
   long mtype;
   char mtext[70];
};

void main()
{
    
    
}