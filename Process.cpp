#include <iostream>
#include <bits/stdc++.h>
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
struct msgbuff
{
   long mtype;  //mtype = 1
   int pid; 
   char op; 
   char mtext[65];
};
struct ProcessInformation
{
    int RequestTime;
    string operation;
    string data;
    bool operator < (const ProcessInformation & a)const
    {
        if(RequestTime == a.RequestTime)
            return true;
        return RequestTime < a.RequestTime;
    }
};
void Read_File(string Input, vector<ProcessInformation> & MyProcesses)
{
    ifstream File;
    File.open(Input);
    if (!File)
        return;
    ProcessInformation temp;
    string x;
    while (getline(File,x))
    {
        int split = x.find(' ');
        temp.RequestTime = stoi(x.substr(0,split));
        x = x.substr(split+1);
        split = x.find(' ');
        temp.operation = x.substr(0,split);
        temp.data = x.substr(split+1);
        MyProcesses.push_back(temp);
    }
    File.close();
}

void handler(int signum);
int CurrentTime;

int main(int argc , char* argv[])
{
    signal(SIGUSR2 , handler);
    int num = atoi(argv[1]);
    string Input = "test/process" + to_string(num) + ".txt";
    vector<ProcessInformation> MyProcesses;
    Read_File(Input, MyProcesses);
    sort(MyProcesses.begin(),MyProcesses.end());
    key_t Qid = stoi(argv[2]);

    struct msgbuff message;
    
    while( MyProcesses.size() != 0)
    {
        if(MyProcesses[0].RequestTime == CurrentTime)
        {
            message.mtype = 1;      
            strcpy(message.mtext, MyProcesses[0].data.c_str());
            message.op = MyProcesses[0].operation[0];
            message.pid = num ;
            int send_val = msgsnd(Qid, &message, sizeof(message)-sizeof(long), IPC_NOWAIT);
            if(send_val == -1)
                perror("Errror in send");
            MyProcesses.erase(MyProcesses.begin());
        }
    }
    return 0; 
}

void handler(int signum)
{
    CurrentTime++;
}
