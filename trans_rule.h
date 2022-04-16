#ifndef _TRANS_RULE_H_
#define _TRANS_RULE_H_

#define IDSIZE 4

#define ISCNT 0
#define MSG 1

struct DataHead{
    char id[IDSIZE];
    int type;
    unsigned long long appendsize;
};

#endif