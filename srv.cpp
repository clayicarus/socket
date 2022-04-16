#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdio.h>

#include<iostream>
#include<sstream>
using namespace std;

#define MIN(X,Y) ((X)>(Y))?(Y):(X)
void ErrHdl(const char * errmsg)
{
    cout<<errmsg<<endl;
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

#define IPLEN 25
#define BUFLEN 100
int main()
{
    char buf[BUFLEN];
    char clnt_ip[IPLEN];
    int ssk,csk;
    int recv_len,sgl_len,ngr_len;
    int skopt;

    socklen_t c_adr_len,optlen;
    sockaddr_in s_adr,c_adr;
    uint16_t clnt_port;
    uint data_len;
    string clntname;

    sockaddr_v4(&s_adr,9090,"127.0.0.1");
    
    cout<<"Host: "<<inet_ntoa(((sockaddr_in *)&s_adr)->sin_addr)<<':'<<ntohs(((sockaddr_in *)&s_adr)->sin_port)<<endl;

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

    //get opt bufsize
    int skbufsize;
    optlen=sizeof(skbufsize);
    getsockopt(ssk,SOL_SOCKET,SO_SNDBUF,(void *)&skbufsize,&optlen);
    cout<<"obuf size: "<<skbufsize<<endl;
    getsockopt(ssk,SOL_SOCKET,SO_RCVBUF,(void *)&skbufsize,&optlen);
    cout<<"ibuf size: "<<skbufsize<<endl;

    c_adr_len=sizeof(c_adr);    
    cout<<"Waiting for connecting..."<<endl;
    csk=accept(ssk,(sockaddr *)&c_adr,&c_adr_len);
    if(csk==-1)
        ErrHdl("accept() err.");
    
    stringstream ss;
    ss<<inet_ntoa(c_adr.sin_addr);
    ss<<':'<<ntohs(c_adr.sin_port);
    ss>>clntname;
    ss.clear();
    cout<<clntname<<" connected."<<endl;

    recv_len=0;
    sgl_len=0;
    while(recv_len<sizeof(uint))
    {
        sgl_len=read(csk,(char *)&data_len+recv_len,sizeof(uint));
        if(sgl_len==-1)
        {
            cout<<"recv err."<<endl;
            break;
        }
        recv_len+=sgl_len;
    }
    ngr_len=(data_len-BUFLEN)>0?(data_len-BUFLEN):0;
    data_len=MIN(data_len,BUFLEN);
    cout<<"Receiving "<<data_len<<" bytes."<<endl;
    cout<<ngr_len<<" bytes lost."<<endl;

    recv_len=0;
    sgl_len=0;
    while(recv_len<data_len)
    {
        sgl_len=read(csk,buf+recv_len,data_len);
        if(sgl_len==-1)
        {
            cout<<"recv err."<<endl;
            break;
        }
        recv_len+=sgl_len;
    }
    buf[min(recv_len,BUFLEN-1)]='\0';
    
    string str(buf);
    //clean in_buf
    cout<<read(csk,buf,ngr_len)<<" bytes cleaned."<<endl;

    cout<<'['<<clntname<<"] "<<str<<endl;
    write(csk,"Server had got your message.",sizeof("Server had got your message."));

    close(csk);
    cout<<"clnt_socket closed."<<endl;

    close(ssk);
    cout<<"srv_socket closed."<<endl;

    return 0;
}