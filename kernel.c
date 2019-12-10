#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
int clk;
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
        while(1)    //kernel processing
        {
            
        }
    }
    
}