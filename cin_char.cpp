//cin will exceed border.

#include<iostream>
#include<string.h>
using namespace std;

#define LEN 10

int main()
{
    int i;
    char msg[LEN];
    string temp;
    cout<<"Enter msg(q to quit): ";
    cin>>temp;
    
    i=0;
    while(i<LEN&&i<temp.size())
    {
        msg[i]=temp.at(i);
        i++;
    }
    if(i==LEN)
        msg[i-1]='\0';
    else
        msg[i]='\0';
    cout<<strlen(msg)<<endl;
    cout<<msg<<endl;
    if(strcmp(msg,"aaa")==0)
        cout<<"good"<<endl;
    if("aaa"=="aaa")
        cout<<"goode"<<endl;

    return 0;
}
