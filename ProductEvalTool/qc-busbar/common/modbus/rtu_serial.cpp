/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "rtu_serial.h"

Rtu_Serial::Rtu_Serial(QObject *parent) : QThread(parent)
{
    mSerial = nullptr;
}

int Rtu_Serial::readSerial(quint8 *cmd, int secs)
{
    int rtn = 0;
    if(!mSerial) return rtn;

    sProgress *pro = sDataPacket::bulid()->getPro();
    for(int i=0; i<=secs; ++i) {
        if((pro->step < Test_Over)){
            rtn = mSerial->read(cmd, 1);
            if(rtn > 0) break;
        }
    }

    return rtn;
}

bool Rtu_Serial::writeSerial(quint8 *cmd, int len)
{
    bool ret = false;
    if(mSerial) {
        ret = mSerial->isOpened();
        if(ret) {
            mSerial->write(cmd, len);
        } else {
            qDebug() << "Rtu_Serial writeSerial err !" << ret;
        }
    } else {
        qDebug() << "Rtu_Serial is nullptr !" ;
    }

    return ret;
}

int Rtu_Serial::transmit(uchar *sent, int len, uchar *recv, int secs)
{
    int rtn = 0;
    bool ret = writeSerial(sent, len);
    if(ret) {
        rtn = readSerial(recv, secs);
    }

    return rtn;
}


ushort Rtu_Serial::calccrc (ushort crc, uchar crcbuf)
{
    uchar x, kkk=0;
    crc = crc^crcbuf;
    for(x=0;x<8;x++)
    {
        kkk = crc&1;
        crc >>= 1;
        crc &= 0x7FFF;
        if(kkk == 1)
            crc = crc^0xa001;
        crc=crc&0xffff;
    }
    return crc;
}

/**
  * 功　能：CRC校验
  * 入口参数：buf -> 缓冲区  len -> 长度
  * 返回值：CRC
  */
ushort Rtu_Serial::rtu_crc(uchar *buf, int len)
{
    ushort crc = 0xffff;
    for(int i=0; i<len; i++)
        crc = calccrc(crc, buf[i]);
    return crc;
}

/**
  * 功　能：XOR和校验
  * 入口参数：buf -> 缓冲区  len -> 长度
  * 返回值：XOR
  */
uchar Rtu_Serial::xorNum(uchar *buf, int len)
{
    uchar xorsum = 0x00;
    for(int i=0; i<len; i++)
        xorsum ^= buf[i];
    return xorsum;
}


bool Rtu_Serial::changeBaudRate()
{
    int br = mSerial->baudRate();
    switch (br) {
    case 9600: br = 19200; break;
    case 19200: br = 9600; break;
    default: br = 19200; break;
    }
    return mSerial->setBaudRate(br);;
}


