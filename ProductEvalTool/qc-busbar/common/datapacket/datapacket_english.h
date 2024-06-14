#ifndef DATAPACKET_ENGLISH_H
#define DATAPACKET_ENGLISH_H
#include "datapacket.h"

class datapacket_English
{
    datapacket_English();
public:
    static datapacket_English *bulid();

    void init();
    sProgress *getPro() {return pro;}
    sDevData *getDev(int id=1) {return dev[id];}

protected:
    void clear(int id=1);
    sDevData *dev[DEV_NUM];
    sProgress *pro;

};

#endif // DATAPACKET_ENGLISH_H
