#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include<signal.h>
int clk,latency;
void handler_SIGUSER2(int signum)
{
    clk++;
    latency--;
    latency=max(latency,0);
    alarm(1);
}
void handler_ALARM(int signum)
{
    printf("Alarm handler\n");
    killpg(getpgrp(),SIGUSR2);
}
struct msgbuffPr
{
   long mtype;  //mtype = 1
   int pid; 
   char op; 
   char mtext[64];
};

struct msgbuffDiskUP
{
    long mtype;  // 0 -> dist status 
    int numberOfFreeSlots;
    int status[10];
};

struct msgbuffDiskDOWN
{ 
    long mtype;     //2 -> disk operation
    char op; 
    char mtext[64];
};
void main()
{
    key_t pr_Queue_Id=msgget(IPC_PRIVATE,0664);// Processes
    key_t disk_Queue_Up=msgget(IPC_PRIVATE,0664);// Disk Up
    key_t disk_Queue_Down=msgget(IPC_PRIVATE,0664);//Disk Down 
    pid_t Disk_pid=fork();
    if(Disk_pid==-1)
    {
        perror("Error in Forking\n");
    }
    if(Disk_pid==0) //child
    {
        char up_queue[10],down_queue[10];
        sprintf(up_queue,"%d",disk_Queue_Up);
        sprintf(down_queue,"%d",disk_Queue_Down);
        char*args[]={"./Disk",up_queue,down_queue,NULL};
        execv(args[0],args);
    }
    else            //parent
    {
        int n=3;
        for(int i=0;i<n;i++)
        {
            pid_t child = fork();
            if(child==-1)
            {
                perror("Error in forking child\n");
            }
            if(child==0)
            {
                char snum[2],queue_id[10];
                sprintf(snum,"%d",i);
                sprintf(queue_id,"%d",pr_Queue_Id);
                char*args[]={"./process",snum,queue_id,NULL};
                execv(args[0],args);
            }
        }
        FILE *ptr=fopen("Logs.txt","w");
        signal(SIGUSR2,handler_SIGUSER2);
        signal(SIGALRM,handler_ALARM);
        alarm(1);
        printf("kernel %d \n",getpgrp());
        while(1)    //kernel processing
        {  
            while(latency);                         //wait until the disk completes its job
            struct msgbuffPr process_msg;           //take message from processes
            int process_rec_val = msgrcv(pr_Queue_Id,&process_msg,sizeof(process_msg)-sizeof(long),0,IPC_NOWAIT);
            if(process_rec_val == -1)
                continue;
            kill(SIGUSR1,Disk_pid);                 //ask for disk status
            struct msgbuffDiskUP Disk_status;       //recieve disk status
            int Disk_rec_status = msgrcv(disk_Queue_Up,&Disk_status,sizeof(Disk_status)-sizeof(long),0,!IPC_NOWAIT);
            if(process_msg.op == 'A')
            {
                if(Disk_status.numberOfFreeSlots>0)
                {
                    struct msgbuffDiskDOWN msg;
                    msg.mtype=2;
                    msg.op='A';
                    strcpy(msg.mtext,process_msg.mtext);
                    msgsnd(disk_Queue_Down,&msg,sizeof(msg)-sizeof(long),IPC_NOWAIT);
                    fprintf(ptr,"Successful Add from process %d \n",process_msg.pid);
                    latency=3;
                }
                else
                    fprintf(ptr,"UnSuccessful Add from process %d \n",process_msg.pid);
            }
            if(process_msg.op == 'D')
            {
                if(Disk_status.status[process_msg.mtext[0]-'0'])
                {
                    struct msgbuffDiskDOWN msg;
                    msg.mtype=2;
                    msg.op='D';
                    strcpy(msg.mtext,process_msg.mtext[0]);
                    msgsnd(disk_Queue_Down,&msg,sizeof(msg)-sizeof(long),IPC_NOWAIT);
                    fprintf(ptr,"Successful Delete from process %d \n",process_msg.pid);
                    latency=1;
                }
                else
                    fprintf(ptr,"UnSuccessful Delete from process %d \n",process_msg.pid);
            }
        }
    }
    
}