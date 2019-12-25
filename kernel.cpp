#include<iostream>
#include<bits/stdc++.h>
#include<signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <wait.h>
#include <string.h>
#include<fstream>
using namespace std;

int clk,latency,killed;
void handler_SIGUSER2(int signum)
{
    clk++;
    if(latency)
        latency--;
    alarm(1);
}
void handler_ALARM(int signum)
{
    killpg(getpgrp(),SIGUSR2);
}
void handler_SIGCHLD(int signum)
{
    int pid,status;
    while((pid = waitpid(-1,&status,WNOHANG))>0)
    {
        killed++;
    }
}
struct msgbuffPr
{
   long mtype;  //mtype = 1
   int pid; 
   char op; 
   char mtext[65];
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
    char mtext[65];
};
int main()
{
    key_t pr_Queue_Id=msgget(IPC_PRIVATE,0664);// Processes
    key_t disk_Queue_Up=msgget(IPC_PRIVATE,0664);// Disk Up
    key_t disk_Queue_Down=msgget(IPC_PRIVATE,0664);//Disk Down 
    if(pr_Queue_Id == -1 || disk_Queue_Down == -1 || disk_Queue_Up == -1)
    {
        perror("Error in Creating Message Queues \n");
        return 0;
    }
    pid_t Disk_pid=fork();
    if(Disk_pid==-1)
    {
        perror("Error in Forking\n");
    }
    if(Disk_pid==0) //child
    {
        char up_queue[20],down_queue[20];
        sprintf(up_queue,"%d",disk_Queue_Up);
        sprintf(down_queue,"%d",disk_Queue_Down);
        char*args[]={"./Disk",up_queue,down_queue,NULL};
        execv(args[0],args);
    }
    else            //parent
    {
        int n=2;
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
        // FILE *ptr = fopen("Logs.txt","w");
        ofstream out("Logs.txt");
        signal(SIGUSR2,handler_SIGUSER2);
        signal(SIGALRM,handler_ALARM);
        signal(SIGCHLD,handler_SIGCHLD);
        queue<msgbuffPr>q;
        alarm(1);
        while(1)    //kernel processing
        {  
            struct msgbuffPr process_msg;           //take message from processes
            int process_rec_val = msgrcv(pr_Queue_Id,&process_msg,sizeof(process_msg)-sizeof(long),0,IPC_NOWAIT);
            if(process_rec_val == -1 && killed == n && !q.size())
            {
                while(latency);
                kill(Disk_pid,SIGKILL);
                break;
            }
            if(process_rec_val != -1)
            {
                q.push(process_msg);
                out << clk <<"   Recieved a Message From Process " << process_msg.pid << " says "<<process_msg.op<<": "<<string(process_msg.mtext) << endl;
            }
            if(latency || !q.size())                //Disk is Busy or there are no messages to process
                continue;
            kill(Disk_pid,SIGUSR1);                 //ask for disk status
            struct msgbuffDiskUP Disk_status;       //recieve disk status
            int Disk_rec_status = msgrcv(disk_Queue_Up,&Disk_status,sizeof(Disk_status)-sizeof(long),0,!IPC_NOWAIT);
            out << endl;
            out << clk <<"   Disk's Number of Free Slots" << " is " << Disk_status.numberOfFreeSlots << endl;
            process_msg = q.front();
            q.pop();
            if(process_msg.op == 'A')
            {
                if(Disk_status.numberOfFreeSlots>0)
                {
                    struct msgbuffDiskDOWN msg;
                    msg.mtype=2;
                    msg.op='A';
                    strcpy(msg.mtext,process_msg.mtext);
                    msgsnd(disk_Queue_Down,&msg,sizeof(msg)-sizeof(long),IPC_NOWAIT);
                    latency=3;
                    out << clk <<"   Successful Add from process "<< process_msg.pid << " message : " << string(msg.mtext) << endl;
                }
                else
                    out << clk <<"   UnSuccessful Add from process " << process_msg.pid << endl;
            }
            if(process_msg.op == 'D')
            {
                int index = process_msg.mtext[0]-'0';
                if(index >= 0 && index<=9 && Disk_status.status[index])
                {
                    struct msgbuffDiskDOWN msg;
                    msg.mtype=2;
                    msg.op='D';
                    process_msg.mtext[1]='\0';
                    strcpy(msg.mtext,process_msg.mtext);
                    msgsnd(disk_Queue_Down,&msg,sizeof(msg)-sizeof(long),IPC_NOWAIT);
                    latency=1;
                    out << clk << "   Successful Delete from process " << process_msg.pid << " index : " << msg.mtext[0] << endl;
                }
                else
                    out << clk <<"   UnSuccessful Delete from process " << process_msg.pid << " index : " << string(process_msg.mtext) << endl;
            }
            out << endl;
        }
        out<< clk <<"   Kernel is Done " << endl;
        msgctl(pr_Queue_Id, IPC_RMID, (struct msqid_ds *) 0);
        msgctl(disk_Queue_Up, IPC_RMID, (struct msqid_ds *) 0);
        msgctl(disk_Queue_Down, IPC_RMID, (struct msqid_ds *) 0);
    }
}
