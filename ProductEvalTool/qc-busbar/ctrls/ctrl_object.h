#ifndef CTRL_OBJECT_H
#define CTRL_OBJECT_H

#include "sn_serialnum.h"

class Ctrl_Object : public Dev_Object
{
    Q_OBJECT
public:
    explicit Ctrl_Object(QObject *parent = nullptr);
    virtual bool setCurTh(int i)=0;
    virtual bool setVolTh(int i)=0;
    virtual bool factorySet(){return true;}
    virtual void eleClean(){}

protected:
    bool readPduData(){return false;}
};

#endif // CTRL_OBJECT_H
