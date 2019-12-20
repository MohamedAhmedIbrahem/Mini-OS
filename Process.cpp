#include <iostream>
#include <unistd.h>
#include <sys/wait.h> 
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <cstring>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

struct ProcessInformation
{
    float RequestTime;
    string operation;
    string data;
    
};
struct msgbuff
{
   long mtype;
   char mtext[65];
   int pid;
   char op;
};
void Read_File(string Input, vector<ProcessInformation> & MyProcesses)
{
    ifstream File;
    File.open(Input);
    if (!File)
        return;
    
    int count = 0;
    ProcessInformation temp;
    float x;
    while (File >> x) 
    {
        switch(count)
        {
            case(0):
                temp.RequestTime = x;
            break;

            case(1):
                temp.operation = x;
            break;

            case(2):
                temp.data = x;
            break;
        }
        count++;
        if(count == 3)
        {
            MyProcesses.push_back(temp);
            count = 0;
        }
    }
}
void Sort(std::vector<ProcessInformation> & Processes)
{
    ProcessInformation key;
    int i, j;  
    for (i = 1; i < Processes.size(); i++) 
    {  
        key = Processes[i];  
        j = i - 1;  
        while (j >= 0 && Processes[j].RequestTime > key.RequestTime) 
        {  
            Processes[j + 1] = Processes[j];  
            j = j - 1;  
        }  
        Processes[j + 1] = key;
    } 
}

void handler(int signum);
int CurrentTime;

int main(int argc , char* argv[])
{
    signal(SIGUSR2 , handler);

    int num = atoi(argv[1]);
    string Input = "process" + to_string(num) + ".txt";
    vector<ProcessInformation> MyProcesses;
    Read_File(Input, MyProcesses);
    Sort(MyProcesses);

    //key_t msgqid;
    //msgqid = msgget(IPC_PRIVATE, 0644);

    struct msgbuff message;

    while( MyProcesses.size() != 0)
    {
        if(MyProcesses[0].RequestTime <= CurrentTime)
        {

            message.mtype = 1;      
            strcpy(message.mtext, MyProcesses[0].data.c_str());
            message.op = MyProcesses[0].operation[0];
            message.pid = num ;
            int send_val = msgsnd(message.pid, &message, sizeof(message.mtext), !IPC_NOWAIT);
            if(send_val == -1)
                perror("Errror in send");
            //sleep(10);


        }
    }
    
    return 0; 
}

void handler(int signum)
{
    CurrentTime++;
}