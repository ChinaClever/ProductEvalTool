/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "sn_devid.h"

Sn_DevId::Sn_DevId(QObject *parent) : Dev_Object(parent)
{
    mTypeDef = Sn_DevType::bulid();
    mItem = Cfg::bulid()->item;
}

Sn_DevId *Sn_DevId::bulid(QObject *parent)
{
    static Sn_DevId* sington = nullptr;
    if(sington == nullptr)
        sington = new Sn_DevId(parent);
    return sington;
}

void Sn_DevId::initReadType(sRtuItem &it)
{
    it.addr = mItem->addr;
    it.fn = 0x03;
    it.reg = 0xA001;
    it.num = 2;
}

bool Sn_DevId::analysDevType(uchar *buf, int len)
{
    bool ret = false;
    QString str = tr("读取设备类型成功");
    if(len != 4) {
        str = tr("读取设备类型失败：返回长度为%1").arg(len);
        return mPacket->updatePro(str, ret);
    }

    uint id = 0;
    for(int i=0; i<len; ++i) {
        id *= 256;  id += buf[i];
    }

    ret = mTypeDef->analysDevType(id);
    if(!ret) str = tr("不支持此设备类型 ID：%1").arg(id, 0, 16);

    return mPacket->updatePro(str, ret);
}

bool Sn_DevId::readDevId()
{
    sRtuItem it;
    initReadType(it);
    // QString str = tr("请确认插接箱的IN口与OUT口是否连接");
    int len = 0;
    static uchar recv[256] = {0};
    for(int i=0; i<5; ++i) {
        len = mModbus->readSn(it, recv);
        if(len==4) break; else if(!mPacket->delay(3+i)) break;
        // if(i>=0) {
        //     // emit TipImageSig(3); emit TipSig(str);
        // }
    }

    return analysDevType(recv, len);
}

bool Sn_DevId::readDevType()
{
    mDev->devType.dev_type.clear();
    QString str = tr("开始识别设备类型");
    bool ret = mPacket->updatePro(str);
    if(ret) {
        ret = readDevId();
    }

    return ret;
}
