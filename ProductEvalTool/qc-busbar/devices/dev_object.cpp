/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "dev_object.h"

Dev_Object::Dev_Object(QObject *parent) : QThread(parent)
{
    mPacket = sDataPacket::bulid();
    mItem = Cfg::bulid()->item;
    mPro = mPacket->getPro();
    mDev = mPacket->getDev();
    mDt = &(mDev->devType);
    mBusData = get_share_mem();
    QTimer::singleShot(850,this,SLOT(initFunSlot()));
}

void Dev_Object::initFunSlot()
{
    setModbus(2);
}

void Dev_Object::setModbus(int id)
{
    mModbus = Rtu_Modbus::bulid(this)->get(id);
}

uchar *Dev_Object::toInt(uchar *ptr, int line, uint *value)
{
    for(int i=0; i<line; ++i) {
        value[i] =  (*ptr) * 256 + *(ptr+1);  ptr += 2; // 读取电能高8位
        value[i] <<= 16; // 左移8位
        value[i] +=  (*ptr) * 256 + *(ptr+1);  ptr += 2; // 读取电能底8位
    }

    return ptr;
}

uchar *Dev_Object::toOutputEle(uchar *ptr, int line, uint *value)
{
    for(int i=0; i<line; ++i) {
        value[i] = (*ptr++) << 16;
        value[i] += (*ptr++) << 8;
        value[i] += *ptr++;
    }

    return ptr;
}

uchar *Dev_Object::toShort(uchar *ptr, int line, ushort *value)
{
    for(int i=0; i<line; ++i) {
        value[i] =  (*ptr) * 256 + *(ptr+1);  ptr += 2;
    }

    return ptr;
}

uchar *Dev_Object::toChar(uchar *ptr, int line, uchar *value)
{
    for(int i=0; i<line; ++i) {
        value[i] = *(ptr++); // 读取电压
    }

    return ptr;
}

uchar *Dev_Object::toShort10(uchar *ptr, int line, ushort *value,int v)
{
    for(int i=0; i<line; ++i) {
        value[i] =  ((*ptr) * 256 + *(ptr+1))*v;  ptr += 2;
    }

    return ptr;
}

uchar *Dev_Object::toThreshold(uchar *ptr, int line, sDataUnit &unit)
{
    for(int i=0; i<line; ++i) {
        ptr = toShort(ptr, 1, &unit.min[i]);
        ptr = toShort(ptr, 1, &unit.max[i]);
    }

    return ptr;
}

uchar *Dev_Object::toThreshold10(uchar *ptr, int line, sDataUnit &unit)
{
    for(int i=0; i<line; ++i) {
        ptr = toShort10(ptr, 1, &unit.min[i] , 10);
        ptr = toShort10(ptr, 1, &unit.max[i] , 10);
    }

    return ptr;
}

uchar *Dev_Object::toEnvTh(uchar *ptr, sDataUnit &unit)
{
    unit.max[0] = *ptr++;
    unit.min[0] = *ptr++;
    return ptr;
}

ushort Dev_Object::getShort(uchar *ptr)
{
    return  (*ptr) * 256 + *(ptr+1);
}

uchar *Dev_Object::getShort(uchar *ptr, int line, uchar *value)
{
    for(int i=0; i<line; ++i) {
        value[i] = getShort(ptr); ptr += 2;
    }

    return  ptr;
}

ushort Dev_Object::rtu_crc(uchar *buf, int len)
{
    ushort crc = 0xffff;
    for(int i=0; i<len; i++)
        crc = calccrc(crc, buf[i]);
    return crc;
}

ushort Dev_Object::calccrc(ushort crc, uchar crcbuf)
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
