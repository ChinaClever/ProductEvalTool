#include "rk_serial.h"

Rk_Serial::Rk_Serial(QObject *parent) : QThread(parent)
{
    mSerial = nullptr;
    mModbus = Ad_Modbus::bulid(this);
}

int Rk_Serial::transmit(uchar *recv)
{
    if(!mSerial) return 0;
    static uchar sent[] = {0x55, 0xAA, 0x00, 0x01};
    int len = mSerial->transmit(sent, sizeof(sent), recv, 1);
    if(len < 10) { qDebug() << " Rk_Serial read len err" << len; return 0; }
    else if(memcmp(sent, recv, 3)) { qDebug() << " Rk_Serial read Data err" << len; return 0; }
    return len;
}

QString Rk_Serial::rkType(int id)
{
    QString str;
    switch (id) {
    case 0: str = "RK9800N"; break;
    case 1: str = "RK9901N"; break;
    case 2: str = "RK9813N"; break;
    case 3: str = "RK9940N"; break;
    case 4: str = "RK9980N"; break;
    }
    return str;
}

bool Rk_Serial::readPacket(sRkItem &it)
{
    bool ret = true;
    static uchar recv[2048] = {0};
    int len = transmit(recv);
    if(len > 0) {
        QByteArray array((char *)recv, len);
        QDataStream rxStream(array);
        rxStream.setByteOrder(QDataStream::LittleEndian);
        rxStream >> it.header >> it.flag >> it.rkId >> it.enAlarm
                >> it.vol >> it.curUnit >> it.cur >> it.pow
                >> it.ele >> it.hz >> it.pf >> it.crc;
        it.rkStr = rkType(it.rkId);
    } else ret = false;

    return ret;
}

bool Rk_Serial::readPacket1(sRkItem &it)
{
    sRtuItemRK item1;
    sRtuItemRK item2;
    item1.addr = 0x01;
    item1.fn = 0x03;
    item1.reg = 0x0063;
    item1.num = 12;

    item2.addr = 0x01;
    item2.fn = 0x03;
    item2.reg = 0x00B4;
    item2.num = 6;

    bool ret = true;
    static uchar recv1[1024] = {0};
    static uchar recv2[2048] = {0};

    mModbus->init(mSerial);

    int len = mModbus->read(item1,recv1);
    qDebug()<<len<<"???";
    if(len > 0) {
        QByteArray array((char *)recv1, len);
        QDataStream rxStream(array);
        //rxStream.setByteOrder(QDataStream::LittleEndian);
        rxStream >> it.vola[0] >> it.vola[1] >>it.vola[2]
                >> it.cura[0] >> it.cura[1] >> it.cura[2];
    } else ret = false;

    len = mModbus->read(item2,recv2);
    if(len > 0) {
        QByteArray array((char *)recv2, len);
        QDataStream rxStream(array);
        //rxStream.setByteOrder(QDataStream::LittleEndian);
        rxStream >> it.powa[0] >> it.powa[1] >> it.powa[2];
    } else ret = false;

    return ret;
}

bool Rk_Serial::clearEle()
{
    bool ret = true;
    static uchar sent[] = {0x55, 0xAA, 0x03, 0xFE};
    if(mSerial) {
        mSerial->write(sent, sizeof(sent));
    } else {
        ret = false;
    }

    return ret;
}
