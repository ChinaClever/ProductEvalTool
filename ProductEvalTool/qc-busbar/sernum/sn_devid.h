#ifndef DEV_TYPEID_H
#define DEV_TYPEID_H
#include "sn_devtype.h"

class Sn_DevId: public Dev_Object
{
    Q_OBJECT
    explicit Sn_DevId(QObject *parent = nullptr);
public:
    static Sn_DevId *bulid(QObject *parent = nullptr);

    bool readDevType();
protected:
    bool readDevId();
    void initReadType(sRtuItem &it);
    bool readPduData(){return false;}
    bool analysDevType(uchar *buf, int len);

private:
    Sn_DevType *mTypeDef;
};

#endif // DEV_TYPEID_H
