#include<sys/socket.h>
#include<arpa/inet.h>

#include<signal.h>
#include<unistd.h>

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

int CycleRecv(int sock,size_t data_len,void *desti,size_t desti_max_size,void (*err_deal)(int sock,string errmsg,ostream &err));
int CustomSend(int sock,const void *buf,uint send_size,void (*err_deal)(int sock,string errmsg,ostream &err));
void CleanSockBuf(int sock,void (*err_deal)(int sock,string errmsg,ostream &err),int send_size=512);

void ErrHdl(string errmsg,ostream &err=cout);
void ErrHdl(int sock,string errmsg,ostream &err=cout);

void EnterId(char *des,size_t max_size,istream &is=cin);

#define BUFLEN 100

int main()
{
    int ngr_len;
    uint send_len,recv_len;
    string msg;

    char buf[BUFLEN];

    sockaddr adr;
    int skfd;

    sockaddr_v4(&adr,9090,"127.0.0.1");
    skfd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(skfd==-1)
        ErrHdl("socket() err.");
    if(connect(skfd,&adr,sizeof(adr))==-1)
        ErrHdl(skfd,"connect() err.");

    while(1)
    {
        cout<<"Enter sth to send (null to exit): ";
        getline(cin,msg);

        // //send data_size
        CustomSend(skfd,msg.c_str(),msg.size(),ErrHdl);
        if(msg.size()==0)
        {
            cout<<"Disconnected."<<endl;
            close(skfd);
            cout<<"Socket closed."<<endl;
            break;
        }

        //get echo (ignore srv cut len)
        CycleRecv(skfd,sizeof(uint),(void*)&recv_len,sizeof(uint),ErrHdl);
        ngr_len=CycleRecv(skfd,recv_len,buf,BUFLEN-1,ErrHdl);
        buf[recv_len-ngr_len]='\0';
        msg=buf;

        //!!!!read_blocked.
        CleanSockBuf(skfd,ErrHdl);

        cout<<"[Server] "<<msg<<endl;
    }

    return 0;
}

void EnterId(char *des,size_t max_size,istream &is)
{
    int i;
    string temp;
    is>>temp;
    is.clear();
    for(i=0;i<temp.size()&&i<max_size-1;i++)
        des[i]=temp.at(i);
    des[i]='\0';
}

void ErrHdl(string errmsg,ostream &err)
{
    err<<errmsg<<endl;
    exit(-1);
}

void ErrHdl(int sock,string errmsg,ostream &err)
{
    close(sock);
    err<<errmsg<<endl;
    exit(-1);
}

int CycleRecv(int sock,size_t data_len,void *desti,size_t desti_max_size,void (*err_deal)(int sock,string errmsg,ostream &err))
{
    int sgl_len,recv_len,ngr_len;

    if(desti_max_size<data_len)
        ngr_len=data_len-desti_max_size;
    else
        ngr_len=0;
    data_len-=ngr_len;

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
