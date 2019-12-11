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
struct msgbuff
{
   long mtype;
   int pid;
   char op;
   char mtext[70];
};
void handler(int signum)
{
    printf("hello aly\n");
}
void do_parent(int pnum,key_t msgqid)
{
    printf("msgqid = %d from process %d \n",msgqid,pnum);
    int send_val;
    char str[2];
    sprintf(str,"%d",pnum);
    struct msgbuff msg;
    msg.mtype =1;
    strcpy(msg.mtext,str);
    int id = msgget(msgqid,IPC_CREAT|IPC_EXCL);
    printf("process %d %d \n",pnum,id);
    send_val = msgsnd(msgqid,&msg,sizeof(msg)-sizeof(long),!IPC_NOWAIT);
    if(send_val==-1)
        perror("Error in send\n");
    
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