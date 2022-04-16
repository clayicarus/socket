#include<sys/socket.h>
#include<arpa/inet.h>

#include<signal.h>
#include<unistd.h>
#include<sys/times.h>

#include<iostream>
#include"trans_rule.h"
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
void CustomSend(int sock,const void *buf,uint send_size,void (*err_deal)(int sock,string errmsg,ostream &err));
void CustomSend(int sock,const DataHead *pdh ,uint head_size,const void *p_apdata,void (*err_deal)(int sock,string errmsg,ostream &err));
void CleanSockBuf(int sock,void (*err_deal)(int sock,string errmsg,ostream &err),int send_size=512);

void ErrHdl(string errmsg,ostream &err=cout);
void ErrHdl(int sock,string errmsg,ostream &err=cout);

void EnterId(char *des,size_t max_size,istream &is=cin);

#define BUFLEN 100

int main()
{
    uint send_len;
    string msg;

    DataHead sdh,rdh;
    uint headlen;
    int ngr_len;

    char buf[BUFLEN];

    sockaddr adr;
    int skfd;

    fd_set srv_recv,recv_copy;
    timeval timeout;
    int isrecv;

    sockaddr_v4(&adr,9090,"127.0.0.1");
    skfd=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(skfd==-1)
        ErrHdl("socket() err.");
    if(connect(skfd,&adr,sizeof(adr))==-1)
        ErrHdl(skfd,"connect() err.");

    cout<<"Enter your id (3 chars): ";
    EnterId(sdh.id,IDSIZE);

    FD_ZERO(&srv_recv);
    FD_SET(skfd,&srv_recv);
    timeout.tv_sec=0;
    timeout.tv_usec=0;

    while(1)
    {   
        recv_copy=srv_recv;

        isrecv=select(skfd+1,&recv_copy,0,0,&timeout);

        if(isrecv==-1)
            ErrHdl("select() err.");
        if(isrecv==0)
        {
            // cout<<"timeout"<<endl;
            // continue;
        }
        else
        {
            if(FD_ISSET(skfd,&recv_copy))
            {
                CycleRecv(skfd,sizeof(uint),(void *)&headlen,sizeof(uint),ErrHdl);
                CycleRecv(skfd,headlen,(void *)&rdh,sizeof(rdh),ErrHdl);
                if(rdh.type==MSG)
                {
                    ngr_len=CycleRecv(skfd,rdh.appendsize,buf,BUFLEN-1,ErrHdl);
                    buf[rdh.appendsize-ngr_len]='\0';
                    CleanSockBuf(skfd,ErrHdl);
                    cout<<"[Srv] "<<msg<<endl;
                }
                else if(rdh.type==EOF)
                {
                    close(skfd);
                    cout<<"Srv disconnected."<<endl;
                    break;
                }
            }
        }
        cout<<"Enter sth to send (null to exit): ";
        getline(cin,msg);

        if(msg.size()>0)
        {
            sdh.appendsize=msg.size();
            sdh.type=MSG;
            CustomSend(skfd,&sdh,sizeof(sdh),msg.c_str(),ErrHdl);
        }
        else
        {
            sdh.appendsize=0;
            sdh.type=EOF;
            CustomSend(skfd,&sdh,sizeof(sdh),msg.c_str(),ErrHdl);
            close(skfd);
            cout<<"Disconnected."<<endl;
            break;
        }
        
    }

    return 0;
}

void EnterId(char *des,size_t max_size,istream &is)
{
    int i;
    string temp;
    is>>temp;
    for(i=0;i<temp.size()&&i<max_size-1;i++)
        des[i]=temp.at(i);
    des[i]='\0';
    getline(is,temp);   //clean '\n'
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

void CustomSend(int sock,const void *buf,uint send_size,void (*err_deal)(int sock,string errmsg,ostream &err))
{
    if(write(sock,(void*)&send_size,sizeof(uint))==-1)
        err_deal(sock,"write() err.",cout);
    if(write(sock,buf,send_size)==-1)
        err_deal(sock,"write() err.",cout);
}

void CustomSend(int sock,const DataHead *pdh ,uint head_size,const void *p_apdata,void (*err_deal)(int sock,string errmsg,ostream &err))
{
    if(write(sock,(const void*)&head_size,sizeof(uint))==-1)
        err_deal(sock,"write_headsize err.",cout);
    if(write(sock,(const char *)pdh,head_size)==-1)
        err_deal(sock,"write_head err.",cout);
    if(write(sock,(const void *)p_apdata,pdh->appendsize)==-1)
        err_deal(sock,"write_append err.",cout);
}

void CleanSockBuf(int sock,void (*err_deal)(int sock,string errmsg,ostream &err),int buf_size)
{
    char *buf=new char[buf_size];
    while(recv(sock,buf,buf_size,MSG_DONTWAIT)>0)
        continue;
    delete[] buf;
}
