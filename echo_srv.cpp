//sk class

#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>

#include<sys/wait.h>
#include<signal.h>

#include<iostream>
#include<sstream>
using namespace std;

#define MIN(X,Y) ((X)>(Y))?(Y):(X)

#define IPLEN 25
#define BUFLEN 100

bool sockaddr_v4(sockaddr_in *pskadr,uint16_t host_port,string ipv4);
bool sockaddr_v4(sockaddr *pskadr,uint16_t host_port,string ipv4);
void ErrHdl(int sock,string errmsg,ostream &err=cout);
void ErrHdl(string errmsg,ostream &err=cout);

string GetSockInfo(sockaddr_in &addr);

void InitSigProc();
void read_childproc(int sig);

int CycleRecv(int sock,size_t data_len,void *desti,size_t desti_max_size,void (*err_deal)(int sock,string errmsg,ostream &err));
int CustomSend(int sock,const void *buf,uint send_size,void (*err_deal)(int sock,string errmsg,ostream &err));
void CleanSockBuf(int sock,void (*err_deal)(int sock,string errmsg,ostream &err),int send_size=1024);

void EnterId(char *des,size_t max_size,istream &is=cin);

int main()
{
    char buf[BUFLEN];
    int ssk,csk;
    int recv_len,sgl_len,ngr_len;
    int skopt;

    socklen_t c_adr_len,optlen;
    sockaddr_in s_adr,c_adr;
    uint16_t clnt_port;
    uint data_len;
    string clntname;
    string context;

    pid_t pid;

    InitSigProc();

    sockaddr_v4(&s_adr,9090,"127.0.0.1");
    
    // cout<<"Host: "<<inet_ntoa(((sockaddr_in *)&s_adr)->sin_addr)<<':'<<ntohs(((sockaddr_in *)&s_adr)->sin_port)<<endl;
    cout<<"Host: "<<GetSockInfo(s_adr)<<endl;

    ssk=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(ssk==-1)
        ErrHdl("socket() err.");
    cout<<"Socket created."<<endl;

    //set reuseaddr
    optlen=sizeof(skopt);
    skopt=true;
    setsockopt(ssk,SOL_SOCKET,SO_REUSEADDR,(void*)&skopt,optlen);

    if(bind(ssk,(sockaddr *)&s_adr,sizeof(s_adr))==-1)
        ErrHdl("bind() err.");
    cout<<"Socket binded."<<endl;

    if(listen(ssk,5)==-1)
        ErrHdl("listen() err.");
    cout<<"Socket listened."<<endl;

    c_adr_len=sizeof(c_adr);    
    cout<<"Waiting for connecting..."<<endl;
    //waiting
    while(1)
    {   
        //waiting for connecting.
        csk=accept(ssk,(sockaddr *)&c_adr,&c_adr_len);
        if(csk==-1)
            continue;
        else
        {
            clntname=GetSockInfo(c_adr);
            cout<<clntname<<" connected."<<endl;
        }
        pid=fork();
        if(pid==-1)
        {
            close(csk);
            continue;
        }
        if(pid==0)
        {
            close(ssk); //parent use ssk.
            while(1)
            {
                //get head data
                CycleRecv(csk,sizeof(uint),(void*)&data_len,sizeof(uint),ErrHdl);
                if(data_len==0)
                {
                    cout<<clntname<<" disconnected."<<endl;
                    close(csk);
                    exit(0);
                }

                //get real data
                ngr_len=CycleRecv(csk,data_len,buf,BUFLEN-1,ErrHdl);
                buf[data_len-ngr_len]='\0';
                CleanSockBuf(csk,ErrHdl);

                context=buf;

                cout<<'['<<clntname<<"] "<<context<<endl;
                //send echo
                CustomSend(csk,context.c_str(),context.size(),ErrHdl);

            }
        }
        else
        {
            close(csk); //child ues ssk.
        }
    }

    close(ssk);
    cout<<"srv_socket closed."<<endl;

    return 0;
}

void ErrHdl(string errmsg,ostream &err)
{
    err<<errmsg<<endl;
    exit(1);
}
void ErrHdl(int sock,string errmsg,ostream &err)
{
    close(sock);
    err<<errmsg<<endl;
    exit(1);
}

bool sockaddr_v4(sockaddr *pskadr,uint16_t host_port,string ipv4)
{
    int i;
    // char *pd,*ps;
    
    // sockaddr skadr;
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
    
    // pd=(char *)pskadr;
    // ps=(char *)&temp;
    // while(ps<(char *)&temp+sizeof(temp))
    //     *pd++=*ps++;
    *pskadr=*((sockaddr *)&temp);
    return true;
}
bool sockaddr_v4(sockaddr_in *pskadr,uint16_t host_port,string ipv4)
{
    int i;
    // char *pd,*ps;
    
    // sockaddr skadr;
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
    
    // pd=(char *)pskadr;
    // ps=(char *)&temp;
    // while(ps<(char *)&temp+sizeof(temp))
    //     *pd++=*ps++;
    *pskadr=temp;
    return true;
}

string GetSockInfo(sockaddr_in &addr)
{
    string clntname;
    stringstream ss;
    ss<<inet_ntoa(addr.sin_addr);
    ss<<':'<<ntohs(addr.sin_port);
    ss>>clntname;
    ss.clear();

    return clntname;
}

void read_childproc(int sig)
{
    int status;
    waitpid(-1,&status,WNOHANG);
}

void InitSigProc()
{
    struct sigaction act;

    act.sa_flags=0;
    sigemptyset(&act.sa_mask);
    act.sa_handler=read_childproc;

    sigaction(SIGCHLD,&act,0);
    sigaction(SIGINT,&act,0);
}

int CycleRecv(int sock,size_t data_len,void *desti,size_t desti_max_size,void (*err_deal)(int sock,string errmsg,ostream &err))
{
    int sgl_len,recv_len,ngr_len;

    if(desti_max_size<data_len)
        ngr_len=data_len-desti_max_size;
    else
        ngr_len=0;
    data_len-=ngr_len;

    // cout<<data_len<<endl;
    recv_len=0;
    sgl_len=0;
    while(recv_len<data_len)
    {
        sgl_len=read(sock,(char*)desti+recv_len,data_len);
        if(sgl_len==-1)
        {
            err_deal(sock,"recv head_info err.",cout);
        }
        recv_len+=sgl_len;
    }

    return ngr_len;
}

int CustomSend(int sock,const void *buf,uint send_size,void (*err_deal)(int sock,string errmsg,ostream &err))
{
    if(write(sock,(void*)&send_size,sizeof(uint))==-1)
        err_deal(sock,"write() err.",cout);
    if(write(sock,buf,send_size)==-1)
        err_deal(sock,"write() err.",cout);

    return send_size;
}

void CleanSockBuf(int sock,void (*err_deal)(int sock,string errmsg,ostream &err),int buf_size)
{
    char *buf=new char[buf_size];
    while(recv(sock,buf,buf_size,MSG_DONTWAIT)>0)
        continue;
    delete[] buf;
}
