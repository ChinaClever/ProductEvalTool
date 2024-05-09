#ifndef DEV_SERIALNUM_H
#define DEV_SERIALNUM_H

#include "sn_devid.h"

struct sSnItem
{
    sSnItem() {}
    uchar devType[4];
    uchar date[4];
    ushort num;
    uchar pc;
    uchar exor;
    QString sn;
};


class Sn_SerialNum : public Dev_Object
{
    Q_OBJECT
    explicit Sn_SerialNum(QObject *parent = nullptr);
public:
    static Sn_SerialNum *bulid(QObject *parent = nullptr);
    bool snEnter();

protected:
    void toSnStr(sSnItem &it);
    bool checkSn(uchar *sn, int len);
    void initReadCmd(sRtuItem &item);
    void initDevType(sSnItem &it);
    bool readSn(sSnItem &itSn);
    bool readPduData(){return false;}
    bool analySn(uchar *sn, int len, sSnItem &it);

    void writeStatus(bool ret);
    void initWriteCmd(sRtuSetItems &item, uchar *data, int len);
    void createSn(sSnItem &it);
    int toSnData(sSnItem &it, uchar *data);
    bool writeSn(sSnItem &itSn);
private:
    sSnItem mSnItem;
    Sn_DevId *mTypeId;
};

#endif // DEV_SERIALNUM_H
