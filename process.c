///////////////////////////////////////
/*

        this code is for testing purposes only 

*/
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include<signal.h>
int clk;
struct msgbuff
{
   long mtype;
   int pid;
   char op;
   char mtext[65];
};
void handler(int signum)
{
    clk++;
}
void do_parent(int pnum,key_t msgqid)
{
    for(int i=0;i<2;i++)
    {
        struct msgbuff msg;
        msg.mtype =1;
        msg.pid=pnum;
        msg.op='A';
        char str[]="alii khaled\n";
        strcpy(msg.mtext,str);
        int send_val = msgsnd(msgqid,&msg,sizeof(msg)-sizeof(long),IPC_NOWAIT);
        if(send_val==-1)
            perror("Error in send\n");
        sleep(10);
    }
    struct msgbuff msg;
    msg.mtype =1;
    msg.pid=pnum;
    msg.op='D';
    char str[]="0";
    strcpy(msg.mtext,str);
    int send_val = msgsnd(msgqid,&msg,sizeof(msg)-sizeof(long),IPC_NOWAIT);
    if(send_val==-1)
        perror("Error in send\n");
    sleep(10);
}

void main(int argc,char*argv[])
{   
    int pnum=atoi(argv[1]);
    key_t queue_id=atoi(argv[2]);
    printf("process %d %d \n",pnum,getgid());
    signal(SIGUSR2,handler);
    pause();
    do_parent(pnum,queue_id);
}