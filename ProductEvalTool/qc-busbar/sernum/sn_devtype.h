#ifndef DEV_DEFTYPE_H
#define DEV_DEFTYPE_H
#include "dev_source.h"

class Sn_DevType
{
    Sn_DevType();
public:
    static Sn_DevType *bulid();
    bool analysDevType(uint id);

protected:
    QString getDevStr(uint id);
    int getDevType(const QString &str);
    int getAcType(const QString &str);
    int getColMode(const QString &str);
    int getLineNum(const QString &str);
    int getSerie(const QString &str);
    int getSceenType(const QString &str);

private:
    sDevType *mDt;
};

#endif // DEV_DEFTYPE_H
