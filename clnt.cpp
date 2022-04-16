#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<iostream>
using namespace std;

bool sockaddr_v4(sockaddr *pskadr,uint16_t host_port,string ipv4)
{
    int i;

    sockaddr_in temp;
    in_addr iptemp;

    if(!inet_aton(ipv4.c_str(),&iptemp))
        return false;
    temp={
        AF_INET,
        htons(host_port),
        iptemp,
        {0,0,0,0,0,0,0,0},
    };
    
    *pskadr=*((sockaddr *)&temp);

    return true;
}

#define MSGLEN 60
#define BUFLEN 1024
int main()
{
    char buf[BUFLEN];
    sockaddr adr;
    int skfd;
    int i;

    sockaddr_v4(&adr,9090,"127.0.0.1");
    skfd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(skfd==-1)
    {
        cout<<"sk() err."<<endl;
        exit(-1);
    }
    if(connect(skfd,&adr,sizeof(adr))==-1)
    {
        cout<<"cnt() err."<<endl;
        exit(-1);
    }
    
    // for(i=0;i<MSGLEN-1;i++)
    //     msg[i]='a'+i%26;
    // msg[i]='\0';

    string str;

    cout<<"Enter sth to send: ";
    getline(cin,str);

    uint send_len;
    send_len=str.size();

    //send data_size
    write(skfd,(void*)&send_len,sizeof(send_len));
    cout<<"Sending "<<send_len<<" bytes."<<endl;
    //send real data
    write(skfd,str.c_str(),str.size());

    read(skfd,buf,sizeof(buf));
    cout<<buf<<endl;

    close(skfd);
    cout<<"Socket closed."<<endl;

    return 0;
}