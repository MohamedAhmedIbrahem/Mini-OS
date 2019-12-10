#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include<signal.h>
int clk;
void handler_SIGUSER2(int signum)
{
    clk++;
    alarm(1);
}
void handler_ALARM(int signum)
{
    killpg(getgid(),SIGUSR2);
}
struct msgbuff
{
   long mtype;
   char mtext[70];
};

void main()
{
    pid_t Disk_pid=fork();
    if(Disk_pid==-1)
    {
        perror("Error in Forking\n");
    }
    if(Disk_pid==0) //child
    {
        char*args[]={"./Disk.c",NULL};
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
                char*args[]={"./process.c",i};
                execv(args[0],args);
            }
        }
        FILE *ptr=fopen("Logs.txt","w");
        signal(SIGUSR2,handler_SIGUSER2);
        signal(SIGALRM,handler_ALARM);
        /*
        this part is for message queues
        */
        alarm(1);
        while(1)    //kernel processing
        {  
            
        }
    }
    
}