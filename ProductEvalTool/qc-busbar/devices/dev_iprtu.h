#ifndef DEV_IPTHREAD_H
#define DEV_IPTHREAD_H

#include "dev_ipsnmp.h"

class Dev_IpRtu : public Dev_Object
{
    Q_OBJECT
protected:
    explicit Dev_IpRtu(QObject *parent = nullptr);
public:
    static Dev_IpRtu *bulid(QObject *parent = nullptr);
    bool readPduData();
    int m_Line;

protected:
    int recvLine(int len);
    void initRtuItem(sRtuItem &it);
    bool recvPacket(uchar *buf, int len);

    int recvDataV1(uchar *ptr);
    int recvDataV3(uchar *ptr);
    uchar *getSwitch(uchar *ptr, int line, uchar *value);
    uchar *calcaPow(uchar *ptr, int line, ushort *value , ushort *vol, ushort *cur);
    uchar *toCurThreshold(uchar *ptr, int line, sDataUnit &unit);


    void initRtuBaseValueItem(sRtuItem &it);
    void initRtuLineNumItem(sRtuItem &it);
    void initRtuCurRangeItem(sRtuItem &it);
    void initRtuVolRangeItem(sRtuItem &it);
    void initRtuTemRangeItem(sRtuItem &it);
    void initRtuHumRangeItem(sRtuItem &it);
    void initRtuCurTemHumItem(sRtuItem &it);

    int recvBaseValuePacket(uchar *buf);
    int recvLineNumPacket(uchar *buf);
    int recvCurRangePacket(uchar *buf);
    int recvVolRangePacket(uchar *buf);
    int recvTemRangePacket(uchar *buf);
    int recvHumRangePacket(uchar *buf);
    int recvCurTemHumPacket(uchar *buf);
};

#endif // DEV_IPTHREAD_H
